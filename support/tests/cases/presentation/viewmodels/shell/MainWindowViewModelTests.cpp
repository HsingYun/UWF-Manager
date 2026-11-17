// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QEvent>
#include <QPointer>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QTranslator>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/enhanced/EnhancedModeManager.h"
#include "application/operations/contracts/OperationError.h"
#include "application/staging/editing/FileStagingEditor.h"
#include "application/state/contracts/ApplicationStateSource.h"
#include "cases/presentation/viewmodels/common/ViewModelStateFixture.h"
#include "cases/presentation/viewmodels/enhanced/FakeEnhancedModeActions.h"
#include "cases/presentation/viewmodels/staging/MemoryStagingStore.h"
#include "presentation/viewmodels/common/AutomaticCommitText.h"
#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/common/RuntimeWriteText.h"
#include "presentation/viewmodels/common/ViewModelFactory.h"
#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"
#include "presentation/viewmodels/shell/MainWindowViewModel.h"
#include "simulation/app/ConfigurationState.h"
#include "simulation/app/EnhancedMode.h"
#include "simulation/app/MemoryFileStagingStore.h"
#include "support/app/FileTargetFixture.h"
#include "support/app/UwfMaintenanceFixture.h"
#include "support/app/WorkerGate.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;
using namespace uwf::presentation;
using namespace uwf::test::application;

enum class Partition { Unread, Ready, Unavailable, NotElevated, Horm, Refreshing, Applying, Reconciliation, RefreshFailed };
enum class Command { SafeShutdown, SafeRestart, DirectRestart };
enum class Restriction { None, Unavailable, Administrator, Horm, Wait, Reconciliation, ReadFailure };
enum class UsageObservation { Enabled, Disabled, Unavailable, Failed };
enum class RuntimeModeScenario {
  Combined,
  Horm,
  ReleasedRom,
  FilterOff,
  NextOnly,
  FlagsUnknown,
  FlagsFailed,
  Unavailable,
  WithoutCompatibility,
  FullCommit,
  Ram,
  RomUnknown
};

class EmptyTranslator final : public QTranslator {
 public:
  bool isEmpty() const override { return false; }
  QString translate(const char*, const char*, const char*, int) const override { return QLatin1String(""); }
};

QString restrictionText(const Restriction restriction) {
  switch (restriction) {
    case Restriction::None:
      return {};
    case Restriction::Unavailable:
      return PresentationText::tr("UWF is currently unavailable");
    case Restriction::Administrator:
      return PresentationText::tr("Administrator privileges are required; the overlay configuration is read-only.");
    case Restriction::Horm:
      return PresentationText::tr("UWF settings cannot be changed while HORM is active.");
    case Restriction::Wait:
      return PresentationText::tr("Please wait for the operation to finish.");
    case Restriction::Reconciliation:
      return PresentationText::tr("Some changes may already have been written. Refresh the system state before retrying.");
    case Restriction::ReadFailure:
      return QStringLiteral("published state unavailable");
  }
  return {};
}

app::ApplicationState availableState() {
  app::ApplicationState state;
  state.snapshot.uwfAvailable = true;
  state.snapshot.elevated = true;
  state.snapshot.next.filter.enabled = true;
  state.snapshot.next.overlay = {core::OverlayType::RAM, 8192, 4096, 6144};
  state.snapshot.next.volumes = {{"Volume{11111111-2222-3333-4444-555555555555}", "C:", true, core::VolumeBinding::DriveLetter, std::string("C:")}};
  state.snapshot.current = state.snapshot.next;
  state.volumes = {{"C:", "Volume{11111111-2222-3333-4444-555555555555}", "NTFS", "System", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported}};
  state.hormEnabled = false;
  state.snapshot.runtime.hormEnabled = false;
  state.snapshot.runtime.currentOverlayFlags = 0;
  state.snapshot.runtime.systemVolumeProtected = true;
  state.snapshot.runtime.romDisposition = 0;
  state.snapshot.runtime.fullCommit = false;
  state.overlayPolicy.transportAvailable = true;
  state.overlayPolicy.current.filterEnabled = state.overlayPolicy.next.filterEnabled = {app::PolicyObservationStatus::Available, true};
  state.overlayPolicy.current.overlayType = state.overlayPolicy.next.overlayType = {app::PolicyObservationStatus::Available, 0};
  state.overlayPolicy.current.flags = state.overlayPolicy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{0}};
  state.overlayPolicy.resetPersistentState = {app::PolicyObservationStatus::Available, 0};
  return state;
}

class ControlledStateSource final : public app::ApplicationStateSource {
 public:
  app::ApplicationState next = availableState();
  std::shared_ptr<test::WorkerGate> gate;
  int reads = 0;
  int publishedReads = 0;
  bool failPublished = false;
  bool failReads = false;
  ReadOperation readOperation(app::UwfCapability) override {
    ++reads;
    return prepareOperation(failReads);
  }
  ReadOperation publishedOperation(app::UwfCapability) override {
    ++publishedReads;
    return prepareOperation(failPublished);
  }

 private:
  ReadOperation prepareOperation(const bool fail) const {
    return [candidate = next, gate = gate, fail](const std::stop_token stop) {
      if (gate) gate->wait(stop);
      if (fail) throw std::runtime_error("published state unavailable");
      return candidate;
    };
  }
};

class WindowStagingSource final : public app::CommitTraversalSource {
 public:
  const std::thread::id owner = std::this_thread::get_id();
  std::shared_ptr<test::WorkerGate> gate = std::make_shared<test::WorkerGate>();
  mutable std::atomic_bool blockNext{false};
  mutable std::atomic_int ownerObservations{0}, workerObservations{0};
  app::FileCommitTarget resolveFileTarget(const QString& path) const override {
    observe();
    if (blockNext.exchange(false)) gate->wait();
    return memoryFileTarget(path);
  }
  std::string driveLetterForPath(const QString& path) const override {
    observe();
    return path.left(2).toUpper().toStdString();
  }
  std::optional<app::CommitNode> inspect(const app::CommitNode&) const override { throw std::logic_error("Unexpected metadata read"); }
  bool exists(const app::CommitNode&) const override { throw std::logic_error("Unexpected existence read"); }
  QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override {
    throw std::logic_error("Unexpected traversal");
  }

 private:
  void observe() const {
    if (std::this_thread::get_id() == owner)
      ++ownerObservations;
    else
      ++workerObservations;
  }
};

class ControlledConfigurationActions final : public app::ConfigurationActions {
 public:
  std::shared_ptr<test::WorkerGate> gate = std::make_shared<test::WorkerGate>();
  std::optional<app::ConfigurationApplyResult> applyResult;
  int prepared = 0;
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
    ++prepared;
    return [gate = gate] {
      gate->wait();
      app::ConfigurationApplyResult result;
      result.steps.push_back({"Filter", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::None,
                              app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}});
      return result;
    };
  }
  ApplyOperation prepareApply(const app::ConfigurationPlan& plan) override {
    if (!applyResult) return ConfigurationActions::prepareApply(plan);
    ++prepared;
    return [gate = gate, result = *applyResult] {
      gate->wait();
      return result;
    };
  }
};

class WindowMaintenanceActions final : public app::OverlayMaintenanceActions {
 public:
  app::OverlayMaintenanceSnapshot current;
  std::optional<app::OverlayMaintenanceSnapshot> observedAfterRead;
  std::shared_ptr<test::WorkerGate> readGate;
  std::atomic_int reads{0}, writes{0};
  WindowMaintenanceActions() {
    current.filterEnabled = true;
    current.overlayCommit = false;
    current.hormEnabled = false;
    current.hibernationAvailable = true;
    current.currentOverlayType = 0;
    current.currentFlags = app::OverlayFlags{0};
    current.nextFlags = app::OverlayFlags{0};
    current.romDisposition = 0;
  }
  app::OverlayMaintenanceSnapshot observe(const std::stop_token stop) override {
    ++reads;
    if (readGate) readGate->wait(stop);
    if (observedAfterRead) current = *observedAfterRead;
    return current;
  }
  std::optional<app::OverlayMaintenanceSnapshot> snapshot() const override { return current; }
  app::OverlayMaintenanceResult fullCommit(std::stop_token) override {
    ++writes;
    return {};
  }
  app::OverlayMaintenanceResult createSwap(const QString&, std::stop_token) override {
    ++writes;
    return {};
  }
  app::OverlayMaintenanceResult setHorm(bool, std::stop_token) override {
    ++writes;
    return {};
  }
};

class WindowUsageReader final : public app::OverlayUsageReader {
 public:
  app::OverlayUsageState next{app::OverlayUsageUnavailable{}};
  bool fail = false;
  mutable std::atomic_int calls{0};
  app::OverlayUsageState read() const override {
    ++calls;
    if (fail) throw std::runtime_error("usage read interrupted");
    return next;
  }
};

// Window workflows use semantic ports and require no widgets or native I/O.
class WindowModels final : public ViewModelFactory {
 public:
  std::shared_ptr<ControlledConfigurationActions> actions = std::make_shared<ControlledConfigurationActions>();
  std::shared_ptr<WindowUsageReader> usageReader = std::make_shared<WindowUsageReader>();
  std::shared_ptr<OverlayViewModel> overlayModel = std::make_shared<OverlayViewModel>(usageReader, std::chrono::hours{24});
  std::shared_ptr<const app::CommitTraversalSource> stagingSource = std::make_shared<StoredPathSource>();
  int reviews = 0;
  int enhancedModels = 0;
  int maintenanceModels = 0;
  std::function<std::shared_ptr<OverlayMaintenanceViewModel>(std::shared_ptr<app::OverlayMaintenanceEditor>)> maintenanceFactory;
  std::unique_ptr<MainWindowViewModel> mainWindow(app::ApplicationStateSource&, app::UwfCapability, std::shared_ptr<app::FileStagingDraft>) override {
    return {};
  }
  std::shared_ptr<SystemInformationViewModel> systemInformation() override { return {}; }
  std::shared_ptr<ImportViewModel> importCommands() override { return std::make_shared<ImportViewModel>(); }
  std::shared_ptr<LogViewModel> log() override { return {}; }
  std::shared_ptr<UserConfigExportViewModel> userConfigExport() override { return {}; }
  std::shared_ptr<FileStagingViewModel> fileStaging(QString drive, app::FileStagingDraft& draft, app::FileStagingConflictPolicy* conflicts,
                                                    const app::FileStagingScope& scope) override {
    return std::make_shared<FileStagingViewModel>(std::make_shared<app::FileStagingEditor>(std::move(drive), draft, stagingSource, conflicts, scope));
  }
  std::shared_ptr<ApplyPlanViewModel> applyPlan(app::ConfigurationChanges changes, const core::UwfSnapshot& snapshot, const app::OverlayPolicySnapshot& policy,
                                                std::shared_ptr<app::FileStagingDraft> staging,
                                                std::optional<app::OverlayMaintenanceSnapshot> maintenance = {}) override {
    ++reviews;
    return std::make_shared<ApplyPlanViewModel>(
        app::reviewApplicationConfiguration(std::move(changes), snapshot, policy, staging ? staging->editSnapshot() : std::nullopt, std::move(maintenance)),
        actions);
  }
  std::shared_ptr<EnhancedModeViewModel> enhancedMode(app::EnhancedModeActions& actions) override {
    ++enhancedModels;
    return std::make_shared<EnhancedModeViewModel>(actions);
  }
  std::shared_ptr<OverlayViewModel> overlay() override { return overlayModel; }
  std::shared_ptr<OverlayMaintenanceViewModel> overlayMaintenance(std::shared_ptr<app::OverlayMaintenanceEditor> editor = {}) override {
    ++maintenanceModels;
    return maintenanceFactory ? maintenanceFactory(std::move(editor)) : nullptr;
  }
  std::shared_ptr<OverlayFilesViewModel> overlayFiles(std::shared_ptr<VolumeConfigurationViewModel>, MainWindowViewModel*) override { return {}; }
  std::shared_ptr<RegistryPickerViewModel> registryPicker(RegistryPickerViewModel::Mode) override { return {}; }
};

class WindowFixture final {
 public:
  struct PowerRequest {
    app::PowerAction action;
    app::PowerActionDialogMode mode;
  };
  ControlledStateSource source;
  UwfMaintenanceFixture maintenance;
  std::shared_ptr<uwf::simulation::application::MemoryFileStagingStore> store = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
  std::shared_ptr<app::FileStagingDraft> draft = std::make_shared<app::FileStagingDraft>(store);
  std::shared_ptr<WindowModels> models = std::make_shared<WindowModels>();
  std::shared_ptr<uwf::simulation::application::MemoryEnhancedModeServiceControl> service =
      std::make_shared<uwf::simulation::application::MemoryEnhancedModeServiceControl>();
  uwf::simulation::application::MemoryEnhancedModeAgent borrowedAgent;
  QPointer<app::EnhancedModeAgentConnection> agent;
  QPointer<app::EnhancedModeSession> session;
  QPointer<app::PowerCoordinator> power;
  std::unique_ptr<app::EnhancedModeSession> borrowedSession;
  std::vector<PowerRequest> powerRequests;
  int warnings = 0;
  std::function<void()> duringConfirmation;
  std::unique_ptr<MainWindowViewModel> window;

  explicit WindowFixture(const bool ownSession = true, const bool ownAgent = false, const bool includeEnhancedMode = true,
                         std::function<void(WindowFixture&)> beforeInitialize = {}, const bool initializeConfiguration = true) {
    const auto repository = maintenance.repository();
    service->current = uwf::simulation::application::completeEnhancedModeStatus();
    std::unique_ptr<app::EnhancedModeSession> enhanced;
    if (includeEnhancedMode) {
      auto actions = std::make_unique<app::EnhancedModeManager>(service, repository, app::UwfCapability::Available);
      if (ownAgent) {
        auto ownedAgent = std::make_unique<uwf::simulation::application::MemoryEnhancedModeAgent>();
        agent = ownedAgent.get();
        enhanced = std::make_unique<app::EnhancedModeSession>(std::move(actions), std::move(ownedAgent));
      } else {
        agent = &borrowedAgent;
        enhanced = std::make_unique<app::EnhancedModeSession>(std::move(actions), borrowedAgent);
      }
      session = enhanced.get();
    }
    app::PowerInteraction interaction;
    interaction.confirm = [this](const app::PowerActionDialogRequest request) {
      powerRequests.push_back({request.action, request.mode});
      if (duringConfirmation) duringConfirmation();
      return app::PowerActionDialogOutcome::Canceled;
    };
    interaction.warning = [this](const QString&, const QString&) { ++warnings; };
    interaction.describeStagingResult = [](const auto&) { return QString{}; };
    auto powerCoordinator = std::make_unique<app::PowerCoordinator>(
        app::PowerServices{repository, *store, app::UwfCapability::Available, {}, enhanced ? enhanced->preshutdownControl() : app::PreshutdownCommitControl{}},
        std::move(interaction));
    power = powerCoordinator.get();
    window = std::make_unique<MainWindowViewModel>(source, app::UwfCapability::Available, draft);
    if (initializeConfiguration) window->initializeConfiguration(16 * 1024, QStringLiteral("C:"));
    if (!ownSession) borrowedSession = std::move(enhanced);
    if (beforeInitialize) beforeInitialize(*this);
    window->initializeWorkflows(models, std::move(enhanced), std::move(powerCoordinator));
    if (borrowedSession) {
      window->bindEnhancedMode(*borrowedSession);
      window->refreshEnhancedModeStatus();
    }
  }
  ~WindowFixture() {
    if (source.gate) source.gate->release();
    models->actions->gate->release();
    window.reset();
    borrowedSession.reset();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
  std::shared_ptr<ApplyPlanViewModel> reviewEdit() {
    window->configuration()->global()->editFilterEnabled(false);
    return window->reviewConfiguration();
  }
  void invoke(const Command command) {
    switch (command) {
      case Command::SafeShutdown:
        window->safeShutdown();
        break;
      case Command::SafeRestart:
        window->safeRestart();
        break;
      case Command::DirectRestart:
        window->directRestart();
        break;
    }
  }
};

class MainWindowViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void interactionPartitionsAgreeWithCommandRouting_data();
  void interactionPartitionsAgreeWithCommandRouting();
  void configurationApplyBlocksPowerActionsUntilCompletion();
  void sharedStagingPreparationBlocksWindowInteractionsUntilCompletion();
  void reconciliationProjectsPublishedStateAndUserRefreshReads_data();
  void reconciliationProjectsPublishedStateAndUserRefreshReads();
  void powerConfirmationBlocksWindowWritesAndExit();
  void ownedWorkflowsReleaseBorrowedDependenciesInOrder_data();
  void ownedWorkflowsReleaseBorrowedDependenciesInOrder();
  void initializationObserversMayDestroyTheWindow();
  void absentEnhancedModeHasNoModelOrAffordance();
  void shellVolumeProjection_data();
  void shellVolumeProjection();
  void protectedUnletteredPartitionsFollowLetteredTabsAndKeepIndependentStates();
  void savedStagingTargetsKeepExactlyOneManagementPageAcrossMountChanges_data();
  void savedStagingTargetsKeepExactlyOneManagementPageAcrossMountChanges();
  void runtimeModeRestrictionsAreSharedWithoutBlockingConfigurationForRom_data();
  void runtimeModeRestrictionsAreSharedWithoutBlockingConfigurationForRom();
  void knownHormControlsChangesIndependentlyOfOverlayMetadata_data();
  void knownHormControlsChangesIndependentlyOfOverlayMetadata();
  void registryCommitAvailabilityFollowsTheSystemVolume_data();
  void registryCommitAvailabilityFollowsTheSystemVolume();
  void windowRefreshObserversMayDestroyPresentation();
  void stagingRefreshKeepsObservationsOffTheOwnerThread_data();
  void stagingRefreshKeepsObservationsOffTheOwnerThread();
  void destroyingARefreshingWindowReleasesTheSharedDraft();
  void aNewRefreshSupersedesTheOwnedStagingRead();
  void explicitRefreshPublishesIndependentStateWhenStagingReadFails_data();
  void explicitRefreshPublishesIndependentStateWhenStagingReadFails();
  void explicitRefreshRecoversIndependentStagingWhenStateReadFails();
  void explicitRefreshRetainsBothDraftsWhenStagingDiscardFails();
  void explicitRefreshCanRetryAnUnknownWriteWithoutDiscardingIntentOnFailure();
  void refreshDiscardsMaintenanceOnlyWhenRequestedAndItsDependenciesSucceed_data();
  void refreshDiscardsMaintenanceOnlyWhenRequestedAndItsDependenciesSucceed();
  void reviewRetriesStagingInTheBackgroundAndPreservesConfigurationEdits();
  void discardingStagingUsesTheKnownCachedBaseline();
  void windowStateRetainsCommittedBaselineAfterReadFailure();
  void refreshFailureDisablesUwfOperationsUntilSuccessfulRefresh();
  void refreshFailureBlocksAnAlreadyOpenedEnhancedModeEditor();
  void persistentAutomaticCommitUsesAppliedStateAndSavedTargets();
  void readOnlyMediaPausesAutomaticCommitsForTheCurrentSession_data();
  void readOnlyMediaPausesAutomaticCommitsForTheCurrentSession();
  void hostMetadataEditingUsesSystemRegistryDurability_data();
  void hostMetadataEditingUsesSystemRegistryDurability();
  void automaticCommitStatusRequiresKnownCurrentFacts_data();
  void automaticCommitStatusRequiresKnownCurrentFacts();
  void automaticCommitSummaryKeepsUnknownAndKnownScopesUntilRefresh();
  void runtimeModeTextUsesConfirmedCurrentState_data();
  void runtimeModeTextUsesConfirmedCurrentState();
  void runtimeModeTextRetranslatesWithoutAnotherRead();
  void windowReconciliationRequiresEveryWrittenStoreToBeReadable_data();
  void windowReconciliationRequiresEveryWrittenStoreToBeReadable();
  void postApplyObservationRetainsEveryUnfinishedOrdinaryIntent_data();
  void postApplyObservationRetainsEveryUnfinishedOrdinaryIntent();
  void windowStatusBindingSurvivesItsSessionAndCanBeReplaced();
  void maintenanceDraftUsesTheUnifiedReviewAndDiscardWithoutReleasingCurrentHorm_data();
  void maintenanceDraftUsesTheUnifiedReviewAndDiscardWithoutReleasingCurrentHorm();
  void windowRefreshAdoptsMaintenanceWithoutStartingAnotherRead();
  void maintenanceReadDoesNotBlockWindowInteractionsOrPower();
  void protectAllDraftReconcilesOnlySuccessfulConfirmedApply_data();
  void protectAllDraftReconcilesOnlySuccessfulConfirmedApply();
  void liveUsageFollowsWindowCompositionWithoutWidgets_data();
  void liveUsageFollowsWindowCompositionWithoutWidgets();
  void importModelCoordinatesDraftsAndRechecksAdmission_data();
  void importModelCoordinatesDraftsAndRechecksAdmission();
  void importWaitsForMaintenanceDetails_data();
  void importWaitsForMaintenanceDetails();
};

