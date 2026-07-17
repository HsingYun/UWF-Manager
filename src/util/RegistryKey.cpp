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
#include "RegistryKey.h"

#include <windows.h>

#include <atomic>
#include <format>
#include <system_error>
#include <utility>

#include "../core/Config.h"
#include "StringUtil.h"

namespace uwf::regkey {

namespace {

// 5 个标准注册表 hive：简写、长写、对应的预定义 HKEY 句柄。normalize 用前两列
// 做"任意写法 → 长写"归一，openForRead 用长写列定位到 HKEY——同一张表服务两处。
// 不能 constexpr——HKEY 的预定义值（HKEY_LOCAL_MACHINE 等）是 reinterpret_cast
// 宏，不是常量表达式。
struct Hive {
  const char* shortForm;
  const char* longForm;
  HKEY handle;
};
const Hive kHives[] = {
    {"HKLM", "HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE},   {"HKCU", "HKEY_CURRENT_USER", HKEY_CURRENT_USER},
    {"HKCR", "HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT},     {"HKU", "HKEY_USERS", HKEY_USERS},
    {"HKCC", "HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG},
};

class RegistryHandle final {
 public:
  RegistryHandle() = default;
  ~RegistryHandle() {
    if (m_handle) RegCloseKey(m_handle);
  }
  RegistryHandle(const RegistryHandle&) = delete;
  RegistryHandle& operator=(const RegistryHandle&) = delete;

  [[nodiscard]] HKEY get() const noexcept { return m_handle; }
  [[nodiscard]] HKEY* put() noexcept {
    if (m_handle) RegCloseKey(m_handle);
    m_handle = nullptr;
    return &m_handle;
  }

 private:
  HKEY m_handle = nullptr;
};

// 把 key 归一、解析 hive、以只读方式打开。成功时 outKey 拥有已打开句柄；
// hive 无法识别或键不存在返回对应的 Win32 状态码。
// 单独 hive（如 "HKEY_LOCAL_MACHINE"，无子键）合法——RegOpenKeyExW 收空 lpSubKey
// 会返回一份指向该 hive 自身的新句柄，可被 RegEnumKeyEx / RegCloseKey 正常使用。
// picker 树根节点展开就走这条路径。
LONG openForReadStatus(std::string_view key, RegistryHandle& outKey) {
  const std::string normalized = normalize(std::string(key));
  if (normalized.empty()) return ERROR_INVALID_PARAMETER;
  const size_t firstSlash = normalized.find('\\');

  // 没有反斜杠时整串就是 hive；有时反斜杠前是 hive、之后是子键路径。
  const std::string hive = toUpperAscii(firstSlash == std::string::npos ? normalized : normalized.substr(0, firstSlash));
  HKEY root = nullptr;
  for (const auto& h : kHives) {
    if (hive == h.longForm) {
      root = h.handle;
      break;
    }
  }
  if (!root) return ERROR_INVALID_PARAMETER;  // hive 无法识别

  const std::wstring subkey = firstSlash == std::string::npos ? std::wstring{} : utf8ToWide(normalized.substr(firstSlash + 1));
  return RegOpenKeyExW(root, subkey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, outKey.put());
}

bool isNotFound(const LONG status) { return status == ERROR_FILE_NOT_FOUND || status == ERROR_PATH_NOT_FOUND; }

[[noreturn]] void throwRegistryProbeError(const LONG status, const std::string_view operation) {
  throw std::system_error(static_cast<int>(status), std::system_category(), std::string(operation));
}

[[noreturn]] void throwRegistryTypeError(const DWORD actualType, const std::string_view expectedType) {
  throw std::system_error(ERROR_DATATYPE_MISMATCH, std::system_category(),
                          std::format("read registry value: expected {}, got type {}", expectedType, actualType));
}

bool openForRead(std::string_view key, RegistryHandle& outKey) {
  const LONG status = openForReadStatus(key, outKey);
  if (isNotFound(status)) return false;
  if (status != ERROR_SUCCESS) throwRegistryProbeError(status, "open registry key for reading");
  return true;
}

}  // namespace

std::string normalize(const std::string& key) {
  std::string path = trim(key);
  while (!path.empty() && path.back() == '\\') path.pop_back();
  if (path.empty()) return {};

  // hive = 第一个反斜杠之前的部分；rest = 其后（含起始反斜杠），原样保留。
  const size_t firstSlash = path.find('\\');
  const std::string hive = (firstSlash == std::string::npos) ? path : path.substr(0, firstSlash);
  const std::string rest = (firstSlash == std::string::npos) ? std::string{} : path.substr(firstSlash);

  // 简写或长写（大小写不敏感）→ 统一输出长写。
  const std::string hiveUpper = toUpperAscii(hive);
  for (const auto& h : kHives) {
    if (hiveUpper == h.shortForm || hiveUpper == h.longForm) return std::string(h.longForm) + rest;
  }
  return path;  // hive 不认识——原样返回（已 trim、去尾反斜杠）
}

bool keyExists(std::string_view key) {
  RegistryHandle opened;
  const LONG status = openForReadStatus(key, opened);
  if (isNotFound(status)) return false;
  if (status != ERROR_SUCCESS) throwRegistryProbeError(status, "probe registry key existence");
  return true;
}

bool valueExists(std::string_view key, std::string_view valueName) {
  RegistryHandle opened;
  const LONG openStatus = openForReadStatus(key, opened);
  if (isNotFound(openStatus)) return false;
  if (openStatus != ERROR_SUCCESS) throwRegistryProbeError(openStatus, "probe registry value existence");
  // valueName 为空 → utf8ToWide 得空宽串 → RegQueryValueExW 查的就是键的默认值
  // (Default)；非空则查该命名值。无条件查值，不为空 valueName 开特例。
  const std::wstring wideValue = utf8ToWide(valueName);
  const LONG queryStatus = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, nullptr, nullptr, nullptr);
  if (isNotFound(queryStatus)) return false;
  if (queryStatus != ERROR_SUCCESS) throwRegistryProbeError(queryStatus, "probe registry value existence");
  return true;
}

std::optional<std::string> readString(std::string_view key, std::string_view valueName) {
  RegistryHandle opened;
  if (!openForRead(key, opened)) return std::nullopt;

  const std::wstring wideValue = utf8ToWide(valueName);
  DWORD type = 0;
  DWORD bytes = 0;
  // 先量长度并确认类型。缺失是正常的可选状态；存在但类型不符是数据契约错误。
  const LONG sizeStatus = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, &type, nullptr, &bytes);
  if (isNotFound(sizeStatus)) return std::nullopt;
  if (sizeStatus != ERROR_SUCCESS) throwRegistryProbeError(sizeStatus, "read registry string size");
  if (type != REG_SZ && type != REG_EXPAND_SZ) throwRegistryTypeError(type, "REG_SZ or REG_EXPAND_SZ");
  if (bytes == 0) return std::string{};
  if (bytes % sizeof(wchar_t) != 0) throwRegistryProbeError(ERROR_INVALID_DATA, "read registry string with an invalid byte length");

  // 多留 1 个 wchar_t：即使注册表数据未以 NUL 结尾，缓冲区也必有 NUL 收尾。
  std::wstring buf(bytes / sizeof(wchar_t) + 1, L'\0');
  DWORD got = static_cast<DWORD>(buf.size() * sizeof(wchar_t));
  DWORD readType = 0;
  const LONG readStatus = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, &readType, reinterpret_cast<LPBYTE>(buf.data()), &got);
  if (isNotFound(readStatus)) return std::nullopt;
  if (readStatus != ERROR_SUCCESS) throwRegistryProbeError(readStatus, "read registry string");
  if (readType != REG_SZ && readType != REG_EXPAND_SZ) throwRegistryTypeError(readType, "REG_SZ or REG_EXPAND_SZ");
  if (got % sizeof(wchar_t) != 0) throwRegistryProbeError(ERROR_INVALID_DATA, "read registry string with an invalid byte length");

