// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QScopeGuard>
#include <QTest>
#include <QThreadPool>
#include <QTimer>
#include <optional>
#include <stdexcept>

#include "app/ports/SystemInformationSource.h"
#include "presentation/common/ViewModelFactory.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/RecordingWmiOperations.h"
#include "support/app/WorkerGate.h"
#include "support/ui/FileDialogStub.h"
#include "ui/appearance/I18n.h"
#include "ui/overlay/OverlayContextMenu.h"
#include "ui/overlay/OverlayFloatingWidget.h"
#include "ui/shell/MainWindow.h"
#include "ui/shell/WindowWorkflowFactory.h"

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

// Only the main-window composition is exercised. Unexpected dialog creation
// fails the test instead of falling through to a native platform service.
class ViewModels final : public presentation::ViewModelFactory {
 public:
  std::unique_ptr<presentation::MainWindowViewModel> mainWindow(app::ApplicationStateSource& source, app::UwfCapability capability,
                                                               std::shared_ptr<app::FileStagingDraft> staging) override {
    auto model = std::make_unique<presentation::MainWindowViewModel>(source, capability, std::move(staging));
    owner = model.get();
    return model;
  }
  std::shared_ptr<presentation::SystemInformationViewModel> systemInformation() override {
    return std::make_shared<presentation::SystemInformationViewModel>(information);
  }
  std::shared_ptr<presentation::OverlayViewModel> overlay() override {
    return std::make_shared<presentation::OverlayViewModel>(std::make_shared<UsageReader>());
  }
  std::shared_ptr<presentation::ImportViewModel> importCommands() override { throw std::logic_error("unexpected import dialog"); }
  std::shared_ptr<presentation::LogViewModel> log() override { throw std::logic_error("unexpected log dialog"); }
  std::shared_ptr<presentation::UserConfigExportViewModel> userConfigExport() override { throw std::logic_error("unexpected export dialog"); }
  std::shared_ptr<presentation::FileStagingViewModel> fileStaging(QString, app::FileStagingDraft&, app::FileStagingConflictPolicy*) override {
    throw std::logic_error("unexpected staging dialog");
  }
  std::shared_ptr<presentation::ApplyPlanViewModel> applyPlan(core::PendingChanges, const core::UwfSnapshot&,
                                                            std::shared_ptr<app::FileStagingDraft>) override {
    throw std::logic_error("unexpected apply dialog");
  }
  std::shared_ptr<presentation::EnhancedModeViewModel> enhancedMode(app::EnhancedModeActions&) override {
    throw std::logic_error("unexpected enhanced-mode dialog");
  }
  std::shared_ptr<presentation::OverlayFilesViewModel> overlayFiles(const QString&) override {
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
  explicit Workflows(std::shared_ptr<MemoryFileStagingStore> store) : store(std::move(store)) {}
  std::unique_ptr<app::EnhancedModeSession> enhancedMode(app::UwfCapability) override { return {}; }
  std::unique_ptr<app::FileStagingCoordinator> fileStaging(app::UwfCapability, QWidget*) override {
    return std::make_unique<app::FileStagingCoordinator>([](auto, auto) -> std::unique_ptr<app::FileStagingCommandOperation> {
      throw std::logic_error("unexpected staging execution");
    });
  }
  std::unique_ptr<app::PowerCoordinator> power(app::UwfCapability capability, app::FileStagingCoordinator& staging,
                                               app::PreshutdownCommitControl preshutdown, QWidget*) override {
    app::PowerInteraction interaction;
    interaction.confirm = [this](app::PowerActionDialogRequest) {
      ++confirmations;
      return app::PowerActionDialogOutcome::Canceled;
    };
    interaction.warning = [this](const QString&, const QString&) { ++warnings; };
    interaction.describeStagingResult = [](const app::FileStagingCommitResult&) { return QString{}; };
    return std::make_unique<app::PowerCoordinator>(app::PowerServices{wmi, *store, capability, staging, {}, std::move(preshutdown)},
                                                 std::move(interaction));
  }
  std::unique_ptr<ui::CommitDispatcher> commit(const core::UwfSnapshot&, QTimer*, QWidget*) override { return {}; }
  std::shared_ptr<MemoryFileStagingStore> store;
  RecordingWmiOperations wmi;
  int confirmations = 0;
  int warnings = 0;
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

QAction* command(ui::MainWindow& window, const QString& text) {
  const auto* commands = window.findChild<QWidget*>(QStringLiteral("mainCommands"));
  if (commands)
    for (const auto actions = commands->findChildren<QAction*>(); auto* action : actions)
      if (action->text() == text) return action;
  return nullptr;
}

std::optional<bool> hubPowerAvailability(bool requestPower = false) {
  ui::OverlayFloatingWidget* hub = nullptr;
  for (const auto widgets = QApplication::allWidgets(); auto* widget : widgets)
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
    if (shutdown && restart && shutdown->isEnabled() == restart->isEnabled()) allowed = restart->isEnabled();
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
    auto workflows = std::make_shared<Workflows>(store);
    test::FileDialogStub files;
    ui::MainWindow window({source, store, models, workflows, files}, {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
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
    QVERIFY(workflows->wmi.invocations.empty());
    QCOMPARE(store->writes, 0);
  }

  void applyingBlocksPowerButPendingDraftAndReconciliationDoNot() {
    auto gate = std::make_shared<test::WorkerGate>();
    const auto release = qScopeGuard([&] {
      gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    MutableApplicationStateSource source;
    source.snapshot = snapshot();
    auto store = std::make_shared<MemoryFileStagingStore>();
    auto models = std::make_shared<ViewModels>();
    auto workflows = std::make_shared<Workflows>(store);
    test::FileDialogStub files;
    ui::MainWindow window({source, store, models, workflows, files}, {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
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

    model->setExternalOperationActive(true);
    QVERIFY(!model->exitAllowed());
    QVERIFY(model->powerActionsAllowed());
    model->setExternalOperationActive(false);
    model->requireReconciliation();
    QVERIFY(!model->configurationWritesAllowed());
    QVERIFY(model->powerActionsAllowed());
    model->refresh();
    QVERIFY(model->refreshing());
    QVERIFY(model->powerActionsAllowed());
    QTRY_VERIFY(!model->refreshing());
    QVERIFY(workflows->wmi.invocations.empty());
    QCOMPARE(workflows->warnings, 0);
    QCOMPARE(store->writes, 0);
  }
};
}  // namespace

QTEST_MAIN(PowerAvailabilityTests)
#include "PowerAvailabilityTests.moc"
