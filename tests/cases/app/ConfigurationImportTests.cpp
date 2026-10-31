// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <limits>
#include <stdexcept>

#include "app/configuration/ConfigurationImporter.h"
#include "app/configuration/GlobalConfigurationEditor.h"
#include "app/configuration/VolumeConfigurationEditor.h"

namespace {
using namespace uwf;

QList<app::UwfmgrCommand> commands(const std::string& text) {
  const auto parsed = app::parseUwfmgrText(text);
  return {parsed.begin(), parsed.end()};
}

app::DriveResolver noDriveResolution() {
  return [](const std::string&) -> std::string { throw std::logic_error("unexpected drive resolution"); };
}

core::SessionSnapshot baseline(uint32_t warning = 1000, uint32_t critical = 3000) {
  core::SessionSnapshot state;
  state.overlay = {.type = core::OverlayType::Disk, .maximumSizeMb = 8192, .warningThresholdMb = warning, .criticalThresholdMb = critical};
  return state;
}

class MemoryVolumeEditor final : public app::VolumeConfigurationEditing {
 public:
  bool protectedVolume = false;
  QStringList added;
  [[nodiscard]] QString driveLetter() const override { return QStringLiteral("Z:"); }
  bool importProtect(const bool value) override {
    const bool changed = protectedVolume != value;
    protectedVolume = value;
    return changed;
  }
  app::ExclusionImportOutcome importAddFileExclusion(const QString& path) override {
    added.append(path);
    return app::ExclusionImportOutcome::Applied;
  }
  app::ExclusionImportOutcome importRemoveFileExclusion(const QString&) override {
    throw std::logic_error("unexpected exclusion removal");
  }
};

class ConfigurationImportTests final : public QObject {
  Q_OBJECT

 private slots:
  void reportsNormalizedThresholds_data();
  void reportsNormalizedThresholds();
  void reportsEditorLimitsAndRepeatedNoOp();
  void matchingOutOfEditorRangeBaselineDoesNotQueueChange();
  void repeatedFieldsReportTheirFinalValueWithoutReordering();
  void reverseCommandsReportNoNetChange_data();
  void reverseCommandsReportNoNetChange();
  void oneUnchangedSettingDoesNotHideOtherPendingChanges_data();
  void oneUnchangedSettingDoesNotHideOtherPendingChanges();
  void supersededGlobalCommandsKeepTheirAcceptedDependencies();
  void normalizationBackToTheOriginalValueIsANoOp();
  void unchangedCommandRemainsANoOpWhenAnotherFieldNormalizesIt();
  void unavailableGlobalRejectsCommandsAndTheirDependents_data();
  void unavailableGlobalRejectsCommandsAndTheirDependents();
  void parseFailuresAndNoOpsKeepDependencySemantics();
  void fileRoutingUsesTheInjectedResolver();
  void importingTheSystemValueRestoresAManualDraft_data();
  void importingTheSystemValueRestoresAManualDraft();
  void importingTheOriginalVolumeProtectionRestoresAManualDraft();
  void importingAnOriginalExclusionRestoresAManualDraft();
};

void ConfigurationImportTests::reportsNormalizedThresholds_data() {
  QTest::addColumn<bool>("criticalFirst");
  QTest::newRow("warning-first") << false;
  QTest::newRow("critical-first") << true;
}

void ConfigurationImportTests::reportsNormalizedThresholds() {
  QFETCH(bool, criticalFirst);
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto warning = std::string("uwfmgr overlay set-warningthreshold 500");
  const auto critical = std::string("uwfmgr overlay set-criticalthreshold 400");
  const auto input = commands(criticalFirst ? critical + "\n" + warning : warning + "\n" + critical);
  const auto report = app::applyImportCommands(input, &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 2);
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{399});
  QCOMPARE(editor.draft().overlay.criticalThresholdMb, uint32_t{400});
  const auto& warningReport = report.at(criticalFirst ? 1 : 0);
  const auto& criticalReport = report.at(criticalFirst ? 0 : 1);
  QCOMPARE(warningReport.status, app::ImportResult::Status::Success);
  QVERIFY(warningReport.detail.contains(QStringLiteral("→ 399 MB")));
  QVERIFY(warningReport.detail.contains(QStringLiteral("requested 500 MB")));
  QCOMPARE(criticalReport.status, app::ImportResult::Status::Success);
  QVERIFY(criticalReport.detail.contains(QStringLiteral("→ 400 MB")));
  QCOMPARE(report.at(0).lineNo, 1);
  QCOMPARE(report.at(1).lineNo, 2);
  QCOMPARE(report.at(0).lineText, QString::fromStdString(input.at(0).rawLine));
  QCOMPARE(report.at(1).lineText, QString::fromStdString(input.at(1).rawLine));
}

