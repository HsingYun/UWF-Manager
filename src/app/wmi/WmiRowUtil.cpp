// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WmiRowUtil.h"

#include <charconv>
#include <cctype>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <utility>

#include "shared/Log.h"
#include "shared/StringUtil.h"

namespace uwf::app::wmi::row {

namespace {

[[noreturn]] void throwFieldError(const std::string& key, const char* reason) {
  throw WmiDecodeError("decode WMI row", std::string("field '") + key + "' " + reason);
}

bool isIntegerLike(const WmiValue::Kind kind) { return kind == WmiValue::Kind::Int || kind == WmiValue::Kind::UInt || kind == WmiValue::Kind::String; }

struct MofLiteral {
  std::string value;
  bool quoted = false;
};

// Embedded UWF instances contain a flat property list, either by itself or
// inside an instance wrapper. Parse that boundary once for every value kind;
// never search through quoted strings or comments for a property assignment.
class MofPropertyReader final {
 public:
  explicit MofPropertyReader(const std::string_view text) : m_text(text) {}

  std::optional<MofLiteral> find(const std::string& property) {
    skipTrivia();
    if (m_position == m_text.size()) return std::nullopt;
    bool wrapped = take('{');
    std::optional<std::string_view> first;
    if (!wrapped) {
      first = name();
      if (toLowerAscii(std::string(*first)) == "instance") {
        if (toLowerAscii(std::string(name())) != "of") malformed("expected 'of' after 'instance'");
        static_cast<void>(name());
        expect('{');
        wrapped = true;
        first.reset();
      } else if (take('{')) {
        wrapped = true;
        first.reset();
      }
    }

    std::optional<MofLiteral> found;
    std::unordered_set<std::string> properties;
    const auto wanted = toLowerAscii(property);
    while (true) {
      skipTrivia();
      if (wrapped && take('}')) {
        expect(';');
        skipTrivia();
        if (m_position != m_text.size()) malformed("unexpected text after the instance");
        return found;
      }
      if (m_position == m_text.size()) {
        if (wrapped) malformed("unterminated instance");
        return found;
      }
      std::string_view key;
      if (first) {
        key = *first;
        first.reset();
      } else {
        key = name();
      }
      expect('=');
      auto value = literal();
      expect(';');
      const auto folded = toLowerAscii(std::string(key));
      if (!properties.insert(folded).second) malformed("duplicate property");
      if (folded == wanted) found = std::move(value);
    }
  }

 private:
  [[noreturn]] static void malformed(const char* reason) { throw WmiDecodeError("decode embedded MOF", reason); }

  void skipTrivia() {
    while (m_position < m_text.size()) {
      if (std::isspace(static_cast<unsigned char>(m_text[m_position]))) {
        ++m_position;
      } else if (m_text.substr(m_position, 2) == "//") {
        const auto end = m_text.find('\n', m_position + 2);
        m_position = end == std::string_view::npos ? m_text.size() : end + 1;
      } else if (m_text.substr(m_position, 2) == "/*") {
        const auto end = m_text.find("*/", m_position + 2);
        if (end == std::string_view::npos) malformed("unterminated comment");
        m_position = end + 2;
      } else {
        break;
      }
    }
  }

  bool take(const char character) {
    skipTrivia();
    if (m_position == m_text.size() || m_text[m_position] != character) return false;
    ++m_position;
    return true;
  }

  void expect(const char character) {
    if (!take(character)) malformed("missing property or instance delimiter");
  }

  std::string_view name() {
    skipTrivia();
    const auto begin = m_position;
    if (begin == m_text.size() || (!std::isalpha(static_cast<unsigned char>(m_text[begin])) && m_text[begin] != '_'))
      malformed("invalid property or class name");
    while (m_position < m_text.size()) {
      const char character = m_text[m_position];
      if (!std::isalnum(static_cast<unsigned char>(character)) && character != '_' && character != '.') break;
      ++m_position;
    }
    return m_text.substr(begin, m_position - begin);
  }

  MofLiteral literal() {
    skipTrivia();
    MofLiteral result;
    result.quoted = take('"');
    if (result.quoted) {
      while (m_position < m_text.size()) {
        const char character = m_text[m_position++];
        if (character == '"') return result;
        if (character == '\n' || character == '\r') malformed("newline inside a quoted value");
        if (character == '\\') {
          if (m_position == m_text.size()) malformed("unterminated quoted escape");
          const char escaped = m_text[m_position++];
          if (escaped == '\n' || escaped == '\r') malformed("newline inside a quoted escape");
          // Preserve the existing provider string convention: only escaped
          // quotes and backslashes are unescaped; other path escapes survive.
          if (escaped != '\\' && escaped != '"') result.value += '\\';
          result.value += escaped;
        } else {
          result.value += character;
        }
      }
      malformed("unterminated quoted value");
    }
    const auto begin = m_position;
    while (m_position < m_text.size()) {
      const char character = m_text[m_position];
      if (std::isspace(static_cast<unsigned char>(character)) || character == ';' || character == '}' ||
          m_text.substr(m_position, 2) == "//" || m_text.substr(m_position, 2) == "/*")
        break;
      if (!std::isalnum(static_cast<unsigned char>(character)) && character != '_' && character != '.' && character != '+' && character != '-')
        malformed("unsupported or malformed property value");
      ++m_position;
    }
    if (m_position == begin) malformed("missing property value");
    result.value = m_text.substr(begin, m_position - begin);
    return result;
  }

  std::string_view m_text;
  std::size_t m_position = 0;
};

std::optional<MofLiteral> mofProperty(const std::string& mof, const std::string& property) { return MofPropertyReader(mof).find(property); }

}  // namespace

bool requireBool(const WmiRow& r, const std::string& key) {
  const auto it = r.find(key);
  if (it == r.end() || it->second.kind() != WmiValue::Kind::Bool) throwFieldError(key, it == r.end() ? "is missing" : "has the wrong type");
  return it->second.toBool();
}

bool requireBoolOrFalseIfNull(const WmiRow& r, const std::string& key) {
  const auto it = r.find(key);
  if (it == r.end()) throwFieldError(key, "is missing");
  if (!it->second.isValid()) return false;
  if (it->second.kind() != WmiValue::Kind::Bool) throwFieldError(key, "has the wrong type");
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
  const auto value = mofProperty(mof, propName);
  if (!value) return {};
  if (!value->quoted) throwFieldError(propName, "is not a quoted string in embedded MOF");
  return value->value;
}

std::string requireEmbeddedString(const WmiRow& r, const std::string& propName) {
  if (r.contains(propName)) {
    return requireString(r, propName, EmptyString::Reject);
  }

  const auto mof = requireString(r, "__MOF", EmptyString::Reject);
  auto value = extractFromMof(mof, propName);
  if (value.empty()) throwFieldError(propName, "is missing from embedded MOF");
  return value;
}

uint64_t requireEmbeddedUInt64(const WmiRow& r, const std::string& propName) {
  if (r.contains(propName)) return requireUInt64(r, propName);
  const auto value = mofProperty(requireString(r, "__MOF", EmptyString::Reject), propName);
  if (!value) throwFieldError(propName, "is missing from embedded MOF");
  const auto& text = value->value;
  uint64_t decoded = 0;
  const auto result = std::from_chars(text.data(), text.data() + text.size(), decoded);
  if (text.empty() || text.front() < '0' || text.front() > '9' || result.ec != std::errc{} || result.ptr != text.data() + text.size())
    throwFieldError(propName, "is not a valid UInt64 in embedded MOF");
  return decoded;
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

}  // namespace uwf::app::wmi::row
