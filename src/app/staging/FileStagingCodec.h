// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QString>
#include <string>
#include <vector>

#include "FileStagingPath.h"
#include "app/api/FileStagingTypes.h"

namespace uwf::app {

// 注册表持久化格式是一个 REG_MULTI_SZ：每条记录以 F| 或 D| 标记文件/目录，
// 后面保留完整路径。编码与存储传输解耦，纯协议测试不必链接注册表和 WMI。
[[nodiscard]] std::vector<std::string> encodeFileStagingEntries(const QList<FileStagingEntry>& entries);
[[nodiscard]] QList<FileStagingEntry> decodeFileStagingEntries(const std::vector<std::string>& records);

}  // namespace uwf::app
