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
#pragma once

// 极简日志：写入进程内一个 ~10MB 的环形缓冲，由 UI 的"日志"对话框读出
// 展示。不写文件、不写 stdout——避免 GUI 启动时弹 cmd 控制台。容量按
// 字节计：超过上限时从最旧的行开始丢弃，保证近期诊断可用又不让长时间
// 运行的实例无限占内存。用户也可在日志窗口里手动 Clear。
//
// 使用方式：
//   UWF_LOG_I("wmi") << "connect namespace " << ns;
//
// 采用 streaming 风格是为了在调用端保留跨类型拼接的便利；
// 如果只是格式化一段字符串，优先用 std::format 再把结果传进来。

#include <sstream>
#include <string>
#include <vector>

namespace uwf {

// 直接写一行（不经过 LogStream），供需要自行 format 的场景使用。
void logLine(char level, const std::string& category, const std::string& message);

// 拷贝一份当前所有日志行（从最旧到最新）。线程安全。
std::vector<std::string> recentLogLines();

// 清空缓冲区。线程安全。由 UI "Clear" 按钮触发。
void clearLogLines();

// streaming helper：析构时把 oss 的内容 flush 成一行日志。
struct LogStream {
  char level;
  std::string category;
  std::ostringstream oss;

  LogStream(char lv, std::string cat) : level(lv), category(std::move(cat)) {}
  ~LogStream() noexcept {
    try {
      logLine(level, category, oss.str());
    } catch (...) {
      // 日志不能因为内存不足或格式化失败而终止业务流程，尤其不能在栈展开
      // 期间从析构函数再次抛异常。
    }
  }

  template <typename T>
  LogStream& operator<<(const T& v) {
    if constexpr (std::is_same_v<T, bool>) {
      oss << (v ? "true" : "false");
    } else {
      oss << v;
    }
    return *this;
  }
};

}  // namespace uwf

#define UWF_LOG_I(cat) ::uwf::LogStream('I', (cat))
#define UWF_LOG_D(cat) ::uwf::LogStream('D', (cat))
#define UWF_LOG_W(cat) ::uwf::LogStream('W', (cat))
#define UWF_LOG_E(cat) ::uwf::LogStream('E', (cat))