void MainWindowViewModelTests::runtimeModeTextUsesConfirmedCurrentState_data() {
  QTest::addColumn<int>("scenario");
  QTest::addColumn<QString>("expected");
  const auto row = [](const char* name, RuntimeModeScenario scenario, const QString& expected) {
    QTest::newRow(name) << static_cast<int>(scenario) << expected;
  };
  const QString flags = QStringLiteral("Freespace Passthrough · Persistent Overlay · Compatibility mode");
  row("combined", RuntimeModeScenario::Combined, QStringLiteral("Read-only ROM · ") + flags);
  row("horm-suppresses-duplicate-rom", RuntimeModeScenario::Horm, QStringLiteral("HORM · ") + flags);
  row("rom-released-at-runtime", RuntimeModeScenario::ReleasedRom, flags);
  row("current-filter-off", RuntimeModeScenario::FilterOff, QStringLiteral("Compatibility mode"));
  row("next-flags-are-not-running", RuntimeModeScenario::NextOnly, QStringLiteral("Compatibility mode"));
  row("unknown-flags", RuntimeModeScenario::FlagsUnknown, QStringLiteral("Compatibility mode"));
  row("failed-flags-with-stale-value", RuntimeModeScenario::FlagsFailed, QStringLiteral("Compatibility mode"));
  row("uwf-unavailable", RuntimeModeScenario::Unavailable, {});
  row("without-compatibility", RuntimeModeScenario::WithoutCompatibility, QStringLiteral("Read-only ROM · Freespace Passthrough · Persistent Overlay"));
  row("full-commit-remains-rom", RuntimeModeScenario::FullCommit, QStringLiteral("Read-only ROM · ") + flags);
  row("ram-does-not-have-persistent-overlay", RuntimeModeScenario::Ram, QStringLiteral("Freespace Passthrough · Compatibility mode"));
  row("unknown-rom-disposition", RuntimeModeScenario::RomUnknown, flags);
}

void MainWindowViewModelTests::runtimeModeTextUsesConfirmedCurrentState() {
  QFETCH(int, scenario);
  QFETCH(QString, expected);
  const auto variation = static_cast<RuntimeModeScenario>(scenario);
  ControlledStateSource source;
  auto& state = source.next;
  state.snapshot.current.overlay.type = core::OverlayType::Disk;
  state.snapshot.runtime.romDisposition = 0;
  state.snapshot.runtime.fullCommit = false;
  state.overlayPolicy.current.flags = {app::PolicyObservationStatus::Available,
                                       app::OverlayFlags{app::OverlayFlags::Passthrough | app::OverlayFlags::Persistent | app::OverlayFlags::ReadOnlyMedia}};
  state.overlayPolicy.next.flags = state.overlayPolicy.current.flags;
  switch (variation) {
    case RuntimeModeScenario::Horm:
      state.hormEnabled = true;
      break;
    case RuntimeModeScenario::ReleasedRom:
      state.snapshot.runtime.romDisposition = 3;
      break;
    case RuntimeModeScenario::FilterOff:
      state.snapshot.current.filter.enabled = false;
      break;
    case RuntimeModeScenario::NextOnly:
      state.overlayPolicy.current.flags.value = app::OverlayFlags{};
      break;
    case RuntimeModeScenario::FlagsUnknown:
      state.overlayPolicy.current.flags = {};
      break;
    case RuntimeModeScenario::FlagsFailed:
      state.overlayPolicy.current.flags.status = app::PolicyObservationStatus::Failed;
      break;
    case RuntimeModeScenario::Unavailable:
      state.snapshot.uwfAvailable = false;
      break;
    case RuntimeModeScenario::FullCommit:
      state.snapshot.runtime.romDisposition = 4;
      state.snapshot.runtime.fullCommit = true;
      break;
    case RuntimeModeScenario::Ram:
      state.snapshot.current.overlay.type = core::OverlayType::RAM;
      state.overlayPolicy.current.flags.value->raw &= ~app::OverlayFlags::ReadOnlyMedia;
      break;
    case RuntimeModeScenario::RomUnknown:
      state.snapshot.runtime.romDisposition.reset();
      break;
    default:
      break;
  }
  MainWindowViewModel model(source, app::UwfCapability::Available);
  QVERIFY(model.runtimeModesText(true).isEmpty());
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(model.runtimeModesText(variation != RuntimeModeScenario::WithoutCompatibility), expected);
  QCOMPARE(source.reads, 1);
}

void MainWindowViewModelTests::runtimeModeTextRetranslatesWithoutAnotherRead() {
  class ModeTranslator final : public QTranslator {
   public:
    bool isEmpty() const override { return false; }
    QString translate(const char* context, const char* text, const char*, int) const override {
      const auto source = QString::fromUtf8(text);
      if (qstrcmp(context, "uwf") == 0 &&
          (source == QLatin1String("Freespace Passthrough") || source == QLatin1String("Persistent Overlay") || source == QLatin1String("Compatibility mode")))
        return QStringLiteral("translated:") + source;
      return {};
    }
  };
  ControlledStateSource source;
  source.next.snapshot.current.overlay.type = core::OverlayType::Disk;
  source.next.overlayPolicy.current.flags = {app::PolicyObservationStatus::Available,
                                             app::OverlayFlags{app::OverlayFlags::Passthrough | app::OverlayFlags::Persistent}};
  MainWindowViewModel model(source, app::UwfCapability::Available);
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  const auto original = model.runtimeModesText(true);
  ModeTranslator translator;
  QVERIFY(QCoreApplication::installTranslator(&translator));
  const auto removeTranslator = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });
  QSignalSpy changed(&model, &MainWindowViewModel::interactionAvailabilityChanged);
  model.retranslate();
  QCOMPARE(changed.count(), 1);
  QCOMPARE(model.runtimeModesText(true), QStringLiteral("translated:Freespace Passthrough · translated:Persistent Overlay · translated:Compatibility mode"));
  QCOMPARE(source.reads, 1);
  QVERIFY(model.runtimeModesText(true) != original);
}

void MainWindowViewModelTests::persistentAutomaticCommitUsesAppliedStateAndSavedTargets() {
  WindowFixture fixture;
  const app::FileStagingEntry first{app::FileStagingKind::File, QStringLiteral("C:\\Data\\one.bin"),
                                    app::FileCommitTarget{fixtureVolumeId, QStringLiteral("\\Data\\one.bin")}};
  const app::FileStagingEntry second{app::FileStagingKind::File, QStringLiteral("D:\\Data\\two.bin"),
                                     app::FileCommitTarget{otherFixtureVolumeId, QStringLiteral("\\Data\\two.bin")}};
  fixture.store->entries = {first, second};
  auto& state = fixture.source.next;
  state.snapshot.current.volumes.push_back({uwf::simulation::application::fixtureVolumeDName, "D:", true, core::VolumeBinding::VolumeId, std::string("D:")});
  state.snapshot.next.volumes.push_back({uwf::simulation::application::fixtureVolumeDName, "D:", false, core::VolumeBinding::VolumeId, std::string("D:")});
  state.snapshot.current.overlay.type = state.snapshot.next.overlay.type = core::OverlayType::Disk;
  auto& policy = state.overlayPolicy;
  policy.transportAvailable = true;
  policy.current.filterEnabled = policy.next.filterEnabled = {app::PolicyObservationStatus::Available, true};
  policy.current.overlayType = policy.next.overlayType = {app::PolicyObservationStatus::Available, 1};
  policy.current.flags = policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{app::OverlayFlags::Persistent}};
  policy.resetPersistentState = {app::PolicyObservationStatus::Available, 0};
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  auto enhanced = fixture.window->enhancedModeModel();
  QVERIFY(enhanced);
  enhanced->refreshStatus();
  QCOMPARE(enhanced->statusText().staging, QStringLiteral("Partially paused"));
  QVERIFY(fixture.window->powerActionsAllowed());
  QVERIFY(enhanced->statusText().canApply);
  QVERIFY(enhanced->statusText().canRemove);

  // An unapplied removal must not hide D:'s still-saved automatic commit target.
  fixture.draft->replace({first});
  QCOMPARE(enhanced->statusText().staging, QStringLiteral("Partially paused"));

  state.snapshot.next.volumes.back().isProtected = true;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  QCOMPARE(enhanced->statusText().staging, QStringLiteral("Paused"));

  fixture.window->configuration()->global()->editFilterEnabled(false);
  QCOMPARE(enhanced->statusText().staging, QStringLiteral("Paused"));
  policy.resetPersistentState.value = 1;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  QCOMPARE(enhanced->statusText().staging, QStringLiteral("Ready"));

  policy.resetPersistentState.value = 0;
  state.snapshot.next.filter.enabled = false;
  policy.next.filterEnabled.value = false;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  QCOMPARE(enhanced->statusText().staging, QStringLiteral("Ready"));
  QCOMPARE(fixture.store->entries, (QList<app::FileStagingEntry>{first, second}));
  QCOMPARE(fixture.service->disables, 0);
}

void MainWindowViewModelTests::readOnlyMediaPausesAutomaticCommitsForTheCurrentSession_data() {
  QTest::addColumn<bool>("filterEnabled");
  QTest::addColumn<bool>("currentRom");
  QTest::addColumn<bool>("nextRom");
  QTest::addColumn<unsigned>("disposition");
  QTest::addColumn<bool>("paused");
  QTest::addColumn<bool>("metadataWritable");
  QTest::newRow("active-rom-next-off") << true << true << false << 0u << true << false;
  QTest::newRow("active-rom-next-on") << true << true << true << 0u << true << false;
  QTest::newRow("next-only") << true << false << true << 0u << false << true;
  QTest::newRow("filter-off") << false << true << true << 0u << false << true;
  QTest::newRow("released-rom") << true << true << true << 1u << false << true;
  QTest::newRow("full-commit-remains-rom") << true << true << false << 4u << true << true;
}

void MainWindowViewModelTests::readOnlyMediaPausesAutomaticCommitsForTheCurrentSession() {
  QFETCH(bool, filterEnabled);
  QFETCH(bool, currentRom);
  QFETCH(bool, nextRom);
  QFETCH(unsigned, disposition);
  QFETCH(bool, paused);
  QFETCH(bool, metadataWritable);
  WindowFixture fixture;
  const app::FileStagingEntry entry{app::FileStagingKind::File, QStringLiteral("C:\\Data\\saved.bin"),
                                    app::FileCommitTarget{fixtureVolumeId, QStringLiteral("\\Data\\saved.bin")}};
  fixture.store->entries = {entry};
  auto& state = fixture.source.next;
  state.snapshot.current.filter.enabled = filterEnabled;
  state.snapshot.runtime.romDisposition = disposition;
  state.snapshot.runtime.fullCommit = false;
  state.snapshot.runtime.currentOverlayFlags = currentRom ? app::OverlayFlags::ReadOnlyMedia : 0u;
  state.overlayPolicy.transportAvailable = true;
  state.overlayPolicy.current.filterEnabled = {app::PolicyObservationStatus::Available, filterEnabled};
  state.overlayPolicy.next.filterEnabled = {app::PolicyObservationStatus::Available, state.snapshot.next.filter.enabled};
  state.overlayPolicy.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{currentRom ? app::OverlayFlags::ReadOnlyMedia : 0u}};
  state.overlayPolicy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{nextRom ? app::OverlayFlags::ReadOnlyMedia : 0u}};
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  auto enhanced = fixture.window->enhancedModeModel();
  QVERIFY(enhanced);
  enhanced->refreshStatus();
  const auto text = enhanced->statusText();
  QCOMPARE(text.staging, paused ? QStringLiteral("Paused") : filterEnabled ? QStringLiteral("Ready") : QStringLiteral("No files to commit"));
  if (paused) {
    QVERIFY(text.stagingDescription.contains(disposition == compat::disposition::fullCommit
                                                 ? automaticCommitDescription(app::AutomaticCommitDisposition::SkipFullCommit)
                                                 : automaticCommitDescription(app::AutomaticCommitDisposition::SkipReadOnlyMedia)));
    QVERIFY(!text.stagingDescription.contains(QStringLiteral("Persistent")));
  }
  QCOMPARE(text.canApply, metadataWritable);
  QCOMPARE(text.canRemove, metadataWritable);
  if (!metadataWritable)
    QCOMPARE(text.issues, QStringLiteral("File and registry commits are unavailable while Read-Only Media mode is active."));
  else
    QVERIFY(!text.issues.contains(QStringLiteral("Read-Only Media")));
  QVERIFY(fixture.window->powerActionsAllowed());
  QVERIFY(fixture.window->interactionState().powerHint.isEmpty());
  fixture.window->safeShutdown();
  fixture.window->safeRestart();
  QCOMPARE(fixture.powerRequests.size(), std::size_t{2});
  QCOMPARE(fixture.powerRequests.front().mode, app::PowerActionDialogMode::Safe);
  QCOMPARE(fixture.powerRequests.back().mode, app::PowerActionDialogMode::Safe);
  const auto volume = fixture.window->configuration()->volume(QStringLiteral("C:"));
  QVERIFY(volume);
  QCOMPARE(volume->commitAvailability().files, filterEnabled && !paused);
  const auto staging = fixture.window->fileStagingModel(volume);
  QVERIFY(staging);
  staging->load();
  QTRY_VERIFY(!staging->busy());
  QCOMPARE(staging->rowCount(), 1);
  QCOMPARE(staging->writable(), metadataWritable);
  app::AutomaticCommitSummary expectedAutomatic;
  expectedAutomatic.include(paused          ? (disposition == compat::disposition::fullCommit ? app::AutomaticCommitDisposition::SkipFullCommit
                                                                                              : app::AutomaticCommitDisposition::SkipReadOnlyMedia)
                            : filterEnabled ? app::AutomaticCommitDisposition::CheckDurability
                                            : app::AutomaticCommitDisposition::AlreadyPersistent);
  QCOMPARE(staging->availability().automaticCommit, expectedAutomatic);
  QCOMPARE(staging->index(0).data(StatusTextRole).toString(), automaticCommitPresentation(expectedAutomatic).status);
  QCOMPARE(fixture.store->entries, (QList<app::FileStagingEntry>{entry}));
}

void MainWindowViewModelTests::hostMetadataEditingUsesSystemRegistryDurability_data() {
  QTest::addColumn<int>("scenario");
  QTest::addColumn<app::RuntimeWriteRestriction>("restriction");
  QTest::newRow("protected-system-ROM") << 0 << app::RuntimeWriteRestriction::ReadOnlyMediaActive;
  QTest::newRow("future-writes-direct-after-FullCommit-trigger") << 1 << app::RuntimeWriteRestriction::None;
  QTest::newRow("future-writes-direct-after-FullCommit-observation") << 2 << app::RuntimeWriteRestriction::None;
  QTest::newRow("unprotected-system-ROM") << 3 << app::RuntimeWriteRestriction::None;
  QTest::newRow("unknown-system-protection") << 4 << app::RuntimeWriteRestriction::StateUnavailable;
  QTest::newRow("UWF-unavailable-does-not-need-a-registry-commit") << 5 << app::RuntimeWriteRestriction::None;
}

