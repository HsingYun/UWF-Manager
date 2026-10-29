// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitDispatcher.h"

#include <QPointer>
#include <QTimer>
#include <stdexcept>
#include <utility>
namespace uwf::ui {
namespace {
class ScopedTimerPause {
 public:
  explicit ScopedTimerPause(QTimer* timer) : m_timer(timer), m_wasActive(timer && timer->isActive()) {
    if (m_wasActive) m_timer->stop();
  }
  ~ScopedTimerPause() {
    if (m_wasActive && m_timer) m_timer->start();
  }
  ScopedTimerPause(const ScopedTimerPause&) = delete;
  ScopedTimerPause& operator=(const ScopedTimerPause&) = delete;

 private:
  QPointer<QTimer> m_timer;
  bool m_wasActive;
};

}  // namespace
CommitDispatcher::CommitDispatcher(std::unique_ptr<app::CommitWorkflow> workflow, QTimer* usageTimer)
    : m_usageTimer(usageTimer), m_workflow(std::move(workflow)) {
  if (!m_workflow) throw std::invalid_argument("commit dispatcher requires a workflow");
}

void CommitDispatcher::commitFilePath(const QString& path) {
  if (path.isEmpty()) return;
  const ScopedTimerPause usagePause(m_usageTimer);
  m_workflow->commitFilePath(path);
}

void CommitDispatcher::commitFileDeletionPath(const QString& path) {
  if (path.isEmpty()) return;
  const ScopedTimerPause usagePause(m_usageTimer);
  m_workflow->commitFileDeletionPath(path);
}

void CommitDispatcher::commitRegistryKey(const QString& key, const QString& valueName) {
  if (key.isEmpty()) return;
  const ScopedTimerPause usagePause(m_usageTimer);
  m_workflow->commitRegistryKey(key, valueName);
}

void CommitDispatcher::commitRegistryDeletionKey(const QString& key, const QString& valueName) {
  if (key.isEmpty()) return;
  const ScopedTimerPause usagePause(m_usageTimer);
  m_workflow->commitRegistryDeletionKey(key, valueName);
}

}  // namespace uwf::ui
