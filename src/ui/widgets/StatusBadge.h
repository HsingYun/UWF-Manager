// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QIcon>

#include "TextLabel.h"
namespace uwf::ui {
class StatusBadge final : public TextLabel {
 public:
  explicit StatusBadge(QWidget* parent = nullptr);
  void setTone(design::Color foreground, design::Color background);
  void setIcon(const QIcon& icon);
  [[nodiscard]] QSize sizeHint() const override;
  [[nodiscard]] QSize minimumSizeHint() const override;

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  design::Color m_foreground = design::Color::Muted;
  design::Color m_background = design::Color::SurfaceSoft;
  QIcon m_icon;
};
}  // namespace uwf::ui
