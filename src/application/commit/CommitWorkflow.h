// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "application/commit/contracts/CommitInteraction.h"
#include "application/commit/contracts/CommitPlan.h"
#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"
namespace uwf::app {
class CommitWorkflow final {
 public:
  CommitWorkflow(std::shared_ptr<UwfMaintenanceRepository> repository, CommitInteraction interaction, std::shared_ptr<const CommitTraversalSource> source);
  void commitFilePath(const QString& path);
  void commitFileDeletionPath(const QString& path);
  void commitRegistryKey(const QString& key, const std::optional<QString>& valueName);
  void commitRegistryDeletionKey(const QString& key, const std::optional<QString>& valueName);

 private:
  void run(CommitOperation operation, CommitTargetKind kind, const QString& path, const QString& valueName = {});
  std::shared_ptr<UwfMaintenanceRepository> m_repository;
  CommitInteraction m_interaction;
  std::shared_ptr<const CommitTraversalSource> m_source;
};
}  // namespace uwf::app
