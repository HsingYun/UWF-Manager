// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/enhanced/contracts/EnhancedModeActions.h"

#include <QFutureWatcher>
#include <QPointer>

#include "application/execution/BackgroundWork.h"

namespace uwf::app {
void EnhancedModeActions::requestChange(const EnhancedModeAction action, const QString& description) {
  if (m_operationInProgress) return;
  // Reserve admission before calling the host's preparation boundary. Nested
  // requests must not start a second service mutation while this one prepares.
  m_operationInProgress = true;
  const QPointer guard(this);
  using Result = BackgroundResult<EnhancedModeChangeResult>;
  QPointer<QFutureWatcher<Result>> watcher;
  try {
    auto work = prepareChange(action, description);
    if (!guard) return;
    watcher = new QFutureWatcher<Result>(this);
    connect(watcher.data(), &QFutureWatcher<Result>::finished, this, [this, watcher = watcher.data()] {
      watcher->deleteLater();
      auto outcome = watcher->future().takeResult();
      if (auto* result = std::get_if<EnhancedModeChangeResult>(&outcome)) {
        const QPointer guard(this);
        try {
          acceptChangeResult(*result);
        } catch (...) {
          if (guard) {
            m_operationInProgress = false;
            emit operationFinished({}, std::current_exception());
          }
          return;
        }
        if (!guard) return;
        m_operationInProgress = false;
        emit operationFinished(*result, {});
      } else {
        m_operationInProgress = false;
        emit operationFinished({}, std::get<std::exception_ptr>(outcome));
      }
    });
    emit operationStarted();
    if (!guard) return;
    watcher->setFuture(launchBackground([work = std::move(work)](std::stop_token) { return work(); }));
  } catch (...) {
    if (guard) {
      if (watcher) watcher->deleteLater();
      m_operationInProgress = false;
      emit operationFinished({}, std::current_exception());
    }
    return;
  }
}
}  // namespace uwf::app
