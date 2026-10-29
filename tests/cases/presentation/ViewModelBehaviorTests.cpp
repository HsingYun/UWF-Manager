// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractItemModelTester>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QPersistentModelIndex>
#include <QSignalSpy>
#include <QStringListModel>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <algorithm>
#include <array>
#include <atomic>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <thread>

#include "app/commit/CommitBatch.h"
#include "app/diagnostics/ProcessLogSource.h"
#include "app/paths/PathPolicy.h"
#include "app/ports/FileStagingStore.h"
#include "app/staging/FileStagingEditor.h"
#include "app/wmi/WmiException.h"
#include "presentation/commit/CommitReportViewModel.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PageProxyModel.h"
#include "presentation/configuration/ApplyPlanViewModel.h"
#include "presentation/configuration/ConfigurationViewModel.h"
#include "presentation/configuration/ImportViewModel.h"
#include "presentation/configuration/VolumeConfigurationViewModel.h"
#include "presentation/diagnostics/LogViewModel.h"
#include "presentation/enhanced/EnhancedModeViewModel.h"
#include "presentation/overlay/OverlayFilesViewModel.h"
#include "presentation/overlay/OverlayViewModel.h"
#include "presentation/power/PowerDialogViewModel.h"
#include "presentation/registry/RegistryPickerViewModel.h"
#include "presentation/shell/MainWindowViewModel.h"
#include "presentation/staging/FileStagingViewModel.h"
#include "support/app/WorkerGate.h"
#include "windows/commit/WindowsCommitTraversalSource.h"

namespace {
using namespace uwf;

core::UwfSnapshot snapshot() {
  core::UwfSnapshot result;
  result.uwfAvailable = true;
  result.elevated = true;
  result.next.filter.enabled = true;
  result.next.overlay = {core::OverlayType::RAM, 8192, 4096, 6144};
  result.next.volumes = {{"Volume{c}", "C:", true, true}, {"Volume{d}", "D:", false, true}};
  result.current = result.next;
  return result;
}

core::VolumeInfo disk(const std::string& letter = "C:") {
  return {letter, letter == "C:" ? "Volume{c}" : "Volume{d}", "NTFS", "Data", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported};
}

class FakeConfigurationActions final : public app::ConfigurationActions {
 public:
  app::ConfigurationApplyResult result;
  std::optional<core::UwfSnapshot> observedSnapshot;
  int calls = 0;
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot& snapshot) override {
    ++calls;
    observedSnapshot = snapshot;
    return [value = result] { return value; };
  }
};
class MemoryStagingStore final : public app::FileStagingStore {
 public:
  QList<app::FileStagingEntry> entries;
  bool failRead = false;
  bool failAfterReplace = false;
  bool failReadAfterReplace = false;
  int writes = 0;
  QList<app::FileStagingEntry> load() const override {
    if (failRead) throw std::runtime_error("read failed");
    return entries;
  }
  void replace(const QList<app::FileStagingEntry>& value) override {
    ++writes;
    entries = value;
    if (failReadAfterReplace) failRead = true;
    if (failAfterReplace) throw std::runtime_error("persistence incomplete");
  }
};
class FakeEnhancedActions final : public app::EnhancedModeActions {
 public:
  void setAgentState(app::EnhancedModeAgentState state) override {
    current.agentState = state;
    emit agentConnectionStateChanged(state);
  }

  app::EnhancedModeStatus current;
  bool failRead = false;
  bool failWrite = false;
  bool failPublish = false;
  QString persistenceWarning;
  QString lastDescription;
  int starts = 0;
  int enables = 0;
  int disables = 0;
  std::shared_ptr<test::WorkerGate> gate;
  app::EnhancedModeStatus status() const override {
    if (failRead) throw std::runtime_error("SCM unavailable");
    return current;
  }
  app::EnhancedModeChangeResult enable(const QString& description) override {
    ++enables;
    lastDescription = description;
    return change(app::EnhancedModeState::Enabled);
  }
  app::EnhancedModeChangeResult start() override {
    ++starts;
    return change(app::EnhancedModeState::Enabled);
  }
  app::EnhancedModeChangeResult disable() override {
    ++disables;
    return change(app::EnhancedModeState::Disabled);
  }
  ChangeWork prepareChange(const app::EnhancedModeAction action, const QString& description) override {
    if (action == app::EnhancedModeAction::Disable)
      ++disables;
    else if (action == app::EnhancedModeAction::Start)
      ++starts;
    else {
      ++enables;
      lastDescription = description;
    }
    auto next = current;
    next.state = action == app::EnhancedModeAction::Disable ? app::EnhancedModeState::Disabled : app::EnhancedModeState::Enabled;
    return [failed = failWrite, gate = gate, result = app::EnhancedModeChangeResult{next, persistenceWarning}] {
      if (gate) gate->wait();
      if (failed) throw std::runtime_error("write failed");
      return result;
    };
  }
  void acceptChangeResult(app::EnhancedModeChangeResult& result) override {
    current = result.status;
    if (failPublish) throw std::runtime_error("agent startup failed");
    emit statusChanged(current);
  }

 private:
  app::EnhancedModeChangeResult change(app::EnhancedModeState state) {
    if (failWrite) throw std::runtime_error("write failed");
    current.state = state;
    return {current, persistenceWarning};
  }
};
class FakeUsageReader final : public app::OverlayUsageReader {
 public:
  app::OverlayUsageState next{app::OverlayUsageDisabled{}};
  mutable int calls = 0;
  bool fail = false;
  app::OverlayUsageState read() const override {
    ++calls;
    if (fail) throw std::runtime_error("incomplete read");
    return next;
  }
};
class FakeStateSource final : public app::ApplicationStateSource {
 public:
  app::ApplicationState next;
  bool fail = false;
  ReadOperation readOperation(uwf::app::UwfCapability) override {
    return [candidate = next, failed = fail](std::stop_token) {
      if (failed) throw std::runtime_error("snapshot failed");
      return candidate;
    };
  }
};
class FakeRegistryBrowser final : public app::RegistryBrowser {
 public:
  std::string deniedChildren;
  mutable std::string lastValueKey;
  bool denyValues = false;
  std::vector<std::string> rootHiveLongNames() const override { return {"HKEY_LOCAL_MACHINE"}; }
  std::vector<std::string> subkeyNames(std::string_view key) const override {
    if (key == deniedChildren) throw std::runtime_error("拒绝访问");
    return {"SOFTWARE", "PERSISTENT_INTERNAL", "Restricted"};
  }
  bool hasSubkeys(std::string_view key) const override {
    if (key.ends_with("Restricted")) throw std::system_error(std::make_error_code(std::errc::permission_denied));
    return true;
  }
  std::vector<uwf::app::RegistryValueInfo> values(std::string_view key) const override {
    lastValueKey = key;
    if (denyValues) throw std::runtime_error("拒绝访问");
    return {{"Named", 1u, std::vector<uint8_t>{}}};
  }
};

class ViewModelBehaviorTests final : public QObject {
  Q_OBJECT
 private slots:
  void applyRetryAndReconciliationDoNotReplayWrites();
  void configurationReviewUsesTheNextSessionBaselineAndRetainsNonCliChanges();
  void configurationReviewIdentifiesStagingDependencies_data();
  void configurationReviewIdentifiesStagingDependencies();
  void stagingRegistryTooltipExplainsTheDependencyWithoutChangingRemovalRules();
  void importReportsAccumulateAndFailuresRetainInput();
  void stagingEditsPreserveOtherVolumesAndReloadPartialWrites();
  void stagingAvailabilityKeepsReadOnlyAndEnhancedIndependent();
  void stagingRowsProjectTheCompleteDraftWithoutReadSideEffects();
  void enhancedModeRetainsFeedbackAcrossAgentChanges();
  void enhancedModeWorkersRespectReceiverLifetimes_data();
  void enhancedModeWorkersRespectReceiverLifetimes();
  void enhancedModePublicationFailureIsReportedWithoutEscapingTheEventLoop();
  void enhancedModeStatusProjection_data();
  void enhancedModeStatusProjection();
  void overlayPublishesOnlyCompleteSuccessfulReads();
  void currentUsageIsIndependentOfPendingConfiguration();
  void powerPreparationAndCommitRespectTimingAndCancelBoundaries();
  void powerDirectAndCompletedFailureDoNotRepeatPreparation();
  void powerFailureContinuationRequiresRecordedTargets_data();
  void powerFailureContinuationRequiresRecordedTargets();
  void windowStateRetainsCommittedBaselineAfterReadFailure();
  void windowReconciliationRequiresEveryWrittenStoreToBeReadable_data();
  void windowReconciliationRequiresEveryWrittenStoreToBeReadable();
  void windowRefreshObserversMayDestroyPresentation();
  void windowStatusBindingSurvivesItsSessionAndCanBeReplaced();
  void registryModelRetainsDefaultValueAndRestrictedChildSemantics();
  void registryAccessErrorsTrackTheRequestedKeyAndRecover();
  void registryNavigationResolvesAccessibleAncestors_data();
  void registryNavigationResolvesAccessibleAncestors();
  void overlayFileModelNormalizesStreamsAndCancelsItsWorker();
  void overlayFilesCancellationRemainsSilent();
  void reportModelPreservesAllRowsAcrossPages();
  void commitBatchesKeepCallbacksLazyAndObserveTheOriginalTargets();
  void negativeEditorValuesClampToTheLowerBound();
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
  void volumeBaselinesKeepCurrentOnlyAndMissingSemantics();
  void globalImportProducesConstrainedPendingDelta();
  void unavailableGlobalConfigurationRejectsImports();
  void fileExclusionImportMaintainsCaseInsensitiveDeltas();
  void fileExclusionImportRejectsWrongVolumeAndUwfBlacklist();
  void registryExclusionImportsFollowTheEditingPolicy();
  void volumeEditsTrackOnlyChangesFromTheBaseline();
  void volumeCommitAvailability_data();
  void volumeCommitAvailability();
  void shellVolumeProjection_data();
  void shellVolumeProjection();
  void oneEditSessionCollectsEveryEntryPoint();
  void registryProtectionRefreshesWhenStagingChanges();
  void failedProbeProtectsTheWholeSelection();
  void itemModelsHonorTheQtContractAcrossUpdates();
  void pageProxyPreservesSourceMappingAndIndependentViewState();
  void pageProxyTracksSourceChangesAndDetachment();
  void pageProxyTracksReorderingAndSourceDestruction();
  void stagingStorageRecoveryClearsErrorsAndPublishesAvailability();
  void unicodePathsShareExclusionAndStagingDecisions();
  void exclusionCaseAndReversalSemanticsRemainStable();
  void exclusionKindsResolveAgainstTheSelectedVolume();
};

void ViewModelBehaviorTests::exclusionKindsResolveAgainstTheSelectedVolume() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const auto folder = directory.filePath(QStringLiteral("folder.with.dot"));
  const auto file = directory.filePath(QStringLiteral("extensionless"));
  QVERIFY(QDir().mkpath(folder));
  QFile output(file);
  QVERIFY(output.open(QIODevice::WriteOnly));
  output.close();
  presentation::ExclusionListViewModel model(app::ExclusionEditor::Kind::File);
  const auto nativeFolder = QDir::toNativeSeparators(folder);
  const auto nativeFile = QDir::toNativeSeparators(file);
  model.setDriveLetter(nativeFolder.left(2));
  model.setBaseline({nativeFolder.mid(2), nativeFile.mid(2)}, {nativeFolder.mid(2), nativeFile.mid(2)});
  int folders = 0, files = 0;
  for (int row = 0; row < model.rowCount(); ++row) {
    const auto index = model.index(row);
    const auto icon = index.data(presentation::IconResourceRole).toString();
    if (icon == QStringLiteral(":/icons/folder.svg")) ++folders;
    if (icon == QStringLiteral(":/icons/file.svg")) ++files;
  }
  QCOMPARE(folders, 1);
  QCOMPARE(files, 1);
}

