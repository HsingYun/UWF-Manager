// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/api/CommitPreparation.h"
#include "app/ports/WmiOperations.h"
#include "core/UwfModel.h"
namespace uwf::app {
class CommitWorkflow final {
 public:
  CommitWorkflow(wmi::WmiOperations& session, const core::UwfSnapshot& snapshot, CommitInteraction interaction,
                 std::shared_ptr<const CommitTraversalSource> source);
  void commitFilePath(const QString& path);
  void commitFileDeletionPath(const QString& path);
  void commitRegistryKey(const QString& key, const QString& valueName);
  void commitRegistryDeletionKey(const QString& key, const QString& valueName);

 private:
  void run(CommitOperation operation, CommitTargetKind kind, const QString& path, const QString& valueName = {});
  std::function<wmi::WmiOperations&()> m_session;
  const core::UwfSnapshot& m_snapshot;
  CommitInteraction m_interaction;
  std::shared_ptr<const CommitTraversalSource> m_source;
};
}  // namespace uwf::app
