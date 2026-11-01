// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/management/wmi/WmiSession.h"

#include <comdef.h>
#include <oleauto.h>
#include <wbemidl.h>
#include <windows.h>

#include <algorithm>
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

#include "foundation/logging/Log.h"
#include "foundation/text/StringUtil.h"
#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/management/wmi/WmiMethodCompletion.h"
#include "infrastructure/windows/management/wmi/WmiNamespaces.h"
#include "infrastructure/windows/resources/ComPtr.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::windows {

namespace {
constexpr auto kQueryTimeout = std::chrono::seconds(30);
constexpr LONG kEnumerationPollMilliseconds = 100;

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
      throw windows::wmi::WmiTransportError(static_cast<int32_t>(hr), operation, std::format("BeginEnumeration failed: {}", hrText(hr)));
    }
    if (FAILED(hr)) throw windows::wmi::WmiDecodeError(operation, std::format("BeginEnumeration failed: {}", hrText(hr)));
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
      throw windows::wmi::WmiTransportError(static_cast<int32_t>(hr), operation, std::format("EndEnumeration failed: {}", hrText(hr)));
    }
    if (FAILED(hr)) throw windows::wmi::WmiDecodeError(operation, std::format("EndEnumeration failed: {}", hrText(hr)));
  }

 private:
  IWbemClassObject& m_object;
  bool m_finished = false;
};

