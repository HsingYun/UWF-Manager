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
#include "WmiRowUtil.h"

#include <cstring>
#include <regex>

#include "../../util/Log.h"

namespace uwf::rowutil {

namespace {

void setFieldError(std::string* error, const std::string& key, const char* reason) {
  if (error) *error = std::string("WMI field '") + key + "' " + reason;
}

bool isIntegerLike(const WmiValue::Kind kind) { return kind == WmiValue::Kind::Int || kind == WmiValue::Kind::UInt || kind == WmiValue::Kind::String; }

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

std::optional<bool> requireBool(const WmiRow& r, const std::string& key, std::string* error) {
  const auto it = r.find(key);
  if (it == r.end() || it->second.kind() != WmiValue::Kind::Bool) {
    setFieldError(error, key, it == r.end() ? "is missing" : "has the wrong type");
    return std::nullopt;
  }
  return it->second.toBool();
}

std::optional<int32_t> requireInt(const WmiRow& r, const std::string& key, std::string* error) {
  const auto it = r.find(key);
  if (it == r.end() || !isIntegerLike(it->second.kind())) {
    setFieldError(error, key, it == r.end() ? "is missing" : "has the wrong type");
    return std::nullopt;
  }
  bool ok = false;
  const auto value = it->second.toInt(&ok);
  if (!ok) {
    setFieldError(error, key, "is not a valid Int32");
    return std::nullopt;
  }
  return value;
}

std::optional<uint32_t> requireUInt(const WmiRow& r, const std::string& key, std::string* error) {
  const auto it = r.find(key);
  if (it == r.end() || !isIntegerLike(it->second.kind())) {
    setFieldError(error, key, it == r.end() ? "is missing" : "has the wrong type");
    return std::nullopt;
  }
  bool ok = false;
  const auto value = it->second.toUInt(&ok);
  if (!ok) {
    setFieldError(error, key, "is not a valid UInt32");
    return std::nullopt;
  }
  return value;
}

std::optional<uint64_t> requireUInt64(const WmiRow& r, const std::string& key, std::string* error) {
  const auto it = r.find(key);
  if (it == r.end() || !isIntegerLike(it->second.kind())) {
    setFieldError(error, key, it == r.end() ? "is missing" : "has the wrong type");
    return std::nullopt;
  }
  bool ok = false;
  const auto value = it->second.toULongLong(&ok);
  if (!ok) {
    setFieldError(error, key, "is not a valid UInt64");
    return std::nullopt;
  }
  return value;
}

std::optional<std::string> requireString(const WmiRow& r, const std::string& key, std::string* error, const bool allowEmpty) {
  const auto it = r.find(key);
  if (it == r.end() || it->second.kind() != WmiValue::Kind::String) {
    setFieldError(error, key, it == r.end() ? "is missing" : "has the wrong type");
    return std::nullopt;
  }
  auto value = it->second.toString();
  if (!allowEmpty && value.empty()) {
    setFieldError(error, key, "is empty");
    return std::nullopt;
  }
  return value;
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

std::optional<std::string> requireEmbeddedString(const WmiRow& r, const std::string& propName, std::string* error) {
  if (const auto direct = r.find(propName); direct != r.end() && direct->second.isValid()) {
    return requireString(r, propName, error, false);
  }

  const auto mof = requireString(r, "__MOF", error, false);
  if (!mof) return std::nullopt;
  auto value = extractFromMof(*mof, propName);
  if (value.empty()) {
    setFieldError(error, propName, "is missing from embedded MOF");
    return std::nullopt;
  }
  return value;
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
