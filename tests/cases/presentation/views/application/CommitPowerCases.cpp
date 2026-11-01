// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QScreen>
#include <QSignalSpy>
#include <QTableView>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTimer>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

#include "application/operations/contracts/OperationError.h"
#include "application/power/PowerCoordinator.h"
#include "application/uwf/contracts/UwfCapability.h"
#include "cases/presentation/views/application/ApplicationUiFixture.h"
#include "cases/presentation/views/application/ApplicationUiTest.h"
#include "composition/desktop/ViewModelComposition.h"
#include "composition/desktop/WorkflowComposition.h"
#include "domain/uwf/model/UwfModel.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "infrastructure/windows/filesystem/paths/VolumePaths.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/commit/CommitBatchViewModel.h"
#include "presentation/viewmodels/overlay/OverlayViewModel.h"
#include "presentation/views/commit/CommitDispatcher.h"
#include "presentation/views/commit/CommitReportDialog.h"
#include "presentation/views/configuration/volumes/VolumeTab.h"
#include "presentation/views/power/PowerActionDialog.h"
#include "support/app/CommitBatchFixture.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/UwfMaintenanceFixture.h"
#include "support/ui/CommitDialogDriver.h"
#include "support/ui/DesktopPlatformFixture.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/LayoutSettling.h"
#include "support/ui/ViewMetrics.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/InkBounds.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

class ApplicationCommitPowerTests final : public ApplicationUiTest {
  Q_OBJECT
 private slots:
  void powerFlowVisualStates_data();
  void powerFlowVisualStates();
  void powerButtonVisualStates_data();
  void powerButtonVisualStates();
  void powerButtonContentAlignment_data();
  void powerButtonContentAlignment();
  void diskCommitActionsRouteSelectedTargetsAndHonorCancellation();
  void commitDispatcherRejectsUnaddressablePathsAndReleasesObservationPause();
  void commitDispatcherConfirmsAndReportsARealFileThroughTheRepository();
  void commitDispatcherRoutesAnExistingRegistryValueThroughTheRepository();
  void safePowerActionsRequireConfirmationAndUseTheInjectedRepository();
  void commitBatchKeepsApiFailuresAlongsideExistenceObservations();
};

using namespace uwf;
using namespace uwf::test::application;
namespace {
bool capturePowerThemes(QWidget& dialog, const QString& state, const bool animated = false) {
  for (const bool dark : {false, true}) {
    ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
    if (!test::waitForLayoutSettled(dialog)) return false;
    const auto name = state + (dark ? QStringLiteral("-dark") : QStringLiteral("-light"));
    if (!(animated ? test::saveAnimatedScreenshot(dialog, name) : test::saveScreenshot(dialog, name))) return false;
  }
  return true;
}

class ObservedPowerRepository final : public app::UwfMaintenanceRepository {
 public:
  ObservedPowerRepository(std::shared_ptr<app::UwfMaintenanceRepository> repository, std::function<void()> beforePower)
      : m_repository(std::move(repository)), m_beforePower(std::move(beforePower)) {}
  app::CommitRequirement fileCommitRequirement(const std::string& volume) const override { return m_repository->fileCommitRequirement(volume); }
  app::CommitRequirement registryCommitRequirement() const override { return m_repository->registryCommitRequirement(); }
  app::AutomaticCommitDisposition automaticFileCommitDisposition(const std::string& volume) const override {
    return m_repository->automaticFileCommitDisposition(volume);
  }
  app::CommitRequirement registryWriteRequirement() const override { return m_repository->registryWriteRequirement(); }
  std::vector<std::shared_ptr<app::UwfVolumeTarget>> prepareVolumeTargets() const override { return m_repository->prepareVolumeTargets(); }
  std::shared_ptr<app::UwfRegistryMaintenanceTarget> prepareRegistryMaintenance() const override { return m_repository->prepareRegistryMaintenance(); }
  std::shared_ptr<app::UwfRegistryExclusionConfiguration> prepareRegistryExclusions() const override { return m_repository->prepareRegistryExclusions(); }
  std::function<void()> preparePowerAction(app::PowerAction action) const override {
    return [work = m_repository->preparePowerAction(action), before = m_beforePower] {
      before();
      work();
    };
  }

 private:
  std::shared_ptr<app::UwfMaintenanceRepository> m_repository;
  std::function<void()> m_beforePower;
};
std::vector<compat::Request> writes(const UwfMaintenanceFixture& fixture) {
  std::vector<compat::Request> result;
  for (const auto& entry : fixture.backend->trace())
    if (compat::isWrite(entry.request)) result.push_back(entry.request);
  return result;
}

ui::ContentHintBox* visiblePowerFailureDetails(QDialog* dialog) {
  auto* details = dialog->findChild<ui::ContentHintBox*>(QStringLiteral("powerStagingFailureDetails"));
  if (!details) return nullptr;
  auto* disclosure = dialog->findChild<ui::Disclosure*>(QStringLiteral("powerFailureDisclosure"));
  if (!disclosure || !disclosure->isVisible()) return nullptr;
  if (!disclosure->isOpen()) QTest::mouseClick(disclosure->toggleButton(), Qt::LeftButton);
  return details;
}
void approveNestedPowerConfirmation(QObject* context) {
  QTimer::singleShot(0, context, [] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QVERIFY(dialog);
    auto* button = test::buttonByRole(dialog, QDialogButtonBox::AcceptRole);
    QVERIFY(button);
    QTest::mouseClick(button, Qt::LeftButton);
  });
}
}  // namespace

