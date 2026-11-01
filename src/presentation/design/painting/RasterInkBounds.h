// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QRect>

class QImage;

namespace uwf::ui {
[[nodiscard]] QRect rasterInkBounds(const QImage& image);
}
