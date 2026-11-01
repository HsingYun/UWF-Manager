// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QCoreApplication>
#include <QEvent>
#include <QGuiApplication>
#include <QPointer>
#include <QTest>
#include <QWidget>
#include <QWindow>

namespace uwf::test {
inline bool activateWindowForInput(QWidget& widget, const int timeoutMs = 2000) {
  const QPointer<QWidget> owner(widget.window());
  // Drain deferred layout and companion-window shows before choosing the
  // actual input window. An active owner can still have a focused tool window.
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QGuiApplication::sync();
  const QPointer<QWindow> handle(owner ? owner->windowHandle() : nullptr);
  if (!handle) return false;
  handle->requestActivate();
  return QTest::qWaitFor([handle] { return handle && QGuiApplication::focusWindow() == handle.data(); }, timeoutMs);
}
}  // namespace uwf::test
