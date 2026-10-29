// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WmiSession.h"

#include <comdef.h>
#include <oleauto.h>
#include <wbemidl.h>
#include <windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <charconv>
#include <chrono>
#include <cmath>
#include <exception>
#include <format>
#include <limits>
#include <mutex>
#include <new>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>

#include "app/wmi/WmiException.h"
#include "shared/Log.h"
#include "shared/StringUtil.h"
#include "windows/resources/ComPtr.h"
#include "windows/system/UnicodeConversion.h"
#include "windows/wmi/WmiNamespaces.h"

namespace uwf::windows {

namespace {
constexpr auto kQueryTimeout = std::chrono::seconds(30);
constexpr LONG kEnumerationPollMilliseconds = 100;

bool isWmiConnectionFailure(int32_t hresult);
bool isKnownPreInvocationRejection(int32_t hresult);

class UniqueBstr final {
 public:
  UniqueBstr() = default;
  explicit UniqueBstr(const std::wstring& value) : m_value(SysAllocStringLen(value.data(), static_cast<UINT>(value.size()))) {
    if (!m_value) throw std::bad_alloc();
  }
  explicit UniqueBstr(const wchar_t* value) : m_value(SysAllocString(value)) {
    if (!m_value) throw std::bad_alloc();
  }
  ~UniqueBstr() { SysFreeString(m_value); }
  UniqueBstr(const UniqueBstr&) = delete;
  UniqueBstr& operator=(const UniqueBstr&) = delete;
  UniqueBstr(UniqueBstr&& other) noexcept : m_value(std::exchange(other.m_value, nullptr)) {}
  UniqueBstr& operator=(UniqueBstr&& other) noexcept {
    if (this != &other) {
      SysFreeString(m_value);
      m_value = std::exchange(other.m_value, nullptr);
    }
    return *this;
  }
  [[nodiscard]] static UniqueBstr adopt(BSTR value) noexcept { return UniqueBstr(value, AdoptTag{}); }
  [[nodiscard]] BSTR get() const noexcept { return m_value; }
  [[nodiscard]] BSTR release() noexcept { return std::exchange(m_value, nullptr); }
  [[nodiscard]] operator BSTR() const noexcept { return m_value; }

 private:
  struct AdoptTag {};
  UniqueBstr(BSTR value, AdoptTag) noexcept : m_value(value) {}
  BSTR m_value = nullptr;
};

class UniqueVariant final {
 public:
  UniqueVariant() { VariantInit(&m_value); }
  ~UniqueVariant() { VariantClear(&m_value); }
  UniqueVariant(const UniqueVariant&) = delete;
  UniqueVariant& operator=(const UniqueVariant&) = delete;
  [[nodiscard]] VARIANT* put() noexcept { return &m_value; }
  [[nodiscard]] VARIANT& get() noexcept { return m_value; }
  [[nodiscard]] const VARIANT& get() const noexcept { return m_value; }

 private:
  VARIANT m_value;
};

std::string hrText(HRESULT hr);

std::string bstrToUtf8(const BSTR value) {
  if (!value) return {};
  return wideToUtf8(std::wstring_view(value, SysStringLen(value)));
}

class WbemEnumeration final {
 public:
  WbemEnumeration(IWbemClassObject& object, const std::string& operation) : m_object(object) {
    const HRESULT hr = m_object.BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
    if (isWmiConnectionFailure(static_cast<int32_t>(hr))) {
      throw app::wmi::WmiTransportError(static_cast<int32_t>(hr), operation, std::format("BeginEnumeration failed: {}", hrText(hr)));
    }
    if (FAILED(hr)) throw app::wmi::WmiDecodeError(operation, std::format("BeginEnumeration failed: {}", hrText(hr)));
  }
  ~WbemEnumeration() {
    if (!m_finished) m_object.EndEnumeration();
  }
  WbemEnumeration(const WbemEnumeration&) = delete;
  WbemEnumeration& operator=(const WbemEnumeration&) = delete;

  void finish(const std::string& operation) {
    if (m_finished) return;
    const HRESULT hr = m_object.EndEnumeration();
    m_finished = true;
    if (isWmiConnectionFailure(static_cast<int32_t>(hr))) {
      throw app::wmi::WmiTransportError(static_cast<int32_t>(hr), operation, std::format("EndEnumeration failed: {}", hrText(hr)));
    }
    if (FAILED(hr)) throw app::wmi::WmiDecodeError(operation, std::format("EndEnumeration failed: {}", hrText(hr)));
  }