void MainWindowViewModelTests::hostMetadataEditingUsesSystemRegistryDurability() {
  QFETCH(int, scenario);
  QFETCH(app::RuntimeWriteRestriction, restriction);
  WindowFixture fixture;
  auto& state = fixture.source.next;
  state.volumes.push_back({"D:", uwf::simulation::application::fixtureVolumeDName, "NTFS", "Data", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported});
  state.snapshot.current.volumes.push_back({uwf::simulation::application::fixtureVolumeDName, "D:", true, core::VolumeBinding::VolumeId, std::string("D:")});
  state.snapshot.next.volumes.push_back(state.snapshot.current.volumes.back());
  state.snapshot.runtime.currentOverlayFlags = app::OverlayFlags::ReadOnlyMedia;
  state.overlayPolicy.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{app::OverlayFlags::ReadOnlyMedia}};
  if (scenario == 1) state.snapshot.runtime.romDisposition = compat::disposition::fullCommit;
  if (scenario == 2) state.snapshot.runtime.fullCommit = true;
  if (scenario == 3) {
    state.snapshot.runtime.systemVolumeProtected = false;
    state.snapshot.current.volumes.front().isProtected = false;
  }
  if (scenario == 4) state.snapshot.runtime.systemVolumeProtected.reset();
  if (scenario == 5) {
    state.snapshot.uwfAvailable = false;
    state.snapshot.runtime = {};
    state.overlayPolicy = {};
  }
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  QCOMPARE(fixture.window->stagingEditingRestriction(), restriction);
  auto enhanced = fixture.window->enhancedModeModel();
  QVERIFY(enhanced);
  enhanced->refreshStatus();
  QCOMPARE(enhanced->statusText().canApply, restriction == app::RuntimeWriteRestriction::None);
  QCOMPARE(enhanced->statusText().canRemove, restriction == app::RuntimeWriteRestriction::None);
  if (scenario == 5) return;
  const auto data = fixture.window->configuration()->volume(QStringLiteral("D:"));
  QVERIFY(data);
  const auto staging = fixture.window->fileStagingModel(data);
  QVERIFY(staging);
  QCOMPARE(staging->writable(), restriction == app::RuntimeWriteRestriction::None);
  // A direct future registry write never authorizes committing historical ROM data.
  QVERIFY(!data->commitAvailability().files);
}

void MainWindowViewModelTests::automaticCommitStatusRequiresKnownCurrentFacts_data() {
  QTest::addColumn<int>("scenario");
  QTest::addColumn<QString>("expected");
  QTest::newRow("ready") << 0 << QStringLiteral("Ready");
  QTest::newRow("unread") << 1 << QStringLiteral("Unavailable");
  QTest::newRow("refresh-failed") << 2 << QStringLiteral("Unavailable");
  QTest::newRow("reconciliation") << 3 << QStringLiteral("Unavailable");
  QTest::newRow("uwf-unavailable") << 4 << QStringLiteral("Unavailable");
  QTest::newRow("flags-unreadable") << 5 << QStringLiteral("Unavailable");
  QTest::newRow("filter-unreadable") << 6 << QStringLiteral("Unavailable");
  QTest::newRow("horm-unknown") << 7 << QStringLiteral("Unavailable");
  QTest::newRow("target-not-protected-current") << 8 << QStringLiteral("No files to commit");
  QTest::newRow("target-unresolved") << 9 << QStringLiteral("Unavailable");
  QTest::newRow("failed-flags-retain-stale-ROM-value") << 10 << QStringLiteral("Unavailable");
  QTest::newRow("failed-next-flags-retain-stale-persistent-value") << 11 << QStringLiteral("Ready");
}

void MainWindowViewModelTests::automaticCommitStatusRequiresKnownCurrentFacts() {
  QFETCH(int, scenario);
  QFETCH(QString, expected);
  WindowFixture fixture;
  app::FileStagingEntry entry{app::FileStagingKind::File, QStringLiteral("C:\\Data\\saved.bin"),
                              app::FileCommitTarget{fixtureVolumeId, QStringLiteral("\\Data\\saved.bin")}};
  if (scenario == 9) entry.fileTarget.reset();
  fixture.store->entries = {entry};
  auto& state = fixture.source.next;
  state.snapshot.runtime.romDisposition = 0;
  state.snapshot.runtime.fullCommit = false;
  state.overlayPolicy.transportAvailable = true;
  state.overlayPolicy.current.filterEnabled = state.overlayPolicy.next.filterEnabled = {app::PolicyObservationStatus::Available, true};
  state.overlayPolicy.current.flags = state.overlayPolicy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{0}};
  if (scenario == 4) state.snapshot.uwfAvailable = false;
  if (scenario == 5) {
    state.overlayPolicy.current.flags.status = app::PolicyObservationStatus::Failed;
    state.snapshot.runtime.currentOverlayFlags.reset();
  }
  if (scenario == 10) {
    state.overlayPolicy.current.flags = {app::PolicyObservationStatus::Failed, app::OverlayFlags{app::OverlayFlags::ReadOnlyMedia}};
    state.snapshot.runtime.currentOverlayFlags.reset();
  }
  if (scenario == 11) {
    state.snapshot.current.overlay.type = state.snapshot.next.overlay.type = core::OverlayType::Disk;
    state.overlayPolicy.current.overlayType = state.overlayPolicy.next.overlayType = {app::PolicyObservationStatus::Available, 1};
    state.overlayPolicy.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{app::OverlayFlags::Persistent}};
    state.overlayPolicy.next.flags = {app::PolicyObservationStatus::Failed, app::OverlayFlags{app::OverlayFlags::Persistent}};
  }
  if (scenario == 6) state.overlayPolicy.current.filterEnabled.status = app::PolicyObservationStatus::Failed;
  if (scenario == 7) {
    state.hormEnabled.reset();
    state.snapshot.runtime.hormEnabled.reset();
  }
  if (scenario == 8) {
    state.snapshot.current.volumes.clear();
    state.snapshot.runtime.systemVolumeProtected = false;
  }
  if (scenario != 1) {
    fixture.window->refresh();
    QTRY_VERIFY(!fixture.window->refreshing());
  }
  auto enhanced = fixture.window->enhancedModeModel();
  QVERIFY(enhanced);
  enhanced->refreshStatus();
  if (scenario == 2) {
    fixture.source.failReads = true;
    fixture.window->refresh();
    QTRY_VERIFY(!fixture.window->refreshing());
  }
  if (scenario == 3) fixture.window->requireReconciliation();
  QCOMPARE(enhanced->statusText().staging, expected);
  if (scenario == 10) {
    QVERIFY(!enhanced->statusText().stagingDescription.contains(automaticCommitDescription(app::AutomaticCommitDisposition::SkipReadOnlyMedia)));
    QVERIFY(!enhanced->statusText().showFallbackCommitHint);
    QCOMPARE(fixture.window->stagingEditingRestriction(), app::RuntimeWriteRestriction::StateUnavailable);
  }
  if (scenario == 11) {
    QVERIFY(!enhanced->statusText().stagingDescription.contains(automaticCommitDescription(app::AutomaticCommitDisposition::SkipRetainedOverlay)));
    QVERIFY(enhanced->statusText().canApply);
    QVERIFY(enhanced->statusText().canRemove);
  }
}

void MainWindowViewModelTests::automaticCommitSummaryKeepsUnknownAndKnownScopesUntilRefresh() {
  WindowFixture fixture;
  const app::FileStagingEntry known{app::FileStagingKind::File, QStringLiteral("C:\\Data\\known.bin"),
                                    app::FileCommitTarget{fixtureVolumeId, QStringLiteral("\\Data\\known.bin")}};
  const app::FileStagingEntry unresolved{app::FileStagingKind::File, QStringLiteral("D:\\Data\\unresolved.bin")};
  fixture.store->entries = {known, unresolved};
  auto& state = fixture.source.next;
  state.overlayPolicy.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{app::OverlayFlags::ReadOnlyMedia}};
  state.snapshot.runtime.currentOverlayFlags = app::OverlayFlags::ReadOnlyMedia;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  auto enhanced = fixture.window->enhancedModeModel();
  QVERIFY(enhanced);
  enhanced->refreshStatus();
  const auto unknown = enhanced->statusText();
  QCOMPARE(unknown.staging, QStringLiteral("Unavailable"));
  QVERIFY(unknown.stagingDescription.contains(automaticCommitDescription(app::AutomaticCommitDisposition::Unavailable)));
  QVERIFY(unknown.stagingDescription.contains(automaticCommitDescription(app::AutomaticCommitDisposition::SkipReadOnlyMedia)));
  QVERIFY(!unknown.showFallbackCommitHint);
  // An in-memory draft edit cannot replace the saved target inventory.
  fixture.draft->replace({known});
  QCOMPARE(enhanced->statusText().stagingDescription, unknown.stagingDescription);
  QCOMPARE(fixture.draft->baseline().size(), 2);

  QSignalSpy changed(enhanced.get(), &EnhancedModeViewModel::feedbackChanged);
  state.overlayPolicy.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{0}};
  state.snapshot.runtime.currentOverlayFlags = 0;
  fixture.store->entries = {known};
  // Observing a changed saved list is explicit while an unapplied draft exists.
  // Reconciliation refreshes its baseline without treating the draft as proof.
  fixture.draft->reconcile();
  QCOMPARE(fixture.draft->baseline().size(), 1);
  QCOMPARE(fixture.draft->baseline().constFirst().fileTarget, known.fileTarget);
  QVERIFY(!fixture.draft->changes());
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  QVERIFY(changed.count() > 0);
  QCOMPARE(enhanced->statusText().staging, QStringLiteral("Ready"));
  QVERIFY(!enhanced->statusText().stagingDescription.contains(automaticCommitDescription(app::AutomaticCommitDisposition::SkipReadOnlyMedia)));
  QVERIFY(enhanced->statusText().showFallbackCommitHint);
  QCOMPARE(fixture.service->disables, 0);
}

void MainWindowViewModelTests::liveUsageFollowsWindowCompositionWithoutWidgets_data() {
  QTest::addColumn<bool>("earlyConfiguration");
  QTest::addColumn<UsageObservation>("observation");
  for (const bool early : {false, true})
    for (const auto& [name, observation] : {std::pair{"enabled", UsageObservation::Enabled}, std::pair{"disabled", UsageObservation::Disabled},
                                            std::pair{"unavailable", UsageObservation::Unavailable}, std::pair{"failed", UsageObservation::Failed}})
      QTest::newRow(qUtf8Printable(QStringLiteral("configuration-%1/%2").arg(early ? "early" : "late", name))) << early << observation;
}

void MainWindowViewModelTests::liveUsageFollowsWindowCompositionWithoutWidgets() {
  QFETCH(bool, earlyConfiguration);
  QFETCH(UsageObservation, observation);
  WindowFixture fixture(true, false, false, {}, earlyConfiguration);
  QVERIFY(!fixture.window->importModel());
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  const auto overlay = fixture.window->overlay();
  QCOMPARE(overlay, fixture.models->overlayModel);
  QVERIFY(std::holds_alternative<app::OverlayUsageEnabled>(overlay->usage()));
  const auto reader = fixture.models->usageReader;
  const auto& snapshot = fixture.source.next.snapshot;
  reader->next = app::OverlayUsageEnabled{{1, 8191}, snapshot.current.overlay};
  overlay->refreshUsage();
  QTRY_VERIFY(!overlay->refreshing());
  QCOMPARE(reader->calls.load(), 1);
  if (!earlyConfiguration) fixture.window->initializeConfiguration(16 * 1024, QStringLiteral("C:"));
  const auto global = fixture.window->configuration()->global();
  QCOMPARE(global->usage().severity, core::OverlayUsageSeverity::Critical);
  global->editWarningThreshold(100);
  const auto pending = global->pendingOverlay();
  QSignalSpy changes(global.get(), &GlobalConfigurationViewModel::usageChanged);
  if (observation == UsageObservation::Enabled)
    reader->next = app::OverlayUsageEnabled{{8191, 1}, snapshot.current.overlay};
  else if (observation == UsageObservation::Disabled)
    reader->next = app::OverlayUsageDisabled{};
  else if (observation == UsageObservation::Unavailable)
    reader->next = app::OverlayUsageUnavailable{};
  else
    reader->fail = true;
  overlay->refreshUsage();
  QTRY_VERIFY(!overlay->refreshing());
  QCOMPARE(changes.count(), 1);
  QCOMPARE(global->pendingOverlay().warningThresholdMb, pending.warningThresholdMb);
  const auto usage = global->usage();
  QCOMPARE(usage.enabled, observation == UsageObservation::Enabled || observation == UsageObservation::Failed);
  QCOMPARE(usage.stale, observation == UsageObservation::Failed);
  if (observation == UsageObservation::Enabled) QCOMPARE(usage.severity, core::OverlayUsageSeverity::Normal);
  if (observation == UsageObservation::Failed) {
    QCOMPARE(usage.severity, core::OverlayUsageSeverity::Critical);
    QVERIFY(usage.warningText.contains(QStringLiteral("usage read interrupted")));
  }
}

void MainWindowViewModelTests::importModelCoordinatesDraftsAndRechecksAdmission_data() {
  QTest::addColumn<bool>("revokeAdmission");
  QTest::addColumn<bool>("destroyOwner");
  QTest::newRow("editable") << false << false;
  QTest::newRow("read-only-after-opening") << true << false;
  QTest::newRow("owner-destroyed-after-opening") << false << true;
}

void MainWindowViewModelTests::importModelCoordinatesDraftsAndRechecksAdmission() {
  QFETCH(bool, revokeAdmission);
  QFETCH(bool, destroyOwner);
  auto actions = std::make_shared<WindowMaintenanceActions>();
  std::shared_ptr<OverlayMaintenanceViewModel> maintenance;
  WindowFixture fixture(true, false, false, [&](WindowFixture& target) {
    target.models->maintenanceFactory = [&maintenance, actions](std::shared_ptr<app::OverlayMaintenanceEditor> editor) {
      maintenance = std::make_shared<OverlayMaintenanceViewModel>(actions, std::move(editor));
      return maintenance;
    };
  });
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing() && maintenance->hasSnapshot() && !maintenance->busy());
  auto importer = fixture.window->importModel();
  QVERIFY(importer);
  QTRY_VERIFY(!importer->preparing());
  const auto configuration = fixture.window->configuration();
  if (revokeAdmission) fixture.window->setExternalOperationActive(true);
  if (destroyOwner) fixture.window.reset();
  importer->setText(QStringLiteral("filter disable\nvolume protect all\nvolume protect all\n"));
  importer->importText();
  const auto& totals = importer->totals();
  QCOMPARE(totals.success, !revokeAdmission && !destroyOwner ? 2 : 0);
  QCOMPARE(totals.duplicate, !revokeAdmission && !destroyOwner ? 1 : 0);
  QCOMPARE(totals.failed, revokeAdmission ? 3 : 0);
  QCOMPARE(configuration->global()->pendingFilterEnabled(), !revokeAdmission && !destroyOwner ? std::optional<bool>{false} : std::optional<bool>{});
  QCOMPARE(maintenance->draft().allVolumesProtected, !revokeAdmission && !destroyOwner ? std::optional<bool>{true} : std::optional<bool>{});
  QCOMPARE(actions->writes.load(), 0);
}

void MainWindowViewModelTests::importWaitsForMaintenanceDetails_data() {
  QTest::addColumn<int>("scenario");
  QTest::newRow("first-import-after-published-adoption") << 0;
  QTest::newRow("explicit-import-refreshes-cached-details") << 1;
  QTest::newRow("shares-an-existing-read") << 2;
  QTest::newRow("failed-detail-read-keeps-ordinary-import-available") << 3;
  QTest::newRow("cached-inventory-hibernation-detail-missing") << 4;
  QTest::newRow("explicit-import-rechecks-known-unavailable-hibernation") << 5;
}

void MainWindowViewModelTests::importWaitsForMaintenanceDetails() {
  QFETCH(int, scenario);
  const bool detailsAlreadyRead = scenario == 1 || scenario == 5;
  const bool inventoryAlreadyRead = detailsAlreadyRead || scenario == 4;
  const bool readFails = scenario == 3;
  auto actions = std::make_shared<WindowMaintenanceActions>();
  actions->current.filterEnabled = false;
  actions->current.currentOverlayType = actions->current.nextOverlayType = 1;
  actions->current.maximumOverlaySize = 4096;
  actions->current.volumesReadable = inventoryAlreadyRead;
  actions->current.hibernationAvailable = detailsAlreadyRead ? std::optional{scenario != 5} : std::nullopt;
  actions->current.volumes = {{QStringLiteral("11111111-2222-3333-4444-555555555555"), QStringLiteral("C:"), {}, 16ULL << 30, true, 0, 0}};
  actions->observedAfterRead = actions->current;
  actions->observedAfterRead->volumesReadable = !readFails;
  actions->observedAfterRead->hibernationAvailable = readFails ? std::nullopt : std::optional{true};
  if (readFails) actions->observedAfterRead->readError = QStringLiteral("Physical inventory read failed");
  auto gate = actions->readGate = std::make_shared<test::WorkerGate>();
  std::shared_ptr<OverlayMaintenanceViewModel> maintenance;
  WindowFixture fixture(true, false, false, [&](WindowFixture& target) {
    for (auto* session : {&target.source.next.snapshot.current, &target.source.next.snapshot.next}) {
      session->filter.enabled = false;
      session->overlay.type = core::OverlayType::Disk;
      session->overlay.maximumSizeMb = 4096;
    }
    target.source.next.overlayPolicy.current.filterEnabled.value = target.source.next.overlayPolicy.next.filterEnabled.value = false;
    target.source.next.overlayPolicy.current.overlayType.value = target.source.next.overlayPolicy.next.overlayType.value = 1;
    target.models->maintenanceFactory = [&maintenance, actions](std::shared_ptr<app::OverlayMaintenanceEditor> editor) {
      maintenance = std::make_shared<OverlayMaintenanceViewModel>(actions, std::move(editor));
      return maintenance;
    };
  });
  const auto release = qScopeGuard([gate] { gate->release(); });
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing() && maintenance->hasSnapshot());
  QCOMPARE(maintenance->snapshot().volumesReadable, inventoryAlreadyRead);
  QCOMPARE(maintenance->snapshot().hibernationAvailable, actions->current.hibernationAvailable);
  QCOMPARE(actions->reads.load(), 0);
  if (scenario == 2) maintenance->refresh();
  auto importer = fixture.window->importModel();
  QVERIFY(importer);
  importer->setText(QStringLiteral("uwfmgr filter enable\nuwfmgr volume create-swapfile C:"));
  QVERIFY(importer->preparing());
  QTRY_COMPARE(gate->started.load(), 1);
  QCOMPARE(actions->reads.load(), 1);
  QVERIFY(!importer->canImport());
  importer->importText();
  QCOMPARE(importer->totals().batches, 0);
  QVERIFY(fixture.window->configuration()->pendingConfiguration().empty());
  auto secondImporter = fixture.window->importModel();
  QVERIFY(secondImporter && secondImporter->preparing());
  QCOMPARE(actions->reads.load(), 1);
  gate->release();
  QTRY_VERIFY(!importer->preparing() && !secondImporter->preparing());
  QVERIFY(importer->canImport());
  QCOMPARE(maintenance->snapshot().hibernationAvailable, readFails ? std::nullopt : std::optional{true});
  importer->importText();
  QCOMPARE(importer->totals().batches, 1);
  QCOMPARE(importer->totals().success, readFails ? 1 : 2);
  QCOMPARE(importer->totals().failed, readFails ? 1 : 0);
  QCOMPARE(fixture.window->configuration()->pendingConfiguration().maintenance.storageVolume,
           readFails ? std::optional<QString>{} : std::optional{QStringLiteral("11111111-2222-3333-4444-555555555555")});
  QCOMPARE(actions->reads.load(), 1);
  QCOMPARE(actions->writes.load(), 0);
}

