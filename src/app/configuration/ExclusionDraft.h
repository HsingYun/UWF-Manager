// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QSet>
#include <QString>
#include <QStringList>
#include <optional>

namespace uwf::app {

// 保留路径原写法；待增删集合按 Windows 大小写不敏感语义查询和移除。
[[nodiscard]] bool containsExclusion(const QSet<QString>& set, const QString& key);
void removeExclusion(QSet<QString>& set, const QString& key);

enum class ExclusionImportOutcome {
  Applied,              // 状态发生了变化（加入 added 或 removed，或撤销了对应集合里的旧条目）
  NoOp,                 // 已经在目标状态，无须再动
  RejectedNotOnVolume,  // 仅 File：路径不在本卷
  RejectedForbidden,    // 触发 UWF 不允许排除的黑名单
  RejectedConflict,     // 与同一卷的文件暂存路径相交
  RejectedRequired,     // Registry：文件暂存有数据时不得移除其持久化排除项
};

struct PersistenceFlagDraft {
  bool baseCurrent = false;
  bool baseNext = false;
  std::optional<bool> pendingNext;
  [[nodiscard]] bool visible() const { return baseCurrent || baseNext || pendingNext == true; }
  [[nodiscard]] bool effectiveNext() const { return pendingNext.value_or(baseNext); }
};

class ExclusionDraft final {
 public:
  QStringList current;
  QStringList next;
  QSet<QString> added;
  QSet<QString> removed;
  PersistenceFlagDraft domainSecretKey;
  PersistenceFlagDraft tscal;

  [[nodiscard]] bool add(const QString& path);
  [[nodiscard]] bool remove(const QString& path);
  void reset();
  void togglePersistence(PersistenceFlagDraft& flag);
  void enablePersistence(PersistenceFlagDraft& flag);
};

}  // namespace uwf::app
