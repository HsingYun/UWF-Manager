// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <optional>
#include <string>

#include "app/configuration/ConfigurationDraft.h"
#include "app/configuration/ExclusionDraft.h"
#include "app/state/ApplicationStateSession.h"

namespace {

class ConfigurationDraftTests final : public QObject {
  Q_OBJECT

 private slots:
  void configurationDraftPreservesFieldIntentAndVolumeBoundaries();
  void configurationDraftAccumulatesRepeatedVolumeEntries();
  void exclusionDraftReversesEditsWithoutChangingTheBaseline();
  void applicationStateSessionPreservesReferencesAndReconciliationGates();
};

void ConfigurationDraftTests::configurationDraftPreservesFieldIntentAndVolumeBoundaries() {
  uwf::app::GlobalConfigurationDraft global;
  global.baselineFilter.enabled = true;
  global.filterEnabled = true;
  global.baselineOverlay = {.type = uwf::core::OverlayType::RAM, .maximumSizeMb = 4096, .warningThresholdMb = 1000, .criticalThresholdMb = 3000};
  global.overlay = global.baselineOverlay;
  QVERIFY(!global.pendingFilterEnabled());
  QVERIFY(global.pendingOverlay().empty());
  global.overlay.maximumSizeMb = 8192;
  global.overlay.warningThresholdMb = 0;
  const auto overlay = global.pendingOverlay();
  QVERIFY(!overlay.type);
  QCOMPARE(overlay.maximumSizeMb, std::optional<uint32_t>{8192});
  QCOMPARE(overlay.warningThresholdMb, std::optional<uint32_t>{0});
  QVERIFY(!overlay.criticalThresholdMb);

  uwf::app::ConfigurationDraft draft;
  draft.overlay = overlay;
  uwf::app::VolumeConfigurationChanges first;
  first.driveLetter = "C:";
  first.supported = true;
  first.protectedVolume = false;
  first.fileAdded = {R"(\Data)"};
  draft.registry.persistTSCAL = false;
  draft.volumes.push_back(first);
  auto unsupported = first;
  unsupported.driveLetter = "D:";
  unsupported.supported = false;
  draft.volumes.push_back(unsupported);
  uwf::app::VolumeConfigurationChanges empty;
  empty.driveLetter = "E:";
  empty.supported = true;
  draft.volumes.push_back(empty);
  const auto changes = draft.pendingChanges();
  QCOMPARE(changes.volumeProtect.size(), std::size_t{1});
  QCOMPARE(changes.volumeProtect.at("C:"), false);
  QCOMPARE(changes.addFileExclusions.size(), std::size_t{1});
  QCOMPARE(changes.addFileExclusions.at("C:"), first.fileAdded);
  QVERIFY(!changes.removeFileExclusions.contains("E:"));
  QCOMPARE(changes.setPersistTSCAL, std::optional<bool>{false});
  QCOMPARE(changes.count(), std::size_t{5});
  const auto ram = uwf::app::overlaySizeBounds(uwf::core::OverlayType::RAM, 8192, 2147483647);
  QCOMPARE(ram.minimum, 0);
  QCOMPARE(ram.maximum, 8192);
  const auto disk = uwf::app::overlaySizeBounds(uwf::core::OverlayType::Disk, 8192, 2147483647);
  QCOMPARE(disk.minimum, 1024);
  QCOMPARE(uwf::app::strictThresholdUpper(1), 0);
}

void ConfigurationDraftTests::configurationDraftAccumulatesRepeatedVolumeEntries() {
  uwf::app::ConfigurationDraft draft;
  uwf::app::VolumeConfigurationChanges first;
  first.driveLetter = "C:";
  first.supported = true;
  first.fileAdded = {R"(\First)"};
  first.fileRemoved = {R"(\OldFirst)"};
  draft.volumes.push_back(first);
  auto second = first;
  second.fileAdded = {R"(\Second)"};
  second.fileRemoved = {R"(\OldSecond)"};
  draft.volumes.push_back(second);
  auto empty = first;
  empty.fileAdded.clear();
  empty.fileRemoved.clear();
  draft.volumes.push_back(empty);
  auto unsupported = first;
  unsupported.supported = false;
  draft.volumes.push_back(unsupported);

  const auto changes = draft.pendingChanges();
  QCOMPARE(changes.addFileExclusions.size(), std::size_t{1});
  QCOMPARE(changes.removeFileExclusions.size(), std::size_t{1});
  QCOMPARE(changes.addFileExclusions.at("C:"), (std::vector<std::string>{R"(\First)", R"(\Second)"}));
  QCOMPARE(changes.removeFileExclusions.at("C:"), (std::vector<std::string>{R"(\OldFirst)", R"(\OldSecond)"}));
  QCOMPARE(changes.count(), std::size_t{4});
}

void ConfigurationDraftTests::exclusionDraftReversesEditsWithoutChangingTheBaseline() {
  uwf::app::ExclusionDraft draft;
  draft.current = {QStringLiteral("\\Current")};
  draft.next = {QStringLiteral("\\Data")};
  QVERIFY(!draft.add(QStringLiteral("\\data")));
  QVERIFY(draft.remove(QStringLiteral("\\DATA")));
  QVERIFY(draft.add(QStringLiteral("\\data")));
  QVERIFY(draft.removed.isEmpty());
  QVERIFY(draft.add(QStringLiteral("\\New")));
  QVERIFY(draft.remove(QStringLiteral("\\new")));
  QVERIFY(draft.added.isEmpty());
  QCOMPARE(draft.next, QStringList{QStringLiteral("\\Data")});
  draft.domainSecretKey.baseNext = true;
  draft.togglePersistence(draft.domainSecretKey);
  QCOMPARE(draft.domainSecretKey.pendingNext, std::optional<bool>{false});
  draft.enablePersistence(draft.domainSecretKey);
  QVERIFY(!draft.domainSecretKey.pendingNext);
  draft.enablePersistence(draft.tscal);
  QCOMPARE(draft.tscal.pendingNext, std::optional<bool>{true});
  draft.reset();
  QVERIFY(!draft.tscal.pendingNext);
  QCOMPARE(draft.current, QStringList{QStringLiteral("\\Current")});
}

void ConfigurationDraftTests::applicationStateSessionPreservesReferencesAndReconciliationGates() {
  uwf::app::ApplicationStateSession session;
  const auto* boundSnapshot = &session.snapshot();
  QVERIFY(!session.hasCommittedState());
  QVERIFY(!session.configurationWritesAllowed());
  uwf::app::ApplicationState state;
  state.snapshot.uwfAvailable = true;
  state.snapshot.elevated = true;
  state.snapshot.next.overlay.maximumSizeMb = 4096;
  session.commit(std::move(state));
  QCOMPARE(&session.snapshot(), boundSnapshot);
  QVERIFY(session.configurationWritesAllowed());
  session.requireReconciliation();
  QVERIFY(session.reconciliationRequired());
  QVERIFY(!session.configurationWritesAllowed());
  QCOMPARE(boundSnapshot->next.overlay.maximumSizeMb, uint32_t{4096});
  uwf::app::ApplicationState unavailable;
  session.commit(std::move(unavailable));
  QCOMPARE(&session.snapshot(), boundSnapshot);
  QVERIFY(session.hasCommittedState());
  QVERIFY(!session.reconciliationRequired());
  QVERIFY(!session.configurationWritesAllowed());
}

}  // namespace

QTEST_APPLESS_MAIN(ConfigurationDraftTests)

#include "ConfigurationDraftTests.moc"
