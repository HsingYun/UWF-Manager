/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "WmiClient.h"

#include <comdef.h>
#include <oleauto.h>
#include <wbemidl.h>
#include <windows.h>

#include <atomic>
#include <cmath>
#include <condition_variable>
#include <format>
#include <limits>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>

#include "../../core/Config.h"
#include "../../util/Log.h"
#include "../../util/StringUtil.h"
#include "WmiResult.h"

namespace uwf {

namespace {

// 把 HRESULT 渲染成 "描述 (0xXXXXXXXX)" 的形式，方便日志。
std::string hrText(const HRESULT hr) {
  _com_error err(hr);
  return std::format("{} (0x{:08x})", wideToUtf8(err.ErrorMessage()), static_cast<uint32_t>(hr));
}

WmiValue variantToValue(const VARIANT& v, const CIMTYPE cimType = 0) {
  switch (v.vt) {
    case VT_NULL:
    case VT_EMPTY:
      return {};
    case VT_BOOL:
      return WmiValue::fromBool(v.boolVal != VARIANT_FALSE);
    case VT_I2:
      if (cimType == CIM_UINT16) return WmiValue::fromUInt(static_cast<uint16_t>(v.iVal));
      return WmiValue::fromInt(v.iVal);
    case VT_I4:
      // WMI Automation 把 CIM_UINT32 装箱成 VT_I4。只看 VARTYPE 会把
      // 0x8004xxxx 这类合法的 UInt32（UWF ReturnValue 常用 HRESULT 位型）
      // 误解成负数；IWbemClassObject::Next 同时返回的 CIMTYPE 才是字段语义。
      if (cimType == CIM_UINT32) return WmiValue::fromUInt(static_cast<uint32_t>(v.lVal));
      return WmiValue::fromInt(v.lVal);
    case VT_UI2:
      return WmiValue::fromUInt(v.uiVal);
    case VT_UI4:
      return WmiValue::fromUInt(v.ulVal);
    case VT_I8:
      if (cimType == CIM_UINT64) return WmiValue::fromUInt(static_cast<uint64_t>(v.llVal));
      return WmiValue::fromInt(v.llVal);
    case VT_UI8:
      return WmiValue::fromUInt(v.ullVal);
    case VT_R4:
      return WmiValue::fromDouble(v.fltVal);
    case VT_R8:
      return WmiValue::fromDouble(v.dblVal);
    case VT_BSTR:
      return v.bstrVal ? WmiValue::fromString(wideToUtf8(v.bstrVal)) : WmiValue::fromString({});
    case VT_DATE:
      return WmiValue::fromDouble(v.date);
    default:
      if (v.vt & VT_ARRAY) return WmiValue::fromString("<array>");
      return {};
  }
}

// WmiValue → VARIANT（调用方负责 VariantClear）。只支持标量。
bool valueToVariant(const WmiValue& src, VARIANT& v) {
  VariantInit(&v);
  switch (src.kind()) {
    case WmiValue::Kind::Bool:
      v.vt = VT_BOOL;
      v.boolVal = src.toBool() ? VARIANT_TRUE : VARIANT_FALSE;
      return true;
    case WmiValue::Kind::Int:
      if (src.toInt64() < std::numeric_limits<LONG>::min() || src.toInt64() > std::numeric_limits<LONG>::max()) return false;
      v.vt = VT_I4;
      v.lVal = static_cast<LONG>(src.toInt64());
      return true;
    case WmiValue::Kind::UInt:
      // WMI 对 CIM_UINT32 入参的 VARIANT 装箱约定是 VT_I4（参见微软
      // 文档 "Numbers (WMI)"：learn.microsoft.com/windows/win32/wmisdk/numbers），
      // 传 VT_UI4 会让 IWbemClassObject::Put 退回
      // WBEM_E_INVALID_PARAMETER (0x80041008)，进而 ExecMethod 失败。
      // CIM_UINT32 的最大值（4G-1）超过 LONG 可表示的范围，但 UWF 的所有
      // UInt32 入参（type/size/threshold）都远小于 INT_MAX，cast 安全。
      if (src.toULongLong() > static_cast<uint64_t>(std::numeric_limits<LONG>::max())) return false;
      v.vt = VT_I4;
      v.lVal = static_cast<LONG>(src.toULongLong());
      return true;
    case WmiValue::Kind::Double:
      if (!std::isfinite(src.toDouble())) return false;
      v.vt = VT_R8;
      v.dblVal = src.toDouble();
      return true;
    case WmiValue::Kind::String: {
      const auto w = utf8ToWide(src.toString());
      v.vt = VT_BSTR;
      v.bstrVal = SysAllocString(w.c_str());
      return v.bstrVal != nullptr;
    }
    default:
      return false;
  }
}

bool readObjectProps(IWbemClassObject* obj, WmiRow& row, std::string& error, const bool requirePath, HRESULT* failure = nullptr) {
  row.clear();
  if (failure) *failure = S_OK;
  if (!obj) {
    if (failure) *failure = E_POINTER;
    error = "WMI returned a null object";
    return false;
  }

  HRESULT hr = obj->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
  if (FAILED(hr)) {
    if (failure) *failure = hr;
    error = std::format("BeginEnumeration failed: {}", hrText(hr));
    return false;
  }
  while (true) {
    BSTR name = nullptr;
    VARIANT val;
    CIMTYPE type = 0;
    VariantInit(&val);
    const HRESULT hn = obj->Next(0, &name, &val, &type, nullptr);
    if (hn == WBEM_S_NO_MORE_DATA) {
      if (name) SysFreeString(name);
      VariantClear(&val);
      break;
    }
    if (hn != WBEM_S_NO_ERROR) {
      if (name) SysFreeString(name);
      VariantClear(&val);
      obj->EndEnumeration();
      if (failure) *failure = hn;
      error = std::format("property enumeration failed: {}", hrText(hn));
      row.clear();
      return false;
    }
    row.emplace(wideToUtf8(name), variantToValue(val, type));
    SysFreeString(name);
    VariantClear(&val);
  }
  hr = obj->EndEnumeration();
  if (FAILED(hr)) {
    if (failure) *failure = hr;
    error = std::format("EndEnumeration failed: {}", hrText(hr));
    row.clear();
    return false;
  }

  VARIANT vPath;
  VariantInit(&vPath);
  BSTR pn = SysAllocString(L"__PATH");
  hr = obj->Get(pn, 0, &vPath, nullptr, nullptr);
  SysFreeString(pn);
  if (SUCCEEDED(hr) && vPath.vt == VT_BSTR && vPath.bstrVal) {
    row.emplace("__PATH", WmiValue::fromString(wideToUtf8(vPath.bstrVal)));
  } else if (requirePath) {
    const VARTYPE pathType = vPath.vt;
    VariantClear(&vPath);
    if (FAILED(hr)) {
      if (failure) *failure = hr;
      error = std::format("Get(__PATH) failed for a locatable WMI object: {}", hrText(hr));
    } else {
      if (failure) *failure = E_UNEXPECTED;
      error = std::format("locatable WMI object has no string __PATH (VARIANT type {})", static_cast<unsigned>(pathType));
    }
    row.clear();
    return false;
  }
  VariantClear(&vPath);
  return true;
}

// 处理 ExecMethod 的 out 参数里 VT_ARRAY 类型字段：展开为 vector<WmiRow>。
// - 元素是 IUnknown → QI 成 IWbemClassObject，展开为行；
// - 元素是 BSTR（EmbeddedInstance 常见） → 放进行的 __MOF 字段。
bool expandArrayVariant(const VARIANT& v, std::vector<WmiRow>& rows, std::string& error) {
  rows.clear();
  if (!(v.vt & VT_ARRAY) || !v.parray) {
    error = "output value is not an array";
    return false;
  }

  SAFEARRAY* sa = v.parray;
  if (SafeArrayGetDim(sa) != 1) {
    error = "only one-dimensional WMI output arrays are supported";
    return false;
  }
  VARTYPE vt = VT_EMPTY;
  HRESULT hr = SafeArrayGetVartype(sa, &vt);
  if (FAILED(hr)) {
    error = std::format("SafeArrayGetVartype failed: {}", hrText(hr));
    return false;
  }
  LONG lb = 0, ub = 0;
  hr = SafeArrayGetLBound(sa, 1, &lb);
  if (FAILED(hr)) {
    error = std::format("SafeArrayGetLBound failed: {}", hrText(hr));
    return false;
  }
  hr = SafeArrayGetUBound(sa, 1, &ub);
  if (FAILED(hr)) {
    error = std::format("SafeArrayGetUBound failed: {}", hrText(hr));
    return false;
  }

  for (LONG i = lb; i <= ub; ++i) {
    if (vt == VT_UNKNOWN) {
      IUnknown* unk = nullptr;
      hr = SafeArrayGetElement(sa, &i, &unk);
      if (FAILED(hr)) {
        if (unk) unk->Release();
        error = std::format("SafeArrayGetElement({}) failed: {}", i, hrText(hr));
        return false;
      }
      if (!unk) {
        error = std::format("SafeArrayGetElement({}) returned a null IUnknown", i);
        return false;
      }
      IWbemClassObject* item = nullptr;
      hr = unk->QueryInterface(IID_IWbemClassObject, reinterpret_cast<LPVOID*>(&item));
      unk->Release();
      if (FAILED(hr)) {
        if (item) item->Release();
        error = std::format("array element is not an IWbemClassObject: {}", hrText(hr));
        return false;
      }
      if (!item) {
        error = "QueryInterface(IWbemClassObject) succeeded without returning an object";
        return false;
      }
      WmiRow row;
      const bool decoded = readObjectProps(item, row, error, false);
      item->Release();
      if (!decoded) return false;
      rows.push_back(std::move(row));
    } else if (vt == VT_BSTR) {
      BSTR s = nullptr;
      hr = SafeArrayGetElement(sa, &i, &s);
      if (FAILED(hr)) {
        if (s) SysFreeString(s);
        error = std::format("SafeArrayGetElement({}) failed: {}", i, hrText(hr));
        return false;
      }
      if (!s) {
        error = std::format("SafeArrayGetElement({}) returned a null BSTR", i);
        return false;
      }
      WmiRow row;
      row.emplace("__MOF", WmiValue::fromString(wideToUtf8(s)));
      rows.push_back(std::move(row));
      SysFreeString(s);
    } else {
      error = std::format("unsupported WMI output array element type {}", static_cast<unsigned>(vt));
      return false;
    }
  }
  return true;
}

std::string classNameFromObjectPath(const std::string& objectPath) {
  std::string className = objectPath;
  if (className.starts_with("\\\\")) {
    if (const auto colon = className.find(':'); colon != std::string::npos) className = className.substr(colon + 1);
  }
  if (const auto dot = className.find('.'); dot != std::string::npos) className.resize(dot);
  return className;
}

struct PreparedMethodCall {
  BSTR objectPath = nullptr;
  BSTR methodName = nullptr;
  IWbemClassObject* inputs = nullptr;
  std::string className;