void ViewModelBehaviorTests::editingDoesNotRequireWidgets() {
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

void ViewModelBehaviorTests::configurationResetPreservesTheSystemBaseline_data() {
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
}

void ViewModelBehaviorTests::configurationResetPreservesTheSystemBaseline() {
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
  QVERIFY(model.maximumRange().minimum <= int(maximum));
  QVERIFY(model.maximumRange().maximum >= int(maximum));
  QVERIFY(model.warningRange().maximum >= int(warning));
  QVERIFY(model.criticalRange().maximum >= int(critical));
  model.editFilterEnabled(false);
  QVERIFY(model.pendingOverlay().empty());
  model.editCriticalThreshold(static_cast<int>(critical + 1));
  QCOMPARE(model.draft().overlay.maximumSizeMb, maximum);
  model.reset(baseline);
  QVERIFY(!model.pendingFilterEnabled());
  QVERIFY(model.pendingOverlay().empty());
}

void ViewModelBehaviorTests::unrelatedImportsPreserveLargeSystemThresholds() {
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
  QCOMPARE(model.warningRange().maximum, std::numeric_limits<int>::max());

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

void ViewModelBehaviorTests::thresholdEditsAndImportsUseTheSameConstraints_data() {
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
}

void ViewModelBehaviorTests::thresholdEditsAndImportsUseTheSameConstraints() {
  QFETCH(uint32_t, warning);
  QFETCH(uint32_t, critical);
  QFETCH(uint32_t, expectedWarning);
  core::SessionSnapshot baseline;
  baseline.overlay = {core::OverlayType::RAM, 1024, 512, 1024};
  presentation::GlobalConfigurationViewModel edited(16384);
  presentation::GlobalConfigurationViewModel imported(16384);
  edited.reset(baseline);
  imported.reset(baseline);
  edited.editCriticalThreshold(static_cast<int>(critical));
  edited.editWarningThreshold(static_cast<int>(warning));
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

void ViewModelBehaviorTests::batchImportPreservesUnconstrainedIntentUntilCompletion() {
  presentation::GlobalConfigurationViewModel model(16384);
  model.reset(snapshot().next);
  QVERIFY(model.importOverlayCritMb(70000));
  QVERIFY(model.importOverlayWarnMb(60000));
  QVERIFY(model.importOverlayMaxMb(100000));
  QVERIFY(model.importOverlayType(core::OverlayType::Disk));
  QCOMPARE(model.draft().overlay.criticalThresholdMb, uint32_t{70000});
  QCOMPARE(model.draft().overlay.warningThresholdMb, uint32_t{60000});
  model.finishImport();
  QCOMPARE(model.draft().overlay.maximumSizeMb, uint32_t{100000});
  QCOMPARE(model.draft().overlay.criticalThresholdMb, uint32_t{70000});
  QCOMPARE(model.draft().overlay.warningThresholdMb, uint32_t{60000});
  QVERIFY(model.importOverlayMaxMb(std::numeric_limits<uint32_t>::max()));
  model.finishImport();
  QCOMPARE(model.draft().overlay.maximumSizeMb, static_cast<uint32_t>(std::numeric_limits<int>::max()));
}

void ViewModelBehaviorTests::thresholdEventsCanBeDisabledIndependently() {
  presentation::GlobalConfigurationViewModel model(16384);
  model.reset(snapshot().next);
  model.editCriticalThreshold(0);
  QCOMPARE(model.draft().overlay.warningThresholdMb, uint32_t{4096});
  QCOMPARE(model.warningRange().maximum, std::numeric_limits<int>::max());
  model.editWarningThreshold(0);
  model.editCriticalThreshold(1);
  QCOMPARE(model.draft().overlay.warningThresholdMb, uint32_t{0});
  model.editOverlayType(core::OverlayType::Disk);
  model.editMaximumSize(0);
  QCOMPARE(model.draft().overlay.maximumSizeMb, uint32_t{1024});
}

void ViewModelBehaviorTests::unavailableStateHidesPendingWithoutDiscardingDraft() {
  presentation::GlobalConfigurationViewModel model(16384);
  model.reset(snapshot().next);
  model.editFilterEnabled(false);
  model.setAvailable(false);
  QVERIFY(!model.importFilterEnabled(true));
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

void ViewModelBehaviorTests::editStateOutlivesItsPresentationObject() {
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

void ViewModelBehaviorTests::volumeEditsTrackOnlyChangesFromTheBaseline() {
  presentation::VolumeStatusViewModel model;
  const core::VolumeRecord current{"Volume{A}", "C:", true, true};
  const core::VolumeRecord next{"Volume{A}", "C:", false, false};
  model.reset(&current, &next);

  QVERIFY(!model.pendingProtection());
  QVERIFY(!model.pendingBinding());
  QVERIFY(!model.editProtection(false));
  QVERIFY(model.editProtection(true));
  QCOMPARE(model.pendingProtection(), std::optional<bool>(true));
  QVERIFY(model.editProtection(false));
  QVERIFY(!model.pendingProtection());

  QVERIFY(!model.editBinding(true));
  QVERIFY(model.editBinding(false));
  QCOMPARE(model.pendingBinding(), std::optional<bool>(false));

  model.reset(nullptr, nullptr);
  QVERIFY(!model.editProtection(true));
  QVERIFY(!model.pendingProtection());
  QVERIFY(!model.pendingBinding());
}

void ViewModelBehaviorTests::volumeBaselinesKeepCurrentOnlyAndMissingSemantics() {
  presentation::VolumeStatusViewModel model;
  QSignalSpy availability(&model, &presentation::VolumeStatusViewModel::availabilityChanged);
  const core::VolumeRecord current{"Volume{c}", "C:", true, false};
  model.reset(&current, nullptr);
  QVERIFY(!model.pendingProtection());
  QVERIFY(!model.pendingBinding());
  QVERIFY(model.editProtection(false));
  QCOMPARE(model.pendingProtection(), std::optional<bool>(false));
  model.reset(nullptr, nullptr);
  QVERIFY(!model.pendingProtection());
  QVERIFY(!model.editProtection(true));
  QVERIFY(!model.draft().protectedVolume);
  model.setUnsupported();
  QCOMPARE(availability.count(), 3);
  QVERIFY(!model.controlsEnabled());
  model.setUnsupported();
  QCOMPARE(availability.count(), 3);
  QVERIFY(!model.editBinding(true));
}

void ViewModelBehaviorTests::oneEditSessionCollectsEveryEntryPoint() {
  presentation::ConfigurationViewModel model(16384);
  const auto state = snapshot();
  model.global()->reset(state.next);
  auto first = model.addVolume(disk(), true);
  auto second = model.addVolume(disk("D:"), false);
  first->applySnapshot(state);
  second->applySnapshot(state);
  const auto parsed = app::parseUwfmgrText("uwfmgr.exe filter disable\nuwfmgr.exe volume protect D:\nuwfmgr.exe volume unprotect C:");
  QList<app::UwfmgrCommand> commands;
  for (const auto& command : parsed) commands.append(command);
  const auto report = model.importCommands(commands);
  QCOMPARE(report.size(), 3);
  for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
  const auto pending = model.pendingChanges();
  QCOMPARE(pending.setFilterEnabled, std::optional<bool>(false));
  QCOMPARE(pending.volumeProtect.at("C:"), false);
  QCOMPARE(pending.volumeProtect.at("D:"), true);
  QVERIFY(first->status()->editProtection(true));
  QVERIFY(!model.pendingChanges().volumeProtect.contains("C:"));
}

void ViewModelBehaviorTests::registryProtectionRefreshesWhenStagingChanges() {
  presentation::ConfigurationViewModel model(16384);
  auto state = snapshot();
  const auto key = std::string(app::kFileStagingRegistryRoot);
  state.current.registryExclusions = {key};
  state.next.registryExclusions = {key};
  model.global()->reset(state.next);
  auto volume = model.addVolume(disk(), true);
  volume->applySnapshot(state);
  bool savedEntries = false;
  volume->registry()->setFileStagingDataProbe([&] { return savedEntries; });
  QCOMPARE(volume->registry()->importRemove(QString::fromStdString(key)), app::ExclusionImportOutcome::Applied);
  QVERIFY(!model.pendingChanges().removeRegistryExclusions.empty());
  savedEntries = true;
  model.refreshStagingConstraints();
  QVERIFY(model.pendingChanges().removeRegistryExclusions.empty());
  QVERIFY(volume->registry()->draft().removed.isEmpty());
}

void ViewModelBehaviorTests::failedProbeProtectsTheWholeSelection() {
  presentation::ExclusionListViewModel model(presentation::ExclusionListViewModel::Kind::Registry);
  const QString root = QString::fromStdString(std::string(app::kFileStagingRegistryRoot));
  const QString other = QStringLiteral("HKLM\\SOFTWARE\\Vendor");
  model.setBaseline({root, other}, {root, other});
  model.setFileStagingDataProbe([]() -> bool { throw std::runtime_error("mock storage failure"); });
  const QVector<presentation::ExclusionListViewModel::Selection> selection{{root, std::nullopt}, {other, std::nullopt}};
  QVERIFY(!model.removeSelection(selection));
  QVERIFY(model.pendingRemoved().isEmpty());
}

void ViewModelBehaviorTests::exclusionCaseAndReversalSemanticsRemainStable() {
  presentation::ExclusionListViewModel model(presentation::ExclusionListViewModel::Kind::Registry);
  model.setBaseline({}, {QStringLiteral("HKLM\\SOFTWARE\\Vendor")});
  QCOMPARE(model.importRemove(QStringLiteral("hklm\\software\\vendor")), app::ExclusionImportOutcome::Applied);
  QCOMPARE(model.pendingRemoved().size(), 1);
  QCOMPARE(model.importAdd(QStringLiteral("HKLM\\SOFTWARE\\VENDOR")), app::ExclusionImportOutcome::Applied);
  QVERIFY(model.pendingRemoved().isEmpty());
  QVERIFY(model.pendingAdded().isEmpty());
  const QString path = model.draft().next.constFirst();
  model.setPersistBaseline(true, true, true, true);
  const QVector<presentation::ExclusionListViewModel::Selection> selected{{path, std::nullopt},
                                                                          {QString(), presentation::ExclusionListViewModel::PersistenceItem::DomainSecretKey},
                                                                          {QString(), presentation::ExclusionListViewModel::PersistenceItem::Tscal}};
  QVERIFY(model.removeSelection(selected));
  QCOMPARE(model.pendingRemoved().size(), 1);
  QCOMPARE(model.pendingPersistDomainSecretKey(), std::optional<bool>(false));
  QCOMPARE(model.pendingPersistTSCAL(), std::optional<bool>(false));
  model.setReadOnly(true);
  QVERIFY(!model.restoreSelection(selected));
  QCOMPARE(model.pendingRemoved().size(), 1);
  model.setReadOnly(false);
  QVERIFY(model.restoreSelection(selected));
  QVERIFY(model.pendingRemoved().isEmpty());
  QVERIFY(!model.pendingPersistDomainSecretKey().has_value());
  QVERIFY(!model.pendingPersistTSCAL().has_value());
}

void ViewModelBehaviorTests::applyRetryAndReconciliationDoNotReplayWrites() {
  const auto state = snapshot();
  auto actions = std::make_shared<FakeConfigurationActions>();
  core::PendingChanges changes;
  changes.setFilterEnabled = false;
  presentation::ApplyPlanViewModel model(changes, state, actions);
  connect(&model, &presentation::ApplyPlanViewModel::applyingChanged, &model, [&] {
    if (model.applying()) model.apply();
  });
  actions->result.connectionEstablished = false;
  model.apply();
  QTRY_VERIFY(!model.applying());
  QCOMPARE(actions->calls, 1);
  QVERIFY(model.canApply());
  QStringList notifications;
  connect(&model, &presentation::ApplyPlanViewModel::resultChanged, &model, [&] { notifications.append(QStringLiteral("result")); });
  connect(&model, &presentation::ApplyPlanViewModel::reconciliationRequired, &model, [&] { notifications.append(QStringLiteral("reconcile")); });
  actions->result.connectionEstablished = true;
  actions->result.reconciliationRequired = true;
  model.apply();
  QTRY_VERIFY(!model.applying());
  // The owner must install its reconciliation gate before result observers
  // can expose actions again.
  QCOMPARE(notifications, QStringList({QStringLiteral("reconcile"), QStringLiteral("result")}));
  QVERIFY(!model.canApply());
  model.apply();
  QTRY_VERIFY(!model.applying());
  QCOMPARE(actions->calls, 2);
}
void ViewModelBehaviorTests::configurationReviewUsesTheNextSessionBaselineAndRetainsNonCliChanges() {
  auto state = snapshot();
  state.current.filter.enabled = false;
  state.next.filter.enabled = true;
  state.current.overlay.maximumSizeMb = 1024;
  state.next.overlay.maximumSizeMb = 2048;
  state.next.volumes = {{"Volume{c}", "C:", true, true}};
  core::PendingChanges changes;
  changes.setFilterEnabled = false;
  changes.setOverlay.maximumSizeMb = 4096;
  changes.volumeProtect["C:"] = false;
  changes.volumeBindByVolumeName["C:"] = true;
  changes.addFileExclusions["C:"] = {"C:\\<literal>&data"};
  changes.removeFileExclusions["C:"] = {"C:\\remove"};
  changes.addRegistryExclusions = {"HKLM\\SOFTWARE\\Vendor"};
  changes.setPersistDomainSecretKey = true;
  changes.setPersistTSCAL = true;
  auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(changes, state, actions);
  QCOMPARE(model.groups().size(), 3);
  const auto& protection = model.groups()[0].items;
  QCOMPARE(protection[0].before, QStringLiteral("On"));
  QCOMPARE(protection[0].after, QStringLiteral("Off"));
  QCOMPARE(protection[1].before, QStringLiteral("2,048 MB"));
  QCOMPARE(protection[1].after, QStringLiteral("4,096 MB"));
  QCOMPARE(protection[3].before, QStringLiteral("By drive letter"));
  QCOMPARE(protection[3].after, QStringLiteral("By volume ID"));
  QCOMPARE(model.groups()[1].items[0].title, QStringLiteral("C:\\<literal>&data"));
  qsizetype reviewed = 0;
  for (const auto& group : model.groups()) reviewed += group.items.size();
  QCOMPARE(static_cast<std::size_t>(reviewed), changes.count());
  const auto target = model.targetCommandsText();
  state.next.overlay.maximumSizeMb = 512;
  state.next.filter.enabled = false;
  QCOMPARE(model.targetCommandsText(), target);
  QCOMPARE(protection[0].before, QStringLiteral("On"));
  QCOMPARE(actions->calls, 0);
  QVERIFY(model.changeCommandsText().contains(QStringLiteral("filter disable")));
  QVERIFY(model.changeCommandsText().startsWith(QString::fromUtf8(app::kUwfmgrScriptMarker.data(), static_cast<qsizetype>(app::kUwfmgrScriptMarker.size()))));
  model.apply();
  QTRY_VERIFY(!model.applying());
  QCOMPARE(actions->calls, 1);
  QVERIFY(actions->observedSnapshot.has_value());
  QCOMPARE(actions->observedSnapshot->next.overlay.maximumSizeMb, uint32_t{2048});
  QVERIFY(actions->observedSnapshot->next.filter.enabled);
}

void ViewModelBehaviorTests::configurationReviewIdentifiesStagingDependencies_data() {
  QTest::addColumn<QString>("key");
  QTest::addColumn<bool>("activeStaging");
  QTest::addColumn<bool>("added");
  QTest::addColumn<bool>("explicitChange");
  QTest::addColumn<bool>("builtIn");
  QTest::addColumn<int>("registryRows");
  const auto root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), app::kFileStagingRegistryRoot.size());
  QTest::newRow("automatic-root") << root << true << true << false << true << 1;
  QTest::newRow("explicit-root-with-staging") << root << true << true << true << true << 1;
  QTest::newRow("mixed-case-hive-alias") << QStringLiteral("hKlM\\software\\hsingyun\\uwf manager") << true << true << true << true << 1;
  QTest::newRow("manual-root-without-staging") << root << false << true << true << false << 1;
  QTest::newRow("sibling") << root + QStringLiteral(" Backup") << true << true << true << false << 2;
  QTest::newRow("descendant") << root + QStringLiteral("\\Other") << true << true << true << false << 2;
  QTest::newRow("covering-parent") << QStringLiteral("HKLM\\SOFTWARE\\HsingYun") << true << true << true << false << 1;
  QTest::newRow("clear-staging-and-remove-root") << root << false << false << true << false << 1;
}

void ViewModelBehaviorTests::configurationReviewIdentifiesStagingDependencies() {
  QFETCH(QString, key);
  QFETCH(bool, activeStaging);
  QFETCH(bool, added);
  QFETCH(bool, explicitChange);
  QFETCH(bool, builtIn);
  QFETCH(int, registryRows);
  const QString dependency = QStringLiteral("File staging relies on this registry exclusion to preserve its list across restarts.");
  auto state = snapshot();
  core::PendingChanges changes;
  if (explicitChange) {
    if (added) {
      app::ExclusionEditor editor(app::ExclusionEditor::Kind::Registry);
      const auto addition = editor.addEntry(key);
      QCOMPARE(addition.outcome, app::ExclusionImportOutcome::Applied);
      key = addition.path;
    }
    (added ? changes.addRegistryExclusions : changes.removeRegistryExclusions).push_back(key.toStdString());
  }
  std::optional<app::FileStagingChanges> staging;
  if (activeStaging || !added) {
    staging.emplace();
    auto& entries = activeStaging ? staging->after : staging->before;
    entries.append({app::FileStagingKind::File, QStringLiteral("C:\\Data\\settings.json")});
  }
  if (!added) state.current.registryExclusions = state.next.registryExclusions = {key.toStdString()};
  auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(changes, state, actions, staging);
  const auto group = std::ranges::find_if(model.groups(), [](const auto& value) { return value.title == QStringLiteral("Registry exclusions"); });
  QVERIFY(group != model.groups().end());
  QCOMPARE(group->items.size(), registryRows);
  QModelIndex entry;
  for (int row = 0; row < model.rowCount(); ++row) {
    const auto index = model.index(row, 0);
    if (index.data(presentation::ApplyPlanViewModel::KindRole).toInt() == presentation::ApplyPlanViewModel::Entry && index.data().toString() == key)
      entry = index;
  }
  QVERIFY(entry.isValid());
  const QString badge = builtIn ? QStringLiteral("Built-in") : added ? QStringLiteral("Added") : QStringLiteral("Removed");
  const QString description = builtIn ? dependency
                             : added ? QStringLiteral("Future writes to this key will not be protected by UWF.")
                                     : QStringLiteral("Remove this key from the next-session exclusions.");
  QCOMPARE(entry.data(presentation::ApplyPlanViewModel::AfterRole).toString(), badge);
  QCOMPARE(entry.data(presentation::ApplyPlanViewModel::DescriptionRole).toString(), description);
  QVERIFY(entry.data(Qt::AccessibleTextRole).toString().contains(badge));
  QVERIFY(entry.data(Qt::AccessibleTextRole).toString().contains(description));
  QVERIFY(entry.data(presentation::ApplyPlanViewModel::PathRole).toBool());
  QCOMPARE(actions->calls, 0);
}

void ViewModelBehaviorTests::stagingRegistryTooltipExplainsTheDependencyWithoutChangingRemovalRules() {
  presentation::ExclusionListViewModel model(app::ExclusionEditor::Kind::Registry);
  const QString root = QStringLiteral("hklm\\software\\hsingyun\\uwf manager");
  const QString sibling = root + QStringLiteral(" Backup");
  model.setBaseline({}, {root, sibling});
  const QString dependency = QStringLiteral("File staging relies on this registry exclusion to preserve its list across restarts.");
  bool required = true;
  model.setFileStagingDataProbe([&] { return required; });
  for (int row = 0; row < model.rowCount(); ++row) {
    const auto index = model.index(row, 0);
    const bool stagingRoot = index.data(presentation::PathRole).toString().endsWith(QStringLiteral("uwf manager"));
    QCOMPARE(index.data(presentation::RequiredRole).toBool(), stagingRoot);
    QCOMPARE(index.data(Qt::ToolTipRole).toString().contains(dependency), stagingRoot);
    QCOMPARE(index.data(presentation::BadgeRole).toInt(), static_cast<int>(presentation::ItemBadge::Add));
  }
  QVERIFY(!model.canRemoveSelection({root}));
  QVERIFY(model.canRemoveSelection({sibling}));
  required = false;
  model.editor()->refreshFileStagingConstraints();
  QVERIFY(model.canRemoveSelection({root}));
  QCOMPARE(model.importRemove(root), app::ExclusionImportOutcome::Applied);
}

void ViewModelBehaviorTests::importReportsAccumulateAndFailuresRetainInput() {
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
void ViewModelBehaviorTests::stagingEditsPreserveOtherVolumesAndReloadPartialWrites() {
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\one.txt")}, {app::FileStagingKind::File, QStringLiteral("D:\\two.txt")}};
  app::FileStagingDraft draft(storeOwner);
  auto editing = std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, uwf::windows::systemCommitTraversalSource());
  presentation::FileStagingViewModel model(editing);
  model.load();
  QCOMPARE(model.entries().size(), 1);
  QSignalSpy warnings(&model, &presentation::FileStagingViewModel::warningRaised);
  store.failAfterReplace = true;
  model.removePaths({QStringLiteral("c:\\ONE.txt")});
  QCOMPARE(store.writes, 0);
  QCOMPARE(store.entries.size(), 2);
  QCOMPARE(draft.load().size(), 1);
  QCOMPARE(draft.load().constFirst().path, QStringLiteral("D:\\two.txt"));
  QVERIFY(model.entries().isEmpty());
  QVERIFY(model.storageAvailable());
  QCOMPARE(warnings.count(), 0);
  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(model.index(0, 0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Removed));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, draft.apply(*draft.changes()));
  QVERIFY(draft.changes());
  store.failAfterReplace = false;
  draft.apply(*draft.changes());
  QVERIFY(!draft.changes());
  QCoreApplication::processEvents();
  QCOMPARE(model.rowCount(), 0);
  store.failRead = true;
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, draft.reload());
  QVERIFY(model.storageAvailable());
}
void ViewModelBehaviorTests::stagingRowsProjectTheCompleteDraftWithoutReadSideEffects() {
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  for (int index = 0; index < 2000; ++index)
    store.entries.append({index % 2 ? app::FileStagingKind::Directory : app::FileStagingKind::File, QStringLiteral("C:/Original/item-%1").arg(index)});
  app::FileStagingDraft draft(storeOwner);
  presentation::FileStagingViewModel model(std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, uwf::windows::systemCommitTraversalSource()));
  model.load();
  QAbstractItemModelTester contract(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
  QSignalSpy resets(&model, &QAbstractItemModel::modelReset);
  auto next = store.entries.mid(700);
  for (auto& entry : next) entry.path = entry.path.toUpper().replace(u'/', u'\\');
  for (int index = 0; index < 500; ++index) next.append({app::FileStagingKind::File, QStringLiteral("C:/New/item-%1").arg(index)});
  draft.replace(next);
  QTRY_COMPARE(resets.count(), 1);
  QCOMPARE(model.rowCount(), 2500);
  QCOMPARE(model.summaryText(), QStringLiteral("1800 staged entries · 1150 files · 650 folders"));
  for (int pass = 0; pass < 3; ++pass) {
    int added = 0, removed = 0, retained = 0;
    for (int row = 0; row < model.rowCount(); ++row) {
      const auto index = model.index(row);
      const auto status = static_cast<presentation::ItemStatus>(index.data(presentation::StatusRole).toInt());
      if (status == presentation::ItemStatus::Added)
        ++added;
      else if (status == presentation::ItemStatus::Removed)
        ++removed;
      else
        ++retained;
      QVERIFY(index.data(presentation::PathRole).isValid());
      QVERIFY(index.data(presentation::BadgeRole).isValid());
    }
    QCOMPARE(added, 500);
    QCOMPARE(removed, 700);
    QCOMPARE(retained, 1300);
  }
  QCOMPARE(resets.count(), 1);
  QCOMPARE(store.writes, 0);
}

void ViewModelBehaviorTests::stagingAvailabilityKeepsReadOnlyAndEnhancedIndependent() {
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\one.txt")}};
  app::FileStagingDraft draft(storeOwner);
  presentation::FileStagingViewModel model(std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, uwf::windows::systemCommitTraversalSource()));
  model.load();
  QVERIFY(!model.enhancedModeAvailable());
  QVERIFY(model.writable());
  model.setReadOnly(true);
  model.removePaths({QStringLiteral("C:\\one.txt")});
  QCOMPARE(store.writes, 0);
  model.setEnhancedModeAvailable(true);
  QVERIFY(!model.writable());
  model.setReadOnly(false);
  model.setEnhancedModeAvailable(false);
  model.removePaths({QStringLiteral("C:\\one.txt")});
  QCOMPARE(store.writes, 0);
  QVERIFY(model.entries().isEmpty());
}
void ViewModelBehaviorTests::shellVolumeProjection_data() {
  QTest::addColumn<std::vector<core::VolumeInfo>>("volumes");
  QTest::addColumn<QString>("systemDrive");
  QTest::addColumn<int>("registryHost");
  auto c = disk();
  c.label.clear();
  auto d = disk("D:");
  auto unsupported = c;
  unsupported.support = core::VolumeSupport::NotFixedLocalDisk;
  auto limited = d;
  limited.support = core::VolumeSupport::FileSystemLimited;
  QTest::newRow("empty") << std::vector<core::VolumeInfo>{} << QStringLiteral("C:") << -1;
  QTest::newRow("system-is-case-insensitive") << std::vector{c, d} << QStringLiteral("c:") << 0;
  QTest::newRow("missing-system-prefers-protectable") << std::vector{unsupported, limited} << QStringLiteral("X:") << 1;
  QTest::newRow("unsupported-system-still-hosts-global-registry") << std::vector{unsupported, d} << QStringLiteral("C:") << 0;
  QTest::newRow("no-protectable-falls-back-to-first") << std::vector{unsupported} << QStringLiteral("X:") << 0;
}
void ViewModelBehaviorTests::shellVolumeProjection() {
  QFETCH(std::vector<core::VolumeInfo>, volumes);
  QFETCH(QString, systemDrive);
  QFETCH(int, registryHost);
  for (const bool available : {false, true}) {
    FakeStateSource source;
    source.next.volumes = volumes;
    source.next.snapshot = snapshot();
    presentation::MainWindowViewModel model(source, available ? app::UwfCapability::Available : app::UwfCapability::Unavailable);
    model.refresh();
    QTRY_VERIFY(!model.refreshing());
    const auto pages = model.volumePages(systemDrive);
    QCOMPARE(pages.size(), volumes.size());
    for (std::size_t row = 0; row < pages.size(); ++row) {
      const auto& page = pages[row];
      QCOMPARE(page.hostsRegistry, static_cast<int>(row) == registryHost);
      QCOMPARE(page.hasStaging, available && (page.hostsRegistry || volumes[row].support == core::VolumeSupport::Supported));
      QCOMPARE(page.system, QString::fromStdString(volumes[row].driveLetter).compare(systemDrive, Qt::CaseInsensitive) == 0);
      QVERIFY(!page.tooltip.isEmpty());
      QVERIFY(!page.icon.isEmpty());
      QCOMPARE(page.title, volumes[row].label.empty()
                               ? QString::fromStdString(volumes[row].driveLetter)
                               : QStringLiteral("%1 (%2)").arg(QString::fromStdString(volumes[row].label), QString::fromStdString(volumes[row].driveLetter)));
    }
  }
}
void ViewModelBehaviorTests::volumeCommitAvailability_data() {
  using Support = core::VolumeSupport;
  QTest::addColumn<Support>("support");
  QTest::addColumn<QString>("fileSystem");
  QTest::addColumn<bool>("fileOperations");
  QTest::addColumn<bool>("protection");
  QTest::addColumn<bool>("registryHost");
  QTest::addColumn<bool>("filter");
  QTest::addColumn<bool>("elevated");
  QTest::addColumn<bool>("protectedVolume");
  struct FileSystem {
    Support support;
    const char* name;
    bool files, protectable;
  };
  constexpr std::array systems{FileSystem{Support::Supported, "NTFS", true, true}, FileSystem{Support::Supported, "FAT32", true, true},
                               FileSystem{Support::FileSystemLimited, "exFAT", false, true}, FileSystem{Support::NotFixedLocalDisk, "NTFS", false, false},
                               FileSystem{Support::ExceedsMaxSize, "NTFS", false, false}};
  for (const auto& system : systems)
    for (const bool registryHost : {false, true})
      for (const bool filter : {false, true})
        for (const bool elevated : {false, true})
          for (const bool protectedVolume : {false, true}) {
            const auto name = QStringLiteral("%1-%2-host%3-filter%4-elevated%5-protected%6")
                                  .arg(system.name)
                                  .arg(static_cast<int>(system.support))
                                  .arg(registryHost)
                                  .arg(filter)
                                  .arg(elevated)
                                  .arg(protectedVolume);
            QTest::newRow(qUtf8Printable(name)) << system.support << QString::fromLatin1(system.name) << system.files << system.protectable << registryHost
                                                << filter << elevated << protectedVolume;
          }
}
void ViewModelBehaviorTests::volumeCommitAvailability() {
  QFETCH(core::VolumeSupport, support);
  QFETCH(QString, fileSystem);
  QFETCH(bool, fileOperations);
  QFETCH(bool, protection);
  QFETCH(bool, registryHost);
  QFETCH(bool, filter);
  QFETCH(bool, elevated);
  QFETCH(bool, protectedVolume);
  auto volume = disk();
  volume.support = support;
  volume.fileSystem = fileSystem.toStdString();
  volume.freeBytes = 1ULL << 30;
  volume.totalBytes = 2ULL << 30;
  presentation::VolumeConfigurationViewModel model(volume, registryHost);
  const auto initial = model.commitAvailability();
  QVERIFY(!initial.files && !initial.registry && !initial.overlay);
  auto current = snapshot();
  current.current.filter.enabled = filter;
  current.current.volumes[0].isProtected = protectedVolume;
  current.elevated = elevated;
  model.applySnapshot(current);
  const auto available = model.commitAvailability();
  QCOMPARE(available.files, fileOperations && filter && elevated && protectedVolume);
  QCOMPARE(available.registry, registryHost && filter && elevated);
  QCOMPARE(available.overlay, protection && fileSystem == QStringLiteral("NTFS") && filter && elevated && protectedVolume);
  QVERIFY(!available.overlayHint.isEmpty());
  QVERIFY(!available.commitHint.isEmpty());
  QCOMPARE(model.capacityText(), fileSystem + QStringLiteral(" · 1 GB free / 2 GB"));
  QCOMPARE(presentation::volumeSupportText(support, volume.fileSystem).isEmpty(), support == core::VolumeSupport::Supported);
}
void ViewModelBehaviorTests::enhancedModeStatusProjection_data() {
  using Status = app::EnhancedModeStatus;
  using State = app::EnhancedModeState;
  using Tone = presentation::EnhancedModeViewModel::FeedbackTone;
  QTest::addColumn<Status>("status");
  QTest::addColumn<QString>("label");
  QTest::addColumn<QString>("action");
  QTest::addColumn<QString>("issue");
  QTest::addColumn<Tone>("issueTone");
  Status enabled;
  enabled.state = State::Enabled;
  enabled.serviceExists = enabled.ownProcess = enabled.automaticStart = enabled.localSystemAccount = enabled.running = true;
  enabled.executableMatches = enabled.preshutdownTimeoutConfigured = enabled.requiredPrivilegesConfigured = enabled.preshutdownAccepted = true;
  enabled.agentState = app::EnhancedModeAgentState::Connected;
  QTest::newRow("enabled") << enabled << QStringLiteral("Enabled") << QStringLiteral("Disable enhanced mode") << QString{} << Tone::Neutral;
  QTest::newRow("absent") << Status{} << QStringLiteral("Disabled") << QStringLiteral("Enable enhanced mode") << QString{} << Tone::Neutral;
  auto stopped = enabled;
  stopped.running = false;
  stopped.state = State::Stopped;
  QTest::newRow("stopped") << stopped << QStringLiteral("Service stopped") << QStringLiteral("Start service")
                           << QStringLiteral("The service is installed but stopped. Start it to restore automatic saving.") << Tone::Warning;
  struct Requirement {
    bool Status::* field;
    const char* name;
    const char* issue;
  };
  const std::array requirements{
      Requirement{&Status::ownProcess, "process", "The service is not configured as an independent process."},
      Requirement{&Status::automaticStart, "startup", "The service is not configured for automatic startup."},
      Requirement{&Status::localSystemAccount, "account", "The service is not configured to run as LocalSystem."},
      Requirement{&Status::executableMatches, "executable", "The service executable path or startup arguments do not match this executable."},
      Requirement{&Status::preshutdownTimeoutConfigured, "timeout", "The service preshutdown timeout is not configured correctly."},
      Requirement{&Status::requiredPrivilegesConfigured, "privileges", "The service does not declare the privileges required to start the UI agent."},
      Requirement{&Status::preshutdownAccepted, "notifications", "The service has not accepted Windows preshutdown notifications."},
  };
  for (const auto& requirement : requirements) {
    auto broken = enabled;
    broken.state = State::RepairRequired;
    broken.*(requirement.field) = false;
    QTest::newRow(requirement.name) << broken << QStringLiteral("Repair required") << QStringLiteral("Repair enhanced mode")
                                    << QString::fromLatin1(requirement.issue) << Tone::Warning;
  }
  auto connecting = enabled;
  connecting.agentState = app::EnhancedModeAgentState::Connecting;
  QTest::newRow("agent-connecting") << connecting << QStringLiteral("Enabled") << QStringLiteral("Disable enhanced mode")
                                    << QStringLiteral("Waiting for the UI agent to complete authentication.") << Tone::Warning;
  connecting.agentState = app::EnhancedModeAgentState::Disconnected;
  QTest::newRow("agent-disconnected") << connecting << QStringLiteral("Enabled") << QStringLiteral("Disable enhanced mode")
                                      << QStringLiteral("The service is running, but no authenticated UI agent is connected.") << Tone::Error;
  auto detailed = enabled;
  detailed.detail = QStringLiteral("controlled native detail");
  QTest::newRow("native-detail") << detailed << QStringLiteral("Enabled") << QStringLiteral("Disable enhanced mode") << detailed.detail << Tone::Error;
}
void ViewModelBehaviorTests::enhancedModeStatusProjection() {
  QFETCH(app::EnhancedModeStatus, status);
  QFETCH(QString, label);
  QFETCH(QString, action);
  QFETCH(QString, issue);
  QFETCH(presentation::EnhancedModeViewModel::FeedbackTone, issueTone);
  FakeEnhancedActions actions;
  actions.current = status;
  presentation::EnhancedModeViewModel model(actions);
  model.refreshStatus();
  const auto text = model.statusText();
  QCOMPARE(text.label, label);
  QCOMPARE(text.action, action);
  QCOMPARE(text.issues, issue);
  QCOMPARE(text.issueTone, issueTone);
  QCOMPARE(text.canRemove, status.hasRemnants());
  QCOMPARE(text.staging, status.serviceContractSatisfied() ? QStringLiteral("Ready") : QStringLiteral("Unavailable"));
  QCOMPARE(text.primaryAction, status.state != app::EnhancedModeState::Enabled);
}
void ViewModelBehaviorTests::enhancedModeWorkersRespectReceiverLifetimes_data() {
  QTest::addColumn<int>("lifetime");
  QTest::newRow("keep-open") << 0;
  QTest::newRow("close-presentation") << 1;
  QTest::newRow("destroy-session") << 2;
  QTest::newRow("destroy-during-status-notification") << 3;
}
void ViewModelBehaviorTests::enhancedModeWorkersRespectReceiverLifetimes() {
  QFETCH(int, lifetime);
  auto gate = std::make_shared<test::WorkerGate>();
  auto actions = std::make_unique<FakeEnhancedActions>();
  actions->gate = gate;
  auto model = std::make_unique<presentation::EnhancedModeViewModel>(*actions);
  model->refreshStatus();
  int completions = 0;
  const auto owner = std::this_thread::get_id();
  connect(actions.get(), &app::EnhancedModeActions::operationFinished, this, [&](const auto&, const auto failure) {
    QCOMPARE(std::this_thread::get_id(), owner);
    QVERIFY(!failure);
    ++completions;
  });
  model->applyRequestedState();
  QTRY_COMPARE(gate->started.load(), 1);
  QCOMPARE(gate->finished.load(), 0);
  QVERIFY(model->busy());
  model->applyRequestedState();
  model->removeService();
  QCOMPARE(actions->enables, 1);
  QCOMPARE(actions->disables, 0);
  bool eventDelivered = false;
  QTimer::singleShot(0, this, [&] { eventDelivered = true; });
  QTRY_VERIFY(eventDelivered);
  QCOMPARE(gate->finished.load(), 0);
  if (lifetime != 0) model.reset();
  if (lifetime == 2) actions.reset();
  if (lifetime == 3) connect(actions.get(), &app::EnhancedModeActions::statusChanged, this, [&](const auto&) { actions.reset(); });
  gate->release();
  QTRY_COMPARE(gate->finished.load(), 1);
  if (lifetime < 2) {
    QTRY_COMPARE(completions, 1);
    QVERIFY(!actions->operationInProgress());
    QCOMPARE(actions->current.state, app::EnhancedModeState::Enabled);
  } else {
    QTRY_VERIFY(!actions);
    QCOMPARE(completions, 0);
  }
}