std::string hrText(const HRESULT hr) {
  std::string description;
  try {
    const _com_error error(hr);
    description = wideToUtf8(error.ErrorMessage());
  } catch (...) {
    // Message conversion must not obscure the original HRESULT.
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

windows::wmi::WmiValue variantToValue(const VARIANT& v, const CIMTYPE cimType, const std::string& operation) {
  switch (v.vt) {
    case VT_NULL:
    case VT_EMPTY:
      return {};
    case VT_BOOL:
      return windows::wmi::WmiValue::fromBool(v.boolVal != VARIANT_FALSE);
    case VT_I1:
      if (cimType == CIM_UINT8) return windows::wmi::WmiValue::fromUInt(static_cast<std::uint8_t>(v.cVal));
      return windows::wmi::WmiValue::fromInt(static_cast<std::int8_t>(v.cVal));
    case VT_UI1:
      return windows::wmi::WmiValue::fromUInt(v.bVal);
    case VT_I2:
      if (cimType == CIM_UINT16) return windows::wmi::WmiValue::fromUInt(static_cast<uint16_t>(v.iVal));
      return windows::wmi::WmiValue::fromInt(v.iVal);
    case VT_I4:
      // WMI boxes CIM_UINT32 as VT_I4; CIMTYPE preserves unsigned HRESULT bits.
      if (cimType == CIM_UINT32) return windows::wmi::WmiValue::fromUInt(static_cast<uint32_t>(v.lVal));
      return windows::wmi::WmiValue::fromInt(v.lVal);
    case VT_UI2:
      return windows::wmi::WmiValue::fromUInt(v.uiVal);
    case VT_UI4:
      return windows::wmi::WmiValue::fromUInt(v.ulVal);
    case VT_INT:
      if (cimType == CIM_UINT32) return windows::wmi::WmiValue::fromUInt(static_cast<std::uint32_t>(v.intVal));
      return windows::wmi::WmiValue::fromInt(v.intVal);
    case VT_UINT:
      return windows::wmi::WmiValue::fromUInt(v.uintVal);
    case VT_I8:
      if (cimType == CIM_UINT64) return windows::wmi::WmiValue::fromUInt(static_cast<uint64_t>(v.llVal));
      return windows::wmi::WmiValue::fromInt(v.llVal);
    case VT_UI8:
      return windows::wmi::WmiValue::fromUInt(v.ullVal);
    case VT_R4:
      return windows::wmi::WmiValue::fromDouble(v.fltVal);
    case VT_R8:
      return windows::wmi::WmiValue::fromDouble(v.dblVal);
    case VT_BSTR:
      return windows::wmi::WmiValue::fromString(bstrToUtf8(v.bstrVal));
    case VT_DATE:
      return windows::wmi::WmiValue::fromDouble(v.date);
    default:
      throw windows::wmi::WmiDecodeError(operation, std::format("unsupported VARIANT type {}", static_cast<unsigned>(v.vt)));
  }
}

// Encodes supported scalars; the caller owns VariantClear.
void valueToVariant(const windows::wmi::WmiValue& src, VARIANT& v, const std::string& operation, const std::string& subject) {
  VariantInit(&v);
  switch (src.kind()) {
    case windows::wmi::WmiValue::Kind::Bool:
      v.vt = VT_BOOL;
      v.boolVal = src.toBool() ? VARIANT_TRUE : VARIANT_FALSE;
      return;
    case windows::wmi::WmiValue::Kind::Int: {
      const int64_t value = src.toInt64();
      if (value < std::numeric_limits<LONG>::min() || value > std::numeric_limits<LONG>::max()) {
        throw windows::wmi::WmiProtocolError(operation, subject + " is outside the supported Int32 range");
      }
      v.vt = VT_I4;
      v.lVal = static_cast<LONG>(value);
      return;
    }
    case windows::wmi::WmiValue::Kind::UInt: {
      // CIM_UINT32 inputs require VT_I4, not VT_UI4; see SDK "Numbers (WMI)":
      // learn.microsoft.com/windows/win32/wmisdk/numbers.
      const uint64_t value = src.toULongLong();
      if (value > std::numeric_limits<uint32_t>::max()) {
        throw windows::wmi::WmiProtocolError(operation, subject + " is outside the supported UInt32 input range");
      }
      v.vt = VT_I4;
      static_assert(sizeof(LONG) == sizeof(uint32_t));
      v.lVal = std::bit_cast<LONG>(static_cast<uint32_t>(value));
      return;
    }
    case windows::wmi::WmiValue::Kind::Double:
      v.vt = VT_R8;
      v.dblVal = src.toDouble();
      return;
    case windows::wmi::WmiValue::Kind::String: {
      const auto w = utf8ToWide(src.toString());
      UniqueBstr encoded(w);
      v.vt = VT_BSTR;
      v.bstrVal = encoded.release();
      return;
    }
    default:
      throw windows::wmi::WmiProtocolError(operation, subject + " is null or has an unsupported type");
  }
}

enum class PathRequirement { Optional, Required };

windows::wmi::WmiRow readObjectProps(IWbemClassObject& object, const PathRequirement pathRequirement, const std::string& operation) {
  windows::wmi::WmiRow row;
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
        throw windows::wmi::WmiTransportError(static_cast<int32_t>(hr), operation, std::format("property enumeration failed: {}", hrText(hr)));
      }
      throw windows::wmi::WmiDecodeError(operation, std::format("property enumeration failed: {}", hrText(hr)));
    }
    if (!rawName) throw windows::wmi::WmiDecodeError(operation, "property enumeration returned a value without a name");
    const std::string key = bstrToUtf8(name.get());
    if (!row.emplace(key, variantToValue(value.get(), type, operation)).second) {
      throw windows::wmi::WmiDecodeError(operation, "property enumeration returned duplicate field '" + key + "'");
    }
  }
  enumeration.finish(operation);

  UniqueVariant path;
  const UniqueBstr pathName(L"__PATH");
  const HRESULT pathHr = object.Get(pathName, 0, path.put(), nullptr, nullptr);
  if (isWmiConnectionFailure(static_cast<int32_t>(pathHr))) {
    throw windows::wmi::WmiTransportError(static_cast<int32_t>(pathHr), operation, std::format("Get(__PATH) failed: {}", hrText(pathHr)));
  }
  if (pathHr == static_cast<HRESULT>(WBEM_E_NOT_FOUND) && pathRequirement == PathRequirement::Optional) return row;
  if (FAILED(pathHr)) {
    throw windows::wmi::WmiDecodeError(operation, std::format("Get(__PATH) failed: {}", hrText(pathHr)));
  }
  if (path.get().vt == VT_BSTR && path.get().bstrVal && SysStringLen(path.get().bstrVal) != 0) {
    if (!row.emplace("__PATH", windows::wmi::WmiValue::fromString(bstrToUtf8(path.get().bstrVal))).second) {
      throw windows::wmi::WmiDecodeError(operation, "property enumeration returned duplicate field '__PATH'");
    }
  } else if (pathRequirement == PathRequirement::Required) {
    throw windows::wmi::WmiDecodeError(operation, std::format("locatable object has no string __PATH (VARIANT type {})", static_cast<unsigned>(path.get().vt)));
  }
  return row;
}