  ~PreparedMethodCall() {
    if (inputs) inputs->Release();
    if (methodName) SysFreeString(methodName);
    if (objectPath) SysFreeString(objectPath);
  }
};

bool prepareMethodCall(IWbemServices* services, const std::string& objectPath, const std::string& methodName, const WmiRow& inputs,
                       PreparedMethodCall& prepared, WmiMethodResult& result) {
  if (!services || objectPath.empty()) {
    result.error = services ? "object path is empty (call read()/readAll() on the row first)" : "WMI services proxy is unavailable";
    return false;
  }

  prepared.className = classNameFromObjectPath(objectPath);
  const auto classW = utf8ToWide(prepared.className);
  BSTR classBstr = SysAllocString(classW.c_str());
  IWbemClassObject* classObject = nullptr;
  HRESULT hr = services->GetObject(classBstr, 0, nullptr, &classObject, nullptr);
  SysFreeString(classBstr);
  if (FAILED(hr) || !classObject) {
    if (classObject) classObject->Release();
    const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
    result.hresult = static_cast<int32_t>(failure);
    result.error = std::format("GetObject({}) failed: {}", prepared.className, hrText(failure));
    return false;
  }

  const auto methodW = utf8ToWide(methodName);
  prepared.methodName = SysAllocString(methodW.c_str());
  IWbemClassObject* signature = nullptr;
  hr = classObject->GetMethod(prepared.methodName, 0, &signature, nullptr);
  classObject->Release();
  if (FAILED(hr)) {
    if (signature) signature->Release();
    result.hresult = static_cast<int32_t>(hr);
    result.error = std::format("GetMethod({}::{}) failed: {}", prepared.className, methodName, hrText(hr));
    return false;
  }

  if (signature) {
    hr = signature->SpawnInstance(0, &prepared.inputs);
    signature->Release();
    if (FAILED(hr)) {
      result.hresult = static_cast<int32_t>(hr);
      result.error = std::format("SpawnInstance({}::{}) failed: {}", prepared.className, methodName, hrText(hr));
      return false;
    }
  }

  if (!inputs.empty() && !prepared.inputs) {
    result.hresult = static_cast<int32_t>(WBEM_E_INVALID_METHOD_PARAMETERS);
    result.error = std::format("{}::{} has no input signature", prepared.className, methodName);
    return false;
  }
  for (const auto& [name, value] : inputs) {
    VARIANT variant;
    if (!valueToVariant(value, variant)) {
      result.hresult = E_INVALIDARG;
      result.error = std::format("unsupported input value for {}::{} parameter {}", prepared.className, methodName, name);
      return false;
    }
    const auto nameW = utf8ToWide(name);
    BSTR nameBstr = SysAllocString(nameW.c_str());
    const HRESULT putHr = prepared.inputs->Put(nameBstr, 0, &variant, 0);
    SysFreeString(nameBstr);
    VariantClear(&variant);
    if (FAILED(putHr)) {
      result.hresult = static_cast<int32_t>(putHr);
      result.error = std::format("inParams->Put({}) on {} failed: {}", name, methodName, hrText(putHr));
      return false;
    }
  }

  const auto pathW = utf8ToWide(objectPath);
  prepared.objectPath = SysAllocString(pathW.c_str());
  return true;
}

bool decodeMethodOutput(IWbemClassObject* output, const std::string& className, const std::string& methodName, WmiMethodResult& result) {
  if (!output) {
    result.error = std::format("{}::{} returned no output parameters", className, methodName);
    return false;
  }

  HRESULT hr = output->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
  if (FAILED(hr)) {
    result.hresult = static_cast<int32_t>(hr);
    result.error = std::format("output BeginEnumeration failed: {}", hrText(hr));
    return false;
  }
  while (true) {
    BSTR name = nullptr;
    VARIANT value;
    CIMTYPE type = 0;
    VariantInit(&value);
    const HRESULT nextHr = output->Next(0, &name, &value, &type, nullptr);
    if (nextHr == WBEM_S_NO_MORE_DATA) {
      if (name) SysFreeString(name);
      VariantClear(&value);
      break;
    }
    if (nextHr != WBEM_S_NO_ERROR) {
      if (name) SysFreeString(name);
      VariantClear(&value);
      output->EndEnumeration();
      result.hresult = static_cast<int32_t>(nextHr);
      result.error = std::format("output property enumeration failed: {}", hrText(nextHr));
      return false;
    }

    const std::string key = wideToUtf8(name);
    if (value.vt & VT_ARRAY) {
      std::vector<WmiRow> values;
      std::string arrayError;
      if (!expandArrayVariant(value, values, arrayError)) {
        SysFreeString(name);
        VariantClear(&value);
        output->EndEnumeration();
        result.hresult = E_UNEXPECTED;
        result.error = std::format("failed to decode output array {}: {}", key, arrayError);
        return false;
      }
      result.outArrays.emplace(key, std::move(values));
    } else {
      result.outParams.emplace(key, variantToValue(value, type));
    }
    SysFreeString(name);
    VariantClear(&value);
  }

  hr = output->EndEnumeration();
  if (FAILED(hr)) {
    result.hresult = static_cast<int32_t>(hr);
    result.error = std::format("output EndEnumeration failed: {}", hrText(hr));
    return false;
  }

  const auto returnIt = result.outParams.find("ReturnValue");
  bool converted = false;
  if (returnIt != result.outParams.end()) result.returnValue = returnIt->second.toUInt(&converted);
  result.returnValuePresent = returnIt != result.outParams.end() && converted;
  if (!result.returnValuePresent) {
    result.error = std::format("{}::{} returned an incomplete response without a valid ReturnValue", className, methodName);
    return false;
  }
  if (result.returnValue != 0) result.error = std::format("{}::{} returned {}", className, methodName, result.returnValue);
  return true;
}

struct AsyncMethodState {
  ~AsyncMethodState() {
    if (output) output->Release();
  }

