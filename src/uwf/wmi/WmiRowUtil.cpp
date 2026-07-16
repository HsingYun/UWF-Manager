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
#include <string_view>

#include "../../util/Log.h"

namespace uwf::rowutil {

namespace {

[[noreturn]] void throwFieldError(const std::string& key, const char* reason) {
  throw WmiDecodeError("decode WMI row", std::string("field '") + key + "' " + reason);
}

bool isIntegerLike(const WmiValue::Kind kind) { return kind == WmiValue::Kind::Int || kind == WmiValue::Kind::UInt || kind == WmiValue::Kind::String; }

std::string decodeMofString(std::string_view encoded) {
  std::string decoded;
  decoded.reserve(encoded.size());
  for (std::size_t i = 0; i < encoded.size(); ++i) {
    const char character = encoded[i];
    if (character == '\\' && i + 1 < encoded.size() && (encoded[i + 1] == '\\' || encoded[i + 1] == '"')) {
      decoded += encoded[++i];
    } else {
      decoded += character;
    }
  }
  return decoded;
}

}  // namespace

bool requireBool(const WmiRow& r, const std::string& key) {
  const auto it = r.find(key);
  if (it == r.end() || it->second.kind() != WmiValue::Kind::Bool) throwFieldError(key, it == r.end() ? "is missing" : "has the wrong type");
  return it->second.toBool();
}

int32_t requireInt(const WmiRow& r, const std::string& key) {
  const auto it = r.find(key);
  if (it == r.end() || !isIntegerLike(it->second.kind())) throwFieldError(key, it == r.end() ? "is missing" : "has the wrong type");
  try {
    return it->second.toInt();
  } catch (const WmiValueConversionError&) {
    throwFieldError(key, "is not a valid Int32");
  }
}

uint32_t requireUInt(const WmiRow& r, const std::string& key) {
  const auto it = r.find(key);
  if (it == r.end() || !isIntegerLike(it->second.kind())) throwFieldError(key, it == r.end() ? "is missing" : "has the wrong type");
  try {
    return it->second.toUInt();
  } catch (const WmiValueConversionError&) {
    throwFieldError(key, "is not a valid UInt32");
  }
}

uint64_t requireUInt64(const WmiRow& r, const std::string& key) {
  const auto it = r.find(key);
  if (it == r.end() || !isIntegerLike(it->second.kind())) throwFieldError(key, it == r.end() ? "is missing" : "has the wrong type");
  try {
    return it->second.toULongLong();
  } catch (const WmiValueConversionError&) {
    throwFieldError(key, "is not a valid UInt64");
  }
}

std::string requireString(const WmiRow& r, const std::string& key, const EmptyString empty) {
  const auto it = r.find(key);
  if (it == r.end() || it->second.kind() != WmiValue::Kind::String) throwFieldError(key, it == r.end() ? "is missing" : "has the wrong type");
  auto value = it->second.toString();
  if (empty == EmptyString::Reject && value.empty()) throwFieldError(key, "is empty");
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
  return decodeMofString(m[1].str());
}

std::string requireEmbeddedString(const WmiRow& r, const std::string& propName) {
  if (const auto direct = r.find(propName); direct != r.end() && direct->second.isValid()) {
    return requireString(r, propName, EmptyString::Reject);
  }

  const auto mof = requireString(r, "__MOF", EmptyString::Reject);
  auto value = extractFromMof(mof, propName);
  if (value.empty()) throwFieldError(propName, "is missing from embedded MOF");
  return value;
}

void dumpRow(const char* tag, const WmiRow& r) noexcept {
#if !defined(UWF_DEBUG_LOGGING)
  (void)tag;
  (void)r;
#else
  try {
    std::string kv;
    bool first = true;
    for (const auto& [k, v] : r) {
      if (!first) kv += ", ";
      first = false;
      kv += k;
      kv += '=';
      // NULL 是 WMI 行中的合法值（例如无盘符卷的 DriveLetter）。诊断渲染不能
      // 复用严格业务转换并因此改变读取结果；只有业务字段解码才决定 NULL 是否可接受。
      kv += v.isValid() ? v.toString() : "<null>";
    }
    logLine('D', "wmi", std::string(tag) + " { " + kv + " }");
  } catch (...) {
    // 诊断日志是旁路；内存或格式化失败不能把成功的 provider 响应变成业务失败。
  }
#endif
}

}  // namespace uwf::rowutil
