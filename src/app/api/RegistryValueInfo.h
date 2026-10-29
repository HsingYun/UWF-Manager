// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace uwf::app {

// 单个值的元信息：name + Win32 类型码（REG_SZ / REG_DWORD / REG_BINARY 等）+
// 可选的预览数据。nullopt 表示值存在，但原始数据超过选择器的安全预览上限；
// 它不能被编码成空 vector，否则 UI 会把“未读取”错误显示成“空值”。
struct RegistryValueInfo {
  std::string name;                          // 空串 = (Default) 值
  uint32_t type;                             // REG_SZ / REG_DWORD 等 Win32 类型码
  std::optional<std::vector<uint8_t>> data;  // 原始预览字节；REG_SZ 等是 UTF-16 LE（含 null）
};

}  // namespace uwf::app
