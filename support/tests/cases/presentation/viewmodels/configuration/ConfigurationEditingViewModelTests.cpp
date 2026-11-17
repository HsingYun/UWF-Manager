// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "cases/presentation/viewmodels/common/ViewModelStateFixture.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/viewmodels/configuration/ImportViewModel.h"
#include "support/app/ConfigurationFixture.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

class ConfigurationEditingViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void editingDoesNotRequireWidgets();
  void configurationResetPreservesTheSystemBaseline_data();
  void configurationResetPreservesTheSystemBaseline();
  void unrelatedImportsPreserveLargeSystemThresholds();
  void thresholdEditsAndImportsUseTheSameConstraints_data();
  void thresholdEditsAndImportsUseTheSameConstraints();
  void batchImportPreservesUnconstrainedIntentUntilCompletion();
  void thresholdEventsCanBeDisabledIndependently();
  void unavailableStateHidesPendingWithoutDiscardingDraft();
  void editStateOutlivesItsPresentationObject();
  void importReportsAccumulateAndFailuresRetainInput();
  void negativeEditorValuesClampToTheLowerBound();
  void globalImportProducesConstrainedPendingDelta();
  void unavailableGlobalConfigurationRejectsImports();
};

void ConfigurationEditingViewModelTests::editingDoesNotRequireWidgets() {
  presentation::GlobalConfigurationViewModel model(16384);
  model.reset(snapshot().next);
  QSignalSpy pending(&model, &presentation::GlobalConfigurationViewModel::pendingChanged);
  model.editFilterEnabled(false);
  model.editMaximumSize(4096);
  QCOMPARE(pending.count(), 2);
  QCOMPARE(model.pendingFilterEnabled(), std::optional<bool>(false));
  const auto changes = model.pendingOverlay();
  QCOMPARE(changes.maximumSizeMb, std::optional<uint32_t>(4096));
  QVERIFY(!changes.criticalThresholdMb);
  QVERIFY(!changes.warningThresholdMb);
}

void ConfigurationEditingViewModelTests::configurationResetPreservesTheSystemBaseline_data() {
  QTest::addColumn<int>("type");
  QTest::addColumn<uint32_t>("maximum");
  QTest::addColumn<uint32_t>("warning");
  QTest::addColumn<uint32_t>("critical");
  QTest::addColumn<uint32_t>("ram");
  QTest::newRow("default") << int(core::OverlayType::RAM) << 1024u << 512u << 1024u << 16384u;
  QTest::newRow("thresholds-above-size") << int(core::OverlayType::RAM) << 1024u << 1536u << 2048u << 16384u;
  QTest::newRow("critical-event-disabled") << int(core::OverlayType::RAM) << 1024u << 2048u << 0u << 16384u;
  QTest::newRow("both-events-disabled") << int(core::OverlayType::RAM) << 1024u << 0u << 0u << 16384u;
  QTest::newRow("system-size-above-editor-budget") << int(core::OverlayType::RAM) << 8192u << 4096u << 6144u << 4096u;
  QTest::newRow("disk") << int(core::OverlayType::Disk) << 32768u << 8192u << 16384u << 4096u;
  QTest::newRow("observed-warning-above-critical") << int(core::OverlayType::RAM) << 1024u << 500u << 400u << 4096u;
  const auto maximum = std::numeric_limits<uint32_t>::max();
  QTest::newRow("complete-uint32-observation") << int(core::OverlayType::Disk) << maximum << maximum - 1 << maximum << 4096u;
  QTest::newRow("uint32-threshold-with-disabled-critical") << int(core::OverlayType::RAM) << 1024u << maximum << 0u << 4096u;
}

