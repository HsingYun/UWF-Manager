// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QObject>
#include <QVariantAnimation>
class QWidget;
namespace uwf::ui {
class ControlAnimation final : public QObject {
  Q_OBJECT
 public:
  static void attach(QWidget* widget);
  [[nodiscard]] static qreal hover(const QWidget* widget, bool hovered);
  [[nodiscard]] static qreal press(const QWidget* widget, bool pressed);

 protected:
  bool eventFilter(QObject* object, QEvent* event) override;

 private:
  explicit ControlAnimation(QWidget* widget);
  void setHovered(bool hovered);
  void setPressed(bool pressed);
  QWidget* m_widget;
  bool m_hoverTarget = false;
  bool m_pressTarget = false;
  QVariantAnimation m_hover;
  QVariantAnimation m_press;
  QVariantAnimation m_busy;
  QVariantAnimation m_progress;
  int m_progressMinimum = 0;
  int m_progressMaximum = 0;
};
}  // namespace uwf::ui
