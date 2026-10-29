// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "BadgePainting.h"

#include <QPainter>

#include "ControlShape.h"
#include "DesignSystem.h"
#include "IconPainting.h"
#include "TextPainting.h"
namespace uwf::ui {
QSize countBadgeSize(const QString& text, const QFont& font) { return {std::max(20, qCeil(textLineSize(text, font).width()) + 14), 20}; }
void paintCountBadge(QPainter& painter, const QRectF& bounds, const QString& text, const QFont& font) {
  const auto& colors = DesignSystem::instance();
  painter.save();
  paintControlShape(painter, bounds, 4, colors.color(design::Color::SurfaceSoft), colors.color(design::Color::Line));
  painter.setPen(colors.color(design::Color::Muted));
  paintTextLine(painter, bounds, text, font, Qt::AlignCenter);
  painter.restore();
}
QSize statusBadgeSize(const QString& text, const QFont& font, const QIcon& icon) {
  return {qCeil(textLineSize(text, font).width()) + 16 + (icon.isNull() ? 0 : 18), 24};
}
void paintStatusBadge(QPainter& painter, const QRectF& bounds, const QString& text, const QFont& font, const QIcon& icon, const design::Color foreground,
                      const design::Color background, const Qt::LayoutDirection direction) {
  const auto& colors = DesignSystem::instance();
  painter.save();
  painter.setRenderHint(QPainter::Antialiasing);
  paintControlShape(painter, bounds, 4, colors.color(background));
  QRectF copy = bounds.adjusted(8, 0, -8, 0);
  if (!icon.isNull()) {
    const bool rtl = direction == Qt::RightToLeft;
    paintIcon(painter, icon, QRectF(rtl ? bounds.right() - 21 : bounds.left() + 8, bounds.center().y() - 6.5, 13, 13), colors.color(foreground));
    copy.adjust(rtl ? 0 : 18, 0, rtl ? -18 : 0, 0);
  }
  painter.setPen(colors.color(foreground));
  paintTextLine(painter, copy, text, font, Qt::AlignCenter);
  painter.restore();
}
}  // namespace uwf::ui
