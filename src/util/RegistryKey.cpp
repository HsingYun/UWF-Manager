#include "RegistryKey.h"

#include <cctype>

namespace uwf::regkey {

namespace {

std::string trimmed(const std::string& s) {
  size_t b = 0;
  size_t e = s.size();
  while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
  while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
  return s.substr(b, e - b);
}

std::string toUpper(const std::string& s) {
  std::string out = s;
  for (char& c : out) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  return out;
}

}  // namespace

std::string normalize(const std::string& key) {
  std::string s = trimmed(key);
  while (!s.empty() && s.back() == '\\') s.pop_back();
  if (s.empty()) return {};

  // hive = 第一个反斜杠之前的部分；rest = 其后（含起始反斜杠），原样保留。
  const size_t bs = s.find('\\');
  const std::string hive = (bs == std::string::npos) ? s : s.substr(0, bs);
  const std::string rest = (bs == std::string::npos) ? std::string{} : s.substr(bs);

  // 5 个标准 hive：长写形式 + 它的简写。大小写不敏感匹配 → 统一输出长写。
  struct Hive {
    const char* form;       // 可能的写法（简写或长写）
    const char* canonical;  // 归一到的长写
  };
  static constexpr Hive kHives[] = {
      {"HKLM", "HKEY_LOCAL_MACHINE"}, {"HKEY_LOCAL_MACHINE", "HKEY_LOCAL_MACHINE"},
      {"HKCU", "HKEY_CURRENT_USER"},  {"HKEY_CURRENT_USER", "HKEY_CURRENT_USER"},
      {"HKCR", "HKEY_CLASSES_ROOT"},  {"HKEY_CLASSES_ROOT", "HKEY_CLASSES_ROOT"},
      {"HKU", "HKEY_USERS"},          {"HKEY_USERS", "HKEY_USERS"},
      {"HKCC", "HKEY_CURRENT_CONFIG"}, {"HKEY_CURRENT_CONFIG", "HKEY_CURRENT_CONFIG"},
  };
  const std::string hiveUpper = toUpper(hive);
  for (const auto& h : kHives) {
    if (hiveUpper == h.form) return std::string(h.canonical) + rest;
  }
  return s;  // hive 不认识——原样返回（已 trim、去尾反斜杠）
}

}  // namespace uwf::regkey
