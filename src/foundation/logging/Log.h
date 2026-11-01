// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Thread-safe in-memory logging; no file or standard-stream output.
// Oldest lines are evicted when the byte budget is exceeded.

#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace uwf {

void logLine(char level, std::string_view category, std::string_view message);

// Return a thread-safe snapshot in chronological order.
std::vector<std::string> recentLogLines();
// Limit the snapshot to recent lines and a separate UTF-8 byte budget per line.
std::vector<std::string> recentLogLines(std::size_t maxLines, std::size_t maxBytesPerLine);

void clearLogLines();

// Emit one line on destruction; allocation and formatting failures are contained.
struct LogStream {
  char level;
  std::optional<std::string> category;
  std::optional<std::ostringstream> oss;

  LogStream(char lv, std::string_view cat) noexcept : level(lv) {
    try {
      category.emplace(cat);
      oss.emplace();
    } catch (...) {
      // Logging failure must not interrupt the caller.
      category.reset();
      oss.reset();
    }
  }
  ~LogStream() noexcept {
    try {
      if (category && oss) logLine(level, *category, oss->str());
    } catch (...) {
      // Destruction must remain safe during exception unwinding.
    }
  }

  template <typename T>
  LogStream& operator<<(const T& v) noexcept {
    if (!oss) return *this;
    try {
      if constexpr (std::is_same_v<T, bool>) {
        *oss << (v ? "true" : "false");
      } else {
        *oss << v;
      }
    } catch (...) {
      // Discard the incomplete line rather than publish misleading partial text.
      oss.reset();
    }
    return *this;
  }
};

// Preserve diagnostic text on one physical line with quoted field boundaries.
void logQuoted(LogStream& log, std::string_view value) noexcept;

}  // namespace uwf

#define UWF_LOG_I(cat) ::uwf::LogStream('I', (cat))
#if defined(UWF_DEBUG_LOGGING)
#define UWF_LOG_D(cat) ::uwf::LogStream('D', (cat))
#else
// Disabled debug logging does not evaluate its arguments.
#define UWF_LOG_D(cat) \
  while (false) ::uwf::LogStream('D', (cat))
#endif
#define UWF_LOG_W(cat) ::uwf::LogStream('W', (cat))
#define UWF_LOG_E(cat) ::uwf::LogStream('E', (cat))
