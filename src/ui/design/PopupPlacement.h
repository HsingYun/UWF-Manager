// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QRect>
class QWidget;

namespace uwf::ui {
[[nodiscard]] QRect anchoredPopupGeometry(const QRect& anchor, QSize preferredSize, const QRect& workArea, Qt::LayoutDirection direction);
void bindPopupToAnchor(QWidget* popup, QWidget* anchor);
}  // namespace uwf::ui