  std::size_t characters = got / sizeof(wchar_t);
  while (characters > 0 && buf[characters - 1] == L'\0') --characters;
  return wideToUtf8(std::wstring_view(buf.data(), characters));
}

std::optional<std::uint32_t> readDword(std::string_view key, std::string_view valueName) {
  RegistryHandle opened;
  if (!openForRead(key, opened)) return std::nullopt;

  const std::wstring wideValue = utf8ToWide(valueName);
  DWORD value = 0;
  DWORD type = 0;
  DWORD bytes = sizeof(value);
  const LONG status = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(&value), &bytes);
  if (isNotFound(status)) return std::nullopt;
  if (status != ERROR_SUCCESS) throwRegistryProbeError(status, "read registry DWORD");
  if (type != REG_DWORD) throwRegistryTypeError(type, "REG_DWORD");
  if (bytes != sizeof(value)) throwRegistryProbeError(ERROR_INVALID_DATA, "read registry DWORD with an invalid byte length");
  return value;
}

std::vector<std::string> subkeyNames(std::string_view key) {
  std::vector<std::string> out;
  RegistryHandle opened;
  if (!openForRead(key, opened)) return out;
  wchar_t name[config::kRegistryKeyNameBufChars];
  for (DWORD i = 0;; ++i) {
    DWORD len = config::kRegistryKeyNameBufChars;
    const LONG status = RegEnumKeyExW(opened.get(), i, name, &len, nullptr, nullptr, nullptr, nullptr);
    if (status == ERROR_NO_MORE_ITEMS) break;
    if (status != ERROR_SUCCESS) throwRegistryProbeError(status, "enumerate registry subkeys");
    out.push_back(wideToUtf8(std::wstring(name, len)));
  }
  return out;
}

