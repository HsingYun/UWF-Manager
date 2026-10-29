// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <vector>

#include "app/configuration/UwfmgrCommands.h"
#include "core/OverlayWritePlan.h"
#include "core/UwfModel.h"

namespace {
class OverlayPlanningTests final : public QObject {
  Q_OBJECT
 private slots:
  void overlayWritePlanPreservesThresholdConstraints();
  void overlayWritePlanPreservesStorageConstraints();
};

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
          const OverlayConfig baseline{.warningThresholdMb = oldWarning, .criticalThresholdMb = oldCritical};
          const auto pendingCommands = uwf::app::renderPendingChanges(changes, baseline);
          QCOMPARE(pendingCommands.size(), steps.size());
          SessionSnapshot session;
          session.overlay = {OverlayType::RAM, UINT32_MAX, oldWarning, oldCritical};
          auto replay = session.overlay;
          const auto commands = uwf::app::renderPlannedSession(session, changes);
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
  constexpr uint32_t values[] = {0, 512, 1024, 2048, 4096};
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
          for (const auto& command : uwf::app::renderPlannedSession(session, changes)) {
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

}  // namespace

QTEST_APPLESS_MAIN(OverlayPlanningTests)
#include "OverlayPlanningTests.moc"
