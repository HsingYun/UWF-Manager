// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <QVector>
#include <functional>
#include <stop_token>

namespace uwf::app {

// 原始名称保留用于诊断；操作使用移除 NTFS 流后缀的绝对路径。
// 同路径流合并时大小饱和求和，目录和系统元数据标记取并集。
struct OverlayFileEntry {
  QString rawName;
  QString absolutePath;
  bool isDirectory = false;
  bool isSystemMetadata = false;
  qulonglong fileSize = 0;
};

using OverlayFileLoader = std::function<QVector<OverlayFileEntry>(const QString& driveLetter, std::stop_token stopToken)>;

}  // namespace uwf::app
