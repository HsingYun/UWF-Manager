// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QThreadPool>
#include <QTimer>
#include <array>
#include <functional>
#include <optional>
#include <stdexcept>
#include <utility>

#include "application/diagnostics/contracts/SystemInformationSource.h"
#include "application/power/contracts/PowerInteraction.h"
#include "application/state/contracts/ApplicationStateSource.h"
#include "presentation/appearance/I18n.h"
#include "presentation/viewmodels/common/ViewModelFactory.h"
#include "presentation/views/commit/CommitDispatcher.h"
#include "presentation/views/overlay/coordination/OverlayContextMenu.h"
#include "presentation/views/overlay/hud/OverlayFloatingWidget.h"
#include "presentation/views/shell/MainWindow.h"
#include "presentation/views/shell/WindowWorkflowFactory.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/FileTargetFixture.h"
#include "support/app/UwfMaintenanceFixture.h"
#include "support/app/WorkerGate.h"
#include "support/ui/FileDialogStub.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

core::UwfSnapshot snapshot(bool elevated = true, bool available = true, bool protectedFilter = true) {
  core::UwfSnapshot value;
  value.elevated = elevated;
  value.uwfAvailable = available;
  value.current.filter.enabled = protectedFilter;
  value.current.overlay = {core::OverlayType::RAM, 4096, 1024, 3072};
  value.next = value.current;
  return value;
}

class SystemInformation final : public app::SystemInformationSource {
 public:
  uint32_t totalRamMb() const override { return 16384; }
  QString systemDriveLetter() const override { return QStringLiteral("C:"); }
  const app::WindowsVersionInfo& windowsVersion() const override { return version; }
  const app::SystemHardwareInfo& hardware() const override { return hardwareInfo; }
  app::SystemDiagnosticFacts diagnostics() const override { return {}; }
  app::WindowsVersionInfo version;
  app::SystemHardwareInfo hardwareInfo;
};

class UsageReader final : public app::OverlayUsageReader {
 public:
  app::OverlayUsageState read() const override { return app::OverlayUsageUnavailable{}; }
};

// Reject unexpected dialog creation to keep this fixture within the main-window composition.
class ViewModels final : public presentation::ViewModelFactory {
 public:
  std::unique_ptr<presentation::MainWindowViewModel> mainWindow(app::ApplicationStateSource& source, app::UwfCapability capability,
                                                                std::shared_ptr<app::FileStagingDraft> staging) override {
    auto model = std::make_unique<presentation::MainWindowViewModel>(source, capability, std::move(staging));
    owner = model.get();
    return model;
  }
  std::shared_ptr<presentation::SystemInformationViewModel> systemInformation() override {
    return std::make_shared<presentation::SystemInformationViewModel>(std::make_shared<SystemInformation>(information));
  }
  std::shared_ptr<presentation::OverlayViewModel> overlay() override {
    return std::make_shared<presentation::OverlayViewModel>(std::make_shared<UsageReader>());
  }
  std::shared_ptr<presentation::ImportViewModel> importCommands() override { throw std::logic_error("unexpected import dialog"); }
  std::shared_ptr<presentation::LogViewModel> log() override { throw std::logic_error("unexpected log dialog"); }
  std::shared_ptr<presentation::UserConfigExportViewModel> userConfigExport() override { throw std::logic_error("unexpected export dialog"); }
  std::shared_ptr<presentation::FileStagingViewModel> fileStaging(QString, app::FileStagingDraft&, app::FileStagingConflictPolicy*,
                                                                  const app::FileStagingScope&) override {
    throw std::logic_error("unexpected staging dialog");
  }
  std::shared_ptr<presentation::ApplyPlanViewModel> applyPlan(app::ConfigurationChanges, const core::UwfSnapshot&, const app::OverlayPolicySnapshot&,
                                                              std::shared_ptr<app::FileStagingDraft>,
                                                              std::optional<app::OverlayMaintenanceSnapshot> = {}) override {
    throw std::logic_error("unexpected apply dialog");
  }
  std::shared_ptr<presentation::EnhancedModeViewModel> enhancedMode(app::EnhancedModeActions&) override {
    throw std::logic_error("unexpected enhanced-mode dialog");
  }
  std::shared_ptr<presentation::OverlayFilesViewModel> overlayFiles(std::shared_ptr<presentation::VolumeConfigurationViewModel>,
                                                                    presentation::MainWindowViewModel*) override {
    throw std::logic_error("unexpected overlay enumeration");
  }
  std::shared_ptr<presentation::RegistryPickerViewModel> registryPicker(presentation::RegistryPickerViewModel::Mode) override {
    throw std::logic_error("unexpected registry dialog");
  }
  presentation::MainWindowViewModel* owner = nullptr;
  SystemInformation information;
};