void MainWindowViewModelTests::interactionPartitionsAgreeWithCommandRouting_data() {
  QTest::addColumn<int>("partitionValue");
  QTest::addColumn<int>("commandValue");
  QTest::addColumn<QList<bool>>("expected");
  QTest::addColumn<int>("restrictionValue");
  struct Row {
    Partition partition;
    const char* name;
    // Refresh, configuration, power, exit, enhanced, volume pages, known, elevated.
    QList<bool> expected;
    Restriction restriction;
  };
  const std::vector<Row> rows{
      {Partition::Unread, "unread", {true, false, false, true, true, true, false, false}, Restriction::Unavailable},
      {Partition::Ready, "ready", {true, true, true, true, true, true, true, true}, Restriction::None},
      {Partition::Unavailable, "unavailable", {true, false, false, true, true, true, true, true}, Restriction::Unavailable},
      {Partition::NotElevated, "not-elevated", {true, false, false, true, false, true, true, false}, Restriction::Administrator},
      {Partition::Horm, "horm", {true, false, true, true, true, true, true, true}, Restriction::Horm},
      {Partition::Refreshing, "refreshing", {false, false, false, false, false, false, true, true}, Restriction::Wait},
      {Partition::Applying, "applying", {false, false, false, false, false, false, true, true}, Restriction::Wait},
      {Partition::Reconciliation, "reconciliation", {true, false, true, true, true, false, true, true}, Restriction::Reconciliation},
      {Partition::RefreshFailed, "refresh-failed", {true, false, false, true, false, false, true, true}, Restriction::ReadFailure},
  };
  for (const auto& row : rows)
    for (const auto command : {Command::SafeShutdown, Command::SafeRestart, Command::DirectRestart}) {
      const auto label = QStringLiteral("%1/command-%2").arg(QString::fromLatin1(row.name)).arg(static_cast<int>(command));
      QTest::newRow(qUtf8Printable(label)) << static_cast<int>(row.partition) << static_cast<int>(command) << row.expected << static_cast<int>(row.restriction);
    }
}

void MainWindowViewModelTests::interactionPartitionsAgreeWithCommandRouting() {
  QFETCH(int, partitionValue);
  QFETCH(int, commandValue);
  QFETCH(QList<bool>, expected);
  QFETCH(int, restrictionValue);
  const auto partition = static_cast<Partition>(partitionValue);
  const auto command = static_cast<Command>(commandValue);
  WindowFixture fixture;
  if (partition == Partition::Unavailable) fixture.source.next.snapshot.uwfAvailable = false;
  if (partition == Partition::NotElevated) fixture.source.next.snapshot.elevated = false;
  if (partition == Partition::Horm) fixture.source.next.hormEnabled = true;
  if (partition != Partition::Unread) {
    fixture.window->refresh();
    QTRY_VERIFY(!fixture.window->refreshing());
    QVERIFY(fixture.window->state().hasCommittedState());
  }
  std::shared_ptr<ApplyPlanViewModel> applying;
  if (partition == Partition::Refreshing) {
    fixture.source.gate = std::make_shared<test::WorkerGate>();
    fixture.window->refresh();
    QVERIFY(fixture.window->refreshing());
  }
  if (partition == Partition::Applying) {
    applying = fixture.reviewEdit();
    QVERIFY(applying);
    applying->apply();
    QVERIFY(fixture.window->applyingConfiguration());
  }
  if (partition == Partition::Reconciliation) fixture.window->requireReconciliation();
  if (partition == Partition::RefreshFailed) {
    fixture.source.failReads = true;
    fixture.window->refresh();
    QTRY_VERIFY(!fixture.window->refreshing());
  }

  const auto interaction = fixture.window->interactionState();
  QCOMPARE((QList<bool>{interaction.refreshEnabled, interaction.configurationEnabled, interaction.powerEnabled, interaction.exitEnabled,
                        interaction.enhancedModeEnabled, interaction.volumePagesEnabled, interaction.accessKnown, interaction.elevated}),
           expected);
  QCOMPARE(fixture.window->configurationWritesAllowed(), expected[1]);
  const bool maintenanceReady = expected[1] || partition == Partition::Horm;
  QCOMPARE(fixture.window->maintenanceActionsAllowed(), maintenanceReady);
  QCOMPARE(fixture.window->maintenanceActionRestriction(), maintenanceReady ? QString{} : restrictionText(static_cast<Restriction>(restrictionValue)));
  QCOMPARE(fixture.window->configuration()->global()->controlsEnabled(), expected[1]);
  for (const auto& page : fixture.window->volumePages(QStringLiteral("C:"))) {
    const auto& volume = page.model;
    QCOMPARE(volume->status()->controlsEnabled(), expected[1]);
    QCOMPARE(volume->files()->readOnly(), !expected[1]);
    if (volume->registry()) QCOMPARE(volume->registry()->readOnly(), !expected[1]);
    const auto fileStaging = fixture.window->fileStagingModel(volume);
    QVERIFY(fileStaging);
    QCOMPARE(fileStaging->readOnly(), !expected[1]);
    const auto before = volume->status()->draft().protectedVolume;
    QCOMPARE(volume->status()->editProtection(!before), expected[1]);
    if (expected[1]) QVERIFY(volume->status()->editProtection(before));
  }
  QCOMPARE(interaction.configurationHint, restrictionText(static_cast<Restriction>(restrictionValue)));
  if (interaction.configurationEnabled) {
    QVERIFY(!interaction.importHint.isEmpty());
    QVERIFY(!interaction.reviewHint.isEmpty());
  } else {
    QCOMPARE(interaction.importHint, interaction.configurationHint);
    QCOMPARE(interaction.reviewHint, interaction.configurationHint);
    QVERIFY(!fixture.window->reviewConfiguration());
  }

  fixture.invoke(command);
  const bool directHormRestart = partition == Partition::Horm && command == Command::DirectRestart;
  QCOMPARE(fixture.powerRequests.size(), expected[2] || directHormRestart ? std::size_t{1} : std::size_t{0});
  if (expected[2] || (partition == Partition::Horm && command == Command::DirectRestart)) {
    const auto& request = fixture.powerRequests.front();
    QCOMPARE(request.action, command == Command::SafeShutdown ? app::PowerAction::Shutdown : app::PowerAction::Restart);
    QCOMPARE(request.mode, command == Command::DirectRestart ? app::PowerActionDialogMode::Direct : app::PowerActionDialogMode::Safe);
  }
  QCOMPARE(fixture.warnings, 0);
  QVERIFY(fixture.maintenance.backend->snapshot().powerRequests.empty());
  if (fixture.source.gate) {
    fixture.source.gate->release();
    QTRY_VERIFY(!fixture.window->refreshing());
  }
  if (applying) {
    fixture.models->actions->gate->release();
    QTRY_VERIFY(!fixture.window->applyingConfiguration());
  }
}

void MainWindowViewModelTests::configurationApplyBlocksPowerActionsUntilCompletion() {
  WindowFixture fixture;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  auto plan = fixture.reviewEdit();
  QVERIFY(plan);
  plan->apply();
  QTRY_COMPARE(fixture.models->actions->gate->started.load(), 1);
  QVERIFY(fixture.window->applyingConfiguration());
  fixture.power->safeShutdown();
  fixture.power->safeRestart();
  fixture.power->directRestart();
  QVERIFY(fixture.powerRequests.empty());
  fixture.models->actions->gate->release();
  QTRY_VERIFY(!fixture.window->applyingConfiguration());
  fixture.window->safeRestart();
  QCOMPARE(fixture.powerRequests.size(), std::size_t{1});
}

void MainWindowViewModelTests::sharedStagingPreparationBlocksWindowInteractionsUntilCompletion() {
  WindowFixture fixture;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  auto plan = fixture.reviewEdit();
  QVERIFY(plan && plan->canApply());
  const auto reads = fixture.source.reads;
  const auto reviews = fixture.models->reviews;
  QSignalSpy interactions(fixture.window.get(), &MainWindowViewModel::interactionAvailabilityChanged);
  const auto release = qScopeGuard([&] { fixture.draft->setBusy(false); });

  fixture.draft->setBusy(true);
  const auto blocked = fixture.window->interactionState();
  QVERIFY(!blocked.refreshEnabled);
  QVERIFY(!blocked.configurationEnabled);
  QVERIFY(!blocked.reviewEnabled);
  QVERIFY(!blocked.powerEnabled);
  QVERIFY(!blocked.exitEnabled);
  QVERIFY(!blocked.enhancedModeEnabled);
  QVERIFY(!blocked.volumePagesEnabled);
  QVERIFY(!fixture.window->configuration()->global()->controlsEnabled());
  QVERIFY(!fixture.window->maintenanceActionsAllowed());
  QVERIFY(!plan->canApply());
  QCOMPARE(blocked.configurationHint, restrictionText(Restriction::Wait));
  QVERIFY(!fixture.window->reviewConfiguration());
  QVERIFY(!fixture.window->importModel());
  QVERIFY(!fixture.window->enhancedModeModel());
  QVERIFY(!fixture.window->discardPendingChanges());
  fixture.window->refresh();
  QCOMPARE(fixture.source.reads, reads);
  QCOMPARE(fixture.models->reviews, reviews);
  QVERIFY(!fixture.window->refreshing());
  plan->apply();
  QCOMPARE(fixture.models->actions->prepared, 0);
  fixture.window->safeShutdown();
  fixture.window->safeRestart();
  fixture.window->directRestart();
  fixture.power->safeRestart();
  QVERIFY(fixture.powerRequests.empty());
  QCOMPARE(interactions.count(), 1);

  fixture.draft->setBusy(false);
  const auto ready = fixture.window->interactionState();
  QVERIFY(ready.refreshEnabled);
  QVERIFY(ready.configurationEnabled);
  QVERIFY(ready.reviewEnabled);
  QVERIFY(ready.powerEnabled);
  QVERIFY(ready.exitEnabled);
  QVERIFY(ready.volumePagesEnabled);
  QVERIFY(fixture.window->configuration()->global()->controlsEnabled());
  QVERIFY(fixture.window->maintenanceActionsAllowed());
  QVERIFY(plan->canApply());
  QCOMPARE(interactions.count(), 2);
  fixture.window->safeRestart();
  QCOMPARE(fixture.powerRequests.size(), std::size_t{1});
  QVERIFY(fixture.window->reviewConfiguration());
  fixture.window->refresh();
  QCOMPARE(fixture.source.reads, reads + 1);
  QTRY_VERIFY(!fixture.window->refreshing());
}

void MainWindowViewModelTests::reconciliationProjectsPublishedStateAndUserRefreshReads_data() {
  QTest::addColumn<bool>("uncertain");
  QTest::addColumn<bool>("publishedReadable");
  for (const bool uncertain : {false, true})
    for (const bool readable : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("uncertain-%1/published-readable-%2").arg(uncertain).arg(readable))) << uncertain << readable;
}

void MainWindowViewModelTests::reconciliationProjectsPublishedStateAndUserRefreshReads() {
  QFETCH(bool, uncertain);
  QFETCH(bool, publishedReadable);
  WindowFixture fixture;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  QCOMPARE(fixture.source.reads, 1);
  QCOMPARE(fixture.source.publishedReads, 0);
  auto plan = fixture.reviewEdit();
  QVERIFY(plan && plan->canApply());
  fixture.source.next.snapshot.next.filter.enabled = false;
  fixture.source.failPublished = !publishedReadable;
  app::ConfigurationApplyResult result;
  result.steps.push_back({"Filter", uncertain ? app::ConfigurationStepOutcome::Failed : app::ConfigurationStepOutcome::Succeeded,
                          uncertain ? app::WriteEffect::Uncertain : app::WriteEffect::Confirmed,
                          app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}});
  fixture.models->actions->applyResult = result;
  QSignalSpy failed(fixture.window.get(), &MainWindowViewModel::refreshFailed);
  plan->apply();
  fixture.models->actions->gate->release();
  QTRY_COMPARE(fixture.source.publishedReads, 1);
  QTRY_VERIFY(!fixture.window->applyingConfiguration() && !fixture.window->refreshing());
  QCOMPARE(fixture.source.reads, 1);
  QCOMPARE(fixture.window->state().snapshot().next.filter.enabled, !publishedReadable);
  QCOMPARE(fixture.window->state().reconciliationRequired(), !publishedReadable);
  QCOMPARE(failed.count(), publishedReadable ? 0 : 1);

  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  QCOMPARE(fixture.source.reads, 2);
  QCOMPARE(fixture.source.publishedReads, 1);
  QVERIFY(!fixture.window->state().snapshot().next.filter.enabled);
  QVERIFY(!fixture.window->state().reconciliationRequired());
}

void MainWindowViewModelTests::powerConfirmationBlocksWindowWritesAndExit() {
  WindowFixture fixture;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  fixture.duringConfirmation = [&] {
    QVERIFY(fixture.power->active());
    QVERIFY(!fixture.window->configurationWritesAllowed());
    QVERIFY(!fixture.window->exitAllowed());
    QVERIFY(!fixture.window->powerActionsAllowed());
    QVERIFY(!fixture.window->configuration()->global()->controlsEnabled());
    fixture.window->safeShutdown();
    fixture.window->directRestart();
    QCOMPARE(fixture.powerRequests.size(), std::size_t{1});
  };
  fixture.window->safeRestart();
  QVERIFY(!fixture.power->active());
  QVERIFY(fixture.window->configurationWritesAllowed());
  QVERIFY(fixture.window->exitAllowed());
}

void MainWindowViewModelTests::initializationObserversMayDestroyTheWindow() {
  QPointer<MainWindowViewModel> receiver;
  int notifications = 0;
  WindowFixture fixture(true, false, true, [&](WindowFixture& constructing) {
    receiver = constructing.window.get();
    connect(receiver, &MainWindowViewModel::enhancedModeStatusChanged, this, [&, owner = &constructing] {
      ++notifications;
      owner->window.reset();
    });
  });
  QCOMPARE(notifications, 1);
  QVERIFY(receiver.isNull());
  QVERIFY(!fixture.window);
  QVERIFY(fixture.power.isNull());
  QVERIFY(fixture.session.isNull());
  QVERIFY(!fixture.borrowedAgent.running());
}

void MainWindowViewModelTests::ownedWorkflowsReleaseBorrowedDependenciesInOrder_data() {
  QTest::addColumn<bool>("ownSession");
  QTest::addColumn<bool>("ownAgent");
  for (const bool session : {false, true})
    for (const bool agent : {false, true}) {
      const auto label = QStringLiteral("session-%1/agent-%2")
                             .arg(session ? QStringLiteral("owned") : QStringLiteral("borrowed"), agent ? QStringLiteral("owned") : QStringLiteral("borrowed"));
      QTest::newRow(qUtf8Printable(label)) << session << agent;
    }
}

void MainWindowViewModelTests::ownedWorkflowsReleaseBorrowedDependenciesInOrder() {
  QFETCH(bool, ownSession);
  QFETCH(bool, ownAgent);
  WindowFixture fixture(ownSession, ownAgent);
  QVERIFY(fixture.window->hasEnhancedMode());
  QVERIFY(fixture.agent->running());
  auto enhanced = fixture.window->enhancedModeModel();
  QVERIFY(enhanced);
  QCOMPARE(fixture.models->enhancedModels, 1);
  enhanced.reset();
  bool powerReleasedBeforeDependencies = false;
  connect(fixture.power, &QObject::destroyed, this, [&] { powerReleasedBeforeDependencies = fixture.session && fixture.agent->running(); });
  fixture.window.reset();
  QVERIFY(fixture.power.isNull());
  QVERIFY(powerReleasedBeforeDependencies);
  QCOMPARE(fixture.session.isNull(), ownSession);
  if (!ownSession) {
    QVERIFY(fixture.agent->running());
    fixture.borrowedSession.reset();
  }
  if (ownAgent) {
    QVERIFY(fixture.agent.isNull());
  } else {
    QCOMPARE(fixture.agent.data(), static_cast<app::EnhancedModeAgentConnection*>(&fixture.borrowedAgent));
    QVERIFY(!fixture.borrowedAgent.running());
    QCOMPARE(fixture.borrowedAgent.stops, 1);
  }
}

void MainWindowViewModelTests::absentEnhancedModeHasNoModelOrAffordance() {
  WindowFixture fixture(true, false, false);
  QVERIFY(!fixture.window->hasEnhancedMode());
  QVERIFY(!fixture.window->enhancedModeModel());
  QVERIFY(!fixture.window->interactionState().enhancedModeEnabled);
  QCOMPARE(fixture.models->enhancedModels, 0);
}

void MainWindowViewModelTests::shellVolumeProjection_data() {
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
  QTest::newRow("missing-system-keeps-registry-hidden") << std::vector{unsupported, limited} << QStringLiteral("X:") << -1;
  QTest::newRow("unsupported-system-still-hosts-global-registry") << std::vector{unsupported, d} << QStringLiteral("C:") << 0;
  QTest::newRow("no-system-keeps-registry-hidden") << std::vector{unsupported} << QStringLiteral("X:") << -1;
}

void MainWindowViewModelTests::shellVolumeProjection() {
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
      QCOMPARE(page.title, QString::fromStdString(volumes[row].driveLetter));
    }
  }
}

