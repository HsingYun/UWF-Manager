// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QImage>
#include <QJsonObject>
#include <QRect>

namespace uwf::test::tools {

struct PixelDifference {
  qint64 changedPixels = 0;
  QRect bounds;
  bool sizeMismatch = false;
};

PixelDifference comparePixels(const QImage& reference, const QImage& actual);
QJsonObject compareImageDirectories(const QString& reference, const QString& actual, qsizetype expectedCount = 0);

}  // namespace uwf::test::tools