// Embedded output arrays contain IWbemClassObject references or BSTR MOF text.
std::vector<windows::wmi::WmiRow> expandArrayVariant(const VARIANT& v, const std::string& operation) {
  if (!(v.vt & VT_ARRAY) || !v.parray) {
    throw windows::wmi::WmiDecodeError(operation, "output value is not an array");
  }

  SAFEARRAY* sa = v.parray;
  if (SafeArrayGetDim(sa) != 1) {
    throw windows::wmi::WmiDecodeError(operation, "only one-dimensional WMI output arrays are supported");
  }
  VARTYPE vt = VT_EMPTY;
  HRESULT hr = SafeArrayGetVartype(sa, &vt);
  if (FAILED(hr)) throw windows::wmi::WmiDecodeError(operation, std::format("SafeArrayGetVartype failed: {}", hrText(hr)));
  LONG lb = 0, ub = 0;
  hr = SafeArrayGetLBound(sa, 1, &lb);
  if (FAILED(hr)) throw windows::wmi::WmiDecodeError(operation, std::format("SafeArrayGetLBound failed: {}", hrText(hr)));
  hr = SafeArrayGetUBound(sa, 1, &ub);
  if (FAILED(hr)) throw windows::wmi::WmiDecodeError(operation, std::format("SafeArrayGetUBound failed: {}", hrText(hr)));

  std::vector<windows::wmi::WmiRow> rows;
  if (ub < lb) return rows;
  for (LONG i = lb;; ++i) {
    if (vt == VT_UNKNOWN) {
      windows::ComPtr<IUnknown> unknown;
      hr = SafeArrayGetElement(sa, &i, unknown.put());
      if (FAILED(hr)) throw windows::wmi::WmiDecodeError(operation, std::format("SafeArrayGetElement({}) failed: {}", i, hrText(hr)));
      if (!unknown) throw windows::wmi::WmiDecodeError(operation, std::format("SafeArrayGetElement({}) returned a null IUnknown", i));
      windows::ComPtr<IWbemClassObject> item;
      hr = unknown->QueryInterface(IID_IWbemClassObject, item.putVoid());
      if (FAILED(hr) || !item) throw windows::wmi::WmiDecodeError(operation, std::format("array element is not an IWbemClassObject: {}", hrText(hr)));
      rows.push_back(readObjectProps(*item, PathRequirement::Optional, operation));
    } else if (vt == VT_BSTR) {
      BSTR raw = nullptr;
      hr = SafeArrayGetElement(sa, &i, &raw);
      if (FAILED(hr)) throw windows::wmi::WmiDecodeError(operation, std::format("SafeArrayGetElement({}) failed: {}", i, hrText(hr)));
      if (!raw) throw windows::wmi::WmiDecodeError(operation, std::format("SafeArrayGetElement({}) returned a null BSTR", i));
      const UniqueBstr string = UniqueBstr::adopt(raw);
      windows::wmi::WmiRow row;
      row.emplace("__MOF", windows::wmi::WmiValue::fromString(bstrToUtf8(string.get())));
      rows.push_back(std::move(row));
    } else {
      throw windows::wmi::WmiDecodeError(operation, std::format("unsupported WMI output array element type {}", static_cast<unsigned>(vt)));
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
    throw windows::wmi::WmiTransportError(static_cast<int32_t>(hr), std::move(operation), std::move(detail));
  }
  throw windows::wmi::WmiInfrastructureError(static_cast<int32_t>(hr), std::move(operation), std::move(detail));
}

// A dispatched write's HRESULT cannot rule out partial provider effects.
[[noreturn]] void throwWriteInvocationFailure(const HRESULT hr, std::string operation, std::string detail) {
  throw windows::wmi::WmiInvocationUncertain(static_cast<int32_t>(hr), std::move(operation), std::move(detail));
}

PreparedMethodCall prepareMethodCall(IWbemServices& services, const std::string& objectPath, const std::string& methodName,
                                     const windows::wmi::WmiRow& inputs) {
  const std::string operation = std::format("prepare WMI method {} on {}", methodName, objectPath);
  if (objectPath.empty()) throw windows::wmi::WmiProtocolError(operation, "object path is empty; read the row before invoking an instance method");

  PreparedMethodCall prepared;
  prepared.className = classNameFromObjectPath(objectPath);
  if (prepared.className.empty()) throw windows::wmi::WmiProtocolError(operation, "object path has no class name");
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
    throw windows::wmi::WmiProtocolError(operation, std::format("{}::{} has no input signature", prepared.className, methodName));
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

windows::wmi::WmiMethodOutput decodeMethodOutput(IWbemClassObject& output, const std::string& className, const std::string& methodName,
                                                 const windows::wmi::WriteEffect effect = windows::wmi::WriteEffect::None) {
  const std::string operation = std::format("invoke WMI method {}::{}", className, methodName);

  // Decode ReturnValue first to preserve failures even if other outputs are
  // malformed. A failed mutating method may retain earlier system effects.
  UniqueVariant returnValueVariant;
  CIMTYPE returnValueType = 0;
  const UniqueBstr returnValueName(L"ReturnValue");
  const HRESULT returnValueHr = output.Get(returnValueName, 0, returnValueVariant.put(), &returnValueType, nullptr);
  if (isWmiConnectionFailure(static_cast<int32_t>(returnValueHr))) {
    throw windows::wmi::WmiTransportError(static_cast<int32_t>(returnValueHr), operation, std::format("reading ReturnValue failed: {}", hrText(returnValueHr)));
  }
  if (FAILED(returnValueHr)) {
    throw windows::wmi::WmiDecodeError(operation, std::format("reading ReturnValue failed: {}", hrText(returnValueHr)));
  }
  uint32_t providerReturnValue = 0;
  try {
    providerReturnValue = variantToValue(returnValueVariant.get(), returnValueType, operation).toUInt();
  } catch (const windows::wmi::WmiValueConversionError& error) {
    throw windows::wmi::WmiProtocolError(operation, std::format("response has an invalid ReturnValue: {}", error.what()));
  }
  if (providerReturnValue != 0) throw windows::wmi::WmiProviderError(providerReturnValue, operation, {}, effect);

  windows::wmi::WmiMethodOutput decoded;
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
        throw windows::wmi::WmiTransportError(static_cast<int32_t>(nextHr), operation, std::format("output enumeration failed: {}", hrText(nextHr)));
      throw windows::wmi::WmiDecodeError(operation, std::format("output enumeration failed: {}", hrText(nextHr)));
    }
    if (!rawName) throw windows::wmi::WmiDecodeError(operation, "output enumeration returned a value without a name");

    const std::string key = bstrToUtf8(name.get());
    if (value.get().vt & VT_ARRAY) {
      if (decoded.values.contains(key) || !decoded.arrays.emplace(key, expandArrayVariant(value.get(), operation)).second) {
        throw windows::wmi::WmiDecodeError(operation, "response returned duplicate output '" + key + "'");
      }
    } else {
      if (decoded.arrays.contains(key) || !decoded.values.emplace(key, variantToValue(value.get(), type, operation)).second) {
        throw windows::wmi::WmiDecodeError(operation, "response returned duplicate output '" + key + "'");
      }
    }
  }
  enumeration.finish(operation);

  return decoded;
}