void MainWindowViewModelTests::protectedUnletteredPartitionsFollowLetteredTabsAndKeepIndependentStates() {
  constexpr auto currentId = "Volume{AAAAAAAA-1111-2222-3333-444444444444}";
  constexpr auto nextId = "Volume{BBBBBBBB-1111-2222-3333-444444444444}";
  FakeStateSource source;
  source.next = availableState();
  source.next.volumes.insert(source.next.volumes.begin(),
                             core::VolumeInfo{.driveLetter = {}, .volumeName = nextId, .fileSystem = {}, .label = "System reserved"});
  source.next.volumes.push_back({.driveLetter = {}, .volumeName = "Volume{CCCCCCCC-1111-2222-3333-444444444444}", .fileSystem = {}, .label = {}});
  source.next.snapshot.current.volumes.push_back({currentId, "", true, core::VolumeBinding::VolumeId});
  source.next.snapshot.next.volumes.push_back({currentId, "", false, core::VolumeBinding::VolumeId});
  source.next.snapshot.next.volumes.push_back({nextId, "", true, core::VolumeBinding::VolumeId});
  presentation::MainWindowViewModel model(source, app::UwfCapability::Available);
  model.initializeConfiguration(8192, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  auto pages = model.volumePages(QStringLiteral("C:"));
  QCOMPARE(pages.size(), std::size_t{3});
  QCOMPARE(pages[0].volume.driveLetter, std::string("C:"));
  QVERIFY(pages[1].volume.driveLetter.empty());
  QVERIFY(pages[2].volume.driveLetter.empty());
  QCOMPARE(pages[1].title, QStringLiteral("Partition"));
  QCOMPARE(pages[2].title, QStringLiteral("Partition"));
  QCOMPARE(pages[2].volume.label, std::string("System reserved"));
  QVERIFY(pages[1].identity != pages[2].identity);
  QVERIFY(pages[1].model != pages[2].model);
  for (const auto index : {std::size_t{1}, std::size_t{2}}) {
    QVERIFY(!pages[index].title.contains(QStringLiteral("Volume{"), Qt::CaseInsensitive));
    QVERIFY(pages[index].title.size() <= 40);
    QVERIFY(pages[index].tooltip.contains(QString::fromStdString(pages[index].volume.volumeName)));
    QVERIFY(!pages[index].hasStaging);
    QVERIFY(!pages[index].hostsRegistry);
    QVERIFY(!pages[index].system);
    QVERIFY(pages[index].model->snapshotState().editable);
    QVERIFY(pages[index].model->status()->controlsEnabled());
    QVERIFY(!pages[index].model->status()->bindingEnabled());
  }
  QVERIFY(pages[1].model->status()->currentProtected());
  QVERIFY(!pages[1].model->status()->draft().protectedVolume);
  QVERIFY(!pages[2].model->snapshotState().current);
  QVERIFY(pages[2].model->status()->draft().protectedVolume);
  QVERIFY(model.configuration()->pendingChanges().empty());
  // Cancelling Next protection leaves Current protection visible until restart.
  source.next.snapshot.next.volumes.back().isProtected = false;
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  pages = model.volumePages(QStringLiteral("C:"));
  QCOMPARE(pages.size(), std::size_t{2});
  QCOMPARE(pages[1].volume.volumeName, std::string(currentId));
  QVERIFY(pages[1].model->status()->currentProtected());
  QVERIFY(!pages[1].model->status()->draft().protectedVolume);
}

void MainWindowViewModelTests::savedStagingTargetsKeepExactlyOneManagementPageAcrossMountChanges_data() {
  QTest::addColumn<bool>("offline");
  QTest::newRow("saved-target-goes-offline") << true;
  QTest::newRow("saved-target-loses-file-operation-support") << false;
}

void MainWindowViewModelTests::savedStagingTargetsKeepExactlyOneManagementPageAcrossMountChanges() {
  QFETCH(bool, offline);
  class MountedPaths final : public app::CommitTraversalSource {
   public:
    bool offline = false;
    app::FileCommitTarget resolveFileTarget(const QString& path) const override {
      const auto drive = path.left(2).toUpper();
      if (drive != QStringLiteral("C:") && drive != QStringLiteral("D:")) throw std::runtime_error("Unknown test drive");
      if (offline && drive == QStringLiteral("D:")) throw std::runtime_error("Saved volume is offline");
      return memoryFileTarget(path, drive == QStringLiteral("D:") ? otherFixtureVolumeId : fixtureVolumeId);
    }
    std::string driveLetterForPath(const QString& path) const override {
      (void)resolveFileTarget(path);
      return path.left(2).toUpper().toStdString();
    }
    std::optional<app::CommitNode> inspect(const app::CommitNode&) const override { throw std::logic_error("Unexpected metadata read"); }
    bool exists(const app::CommitNode&) const override { throw std::logic_error("Unexpected existence read"); }
    QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override {
      throw std::logic_error("Unexpected traversal");
    }
  };
  const app::FileStagingEntry saved{app::FileStagingKind::File, QStringLiteral("D:\\Data\\saved.bin")};
  ControlledStateSource state;
  state.next.volumes.push_back(disk("D:"));
  state.next.snapshot.current.volumes.push_back(
      {uwf::simulation::application::fixtureVolumeDName, "D:", false, core::VolumeBinding::VolumeId, std::string("D:")});
  state.next.snapshot.next.volumes.push_back(state.next.snapshot.current.volumes.back());
  const auto mountedState = state.next;
  UwfMaintenanceFixture maintenance;
  auto store = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
  store->entries = {saved};
  auto source = std::make_shared<MountedPaths>();
  auto draft = std::make_shared<app::FileStagingDraft>(store, source);
  auto models = std::make_shared<WindowModels>();
  models->stagingSource = source;
  MainWindowViewModel model(state, app::UwfCapability::Available, draft);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  model.initializeWorkflows(models, {},
                            std::make_unique<app::PowerCoordinator>(app::PowerServices{maintenance.repository(), *store, app::UwfCapability::Available, {}, {}},
                                                                    app::PowerInteraction{}));
  const auto managementPages = [&] {
    std::vector<std::pair<QString, std::shared_ptr<FileStagingViewModel>>> result;
    for (const auto& page : model.volumePages(QStringLiteral("C:")))
      if (page.hasStaging) result.emplace_back(QString::fromStdString(page.volume.driveLetter), model.fileStagingModel(page.model));
    return result;
  };
  const auto rowCount = [](const auto& pages) {
    int count = 0;
    for (const auto& [drive, staging] : pages) {
      (void)drive;
      count += staging->rowCount();
    }
    return count;
  };
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  auto pages = managementPages();
  QCOMPARE(pages.size(), std::size_t{2});
  for (const auto& [drive, staging] : pages) {
    QVERIFY(staging);
    staging->load();
    QTRY_VERIFY(!staging->busy());
    QCOMPARE(staging->entries().size(), drive == QStringLiteral("D:") ? qsizetype{1} : qsizetype{0});
  }
  QCOMPARE(rowCount(pages), 1);
  pages.clear();

  source->offline = offline;
  if (offline) {
    state.next.volumes.pop_back();
    state.next.snapshot.current.volumes.back().driveLetter.clear();
    state.next.snapshot.next.volumes.back().driveLetter.clear();
    state.next.snapshot.current.volumes.back().physicalDriveLetter = std::string{};
    state.next.snapshot.next.volumes.back().physicalDriveLetter = std::string{};
  } else {
    state.next.volumes.back().support = core::VolumeSupport::FileSystemLimited;
  }
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  pages = managementPages();
  QCOMPARE(pages.size(), std::size_t{1});
  auto fallback = pages.front().second;
  QVERIFY(fallback);
  QCOMPARE(pages.front().first, QStringLiteral("C:"));
  fallback->load();
  QTRY_VERIFY(!fallback->busy());
  QCOMPARE(fallback->entries(), (QList<app::FileStagingEntry>{saved}));
  QCOMPARE(rowCount(pages), 1);
  app::AutomaticCommitSummary expectedAutomatic;
  expectedAutomatic.include(offline ? app::AutomaticCommitDisposition::Unavailable : app::AutomaticCommitDisposition::AlreadyPersistent);
  // D is unprotected while its management host C is protected. The host cannot
  // supply the file's automatic-commit policy or its volume identity.
  QCOMPARE(fallback->index(0).data(StatusTextRole).toString(), automaticCommitPresentation(expectedAutomatic).status);
  QCOMPARE(fallback->automaticCommitPresentation().status, automaticCommitPresentation(expectedAutomatic).status);
  if (!offline) {
    QVERIFY(fallback->entries().constFirst().fileTarget);
    QCOMPARE(fallback->entries().constFirst().fileTarget->volumeId, std::string(otherFixtureVolumeId));
  }
  fallback->removePaths({saved.path});
  QTRY_VERIFY(!fallback->busy());
  QVERIFY(fallback->entries().isEmpty());
  QVERIFY(draft->changes());
  QSignalSpy rejected(fallback.get(), &FileStagingViewModel::warningRaised);
  fallback->restorePaths({saved.path});
  QTRY_VERIFY(!fallback->busy());
  QCOMPARE(rejected.count(), 1);
  QVERIFY(fallback->entries().isEmpty());
  QVERIFY(draft->changes()->after.isEmpty());
  QCOMPARE(store->entries, (QList<app::FileStagingEntry>{saved}));
  QCOMPARE(store->writes, 0);
  model.refreshDiscardingChanges();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(fallback->entries(), (QList<app::FileStagingEntry>{saved}));
  QVERIFY(!draft->changes());
  pages.clear();
  fallback.reset();

  source->offline = false;
  state.next = mountedState;
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  pages = managementPages();
  QCOMPARE(pages.size(), std::size_t{2});
  for (const auto& [drive, staging] : pages) {
    QVERIFY(staging);
    staging->load();
    QTRY_VERIFY(!staging->busy());
    QCOMPARE(staging->entries().size(), drive == QStringLiteral("D:") ? qsizetype{1} : qsizetype{0});
    if (drive == QStringLiteral("D:")) {
      staging->removePaths({saved.path});
      QTRY_VERIFY(!staging->busy());
      QVERIFY(staging->entries().isEmpty());
      staging->restorePaths({saved.path});
      QTRY_VERIFY(!staging->busy());
      QCOMPARE(staging->entries(), (QList<app::FileStagingEntry>{saved}));
    }
  }
  QCOMPARE(rowCount(pages), 1);
  QVERIFY(!draft->changes());
  QCOMPARE(store->entries, (QList<app::FileStagingEntry>{saved}));
  QCOMPARE(store->writes, 0);
}

void MainWindowViewModelTests::runtimeModeRestrictionsAreSharedWithoutBlockingConfigurationForRom_data() {
  QTest::addColumn<int>("mode");
  QTest::addColumn<bool>("rom");
  QTest::newRow("ready") << 0 << false;
  QTest::newRow("rom") << 0 << true;
  QTest::newRow("horm-unknown") << 1 << false;
  QTest::newRow("horm-active") << 2 << true;
  QTest::newRow("restart-required") << 3 << true;
}
void MainWindowViewModelTests::runtimeModeRestrictionsAreSharedWithoutBlockingConfigurationForRom() {
  QFETCH(int, mode);
  QFETCH(bool, rom);
  auto actions = std::make_shared<WindowMaintenanceActions>();
  actions->current.hormEnabled = mode == 1 ? std::nullopt : std::optional{mode >= 2};
  actions->current.currentFlags = app::OverlayFlags{rom ? app::OverlayFlags::ReadOnlyMedia : 0};
  actions->current.nextFlags = actions->current.currentFlags;
  std::shared_ptr<OverlayMaintenanceViewModel> maintenance;
  WindowFixture fixture(true, false, false, [&](WindowFixture& target) {
    target.source.next.hormEnabled = actions->current.hormEnabled;
    target.source.next.snapshot.runtime.hormEnabled = actions->current.hormEnabled;
    target.source.next.snapshot.current.filter.enabled = true;
    target.source.next.snapshot.runtime.romDisposition = 0;
    target.source.next.snapshot.runtime.fullCommit = false;
    target.source.next.snapshot.runtime.currentOverlayFlags = rom ? app::OverlayFlags::ReadOnlyMedia : 0u;
    target.source.next.overlayPolicy.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{rom ? app::OverlayFlags::ReadOnlyMedia : 0}};
    target.models->maintenanceFactory = [&maintenance, actions](std::shared_ptr<app::OverlayMaintenanceEditor> editor) {
      maintenance = std::make_shared<OverlayMaintenanceViewModel>(actions, std::move(editor));
      return maintenance;
    };
  });
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing() && maintenance->hasSnapshot() && !maintenance->busy());
  if (mode == 3) {
    actions->current.hormEnabled = false;
    actions->current.romDisposition = 3;
    maintenance->reconcileAppliedDraft({.hormEnabled = false}, actions->current);
    fixture.source.next.hormEnabled = false;
    fixture.source.next.snapshot.runtime.hormEnabled = false;
    fixture.source.next.snapshot.runtime.romDisposition = 3;
    fixture.window->refresh();
    QTRY_VERIFY(!fixture.window->refreshing() && !maintenance->busy());
    QCOMPARE(fixture.window->state().hormEnabled(), std::optional{false});
    QVERIFY(maintenance->restartRequired());
  }
  const auto common = mode == 1   ? app::RuntimeWriteRestriction::HormUnknown
                      : mode == 2 ? app::RuntimeWriteRestriction::HormActive
                                  : app::RuntimeWriteRestriction::None;
  const bool writable = mode != 1 && mode != 2;
  QCOMPARE(fixture.window->configurationWritesAllowed(), writable);
  QVERIFY(fixture.window->maintenanceActionsAllowed());
  QCOMPARE(fixture.window->configurationWriteRestriction(), runtimeWriteRestrictionText(common));
  QVERIFY(fixture.window->maintenanceActionRestriction().isEmpty());
  QCOMPARE(fixture.window->stagingEditingRestriction(), mode == 0 && rom ? app::RuntimeWriteRestriction::ReadOnlyMediaActive : common);
  QVERIFY(fixture.window->powerActionsAllowed());
  if (mode == 3) {
    const auto volume = fixture.window->configuration()->volume(QStringLiteral("C:"));
    QVERIFY(volume && volume->commitAvailability().files);
    QVERIFY(volume->commitAvailability().registry);
  }
}

void MainWindowViewModelTests::knownHormControlsChangesIndependentlyOfOverlayMetadata_data() {
  QTest::addColumn<bool>("hormKnown");
  QTest::addColumn<bool>("hormEnabled");
  QTest::addColumn<bool>("readOnlyMedia");
  QTest::addColumn<bool>("flagsKnown");
  for (const auto& [name, horm] :
       {std::pair{"unknown", std::optional<bool>{}}, std::pair{"off", std::optional<bool>{false}}, std::pair{"on", std::optional<bool>{true}}})
    for (const bool rom : {false, true}) {
      if (horm == true && !rom) continue;  // HORM requires the active RAM/ROM configuration.
      for (const bool flagsKnown : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("horm-%1/rom-%2/flags-known-%3").arg(QString::fromLatin1(name)).arg(rom).arg(flagsKnown)))
            << horm.has_value() << horm.value_or(false) << rom << flagsKnown;
    }
}

void MainWindowViewModelTests::knownHormControlsChangesIndependentlyOfOverlayMetadata() {
  QFETCH(bool, hormKnown);
  QFETCH(bool, hormEnabled);
  QFETCH(bool, readOnlyMedia);
  QFETCH(bool, flagsKnown);
  FakeStateSource source;
  source.next.volumes = {disk("C:"), disk("D:")};
  source.next.snapshot = snapshot();
  source.next.snapshot.current.filter.enabled = true;
  source.next.snapshot.current.volumes[1].isProtected = true;
  source.next.hormEnabled = hormKnown ? std::optional{hormEnabled} : std::nullopt;
  source.next.snapshot.runtime.hormEnabled = source.next.hormEnabled;
  source.next.snapshot.runtime.systemVolumeProtected = true;
  source.next.snapshot.runtime.currentOverlayFlags =
      flagsKnown ? std::optional<std::uint32_t>{readOnlyMedia ? app::OverlayFlags::ReadOnlyMedia : 0u} : std::nullopt;
  auto& policy = source.next.overlayPolicy;
  policy.transportAvailable = true;
  policy.current.filterEnabled = policy.next.filterEnabled = {app::PolicyObservationStatus::Available, true};
  policy.current.overlayType = policy.next.overlayType = {app::PolicyObservationStatus::Available, 0};
  policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{readOnlyMedia ? app::OverlayFlags::ReadOnlyMedia : 0u}};
  policy.current.flags = flagsKnown ? policy.next.flags : app::PolicyObservation<app::OverlayFlags>{app::PolicyObservationStatus::Failed};
  const bool allowed = !hormKnown || !hormEnabled;
  const bool commitsAllowed = hormKnown && !hormEnabled && flagsKnown && !readOnlyMedia;
  const bool ordinaryWritesAllowed = commitsAllowed;
  presentation::MainWindowViewModel model(source, app::UwfCapability::Available);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(model.state().hasCommittedState());
  QCOMPARE(model.state().hormEnabled(), source.next.hormEnabled);
  QCOMPARE(model.configurationWritesAllowed(), allowed);
  QCOMPARE(model.configuration()->global()->controlsEnabled(), allowed);
  QVERIFY(model.maintenanceActionsAllowed());
  QVERIFY(model.maintenanceActionRestriction().isEmpty());
  for (const auto& letter : {QStringLiteral("C:"), QStringLiteral("D:")}) {
    const auto volume = model.configuration()->volume(letter);
    QVERIFY(volume);
    QCOMPARE(volume->commitAvailability().files, commitsAllowed);
    QCOMPARE(volume->status()->controlsEnabled(), ordinaryWritesAllowed);
    QCOMPARE(volume->files()->readOnly(), !ordinaryWritesAllowed);
    QVERIFY(volume->commitAvailability().overlay);  // Diagnostics remain readable.
  }
  const auto system = model.configuration()->volume(QStringLiteral("C:"));
  QCOMPARE(system->commitAvailability().registry, commitsAllowed);
  QCOMPARE(system->registry()->readOnly(), !ordinaryWritesAllowed);
  const auto edit = model.configuration()->overlayPolicy()->beginEdit();
  if (!allowed) {
    const auto settingReason = QStringLiteral("UWF settings cannot be changed while HORM is active.");
    const auto commitReason = QStringLiteral("File and registry commits are unavailable while HORM is active.");
    QCOMPARE(model.configurationWriteRestriction(), settingReason);
    QCOMPARE(system->commitAvailability().commitHint, commitReason);
    QCOMPARE(model.configuration()->overlayPolicy()->disabledReason(edit, presentation::OverlayAdvancedViewModel::Field::ReadOnlyMedia), settingReason);
    QVERIFY(!model.configuration()->overlayPolicy()->canCommit(edit));
  } else {
    QVERIFY(model.configurationWriteRestriction().isEmpty());
    if (!commitsAllowed && hormKnown && flagsKnown) {
      const auto commitReason = QStringLiteral("File and registry commits are unavailable while Read-Only Media mode is active.");
      QCOMPARE(system->commitAvailability().commitHint, commitReason);
    } else if (!commitsAllowed) {
      QVERIFY(!system->commitAvailability().commitHint.isEmpty());
    }
  }
  QVERIFY(model.configuration()->pendingConfiguration().empty());
  const auto restriction = hormKnown && hormEnabled    ? app::RuntimeWriteRestriction::HormActive
                           : !hormKnown || !flagsKnown ? app::RuntimeWriteRestriction::StateUnavailable
                           : readOnlyMedia             ? app::RuntimeWriteRestriction::ReadOnlyMediaActive
                                                       : app::RuntimeWriteRestriction::None;
  QCOMPARE(model.stagingEditingRestriction(), restriction);
  QVERIFY(model.powerActionsAllowed());
  EmptyTranslator translator;
  QVERIFY(QCoreApplication::installTranslator(&translator));
  const auto removeTranslator = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });
  QVERIFY(presentation::runtimeWriteRestrictionText(restriction).isEmpty());
  QVERIFY(model.powerActionsAllowed());
}

