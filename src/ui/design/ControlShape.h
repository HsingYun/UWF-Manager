// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QColor>
#include <QRectF>
class QPainter;
namespace uwf::ui {
void paintControlShape(QPainter& painter, const QRectF& rect, qreal radius, const QColor& fill, const QColor& border = {}, const QColor& bottomBorder = {});
}  // namespace uwf::ui