  std::mutex mutex;
  std::condition_variable changed;
  bool completed = false;
  HRESULT status = E_UNEXPECTED;
  IWbemClassObject* output = nullptr;
};

class AsyncMethodSink final : public IWbemObjectSink {
 public:
  explicit AsyncMethodSink(std::shared_ptr<AsyncMethodState> state) : m_state(std::move(state)) {}

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override {
    if (!object) return E_POINTER;
    if (iid == IID_IUnknown || iid == IID_IWbemObjectSink) {
      *object = static_cast<IWbemObjectSink*>(this);
      AddRef();
      return S_OK;
    }
    *object = nullptr;
    return E_NOINTERFACE;
  }

  ULONG STDMETHODCALLTYPE AddRef() override { return ++m_refs; }
  ULONG STDMETHODCALLTYPE Release() override {
    const ULONG refs = --m_refs;
    if (refs == 0) delete this;
    return refs;
  }

  HRESULT STDMETHODCALLTYPE Indicate(const LONG count, IWbemClassObject** objects) override {
    if (count != 1 || !objects || !objects[0]) return static_cast<HRESULT>(WBEM_E_FAILED);
    std::lock_guard lock(m_state->mutex);
    if (m_state->output) return static_cast<HRESULT>(WBEM_E_FAILED);
    objects[0]->AddRef();
    m_state->output = objects[0];
    return WBEM_S_NO_ERROR;
  }

  HRESULT STDMETHODCALLTYPE SetStatus(const LONG flags, const HRESULT result, BSTR, IWbemClassObject*) override {
    if (flags != WBEM_STATUS_COMPLETE) return WBEM_S_NO_ERROR;
    {
      std::lock_guard lock(m_state->mutex);
      m_state->status = result;
      m_state->completed = true;
    }
    m_state->changed.notify_all();
    return WBEM_S_NO_ERROR;
  }