 private:
  IWbemClassObject& m_object;
  bool m_finished = false;
};

// 把 HRESULT 渲染成 "描述 (0xXXXXXXXX)" 的形式，方便日志。
std::string hrText(const HRESULT hr) {
  std::string description;
  try {
    const _com_error error(hr);
    description = wideToUtf8(error.ErrorMessage());
  } catch (...) {
    // HRESULT 是错误事实本身；本地化消息只是诊断附加信息，转换失败不能遮蔽
    // 正在构造的 WMI 异常。
  }
  if (description.empty()) return std::format("HRESULT 0x{:08x}", static_cast<uint32_t>(hr));
  return std::format("{} (0x{:08x})", description, static_cast<uint32_t>(hr));
}

std::string extendedErrorDetail() noexcept {
  try {
    windows::ComPtr<IErrorInfo> errorInfo;
    if (GetErrorInfo(0, errorInfo.put()) != S_OK || !errorInfo) return {};
    BSTR rawDescription = nullptr;
    if (errorInfo->GetDescription(&rawDescription) != S_OK || !rawDescription) return {};
    const UniqueBstr description = UniqueBstr::adopt(rawDescription);
    return bstrToUtf8(description.get());
  } catch (...) {
    return {};
  }
}

app::wmi::WmiValue variantToValue(const VARIANT& v, const CIMTYPE cimType, const std::string& operation) {
  switch (v.vt) {
    case VT_NULL:
    case VT_EMPTY:
      return {};
    case VT_BOOL:
      return app::wmi::WmiValue::fromBool(v.boolVal != VARIANT_FALSE);
    case VT_I1:
      if (cimType == CIM_UINT8) return app::wmi::WmiValue::fromUInt(static_cast<std::uint8_t>(v.cVal));
      return app::wmi::WmiValue::fromInt(static_cast<std::int8_t>(v.cVal));
    case VT_UI1:
      return app::wmi::WmiValue::fromUInt(v.bVal);
    case VT_I2:
      if (cimType == CIM_UINT16) return app::wmi::WmiValue::fromUInt(static_cast<uint16_t>(v.iVal));
      return app::wmi::WmiValue::fromInt(v.iVal);
    case VT_I4:
      // WMI Automation 把 CIM_UINT32 装箱成 VT_I4。只看 VARTYPE 会把
      // 0x8004xxxx 这类合法的 UInt32（UWF ReturnValue 常用 HRESULT 位型）
      // 误解成负数；IWbemClassObject::Next 同时返回的 CIMTYPE 才是字段语义。
      if (cimType == CIM_UINT32) return app::wmi::WmiValue::fromUInt(static_cast<uint32_t>(v.lVal));
      return app::wmi::WmiValue::fromInt(v.lVal);
    case VT_UI2:
      return app::wmi::WmiValue::fromUInt(v.uiVal);
    case VT_UI4:
      return app::wmi::WmiValue::fromUInt(v.ulVal);
    case VT_INT:
      if (cimType == CIM_UINT32) return app::wmi::WmiValue::fromUInt(static_cast<std::uint32_t>(v.intVal));
      return app::wmi::WmiValue::fromInt(v.intVal);
    case VT_UINT:
      return app::wmi::WmiValue::fromUInt(v.uintVal);
    case VT_I8:
      if (cimType == CIM_UINT64) return app::wmi::WmiValue::fromUInt(static_cast<uint64_t>(v.llVal));
      return app::wmi::WmiValue::fromInt(v.llVal);
    case VT_UI8:
      return app::wmi::WmiValue::fromUInt(v.ullVal);
    case VT_R4:
      return app::wmi::WmiValue::fromDouble(v.fltVal);
    case VT_R8:
      return app::wmi::WmiValue::fromDouble(v.dblVal);
    case VT_BSTR:
      return app::wmi::WmiValue::fromString(bstrToUtf8(v.bstrVal));
    case VT_DATE:
      return app::wmi::WmiValue::fromDouble(v.date);
    default:
      throw app::wmi::WmiDecodeError(operation, std::format("unsupported VARIANT type {}", static_cast<unsigned>(v.vt)));
  }
}

// WmiValue → VARIANT（调用方负责 VariantClear）。只支持标量；无法编码的值
// 直接抛出带语境的协议异常，不再返回 bool 让调用方二次解释。
void valueToVariant(const app::wmi::WmiValue& src, VARIANT& v, const std::string& operation, const std::string& subject) {
  VariantInit(&v);
  switch (src.kind()) {
    case app::wmi::WmiValue::Kind::Bool:
      v.vt = VT_BOOL;
      v.boolVal = src.toBool() ? VARIANT_TRUE : VARIANT_FALSE;
      return;
    case app::wmi::WmiValue::Kind::Int: {
      const int64_t value = src.toInt64();
      if (value < std::numeric_limits<LONG>::min() || value > std::numeric_limits<LONG>::max()) {
        throw app::wmi::WmiProtocolError(operation, subject + " is outside the supported Int32 range");
      }
      v.vt = VT_I4;
      v.lVal = static_cast<LONG>(value);
      return;
    }
    case app::wmi::WmiValue::Kind::UInt: {
      // WMI 对 CIM_UINT32 入参的 VARIANT 装箱约定是 VT_I4（参见微软
      // 文档 "Numbers (WMI)"：learn.microsoft.com/windows/win32/wmisdk/numbers），
      // 传 VT_UI4 会让 IWbemClassObject::Put 退回
      // WBEM_E_INVALID_PARAMETER (0x80041008)，进而 ExecMethod 失败。
      const uint64_t value = src.toULongLong();
      if (value > std::numeric_limits<uint32_t>::max()) {
        throw app::wmi::WmiProtocolError(operation, subject + " is outside the supported UInt32 input range");
      }
      v.vt = VT_I4;
      static_assert(sizeof(LONG) == sizeof(uint32_t));
      v.lVal = std::bit_cast<LONG>(static_cast<uint32_t>(value));
      return;
    }
    case app::wmi::WmiValue::Kind::Double:
      v.vt = VT_R8;
      v.dblVal = src.toDouble();
      return;
    case app::wmi::WmiValue::Kind::String: {
      const auto w = utf8ToWide(src.toString());
      UniqueBstr encoded(w);
      v.vt = VT_BSTR;
      v.bstrVal = encoded.release();
      return;
    }
    default:
      throw app::wmi::WmiProtocolError(operation, subject + " is null or has an unsupported type");
  }
}

enum class PathRequirement { Optional, Required };

app::wmi::WmiRow readObjectProps(IWbemClassObject& object, const PathRequirement pathRequirement, const std::string& operation) {
  app::wmi::WmiRow row;
  WbemEnumeration enumeration(object, operation);
  while (true) {
    BSTR rawName = nullptr;
    UniqueVariant value;
    CIMTYPE type = 0;
    const HRESULT hr = object.Next(0, &rawName, value.put(), &type, nullptr);
    UniqueBstr name = UniqueBstr::adopt(rawName);
    if (hr == WBEM_S_NO_MORE_DATA) break;
    if (hr != WBEM_S_NO_ERROR) {
      if (isWmiConnectionFailure(static_cast<int32_t>(hr))) {
        throw app::wmi::WmiTransportError(static_cast<int32_t>(hr), operation, std::format("property enumeration failed: {}", hrText(hr)));
      }
      throw app::wmi::WmiDecodeError(operation, std::format("property enumeration failed: {}", hrText(hr)));
    }
    if (!rawName) throw app::wmi::WmiDecodeError(operation, "property enumeration returned a value without a name");
    const std::string key = bstrToUtf8(name.get());
    if (!row.emplace(key, variantToValue(value.get(), type, operation)).second) {
      throw app::wmi::WmiDecodeError(operation, "property enumeration returned duplicate field '" + key + "'");
    }
  }
  enumeration.finish(operation);

  UniqueVariant path;
  const UniqueBstr pathName(L"__PATH");
  const HRESULT pathHr = object.Get(pathName, 0, path.put(), nullptr, nullptr);
  if (isWmiConnectionFailure(static_cast<int32_t>(pathHr))) {
    throw app::wmi::WmiTransportError(static_cast<int32_t>(pathHr), operation, std::format("Get(__PATH) failed: {}", hrText(pathHr)));
  }
  if (pathHr == static_cast<HRESULT>(WBEM_E_NOT_FOUND) && pathRequirement == PathRequirement::Optional) return row;
  if (FAILED(pathHr)) {
    throw app::wmi::WmiDecodeError(operation, std::format("Get(__PATH) failed: {}", hrText(pathHr)));
  }
  if (path.get().vt == VT_BSTR && path.get().bstrVal && SysStringLen(path.get().bstrVal) != 0) {
    if (!row.emplace("__PATH", app::wmi::WmiValue::fromString(bstrToUtf8(path.get().bstrVal))).second) {
      throw app::wmi::WmiDecodeError(operation, "property enumeration returned duplicate field '__PATH'");
    }
  } else if (pathRequirement == PathRequirement::Required) {
    throw app::wmi::WmiDecodeError(operation, std::format("locatable object has no string __PATH (VARIANT type {})", static_cast<unsigned>(path.get().vt)));
  }
  return row;
}

// 处理 ExecMethod 的 out 参数里 VT_ARRAY 类型字段：展开为 vector<WmiRow>。
// - 元素是 IUnknown → QI 成 IWbemClassObject，展开为行；
// - 元素是 BSTR（EmbeddedInstance 常见） → 放进行的 __MOF 字段。
std::vector<app::wmi::WmiRow> expandArrayVariant(const VARIANT& v, const std::string& operation) {
  if (!(v.vt & VT_ARRAY) || !v.parray) {
    throw app::wmi::WmiDecodeError(operation, "output value is not an array");
  }

  SAFEARRAY* sa = v.parray;
  if (SafeArrayGetDim(sa) != 1) {
    throw app::wmi::WmiDecodeError(operation, "only one-dimensional WMI output arrays are supported");
  }
  VARTYPE vt = VT_EMPTY;
  HRESULT hr = SafeArrayGetVartype(sa, &vt);
  if (FAILED(hr)) throw app::wmi::WmiDecodeError(operation, std::format("SafeArrayGetVartype failed: {}", hrText(hr)));
  LONG lb = 0, ub = 0;
  hr = SafeArrayGetLBound(sa, 1, &lb);
  if (FAILED(hr)) throw app::wmi::WmiDecodeError(operation, std::format("SafeArrayGetLBound failed: {}", hrText(hr)));
  hr = SafeArrayGetUBound(sa, 1, &ub);
  if (FAILED(hr)) throw app::wmi::WmiDecodeError(operation, std::format("SafeArrayGetUBound failed: {}", hrText(hr)));

  std::vector<app::wmi::WmiRow> rows;
  if (ub < lb) return rows;
  for (LONG i = lb;; ++i) {
    if (vt == VT_UNKNOWN) {
      windows::ComPtr<IUnknown> unknown;
      hr = SafeArrayGetElement(sa, &i, unknown.put());
      if (FAILED(hr)) throw app::wmi::WmiDecodeError(operation, std::format("SafeArrayGetElement({}) failed: {}", i, hrText(hr)));
      if (!unknown) throw app::wmi::WmiDecodeError(operation, std::format("SafeArrayGetElement({}) returned a null IUnknown", i));
      windows::ComPtr<IWbemClassObject> item;
      hr = unknown->QueryInterface(IID_IWbemClassObject, item.putVoid());
      if (FAILED(hr) || !item) throw app::wmi::WmiDecodeError(operation, std::format("array element is not an IWbemClassObject: {}", hrText(hr)));
      rows.push_back(readObjectProps(*item, PathRequirement::Optional, operation));
    } else if (vt == VT_BSTR) {
      BSTR raw = nullptr;
      hr = SafeArrayGetElement(sa, &i, &raw);
      if (FAILED(hr)) throw app::wmi::WmiDecodeError(operation, std::format("SafeArrayGetElement({}) failed: {}", i, hrText(hr)));
      if (!raw) throw app::wmi::WmiDecodeError(operation, std::format("SafeArrayGetElement({}) returned a null BSTR", i));
      const UniqueBstr string = UniqueBstr::adopt(raw);
      app::wmi::WmiRow row;
      row.emplace("__MOF", app::wmi::WmiValue::fromString(bstrToUtf8(string.get())));
      rows.push_back(std::move(row));
    } else {
      throw app::wmi::WmiDecodeError(operation, std::format("unsupported WMI output array element type {}", static_cast<unsigned>(vt)));
    }
    if (i == ub) break;
  }
  return rows;
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
  UniqueBstr objectPath;
  UniqueBstr methodName;
  windows::ComPtr<IWbemClassObject> inputs;
  std::string className;
};

[[noreturn]] void throwComFailure(const HRESULT hr, std::string operation, std::string detail) {
  if (isWmiConnectionFailure(static_cast<int32_t>(hr))) {
    throw app::wmi::WmiTransportError(static_cast<int32_t>(hr), std::move(operation), std::move(detail));
  }
  throw app::wmi::WmiInfrastructureError(static_cast<int32_t>(hr), std::move(operation), std::move(detail));
}

// 只有 WMI 在请求校验阶段明确拒绝时，才能确定失败的有副作用调用没有落地。
// 其余失败统一转换为独立的写边界异常，防止调用方意外重放。
[[noreturn]] void throwWriteInvocationFailure(const HRESULT hr, std::string operation, std::string detail) {
  if (isKnownPreInvocationRejection(static_cast<int32_t>(hr))) {
    throwComFailure(hr, std::move(operation), std::move(detail));
  }
  throw app::wmi::WmiInvocationUncertain(static_cast<int32_t>(hr), std::move(operation), std::move(detail));
}

PreparedMethodCall prepareMethodCall(IWbemServices& services, const std::string& objectPath, const std::string& methodName, const app::wmi::WmiRow& inputs) {
  const std::string operation = std::format("prepare WMI method {} on {}", methodName, objectPath);
  if (objectPath.empty()) throw app::wmi::WmiProtocolError(operation, "object path is empty; read the row before invoking an instance method");

  PreparedMethodCall prepared;
  prepared.className = classNameFromObjectPath(objectPath);
  if (prepared.className.empty()) throw app::wmi::WmiProtocolError(operation, "object path has no class name");
  const UniqueBstr className(utf8ToWide(prepared.className));
  windows::ComPtr<IWbemClassObject> classObject;
  HRESULT hr = services.GetObject(className, 0, nullptr, classObject.put(), nullptr);
  if (FAILED(hr) || !classObject) {
    const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
    throwComFailure(failure, operation, std::format("GetObject({}) failed: {}", prepared.className, hrText(failure)));
  }

  prepared.methodName = UniqueBstr(utf8ToWide(methodName));
  windows::ComPtr<IWbemClassObject> signature;
  hr = classObject->GetMethod(prepared.methodName, 0, signature.put(), nullptr);
  if (FAILED(hr)) throwComFailure(hr, operation, std::format("GetMethod({}::{}) failed: {}", prepared.className, methodName, hrText(hr)));

  if (signature) {
    hr = signature->SpawnInstance(0, prepared.inputs.put());
    if (FAILED(hr)) throwComFailure(hr, operation, std::format("SpawnInstance({}::{}) failed: {}", prepared.className, methodName, hrText(hr)));
  }
  if (!inputs.empty() && !prepared.inputs)
    throw app::wmi::WmiProtocolError(operation, std::format("{}::{} has no input signature", prepared.className, methodName));
  for (const auto& [name, value] : inputs) {
    UniqueVariant variant;
    valueToVariant(value, variant.get(), operation, "input parameter " + name);
    const UniqueBstr parameterName(utf8ToWide(name));
    const HRESULT putHr = prepared.inputs->Put(parameterName, 0, &variant.get(), 0);
    if (FAILED(putHr)) throwComFailure(putHr, operation, std::format("inParams->Put({}) failed: {}", name, hrText(putHr)));
  }

  prepared.objectPath = UniqueBstr(utf8ToWide(objectPath));
  return prepared;
}

app::wmi::WmiMethodOutput decodeMethodOutput(IWbemClassObject& output, const std::string& className, const std::string& methodName) {
  const std::string operation = std::format("invoke WMI method {}::{}", className, methodName);

  // ReturnValue 是 provider 对调用本身的权威结论，必须先于其余输出解码。
  // 若 provider 已明确拒绝，附带输出中的损坏不能把 Rejected 错误覆盖成
  // Uncertain；只有 ReturnValue==0 后的本地解码失败才代表“调用可能已落地”。
  UniqueVariant returnValueVariant;
  CIMTYPE returnValueType = 0;
  const UniqueBstr returnValueName(L"ReturnValue");
  const HRESULT returnValueHr = output.Get(returnValueName, 0, returnValueVariant.put(), &returnValueType, nullptr);
  if (isWmiConnectionFailure(static_cast<int32_t>(returnValueHr))) {
    throw app::wmi::WmiTransportError(static_cast<int32_t>(returnValueHr), operation, std::format("reading ReturnValue failed: {}", hrText(returnValueHr)));
  }
  if (FAILED(returnValueHr)) {
    throw app::wmi::WmiDecodeError(operation, std::format("reading ReturnValue failed: {}", hrText(returnValueHr)));
  }
  uint32_t providerReturnValue = 0;
  try {
    providerReturnValue = variantToValue(returnValueVariant.get(), returnValueType, operation).toUInt();
  } catch (const app::wmi::WmiValueConversionError& error) {
    throw app::wmi::WmiProtocolError(operation, std::format("response has an invalid ReturnValue: {}", error.what()));
  }
  if (providerReturnValue != 0) throw app::wmi::WmiProviderError(providerReturnValue, operation);

  app::wmi::WmiMethodOutput decoded;
  WbemEnumeration enumeration(output, operation);
  while (true) {
    BSTR rawName = nullptr;
    UniqueVariant value;
    CIMTYPE type = 0;
    const HRESULT nextHr = output.Next(0, &rawName, value.put(), &type, nullptr);
    const UniqueBstr name = UniqueBstr::adopt(rawName);
    if (nextHr == WBEM_S_NO_MORE_DATA) break;
    if (nextHr != WBEM_S_NO_ERROR) {
      if (isWmiConnectionFailure(static_cast<int32_t>(nextHr)))
        throw app::wmi::WmiTransportError(static_cast<int32_t>(nextHr), operation, std::format("output enumeration failed: {}", hrText(nextHr)));
      throw app::wmi::WmiDecodeError(operation, std::format("output enumeration failed: {}", hrText(nextHr)));
    }
    if (!rawName) throw app::wmi::WmiDecodeError(operation, "output enumeration returned a value without a name");

    const std::string key = bstrToUtf8(name.get());
    if (value.get().vt & VT_ARRAY) {
      if (decoded.values.contains(key) || !decoded.arrays.emplace(key, expandArrayVariant(value.get(), operation)).second) {
        throw app::wmi::WmiDecodeError(operation, "response returned duplicate output '" + key + "'");
      }
    } else {
      if (decoded.arrays.contains(key) || !decoded.values.emplace(key, variantToValue(value.get(), type, operation)).second) {
        throw app::wmi::WmiDecodeError(operation, "response returned duplicate output '" + key + "'");
      }
    }
  }
  enumeration.finish(operation);

  return decoded;
}

app::wmi::WmiMethodOutput decodeWriteMethodOutput(IWbemClassObject& output, const std::string& className, const std::string& methodName,
                                                  const std::string& operation) {
  try {
    return decodeMethodOutput(output, className, methodName);
  } catch (const app::wmi::WmiProviderError&) {
    // ReturnValue 是 provider 的明确拒绝，因此不属于结果不确定的写操作。
    throw;
  } catch (const app::wmi::WmiException& error) {
    if (error.code().category() == app::wmi::wmiErrorCategory()) {
      std::throw_with_nested(app::wmi::WmiInvocationUncertain(static_cast<int32_t>(error.code().value()), operation,
                                                              "provider outcome could not be decoded: " + std::string(error.what())));
    }
    std::throw_with_nested(app::wmi::WmiInvocationUncertain(operation, "provider outcome could not be decoded: " + std::string(error.what())));
  } catch (const std::exception& error) {
    std::throw_with_nested(app::wmi::WmiInvocationUncertain(operation, "provider outcome could not be decoded: " + std::string(error.what())));
  } catch (...) {
    throw app::wmi::WmiInvocationUncertain(operation, "provider outcome could not be decoded because of a non-standard exception");
  }
}

struct AsyncMethodState {
  SRWLOCK lock = SRWLOCK_INIT;
  CONDITION_VARIABLE changed = CONDITION_VARIABLE_INIT;
  bool completed = false;
  HRESULT status = E_UNEXPECTED;
  windows::ComPtr<IWbemClassObject> output;
};

class SrwExclusiveLock final {
 public:
  explicit SrwExclusiveLock(SRWLOCK& lock) noexcept : m_lock(&lock) { AcquireSRWLockExclusive(m_lock); }
  ~SrwExclusiveLock() {
    if (m_lock) ReleaseSRWLockExclusive(m_lock);
  }
  SrwExclusiveLock(const SrwExclusiveLock&) = delete;
  SrwExclusiveLock& operator=(const SrwExclusiveLock&) = delete;

