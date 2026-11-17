// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <cstdint>
#include <set>
#include <stdexcept>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/planning/ConfigurationScript.h"
#include "domain/uwf/contracts/Types.h"
#include "domain/uwf/model/UwfModel.h"
#include "domain/uwf/policies/OverlayWritePlan.h"
#include "domain/uwf/policies/UwfConstraints.h"

namespace {
uwf::app::ConfigurationPlan configurationPlan(const uwf::core::SessionSnapshot& next, const uwf::core::PendingChanges& changes) {
  uwf::app::ConfigurationPlan plan;
  plan.snapshot.next = next;
  plan.changes = changes;
  return plan;
}
class OverlayPlanningTests final : public QObject {
  Q_OBJECT
 private slots:
  void overlayWritePlanPreservesThresholdConstraints();
  void overlayWritePlanPreservesStorageConstraints();
  void invalidStorageTargetsFailBeforeExecutablePhasesOrExport();
  void observedInvalidStorageIsOmittedUntilExplicitlyRepaired();
  void bulkProtectionExportUsesAllOnlyForProtectAndCompleteGuidSnapshotForUnprotect();
  void unprotectExportRequiresAnAuthoritativeCompleteSnapshot();
};

void OverlayPlanningTests::bulkProtectionExportUsesAllOnlyForProtectAndCompleteGuidSnapshotForUnprotect() {
  using namespace uwf;
  app::ConfigurationPlan plan;
  plan.snapshot.uwfAvailable = true;
  plan.snapshot.next.volumes = {{"Volume{11111111-1111-1111-1111-111111111111}", "C:", true},
                                {"Volume{22222222-2222-2222-2222-222222222222}", {}, false},
                                {"Volume{33333333-3333-3333-3333-333333333333}", {}, true}};
  plan.maintenance.allVolumesProtected = true;
  for (const auto& script : {app::configurationChangeScript(plan), app::configurationTargetScript(plan)}) {
    QVERIFY(script.exportSupported);
    QCOMPARE(std::ranges::count_if(script.commands, [](const auto& command) { return command.kind == app::UwfmgrKind::VolumeProtectAll; }), 1);
    QVERIFY(app::renderConfigurationScript(script).find("uwfmgr.exe volume protect all") != std::string::npos);
    QVERIFY(std::ranges::none_of(script.omissions, [](const auto& value) { return value.kind == app::ConfigurationScriptOmissionKind::CurrentSessionAction; }));
  }
  plan.maintenance.allVolumesProtected = false;
  plan.maintenanceSnapshot.emplace();
  plan.maintenanceSnapshot->unprotectScopeConfirmed = true;
  plan.maintenanceSnapshot->unprotectTargetVolumes = {
      QStringLiteral("Volume{11111111-1111-1111-1111-111111111111}"), QStringLiteral("Volume{22222222-2222-2222-2222-222222222222}"),
      QStringLiteral("Volume{33333333-3333-3333-3333-333333333333}"), QStringLiteral("Volume{44444444-4444-4444-4444-444444444444}")};
  for (const auto& script : {app::configurationChangeScript(plan), app::configurationTargetScript(plan)}) {
    QVERIFY(script.exportSupported);
    std::set<std::string> identities;
    for (const auto& command : script.commands)
      if (command.kind == app::UwfmgrKind::VolumeUnprotect && !command.args.empty())
        if (const auto id = compat::parseVolumeId(command.args.front())) {
          identities.insert(*id);
          QCOMPARE(command.args.front(), compat::volumePath(*id));
        }
    // The complete batch scope also retains an offline/stale registration that
    // the ordinary settings snapshot did not include.
    QCOMPARE(identities.size(), std::size_t{4});
    QVERIFY(identities.contains("44444444-4444-4444-4444-444444444444"));
    const auto text = app::renderConfigurationScript(script);
    QVERIFY(text.find("volume unprotect all") == std::string::npos);
    for (const auto& parsed : app::parseUwfmgrText(text))
      if (parsed.parseError != app::ParseError::Comment) QCOMPARE(parsed.parseError, app::ParseError::None);
  }
}
void OverlayPlanningTests::unprotectExportRequiresAnAuthoritativeCompleteSnapshot() {
  using namespace uwf;
  app::ConfigurationPlan plan;
  plan.maintenance.allVolumesProtected = false;
  plan.snapshot.next.volumes = {{"Volume{11111111-1111-1111-1111-111111111111}", {}, true}};
  auto script = app::configurationChangeScript(plan);
  QVERIFY(!script.exportSupported);
  QVERIFY(script.commands.empty());
  plan.snapshot.uwfAvailable = true;
  script = app::configurationChangeScript(plan);
  QVERIFY(!script.exportSupported);  // Availability does not certify the scope.
  QVERIFY(script.commands.empty());
  plan.maintenanceSnapshot.emplace();
  plan.maintenanceSnapshot->unprotectTargetVolumes = {QStringLiteral("Volume{11111111-1111-1111-1111-111111111111}")};
  script = app::configurationChangeScript(plan);
  QVERIFY(!script.exportSupported);  // An unconfirmed list is not authoritative.
  QVERIFY(script.commands.empty());
  plan.maintenanceSnapshot->unprotectScopeConfirmed = true;
  plan.maintenanceSnapshot->unprotectTargetVolumes.push_back(QStringLiteral("Malformed configured identity"));
  script = app::configurationChangeScript(plan);
  QVERIFY(!script.exportSupported);
  QVERIFY(script.commands.empty());
  plan.maintenanceSnapshot->unprotectTargetVolumes.clear();
  plan.maintenanceSnapshot->unprotectScopeConfirmed = false;
  script = app::configurationChangeScript(plan);
  QVERIFY(!script.exportSupported);  // Empty and unreadable remain distinct.
  QVERIFY(script.commands.empty());
  plan.maintenanceSnapshot->unprotectScopeConfirmed = true;
  script = app::configurationChangeScript(plan);
  QVERIFY(script.exportSupported);
  QVERIFY(script.commands.empty());  // Explicitly known no registered Next targets.
}

void OverlayPlanningTests::overlayWritePlanPreservesThresholdConstraints() {
  using namespace uwf::core;
  constexpr uint32_t values[] = {0, 1, 2, 512, 768, 1024, 1536, UINT32_MAX};
  const auto valid = [](uint32_t warning, uint32_t critical) { return warning == 0 || critical == 0 || warning < critical; };
  for (const auto oldWarning : values) {
    for (const auto oldCritical : values) {
      if (!valid(oldWarning, oldCritical)) continue;
      for (const auto warning : values) {
        for (const auto critical : values) {
          if (!valid(warning, critical)) continue;
          OverlayConfig state{.warningThresholdMb = oldWarning, .criticalThresholdMb = oldCritical};
          OverlayConfigDelta delta;
          if (oldWarning != warning) delta.warningThresholdMb = warning;
          if (oldCritical != critical) delta.criticalThresholdMb = critical;
          const auto steps = planOverlayWrites(state, delta);
          for (const auto& step : steps) {
            if (step.field == OverlayField::WarningThreshold)
              state.warningThresholdMb = warning;
            else {
              QCOMPARE(step.field, OverlayField::CriticalThreshold);
              state.criticalThresholdMb = critical;
            }
            QVERIFY(valid(state.warningThresholdMb, state.criticalThresholdMb));
          }
          QCOMPARE(state.warningThresholdMb, warning);
          QCOMPARE(state.criticalThresholdMb, critical);

          PendingChanges changes;
          changes.setOverlay = delta;
          SessionSnapshot session;
          session.overlay = {OverlayType::RAM, UINT32_MAX, oldWarning, oldCritical};
          const auto plan = configurationPlan(session, changes);
          const auto pendingCommands = uwf::app::configurationChangeScript(plan).commands;
          QCOMPARE(pendingCommands.size(), steps.size());
          auto replay = session.overlay;
          const auto commands = uwf::app::configurationTargetScript(plan).commands;
          for (const auto& command : commands) {
            using enum uwf::app::UwfmgrKind;
            if (command.kind == OverlaySetWarningThreshold) replay.warningThresholdMb = static_cast<uint32_t>(std::stoul(command.args.front()));
            if (command.kind == OverlaySetCriticalThreshold) replay.criticalThresholdMb = static_cast<uint32_t>(std::stoul(command.args.front()));
            QVERIFY(valid(replay.warningThresholdMb, replay.criticalThresholdMb));
          }
          QCOMPARE(replay.warningThresholdMb, warning);
          QCOMPARE(replay.criticalThresholdMb, critical);
        }
      }
    }
  }
}

void OverlayPlanningTests::overlayWritePlanPreservesStorageConstraints() {
  using namespace uwf::core;
  constexpr uint32_t values[] = {1, 512, 1024, 2048, 4096};
  for (const auto oldType : {OverlayType::RAM, OverlayType::Disk}) {
    for (const auto oldSize : values) {
      if (oldType == OverlayType::Disk && oldSize < 1024) continue;
      for (const auto type : {OverlayType::RAM, OverlayType::Disk}) {
        for (const auto size : values) {
          if (type == OverlayType::Disk && size < 1024) continue;
          const OverlayConfig baseline{.type = oldType, .maximumSizeMb = oldSize};
          OverlayConfigDelta delta;
          if (oldType != type) delta.type = type;
          if (oldSize != size) delta.maximumSizeMb = size;
          auto state = baseline;
          for (const auto& step : planOverlayWrites(baseline, delta)) {
            if (step.field == OverlayField::Type)
              state.type = type;
            else {
              QCOMPARE(step.field, OverlayField::MaximumSize);
              state.maximumSizeMb = size;
            }
            QVERIFY(state.type != OverlayType::Disk || state.maximumSizeMb >= 1024);
          }
          QCOMPARE(state.type, type);
          QCOMPARE(state.maximumSizeMb, size);

          SessionSnapshot session;
          session.overlay = baseline;
          PendingChanges changes;
          changes.setOverlay = delta;
          state = baseline;
          for (const auto& command : uwf::app::configurationTargetScript(configurationPlan(session, changes)).commands) {
            using enum uwf::app::UwfmgrKind;
            if (command.kind == OverlaySetType) state.type = command.args.front() == "RAM" ? OverlayType::RAM : OverlayType::Disk;
            if (command.kind == OverlaySetSize) state.maximumSizeMb = static_cast<uint32_t>(std::stoul(command.args.front()));
            QVERIFY(state.type != OverlayType::Disk || state.maximumSizeMb >= 1024);
          }
          QCOMPARE(state.type, type);
          QCOMPARE(state.maximumSizeMb, size);
        }
      }
    }
  }
}

void OverlayPlanningTests::invalidStorageTargetsFailBeforeExecutablePhasesOrExport() {
  using namespace uwf;
  for (const auto type : {core::OverlayType::RAM, core::OverlayType::Disk})
    for (const auto size : {0u, 1u, 1023u, 1024u}) {
      core::SessionSnapshot next;
      next.overlay = {.type = type, .maximumSizeMb = 2048};
      core::PendingChanges changes;
      changes.setFilterEnabled = true;
      changes.setOverlay.maximumSizeMb = size;
      changes.setOverlay.warningThresholdMb = 0;  // Threshold zero stays legal.
      const auto plan = configurationPlan(next, changes);
      if (core::validOverlayTarget(type, size)) {
        QVERIFY(!app::configurationPhases(plan).empty());
        QVERIFY(!app::configurationChangeScript(plan).commands.empty());
      } else {
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)app::configurationPhases(plan));
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)app::configurationChangeScript(plan));
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)app::configurationTargetScript(plan));
      }
    }
}