void ConfigurationEditingViewModelTests::configurationResetPreservesTheSystemBaseline() {
  QFETCH(int, type);
  QFETCH(uint32_t, maximum);
  QFETCH(uint32_t, warning);
  QFETCH(uint32_t, critical);
  QFETCH(uint32_t, ram);
  core::SessionSnapshot baseline;
  baseline.filter.enabled = true;
  baseline.overlay = {static_cast<core::OverlayType>(type), maximum, warning, critical};
  presentation::GlobalConfigurationViewModel model(ram);
  model.reset(baseline);
  QCOMPARE(model.draft().overlay.type, baseline.overlay.type);
  QCOMPARE(model.draft().overlay.maximumSizeMb, maximum);
  QCOMPARE(model.draft().overlay.warningThresholdMb, warning);
  QCOMPARE(model.draft().overlay.criticalThresholdMb, critical);
  QVERIFY(model.pendingOverlay().empty());
  QVERIFY(!model.pendingFilterEnabled());
  QVERIFY(model.maximumRange().minimum <= maximum);
  QVERIFY(model.maximumRange().maximum >= maximum);
  QVERIFY(model.warningRange().maximum >= warning);
  QVERIFY(model.criticalRange().maximum >= critical);
  model.editFilterEnabled(false);
  QVERIFY(model.pendingOverlay().empty());
  model.editCriticalThreshold(static_cast<int64_t>(critical) + 1);
  QCOMPARE(model.draft().overlay.maximumSizeMb, maximum);
  model.reset(baseline);
  QVERIFY(!model.pendingFilterEnabled());
  QVERIFY(model.pendingOverlay().empty());
}

void ConfigurationEditingViewModelTests::unrelatedImportsPreserveLargeSystemThresholds() {
  app::ConfigurationEditSession session(8192);
  presentation::GlobalConfigurationViewModel model(session.globalEditor());
  core::SessionSnapshot baseline;
  constexpr auto providerMaximum = std::numeric_limits<uint32_t>::max();
  baseline.overlay = {core::OverlayType::Disk, providerMaximum, providerMaximum - 1, providerMaximum};
  model.reset(baseline);
  model.finishImport();
  QVERIFY(model.pendingOverlay().empty());
  QVERIFY(session.importCommands({}).isEmpty());
  QVERIFY(model.pendingOverlay().empty());
  QCOMPARE(model.warningRange().maximum, providerMaximum - 1);

  session.registryEditor()->setBaseline({}, {});
  const auto parsed = app::parseUwfmgrText(R"(uwfmgr registry add-exclusion HKLM\SOFTWARE\Vendor\Product)");
  const auto reports = session.importCommands(QList<app::UwfmgrCommand>(parsed.begin(), parsed.end()));
  QCOMPARE(reports.size(), 1);
  QCOMPARE(reports.constFirst().status, app::ImportResult::Status::Success);
  QCOMPARE(session.pendingChanges().addRegistryExclusions.size(), std::size_t{1});
  QVERIFY(model.pendingOverlay().empty());
  QCOMPARE(model.draft().overlay.maximumSizeMb, providerMaximum);
  QCOMPARE(model.draft().overlay.warningThresholdMb, providerMaximum - 1);
  QCOMPARE(model.draft().overlay.criticalThresholdMb, providerMaximum);

  model.editMaximumSize(65536);
  const auto changed = model.pendingOverlay();
  QCOMPARE(changed.maximumSizeMb, std::optional<uint32_t>{65536});
  QVERIFY(!changed.warningThresholdMb);
  QVERIFY(!changed.criticalThresholdMb);
}

void ConfigurationEditingViewModelTests::thresholdEditsAndImportsUseTheSameConstraints_data() {
  QTest::addColumn<uint32_t>("warning");
  QTest::addColumn<uint32_t>("critical");
  QTest::addColumn<uint32_t>("expectedWarning");
  QTest::newRow("critical-at-size") << 900u << 1024u << 900u;
  QTest::newRow("thresholds-above-size") << 1536u << 2048u << 1536u;
  QTest::newRow("critical-disabled") << 4096u << 0u << 4096u;
  QTest::newRow("warning-disabled") << 0u << 2048u << 0u;
  QTest::newRow("both-disabled") << 0u << 0u << 0u;
  QTest::newRow("warning-at-critical") << 700u << 700u << 699u;
  QTest::newRow("lowest-enabled-critical") << 1u << 1u << 0u;
  const auto maximum = std::numeric_limits<uint32_t>::max();
  QTest::newRow("complete-uint32-thresholds") << maximum << maximum << maximum - 1;
  QTest::newRow("complete-uint32-disabled-critical") << maximum << 0u << maximum;
}