void ApplicationCommitPowerTests::diskCommitActionsRouteSelectedTargetsAndHonorCancellation() {
  FileDialogStub files;
  const core::VolumeInfo disk{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  ui::VolumeTab tab(std::make_shared<presentation::VolumeConfigurationViewModel>(disk, true), files, nullptr,
                    uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts()));
  tab.viewModel().applySnapshot(editableSnapshot());
  tab.resize(test::boundedWindowSize(tab, QSize(900, 600)));
  tab.show();
  QVERIFY(QTest::qWaitForWindowExposed(&tab));
  QSignalSpy commit(&tab, &ui::VolumeTab::commitFileRequested);
  QSignalSpy deletion(&tab, &ui::VolumeTab::commitFileDeletionRequested);

  auto* commitFile = actionWithText(&tab, QStringLiteral("Commit file changes…"));
  auto* commitFolder = actionWithText(&tab, QStringLiteral("Commit folder changes…"));
  auto* deleteFile = actionWithText(&tab, QStringLiteral("Delete and commit file…"));
  auto* deleteFolder = actionWithText(&tab, QStringLiteral("Delete and commit folder…"));
  QVERIFY(commitFile && commitFile->isEnabled());
  QVERIFY(commitFolder && commitFolder->isEnabled());
  QVERIFY(deleteFile && deleteFile->isEnabled());
  QVERIFY(deleteFolder && deleteFolder->isEnabled());

  files.openedFile = QStringLiteral("C:/Data/state.bin");

  files.openedFiles = {files.openedFile};
  commitFile->trigger();
  QCOMPARE(commit.count(), 1);
  QCOMPARE(commit.front().front().toString(), QStringLiteral("C:\\Data\\state.bin"));

  files.selectedDirectory = QStringLiteral("C:/Cache");
  commitFolder->trigger();
  QCOMPARE(commit.count(), 2);
  QCOMPARE(commit.back().front().toString(), QStringLiteral("C:\\Cache"));

  files.openedFile = QStringLiteral("C:/Delete/me.bin");

  files.openedFiles = {files.openedFile};
  deleteFile->trigger();
  files.selectedDirectory = QStringLiteral("C:/Delete/tree");
  deleteFolder->trigger();
  QCOMPARE(deletion.count(), 2);
  QCOMPARE(deletion.front().front().toString(), QStringLiteral("C:\\Delete\\me.bin"));
  QCOMPARE(deletion.back().front().toString(), QStringLiteral("C:\\Delete\\tree"));

  files.openedFile.clear();
  files.openedFiles.clear();
  commitFile->trigger();
  QCOMPARE(commit.count(), 2);
  QCOMPARE(files.requests.size(), 5);
  QVERIFY(std::ranges::all_of(files.requests, [](const auto& request) { return request.initialPath == QStringLiteral("C:\\"); }));
}

void ApplicationCommitPowerTests::commitDispatcherRejectsUnaddressablePathsAndReleasesObservationPause() {
  UwfMaintenanceFixture fixture;
  const auto ports = uwf::test::localDesktopPorts();
  const auto overlay = uwf::composition::createViewModelFactory(fixture.backend, ports)->overlay();
  auto dispatcher = uwf::composition::createCommitDispatcher(
      fixture.repository(), [overlay] { return overlay->pauseObservation(); }, ports.traversal, ports.writeText, ports.fileDialogs, nullptr);
  const auto before = fixture.backend->snapshot();

  dispatcher->commitFilePath({});
  QVERIFY(!overlay->observationPaused());
  QVERIFY(test::driveModalDialog(*this, [&] { dispatcher->commitFilePath(QStringLiteral("relative/path.txt")); }, [](QDialog& warning) { warning.accept(); }));
  QVERIFY(!overlay->observationPaused());
  QVERIFY(writes(fixture).empty());
  QVERIFY(fixture.backend->snapshot() == before);
  QCOMPARE(fixture.state->volumeReads, 0);
  QCOMPARE(fixture.state->registryTargets, 0);
  QCOMPARE(fixture.state->exclusionReads, 0);
}

void ApplicationCommitPowerTests::commitDispatcherConfirmsAndReportsARealFileThroughTheRepository() {
  QTemporaryFile file(QDir::temp().filePath(QStringLiteral("uwf-commit-test-XXXXXX.tmp")));
  QVERIFY(file.open());
  QVERIFY(file.write("committed payload") > 0);
  QVERIFY(file.flush());
  const QString path = QDir::toNativeSeparators(file.fileName());
  const QString drive = QString::fromStdString(drive::fromPath(path.toStdString()));
  QVERIFY(!drive.isEmpty());

  UwfMaintenanceFixture fixture;
  (void)fixture.useFileSystemTarget(*windows::systemCommitTraversalSource(), path);
  fixture.stageFile(path, "committed payload");

  const auto ports = uwf::test::localDesktopPorts();
  const auto overlay = uwf::composition::createViewModelFactory(fixture.backend, ports)->overlay();
  auto dispatcher = uwf::composition::createCommitDispatcher(
      fixture.repository(), [overlay] { return overlay->pauseObservation(); }, ports.traversal, ports.writeText, ports.fileDialogs, nullptr);

  bool confirmationAccepted = false;
  bool reportObserved = false;
  bool reportSucceeded = false;
  bool observationWasPaused = false;
  QTimer modalDriver;
  modalDriver.setInterval(1);
  connect(&modalDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    observationWasPaused |= overlay->observationPaused();
    if (test::driveCommitOperation(dialog, reportObserved, confirmationAccepted)) return;
    if (auto* table = dialog->findChild<QTableView*>(); table && table->model()->columnCount() == 4) {
      reportObserved = true;
      reportSucceeded = table->model()->rowCount() == 1 && table->model()->index(0, 0).isValid() &&
                        table->model()->index(0, 0).data().toString() == QStringLiteral("Succeeded") && table->model()->index(0, 1).isValid() &&
                        table->model()->index(0, 1).data(presentation::FullTextRole).toString() == path;
      dialog->accept();
      return;
    }
    if (auto* box = dialog->findChild<QDialogButtonBox*>()) {
      for (auto buttons = box->buttons(); auto* button : std::as_const(buttons)) {
        if (box->buttonRole(button) == QDialogButtonBox::AcceptRole) {
          confirmationAccepted = true;
          QTest::mouseClick(button, Qt::LeftButton);
          return;
        }
      }
    }
  });
  modalDriver.start();
  dispatcher->commitFilePath(path);
  modalDriver.stop();

  QVERIFY(confirmationAccepted);
  QVERIFY(reportObserved);
  QVERIFY(reportSucceeded);
  QVERIFY(observationWasPaused);
  QVERIFY(!overlay->observationPaused());
  QCOMPARE(fixture.state->commits, std::vector<std::string>{windows::systemCommitTraversalSource()->resolveFileTarget(path).relativePath.toStdString()});

  QVERIFY(test::driveModalDialog(*this, [&] { dispatcher->commitFilePath(path); }, [](QDialog& confirmation) { confirmation.reject(); }));
  QCOMPARE(fixture.state->commits.size(), std::size_t{1});
  QVERIFY(!overlay->observationPaused());
}