void ConfigurationImportTests::reportsEditorLimitsAndRepeatedNoOp() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto initial = app::applyImportCommands(commands("uwfmgr overlay set-size 4294967295"), &editor, {}, nullptr, noDriveResolution());
  QCOMPARE(initial.size(), 1);
  QCOMPARE(initial.at(0).status, app::ImportResult::Status::Success);
  QVERIFY(initial.at(0).detail.contains(QStringLiteral("→ 2147483647 MB")));
  QVERIFY(initial.at(0).detail.contains(QStringLiteral("requested 4294967295 MB")));
  QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{2147483647});

  editor.reset(baseline());
  const auto report = app::applyImportCommands(commands("uwfmgr overlay set-size 4294967295\nuwfmgr overlay set-size 4294967295"), &editor, {},
                                               nullptr, noDriveResolution());

  QCOMPARE(report.size(), 2);
  QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{2147483647});
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Success);
  QVERIFY(report.at(0).detail.contains(QStringLiteral("replaced by a later command")));
  QVERIFY(report.at(0).detail.contains(QStringLiteral("Final draft value → 2147483647 MB")));
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.at(1).detail.contains(QStringLiteral("→ 2147483647 MB")));
  QVERIFY(report.at(1).detail.contains(QStringLiteral("requested 4294967295 MB")));
  QVERIFY(!editor.importOverlayMaxMb(std::numeric_limits<uint32_t>::max()));
}

void ConfigurationImportTests::repeatedFieldsReportTheirFinalValueWithoutReordering() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto input = commands("uwfmgr overlay set-warningthreshold 500\nuwfmgr overlay set-criticalthreshold 800\n"
                              "uwfmgr overlay set-warningthreshold 600\nuwfmgr overlay set-warningthreshold 600");
  const auto report = app::applyImportCommands(input, &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 4);
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{600});
  QCOMPARE(editor.draft().overlay.criticalThresholdMb, uint32_t{800});
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Success);
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Success);
  QCOMPARE(report.at(2).status, app::ImportResult::Status::Success);
  QCOMPARE(report.at(3).status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.at(0).detail.contains(QStringLiteral("Final draft value → 600 MB")));
  QVERIFY(report.at(2).detail.contains(QStringLiteral("Final draft value → 600 MB")));
  QVERIFY(report.at(3).detail.contains(QStringLiteral("Final draft warning threshold → 600 MB")));
  for (qsizetype index = 0; index < report.size(); ++index) {
    QCOMPARE(report.at(index).lineNo, input.at(index).sourceLineNo);
    QCOMPARE(report.at(index).lineText, QString::fromStdString(input.at(index).rawLine));
  }
}

void ConfigurationImportTests::normalizationBackToTheOriginalValueIsANoOp() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline(399, 400));
  const auto report = app::applyImportCommands(commands("uwfmgr overlay set-warningthreshold 500"), &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 1);
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{399});
  QVERIFY(editor.pendingOverlay().empty());
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.at(0).detail.contains(QStringLiteral("→ 399 MB")));
  QVERIFY(report.at(0).detail.contains(QStringLiteral("requested 500 MB")));
}

void ConfigurationImportTests::reverseCommandsReportNoNetChange_data() {
  QTest::addColumn<QString>("text");
  QTest::newRow("warning") << QStringLiteral("uwfmgr overlay set-warningthreshold 500\nuwfmgr overlay set-warningthreshold 1000");
  QTest::newRow("type") << QStringLiteral("uwfmgr overlay set-type RAM\nuwfmgr overlay set-type Disk");
  QTest::newRow("filter") << QStringLiteral("uwfmgr filter enable\nuwfmgr filter disable");
  QTest::newRow("size") << QStringLiteral("uwfmgr overlay set-size 16000\nuwfmgr overlay set-size 8192");
  QTest::newRow("critical") << QStringLiteral("uwfmgr overlay set-criticalthreshold 2000\nuwfmgr overlay set-criticalthreshold 3000");
}

