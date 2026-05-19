#include "WmiRowUtil.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <regex>

#include "../../util/Log.h"

namespace uwf::rowutil {

namespace {

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
  std::string direct = r.value(propName).toString();
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
