// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#pragma once
#include <QAbstractItemView>
#include <QAction>
#include <QPushButton>

#include "presentation/common/PageProxyModel.h"
namespace uwf::test::widgets {
QAction* actionWithText(QWidget* root, const QString& text);
QPushButton* buttonWithText(QWidget* root, const QString& text);
void checkWholeRowScrolling(QAbstractItemView* view);
void checkHoverAndSmoothDragging(QAbstractItemView* view, presentation::PageProxyModel& pages);
void initializeWidgets();
}  // namespace uwf::test::widgets