class Workflows final : public ui::WindowWorkflowFactory {
 public:
  Workflows(app::ApplicationStateSource& source, std::shared_ptr<MemoryFileStagingStore> store) : source(source), store(std::move(store)) {}
  std::unique_ptr<presentation::MainWindowViewModel> mainWindow(std::shared_ptr<presentation::ViewModelFactory> models, app::UwfCapability capability,
                                                                QWidget*) override {
    auto model = models->mainWindow(source, capability, std::make_shared<app::FileStagingDraft>(store));
    app::PowerInteraction interaction;
    interaction.confirm = [this](app::PowerActionDialogRequest) {
      ++confirmations;
      if (duringConfirmation) duringConfirmation();
      return app::PowerActionDialogOutcome::Canceled;
    };
    interaction.warning = [this](const QString&, const QString&) { ++warnings; };
    interaction.describeStagingResult = [](const app::FileStagingCommitResult&) { return QString{}; };
    auto power = std::make_unique<app::PowerCoordinator>(app::PowerServices{backend.repository(), *store, capability, {}, {}}, std::move(interaction));
    model->initializeWorkflows(std::move(models), {}, std::move(power));
    return model;
  }
  std::unique_ptr<ui::CommitDispatcher> commit(ui::CommitDispatcher::PauseObservation pauseObservation, ui::CommitDispatcher::ActivityChanged activityChanged,
                                               QWidget*) override {
    app::CommitInteraction interaction;
    interaction.run = [this](const app::CommitRequest& request, std::shared_ptr<app::CommitBackend>) {
      ++commitInteractions;
      if (duringCommit) duringCommit(request);
    };
    auto result = std::make_unique<ui::CommitDispatcher>(
        std::make_unique<app::CommitWorkflow>(backend.repository(), std::move(interaction), std::make_shared<StoredPathSource>()), std::move(pauseObservation),
        std::move(activityChanged));
    commitDispatcher = result.get();
    return result;
  }
  app::ApplicationStateSource& source;
  std::shared_ptr<MemoryFileStagingStore> store;
  test::application::UwfMaintenanceFixture backend;
  int confirmations = 0;
  int warnings = 0;
  int commitInteractions = 0;
  ui::CommitDispatcher* commitDispatcher = nullptr;
  std::function<void()> duringConfirmation;
  std::function<void(const app::CommitRequest&)> duringCommit;
};

class ApplyActions final : public app::ConfigurationActions {
 public:
  explicit ApplyActions(std::shared_ptr<test::WorkerGate> gate) : gate(std::move(gate)) {}
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
    return [gate = gate] {
      gate->wait({}, false);
      return app::ConfigurationApplyResult{};
    };
  }
  std::shared_ptr<test::WorkerGate> gate;
};

class ObservationSource final : public app::ApplicationStateSource {
 public:
  ReadOperation readOperation(app::UwfCapability) override {
    return [candidate = observation, gate = gate](std::stop_token stop) {
      if (gate) gate->wait(stop);
      return candidate;
    };
  }
  app::ApplicationState observation;
  std::shared_ptr<test::WorkerGate> gate;
};

enum class ConfigurationCondition { Ready, Horm, ReadOnly, Unavailable, Reconciliation, ExternalOperation, Refreshing, Applying };

QAction* command(ui::MainWindow& window, const QString& text) {
  const auto* commands = window.findChild<QWidget*>(QStringLiteral("mainCommands"));
  if (commands)
    for (auto actions = commands->findChildren<QAction*>(); auto* action : std::as_const(actions))
      if (action->text() == text) return action;
  return nullptr;
}

