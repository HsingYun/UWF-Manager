// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPixmap>
#include <QRectF>
#include <QString>
#include <memory>

class QWidget;

namespace uwf::presentation {
class LogViewModel;
}

namespace uwf::test {

bool visualComparisonEnabled();
void initializeVisualComparison();
QString visualFixtureDirectory(const QString& fallback);
std::shared_ptr<presentation::LogViewModel> screenshotLogModel();
void prepareScreenshotWindow(QWidget& widget, bool preservePointer = false);
QPixmap screenshot(QWidget& widget, bool preservePointer = false);
QRectF renderedInkBounds(QWidget& widget, qreal scale = 0, const QRectF& region = {});
bool saveScreenshot(QWidget& widget, const QString& state, bool preservePointer = false);
bool saveAnimatedScreenshot(QWidget& widget, const QString& state);
QPixmap nativeScreenshot(QWidget& widget);
QPixmap windowScreenshot(QWidget& widget);

}  // namespace uwf::test
