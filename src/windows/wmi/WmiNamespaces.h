// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

namespace uwf::windows {

// UWF 的 UWF_* 类所在命名空间。
inline constexpr const char* kWmiNamespaceEmbedded = "root\\standardcimv2\\embedded";
// 标准 Windows CIM 命名空间，Win32_*（逻辑磁盘 / 卷）所在。
inline constexpr const char* kWmiNamespaceCimv2 = "root\\cimv2";

}  // namespace uwf::windows