  void unlock() noexcept {
    ReleaseSRWLockExclusive(m_lock);
    m_lock = nullptr;
  }
  void lock(SRWLOCK& value) noexcept {
    AcquireSRWLockExclusive(&value);
    m_lock = &value;
  }

 private:
  SRWLOCK* m_lock;
};

class AsyncMethodSink final : public IWbemObjectSink {
 public:
  explicit AsyncMethodSink(std::shared_ptr<AsyncMethodState> state) : m_state(std::move(state)) {}

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) noexcept override {
    if (!object) return E_POINTER;
    if (iid == IID_IUnknown || iid == IID_IWbemObjectSink) {
      *object = static_cast<IWbemObjectSink*>(this);
      AddRef();
      return S_OK;
    }
    *object = nullptr;
    return E_NOINTERFACE;
  }

  ULONG STDMETHODCALLTYPE AddRef() noexcept override { return ++m_refs; }
  ULONG STDMETHODCALLTYPE Release() noexcept override {
    const ULONG refs = --m_refs;
    // The COM ABI transfers ownership through AddRef/Release. The final release
    // destroys the sink; consumers hold it through ComPtr, not a second owner.
    if (refs == 0) delete this;
    return refs;
  }

  HRESULT STDMETHODCALLTYPE Indicate(const LONG count, IWbemClassObject** objects) noexcept override {
    if (count != 1 || !objects || !objects[0]) return static_cast<HRESULT>(WBEM_E_FAILED);
    SrwExclusiveLock lock(m_state->lock);
    if (m_state->output) return static_cast<HRESULT>(WBEM_E_FAILED);
    objects[0]->AddRef();
    m_state->output = windows::ComPtr<IWbemClassObject>::adopt(objects[0]);
    return WBEM_S_NO_ERROR;
  }

