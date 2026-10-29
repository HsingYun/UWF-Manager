// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "TextPainting.h"

#include <QFontMetricsF>
#include <QGlyphRun>
#include <QPainter>
#include <QRawFont>
#include <QStyle>
#include <QTextLayout>
#include <algorithm>

#include "DesignSystem.h"

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
  for (const auto runs = layout.glyphRuns(from, length); const auto& run : runs) {
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
QSizeF textLineSize(const QString& text, const QFont& font, const bool mnemonic) {
  if (text.isEmpty()) return {0, 0};
  QTextLayout layout(displayText(text, mnemonic), font.resolve(DesignSystem::defaultFont()));
  const auto line = createLine(layout, 100000, Qt::AlignLeading, Qt::LayoutDirectionAuto);
  return line.isValid() ? QSizeF(line.naturalTextWidth(), line.height()) : QSizeF(0, 0);
}
void paintTextLine(QPainter& painter, const QRectF& bounds, const QString& text, const QFont& font, const Qt::Alignment alignment, const bool mnemonic,
                   const TextVerticalReference verticalReference) {
  if (text.isEmpty() || bounds.isEmpty()) return;
  const auto resolvedFont = font.resolve(DesignSystem::defaultFont());
  QTextLayout layout(displayText(text, mnemonic), resolvedFont);
  const auto line = createLine(layout, bounds.width(), alignment, painter.layoutDirection());
  if (!line.isValid()) return;
  qreal top = bounds.top();
  if (alignment.testFlag(Qt::AlignVCenter)) {
    if (verticalReference == TextVerticalReference::FontMetrics) {
      // A live label keeps the primary font's line box and baseline even when
      // descenders, accents or fallback scripts change the current glyph bounds.
      const QFontMetricsF metrics(resolvedFont);
      top += bounds.height() / 2 + (metrics.ascent() - metrics.descent()) / 2 - line.ascent();
    } else {
      const auto ink = textInkBounds(layout);
      top += ink.isEmpty() ? (bounds.height() - line.height()) / 2 : bounds.height() / 2 - ink.center().y();
    }
  } else if (alignment.testFlag(Qt::AlignBottom))
    top += bounds.height() - line.height();
  // Keep the chosen baseline on a physical pixel so small horizontal strokes
  // remain sharp on fractionally scaled displays.
  const auto transform = painter.deviceTransform();
  if (transform.type() <= QTransform::TxScale && transform.isInvertible()) {
    auto baseline = transform.map(QPointF(bounds.left(), top + line.ascent()));
    baseline.setY(qRound(baseline.y()));
    top = transform.inverted().map(baseline).y() - line.ascent();
  }
  painter.save();
  painter.setClipRect(bounds, Qt::IntersectClip);
  layout.draw(&painter, QPointF(bounds.left(), top));
  painter.restore();
}
}  // namespace uwf::ui