void MainWindowViewModelTests::registryCommitAvailabilityFollowsTheSystemVolume_data() {
  QTest::addColumn<bool>("filterEnabled");
  QTest::addColumn<bool>("systemProtected");
  QTest::addColumn<bool>("dataProtected");
  QTest::addColumn<bool>("readOnlyMedia");
  QTest::addColumn<bool>("nextReadOnlyMedia");
  QTest::addColumn<bool>("hormEnabled");
  for (const bool filter : {false, true})
    for (const bool system : {false, true})
      for (const bool data : {false, true})
        for (const bool rom : {false, true})
          for (const bool nextRom : {false, true})
            for (const bool horm : {false, true}) {
              const auto name = QStringLiteral("filter-%1/system-%2/data-%3/current-rom-%4/next-rom-%5/horm-%6")
                                    .arg(filter)
                                    .arg(system)
                                    .arg(data)
                                    .arg(rom)
                                    .arg(nextRom)
                                    .arg(horm);
              QTest::newRow(qPrintable(name)) << filter << system << data << rom << nextRom << horm;
            }
}

void MainWindowViewModelTests::registryCommitAvailabilityFollowsTheSystemVolume() {
  QFETCH(bool, filterEnabled);
  QFETCH(bool, systemProtected);
  QFETCH(bool, dataProtected);
  QFETCH(bool, readOnlyMedia);
  QFETCH(bool, nextReadOnlyMedia);
  QFETCH(bool, hormEnabled);
  FakeStateSource source;
  source.next.volumes = {disk("C:"), disk("D:")};
  source.next.snapshot = snapshot();
  source.next.snapshot.current.filter.enabled = filterEnabled;
  source.next.snapshot.current.volumes[0].isProtected = systemProtected;
  source.next.snapshot.current.volumes[1].isProtected = dataProtected;
  source.next.hormEnabled = hormEnabled;
  source.next.snapshot.runtime.hormEnabled = hormEnabled;
  source.next.snapshot.runtime.systemVolumeProtected = systemProtected;
  source.next.snapshot.runtime.currentOverlayFlags = readOnlyMedia ? app::OverlayFlags::ReadOnlyMedia : 0u;
  // Future protection must not invent a registry overlay in this session.
  source.next.snapshot.next.volumes[0].isProtected = !systemProtected;
  source.next.overlayPolicy.current.filterEnabled.value = filterEnabled;
  source.next.overlayPolicy.current.flags.status = app::PolicyObservationStatus::Available;
  source.next.overlayPolicy.current.flags.value = app::OverlayFlags{readOnlyMedia ? app::OverlayFlags::ReadOnlyMedia : 0u};
  source.next.overlayPolicy.next.flags = {app::PolicyObservationStatus::Available,
                                          app::OverlayFlags{nextReadOnlyMedia ? app::OverlayFlags::ReadOnlyMedia : 0u}};
  if (readOnlyMedia || nextReadOnlyMedia) {
    source.next.snapshot.current.overlay.type = source.next.snapshot.next.overlay.type = core::OverlayType::RAM;
    source.next.overlayPolicy.current.overlayType = source.next.overlayPolicy.next.overlayType = {app::PolicyObservationStatus::Available, 0};
  }
  presentation::MainWindowViewModel model(source, app::UwfCapability::Available);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(model.state().hasCommittedState());
  const auto system = model.configuration()->volume(QStringLiteral("C:"));
  const auto data = model.configuration()->volume(QStringLiteral("D:"));
  QVERIFY(system && data);
  const bool registryAvailable = filterEnabled && systemProtected && !readOnlyMedia && !hormEnabled;
  QCOMPARE(system->commitAvailability().registry, registryAvailable);
  QVERIFY(!data->commitAvailability().registry);
  QCOMPARE(system->commitAvailability().files, registryAvailable);
  QCOMPARE(data->commitAvailability().files, filterEnabled && dataProtected && !readOnlyMedia && !hormEnabled);
  QVERIFY(model.maintenanceActionsAllowed());
  QVERIFY(model.maintenanceActionRestriction().isEmpty());
  QCOMPARE(model.configurationWritesAllowed(), !hormEnabled);
  // Window readiness authorizes opening the operation. The real repository still
  // admits each exact Current target, including the already-persistent case.
  for (const auto& [target, protectedTarget] : {std::pair{system, systemProtected}, std::pair{data, dataProtected}}) {
    UwfMaintenanceFixture maintenance;
    maintenance.state->volumeId = target->volume().volumeName;
    maintenance.state->systemVolumeId = maintenance.state->volumeId;
    maintenance.state->drive = target->driveLetter();
    maintenance.state->protectedVolume = protectedTarget;
    maintenance.state->filterEnabled = filterEnabled;
    const auto repository = maintenance.repository();
    auto live = maintenance.backend->snapshot();
    live.current.overlayFlags = readOnlyMedia ? app::OverlayFlags::ReadOnlyMedia : 0u;
    live.next.overlayFlags = nextReadOnlyMedia ? app::OverlayFlags::ReadOnlyMedia : 0u;
    live.runtime.hormEnabled = hormEnabled;
    maintenance.backend->seed(std::move(live));
    if (filterEnabled && protectedTarget && (readOnlyMedia || hormEnabled)) {
      QVERIFY_THROWS_EXCEPTION(app::OperationError, (void)repository->fileCommitRequirement(maintenance.state->volumeId));
      if (target == system) QVERIFY_THROWS_EXCEPTION(app::OperationError, (void)repository->registryCommitRequirement());
    } else {
      QCOMPARE(repository->fileCommitRequirement(maintenance.state->volumeId),
               filterEnabled && protectedTarget ? app::CommitRequirement::RequiresUwfCommit : app::CommitRequirement::AlreadyPersistent);
      if (target == system)
        QCOMPARE(repository->registryCommitRequirement(),
                 filterEnabled && protectedTarget ? app::CommitRequirement::RequiresUwfCommit : app::CommitRequirement::AlreadyPersistent);
    }
  }
  if (filterEnabled && !systemProtected) {
    QCOMPARE(system->commitAvailability().commitHint,
             QStringLiteral("The system volume is not currently protected by UWF; there are no registry changes to commit."));
    QVERIFY(!system->commitAvailability().commitHint.contains(QStringLiteral("Only registry commit")));
  }
  model.setExternalOperationActive(true);
  QVERIFY(!model.maintenanceActionsAllowed());
  QCOMPARE(model.maintenanceActionRestriction(), restrictionText(Restriction::Wait));
  model.setExternalOperationActive(false);
  QVERIFY(model.maintenanceActionsAllowed());
  QCOMPARE(system->commitAvailability().registry, registryAvailable);
}

void MainWindowViewModelTests::windowRefreshObserversMayDestroyPresentation() {
  FakeStateSource source;
  source.next = {{disk()}, snapshot()};
  auto model = std::make_unique<presentation::MainWindowViewModel>(source, app::UwfCapability::Available);
  connect(model.get(), &presentation::MainWindowViewModel::stateCommitted, this, [&] { model.reset(); });
  model->refresh();
  QTRY_VERIFY(!model);
  QVERIFY(!model);
}

void MainWindowViewModelTests::stagingRefreshKeepsObservationsOffTheOwnerThread_data() {
  QTest::addColumn<int>("mode");
  QTest::newRow("initial-read") << 0;
  QTest::newRow("ordinary-reload") << 1;
  QTest::newRow("unknown-write-reconciliation") << 2;
}