  HRESULT STDMETHODCALLTYPE SetStatus(const LONG flags, const HRESULT result, BSTR, IWbemClassObject*) noexcept override {
    if (flags != WBEM_STATUS_COMPLETE) return WBEM_S_NO_ERROR;
    {
      SrwExclusiveLock lock(m_state->lock);
      m_state->status = result;
      m_state->completed = true;
    }
    WakeAllConditionVariable(&m_state->changed);
    return WBEM_S_NO_ERROR;
  }

 private:
  std::atomic<ULONG> m_refs{1};
  std::shared_ptr<AsyncMethodState> m_state;
};

}  // namespace

struct WmiSession::Impl {
  explicit Impl(std::string path) : namespacePath(std::move(path)), ownerThread(std::this_thread::get_id()) {}

  class OperationGuard {
   public:
    explicit OperationGuard(Impl& owner) : m_owner(owner) {
      if (owner.ownerThread != std::this_thread::get_id()) {
        throw app::wmi::WmiProtocolError("access WMI session", "thread-local session was used from a different thread");
      }
      m_acquired = !owner.operationActive;
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

  windows::ComPtr<IWbemLocator> locator;
  windows::ComPtr<IWbemServices> services;
  std::string namespacePath;
  std::thread::id ownerThread;
  bool operationActive = false;

  void reset() {
    services.reset();
    locator.reset();
  }
};

WmiSession::WmiSession(std::string namespacePath) : m_impl(std::make_unique<Impl>(std::move(namespacePath))) {}
std::function<app::wmi::WmiOperations&()> WmiSession::threadAccessor() {
  const bool embedded = m_impl->namespacePath == windows::kWmiNamespaceEmbedded;
  return [embedded]() -> app::wmi::WmiOperations& { return embedded ? embeddedWmiSession() : cimv2WmiSession(); };
}
WmiSession::~WmiSession() = default;

void WmiSession::invalidate() const { m_impl->reset(); }

void WmiSession::ensureConnected() const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw app::wmi::WmiProtocolError("connect WMI session", "reentrant request on the same thread");
  for (int attempt = 0; attempt < 2; ++attempt) {
    try {
      ensureConnectedWithinOperation();
      return;
    } catch (const app::wmi::WmiTransportError&) {
      invalidate();
      if (attempt == 0) continue;
      throw;
    }
  }
  throw app::wmi::WmiProtocolError("connect WMI session", "retry loop exhausted without a connection");
}

void WmiSession::ensureConnectedWithinOperation() const {
  if (!m_impl->services) reconnectWithinOperation();
}

void WmiSession::reconnectWithinOperation() const {
  UWF_LOG_D("wmi") << "connection started: namespace=" << m_impl->namespacePath;

  m_impl->reset();

  ULONG_PTR contextToken = 0;
  HRESULT hr = CoGetContextToken(&contextToken);
  if (FAILED(hr)) {
    throw app::wmi::WmiInfrastructureError(static_cast<int32_t>(hr), "obtain COM apartment", hrText(hr));
  }

  hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, m_impl->locator.putVoid());
  if (FAILED(hr) || !m_impl->locator) {
    const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
    m_impl->reset();
    throw app::wmi::WmiInfrastructureError(static_cast<int32_t>(failure), "create WMI locator", hrText(failure));
  }

