#include "WmiClient.h"

#include <comdef.h>
#include <oleauto.h>
#include <wbemidl.h>
#include <windows.h>

#include <format>
#include <mutex>
#include <string>

#include "../../util/Log.h"
#include "../../util/StringUtil.h"

namespace uwf {

namespace {

// 把 HRESULT 渲染成 "描述 (0xXXXXXXXX)" 的形式，方便日志。
std::string hrText(const HRESULT hr) {
  _com_error err(hr);
  return std::format("{} (0x{:08x})", wideToUtf8(err.ErrorMessage()), static_cast<uint32_t>(hr));
}

WmiValue variantToValue(const VARIANT& v) {
  switch (v.vt) {
    case VT_NULL:
    case VT_EMPTY:
      return {};
    case VT_BOOL:
      return WmiValue::fromBool(v.boolVal != VARIANT_FALSE);
    case VT_I2:
      return WmiValue::fromInt(v.iVal);
    case VT_I4:
      return WmiValue::fromInt(v.lVal);
    case VT_UI2:
      return WmiValue::fromUInt(v.uiVal);
    case VT_UI4:
      return WmiValue::fromUInt(v.ulVal);
    case VT_I8:
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
      v.vt = VT_I4;
      v.lVal = src.toInt();
      return true;
    case WmiValue::Kind::UInt:
      // WMI 对 CIM_UINT32 入参的 VARIANT 装箱约定是 VT_I4（参见微软
      // 文档 "Numbers (WMI)"：learn.microsoft.com/windows/win32/wmisdk/numbers），
      // 传 VT_UI4 会让 IWbemClassObject::Put 退回
      // WBEM_E_INVALID_PARAMETER (0x80041008)，进而 ExecMethod 失败。
      // CIM_UINT32 的最大值（4G-1）超过 LONG 可表示的范围，但 UWF 的所有
      // UInt32 入参（type/size/threshold）都远小于 INT_MAX，cast 安全。
      v.vt = VT_I4;
      v.lVal = static_cast<LONG>(src.toUInt());
      return true;
    case WmiValue::Kind::Double:
      v.vt = VT_R8;
      v.dblVal = src.toDouble();
      return true;
    case WmiValue::Kind::String: {
      const auto w = utf8ToWide(src.toString());
      v.vt = VT_BSTR;
      v.bstrVal = SysAllocString(w.c_str());
      return true;
    }
    default:
      return false;
  }
}

WmiRow readObjectProps(IWbemClassObject* obj) {
  WmiRow row;
  if (!obj) return row;

  obj->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
  while (true) {
    BSTR name = nullptr;
    VARIANT val;
    VariantInit(&val);
    const HRESULT hn = obj->Next(0, &name, &val, nullptr, nullptr);
    if (hn != WBEM_S_NO_ERROR) {
      if (name) SysFreeString(name);
      VariantClear(&val);
      break;
    }
    row.emplace(wideToUtf8(name), variantToValue(val));
    SysFreeString(name);
    VariantClear(&val);
  }
  obj->EndEnumeration();

  VARIANT vPath;
  VariantInit(&vPath);
  BSTR pn = SysAllocString(L"__PATH");
  if (SUCCEEDED(obj->Get(pn, 0, &vPath, nullptr, nullptr)) && vPath.vt == VT_BSTR) {
    row.emplace("__PATH", WmiValue::fromString(wideToUtf8(vPath.bstrVal)));
  }
  SysFreeString(pn);
  VariantClear(&vPath);

  return row;
}

// 处理 ExecMethod 的 out 参数里 VT_ARRAY 类型字段：展开为 vector<WmiRow>。
// - 元素是 IUnknown → QI 成 IWbemClassObject，展开为行；
// - 元素是 BSTR（EmbeddedInstance 常见） → 放进行的 __MOF 字段。
std::vector<WmiRow> expandArrayVariant(const VARIANT& v) {
  std::vector<WmiRow> rows;
  if (!(v.vt & VT_ARRAY) || !v.parray) return rows;

  SAFEARRAY* sa = v.parray;
  VARTYPE vt = VT_EMPTY;
  SafeArrayGetVartype(sa, &vt);
  LONG lb = 0, ub = 0;
  SafeArrayGetLBound(sa, 1, &lb);
  SafeArrayGetUBound(sa, 1, &ub);

  for (LONG i = lb; i <= ub; ++i) {
    if (vt == VT_UNKNOWN) {
      IUnknown* unk = nullptr;
      if (FAILED(SafeArrayGetElement(sa, &i, &unk)) || !unk) continue;
      IWbemClassObject* item = nullptr;
      unk->QueryInterface(IID_IWbemClassObject, reinterpret_cast<LPVOID*>(&item));
      unk->Release();
      if (!item) continue;
      rows.push_back(readObjectProps(item));
      item->Release();
    } else if (vt == VT_BSTR) {
      BSTR s = nullptr;
      if (FAILED(SafeArrayGetElement(sa, &i, &s)) || !s) continue;
      WmiRow row;
      row.emplace("__MOF", WmiValue::fromString(wideToUtf8(s)));
      rows.push_back(std::move(row));
      SysFreeString(s);
    }
  }
  return rows;
}

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
      return static_cast<int64_t>(m_uint);
    case Kind::Double:
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
      return m_int < 0 ? def : static_cast<uint64_t>(m_int);
    case Kind::UInt:
      return m_uint;
    case Kind::Double:
      return m_double < 0 ? def : static_cast<uint64_t>(m_double);
    case Kind::String: {
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

struct WmiSession::Impl {
  IWbemLocator* locator = nullptr;
  IWbemServices* services = nullptr;
  bool connected = false;

  ~Impl() {
    if (services) services->Release();
    if (locator) locator->Release();
  }
};

WmiSession::WmiSession() : d(std::make_unique<Impl>()) {}
WmiSession::~WmiSession() = default;

bool WmiSession::isConnected() const { return d->connected; }

bool WmiSession::connect(const std::string& namespacePath, std::string* error) const {
  if (!initComOnce(error)) return false;
  UWF_LOG_D("wmi") << "connect attempt: namespace=" << namespacePath;

  // 重入保护：若上一次 connect 已经拿到 locator/services，先释放再重建，
  // 否则下面的 CoCreateInstance / ConnectServer 会直接覆盖指针让前一份永远
  // 漏掉 Release。
  if (d->services) {
    d->services->Release();
    d->services = nullptr;
  }
  if (d->locator) {
    d->locator->Release();
    d->locator = nullptr;
  }
  d->connected = false;

  HRESULT hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<LPVOID*>(&d->locator));
  if (FAILED(hr)) {
    if (error) *error = std::format("CoCreateInstance(WbemLocator) failed: {}", hrText(hr));
    return false;
  }

  const std::wstring nsW = utf8ToWide(namespacePath);
  BSTR ns = SysAllocString(nsW.c_str());
  hr = d->locator->ConnectServer(ns, nullptr, nullptr, nullptr, WBEM_FLAG_CONNECT_USE_MAX_WAIT, nullptr, nullptr, &d->services);
  SysFreeString(ns);
  if (FAILED(hr)) {
    // 命名空间路径只进日志，不进面向用户的 error——UWF 不可用横幅只展示
    // HRESULT 文本（namespace 对用户无意义，详情查日志即可）。
    UWF_LOG_E("wmi") << "ConnectServer failed: namespace=" << namespacePath << "; " << hrText(hr);
    if (error) *error = hrText(hr);
    return false;
  }

  hr = CoSetProxyBlanket(d->services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
  if (FAILED(hr)) {
    if (error) *error = std::format("CoSetProxyBlanket failed: {}", hrText(hr));
    return false;
  }
  d->connected = true;
  UWF_LOG_I("wmi") << "connect ok: namespace=" << namespacePath;
  return true;
}

std::vector<WmiRow> WmiSession::query(const std::string& wql, std::string* error) const {
  std::vector<WmiRow> rows;
  if (!d->connected) {
    if (error) *error = "WMI session not connected";
    UWF_LOG_E("wmi") << "query rejected: session not connected; wql=" << wql;
    return rows;
  }

  const std::wstring wqlW = utf8ToWide(wql);
  BSTR lang = SysAllocString(L"WQL");
  BSTR text = SysAllocString(wqlW.c_str());
  IEnumWbemClassObject* enumerator = nullptr;
  HRESULT hr = d->services->ExecQuery(lang, text, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &enumerator);
  SysFreeString(lang);
  SysFreeString(text);

  if (FAILED(hr) || !enumerator) {
    if (error) *error = std::format("ExecQuery failed: {}\nWQL: {}", hrText(hr), wql);
    UWF_LOG_E("wmi") << "ExecQuery failed: " << hrText(hr) << "; wql=" << wql;
    return rows;
  }

  while (enumerator) {
    IWbemClassObject* obj = nullptr;
    ULONG got = 0;
    hr = enumerator->Next(static_cast<LONG>(WBEM_INFINITE), 1, &obj, &got);
    if (FAILED(hr)) {
      // 枚举中途失败：rows 里已收集的是不完整结果。记日志并经 error 上报，
      // 避免截断的结果被调用方当成完整结果（少几行而毫无征兆）。
      if (error) *error = std::format("enumeration failed after {} row(s): {}", rows.size(), hrText(hr));
      UWF_LOG_E("wmi") << std::format("query: Next failed after {} row(s); wql={}; {}", rows.size(), wql, hrText(hr));
      break;
    }
    if (got == 0 || !obj) break;  // 正常枚举到底
    rows.push_back(readObjectProps(obj));
    obj->Release();
  }

  enumerator->Release();
  UWF_LOG_D("wmi") << std::format("query ok: {} row(s); wql={}", rows.size(), wql);
  return rows;
}

bool WmiSession::classExists(const std::string& className) const {
  if (!d->connected) {
    UWF_LOG_E("wmi") << "classExists rejected: session not connected; class=" << className;
    return false;
  }
  const auto clsW = utf8ToWide(className);
  BSTR clsBstr = SysAllocString(clsW.c_str());
  IWbemClassObject* classObj = nullptr;
  const HRESULT hr = d->services->GetObject(clsBstr, 0, nullptr, &classObj, nullptr);
  SysFreeString(clsBstr);
  if (classObj) classObj->Release();
  if (hr == static_cast<HRESULT>(WBEM_E_INVALID_CLASS)) {
    UWF_LOG_W("wmi") << "classExists: " << className << " is not registered";
    return false;
  }
  if (FAILED(hr))
    UWF_LOG_W("wmi") << std::format("classExists({}): GetObject failed ({}); assuming present", className, hrText(hr));
  else
    UWF_LOG_D("wmi") << "classExists: " << className << " present";
  return true;
}

WmiMethodResult WmiSession::callMethod(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs) const {
  WmiMethodResult result;
  UWF_LOG_D("wmi") << "callMethod start: " << methodName << " @ " << objectPath;

  if (!d->connected) {
    result.error = "WMI session not connected";
    UWF_LOG_E("wmi") << "callMethod rejected: session not connected; method=" << methodName;
    return result;
  }

  // 从 objectPath 推导类名。实际到达 callMethod 的 objectPath 只有两种（见各 api 调用方）：
  //   完整 __PATH  "\\\\host\\root\\...:UWF_Xxx.Key=Val,..." —— read()/readAll() 存的 __PATH
  //   相对路径     "UWF_Xxx.Key=Val,DriveLetter=\"C:\",..."   —— ensureNextSessionEntry 构造
  // （header 契约还允许裸类名 "UWF_Xxx" 调 static 方法，当前无调用方走此路；下面逻辑也兼容。）
  // 只有完整 __PATH（以 "\\" 开头）带 "\\host\\ns:" 前缀，需按命名空间分隔符 ':' 剥掉；
  // 相对路径不能按 ':' 切——键值里可能含 ':'（如 DriveLetter="C:"），否则类名会被截断成
  // 垃圾导致 GetObject 失败。剥掉前缀后 '.' 永远是类名与键的分隔符。
  std::string className = objectPath;
  if (className.starts_with("\\\\")) {
    if (const auto colon = className.find(':'); colon != std::string::npos) className = className.substr(colon + 1);
  }
  if (const auto dot = className.find('.'); dot != std::string::npos) className.resize(dot);

  // GetObject 拿到类 blueprint，用于 GetMethod + SpawnInstance(in-params)。
  IWbemClassObject* classObj = nullptr;
  {
    const auto clsW = utf8ToWide(className);
    BSTR clsBstr = SysAllocString(clsW.c_str());
    HRESULT hr = d->services->GetObject(clsBstr, 0, nullptr, &classObj, nullptr);
    SysFreeString(clsBstr);
    if (FAILED(hr) || !classObj) {
      result.error = std::format("GetObject({}) failed: {}", className, hrText(hr));
      return result;
    }
  }

  IWbemClassObject* inSig = nullptr;
  const auto methodW = utf8ToWide(methodName);
  BSTR methodBstr = SysAllocString(methodW.c_str());
  HRESULT hr = classObj->GetMethod(methodBstr, 0, &inSig, nullptr);
  classObj->Release();

  IWbemClassObject* inParams = nullptr;
  if (SUCCEEDED(hr) && inSig) {
    inSig->SpawnInstance(0, &inParams);
    inSig->Release();
  }

  if (inParams) {
    for (const auto& [name, val] : inputs) {
      VARIANT v;
      if (!valueToVariant(val, v)) continue;
      const auto nameW = utf8ToWide(name);
      BSTR nameBstr = SysAllocString(nameW.c_str());
      const HRESULT hPut = inParams->Put(nameBstr, 0, &v, 0);
      if (FAILED(hPut)) {
        UWF_LOG_W("wmi") << std::format("inParams->Put({}) on {} failed: {} (field name may not match WMI schema)", name, methodName, hrText(hPut));
      }
      SysFreeString(nameBstr);
      VariantClear(&v);
    }
  }

  // 调试：枚举即将发给 ExecMethod 的 inParams 里的字段名 + 类型 + 当前值，
  // 用于定位 "为什么 WBEM_E_INVALID_PARAMETER" —— 一眼看清 WMI schema 期望
  // 的字段集与我们传的是否一致。
  if (inParams) {
    std::string dump;
    inParams->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
    while (true) {
      BSTR name = nullptr;
      VARIANT v;
      CIMTYPE ctype = 0;
      VariantInit(&v);
      const HRESULT hn = inParams->Next(0, &name, &v, &ctype, nullptr);
      if (hn != WBEM_S_NO_ERROR) {
        if (name) SysFreeString(name);
        VariantClear(&v);
        break;
      }
      if (!dump.empty()) dump += ", ";
      dump += std::format("{}(vt={}, cim={}, val={})", wideToUtf8(name), static_cast<int>(v.vt), static_cast<int>(ctype), variantToValue(v).toString());
      SysFreeString(name);
      VariantClear(&v);
    }
    inParams->EndEnumeration();
    UWF_LOG_D("wmi") << std::format("inParams for {}: [{}]", methodName, dump);
  }

  const auto pathW = utf8ToWide(objectPath);
  BSTR pathBstr = SysAllocString(pathW.c_str());
  IWbemClassObject* outParams = nullptr;
  hr = d->services->ExecMethod(pathBstr, methodBstr, 0, nullptr, inParams, &outParams, nullptr);
  SysFreeString(pathBstr);
  SysFreeString(methodBstr);
  if (inParams) inParams->Release();

  if (FAILED(hr)) {
    result.hresult = static_cast<int32_t>(hr);
    result.error = std::format("ExecMethod {}::{} failed: {}", objectPath, methodName, hrText(hr));
    UWF_LOG_E("wmi") << result.error;
    if (outParams) outParams->Release();
    return result;
  }

  result.invoked = true;

  if (!outParams) return result;

  // 遍历 out 参数：标量落入 outParams，VT_ARRAY 落入 outArrays。
  outParams->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
  while (true) {
    BSTR name = nullptr;
    VARIANT v;
    VariantInit(&v);
    const HRESULT hn = outParams->Next(0, &name, &v, nullptr, nullptr);
    if (hn != WBEM_S_NO_ERROR) {
      if (name) SysFreeString(name);
      VariantClear(&v);
      break;
    }
    const std::string key = wideToUtf8(name);
    if (v.vt & VT_ARRAY) {
      result.outArrays.emplace(key, expandArrayVariant(v));
    } else {
      result.outParams.emplace(key, variantToValue(v));
    }
    SysFreeString(name);
    VariantClear(&v);
  }
  outParams->EndEnumeration();
  outParams->Release();

  if (const auto rv = result.outParams.value("ReturnValue"); rv.isValid()) result.returnValue = rv.toUInt();

  UWF_LOG_D("wmi") << std::format("callMethod ok: {}::{} invoked={} rv={} outArrays={}", objectPath, methodName, result.invoked, result.returnValue,
                                  result.outArrays.size());
  return result;
}

bool WmiSession::putInstance(const std::string& className, const WmiRow& props, std::string* error) const {
  if (!d->connected) {
    if (error) *error = "WMI session not connected";
    UWF_LOG_E("wmi") << "putInstance rejected: session not connected; class=" << className;
    return false;
  }

  // 1) 拿到类的 blueprint —— SpawnInstance 要从 class object 出发。
  IWbemClassObject* classObj = nullptr;
  {
    const auto clsW = utf8ToWide(className);
    BSTR clsBstr = SysAllocString(clsW.c_str());
    HRESULT hr = d->services->GetObject(clsBstr, 0, nullptr, &classObj, nullptr);
    SysFreeString(clsBstr);
    if (FAILED(hr) || !classObj) {
      if (error) *error = std::format("GetObject({}) failed: {}", className, hrText(hr));
      return false;
    }
  }

  // 2) SpawnInstance 创建本地空实例（还没写入 store）。
  IWbemClassObject* inst = nullptr;
  HRESULT hr = classObj->SpawnInstance(0, &inst);
  classObj->Release();
  if (FAILED(hr) || !inst) {
    if (error) *error = std::format("SpawnInstance({}) failed: {}", className, hrText(hr));
    return false;
  }

  // 3) 设置每个属性。WMI 对 BOOL key 期望 VT_BOOL，对 string 期望 VT_BSTR，
  //    valueToVariant 已经按 WmiValue::Kind 选好；对 UInt 用 VT_I4
  //    （兼容 CIM_UINT*，参见 ThemeManager 那条注释）。
  for (const auto& [name, val] : props) {
    VARIANT v;
    if (!valueToVariant(val, v)) continue;
    const auto nameW = utf8ToWide(name);
    BSTR nameBstr = SysAllocString(nameW.c_str());
    const HRESULT hPut = inst->Put(nameBstr, 0, &v, 0);
    if (FAILED(hPut)) {
      UWF_LOG_W("wmi") << std::format("inst->Put({}) on {} failed: {}", name, className, hrText(hPut));
    }
    SysFreeString(nameBstr);
    VariantClear(&v);
  }

  // 4) PutInstance 把 inst 写入 WMI store；CREATE_OR_UPDATE：不存在就创建。
  IWbemCallResult* callResult = nullptr;
  hr = d->services->PutInstance(inst, WBEM_FLAG_CREATE_OR_UPDATE, nullptr, &callResult);
  if (callResult) callResult->Release();
  inst->Release();
  if (FAILED(hr)) {
    if (error) *error = std::format("PutInstance({}) failed: {}", className, hrText(hr));
    UWF_LOG_E("wmi") << std::format("putInstance failed: class={}; {}", className, hrText(hr));
    return false;
  }
  UWF_LOG_I("wmi") << std::format("putInstance ok: class={} props={}", className, props.size());
  return true;
}

bool WmiSession::deleteInstance(const std::string& objectPath, std::string* error) const {
  if (!d->connected) {
    if (error) *error = "WMI session not connected";
    UWF_LOG_E("wmi") << "deleteInstance rejected: session not connected; path=" << objectPath;
    return false;
  }
  const auto pathW = utf8ToWide(objectPath);
  BSTR pathBstr = SysAllocString(pathW.c_str());
  HRESULT hr = d->services->DeleteInstance(pathBstr, 0, nullptr, nullptr);
  SysFreeString(pathBstr);
  if (FAILED(hr)) {
    if (error) *error = std::format("DeleteInstance({}) failed: {}", objectPath, hrText(hr));
    UWF_LOG_E("wmi") << std::format("deleteInstance failed: path={}; {}", objectPath, hrText(hr));
    return false;
  }
  UWF_LOG_I("wmi") << "deleteInstance ok: path=" << objectPath;
  return true;
}

bool initComOnce(std::string* error) {
  static std::once_flag flag;
  static bool ok = false;
  static std::string err;

  std::call_once(flag, [] {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
      err = std::format("CoInitializeEx failed: {}", hrText(hr));
      return;
    }
    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
    if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
      err = std::format("CoInitializeSecurity failed: {}", hrText(hr));
      return;
    }
    ok = true;
  });

  if (!ok && error) *error = err;
  return ok;
}

}  // namespace uwf
