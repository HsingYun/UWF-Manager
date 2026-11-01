// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/paths/ExclusionCoverage.h"
#include "application/paths/PathPolicy.h"
#include "cases/presentation/viewmodels/common/ViewModelStateFixture.h"
#include "domain/uwf/contracts/Types.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"

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
                           app::FileCommitTarget{test::application::fixtureVolumeDName, QStringLiteral("\\Data\\state.bin")}}});
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
  QCOMPARE(pending.volumeProtect.at(compat::normalizeVolumeId(test::application::fixtureVolumeCName)), false);
  QCOMPARE(pending.volumeProtect.at(compat::normalizeVolumeId(test::application::fixtureVolumeDName)), true);
  QVERIFY(first->status()->editProtection(true));
  QVERIFY(!model.pendingChanges().volumeProtect.contains(compat::normalizeVolumeId(test::application::fixtureVolumeCName)));
}

void ConfigurationWorkflowViewModelTests::unicodePathsShareExclusionAndStagingDecisions() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString parent = QDir::toNativeSeparators(directory.filePath(QString::fromUtf8("Äpfel")));
  QVERIFY(QDir().mkpath(parent));
  const QString alternate = directory.filePath(QString::fromUtf8("äPFEL"));
  QVERIFY(QFileInfo::exists(alternate));
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