  const std::wstring nsW = utf8ToWide(m_impl->namespacePath);
  const UniqueBstr ns(nsW);
  hr = m_impl->locator->ConnectServer(ns, nullptr, nullptr, nullptr, WBEM_FLAG_CONNECT_USE_MAX_WAIT, nullptr, nullptr, m_impl->services.put());
  if (FAILED(hr) || !m_impl->services) {
    const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
    m_impl->reset();
    throwComFailure(failure, "connect WMI namespace " + m_impl->namespacePath, hrText(failure));
  }

  hr = CoSetProxyBlanket(m_impl->services.get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr,
                         EOAC_NONE);
  if (FAILED(hr)) {
    m_impl->reset();
    throwComFailure(hr, "configure WMI proxy security", hrText(hr));
  }
  UWF_LOG_I("wmi") << "connection established: namespace=" << m_impl->namespacePath;
}

std::vector<app::wmi::WmiRow> WmiSession::query(const std::string& wql) const { return executeQuery(wql, QueryMode::Projection); }

std::vector<app::wmi::WmiRow> WmiSession::queryInstances(const std::string& wql) const { return executeQuery(wql, QueryMode::LocatableInstances); }

std::vector<app::wmi::WmiRow> WmiSession::executeQuery(const std::string& wql, const QueryMode mode) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw app::wmi::WmiProtocolError("execute WMI query", "reentrant request on the same thread");
  const auto deadline = std::chrono::steady_clock::now() + kQueryTimeout;
  for (int attempt = 0; attempt < 2; ++attempt) {
    try {
      ensureConnectedWithinOperation();
      std::vector<app::wmi::WmiRow> rows;
      const UniqueBstr language(L"WQL");
      const UniqueBstr queryText(utf8ToWide(wql));
      windows::ComPtr<IEnumWbemClassObject> enumerator;
      const long flags = WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY | (mode == QueryMode::LocatableInstances ? WBEM_FLAG_ENSURE_LOCATABLE : 0);
      const HRESULT queryHr = m_impl->services->ExecQuery(language, queryText, flags, nullptr, enumerator.put());
      if (FAILED(queryHr)) throwComFailure(queryHr, "execute WMI query", std::format("{}; WQL: {}", hrText(queryHr), wql));
      if (!enumerator) throw app::wmi::WmiProtocolError("execute WMI query", std::format("provider returned no enumerator; WQL: {}", wql));

      while (true) {
        if (std::chrono::steady_clock::now() >= deadline)
          throw app::wmi::WmiInfrastructureError(static_cast<int32_t>(WBEM_E_TIMED_OUT), "enumerate WMI query",
                                                 std::format("query deadline exceeded; WQL: {}", wql));
        windows::ComPtr<IWbemClassObject> obj;
        ULONG got = 0;
        const HRESULT nextHr = enumerator->Next(kEnumerationPollMilliseconds, 1, obj.put(), &got);
        if (FAILED(nextHr)) throwComFailure(nextHr, "enumerate WMI query", std::format("{}; WQL: {}", hrText(nextHr), wql));
        if (got > 1 || (got == 1) != static_cast<bool>(obj)) {
          throw app::wmi::WmiProtocolError("enumerate WMI query", std::format("inconsistent row count/object pair; WQL: {}", wql));
        }
        if (got == 0) {
          if (nextHr == WBEM_S_FALSE) break;
          if (nextHr == WBEM_S_TIMEDOUT) continue;
          throw app::wmi::WmiProtocolError("enumerate WMI query", std::format("no object without completion status {}; WQL: {}", hrText(nextHr), wql));
        }
        const PathRequirement pathRequirement = mode == QueryMode::LocatableInstances ? PathRequirement::Required : PathRequirement::Optional;
        rows.push_back(readObjectProps(*obj, pathRequirement, std::format("decode WMI query row {}; WQL: {}", rows.size(), wql)));
        if (nextHr == WBEM_S_FALSE) break;
      }
      UWF_LOG_D("wmi") << "query completed: rows=" << rows.size() << " wql=" << wql;
      return rows;
    } catch (const app::wmi::WmiTransportError&) {
      invalidate();
      if (attempt == 0) continue;
      throw;
    }
  }
  throw app::wmi::WmiProtocolError("execute WMI query", "retry loop exhausted without a result");
}

