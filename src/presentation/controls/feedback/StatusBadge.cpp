// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/feedback/StatusBadge.h"

#include <QPainter>

#include "presentation/design/painting/BadgePainting.h"

namespace uwf::ui {
StatusBadge::StatusBadge(QWidget* parent) : TextLabel({}, TextStyle::Eyebrow, parent) { setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed); }
void StatusBadge::setTone(const design::Color foreground, const design::Color background) {
  m_foreground = foreground;
  m_background = background;
  update();
}
void StatusBadge::setIcon(const QIcon& icon) {
  m_icon = icon;
  updateGeometry();
  update();
}
QSize StatusBadge::sizeHint() const { return statusBadgeSize(text(), font(), m_icon); }
QSize StatusBadge::minimumSizeHint() const { return sizeHint(); }
void StatusBadge::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  paintStatusBadge(painter, rect(), text(), font(), m_icon, m_foreground, m_background, layoutDirection());
}
}  // namespace uwf::ui