void ApplicationCommitPowerTests::commitDispatcherRoutesAnExistingRegistryValueThroughTheRepository() {
  const QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
  const QString valueName = QStringLiteral("ProductName");
  UwfMaintenanceFixture fixture;
  fixture.stageRegistryValue(key.toStdString(), valueName.toStdString(), "Controlled Windows product");

  const auto ports = uwf::test::localDesktopPorts();
  const auto overlay = uwf::composition::createViewModelFactory(fixture.backend, ports)->overlay();
  auto dispatcher = uwf::composition::createCommitDispatcher(
      fixture.repository(), [overlay] { return overlay->pauseObservation(); }, ports.traversal, ports.writeText, ports.fileDialogs, nullptr);

  bool confirmationAccepted = false;
  bool reportSucceeded = false;
  bool reportObserved = false;
  bool observationWasPaused = false;
  QTimer modalDriver;
  modalDriver.setInterval(1);
  connect(&modalDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    observationWasPaused |= overlay->observationPaused();
    if (test::driveCommitOperation(dialog, reportObserved, confirmationAccepted)) return;
    if (auto* table = dialog->findChild<QTableView*>(); table && table->model()->columnCount() == 4) {
      reportObserved = true;
      reportSucceeded = table->model()->rowCount() == 1 && table->model()->index(0, 0).isValid() &&
                        table->model()->index(0, 0).data().toString() == QStringLiteral("Succeeded") && table->model()->index(0, 1).isValid() &&
                        table->model()->index(0, 1).data().toString().contains(valueName);
      dialog->accept();
      return;
    }
    if (auto* box = dialog->findChild<QDialogButtonBox*>()) {
      for (auto buttons = box->buttons(); auto* button : std::as_const(buttons)) {
        if (box->buttonRole(button) == QDialogButtonBox::AcceptRole) {
          confirmationAccepted = true;
          QTest::mouseClick(button, Qt::LeftButton);
          return;
        }
      }
    }
  });
  modalDriver.start();
  dispatcher->commitRegistryKey(key, valueName);
  modalDriver.stop();

  QVERIFY(confirmationAccepted);
  QVERIFY(reportSucceeded);
  QVERIFY(observationWasPaused);
  QVERIFY(!overlay->observationPaused());
  QCOMPARE(fixture.state->registryCommits.size(), std::size_t{1});
  QCOMPARE(fixture.state->registryCommits.front().key, key.toStdString());
  QCOMPARE(fixture.state->registryCommits.front().valueName, valueName.toStdString());
}