app::wmi::WmiClassStatus WmiSession::classStatus(const std::string& className) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw app::wmi::WmiProtocolError("probe WMI class", "reentrant request on the same thread");
  for (int attempt = 0; attempt < 2; ++attempt) {
    try {
      ensureConnectedWithinOperation();
      const UniqueBstr classPath(utf8ToWide(className));
      windows::ComPtr<IWbemClassObject> classObject;
      const HRESULT hr = m_impl->services->GetObject(classPath, 0, nullptr, classObject.put(), nullptr);
      if (hr == static_cast<HRESULT>(WBEM_E_INVALID_CLASS) || hr == static_cast<HRESULT>(WBEM_E_NOT_FOUND)) return app::wmi::WmiClassStatus::Missing;
      if (FAILED(hr)) throwComFailure(hr, "probe WMI class " + className, hrText(hr));
      if (!classObject) throw app::wmi::WmiProtocolError("probe WMI class " + className, "provider returned no class object");
      return app::wmi::WmiClassStatus::Present;
    } catch (const app::wmi::WmiTransportError&) {
      invalidate();
      if (attempt == 0) continue;
      throw;
    }
  }
  throw app::wmi::WmiProtocolError("probe WMI class " + className, "retry loop exhausted without a result");
}

app::wmi::WmiRow WmiSession::getObject(const std::string& objectPath) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw app::wmi::WmiProtocolError("read WMI object", "reentrant request on the same thread");
  for (int attempt = 0; attempt < 2; ++attempt) {
    try {
      ensureConnectedWithinOperation();
      const UniqueBstr path(utf8ToWide(objectPath));
      windows::ComPtr<IWbemClassObject> object;
      const HRESULT hr = m_impl->services->GetObject(path, 0, nullptr, object.put(), nullptr);
      if (FAILED(hr)) throwComFailure(hr, "read WMI object " + objectPath, hrText(hr));
      if (!object) throw app::wmi::WmiProtocolError("read WMI object " + objectPath, "provider returned no object");
      return readObjectProps(*object, PathRequirement::Required, "decode WMI object " + objectPath);
    } catch (const app::wmi::WmiTransportError&) {
      invalidate();
      if (attempt == 0) continue;
      throw;
    }
  }
  throw app::wmi::WmiProtocolError("read WMI object " + objectPath, "retry loop exhausted without a result");
}

app::wmi::WmiMethodOutput WmiSession::executeReadMethodOnce(const std::string& objectPath, const std::string& methodName,
                                                            const app::wmi::WmiRow& inputs) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw app::wmi::WmiProtocolError("invoke WMI method " + methodName, "reentrant request on the same thread");
  const std::string operationName = std::format("invoke WMI method {} on {}", methodName, objectPath);
  try {
    ensureConnectedWithinOperation();
    PreparedMethodCall prepared = prepareMethodCall(*m_impl->services, objectPath, methodName, inputs);
    windows::ComPtr<IWbemClassObject> output;
    const HRESULT hr = m_impl->services->ExecMethod(prepared.objectPath, prepared.methodName, 0, nullptr, prepared.inputs.get(), output.put(), nullptr);
    if (FAILED(hr)) {
      std::string detail = hrText(hr);
      if (const std::string extended = extendedErrorDetail(); !extended.empty()) detail += std::format("; provider detail: {}", extended);
      throwComFailure(hr, operationName, std::move(detail));
    }
    if (!output) throw app::wmi::WmiProtocolError(operationName, "provider returned no output parameters");
    return decodeMethodOutput(*output, prepared.className, methodName);
  } catch (const app::wmi::WmiTransportError&) {
    invalidate();
    throw;
  }
}