windows::wmi::WmiMethodOutput decodeWriteMethodOutput(IWbemClassObject& output, const std::string& className, const std::string& methodName,
                                                      const std::string& operation) {
  try {
    return decodeMethodOutput(output, className, methodName, windows::wmi::WriteEffect::Uncertain);
  } catch (const windows::wmi::WmiProviderError&) {
    // Provider failure does not roll back earlier system effects.
    throw;
  } catch (const windows::wmi::WmiException& error) {
    if (error.code().category() == windows::wmi::wmiErrorCategory()) {
      std::throw_with_nested(windows::wmi::WmiInvocationUncertain(static_cast<int32_t>(error.code().value()), operation,
                                                                  "provider outcome could not be decoded: " + std::string(error.what())));
    }
    std::throw_with_nested(windows::wmi::WmiInvocationUncertain(operation, "provider outcome could not be decoded: " + std::string(error.what())));
  } catch (const std::exception& error) {
    std::throw_with_nested(windows::wmi::WmiInvocationUncertain(operation, "provider outcome could not be decoded: " + std::string(error.what())));
  } catch (...) {
    throw windows::wmi::WmiInvocationUncertain(operation, "provider outcome could not be decoded because of a non-standard exception");
  }
}

class AsyncMethodSink final : public IWbemObjectSink {
 public:
  explicit AsyncMethodSink(std::shared_ptr<WmiMethodCompletion> state) : m_state(std::move(state)) {}

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
    // The final COM reference owns sink destruction.
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
    m_state->complete(result);
    return WBEM_S_NO_ERROR;
  }

 private:
  std::atomic<ULONG> m_refs{1};
  std::shared_ptr<WmiMethodCompletion> m_state;
};

}  // namespace

