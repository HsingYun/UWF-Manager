// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "app/ports/WmiOperations.h"

namespace uwf::windows {

// 每个线程各自拥有两个固定 namespace 的长生命周期 session。首次访问会初始化
// 当前线程的 COM apartment。专用线程在业务对象销毁后调用
// shutdownWmiRuntime()；复用的线程池线程把上下文保留到线程退出，由 TLS 析构释放。
// 一旦显式销毁，本线程上下文永久退休，后续访问会抛出生命周期错误而不会重建。
app::wmi::WmiOperations& embeddedWmiSession();
app::wmi::WmiOperations& cimv2WmiSession();

// 建立/销毁当前线程唯一的 COM/WMI 上下文。调用方在线程入口初始化，并用已有
// 的作用域守卫保证在线程函数返回前销毁。
void initializeWmiRuntime();
void shutdownWmiRuntime() noexcept;

}  // namespace uwf::windows
