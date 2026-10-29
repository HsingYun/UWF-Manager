// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QIcon>
#include <QWidget>

#include "design/DesignTokens.h"

namespace uwf::ui {
class IconView final : public QWidget {
 public:
  explicit IconView(const QIcon& icon, int size = design::iconSize, design::Color color = design::Color::Muted, QWidget* parent = nullptr);
  void setIcon(const QIcon& icon);
  void setColorRole(design::Color color);
  [[nodiscard]] QSize sizeHint() const override;

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  QIcon m_icon;
  int m_size;
  design::Color m_color;
};
}  // namespace uwf::ui
