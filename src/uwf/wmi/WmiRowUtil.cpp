#include "WmiRowUtil.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <regex>

#include "../../util/Log.h"

namespace uwf::rowutil {

namespace {

std::string trim(const std::string& s) {
  const auto notSpace = [](const unsigned char c) { return !std::isspace(c); };
  const auto b = std::find_if(s.begin(), s.end(), notSpace);
  const auto e = std::find_if(s.rbegin(), s.rend(), notSpace).base();
  return b < e ? std::string(b, e) : std::string();
}

std::string replaceAll(std::string s, const std::string& from, const std::string& to) {
  if (from.empty()) return s;
  size_t pos = 0;
  while ((pos = s.find(from, pos)) != std::string::npos) {
    s.replace(pos, from.size(), to);
    pos += to.size();
  }
  return s;
}

}  // namespace

bool getBool(const WmiRow& r, const std::string& key, bool def) {
  auto it = r.find(key);
  if (it == r.end() || !it->second.isValid()) return def;
  return it->second.toBool(def);
}

int32_t getInt(const WmiRow& r, const std::string& key, int32_t def) {
  auto it = r.find(key);
  if (it == r.end() || !it->second.isValid()) return def;
  bool ok = false;
  const int32_t v = it->second.toInt(&ok, def);
  return ok ? v : def;
}

uint32_t getUInt(const WmiRow& r, const std::string& key, uint32_t def) {
  auto it = r.find(key);
  if (it == r.end() || !it->second.isValid()) return def;
  bool ok = false;
  const uint32_t v = it->second.toUInt(&ok, def);
  return ok ? v : def;
}

uint64_t getUInt64(const WmiRow& r, const std::string& key, uint64_t def) {
  auto it = r.find(key);
  if (it == r.end() || !it->second.isValid()) return def;
  bool ok = false;
  const uint64_t v = it->second.toULongLong(&ok, def);
  return ok ? v : def;
}

std::string getString(const WmiRow& r, const std::string& key) {
  auto it = r.find(key);
  if (it == r.end()) return {};
  return it->second.toString();
}

std::string normalizeDriveLetter(const std::string& raw) {
  // 该函数是 *校验+规范化*，不是从任意路径里抽盘符——调用方传入的是 WMI
  // DriveLetter 列，规范输入是 `""` / `"C"` / `"C:"` / 偶尔 `"C:\"`。
  // 注意：Windows NT 内核允许多字母盘符（VHD/SUBST/ImDisk 等在 26 个 A-Z
  // 用完后会注册 "CC:"、"XYZ:" 这种），WMI 也会原样回传，所以接受任意长度
  // 的纯字母前缀，不要假设单字符。
  // 任何 alpha 之外的字符（路径分隔符以外）都视为非法 → 返回空串，让 caller
  // 一眼看出这条记录的 DriveLetter 字段没读对，而不是被静默"修正"成假盘符。
  std::string s = trim(raw);
  while (!s.empty() && (s.back() == '\\' || s.back() == '/')) s.pop_back();
  if (s.empty()) return {};

  // 数前导字母数量；后面要么直接到结尾（"C" / "CC"），要么后面只跟一个 ':'。
  size_t letterCount = 0;
  while (letterCount < s.size() && std::isalpha(static_cast<unsigned char>(s[letterCount]))) ++letterCount;
  if (letterCount == 0) return {};
  if (letterCount != s.size() && !(letterCount + 1 == s.size() && s.back() == ':')) return {};

  std::string out;
  out.reserve(letterCount + 1);
  for (size_t i = 0; i < letterCount; ++i) out += static_cast<char>(std::toupper(static_cast<unsigned char>(s[i])));
  out += ':';
  return out;
}

std::string extractFromMof(const std::string& mof, const std::string& propName) {
  // 把 propName 里所有 ECMAScript regex 元字符转义，防止字段名碰巧含
  // `.` `(` `[` 之类导致正则被解释错或抛 std::regex_error。
  std::string escaped;
  escaped.reserve(propName.size() + 8);
  for (const char c : propName) {
    if (std::strchr(R"(.^$|()[]{}*+?\)", c)) escaped += '\\';
    escaped += c;
  }
  const std::regex re(escaped + R"RX(\s*=\s*"((?:\\.|[^"\\])*)")RX");
  std::smatch m;
  if (!std::regex_search(mof, m, re)) return {};
  std::string v = m[1].str();
  v = replaceAll(v, "\\\\", "\\");
  v = replaceAll(v, "\\\"", "\"");
  return v;
}

std::string readExcludedKey(const WmiRow& r, const std::string& propName) {
  const std::string direct = r.value(propName).toString();
  if (!direct.empty()) return direct;
  const std::string mof = r.value("__MOF").toString();
  if (!mof.empty()) return extractFromMof(mof, propName);
  return {};
}

void dumpRow(const char* tag, const WmiRow& r) {
  std::string kv;
  bool first = true;
  for (const auto& [k, v] : r) {
    if (!first) kv += ", ";
    first = false;
    kv += k;
    kv += '=';
    kv += v.toString();
  }
  UWF_LOG_D("wmi") << tag << " { " << kv << " }";
}

}  // namespace uwf::rowutil
