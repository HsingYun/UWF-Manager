// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <stdexcept>
#include <utility>

#include "app/configuration/ConfigurationEditSession.h"
#include "app/configuration/GlobalConfigurationEditor.h"
#include "app/configuration/VolumeConfigurationEditor.h"
#include "app/ports/FileStagingStore.h"

namespace {
using namespace uwf;

QList<app::UwfmgrCommand> commands(const std::string& text) {
  QList<app::UwfmgrCommand> result;
  for (const auto& command : app::parseUwfmgrText(text)) result.append(command);
  return result;
}

std::shared_ptr<app::VolumeConfigurationEditor> addVolume(app::ConfigurationEditSession& session) {
  const core::VolumeInfo volume{"C:", "Volume{c}", "NTFS", "System", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported};
  auto editor = session.editVolume(volume, true);
  core::UwfSnapshot state;
  state.uwfAvailable = true;
  state.elevated = true;
  state.current.overlay = {core::OverlayType::RAM, 4096, 1024, 3072};
  state.current.volumes = {{"Volume{c}", "C:", true, true}};
  state.next = state.current;
  session.globalEditor()->reset(state.next);
  editor->applySnapshot(state);
  return editor;
}

class ConfigurationEditSessionTests final : public QObject {
  Q_OBJECT
 private slots:
  void activeImportRetainsItsTargetUntilTheCommandReturns_data() {
    QTest::addColumn<QString>("script");
    QTest::newRow("volume") << QStringLiteral("uwfmgr.exe volume unprotect C:\nuwfmgr.exe volume protect C:");
    QTest::newRow("file-exclusions") << QStringLiteral("uwfmgr.exe file add-exclusion C:\\UwfData\nuwfmgr.exe file add-exclusion C:\\UwfOther");
  }
  void activeImportRetainsItsTargetUntilTheCommandReturns() {
    QFETCH(QString, script);
    app::ConfigurationEditSession session;
    auto editor = addVolume(session);
    QPointer<app::VolumeConfigurationEditor> lifetime(editor.get());
    bool reset = false;
    bool aliveDuringReset = false;
    connect(editor.get(), &app::VolumeConfigurationEditor::pendingChanged, this, [&] {
      session.clearVolumes();
      reset = true;
      aliveDuringReset = !lifetime.isNull();
    });
    editor.reset();
    const auto report = session.importCommands(commands(script.toStdString()));
    QVERIFY(reset);
    QVERIFY(aliveDuringReset);
    QVERIFY(!lifetime);
    QCOMPARE(report.size(), 2);
    QCOMPARE(report[0].status, app::ImportResult::Status::Success);
    QCOMPARE(report[1].status, app::ImportResult::Status::Failed);
    QVERIFY(session.pendingChanges().empty());
  }
  void targetsRemovedByAnEarlierCommandAreNotDispatched() {
    app::ConfigurationEditSession session;
    auto editor = addVolume(session);
    QPointer<app::VolumeConfigurationEditor> lifetime(editor.get());
    editor.reset();
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::pendingChanged, this, [&] { session.clearVolumes(); });
    const auto report = session.importCommands(commands("uwfmgr.exe filter enable\nuwfmgr.exe volume unprotect C:"));
    QCOMPARE(report.size(), 2);
    QCOMPARE(report[0].status, app::ImportResult::Status::Success);
    QCOMPARE(report[1].status, app::ImportResult::Status::Failed);
    QVERIFY(!lifetime);
    QCOMPARE(session.pendingChanges().setFilterEnabled, std::optional<bool>(true));
  }
  void refreshingStagingConstraintsDoesNotRequireVolumeEditorsToStayAlive() {
    app::ConfigurationEditSession session;
    auto editor = addVolume(session);
    const auto root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), app::kFileStagingRegistryRoot.size());
    bool stagingHasData = false;
    editor->registry()->setFileStagingDataProbe([&] { return stagingHasData; });
    editor->registry()->setBaseline({root}, {root});
    QCOMPARE(editor->registry()->importRemove(root), app::ExclusionImportOutcome::Applied);
    QPointer<app::VolumeConfigurationEditor> lifetime(editor.get());
    bool notified = false;
    bool aliveDuringReset = false;
    connect(editor->registry().get(), &app::ExclusionEditor::stateChanged, this, [&] {
      session.clearVolumes();
      notified = true;
      aliveDuringReset = !lifetime.isNull();
    });
    editor.reset();
    stagingHasData = true;
    session.registryEditor()->refreshFileStagingConstraints();
    const auto pending = session.pendingChanges();
    QVERIFY(notified);
    QVERIFY(!aliveDuringReset);
    QVERIFY(!lifetime);
    QVERIFY(pending.empty());
  }
  void stagingConstraintUpdatesAreExplicitAndQueriesAreReadOnly_data() {
    QTest::addColumn<bool>("required");
    QTest::addColumn<bool>("readFails");
    QTest::newRow("empty") << false << false;
    QTest::newRow("contains-entries") << true << false;
    QTest::newRow("unreadable") << false << true;
  }
  void stagingConstraintUpdatesAreExplicitAndQueriesAreReadOnly() {
    QFETCH(bool, required);
    QFETCH(bool, readFails);
    app::ConfigurationEditSession session;
    addVolume(session);
    const auto registry = session.registryEditor();
    const auto root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), app::kFileStagingRegistryRoot.size());
    bool stagingHasData = false;
    bool stagingReadFails = false;
    int probes = 0;
    registry->setFileStagingDataProbe([&] {
      ++probes;
      if (stagingReadFails) throw std::runtime_error("staging read failed");
      return stagingHasData;
    });
    registry->setBaseline({root}, {root});
    QCOMPARE(registry->importRemove(root), app::ExclusionImportOutcome::Applied);
    QSignalSpy states(registry.get(), &app::ExclusionEditor::stateChanged);
    QSignalSpy pending(registry.get(), &app::ExclusionEditor::pendingChanged);
    QSignalSpy availability(registry.get(), &app::ExclusionEditor::removalAvailabilityChanged);
    stagingHasData = required;
    stagingReadFails = readFails;
    const int probesBeforeQuery = probes;
    const auto& readOnlySession = std::as_const(session);
    for (int iteration = 0; iteration < 2; ++iteration) {
      QCOMPARE(readOnlySession.pendingChanges().removeRegistryExclusions, std::vector<std::string>{root.toStdString()});
      QCOMPARE(registry->pendingRemoved(), QStringList{root});
    }
    QCOMPARE(probes, probesBeforeQuery);
    QCOMPARE(states.size(), 0);
    QCOMPARE(pending.size(), 0);
    QCOMPARE(availability.size(), 0);

    registry->refreshFileStagingConstraints();
    const bool protectedRoot = required || readFails;
    QCOMPARE(registry->pendingRemoved().isEmpty(), protectedRoot);
    QCOMPARE(readOnlySession.pendingChanges().removeRegistryExclusions.empty(), protectedRoot);
    QCOMPARE(states.size(), protectedRoot ? 1 : 0);
    QCOMPARE(pending.size(), protectedRoot ? 1 : 0);
    QCOMPARE(availability.size(), 1);
    stagingHasData = false;
    stagingReadFails = false;
    registry->refreshFileStagingConstraints();
    QCOMPARE(registry->pendingRemoved().isEmpty(), protectedRoot);
    QCOMPARE(availability.size(), 2);
  }
  void globalRegistryIntentDoesNotDependOnVolumeSupportOrLifetime_data() {
    QTest::addColumn<core::VolumeSupport>("support");
    QTest::addColumn<bool>("removeVolumes");
    for (const auto support :
         {core::VolumeSupport::Supported, core::VolumeSupport::FileSystemLimited, core::VolumeSupport::ExceedsMaxSize, core::VolumeSupport::NotFixedLocalDisk})
      for (const bool remove : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("support-%1-remove-%2").arg(static_cast<int>(support)).arg(remove))) << support << remove;
  }
  void globalRegistryIntentDoesNotDependOnVolumeSupportOrLifetime() {
    QFETCH(core::VolumeSupport, support);
    QFETCH(bool, removeVolumes);
    app::ConfigurationEditSession session;
    core::SessionSnapshot baseline;
    baseline.overlay.type = core::OverlayType::RAM;
    session.globalEditor()->reset(baseline);
    core::VolumeInfo volume;
    volume.driveLetter = "C:";
    volume.support = support;
    auto editor = session.editVolume(volume, true);
    const QString path = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfData");
    QCOMPARE(editor->registry()->importAdd(path), app::ExclusionImportOutcome::Applied);
    if (removeVolumes) session.clearVolumes();
    QCOMPARE(session.pendingChanges().addRegistryExclusions, std::vector<std::string>{path.toStdString()});
    const auto report = session.importCommands(commands("uwfmgr.exe registry remove-exclusion HKLM\\SOFTWARE\\UwfData"));
    QCOMPARE(report.front().status, app::ImportResult::Status::Success);
    QVERIFY(session.pendingChanges().empty());
  }
};
}  // namespace

QTEST_APPLESS_MAIN(ConfigurationEditSessionTests)
#include "ConfigurationEditSessionTests.moc"
