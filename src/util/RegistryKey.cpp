#include "RegistryKey.h"

#include <windows.h>

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
    {"HKLM", "HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE},
    {"HKCU", "HKEY_CURRENT_USER", HKEY_CURRENT_USER},
    {"HKCR", "HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT},
    {"HKU", "HKEY_USERS", HKEY_USERS},
    {"HKCC", "HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG},
};

// 把 key 归一、解析 hive、以只读方式打开。成功时 outKey 为已打开句柄（调用方
// 负责 RegCloseKey）；hive 无法识别、缺子键或键不存在均返回 false。
bool openForRead(std::string_view key, HKEY& outKey) {
  const std::string normalized = normalize(std::string(key));
  const size_t firstSlash = normalized.find('\\');
  if (firstSlash == std::string::npos) return false;  // 只有 hive、没有子键

  const std::string hive = toUpperAscii(normalized.substr(0, firstSlash));
  HKEY root = nullptr;
  for (const auto& h : kHives) {
    if (hive == h.longForm) {
      root = h.handle;
      break;
    }
  }
  if (!root) return false;  // hive 无法识别

  const std::wstring subkey = utf8ToWide(normalized.substr(firstSlash + 1));
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
  if (RegQueryValueExW(opened, wideValue.c_str(), nullptr, &type, nullptr, &bytes) == ERROR_SUCCESS &&
      (type == REG_SZ || type == REG_EXPAND_SZ) && bytes > 0) {
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

}  // namespace uwf::regkey
