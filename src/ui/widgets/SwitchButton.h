// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractButton>
#include <QVariantAnimation>
namespace uwf::ui {
class SwitchButton : public QAbstractButton {
  Q_OBJECT
 public:
  explicit SwitchButton(QWidget* parent = nullptr);
  [[nodiscard]] QSize sizeHint() const override;

 protected:
  void paintEvent(QPaintEvent* event) override;
  void checkStateSet() override;
  void nextCheckState() override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  bool event(QEvent* event) override;

 private:
  void updatePosition(bool animate);
  bool m_userChange = false;
  qreal m_target = 0;
  QVariantAnimation m_position;
  qreal m_checked = 0;
  QPointF m_pressPosition;
  qreal m_dragOrigin = 0;
  bool m_pointerActive = false;
  bool m_dragging = false;
  qreal m_stretch = 0;
};
}  // namespace uwf::ui