void ConfigurationImportTests::reverseCommandsReportNoNetChange() {
  QFETCH(QString, text);
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto input = commands(text.toStdString());
  const auto report = app::applyImportCommands(input, &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 2);
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Success);
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.at(1).detail.contains(QStringLiteral("No net change for this setting")));
  QVERIFY(!report.at(1).detail.contains(QStringLiteral("Pending")));
  QVERIFY(!report.at(0).detail.contains(QStringLiteral("Pending")));
  QVERIFY(report.at(0).detail.contains(QStringLiteral("replaced by a later command")));
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{1000});
  QCOMPARE(editor.draft().overlay.type, core::OverlayType::Disk);
  QVERIFY(!editor.draft().filterEnabled);
  QVERIFY(editor.pendingOverlay().empty());
  QVERIFY(!editor.pendingFilterEnabled());
  QCOMPARE(report.at(0).lineText, QString::fromStdString(input.at(0).rawLine));
  QCOMPARE(report.at(1).lineText, QString::fromStdString(input.at(1).rawLine));
}

void ConfigurationImportTests::oneUnchangedSettingDoesNotHideOtherPendingChanges_data() {
  QTest::addColumn<QString>("script");
  QTest::addColumn<bool>("changesCritical");
  QTest::newRow("warning") << QStringLiteral("uwfmgr overlay set-warningthreshold 500\nuwfmgr overlay set-warningthreshold 1000\n"
                                            "uwfmgr overlay set-criticalthreshold 2000") << true;
  QTest::newRow("critical") << QStringLiteral("uwfmgr overlay set-criticalthreshold 2000\nuwfmgr overlay set-criticalthreshold 3000\n"
                                             "uwfmgr overlay set-warningthreshold 500") << false;
  QTest::newRow("maximum") << QStringLiteral("uwfmgr overlay set-size 16000\nuwfmgr overlay set-size 8192\n"
                                            "uwfmgr overlay set-warningthreshold 500") << false;
  QTest::newRow("type") << QStringLiteral("uwfmgr overlay set-type RAM\nuwfmgr overlay set-type Disk\n"
                                         "uwfmgr overlay set-warningthreshold 500") << false;
  QTest::newRow("filter") << QStringLiteral("uwfmgr filter enable\nuwfmgr filter disable\nuwfmgr overlay set-warningthreshold 500") << false;
}

void ConfigurationImportTests::oneUnchangedSettingDoesNotHideOtherPendingChanges() {
  QFETCH(QString, script);
  QFETCH(bool, changesCritical);
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto input = commands(script.toStdString());
  const auto report = app::applyImportCommands(input, &editor, {}, nullptr, noDriveResolution());
  QCOMPARE(report.size(), 3);
  QVERIFY(!editor.pendingOverlay().empty());
  QCOMPARE(editor.draft().overlay.warningThresholdMb, changesCritical ? uint32_t{1000} : uint32_t{500});
  QCOMPARE(editor.draft().overlay.criticalThresholdMb, changesCritical ? uint32_t{2000} : uint32_t{3000});
  QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{8192});
  QCOMPARE(editor.draft().overlay.type, core::OverlayType::Disk);
  QVERIFY(!editor.draft().filterEnabled);
  QCOMPARE(report[0].status, app::ImportResult::Status::Success);
  QCOMPARE(report[1].status, app::ImportResult::Status::Duplicate);
  QCOMPARE(report[2].status, app::ImportResult::Status::Success);
  QVERIFY(report[0].detail.contains(QStringLiteral("replaced by a later command")));
  QVERIFY(!report[0].detail.contains(QStringLiteral("Pending")));
  QVERIFY(report[1].detail.contains(QStringLiteral("No net change for this setting")));
  QVERIFY(report[2].detail.startsWith(QStringLiteral("Final draft ")));
  QVERIFY(!report[2].detail.contains(QStringLiteral("Pending")));
  for (qsizetype row = 0; row < report.size(); ++row) {
    QVERIFY(!report[row].detail.contains(QStringLiteral("No net change from this import")));
    QCOMPARE(report[row].lineNo, input[row].sourceLineNo);
    QCOMPARE(report[row].lineText, QString::fromStdString(input[row].rawLine));
  }
}

void ConfigurationImportTests::supersededGlobalCommandsKeepTheirAcceptedDependencies() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  auto volume = std::make_shared<MemoryVolumeEditor>();
  const auto input = commands("uwfmgr overlay set-warningthreshold 500 && uwfmgr volume protect Z:\n"
                              "uwfmgr overlay set-warningthreshold 1000\nuwfmgr overlay set-criticalthreshold 2000");
  const auto report = app::applyImportCommands(input, &editor, {volume}, nullptr, noDriveResolution());
  QCOMPARE(report.size(), 4);
  QVERIFY(volume->protectedVolume);
  QCOMPARE(report[0].status, app::ImportResult::Status::Success);
  QCOMPARE(report[1].status, app::ImportResult::Status::Success);
  QCOMPARE(report[2].status, app::ImportResult::Status::Duplicate);
  QCOMPARE(report[3].status, app::ImportResult::Status::Success);
  QVERIFY(report[0].detail.contains(QStringLiteral("replaced by a later command")));
  QVERIFY(!report[0].detail.contains(QStringLiteral("Pending")));
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{1000});
  QCOMPARE(editor.draft().overlay.criticalThresholdMb, uint32_t{2000});
}

