// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QTest>
#include <QTimer>
#include <QVBoxLayout>

#include "presentation/controls/feedback/ContentHintBox.h"
#include "support/ui/TextLayoutChecks.h"

namespace {
class TextLayoutChecksTests final : public QObject {
  Q_OBJECT

 private slots:
  void detectsClippedReadOnlyDetails() {
    QWidget root;
    uwf::ui::ContentHintBox text(&root);
    text.setGeometry(20, 20, 360, 96);
    text.setPlainText(QStringLiteral("Complete diagnostic information."));
    root.resize(400, 200);
    root.show();
    QVERIFY(QTest::qWaitForWindowExposed(&root));
    QVERIFY(uwf::test::clippedText(root).isEmpty());
    text.setPlainText(QStringLiteral("Complete diagnostic information.\n").repeated(100));
    QVERIFY(!uwf::test::clippedText(root).isEmpty());
  }
  void detectsClippedLabelsAndIgnoresHiddenText_data() {
    QTest::addColumn<bool>("wrapped");
    QTest::newRow("single-line") << false;
    QTest::newRow("wrapped") << true;
  }
  void detectsClippedLabelsAndIgnoresHiddenText() {
    QFETCH(bool, wrapped);
    QWidget root;
    QLabel label(QStringLiteral("A complete label with enough words to occupy several lines"), &root);
    label.setWordWrap(wrapped);
    label.resize(wrapped ? QSize(120, 200) : label.sizeHint());
    root.resize(800, 240);
    root.show();
    QVERIFY(QTest::qWaitForWindowExposed(&root));
    QVERIFY(uwf::test::clippedText(root).isEmpty());
    label.resize(wrapped ? QSize(120, 1) : QSize(10, label.height()));
    QVERIFY(!uwf::test::clippedText(root).isEmpty());
    label.hide();
    QVERIFY(uwf::test::clippedText(root).isEmpty());
  }
  void detectsClippedButtonText() {
    QWidget root;
    QPushButton button(QStringLiteral("A complete action label"), &root);
    button.resize(button.sizeHint());
    root.resize(400, 100);
    root.show();
    QVERIFY(QTest::qWaitForWindowExposed(&root));
    QVERIFY(uwf::test::clippedText(root).isEmpty());
    button.resize(5, button.height());
    QVERIFY(!uwf::test::clippedText(root).isEmpty());
  }
  void inspectsActualModelViewHeadersAndIgnoresHiddenColumns() {
    QWidget root;
    QVBoxLayout layout(&root);
    QTableView table;
    QStandardItemModel model(1, 1);
    model.setHeaderData(0, Qt::Horizontal, QStringLiteral("A complete header"));
    table.setModel(&model);
    table.horizontalHeader()->setMinimumSectionSize(1);
    table.horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    layout.addWidget(&table);
    root.resize(600, 200);
    root.show();
    QVERIFY(QTest::qWaitForWindowExposed(&root));
    table.setColumnWidth(0, 300);
    QVERIFY(uwf::test::clippedText(root).isEmpty());
    table.setColumnWidth(0, 5);
    QVERIFY(!uwf::test::clippedText(root).isEmpty());
    table.setColumnHidden(0, true);
    QVERIFY(uwf::test::clippedText(root).isEmpty());
  }
  void rejectsInspectionOfAnInvisibleRoot() {
    QWidget root;
    QVERIFY(!uwf::test::clippedText(root).isEmpty());
  }
  void waitsForAsynchronouslyCorrectedText() {
    QWidget root;
    QLabel label(QStringLiteral("A complete label that initially needs more width"), &root);
    const auto completeSize = label.sizeHint();
    label.resize(10, completeSize.height());
    root.resize(completeSize.width() + 20, completeSize.height() + 20);
    root.show();
    QVERIFY(QTest::qWaitForWindowExposed(&root));
    QVERIFY(!uwf::test::clippedText(root).isEmpty());
    QTimer::singleShot(0, &label, [&] { label.resize(completeSize); });
    const auto clipping = uwf::test::waitForUnclippedText(root);
    QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
  }
  void timeoutPreservesTheClippedTextError() {
    QWidget root;
    QLabel label(QStringLiteral("A complete label that remains clipped"), &root);
    label.setObjectName(QStringLiteral("clippedLabel"));
    label.resize(10, label.sizeHint().height());
    root.resize(400, 100);
    root.show();
    QVERIFY(QTest::qWaitForWindowExposed(&root));
    const auto expected = uwf::test::clippedText(root);
    QVERIFY(!expected.isEmpty());
    QCOMPARE(uwf::test::waitForUnclippedText(root, 20), expected);
  }
};
}  // namespace

QTEST_MAIN(TextLayoutChecksTests)
#include "TextLayoutChecksTests.moc"