void ViewModelBehaviorTests::enhancedModePublicationFailureIsReportedWithoutEscapingTheEventLoop() {
  FakeEnhancedActions actions;
  actions.failPublish = true;
  presentation::EnhancedModeViewModel model(actions);
  model.refreshStatus();
  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QVERIFY(model.operationFeedback());
  QCOMPARE(model.operationFeedback()->tone, presentation::EnhancedModeViewModel::FeedbackTone::Error);
  QVERIFY(model.operationFeedback()->detail.contains(QStringLiteral("agent startup failed")));
  QCOMPARE(model.status()->state, app::EnhancedModeState::Enabled);
  QCOMPARE(actions.enables, 1);
}

void ViewModelBehaviorTests::enhancedModeRetainsFeedbackAcrossAgentChanges() {
  FakeEnhancedActions actions;
  presentation::EnhancedModeViewModel model(actions);
  model.refreshStatus();
  QCOMPARE(model.action(), app::EnhancedModeAction::Enable);
  actions.persistenceWarning = QStringLiteral("registry commit deferred");
  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.enables, 1);
  QCOMPARE(actions.lastDescription, QStringLiteral("UWF Manager enhanced mode helper service"));
  QCOMPARE(model.operationFeedback()->tone, presentation::EnhancedModeViewModel::FeedbackTone::Warning);
  emit actions.agentConnectionStateChanged(app::EnhancedModeAgentState::Connected);
  QCOMPARE(model.status()->agentState, app::EnhancedModeAgentState::Connected);
  QVERIFY(model.operationFeedback()->detail.contains(actions.persistenceWarning));
  actions.failWrite = true;
  model.removeService();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(model.operationFeedback()->tone, presentation::EnhancedModeViewModel::FeedbackTone::Error);
  actions.failRead = true;
  model.refreshStatus();
  QVERIFY(!model.status());
  QVERIFY(!model.operationFeedback());
}
void ViewModelBehaviorTests::currentUsageIsIndependentOfPendingConfiguration() {
  presentation::GlobalConfigurationViewModel model(16384);
  auto current = snapshot().current;
  current.overlay = {core::OverlayType::RAM, 8192, 100, 200};
  model.setCurrentSession(current, {8000, 192});
  model.reset(current);
  QSignalSpy changes(&model, &presentation::GlobalConfigurationViewModel::usageChanged);
  QCOMPARE(model.usage().severity, core::OverlayUsageSeverity::Warning);
  const auto original = model.usage();
  model.importOverlayWarnMb(400);
  model.importOverlayCritMb(600);
  model.finishImport();
  QCOMPARE(model.usage().warningLegend, original.warningLegend);
  QCOMPARE(model.usage().criticalLegend, original.criticalLegend);
  QCOMPARE(model.usage().severity, original.severity);
  const auto pending = model.pendingOverlay();
  model.applyUsageState(app::OverlayUsageEnabled{{7992, 200}, current.overlay});
  QCOMPARE(changes.count(), 1);
  QCOMPARE(model.usage().severity, core::OverlayUsageSeverity::Critical);
  QVERIFY(!model.usage().warningText.isEmpty());
  QCOMPARE(model.pendingOverlay().warningThresholdMb, pending.warningThresholdMb);
  QCOMPARE(model.pendingOverlay().criticalThresholdMb, pending.criticalThresholdMb);
  const auto maximum = std::numeric_limits<uint32_t>::max();
  model.applyUsageState(app::OverlayUsageEnabled{{maximum, maximum}, current.overlay});
  QCOMPARE(model.usage().valueText, QStringLiteral("4,294,967,295 / 8,589,934,590 MB"));
  QCOMPARE(model.usage().percentageText, QStringLiteral("50%"));
  auto liveConfig = current.overlay;
  liveConfig.warningThresholdMb = 1000;
  liveConfig.criticalThresholdMb = 2000;
  model.applyUsageState(app::OverlayUsageEnabled{{7800, 392}, liveConfig});
  QCOMPARE(model.usage().severity, core::OverlayUsageSeverity::Normal);
  QCOMPARE(model.usage().warningMb, uint32_t{1000});
  QCOMPARE(model.pendingOverlay().warningThresholdMb, pending.warningThresholdMb);
  model.applyUsageState(app::OverlayUsageDisabled{});
  QVERIFY(!model.usage().enabled && model.usage().warningText.isEmpty());
  QCOMPARE(model.pendingOverlay().criticalThresholdMb, pending.criticalThresholdMb);
  current.filter.enabled = false;
  model.setCurrentSession(current, {});
  QVERIFY(!model.usage().enabled && model.usage().warningText.isEmpty());
  QCOMPARE(model.usage().percentageText, QStringLiteral("—"));
  const auto beforeUnavailable = changes.count();
  model.setAvailable(false);
  QCOMPARE(changes.count(), beforeUnavailable + 1);
  QVERIFY(!model.usage().enabled && model.usage().warningText.isEmpty());
  QCOMPARE(model.usage().valueText, QStringLiteral("Usage unavailable"));
}

