// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <array>
#include <memory>
#include <stdexcept>

#include "application/configuration/exclusions/ExclusionEditor.h"
#include "application/configuration/exclusions/ExclusionRules.h"
#include "application/paths/ExclusionCoverage.h"
#include "domain/uwf/contracts/Types.h"
#include "support/app/ReadOnlyFileBrowserFixture.h"

namespace {
class ExclusionEditingTests final : public QObject {
  Q_OBJECT
 private slots:
  void storedFileAddressesShareIdentityAndRetainTheirRemovalSources() {
    using namespace uwf::app;
    const std::string volume = "Volume{11111111-2222-3333-4444-555555555555}";
    const QString drive = QStringLiteral("Z:\\Data\\state.bin");
    const QString guid = QString::fromStdString(uwf::compat::volumePath(volume)) + QStringLiteral("Data\\state.bin");
    for (const bool reverse : {false, true}) {
      ExclusionEditor editor(ExclusionEditor::Kind::File);
      editor.setDriveLetter(QStringLiteral("Z:"));
      editor.setFileVolumeIdentity(volume);
      const QStringList aliases = reverse ? QStringList{guid, drive} : QStringList{drive, guid};
      editor.setBaseline({guid}, aliases);
      QCOMPARE(editor.draft().next().size(), 2);
      QCOMPARE(editor.effectiveFileExclusions().size(), 1);
      QVERIFY(editor.draft().equal(drive, guid));
      QCOMPARE(editor.draft().filePattern(guid), QStringLiteral("\\Data\\state.bin"));
      QVERIFY(editor.removeSelection({{drive, std::nullopt}}));
      QCOMPARE(editor.pendingRemoved().size(), 2);
      QVERIFY(editor.pendingRemoved().contains(drive));
      QVERIFY(editor.pendingRemoved().contains(guid));
      QVERIFY(editor.restoreSelection({{guid, std::nullopt}}));
      QVERIFY(editor.pendingRemoved().isEmpty());
      QVERIFY(editor.pendingAdded().isEmpty());
    }
  }
  void fileAddressIdentityDoesNotMergeDifferentVolumesOrPatternSuffixes() {
    using namespace uwf::app;
    const std::string volume = "Volume{11111111-2222-3333-4444-555555555555}";
    const std::string other = "Volume{aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee}";
    const QString drive = QStringLiteral("Z:\\Data");
    const QString guid = QString::fromStdString(uwf::compat::volumePath(volume)) + QStringLiteral("Data");
    const QString otherGuid = QString::fromStdString(uwf::compat::volumePath(other)) + QStringLiteral("Data");
    const QString upper = QStringLiteral("Z:\\Data\\\u00c4");
    const QString lower = QStringLiteral("Z:\\Data\\\u00e4");
    ExclusionEditor editor(ExclusionEditor::Kind::File);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setFileVolumeIdentity(volume);
    editor.setBaseline({}, {drive, guid, otherGuid, drive + u'\\', drive + QStringLiteral("\\.\\"), drive + u' ', upper, lower});
    const auto effective = editor.effectiveFileExclusions();
    QCOMPARE(effective.size(), 7);
    QVERIFY(effective.contains(upper));
    QVERIFY(effective.contains(lower));
    QVERIFY(!editor.draft().equal(drive, otherGuid));
    QVERIFY(!editor.draft().equal(drive, drive + u'\\'));
    QVERIFY(!editor.draft().equal(drive, drive + u' '));
    ExclusionEditor unresolved(ExclusionEditor::Kind::File);
    unresolved.setDriveLetter(QStringLiteral("Z:"));
    unresolved.setBaseline({}, {drive, guid});
    QCOMPARE(unresolved.effectiveFileExclusions().size(), 2);
  }
  void effectiveFileExclusionsKeepCurrentAndDropRemovedNext() {
    using namespace uwf::app;
    const std::string volume = "Volume{11111111-2222-3333-4444-555555555555}";
    const auto current = paths::volumeFilePath({volume, QStringLiteral("\\Data\\Current")});
    const QString alias = QStringLiteral("Z:\\Data\\Current");
    const QString nextOnly = QStringLiteral("Z:\\Data\\Next");
    const QString added = QStringLiteral("Z:\\Data\\Added");
    ExclusionEditor editor(ExclusionEditor::Kind::File);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setFileVolumeIdentity(volume);
    editor.setBaseline({current}, {alias, current, nextOnly});
    QCOMPARE(editor.importRemove(alias), ExclusionEditor::ImportOutcome::Applied);
    QCOMPARE(editor.importRemove(nextOnly.toLower()), ExclusionEditor::ImportOutcome::Applied);
    QCOMPARE(editor.importAdd(added), ExclusionEditor::ImportOutcome::Applied);
    const QStringList expected{current, added};
    QCOMPARE(editor.effectiveFileExclusions(), expected);
    QCOMPARE(editor.pendingRemoved().size(), 3);
  }
  void batchedFileEditsPublishConflictInputsImmediately() {
    using namespace uwf::app;
    FileStagingConflictPolicy conflicts;
    ExclusionEditor editor(ExclusionEditor::Kind::File, &conflicts);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setBaseline({}, {});
    const QString path = QStringLiteral("Z:\\Batch\\Cache");
    const QString child = path + QStringLiteral("\\state.bin");
    QSignalSpy states(&editor, &ExclusionEditor::stateChanged);
    QSignalSpy pending(&editor, &ExclusionEditor::pendingChanged);
    {
      const auto batch = editor.beginBatch();
      QCOMPARE(editor.importAdd(path), ExclusionEditor::ImportOutcome::Applied);
      QCOMPARE(conflicts.conflictingExclusion(child, FileStagingKind::File), std::optional{path});
      QCOMPARE(editor.importRemove(path), ExclusionEditor::ImportOutcome::Applied);
      QVERIFY(!conflicts.conflictingExclusion(child, FileStagingKind::File));
      QCOMPARE(states.size(), 0);
      QCOMPARE(pending.size(), 0);
    }
    QCOMPARE(states.size(), 1);
    QCOMPARE(pending.size(), 1);
  }
  void conflictSnapshotsStayFrozenWhileLiveQueriesFollowTheDraft() {
    using namespace uwf::app;
    FileStagingConflictPolicy conflicts;
    ExclusionEditor editor(ExclusionEditor::Kind::File, &conflicts);
    editor.setDriveLetter(QStringLiteral("Z:"));
    const QString current = QStringLiteral("Z:\\Current");
    const QString next = QStringLiteral("Z:\\Next");
    editor.setBaseline({current}, {next});
    const auto before = conflicts.exclusionChecker();
    const auto batch = editor.beginBatch();
    QCOMPARE(editor.importRemove(next), ExclusionImportOutcome::Applied);
    QCOMPARE(editor.importRemove(current), ExclusionImportOutcome::NoOp);
    const QString added = QStringLiteral("Z:\\Added");
    QCOMPARE(editor.importAdd(added), ExclusionImportOutcome::Applied);
    QVERIFY(!conflicts.conflictingExclusion(next, FileStagingKind::File));
    QCOMPARE(conflicts.conflictingExclusion(current, FileStagingKind::File), std::optional{current});
    QCOMPARE(conflicts.conflictingExclusion(added, FileStagingKind::File), std::optional{added});
    QCOMPARE(before({FileStagingKind::File, next}), std::optional{next});
    QVERIFY(!before({FileStagingKind::File, added}));
    const auto after = conflicts.exclusionChecker();
    editor.resetPending();
    QCOMPARE(conflicts.conflictingExclusion(next, FileStagingKind::File), std::optional{next});
    QVERIFY(!conflicts.conflictingExclusion(added, FileStagingKind::File));
    QVERIFY(!after({FileStagingKind::File, next}));
    QCOMPARE(after({FileStagingKind::File, added}), std::optional{added});
  }
  void identityContextAndBaselineChangesRebuildMembershipForEveryEdit() {
    using namespace uwf::app;
    const std::string volume = "Volume{11111111-2222-3333-4444-555555555555}";
    const QString path = QStringLiteral("Z:\\Data");
    const QString alias = paths::volumeFilePath({volume, QStringLiteral("\\Data")});
    ExclusionEditor editor(ExclusionEditor::Kind::File);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setBaseline({}, {path, alias});
    QCOMPARE(editor.importRemove(path), ExclusionImportOutcome::Applied);
    QCOMPARE(editor.pendingRemoved().size(), 1);
    editor.setFileVolumeIdentity(volume);
    QCOMPARE(editor.importRemove(alias), ExclusionImportOutcome::Applied);
    QCOMPARE(editor.pendingRemoved().size(), 2);
    QCOMPARE(editor.importAdd(alias), ExclusionImportOutcome::Applied);
    QVERIFY(editor.pendingRemoved().isEmpty());
    QCOMPARE(editor.importAdd(path), ExclusionImportOutcome::NoOp);
    QCOMPARE(editor.importRemove(path), ExclusionImportOutcome::Applied);
    QCOMPARE(editor.pendingRemoved().size(), 2);
    editor.resetPending();
    QVERIFY(editor.pendingRemoved().isEmpty());
    QCOMPARE(editor.importAdd(alias), ExclusionImportOutcome::NoOp);
    editor.setBaseline({}, {});
    QCOMPARE(editor.importAdd(alias), ExclusionImportOutcome::Applied);
    QCOMPARE(editor.importAdd(path), ExclusionImportOutcome::NoOp);
    QCOMPARE(editor.importRemove(path), ExclusionImportOutcome::Applied);
    QVERIFY(editor.pendingAdded().isEmpty());
  }
  void providerPatternsReceiveOneExplicitShellWithoutReinterpretingTheirContents() {
    using namespace uwf::app;
    const std::string volume = "Volume{11111111-2222-3333-4444-555555555555}";
    const QString apparentAddress = QString::fromStdString(uwf::compat::volumePath(volume)) + QStringLiteral("Data");
    const QStringList raw{QStringLiteral("\\Data"), apparentAddress, QStringLiteral("Z:\\Data"), QStringLiteral("Data"), QString{}};
    ExclusionEditor editor(ExclusionEditor::Kind::File);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setFileVolumeIdentity(volume);
    editor.setProviderBaseline(raw, raw);
    QCOMPARE(editor.draft().next().size(), raw.size());
    for (const auto& pattern : raw) {
      const auto wrapped = QStringLiteral("Z:") + pattern;
      QVERIFY(editor.draft().next().contains(wrapped));
      QCOMPARE(editor.draft().filePattern(wrapped), pattern);
    }
    QVERIFY(!editor.draft().equal(QStringLiteral("Z:") + apparentAddress, apparentAddress));
    QCOMPARE(editor.effectiveFileExclusions().size(), raw.size());
    QVERIFY(editor.removeSelection({{QStringLiteral("Z:") + apparentAddress, std::nullopt}}));
    QCOMPARE(editor.pendingRemoved(), QStringList{QStringLiteral("Z:") + apparentAddress});
    QCOMPARE(editor.draft().filePattern(editor.pendingRemoved().constFirst()), apparentAddress);
  }
  void currentFileExclusionBlocksCommitThroughAnotherVolumeAddress() {
    using namespace uwf::app;
    const std::string volume = "Volume{11111111-2222-3333-4444-555555555555}";
    const QString guid = QString::fromStdString(uwf::compat::volumePath(volume)) + QStringLiteral("Data\\state.bin");
    ExclusionEditor editor(ExclusionEditor::Kind::File, nullptr, std::make_shared<uwf::test::application::ReadOnlyFileBrowser>());
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setFileVolumeIdentity(volume);
    editor.setBaseline({QStringLiteral("Z:\\Data")}, {});
    editor.setCommitEnabled(true);
    QVERIFY(!editor.canCommitFile(guid));
  }
  void literalProviderPatternsCannotAuthorizeOrProbeAnotherFileObject() {
    using namespace uwf::app;
    auto files = std::make_shared<uwf::test::application::ReadOnlyFileBrowser>();
    ExclusionEditor editor(ExclusionEditor::Kind::File, nullptr, files);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setFileVolumeIdentity("Volume{11111111-2222-3333-4444-555555555555}");
    const QStringList raw{QStringLiteral("Data"), QStringLiteral("Z:\\Data"), QStringLiteral("\\\\?\\Volume{11111111-2222-3333-4444-555555555555}\\Data"),
                          QString{}};
    editor.setProviderBaseline({}, raw);
    editor.setCommitEnabled(true);
    const auto inspect = editor.fileEntryKindOperation();
    QVERIFY(inspect);
    for (const auto& path : editor.draft().next()) {
      QVERIFY(!inspect(path));
      QVERIFY(!editor.canCommitFile(path));
    }
    QVERIFY(files->inspections.isEmpty());
  }
  void currentOnlyGuidEntryUsesTheSharedVolumeContextWhenRestoringNext() {
    using namespace uwf::app;
    const std::string volume = "Volume{11111111-2222-3333-4444-555555555555}";
    const auto guid = paths::volumeFilePath({volume, QStringLiteral("\\Data\\.\\state.bin")});
    ExclusionEditor editor(ExclusionEditor::Kind::File);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setFileVolumeIdentity(volume);
    editor.setBaseline({guid}, {});
    QVERIFY(editor.canRestoreSelection({{guid, std::nullopt}}));
    QVERIFY(editor.restoreSelection({{guid, std::nullopt}}));
    QCOMPARE(editor.pendingAdded(), QStringList{guid});
    QCOMPARE(editor.draft().filePattern(guid), QStringLiteral("\\Data\\.\\state.bin"));

    // Resolving a same-volume alias does not bypass existing exclusion bans.
    editor.setSystemDriveLetter(QStringLiteral("Z:"));
    const auto forbidden = paths::volumeFilePath({volume, QStringLiteral("\\Windows")});
    editor.setBaseline({forbidden}, {});
    const auto rejected = editor.restorationResult({{forbidden, std::nullopt}});
    QCOMPARE(rejected.outcome, ExclusionImportOutcome::RejectedForbidden);
    QVERIFY(!editor.restoreSelection({{forbidden, std::nullopt}}));
    QVERIFY(editor.pendingAdded().isEmpty());
  }
  void foreignAndUnresolvedAddressesCannotBeReboundWhenExtractingPatterns() {
    using namespace uwf::app;
    const std::string volume = "Volume{11111111-2222-3333-4444-555555555555}";
    const std::string other = "Volume{aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee}";
    const auto own = paths::volumeFilePath({volume, QStringLiteral("\\Data")});
    const auto foreign = paths::volumeFilePath({other, QStringLiteral("\\Data")});
    ExclusionEditor editor(ExclusionEditor::Kind::File);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setFileVolumeIdentity(volume);
    QCOMPARE(editor.draft().filePattern(own), QStringLiteral("\\Data"));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)editor.draft().filePattern(foreign));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)editor.draft().filePattern(QStringLiteral("Y:\\Data")));
    QCOMPARE(editor.importAdd(foreign), ExclusionImportOutcome::RejectedNotOnVolume);

    ExclusionEditor legacy(ExclusionEditor::Kind::File);
    legacy.setDriveLetter(QStringLiteral("Z:"));
    QCOMPARE(legacy.draft().filePattern(QStringLiteral("z:\\Data")), QStringLiteral("\\Data"));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)legacy.draft().filePattern(own));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)legacy.draft().filePattern(QStringLiteral("Y:\\Data")));
    QCOMPARE(legacy.importAdd(own), ExclusionImportOutcome::RejectedNotOnVolume);
    QCOMPARE(legacy.importAdd(QStringLiteral("Z:\\Data")), ExclusionImportOutcome::Applied);
  }

  void newExclusionInputsPreserveUnicodeIdentity_data() {
    QTest::addColumn<QString>("suffix");
    QTest::addColumn<bool>("representable");
    QTest::newRow("supplementary-pair") << QStringLiteral("数据\U0001F680") << true;
    QTest::newRow("orphan-high") << QString(QChar(0xd800)) << false;
    QTest::newRow("orphan-low") << QString(QChar(0xdc00)) << false;
    QTest::newRow("high-before-bmp") << (QString(QChar(0xd800)) + u'A') << false;
  }
  void newExclusionInputsPreserveUnicodeIdentity() {
    QFETCH(QString, suffix);
    QFETCH(bool, representable);
    using Editor = uwf::app::ExclusionEditor;
    for (const auto kind : {Editor::Kind::File, Editor::Kind::Registry}) {
      const bool file = kind == Editor::Kind::File;
      const auto path = (file ? QStringLiteral("Z:\\Data\\") : QStringLiteral("HKLM\\Software\\")) + suffix;
      const auto reason =
          file ? uwf::app::FileExclusionRules::rejectionReason(path, QStringLiteral("Z:"), {}) : uwf::app::RegistryExclusionRules::rejectionReason(path);
      QCOMPARE(reason.isEmpty(), representable);
      Editor editor(kind);
      editor.setDriveLetter(QStringLiteral("Z:"));
      const auto result = editor.addSelectedPath(path);
      QCOMPARE(result.path, path);
      QCOMPARE(result.outcome, representable ? Editor::ImportOutcome::Applied : Editor::ImportOutcome::RejectedForbidden);
      QCOMPARE(editor.pendingAdded(), representable ? QStringList{path} : QStringList{});
    }
  }

  void qualifyingProviderPatternsPreservesTheirSpelling_data() {
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("qualified");
    const QStringList patterns{QStringLiteral("\\Data\\.\\Cache\\"), QStringLiteral("/Data/Cache"), QStringLiteral("\\\\?\\Entry"),
                               QStringLiteral("C:\\Data\\Cache "),   QStringLiteral("Data\\Cache"), QString{}};
    for (qsizetype index = 0; index < patterns.size(); ++index) {
      const auto& pattern = patterns[index];
      const auto qualified = index < 2 ? QStringLiteral("Z:") + pattern : pattern;
      QTest::newRow(qUtf8Printable(QString::number(index))) << pattern << qualified;
    }
  }
  void qualifyingProviderPatternsPreservesTheirSpelling() {
    QFETCH(QString, pattern);
    QFETCH(QString, qualified);
    uwf::app::ExclusionEditor editor(uwf::app::ExclusionEditor::Kind::File);
    editor.setDriveLetter(QStringLiteral("Z:"));
    QCOMPARE(editor.absoluteFilePath(pattern), qualified);
    QCOMPARE(uwf::app::qualifyFileExclusionPath(pattern, QStringLiteral("Z:")), qualified);
    QCOMPARE(uwf::app::qualifyFileExclusionPath(pattern, {}), pattern);
  }

  void restorationChecksCrossFeatureInvariantsForTheWholeSelection_data() {
    QTest::addColumn<bool>("undo");
    QTest::addColumn<bool>("directory");
    QTest::addColumn<bool>("overlap");
    for (const bool undo : {false, true})
      for (const bool directory : {false, true})
        for (const bool overlap : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("undo-%1-directory-%2-overlap-%3").arg(undo).arg(directory).arg(overlap)))
              << undo << directory << overlap;
  }
  void restorationChecksCrossFeatureInvariantsForTheWholeSelection() {
    QFETCH(bool, undo);
    QFETCH(bool, directory);
    QFETCH(bool, overlap);
    using namespace uwf::app;
    const QString pattern = QStringLiteral("Z:\\Data\\Pattern");
    const QString unrelated = QStringLiteral("Z:\\Other\\Pattern");
    const QString staged = overlap ? pattern + QStringLiteral("\\child.bin") : QStringLiteral("Z:\\Database\\child.bin");
    FileStagingConflictPolicy conflicts;
    ExclusionEditor editor(ExclusionEditor::Kind::File, &conflicts);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setBaseline(undo ? QStringList{} : QStringList{pattern, unrelated}, undo ? QStringList{pattern, unrelated} : QStringList{});
    if (undo) QVERIFY(editor.removeSelection({{pattern, std::nullopt}, {unrelated, std::nullopt}}));
    conflicts.setStagedEntries({{directory ? FileStagingKind::Directory : FileStagingKind::File, staged}});
    const auto beforeAdded = editor.pendingAdded();
    const auto beforeRemoved = editor.pendingRemoved();
    QSignalSpy changed(&editor, &ExclusionEditor::pendingChanged);
    const QVector<ExclusionEditor::Selection> selection{{unrelated, std::nullopt}, {pattern, std::nullopt}};
    QCOMPARE(editor.canRestoreSelection(selection), !overlap);
    QCOMPARE(editor.restoreSelection(selection), !overlap);
    if (overlap) {
      const auto rejected = editor.restorationResult(selection);
      QCOMPARE(rejected.outcome, ExclusionImportOutcome::RejectedConflict);
      QCOMPARE(rejected.conflict, staged);
      QCOMPARE(editor.pendingAdded(), beforeAdded);
      QCOMPARE(editor.pendingRemoved(), beforeRemoved);
      QCOMPARE(changed.size(), 0);
    } else {
      QVERIFY(editor.pendingRemoved().isEmpty());
      QCOMPARE(editor.pendingAdded().size(), undo ? 0 : 2);
      QCOMPARE(changed.size(), 1);
    }
  }

  void readOnlyAdmissionCoversPickerImportAndSelection_data() {
    QTest::addColumn<bool>("registry");
    QTest::newRow("file") << false;
    QTest::newRow("registry") << true;
  }
  void readOnlyAdmissionCoversPickerImportAndSelection() {
    QFETCH(bool, registry);
    using Editor = uwf::app::ExclusionEditor;
    Editor editor(registry ? Editor::Kind::Registry : Editor::Kind::File);
    editor.setDriveLetter(QStringLiteral("Z:"));
    const auto prefix = registry ? QStringLiteral("HKLM\\SOFTWARE\\") : QStringLiteral("Z:\\Data\\");
    const auto removed = prefix + QStringLiteral("Removed");
    const auto retained = prefix + QStringLiteral("Retained");
    const auto added = prefix + QStringLiteral("New");
    editor.setBaseline({removed, retained}, {removed, retained});
    editor.setPersistBaseline(false, false, false, false);
    QCOMPARE(editor.importRemove(removed), Editor::ImportOutcome::Applied);
    editor.setReadOnly(true);
    QSignalSpy states(&editor, &Editor::stateChanged);
    QSignalSpy pending(&editor, &Editor::pendingChanged);
    QCOMPARE(editor.importAdd(added), Editor::ImportOutcome::RejectedReadOnly);
    QCOMPARE(editor.importRemove(retained), Editor::ImportOutcome::RejectedReadOnly);
    const auto selected = editor.addSelectedPath(added);
    QCOMPARE(selected.outcome, Editor::ImportOutcome::RejectedReadOnly);
    QVERIFY(!selected.reason.isEmpty());
    QVERIFY(!editor.removeSelection({{retained, std::nullopt}}));
    QVERIFY(!editor.restoreSelection({{removed, std::nullopt}}));
    editor.enablePersistence(Editor::PersistenceItem::DomainSecretKey);
    editor.enablePersistence(Editor::PersistenceItem::Tscal);
    QCOMPARE(editor.pendingRemoved(), QStringList{removed});
    QVERIFY(editor.pendingAdded().isEmpty());
    QVERIFY(!editor.pendingPersistDomainSecretKey());
    QVERIFY(!editor.pendingPersistTSCAL());
    QCOMPARE(states.size(), 0);
    QCOMPARE(pending.size(), 0);
    editor.setReadOnly(false);
    QCOMPARE(editor.importAdd(added), Editor::ImportOutcome::Applied);
  }

  void restoringAConfiguredFileEntryDoesNotRevalidateAddSyntax_data() {
    QTest::addColumn<QString>("path");
    QTest::newRow("invalid-character") << QStringLiteral("Z:\\Data\\Old?Pattern");
    QTest::newRow("forward-slash") << QStringLiteral("Z:\\Data/OldPattern");
    QTest::newRow("control-character") << QStringLiteral("Z:\\Data\\Before") + QChar(0x1e) + QStringLiteral("After");
    QTest::newRow("ordinary-length-exceeded") << QStringLiteral("Z:\\") + QString(260, u'X');
  }
  void restoringAConfiguredFileEntryDoesNotRevalidateAddSyntax() {
    QFETCH(QString, path);
    using Editor = uwf::app::ExclusionEditor;
    Editor editor(Editor::Kind::File);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setBaseline({}, {path});
    QVERIFY(editor.removeSelection({{path, std::nullopt}}));
    QCOMPARE(editor.pendingRemoved(), QStringList{path});
    QVERIFY(editor.canRestoreSelection({{path, std::nullopt}}));
    QVERIFY(editor.restoreSelection({{path, std::nullopt}}));
    QVERIFY(editor.pendingRemoved().isEmpty());
    QVERIFY(editor.pendingAdded().isEmpty());
    QVERIFY(editor.importAdd(path) == Editor::ImportOutcome::RejectedForbidden);
    QVERIFY(editor.pendingAdded().isEmpty());
  }

  void fileTargetAdmissionDistinguishesUndoFromNewAdd_data() {
    QTest::addColumn<bool>("available");
    QTest::addColumn<bool>("undoNextRemoval");
    for (const bool available : {false, true})
      for (const bool undo : {false, true})
        QTest::newRow(available ? (undo ? "available/undo" : "available/current-only") : (undo ? "unavailable/undo" : "unavailable/current-only"))
            << available << undo;
  }
  void fileTargetAdmissionDistinguishesUndoFromNewAdd() {
    QFETCH(bool, available);
    QFETCH(bool, undoNextRemoval);
    using Editor = uwf::app::ExclusionEditor;
    Editor editor(Editor::Kind::File);
    const auto path = QStringLiteral("Z:\\Data\\original.bin");
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setBaseline({path}, undoNextRemoval ? QStringList{path} : QStringList{});
    if (undoNextRemoval) QVERIFY(editor.removeSelection({{path, std::nullopt}}));
    editor.setFileAddTargetAvailable(available);
    const bool restorable = available || undoNextRemoval;
    QCOMPARE(editor.canRestoreSelection({{path, std::nullopt}}), restorable);
    QCOMPARE(editor.restoreSelection({{path, std::nullopt}}), restorable);
    QVERIFY(editor.pendingRemoved().isEmpty());
    QCOMPARE(editor.pendingAdded(), available && !undoNextRemoval ? QStringList{path} : QStringList{});
    QCOMPARE(editor.canAdd(), available);
    const auto addition = editor.addSelectedPath(QStringLiteral("Z:\\Data\\new.bin"));
    QCOMPARE(addition.outcome, available ? Editor::ImportOutcome::Applied : Editor::ImportOutcome::RejectedMissingVolume);
    if (!available) QVERIFY(!addition.reason.isEmpty());
  }

  void registryExclusionInputUsesNativeTargetsAndDedicatedPersistence_data() {
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("reason");
    for (const auto& key : {QStringLiteral("HKLM\\HARDWARE\\Vendor"), QStringLiteral("HKLM\\BCD00000000"), QStringLiteral("HKLM\\Custom\\Vendor"),
                            QStringLiteral("hkey_local_machine\\system\\CurrentControlSet\\Services\\Demo"), QStringLiteral("HKLM\\SOFTWARE\\Vendor "),
                            QStringLiteral("HKLM\\SYSTEM ")})
      QTest::newRow(qUtf8Printable(key)) << key << QString{};
    for (const auto& key :
         {QStringLiteral("HKLM"), QStringLiteral("HKLM\\SYSTEM"), QStringLiteral("HKLM\\SYSTEM\\CurrentControlSet"), QStringLiteral("HKLM\\SOFTWARE"),
          QStringLiteral("HKLM\\SAM"), QStringLiteral("HKLM\\SECURITY"), QStringLiteral("HKLM\\COMPONENTS")})
      QTest::newRow(qUtf8Printable(key)) << key << QStringLiteral("This key cannot be excluded itself. Select one of its subkeys.");
    QTest::newRow("literal-slash") << QStringLiteral("HKLM\\SOFTWARE\\Slash/Name") << QString{};
    QTest::newRow("literal-control") << QStringLiteral("HKLM\\SOFTWARE\\Control") + QChar(1) + QStringLiteral("Name") << QString{};
    QTest::newRow("literal-tab") << QStringLiteral("HKLM\\SOFTWARE\\Tab\tName") << QString{};
    QTest::newRow("literal-newline") << QStringLiteral("HKLM\\SOFTWARE\\Line\nName") << QString{};
    QTest::newRow("repeated-separators") << QStringLiteral("HKLM\\\\SOFTWARE\\\\Vendor\\\\") << QString{};
    QTest::newRow("component-limit") << QStringLiteral("HKLM\\SOFTWARE\\") + QString(255, u'X') << QString{};
    QTest::newRow("component-too-long") << QStringLiteral("HKLM\\SOFTWARE\\") + QString(256, u'X') << QStringLiteral("A registry key component is too long.");
    QTest::newRow("nesting-limit") << QStringLiteral("HKLM\\") + QStringLiteral("X\\").repeated(511) + u'X' << QString{};
    QTest::newRow("nesting-too-deep") << QStringLiteral("HKLM\\") + QStringLiteral("X\\").repeated(512) + u'X'
                                      << QStringLiteral("The registry path exceeds UWF's nesting limit.");
    QTest::newRow("embedded-null") << QStringLiteral("HKLM\\SOFTWARE\\Before") + QChar(0) + QStringLiteral("After")
                                   << QStringLiteral("Registry paths cannot contain null characters.");
    QTest::newRow("other-hive") << QStringLiteral("HKCU\\Software\\Vendor")
                                << QStringLiteral("Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).");
    QTest::newRow("machine-account") << QStringLiteral("HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC")
                                     << QStringLiteral("Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).");
  }
  void registryExclusionInputUsesNativeTargetsAndDedicatedPersistence() {
    QFETCH(QString, key);
    QFETCH(QString, reason);
    using Editor = uwf::app::ExclusionEditor;
    using Outcome = Editor::ImportOutcome;
    QCOMPARE(uwf::app::RegistryExclusionRules::rejectionReason(key), reason);
    Editor editor(Editor::Kind::Registry);
    const auto result = editor.addSelectedPath(key);
    QCOMPARE(result.path, key);
    QCOMPARE(result.outcome, reason.isEmpty() ? Outcome::Applied : Outcome::RejectedForbidden);
    QCOMPARE(result.reason, reason);
    QCOMPARE(editor.importAdd(result.path), reason.isEmpty() ? Outcome::NoOp : Outcome::RejectedForbidden);
    QCOMPARE(editor.pendingAdded().size(), reason.isEmpty() ? 1 : 0);
  }
  void configuredNonAsciiEntriesKeepIndependentDrafts_data() {
    QTest::addColumn<bool>("registry");
    QTest::newRow("file") << false;
    QTest::newRow("registry") << true;
  }
  void configuredNonAsciiEntriesKeepIndependentDrafts() {
    QFETCH(bool, registry);
    using Editor = uwf::app::ExclusionEditor;
    using Outcome = Editor::ImportOutcome;
    Editor editor(registry ? Editor::Kind::Registry : Editor::Kind::File);
    editor.setDriveLetter(QStringLiteral("Z:"));
    const auto root = registry ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Identity\\") : QStringLiteral("Z:\\Data\\");
    const auto upper = root + QChar(0x00c4);
    const auto lower = root + QChar(0x00e4);
    editor.setBaseline({upper, lower, root.toLower() + QChar(0x00c4)}, {upper});
    QCOMPARE(editor.draft().current().size(), 2);
    QCOMPARE(editor.importAdd(lower), Outcome::Applied);
    QCOMPARE(editor.importAdd(lower), Outcome::NoOp);
    QCOMPARE(editor.pendingAdded(), QStringList{lower});
    QCOMPARE(editor.importRemove(upper), Outcome::Applied);
    QCOMPARE(editor.pendingRemoved(), QStringList{upper});
    QCOMPARE(editor.importRemove(lower), Outcome::Applied);
    QVERIFY(editor.pendingAdded().isEmpty());
    QCOMPARE(editor.pendingRemoved(), QStringList{upper});
  }
  void restorationDistinguishesExistingEntriesFromNewAdditions_data() {
    QTest::addColumn<QString>("key");
    QTest::addColumn<bool>("nextEntry");
    QTest::addColumn<bool>("newEntryAllowed");
    for (const bool nextEntry : {false, true}) {
      QTest::newRow(nextEntry ? "machine-secret/next" : "machine-secret/current-only")
          << QStringLiteral("HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC") << nextEntry << false;
      QTest::newRow(nextEntry ? "reserved-root-with-tail/next" : "reserved-root-with-tail/current-only")
          << QStringLiteral("HKLM\\SYSTEM\\") << nextEntry << true;
      QTest::newRow(nextEntry ? "literal-space/next" : "literal-space/current-only") << QStringLiteral("HKLM\\SYSTEM ") << nextEntry << true;
      QTest::newRow(nextEntry ? "ordinary/next" : "ordinary/current-only") << QStringLiteral("HKLM\\SOFTWARE\\Restoration") << nextEntry << true;
    }
  }
  void restorationDistinguishesExistingEntriesFromNewAdditions() {
    QFETCH(QString, key);
    QFETCH(bool, nextEntry);
    QFETCH(bool, newEntryAllowed);
    using Editor = uwf::app::ExclusionEditor;
    Editor editor(Editor::Kind::Registry);
    editor.setBaseline({key}, nextEntry ? QStringList{key} : QStringList{});
    if (nextEntry) {
      QVERIFY(editor.removeSelection({{key, std::nullopt}}));
      QCOMPARE(editor.pendingRemoved(), QStringList{key});
    }
    QCOMPARE(editor.restoreSelection({{key, std::nullopt}}), nextEntry || newEntryAllowed);
    QVERIFY(editor.pendingRemoved().isEmpty());
    QCOMPARE(editor.pendingAdded(), !nextEntry && newEntryAllowed ? QStringList{key} : QStringList{});
  }
  void selectedFileSpellingUsesConfiguredComparison_data() {
    QTest::addColumn<QString>("alias");
    QTest::addColumn<bool>("baselineFirst");
    const QStringList aliases{QStringLiteral("z:/DATA/state.bin"), QStringLiteral("Z:\\DATA\\state.bin")};
    for (qsizetype index = 0; index < aliases.size(); ++index)
      for (const bool baselineFirst : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("alias-%1-baseline-first-%2").arg(index).arg(baselineFirst))) << aliases.at(index) << baselineFirst;
  }
  void selectedFileSpellingUsesConfiguredComparison() {
    QFETCH(QString, alias);
    QFETCH(bool, baselineFirst);
    using Editor = uwf::app::ExclusionEditor;
    using Outcome = Editor::ImportOutcome;
    Editor editor(Editor::Kind::File);
    if (!baselineFirst) editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setBaseline({QStringLiteral("\\Data\\state.bin"), QStringLiteral("Z:\\Data\\state.bin")}, {QStringLiteral("\\Data\\state.bin")});
    if (baselineFirst) editor.setDriveLetter(QStringLiteral("Z:"));
    QCOMPARE(editor.draft().current(), QStringList{QStringLiteral("Z:\\Data\\state.bin")});
    QCOMPARE(editor.draft().next(), editor.draft().current());
    QCOMPARE(editor.addSelectedPath(alias).outcome, Outcome::NoOp);
    const auto configuredPath = QStringLiteral("Z:\\Data\\state.bin");
    QCOMPARE(editor.importAdd(configuredPath), Outcome::NoOp);
    QVERIFY(editor.pendingAdded().isEmpty());

    QVERIFY(editor.removeSelection({{configuredPath, std::nullopt}}));
    QCOMPARE(editor.pendingRemoved().size(), 1);
    QVERIFY(editor.restoreSelection({{configuredPath, std::nullopt}}));
    QVERIFY(editor.pendingRemoved().isEmpty());
    QCOMPARE(editor.importRemove(configuredPath), Outcome::Applied);
    QCOMPARE(editor.importRemove(configuredPath), Outcome::NoOp);
    QCOMPARE(editor.importAdd(configuredPath), Outcome::Applied);
    QVERIFY(editor.pendingAdded().isEmpty());
    QVERIFY(editor.pendingRemoved().isEmpty());
  }
  void driveBindingQualifiesPendingFilePaths() {
    using Editor = uwf::app::ExclusionEditor;
    using Outcome = Editor::ImportOutcome;
    Editor editor(Editor::Kind::File);
    editor.setBaseline({}, {QStringLiteral("\\Data\\Old.bin")});
    QCOMPARE(editor.importRemove(QStringLiteral("\\Data\\Old.bin")), Outcome::Applied);
    QCOMPARE(editor.importAdd(QStringLiteral("\\Data\\New.bin")), Outcome::Applied);
    const auto provisional = editor.draft().fileAddress(QStringLiteral("\\Data\\New.bin"));
    QVERIFY(provisional);
    QVERIFY(provisional->volume.empty());
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)editor.draft().filePattern(QStringLiteral("\\Data\\New.bin")));
    QCOMPARE(editor.importAdd(QStringLiteral("Y:\\Data\\New.bin")), Outcome::RejectedNotOnVolume);
    QCOMPARE(editor.importAdd(QStringLiteral("\\\\?\\Volume{11111111-2222-3333-4444-555555555555}\\Data\\New.bin")), Outcome::RejectedNotOnVolume);
    editor.setDriveLetter(QStringLiteral("Z:"));
    QCOMPARE(editor.pendingRemoved(), QStringList{QStringLiteral("Z:\\Data\\Old.bin")});
    QCOMPARE(editor.pendingAdded(), QStringList{QStringLiteral("Z:\\Data\\New.bin")});
    QCOMPARE(editor.importAdd(QStringLiteral("Z:\\Data\\Old.bin")), Outcome::Applied);
    QCOMPARE(editor.importRemove(QStringLiteral("Z:\\Data\\New.bin")), Outcome::Applied);
    QVERIFY(editor.pendingAdded().isEmpty());
    QVERIFY(editor.pendingRemoved().isEmpty());
  }
  void storedFilePatternsAreNotRewrittenAsSelectedObjects_data() {
    QTest::addColumn<QString>("stored");
    QTest::newRow("volume-relative-mixed-separators") << QStringLiteral("\\Data/Cache");
    QTest::newRow("absolute-forward-separators") << QStringLiteral("Z:/Data/Cache");
  }
  void storedFilePatternsAreNotRewrittenAsSelectedObjects() {
    QFETCH(QString, stored);
    using Editor = uwf::app::ExclusionEditor;
    using Outcome = Editor::ImportOutcome;
    uwf::app::FileStagingConflictPolicy conflicts;
    Editor editor(Editor::Kind::File, &conflicts);
    editor.setDriveLetter(QStringLiteral("Z:"));
    editor.setBaseline({stored}, {stored});
    const auto qualified = stored.startsWith(u'\\') ? QStringLiteral("Z:") + stored : stored;
    QCOMPARE(editor.draft().current(), QStringList{qualified});
    QCOMPARE(editor.draft().next(), QStringList{qualified});
    QCOMPARE(editor.effectiveFileExclusions(), QStringList{qualified});
    QVERIFY(!conflicts.conflictingExclusion(QStringLiteral("Z:\\Data\\Cache\\Child"), uwf::app::FileStagingKind::File));
    QVERIFY(editor.removeSelection({{qualified, std::nullopt}}));
    QCOMPARE(editor.pendingRemoved(), QStringList{qualified});
    QCOMPARE(editor.importRemove(qualified), Outcome::NoOp);
    QVERIFY(editor.restoreSelection({{qualified, std::nullopt}}));
    QVERIFY(editor.pendingRemoved().isEmpty());
    QCOMPARE(editor.importAdd(qualified), Outcome::RejectedForbidden);
    QCOMPARE(uwf::app::FileExclusionRules::rejectionReason(qualified, QStringLiteral("Z:"), {}), QStringLiteral("UWF exclusion paths must use backslashes."));
    const auto selected = editor.addSelectedPath(QStringLiteral("Z:/Data/Cache"));
    QCOMPARE(selected.outcome, Outcome::Applied);
    QCOMPARE(selected.path, QStringLiteral("Z:\\Data\\Cache"));
    QCOMPARE(editor.pendingAdded(), QStringList{QStringLiteral("Z:\\Data\\Cache")});
    QCOMPARE(editor.draft().current(), QStringList{qualified});
    QCOMPARE(editor.draft().next(), QStringList{qualified});
  }
  void forbiddenFilePathsRemainForbiddenThroughAliases_data() {
    QTest::addColumn<QString>("drive");
    QTest::addColumn<QString>("path");
    const QString system = QStringLiteral("R:");
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
    editor.setSystemDriveLetter(QStringLiteral("R:"));
    QVERIFY(!uwf::app::FileExclusionRules::rejectionReason(path, drive, QStringLiteral("R:")).isEmpty());
    QCOMPARE(editor.addSelectedPath(path).outcome, Editor::ImportOutcome::RejectedForbidden);
    QCOMPARE(editor.importAdd(path), Editor::ImportOutcome::RejectedForbidden);
    QVERIFY(editor.pendingAdded().isEmpty());
  }
  void systemDirectoryPolicyUsesTheSuppliedSystemDrive() {
    using Rules = uwf::app::FileExclusionRules;
    QVERIFY(!Rules::rejectionReason(QStringLiteral("R:\\Windows"), QStringLiteral("R:"), QStringLiteral("r:")).isEmpty());
    QVERIFY(Rules::rejectionReason(QStringLiteral("Z:\\Windows"), QStringLiteral("Z:"), QStringLiteral("R:")).isEmpty());
    QVERIFY(Rules::rejectionReason(QStringLiteral("R:\\Windows"), QStringLiteral("R:"), {}).isEmpty());
  }
  void forbiddenFileNamesDoNotConsumeLiteralSuffixes_data() {
    QTest::addColumn<QString>("path");
    for (const auto& name : {QStringLiteral("Windows"), QStringLiteral("Windows\\System32"), QStringLiteral("Windows\\System32\\Drivers"),
                             QStringLiteral("pagefile.sys"), QStringLiteral("swapfile.sys"), QStringLiteral("hiberfil.sys"),
                             QStringLiteral("Windows\\System32\\config\\SYSTEM"), QStringLiteral("Users\\Alice\\NTUSER.DAT")})
      QTest::newRow(qUtf8Printable(name)) << QStringLiteral("R:\\") + name;
  }
  void forbiddenFileNamesDoNotConsumeLiteralSuffixes() {
    QFETCH(QString, path);
    using Editor = uwf::app::ExclusionEditor;
    Editor editor(Editor::Kind::File);
    editor.setDriveLetter(QStringLiteral("R:"));
    editor.setSystemDriveLetter(QStringLiteral("R:"));
    QVERIFY(!uwf::app::FileExclusionRules::rejectionReason(path, QStringLiteral("R:"), QStringLiteral("R:")).isEmpty());
    const auto selected = path + u' ';
    QVERIFY(uwf::app::FileExclusionRules::rejectionReason(selected, QStringLiteral("R:"), QStringLiteral("R:")).isEmpty());
    const auto result = editor.addSelectedPath(selected);
    QCOMPARE(result.outcome, Editor::ImportOutcome::Applied);
    QCOMPARE(result.path, selected);
    QCOMPARE(editor.pendingAdded(), QStringList{selected});
  }
  void fileMetadataUsesTheExactInjectedObject_data() {
    QTest::addColumn<QString>("suffix");
    QTest::addColumn<bool>("directory");
    QTest::addColumn<int>("observation");
    const QStringList suffixes{QString{}, QStringLiteral(" "), QStringLiteral(".")};
    for (qsizetype suffix = 0; suffix < suffixes.size(); ++suffix)
      for (const bool directory : {false, true})
        for (const int observation : {0, 1, 2, 3})
          QTest::newRow(qPrintable(QStringLiteral("suffix-%1-directory-%2-observation-%3").arg(suffix).arg(directory).arg(observation)))
              << suffixes.at(suffix) << directory << observation;
  }
  void fileMetadataUsesTheExactInjectedObject() {
    QFETCH(QString, suffix);
    QFETCH(bool, directory);
    QFETCH(int, observation);  // Existing, missing, failed read, absent dependency.
    using Editor = uwf::app::ExclusionEditor;
    const auto base = QStringLiteral("Z:\\Data\\Entry.bin");
    const auto target = base + suffix;
    const auto files = std::make_shared<uwf::test::application::ReadOnlyFileBrowser>();
    files->entries[base] = {.path = base, .name = {}, .directory = !directory};
    files->entries[target] = {.path = target, .name = {}, .directory = directory, .size = 0, .modified = {}, .exists = observation == 0};
    if (observation == 2) files->readFailures.insert(target);
    Editor editor(Editor::Kind::File, nullptr, observation == 3 ? nullptr : files);
    editor.setCommitEnabled(true);
    const auto kind = directory ? Editor::FileEntryKind::Directory : Editor::FileEntryKind::File;
    QCOMPARE(editor.canCommitFile(target), observation != 3);
    QVERIFY(files->inspections.isEmpty());
    const auto read = editor.fileEntryKindOperation();
    QCOMPARE(read ? read(target) : std::nullopt, observation == 0 ? std::optional{kind} : std::nullopt);
    QVERIFY(std::ranges::all_of(files->inspections, [&](const auto& path) { return path == target; }));
    QCOMPARE(files->inspections.size(), observation == 3 ? 0 : 1);
  }
  void fileCommitEligibilityReflectsCurrentProtectionAndFileFacts_data() {
    QTest::addColumn<QString>("relativePath");
    QTest::addColumn<QString>("currentExclusion");
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("exists");
    QTest::addColumn<bool>("directory");
    QTest::addColumn<bool>("eligible");
    for (const bool directory : {false, true}) {
      const auto path = directory ? QStringLiteral("Data") : QStringLiteral("Data/data.txt");
      const auto kind = directory ? QStringLiteral("directory") : QStringLiteral("file");
      QTest::newRow(qUtf8Printable(kind + QStringLiteral("-enabled"))) << path << QString{} << true << true << directory << true;
      QTest::newRow(qUtf8Printable(kind + QStringLiteral("-disabled"))) << path << QString{} << false << true << directory << false;
      QTest::newRow(qUtf8Printable(kind + QStringLiteral("-missing"))) << path << QString{} << true << false << directory << false;
    }
    for (int trailing = 0; trailing <= 2; ++trailing) {
      const auto separators = QString(trailing, u'\\');
      QTest::newRow(qPrintable(QStringLiteral("exact-pattern-tail-%1").arg(trailing)))
          << QStringLiteral("Cache/data.txt") << QStringLiteral("Cache/data.txt") + separators << true << true << false << (trailing != 0);
      QTest::newRow(qPrintable(QStringLiteral("ancestor-pattern-tail-%1").arg(trailing)))
          << QStringLiteral("Cache/data.txt") << QStringLiteral("Cache") + separators << true << true << false << (trailing == 2);
      QTest::newRow(qPrintable(QStringLiteral("sibling-pattern-tail-%1").arg(trailing)))
          << QStringLiteral("Cache/data.txt") << QStringLiteral("CacheSibling") + separators << true << true << false << true;
    }
    QTest::newRow("literal-dot-pattern") << QStringLiteral("Cache/data.txt") << QStringLiteral("Scratch/../Cache") << true << true << false << true;
    struct Spelling {
      const char* name;
      QString actual;
      QString configured;
      bool managementMatches;
    };
    const std::array spellings{Spelling{"ascii", QStringLiteral("Entry"), QStringLiteral("eNTRY"), true},
                               Spelling{"latin-case", QString(QChar(0x00c4)), QString(QChar(0x00e4)), false},
                               Spelling{"different-utf8-length", QString(QChar(0x023a)), QString(QChar(0x2c65)), false},
                               Spelling{"micro-sign-is-not-greek-mu", QString(QChar(0x00b5)), QString(QChar(0x03bc)), false},
                               Spelling{"sharp-s-is-not-capital-sharp-s", QString(QChar(0x00df)), QString(QChar(0x1e9e)), false}};
    for (const auto& spelling : spellings) {
      const auto path = spelling.actual + QStringLiteral("/data.txt");
      const auto tag = QString::fromLatin1(spelling.name);
      QTest::newRow(qUtf8Printable(tag + QStringLiteral("-exact")))
          << path << spelling.configured + QStringLiteral("/data.txt") << true << true << false << !spelling.managementMatches;
      QTest::newRow(qUtf8Printable(tag + QStringLiteral("-ancestor"))) << path << spelling.configured << true << true << false << !spelling.managementMatches;
      QTest::newRow(qUtf8Printable(tag + QStringLiteral("-sibling")))
          << path << spelling.configured + QStringLiteral("-sibling") << true << true << false << true;
    }
  }
  void fileCommitEligibilityReflectsCurrentProtectionAndFileFacts() {
    QFETCH(QString, relativePath);
    QFETCH(QString, currentExclusion);
    QFETCH(bool, enabled);
    QFETCH(bool, exists);
    QFETCH(bool, directory);
    QFETCH(bool, eligible);
    const auto root = QStringLiteral("Z:\\Fixture");
    const auto target = root + u'\\' + QString(relativePath).replace(u'/', u'\\');
    const auto files = std::make_shared<uwf::test::application::ReadOnlyFileBrowser>();
    files->entries[target] = {.path = target, .name = {}, .directory = directory, .size = 0, .modified = {}, .exists = exists};
    uwf::app::ExclusionEditor editor(uwf::app::ExclusionEditor::Kind::File, nullptr, files);
    editor.setCommitEnabled(enabled);
    const auto pattern = root + u'\\' + QString(currentExclusion).replace(u'/', u'\\');
    editor.setBaseline(currentExclusion.isEmpty() ? QStringList{} : QStringList{pattern}, {target});
    if (!currentExclusion.isEmpty()) QCOMPARE(editor.draft().current(), QStringList{pattern});
    const auto kind = directory ? uwf::app::ExclusionEditor::FileEntryKind::Directory : uwf::app::ExclusionEditor::FileEntryKind::File;
    const auto read = editor.fileEntryKindOperation();
    const auto observed = read ? read(target) : std::nullopt;
    const auto actual = editor.canCommitFile(target) ? observed : std::nullopt;
    QCOMPARE(actual, eligible ? std::optional{kind} : std::nullopt);
  }
};
}  // namespace
QTEST_APPLESS_MAIN(ExclusionEditingTests)
#include "ExclusionEditingTests.moc"
