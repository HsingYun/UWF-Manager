// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "app/configuration/ExclusionEditor.h"
#include "app/configuration/ExclusionRules.h"
#include "app/ports/VolumePaths.h"
namespace {
class ExclusionEditingTests final : public QObject {
  Q_OBJECT
 private slots:
  void equivalentFilePathsShareOneDraft_data() {
    QTest::addColumn<QString>("alias");
    QTest::addColumn<bool>("baselineFirst");
    const QStringList aliases{QStringLiteral("z:/DATA/state.bin"), QStringLiteral("/Data/./state.bin"), QStringLiteral("Z:/Scratch/../Data/state.bin/"),
                              QStringLiteral("\\Scratch\\..\\Data\\state.bin")};
    for (qsizetype index = 0; index < aliases.size(); ++index)
      for (const bool baselineFirst : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("alias-%1-baseline-first-%2").arg(index).arg(baselineFirst))) << aliases.at(index) << baselineFirst;
  }
  void equivalentFilePathsShareOneDraft() {
    QFETCH(QString, alias);
    QFETCH(bool, baselineFirst);
    using Editor = uwf::app::ExclusionEditor;
    using Outcome = Editor::ImportOutcome;
    Editor editor(Editor::Kind::File);
    if (!baselineFirst) editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setBaseline({QStringLiteral("\\Data\\state.bin"), QStringLiteral("Z:/Data/./state.bin/")}, {QStringLiteral("\\Data\\Nested\\..\\state.bin")});
    if (baselineFirst) editor.setDriveLetter(QStringLiteral("Z:"));
    QCOMPARE(editor.draft().current, QStringList{QStringLiteral("Z:\\Data\\state.bin")});
    QCOMPARE(editor.draft().next, editor.draft().current);
    QCOMPARE(editor.addEntry(alias).outcome, Outcome::NoOp);
    QCOMPARE(editor.importAdd(alias), Outcome::NoOp);
    QVERIFY(editor.pendingAdded().isEmpty());

    QVERIFY(editor.removeSelection({{alias, std::nullopt}}));
    QCOMPARE(editor.pendingRemoved().size(), 1);
    QVERIFY(editor.restoreSelection({{alias, std::nullopt}}));
    QVERIFY(editor.pendingRemoved().isEmpty());
    QCOMPARE(editor.importRemove(alias), Outcome::Applied);
    QCOMPARE(editor.importRemove(alias), Outcome::NoOp);
    QCOMPARE(editor.importAdd(alias), Outcome::Applied);
    QVERIFY(editor.pendingAdded().isEmpty());
    QVERIFY(editor.pendingRemoved().isEmpty());
  }
  void driveBindingNormalizesPendingFilePaths() {
    using Editor = uwf::app::ExclusionEditor;
    using Outcome = Editor::ImportOutcome;
    Editor editor(Editor::Kind::File);
    editor.setBaseline({}, {QStringLiteral("\\Data\\Old.bin")});
    QCOMPARE(editor.importRemove(QStringLiteral("/Data/./Old.bin")), Outcome::Applied);
    QCOMPARE(editor.importAdd(QStringLiteral("/Data/Cache/../New.bin")), Outcome::Applied);
    editor.setDriveLetter(QStringLiteral("Z:"));
    QCOMPARE(editor.pendingRemoved(), QStringList{QStringLiteral("Z:\\Data\\Old.bin")});
    QCOMPARE(editor.pendingAdded(), QStringList{QStringLiteral("Z:\\Data\\New.bin")});
    QCOMPARE(editor.importAdd(QStringLiteral("Z:/Data/Old.bin")), Outcome::Applied);
    QCOMPARE(editor.importRemove(QStringLiteral("Z:/Data/New.bin")), Outcome::Applied);
    QVERIFY(editor.pendingAdded().isEmpty());
    QVERIFY(editor.pendingRemoved().isEmpty());
  }
  void forbiddenFilePathsRemainForbiddenThroughAliases_data() {
    QTest::addColumn<QString>("drive");
    QTest::addColumn<QString>("path");
    const QString system = QString::fromStdString(uwf::drive::systemLetter());
    QTest::newRow("volume-root") << QStringLiteral("Z:") << QStringLiteral("Z:/Data/..");
    QTest::newRow("lowercase-volume-root") << QStringLiteral("z:") << QStringLiteral("z:/Data/..");
    QTest::newRow("pagefile") << QStringLiteral("Z:") << QStringLiteral("Z:/Data/../pagefile.sys");
    QTest::newRow("swapfile") << QStringLiteral("Z:") << QStringLiteral("/Data/../swapfile.sys");
    QTest::newRow("hibernation") << QStringLiteral("Z:") << QStringLiteral("/./hiberfil.sys");
    QTest::newRow("windows") << system << system + QStringLiteral("/Data/../Windows");
    QTest::newRow("system32") << system << QStringLiteral("/Windows/Temp/../System32");
    QTest::newRow("registry-hive") << system << QStringLiteral("/Windows/System32/Temp/../config/SYSTEM");
    QTest::newRow("user-hive") << system << QStringLiteral("/Users/Alice/Cache/../NTUSER.DAT");
  }
  void forbiddenFilePathsRemainForbiddenThroughAliases() {
    QFETCH(QString, drive);
    QFETCH(QString, path);
    using Editor = uwf::app::ExclusionEditor;
    Editor editor(Editor::Kind::File);
    editor.setDriveLetter(drive);
    QVERIFY(!uwf::app::FileExclusionRules::rejectionReason(path, drive).isEmpty());
    QCOMPARE(editor.addEntry(path).outcome, Editor::ImportOutcome::RejectedForbidden);
    QCOMPARE(editor.importAdd(path), Editor::ImportOutcome::RejectedForbidden);
    QVERIFY(editor.pendingAdded().isEmpty());
  }
  void fileCommitEligibilityReflectsCurrentProtectionAndFileFacts() {
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString filePath = QDir::toNativeSeparators(temporary.filePath(QStringLiteral("data.txt")));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    uwf::app::ExclusionEditor editor(uwf::app::ExclusionEditor::Kind::File);
    QVERIFY(!editor.fileCommitKind(filePath));
    editor.setCommitEnabled(true);
    QCOMPARE(editor.fileCommitKind(filePath), std::optional{uwf::app::ExclusionEditor::FileEntryKind::File});
    QCOMPARE(editor.fileCommitKind(temporary.path()), std::optional{uwf::app::ExclusionEditor::FileEntryKind::Directory});
    QVERIFY(!editor.fileCommitKind(temporary.filePath(QStringLiteral("missing"))));
    editor.setBaseline({filePath.toUpper()}, {});
    QVERIFY(!editor.fileCommitKind(filePath));
  }
};
}  // namespace
QTEST_APPLESS_MAIN(ExclusionEditingTests)
#include "ExclusionEditingTests.moc"
