// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/commit/CommitDispatcher.h"

#include <QScopeGuard>
#include <stdexcept>
#include <utility>
namespace uwf::ui {
CommitDispatcher::CommitDispatcher(std::unique_ptr<app::CommitWorkflow> workflow, PauseObservation pauseObservation, ActivityChanged activityChanged)
    : m_pauseObservation(std::move(pauseObservation)), m_activityChanged(std::move(activityChanged)), m_workflow(std::move(workflow)) {
  if (!m_workflow) throw std::invalid_argument("commit dispatcher requires a workflow");
}

void CommitDispatcher::dispatch(const QString& target, const std::function<void()>& action) {
  if (target.isEmpty() || m_active) return;
  m_active = true;
  const auto finish = qScopeGuard([this] {
    m_active = false;
    if (m_activityChanged) m_activityChanged(false);
  });
  if (m_activityChanged) m_activityChanged(true);
  const auto observationPause = m_pauseObservation ? m_pauseObservation() : presentation::ObservationPause{};
  action();
}

void CommitDispatcher::commitFilePath(const QString& path) {
  dispatch(path, [&] { m_workflow->commitFilePath(path); });
}

void CommitDispatcher::commitFileDeletionPath(const QString& path) {
  dispatch(path, [&] { m_workflow->commitFileDeletionPath(path); });
}

void CommitDispatcher::commitRegistryKey(const QString& key, const std::optional<QString>& valueName) {
  dispatch(key, [&] { m_workflow->commitRegistryKey(key, valueName); });
}

void CommitDispatcher::commitRegistryDeletionKey(const QString& key, const std::optional<QString>& valueName) {
  dispatch(key, [&] { m_workflow->commitRegistryDeletionKey(key, valueName); });
}

}  // namespace uwf::ui
