// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QColor>
#include <QIcon>
#include <QRectF>
#include <QString>
class QPainter;
class QPainterPath;

namespace uwf::ui {
[[nodiscard]] QIcon coloredSvgIcon(const QString& resourcePath, const QColor& ink, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter);
void paintIcon(QPainter& painter, const QIcon& icon, const QRectF& bounds, const QColor& ink = {});
void paintIconPath(QPainter& painter, const QPainterPath& path, const QRectF& bounds, const QColor& ink, qreal strokeWidth);
}  // namespace uwf::ui
