// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QVariantAnimation>
#include <QWidget>
#include "design/DesignTokens.h"
namespace uwf::ui {
class BusyIndicator final : public QWidget {
  Q_OBJECT
 public:
  explicit BusyIndicator(QWidget* parent = nullptr);
  [[nodiscard]] QSize sizeHint() const override;
  void setColorRole(design::Color role);
  void setStrokeWidth(qreal width);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;

 private:
  QVariantAnimation m_rotation;
  design::Color m_color = design::Color::Accent;
  qreal m_strokeWidth = 1.65;
};
}  // namespace uwf::ui
