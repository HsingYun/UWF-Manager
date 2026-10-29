// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QRectF>
class QPainter;
class QWidget;
namespace uwf::ui {
void configurePopupAppearance(QWidget* popup);
void paintPopupSurface(QPainter& painter, const QRectF& bounds);
}  // namespace uwf::ui