 private:
  std::atomic<ULONG> m_refs{1};
  std::shared_ptr<AsyncMethodState> m_state;
};

}  // namespace

WmiValue WmiValue::fromBool(bool v) {
  WmiValue x;
  x.m_kind = Kind::Bool;
  x.m_bool = v;
  return x;
}
WmiValue WmiValue::fromInt(int64_t v) {
  WmiValue x;
  x.m_kind = Kind::Int;
  x.m_int = v;
  return x;
}
WmiValue WmiValue::fromUInt(uint64_t v) {
  WmiValue x;
  x.m_kind = Kind::UInt;
  x.m_uint = v;
  return x;
}
WmiValue WmiValue::fromDouble(double v) {
  WmiValue x;
  x.m_kind = Kind::Double;
  x.m_double = v;
  return x;
}
WmiValue WmiValue::fromString(std::string v) {
  WmiValue x;
  x.m_kind = Kind::String;
  x.m_string = std::move(v);
  return x;
}

bool WmiValue::toBool(bool def) const {
  switch (m_kind) {
    case Kind::Bool:
      return m_bool;
    case Kind::Int:
      return m_int != 0;
    case Kind::UInt:
      return m_uint != 0;
    case Kind::Double:
      return m_double != 0.0;
    case Kind::String: {
      if (m_string.empty()) return def;
      if (m_string == "true" || m_string == "TRUE" || m_string == "True" || m_string == "1") return true;
      if (m_string == "false" || m_string == "FALSE" || m_string == "False" || m_string == "0") return false;
      return def;
    }
    default:
      return def;
  }
}

int32_t WmiValue::toInt(bool* ok, int32_t def) const {
  // toInt64 失败时返回的 def 必落在 int32 范围内，不会误入下面的越界分支；
  // 此处只拦真正超出 int32 的值——避免静默截断且 ok 仍为 true。
  const int64_t v = toInt64(ok, def);
  if (v < INT32_MIN || v > INT32_MAX) {
    if (ok) *ok = false;
    return def;
  }
  return static_cast<int32_t>(v);
}

uint32_t WmiValue::toUInt(bool* ok, uint32_t def) const {
  const uint64_t v = toULongLong(ok, def);
  if (v > UINT32_MAX) {
    if (ok) *ok = false;
    return def;
  }
  return static_cast<uint32_t>(v);
}

int64_t WmiValue::toInt64(bool* ok, int64_t def) const {
  if (ok) *ok = true;
  switch (m_kind) {
    case Kind::Bool:
      return m_bool ? 1 : 0;
    case Kind::Int:
      return m_int;
    case Kind::UInt:
      if (m_uint > static_cast<uint64_t>(INT64_MAX)) {
        if (ok) *ok = false;
        return def;
      }
      return static_cast<int64_t>(m_uint);
    case Kind::Double:
      if (!std::isfinite(m_double) || m_double < static_cast<double>(INT64_MIN) || m_double >= -static_cast<double>(INT64_MIN)) {
        if (ok) *ok = false;
        return def;
      }
      return static_cast<int64_t>(m_double);
    case Kind::String: {
      try {
        size_t pos = 0;
        int64_t v = std::stoll(m_string, &pos, 10);
        if (pos != m_string.size()) {
          if (ok) *ok = false;
          return def;
        }
        return v;
      } catch (...) {
        if (ok) *ok = false;
        return def;
      }
    }
    default:
      if (ok) *ok = false;
      return def;
  }
}

uint64_t WmiValue::toULongLong(bool* ok, uint64_t def) const {
  if (ok) *ok = true;
  switch (m_kind) {
    case Kind::Bool:
      return m_bool ? 1u : 0u;
    case Kind::Int:
      if (m_int < 0) {
        if (ok) *ok = false;
        return def;
      }
      return static_cast<uint64_t>(m_int);
    case Kind::UInt:
      return m_uint;
    case Kind::Double:
      if (!std::isfinite(m_double) || m_double < 0 || m_double >= -static_cast<double>(INT64_MIN) * 2.0) {
        if (ok) *ok = false;
        return def;
      }
      return static_cast<uint64_t>(m_double);
    case Kind::String: {
      if (m_string.starts_with('-')) {
        if (ok) *ok = false;
        return def;
      }
      try {
        size_t pos = 0;
        uint64_t v = std::stoull(m_string, &pos, 10);
        if (pos != m_string.size()) {
          if (ok) *ok = false;
          return def;
        }
        return v;
      } catch (...) {
        if (ok) *ok = false;
        return def;
      }
    }
    default:
      if (ok) *ok = false;
      return def;
  }
}

double WmiValue::toDouble(bool* ok, double def) const {
  if (ok) *ok = true;
  switch (m_kind) {
    case Kind::Bool:
      return m_bool ? 1.0 : 0.0;
    case Kind::Int:
      return static_cast<double>(m_int);
    case Kind::UInt:
      return static_cast<double>(m_uint);
    case Kind::Double:
      return m_double;
    case Kind::String: {
      try {
        size_t pos = 0;
        double v = std::stod(m_string, &pos);
        if (pos != m_string.size()) {
          if (ok) *ok = false;
          return def;
        }
        return v;
      } catch (...) {
        if (ok) *ok = false;
        return def;
      }
    }
    default:
      if (ok) *ok = false;
      return def;
  }
}

std::string WmiValue::toString() const {
  switch (m_kind) {
    case Kind::Bool:
      return m_bool ? "true" : "false";
    case Kind::Int:
      return std::format("{}", m_int);
    case Kind::UInt:
      return std::format("{}", m_uint);
    case Kind::Double:
      return std::format("{}", m_double);
    case Kind::String:
      return m_string;
    default:
      return {};
  }
}

struct WmiThreadState {
  bool ready = false;
  HRESULT hresult = E_UNEXPECTED;
  std::string error;
};

struct WmiSession::Impl {
  Impl(std::string path, const WmiThreadState* state) : namespacePath(std::move(path)), threadState(state) {}

  class OperationGuard {
   public:
    explicit OperationGuard(Impl& owner) : m_owner(owner), m_acquired(!owner.operationActive) {
      if (m_acquired) m_owner.operationActive = true;
    }
    ~OperationGuard() {
      if (m_acquired) m_owner.operationActive = false;
    }
    [[nodiscard]] explicit operator bool() const { return m_acquired; }

   private:
    Impl& m_owner;
    bool m_acquired;
  };

  IWbemLocator* locator = nullptr;
  IWbemServices* services = nullptr;
  std::string namespacePath;
  const WmiThreadState* threadState = nullptr;
  bool connected = false;
  bool operationActive = false;
  HRESULT lastHresult = S_OK;

  void reset() {
    if (services) services->Release();
    if (locator) locator->Release();
    services = nullptr;
    locator = nullptr;
    connected = false;
  }

  ~Impl() { reset(); }
};

WmiSession::WmiSession(std::string namespacePath, const WmiThreadState* threadState) : d(std::make_unique<Impl>(std::move(namespacePath), threadState)) {}
WmiSession::~WmiSession() = default;

void WmiSession::invalidate() const { d->reset(); }

bool WmiSession::ensureConnected(std::string* error) const {
  Impl::OperationGuard operation(*d);
  if (!operation) {
    if (error) *error = "WMI session rejected a reentrant connection request on the same thread";
    return false;
  }
  return ensureConnectedWithinOperation(error);
}

bool WmiSession::ensureConnectedWithinOperation(std::string* error) const {
  if (d->connected) {
    if (error) error->clear();
    return true;
  }
  return reconnectWithinOperation(error);
}

bool WmiSession::reconnectWithinOperation(std::string* error) const {
  if (error) error->clear();
  UWF_LOG_D("wmi") << "connect attempt: namespace=" << d->namespacePath;

  d->reset();
  d->lastHresult = S_OK;

  if (!d->threadState || !d->threadState->ready) {
    d->lastHresult = d->threadState ? d->threadState->hresult : E_UNEXPECTED;
    if (error) *error = d->threadState ? d->threadState->error : "WMI thread runtime is not initialized";
    return false;
  }

  ULONG_PTR contextToken = 0;
  HRESULT hr = CoGetContextToken(&contextToken);
  if (FAILED(hr)) {
    d->lastHresult = hr;
    if (error) *error = std::format("COM apartment is not available on this thread: {}", hrText(hr));
    return false;
  }

  hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<LPVOID*>(&d->locator));
  if (FAILED(hr) || !d->locator) {
    const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
    d->lastHresult = failure;
    if (error) *error = std::format("CoCreateInstance(WbemLocator) failed: {}", hrText(failure));
    d->reset();
    return false;
  }