void ApplicationCommitPowerTests::safePowerActionsRequireConfirmationAndUseTheInjectedRepository() {
  UwfMaintenanceFixture fixture;
  MemoryFileStagingStore staging;
  auto controller = uwf::composition::createPowerCoordinator(
      {fixture.repository(), staging, uwf::app::UwfCapability::Available, uwf::windows::systemCommitTraversalSource()}, nullptr);

  QVERIFY(test::driveModalDialog(*this, [&] { controller->safeShutdown(); }, [](QDialog& dialog) { dialog.reject(); }));
  QVERIFY(writes(fixture).empty());
  QCOMPARE(fixture.state->filterReads, 0);

  UwfMaintenanceFixture unreadable;
  MemoryFileStagingStore unreadableStore;
  unreadableStore.readFailure = QStringLiteral("registry staging data is corrupt");
  auto unreadableController = uwf::composition::createPowerCoordinator(
      {unreadable.repository(), unreadableStore, uwf::app::UwfCapability::Available, uwf::windows::systemCommitTraversalSource()}, nullptr);
  bool readFailureStayedInPowerDialog = false;
  bool readFailureDetailsVisible = false;
  QTimer unreadableDriver;
  unreadableDriver.setInterval(1);
  connect(&unreadableDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* details = visiblePowerFailureDetails(dialog);
    if (!details || !details->isVisible()) return;
    readFailureStayedInPowerDialog = dialog->objectName() == QStringLiteral("powerConfirmDialog");
    readFailureDetailsVisible = details->toPlainText().contains(unreadableStore.readFailure);
    dialog->reject();
    unreadableDriver.stop();
  });
  unreadableDriver.start();
  unreadableController->safeShutdown();
  QVERIFY(readFailureStayedInPowerDialog);
  QVERIFY(readFailureDetailsVisible);
  QCOMPARE(unreadable.state->filterReads, 0);
  QVERIFY(writes(unreadable).empty());

  UwfMaintenanceFixture approvedUnreadable;

  MemoryFileStagingStore approvedUnreadableStore;
  approvedUnreadableStore.readFailure = unreadableStore.readFailure;
  auto approvedUnreadableController = uwf::composition::createPowerCoordinator(
      {approvedUnreadable.repository(), approvedUnreadableStore, uwf::app::UwfCapability::Available, uwf::windows::systemCommitTraversalSource()}, nullptr);
  bool unreadableContinuationBlocked = false;
  QVERIFY(test::driveModalDialog(
      *this, [&] { approvedUnreadableController->safeShutdown(); },
      [&](QDialog& modal) {
        auto* dialog = &modal;
        auto* disclosure = dialog->findChild<ui::Disclosure*>(QStringLiteral("powerFailureDisclosure"));
        QVERIFY(disclosure);
        QTRY_VERIFY(disclosure->isVisible());
        QVERIFY(disclosure->isOpen());
        auto* details = dialog->findChild<ui::ContentHintBox*>(QStringLiteral("powerStagingFailureDetails"));
        auto* shutdown = dialog->findChild<QPushButton*>(QStringLiteral("dangerBtn"));
        QVERIFY(details && details->isVisible() && shutdown);
        QVERIFY(details->toPlainText().contains(approvedUnreadableStore.readFailure));
        QVERIFY(!shutdown->isVisibleTo(dialog) && !shutdown->isEnabled());
        shutdown->click();
        QVERIFY(dialog->isVisible());
        unreadableContinuationBlocked = true;
        // Even programmatic dialog acceptance must not bypass failed preparation.
        dialog->accept();
        QCOMPARE(dialog->result(), QDialog::Accepted);
      }));
  QVERIFY(unreadableContinuationBlocked);
  QVERIFY(writes(approvedUnreadable).empty());
  QCOMPARE(approvedUnreadable.state->filterReads, 0);

  UwfMaintenanceFixture rejectedPower;

  rejectedPower.backend->inject({.request = compat::Power{compat::PowerAction::Shutdown},
                                 .operation = {},
                                 .timing = test::FakeUwfBackend::FaultTiming::AfterEffect,
                                 .kind = compat::ErrorKind::Unavailable,
                                 .message = "Shutdown acknowledgement failed"});
  std::vector<bool> rejectedPowerPreshutdownStates;
  bool powerNotInvokedBeforePreshutdownToken = false;
  bool powerInvokedWithPreshutdownToken = false;
  auto rejectedRepository = std::make_shared<ObservedPowerRepository>(
      rejectedPower.repository(), [&] { powerInvokedWithPreshutdownToken = rejectedPowerPreshutdownStates == std::vector<bool>{true}; });
  using PreshutdownControlResult = app::PreshutdownControlResult;
  auto rejectedPowerController = uwf::composition::createPowerCoordinator({rejectedRepository,
                                                                           staging,
                                                                           uwf::app::UwfCapability::Available,
                                                                           uwf::windows::systemCommitTraversalSource(),
                                                                           {[&] {
                                                                              powerNotInvokedBeforePreshutdownToken = writes(rejectedPower).empty();
                                                                              rejectedPowerPreshutdownStates.push_back(true);
                                                                              return PreshutdownControlResult::Acknowledged;
                                                                            },
                                                                            [&]() -> PreshutdownControlResult {
                                                                              rejectedPowerPreshutdownStates.push_back(false);
                                                                              throw std::runtime_error("Injected skip-token revocation failure");
                                                                            }}},
                                                                          nullptr);
  bool rejectedPowerConfirmed = false;
  bool rejectedPowerWarningClosed = false;
  bool rejectedPowerOriginalErrorShown = false;
  QTimer rejectedPowerDriver;
  rejectedPowerDriver.setInterval(1);
  connect(&rejectedPowerDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (dialog->objectName() == QStringLiteral("powerConfirmDialog")) {
      auto* shutdown = dialog->findChild<QPushButton*>(QStringLiteral("dangerBtn"));
      if (!shutdown || !shutdown->isEnabled()) return;
      rejectedPowerConfirmed = true;
      QTest::mouseClick(shutdown, Qt::LeftButton);
      return;
    }
    rejectedPowerWarningClosed = dialog->windowTitle() == QStringLiteral("Safe shutdown failed");
    for (auto bodies = dialog->findChildren<ui::ContentHintBox*>(); const auto* body : std::as_const(bodies))
      if (body->toPlainText().contains(QStringLiteral("Shutdown acknowledgement failed"))) rejectedPowerOriginalErrorShown = true;
    dialog->accept();
    rejectedPowerDriver.stop();
  });
  rejectedPowerDriver.start();
  rejectedPowerController->safeShutdown();
  QVERIFY(rejectedPowerConfirmed);
  QVERIFY(rejectedPowerWarningClosed);
  QVERIFY(rejectedPowerOriginalErrorShown);
  QVERIFY(powerNotInvokedBeforePreshutdownToken);
  QVERIFY(powerInvokedWithPreshutdownToken);
  QCOMPARE(rejectedPowerPreshutdownStates, (std::vector<bool>{true, false}));
  const auto rejectedWrites = writes(rejectedPower);
  QCOMPARE(rejectedWrites.size(), std::size_t{1});
  QCOMPARE(rejectedWrites.front(), (compat::Request{compat::Power{compat::PowerAction::Shutdown}}));
  const auto rejectedTrace = rejectedPower.backend->trace();
  const auto rejectedAttempt = std::ranges::find_if(rejectedTrace, [](const auto& entry) { return std::holds_alternative<compat::Power>(entry.request); });
  QVERIFY(rejectedAttempt != rejectedTrace.end());
  QVERIFY(rejectedAttempt->response.failure.has_value());
  QCOMPARE(rejectedAttempt->response.effect, compat::Effect::Uncertain);

  bool emptyPreparationBlockedAction = false;
  QTimer emptyPreparationDriver;
  emptyPreparationDriver.setInterval(1);
  connect(&emptyPreparationDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* restart = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"));
    if (!restart) return;
    if (!restart->isEnabled()) {
      emptyPreparationBlockedAction = true;
      return;
    }
    QTest::mouseClick(restart, Qt::LeftButton);
    emptyPreparationDriver.stop();
  });
  emptyPreparationDriver.start();
  controller->safeRestart();
  QVERIFY(emptyPreparationBlockedAction);
  QCOMPARE(fixture.backend->snapshot().powerRequests, std::vector<compat::PowerAction>{compat::PowerAction::Restart});

  UwfMaintenanceFixture directFromSafe;

  MemoryFileStagingStore directFromSafeStore;
  directFromSafeStore.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\never-commit.bin")}};
  int directFromSafePreshutdownMarks = 0;
  auto directFromSafeController = uwf::composition::createPowerCoordinator({directFromSafe.repository(),
                                                                            directFromSafeStore,
                                                                            uwf::app::UwfCapability::Available,
                                                                            uwf::windows::systemCommitTraversalSource(),
                                                                            {[&] {
                                                                               ++directFromSafePreshutdownMarks;
                                                                               return PreshutdownControlResult::Acknowledged;
                                                                             },
                                                                             {}}},
                                                                           nullptr);
  bool directOptionAvailableDuringPreparation = false;
  QTimer directFromSafeDriver;
  directFromSafeDriver.setInterval(1);
  connect(&directFromSafeDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* safeRestart = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"));
    auto* directRestart = dialog->findChild<QPushButton*>(QStringLiteral("directRestartBtn"));
    if (!safeRestart || !directRestart) return;
    directOptionAvailableDuringPreparation = !safeRestart->isEnabled() && directRestart->isEnabled();
    approveNestedPowerConfirmation(this);
    QTest::mouseClick(directRestart, Qt::LeftButton);
    directFromSafeDriver.stop();
  });
  directFromSafeDriver.start();
  directFromSafeController->safeRestart();
  QVERIFY(directOptionAvailableDuringPreparation);
  QCOMPARE(directFromSafeStore.loads, 0);
  QCOMPARE(directFromSafe.backend->snapshot().powerRequests, std::vector<compat::PowerAction>{compat::PowerAction::Restart});
  QCOMPARE(directFromSafePreshutdownMarks, 1);

  UwfMaintenanceFixture directShutdown;

  MemoryFileStagingStore directShutdownStore;
  directShutdownStore.entries = directFromSafeStore.entries;
  auto directShutdownController = uwf::composition::createPowerCoordinator(
      {directShutdown.repository(), directShutdownStore, uwf::app::UwfCapability::Available, uwf::windows::systemCommitTraversalSource()}, nullptr);
  bool directShutdownConfirmed = false;
  QTimer directShutdownDriver;
  directShutdownDriver.setInterval(1);
  connect(&directShutdownDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* directShutdown = dialog->findChild<QPushButton*>(QStringLiteral("directShutdownBtn"));
    if (!directShutdown || !directShutdown->isEnabled()) return;
    directShutdownConfirmed = true;
    approveNestedPowerConfirmation(this);
    QTest::mouseClick(directShutdown, Qt::LeftButton);
    directShutdownDriver.stop();
  });
  directShutdownDriver.start();
  directShutdownController->safeShutdown();
  QVERIFY(directShutdownConfirmed);
  QCOMPARE(directShutdownStore.loads, 0);
  QCOMPARE(directShutdown.backend->snapshot().powerRequests, std::vector<compat::PowerAction>{compat::PowerAction::Shutdown});

  UwfMaintenanceFixture direct;

  MemoryFileStagingStore directStore;
  directStore.entries = directFromSafeStore.entries;
  auto directController = uwf::composition::createPowerCoordinator(
      {direct.repository(), directStore, uwf::app::UwfCapability::Available, uwf::windows::systemCommitTraversalSource()}, nullptr);
  bool directDialogConfirmed = false;
  QTimer directDriver;
  directDriver.setInterval(1);
  connect(&directDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* directRestart = test::buttonByRole(dialog, QDialogButtonBox::AcceptRole);
    if (!directRestart || !directRestart->isEnabled()) return;
    directDialogConfirmed =
        dialog->windowTitle() == QStringLiteral("Direct restart?") && dialog->findChild<QPushButton*>(QStringLiteral("restartBtn")) == nullptr;
    QTest::mouseClick(directRestart, Qt::LeftButton);
    directDriver.stop();
  });
  directDriver.start();
  directController->directRestart();
  QVERIFY(directDialogConfirmed);
  QCOMPARE(directStore.loads, 0);
  QCOMPARE(direct.backend->snapshot().powerRequests, std::vector<compat::PowerAction>{compat::PowerAction::Restart});

  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString stagedPath = QDir::toNativeSeparators(directory.filePath(QStringLiteral("staged.bin")));
  QFile stagedFile(stagedPath);
  QVERIFY(stagedFile.open(QIODevice::WriteOnly));
  QVERIFY(stagedFile.write("staged") > 0);
  stagedFile.close();
  QVERIFY(QDir(directory.path()).mkpath(QStringLiteral("nested")));
  const QString nestedPath = QDir::toNativeSeparators(directory.filePath(QStringLiteral("nested/second.bin")));
  QFile nestedFile(nestedPath);
  QVERIFY(nestedFile.open(QIODevice::WriteOnly));
  QVERIFY(nestedFile.write("nested") > 0);
  nestedFile.close();
  const QString thirdPath = QDir::toNativeSeparators(directory.filePath(QStringLiteral("nested/third.bin")));
  QFile thirdFile(thirdPath);
  QVERIFY(thirdFile.open(QIODevice::WriteOnly));
  QVERIFY(thirdFile.write("third") > 0);
  thirdFile.close();
  const QString drive = QString::fromStdString(drive::fromPath(stagedPath.toStdString()));
  QVERIFY(!drive.isEmpty());

  UwfMaintenanceFixture staged;
  (void)staged.useFileSystemTarget(*windows::systemCommitTraversalSource(), stagedPath);
  staged.stageFiles({stagedPath, nestedPath, thirdPath});

  MemoryFileStagingStore stagedStore;
  stagedStore.entries = {{app::FileStagingKind::File, stagedPath}, {app::FileStagingKind::Directory, QDir::toNativeSeparators(directory.path())}};
  std::vector<bool> stagedPreshutdownStates;
  auto stagedController = uwf::composition::createPowerCoordinator({staged.repository(),
                                                                    stagedStore,
                                                                    uwf::app::UwfCapability::Available,
                                                                    uwf::windows::systemCommitTraversalSource(),
                                                                    {[&] {
                                                                       stagedPreshutdownStates.push_back(true);
                                                                       return PreshutdownControlResult::Acknowledged;
                                                                     },
                                                                     [&] {
                                                                       stagedPreshutdownStates.push_back(false);
                                                                       return PreshutdownControlResult::Acknowledged;
                                                                     }}},
                                                                   nullptr);
  bool calculationStateVisible = false;
  bool expandedFileCountVisible = false;
  QTimer stagedDriver;
  stagedDriver.setInterval(1);
  connect(&stagedDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* restart = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"));
    auto* stagingDetail = dialog->findChild<QLabel*>(QStringLiteral("powerStagingDetail"));
    if (!restart || !stagingDetail) return;
    if (!restart->isEnabled()) {
      const auto* count = dialog->findChild<QLabel*>(QStringLiteral("powerFileCount"));
      calculationStateVisible = count && count->isVisible() && count->text() == QStringLiteral("Counting staged files…");
      return;
    }
    if (stagingDetail->isVisible()) {
      const auto* count = dialog->findChild<QLabel*>(QStringLiteral("powerFileCount"));
      expandedFileCountVisible = count && count->text() == QStringLiteral("3");
    }
    QTest::mouseClick(restart, Qt::LeftButton);
    stagedDriver.stop();
  });
  stagedDriver.start();
  stagedController->safeRestart();
  QVERIFY(calculationStateVisible);
  QVERIFY(expandedFileCountVisible);
  QCOMPARE(staged.state->commits.size(), std::size_t{3});
  QCOMPARE(staged.backend->snapshot().powerRequests, std::vector<compat::PowerAction>{compat::PowerAction::Restart});
  const auto stagedWrites = writes(staged);
  QCOMPARE(stagedWrites.size(), std::size_t{4});
  for (std::size_t index = 0; index < 3; ++index) QVERIFY(std::holds_alternative<compat::CommitFile>(stagedWrites[index]));
  QCOMPARE(stagedWrites.back(), (compat::Request{compat::Power{compat::PowerAction::Restart}}));
  QCOMPARE(stagedPreshutdownStates, std::vector<bool>{true});

  UwfMaintenanceFixture failed;
  (void)failed.useFileSystemTarget(*windows::systemCommitTraversalSource(), stagedPath);
  failed.stageFiles({stagedPath, nestedPath, thirdPath});

  failed.state->beforeCommit = [state = failed.state] {
    if (state->commits.size() == 1)
      throw app::OperationError(std::make_error_code(std::errc::permission_denied), "provider failure", app::WriteEffect::Uncertain);
  };
  auto failedController = uwf::composition::createPowerCoordinator(
      {failed.repository(), stagedStore, uwf::app::UwfCapability::Available, uwf::windows::systemCommitTraversalSource()}, nullptr);
  QDialog* failedDialog = nullptr;
  bool failedActionClicked = false;
  bool failureDetailsVisible = false;
  bool failureStayedInDialog = false;
  QTimer failureDriver;
  failureDriver.setInterval(1);
  connect(&failureDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (!failedDialog) failedDialog = dialog;
    if (auto* details = visiblePowerFailureDetails(dialog); details && details->isVisible()) {
      failureDetailsVisible = details->toPlainText().contains(QStringLiteral("provider failure"));
      failureStayedInDialog = dialog == failedDialog;
      dialog->reject();
      failureDriver.stop();
      return;
    }
    if (!failedActionClicked) {
      if (auto* shutdown = dialog->findChild<QPushButton*>(QStringLiteral("dangerBtn"))) {
        if (!shutdown->isEnabled()) return;
        failedActionClicked = true;
        QTest::mouseClick(shutdown, Qt::LeftButton);
      }
    }
  });
  failureDriver.start();
  failedController->safeShutdown();
  QCOMPARE(failed.state->commits.size(), std::size_t{3});
  QVERIFY(failed.backend->snapshot().powerRequests.empty());
  QVERIFY(failureDetailsVisible);
  QVERIFY(failureStayedInDialog);

  UwfMaintenanceFixture continued;
  (void)continued.useFileSystemTarget(*windows::systemCommitTraversalSource(), stagedPath);
  continued.stageFiles({stagedPath, nestedPath, thirdPath});

  continued.state->beforeCommit = [state = continued.state] {
    if (state->commits.size() == 1)
      throw app::OperationError(std::make_error_code(std::errc::permission_denied), "provider failure", app::WriteEffect::Uncertain);
  };
  auto continuedController = uwf::composition::createPowerCoordinator(
      {continued.repository(), stagedStore, uwf::app::UwfCapability::Available, uwf::windows::systemCommitTraversalSource()}, nullptr);
  bool continueActionClicked = false;
  bool continueDecisionClicked = false;
  QTimer continueDriver;
  continueDriver.setInterval(1);
  connect(&continueDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* details = visiblePowerFailureDetails(dialog);
    auto* shutdown = dialog->findChild<QPushButton*>(QStringLiteral("dangerBtn"));
    if (details && details->isVisible() && shutdown) {
      continueDecisionClicked = true;
      QTest::mouseClick(shutdown, Qt::LeftButton);
      continueDriver.stop();
      return;
    }
    if (!continueActionClicked && shutdown) {
      if (!shutdown->isEnabled()) return;
      continueActionClicked = true;
      QTest::mouseClick(shutdown, Qt::LeftButton);
    }
  });
  continueDriver.start();
  continuedController->safeShutdown();
  QVERIFY(continueDecisionClicked);
  QCOMPARE(continued.state->commits.size(), std::size_t{3});
  QCOMPARE(continued.backend->snapshot().powerRequests, std::vector<compat::PowerAction>{compat::PowerAction::Shutdown});

  UwfMaintenanceFixture disabled;
  disabled.state->filterEnabled = false;

  auto disabledController = uwf::composition::createPowerCoordinator(
      {disabled.repository(), stagedStore, uwf::app::UwfCapability::Available, uwf::windows::systemCommitTraversalSource()}, nullptr);
  bool noFilesToCommitVisible = false;
  bool disabledActionInitiallyBlocked = false;
  QTimer disabledDriver;
  disabledDriver.setInterval(1);
  connect(&disabledDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* shutdown = dialog->findChild<QPushButton*>(QStringLiteral("dangerBtn"));
    auto* stagingDetail = dialog->findChild<QLabel*>(QStringLiteral("powerStagingDetail"));
    if (!shutdown || !stagingDetail) return;
    if (!shutdown->isEnabled()) {
      disabledActionInitiallyBlocked = true;
      return;
    }
    const auto* count = dialog->findChild<QLabel*>(QStringLiteral("powerFileCount"));
    noFilesToCommitVisible = count && count->isVisible() && count->text() == QStringLiteral("No files to commit");
    QTest::mouseClick(shutdown, Qt::LeftButton);
    disabledDriver.stop();
  });
  disabledDriver.start();
  disabledController->safeShutdown();
  QVERIFY(disabledActionInitiallyBlocked);
  QVERIFY(noFilesToCommitVisible);
  QCOMPARE(disabled.backend->snapshot().powerRequests, std::vector<compat::PowerAction>{compat::PowerAction::Shutdown});
}

