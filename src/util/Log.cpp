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
#include "Log.h"

#include <chrono>
#include <deque>
#include <format>
#include <mutex>

namespace uwf {

namespace {

// 环形缓冲容量上限（字节）。10 MB 够覆盖一次长会话的诊断量，超过时
// 从最旧的行开始丢弃。100B/行估算 → 大约 10 万行。
constexpr size_t kCapBytes = 10 * 1024 * 1024;

// 环形缓冲、已用字节数与其锁集中在一个进程级状态对象中。后台任务都由
// 所属 UI 对象以 jthread 管理并在析构前收敛，因此这里可以遵守正常 RAII，
// 不需要用故意泄漏掩盖线程所有权问题。
struct LogState {
  std::deque<std::string> buffer;
  size_t bufferBytes = 0;
  std::mutex mutex;
};
LogState& state() {
  static LogState s;
  return s;
}

// 格式化为 "HH:MM:SS.mmm" 的时间戳。
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
  // 防住不经 UWF_LOG_D 宏的调试日志入口。
  if (level == 'D') return;
#endif
  // 锁纪律：缓冲锁仅在 deque 操作期间持有，且持锁期间不调用任何用户提供
  // 的回调或可能再次取该锁的代码（包括 UWF_LOG_*）。这条规则保证 logLine
  // 之间不会自死锁，也不会与外部锁形成循环。维护时如果将来要在持锁期间调
  // 任何东西，必须先确认它不会回到这里。
  std::string line = std::format("[{} {} {}] {}", timestamp(), level, category, message);

  LogState& s = state();
  std::lock_guard<std::mutex> lk(s.mutex);
  s.bufferBytes += line.size();
  s.buffer.push_back(std::move(line));
  // 单行长度可能超过整个上限（极端 case），所以保留至少一行避免 deque
  // 被清空又立刻 push 时反复抖动。
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

void clearLogLines() {
  LogState& s = state();
  std::lock_guard<std::mutex> lk(s.mutex);
  s.buffer.clear();
  s.bufferBytes = 0;
}

}  // namespace uwf