  const std::wstring nsW = utf8ToWide(d->namespacePath);
  BSTR ns = SysAllocString(nsW.c_str());
  hr = d->locator->ConnectServer(ns, nullptr, nullptr, nullptr, WBEM_FLAG_CONNECT_USE_MAX_WAIT, nullptr, nullptr, &d->services);
  SysFreeString(ns);
  if (FAILED(hr) || !d->services) {
    const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
    d->lastHresult = failure;
    // 命名空间路径只进日志，不进面向用户的 error——UWF 不可用横幅只展示
    // HRESULT 文本（namespace 对用户无意义，详情查日志即可）。
    UWF_LOG_E("wmi") << "ConnectServer failed: namespace=" << d->namespacePath << "; " << hrText(failure);
    if (error) *error = hrText(failure);
    d->reset();
    return false;
  }

  hr = CoSetProxyBlanket(d->services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
  if (FAILED(hr)) {
    d->lastHresult = hr;
    if (error) *error = std::format("CoSetProxyBlanket failed: {}", hrText(hr));
    d->reset();
    return false;
  }
  d->connected = true;
  UWF_LOG_I("wmi") << "connect ok: namespace=" << d->namespacePath;
  return true;
}

std::vector<WmiRow> WmiSession::query(const std::string& wql, std::string* error) const { return executeQuery(wql, false, error); }

std::vector<WmiRow> WmiSession::queryInstances(const std::string& wql, std::string* error) const { return executeQuery(wql, true, error); }

std::vector<WmiRow> WmiSession::executeQuery(const std::string& wql, const bool ensureLocatable, std::string* error) const {
  if (error) error->clear();
  Impl::OperationGuard operation(*d);
  if (!operation) {
    if (error) *error = "WMI session rejected a reentrant query on the same thread";
    return {};
  }
  for (int attempt = 0; attempt < 2; ++attempt) {
    std::string connectError;
    if (!ensureConnectedWithinOperation(&connectError)) {
      if (error) *error = connectError;
      return {};
    }

    std::vector<WmiRow> rows;
    const std::wstring wqlW = utf8ToWide(wql);
    BSTR lang = SysAllocString(L"WQL");
    BSTR text = SysAllocString(wqlW.c_str());
    IEnumWbemClassObject* enumerator = nullptr;
    const long flags = WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY | (ensureLocatable ? WBEM_FLAG_ENSURE_LOCATABLE : 0);
    HRESULT hr = d->services->ExecQuery(lang, text, flags, nullptr, &enumerator);
    SysFreeString(lang);
    SysFreeString(text);

    bool complete = false;
    std::string failureDetail;
    if (SUCCEEDED(hr) && enumerator) {
      while (true) {
        IWbemClassObject* obj = nullptr;
        ULONG got = 0;
        const HRESULT nextHr = enumerator->Next(static_cast<LONG>(WBEM_INFINITE), 1, &obj, &got);
        hr = nextHr;
        if (FAILED(nextHr)) {
          if (obj) obj->Release();
          failureDetail = std::format("WMI enumerator failed: {}", hrText(nextHr));
          break;
        }
        if (got > 1 || (got == 1) != (obj != nullptr)) {
          if (obj) obj->Release();
          hr = E_UNEXPECTED;
          failureDetail = "WMI enumerator returned an inconsistent row count/object pair";
          break;
        }
        if (got == 0) {
          if (nextHr == WBEM_S_FALSE) {
            complete = true;
            break;
          }
          hr = E_UNEXPECTED;
          failureDetail = std::format("WMI enumerator returned no object without completing: {}", hrText(nextHr));
          break;
        }
        WmiRow row;
        std::string decodeError;
        HRESULT decodeHr = S_OK;
        if (!readObjectProps(obj, row, decodeError, ensureLocatable, &decodeHr)) {
          obj->Release();
          hr = FAILED(decodeHr) ? decodeHr : E_UNEXPECTED;
          failureDetail = std::format("WMI row decoding failed: {}", decodeError);
          break;
        }
        rows.push_back(std::move(row));
        obj->Release();
        if (nextHr == WBEM_S_FALSE) {
          complete = true;
          break;
        }
      }
    } else if (SUCCEEDED(hr)) {
      hr = E_UNEXPECTED;
      failureDetail = "ExecQuery succeeded without returning an enumerator";
    }
    if (enumerator) enumerator->Release();

    if (complete) {
      UWF_LOG_D("wmi") << std::format("query ok: {} row(s); wql={}", rows.size(), wql);
      return rows;
    }

    const auto droppedCount = rows.size();
    rows.clear();
    const bool retryable = isWmiConnectionFailure(static_cast<int32_t>(hr));
    if (retryable) invalidate();
    if (retryable && attempt == 0) {
      UWF_LOG_W("wmi") << "query connection lost; reconnecting once: " << hrText(hr) << "; wql=" << wql;
      continue;
    }
    if (error) {
      *error = failureDetail.empty() ? std::format("WMI query failed after {} row(s): {}\nWQL: {}", droppedCount, hrText(hr), wql)
                                     : std::format("{}\nWQL: {}", failureDetail, wql);
    }
    UWF_LOG_E("wmi") << std::format("query failed after {} row(s) (partial result discarded): {}; wql={}", droppedCount,
                                    failureDetail.empty() ? hrText(hr) : failureDetail, wql);
    return {};
  }
  return {};
}

WmiClassStatus WmiSession::classStatus(const std::string& className, std::string* error) const {
  if (error) error->clear();
  Impl::OperationGuard operation(*d);
  if (!operation) {
    if (error) *error = "WMI session rejected a reentrant class probe on the same thread";
    return WmiClassStatus::Unknown;
  }
  for (int attempt = 0; attempt < 2; ++attempt) {
    std::string connectError;
    if (!ensureConnectedWithinOperation(&connectError)) {
      if (error) *error = connectError;
      return WmiClassStatus::Unknown;
    }
    const auto clsW = utf8ToWide(className);
    BSTR clsBstr = SysAllocString(clsW.c_str());
    IWbemClassObject* classObj = nullptr;
    const HRESULT hr = d->services->GetObject(clsBstr, 0, nullptr, &classObj, nullptr);
    SysFreeString(clsBstr);
    const bool hasClassObject = classObj != nullptr;
    if (classObj) classObj->Release();
    if (hr == static_cast<HRESULT>(WBEM_E_INVALID_CLASS)) {
      UWF_LOG_W("wmi") << "classStatus: " << className << " is not registered";
      return WmiClassStatus::Missing;
    }
    if (isWmiConnectionFailure(static_cast<int32_t>(hr))) {
      invalidate();
      if (attempt == 0) continue;
    }
    if (FAILED(hr) || !hasClassObject) {
      const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
      if (error) *error = std::format("GetObject({}) failed: {}", className, hrText(failure));
      return WmiClassStatus::Unknown;
    }
    UWF_LOG_D("wmi") << "classStatus: " << className << " present";
    return WmiClassStatus::Present;
  }
  if (error) *error = std::format("unable to determine whether WMI class {} exists", className);
  return WmiClassStatus::Unknown;
}

std::optional<WmiRow> WmiSession::getObject(const std::string& objectPath, std::string* error) const {
  if (error) error->clear();
  Impl::OperationGuard operation(*d);
  if (!operation) {
    if (error) *error = "WMI session rejected a reentrant object read on the same thread";
    return std::nullopt;
  }
  for (int attempt = 0; attempt < 2; ++attempt) {
    std::string connectError;
    if (!ensureConnectedWithinOperation(&connectError)) {
      if (error) *error = connectError;
      return std::nullopt;
    }

    const auto pathW = utf8ToWide(objectPath);
    BSTR pathBstr = SysAllocString(pathW.c_str());
    IWbemClassObject* object = nullptr;
    const HRESULT hr = d->services->GetObject(pathBstr, 0, nullptr, &object, nullptr);
    SysFreeString(pathBstr);
    if (SUCCEEDED(hr) && object) {
      WmiRow row;
      std::string decodeError;
      HRESULT decodeHr = S_OK;
      const bool decoded = readObjectProps(object, row, decodeError, true, &decodeHr);
      object->Release();
      if (!decoded) {
        if (isWmiConnectionFailure(static_cast<int32_t>(decodeHr))) {
          invalidate();
          if (attempt == 0) continue;
        }
        if (error) *error = std::move(decodeError);
        return std::nullopt;
      }
      return row;
    }
    if (object) object->Release();
    const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
    if (isWmiConnectionFailure(static_cast<int32_t>(failure))) {
      invalidate();
      if (attempt == 0) continue;
    }
    if (error) *error = std::format("GetObject({}) failed: {}", objectPath, hrText(failure));
    return std::nullopt;
  }
  if (error) *error = std::format("GetObject({}) failed after reconnect", objectPath);
  return std::nullopt;
}

WmiMethodResult WmiSession::callMethod(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs) const {
  WmiMethodResult result;
  Impl::OperationGuard operation(*d);
  if (!operation) {
    result.hresult = static_cast<int32_t>(WBEM_E_INVALID_OPERATION);
    result.error = std::format("WMI session rejected reentrant method {} on the same thread", methodName);
    return result;
  }
  UWF_LOG_D("wmi") << "callMethod start: " << methodName << " @ " << objectPath;

  std::string connectError;
  if (!ensureConnectedWithinOperation(&connectError)) {
    result.hresult = static_cast<int32_t>(d->lastHresult);
    result.error = connectError.empty() ? "WMI connection failed" : connectError;
    UWF_LOG_E("wmi") << "callMethod connect failed: method=" << methodName << "; " << result.error;
    return result;
  }

  PreparedMethodCall prepared;
  if (!prepareMethodCall(d->services, objectPath, methodName, inputs, prepared, result)) {
    if (isWmiConnectionFailure(result.hresult)) invalidate();
    UWF_LOG_E("wmi") << result.error;
    return result;
  }

  IWbemClassObject* outParams = nullptr;
  result.attempted = true;
  HRESULT hr = d->services->ExecMethod(prepared.objectPath, prepared.methodName, 0, nullptr, prepared.inputs, &outParams, nullptr);

  if (FAILED(hr)) {
    result.hresult = static_cast<int32_t>(hr);
    result.error = std::format("ExecMethod {}::{} failed: {}", objectPath, methodName, hrText(hr));
    UWF_LOG_E("wmi") << result.error;
    // WMI 在 IErrorInfo / IWbemClassObject (error object) 上挂着 provider 返回
    // 的扩展错误。GetErrorInfo() 拿到的 IErrorInfo 通常能 QI 出 IWbemClassObject
    // 给出 __ExtendedStatus / Description / Operation / ParameterInfo 等。
    {
      IErrorInfo* errInfo = nullptr;
      if (GetErrorInfo(0, &errInfo) == S_OK && errInfo) {
        BSTR desc = nullptr;
        const HRESULT descriptionHr = errInfo->GetDescription(&desc);
        if (descriptionHr == S_OK && desc) {
          UWF_LOG_E("wmi") << "  IErrorInfo.Description: " << wideToUtf8(desc);
        }
        if (desc) SysFreeString(desc);
        IWbemClassObject* errObj = nullptr;
        const HRESULT errorObjectHr = errInfo->QueryInterface(IID_IWbemClassObject, reinterpret_cast<void**>(&errObj));
        if (errorObjectHr == S_OK && errObj) {
          std::string edump;
          errObj->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
          while (true) {
            BSTR ename = nullptr;
            VARIANT ev;
            CIMTYPE ect = 0;
            VariantInit(&ev);
            const HRESULT ehn = errObj->Next(0, &ename, &ev, &ect, nullptr);
            if (ehn != WBEM_S_NO_ERROR) {
              if (ename) SysFreeString(ename);
              VariantClear(&ev);
              break;
            }
            if (!edump.empty()) edump += ", ";
            edump += std::format("{}={}", wideToUtf8(ename), variantToValue(ev, ect).toString());
            SysFreeString(ename);
            VariantClear(&ev);
          }
          errObj->EndEnumeration();
          UWF_LOG_E("wmi") << "  WMI error object: [" << edump << "]";
        }
        if (errObj) errObj->Release();
        errInfo->Release();
      } else {
        if (errInfo) errInfo->Release();
        UWF_LOG_E("wmi") << "  (no extended error info available)";
      }
    }
    if (outParams) outParams->Release();
    if (isWmiConnectionFailure(result.hresult)) invalidate();
    return result;
  }

  result.invoked = true;
  const bool decoded = decodeMethodOutput(outParams, prepared.className, methodName, result);
  if (outParams) outParams->Release();
  if (!decoded && isWmiConnectionFailure(result.hresult)) invalidate();
  if (!decoded || !result.ok()) {
    UWF_LOG_E("wmi") << "callMethod: " << result.error;
  } else {
    UWF_LOG_D("wmi") << std::format("callMethod ok: {}::{} outArrays={}", objectPath, methodName, result.outArrays.size());
  }
  return result;
}

WmiMethodResult WmiSession::callMethodRead(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs) const {
  auto result = callMethod(objectPath, methodName, inputs);
  if (!isWmiConnectionFailure(result.hresult)) return result;

  std::string reconnectError;
  if (!ensureConnected(&reconnectError)) {
    if (!reconnectError.empty()) result.error += std::format("; reconnect failed: {}", reconnectError);
    return result;
  }
  UWF_LOG_W("wmi") << "read-only method connection lost; retrying once: " << methodName << " @ " << objectPath;
  return callMethod(objectPath, methodName, inputs);
}

WmiMethodResult WmiSession::callMethodReadCancelable(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs,
                                                     const std::stop_token stopToken) const {
  WmiMethodResult lastResult;
  Impl::OperationGuard operation(*d);
  if (!operation) {
    lastResult.hresult = static_cast<int32_t>(WBEM_E_INVALID_OPERATION);
    lastResult.error = std::format("WMI session rejected reentrant method {} on the same thread", methodName);
    return lastResult;
  }
  for (int attempt = 0; attempt < 2; ++attempt) {
    lastResult = {};
    if (stopToken.stop_requested()) {
      lastResult.hresult = static_cast<int32_t>(WBEM_E_CALL_CANCELLED);
      lastResult.error = std::format("{} canceled", methodName);
      return lastResult;
    }

    std::string connectError;
    if (!ensureConnectedWithinOperation(&connectError)) {
      lastResult.hresult = static_cast<int32_t>(d->lastHresult);
      lastResult.error = connectError;
      return lastResult;
    }

    PreparedMethodCall prepared;
    if (!prepareMethodCall(d->services, objectPath, methodName, inputs, prepared, lastResult)) {
      if (isWmiConnectionFailure(lastResult.hresult)) invalidate();
      if (attempt == 0 && isWmiConnectionFailure(lastResult.hresult)) continue;
      return lastResult;
    }

    auto state = std::make_shared<AsyncMethodState>();
    auto* sink = new AsyncMethodSink(state);
    const HRESULT startHr = d->services->ExecMethodAsync(prepared.objectPath, prepared.methodName, 0, nullptr, prepared.inputs, sink);
    if (FAILED(startHr)) {
      sink->Release();
      lastResult.hresult = static_cast<int32_t>(startHr);
      lastResult.error = std::format("ExecMethodAsync {}::{} failed: {}", objectPath, methodName, hrText(startHr));
      if (isWmiConnectionFailure(lastResult.hresult)) invalidate();
      if (attempt == 0 && isWmiConnectionFailure(lastResult.hresult)) continue;
      return lastResult;
    }
    lastResult.attempted = true;

    std::stop_callback wakeOnStop(stopToken, [&state] { state->changed.notify_all(); });
    std::unique_lock lock(state->mutex);
    state->changed.wait(lock, [&state, &stopToken] { return state->completed || stopToken.stop_requested(); });
    if (stopToken.stop_requested() && !state->completed) {
      lock.unlock();
      const HRESULT cancelHr = d->services->CancelAsyncCall(sink);
      sink->Release();
      if (FAILED(cancelHr)) {
        lastResult.hresult = static_cast<int32_t>(cancelHr);
        lastResult.error = std::format("{} cancellation failed: {}", methodName, hrText(cancelHr));
        if (isWmiConnectionFailure(lastResult.hresult)) invalidate();
      } else {
        lastResult.hresult = static_cast<int32_t>(WBEM_E_CALL_CANCELLED);
        lastResult.error = std::format("{} canceled", methodName);
      }
      return lastResult;
    }

    const HRESULT completionHr = state->status;
    IWbemClassObject* output = state->output;
    if (output) output->AddRef();
    lock.unlock();
    sink->Release();

    if (FAILED(completionHr)) {
      if (output) output->Release();
      lastResult.hresult = static_cast<int32_t>(completionHr);
      lastResult.error = std::format("ExecMethodAsync {}::{} completed with {}", objectPath, methodName, hrText(completionHr));
      if (isWmiConnectionFailure(lastResult.hresult)) invalidate();
      if (attempt == 0 && isWmiConnectionFailure(lastResult.hresult)) continue;
      return lastResult;
    }

    lastResult.invoked = true;
    const bool decoded = decodeMethodOutput(output, prepared.className, methodName, lastResult);
    if (output) output->Release();
    if (!decoded && isWmiConnectionFailure(lastResult.hresult)) {
      invalidate();
      if (attempt == 0) continue;
    }
    return lastResult;
  }
  return lastResult;
}

WmiResult WmiSession::putInstance(const std::string& className, const WmiRow& props, const WmiPutMode mode) const {
  Impl::OperationGuard operation(*d);
  if (!operation) {
    auto out = WmiResult::failed(std::format("WMI session rejected reentrant PutInstance({}) on the same thread", className));
    out.hresult = static_cast<int32_t>(WBEM_E_INVALID_OPERATION);
    return out;
  }
  std::string connectError;
  if (!ensureConnectedWithinOperation(&connectError)) {
    UWF_LOG_E("wmi") << "putInstance connect failed: class=" << className << "; " << connectError;
    auto out = WmiResult::failed(connectError.empty() ? "WMI connection failed" : connectError);
    out.hresult = static_cast<int32_t>(d->lastHresult);
    return out;
  }

  // 1) 拿到类的 blueprint —— SpawnInstance 要从 class object 出发。
  IWbemClassObject* classObj = nullptr;
  {
    const auto clsW = utf8ToWide(className);
    BSTR clsBstr = SysAllocString(clsW.c_str());
    HRESULT hr = d->services->GetObject(clsBstr, 0, nullptr, &classObj, nullptr);
    SysFreeString(clsBstr);
    if (FAILED(hr) || !classObj) {
      if (classObj) classObj->Release();
      const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
      auto out = WmiResult::failed(std::format("GetObject({}) failed: {}", className, hrText(failure)));
      out.hresult = static_cast<int32_t>(failure);
      if (isWmiConnectionFailure(out.hresult)) invalidate();
      return out;
    }
  }

  // 2) SpawnInstance 创建本地空实例（还没写入 store）。
  IWbemClassObject* inst = nullptr;
  HRESULT hr = classObj->SpawnInstance(0, &inst);
  classObj->Release();
  if (FAILED(hr) || !inst) {
    if (inst) inst->Release();
    const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
    auto out = WmiResult::failed(std::format("SpawnInstance({}) failed: {}", className, hrText(failure)));
    out.hresult = static_cast<int32_t>(failure);
    if (isWmiConnectionFailure(out.hresult)) invalidate();
    return out;
  }

  // 3) 设置每个属性。WMI 对 BOOL key 期望 VT_BOOL，对 string 期望 VT_BSTR，
  //    valueToVariant 已经按 WmiValue::Kind 选好；对 UInt 用 VT_I4
  //    （兼容 CIM_UINT*，参见 ThemeManager 那条注释）。
  for (const auto& [name, val] : props) {
    VARIANT v;
    if (!valueToVariant(val, v)) {
      inst->Release();
      return WmiResult::failed(std::format("unsupported or out-of-range value for {}.{}", className, name));
    }
    const auto nameW = utf8ToWide(name);
    BSTR nameBstr = SysAllocString(nameW.c_str());
    const HRESULT hPut = inst->Put(nameBstr, 0, &v, 0);
    if (FAILED(hPut)) {
      SysFreeString(nameBstr);
      VariantClear(&v);
      inst->Release();
      auto out = WmiResult::failed(std::format("inst->Put({}) on {} failed: {}", name, className, hrText(hPut)));
      out.hresult = static_cast<int32_t>(hPut);
      if (isWmiConnectionFailure(out.hresult)) invalidate();
      UWF_LOG_E("wmi") << out.detail;
      return out;
    }
    SysFreeString(nameBstr);
    VariantClear(&v);
  }

  // 4) PutInstance 把 inst 写入 WMI store；创建与更新语义由领域层明确选择。
  const long flags = mode == WmiPutMode::CreateOnly ? WBEM_FLAG_CREATE_ONLY : WBEM_FLAG_UPDATE_ONLY;
  // 不传 IWbemCallResult，明确要求同步完成。若传非空 ppCallResult，WMI 会把
  // 真实完成状态放进该对象，PutInstance 本身只返回“操作已启动”；直接释放它
  // 会把尚未完成甚至最终失败的写误报为成功，也会破坏紧随其后的状态确认。
  hr = d->services->PutInstance(inst, flags, nullptr, nullptr);
  inst->Release();
  if (FAILED(hr)) {
    UWF_LOG_E("wmi") << std::format("putInstance failed: class={}; {}", className, hrText(hr));
    WmiResult out = WmiResult::failed(std::format("PutInstance({}) failed: {}", className, hrText(hr)));
    out.attempted = true;
    out.hresult = static_cast<int32_t>(hr);
    out.outcomeUncertain = isWmiConnectionFailure(out.hresult);
    if (isWmiConnectionFailure(out.hresult)) invalidate();
    return out;
  }
  UWF_LOG_I("wmi") << std::format("putInstance ok: class={} props={}", className, props.size());
  WmiResult out;
  out.attempted = true;
  out.ok = true;
  return out;
}

WmiResult WmiSession::deleteInstance(const std::string& objectPath) const {
  Impl::OperationGuard operation(*d);
  if (!operation) {
    auto out = WmiResult::failed(std::format("WMI session rejected reentrant DeleteInstance({}) on the same thread", objectPath));
    out.hresult = static_cast<int32_t>(WBEM_E_INVALID_OPERATION);
    return out;
  }
  std::string connectError;
  if (!ensureConnectedWithinOperation(&connectError)) {
    UWF_LOG_E("wmi") << "deleteInstance connect failed: path=" << objectPath << "; " << connectError;
    auto out = WmiResult::failed(connectError.empty() ? "WMI connection failed" : connectError);
    out.hresult = static_cast<int32_t>(d->lastHresult);
    return out;
  }
  const auto pathW = utf8ToWide(objectPath);
  BSTR pathBstr = SysAllocString(pathW.c_str());
  HRESULT hr = d->services->DeleteInstance(pathBstr, 0, nullptr, nullptr);
  SysFreeString(pathBstr);
  if (FAILED(hr)) {
    UWF_LOG_E("wmi") << std::format("deleteInstance failed: path={}; {}", objectPath, hrText(hr));
    WmiResult out = WmiResult::failed(std::format("DeleteInstance({}) failed: {}", objectPath, hrText(hr)));
    out.attempted = true;
    out.hresult = static_cast<int32_t>(hr);
    out.outcomeUncertain = isWmiConnectionFailure(out.hresult);
    if (isWmiConnectionFailure(out.hresult)) invalidate();
    return out;
  }
  UWF_LOG_I("wmi") << "deleteInstance ok: path=" << objectPath;
  WmiResult out;
  out.attempted = true;
  out.ok = true;
  return out;
}

bool isWmiConnectionFailure(const int32_t hresult) {
  const auto hr = static_cast<HRESULT>(hresult);
  return hr == RPC_E_DISCONNECTED || hr == RPC_E_SERVER_DIED || hr == RPC_E_SERVER_DIED_DNE || hr == CO_E_SERVER_STOPPING || hr == CO_E_OBJNOTCONNECTED ||
         hr == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) || hr == HRESULT_FROM_WIN32(RPC_S_CALL_FAILED) || hr == HRESULT_FROM_WIN32(RPC_S_CALL_FAILED_DNE) ||
         hr == static_cast<HRESULT>(WBEM_E_TRANSPORT_FAILURE) || hr == static_cast<HRESULT>(WBEM_E_FATAL_TRANSPORT_ERROR) ||
         hr == static_cast<HRESULT>(WBEM_E_CONNECTION_FAILED) || hr == static_cast<HRESULT>(WBEM_E_SHUTTING_DOWN);
}

