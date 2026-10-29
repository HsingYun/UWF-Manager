// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <cstdint>
#include <string>
namespace uwf::app {
// 返回 Win32 类型常量名；未知类型使用 UNKNOWN(<数值>)。协议标识保持原文。
[[nodiscard]] std::string valueTypeName(uint32_t type);
}  // namespace uwf::app