void ConfigurationEditingViewModelTests::thresholdEditsAndImportsUseTheSameConstraints() {
  QFETCH(uint32_t, warning);
  QFETCH(uint32_t, critical);
  QFETCH(uint32_t, expectedWarning);
  core::SessionSnapshot baseline;
  baseline.overlay = {core::OverlayType::RAM, 1024, 512, 1024};
  presentation::GlobalConfigurationViewModel edited(16384);
  presentation::GlobalConfigurationViewModel imported(16384);
  edited.reset(baseline);
  imported.reset(baseline);
  edited.editCriticalThreshold(critical);
  edited.editWarningThreshold(warning);
  imported.importOverlayWarnMb(warning);
  imported.importOverlayCritMb(critical);
  imported.finishImport();
  for (const auto* model : {&edited, &imported}) {
    QCOMPARE(model->draft().overlay.maximumSizeMb, baseline.overlay.maximumSizeMb);
    QCOMPARE(model->draft().overlay.warningThresholdMb, expectedWarning);
    QCOMPARE(model->draft().overlay.criticalThresholdMb, critical);
    QVERIFY(!model->pendingOverlay().maximumSizeMb);
  }
  QCOMPARE(edited.pendingOverlay().warningThresholdMb, imported.pendingOverlay().warningThresholdMb);
  QCOMPARE(edited.pendingOverlay().criticalThresholdMb, imported.pendingOverlay().criticalThresholdMb);
}

void ConfigurationEditingViewModelTests::batchImportPreservesUnconstrainedIntentUntilCompletion() {
  presentation::GlobalConfigurationViewModel model(16384);
  model.reset(snapshot().next);
  QCOMPARE((model.importOverlayCritMb(70000)).outcome, uwf::app::GlobalEditOutcome::Applied);
  QCOMPARE((model.importOverlayWarnMb(60000)).outcome, uwf::app::GlobalEditOutcome::Applied);
  QCOMPARE((model.importOverlayMaxMb(100000)).outcome, uwf::app::GlobalEditOutcome::Applied);
  QCOMPARE((model.importOverlayType(core::OverlayType::Disk)).outcome, uwf::app::GlobalEditOutcome::Applied);
  QCOMPARE(model.draft().overlay.criticalThresholdMb, uint32_t{70000});
  QCOMPARE(model.draft().overlay.warningThresholdMb, uint32_t{60000});
  model.finishImport();
  QCOMPARE(model.draft().overlay.maximumSizeMb, uint32_t{100000});
  QCOMPARE(model.draft().overlay.criticalThresholdMb, uint32_t{70000});
  QCOMPARE(model.draft().overlay.warningThresholdMb, uint32_t{60000});
  QCOMPARE((model.importOverlayMaxMb(std::numeric_limits<uint32_t>::max())).outcome, uwf::app::GlobalEditOutcome::Applied);
  model.finishImport();
  QCOMPARE(model.draft().overlay.maximumSizeMb, std::numeric_limits<uint32_t>::max());
}

void ConfigurationEditingViewModelTests::thresholdEventsCanBeDisabledIndependently() {
  presentation::GlobalConfigurationViewModel model(16384);
  model.reset(snapshot().next);
  model.editCriticalThreshold(0);
  QCOMPARE(model.draft().overlay.warningThresholdMb, uint32_t{4096});
  QCOMPARE(model.warningRange().maximum, std::numeric_limits<uint32_t>::max());
  model.editWarningThreshold(0);
  model.editCriticalThreshold(1);
  QCOMPARE(model.draft().overlay.warningThresholdMb, uint32_t{0});
  model.editOverlayType(core::OverlayType::Disk);
  model.editMaximumSize(0);
  QCOMPARE(model.draft().overlay.maximumSizeMb, uint32_t{1024});
  model.editOverlayType(core::OverlayType::RAM);
  model.editMaximumSize(0);
  QCOMPARE(model.draft().overlay.maximumSizeMb, uint32_t{1});
}