void WmiSession::invokeMethod(const std::string& objectPath, const std::string& methodName, const app::wmi::WmiRow& inputs) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw app::wmi::WmiProtocolError("invoke WMI method " + methodName, "reentrant request on the same thread");
  const std::string operationName = std::format("invoke WMI method {} on {}", methodName, objectPath);
  try {
    ensureConnectedWithinOperation();
    PreparedMethodCall prepared = prepareMethodCall(*m_impl->services, objectPath, methodName, inputs);
    windows::ComPtr<IWbemClassObject> output;
    const HRESULT hr = m_impl->services->ExecMethod(prepared.objectPath, prepared.methodName, 0, nullptr, prepared.inputs.get(), output.put(), nullptr);
    if (FAILED(hr)) {
      std::string detail = hrText(hr);
      if (const std::string extended = extendedErrorDetail(); !extended.empty()) detail += std::format("; provider detail: {}", extended);
      throwWriteInvocationFailure(hr, operationName, std::move(detail));
    }
    if (!output) throw app::wmi::WmiInvocationUncertain(operationName, "provider returned no output parameters");
    (void)decodeWriteMethodOutput(*output, prepared.className, methodName, operationName);
  } catch (const app::wmi::WmiTransportError&) {
    // 连接在准备阶段失效，写操作尚未发出。
    invalidate();
    throw;
  } catch (const app::wmi::WmiInvocationUncertain& error) {
    if (error.code().category() == app::wmi::wmiErrorCategory() && isWmiConnectionFailure(static_cast<int32_t>(error.code().value()))) invalidate();
    throw;
  }
}

app::wmi::WmiMethodOutput WmiSession::callMethodRead(const std::string& objectPath, const std::string& methodName, const app::wmi::WmiRow& inputs) const {
  try {
    return executeReadMethodOnce(objectPath, methodName, inputs);
  } catch (const app::wmi::WmiTransportError&) {
    return executeReadMethodOnce(objectPath, methodName, inputs);
  }
}

app::wmi::WmiMethodOutput WmiSession::callMethodReadCancelable(const std::string& objectPath, const std::string& methodName, const app::wmi::WmiRow& inputs,
                                                               const std::stop_token stopToken) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw app::wmi::WmiProtocolError("invoke cancellable WMI method " + methodName, "reentrant request on the same thread");
  const std::string operationName = std::format("invoke cancellable WMI method {} on {}", methodName, objectPath);
  const auto deadline = std::chrono::steady_clock::now() + kQueryTimeout;
  for (int attempt = 0; attempt < 2; ++attempt) {
    try {
      if (stopToken.stop_requested()) throw app::wmi::WmiCancelled(operationName);
      ensureConnectedWithinOperation();
      PreparedMethodCall prepared = prepareMethodCall(*m_impl->services, objectPath, methodName, inputs);
      auto state = std::make_shared<AsyncMethodState>();
      auto sink = windows::ComPtr<AsyncMethodSink>::adopt(new AsyncMethodSink(state));
      const HRESULT startHr = m_impl->services->ExecMethodAsync(prepared.objectPath, prepared.methodName, 0, nullptr, prepared.inputs.get(), sink.get());
      if (FAILED(startHr)) throwComFailure(startHr, operationName, std::format("ExecMethodAsync failed: {}", hrText(startHr)));

      std::stop_callback wakeOnStop(stopToken, [state = state.get()]() noexcept {
        // 修改等待谓词与唤醒使用同一把锁，避免 stop 恰好落在谓词检查和
        // SleepConditionVariableSRW 原子释放锁之间时丢失通知。
        SrwExclusiveLock lock(state->lock);
        WakeAllConditionVariable(&state->changed);
      });
      SrwExclusiveLock lock(state->lock);
      while (!state->completed && !stopToken.stop_requested()) {
        if (!SleepConditionVariableSRW(&state->changed, &state->lock, static_cast<DWORD>(kEnumerationPollMilliseconds), 0)) {
          const DWORD waitError = GetLastError();
          if (waitError == ERROR_TIMEOUT && std::chrono::steady_clock::now() < deadline) continue;
          const HRESULT waitHr = HRESULT_FROM_WIN32(waitError);
          lock.unlock();
          const HRESULT cancelHr = m_impl->services->CancelAsyncCall(sink.get());
          std::string detail = std::format("waiting for asynchronous completion failed: {}", hrText(waitHr));
          if (FAILED(cancelHr)) detail += std::format("; cancellation also failed: {}", hrText(cancelHr));
          throw app::wmi::WmiInfrastructureError(static_cast<int32_t>(waitHr), operationName, std::move(detail));
        }
      }
      if (stopToken.stop_requested() && !state->completed) {
        lock.unlock();
        const HRESULT cancelHr = m_impl->services->CancelAsyncCall(sink.get());
        if (SUCCEEDED(cancelHr)) throw app::wmi::WmiCancelled(operationName);

        // 完成通知可能恰好越过 stop 检查。取消失败后重新观察受互斥量保护的
        // 终态；若调用已经完成，就以 provider 的完成状态为准，而不是把一个
        // 已完成的只读请求误报成取消失败。
        lock.lock(state->lock);
        if (!state->completed) {
          lock.unlock();
          throwComFailure(cancelHr, operationName, std::format("CancelAsyncCall failed: {}", hrText(cancelHr)));
        }
      }

      const HRESULT completionHr = state->status;
      auto output = windows::ComPtr<IWbemClassObject>::retain(state->output.get());
      lock.unlock();
      if (FAILED(completionHr)) throwComFailure(completionHr, operationName, std::format("asynchronous completion failed: {}", hrText(completionHr)));
      if (!output) throw app::wmi::WmiProtocolError(operationName, "provider completed without output parameters");
      return decodeMethodOutput(*output, prepared.className, methodName);
    } catch (const app::wmi::WmiTransportError&) {
      invalidate();
      if (attempt == 0) continue;
      throw;
    }
  }
  throw app::wmi::WmiProtocolError(operationName, "retry loop exhausted without a result");
}