void MainWindowViewModelTests::stagingRefreshKeepsObservationsOffTheOwnerThread() {
  QFETCH(int, mode);
  const app::FileStagingEntry original{app::FileStagingKind::File, QStringLiteral("C:\\original.bin")};
  const app::FileStagingEntry intended{app::FileStagingKind::File, QStringLiteral("C:\\intended.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {original};
  auto source = std::make_shared<WindowStagingSource>();
  auto draft = std::make_shared<app::FileStagingDraft>(store, source);
  if (mode != 0) static_cast<void>(draft->load());
  if (mode == 1) store->entries = {intended};
  if (mode == 2) {
    draft->replace({intended});
    store->failReadAfterReplace = true;
    QVERIFY_THROWS_EXCEPTION(app::OperationError, draft->apply(*draft->changes()));
    QVERIFY(draft->reconciliationRequired());
    store->failRead = false;
  }
  source->ownerObservations = source->workerObservations = 0;
  source->blockNext = true;
  const auto release = qScopeGuard([gate = source->gate] { gate->release(); });
  ControlledStateSource state;
  MainWindowViewModel model(state, app::UwfCapability::Available, draft);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  if (mode == 2) {
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, model.discardPendingChanges());
    QCOMPARE(draft->changes()->after, QList<app::FileStagingEntry>{intended});
  }
  QSignalSpy committed(&model, &MainWindowViewModel::stateCommitted);
  model.refresh();
  QVERIFY(model.refreshing());
  QVERIFY(draft->busy());
  QVERIFY(!model.configurationWritesAllowed());
  QTRY_COMPARE(source->gate->started.load(), 1);
  bool heartbeat = false;
  QTimer::singleShot(0, &model, [&heartbeat] { heartbeat = true; });
  QTRY_VERIFY(heartbeat);
  QCOMPARE(committed.count(), 0);
  QCOMPARE(source->gate->finished.load(), 0);
  QCOMPARE(source->ownerObservations.load(), 0);
  source->gate->release();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(!draft->busy());
  QCOMPARE(committed.count(), 1);
  QCOMPARE(source->ownerObservations.load(), 0);
  QVERIFY(source->workerObservations.load() > 0);
  QVERIFY(model.configurationWritesAllowed());
  QVERIFY(!draft->reconciliationRequired());
  QCOMPARE(draft->cachedSnapshot()->entries, QList<app::FileStagingEntry>{mode == 0 ? original : intended});
  QCOMPARE(store->writes, mode == 2 ? 1 : 0);
  if (mode == 2) QVERIFY(draft->changes()->persistencePending);
}

void MainWindowViewModelTests::destroyingARefreshingWindowReleasesTheSharedDraft() {
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {{app::FileStagingKind::File, QStringLiteral("C:\\original.bin")}};
  auto source = std::make_shared<WindowStagingSource>();
  source->blockNext = true;
  const auto release = qScopeGuard([gate = source->gate] { gate->release(); });
  auto draft = std::make_shared<app::FileStagingDraft>(store, source);
  ControlledStateSource state;
  auto model = std::make_unique<MainWindowViewModel>(state, app::UwfCapability::Available, draft);
  model->refresh();
  QTRY_COMPARE(source->gate->started.load(), 1);
  QVERIFY(draft->busy());
  model.reset();
  QVERIFY(!draft->busy());
  source->gate->release();
  QTRY_COMPARE(source->gate->finished.load(), 1);
  QVERIFY(!draft->cachedSnapshot());
  QCOMPARE(store->writes, 0);
}

void MainWindowViewModelTests::aNewRefreshSupersedesTheOwnedStagingRead() {
  const app::FileStagingEntry original{app::FileStagingKind::File, QStringLiteral("C:\\original.bin")};
  const app::FileStagingEntry latest{app::FileStagingKind::File, QStringLiteral("C:\\latest.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {original};
  auto source = std::make_shared<WindowStagingSource>();
  source->blockNext = true;
  const auto release = qScopeGuard([gate = source->gate] { gate->release(); });
  auto draft = std::make_shared<app::FileStagingDraft>(store, source);
  ControlledStateSource state;
  MainWindowViewModel model(state, app::UwfCapability::Available, draft);
  QSignalSpy committed(&model, &MainWindowViewModel::stateCommitted);
  QSignalSpy busy(draft.get(), &app::FileStagingDraft::busyChanged);
  model.refresh();
  QTRY_COMPARE(source->gate->started.load(), 1);
  store->entries = {latest};
  model.refresh();
  QVERIFY(draft->busy());
  QCOMPARE(busy.count(), 1);
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(!draft->busy());
  QCOMPARE(busy.count(), 2);
  QCOMPARE(committed.count(), 1);
  QCOMPARE(draft->cachedSnapshot()->entries, QList<app::FileStagingEntry>{latest});
  source->gate->release();
  QTRY_COMPARE(source->gate->finished.load(), 1);
  QCoreApplication::processEvents();
  QCOMPARE(committed.count(), 1);
  QCOMPARE(draft->cachedSnapshot()->entries, QList<app::FileStagingEntry>{latest});
  QCOMPARE(store->writes, 0);
}

void MainWindowViewModelTests::explicitRefreshPublishesIndependentStateWhenStagingReadFails_data() {
  QTest::addColumn<bool>("stagingLoaded");
  QTest::addColumn<bool>("initialStateFailed");
  QTest::addColumn<bool>("configurationPending");
  for (const bool loaded : {false, true})
    for (const bool stateFailed : {false, true})
      for (const bool pending : {false, true}) {
        if (stateFailed && pending) continue;  // An unavailable initial workspace cannot be edited.
        QTest::newRow(qUtf8Printable(QStringLiteral("staging-loaded-%1/state-failed-%2/configuration-pending-%3").arg(loaded).arg(stateFailed).arg(pending)))
            << loaded << stateFailed << pending;
      }
}

void MainWindowViewModelTests::explicitRefreshPublishesIndependentStateWhenStagingReadFails() {
  QFETCH(bool, stagingLoaded);
  QFETCH(bool, initialStateFailed);
  QFETCH(bool, configurationPending);
  const QString readFailure = QStringLiteral("Saved staging list is unreadable");
  const app::FileStagingEntry saved{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")};
  auto store = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
  store->entries = {saved};
  if (!stagingLoaded) store->readFailure = readFailure;
  auto draft = std::make_shared<app::FileStagingDraft>(store);
  ControlledStateSource source;
  source.failReads = initialStateFailed;
  MainWindowViewModel model(source, app::UwfCapability::Available, draft);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(model.state().hasCommittedState(), !initialStateFailed);
  QCOMPARE(model.workspaceDiagnostic().has_value(), initialStateFailed);
  QCOMPARE(draft->editSnapshot().has_value(), stagingLoaded);
  QVERIFY(!draft->changes());
  QVERIFY(!draft->reconciliationRequired());
  if (configurationPending) {
    model.configuration()->global()->editWarningThreshold(3500);
    QCOMPARE(model.configuration()->global()->pendingOverlay().warningThresholdMb, std::optional<uint32_t>{3500});
    QVERIFY(model.pendingChangeCount() > 0);
  } else {
    QCOMPARE(model.pendingChangeCount(), std::size_t{0});
  }

  // Model an independent Next-session change while the saved staging list
  // remains unreadable. Refresh has no staging edits or uncertain write to discard.
  source.failReads = false;
  source.next.snapshot.next.filter.enabled = false;
  source.next.overlayPolicy.next.filterEnabled = {app::PolicyObservationStatus::Available, false};
  store->readFailure = readFailure;
  QSignalSpy committed(&model, &MainWindowViewModel::stateCommitted);
  QSignalSpy failed(&model, &MainWindowViewModel::refreshFailed);
  model.refreshDiscardingChanges();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(committed.count(), 1);
  QCOMPARE(failed.count(), 0);
  QVERIFY(model.state().hasCommittedState());
  QVERIFY(model.state().snapshot().current.filter.enabled);
  QVERIFY(!model.state().snapshot().next.filter.enabled);
  QVERIFY(!model.workspaceDiagnostic());
  QVERIFY(!model.state().reconciliationRequired());
  QVERIFY(model.configurationWritesAllowed());
  QVERIFY(model.configuration()->pendingChanges().empty());
  QCOMPARE(model.pendingChangeCount(), std::size_t{0});
  QVERIFY(!draft->storageAvailable());
  QCOMPARE(draft->storageError(), readFailure);
  QCOMPARE(draft->editSnapshot().has_value(), stagingLoaded);
  if (stagingLoaded) QCOMPARE(draft->baseline(), QList<app::FileStagingEntry>{saved});
  QVERIFY(!draft->changes());
  QVERIFY(!draft->reconciliationRequired());
  QCOMPARE(source.reads, 2);
  QCOMPARE(store->writes, 0);
}

void MainWindowViewModelTests::explicitRefreshRecoversIndependentStagingWhenStateReadFails() {
  const app::FileStagingEntry saved{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")};
  auto store = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
  store->entries = {saved};
  store->readFailure = QStringLiteral("Saved staging list is unreadable");
  auto draft = std::make_shared<app::FileStagingDraft>(store);
  ControlledStateSource source;
  MainWindowViewModel model(source, app::UwfCapability::Available, draft);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(model.state().snapshot().next.filter.enabled);
  QVERIFY(!draft->storageAvailable());
  QVERIFY(!draft->editSnapshot());
  QCOMPARE(model.pendingChangeCount(), std::size_t{0});

  store->readFailure.clear();
  source.failReads = true;
  source.next.snapshot.next.filter.enabled = false;
  QSignalSpy committed(&model, &MainWindowViewModel::stateCommitted);
  QSignalSpy failed(&model, &MainWindowViewModel::refreshFailed);
  QSignalSpy recovered(draft.get(), &app::FileStagingDraft::storageAvailabilityChanged);
  model.refreshDiscardingChanges();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(committed.count(), 0);
  QCOMPARE(failed.count(), 1);
  QVERIFY(model.state().snapshot().next.filter.enabled);
  QCOMPARE(model.workspaceDiagnostic(), std::optional{QStringLiteral("published state unavailable")});
  QVERIFY(model.state().reconciliationRequired());
  QVERIFY(!model.configurationWritesAllowed());
  QVERIFY(draft->storageAvailable());
  QVERIFY(draft->storageError().isEmpty());
  QCOMPARE(recovered.count(), 1);
  QCOMPARE(draft->cachedSnapshot()->entries, QList<app::FileStagingEntry>{saved});
  QCOMPARE(draft->baseline(), QList<app::FileStagingEntry>{saved});
  QVERIFY(!draft->changes());
  QVERIFY(!draft->reconciliationRequired());
  QCOMPARE(store->writes, 0);
}

void MainWindowViewModelTests::explicitRefreshRetainsBothDraftsWhenStagingDiscardFails() {
  const app::FileStagingEntry original{app::FileStagingKind::File, QStringLiteral("C:\\original.bin")};
  const app::FileStagingEntry intended{app::FileStagingKind::File, QStringLiteral("C:\\intended.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {original};
  auto draft = std::make_shared<app::FileStagingDraft>(store);
  ControlledStateSource source;
  MainWindowViewModel model(source, app::UwfCapability::Available, draft);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  model.configuration()->global()->editWarningThreshold(3500);
  draft->replace({intended});
  QVERIFY(!draft->reconciliationRequired());
  const auto before = model.pendingChangeCount();
  QVERIFY(before > 0);

  store->failRead = true;
  source.next.snapshot.next.filter.enabled = false;
  source.next.overlayPolicy.next.filterEnabled = {app::PolicyObservationStatus::Available, false};
  QSignalSpy committed(&model, &MainWindowViewModel::stateCommitted);
  QSignalSpy failed(&model, &MainWindowViewModel::refreshFailed);
  model.refreshDiscardingChanges();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(committed.count(), 0);
  QCOMPARE(failed.count(), 1);
  QVERIFY(model.state().snapshot().next.filter.enabled);
  QCOMPARE(model.configuration()->global()->pendingOverlay().warningThresholdMb, std::optional<uint32_t>{3500});
  QCOMPARE(model.pendingChangeCount(), before);
  QCOMPARE(draft->changes()->after, QList<app::FileStagingEntry>{intended});
  QCOMPARE(draft->baseline(), QList<app::FileStagingEntry>{original});
  QVERIFY(!draft->storageAvailable());
  QVERIFY(!draft->reconciliationRequired());
  QVERIFY(model.interactionState().refreshEnabled);
  QCOMPARE(store->writes, 0);

  store->failRead = false;
  model.refreshDiscardingChanges();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(committed.count(), 1);
  QVERIFY(!model.state().snapshot().next.filter.enabled);
  QVERIFY(model.configuration()->pendingChanges().empty());
  QCOMPARE(model.pendingChangeCount(), std::size_t{0});
  QVERIFY(!draft->changes());
  QVERIFY(draft->storageAvailable());
  QCOMPARE(draft->cachedSnapshot()->entries, QList<app::FileStagingEntry>{original});
  QCOMPARE(store->writes, 0);
}

void MainWindowViewModelTests::explicitRefreshCanRetryAnUnknownWriteWithoutDiscardingIntentOnFailure() {
  const app::FileStagingEntry original{app::FileStagingKind::File, QStringLiteral("C:\\original.bin")};
  const app::FileStagingEntry intended{app::FileStagingKind::File, QStringLiteral("C:\\intended.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {original};
  auto draft = std::make_shared<app::FileStagingDraft>(store);
  ControlledStateSource source;
  MainWindowViewModel model(source, app::UwfCapability::Available, draft);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  model.configuration()->global()->editFilterEnabled(false);
  draft->replace({intended});
  store->failReadAfterReplace = true;
  QVERIFY_THROWS_EXCEPTION(app::OperationError, draft->apply(*draft->changes()));
  model.requireReconciliation();
  const auto before = model.pendingChangeCount();
  QVERIFY(before > 0);
  model.refreshDiscardingChanges();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(model.pendingChangeCount(), before);
  QCOMPARE(model.configuration()->global()->pendingFilterEnabled(), std::optional<bool>{false});
  QCOMPARE(draft->changes()->after, QList<app::FileStagingEntry>{intended});
  QVERIFY(draft->reconciliationRequired());
  QVERIFY(model.interactionState().refreshEnabled);
  store->failRead = false;
  model.refreshDiscardingChanges();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(model.pendingChangeCount(), std::size_t{0});
  QVERIFY(!model.configuration()->global()->pendingFilterEnabled());
  QVERIFY(!draft->changes());
  QVERIFY(!draft->reconciliationRequired());
  QCOMPARE(draft->cachedSnapshot()->entries, store->entries);
  QVERIFY(model.configurationWritesAllowed());
  QCOMPARE(store->writes, 1);
}

void MainWindowViewModelTests::refreshDiscardsMaintenanceOnlyWhenRequestedAndItsDependenciesSucceed_data() {
  QTest::addColumn<bool>("discard");
  QTest::addColumn<bool>("stagingIntent");
  QTest::addColumn<bool>("stateFailure");
  QTest::addColumn<bool>("stagingFailure");
  QTest::newRow("observe-retains-maintenance") << false << false << false << false;
  QTest::newRow("discard-clears-ordinary-and-maintenance") << true << false << false << false;
  QTest::newRow("system-failure-retains-both-drafts") << true << false << true << false;
  QTest::newRow("required-staging-failure-retains-both-drafts") << true << true << false << true;
  QTest::newRow("both-required-reads-fail") << true << true << true << true;
  QTest::newRow("optional-staging-failure-does-not-prevent-discard") << true << false << false << true;
}

void MainWindowViewModelTests::refreshDiscardsMaintenanceOnlyWhenRequestedAndItsDependenciesSucceed() {
  QFETCH(bool, discard);
  QFETCH(bool, stagingIntent);
  QFETCH(bool, stateFailure);
  QFETCH(bool, stagingFailure);
  ControlledStateSource source;
  auto& state = source.next;
  state.snapshot.current.filter.enabled = state.snapshot.next.filter.enabled = false;
  state.snapshot.current.volumes.front().isProtected = state.snapshot.next.volumes.front().isProtected = false;
  state.snapshot.runtime.systemVolumeProtected = false;
  state.overlayPolicy.current.filterEnabled = state.overlayPolicy.next.filterEnabled = {app::PolicyObservationStatus::Available, false};
  auto store = std::make_shared<MemoryStagingStore>();
  auto staging = std::make_shared<app::FileStagingDraft>(store);
  MainWindowViewModel model(source, app::UwfCapability::Available, staging);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  const auto configuration = model.configuration();
  const auto maintenance = configuration->maintenanceEditor();
  maintenance->setInteractionContext(true, false);
  app::OverlayMaintenanceSnapshot observed;
  observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
  observed.systemVolumeProtected = false;
  observed.currentFlags = app::OverlayFlags{0};
  observed.romDisposition = 0;
  observed.allVolumesProtected = observed.allVolumesNextProtected = false;
  observed.protectScopeConfirmed = true;
  observed.protectAllCapability.support = app::OverlayMaintenanceSupport::Supported;
  observed.protectTargetVolumes = {QString::fromStdString(state.snapshot.next.volumes.front().volumeName)};
  const auto generation = maintenance->beginObservation();
  QVERIFY(generation);
  maintenance->finishObservation(*generation, observed);
  const auto parsed = app::parseUwfmgrText("uwfmgr filter enable\nuwfmgr volume protect all");
  const auto report = configuration->importCommands({parsed.begin(), parsed.end()});
  QCOMPARE(report.size(), qsizetype{2});
  for (const auto& result : report) QCOMPARE(result.status, app::ImportResult::Status::Success);
  QCOMPARE(configuration->pendingChanges().setFilterEnabled, std::optional<bool>{true});
  QCOMPARE(maintenance->draft().allVolumesProtected, std::optional<bool>{true});
  QVERIFY(!model.state().snapshot().next.volumes.front().isProtected);
  QCOMPARE(model.pendingChangeCount(), std::size_t{2});
  if (stagingIntent) {
    staging->replace({{app::FileStagingKind::File, QStringLiteral("C:\\pending.bin")}});
    QVERIFY(staging->changes());
  }
  const auto before = model.pendingChangeCount();
  source.failReads = stateFailure;
  store->failRead = stagingFailure;
  QSignalSpy committed(&model, &MainWindowViewModel::stateCommitted);
  QSignalSpy failed(&model, &MainWindowViewModel::refreshFailed);
  if (discard)
    model.refreshDiscardingChanges();
  else
    model.refresh();
  QTRY_VERIFY(!model.refreshing());
  const bool rejected = stateFailure || (stagingIntent && stagingFailure);
  QCOMPARE(failed.count(), rejected ? 1 : 0);
  QCOMPARE(committed.count(), rejected ? 0 : 1);
  if (rejected) {
    QCOMPARE(model.pendingChangeCount(), before);
    QCOMPARE(configuration->pendingChanges().setFilterEnabled, std::optional<bool>{true});
    QCOMPARE(maintenance->draft().allVolumesProtected, std::optional<bool>{true});
    if (stagingIntent) QCOMPARE(staging->changes()->after.constFirst().path, QStringLiteral("C:\\pending.bin"));
    source.failReads = false;
    store->failRead = false;
    model.refreshDiscardingChanges();
    QTRY_VERIFY(!model.refreshing());
    QCOMPARE(committed.count(), 1);
  } else if (!discard) {
    // Observing system state must keep a maintenance workflow intent until an
    // actual discard or confirmed operation receipt consumes it.
    QVERIFY(!configuration->pendingChanges().setFilterEnabled);
    QCOMPARE(maintenance->draft().allVolumesProtected, std::optional<bool>{true});
    QCOMPARE(model.pendingChangeCount(), std::size_t{1});
    model.refreshDiscardingChanges();
    QTRY_VERIFY(!model.refreshing());
    QCOMPARE(committed.count(), 2);
  }
  QVERIFY(configuration->pendingConfiguration().empty());
  QVERIFY(maintenance->draft().empty());
  QVERIFY(!staging->changes());
  QCOMPARE(model.pendingChangeCount(), std::size_t{0});
  QCOMPARE(store->writes, 0);
}

void MainWindowViewModelTests::reviewRetriesStagingInTheBackgroundAndPreservesConfigurationEdits() {
  ControlledStateSource state;
  UwfMaintenanceFixture maintenance;
  auto store = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
  store->readFailure = QStringLiteral("Saved list unavailable");
  auto source = std::make_shared<WindowStagingSource>();
  const auto release = qScopeGuard([gate = source->gate] { gate->release(); });
  auto draft = std::make_shared<app::FileStagingDraft>(store, source);
  auto models = std::make_shared<WindowModels>();
  MainWindowViewModel model(state, app::UwfCapability::Available, draft);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  app::PowerInteraction interaction;
  interaction.confirm = [](app::PowerActionDialogRequest) { return app::PowerActionDialogOutcome::Canceled; };
  interaction.warning = [](const QString&, const QString&) {};
  interaction.describeStagingResult = [](const auto&) { return QString{}; };
  model.initializeWorkflows(models, {},
                            std::make_unique<app::PowerCoordinator>(app::PowerServices{maintenance.repository(), *store, app::UwfCapability::Available, {}, {}},
                                                                    std::move(interaction)));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(!draft->storageAvailable());
  model.configuration()->global()->editFilterEnabled(false);
  QSignalSpy failed(&model, &MainWindowViewModel::reviewFailed);
  QSignalSpy prepared(&model, &MainWindowViewModel::reviewPrepared);
  QSignalSpy committed(&model, &MainWindowViewModel::stateCommitted);
  model.requestReview();
  QVERIFY(model.refreshing());
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(failed.count(), 1);
  QCOMPARE(prepared.count(), 0);
  QCOMPARE(model.configuration()->global()->pendingFilterEnabled(), std::optional<bool>{false});
  QVERIFY(model.configurationReviewAllowed());
  store->readFailure.clear();
  store->entries = {{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
  source->blockNext = true;
  model.requestReview();
  QTRY_COMPARE(source->gate->started.load(), 1);
  bool heartbeat = false;
  QTimer::singleShot(0, &model, [&heartbeat] { heartbeat = true; });
  QTRY_VERIFY(heartbeat);
  QCOMPARE(prepared.count(), 0);
  QCOMPARE(source->ownerObservations.load(), 0);
  source->gate->release();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(prepared.count(), 1);
  QCOMPARE(failed.count(), 1);
  QCOMPARE(committed.count(), 0);
  QCOMPARE(model.configuration()->global()->pendingFilterEnabled(), std::optional<bool>{false});
  QVERIFY(draft->storageAvailable());
  QCOMPARE(draft->cachedSnapshot()->entries, store->entries);
  QCOMPARE(source->ownerObservations.load(), 0);
  QVERIFY(source->workerObservations.load() > 0);
  QCOMPARE(state.reads, 1);
  QCOMPARE(state.publishedReads, 0);
  QCOMPARE(store->writes, 0);
}

void MainWindowViewModelTests::discardingStagingUsesTheKnownCachedBaseline() {
  WindowFixture fixture;
  fixture.store->entries = {{app::FileStagingKind::File, QStringLiteral("C:\\original.bin")}};
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  fixture.draft->replace({{app::FileStagingKind::File, QStringLiteral("C:\\intended.bin")}});
  fixture.window->configuration()->global()->editFilterEnabled(false);
  QVERIFY(fixture.window->pendingChangeCount() > 0);
  const auto reads = fixture.store->loads;
  fixture.store->readFailure = QStringLiteral("Storage must not be read while discarding cached edits");
  QVERIFY(fixture.window->discardPendingChanges());
  QCOMPARE(fixture.store->loads, reads);
  QCOMPARE(fixture.draft->cachedSnapshot()->entries, fixture.store->entries);
  QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{0});
  QCOMPARE(fixture.store->writes, 0);
}

void MainWindowViewModelTests::windowStateRetainsCommittedBaselineAfterReadFailure() {
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

void MainWindowViewModelTests::refreshFailureDisablesUwfOperationsUntilSuccessfulRefresh() {
  FakeStateSource source;
  source.next = availableState();
  MainWindowViewModel model(source, app::UwfCapability::Available);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(model.configurationWritesAllowed());
  QVERIFY(model.maintenanceActionsAllowed());
  QVERIFY(model.powerActionsAllowed());
  const auto priorMaximum = model.state().snapshot().next.overlay.maximumSizeMb;
  source.next.snapshot.next.overlay.maximumSizeMb = priorMaximum + 1024;
  source.fail = true;
  QSignalSpy failures(&model, &MainWindowViewModel::refreshFailed);
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(failures.count(), 1);
  QVERIFY(model.state().hasCommittedState());
  QCOMPARE(model.state().snapshot().next.overlay.maximumSizeMb, priorMaximum);
  QVERIFY(!model.configurationWritesAllowed());
  QVERIFY(!model.configurationReviewAllowed());
  QVERIFY(!model.maintenanceActionsAllowed());
  QVERIFY(!model.powerActionsAllowed());
  QVERIFY(!model.configuration()->global()->controlsEnabled());
  QVERIFY(!model.interactionState().volumePagesEnabled);
  QVERIFY(model.interactionState().refreshEnabled);
  QVERIFY(model.exitAllowed());
  QCOMPARE(model.configurationWriteRestriction(), QStringLiteral("snapshot failed"));
  source.fail = false;
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  QVERIFY(!model.workspaceDiagnostic());
  QVERIFY(model.configurationWritesAllowed());
  QVERIFY(model.maintenanceActionsAllowed());
  QVERIFY(model.powerActionsAllowed());
  QCOMPARE(model.state().snapshot().next.overlay.maximumSizeMb, priorMaximum + 1024);
}

void MainWindowViewModelTests::refreshFailureBlocksAnAlreadyOpenedEnhancedModeEditor() {
  WindowFixture fixture;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  const auto editor = fixture.window->enhancedModeModel();
  QVERIFY(editor);
  editor->refreshStatus();
  QTRY_VERIFY(editor->status().has_value());
  QVERIFY(editor->statusText().canRemove);
  const auto installs = fixture.service->installs;
  const auto starts = fixture.service->starts;
  const auto removals = fixture.service->removals;
  fixture.source.failReads = true;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  QVERIFY(!fixture.window->interactionState().enhancedModeEnabled);
  QVERIFY(!editor->statusText().canApply);
  QVERIFY(!editor->statusText().canRemove);
  editor->applyRequestedState();
  editor->removeService();
  QCOMPARE(fixture.service->installs, installs);
  QCOMPARE(fixture.service->starts, starts);
  QCOMPARE(fixture.service->removals, removals);
  fixture.source.failReads = false;
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  QVERIFY(fixture.window->interactionState().enhancedModeEnabled);
  QVERIFY(editor->statusText().canRemove);
}

void MainWindowViewModelTests::postApplyObservationRetainsEveryUnfinishedOrdinaryIntent_data() {
  QTest::addColumn<bool>("readFails");
  QTest::addColumn<bool>("discard");
  QTest::newRow("observe") << false << false;
  QTest::newRow("failed-observation-retry") << true << false;
  QTest::newRow("discard-after-observation") << false << true;
  QTest::newRow("discard-after-failed-observation") << true << true;
}

void MainWindowViewModelTests::postApplyObservationRetainsEveryUnfinishedOrdinaryIntent() {
  QFETCH(bool, readFails);
  QFETCH(bool, discard);
  WindowFixture fixture(false);
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  const auto configuration = fixture.window->configuration();
  const auto parsed =
      app::parseUwfmgrText("uwfmgr filter disable && uwfmgr volume unprotect C: && uwfmgr registry add-exclusion HKEY_LOCAL_MACHINE\\SOFTWARE\\RetryIntent");
  const auto imported = configuration->importCommands({parsed.begin(), parsed.end()});
  QCOMPARE(imported.size(), 3);
  for (const auto& row : imported) QCOMPARE(row.status, app::ImportResult::Status::Success);
  QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{3});
  app::ConfigurationApplyResult result;
  result.steps = {{"filter", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed,
                   app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}},
                  {"volume", app::ConfigurationStepOutcome::Failed, app::WriteEffect::Uncertain,
                   app::ConfigurationOperation{app::ConfigurationOperationKind::VolumeProtection, fixtureVolumeId}},
                  {"registry", app::ConfigurationStepOutcome::NotAttempted, app::WriteEffect::None,
                   app::ConfigurationOperation{app::ConfigurationOperationKind::AddRegistryExclusion, "HKEY_LOCAL_MACHINE\\SOFTWARE\\RetryIntent"}}};
  fixture.models->actions->applyResult = result;
  fixture.source.next.snapshot.next.filter.enabled = false;
  fixture.source.next.overlayPolicy.next.filterEnabled.value = false;
  fixture.source.failPublished = readFails;
  const auto plan = fixture.window->reviewConfiguration();
  QVERIFY(plan && plan->canApply());
  plan->apply();
  fixture.models->actions->gate->release();
  QTRY_VERIFY(!plan->applying());
  QTRY_COMPARE(fixture.source.publishedReads, 1);
  QTRY_VERIFY(!fixture.window->refreshing());
  QCOMPARE(fixture.window->state().reconciliationRequired(), readFails);
  const auto remaining = configuration->pendingConfiguration();
  QVERIFY(!remaining.settings.setFilterEnabled);
  QCOMPARE(remaining.settings.volumeProtect.at(fixtureVolumeId), false);
  QCOMPARE(remaining.settings.addRegistryExclusions, (std::vector<std::string>{"HKEY_LOCAL_MACHINE\\SOFTWARE\\RetryIntent"}));
  QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{2});
  fixture.source.failPublished = false;
  if (discard) {
    fixture.window->refreshDiscardingChanges();
  } else if (readFails) {
    fixture.window->refresh();
  }
  QTRY_VERIFY(!fixture.window->refreshing());
  QVERIFY(!fixture.window->state().reconciliationRequired());
  const auto refreshed = configuration->pendingConfiguration();
  QVERIFY(!refreshed.settings.setFilterEnabled);
  if (discard) {
    QVERIFY(refreshed.empty());
    QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{0});
  } else {
    QCOMPARE(refreshed.settings.volumeProtect.at(fixtureVolumeId), false);
    QCOMPARE(refreshed.settings.addRegistryExclusions, remaining.settings.addRegistryExclusions);
    QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{2});
  }
  QCOMPARE(fixture.models->actions->prepared, 1);
  QCOMPARE(fixture.store->writes, 0);
}

void MainWindowViewModelTests::windowReconciliationRequiresEveryWrittenStoreToBeReadable_data() {
  QTest::addColumn<bool>("stateReadable");
  QTest::addColumn<bool>("stagingReadable");
  for (const bool state : {false, true})
    for (const bool staging : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("state-%1-staging-%2").arg(state).arg(staging))) << state << staging;
}

void MainWindowViewModelTests::windowReconciliationRequiresEveryWrittenStoreToBeReadable() {
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

void MainWindowViewModelTests::windowStatusBindingSurvivesItsSessionAndCanBeReplaced() {
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
  auto failedActions = std::make_unique<FakeEnhancedModeActions>();
  failedActions->failRead = true;
  auto session = std::make_unique<app::EnhancedModeSession>(std::move(failedActions), agent);
  model.bindEnhancedMode(*session);
  model.refreshEnhancedModeStatus();
  QCOMPARE(model.enhancedModeStatus().state, app::EnhancedModeState::RepairRequired);
  session.reset();
  model.refreshEnhancedModeStatus();

  app::EnhancedModeSession replacement(std::make_unique<FakeEnhancedModeActions>(), agent);
  model.bindEnhancedMode(replacement);
  QSignalSpy observed(&model, &presentation::MainWindowViewModel::enhancedModeStatusChanged);
  model.refreshEnhancedModeStatus();
  QCOMPARE(observed.size(), 1);
  QCOMPARE(model.enhancedModeStatus().state, app::EnhancedModeState::Disabled);
  model.unbindEnhancedMode();
  replacement.refresh();
  QCOMPARE(observed.size(), 1);
}

void MainWindowViewModelTests::maintenanceDraftUsesTheUnifiedReviewAndDiscardWithoutReleasingCurrentHorm_data() {
  QTest::addColumn<bool>("earlyConfiguration");
  QTest::newRow("configuration-before-workflows") << true;
  QTest::newRow("configuration-after-workflows-and-observation") << false;
}

void MainWindowViewModelTests::maintenanceDraftUsesTheUnifiedReviewAndDiscardWithoutReleasingCurrentHorm() {
  QFETCH(bool, earlyConfiguration);
  {
    auto actions = std::make_shared<WindowMaintenanceActions>();
    actions->current.currentFlags = actions->current.nextFlags = app::OverlayFlags{4};
    std::shared_ptr<OverlayMaintenanceViewModel> maintenance;
    WindowFixture fixture(
        true, false, false,
        [&](WindowFixture& target) {
          target.source.next.overlayPolicy.current.flags =
              target.source.next.overlayPolicy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{4}};
          target.source.next.snapshot.runtime.currentOverlayFlags = 4;
          target.source.next.snapshot.runtime.hormEnabled = false;
          target.source.next.snapshot.runtime.systemVolumeProtected = true;
          target.models->maintenanceFactory = [&maintenance, actions](std::shared_ptr<app::OverlayMaintenanceEditor> editor) {
            maintenance = std::make_shared<OverlayMaintenanceViewModel>(actions, std::move(editor));
            return maintenance;
          };
        },
        earlyConfiguration);
    fixture.window->refresh();
    QTRY_VERIFY(!fixture.window->refreshing() && maintenance->hasSnapshot() && !maintenance->busy());
    if (!earlyConfiguration) fixture.window->initializeConfiguration(16 * 1024, QStringLiteral("C:"));
    QCOMPARE(fixture.models->maintenanceModels, 1);
    const auto configuration = fixture.window->configuration();
    QCOMPARE(maintenance->editor(), configuration->maintenanceEditor());
    QSignalSpy configurationPending(configuration.get(), &ConfigurationViewModel::pendingChanged);
    QSignalSpy windowPending(fixture.window.get(), &MainWindowViewModel::pendingChanged);
    QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{0});
    QVERIFY(maintenance->stageFullCommit());
    configuration->global()->editWarningThreshold(4000);
    QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{1});
    QVERIFY(configuration->pendingConfiguration().maintenance.fullCommit);
    QCOMPARE(configuration->pendingConfiguration().maintenance, maintenance->draft());
    QVERIFY(configuration->pendingConfiguration().settings.empty());
    auto plan = fixture.window->reviewConfiguration();
    QVERIFY(plan);
    QCOMPARE(plan->changeCount(), std::size_t{1});
    QVERIFY(plan->hasMaintenanceChanges());
    QVERIFY(plan->maintenanceChanges().fullCommit);
    QVERIFY(std::ranges::any_of(plan->groups(), [](const auto& group) { return group.title == QStringLiteral("Maintenance and recovery"); }));
    QCOMPARE(actions->writes.load(), 0);
    QCOMPARE(fixture.models->actions->prepared, 0);
    configurationPending.clear();
    windowPending.clear();
    QVERIFY(fixture.window->discardPendingChanges());
    QCOMPARE(configurationPending.count(), 1);
    QCOMPARE(windowPending.count(), 1);
    QVERIFY(!maintenance->hasPendingChanges());
    QVERIFY(configuration->pendingConfiguration().empty());
    QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{0});
    QCOMPARE(maintenance->snapshot().overlayCommit, std::optional<bool>{false});
    QVERIFY(maintenance->stageFullCommit());
    configuration->global()->editWarningThreshold(4000);
    configurationPending.clear();
    windowPending.clear();
    configuration->resetSnapshot(fixture.source.next.snapshot, fixture.source.next.overlayPolicy, fixture.source.next.hormEnabled);
    QCOMPARE(configurationPending.count(), 1);
    QCOMPARE(windowPending.count(), 1);
    QVERIFY(configuration->pendingConfiguration().settings.empty());
    QVERIFY(configuration->pendingConfiguration().overlayPolicy.empty());
    QVERIFY(configuration->pendingConfiguration().maintenance.fullCommit);
    QCOMPARE(configuration->pendingConfiguration().maintenance, maintenance->draft());
    QVERIFY(maintenance->hasPendingChanges());
    QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{1});
    configurationPending.clear();
    windowPending.clear();
    configuration->discardChanges(fixture.source.next.snapshot, fixture.source.next.overlayPolicy, fixture.source.next.hormEnabled);
    QCOMPARE(configurationPending.count(), 1);
    QCOMPARE(windowPending.count(), 1);
    QVERIFY(configuration->pendingConfiguration().empty());
    QVERIFY(!maintenance->hasPendingChanges());
    QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{0});
    QCOMPARE(fixture.models->maintenanceModels, 1);
    QCOMPARE(actions->writes.load(), 0);
  }
  {
    auto actions = std::make_shared<WindowMaintenanceActions>();
    actions->current.hormEnabled = true;
    actions->current.currentFlags = actions->current.nextFlags = app::OverlayFlags{4};
    actions->current.romDisposition = 2;
    std::shared_ptr<OverlayMaintenanceViewModel> maintenance;
    WindowFixture fixture(
        true, false, false,
        [&](WindowFixture& target) {
          target.source.next.hormEnabled = true;
          target.source.next.snapshot.runtime.romDisposition = 2;
          target.source.next.overlayPolicy.current.flags =
              target.source.next.overlayPolicy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{4}};
          target.models->maintenanceFactory = [&maintenance, actions](std::shared_ptr<app::OverlayMaintenanceEditor> editor) {
            maintenance = std::make_shared<OverlayMaintenanceViewModel>(actions, std::move(editor));
            return maintenance;
          };
        },
        earlyConfiguration);
    fixture.window->refresh();
    QTRY_VERIFY(!fixture.window->refreshing() && maintenance->hasSnapshot() && !maintenance->busy());
    if (!earlyConfiguration) fixture.window->initializeConfiguration(16 * 1024, QStringLiteral("C:"));
    QCOMPARE(fixture.models->maintenanceModels, 1);
    QCOMPARE(maintenance->editor(), fixture.window->configuration()->maintenanceEditor());
    QVERIFY(maintenance->hormActive());
    QVERIFY(!fixture.window->configurationWritesAllowed());
    QVERIFY(!fixture.window->configurationReviewAllowed());
    QVERIFY(maintenance->stageHorm(false));
    QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{1});
    QCOMPARE(fixture.window->configuration()->pendingConfiguration().maintenance.hormEnabled, std::optional<bool>{false});
    QVERIFY(fixture.window->configuration()->pendingConfiguration().settings.empty());
    QVERIFY(fixture.window->configuration()->pendingConfiguration().overlayPolicy.empty());
    QVERIFY(fixture.window->configurationReviewAllowed());
    auto plan = fixture.window->reviewConfiguration();
    QVERIFY(plan);
    QVERIFY(plan->onlyDisablesHorm());
    QVERIFY(plan->canApply());
    QCOMPARE(plan->maintenanceChanges().hormEnabled, std::optional<bool>{false});
    // An intent to disable HORM is not a new Current observation.
    QVERIFY(maintenance->hormActive());
    QVERIFY(!maintenance->restartRequired());
    QVERIFY(!fixture.window->configurationWritesAllowed());
    QVERIFY(!fixture.window->configuration()->global()->controlsEnabled());
    QCOMPARE(actions->writes.load(), 0);
    QVERIFY(fixture.window->discardPendingChanges());
    QVERIFY(!fixture.window->configurationReviewAllowed());
    QVERIFY(maintenance->hormActive());
    QVERIFY(fixture.window->configuration()->pendingConfiguration().empty());
    QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{0});
    QCOMPARE(actions->writes.load(), 0);
  }
}