void ConfigurationEditingViewModelTests::unavailableStateHidesPendingWithoutDiscardingDraft() {
  presentation::GlobalConfigurationViewModel model(16384);
  model.reset(snapshot().next);
  model.editFilterEnabled(false);
  model.setAvailable(false);
  QCOMPARE((model.importFilterEnabled(true)).outcome, uwf::app::GlobalEditOutcome::Rejected);
  QVERIFY(!model.pendingFilterEnabled());
  QVERIFY(model.pendingOverlay().empty());
  model.setAvailable(true);
  QCOMPARE(model.pendingFilterEnabled(), std::optional<bool>(false));
  for (const bool allowed : {false, true}) {
    model.setControlsEnabled(allowed);
    for (const bool available : {false, true}) {
      model.setAvailable(available);
      QCOMPARE(model.controlsEnabled(), available && allowed);
    }
    QCOMPARE(model.pendingFilterEnabled(), std::optional<bool>(false));
  }
}

void ConfigurationEditingViewModelTests::editStateOutlivesItsPresentationObject() {
  app::ConfigurationEditSession session;
  {
    presentation::GlobalConfigurationViewModel model(session.global(), 16384);
    model.reset(snapshot().next);
    model.editFilterEnabled(false);
    model.editMaximumSize(7000);
  }
  presentation::GlobalConfigurationViewModel replacement(session.global(), 16384);
  QCOMPARE(replacement.pendingFilterEnabled(), std::optional<bool>(false));
  QCOMPARE(replacement.pendingOverlay().maximumSizeMb, std::optional<uint32_t>(7000));
  QCOMPARE(session.pendingChanges().setOverlay.maximumSizeMb, std::optional<uint32_t>(7000));
}

void ConfigurationEditingViewModelTests::importReportsAccumulateAndFailuresRetainInput() {
  presentation::ImportViewModel model;
  QSignalSpy warnings(&model, &presentation::ImportViewModel::warningRaised);
  model.setText(QStringLiteral(":: comment"));
  model.importText();
  QCOMPARE(warnings.count(), 1);
  QCOMPARE(model.text(), QStringLiteral(":: comment"));
  int calls = 0;
  model.setApplier([&](const QList<app::UwfmgrCommand>& commands) {
    ++calls;
    QList<app::ImportResult> rows;
    for (const auto& command : commands) {
      app::ImportResult row;
      row.lineText = QString::fromStdString(command.rawLine);
      row.status = calls == 1 ? app::ImportResult::Status::Success : app::ImportResult::Status::Duplicate;
      rows.append(row);
    }
    return rows;
  });
  model.importText();
  QCOMPARE(calls, 0);
  QCOMPARE(warnings.count(), 2);
  for (int batch = 0; batch < 2; ++batch) {
    model.setText(QStringLiteral(":: comment\nuwfmgr filter enable"));
    model.importText();
    QCOMPARE(model.text(), QStringLiteral(":: comment\nuwfmgr filter enable"));
  }
  QCOMPARE(model.totals().batches, 2);
  QCOMPARE(model.totals().success, 1);
  QCOMPARE(model.totals().duplicate, 1);
  QCOMPARE(model.batches().size(), 2);
}

void ConfigurationEditingViewModelTests::negativeEditorValuesClampToTheLowerBound() {
  presentation::GlobalConfigurationViewModel model(16384);
  model.reset(snapshot().next);
  model.editWarningThreshold(-1);
  QCOMPARE(model.draft().overlay.warningThresholdMb, uint32_t{0});
  model.editCriticalThreshold(-1);
  QCOMPARE(model.draft().overlay.criticalThresholdMb, uint32_t{0});
  model.editMaximumSize(-1);
  QCOMPARE(model.draft().overlay.maximumSizeMb, uint32_t{1});
  model.editOverlayType(core::OverlayType::Disk);
  model.editMaximumSize(-1);
  QCOMPARE(model.draft().overlay.maximumSizeMb, uint32_t{1024});
}