void WmiSession::putInstance(const std::string& className, const app::wmi::WmiRow& props, const app::wmi::WmiPutMode mode) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw app::wmi::WmiProtocolError("write WMI instance " + className, "reentrant request on the same thread");
  const std::string operationName = "write WMI instance " + className;
  try {
    ensureConnectedWithinOperation();
    const UniqueBstr classPath(utf8ToWide(className));
    windows::ComPtr<IWbemClassObject> classObject;
    HRESULT hr = m_impl->services->GetObject(classPath, 0, nullptr, classObject.put(), nullptr);
    if (FAILED(hr)) throwComFailure(hr, operationName, std::format("GetObject failed: {}", hrText(hr)));
    if (!classObject) throw app::wmi::WmiProtocolError(operationName, "provider returned no class object");

    windows::ComPtr<IWbemClassObject> instance;
    hr = classObject->SpawnInstance(0, instance.put());
    if (FAILED(hr)) throwComFailure(hr, operationName, std::format("SpawnInstance failed: {}", hrText(hr)));
    if (!instance) throw app::wmi::WmiProtocolError(operationName, "provider returned no writable instance");

    for (const auto& [name, value] : props) {
      UniqueVariant variant;
      valueToVariant(value, variant.get(), operationName, "property " + name);
      const UniqueBstr propertyName(utf8ToWide(name));
      const HRESULT putHr = instance->Put(propertyName, 0, &variant.get(), 0);
      if (FAILED(putHr)) throwComFailure(putHr, operationName, std::format("setting property {} failed: {}", name, hrText(putHr)));
    }

    const long flags = mode == app::wmi::WmiPutMode::CreateOnly ? WBEM_FLAG_CREATE_ONLY : WBEM_FLAG_UPDATE_ONLY;
    hr = m_impl->services->PutInstance(instance.get(), flags, nullptr, nullptr);
    if (FAILED(hr)) throwWriteInvocationFailure(hr, operationName, hrText(hr));
  } catch (const app::wmi::WmiTransportError&) {
    invalidate();
    throw;
  } catch (const app::wmi::WmiInvocationUncertain& error) {
    if (error.code().category() == app::wmi::wmiErrorCategory() && isWmiConnectionFailure(static_cast<int32_t>(error.code().value()))) invalidate();
    throw;
  }
}

namespace {

bool isWmiConnectionFailure(const int32_t hresult) {
  const auto hr = static_cast<HRESULT>(hresult);
  return hr == RPC_E_DISCONNECTED || hr == RPC_E_SERVER_DIED || hr == RPC_E_SERVER_DIED_DNE || hr == CO_E_SERVER_STOPPING || hr == CO_E_OBJNOTCONNECTED ||
         hr == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) || hr == HRESULT_FROM_WIN32(RPC_S_CALL_FAILED) || hr == HRESULT_FROM_WIN32(RPC_S_CALL_FAILED_DNE) ||
         hr == static_cast<HRESULT>(WBEM_E_TRANSPORT_FAILURE) || hr == static_cast<HRESULT>(WBEM_E_FATAL_TRANSPORT_ERROR) ||
         hr == static_cast<HRESULT>(WBEM_E_CONNECTION_FAILED) || hr == static_cast<HRESULT>(WBEM_E_SHUTTING_DOWN);
}

bool isKnownPreInvocationRejection(const int32_t hresult) {
  // HRESULT 是开放集合，provider 也可以定义自己的错误。这里只列出 WMI
  // 明确在请求校验阶段返回的已知拒绝；未知错误必须保守地视为可能已执行。
  static constexpr std::array kKnownRejections{
      E_INVALIDARG,
      E_ACCESSDENIED,
      static_cast<HRESULT>(WBEM_E_ACCESS_DENIED),
      static_cast<HRESULT>(WBEM_E_INVALID_PARAMETER),
      static_cast<HRESULT>(WBEM_E_INVALID_METHOD),
      static_cast<HRESULT>(WBEM_E_INVALID_METHOD_PARAMETERS),
      static_cast<HRESULT>(WBEM_E_NOT_SUPPORTED),
      static_cast<HRESULT>(WBEM_E_INVALID_OBJECT_PATH),
      static_cast<HRESULT>(WBEM_E_INVALID_CLASS),
      static_cast<HRESULT>(WBEM_E_NOT_FOUND),
      static_cast<HRESULT>(WBEM_E_ALREADY_EXISTS),
      static_cast<HRESULT>(WBEM_E_TYPE_MISMATCH),
      static_cast<HRESULT>(WBEM_E_READ_ONLY),
      static_cast<HRESULT>(WBEM_E_INVALID_OPERATION),
      static_cast<HRESULT>(WBEM_E_INVALID_PROPERTY),
      static_cast<HRESULT>(WBEM_E_METHOD_DISABLED),
  };
  const auto hr = static_cast<HRESULT>(hresult);
  return std::ranges::find(kKnownRejections, hr) != kKnownRejections.end();
}

class ComApartmentLease final {
 public:
  ComApartmentLease() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    // Qt 的 GUI 线程通常已是 STA。不能只把 RPC_E_CHANGED_MODE 当成“COM 已经
    // 可用”：那样本模块没有自己的初始化引用，QApplication 先析构后可能在
    // thread_local session 释放代理之前把 apartment 关闭。用既有模式再初始化
    // 一次，明确取得与 session 生命周期配对的引用。
    if (hr == RPC_E_CHANGED_MODE) hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) throw app::wmi::WmiInfrastructureError(static_cast<int32_t>(hr), "initialize COM apartment", hrText(hr));
  }

  ~ComApartmentLease() { CoUninitialize(); }

  ComApartmentLease(const ComApartmentLease&) = delete;
  ComApartmentLease& operator=(const ComApartmentLease&) = delete;
};

class ThreadComApartment final {
 public:
  ThreadComApartment() {
    static std::once_flag securityOnce;
    static HRESULT securityResult = E_UNEXPECTED;
    std::call_once(securityOnce, [] {
      securityResult = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
    });
    if (FAILED(securityResult) && securityResult != RPC_E_TOO_LATE) {
      throw app::wmi::WmiInfrastructureError(static_cast<int32_t>(securityResult), "initialize COM security", hrText(securityResult));
    }
  }

  ThreadComApartment(const ThreadComApartment&) = delete;
  ThreadComApartment& operator=(const ThreadComApartment&) = delete;

 private:
  // 必须先于安全策略初始化；ThreadComApartment 后续构造中的
  // 任何异常都会自动销毁此租约，不留下未配对的 COM 引用。
  ComApartmentLease m_lease;
};

}  // namespace

struct WmiThreadContext {
  // 声明顺序很重要：逆序析构保证两个 session 的 COM 代理先于 apartment 释放。
  ThreadComApartment apartment;
  WmiSession embedded{windows::kWmiNamespaceEmbedded};
  WmiSession cimv2{windows::kWmiNamespaceCimv2};
};

namespace {

thread_local std::optional<WmiThreadContext> threadContextStorage;
thread_local bool threadContextRetired = false;

WmiThreadContext& threadContext() {
  if (threadContextRetired) throw std::logic_error("WMI thread context was already destroyed");
  if (!threadContextStorage) threadContextStorage.emplace();
  return *threadContextStorage;
}

}  // namespace

app::wmi::WmiOperations& embeddedWmiSession() { return threadContext().embedded; }
app::wmi::WmiOperations& cimv2WmiSession() { return threadContext().cimv2; }
void initializeWmiRuntime() { static_cast<void>(threadContext()); }
void shutdownWmiRuntime() noexcept {
  threadContextStorage.reset();
  threadContextRetired = true;
}

}  // namespace uwf::windows