void ApplicationCommitPowerTests::commitBatchKeepsApiFailuresAlongsideExistenceObservations() {
  QStringList categories;
  QStringList reasons;
  QTimer watcher;
  watcher.setInterval(5);
  connect(&watcher, &QTimer::timeout, &watcher, [&watcher, &categories, &reasons] {
    for (auto widgets = QApplication::topLevelWidgets(); auto* widget : std::as_const(widgets)) {
      auto* dialog = qobject_cast<QDialog*>(widget);
      auto* table = dialog ? dialog->findChild<QTableView*>() : nullptr;
      if (!table || table->model()->columnCount() != 6 || table->model()->rowCount() != 3) continue;
      for (int row = 0; row < table->model()->rowCount(); ++row) {
        categories.append(table->model()->index(row, 0).data().toString());
        reasons.append(table->model()->index(row, 5).data().toString());
      }
      watcher.stop();
      dialog->accept();
      return;
    }
  });
  watcher.start();

  const QList<QString> targets{QStringLiteral("missing"), QStringLiteral("confirmed"), QStringLiteral("still-present")};
  QMap<QString, int> probes;
  QStringList attempts;
  auto displayOf = [](const QString& target) { return target; };
  auto commitOne = [&attempts](const QString& target) {
    attempts.push_back(target);
    if (target == QStringLiteral("missing")) throw app::OperationError(std::make_error_code(std::errc::no_such_file_or_directory), "target not found");
  };
  auto exists = [&probes](const QString& target) {
    const int observation = probes[target]++;
    if (target == QStringLiteral("missing")) return false;
    if (target == QStringLiteral("confirmed")) return observation == 0;
    return true;
  };
  const auto batch = test::makeCommitBatchRequest(app::CommitOperation::DeleteAndPersist, targets, displayOf, commitOne, exists);
  QList<app::CommitNode> reportTargets;
  for (const auto& path : targets) reportTargets.append({app::CommitTargetKind::File, path, {}, {}});
  presentation::CommitBatchViewModel execution(batch, reportTargets);
  execution.run({});
  ui::showCommitReport(nullptr, execution.rows(), batch.operation, {uwf::presentation::CommitCompletion::Cause::Canceled, execution.untouched()});

  QCOMPARE(attempts, targets);
  QCOMPARE(categories.size(), 3);
  QCOMPARE(categories[0], QStringLiteral("Failed"));
  QCOMPARE(categories[1], QStringLiteral("Succeeded"));
  QCOMPARE(categories[2], QStringLiteral("Failed"));
  QCOMPARE(reasons[0], ui::I18n::tr("The system did not find the target."));
  QVERIFY(reasons[2].contains(QStringLiteral("still exists")));
}

