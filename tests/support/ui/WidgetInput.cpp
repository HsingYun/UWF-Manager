// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WidgetInput.h"

#include <QtTest/qtestspontaneevent.h>

#include <QApplication>
#include <QCursor>
#include <QEnterEvent>
#include <QPointer>
#include <QTest>
#include <QWheelEvent>
#include <QWidget>
#include <QWindow>

namespace uwf::test {
namespace {
QPointer<QWindow>& enteredWindow() {
  static QPointer<QWindow> window;
  return window;
}

template <typename Position>
void moveMouseTo(QWindow* window, Position position, const int delay) {
  QVERIFY(window && window->isVisible());
  if (!window->isActive() && (window->type() == Qt::Window || window->type() == Qt::Dialog) && !window->flags().testFlag(Qt::WindowDoesNotAcceptFocus)) {
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));
  }
  // SetCursorPos queues native enter/leave messages. Deliver that handoff
  // before injecting Qt input, otherwise a late OS move can undo its hover.
  // The OS may clamp coordinates at monitor edges; the widget assertions
  // below the driver verify the actual delivered hover and drag behavior.
  QCursor::setPos(window->mapToGlobal(position()));
  constexpr int nativeCursorDeliveryMs = 20;
  QTest::qWait(nativeCursorDeliveryMs);
  // Activation and native events can resize the window and relayout children.
  // Resolve the target again after that handoff, rather than using stale pixels.
  // Qt's QWindow mouse driver only sends movement, and filters unchanged
  // global positions. A newly created dialog at the same position therefore
  // also needs the window enter/leave part of the pointer transition.
  auto& previousWindow = enteredWindow();
  if (previousWindow != window) {
    if (previousWindow) {
      QEvent leave(QEvent::Leave);
      QSpontaneKeyEvent::setSpontaneous(&leave);
      qApp->notify(previousWindow, &leave);
    }
    const auto local = position();
    QEnterEvent enter(local, local, window->mapToGlobal(local));
    QSpontaneKeyEvent::setSpontaneous(&enter);
    qApp->notify(window, &enter);
    previousWindow = window;
  }
  QTest::mouseMove(window, position(), delay);
  QCoreApplication::processEvents();
}
}  // namespace
void moveMouse(QWindow* window, const QPoint position, const int delay) {
  moveMouseTo(window, [=] { return position.isNull() ? QPoint(window->width() / 2, window->height() / 2) : position; }, delay);
}
void moveMouse(QWidget& target, const QPoint position, const int delay) {
  auto* window = target.window();
  QVERIFY(window->isVisible() && window->windowHandle());
  const auto local = position.isNull() ? target.rect().center() : position;
  // Qt's widget press/release driver owns synthetic button state. Native cursor
  // movement would add an OS event with no physical button held during a drag.
  if (QTestPrivate::qtestMouseButtons != Qt::NoButton) {
    QTest::mouseMove(&target, local, delay);
    return;
  }
  moveMouseTo(window->windowHandle(), [&] { return target.mapTo(window, position.isNull() ? target.rect().center() : position); }, delay);
}

// Deliver spontaneous logical coordinates through the native window so high-DPI
// hit testing and parent propagation follow the same path as real wheel input.
void sendWheel(QWidget& target, const QPoint angleDelta) {
  auto* window = target.window();
  QVERIFY(window->isVisible() && window->windowHandle());
  const QPoint position = target.mapTo(window, target.rect().center());
  QWheelEvent event(position, window->mapToGlobal(position), QPoint(), angleDelta, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
  QSpontaneKeyEvent::setSpontaneous(&event);
  qApp->notify(window->windowHandle(), &event);
  QCoreApplication::processEvents();
}

}  // namespace uwf::test