void ConfigurationImportTests::unchangedCommandRemainsANoOpWhenAnotherFieldNormalizesIt() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline(500, 1000));
  const auto report = app::applyImportCommands(commands("uwfmgr overlay set-warningthreshold 500\nuwfmgr overlay set-criticalthreshold 400"), &editor, {},
                                               nullptr, noDriveResolution());

  QCOMPARE(report.size(), 2);
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{399});
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.at(0).detail.contains(QStringLiteral("→ 399 MB")));
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Success);
}

void ConfigurationImportTests::unavailableGlobalRejectsCommandsAndTheirDependents_data() {
  QTest::addColumn<bool>("missing");
  QTest::newRow("missing-global") << true;
  QTest::newRow("unavailable-global") << false;
}

void ConfigurationImportTests::unavailableGlobalRejectsCommandsAndTheirDependents() {
  QFETCH(bool, missing);
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  editor.setAvailable(false);
  auto volume = std::make_shared<MemoryVolumeEditor>();
  const auto input = commands("uwfmgr filter enable && uwfmgr volume protect Z:\nuwfmgr overlay set-size 16000\n"
                              "uwfmgr overlay set-type RAM\nuwfmgr volume protect Z:");
  const auto report = app::applyImportCommands(input, missing ? nullptr : &editor, {volume}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 5);
  for (const qsizetype index : {qsizetype{0}, qsizetype{2}, qsizetype{3}}) {
    QCOMPARE(report.at(index).status, app::ImportResult::Status::Failed);
    QVERIFY(report.at(index).detail.contains(QStringLiteral("unavailable")));
  }
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Failed);
  QVERIFY(report.at(1).detail.contains(QStringLiteral("prerequisite")));
  QCOMPARE(report.at(4).status, app::ImportResult::Status::Success);
  QVERIFY(volume->protectedVolume);
  QVERIFY(!editor.draft().filterEnabled);
  QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{8192});
  QCOMPARE(editor.draft().overlay.type, core::OverlayType::Disk);
}

void ConfigurationImportTests::parseFailuresAndNoOpsKeepDependencySemantics() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto input = commands("uwfmgr overlay set-size nope && uwfmgr filter enable\n"
                              "uwfmgr overlay set-size 8192 && uwfmgr filter enable");
  const auto report = app::applyImportCommands(input, &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 4);
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Failed);
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Failed);
  QVERIFY(report.at(1).detail.contains(QStringLiteral("prerequisite")));
  QCOMPARE(report.at(2).status, app::ImportResult::Status::Duplicate);
  QCOMPARE(report.at(3).status, app::ImportResult::Status::Success);
  QVERIFY(editor.draft().filterEnabled);
}

void ConfigurationImportTests::matchingOutOfEditorRangeBaselineDoesNotQueueChange() {
  app::GlobalConfigurationEditor editor;
  auto state = baseline();
  state.overlay.maximumSizeMb = std::numeric_limits<uint32_t>::max();
  editor.reset(state);
  const auto report = app::applyImportCommands(commands("uwfmgr overlay set-size 4294967295"), &editor, {}, nullptr, noDriveResolution());
  QCOMPARE(report.size(), 1);
  QCOMPARE(report.front().status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.front().detail.contains(QStringLiteral("4294967295 MB")));
  QCOMPARE(editor.draft().overlay.maximumSizeMb, std::numeric_limits<uint32_t>::max());
  QVERIFY(editor.pendingOverlay().empty());
}

void ConfigurationImportTests::importingTheSystemValueRestoresAManualDraft_data() {
  QTest::addColumn<int>("field");
  QTest::addColumn<QString>("script");
  QTest::addColumn<QString>("finalValue");
  QTest::newRow("filter") << 0 << QStringLiteral("uwfmgr filter disable") << QStringLiteral("Disabled");
  QTest::newRow("type") << 1 << QStringLiteral("uwfmgr overlay set-type Disk") << QStringLiteral("Disk");
  QTest::newRow("maximum") << 2 << QStringLiteral("uwfmgr overlay set-size 8192") << QStringLiteral("8192 MB");
  QTest::newRow("warning") << 3 << QStringLiteral("uwfmgr overlay set-warningthreshold 1000") << QStringLiteral("1000 MB");
  QTest::newRow("critical") << 4 << QStringLiteral("uwfmgr overlay set-criticalthreshold 3000") << QStringLiteral("3000 MB");
}

