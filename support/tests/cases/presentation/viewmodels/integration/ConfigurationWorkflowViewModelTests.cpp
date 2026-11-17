// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/paths/ExclusionCoverage.h"
#include "application/paths/PathPolicy.h"
#include "cases/presentation/viewmodels/common/ViewModelStateFixture.h"
#include "domain/uwf/contracts/Types.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "simulation/app/ConfigurationState.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

class ConfigurationWorkflowViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void oneEditSessionCollectsEveryEntryPoint();
  void unicodePathsShareExclusionAndStagingDecisions();
  void stagedAliasesConstrainExclusionsWithoutOpeningTheStagingTab();
};

void ConfigurationWorkflowViewModelTests::stagedAliasesConstrainExclusionsWithoutOpeningTheStagingTab() {
  presentation::ConfigurationViewModel model(16384);
  const auto state = snapshot();
  const auto volume = model.addVolume(disk("D:"), false);
  model.resetSnapshot(state);
  model.setStagedEntries({{app::FileStagingKind::File, QStringLiteral("C:\\MountedD\\Data\\state.bin"),
                           app::FileCommitTarget{uwf::simulation::application::fixtureVolumeDName, QStringLiteral("\\Data\\state.bin")}}});
  QCOMPARE(volume->files()->editor()->addSelectedPath(QStringLiteral("D:\\Data")).outcome, app::ExclusionImportOutcome::RejectedConflict);
}

void ConfigurationWorkflowViewModelTests::oneEditSessionCollectsEveryEntryPoint() {
  presentation::ConfigurationViewModel model(16384);
  const auto state = snapshot();
  auto first = model.addVolume(disk(), true);
  model.addVolume(disk("D:"), false);
  model.resetSnapshot(state);
  const auto parsed = app::parseUwfmgrText("uwfmgr.exe filter disable\nuwfmgr.exe volume protect D:\nuwfmgr.exe volume unprotect C:");
  QList<app::UwfmgrCommand> commands;
  for (const auto& command : parsed) commands.append(command);
  const auto report = model.importCommands(commands);
  QCOMPARE(report.size(), 3);
  for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
  const auto pending = model.pendingChanges();
  QCOMPARE(pending.setFilterEnabled, std::optional<bool>(false));
  QCOMPARE(pending.volumeProtect.at(compat::normalizeVolumeId(uwf::simulation::application::fixtureVolumeCName)), false);
  QCOMPARE(pending.volumeProtect.at(compat::normalizeVolumeId(uwf::simulation::application::fixtureVolumeDName)), true);
  QVERIFY(first->status()->editProtection(true));
  QVERIFY(!model.pendingChanges().volumeProtect.contains(compat::normalizeVolumeId(uwf::simulation::application::fixtureVolumeCName)));
}

void ConfigurationWorkflowViewModelTests::unicodePathsShareExclusionAndStagingDecisions() {
  // Business paths describe the simulated Windows target, not the test host.
  const QString parent = QStringLiteral("C:\\Fixture\\Äpfel");
  const QString alternate = QStringLiteral("C:\\Fixture\\äPFEL");
  const QString child = alternate + QStringLiteral("\\state.bin");
  app::FileStagingConflictPolicy conflicts;
  conflicts.setFileExclusions({parent});
  QVERIFY(conflicts.conflictingExclusion(child, app::FileStagingKind::File).has_value());
  QVERIFY(app::paths::contains(parent, child));
  QCOMPARE(app::fileExclusionCoverage(QStringList{parent}, child).directWriteExclusion, parent);
  QVERIFY(!app::paths::contains(parent, alternate + QStringLiteral("-sibling\\state.bin")));
  QVERIFY(!app::paths::contains(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\.."), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor")));
}

}  // namespace

QTEST_GUILESS_MAIN(ConfigurationWorkflowViewModelTests)
#include "ConfigurationWorkflowViewModelTests.moc"
