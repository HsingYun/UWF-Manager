// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Dispatches commit intents and pauses usage polling while the operation is open.
// Each asynchronous operation owns its request, snapshot and backend.

#include <QPointer>
#include <QString>
#include <memory>

#include "app/commit/CommitWorkflow.h"
#include "core/UwfModel.h"

class QTimer;
class QWidget;

namespace uwf::ui {

class CommitDispatcher {
 public:
  CommitDispatcher(std::unique_ptr<app::CommitWorkflow> workflow, QTimer* usageTimer);

  void commitFilePath(const QString& path);
  void commitFileDeletionPath(const QString& path);
  // valueName 空串 = 递归整棵键子树；非空 = 单个命名值。(Default) 在 picker
  // 层恒禁选，故 valueName 非空必为真实值名，空串无歧义地表示"整键递归"。
  void commitRegistryKey(const QString& key, const QString& valueName);
  void commitRegistryDeletionKey(const QString& key, const QString& valueName);

 private:
  QPointer<QTimer> m_usageTimer;
  std::unique_ptr<app::CommitWorkflow> m_workflow;
};

}  // namespace uwf::ui
