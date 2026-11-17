// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include "cases/presentation/views/application/ApplicationUiTest.h"

#include <QTest>

#include "foundation/logging/Log.h"
#include "presentation/appearance/ThemeManager.h"
#include "support/visual/WidgetCapture.h"

using namespace uwf;

void ApplicationUiTest::initTestCase() {
  uwf::test::initializeVisualComparison();
  m_originalLanguage = ui::I18n::instance().lang();
  m_originalTheme = ui::ThemeManager::instance().current();
  ui::I18n::instance().setLang(ui::I18n::Lang::En);
}

void ApplicationUiTest::cleanupTestCase() {
  ui::I18n::instance().setLang(m_originalLanguage);
  ui::ThemeManager::instance().apply(m_originalTheme);
  clearLogLines();
}

#include "moc_ApplicationUiTest.cpp"