void ViewModelBehaviorTests::overlayPublishesOnlyCompleteSuccessfulReads() {
  auto reader = std::make_shared<FakeUsageReader>();
  presentation::OverlayViewModel model(reader);
  model.refreshUsage();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(reader->calls, 0);
  const auto state = snapshot();
  model.applySnapshot(state);
  QSignalSpy changes(&model, &presentation::OverlayViewModel::usageChanged);
  reader->fail = true;
  model.refreshUsage();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(changes.count(), 0);
  QVERIFY(std::holds_alternative<app::OverlayUsageEnabled>(model.usage()));
  QCOMPARE(std::get<app::OverlayUsageEnabled>(model.usage()).runtime.currentConsumptionMb, state.runtime.currentConsumptionMb);
  reader->fail = false;
  auto runtime = state.runtime;
  runtime.currentConsumptionMb = 2048;
  reader->next = app::OverlayUsageEnabled{runtime, state.current.overlay};
  model.refreshUsage();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(changes.count(), 1);
  QCOMPARE(std::get<app::OverlayUsageEnabled>(model.usage()).runtime.currentConsumptionMb, uint64_t{2048});
}
void ViewModelBehaviorTests::powerPreparationAndCommitRespectTimingAndCancelBoundaries() {
  int polls = 0;
  int steps = 0;
  presentation::PowerDialogViewModel model({app::PowerAction::Restart, [&]() -> std::optional<app::PowerStagingPreparation> {
                                              ++polls;
                                              return app::PowerStagingCommit{
                                                  2, [&] {
                                                    ++steps;
                                                    return app::PowerStagingProgress{
                                                        steps == 1 ? app::PowerStagingState::InProgress : app::PowerStagingState::Failed,
                                                        static_cast<std::size_t>(steps), 2, QStringLiteral("C:\\file"), QStringLiteral("write failed")};
                                                  }};
                                            }});
  QSignalSpy ready(&model, &presentation::PowerDialogViewModel::preparationReady);
  QSignalSpy failed(&model, &presentation::PowerDialogViewModel::failureObserved);
  QSignalSpy accepted(&model, &presentation::PowerDialogViewModel::accepted);
  QElapsedTimer elapsed;
  elapsed.start();
  model.start();
  model.confirm();
  QCOMPARE(steps, 0);
  QTRY_COMPARE_WITH_TIMEOUT(ready.count(), 1, 3500);
  QVERIFY(elapsed.elapsed() >= 1000);
  QCOMPARE(polls, 1);
  model.confirm();
  QCOMPARE(model.state(), presentation::PowerDialogState::State::Staging);
  model.confirmDirect();
  QCOMPARE(accepted.count(), 0);
  QCOMPARE(model.state(), presentation::PowerDialogState::State::Staging);
  QVERIFY_THROWS_EXCEPTION(std::logic_error, static_cast<void>(model.finish(false)));
  QTRY_COMPARE(failed.count(), 1);
  QVERIFY(model.stagingStarted());
  QCOMPARE(steps, 2);
  QCOMPARE(model.finish(false), app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
}
void ViewModelBehaviorTests::powerDirectAndCompletedFailureDoNotRepeatPreparation() {
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument,
                           presentation::PowerDialogViewModel({app::PowerAction::Shutdown, [] { return std::optional<app::PowerStagingPreparation>{}; },
                                                               std::nullopt, app::PowerActionDialogMode::Direct}));
  presentation::PowerDialogViewModel direct({app::PowerAction::Shutdown, {}, std::nullopt, app::PowerActionDialogMode::Direct});
  direct.start();
  direct.confirm();
  QCOMPARE(direct.finish(true), app::PowerActionDialogOutcome::DirectConfirmed);
  app::FileStagingCommitResult result;
  result.failures.append({QStringLiteral("C:\\failed.bin"), app::FileStagingCommitFailureKind::ProviderFailure, QStringLiteral("earlier failure")});
  presentation::PowerDialogViewModel failure({app::PowerAction::Restart, {}, app::PowerStagingFailure{QStringLiteral("earlier failure"), result}});
  QSignalSpy observed(&failure, &presentation::PowerDialogViewModel::failureObserved);
  failure.start();
  QCOMPARE(observed.count(), 1);
  QVERIFY(!failure.preparationRequired());
  failure.confirm();
  QCOMPARE(failure.finish(true), app::PowerActionDialogOutcome::ContinuedAfterStagingFailure);
}
void ViewModelBehaviorTests::powerFailureContinuationRequiresRecordedTargets_data() {
  QTest::addColumn<int>("phase");
  QTest::addColumn<int>("failureKind");
  for (int phase = 0; phase < 3; ++phase)
    for (int kind = 0; kind < 3; ++kind) QTest::newRow(qUtf8Printable(QStringLiteral("phase-%1-failure-%2").arg(phase).arg(kind))) << phase << kind;
}
void ViewModelBehaviorTests::powerFailureContinuationRequiresRecordedTargets() {
  QFETCH(int, phase);
  QFETCH(int, failureKind);
  app::PowerStagingFailure failure{QStringLiteral("Complete original error\n第二行错误详情")};
  if (failureKind != 0) failure.result.emplace();
  if (failureKind == 2)
    failure.result->failures.append({QStringLiteral("C:\\failed.bin"), app::FileStagingCommitFailureKind::ProviderFailure, failure.details});
  app::PowerActionDialogRequest request(app::PowerAction::Restart);
  if (phase == 0) {
    request.completedStagingFailure = failure;
  } else {
    request.pollStagingPreparation = [phase, failure]() -> std::optional<app::PowerStagingPreparation> {
      if (phase == 1) return failure;
      return app::PowerStagingCommit{
          1, [failure] { return app::PowerStagingProgress{app::PowerStagingState::Failed, 0, 1, {}, failure.details, failure.result}; }};
    };
  }
  presentation::PowerDialogViewModel model(std::move(request));
  QSignalSpy accepted(&model, &presentation::PowerDialogViewModel::accepted);
  QSignalSpy failed(&model, &presentation::PowerDialogViewModel::failureObserved);
  QSignalSpy ready(&model, &presentation::PowerDialogViewModel::preparationReady);
  model.start();
  if (phase == 2) {
    QTRY_COMPARE_WITH_TIMEOUT(ready.count(), 1, 3500);
    model.confirm();
  }
  QTRY_COMPARE_WITH_TIMEOUT(failed.count(), 1, 3500);
  QCOMPARE(model.state(), presentation::PowerDialogState::State::FailureDecision);
  QCOMPARE(model.canContinueAfterStagingFailure(), failureKind == 2);
  model.confirmDirect();
  QCOMPARE(accepted.count(), 0);
  model.confirm();
  QCOMPARE(accepted.count(), failureKind == 2 ? 1 : 0);
  QCOMPARE(model.finish(true),
           failureKind == 2 ? app::PowerActionDialogOutcome::ContinuedAfterStagingFailure : app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
}
void ViewModelBehaviorTests::windowRefreshObserversMayDestroyPresentation() {
  FakeStateSource source;
  source.next = {{disk()}, snapshot()};
  auto model = std::make_unique<presentation::MainWindowViewModel>(source, app::UwfCapability::Available);
  connect(model.get(), &presentation::MainWindowViewModel::stateCommitted, this, [&] { model.reset(); });
  model->refresh();
  QTRY_VERIFY(!model);
  QVERIFY(!model);
}
void ViewModelBehaviorTests::windowStateRetainsCommittedBaselineAfterReadFailure() {
  FakeStateSource source;
  source.next = {{disk()}, snapshot()};
  presentation::MainWindowViewModel model(source, uwf::app::UwfCapability::Available);
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(model.state().configurationWritesAllowed());
  model.requireReconciliation();
  QVERIFY(!model.state().configurationWritesAllowed());
  source.fail = true;
  QSignalSpy failed(&model, &presentation::MainWindowViewModel::refreshFailed);
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(failed.count(), 1);
  QVERIFY(model.state().hasCommittedState());
  QCOMPARE(model.state().volumes().size(), std::size_t{1});
  QVERIFY(model.state().reconciliationRequired());
  source.fail = false;
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(!model.state().reconciliationRequired());
}
void ViewModelBehaviorTests::windowReconciliationRequiresEveryWrittenStoreToBeReadable_data() {
  QTest::addColumn<bool>("stateReadable");
  QTest::addColumn<bool>("stagingReadable");
  for (const bool state : {false, true})
    for (const bool staging : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("state-%1-staging-%2").arg(state).arg(staging))) << state << staging;
}
void ViewModelBehaviorTests::windowReconciliationRequiresEveryWrittenStoreToBeReadable() {
  QFETCH(bool, stateReadable);
  QFETCH(bool, stagingReadable);
  FakeStateSource source;
  source.next = {{disk()}, snapshot()};
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  auto draft = std::make_shared<app::FileStagingDraft>(storeOwner);
  presentation::MainWindowViewModel model(source, app::UwfCapability::Available, draft);
  model.initializeConfiguration(8192, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(model.configuration()->global()->controlsEnabled());
  const QList<app::FileStagingEntry> intended{{app::FileStagingKind::File, QStringLiteral("C:\\Data\\state.bin")}};
  draft->replace(intended);
  store.failReadAfterReplace = true;
  QVERIFY_THROWS_EXCEPTION(app::OperationError, draft->apply(*draft->changes()));
  model.requireReconciliation();
  source.fail = !stateReadable;
  store.failRead = !stagingReadable;
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(model.state().configurationWritesAllowed(), stateReadable && stagingReadable);
  QCOMPARE(model.configuration()->global()->controlsEnabled(), stateReadable && stagingReadable);
  QCOMPARE(draft->changes()->after, intended);
  source.fail = store.failRead = false;
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(model.state().configurationWritesAllowed());
  QVERIFY(model.configuration()->global()->controlsEnabled());
  QCOMPARE(draft->load(), intended);
  QVERIFY(draft->changes()->persistencePending);
}
void ViewModelBehaviorTests::windowStatusBindingSurvivesItsSessionAndCanBeReplaced() {
  class IdleAgent final : public app::EnhancedModeAgentConnection {
   public:
    void start() override {}
    void stop() override {}
    bool running() const override { return false; }
    bool markPreshutdownCommitHandled() override { return false; }
    bool requirePreshutdownCommit() override { return false; }
  } agent;
  FakeStateSource source;
  presentation::MainWindowViewModel model(source, app::UwfCapability::Unavailable);
  auto failedActions = std::make_unique<FakeEnhancedActions>();
  failedActions->failRead = true;
  auto session = std::make_unique<app::EnhancedModeSession>(std::move(failedActions), agent);
  model.bindEnhancedMode(*session);
  model.refreshEnhancedModeStatus();
  QCOMPARE(model.enhancedModeStatus().state, app::EnhancedModeState::RepairRequired);
  session.reset();
  model.refreshEnhancedModeStatus();

  app::EnhancedModeSession replacement(std::make_unique<FakeEnhancedActions>(), agent);
  model.bindEnhancedMode(replacement);
  QSignalSpy observed(&model, &presentation::MainWindowViewModel::enhancedModeStatusChanged);
  model.refreshEnhancedModeStatus();
  QCOMPARE(observed.size(), 1);
  QCOMPARE(model.enhancedModeStatus().state, app::EnhancedModeState::Disabled);
  model.unbindEnhancedMode();
  replacement.refresh();
  QCOMPARE(observed.size(), 1);
}
void ViewModelBehaviorTests::registryNavigationResolvesAccessibleAncestors_data() {
  QTest::addColumn<QString>("path");
  QTest::addColumn<QString>("expected");
  QTest::addColumn<bool>("denied");
  const QString root = QStringLiteral("HKEY_LOCAL_MACHINE");
  QTest::newRow("alias") << QStringLiteral("HKLM\\SOFTWARE") << root + QStringLiteral("\\SOFTWARE") << false;
  QTest::newRow("case") << QStringLiteral("hkey_local_machine\\software") << root + QStringLiteral("\\SOFTWARE") << false;
  QTest::newRow("missing-descendant") << QStringLiteral("HKLM\\SOFTWARE\\missing") << root + QStringLiteral("\\SOFTWARE") << false;
  QTest::newRow("access-denied") << QStringLiteral("HKLM\\SOFTWARE") << root << true;
  QTest::newRow("unavailable-hive") << QStringLiteral("HKCU\\SOFTWARE") << QString{} << false;
  QTest::newRow("invalid-hive") << QStringLiteral("invalid\\SOFTWARE") << QString{} << false;
  QTest::newRow("empty") << QString{} << QString{} << false;
}
void ViewModelBehaviorTests::registryNavigationResolvesAccessibleAncestors() {
  QFETCH(QString, path);
  QFETCH(QString, expected);
  QFETCH(bool, denied);
  FakeRegistryBrowser browser;
  if (denied) browser.deniedChildren = "HKEY_LOCAL_MACHINE";
  presentation::RegistryPickerViewModel model(presentation::RegistryPickerViewModel::Mode::CommitValue, browser);
  const auto selected = model.locateKey(path);
  QCOMPARE(selected.data(presentation::PathRole).toString(), expected);
}
void ViewModelBehaviorTests::registryModelRetainsDefaultValueAndRestrictedChildSemantics() {
  FakeRegistryBrowser browser;
  using Model = presentation::RegistryPickerViewModel;
  Model model(Model::Mode::Exclusion, browser);
  const auto children = model.children(QStringLiteral("HKEY_LOCAL_MACHINE"));
  QCOMPARE(children.size(), std::size_t{2});
  QCOMPARE(children.back().subkeys, Model::SubkeyState::Unknown);
  const auto values = model.values(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE"));
  QCOMPARE(values.size(), std::size_t{2});
  QVERIFY(values.front().name.empty());
  QVERIFY(!model.canAccept(QStringLiteral("HKEY_LOCAL_MACHINE")));
  QVERIFY(model.canAccept(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor")));
  model.accept(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor"), QStringLiteral("Named"));
  QVERIFY(model.selection()->valueName.isEmpty());
}

void ViewModelBehaviorTests::registryAccessErrorsTrackTheRequestedKeyAndRecover() {
  FakeRegistryBrowser browser;
  using Model = presentation::RegistryPickerViewModel;
  Model model(Model::Mode::CommitValue, browser);
  QSignalSpy changed(&model, &Model::accessStateChanged);
  const auto root = model.index(0, 0);
  const auto rootPath = QStringLiteral("HKEY_LOCAL_MACHINE");
  model.loadValues(rootPath);
  QCOMPARE(model.valueModel().rowCount(), 2);
  browser.deniedChildren = rootPath.toStdString();
  QVERIFY(!model.loadChildren(root));
  QVERIFY(!changed.isEmpty());
  QVERIFY(model.accessError().has_value());
  QCOMPARE(model.accessError()->operation, Model::AccessError::Operation::Keys);
  QCOMPARE(model.accessError()->key, rootPath);
  QCOMPARE(model.accessError()->message, QStringLiteral("拒绝访问"));
  QCOMPARE(model.valueModel().rowCount(), 0);
  QVERIFY(!model.canFetchMore(root));

  model.loadValues(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE"));
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 2);
  model.loadValues(rootPath);
  QVERIFY(model.accessError());
  QCOMPARE(model.accessError()->operation, Model::AccessError::Operation::Keys);
  QCOMPARE(model.valueModel().rowCount(), 0);
  browser.deniedChildren.clear();
  QVERIFY(model.loadChildren(root));
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 2);

  const auto unselectedChild = model.index(0, 0, root);
  browser.deniedChildren = "HKEY_LOCAL_MACHINE\\SOFTWARE";
  QVERIFY(!model.loadChildren(unselectedChild));
  QCOMPARE(model.accessError()->key, QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE"));
  browser.deniedChildren.clear();
  QVERIFY(model.loadChildren(unselectedChild));
  QVERIFY(!model.accessError());
  QCOMPARE(browser.lastValueKey, rootPath.toStdString());
  QCOMPARE(model.valueModel().rowCount(), 2);

  browser.denyValues = true;
  model.loadValues(QStringLiteral("HKEY_LOCAL_MACHINE\\Restricted"));
  QVERIFY(model.accessError());
  QCOMPARE(model.accessError()->operation, Model::AccessError::Operation::Values);
  QCOMPARE(model.accessError()->key, QStringLiteral("HKEY_LOCAL_MACHINE\\Restricted"));
  QCOMPARE(model.valueModel().rowCount(), 0);
  model.loadValues({});
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 0);
  model.loadValues(rootPath);
  QVERIFY(model.accessError());
  model.resetTree();
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 0);
  browser.denyValues = false;
  model.loadValues(rootPath);
  QVERIFY(!model.accessError());
  QCOMPARE(model.valueModel().rowCount(), 2);
}
void ViewModelBehaviorTests::overlayFileModelNormalizesStreamsAndCancelsItsWorker() {
  presentation::OverlayFilesViewModel model(QStringLiteral("C:"), [](const QString&, std::stop_token) {
    return QVector<app::OverlayFileEntry>{{QStringLiteral("\\dir\\one.txt:stream"), {}, false, false, 8},
                                          {QStringLiteral("\\dir\\ONE.txt"), {}, false, false, 16},
                                          {QStringLiteral("\\$Mft:stream"), {}, false, false, 32}};
  });
  QSignalSpy loaded(&model, &presentation::OverlayFilesViewModel::entriesLoaded);
  model.startLoading();
  QTRY_COMPARE(loaded.count(), 1);
  QCOMPARE(model.entries().size(), 2);
  QVERIFY(model.entries().front().isSystemMetadata);
  QCOMPARE(model.entries().back().fileSize, qulonglong{24});
  QVERIFY(model.exportContent().contains(QStringLiteral("C:\\dir\\one.txt")));
  std::atomic<bool> entered = false;
  std::atomic<bool> stopped = false;
  {
    auto canceling = std::make_unique<presentation::OverlayFilesViewModel>(QStringLiteral("C:"), [&](const QString&, const std::stop_token token) {
      entered = true;
      while (!token.stop_requested()) std::this_thread::yield();
      stopped = true;
      return QVector<app::OverlayFileEntry>{};
    });
    canceling->startLoading();
    QTRY_VERIFY(entered.load());
  }
  QVERIFY(stopped.load());
}
void ViewModelBehaviorTests::overlayFilesCancellationRemainsSilent() {
  std::atomic<bool> entered = false;
  presentation::OverlayFilesViewModel canceled(QStringLiteral("C:"), [&](const QString&, std::stop_token) -> QVector<app::OverlayFileEntry> {
    entered = true;
    throw uwf::app::wmi::WmiCancelled("enumerate overlay files");
  });
  QSignalSpy stopped(&canceled, &presentation::OverlayFilesViewModel::loadingStopped);
  QSignalSpy failed(&canceled, &presentation::OverlayFilesViewModel::loadFailed);
  QSignalSpy loaded(&canceled, &presentation::OverlayFilesViewModel::entriesLoaded);
  canceled.startLoading();
  QTRY_VERIFY(entered.load());
  QTest::qWait(50);
  QVERIFY(stopped.isEmpty());
  QVERIFY(failed.isEmpty());
  QVERIFY(loaded.isEmpty());
  QVERIFY(!canceled.loaded());
  QTRY_COMPARE_WITH_TIMEOUT(stopped.count(), 1, 2000);
  QCOMPARE(canceled.loadState(), presentation::ContentLoadState::Canceled);
  QVERIFY(failed.isEmpty());
  QVERIFY(loaded.isEmpty());

  presentation::OverlayFilesViewModel rejected(
      QStringLiteral("C:"), [](const QString&, std::stop_token) -> QVector<app::OverlayFileEntry> { throw std::runtime_error("enumeration rejected"); });
  QSignalSpy rejection(&rejected, &presentation::OverlayFilesViewModel::loadFailed);
  rejected.startLoading();
  QTRY_COMPARE(rejection.count(), 1);
  QVERIFY(rejection.front().front().toString().contains(QStringLiteral("enumeration rejected")));
}

void ViewModelBehaviorTests::reportModelPreservesAllRowsAcrossPages() {
  using namespace presentation;
  QList<CommitReportRow> rows;
  for (int index = 0; index < 205; ++index)
    rows.append({uwf::presentation::CommitRowStatus::Succeeded, QString::number(index), QStringLiteral("-"), QStringLiteral("-"), {}, {}});
  CommitReportViewModel model(rows, app::CommitOperation::DeleteAndPersist, {CommitCompletion::Cause::Canceled, 2});
  presentation::PageProxyModel pages;
  pages.setPageSize(200);
  pages.setSourceModel(&model);
  QCOMPARE(pages.pager().pageCount(205), 2);
  pages.goNext();
  QCOMPARE(pages.pager().pageStart(), 200);
  QVERIFY(model.showExistence());
  QCOMPARE(model.counts().succeeded, 205);
  QCOMPARE(model.counts().failed, 0);
  QCOMPARE(model.counts().skipped, 0);
  QVERIFY(model.summary().contains(QStringLiteral("205 succeeded")));
  QVERIFY(model.allRowsToText().contains(QStringLiteral("Succeeded\t204\tUnknown\tUnknown")));
  app::CommitTargetResult result;
  result.targetAlreadyAbsent = true;
  const auto row = makeCommitReportRow(result, QStringLiteral("C:\\missing"), app::CommitOperation::DeleteAndPersist);
  QCOMPARE(row.status, presentation::CommitRowStatus::Skipped);
  QCOMPARE(row.existedBefore, std::optional<bool>(false));
}

void ViewModelBehaviorTests::commitBatchesKeepCallbacksLazyAndObserveTheOriginalTargets() {
  QList<QString> targets{QStringLiteral("first"), QStringLiteral("second")};
  QStringList calls;
  auto display = [&](const QString& value) {
    calls.append(QStringLiteral("display:") + value);
    return value;
  };
  auto commit = [&](const QString& value) { calls.append(QStringLiteral("commit:") + value); };
  auto exists = nullptr;
  const auto request = app::makeCommitBatchRequest(targets, display, commit, exists);
  QCOMPARE(request.total, 2);
  QVERIFY(calls.isEmpty());
  targets[1] = QStringLiteral("updated");
  const auto text = request.displayOf(1);
  const auto result = request.execute(1, text);
  QVERIFY(result.succeeded);
  QCOMPARE(calls, QStringList({QStringLiteral("display:updated"), QStringLiteral("commit:updated")}));
}

void ViewModelBehaviorTests::negativeEditorValuesClampToTheLowerBound() {
  presentation::GlobalConfigurationViewModel model(16384);
  model.reset(snapshot().next);
  model.editWarningThreshold(-1);
  QCOMPARE(model.draft().overlay.warningThresholdMb, uint32_t{0});
  model.editCriticalThreshold(-1);
  QCOMPARE(model.draft().overlay.criticalThresholdMb, uint32_t{0});
  model.editMaximumSize(-1);
  QCOMPARE(model.draft().overlay.maximumSizeMb, uint32_t{0});
  model.editOverlayType(core::OverlayType::Disk);
  model.editMaximumSize(-1);
  QCOMPARE(model.draft().overlay.maximumSizeMb, uint32_t{1024});
}

void ViewModelBehaviorTests::itemModelsHonorTheQtContractAcrossUpdates() {
  using Tester = QAbstractItemModelTester;
  using Mode = Tester::FailureReportingMode;
  presentation::ExclusionListViewModel exclusions(presentation::ExclusionListViewModel::Kind::File);
  Tester exclusionContract(&exclusions, Mode::QtTest);
  exclusions.setDriveLetter(QStringLiteral("C:"));
  exclusions.setBaseline({QStringLiteral("C:\\Alpha")}, {QStringLiteral("C:\\Alpha"), QStringLiteral("C:\\Beta")});
  QCOMPARE(exclusions.rowCount(), 2);
  QPersistentModelIndex old = exclusions.index(0, 0);
  QCOMPARE(exclusions.importAdd(QStringLiteral("C:\\Gamma")), app::ExclusionImportOutcome::Applied);
  QVERIFY(!old.isValid());
  exclusions.resetPending();
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  app::FileStagingDraft draft(storeOwner);
  auto editor = std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, uwf::windows::systemCommitTraversalSource());
  presentation::FileStagingViewModel staging(editor);
  Tester stagingContract(&staging, Mode::QtTest);
  store.entries = {{app::FileStagingKind::Directory, QStringLiteral("C:\\Cache")}};
  staging.load();
  QCOMPARE(staging.rowCount(), 1);
  presentation::ImportViewModel imports;
  Tester importContract(&imports, Mode::QtTest);
  imports.appendReport({{app::ImportResult::Status::Success, 1, QStringLiteral("filter enable"), QStringLiteral("Queued")}});
  imports.appendReport({{app::ImportResult::Status::Duplicate, 2, QStringLiteral("filter enable"), QStringLiteral("Unchanged")}});
  QCOMPARE(imports.rowCount(), 3);
  QVERIFY(imports.index(1, 0).data(presentation::SeparatorRole).toBool());
  QList<presentation::CommitReportRow> rows;
  rows.append(
      {uwf::presentation::CommitRowStatus::Succeeded, QStringLiteral("C:\\file.txt"), QStringLiteral("-"), QStringLiteral("-"), std::nullopt, std::nullopt});
  presentation::CommitReportViewModel report(rows, app::CommitOperation::Persist);
  Tester reportContract(&report, Mode::QtTest);
  presentation::OverlayFilesViewModel overlay(QStringLiteral("C:"), [](const QString&, std::stop_token) {
    return QVector<app::OverlayFileEntry>{{QStringLiteral("\\file.txt"), {}, false, false, 123}};
  });
  Tester overlayContract(&overlay, Mode::QtTest);
  overlay.startLoading();
  QTRY_VERIFY(overlay.loaded());
  FakeRegistryBrowser browser;
  presentation::RegistryPickerViewModel registry(presentation::RegistryPickerViewModel::Mode::CommitValue, browser);
  const auto root = registry.index(0, 0);
  QVERIFY(registry.canFetchMore(root));
  Tester registryContract(&registry, Mode::QtTest);
  registryContract.setUseFetchMore(false);
  Tester valueContract(&registry.valueModel(), Mode::QtTest);
  QVERIFY(registry.loadChildren(root));
  QCOMPARE(registry.rowCount(root), 2);
  QCOMPARE(registry.parent(registry.index(0, 0, root)), root);
  QVERIFY(!registry.canFetchMore(root));
  registry.loadValues(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE"));
  QCOMPARE(registry.valueModel().rowCount(), 2);
  registry.resetTree();
  presentation::LogViewModel log(std::make_shared<app::ProcessLogSource>());
  Tester logContract(&log, Mode::QtTest);
  QSignalSpy loaded(&log, &presentation::LogViewModel::loadingFinished);
  log.reload();
  QTRY_COMPARE(loaded.size(), 1);
}

void ViewModelBehaviorTests::pageProxyPreservesSourceMappingAndIndependentViewState() {
  QStringList rows;
  for (int row = 0; row < 205; ++row) rows.append(QString::number(row));
  QStringListModel source(rows);
  presentation::PageProxyModel first;
  presentation::PageProxyModel second;
  first.setPageSize(200);
  second.setPageSize(10);
  first.setSourceModel(&source);
  second.setSourceModel(&source);
  QAbstractItemModelTester contract(&first, QAbstractItemModelTester::FailureReportingMode::QtTest);
  QCOMPARE(first.rowCount(), 200);
  first.goLast();
  QCOMPARE(first.rowCount(), 5);
  QCOMPARE(first.index(0, 0).data().toString(), QStringLiteral("200"));
  QCOMPARE(first.mapToSource(first.index(4, 0)), source.index(204, 0));
  QVERIFY(!first.mapFromSource(source.index(0, 0)).isValid());
  QCOMPARE(second.index(0, 0).data().toString(), QStringLiteral("0"));
  first.setPageSize(10);
  QCOMPARE(first.pager().pageStart(), 200);
  source.setStringList({QStringLiteral("retained")});
  QCOMPARE(first.pager().currentPage, 0);
  QCOMPARE(first.rowCount(), 1);
  first.goNext();
  QCOMPARE(first.pager().currentPage, 0);
  source.setStringList({});
  first.goLast();
  QCOMPARE(first.rowCount(), 0);
  QCOMPARE(first.pager().currentPage, 0);
}

void ViewModelBehaviorTests::pageProxyTracksSourceChangesAndDetachment() {
  QStringListModel source({QStringLiteral("first"), QStringLiteral("second"), QStringLiteral("third")});
  presentation::PageProxyModel pages;
  pages.setPageSize(2);
  pages.setSourceModel(&source);
  QAbstractItemModelTester contract(&pages, QAbstractItemModelTester::FailureReportingMode::QtTest);
  pages.goLast();
  QCOMPARE(pages.rowCount(), 1);
  QVERIFY(source.insertRow(3));
  QVERIFY(source.setData(source.index(3, 0), QStringLiteral("fourth")));
  QCOMPARE(pages.rowCount(), 2);
  QCOMPARE(pages.index(1, 0).data().toString(), QStringLiteral("fourth"));
  QVERIFY(source.removeRows(1, 3));
  QCOMPARE(pages.rowCount(), 1);
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("first"));
  QVERIFY(source.setData(source.index(0, 0), QStringLiteral("updated")));
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("updated"));

  QStringListModel replacement({QStringLiteral("replacement")});
  pages.setSourceModel(&replacement);
  source.setStringList({QStringLiteral("stale")});
  QCOMPARE(pages.rowCount(), 1);
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("replacement"));
  pages.setSourceModel(nullptr);
  QCOMPARE(pages.rowCount(), 0);
  QCOMPARE(pages.totalRows(), 0);
  pages.goLast();
  pages.goNext();
  QVERIFY(!pages.index(0, 0).isValid());
  pages.setSourceModel(&source);
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("stale"));
}