void ApplicationCommitPowerTests::powerButtonVisualStates_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  for (const auto& locale : ui::I18n::languages()) QTest::newRow(qUtf8Printable(QString(locale.code))) << locale.id;
}

void ApplicationCommitPowerTests::powerButtonContentAlignment_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<QString>("screenName");
  for (auto screens = QApplication::screens(); const auto* screen : std::as_const(screens))
    for (const auto locale : {ui::I18n::Lang::En, ui::I18n::Lang::Zh_CN})
      for (const bool dark : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(screen->name(), locale == ui::I18n::Lang::En ? "en" : "zh_CN", dark ? "dark" : "light")))
            << locale << dark << screen->name();
}

void ApplicationCommitPowerTests::powerButtonContentAlignment() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  QFETCH(QString, screenName);
  const auto language = ui::I18n::instance().lang();
  const auto theme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(language);
    ui::ThemeManager::instance().apply(theme);
  });
  ui::I18n::instance().setLang(locale);
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  QScreen* screen = nullptr;
  for (auto screens = QApplication::screens(); auto* candidate : std::as_const(screens))
    if (candidate->name() == screenName) screen = candidate;
  QVERIFY(screen);
  QWidget owner;
  owner.setScreen(screen);
  const auto ownerSize = test::boundedWindowSize(owner, QSize(800, 600));
  QVERIFY(!ownerSize.isEmpty());
  owner.setGeometry(QRect(screen->availableGeometry().center() - QPoint(ownerSize.width() / 2, ownerSize.height() / 2), ownerSize));
  owner.show();
  QVERIFY(QTest::qWaitForWindowExposed(&owner));
  for (const auto action : {app::PowerAction::Shutdown, app::PowerAction::Restart}) {
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        owner, [&] { QCOMPARE(ui::runPowerActionDialog(&owner, {action}), app::PowerActionDialogOutcome::Canceled); },
        [&](QDialog& modal) {
          auto* dialog = &modal;
          QVERIFY(QTest::qWaitForWindowExposed(dialog));
          auto* safe = dialog->findChild<QPushButton*>(action == app::PowerAction::Shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn"));
          QVERIFY(safe);
          QCOMPARE(dialog->screen(), screen);
          QSignalSpy clicks(safe, &QPushButton::clicked);
          const auto check = [&](const QString& state) {
            const auto frame = safe->grab().toImage();
            const qreal scale = frame.devicePixelRatio();
            const QRectF backgroundRegion(safe->width() * test::kVisualRegionInset, safe->height() * .05, safe->width() * (1 - 2 * test::kVisualRegionInset),
                                          safe->height() * .10);
            const auto background = test::regionColor(frame, backgroundRegion);
            auto foreground = ui::DesignSystem::instance().color(ui::design::Color::AccentText);
            if (!safe->isEnabled()) foreground = ui::DesignSystem::mix(background, foreground, .42);
            const auto scan = test::interiorRegion(safe->rect());
            const QRect rasterScan = QRectF(scan.topLeft() * scale, scan.size() * scale).toAlignedRect();
            const int minimumGap = std::max(1, qRound(safe->iconSize().width() * test::kVisualRegionInset * scale));
            const auto runs = test::horizontalInkRuns(frame, foreground, background, rasterScan, minimumGap);
            QVERIFY2(runs.size() >= 2, qUtf8Printable(state));
            const QRectF icon = test::logicalBounds(runs.front(), scale);
            QRect textPixels;
            for (qsizetype i = 1; i < runs.size(); ++i) textPixels = textPixels.united(runs[i]);
            const auto text = test::logicalBounds(textPixels, scale);
            const auto detail = QStringLiteral("%1, DPR %2: icon %3, text %4, button %5")
                                    .arg(state)
                                    .arg(scale)
                                    .arg(icon.center().y())
                                    .arg(text.center().y())
                                    .arg(safe->height() / 2.0);
            QVERIFY2(test::centersAligned(icon, safe->rect()), qUtf8Printable(detail));
            QVERIFY2(test::centersAligned(text, icon), qUtf8Printable(detail));
            if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
              QDir output(directory + QStringLiteral("/power-content/") + QString::fromUtf8(QTest::currentDataTag()));
              QVERIFY(output.mkpath(QStringLiteral(".")));
              QVERIFY(frame.save(output.filePath(QStringLiteral("%1-%2.png").arg(static_cast<int>(action)).arg(state))));
            }
          };
          uwf::test::moveMouse(dialog, QPoint(3, dialog->height() / 2));
          QTest::qWait(160);
          check(QStringLiteral("rest"));
          uwf::test::moveMouse(safe);
          QTest::qWait(160);
          check(QStringLiteral("hover"));
          uwf::test::pressMouse(safe, Qt::LeftButton);
          QTest::qWait(160);
          check(QStringLiteral("pressed"));
          uwf::test::releaseMouse(safe, Qt::LeftButton, Qt::NoModifier, QPoint(-5, -5));
          QTest::keyClick(dialog, Qt::Key_Tab);
          safe->setFocus(Qt::OtherFocusReason);
          QTest::qWait(160);
          check(QStringLiteral("keyboard"));
          safe->setEnabled(false);
          QTest::qWait(160);
          check(QStringLiteral("disabled"));
          QCOMPARE(clicks.count(), 0);
          inspected = true;
        }));
    QVERIFY(inspected);
  }
}

