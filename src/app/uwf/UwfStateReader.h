// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/ports/WmiOperations.h"

// 聚合 UWF 配置和运行状态，并提供独立的磁盘枚举入口。

#include <vector>

#include "app/api/UwfCapability.h"
#include "app/ports/VolumePaths.h"
#include "core/UwfModel.h"

namespace uwf::app {

// 启动期探测 Embedded namespace 与 UWF_Filter 是否注册。namespace 或类被
// provider 明确确认不存在时返回 Unavailable；连接、权限和协议失败抛出异常，
// 不能把暂时的读取故障误判成系统不支持 UWF。
[[nodiscard]] UwfCapability probeUwfCapability(wmi::WmiOperations& embeddedSession);

// 读取完整的配置与运行时摘要（filter / overlay / volumes / exclusions）。
// GetOverlayFiles 是昂贵的按需操作，不属于快照。capability 来自启动期探测，
// 本函数只读取会变化的状态，不在刷新期间重新探测 UWF 能力。连接、协议和
// 解码失败抛出异常。
core::UwfSnapshot readSnapshot(wmi::WmiOperations& embeddedSession, UwfCapability capability, bool elevated, drive::MountLookup mounts);

// 枚举本机所有可见的磁盘卷，并对每个卷计算 VolumeSupport 判定。
std::vector<core::VolumeInfo> enumerateVolumes(wmi::WmiOperations& cimv2Session);

}  // namespace uwf::app