void ViewModelBehaviorTests::pageProxyTracksReorderingAndSourceDestruction() {
  auto source = std::make_unique<QStringListModel>(QStringList{QStringLiteral("0"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")});
  presentation::PageProxyModel pages;
  pages.setPageSize(2);
  pages.setSourceModel(source.get());
  QAbstractItemModelTester contract(&pages, QAbstractItemModelTester::FailureReportingMode::QtTest);
  pages.goLast();
  QVERIFY(source->moveRows({}, 0, 1, {}, 4));
  QCOMPARE(pages.rowCount(), 2);
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("3"));
  QCOMPARE(pages.index(1, 0).data().toString(), QStringLiteral("0"));
  source->sort(0, Qt::DescendingOrder);
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("1"));
  QCOMPARE(pages.index(1, 0).data().toString(), QStringLiteral("0"));
  QSignalSpy changed(&pages, &presentation::PageProxyModel::pageChanged);
  source.reset();
  QCOMPARE(pages.totalRows(), 0);
  QCOMPARE(pages.rowCount(), 0);
  QCOMPARE(pages.pager().currentPage, 0);
  QTRY_VERIFY(!changed.isEmpty());
}

void ViewModelBehaviorTests::stagingStorageRecoveryClearsErrorsAndPublishesAvailability() {
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  app::FileStagingDraft draft(storeOwner);
  auto editor = std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, uwf::windows::systemCommitTraversalSource());
  presentation::FileStagingViewModel model(editor);
  QSignalSpy availability(&model, &presentation::FileStagingViewModel::storageAvailabilityChanged);
  store.failRead = true;
  model.load();
  QVERIFY(!model.storageAvailable());
  QVERIFY(!model.storageError().isEmpty());
  store.failRead = false;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\restored.bin")}};
  model.load();
  QVERIFY(model.storageAvailable());
  QVERIFY(model.storageError().isEmpty());
  QVERIFY(model.writable());
  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(availability.size(), 2);
  QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("C:\\restored.bin"));
}