void OverlayPlanningTests::observedInvalidStorageIsOmittedUntilExplicitlyRepaired() {
  using namespace uwf;
  core::SessionSnapshot next;
  next.overlay = {.type = core::OverlayType::RAM, .maximumSizeMb = 0};
  core::PendingChanges changes;
  changes.setFilterEnabled = true;
  const auto plan = configurationPlan(next, changes);
  QCOMPARE(app::configurationPhases(plan).size(), std::size_t{1});
  const auto script = app::configurationTargetScript(plan);
  QVERIFY(std::ranges::find(script.omissions, app::ConfigurationScriptOmission{app::ConfigurationScriptOmissionKind::InvalidObservedOverlayStorage, {}}) !=
          script.omissions.end());
  for (const auto& command : script.commands) QVERIFY(command.kind != app::UwfmgrKind::OverlaySetType && command.kind != app::UwfmgrKind::OverlaySetSize);
  QCOMPARE(plan.snapshot.next.overlay.maximumSizeMb, uint32_t{0});
  changes.setOverlay.maximumSizeMb = 1;
  const auto repaired = app::configurationTargetScript(configurationPlan(next, changes));
  QVERIFY(std::ranges::none_of(repaired.omissions,
                               [](const auto& omission) { return omission.kind == app::ConfigurationScriptOmissionKind::InvalidObservedOverlayStorage; }));
  QVERIFY(std::ranges::any_of(repaired.commands,
                              [](const auto& command) { return command.kind == app::UwfmgrKind::OverlaySetSize && command.args.front() == "1"; }));
}

}  // namespace

QTEST_APPLESS_MAIN(OverlayPlanningTests)
#include "OverlayPlanningTests.moc"
