// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QFont>
#include <QIcon>
#include <QRectF>

#include "design/DesignTokens.h"
class QPainter;
namespace uwf::ui {
[[nodiscard]] QSize statusBadgeSize(const QString& text, const QFont& font, const QIcon& icon, int height = 24, int padding = 8);
[[nodiscard]] QSize countBadgeSize(const QString& text, const QFont& font);
void paintCountBadge(QPainter& painter, const QRectF& bounds, const QString& text, const QFont& font);
void paintStatusBadge(QPainter& painter, const QRectF& bounds, const QString& text, const QFont& font, const QIcon& icon, design::Color foreground,
                      design::Color background, Qt::LayoutDirection direction, int padding = 8, int radius = 4);
}  // namespace uwf::ui
