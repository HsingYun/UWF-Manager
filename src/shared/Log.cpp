// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "Log.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <deque>
#include <format>
#include <iterator>
#include <mutex>

namespace uwf {

namespace {

// 缓冲按字节限额；超限时优先移除最旧日志。
constexpr size_t kCapBytes = 10 * 1024 * 1024;

// 进程级缓冲采用 RAII；进程退出前由运行时收束后台任务。
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
  std::tm tm{};
  localtime_s(&tm, &t);
  return std::format("{:02}:{:02}:{:02}.{:03}", tm.tm_hour, tm.tm_min, tm.tm_sec, static_cast<int>(ms.count()));
}

}  // namespace

void logLine(const char level, const std::string_view category, const std::string_view message) {
#if !defined(UWF_DEBUG_LOGGING)
  // 直接调用 logLine 也须遵循 Debug 级别过滤。
  if (level == 'D') return;
#endif
  // 缓冲锁仅保护队列操作；持锁时禁止回调或日志调用，避免重入及锁序循环。
  std::string line = std::format("[{} {} {}] {}", timestamp(), level, category, message);

  LogState& s = state();
  std::lock_guard<std::mutex> lk(s.mutex);
  s.bufferBytes += line.size();
  s.buffer.push_back(std::move(line));
  // 至少保留一行，因此单条超长日志可临时超过字节限额。
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
  for (auto current = first; current != s.buffer.cend(); ++current) {
    if (current->size() <= maxBytesPerLine) {
      result.push_back(*current);
    } else if (maxBytesPerLine > kTruncatedSuffix.size()) {
      std::string line = current->substr(0, maxBytesPerLine - kTruncatedSuffix.size());
      line.append(kTruncatedSuffix);
      result.push_back(std::move(line));
    } else {
      result.push_back(current->substr(0, maxBytesPerLine));
    }
  }
  return result;
}

void clearLogLines() {
  LogState& s = state();
  std::lock_guard<std::mutex> lk(s.mutex);
  s.buffer.clear();
  s.bufferBytes = 0;
}

}  // namespace uwf
