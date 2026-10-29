// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <optional>

#include "app/api/FileStagingTypes.h"

namespace uwf::app {

// 文件排除和文件暂存对同一棵卷目录树声明了相反的持久化语义。该模型保存
// 两侧已经提交/待应用的快照，并以路径段为边界判断相交关系；UI 的两个入口
// 共用它，避免只在某一侧拦截而留下顺序相关行为。
class FileStagingConflictPolicy final {
 public:
  void setFileExclusions(QStringList exclusions);
  void setStagedEntries(const QList<FileStagingEntry>& entries);

  [[nodiscard]] std::optional<QString> conflictingExclusion(const QString& stagedPath) const;
  [[nodiscard]] std::optional<QString> conflictingStagedPath(const QString& exclusionPath) const;

 private:
  QStringList m_fileExclusions;
  QStringList m_stagedPaths;
};

}  // namespace uwf::app
