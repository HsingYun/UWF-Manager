#include "RegistryKey.h"

#include <windows.h>

#include "StringUtil.h"

namespace uwf::regkey {

namespace {

// 5 个标准注册表 hive：简写、长写、对应的预定义 HKEY 句柄。normalize 用前两列
// 做"任意写法 → 长写"归一，keyExists 用长写列定位到 HKEY——同一张表服务两处。
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

bool keyExists(const std::string& key, const std::string& valueName) {
  // 内部先 normalize——调用方传简写（HKLM\…）还是长写都行，函数自洽。
  const std::string normalized = normalize(key);
  const size_t firstSlash = normalized.find('\\');
  if (firstSlash == std::string::npos) return false;  // 只有 hive、没有子键

  // 规范长写 hive → 预定义 HKEY 句柄。
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
  HKEY opened = nullptr;
  if (RegOpenKeyExW(root, subkey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &opened) != ERROR_SUCCESS) {
    return false;  // 键不存在
  }
  bool exists = true;
  if (!valueName.empty()) {
    const std::wstring wideValue = utf8ToWide(valueName);
    exists = RegQueryValueExW(opened, wideValue.c_str(), nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS;
  }
  RegCloseKey(opened);
  return exists;
}

}  // namespace uwf::regkey
