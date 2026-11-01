// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPointer>
#include <QPushButton>
#include <QScopeGuard>
#include <QTest>
#include <QTimer>
#include <QVariant>
#include <QWidget>
#include <functional>
#include <utility>

namespace uwf::test {

// File I/O and model refreshes can process events before opening their dialog.
// Keep the driver alive until a new modal appears, and always release exec()
// even when an assertion in inspect returns early.
[[nodiscard]] inline bool driveModalDialog(QObject& owner, const std::function<void()>& trigger, const std::function<void(QDialog&)>& inspect,
                                           const int timeoutMs = 5000) {
  const QPointer<QWidget> previous = QApplication::activeModalWidget();
  const QPointer<QObject> lifetime = &owner;
  QTimer driver;
  driver.setInterval(10);
  bool observed = false;
  QObject::connect(&driver, &QTimer::timeout, &owner, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog || dialog == previous) return;
    driver.stop();
    const QPointer<QDialog> current = dialog;
    const auto close = qScopeGuard([current] {
      if (current && current->isVisible()) current->reject();
    });
    observed = true;
    inspect(*dialog);
  });
  driver.start();
  trigger();
  (void)QTest::qWaitFor([&] { return observed || !lifetime; }, timeoutMs);
  return observed;
}

[[nodiscard]] inline QPushButton* buttonByAccessibleName(QWidget* parent, const QString& name) {
  if (!parent) return nullptr;
  for (auto buttons = parent->findChildren<QPushButton*>(); auto* button : std::as_const(buttons))
    if (button->property("accessibleName").toString() == name) return button;
  return nullptr;
}

[[nodiscard]] inline QPushButton* buttonByRole(QWidget* parent, const QDialogButtonBox::ButtonRole role) {
  if (!parent) return nullptr;
  for (auto boxes = parent->findChildren<QDialogButtonBox*>(); auto* box : std::as_const(boxes))
    for (auto buttons = box->buttons(); auto* button : std::as_const(buttons))
      if (box->buttonRole(button) == role) return qobject_cast<QPushButton*>(button);
  return nullptr;
}

}  // namespace uwf::test