void ApplicationCommitPowerTests::powerButtonVisualStates() {
  QFETCH(ui::I18n::Lang, locale);
  const auto language = ui::I18n::instance().lang();
  const auto theme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(language);
    ui::ThemeManager::instance().apply(theme);
  });
  ui::I18n::instance().setLang(locale);
  // Use LTR, CJK and RTL representatives for native input; screenshot export covers every locale.
  const bool inspectInteraction = locale == ui::I18n::Lang::En || locale == ui::I18n::Lang::Zh_CN || locale == ui::I18n::Lang::Ar ||
                                  !qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR").isEmpty();
  for (const auto action : {app::PowerAction::Shutdown, app::PowerAction::Restart}) {
    bool inspected = false;
    QTimer::singleShot(0, this, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      QVERIFY(dialog);
      const auto close = qScopeGuard([dialog] { dialog->reject(); });
      QVERIFY(QTest::qWaitForWindowExposed(dialog));
      dialog->raise();
      dialog->activateWindow();
      QVERIFY(QTest::qWaitForWindowActive(dialog));
      auto* safe = dialog->findChild<QPushButton*>(action == app::PowerAction::Shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn"));
      auto* direct =
          dialog->findChild<QPushButton*>(action == app::PowerAction::Shutdown ? QStringLiteral("directShutdownBtn") : QStringLiteral("directRestartBtn"));
      auto* cancel = test::buttonByRole(dialog, QDialogButtonBox::RejectRole);
      QVERIFY(safe && direct && cancel && cancel->isDefault());
      for (const bool dark : {false, true}) {
        ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
        safe->setEnabled(true);
        direct->setEnabled(true);
        cancel->setFocus(Qt::OtherFocusReason);
        QVERIFY(test::waitForLayoutSettled(*dialog));
        const auto safeText = action == app::PowerAction::Shutdown ? ui::I18n::tr("Safe shutdown") : ui::I18n::tr("Safe restart");
        QCOMPARE(dialog->windowTitle(), safeText);
        QCOMPARE(safe->text(), safeText);
        QCOMPARE(direct->text(), (action == app::PowerAction::Shutdown ? ui::I18n::tr("Direct shutdown") : ui::I18n::tr("Direct restart")) + QChar(0x2026));
        QCOMPARE(cancel->text(), ui::I18n::tr("Cancel"));
        for (auto* button : {safe, direct, cancel}) {
          QVERIFY(dialog->rect().contains(QRect(button->mapTo(dialog, QPoint()), button->size())));
          QVERIFY(button->width() >= button->minimumSizeHint().width());
        }
        const QString prefix = QStringLiteral("interaction-power-%1-%2-").arg(static_cast<int>(action)).arg(dark ? "dark" : "light");
        QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + QStringLiteral("default")));
        if (inspectInteraction) {
          for (const auto& [name, button] :
               {std::pair{QStringLiteral("safe"), safe}, std::pair{QStringLiteral("direct"), direct}, std::pair{QStringLiteral("cancel"), cancel}}) {
            uwf::test::moveMouse(button);
            QTRY_VERIFY2(button->underMouse(), qUtf8Printable(prefix + name));
            QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + name + QStringLiteral("-hover"), true));
            uwf::test::pressMouse(button, Qt::LeftButton);
            QVERIFY(button->isDown());
            QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + name + QStringLiteral("-pressed"), true));
            uwf::test::releaseMouse(button, Qt::LeftButton, Qt::NoModifier, QPoint(-5, -5));
            QVERIFY(!button->isDown());
            button->setFocus(Qt::TabFocusReason);
            QTRY_VERIFY(button->hasFocus());
            QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + name + QStringLiteral("-focus")));
          }
        }
        safe->setEnabled(false);
        direct->setEnabled(false);
        QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + QStringLiteral("disabled")));
      }
      uwf::test::clickMouse(cancel, Qt::LeftButton);
      QVERIFY(!dialog->isVisible());
      inspected = true;
    });
    QCOMPARE(ui::runPowerActionDialog(nullptr, {action}), app::PowerActionDialogOutcome::Canceled);
    QVERIFY(inspected);
  }
}

