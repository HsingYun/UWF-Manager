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
std::deque<std::string> g_buffer;
size_t g_bufferBytes = 0;
std::mutex g_mutex;

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

void logLine(char level, const std::string& category, const std::string& message) {
  // 锁纪律：g_mutex 仅在 deque 操作期间持有，且持锁期间不调用任何用户提供
  // 的回调或可能再次取 g_mutex 的代码（包括 UWF_LOG_*）。这条规则保证
  // logLine 之间不会自死锁，也不会与外部锁形成 g_mutex ↔ X 的循环。
  // 维护时如果将来要在持锁期间调任何东西，必须先确认它不会回到这里。
  std::string line = std::format("[{} {} {}] {}", timestamp(), level, category, message);

  std::lock_guard<std::mutex> lk(g_mutex);
  g_bufferBytes += line.size();
  g_buffer.push_back(std::move(line));
  // 单行长度可能超过整个上限（极端 case），所以保留至少一行避免 deque
  // 被清空又立刻 push 时反复抖动。
  while (g_bufferBytes > kCapBytes && g_buffer.size() > 1) {
    g_bufferBytes -= g_buffer.front().size();
    g_buffer.pop_front();
  }
}

std::vector<std::string> recentLogLines() {
  std::lock_guard<std::mutex> lk(g_mutex);
  return {g_buffer.begin(), g_buffer.end()};
}

void clearLogLines() {
  std::lock_guard<std::mutex> lk(g_mutex);
  g_buffer.clear();
  g_bufferBytes = 0;
}

}  // namespace uwf
