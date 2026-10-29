// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <filesystem>

namespace uwf::winfs {
[[nodiscard]] std::filesystem::path extendedPath(const std::filesystem::path& path);

// Windows 文件属性的一次权威观察。ReparsePoint 独立于文件/目录分类，调用方
// 可以先建立递归边界，再决定是否处理普通文件或目录。
enum class EntryType {
  Missing,
  File,
  Directory,
  ReparsePoint,
};

// 路径明确不存在返回 Missing；权限、I/O 或无效路径等无法确认事实的情况抛
// std::system_error，不得伪装成“不存在”。
[[nodiscard]] EntryType inspect(const std::filesystem::path& path);

[[nodiscard]] bool existsForVerification(const std::filesystem::path& path);

}  // namespace uwf::winfs
