// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/design/painting/BadgePainting.h"

#include <QPainter>

#include "presentation/design/controls/ControlShape.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/design/painting/IconPainting.h"
#include "presentation/design/painting/TextPainting.h"
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
QSize statusBadgeSize(const QString& text, const QFont& font, const QIcon& icon, const int height, const int padding) {
  return {qCeil(textLineSize(text, font).width()) + padding * 2 + (icon.isNull() ? 0 : 18), height};
}
void paintStatusBadge(QPainter& painter, const QRectF& bounds, const QString& text, const QFont& font, const QIcon& icon, const design::Color foreground,
                      const design::Color background, const Qt::LayoutDirection direction, const int padding, const int radius) {
  const auto& colors = DesignSystem::instance();
  painter.save();
  painter.setRenderHint(QPainter::Antialiasing);
  paintControlShape(painter, bounds, radius, colors.color(background));
  QRectF copy = bounds.adjusted(padding, 0, -padding, 0);
  const bool rtl = direction == Qt::RightToLeft;
  if (!icon.isNull()) copy.adjust(rtl ? 0 : 18, 0, rtl ? -18 : 0, 0);
  painter.setPen(colors.color(foreground));
  const auto textInk = paintTextLine(painter, copy, text, font, Qt::AlignCenter);
  if (!icon.isNull()) {
    const qreal center = textInk.isEmpty() ? bounds.center().y() : textInk.center().y();
    paintIcon(painter, icon, QRectF(rtl ? bounds.right() - padding - 13 : bounds.left() + padding, center - 6.5, 13, 13),
              colors.color(foreground));
  }
  painter.restore();
}
}  // namespace uwf::ui
