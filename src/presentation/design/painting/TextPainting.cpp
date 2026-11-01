// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/design/painting/TextPainting.h"

#include <QFontMetricsF>
#include <QGlyphRun>
#include <QImage>
#include <QPainter>
#include <QRawFont>
#include <QStyle>
#include <QTextLayout>
#include <algorithm>
#include <utility>

#include "presentation/design/painting/RasterInkBounds.h"
#include "presentation/design/theme/DesignSystem.h"

namespace uwf::ui {
namespace {
QString displayText(const QString& text, const bool mnemonic) {
  if (!mnemonic) return text;
  QString value;
  value.reserve(text.size());
  for (qsizetype i = 0; i < text.size(); ++i) {
    if (text[i] == u'&') {
      if (i + 1 < text.size() && text[i + 1] == u'&')
        ++i;
      else
        continue;
    }
    value += text[i];
  }
  return value;
}
QTextLine createLine(QTextLayout& layout, const qreal width, const Qt::Alignment alignment, const Qt::LayoutDirection direction) {
  QTextOption option;
  option.setWrapMode(QTextOption::NoWrap);
  option.setAlignment(QStyle::visualAlignment(direction, alignment) & Qt::AlignHorizontal_Mask);
  option.setTextDirection(direction);
  layout.setTextOption(option);
  layout.beginLayout();
  auto line = layout.createLine();
  if (line.isValid()) line.setLineWidth(width);
  layout.endLayout();
  return line;
}
}  // namespace
QRectF textInkBounds(const QTextLayout& layout, const int from, const int length) {
  QRectF bounds;
  for (auto runs = layout.glyphRuns(from, length); const auto& run : std::as_const(runs)) {
    const auto glyphs = run.glyphIndexes();
    const auto positions = run.positions();
    const auto font = run.rawFont();
    for (qsizetype i = 0; i < std::min(glyphs.size(), positions.size()); ++i) {
      const auto ink = font.boundingRect(glyphs[i]);
      if (!ink.isEmpty()) bounds = bounds.united(ink.translated(positions[i]));
    }
  }
  return bounds;
}
QRectF textCapBounds(const QTextLayout& layout, const bool firstLineOnly) {
  if (layout.lineCount() == 0) return {};
  const QFontMetricsF metrics(layout.font());
  const auto first = layout.lineAt(0);
  const auto last = layout.lineAt(firstLineOnly ? 0 : layout.lineCount() - 1);
  const qreal top = first.y() + first.ascent() - metrics.capHeight();
  const qreal bottom = last.y() + last.ascent();
  return QRectF(0, top, first.width(), bottom - top);
}
QRectF textRasterBounds(const QTextLayout& layout, const QPointF& position, const QRectF& clip, const QTransform& device) {
  if (clip.isEmpty()) return {};
  if (device.type() > QTransform::TxScale || !device.isInvertible()) return textInkBounds(layout).translated(position).intersected(clip);
  const QRect pixels = device.mapRect(clip).toAlignedRect();
  if (pixels.isEmpty()) return {};
  QImage mask(pixels.size(), QImage::Format_ARGB32_Premultiplied);
  mask.fill(Qt::transparent);
  {
    QPainter painter(&mask);
    auto transform = device;
    transform.setMatrix(transform.m11(), transform.m12(), transform.m13(), transform.m21(), transform.m22(), transform.m23(), transform.dx() - pixels.left(),
                        transform.dy() - pixels.top(), transform.m33());
    painter.setTransform(transform);
    painter.setClipRect(clip);
    // Keep the already shaped fallback glyphs; format overrides can alter them.
    for (auto runs = layout.glyphRuns(); const auto& source : std::as_const(runs)) {
      auto glyphs = source;
      glyphs.setUnderline(false);
      glyphs.setStrikeOut(false);
      glyphs.setOverline(false);
      painter.drawGlyphRun(position, glyphs);
    }
  }
  const auto ink = rasterInkBounds(mask);
  return ink.isEmpty() ? QRectF{} : device.inverted().mapRect(QRectF(ink.translated(pixels.topLeft())));
}
QSizeF textLineSize(const QString& text, const QFont& font, const bool mnemonic) {
  if (text.isEmpty()) return {0, 0};
  QTextLayout layout(displayText(text, mnemonic), font.resolve(DesignSystem::defaultFont()));
  const auto line = createLine(layout, 100000, Qt::AlignLeading, Qt::LayoutDirectionAuto);
  return line.isValid() ? QSizeF(line.naturalTextWidth(), line.height()) : QSizeF(0, 0);
}
qreal snapTextBaseline(const qreal baseline, const QTransform& transform) {
  if (transform.type() > QTransform::TxScale || !transform.isInvertible()) return baseline;
  auto pixel = transform.map(QPointF(0, baseline));
  pixel.setY(qRound(pixel.y()));
  return transform.inverted().map(pixel).y();
}
QRectF paintTextLine(QPainter& painter, const QRectF& bounds, const QString& text, const QFont& font, const Qt::Alignment alignment, const bool mnemonic,
                     const TextVerticalReference verticalReference) {
  if (text.isEmpty() || bounds.isEmpty()) return {};
  const auto resolvedFont = font.resolve(DesignSystem::defaultFont());
  QTextLayout layout(displayText(text, mnemonic), resolvedFont);
  const auto line = createLine(layout, bounds.width(), alignment, painter.layoutDirection());
  if (!line.isValid()) return {};
  const auto ink = textInkBounds(layout);
  qreal top = bounds.top();
  if (alignment.testFlag(Qt::AlignVCenter)) {
    if (verticalReference == TextVerticalReference::FontMetrics) {
      // A live label keeps the primary font's line box and baseline even when
      // descenders, accents or fallback scripts change the current glyph bounds.
      const QFontMetricsF metrics(resolvedFont);
      top += bounds.height() / 2 + (metrics.ascent() - metrics.descent()) / 2 - line.ascent();
    } else {
      top += ink.isEmpty() ? (bounds.height() - line.height()) / 2 : bounds.height() / 2 - ink.center().y();
    }
  } else if (alignment.testFlag(Qt::AlignBottom))
    top += bounds.height() - line.height();
  // Keep the chosen baseline on a physical pixel so small horizontal strokes
  // remain sharp on fractionally scaled displays.
  top = snapTextBaseline(top + line.ascent(), painter.deviceTransform()) - line.ascent();
  auto raster = verticalReference == TextVerticalReference::GlyphBounds
                    ? textRasterBounds(layout, QPointF(bounds.left(), top), bounds, painter.deviceTransform())
                    : QRectF{};
  if (alignment.testFlag(Qt::AlignVCenter) && verticalReference == TextVerticalReference::GlyphBounds && !raster.isEmpty()) {
    const qreal aligned = snapTextBaseline(top + bounds.center().y() - raster.center().y() + line.ascent(), painter.deviceTransform()) - line.ascent();
    raster.translate(0, aligned - top);
    top = aligned;
  }
  painter.save();
  painter.setClipRect(bounds, Qt::IntersectClip);
  layout.draw(&painter, QPointF(bounds.left(), top));
  painter.restore();
  return verticalReference == TextVerticalReference::FontMetrics ? textCapBounds(layout, true).translated(bounds.left(), top).intersected(bounds)
                                                                 : raster.intersected(bounds);
}
}  // namespace uwf::ui
