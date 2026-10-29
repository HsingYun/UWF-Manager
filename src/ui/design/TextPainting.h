// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QFont>
#include <QRectF>
#include <QString>

class QPainter;
class QTextLayout;

namespace uwf::ui {
enum class TextVerticalReference { GlyphBounds, FontMetrics };
[[nodiscard]] QRectF textInkBounds(const QTextLayout& layout, int from = 0, int length = -1);
[[nodiscard]] QSizeF textLineSize(const QString& text, const QFont& font, bool mnemonic = false);
void paintTextLine(QPainter& painter, const QRectF& bounds, const QString& text, const QFont& font,
                   Qt::Alignment alignment = Qt::AlignLeading | Qt::AlignVCenter, bool mnemonic = false,
                   TextVerticalReference verticalReference = TextVerticalReference::GlyphBounds);
}  // namespace uwf::ui
