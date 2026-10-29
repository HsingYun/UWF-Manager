// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

namespace uwf::app {

// UWF 类是否已在当前系统注册。该能力在进程启动时探测一次，此后作为不可变
// 的运行环境事实传给每次动态状态读取；WMI 代理重连不会重新定义它。
enum class UwfCapability {
  Available,
  Unavailable,
};

}  // namespace uwf::app