void ViewModelBehaviorTests::unicodePathsShareExclusionAndStagingDecisions() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString parent = directory.filePath(QString::fromUtf8("Äpfel"));
  QVERIFY(QDir().mkpath(parent));
  const QString alternate = directory.filePath(QString::fromUtf8("äPFEL"));
  QVERIFY(QFileInfo::exists(alternate));
  const QString child = alternate + QStringLiteral("\\state.bin");
  app::FileStagingConflictPolicy conflicts;
  conflicts.setFileExclusions({parent});
  QVERIFY(conflicts.conflictingExclusion(child).has_value());
  QVERIFY(app::paths::contains(parent.toStdString(), child.toStdString()));
  QCOMPARE(QString::fromStdString(app::paths::findCoveringExclusion({parent.toStdString()}, child.toStdString())), parent);
  QVERIFY(!app::paths::contains(parent, alternate + QStringLiteral("-sibling\\state.bin")));
  QVERIFY(!app::paths::contains(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\.."), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor")));
}

void ViewModelBehaviorTests::globalImportProducesConstrainedPendingDelta() {
  presentation::GlobalConfigurationViewModel model(16384);
  uwf::core::SessionSnapshot current;
  uwf::core::SessionSnapshot next;
  current.filter.enabled = false;
  next.filter.enabled = false;
  next.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  model.setData(current, next, {4096, 1024});

  QVERIFY(!model.pendingFilterEnabled().has_value());
  QVERIFY(model.pendingOverlay().empty());
  QVERIFY(model.importFilterEnabled(true));
  const auto pendingFilter = model.pendingFilterEnabled();
  QVERIFY(pendingFilter.has_value());
  QCOMPARE(pendingFilter.value_or(false), true);
  QVERIFY(!model.importFilterEnabled(true));

  QVERIFY(model.importOverlayType(uwf::core::OverlayType::Disk));
  QVERIFY(model.importOverlayMaxMb(2000));
  QVERIFY(model.importOverlayWarnMb(2500));
  QVERIFY(model.importOverlayCritMb(2200));
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

  model.setData(current, next, {4096, 1024});
  QVERIFY(!model.pendingFilterEnabled().has_value());
  QVERIFY(model.pendingOverlay().empty());
}

