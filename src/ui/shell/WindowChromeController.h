// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QMargins>
#include <QObject>
#include <QPointer>
#include <QRect>

class QMainWindow;
class QAbstractButton;

namespace uwf::ui {

// Native window geometry, resizing, activation and drag interactions.
class WindowChromeController : public QObject {
 public:
  explicit WindowChromeController(QMainWindow* window, QObject* parent = nullptr);

  void applyTitleBarTheme() const;
  void initializeGeometry() const;
  [[nodiscard]] static QRect startupGeometry(const QRect& available, const QMargins& frameMargins, const QSize& minimumClientSize);
  void raiseToFront() const;
  bool nativeEvent(void* message, qintptr* result);

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  QMainWindow* m_window;
  QPointer<QAbstractButton> m_pressedCaption;
};

}  // namespace uwf::ui
