// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTest>
#include <QTextCursor>
#include <algorithm>

#include "presentation/views/commands/CommandOutputDialog.h"

namespace {
class CommandOutputDialogTests final : public QObject {
  Q_OBJECT

 private slots:
  void textIsCompleteSelectableAndReadOnly_data() {
    QTest::addColumn<QString>("title");
    QTest::addColumn<QString>("text");
    QTest::newRow("help") << QStringLiteral("UWF Manager - Help") << QStringLiteral("Usage: UWF.exe <parameter>\n\n    filter      Configure UWF\n");
    QTest::newRow("unicode-result") << QStringLiteral("UWF Manager")
                                    << QStringLiteral("Current session\n  Path: C:\\资料\\") + QString::fromUcs4(U"\U0001f4be") + QStringLiteral(".txt\n");
    QTest::newRow("partial-report") << QStringLiteral("UWF Manager - Command result")
                                    << QStringLiteral(
                                           "UWF configuration\nStatus: Partially read (1 failed observations)\n\n"
                                           "Current session\n  Filter\n    UWF filter: Enabled\n"
                                           "  Overlay\n    Type: Error: Access denied (0x80070005)\n\n"
                                           "Error: Access denied (0x80070005)\nEffect: None\n");
  }
  void textIsCompleteSelectableAndReadOnly() {
    QFETCH(QString, title);
    QFETCH(QString, text);
    uwf::ui::CommandOutputDialog dialog(title, text);
    auto* output = dialog.findChild<QPlainTextEdit*>();
    QVERIFY(output);
    QCOMPARE(dialog.windowTitle(), title);
    QCOMPARE(output->toPlainText(), text);
    QVERIFY(output->isReadOnly());
    QCOMPARE(output->lineWrapMode(), QPlainTextEdit::NoWrap);
    QVERIFY(output->textInteractionFlags().testFlag(Qt::TextSelectableByMouse));
    QVERIFY(output->textInteractionFlags().testFlag(Qt::TextSelectableByKeyboard));
    QSignalSpy copyAvailable(output, &QPlainTextEdit::copyAvailable);
    output->selectAll();
    QVERIFY(!copyAvailable.isEmpty());
    QVERIFY(copyAvailable.last().front().toBool());
    auto selection = output->textCursor().selectedText();
    selection.replace(QChar(QChar::ParagraphSeparator), QChar(u'\n'));
    QCOMPARE(selection, text);
    QTest::keyClicks(output, "replacement");
    QTest::keyClick(output, Qt::Key_Backspace);
    QCOMPARE(output->toPlainText(), text);
  }

  void longOutputRetainsBothEndsAndScrollsWithoutWrapping() {
    constexpr int lines = 4096;
    QString text;
    for (int index = 0; index < lines; ++index) text += QStringLiteral("Record %1: %2\n").arg(index).arg(QString(256, u'x'));
    uwf::ui::CommandOutputDialog dialog(QStringLiteral("UWF Manager"), text);
    auto* output = dialog.findChild<QPlainTextEdit*>();
    QVERIFY(output);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QTRY_VERIFY(output->verticalScrollBar()->maximum() > 0);
    QTRY_VERIFY(output->horizontalScrollBar()->maximum() > 0);
    QCOMPARE(output->toPlainText(), text);
    QCOMPARE(output->blockCount(), lines + 1);
    output->moveCursor(QTextCursor::End);
    output->ensureCursorVisible();
    QVERIFY(output->textCursor().atEnd());
    QTRY_VERIFY(output->viewport()->rect().intersects(output->cursorRect()));
    QCOMPARE(output->toPlainText(), text);
  }

  void closeActionsRejectTheDialog_data() {
    QTest::addColumn<bool>("escape");
    QTest::newRow("close-button") << false;
    QTest::newRow("escape") << true;
  }
  void closeActionsRejectTheDialog() {
    QFETCH(bool, escape);
    uwf::ui::CommandOutputDialog dialog(QStringLiteral("UWF Manager"), QStringLiteral("Complete output"));
    QSignalSpy finished(&dialog, &QDialog::finished);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    if (escape) {
      auto* output = dialog.findChild<QPlainTextEdit*>();
      QVERIFY(output);
      QTest::keyClick(output, Qt::Key_Escape);
    } else {
      auto* buttons = dialog.findChild<QDialogButtonBox*>();
      QVERIFY(buttons);
      const auto actions = buttons->buttons();
      const auto close = std::find_if(actions.begin(), actions.end(),
                                      [buttons](QAbstractButton* button) { return buttons->buttonRole(button) == QDialogButtonBox::RejectRole; });
      QVERIFY(close != actions.end());
      (*close)->click();
    }
    QCOMPARE(finished.size(), 1);
    QCOMPARE(finished.front().front().toInt(), static_cast<int>(QDialog::Rejected));
    QVERIFY(!dialog.isVisible());
  }
};
}  // namespace
QTEST_MAIN(CommandOutputDialogTests)
#include "CommandOutputDialogTests.moc"