void ViewModelBehaviorTests::unavailableGlobalConfigurationRejectsImports() {
  presentation::GlobalConfigurationViewModel model(16384);
  uwf::core::SessionSnapshot current;
  uwf::core::SessionSnapshot next;
  next.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  model.setData(current, next, {4096, 1024});
  model.setAvailable(false);

  QVERIFY(!model.importFilterEnabled(true));
  QVERIFY(!model.importOverlayType(uwf::core::OverlayType::Disk));
  QVERIFY(!model.importOverlayMaxMb(8192));
  QVERIFY(!model.importOverlayWarnMb(4096));
  QVERIFY(!model.importOverlayCritMb(6144));
  QVERIFY(!model.pendingFilterEnabled().has_value());
  QVERIFY(model.pendingOverlay().empty());
}

void ViewModelBehaviorTests::fileExclusionImportMaintainsCaseInsensitiveDeltas() {
  using Outcome = app::ExclusionImportOutcome;
  presentation::ExclusionListViewModel list(app::ExclusionEditor::Kind::File);
  list.setDriveLetter(QStringLiteral("Z:"));
  list.setBaseline({QStringLiteral("Z:\\Current")}, {QStringLiteral("Z:\\Keep"), QStringLiteral("Z:\\Remove")});

  QCOMPARE(list.importAdd(QStringLiteral("z:/New Folder")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("z:\\New Folder")}));
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\NEW FOLDER")), Outcome::NoOp);
  QCOMPARE(list.importRemove(QStringLiteral("Z:\\new folder")), Outcome::Applied);
  QVERIFY(list.pendingAdded().isEmpty());

  QCOMPARE(list.importRemove(QStringLiteral("z:/remove")), Outcome::Applied);
  QCOMPARE(list.pendingRemoved(), QStringList({QStringLiteral("z:\\remove")}));
  QCOMPARE(list.importRemove(QStringLiteral("Z:\\REMOVE")), Outcome::NoOp);
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\Remove")), Outcome::Applied);
  QVERIFY(list.pendingRemoved().isEmpty());

  list.resetPending();
  QVERIFY(list.pendingAdded().isEmpty());
  QVERIFY(list.pendingRemoved().isEmpty());
  // 当前仍在排除，但下一次启动已安排移除；重新添加必须撤销该安排。
  QCOMPARE(list.importAdd(QStringLiteral("z:\\current")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("z:\\current")}));
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\CURRENT")), Outcome::NoOp);
  QCOMPARE(list.importRemove(QStringLiteral("Z:\\Current")), Outcome::Applied);
  QVERIFY(list.pendingAdded().isEmpty());
}

