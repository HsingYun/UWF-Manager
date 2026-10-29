// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "app/api/EnhancedModeActions.h"

#include <QFutureWatcher>
#include <QPointer>

#include "app/common/BackgroundWork.h"

namespace uwf::app {
void EnhancedModeActions::requestChange(const EnhancedModeAction action, const QString& description) {
  if (m_operationInProgress) return;
  ChangeWork work;
  try {
    work = prepareChange(action, description);
  } catch (...) {
    emit operationFinished({}, std::current_exception());
    return;
  }
  using Result = BackgroundResult<EnhancedModeChangeResult>;
  auto* watcher = new QFutureWatcher<Result>(this);
  connect(watcher, &QFutureWatcher<Result>::finished, this, [this, watcher] {
    watcher->deleteLater();
    auto outcome = watcher->future().takeResult();
    m_operationInProgress = false;
    if (auto* result = std::get_if<EnhancedModeChangeResult>(&outcome)) {
      const QPointer guard(this);
      try {
        acceptChangeResult(*result);
      } catch (...) {
        if (guard) emit operationFinished({}, std::current_exception());
        return;
      }
      if (!guard) return;
      emit operationFinished(*result, {});
    } else {
      emit operationFinished({}, std::get<std::exception_ptr>(outcome));
    }
  });
  m_operationInProgress = true;
  const QPointer guard(this);
  emit operationStarted();
  if (!guard) return;
  watcher->setFuture(launchBackground([work = std::move(work)](std::stop_token) { return work(); }));
}
}  // namespace uwf::app