struct WmiSession::Impl {
  explicit Impl(std::string path) : namespacePath(std::move(path)), ownerThread(std::this_thread::get_id()) {}

  class OperationGuard {
   public:
    explicit OperationGuard(Impl& owner) : m_owner(owner) {
      if (owner.ownerThread != std::this_thread::get_id()) {
        throw windows::wmi::WmiProtocolError("access WMI session", "thread-local session was used from a different thread");
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
std::function<windows::wmi::WmiOperations&()> WmiSession::threadAccessor() {
  const bool embedded = m_impl->namespacePath == windows::kWmiNamespaceEmbedded;
  return [embedded]() -> windows::wmi::WmiOperations& { return embedded ? embeddedWmiSession() : cimv2WmiSession(); };
}
WmiSession::~WmiSession() = default;

void WmiSession::invalidate() const { m_impl->reset(); }

void WmiSession::ensureConnected() const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw windows::wmi::WmiProtocolError("connect WMI session", "reentrant request on the same thread");
  for (int attempt = 0; attempt < 2; ++attempt) {
    try {
      ensureConnectedWithinOperation();
      return;
    } catch (const windows::wmi::WmiTransportError&) {
      invalidate();
      if (attempt == 0) continue;
      throw;
    }
  }
  throw windows::wmi::WmiProtocolError("connect WMI session", "retry loop exhausted without a connection");
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
    throw windows::wmi::WmiInfrastructureError(static_cast<int32_t>(hr), "obtain COM apartment", hrText(hr));
  }

  hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, m_impl->locator.putVoid());
  if (FAILED(hr) || !m_impl->locator) {
    const HRESULT failure = FAILED(hr) ? hr : E_UNEXPECTED;
    m_impl->reset();
    throw windows::wmi::WmiInfrastructureError(static_cast<int32_t>(failure), "create WMI locator", hrText(failure));
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

std::vector<windows::wmi::WmiRow> WmiSession::query(const std::string& wql) const { return executeQuery(wql, QueryMode::Projection); }

std::vector<windows::wmi::WmiRow> WmiSession::queryInstances(const std::string& wql) const { return executeQuery(wql, QueryMode::LocatableInstances); }

std::vector<windows::wmi::WmiRow> WmiSession::executeQuery(const std::string& wql, const QueryMode mode) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw windows::wmi::WmiProtocolError("execute WMI query", "reentrant request on the same thread");
  const auto deadline = std::chrono::steady_clock::now() + kQueryTimeout;
  for (int attempt = 0; attempt < 2; ++attempt) {
    try {
      ensureConnectedWithinOperation();
      std::vector<windows::wmi::WmiRow> rows;
      const UniqueBstr language(L"WQL");
      const UniqueBstr queryText(utf8ToWide(wql));
      windows::ComPtr<IEnumWbemClassObject> enumerator;
      const long flags = WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY | (mode == QueryMode::LocatableInstances ? WBEM_FLAG_ENSURE_LOCATABLE : 0);
      const HRESULT queryHr = m_impl->services->ExecQuery(language, queryText, flags, nullptr, enumerator.put());
      if (FAILED(queryHr)) throwComFailure(queryHr, "execute WMI query", std::format("{}; WQL: {}", hrText(queryHr), wql));
      if (!enumerator) throw windows::wmi::WmiProtocolError("execute WMI query", std::format("provider returned no enumerator; WQL: {}", wql));

      while (true) {
        if (std::chrono::steady_clock::now() >= deadline)
          throw windows::wmi::WmiInfrastructureError(static_cast<int32_t>(WBEM_E_TIMED_OUT), "enumerate WMI query",
                                                     std::format("query deadline exceeded; WQL: {}", wql));
        windows::ComPtr<IWbemClassObject> obj;
        ULONG got = 0;
        const HRESULT nextHr = enumerator->Next(kEnumerationPollMilliseconds, 1, obj.put(), &got);
        if (FAILED(nextHr)) throwComFailure(nextHr, "enumerate WMI query", std::format("{}; WQL: {}", hrText(nextHr), wql));
        if (got > 1 || (got == 1) != static_cast<bool>(obj)) {
          throw windows::wmi::WmiProtocolError("enumerate WMI query", std::format("inconsistent row count/object pair; WQL: {}", wql));
        }
        if (got == 0) {
          if (nextHr == WBEM_S_FALSE) break;
          if (nextHr == WBEM_S_TIMEDOUT) continue;
          throw windows::wmi::WmiProtocolError("enumerate WMI query", std::format("no object without completion status {}; WQL: {}", hrText(nextHr), wql));
        }
        const PathRequirement pathRequirement = mode == QueryMode::LocatableInstances ? PathRequirement::Required : PathRequirement::Optional;
        rows.push_back(readObjectProps(*obj, pathRequirement, std::format("decode WMI query row {}; WQL: {}", rows.size(), wql)));
        if (nextHr == WBEM_S_FALSE) break;
      }
      UWF_LOG_D("wmi") << "query completed: rows=" << rows.size() << " wql=" << wql;
      return rows;
    } catch (const windows::wmi::WmiTransportError&) {
      invalidate();
      if (attempt == 0) continue;
      throw;
    }
  }
  throw windows::wmi::WmiProtocolError("execute WMI query", "retry loop exhausted without a result");
}

windows::wmi::WmiClassStatus WmiSession::classStatus(const std::string& className) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw windows::wmi::WmiProtocolError("probe WMI class", "reentrant request on the same thread");
  for (int attempt = 0; attempt < 2; ++attempt) {
    try {
      ensureConnectedWithinOperation();
      const UniqueBstr classPath(utf8ToWide(className));
      windows::ComPtr<IWbemClassObject> classObject;
      const HRESULT hr = m_impl->services->GetObject(classPath, 0, nullptr, classObject.put(), nullptr);
      if (hr == static_cast<HRESULT>(WBEM_E_INVALID_CLASS) || hr == static_cast<HRESULT>(WBEM_E_NOT_FOUND)) return windows::wmi::WmiClassStatus::Missing;
      if (FAILED(hr)) throwComFailure(hr, "probe WMI class " + className, hrText(hr));
      if (!classObject) throw windows::wmi::WmiProtocolError("probe WMI class " + className, "provider returned no class object");
      return windows::wmi::WmiClassStatus::Present;
    } catch (const windows::wmi::WmiTransportError&) {
      invalidate();
      if (attempt == 0) continue;
      throw;
    }
  }
  throw windows::wmi::WmiProtocolError("probe WMI class " + className, "retry loop exhausted without a result");
}

windows::wmi::WmiRow WmiSession::getObject(const std::string& objectPath) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw windows::wmi::WmiProtocolError("read WMI object", "reentrant request on the same thread");
  for (int attempt = 0; attempt < 2; ++attempt) {
    try {
      ensureConnectedWithinOperation();
      const UniqueBstr path(utf8ToWide(objectPath));
      windows::ComPtr<IWbemClassObject> object;
      const HRESULT hr = m_impl->services->GetObject(path, 0, nullptr, object.put(), nullptr);
      if (FAILED(hr)) throwComFailure(hr, "read WMI object " + objectPath, hrText(hr));
      if (!object) throw windows::wmi::WmiProtocolError("read WMI object " + objectPath, "provider returned no object");
      return readObjectProps(*object, PathRequirement::Required, "decode WMI object " + objectPath);
    } catch (const windows::wmi::WmiTransportError&) {
      invalidate();
      if (attempt == 0) continue;
      throw;
    }
  }
  throw windows::wmi::WmiProtocolError("read WMI object " + objectPath, "retry loop exhausted without a result");
}