void ViewModelBehaviorTests::fileExclusionImportRejectsWrongVolumeAndUwfBlacklist() {
  using Outcome = app::ExclusionImportOutcome;
  presentation::ExclusionListViewModel list(app::ExclusionEditor::Kind::File);
  list.setDriveLetter(QStringLiteral("Z:"));
  list.setBaseline({}, {});

  QCOMPARE(list.importAdd(QStringLiteral("Y:\\Data")), Outcome::RejectedNotOnVolume);
  QCOMPARE(list.importAdd(QStringLiteral("Z:")), Outcome::RejectedForbidden);
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\pagefile.sys")), Outcome::RejectedForbidden);
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\Data\\Cache")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("Z:\\Data\\Cache")}));
}

void ViewModelBehaviorTests::registryExclusionImportsFollowTheEditingPolicy() {
  using Outcome = app::ExclusionImportOutcome;
  auto editor = std::make_shared<uwf::app::ExclusionEditor>(uwf::app::ExclusionEditor::Kind::Registry);
  presentation::ExclusionListViewModel list(editor);
  const QString stagingRoot = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager");
  list.setBaseline({}, {QStringLiteral("HKLM\\SYSTEM\\Existing"), stagingRoot});

  QCOMPARE(list.importAdd(QStringLiteral("hklm\\software\\Vendor\\Product")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("HKEY_LOCAL_MACHINE\\software\\Vendor\\Product")}));
  QCOMPARE(list.importAdd(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\VENDOR\\PRODUCT")), Outcome::NoOp);
  QCOMPARE(list.importAdd(QStringLiteral("HKCU\\Software\\Vendor")), Outcome::RejectedForbidden);
  QCOMPARE(list.importAdd(QStringLiteral("HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC")), Outcome::RejectedForbidden);

  QCOMPARE(list.importRemove(QStringLiteral("hklm\\system\\existing")), Outcome::Applied);
  QCOMPARE(list.pendingRemoved(), QStringList({QStringLiteral("HKEY_LOCAL_MACHINE\\system\\existing")}));

  bool stagingHasData = true;
  list.setFileStagingDataProbe([&stagingHasData] { return stagingHasData; });
  QCOMPARE(list.importRemove(stagingRoot), Outcome::RejectedRequired);
  QCOMPARE(list.pendingRemoved(), QStringList({QStringLiteral("HKEY_LOCAL_MACHINE\\system\\existing")}));

  // 暂存变化通知立即撤销根排除项的移除意图，之后清空暂存也不恢复它。
  stagingHasData = false;
  QCOMPARE(list.importRemove(stagingRoot), Outcome::Applied);
  QVERIFY(list.pendingRemoved().contains(stagingRoot, Qt::CaseInsensitive));
  stagingHasData = true;
  editor->refreshFileStagingConstraints();
  QVERIFY(!list.pendingRemoved().contains(stagingRoot, Qt::CaseInsensitive));
  stagingHasData = false;
  QVERIFY(!list.pendingRemoved().contains(stagingRoot, Qt::CaseInsensitive));
  list.setBaseline({QStringLiteral("HKLM\\SOFTWARE\\Current")}, {});
  QCOMPARE(list.importAdd(QStringLiteral("hklm\\software\\current")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("HKEY_LOCAL_MACHINE\\software\\current")}));
}

}  // namespace

QTEST_GUILESS_MAIN(ViewModelBehaviorTests)
#include "ViewModelBehaviorTests.moc"
