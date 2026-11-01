// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/clipboard/ClipboardText.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QPointer>
#include <QThread>

#include "foundation/logging/Log.h"

namespace uwf::ui {

ClipboardWriteResult writeClipboardText(const QString& text) {
  // Qt can synchronously notify application code during setText. Keep the
  // submitted value independent of the caller's mutable or retiring object.
  const QString submitted = text;
  auto* application = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
  if (!application || QThread::currentThread() != application->thread()) {
    UWF_LOG_W("clipboard") << "event=write result=failed reason=gui-thread-required characters=" << submitted.size() << " owned=false text_verified=false";
    return ClipboardWriteResult::Failed;
  }
  const QPointer clipboard(QGuiApplication::clipboard());
  if (!clipboard) {
    UWF_LOG_W("clipboard") << "event=write result=failed reason=clipboard-unavailable characters=" << submitted.size() << " owned=false text_verified=false";
    return ClipboardWriteResult::Failed;
  }
  if (clipboard->thread() != QThread::currentThread()) {
    UWF_LOG_W("clipboard") << "event=write result=failed reason=clipboard-thread-mismatch characters=" << submitted.size()
                           << " owned=false text_verified=false";
    return ClipboardWriteResult::Failed;
  }
  clipboard->setText(submitted);
  if (!clipboard) {
    UWF_LOG_W("clipboard") << "event=write result=failed reason=clipboard-retired characters=" << submitted.size() << " owned=false text_verified=false";
    return ClipboardWriteResult::Failed;
  }
  bool owned = clipboard->ownsClipboard();
  // Matching old content alone cannot acknowledge a rejected write.
  const bool matches = owned && clipboard && clipboard->text() == submitted;
  owned = owned && clipboard && clipboard->ownsClipboard();
  const bool verified = owned && matches;
  if (!verified) {
    UWF_LOG_W("clipboard") << "event=write result=failed reason=" << (owned ? "text-mismatch" : "ownership-not-confirmed") << " characters=" << submitted.size()
                           << " owned=" << owned << " text_verified=false";
    return ClipboardWriteResult::Failed;
  }
  UWF_LOG_I("clipboard") << "event=write result=confirmed characters=" << submitted.size() << " owned=true text_verified=true";
  return ClipboardWriteResult::Confirmed;
}

}  // namespace uwf::ui