namespace {

class ThreadComApartment final {
 public:
  explicit ThreadComApartment(WmiThreadState& state) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    // Qt 的 GUI 线程通常已是 STA。不能只把 RPC_E_CHANGED_MODE 当成“COM 已经
    // 可用”：那样本模块没有自己的初始化引用，QApplication 先析构后可能在
    // thread_local session 释放代理之前把 apartment 关闭。用既有模式再初始化
    // 一次，明确取得与 session 生命周期配对的引用。
    if (hr == RPC_E_CHANGED_MODE) hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr)) {
      m_uninitialize = true;
    } else {
      state.hresult = hr;
      state.error = std::format("CoInitializeEx failed for thread: {}", hrText(hr));
      UWF_LOG_E("wmi") << state.error;
      return;
    }

    static std::once_flag securityOnce;
    static HRESULT securityResult = E_UNEXPECTED;
    std::call_once(securityOnce, [] {
      securityResult = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
    });
    if (FAILED(securityResult) && securityResult != RPC_E_TOO_LATE) {
      state.hresult = securityResult;
      state.error = std::format("CoInitializeSecurity failed: {}", hrText(securityResult));
      UWF_LOG_E("wmi") << state.error;
      return;
    }
    state.ready = true;
    state.hresult = S_OK;
    state.error.clear();
  }

  ~ThreadComApartment() {
    if (m_uninitialize) CoUninitialize();
  }

  ThreadComApartment(const ThreadComApartment&) = delete;
  ThreadComApartment& operator=(const ThreadComApartment&) = delete;

 private:
  bool m_uninitialize = false;
};

}  // namespace

struct WmiThreadContext {
  // 声明顺序很重要：逆序析构保证两个 session 的 COM 代理先于 apartment 释放。
  WmiThreadState state;
  ThreadComApartment apartment{state};
  WmiSession embedded{config::kWmiNamespaceEmbedded, &state};
  WmiSession cimv2{config::kWmiNamespaceCimv2, &state};
};

namespace {

WmiThreadContext& threadContext() {
  thread_local WmiThreadContext context;
  return context;
}

}  // namespace

WmiSession& embeddedWmiSession() { return threadContext().embedded; }
WmiSession& cimv2WmiSession() { return threadContext().cimv2; }
bool initializeWmiRuntime(std::string* error) {
  const auto& state = threadContext().state;
  if (error) *error = state.error;
  return state.ready;
}

}  // namespace uwf
