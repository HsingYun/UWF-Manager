// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 线程安全的进程内日志缓冲，不写文件或标准输出；超限时丢弃最旧行。
// Debug 日志仅在 Debug 构建启用，其他配置不求值日志参数。

#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace uwf {

// 直接写一行（不经过 LogStream），供需要自行 format 的场景使用。
void logLine(char level, std::string_view category, std::string_view message);

// 拷贝一份当前所有日志行（从最旧到最新）。线程安全。
std::vector<std::string> recentLogLines();
// 只拷贝最新的 maxLines 行（仍按从旧到新排列），且每行最多复制
// maxBytesPerLine 字节。该预算独立于日志环形缓冲容量。
std::vector<std::string> recentLogLines(std::size_t maxLines, std::size_t maxBytesPerLine);

// 清空缓冲区。线程安全。由 UI "Clear" 按钮触发。
void clearLogLines();

// streaming helper：析构时把 oss 的内容 flush 成一行日志。
struct LogStream {
  char level;
  std::optional<std::string> category;
  std::optional<std::ostringstream> oss;

  LogStream(char lv, std::string_view cat) noexcept : level(lv) {
    try {
      category.emplace(cat);
      oss.emplace();
    } catch (...) {
      // 无法创建日志缓冲不能影响业务。
      category.reset();
      oss.reset();
    }
  }
  ~LogStream() noexcept {
    try {
      if (category && oss) logLine(level, *category, oss->str());
    } catch (...) {
      // 日志不能因为内存不足或格式化失败而终止业务流程，尤其不能在栈展开
      // 期间从析构函数再次抛异常。
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
      // 丢弃整条未完成日志；不输出可能误导诊断的截断文本。
      oss.reset();
    }
    return *this;
  }
};

}  // namespace uwf

#define UWF_LOG_I(cat) ::uwf::LogStream('I', (cat))
#if defined(UWF_DEBUG_LOGGING)
#define UWF_LOG_D(cat) ::uwf::LogStream('D', (cat))
#else
// Release 中整个表达式均不执行，日志参数的查询、转换和字符串拼接也不会产生开销。
#define UWF_LOG_D(cat) \
  while (false) ::uwf::LogStream('D', (cat))
#endif
#define UWF_LOG_W(cat) ::uwf::LogStream('W', (cat))
#define UWF_LOG_E(cat) ::uwf::LogStream('E', (cat))