bool hasSubkeys(std::string_view key) {
  RegistryHandle opened;
  if (!openForRead(key, opened)) return false;
  DWORD subkeyCount = 0;
  const LONG rc = RegQueryInfoKeyW(opened.get(), nullptr, nullptr, nullptr, &subkeyCount, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  if (rc != ERROR_SUCCESS) throwRegistryProbeError(rc, "read registry subkey count");
  return subkeyCount > 0;
}

std::vector<std::string> valueNames(std::string_view key) {
  std::vector<std::string> out;
  RegistryHandle opened;
  if (!openForRead(key, opened)) return out;
  std::wstring name(config::kRegistryValueNameBufChars, L'\0');
  for (DWORD i = 0;; ++i) {
    DWORD len = static_cast<DWORD>(name.size());
    const LONG status = RegEnumValueW(opened.get(), i, name.data(), &len, nullptr, nullptr, nullptr, nullptr);
    if (status == ERROR_NO_MORE_ITEMS) break;
    if (status != ERROR_SUCCESS) throwRegistryProbeError(status, "enumerate registry value names");
    out.push_back(wideToUtf8(std::wstring(name.data(), len)));
  }
  return out;
}

std::vector<RegValueInfo> values(std::string_view key) {
  constexpr DWORD kMaxPreviewValueBytes = 16 * 1024 * 1024;
  std::vector<RegValueInfo> out;
  RegistryHandle opened;
  if (!openForRead(key, opened)) return out;
  std::wstring name(config::kRegistryValueNameBufChars, L'\0');
  // 4 KiB 起步够覆盖绝大多数注册表值（典型 < 256 字节）；超过时按返回的 dataLen
  // 扩容重试。RegEnumValueW 在 ERROR_MORE_DATA 时对 lpData 的部分填充语义未明确，
  // 不能依赖——必须 retry。
  std::vector<uint8_t> data(4096);
  for (DWORD i = 0;; ++i) {
    DWORD nameLen = static_cast<DWORD>(name.size());
    DWORD dataLen = static_cast<DWORD>(data.size());
    DWORD type = 0;
    LONG rc = RegEnumValueW(opened.get(), i, name.data(), &nameLen, nullptr, &type, data.data(), &dataLen);
    while (rc == ERROR_MORE_DATA) {
      // Picker 只展示最多 200 字符的预览，不应为异常或恶意注册表值分配任意
      // 大小的内存。名称和类型已经由 RegEnumValueW 返回，超限时保留这一行，
      // 只把预览标为不可用；不能因此丢掉该值或让整张表失败。
      if (dataLen > kMaxPreviewValueBytes) {
        out.push_back({wideToUtf8(std::wstring(name.data(), nameLen)), type, std::nullopt});
        break;
      }
      if (dataLen <= data.size()) throwRegistryProbeError(rc, "enumerate registry values");
      data.resize(dataLen);
      nameLen = static_cast<DWORD>(name.size());
      dataLen = static_cast<DWORD>(data.size());
      rc = RegEnumValueW(opened.get(), i, name.data(), &nameLen, nullptr, &type, data.data(), &dataLen);
    }
    if (rc == ERROR_MORE_DATA) continue;  // 超过安全预览上限，元信息已保留
    if (rc == ERROR_NO_MORE_ITEMS) break;
    if (rc != ERROR_SUCCESS) throwRegistryProbeError(rc, "enumerate registry values");
    out.push_back({wideToUtf8(std::wstring(name.data(), nameLen)), type, std::vector<uint8_t>(data.begin(), data.begin() + dataLen)});
  }
  return out;
}

std::string valueTypeName(uint32_t type) {
  switch (type) {
    case REG_NONE:
      return "REG_NONE";
    case REG_SZ:
      return "REG_SZ";
    case REG_EXPAND_SZ:
      return "REG_EXPAND_SZ";
    case REG_BINARY:
      return "REG_BINARY";
    case REG_DWORD:
      return "REG_DWORD";
    case REG_DWORD_BIG_ENDIAN:
      return "REG_DWORD_BIG_ENDIAN";
    case REG_LINK:
      return "REG_LINK";
    case REG_MULTI_SZ:
      return "REG_MULTI_SZ";
    case REG_RESOURCE_LIST:
      return "REG_RESOURCE_LIST";
    case REG_FULL_RESOURCE_DESCRIPTOR:
      return "REG_FULL_RESOURCE_DESCRIPTOR";
    case REG_RESOURCE_REQUIREMENTS_LIST:
      return "REG_RESOURCE_REQUIREMENTS_LIST";
    case REG_QWORD:
      return "REG_QWORD";
    default:
      return std::format("UNKNOWN({})", type);
  }
}

std::vector<std::string> rootHiveLongNames() {
  std::vector<std::string> out;
  out.reserve(std::size(kHives));
  for (const auto& h : kHives) out.emplace_back(h.longForm);
  return out;
}

std::vector<std::string> collectKeyTree(const std::string& key) {
  std::vector<std::string> out;
  const std::string norm = normalize(key);
  if (!keyExists(norm)) return out;
  // 后序 DFS：先递归收子键、再追加自身——子键天然排在父键之前。
  for (const auto& child : subkeyNames(norm)) {
    for (auto& descendant : collectKeyTree(norm + '\\' + child)) out.push_back(std::move(descendant));
  }
  out.push_back(norm);
  return out;
}

bool collectKeyTree(const std::string& key, const std::atomic_bool& canceled, std::atomic<std::uint64_t>& scanned, std::vector<std::string>& out) {
  if (canceled.load()) return false;

  const std::string norm = normalize(key);
  RegistryHandle opened;
  std::vector<std::string> children;
  if (!openForRead(norm, opened)) return true;
  wchar_t name[config::kRegistryKeyNameBufChars];
  for (DWORD i = 0;; ++i) {
    if (canceled.load()) return false;
    DWORD len = config::kRegistryKeyNameBufChars;
    const LONG rc = RegEnumKeyExW(opened.get(), i, name, &len, nullptr, nullptr, nullptr, nullptr);
    if (rc == ERROR_NO_MORE_ITEMS) break;
    if (rc != ERROR_SUCCESS) throwRegistryProbeError(rc, "enumerate registry key tree");
    children.push_back(wideToUtf8(std::wstring(name, len)));
  }

  for (const auto& child : children) {
    if (!collectKeyTree(norm + '\\' + child, canceled, scanned, out)) return false;
  }

  if (canceled.load()) return false;
  out.push_back(norm);
  ++scanned;
  return true;
}

}  // namespace uwf::regkey
