#include "RegistryKey.h"

#include <windows.h>

#include <format>
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

// 把 key 归一、解析 hive、以只读方式打开。成功时 outKey 为已打开句柄（调用方
// 负责 RegCloseKey）；hive 无法识别或键不存在返回 false。
// 单独 hive（如 "HKEY_LOCAL_MACHINE"，无子键）合法——RegOpenKeyExW 收空 lpSubKey
// 会返回一份指向该 hive 自身的新句柄，可被 RegEnumKeyEx / RegCloseKey 正常使用。
// picker 树根节点展开就走这条路径。
bool openForRead(std::string_view key, HKEY& outKey) {
  const std::string normalized = normalize(std::string(key));
  if (normalized.empty()) return false;
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
  if (!root) return false;  // hive 无法识别

  const std::wstring subkey = firstSlash == std::string::npos ? std::wstring{} : utf8ToWide(normalized.substr(firstSlash + 1));
  return RegOpenKeyExW(root, subkey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &outKey) == ERROR_SUCCESS;
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
  HKEY opened = nullptr;
  if (!openForRead(key, opened)) return false;
  RegCloseKey(opened);
  return true;
}

bool valueExists(std::string_view key, std::string_view valueName) {
  HKEY opened = nullptr;
  if (!openForRead(key, opened)) return false;
  // valueName 为空 → utf8ToWide 得空宽串 → RegQueryValueExW 查的就是键的默认值
  // (Default)；非空则查该命名值。无条件查值，不为空 valueName 开特例。
  const std::wstring wideValue = utf8ToWide(valueName);
  const bool exists = RegQueryValueExW(opened, wideValue.c_str(), nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS;
  RegCloseKey(opened);
  return exists;
}

std::string readString(std::string_view key, std::string_view valueName) {
  HKEY opened = nullptr;
  if (!openForRead(key, opened)) return {};

  std::string result;
  const std::wstring wideValue = utf8ToWide(valueName);
  DWORD type = 0;
  DWORD bytes = 0;
  // 先量长度，并确认是字符串型。
  if (RegQueryValueExW(opened, wideValue.c_str(), nullptr, &type, nullptr, &bytes) == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ) && bytes > 0) {
    // 多留 1 个 wchar_t：即使注册表数据未以 NUL 结尾，缓冲区也必有 NUL 收尾。
    std::wstring buf(bytes / sizeof(wchar_t) + 1, L'\0');
    DWORD got = static_cast<DWORD>(buf.size() * sizeof(wchar_t));
    if (RegQueryValueExW(opened, wideValue.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(buf.data()), &got) == ERROR_SUCCESS) {
      result = wideToUtf8(buf.c_str());  // c_str() 在首个 NUL 处截断尾随终止符
    }
  }
  RegCloseKey(opened);
  return result;
}

std::uint32_t readDword(std::string_view key, std::string_view valueName) {
  HKEY opened = nullptr;
  if (!openForRead(key, opened)) return 0;

  const std::wstring wideValue = utf8ToWide(valueName);
  DWORD value = 0;
  DWORD type = 0;
  DWORD bytes = sizeof(value);
  RegQueryValueExW(opened, wideValue.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(&value), &bytes);
  RegCloseKey(opened);
  return value;
}

std::vector<std::string> subkeyNames(std::string_view key) {
  std::vector<std::string> out;
  HKEY opened = nullptr;
  if (!openForRead(key, opened)) return out;
  wchar_t name[config::kRegistryKeyNameBufChars];
  for (DWORD i = 0;; ++i) {
    DWORD len = config::kRegistryKeyNameBufChars;
    if (RegEnumKeyExW(opened, i, name, &len, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
    out.push_back(wideToUtf8(std::wstring(name, len)));
  }
  RegCloseKey(opened);
  return out;
}

bool hasSubkeys(std::string_view key) {
  HKEY opened = nullptr;
  if (!openForRead(key, opened)) return false;
  DWORD subkeyCount = 0;
  const LONG rc = RegQueryInfoKeyW(opened, nullptr, nullptr, nullptr, &subkeyCount, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  RegCloseKey(opened);
  return rc == ERROR_SUCCESS && subkeyCount > 0;
}

std::vector<std::string> valueNames(std::string_view key) {
  std::vector<std::string> out;
  HKEY opened = nullptr;
  if (!openForRead(key, opened)) return out;
  std::wstring name(config::kRegistryValueNameBufChars, L'\0');
  for (DWORD i = 0;; ++i) {
    DWORD len = static_cast<DWORD>(name.size());
    if (RegEnumValueW(opened, i, name.data(), &len, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
    out.push_back(wideToUtf8(std::wstring(name.data(), len)));
  }
  RegCloseKey(opened);
  return out;
}

std::vector<RegValueInfo> values(std::string_view key) {
  std::vector<RegValueInfo> out;
  HKEY opened = nullptr;
  if (!openForRead(key, opened)) return out;
  std::wstring name(config::kRegistryValueNameBufChars, L'\0');
  for (DWORD i = 0;; ++i) {
    DWORD len = static_cast<DWORD>(name.size());
    DWORD type = 0;
    if (RegEnumValueW(opened, i, name.data(), &len, nullptr, &type, nullptr, nullptr) != ERROR_SUCCESS) break;
    out.push_back({wideToUtf8(std::wstring(name.data(), len)), type});
  }
  RegCloseKey(opened);
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
  // 后序 DFS：先递归收子键、再追加自身——子键天然排在父键之前。
  for (const auto& child : subkeyNames(norm)) {
    for (auto& descendant : collectKeyTree(norm + '\\' + child)) out.push_back(std::move(descendant));
  }
  out.push_back(norm);
  return out;
}

}  // namespace uwf::regkey