windows::wmi::WmiMethodOutput WmiSession::executeReadMethodOnce(const std::string& objectPath, const std::string& methodName,
                                                                const windows::wmi::WmiRow& inputs) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw windows::wmi::WmiProtocolError("invoke WMI method " + methodName, "reentrant request on the same thread");
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
    if (!output) throw windows::wmi::WmiProtocolError(operationName, "provider returned no output parameters");
    return decodeMethodOutput(*output, prepared.className, methodName);
  } catch (const windows::wmi::WmiTransportError&) {
    invalidate();
    throw;
  }
}

windows::wmi::WmiMethodOutput WmiSession::invokeMethod(const std::string& objectPath, const std::string& methodName, const windows::wmi::WmiRow& inputs) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw windows::wmi::WmiProtocolError("invoke WMI method " + methodName, "reentrant request on the same thread");
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
    if (!output) throw windows::wmi::WmiInvocationUncertain(operationName, "provider returned no output parameters");
    return decodeWriteMethodOutput(*output, prepared.className, methodName, operationName);
  } catch (const windows::wmi::WmiTransportError&) {
    // Preparation failed before the write was dispatched.
    invalidate();
    throw;
  } catch (const windows::wmi::WmiInvocationUncertain& error) {
    if (error.code().category() == windows::wmi::wmiErrorCategory() && isWmiConnectionFailure(static_cast<int32_t>(error.code().value()))) invalidate();
    throw;
  }
}

