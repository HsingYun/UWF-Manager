// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAction>
#include <QPushButton>

#include "support/app/ConfigurationFixture.h"
#include "support/app/EnhancedModeFixture.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/RecordingWmiOperations.h"
#include "support/ui/FileDialogStub.h"

namespace uwf::test::application {

using uwf::test::FileDialogStub;

QPushButton* buttonWithText(QWidget* root, const QString& text);
QAction* actionWithText(QWidget* root, const QString& text);
void addLocaleThemeRows();
bool refreshCompleted(QWidget& window);
bool applyCompleted(QWidget& dialog);

}  // namespace uwf::test::application
