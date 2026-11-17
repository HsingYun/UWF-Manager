// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "foundation/logging/Log.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <deque>
#include <format>
#include <iterator>
#include <mutex>

#include "foundation/logging/LocalTime.h"
#include "foundation/text/Utf8.h"

namespace uwf {

namespace {

constexpr size_t kCapBytes = 10 * 1024 * 1024;

struct LogState {
  std::deque<std::string> buffer;
  size_t bufferBytes = 0;
  std::mutex mutex;
};
LogState& state() {
  static LogState s;
  return s;
}

std::string timestamp() {
  using namespace std::chrono;
  const auto now = system_clock::now();
  const auto t = system_clock::to_time_t(now);
  const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
  const auto tm = detail::localTime(t);
  return std::format("{:02}:{:02}:{:02}.{:03}", tm.tm_hour, tm.tm_min, tm.tm_sec, static_cast<int>(ms.count()));
}

}  // namespace

void logLine(const char level, const std::string_view category, const std::string_view message) {
#if !defined(UWF_DEBUG_LOGGING)
  // Direct callers follow the same level filter as the streaming macros.
  if (level == 'D') return;
#endif
  // Format outside the lock; locked operations must not call back into logging.
  std::string line = std::format("[{} {} {}] {}", timestamp(), level, category, message);

  LogState& s = state();
  std::lock_guard<std::mutex> lk(s.mutex);
  s.buffer.push_back(std::move(line));
  s.bufferBytes += s.buffer.back().size();
  // Keep the newest line even when it alone exceeds the budget.
  while (s.bufferBytes > kCapBytes && s.buffer.size() > 1) {
    s.bufferBytes -= s.buffer.front().size();
    s.buffer.pop_front();
  }
}

std::vector<std::string> recentLogLines() {
  LogState& s = state();
  std::lock_guard<std::mutex> lk(s.mutex);
  return {s.buffer.begin(), s.buffer.end()};
}

std::vector<std::string> recentLogLines(const std::size_t maxLines, const std::size_t maxBytesPerLine) {
  LogState& s = state();
  std::lock_guard<std::mutex> lk(s.mutex);
  const std::size_t count = std::min(maxLines, s.buffer.size());
  const auto first = std::next(s.buffer.cend(), -static_cast<std::ptrdiff_t>(count));
  std::vector<std::string> result;
  result.reserve(count);
  constexpr std::string_view kTruncatedSuffix = "... <truncated>";
  for (auto current = first; current != s.buffer.cend(); ++current) result.push_back(truncateUtf8(*current, maxBytesPerLine, kTruncatedSuffix));
  return result;
}

void clearLogLines() {
  LogState& s = state();
  std::lock_guard<std::mutex> lk(s.mutex);
  s.buffer.clear();
  s.bufferBytes = 0;
}

void logQuoted(LogStream& log, const std::string_view value) noexcept {
  log << '"';
  for (const char character : value) {
    switch (character) {
      case '\\':
        log << "\\\\";
        break;
      case '"':
        log << "\\\"";
        break;
      case '\n':
        log << "\\n";
        break;
      case '\r':
        log << "\\r";
        break;
      case '\t':
        log << "\\t";
        break;
      case '\0':
        log << "\\0";
        break;
      default:
        log << character;
        break;
    }
  }
  log << '"';
}

}  // namespace uwf
