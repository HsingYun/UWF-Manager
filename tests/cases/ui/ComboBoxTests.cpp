// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QComboBox>
#include <QListView>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>

#include "ui/appearance/ThemeManager.h"

namespace {
class ComboBoxTests final : public QObject {
  Q_OBJECT

 private slots:
  void styledPopupAllowsVisibleSelection_data() {
    QTest::addColumn<bool>("dark");
    QTest::newRow("light") << false;
    QTest::newRow("dark") << true;
  }
  void styledPopupAllowsVisibleSelection() {
    QFETCH(bool, dark);
    auto& theme = uwf::ui::ThemeManager::instance();
    const auto previous = theme.current();
    const auto restore = qScopeGuard([&] { theme.apply(previous); });
    theme.apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
    QComboBox combo;
    combo.addItem(QStringLiteral("RAM"), 10);
    combo.addItem(QStringLiteral("Disk"), 20);
    combo.resize(240, 40);
    combo.show();
    QVERIFY(QTest::qWaitForWindowExposed(&combo));
    QSignalSpy selection(&combo, &QComboBox::activated);
    combo.showPopup();
    auto* popup = combo.view();
    QTRY_VERIFY(popup->isVisible());
    const auto index = combo.model()->index(1, 0);
    const auto bounds = popup->visualRect(index);
    QVERIFY(popup->viewport()->rect().contains(bounds));
    QTest::mouseClick(popup->viewport(), Qt::LeftButton, Qt::NoModifier, bounds.center());
    QTRY_COMPARE(combo.currentData().toInt(), 20);
    QCOMPARE(combo.currentText(), QStringLiteral("Disk"));
    QCOMPARE(selection.count(), 1);
    QTRY_VERIFY(!popup->isVisible());
  }
};
}  // namespace

QTEST_MAIN(ComboBoxTests)
#include "ComboBoxTests.moc"