windows::wmi::WmiMethodOutput WmiSession::callMethodRead(const std::string& objectPath, const std::string& methodName,
                                                         const windows::wmi::WmiRow& inputs) const {
  try {
    return executeReadMethodOnce(objectPath, methodName, inputs);
  } catch (const windows::wmi::WmiTransportError&) {
    return executeReadMethodOnce(objectPath, methodName, inputs);
  }
}

windows::wmi::WmiMethodOutput WmiSession::callMethodReadOnce(const std::string& objectPath, const std::string& methodName, const windows::wmi::WmiRow& inputs,
                                                             const std::stop_token stopToken) const {
  using windows::wmi::ReadCompletion;
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw windows::wmi::WmiProtocolError("invoke single-dispatch WMI method " + methodName, "reentrant request on the same thread");
  const std::string operationName = std::format("invoke single-dispatch WMI method {} on {}", methodName, objectPath);
  auto completion = ReadCompletion::NotDispatched;
  try {
    if (stopToken.stop_requested()) throw windows::wmi::WmiCancelled(operationName);
    ensureConnectedWithinOperation();
    PreparedMethodCall prepared = prepareMethodCall(*m_impl->services, objectPath, methodName, inputs);
    auto state = std::make_shared<WmiMethodCompletion>();
    auto sink = windows::ComPtr<AsyncMethodSink>::adopt(new AsyncMethodSink(state));
    if (stopToken.stop_requested()) throw windows::wmi::WmiCancelled(operationName);
    // A failed RPC return can still follow a dispatch. Retain the slot unless
    // natural method completion is observed; never replay this expensive read.
    completion = ReadCompletion::Unknown;
    const HRESULT startHr = m_impl->services->ExecMethodAsync(prepared.objectPath, prepared.methodName, 0, nullptr, prepared.inputs.get(), sink.get());
    if (FAILED(startHr)) throwComFailure(startHr, operationName, std::format("ExecMethodAsync failed: {}", hrText(startHr)));
    const DWORD waitError = state->wait(stopToken);
    if (waitError != ERROR_SUCCESS) {
      const HRESULT waitHr = HRESULT_FROM_WIN32(waitError);
      const HRESULT cancelHr = m_impl->services->CancelAsyncCall(sink.get());
      std::string detail = std::format("waiting for asynchronous completion failed: {}", hrText(waitHr));
      if (FAILED(cancelHr)) detail += std::format("; cancellation also failed: {}", hrText(cancelHr));
      throw windows::wmi::WmiInfrastructureError(static_cast<int32_t>(waitHr), operationName, std::move(detail));
    }
    SrwExclusiveLock lock(state->lock);
    if (stopToken.stop_requested() && !state->completed) {
      lock.unlock();
      const HRESULT cancelHr = m_impl->services->CancelAsyncCall(sink.get());
      if (SUCCEEDED(cancelHr)) throw windows::wmi::WmiCancelled(operationName);
      lock.lock(state->lock);
      if (!state->completed) {
        lock.unlock();
        throwComFailure(cancelHr, operationName, std::format("CancelAsyncCall failed: {}", hrText(cancelHr)));
      }
    }
    state->checkReadStatus(completion, operationName);
    auto output = windows::ComPtr<IWbemClassObject>::retain(state->output.get());
    lock.unlock();
    if (!output) throw windows::wmi::WmiProtocolError(operationName, "provider completed without output parameters");
    return decodeMethodOutput(*output, prepared.className, methodName);
  } catch (windows::wmi::WmiException& error) {
    error.setReadCompletion(completion);
    if (dynamic_cast<const windows::wmi::WmiTransportError*>(&error)) invalidate();
    throw;
  } catch (const std::exception& error) {
    windows::wmi::WmiProtocolError failure(operationName, error.what());
    failure.setReadCompletion(completion);
    throw failure;
  } catch (...) {
    windows::wmi::WmiProtocolError failure(operationName, "single-dispatch method raised an unknown exception");
    failure.setReadCompletion(completion);
    throw failure;
  }
}