void ConfigurationEditingViewModelTests::globalImportProducesConstrainedPendingDelta() {
  presentation::ConfigurationViewModel configuration(16384);
  auto& model = *configuration.global();
  uwf::core::SessionSnapshot current;
  uwf::core::SessionSnapshot next;
  current.filter.enabled = false;
  next.filter.enabled = false;
  next.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  configuration.resetSnapshot(test::application::configurationSnapshot(current, next, {4096, 1024}));

  QVERIFY(!model.pendingFilterEnabled().has_value());
  QVERIFY(model.pendingOverlay().empty());
  QCOMPARE((model.importFilterEnabled(true)).outcome, uwf::app::GlobalEditOutcome::Applied);
  const auto pendingFilter = model.pendingFilterEnabled();
  QVERIFY(pendingFilter.has_value());
  QCOMPARE(pendingFilter.value_or(false), true);
  QCOMPARE((model.importFilterEnabled(true)).outcome, uwf::app::GlobalEditOutcome::Unchanged);

  QCOMPARE((model.importOverlayType(uwf::core::OverlayType::Disk)).outcome, uwf::app::GlobalEditOutcome::Applied);
  QCOMPARE((model.importOverlayMaxMb(2000)).outcome, uwf::app::GlobalEditOutcome::Applied);
  QCOMPARE((model.importOverlayWarnMb(2500)).outcome, uwf::app::GlobalEditOutcome::Applied);
  QCOMPARE((model.importOverlayCritMb(2200)).outcome, uwf::app::GlobalEditOutcome::Applied);
  model.finishImport();

  const auto delta = model.pendingOverlay();
  QVERIFY(delta.type.has_value());
  QVERIFY(delta.maximumSizeMb.has_value());
  QVERIFY(delta.criticalThresholdMb.has_value());
  QVERIFY(delta.warningThresholdMb.has_value());
  QCOMPARE(delta.type.value_or(uwf::core::OverlayType::RAM), uwf::core::OverlayType::Disk);
  QCOMPARE(delta.maximumSizeMb.value_or(0), uint32_t{2000});
  const auto warning = delta.warningThresholdMb.value_or(0);
  const auto critical = delta.criticalThresholdMb.value_or(0);
  const auto maximum = delta.maximumSizeMb.value_or(0);
  QVERIFY(warning < critical);
  QCOMPARE(critical, uint32_t{2200});
  QCOMPARE(maximum, uint32_t{2000});

  configuration.resetSnapshot(test::application::configurationSnapshot(current, next, {4096, 1024}));
  QVERIFY(!model.pendingFilterEnabled().has_value());
  QVERIFY(model.pendingOverlay().empty());
}

void ConfigurationEditingViewModelTests::unavailableGlobalConfigurationRejectsImports() {
  presentation::ConfigurationViewModel configuration(16384);
  auto& model = *configuration.global();
  uwf::core::SessionSnapshot current;
  uwf::core::SessionSnapshot next;
  next.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  configuration.resetSnapshot(test::application::configurationSnapshot(current, next, {4096, 1024}));
  model.setAvailable(false);

  QCOMPARE((model.importFilterEnabled(true)).outcome, uwf::app::GlobalEditOutcome::Rejected);
  QCOMPARE((model.importOverlayType(uwf::core::OverlayType::Disk)).outcome, uwf::app::GlobalEditOutcome::Rejected);
  QCOMPARE((model.importOverlayMaxMb(8192)).outcome, uwf::app::GlobalEditOutcome::Rejected);
  QCOMPARE((model.importOverlayWarnMb(4096)).outcome, uwf::app::GlobalEditOutcome::Rejected);
  QCOMPARE((model.importOverlayCritMb(6144)).outcome, uwf::app::GlobalEditOutcome::Rejected);
  QVERIFY(!model.pendingFilterEnabled().has_value());
  QVERIFY(model.pendingOverlay().empty());
}

}  // namespace

QTEST_GUILESS_MAIN(ConfigurationEditingViewModelTests)
#include "ConfigurationEditingViewModelTests.moc"
