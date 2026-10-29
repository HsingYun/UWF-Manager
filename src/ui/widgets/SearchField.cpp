// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SearchField.h"

#include <QEvent>
#include <QPainter>

#include "ui/design/DesignSystem.h"
#include "ui/design/IconPainting.h"
namespace uwf::ui {
SearchField::SearchField(QWidget* parent) : QLineEdit(parent) {
  auto type = font();
  type.setPixelSize(12);
  type.setWeight(QFont::Normal);
  setFont(type);
  setClearButtonEnabled(true);
  updateMargins();
}
void SearchField::updateMargins() {
  const bool rtl = layoutDirection() == Qt::RightToLeft;
  setTextMargins(rtl ? 0 : 23, 0, rtl ? 23 : 0, 0);
}
void SearchField::changeEvent(QEvent* event) {
  QLineEdit::changeEvent(event);
  if (event->type() == QEvent::LayoutDirectionChange) updateMargins();
}
void SearchField::paintEvent(QPaintEvent* event) {
  QLineEdit::paintEvent(event);
  QPainter painter(this);
  const QRectF box(layoutDirection() == Qt::RightToLeft ? width() - 26 : 11, (height() - 15) / 2.0, 15, 15);
  if (!isEnabled()) painter.setOpacity(.5);
  paintIcon(painter, QIcon(QStringLiteral(":/icons/search.svg")), box, DesignSystem::instance().color(design::Color::Muted));
}
}  // namespace uwf::ui
#include "moc_SearchField.cpp"
