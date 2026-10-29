// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ApplicationUiFixture.h"

#include <QDialogButtonBox>
#include <QTest>

#include "ui/appearance/I18n.h"

namespace uwf::test::application {
bool refreshCompleted(QWidget& window) {
  for (const auto actions = window.actions(); auto* action : actions)
    if (action->shortcut() == QKeySequence(QKeySequence::Refresh)) return action->isEnabled();
  return false;
}
bool applyCompleted(QWidget& dialog) {
  for (const auto boxes = dialog.findChildren<QDialogButtonBox*>(); auto* box : boxes)
    for (const auto buttons = box->buttons(); auto* button : buttons)
      if (box->buttonRole(button) == QDialogButtonBox::RejectRole && button->text() == ui::I18n::tr("Close")) return button->isEnabled();
  return false;
}

void addLocaleThemeRows() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  for (const auto& locale : ui::I18n::languages())
    for (const bool dark : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString(locale.code), dark ? "dark" : "light"))) << locale.id << dark;
}

QPushButton* buttonWithText(QWidget* root, const QString& text) {
  for (const auto buttons = root->findChildren<QPushButton*>(); auto* button : buttons) {
    if (button->text() == text || button->property("accessibleName").toString() == text) return button;
  }
  return nullptr;
}

QAction* actionWithText(QWidget* root, const QString& text) {
  for (const auto actions = root->findChildren<QAction*>(); auto* action : actions) {
    if (action->text() == text) return action;
  }
  return nullptr;
}

}  // namespace uwf::test::application
