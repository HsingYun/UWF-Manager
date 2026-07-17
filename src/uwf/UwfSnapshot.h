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

// UwfSnapshot —— "快照"入口：把 UwfFilter / UwfVolume / UwfOverlay /
// UwfOverlayConfig / UwfRegistryFilter 五个子模块读到的数据
// 组合成一份 core::UwfSnapshot。调用方不需要知道每个 WMI 类的细节。
//
// 另外附带 enumerateDisks()：它不属于任何 UWF WMI 类（查的是 root\cimv2 的
// Win32_LogicalDisk），但"快照一次系统当前状态"的入口天然需要它。

#include <vector>

#include "../core/UwfModel.h"

namespace uwf {

// UWF 类是否已在当前系统注册。该能力在进程启动时探测一次，此后作为不可变
// 的运行环境事实传给每次动态状态读取；WMI 代理重连不会重新定义它。
enum class UwfCapability {
  Available,
  Unavailable,
};

// 启动期探测 Embedded namespace 与 UWF_Filter 是否注册。namespace 或类被
// provider 明确确认不存在时返回 Unavailable；连接、权限和协议失败抛出异常，
// 不能把暂时的读取故障误判成系统不支持 UWF。
[[nodiscard]] UwfCapability probeUwfCapability();

// 读取完整的配置与运行时摘要（filter / overlay / volumes / exclusions）。
// GetOverlayFiles 是昂贵的按需操作，不属于快照。capability 来自启动期探测，
// 本函数只读取会变化的状态，不在刷新期间重新探测 UWF 能力。连接、协议和
// 解码失败抛出异常。
core::UwfSnapshot readSnapshot(UwfCapability capability);

// 枚举本机所有可见的磁盘卷，并对每个卷计算 DiskSupport 判定。
std::vector<core::DiskInfo> enumerateDisks();

}  // namespace uwf