void ApplicationCommitPowerTests::powerFlowVisualStates_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("recordedTargets");
  for (const auto& locale : ui::I18n::languages())
    for (const bool recordedTargets : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString(locale.code), recordedTargets ? "files" : "batch"))) << locale.id << recordedTargets;
}

void ApplicationCommitPowerTests::powerFlowVisualStates() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, recordedTargets);
  const auto language = ui::I18n::instance().lang();
  const auto theme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(language);
    ui::ThemeManager::instance().apply(theme);
  });
  ui::I18n::instance().setLang(locale);
  const QString failedPath = QStringLiteral("C:\\Data\\state.bin");
  const QString failure = QStringLiteral("Mock file staging failure\nOriginal provider details <literal & complete>.\n第二行错误详情。");
  for (const auto action : {app::PowerAction::Shutdown, app::PowerAction::Restart}) {
    bool preparationReady = false;
    bool commitFails = false;
    bool inspected = false;
    int advances = 0;
    QTimer::singleShot(0, this, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      QVERIFY(dialog);
      const auto close = qScopeGuard([&] {
        preparationReady = true;
        commitFails = true;
        QTimer::singleShot(100, dialog, [dialog] { dialog->reject(); });
      });
      auto* safe = dialog->findChild<QPushButton*>(action == app::PowerAction::Shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn"));
      QVERIFY(safe && !safe->isEnabled());
      const QString prefix = QStringLiteral("interaction-power-flow-%1-").arg(static_cast<int>(action));
      QVERIFY(capturePowerThemes(*dialog, prefix + QStringLiteral("preparing"), true));
      preparationReady = true;
      QTRY_VERIFY_WITH_TIMEOUT(safe->isEnabled(), 5000);
      QVERIFY(capturePowerThemes(*dialog, prefix + QStringLiteral("ready")));
      QTest::mouseClick(safe, Qt::LeftButton);
      QTRY_VERIFY(advances > 0);
      QVERIFY(!safe->isEnabled());
      QVERIFY(capturePowerThemes(*dialog, prefix + QStringLiteral("committing")));
      commitFails = true;
      auto* details = dialog->findChild<ui::ContentHintBox*>(QStringLiteral("powerStagingFailureDetails"));
      QVERIFY(details);
      auto* disclosure = dialog->findChild<ui::Disclosure*>(QStringLiteral("powerFailureDisclosure"));
      QVERIFY(disclosure);
      QTRY_VERIFY(disclosure->isVisible());
      QCOMPARE(disclosure->isOpen(), !recordedTargets);
      if (recordedTargets) QTest::mouseClick(disclosure->toggleButton(), Qt::LeftButton);
      QTRY_VERIFY(details->isVisible());
      QVERIFY(details->toPlainText().contains(failure));
      if (recordedTargets) QVERIFY(details->toPlainText().contains(failedPath));
      QCOMPARE(safe->isEnabled(), recordedTargets);
      QCOMPARE(safe->isVisibleTo(dialog), recordedTargets);
      auto* cancel = test::buttonByRole(dialog, QDialogButtonBox::RejectRole);
      QVERIFY(cancel && cancel->isDefault());
      QCOMPARE(cancel->text(), recordedTargets ? ui::I18n::tr("Cancel") : ui::I18n::tr("Close"));
      QVERIFY(capturePowerThemes(*dialog, prefix + QStringLiteral("failure")));
      dialog->reject();
      inspected = true;
    });
    app::PowerActionDialogRequest request{action, [&]() -> std::optional<app::PowerStagingPreparation> {
                                            if (!preparationReady) return std::nullopt;
                                            return app::PowerStagingCommit{
                                                2, [&]() -> app::PowerStagingProgress {
                                                  ++advances;
                                                  if (!commitFails) return {app::PowerStagingState::InProgress, 1, 2, failedPath, {}};
                                                  std::optional<app::FileStagingCommitResult> result;
                                                  if (recordedTargets) {
                                                    result.emplace();
                                                    result->discoveredFiles = 2;
                                                    result->committedFiles = 1;
                                                    result->failures.append({failedPath, app::FileStagingCommitFailureKind::ProviderFailure, failure});
                                                  }
                                                  return {app::PowerStagingState::Failed, recordedTargets ? 2u : 1u, 2, failedPath, failure, std::move(result)};
                                                }};
                                          }};
    // Disable minimum display time for the visual matrix; timer behavior is tested separately.
    QCOMPARE(ui::runPowerActionDialog(nullptr, std::move(request), std::chrono::milliseconds::zero()),
             app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
    QVERIFY(inspected);
  }
}

QTEST_MAIN(ApplicationCommitPowerTests)
#include "CommitPowerCases.moc"
