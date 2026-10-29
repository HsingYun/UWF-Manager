// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QObject>
#include <QPointer>
class QFocusFrame;
namespace uwf::ui {
class FocusIndicator final : public QObject {
 public:
  static void install();
  [[nodiscard]] static bool keyboardNavigation();

 protected:
  bool eventFilter(QObject* object, QEvent* event) override;

 private:
  FocusIndicator();
  void updateIndicator();
  void setKeyboardNavigation(bool keyboard);
  bool m_keyboard = false;
  QPointer<QFocusFrame> m_frame;
};
}  // namespace uwf::ui
