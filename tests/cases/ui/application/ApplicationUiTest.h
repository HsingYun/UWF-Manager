// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#pragma once

#include <QObject>

#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"

class ApplicationUiTest : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase();
  void cleanupTestCase();

 private:
  uwf::ui::I18n::Lang m_originalLanguage = uwf::ui::I18n::Lang::En;
  uwf::app::Theme m_originalTheme = uwf::app::Theme::Dark;
};