void ConfigurationImportTests::importingTheSystemValueRestoresAManualDraft() {
  QFETCH(int, field);
  QFETCH(QString, script);
  QFETCH(QString, finalValue);
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  switch (field) {
    case 0: editor.editFilterEnabled(true); break;
    case 1: editor.editOverlayType(core::OverlayType::RAM); break;
    case 2: editor.editMaximumSize(16000); break;
    case 3: editor.editWarningThreshold(500); break;
    case 4: editor.editCriticalThreshold(2000); break;
    default: QFAIL("unknown test field");
  }
  QVERIFY(editor.pendingFilterEnabled().has_value() || !editor.pendingOverlay().empty());
  const auto report = app::applyImportCommands(commands(script.toStdString()), &editor, {}, nullptr, noDriveResolution());
  QVERIFY(!editor.pendingFilterEnabled());
  QVERIFY(editor.pendingOverlay().empty());
  QCOMPARE(report.size(), 1);
  QCOMPARE(report.front().status, app::ImportResult::Status::Success);
  QVERIFY(report.front().detail.startsWith(QStringLiteral("Final draft ")));
  QVERIFY(report.front().detail.contains(QStringLiteral("→ ") + finalValue));
  QVERIFY(!report.front().detail.contains(QStringLiteral("Pending")));
}

void ConfigurationImportTests::importingTheOriginalVolumeProtectionRestoresAManualDraft() {
  core::VolumeInfo info;
  info.driveLetter = "Z:";
  info.support = core::VolumeSupport::Supported;
  auto volume = std::make_shared<app::VolumeConfigurationEditor>(info, false);
  core::UwfSnapshot snapshot;
  snapshot.uwfAvailable = true;
  snapshot.elevated = true;
  snapshot.current.volumes = {{"Volume{fixture}", "Z:", true, true}};
  snapshot.next = snapshot.current;
  volume->applySnapshot(snapshot);
  QVERIFY(volume->status()->editProtection(false));
  QVERIFY(volume->status()->pendingProtection().has_value());
  const auto report = app::applyImportCommands(commands("uwfmgr volume protect Z:"), nullptr, {volume}, nullptr, noDriveResolution());
  QVERIFY(!volume->status()->pendingProtection());
  QCOMPARE(report.size(), 1);
  QCOMPARE(report.front().status, app::ImportResult::Status::Success);
  QVERIFY(report.front().detail.contains(QStringLiteral("draft")));
  QVERIFY(!report.front().detail.contains(QStringLiteral("Pending")));
}

void ConfigurationImportTests::importingAnOriginalExclusionRestoresAManualDraft() {
  app::ExclusionEditor registry(app::ExclusionEditor::Kind::Registry);
  const QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\DraftFixture");
  registry.setBaseline({key}, {key});
  QVERIFY(registry.removeSelection({{key, std::nullopt}}));
  QCOMPARE(registry.pendingRemoved(), QStringList{key});
  const auto report = app::applyImportCommands(commands("uwfmgr registry add-exclusion HKLM\\SOFTWARE\\DraftFixture"), nullptr, {}, &registry,
                                               noDriveResolution());
  QVERIFY(registry.pendingAdded().isEmpty());
  QVERIFY(registry.pendingRemoved().isEmpty());
  QCOMPARE(report.size(), 1);
  QCOMPARE(report.front().status, app::ImportResult::Status::Success);
  QVERIFY(report.front().detail.contains(QStringLiteral("draft")));
  QVERIFY(!report.front().detail.contains(QStringLiteral("pending"), Qt::CaseInsensitive));
}

void ConfigurationImportTests::fileRoutingUsesTheInjectedResolver() {
  auto volume = std::make_shared<MemoryVolumeEditor>();
  QStringList resolved;
  const app::DriveResolver resolve = [&](const std::string& path) {
    resolved.append(QString::fromStdString(path));
    return std::string("Z:");
  };
  const auto report = app::applyImportCommands(commands(R"(uwfmgr file add-exclusion "X:\Data")"), nullptr, {volume}, nullptr, resolve);

  QCOMPARE(report.size(), 1);
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Success);
  QCOMPARE(resolved, QStringList{QStringLiteral("X:\\Data")});
  QCOMPARE(volume->added, QStringList{QStringLiteral("X:\\Data")});
}

}  // namespace

QTEST_APPLESS_MAIN(ConfigurationImportTests)
#include "ConfigurationImportTests.moc"
