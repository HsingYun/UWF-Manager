// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Dispatches commit intents and scopes activity and observation pauses to the workflow.
// Each asynchronous operation owns its request and prepared backend.

#include <QString>
#include <functional>
#include <memory>

#include "application/commit/CommitWorkflow.h"
#include "presentation/viewmodels/common/ObservationPause.h"

namespace uwf::ui {

class CommitDispatcher {
 public:
  using PauseObservation = std::function<presentation::ObservationPause()>;
  using ActivityChanged = std::function<void(bool)>;
  CommitDispatcher(std::unique_ptr<app::CommitWorkflow> workflow, PauseObservation pauseObservation = {}, ActivityChanged activityChanged = {});

  void commitFilePath(const QString& path);
  void commitFileDeletionPath(const QString& path);
  // nullopt selects a key; a present empty name selects its default value.
  void commitRegistryKey(const QString& key, const std::optional<QString>& valueName);
  void commitRegistryDeletionKey(const QString& key, const std::optional<QString>& valueName);

 private:
  void dispatch(const QString& target, const std::function<void()>& action);
  PauseObservation m_pauseObservation;
  ActivityChanged m_activityChanged;
  bool m_active = false;
  std::unique_ptr<app::CommitWorkflow> m_workflow;
};

}  // namespace uwf::ui
