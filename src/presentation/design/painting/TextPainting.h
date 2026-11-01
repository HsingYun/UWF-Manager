// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QFont>
#include <QRectF>
#include <QString>

class QPainter;
class QTextLayout;
class QTransform;

namespace uwf::ui {
enum class TextVerticalReference { GlyphBounds, FontMetrics };
[[nodiscard]] QRectF textInkBounds(const QTextLayout& layout, int from = 0, int length = -1);
// The primary font's cap-height band gives graphics a stable optical reference.
[[nodiscard]] QRectF textCapBounds(const QTextLayout& layout, bool firstLineOnly = false);
[[nodiscard]] QRectF textRasterBounds(const QTextLayout& layout, const QPointF& position, const QRectF& clip, const QTransform& deviceTransform);
[[nodiscard]] QSizeF textLineSize(const QString& text, const QFont& font, bool mnemonic = false);
[[nodiscard]] qreal snapTextBaseline(qreal baseline, const QTransform& deviceTransform);
// Returns the selected alignment reference at the painted position.
QRectF paintTextLine(QPainter& painter, const QRectF& bounds, const QString& text, const QFont& font,
                     Qt::Alignment alignment = Qt::AlignLeading | Qt::AlignVCenter, bool mnemonic = false,
                     TextVerticalReference verticalReference = TextVerticalReference::GlyphBounds);
}  // namespace uwf::ui
