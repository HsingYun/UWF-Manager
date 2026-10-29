// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "IconView.h"

#include <QPainter>

#include "ui/design/DesignSystem.h"
#include "ui/design/IconPainting.h"

namespace uwf::ui {
IconView::IconView(const QIcon& icon, const int size, const design::Color color, QWidget* parent)
    : QWidget(parent), m_icon(icon), m_size(size), m_color(color) {
  setFixedSize(size, size);
  connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
}
void IconView::setIcon(const QIcon& icon) {
  m_icon = icon;
  update();
}
void IconView::setColorRole(const design::Color color) {
  m_color = color;
  update();
}
QSize IconView::sizeHint() const { return {m_size, m_size}; }
void IconView::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  paintIcon(painter, m_icon, QRectF((width() - m_size) / 2.0, (height() - m_size) / 2.0, m_size, m_size), DesignSystem::instance().color(m_color));
}
}  // namespace uwf::ui
