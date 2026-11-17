// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QApplication>
#include <QContextMenuEvent>
#include <QDialog>
#include <QElapsedTimer>
#include <QLabel>
#include <QListView>
#include <QPointer>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTest>
#include <QThreadPool>
#include <QTimer>
#include <QToolButton>
#include <QWindow>
#include <algorithm>
#include <atomic>
#include <utility>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/exclusions/ExclusionEditor.h"
#include "application/operations/contracts/OperationError.h"
#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/UwfStateUpdates.h"
#include "application/uwf/adapters/UwfApplicationStateSource.h"
#include "cases/presentation/views/application/ApplicationUiFixture.h"
#include "composition/desktop/MainWindowComposition.h"
#include "presentation/appearance/I18n.h"
#include "presentation/controls/feedback/BusyIndicator.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/feedback/ContentState.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/inputs/CodeEditor.h"
#include "presentation/controls/inputs/SwitchButton.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/viewmodels/shell/MainWindowViewModel.h"
#include "presentation/views/configuration/global/GlobalStatusPanel.h"
#include "presentation/views/configuration/review/ApplyPlanDialog.h"
#include "presentation/views/configuration/volumes/VolumeTab.h"
#include "presentation/views/overlay/coordination/OverlayContextMenu.h"
#include "presentation/views/overlay/coordination/OverlayHub.h"
#include "presentation/views/overlay/coordination/OverlayHubView.h"
#include "presentation/views/shell/MainWindow.h"
#include "presentation/views/staging/FileStagingWidget.h"
#include "simulation/app/ConfigurationState.h"
#include "simulation/app/MemoryFileStagingStore.h"
#include "simulation/ui/FileDialogStub.h"
#include "simulation/uwf/FakeUwfBackend.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/WorkerGate.h"
#include "support/ui/DesktopPlatformFixture.h"
#include "support/ui/DialogControls.h"
#include "support/uwf/FakeUwfTrace.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

class MenuEndpoint final : public ui::OverlayHubView {
 public:
  MenuEndpoint() : menu(this) { connectContextMenu(menu); }
  int priority() const override { return 0; }
  VerificationResult verifyPresentation() const override { return VerificationResult::Invalid; }
  ui::OverlayContextMenu menu;
};

class EmptyVolumes final : public app::VolumeRepository {
 public:
  std::vector<core::VolumeInfo> read() const override { return {}; }
  std::string driveForPath(const std::string&) const override { return {}; }
  std::vector<std::string> mountedPaths(const std::string&) const override { return {}; }
  std::vector<std::string> mountedLetters(const std::string&) const override { return {}; }
  std::string systemDrive() const override { return "C:"; }
};

// Hold the filter command in flight while the production application workflow owns its interaction gates.
class GatedFilter final : public compat::Uwf {
 public:
  GatedFilter(std::shared_ptr<uwf::simulation::FakeUwfBackend> backend, std::shared_ptr<test::WorkerGate> gate)
      : m_backend(std::move(backend)), m_gate(std::move(gate)) {}
  compat::Capability probe(const compat::Request& request) const override { return m_backend->probe(request); }
  compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
    if (request == compat::Request{compat::SetBoolean{compat::BooleanProperty::FilterEnabled, false}}) m_gate->wait({}, false);
    return m_backend->execute(request, context);
  }
  compat::UwfStateSnapshot stateSnapshot() const override { return m_backend->stateSnapshot(); }
  void updateState(compat::UwfStateFacts state) const override { m_backend->updateState(std::move(state)); }
  void synchronize(const std::function<void()>& work) const override { m_backend->synchronize(work); }
  compat::Response refreshState(const compat::Context& context) const override { return m_backend->refreshState(context); }

 private:
  std::shared_ptr<uwf::simulation::FakeUwfBackend> m_backend;
  std::shared_ptr<test::WorkerGate> m_gate;
};

class ConfirmationActions final : public app::ConfigurationActions {
 public:
  std::shared_ptr<test::WorkerGate> gate = std::make_shared<test::WorkerGate>();
  int failure = 0;
  int prepares = 0;
  int completions = 0;
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
    ++prepares;
    if (failure == 1) throw std::runtime_error("injected preparation failure");
    return [wait = gate, fail = failure] {
      wait->wait({}, false);
      if (fail == 2) throw std::runtime_error("injected worker failure");
      app::ConfigurationApplyResult result;
      result.steps.push_back({"Filter", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::None,
                              app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}});
      return result;
    };
  }
  void completeApply(const app::ConfigurationApplyResult&) override { ++completions; }
};

class ConfigurationLifecycleUiTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() { ui::I18n::instance().setLang(ui::I18n::Lang::En); }

  void hormPreviewExportsTheSelectedScopeWithoutExecuting_data() {
    QTest::addColumn<bool>("enable");
    QTest::newRow("disable-HORM") << false;
    QTest::newRow("enable-HORM") << true;
  }
  void hormPreviewExportsTheSelectedScopeWithoutExecuting() {
    QFETCH(bool, enable);
    app::ConfigurationPlan plan;
    plan.snapshot = uwf::simulation::application::editableSnapshot();
    plan.snapshot.current.overlay.type = core::OverlayType::RAM;
    plan.snapshot.runtime.currentOverlayFlags = app::OverlayFlags::ReadOnlyMedia;
    plan.snapshot.runtime.hormEnabled = !enable;
    plan.snapshot.runtime.romDisposition = enable ? 0u : 2u;
    plan.maintenance.hormEnabled = enable;
    const auto actions = std::make_shared<ConfirmationActions>();
    QString saved;
    int exports = 0;
    const auto model = std::make_shared<presentation::ApplyPlanViewModel>(plan, actions, [&](const QString&, const QString& contents) {
      ++exports;
      saved = contents;
      return app::TextFileWriteResult{};
    });
    QVERIFY(model->canApply());
    QCOMPARE(model->changeCount(), std::size_t{1});
    QCOMPARE(model->changeCommandCount(), std::size_t{1});
    uwf::simulation::FileDialogStub files;
    files.savedFile = QStringLiteral("horm-scope.cmd");
    ui::ApplyPlanDialog dialog(model, files);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* tabs = dialog.findChild<QTabWidget*>(QStringLiteral("reviewTabs"));
    auto* disclosure = dialog.findChild<ui::Disclosure*>(QStringLiteral("changeCommandsDisclosure"));
    auto* preview = dialog.findChild<ui::CodeEditor*>(QStringLiteral("changeCommandPreview"));
    auto* exportButton = buttonWithText(&dialog, QStringLiteral("Export commands"));
    QVERIFY(tabs);
    QVERIFY(disclosure);
    QVERIFY(preview);
    QVERIFY(exportButton);
    QCOMPARE(tabs->currentIndex(), 0);
    disclosure->setOpen(true);
    const auto native = enable ? QStringLiteral("uwfmgr.exe filter enable-HORM") : QStringLiteral("uwfmgr.exe filter disable-HORM");
    QVERIFY(preview->toPlainText().contains(native));
    QVERIFY(!preview->toPlainText().contains(QStringLiteral("not included in command export")));
    QVERIFY(exportButton->isEnabled());
    const auto exportCurrentTab = [&] {
      bool completed = false;
      const bool driven = test::driveModalDialog(
          dialog, [&] { QTest::mouseClick(exportButton, Qt::LeftButton); },
          [&](QDialog& information) {
            QCOMPARE(information.windowTitle(), QStringLiteral("Export finished"));
            completed = true;
            information.accept();
          });
      return driven && completed;
    };
    QVERIFY(exportCurrentTab());
    QCOMPARE(exports, 1);
    auto parsed = app::parseUwfmgrText(saved.toStdString());
    std::erase_if(parsed, [](const auto& command) { return command.parseError == app::ParseError::Comment; });
    QCOMPARE(parsed.size(), std::size_t{1});
    QCOMPARE(parsed.front().kind, enable ? app::UwfmgrKind::FilterEnableHorm : app::UwfmgrKind::FilterDisableHorm);
    QCOMPARE(parsed.front().parseError, app::ParseError::None);
    tabs->setCurrentIndex(1);
    QTRY_VERIFY(exportButton->isEnabled());
    QVERIFY(exportCurrentTab());
    QCOMPARE(exports, 2);
    for (const auto& command : app::parseUwfmgrText(saved.toStdString()))
      QVERIFY(command.kind != app::UwfmgrKind::FilterEnableHorm && command.kind != app::UwfmgrKind::FilterDisableHorm &&
              command.kind != app::UwfmgrKind::OverlayCommit);
    QCOMPARE(actions->prepares, 0);
    QCOMPARE(actions->completions, 0);
    QVERIFY(!model->applying());
  }

  void applyConfirmationOwnsResponsiveProgressUntilCompletion_data() {
    QTest::addColumn<int>("failure");
    QTest::addColumn<bool>("longRunning");
    QTest::newRow("short-success") << 0 << false;
    QTest::newRow("preparation-failure") << 1 << false;
    QTest::newRow("worker-failure") << 2 << false;
    QTest::newRow("long-success") << 0 << true;
  }
  void applyConfirmationOwnsResponsiveProgressUntilCompletion() {
    QFETCH(int, failure);
    QFETCH(bool, longRunning);
    const auto actions = std::make_shared<ConfirmationActions>();
    actions->failure = failure;
    const auto unblock = qScopeGuard([&] { actions->gate->release(); });
    if (!longRunning) actions->gate->release();
    core::PendingChanges changes;
    changes.setFilterEnabled = false;
    const auto model = std::make_shared<presentation::ApplyPlanViewModel>(changes, uwf::simulation::application::editableSnapshot(), actions);
    uwf::simulation::FileDialogStub files;
    ui::ApplyPlanDialog review(model, files);
    review.show();
    QVERIFY(QTest::qWaitForWindowExposed(&review));
    auto* apply = review.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
    QVERIFY(apply && apply->isEnabled());
    apply->click();
    QPointer<ui::DialogWindow> confirmation = review.findChild<ui::DialogWindow*>(QStringLiteral("applyExecutionConfirmation"));
    QVERIFY(confirmation);
    QVERIFY(confirmation->isVisible());
    QCOMPARE(actions->prepares, 0);  // Opening or dismissing a review cannot write.
    auto* spinner = confirmation->findChild<ui::BusyIndicator*>(QStringLiteral("applyExecutionSpinner"));
    auto* elapsed = confirmation->findChild<ui::TextLabel*>(QStringLiteral("applyExecutionElapsed"));
    auto* confirm = confirmation->findChild<QPushButton*>(QStringLiteral("confirmApplyButton"));
    QVERIFY(spinner && elapsed && confirm);
    QVERIFY(!spinner->isVisibleTo(confirmation));
    QElapsedTimer duration;
    qint64 finishedAt = -1;
    duration.start();
    connect(confirmation.data(), &QDialog::finished, &review, [&] { finishedAt = duration.elapsed(); });
    confirmation->accept();
    QCOMPARE(actions->prepares, 1);  // No one-second delay before real work begins.
    QVERIFY(confirmation && confirmation->isVisible());
    QVERIFY(spinner->isVisibleTo(confirmation));
    QVERIFY(!confirm->isEnabled());
    QVERIFY(!confirmation->rejectionEnabled());
    QVERIFY(!review.rejectionEnabled());
    confirmation->accept();
    confirmation->reject();
    review.reject();
    apply->click();
    QCOMPARE(actions->prepares, 1);
    QVERIFY(confirmation->isVisible());
    QVERIFY(review.isVisible());
    int ticks = 0;
    QTimer pulse;
    pulse.setInterval(20);
    connect(&pulse, &QTimer::timeout, &review, [&] { ++ticks; });
    pulse.start();
    QTRY_VERIFY(ticks >= 3);
    QVERIFY(elapsed->text().contains(QStringLiteral("Elapsed:")));
    if (longRunning) {
      QTRY_COMPARE(actions->gate->started.load(), 1);
      QTest::qWait(1100);
      QVERIFY(model->applying());
      QVERIFY(confirmation && confirmation->isVisible());
      const auto releasedAt = duration.elapsed();
      actions->gate->release();
      QTRY_VERIFY_WITH_TIMEOUT(finishedAt >= 0, 900);
      QVERIFY(finishedAt - releasedAt < 1000);  // A long operation gets no additional fixed hold.
    } else {
      QTRY_VERIFY(!model->applying());
      if (duration.elapsed() < 1000) QVERIFY(confirmation && confirmation->isVisible());
      QTRY_VERIFY_WITH_TIMEOUT(finishedAt >= 0, 3000);
    }
    QVERIFY(finishedAt >= 1000);
    QTRY_VERIFY(!confirmation || !confirmation->isVisible());
    QCOMPARE(actions->completions, 1);
    QCOMPARE(model->resultFailed(), failure != 0);
    QVERIFY(review.rejectionEnabled());
    auto* result = review.findChild<ui::Notice*>(QStringLiteral("applyResultNotice"));
    QVERIFY(result && result->isVisibleTo(&review));
    QVERIFY(files.requests.empty());
  }

  void applyConfirmationCancellationAndOwnerDestructionDoNotReplayWork() {
    const auto actions = std::make_shared<ConfirmationActions>();
    const auto unblock = qScopeGuard([&] { actions->gate->release(); });
    core::PendingChanges changes;
    changes.setFilterEnabled = false;
    const auto model = std::make_shared<presentation::ApplyPlanViewModel>(changes, uwf::simulation::application::editableSnapshot(), actions);
    uwf::simulation::FileDialogStub files;
    auto review = std::make_unique<ui::ApplyPlanDialog>(model, files);
    review->show();
    QVERIFY(QTest::qWaitForWindowExposed(review.get()));
    auto* apply = review->findChild<QPushButton*>(QStringLiteral("primaryBtn"));
    QVERIFY(apply);
    apply->click();
    QPointer<ui::DialogWindow> confirmation = review->findChild<ui::DialogWindow*>(QStringLiteral("applyExecutionConfirmation"));
    QVERIFY(confirmation);
    confirmation->reject();
    QCOMPARE(actions->prepares, 0);
    QVERIFY(review->rejectionEnabled());
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    apply->click();
    confirmation = review->findChild<ui::DialogWindow*>(QStringLiteral("applyExecutionConfirmation"));
    QVERIFY(confirmation);
    confirmation->accept();
    QTRY_COMPARE(actions->gate->started.load(), 1);
    review.reset();
    QVERIFY(!confirmation);
    QVERIFY(model->applying());
    actions->gate->release();
    QTRY_VERIFY(!model->applying());
    QCOMPARE(actions->prepares, 1);
    QCOMPARE(actions->completions, 1);
  }

  void renderingOnlyReadsTheEditingContext_data() {
    QTest::addColumn<bool>("allowed");
    QTest::addColumn<bool>("limited");
    for (const bool allowed : {false, true})
      for (const bool limited : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("allowed-%1-limited-%2").arg(allowed).arg(limited))) << allowed << limited;
  }
  void renderingOnlyReadsTheEditingContext() {
    QFETCH(bool, allowed);
    QFETCH(bool, limited);
    presentation::ConfigurationViewModel configuration(16384);
    const core::VolumeInfo physical{"C:",
                                    uwf::simulation::application::fixtureVolumeCName,
                                    limited ? "exFAT" : "NTFS",
                                    "System",
                                    1000,
                                    500,
                                    limited ? core::VolumeSupport::FileSystemLimited : core::VolumeSupport::Supported};
    const auto volume = configuration.addVolume(physical, true);
    configuration.setEditingAllowed(allowed);
    configuration.resetSnapshot(uwf::simulation::application::editableSnapshot());
    QSignalSpy controls(volume->status().get(), &presentation::VolumeStatusViewModel::availabilityChanged);
    QSignalSpy files(volume->files().get(), &presentation::ExclusionListViewModel::editingAvailabilityChanged);
    QSignalSpy registry(volume->registry().get(), &presentation::ExclusionListViewModel::editingAvailabilityChanged);
    uwf::simulation::FileDialogStub dialogs;
    ui::VolumeTab tab(volume, dialogs, {}, {});
    auto* protection = tab.findChild<ui::SwitchButton*>(QStringLiteral("volumeProtectionNext"));
    QVERIFY(protection);
    QCOMPARE(protection->isEnabled(), allowed);
    for (const auto language : {ui::I18n::Lang::Zh_CN, ui::I18n::Lang::En}) {
      ui::I18n::instance().setLang(language);
      QCOMPARE(protection->isEnabled(), allowed);
      QCOMPARE(volume->status()->controlsEnabled(), allowed);
      QCOMPARE(volume->files()->readOnly(), !allowed || limited);
      QCOMPARE(volume->registry()->readOnly(), !allowed);
    }
    QCOMPARE(controls.count(), 0);
    QCOMPARE(files.count(), 0);
    QCOMPARE(registry.count(), 0);
    QVERIFY(dialogs.requests.empty());
  }

  void failedRefreshDisablesPowerUntilSuccessfulRead_data() {
    QTest::addColumn<bool>("withVolumes");
    QTest::addColumn<bool>("restart");
    for (const bool volumes : {false, true})
      for (const bool restart : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(volumes ? "multiple-volumes" : "empty-workspace", restart ? "restart" : "shutdown")))
            << volumes << restart;
  }
  void failedRefreshDisablesPowerUntilSuccessfulRead() {
    QFETCH(bool, withVolumes);
    QFETCH(bool, restart);
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>();
    const auto store = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
    MutableApplicationStateSource source;
    source.snapshot = uwf::simulation::application::editableSnapshot();
    if (withVolumes)
      source.volumes = {{"C:", uwf::simulation::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported},
                        {"D:", uwf::simulation::application::fixtureVolumeDName, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported}};
    auto services = composition::mainWindowServices(system, source, store, test::localDesktopPorts());
    const auto workflows = std::make_shared<CapturingWindowWorkflows>(services.workflows);
    services.workflows = workflows;
    ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const auto model = workflows->observed;
    QVERIFY(model);
    QTRY_VERIFY(model->state().hasCommittedState() && !model->refreshing());
    QTRY_VERIFY(!model->overlayMaintenance()->busy());
    QVERIFY(!model->workspaceDiagnostic());
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCOMPARE(window.findChildren<ui::VolumeTab*>().size(), withVolumes ? 2 : 0);
    const auto verifyWorkspace = [&](const bool diagnostic) {
      const auto states =
          window.findChildren<ui::ContentState*>(withVolumes ? QStringLiteral("volumeUnavailableState") : QStringLiteral("mainUnavailableState"));
      QVERIFY(!states.isEmpty());
      for (auto* state : states) {
        const auto* host = qobject_cast<QStackedWidget*>(state->parentWidget());
        QVERIFY(host);
        QCOMPARE(host->currentWidget() == state, diagnostic);
      }
      for (auto tabs = window.findChildren<ui::VolumeTab*>(); auto* tab : std::as_const(tabs)) {
        const auto* protection = tab->findChild<ui::SwitchButton*>(QStringLiteral("volumeProtectionNext"));
        QVERIFY(protection);
        if (diagnostic) QVERIFY(!protection->isEnabled());
      }
    };
    verifyWorkspace(false);
    source.failure = QStringLiteral("Provider temporarily unavailable");
    window.refresh();
    QTRY_VERIFY(!model->refreshing());
    QCOMPARE(source.reads, 2);
    QVERIFY(model->workspaceDiagnostic());
    verifyWorkspace(true);
    // A retained display snapshot must not authorize another UWF operation.
    auto* power = actionWithText(&window, restart ? ui::I18n::tr("Safe restart") : ui::I18n::tr("Safe shutdown"));
    QVERIFY(power && !power->isEnabled());
    power->trigger();
    model->safeRestart();
    model->safeShutdown();
    model->directRestart();
    QVERIFY(!QApplication::activeModalWidget());
    QCOMPARE(source.reads, 2);
    QVERIFY(!model->configurationWritesAllowed());
    QVERIFY(model->workspaceDiagnostic());
    verifyWorkspace(true);
    for (const auto language : {ui::I18n::Lang::Zh_CN, ui::I18n::Lang::En}) {
      ui::I18n::instance().setLang(language);
      QVERIFY(model->workspaceDiagnostic());
      verifyWorkspace(true);
    }
    source.failure.clear();
    window.refresh();
    QTRY_VERIFY(!model->refreshing());
    QCOMPARE(source.reads, 3);
    QVERIFY(!model->workspaceDiagnostic());
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    verifyWorkspace(false);
    power = actionWithText(&window, restart ? ui::I18n::tr("Safe restart") : ui::I18n::tr("Safe shutdown"));
    QVERIFY(power && power->isEnabled());
    QCOMPARE(store->writes, 0);
    QVERIFY(test::writeRequests(*system).empty());
  }

  void thresholdOnlyReviewAndConfirmationUseTheActualActivationTime_data() {
    QTest::addColumn<bool>("filterEnabled");
    QTest::addColumn<QString>("expectedTiming");
    QTest::newRow("filter-enabled") << true << QStringLiteral("Takes effect when applied");
    QTest::newRow("filter-disabled") << false << QStringLiteral("Used when UWF starts");
  }
  void thresholdOnlyReviewAndConfirmationUseTheActualActivationTime() {
    QFETCH(bool, filterEnabled);
    QFETCH(QString, expectedTiming);
    ui::I18n::instance().setLang(ui::I18n::Lang::En);
    class FakeConfigurationActions final : public app::ConfigurationActions {
     public:
      int calls = 0;
      ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
        ++calls;
        return [] { return app::ConfigurationApplyResult{}; };
      }
    };
    auto snapshot = uwf::simulation::application::editableSnapshot();
    snapshot.current = snapshot.next;
    snapshot.current.filter.enabled = snapshot.next.filter.enabled = filterEnabled;
    core::PendingChanges changes;
    changes.setOverlay.warningThresholdMb = 5120;
    const auto actions = std::make_shared<FakeConfigurationActions>();
    const auto model = std::make_shared<presentation::ApplyPlanViewModel>(changes, snapshot, actions);
    QVERIFY(model->canApply());
    uwf::simulation::FileDialogStub fileDialogs;
    ui::ApplyPlanDialog dialog(model, fileDialogs);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    const auto* badge = dialog.findChild<ui::TextLabel*>(QStringLiteral("applyPlanSummaryBadge"));
    QVERIFY(badge && badge->isVisibleTo(&dialog));
    QCOMPARE(badge->text(), expectedTiming);
    auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
    QVERIFY(apply && apply->isEnabled());
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        dialog, [&] { QTest::mouseClick(apply, Qt::LeftButton); },
        [&](QDialog& confirmation) {
          const auto* body = confirmation.findChild<ui::ContentHintBox*>();
          QVERIFY(body);
          const auto text = body->toPlainText();
          QVERIFY2(text.contains(expectedTiming), qUtf8Printable(text));
          QVERIFY2(!text.contains(QStringLiteral("restart"), Qt::CaseInsensitive), qUtf8Printable(text));
          auto* cancel = test::buttonByRole(&confirmation, QDialogButtonBox::RejectRole);
          QVERIFY(cancel);
          inspected = true;
          QTest::mouseClick(cancel, Qt::LeftButton);
        }));
    QVERIFY(inspected);
    QVERIFY(dialog.isVisible());
    QVERIFY(model->canApply() && !model->applying());
    QCOMPARE(actions->calls, 0);
  }

  void applyActionTracksTheActualWindowAdmissionAndReceipt_data() {
    QTest::addColumn<int>("outcome");
    QTest::newRow("success") << 0;
    QTest::newRow("zero-effect-failure-and-retry") << 1;
    QTest::newRow("uncertain-write-failure") << 2;
  }
  void applyActionTracksTheActualWindowAdmissionAndReceipt() {
    QFETCH(int, outcome);
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = initial.next.filterEnabled = true;
    const auto backend = std::make_shared<uwf::simulation::FakeUwfBackend>(std::move(initial));
    const auto gate = std::make_shared<test::WorkerGate>();
    const auto release = qScopeGuard([&] { gate->release(); });
    const auto system = std::make_shared<GatedFilter>(backend, gate);
    const auto store = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
    MutableApplicationStateSource source;
    source.snapshot = uwf::simulation::application::editableSnapshot();
    source.snapshot.runtime.romDisposition = 0;
    source.snapshot.runtime.fullCommit = false;
    source.hormEnabled = false;
    source.policy.transportAvailable = true;
    source.policy.current.filterEnabled = source.policy.next.filterEnabled = {app::PolicyObservationStatus::Available, true};
    source.policy.current.flags = source.policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{0}};
    source.volumes = {{"C:", uwf::simulation::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
    auto services = composition::mainWindowServices(system, source, store, test::localDesktopPorts());
    const auto workflows = std::make_shared<CapturingWindowWorkflows>(services.workflows);
    services.workflows = workflows;
    ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QTRY_VERIFY(refreshCompleted(window));
    const auto owner = workflows->observed;
    QVERIFY(owner && owner->configurationWritesAllowed());
    const auto importer = owner->importModel();
    QVERIFY(importer);
    QTRY_VERIFY(!importer->preparing());
    importer->setText(QStringLiteral("uwfmgr filter disable"));
    importer->importText();
    QCOMPARE(owner->pendingChangeCount(), 1);
    if (outcome != 0)
      backend->inject(
          {.request = compat::SetBoolean{compat::BooleanProperty::FilterEnabled, false},
           .operation = {},
           .timing = outcome == 1 ? uwf::simulation::FakeUwfBackend::FaultTiming::Before : uwf::simulation::FakeUwfBackend::FaultTiming::AfterEffect,
           .kind = compat::ErrorKind::Unavailable,
           .message = "Injected filter failure"});
    // The public Review entry tracks this plan in the actual window model,
    // keeping its operation and reconciliation gates connected to the dialog.
    const auto reviewed = owner->reviewConfiguration();
    QVERIFY(reviewed && reviewed->canApply());
    uwf::simulation::FileDialogStub fileDialogs;
    ui::ApplyPlanDialog reviewDialog(reviewed, fileDialogs, &window);
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        window, [&] { reviewDialog.exec(); },
        [&](QDialog& modal) {
          auto* dialog = qobject_cast<ui::ApplyPlanDialog*>(&modal);
          QVERIFY(dialog && reviewed);
          auto* apply = dialog->findChild<QPushButton*>(QStringLiteral("primaryBtn"));
          QVERIFY(apply && apply->isVisibleTo(dialog) && apply->isEnabled());
          const auto verifyInteractionLock = [&] {
            owner->setExternalOperationActive(true);
            QVERIFY(!reviewed->canApply());
            QVERIFY(apply->isVisibleTo(dialog) && !apply->isEnabled());
            owner->setExternalOperationActive(false);
            QVERIFY(apply->isVisibleTo(dialog) && apply->isEnabled());
          };
          verifyInteractionLock();
          const auto applyAndWait = [&](const int attempt) {
            gate->reset();
            QPointer<QDialog> execution;
            QVERIFY(test::driveModalDialog(
                *dialog, [&] { QTest::mouseClick(apply, Qt::LeftButton); },
                [&](QDialog& confirmation) {
                  execution = &confirmation;
                  auto* confirm = test::buttonByRole(&confirmation, QDialogButtonBox::AcceptRole);
                  QVERIFY(confirm && confirm->isEnabled());
                  QTest::mouseClick(confirm, Qt::LeftButton);
                }));
            QTRY_COMPARE(gate->started.load(), attempt);
            QVERIFY(reviewed->applying());
            QVERIFY(apply->isVisibleTo(dialog) && !apply->isEnabled());
            gate->release();
            QTRY_VERIFY(!reviewed->applying() && !owner->applyingConfiguration() && !owner->refreshing());
            QTRY_VERIFY(!execution || !execution->isVisible());
            QTRY_VERIFY(applyCompleted(*dialog));
            QCOMPARE(test::writeRequests(*backend).size(), static_cast<std::size_t>(attempt));
          };
          applyAndWait(1);
          QCOMPARE(reviewed->resultFailed(), outcome != 0);
          if (outcome == 1) {
            QVERIFY(!reviewed->result().anyWriteConfirmed() && !reviewed->result().reconciliationRequired());
            QVERIFY(reviewed->canApply());
            QVERIFY(apply->isVisibleTo(dialog) && apply->isEnabled());
            QCOMPARE(apply->text(), ui::I18n::tr("Retry"));
            QCOMPARE(owner->pendingChangeCount(), 1);
            verifyInteractionLock();
            applyAndWait(2);
            QVERIFY(!reviewed->resultFailed());
          } else if (outcome == 2) {
            QVERIFY(reviewed->result().reconciliationRequired());
          }
          QVERIFY(!reviewed->canApply());
          QVERIFY(!apply->isVisibleTo(dialog) && !apply->isEnabled());
          const auto writes = test::writeRequests(*backend).size();
          reviewed->apply();
          QCOMPARE(test::writeRequests(*backend).size(), writes);
          QCOMPARE(store->writes, 0);
          inspected = true;
        }));
    QVERIFY(inspected);
  }

  void resultRestartButtonsFollowActualWriteRefreshFailureAndRecovery() {
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = initial.next.filterEnabled = true;
    const auto backend = std::make_shared<uwf::simulation::FakeUwfBackend>(std::move(initial));
    std::atomic_bool failRefresh = false;
    std::atomic_int refreshes = 0;
    const auto system = std::make_shared<app::PrecheckedUwf>(
        backend->rawBackend(), app::UwfFactsSource{}, false, [&](const compat::Uwf& source, const compat::Context& context) {
          ++refreshes;
          return failRefresh ? compat::Response::fail(compat::ErrorKind::AccessDenied, "Injected post-write refresh failure")
                             : app::refreshUwfState(source, context);
        });
    system->updateState(backend->state());
    const auto volumes = std::make_shared<EmptyVolumes>();
    const auto source = app::uwfApplicationStateSource(system, volumes, true);
    const auto store = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
    auto ports = test::localDesktopPorts();
    ports.volumes = volumes;
    auto services = composition::mainWindowServices(system, *source, store, std::move(ports));
    const auto workflows = std::make_shared<CapturingWindowWorkflows>(services.workflows);
    services.workflows = workflows;
    ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const auto owner = workflows->observed;
    QVERIFY(owner);
    QTRY_VERIFY(owner->state().hasCommittedState() && !owner->refreshing());
    QCOMPARE(refreshes.load(), 1);
    QVERIFY(owner->powerActionsAllowed());
    auto* global = window.findChild<ui::GlobalStatusPanel*>();
    QVERIFY(global);
    QCOMPARE(global->viewModel().importFilterEnabled(false).outcome, app::GlobalEditOutcome::Applied);
    const auto plan = owner->reviewConfiguration();
    QVERIFY(plan && plan->canApply());
    uwf::simulation::FileDialogStub fileDialogs;
    ui::ApplyPlanDialog dialog(plan, fileDialogs, &window);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* safe = dialog.findChild<QPushButton*>(QStringLiteral("restartBtn"));
    auto* direct = dialog.findChild<QPushButton*>(QStringLiteral("directRestartBtn"));
    QVERIFY(safe && direct);
    QSignalSpy safeRequested(&dialog, &ui::ApplyPlanDialog::safeRestartRequested);
    QSignalSpy directRequested(&dialog, &ui::ApplyPlanDialog::directRestartRequested);
    backend->clearTrace();
    failRefresh = true;
    plan->apply();
    QTRY_VERIFY(!plan->applying() && !owner->refreshing() && owner->workspaceDiagnostic().has_value());
    // The filter primitive really completed; its mandatory global refresh failed.
    QVERIFY(!backend->snapshot().next.filterEnabled);
    QCOMPARE(test::writeRequests(*backend).size(), std::size_t{1});
    QCOMPARE(refreshes.load(), 2);
    QVERIFY(system->state().refreshFailure);
    QVERIFY(plan->canRestart());
    QVERIFY(safe->isVisible() && direct->isVisible());
    QVERIFY(!safe->isEnabled() && !direct->isEnabled());
    QVERIFY(!plan->canSafeRestart() && !plan->canDirectRestart());
    QVERIFY(safe->toolTip().contains(QStringLiteral("Injected post-write refresh failure")));
    QVERIFY(direct->toolTip().contains(QStringLiteral("Injected post-write refresh failure")));
    QTest::mouseClick(safe, Qt::LeftButton);
    QTest::mouseClick(direct, Qt::LeftButton);
    QCOMPARE(safeRequested.size(), 0);
    QCOMPARE(directRequested.size(), 0);

    failRefresh = false;
    owner->refresh();
    QTRY_VERIFY(!owner->refreshing() && !owner->workspaceDiagnostic());
    QCOMPARE(refreshes.load(), 3);
    QVERIFY(!system->state().refreshFailure);
    QTRY_VERIFY(safe->isEnabled() && direct->isEnabled());
    QVERIFY(safe->toolTip().isEmpty());
    QCOMPARE(direct->toolTip(), ui::I18n::tr("Direct restart skips staged file commits."));
    owner->setExternalOperationActive(true);
    QVERIFY(!safe->isEnabled() && !direct->isEnabled());
    owner->setExternalOperationActive(false);
    QVERIFY(safe->isEnabled() && direct->isEnabled());
    QCOMPARE(test::writeRequests(*backend).size(), std::size_t{1});
  }

  void operationAvailabilityReachesExistingAndNewHubMenus_data() {
    QTest::addColumn<bool>("power");
    QTest::addColumn<bool>("exit");
    for (const bool power : {false, true})
      for (const bool exit : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("power-%1-exit-%2").arg(power).arg(exit))) << power << exit;
  }
  void operationAvailabilityReachesExistingAndNewHubMenus() {
    QFETCH(bool, power);
    QFETCH(bool, exit);
    ui::OverlayHub hub;
    auto first = std::make_unique<MenuEndpoint>();
    auto* existing = first.get();
    hub.registerView(std::move(first));
    hub.setOperationAvailability(power, exit);
    auto second = std::make_unique<MenuEndpoint>();
    auto* added = second.get();
    hub.registerView(std::move(second));
    for (auto* endpoint : {existing, added}) {
      QCOMPARE(endpoint->menu.findChild<QAction*>(QStringLiteral("overlayMenuSafeRestart"))->isEnabled(), power);
      QCOMPARE(endpoint->menu.findChild<QAction*>(QStringLiteral("overlayMenuExit"))->isEnabled(), exit);
    }
    hub.setOperationAvailability(true, true);
    for (auto* endpoint : {existing, added}) {
      QVERIFY(endpoint->menu.findChild<QAction*>(QStringLiteral("overlayMenuSafeRestart"))->isEnabled());
      QVERIFY(endpoint->menu.findChild<QAction*>(QStringLiteral("overlayMenuExit"))->isEnabled());
    }
  }

  void applicationWriteBlocksEveryExitAndPowerMenu_data() {
    QTest::addColumn<bool>("hubVisible");
    QTest::addColumn<bool>("writeFails");
    for (const bool hub : {false, true})
      for (const bool fails : {false, true}) QTest::newRow(qPrintable(QStringLiteral("hub-%1-failure-%2").arg(hub).arg(fails))) << hub << fails;
  }
  void applicationWriteBlocksEveryExitAndPowerMenu() {
    QFETCH(bool, hubVisible);
    QFETCH(bool, writeFails);
    auto gate = std::make_shared<test::WorkerGate>();
    const auto unblock = qScopeGuard([&] {
      gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    MutableApplicationStateSource source;
    source.snapshot = uwf::simulation::application::editableSnapshot();
    uwf::simulation::FakeUwfBackend::State state;
    state.current.filterEnabled = source.snapshot.current.filter.enabled;
    state.next.filterEnabled = source.snapshot.next.filter.enabled;
    auto backend = std::make_shared<uwf::simulation::FakeUwfBackend>(state);
    if (writeFails)
      backend->inject({.request = compat::SetBoolean{compat::BooleanProperty::FilterEnabled, false},
                       .operation = {},
                       .kind = compat::ErrorKind::AccessDenied,
                       .message = "filter rejected"});
    auto system = std::make_shared<GatedFilter>(backend, gate);
    auto store = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
    ui::MainWindow window(composition::mainWindowServices(system, source, store, uwf::test::localDesktopPorts()),
                          {.uwfCapability = app::UwfCapability::Available, .hubVisible = hubVisible});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const auto verifyMenus = [&](const bool enabled) {
      int menus = 0;
      const auto verifyMenu = [&](ui::OverlayContextMenu* menu) {
        QVERIFY(menu);
        ++menus;
        const auto closeMenu = qScopeGuard([menu] { menu->close(); });
        QSignalSpy exits(menu, &ui::OverlayContextMenu::exitApplicationRequested);
        QSignalSpy restarts(menu, &ui::OverlayContextMenu::safeRestartRequested);
        QSignalSpy shutdowns(menu, &ui::OverlayContextMenu::safeShutdownRequested);
        if (!menu->isVisible()) menu->popup(window.mapToGlobal(QPoint(30, 30)));
        QTest::qWait(30);
        for (const auto* name : {"overlayMenuExit", "overlayMenuSafeRestart", "overlayMenuSafeShutdown"}) {
          auto* action = menu->findChild<QAction*>(QString::fromLatin1(name));
          QVERIFY(action);
          QTRY_COMPARE(action->isEnabled(), enabled);
          if (!enabled) QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(action).center());
        }
        QCOMPARE(exits.size(), 0);
        QCOMPARE(restarts.size(), 0);
        QCOMPARE(shutdowns.size(), 0);
      };
      QList<QPointer<ui::OverlayContextMenu>> existingMenus;
      QList<QPointer<ui::OverlayHubView>> hubEndpoints;
      for (auto widgets = QApplication::allWidgets(); auto* widget : std::as_const(widgets)) {
        if (auto* menu = qobject_cast<ui::OverlayContextMenu*>(widget)) existingMenus.append(menu);
        if (auto* endpoint = qobject_cast<ui::OverlayHubView*>(widget)) hubEndpoints.append(endpoint);
      }
      for (const auto& menu : std::as_const(existingMenus))
        if (menu) verifyMenu(menu);
      int endpoints = 0;
      for (const auto& endpoint : std::as_const(hubEndpoints)) {
        if (!endpoint) continue;
        ++endpoints;
        auto* target = endpoint->findChild<QWidget*>(QStringLiteral("overlayFloatingHandle"));
        if (!target) target = endpoint.data();
        bool verified = false;
        QTimer::singleShot(0, &window, [&] {
          auto* popup = QApplication::activePopupWidget();
          const auto closePopup = qScopeGuard([popup] {
            if (popup) popup->close();
          });
          verifyMenu(qobject_cast<ui::OverlayContextMenu*>(popup));
          verified = true;
        });
        // Context menus are created lazily. Send the semantic event directly;
        // synthesized mouse clicks do not request an OS context menu offscreen.
        const QPoint position(5, 5);
        QContextMenuEvent event(QContextMenuEvent::Mouse, position, window.mapToGlobal(QPoint(30, 30)));
        QApplication::sendEvent(target, &event);
        QTRY_VERIFY_WITH_TIMEOUT(verified, 2000);
      }
      QVERIFY(endpoints > 0);
      QVERIFY(menus > 0);
    };
    auto* global = window.findChild<ui::GlobalStatusPanel*>();
    QVERIFY(global);
    QCOMPARE((global->viewModel().importFilterEnabled(false)).outcome, uwf::app::GlobalEditOutcome::Applied);
    bool inspected = false;
    bool finish = false;
    QTimer closer;
    connect(&closer, &QTimer::timeout, &window, [&] {
      if (finish)
        if (auto* dialog = qobject_cast<ui::ApplyPlanDialog*>(QApplication::activeModalWidget()); dialog && applyCompleted(*dialog)) dialog->reject();
    });
    closer.start(50);
    QTimer::singleShot(0, &window, [&] {
      const auto cleanup = qScopeGuard([&] {
        gate->release();
        finish = true;
      });
      auto* dialog = qobject_cast<ui::ApplyPlanDialog*>(QApplication::activeModalWidget());
      QVERIFY(dialog);
      auto* apply = buttonWithText(dialog, ui::I18n::tr("Apply changes"));
      QVERIFY(apply && apply->isEnabled());
      QVERIFY(test::driveModalDialog(*dialog, [&] { QTest::mouseClick(apply, Qt::LeftButton); }, [](QDialog& confirmation) { confirmation.accept(); }));
      QTRY_COMPARE(gate->started.load(), 1);
      QCOMPARE(gate->finished.load(), 0);
      QVERIFY(!window.close());
      dialog->reject();
      QVERIFY(dialog->isVisible());
      verifyMenus(false);
      inspected = true;
    });
    window.showPlan();
    closer.stop();
    QVERIFY(inspected);
    const auto trace = backend->trace();
    QCOMPARE(std::ranges::count_if(trace, [](const auto& entry) { return compat::isWrite(entry.request); }), 1);
    QCOMPARE(std::ranges::count_if(
                 trace, [](const auto& entry) { return entry.request == compat::Request{compat::SetBoolean{compat::BooleanProperty::FilterEnabled, false}}; }),
             1);
    verifyMenus(true);
  }

  void startupKeepsSummaryAndPlanConsistent_data() {
    QTest::addColumn<bool>("quiet");
    QTest::addColumn<bool>("filter");
    for (const bool quiet : {false, true})
      for (const bool filter : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("quiet-%1-filter-%2").arg(quiet).arg(filter))) << quiet << filter;
  }
  void startupKeepsSummaryAndPlanConsistent() {
    QFETCH(bool, quiet);
    QFETCH(bool, filter);
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>();
    auto storeOwner = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
    auto& store = *storeOwner;
    MutableApplicationStateSource source;
    source.snapshot = uwf::simulation::application::editableSnapshot();
    source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = filter;
    source.volumes = {{"C:", uwf::simulation::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
    ui::MainWindow window(composition::mainWindowServices(system, source, storeOwner, uwf::test::localDesktopPorts()),
                          {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    if (quiet) window.startInTray();
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* global = window.findChild<ui::GlobalStatusPanel*>();
    QVERIFY(global);
    QVERIFY(!global->viewModel().pendingFilterEnabled());
    QVERIFY(global->viewModel().pendingOverlay().empty());
    auto* discard = buttonWithText(&window, ui::I18n::tr("Discard changes"));
    QVERIFY(discard);
    QVERIFY2(!discard->isVisibleTo(&window), "Startup must not present a dirty summary for an empty draft");
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        window, [&] { window.showPlan(); },
        [&](QDialog& modal) {
          auto* plan = qobject_cast<ui::ApplyPlanDialog*>(&modal);
          QVERIFY(plan);
          auto* changes = plan->findChild<QListView*>(QStringLiteral("applyChangesList"));
          auto* apply = buttonWithText(plan, ui::I18n::tr("Apply changes"));
          QVERIFY(changes && apply);
          QCOMPARE(changes->model()->rowCount(), 1);  // Only the command disclosure remains.
          QVERIFY(!apply->isEnabled());
          inspected = true;
        }));
    QVERIFY(inspected);
    QCOMPARE(store.writes, 0);
    QVERIFY(uwf::test::writeRequests(*system).empty());
  }

  void everyUserRefreshEntryConfirmsTheDraft_data() {
    QTest::addColumn<QString>("entry");
    QTest::addColumn<bool>("confirm");
    for (const auto* entry : {"toolbar", "shortcut", "volume-retry", "workspace-retry", "public-slot"})
      for (const bool confirm : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString::fromLatin1(entry), confirm ? "confirm" : "cancel")))
            << QString::fromLatin1(entry) << confirm;
  }
  void everyUserRefreshEntryConfirmsTheDraft() {
    QFETCH(QString, entry);
    QFETCH(bool, confirm);
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>();
    const auto store = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
    MutableApplicationStateSource source;
    source.snapshot = uwf::simulation::application::editableSnapshot();
    if (entry != QLatin1String("workspace-retry"))
      source.volumes = {{"C:", uwf::simulation::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
    auto services = composition::mainWindowServices(system, source, store, test::localDesktopPorts());
    const auto workflows = std::make_shared<CapturingWindowWorkflows>(services.workflows);
    services.workflows = workflows;
    ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QTRY_VERIFY(refreshCompleted(window));
    QCOMPARE(source.reads, 1);
    QVERIFY(!QApplication::activeModalWidget());

    auto* global = window.findChild<ui::GlobalStatusPanel*>();
    QVERIFY(global);
    QCOMPARE((global->viewModel().importFilterEnabled(!source.snapshot.next.filter.enabled)).outcome, uwf::app::GlobalEditOutcome::Applied);
    // An internal read failure retains drafts created while state was healthy.
    // The next user-initiated retry must still confirm discarding those drafts.
    source.failure = QStringLiteral("provider temporarily unavailable");
    QVERIFY(workflows->observed);
    workflows->observed->refresh();
    QTRY_VERIFY(refreshCompleted(window));
    QCOMPARE(source.reads, 2);
    QCOMPARE((global->viewModel().importFilterEnabled(source.snapshot.next.filter.enabled)).outcome, uwf::app::GlobalEditOutcome::Rejected);
    QVERIFY(global->viewModel().pendingFilterEnabled().has_value());
    auto* discard = buttonWithText(&window, ui::I18n::tr("Discard changes"));
    QVERIFY(discard && discard->isVisibleTo(&window));
    source.failure.clear();

    int prompts = 0;
    QString promptTitle;
    QTimer answer;
    connect(&answer, &QTimer::timeout, &window, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      if (!dialog) return;
      ++prompts;
      promptTitle = dialog->windowTitle();
      answer.stop();
      if (confirm)
        dialog->accept();
      else
        dialog->reject();
    });
    answer.start(0);
    if (entry == QLatin1String("toolbar")) {
      const auto* action = actionWithText(&window, ui::I18n::tr("Refresh"));
      QVERIFY(action && action->isEnabled());
      QToolButton* refresh = nullptr;
      for (auto buttons = window.findChildren<QToolButton*>(); auto* button : std::as_const(buttons))
        if (button->defaultAction() == action) refresh = button;
      QVERIFY(refresh && refresh->isVisibleTo(&window));
      QTest::mouseClick(refresh, Qt::LeftButton);
    } else if (entry == QLatin1String("shortcut")) {
      window.activateWindow();
      QVERIFY(QTest::qWaitForWindowActive(&window));
      QTest::keyClick(&window, Qt::Key_F5);
    } else if (entry == QLatin1String("public-slot")) {
      window.refresh();
    } else {
      const auto name = entry == QLatin1String("workspace-retry") ? QStringLiteral("mainUnavailableState") : QStringLiteral("volumeUnavailableState");
      ui::ContentState* retry = nullptr;
      for (auto states = window.findChildren<ui::ContentState*>(name); auto* state : std::as_const(states))
        if (state->isVisibleTo(&window)) retry = state;
      QVERIFY(retry);
      auto* read = buttonWithText(retry, ui::I18n::tr("Read again"));
      QVERIFY(read && read->isVisibleTo(&window));
      QVERIFY(read->isEnabled());
      QTest::mouseClick(read, Qt::LeftButton);
    }
    QTRY_COMPARE(prompts, 1);
    answer.stop();
    QCOMPARE(promptTitle, ui::I18n::tr("Discard pending changes?"));
    QTRY_VERIFY(refreshCompleted(window));
    QCOMPARE(source.reads, confirm ? 3 : 2);
    QCOMPARE(global->viewModel().pendingFilterEnabled().has_value(), !confirm);
    QCOMPARE(discard->isVisibleTo(&window), !confirm);
    QCOMPARE(store->writes, 0);
    QVERIFY(test::writeRequests(*system).empty());
  }

  void confirmedDiscardRestoresAllEditors_data() {
    QTest::addColumn<bool>("confirm");
    QTest::addColumn<bool>("stagingReadFails");
    QTest::addColumn<bool>("allEditors");
    for (const bool all : {false, true}) {
      const auto scope = all ? QStringLiteral("all-editors") : QStringLiteral("filter-only");
      QTest::newRow(qUtf8Printable(scope + QStringLiteral("-cancel"))) << false << false << all;
      QTest::newRow(qUtf8Printable(scope + QStringLiteral("-confirm"))) << true << false << all;
      QTest::newRow(qUtf8Printable(scope + QStringLiteral("-read-fails"))) << true << true << all;
    }
  }
  void confirmedDiscardRestoresAllEditors() {
    QFETCH(bool, confirm);
    QFETCH(bool, stagingReadFails);
    QFETCH(bool, allEditors);
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>();
    auto storeOwner = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
    auto& store = *storeOwner;
    store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\saved.bin")}};
    const auto saved = store.entries;
    MutableApplicationStateSource source;
    source.snapshot = uwf::simulation::application::editableSnapshot();
    source.volumes = {{"C:", uwf::simulation::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
    ui::MainWindow window(composition::mainWindowServices(system, source, storeOwner, uwf::test::localDesktopPorts()),
                          {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* global = window.findChild<ui::GlobalStatusPanel*>();
    auto* volume = window.findChild<ui::VolumeTab*>();
    auto* staged = window.findChild<ui::FileStagingWidget*>();
    QVERIFY(global && volume && staged);
    const auto model = volume->viewModel().editor();
    QCOMPARE((global->viewModel().importFilterEnabled(!source.snapshot.next.filter.enabled)).outcome, uwf::app::GlobalEditOutcome::Applied);
    if (allEditors) {
      QCOMPARE((global->viewModel().importOverlayWarnMb(4000)).outcome, uwf::app::GlobalEditOutcome::Applied);
      QVERIFY(volume->viewModel().importProtect(!source.snapshot.next.volumes.front().isProtected));
      QCOMPARE(model->files()->importAdd(QStringLiteral("C:\\Data")), app::ExclusionImportOutcome::Applied);
      QCOMPARE(model->registry()->importAdd(QStringLiteral("HKLM\\SOFTWARE\\Pending")), app::ExclusionImportOutcome::Applied);
      model->registry()->enablePersistence(app::ExclusionEditor::PersistenceItem::DomainSecretKey);
      staged->viewModel().removePaths({saved.front().path});
      QTRY_VERIFY(!staged->viewModel().busy());
    }
    auto* discard = buttonWithText(&window, ui::I18n::tr("Discard changes"));
    QVERIFY(discard && discard->isVisibleTo(&window));
    if (stagingReadFails) store.readFailure = QStringLiteral("read denied");
    bool answered = false;
    QTimer answer;
    connect(&answer, &QTimer::timeout, &window, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      if (!dialog) return;
      if (dialog->windowTitle() == ui::I18n::tr("Discard pending changes?")) {
        answered = true;
        auto* choice = buttonWithText(dialog, confirm ? ui::I18n::tr("Continue") : ui::I18n::tr("Cancel"));
        QVERIFY(choice);
        QTest::mouseClick(choice, Qt::LeftButton);
      } else {
        dialog->reject();
      }
    });
    answer.start(0);
    QTest::mouseClick(discard, Qt::LeftButton);
    answer.stop();
    QVERIFY(answered);
    // A failure that has not been observed cannot invalidate the known cache.
    // Discard restores that cache without a storage read.
    const bool discarded = confirm;
    const bool otherEditsRemain = allEditors && !discarded;
    QCOMPARE(global->viewModel().pendingFilterEnabled().has_value(), !discarded);
    QCOMPARE(global->viewModel().pendingOverlay().empty(), !otherEditsRemain);
    QCOMPARE(model->status()->pendingProtection().has_value(), otherEditsRemain);
    QCOMPARE(model->files()->pendingAdded().isEmpty(), !otherEditsRemain);
    QCOMPARE(model->registry()->pendingAdded().isEmpty(), !otherEditsRemain);
    QCOMPARE(model->registry()->pendingPersistDomainSecretKey().has_value(), otherEditsRemain);
    QTRY_COMPARE(staged->viewModel().entries().isEmpty(), otherEditsRemain);
    QCOMPARE(discard->isVisibleTo(&window), !discarded);
    QCOMPARE(store.entries, saved);
    QCOMPARE(store.writes, 0);
    QCOMPARE(source.reads, 1);  // Discard restores the committed snapshot, without another provider read.
    QVERIFY(uwf::test::writeRequests(*system).empty());
  }
};
}  // namespace
QTEST_MAIN(ConfigurationLifecycleUiTests)
#include "ConfigurationLifecycleUiTests.moc"
