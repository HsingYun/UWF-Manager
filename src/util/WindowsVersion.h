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

#include <cstdint>
#include <string>

namespace uwf {

enum class WindowsFamily {
  Unknown,
  Windows10,
  Windows11,
  WindowsServer,
};

struct WindowsVersionInfo {
  WindowsFamily family = WindowsFamily::Unknown;
  std::uint32_t major = 0;
  std::uint32_t minor = 0;
  std::uint32_t build = 0;
  std::uint32_t revision = 0;
  std::string productName;     // 已按真实家族修正的展示名
  std::string editionId;       // 注册表 EditionID
  std::string displayVersion;  // 例如 22H2 / 24H2；旧系统回退到 ReleaseId
  bool longTermServicing = false;
};

// 系统版本在一次开机期间不会变化，首次调用后缓存。系统家族和 Build 来自
// RtlGetVersion(OSVERSIONINFOEXW)；注册表只补充展示字段，不能改变家族判断。
[[nodiscard]] const WindowsVersionInfo& windowsVersionInfo();

}  // namespace uwf