void WmiSession::putInstance(const std::string& className, const windows::wmi::WmiRow& props, const windows::wmi::WmiPutMode mode) const {
  Impl::OperationGuard operation(*m_impl);
  if (!operation) throw windows::wmi::WmiProtocolError("write WMI instance " + className, "reentrant request on the same thread");
  const std::string operationName = "write WMI instance " + className;
  try {
    ensureConnectedWithinOperation();
    const UniqueBstr classPath(utf8ToWide(className));
    windows::ComPtr<IWbemClassObject> classObject;
    HRESULT hr = m_impl->services->GetObject(classPath, 0, nullptr, classObject.put(), nullptr);
    if (FAILED(hr)) throwComFailure(hr, operationName, std::format("GetObject failed: {}", hrText(hr)));
    if (!classObject) throw windows::wmi::WmiProtocolError(operationName, "provider returned no class object");

    windows::ComPtr<IWbemClassObject> instance;
    hr = classObject->SpawnInstance(0, instance.put());
    if (FAILED(hr)) throwComFailure(hr, operationName, std::format("SpawnInstance failed: {}", hrText(hr)));
    if (!instance) throw windows::wmi::WmiProtocolError(operationName, "provider returned no writable instance");

    for (const auto& [name, value] : props) {
      UniqueVariant variant;
      valueToVariant(value, variant.get(), operationName, "property " + name);
      const UniqueBstr propertyName(utf8ToWide(name));
      const HRESULT putHr = instance->Put(propertyName, 0, &variant.get(), 0);
      if (FAILED(putHr)) throwComFailure(putHr, operationName, std::format("setting property {} failed: {}", name, hrText(putHr)));
    }

    const long flags = mode == windows::wmi::WmiPutMode::CreateOnly ? WBEM_FLAG_CREATE_ONLY : WBEM_FLAG_UPDATE_ONLY;
    hr = m_impl->services->PutInstance(instance.get(), flags, nullptr, nullptr);
    if (FAILED(hr)) throwWriteInvocationFailure(hr, operationName, hrText(hr));
  } catch (const windows::wmi::WmiTransportError&) {
    invalidate();
    throw;
  } catch (const windows::wmi::WmiInvocationUncertain& error) {
    if (error.code().category() == windows::wmi::wmiErrorCategory() && isWmiConnectionFailure(static_cast<int32_t>(error.code().value()))) invalidate();
    throw;
  }
}

namespace {

class ComApartmentLease final {
 public:
  ComApartmentLease() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    // An STA caller needs a matching lease so its apartment outlives thread-local WMI proxies.
    if (hr == RPC_E_CHANGED_MODE) hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) throw windows::wmi::WmiInfrastructureError(static_cast<int32_t>(hr), "initialize COM apartment", hrText(hr));
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
      throw windows::wmi::WmiInfrastructureError(static_cast<int32_t>(securityResult), "initialize COM security", hrText(securityResult));
    }
  }

  ThreadComApartment(const ThreadComApartment&) = delete;
  ThreadComApartment& operator=(const ThreadComApartment&) = delete;

 private:
  // Acquire COM before security initialization; constructor failure releases it.
  ComApartmentLease m_lease;
};

}  // namespace

struct WmiThreadContext {
  // Reverse destruction releases both sessions before their apartment lease.
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

windows::wmi::WmiOperations& embeddedWmiSession() { return threadContext().embedded; }
windows::wmi::WmiOperations& cimv2WmiSession() { return threadContext().cimv2; }
void initializeWmiRuntime() { static_cast<void>(threadContext()); }
void shutdownWmiRuntime() noexcept {
  threadContextStorage.reset();
  threadContextRetired = true;
}

}  // namespace uwf::windows