void MainWindowViewModelTests::windowRefreshAdoptsMaintenanceWithoutStartingAnotherRead() {
  auto actions = std::make_shared<WindowMaintenanceActions>();
  std::shared_ptr<OverlayMaintenanceViewModel> maintenance;
  WindowFixture fixture(true, false, false, [&](WindowFixture& target) {
    target.models->maintenanceFactory = [&maintenance, actions](std::shared_ptr<app::OverlayMaintenanceEditor> editor) {
      maintenance = std::make_shared<OverlayMaintenanceViewModel>(actions, std::move(editor));
      return maintenance;
    };
  });
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing() && maintenance->hasSnapshot() && !maintenance->busy());
  QCoreApplication::processEvents();
  QCOMPARE(fixture.source.reads, 1);
  QCOMPARE(actions->reads.load(), 0);
  QVERIFY(fixture.window->configurationWritesAllowed());
  QVERIFY(maintenance->snapshot().readError.isEmpty());
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing());
  QCoreApplication::processEvents();
  QCOMPARE(fixture.source.reads, 2);
  QCOMPARE(actions->reads.load(), 0);
  maintenance->refresh();
  QTRY_VERIFY(!maintenance->busy());
  QCOMPARE(actions->reads.load(), 1);
  QCOMPARE(fixture.source.reads, 2);
  QCOMPARE(fixture.source.publishedReads, 0);
  QCOMPARE(actions->writes.load(), 0);
  QVERIFY(fixture.window->configurationWritesAllowed());
}

void MainWindowViewModelTests::maintenanceReadDoesNotBlockWindowInteractionsOrPower() {
  auto actions = std::make_shared<WindowMaintenanceActions>();
  actions->current.currentFlags = app::OverlayFlags{0};
  std::shared_ptr<OverlayMaintenanceViewModel> maintenance;
  WindowFixture fixture(true, false, false, [&](WindowFixture& target) {
    target.models->maintenanceFactory = [&maintenance, actions](std::shared_ptr<app::OverlayMaintenanceEditor> editor) {
      maintenance = std::make_shared<OverlayMaintenanceViewModel>(actions, std::move(editor));
      return maintenance;
    };
  });
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing() && maintenance->hasSnapshot() && !maintenance->busy());
  QSignalSpy interactions(fixture.window.get(), &MainWindowViewModel::interactionAvailabilityChanged);
  QSignalSpy applying(fixture.window.get(), &MainWindowViewModel::configurationApplyingChanged);
  actions->readGate = std::make_shared<test::WorkerGate>();
  maintenance->refresh();
  QVERIFY(maintenance->busy());
  QTRY_COMPARE(actions->readGate->started.load(), 1);
  const auto interaction = fixture.window->interactionState();
  QVERIFY(interaction.refreshEnabled);
  QVERIFY(interaction.configurationEnabled);
  QVERIFY(interaction.reviewEnabled);
  QVERIFY(interaction.powerEnabled);
  QVERIFY(interaction.exitEnabled);
  QVERIFY(interaction.volumePagesEnabled);
  QVERIFY(fixture.window->configuration()->global()->controlsEnabled());
  QVERIFY(fixture.window->maintenanceActionsAllowed());
  QCOMPARE(fixture.window->stagingEditingRestriction(), app::RuntimeWriteRestriction::None);
  QVERIFY(!fixture.window->refreshing());
  QVERIFY(!fixture.window->applyingConfiguration());
  QCOMPARE(interactions.count(), 0);
  QCOMPARE(applying.count(), 0);
  fixture.window->safeRestart();
  QCOMPARE(fixture.powerRequests.size(), std::size_t{1});
  auto plan = fixture.reviewEdit();
  QVERIFY(plan && plan->canApply());
  plan->apply();
  QVERIFY(fixture.window->applyingConfiguration());
  QVERIFY(!fixture.window->configurationWritesAllowed());
  fixture.window->safeRestart();
  QCOMPARE(fixture.powerRequests.size(), std::size_t{1});
  fixture.models->actions->gate->release();
  QTRY_VERIFY(!fixture.window->applyingConfiguration());
  fixture.window->safeRestart();
  QCOMPARE(fixture.powerRequests.size(), std::size_t{2});
  QVERIFY(maintenance->busy());
  // Main state reads remain independently available while a dialog reads.
  fixture.window->refresh();
  QVERIFY(fixture.window->refreshing());
  QTRY_VERIFY(!fixture.window->refreshing());
  QVERIFY(maintenance->busy());
  actions->readGate->release();
  QTRY_VERIFY(!maintenance->busy());
  QVERIFY(fixture.window->configurationWritesAllowed());
  QCOMPARE(actions->writes.load(), 0);
}

void MainWindowViewModelTests::protectAllDraftReconcilesOnlySuccessfulConfirmedApply_data() {
  QTest::addColumn<bool>("enabled");
  QTest::addColumn<bool>("succeeded");
  QTest::addColumn<bool>("confirmed");
  for (const bool enabled : {false, true})
    for (const bool succeeded : {false, true})
      for (const bool confirmed : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("enabled-%1-success-%2-confirmed-%3").arg(enabled).arg(succeeded).arg(confirmed)))
            << enabled << succeeded << confirmed;
}

void MainWindowViewModelTests::protectAllDraftReconcilesOnlySuccessfulConfirmedApply() {
  QFETCH(bool, enabled);
  QFETCH(bool, succeeded);
  QFETCH(bool, confirmed);
  auto actions = std::make_shared<WindowMaintenanceActions>();
  actions->current.allVolumesNextProtected = false;
  actions->current.allVolumesNextUnprotected = false;
  std::shared_ptr<OverlayMaintenanceViewModel> maintenance;
  WindowFixture fixture(true, false, false, [&](WindowFixture& target) {
    target.models->maintenanceFactory = [&maintenance, actions](std::shared_ptr<app::OverlayMaintenanceEditor> editor) {
      maintenance = std::make_shared<OverlayMaintenanceViewModel>(actions, std::move(editor));
      return maintenance;
    };
  });
  fixture.window->refresh();
  QTRY_VERIFY(!fixture.window->refreshing() && maintenance->hasSnapshot() && !maintenance->busy());
  maintenance->replaceDraft({{}, {}, false, enabled});
  QCOMPARE(fixture.window->pendingChangeCount(), std::size_t{1});
  auto observed = actions->current;
  if (enabled)
    observed.allVolumesNextProtected = confirmed;
  else
    observed.allVolumesNextUnprotected = confirmed;
  app::ConfigurationApplyResult result;
  result.observedOverlayMaintenance = observed;
  result.steps.push_back(
      {"Protect all", succeeded ? app::ConfigurationStepOutcome::Succeeded : app::ConfigurationStepOutcome::Failed,
       succeeded ? app::WriteEffect::Confirmed : app::WriteEffect::Uncertain,
       app::ConfigurationOperation{enabled ? app::ConfigurationOperationKind::ProtectAllVolumes : app::ConfigurationOperationKind::UnprotectAllVolumes, {}}});
  fixture.models->actions->applyResult = result;
  auto plan = fixture.window->reviewConfiguration();
  QVERIFY(plan);
  QVERIFY(plan->canApply());
  QVERIFY(!plan->onlyDisablesHorm());
  QCOMPARE(plan->changeCount(), std::size_t{1});
  plan->apply();
  QVERIFY(fixture.window->applyingConfiguration());
  QVERIFY(!fixture.window->configurationWritesAllowed());
  fixture.models->actions->gate->release();
  QTRY_VERIFY(!plan->applying());
  QCOMPARE(maintenance->draft().allVolumesProtected, succeeded && confirmed ? std::optional<bool>{} : std::optional<bool>{enabled});
  QCOMPARE(fixture.window->pendingChangeCount(), succeeded && confirmed ? std::size_t{0} : std::size_t{1});
  QCOMPARE(actions->writes.load(), 0);
}

}  // namespace

QTEST_GUILESS_MAIN(MainWindowViewModelTests)
#include "MainWindowViewModelTests.moc"