std::optional<bool> hubPowerAvailability(bool requestPower = false, std::optional<bool>* exitAllowed = nullptr) {
  ui::OverlayFloatingWidget* hub = nullptr;
  for (auto widgets = QApplication::allWidgets(); auto* widget : std::as_const(widgets))
    if ((hub = qobject_cast<ui::OverlayFloatingWidget*>(widget))) break;
  if (!hub) return std::nullopt;
  auto* handle = hub->findChild<QWidget*>(QStringLiteral("overlayFloatingHandle"));
  if (!handle) return std::nullopt;
  std::optional<bool> allowed;
  QTimer inspector;
  inspector.setSingleShot(true);
  QObject::connect(&inspector, &QTimer::timeout, &inspector, [&] {
    auto* menu = hub->findChild<ui::OverlayContextMenu*>();
    if (!menu) return;
    const auto* shutdown = menu->findChild<QAction*>(QStringLiteral("overlayMenuSafeShutdown"));
    const auto* restart = menu->findChild<QAction*>(QStringLiteral("overlayMenuSafeRestart"));
    const auto* exit = menu->findChild<QAction*>(QStringLiteral("overlayMenuExit"));
    if (shutdown && restart && shutdown->isEnabled() == restart->isEnabled()) allowed = restart->isEnabled();
    if (exitAllowed && exit) *exitAllowed = exit->isEnabled();
    // Direct signal delivery bypasses disabled QAction::trigger(), exercising
    // the execution guard on the actual HUB -> window -> power route.
    if (requestPower && (!QMetaObject::invokeMethod(menu, "safeShutdownRequested", Qt::DirectConnection) ||
                         !QMetaObject::invokeMethod(menu, "safeRestartRequested", Qt::DirectConnection)))
      allowed.reset();
    menu->close();
  });
  inspector.start(0);
  QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(2, 2), QPoint(100, 100));
  QApplication::sendEvent(handle, &event);
  return allowed;
}

class PowerAvailabilityTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() { ui::I18n::instance().setLang(ui::I18n::Lang::En); }

  void menuStartsWithPowerDisabled() {
    ui::OverlayContextMenu menu;
    QVERIFY(!menu.findChild<QAction*>(QStringLiteral("overlayMenuSafeShutdown"))->isEnabled());
    QVERIFY(!menu.findChild<QAction*>(QStringLiteral("overlayMenuSafeRestart"))->isEnabled());
    QVERIFY(menu.findChild<QAction*>(QStringLiteral("overlayMenuExit"))->isEnabled());
  }

  void availabilityAndExecutionUseTheSameCapability_data() {
    QTest::addColumn<bool>("elevated");
    QTest::addColumn<bool>("available");
    QTest::addColumn<bool>("protectedFilter");
    for (const bool elevated : {false, true})
      for (const bool available : {false, true})
        for (const bool protectedFilter : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("elevated-%1-available-%2-protected-%3").arg(elevated).arg(available).arg(protectedFilter)))
              << elevated << available << protectedFilter;
  }
  void availabilityAndExecutionUseTheSameCapability() {
    QFETCH(bool, elevated);
    QFETCH(bool, available);
    QFETCH(bool, protectedFilter);
    MutableApplicationStateSource source;
    source.snapshot = snapshot(elevated, available, protectedFilter);
    auto store = std::make_shared<MemoryFileStagingStore>();
    auto models = std::make_shared<ViewModels>();
    auto workflows = std::make_shared<Workflows>(source, store);
    test::FileDialogStub files;
    ui::MainWindow window({models, workflows, files}, {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    QVERIFY(!models->owner->powerActionsAllowed());
    QCOMPARE(hubPowerAvailability(true), std::optional<bool>(false));
    QCOMPARE(workflows->confirmations, 0);

    window.startInTray();
    QTRY_VERIFY(models->owner->state().hasCommittedState());
    const bool expected = elevated && available;
    QCOMPARE(models->owner->powerActionsAllowed(), expected);
    QCOMPARE(hubPowerAvailability(true), std::optional<bool>(expected));
    auto* shutdown = command(window, QStringLiteral("Safe shutdown"));
    auto* restart = command(window, QStringLiteral("Safe restart"));
    QVERIFY(shutdown);
    QVERIFY(restart);
    QCOMPARE(shutdown->isEnabled(), expected);
    QCOMPARE(restart->isEnabled(), expected);

    QCOMPARE(workflows->confirmations, expected ? 2 : 0);
    QCOMPARE(workflows->warnings, 0);
    QVERIFY(workflows->backend.backend->trace().empty());
    QCOMPARE(store->writes, 0);
  }

  void activeOperationsBlockPowerButPendingDraftAndReconciliationDoNot() {
    auto gate = std::make_shared<test::WorkerGate>();
    const auto release = qScopeGuard([&] {
      gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    MutableApplicationStateSource source;
    source.snapshot = snapshot();
    auto store = std::make_shared<MemoryFileStagingStore>();
    auto models = std::make_shared<ViewModels>();
    auto workflows = std::make_shared<Workflows>(source, store);
    test::FileDialogStub files;
    ui::MainWindow window({models, workflows, files}, {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    window.startInTray();
    auto* model = models->owner;
    QTRY_VERIFY(model->state().hasCommittedState());
    auto* restart = command(window, QStringLiteral("Safe restart"));
    QVERIFY(restart);
    model->configuration()->global()->editFilterEnabled(false);
    QVERIFY(model->pendingChangeCount() > 0);
    QVERIFY(model->powerActionsAllowed());
    QCOMPARE(hubPowerAvailability(true), std::optional<bool>(true));
    QCOMPARE(workflows->confirmations, 2);

    core::PendingChanges changes;
    changes.setFilterEnabled = false;
    auto plan = std::make_shared<presentation::ApplyPlanViewModel>(changes, source.snapshot, std::make_shared<ApplyActions>(gate));
    model->trackApplyPlan(plan);
    QVERIFY(plan->canApply());
    plan->apply();
    QTRY_COMPARE(gate->started.load(), 1);
    QVERIFY(!model->powerActionsAllowed());
    QVERIFY(!restart->isEnabled());
    QCOMPARE(hubPowerAvailability(true), std::optional<bool>(false));
    QCOMPARE(workflows->confirmations, 2);
    gate->release();
    QTRY_VERIFY(!plan->applying());
    QVERIFY(model->powerActionsAllowed());
    QVERIFY(restart->isEnabled());
    QCOMPARE(hubPowerAvailability(), std::optional<bool>(true));

    bool inspectedActivePower = false;
    workflows->duringConfirmation = [&] {
      QVERIFY(!model->exitAllowed());
      QVERIFY(!model->powerActionsAllowed());
      QVERIFY(!restart->isEnabled());
      QCOMPARE(hubPowerAvailability(true), std::optional<bool>(false));
      QCOMPARE(workflows->confirmations, 3);
      inspectedActivePower = true;
    };
    model->safeRestart();
    workflows->duringConfirmation = {};
    QVERIFY(inspectedActivePower);
    QVERIFY(model->exitAllowed());
    QVERIFY(model->powerActionsAllowed());
    QVERIFY(restart->isEnabled());
    model->requireReconciliation();
    QVERIFY(!model->configurationWritesAllowed());
    QVERIFY(model->powerActionsAllowed());
    model->refresh();
    QVERIFY(model->refreshing());
    // The refresh now observes the saved staging list in the same worker.
    QVERIFY(!model->powerActionsAllowed());
    QTRY_VERIFY(!model->refreshing());
    QVERIFY(model->powerActionsAllowed());
    QVERIFY(workflows->backend.backend->trace().empty());
    QCOMPARE(workflows->warnings, 0);
    QCOMPARE(store->writes, 0);
  }

  void manualCommitLifecycleBlocksConflictingWindowActions_data() {
    QTest::addColumn<int>("operation");
    QTest::addColumn<bool>("fails");
    constexpr std::array names{"file", "file-deletion", "registry", "registry-deletion"};
    for (std::size_t operation = 0; operation < names.size(); ++operation)
      for (const bool fails : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2").arg(QString::fromLatin1(names[operation]), fails ? "exception" : "canceled")))
            << static_cast<int>(operation) << fails;
  }
  void manualCommitLifecycleBlocksConflictingWindowActions() {
    QFETCH(int, operation);
    QFETCH(bool, fails);
    MutableApplicationStateSource source;
    source.snapshot = snapshot();
    auto store = std::make_shared<MemoryFileStagingStore>();
    auto models = std::make_shared<ViewModels>();
    auto workflows = std::make_shared<Workflows>(source, store);
    test::FileDialogStub files;
    ui::MainWindow window({models, workflows, files}, {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    window.startInTray();
    auto* model = models->owner;
    QTRY_VERIFY(model->state().hasCommittedState() && !model->refreshing());
    QVERIFY(workflows->commitDispatcher);
    auto* shutdown = command(window, QStringLiteral("Safe shutdown"));
    auto* restart = command(window, QStringLiteral("Safe restart"));
    auto* importAction = command(window, QStringLiteral("Import"));
    auto* reviewAction = command(window, QStringLiteral("Review and apply"));
    QVERIFY(shutdown && restart && importAction && reviewAction);
    QVERIFY(model->configurationWritesAllowed() && model->configurationReviewAllowed() && model->maintenanceActionsAllowed());
    QVERIFY(model->powerActionsAllowed() && model->directRestartAllowed() && model->exitAllowed());

    // Empty selections must not acquire activity or publish transient busy state.
    QSignalSpy availability(model, &presentation::MainWindowViewModel::interactionAvailabilityChanged);
    window.commitFilePath({});
    window.commitFileDeletionPath({});
    window.commitRegistryKey({}, std::nullopt);
    window.commitRegistryDeletionKey({}, std::nullopt);
    workflows->commitDispatcher->commitFilePath({});
    workflows->commitDispatcher->commitFileDeletionPath({});
    workflows->commitDispatcher->commitRegistryKey({}, std::nullopt);
    workflows->commitDispatcher->commitRegistryDeletionKey({}, std::nullopt);
    QCOMPARE(workflows->commitInteractions, 0);
    QCOMPARE(availability.count(), 0);

    bool inspected = false;
    const int initialReads = source.reads;
    workflows->duringCommit = [&](const app::CommitRequest& request) {
      QCOMPARE(workflows->commitInteractions, 1);
      QCOMPARE(request.operation, operation % 2 == 0 ? app::CommitOperation::Persist : app::CommitOperation::DeleteAndPersist);
      QCOMPARE(request.target.kind, operation < 2 ? app::CommitTargetKind::File : app::CommitTargetKind::RegistryKey);
      const auto view = model->interactionState();
      QVERIFY(!view.refreshEnabled && !view.configurationEnabled && !view.reviewEnabled && !view.powerEnabled && !view.exitEnabled);
      QVERIFY(!model->maintenanceActionsAllowed() && !model->directRestartAllowed());
      QVERIFY(!shutdown->isEnabled() && !restart->isEnabled() && !importAction->isEnabled() && !reviewAction->isEnabled());
      std::optional<bool> hubExitAllowed;
      QCOMPARE(hubPowerAvailability(true, &hubExitAllowed), std::optional<bool>{false});
      QCOMPARE(hubExitAllowed, std::optional<bool>{false});
      model->safeShutdown();
      model->safeRestart();
      model->directRestart();
      QCOMPARE(workflows->confirmations, 0);
      QVERIFY(!model->reviewConfiguration());
      QVERIFY(!model->importModel());
      window.refresh();
      model->refresh();
      QCOMPARE(source.reads, initialReads);
      QVERIFY(!model->refreshing());
      model->configuration()->global()->editFilterEnabled(false);
      QCOMPARE(model->pendingChangeCount(), std::size_t{0});
      window.commitFilePath(QStringLiteral("C:\\blocked.txt"));
      workflows->commitDispatcher->commitFilePath(QStringLiteral("C:\\reentrant.txt"));
      QCOMPARE(workflows->commitInteractions, 1);
      // A different activity source cannot clear the manual operation's gate.
      model->setExternalOperationActive(true);
      model->setExternalOperationActive(false);
      QVERIFY(!model->powerActionsAllowed() && !model->exitAllowed());
      inspected = true;
      if (fails) throw std::runtime_error("commit interaction failed");
      // Returning without preparing/executing a target models cancellation.
    };
    const auto dispatch = [&] {
      switch (operation) {
        case 0:
          window.commitFilePath(QStringLiteral("C:\\selected.txt"));
          break;
        case 1:
          window.commitFileDeletionPath(QStringLiteral("C:\\selected.txt"));
          break;
        case 2:
          window.commitRegistryKey(QStringLiteral("HKLM\\SOFTWARE\\Selected"), std::nullopt);
          break;
        case 3:
          window.commitRegistryDeletionKey(QStringLiteral("HKLM\\SOFTWARE\\Selected"), std::nullopt);
          break;
      }
    };
    if (fails)
      QVERIFY_THROWS_EXCEPTION(std::runtime_error, dispatch());
    else
      dispatch();
    workflows->duringCommit = {};
    QVERIFY(inspected);
    QCOMPARE(workflows->commitInteractions, 1);
    QVERIFY(model->configurationWritesAllowed() && model->configurationReviewAllowed() && model->maintenanceActionsAllowed());
    QVERIFY(model->powerActionsAllowed() && model->directRestartAllowed() && model->exitAllowed());
    QVERIFY(model->interactionState().refreshEnabled);
    QVERIFY(shutdown->isEnabled() && restart->isEnabled() && importAction->isEnabled() && reviewAction->isEnabled());
    QCOMPARE(hubPowerAvailability(), std::optional<bool>{true});
    model->safeRestart();
    QCOMPARE(workflows->confirmations, 1);
    QVERIFY(model->exitAllowed() && model->powerActionsAllowed());
    QVERIFY(workflows->backend.backend->trace().empty());
    QCOMPARE(workflows->warnings, 0);
    QCOMPARE(store->writes, 0);
  }

  void configurationActionsKeepStateAndHintsAcrossLanguageRoundTrips_data() {
    QTest::addColumn<int>("condition");
    QTest::addColumn<bool>("initialChinese");
    QTest::addColumn<QString>("restriction");
    struct Scenario {
      ConfigurationCondition condition;
      const char* name;
      const char* restriction;
    };
    const std::array scenarios{
        Scenario{ConfigurationCondition::Ready, "ready", ""},
        Scenario{ConfigurationCondition::Horm, "horm", "UWF settings cannot be changed while HORM is active."},
        Scenario{ConfigurationCondition::ReadOnly, "read-only", "Administrator privileges are required; the overlay configuration is read-only."},
        Scenario{ConfigurationCondition::Unavailable, "unavailable", "UWF is currently unavailable"},
        Scenario{ConfigurationCondition::Reconciliation, "reconciliation",
                 "Some changes may already have been written. Refresh the system state before retrying."},
        Scenario{ConfigurationCondition::ExternalOperation, "external-operation", "Please wait for the operation to finish."},
        Scenario{ConfigurationCondition::Refreshing, "refreshing", "Please wait for the operation to finish."},
        Scenario{ConfigurationCondition::Applying, "applying", "Please wait for the operation to finish."}};
    for (const auto& scenario : scenarios)
      for (const bool initialChinese : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString::fromLatin1(scenario.name), initialChinese ? "zh-en-zh" : "en-zh-en")))
            << static_cast<int>(scenario.condition) << initialChinese << QString::fromLatin1(scenario.restriction);
  }
  void configurationActionsKeepStateAndHintsAcrossLanguageRoundTrips() {
    QFETCH(int, condition);
    QFETCH(bool, initialChinese);
    QFETCH(QString, restriction);
    const auto scenario = static_cast<ConfigurationCondition>(condition);
    auto& i18n = ui::I18n::instance();
    const auto previousLanguage = i18n.lang();
    const auto restoreLanguage = qScopeGuard([&] { i18n.setLang(previousLanguage); });
    const auto initialLanguage = initialChinese ? ui::I18n::Lang::Zh_CN : ui::I18n::Lang::En;
    const auto otherLanguage = initialChinese ? ui::I18n::Lang::En : ui::I18n::Lang::Zh_CN;
    i18n.setLang(initialLanguage);
    QCOMPARE(i18n.lang(), initialLanguage);
    auto gate = std::make_shared<test::WorkerGate>();
    const auto release = qScopeGuard([&] {
      gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    ObservationSource source;
    source.observation.snapshot = snapshot();
    source.observation.hormEnabled = false;
    auto store = std::make_shared<MemoryFileStagingStore>();
    auto models = std::make_shared<ViewModels>();
    auto workflows = std::make_shared<Workflows>(source, store);
    test::FileDialogStub files;
    ui::MainWindow window({models, workflows, files}, {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    window.startInTray();
    auto* model = models->owner;
    QTRY_VERIFY(model->state().hasCommittedState() && !model->refreshing());
    auto* importAction = command(window, ui::I18n::tr("Import"));
    auto* reviewAction = command(window, ui::I18n::tr("Review and apply"));
    QVERIFY(importAction && reviewAction);
    const auto inspectRoundTrip = [&](const bool allowed, const QString& reason) {
      QStringList firstHints;
      for (const auto language : {initialLanguage, otherLanguage, initialLanguage}) {
        i18n.setLang(language);
        QCOMPARE(i18n.lang(), language);
        const auto view = model->interactionState();
        QCOMPARE(view.configurationEnabled, allowed);
        QCOMPARE(importAction->isEnabled(), allowed);
        QCOMPARE(reviewAction->isEnabled(), allowed);
        const auto translatedReason = reason.isEmpty() ? QString{} : QCoreApplication::translate("uwf", qUtf8Printable(reason));
        const auto importHint = allowed
                                    ? ui::I18n::tr("Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.")
                                    : translatedReason;
        const auto reviewHint = allowed ? ui::I18n::tr("Review all pending changes and apply them in one batch.") : translatedReason;
        QVERIFY(!importHint.isEmpty() && !reviewHint.isEmpty());
        QCOMPARE(view.configurationHint, translatedReason);
        QCOMPARE(view.importHint, importHint);
        QCOMPARE(view.reviewHint, reviewHint);
        QCOMPARE(importAction->toolTip(), importHint);
        QCOMPARE(reviewAction->toolTip(), reviewHint);
        const QStringList hints{importAction->toolTip(), reviewAction->toolTip()};
        if (firstHints.isEmpty())
          firstHints = hints;
        else if (language == initialLanguage)
          QCOMPARE(hints, firstHints);
        else
          QVERIFY(hints != firstHints);
      }
    };
    inspectRoundTrip(true, {});
    std::shared_ptr<presentation::ApplyPlanViewModel> plan;
    switch (scenario) {
      case ConfigurationCondition::Horm:
        source.observation.hormEnabled = true;
        break;
      case ConfigurationCondition::ReadOnly:
        source.observation.snapshot.elevated = false;
        break;
      case ConfigurationCondition::Unavailable:
        source.observation.snapshot.uwfAvailable = false;
        break;
      case ConfigurationCondition::Reconciliation:
        model->requireReconciliation();
        break;
      case ConfigurationCondition::ExternalOperation:
        model->setExternalOperationActive(true);
        break;
      case ConfigurationCondition::Refreshing:
        source.gate = gate;
        model->refresh();
        QTRY_COMPARE(gate->started.load(), 1);
        QVERIFY(model->refreshing());
        break;
      case ConfigurationCondition::Applying: {
        core::PendingChanges changes;
        changes.setFilterEnabled = false;
        plan = std::make_shared<presentation::ApplyPlanViewModel>(changes, source.observation.snapshot, std::make_shared<ApplyActions>(gate));
        model->trackApplyPlan(plan);
        QVERIFY(plan->canApply());
        plan->apply();
        QTRY_COMPARE(gate->started.load(), 1);
        QVERIFY(model->applyingConfiguration());
        break;
      }
      case ConfigurationCondition::Ready:
        break;
    }
    if (scenario == ConfigurationCondition::Horm || scenario == ConfigurationCondition::ReadOnly || scenario == ConfigurationCondition::Unavailable) {
      model->refresh();
      QTRY_VERIFY(!model->refreshing());
    }
    inspectRoundTrip(scenario == ConfigurationCondition::Ready, restriction);
    source.observation.snapshot = snapshot();
    source.observation.hormEnabled = false;
    source.gate.reset();
    model->setExternalOperationActive(false);
    gate->release();
    if (plan) QTRY_VERIFY(!plan->applying());
    model->refresh();
    QTRY_VERIFY(!model->refreshing());
    inspectRoundTrip(true, {});
    QVERIFY(workflows->backend.backend->trace().empty());
    QCOMPARE(workflows->confirmations, 0);
    QCOMPARE(workflows->warnings, 0);
    QCOMPARE(store->writes, 0);
  }
};
}  // namespace

QTEST_MAIN(PowerAvailabilityTests)
#include "PowerAvailabilityTests.moc"
