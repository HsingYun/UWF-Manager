// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QSignalSpy>
#include <QTableView>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTimer>
#include <QUuid>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ApplicationUiFixture.h"
#include "ApplicationUiTest.h"
#include "app/api/UwfCapability.h"
#include "app/commit/CommitBatch.h"
#include "app/configuration/UwfmgrCommands.h"
#include "app/ports/FileStagingStore.h"
#include "app/ports/VolumePaths.h"
#include "app/registry/RegistryTreeCommitter.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "bootstrap/MainWindowComposition.h"
#include "bootstrap/ViewModelComposition.h"
#include "bootstrap/WorkflowComposition.h"
#include "core/UwfModel.h"
#include "presentation/commit/CommitBatchViewModel.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/staging/FileStagingResultText.h"
#include "shared/Log.h"
#include "support/ui/CommitDialogDriver.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/InkBounds.h"
#include "support/visual/WidgetCapture.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/commit/CommitDispatcher.h"
#include "ui/commit/CommitReportDialog.h"
#include "ui/configuration/VolumeTab.h"
#include "ui/design/DesignSystem.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/overlay/OverlayHudPalette.h"
#include "ui/power/PowerConfirmDialog.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/Disclosure.h"
#include "windows/commit/WindowsCommitTraversalSource.h"
#include "windows/system/SystemHardwareInfo.h"
#include "windows/system/WindowsShell.h"

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
  void commitDispatcherRejectsUnaddressablePathsAndRestoresTheUsageTimer();
  void commitDispatcherConfirmsAndReportsARealFileThroughTheTransportBoundary();
  void commitDispatcherRoutesAnExistingRegistryValueThroughTheTransportBoundary();
  void safePowerActionsRequireConfirmationAndUseTheInjectedTransport();
  void commitBatchUsesAuthoritativeExistenceForEveryOutcome();
};

using namespace uwf;
using namespace uwf::test::application;
namespace {
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
  const core::VolumeInfo disk{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  ui::VolumeTab tab(std::make_shared<presentation::VolumeConfigurationViewModel>(disk, true), files, nullptr, uwf::bootstrap::createViewModelFactory());
  tab.viewModel().applySnapshot(editableSnapshot());
  tab.resize(900, 600);
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

void ApplicationCommitPowerTests::commitDispatcherRejectsUnaddressablePathsAndRestoresTheUsageTimer() {
  RecordingWmiOperations wmi;
  const auto snapshot = editableSnapshot();
  QTimer usage;
  usage.start(60'000);
  auto dispatcher = uwf::bootstrap::createCommitDispatcher(wmi, snapshot, &usage, nullptr);

  dispatcher->commitFilePath({});
  QVERIFY(usage.isActive());
  QTimer::singleShot(0, this, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->accept();
  });
  dispatcher->commitFilePath(QStringLiteral("relative/path.txt"));
  QVERIFY(usage.isActive());
  QVERIFY(wmi.invocations.empty());
  QVERIFY(wmi.queryResults.empty());
}

void ApplicationCommitPowerTests::commitDispatcherConfirmsAndReportsARealFileThroughTheTransportBoundary() {
  QTemporaryFile file(QDir::temp().filePath(QStringLiteral("uwf-commit-test-XXXXXX.tmp")));
  QVERIFY(file.open());
  QVERIFY(file.write("committed payload") > 0);
  QVERIFY(file.flush());
  const QString path = QDir::toNativeSeparators(file.fileName());
  const QString drive = uwf::windows::shell::extractDriveLetter(path);
  QVERIFY(!drive.isEmpty());

  RecordingWmiOperations wmi;
  wmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                               {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                               {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{test}")},
                               {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                               {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  const auto snapshot = editableSnapshot();
  QTimer usage;
  usage.start(60'000);
  auto dispatcher = uwf::bootstrap::createCommitDispatcher(wmi, snapshot, &usage, nullptr);

  bool confirmationAccepted = false;
  bool reportObserved = false;
  bool reportSucceeded = false;
  QTimer modalDriver;
  modalDriver.setInterval(1);
  connect(&modalDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
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
      for (const auto buttons = box->buttons(); auto* button : buttons) {
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
  QVERIFY(usage.isActive());
  QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("CommitFile")});
  QCOMPARE(wmi.invocationPaths, std::vector<QString>{QStringLiteral("volume-path")});
  QCOMPARE(wmi.invocationInputs.size(), std::size_t{1});
  const QString normalizedTarget = QString::fromStdString(wmi.invocationInputs.front().at("FileName").toString());
  QVERIFY(normalizedTarget.startsWith(QLatin1Char('\\')));
  QVERIFY(path.endsWith(normalizedTarget, Qt::CaseInsensitive));

  wmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                               {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                               {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{test}")},
                               {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                               {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  QTimer::singleShot(0, this, [] {
    if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->reject();
  });
  dispatcher->commitFilePath(path);
  QCOMPARE(wmi.invocations.size(), std::size_t{1});
  QVERIFY(usage.isActive());
}

void ApplicationCommitPowerTests::commitDispatcherRoutesAnExistingRegistryValueThroughTheTransportBoundary() {
  const QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
  const QString valueName = QStringLiteral("ProductName");
  RecordingWmiOperations wmi;
  wmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("registry-path")},
                               {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"PersistDomainSecretKey", uwf::app::wmi::WmiValue::fromBool(false)},
                               {"PersistTSCAL", uwf::app::wmi::WmiValue::fromBool(false)}}});
  const auto snapshot = editableSnapshot();
  QTimer usage;
  usage.start(60'000);
  auto dispatcher = uwf::bootstrap::createCommitDispatcher(wmi, snapshot, &usage, nullptr);

  bool confirmationAccepted = false;
  bool reportSucceeded = false;
  bool reportObserved = false;
  QTimer modalDriver;
  modalDriver.setInterval(1);
  connect(&modalDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
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
      for (const auto buttons = box->buttons(); auto* button : buttons) {
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
  QVERIFY(usage.isActive());
  QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("CommitRegistry")});
  QCOMPARE(wmi.invocationInputs.size(), std::size_t{1});
  QCOMPARE(QString::fromStdString(wmi.invocationInputs.front().at("RegistryKey").toString()), key);
  QCOMPARE(QString::fromStdString(wmi.invocationInputs.front().at("ValueName").toString()), valueName);
}

void ApplicationCommitPowerTests::safePowerActionsRequireConfirmationAndUseTheInjectedTransport() {
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  auto coordinatorOwner = uwf::bootstrap::createFileStagingCoordinator(wmi, staging, uwf::app::UwfCapability::Available, nullptr);
  auto& coordinator = *coordinatorOwner;
  auto controller = uwf::bootstrap::createPowerCoordinator(
      {wmi, staging, uwf::app::UwfCapability::Available, coordinator, uwf::windows::systemCommitTraversalSource()}, nullptr);

  QTimer::singleShot(0, this, [] {
    if (auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget())) dialog->reject();
  });
  controller->safeShutdown();
  QVERIFY(wmi.invocations.empty());
  QVERIFY(wmi.queryResults.empty());

  RecordingWmiOperations unreadableWmi;
  MemoryFileStagingStore unreadableStore;
  unreadableStore.readFailure = QStringLiteral("registry staging data is corrupt");
  auto unreadableCoordinatorOwner = uwf::bootstrap::createFileStagingCoordinator(unreadableWmi, unreadableStore, uwf::app::UwfCapability::Available, nullptr);
  auto& unreadableCoordinator = *unreadableCoordinatorOwner;
  auto unreadableController = uwf::bootstrap::createPowerCoordinator(
      {unreadableWmi, unreadableStore, uwf::app::UwfCapability::Available, unreadableCoordinator, uwf::windows::systemCommitTraversalSource()}, nullptr);
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
  QVERIFY(unreadableWmi.queryResults.empty());
  QVERIFY(unreadableWmi.invocations.empty());

  RecordingWmiOperations approvedUnreadableWmi;
  approvedUnreadableWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                                 {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                                 {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  MemoryFileStagingStore approvedUnreadableStore;
  approvedUnreadableStore.readFailure = unreadableStore.readFailure;
  auto approvedUnreadableCoordinatorOwner =
      uwf::bootstrap::createFileStagingCoordinator(approvedUnreadableWmi, approvedUnreadableStore, uwf::app::UwfCapability::Available, nullptr);
  auto& approvedUnreadableCoordinator = *approvedUnreadableCoordinatorOwner;
  auto approvedUnreadableController =
      uwf::bootstrap::createPowerCoordinator({approvedUnreadableWmi, approvedUnreadableStore, uwf::app::UwfCapability::Available, approvedUnreadableCoordinator,
                                              uwf::windows::systemCommitTraversalSource()},
                                             nullptr);
  bool unreadableContinuationBlocked = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QVERIFY(dialog);
    const auto close = qScopeGuard([dialog] {
      if (dialog->isVisible()) dialog->reject();
    });
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
  });
  approvedUnreadableController->safeShutdown();
  QVERIFY(unreadableContinuationBlocked);
  QVERIFY(approvedUnreadableWmi.invocations.empty());
  QCOMPARE(approvedUnreadableWmi.queryResults.size(), std::size_t{1});

  RecordingWmiOperations rejectedPowerWmi;
  rejectedPowerWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                            {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                            {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  rejectedPowerWmi.invocationFailure = std::make_exception_ptr(uwf::app::wmi::WmiProviderError(5, "request shutdown", "provider rejected shutdown"));
  auto rejectedPowerCoordinatorOwner = uwf::bootstrap::createFileStagingCoordinator(rejectedPowerWmi, staging, uwf::app::UwfCapability::Available, nullptr);
  auto& rejectedPowerCoordinator = *rejectedPowerCoordinatorOwner;
  std::vector<bool> rejectedPowerPreshutdownStates;
  bool filterReadBeforePreshutdownToken = false;
  using PreshutdownControlResult = app::PreshutdownControlResult;
  auto rejectedPowerController = uwf::bootstrap::createPowerCoordinator({rejectedPowerWmi,
                                                                         staging,
                                                                         uwf::app::UwfCapability::Available,
                                                                         rejectedPowerCoordinator,
                                                                         uwf::windows::systemCommitTraversalSource(),
                                                                         {[&] {
                                                                            filterReadBeforePreshutdownToken = rejectedPowerWmi.queryResults.empty();
                                                                            rejectedPowerPreshutdownStates.push_back(true);
                                                                            return PreshutdownControlResult::Acknowledged;
                                                                          },
                                                                          [&] {
                                                                            rejectedPowerPreshutdownStates.push_back(false);
                                                                            return PreshutdownControlResult::Acknowledged;
                                                                          }}},
                                                                        nullptr);
  bool rejectedPowerConfirmed = false;
  bool rejectedPowerWarningClosed = false;
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
    dialog->accept();
    rejectedPowerDriver.stop();
  });
  rejectedPowerDriver.start();
  rejectedPowerController->safeShutdown();
  QVERIFY(rejectedPowerConfirmed);
  QVERIFY(rejectedPowerWarningClosed);
  QVERIFY(filterReadBeforePreshutdownToken);
  QCOMPARE(rejectedPowerPreshutdownStates, (std::vector<bool>{true, false}));

  wmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                               {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
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
  QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("RestartSystem")});

  RecordingWmiOperations directFromSafeWmi;
  directFromSafeWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                             {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                             {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  MemoryFileStagingStore directFromSafeStore;
  directFromSafeStore.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\never-commit.bin")}};
  auto directFromSafeCoordinatorOwner =
      uwf::bootstrap::createFileStagingCoordinator(directFromSafeWmi, directFromSafeStore, uwf::app::UwfCapability::Available, nullptr);
  auto& directFromSafeCoordinator = *directFromSafeCoordinatorOwner;
  int directFromSafePreshutdownMarks = 0;
  auto directFromSafeController = uwf::bootstrap::createPowerCoordinator({directFromSafeWmi,
                                                                          directFromSafeStore,
                                                                          uwf::app::UwfCapability::Available,
                                                                          directFromSafeCoordinator,
                                                                          uwf::windows::systemCommitTraversalSource(),
                                                                          {[&] {
                                                                             ++directFromSafePreshutdownMarks;
                                                                             return PreshutdownControlResult::Acknowledged;
                                                                           },
                                                                           {}}},
                                                                         nullptr);
  bool directOptionAvailableDuringPreparation = false;
  std::optional<app::ApplicationCommandResult> skippedConcurrentCommit;
  QTimer directFromSafeDriver;
  directFromSafeDriver.setInterval(1);
  connect(&directFromSafeDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* safeRestart = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"));
    auto* directRestart = dialog->findChild<QPushButton*>(QStringLiteral("directRestartBtn"));
    if (!safeRestart || !directRestart) return;
    directOptionAvailableDuringPreparation = !safeRestart->isEnabled() && directRestart->isEnabled();
    directFromSafeCoordinator.requestCommit([&](const app::FileStagingBatchResult& result) { skippedConcurrentCommit = result.command; });
    approveNestedPowerConfirmation(this);
    QTest::mouseClick(directRestart, Qt::LeftButton);
    directFromSafeDriver.stop();
  });
  directFromSafeDriver.start();
  directFromSafeController->safeRestart();
  QVERIFY(directOptionAvailableDuringPreparation);
  QVERIFY(skippedConcurrentCommit.has_value());
  QCOMPARE(skippedConcurrentCommit->outcome, app::ApplicationCommandOutcome::Rejected);
  QCOMPARE(directFromSafeStore.loads, 0);
  QCOMPARE(directFromSafeWmi.invocations, std::vector<QString>{QStringLiteral("RestartSystem")});
  QCOMPARE(directFromSafePreshutdownMarks, 1);

  RecordingWmiOperations directShutdownWmi;
  directShutdownWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                             {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                             {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  MemoryFileStagingStore directShutdownStore;
  directShutdownStore.entries = directFromSafeStore.entries;
  auto directShutdownCoordinatorOwner =
      uwf::bootstrap::createFileStagingCoordinator(directShutdownWmi, directShutdownStore, uwf::app::UwfCapability::Available, nullptr);
  auto& directShutdownCoordinator = *directShutdownCoordinatorOwner;
  auto directShutdownController = uwf::bootstrap::createPowerCoordinator(
      {directShutdownWmi, directShutdownStore, uwf::app::UwfCapability::Available, directShutdownCoordinator, uwf::windows::systemCommitTraversalSource()},
      nullptr);
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
  QCOMPARE(directShutdownWmi.invocations, std::vector<QString>{QStringLiteral("ShutdownSystem")});

  RecordingWmiOperations directWmi;
  directWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                     {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                     {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  MemoryFileStagingStore directStore;
  directStore.entries = directFromSafeStore.entries;
  auto directCoordinatorOwner = uwf::bootstrap::createFileStagingCoordinator(directWmi, directStore, uwf::app::UwfCapability::Available, nullptr);
  auto& directCoordinator = *directCoordinatorOwner;
  auto directController = uwf::bootstrap::createPowerCoordinator(
      {directWmi, directStore, uwf::app::UwfCapability::Available, directCoordinator, uwf::windows::systemCommitTraversalSource()}, nullptr);
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
  QCOMPARE(directWmi.invocations, std::vector<QString>{QStringLiteral("RestartSystem")});

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
  const QString drive = uwf::windows::shell::extractDriveLetter(stagedPath);
  QVERIFY(!drive.isEmpty());

  RecordingWmiOperations stagedWmi;
  stagedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                     {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                     {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  stagedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                     {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                     {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                                     {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                     {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                     {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                     {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  stagedWmi.readResults.push_back(fileExclusionResult());
  stagedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                     {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                     {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  MemoryFileStagingStore stagedStore;
  stagedStore.entries = {{app::FileStagingKind::File, stagedPath}, {app::FileStagingKind::Directory, QDir::toNativeSeparators(directory.path())}};
  auto stagedCoordinatorOwner = uwf::bootstrap::createFileStagingCoordinator(stagedWmi, stagedStore, uwf::app::UwfCapability::Available, nullptr);
  auto& stagedCoordinator = *stagedCoordinatorOwner;
  std::vector<bool> stagedPreshutdownStates;
  auto stagedController = uwf::bootstrap::createPowerCoordinator({stagedWmi,
                                                                  stagedStore,
                                                                  uwf::app::UwfCapability::Available,
                                                                  stagedCoordinator,
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
  bool concurrentCommandQueued = false;
  std::optional<app::ApplicationCommandResult> concurrentCommandResult;
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
    if (!concurrentCommandQueued) {
      concurrentCommandQueued = true;
      stagedCoordinator.requestCommit([&](const app::FileStagingBatchResult& result) { concurrentCommandResult = result.command; });
      QCOMPARE(QApplication::activeModalWidget(), dialog);
    }
    QTest::mouseClick(restart, Qt::LeftButton);
    stagedDriver.stop();
  });
  stagedDriver.start();
  stagedController->safeRestart();
  QVERIFY(calculationStateVisible);
  QVERIFY(expandedFileCountVisible);
  QVERIFY(concurrentCommandQueued);
  QVERIFY(concurrentCommandResult.has_value());
  QCOMPARE(concurrentCommandResult->committedFiles, std::size_t{3});
  QCOMPARE(stagedWmi.invocations,
           std::vector<QString>({QStringLiteral("CommitFile"), QStringLiteral("CommitFile"), QStringLiteral("CommitFile"), QStringLiteral("RestartSystem")}));
  QCOMPARE(stagedWmi.invocationPaths,
           std::vector<QString>({QStringLiteral("volume-path"), QStringLiteral("volume-path"), QStringLiteral("volume-path"), QStringLiteral("filter-path")}));
  QCOMPARE(stagedPreshutdownStates, std::vector<bool>{true});

  RecordingWmiOperations failedWmi;
  failedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                     {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                     {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  failedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                     {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                     {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                                     {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                     {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                     {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                     {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  failedWmi.readResults.push_back(fileExclusionResult());
  failedWmi.invocationFailure = std::make_exception_ptr(uwf::app::wmi::WmiProviderError(5, "commit staged file", "provider failure"));
  auto failedCoordinatorOwner = uwf::bootstrap::createFileStagingCoordinator(failedWmi, stagedStore, uwf::app::UwfCapability::Available, nullptr);
  auto& failedCoordinator = *failedCoordinatorOwner;
  auto failedController = uwf::bootstrap::createPowerCoordinator(
      {failedWmi, stagedStore, uwf::app::UwfCapability::Available, failedCoordinator, uwf::windows::systemCommitTraversalSource()}, nullptr);
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
  QCOMPARE(failedWmi.invocations, std::vector<QString>({QStringLiteral("CommitFile"), QStringLiteral("CommitFile"), QStringLiteral("CommitFile")}));
  QVERIFY(failureDetailsVisible);
  QVERIFY(failureStayedInDialog);

  RecordingWmiOperations continuedWmi;
  continuedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                        {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                        {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  continuedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                        {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                        {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                                        {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                        {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                        {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                        {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  continuedWmi.readResults.push_back(fileExclusionResult());
  continuedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                        {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                        {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  continuedWmi.invocationFailure = std::make_exception_ptr(uwf::app::wmi::WmiProviderError(5, "commit staged file", "provider failure"));
  auto continuedCoordinatorOwner = uwf::bootstrap::createFileStagingCoordinator(continuedWmi, stagedStore, uwf::app::UwfCapability::Available, nullptr);
  auto& continuedCoordinator = *continuedCoordinatorOwner;
  auto continuedController = uwf::bootstrap::createPowerCoordinator(
      {continuedWmi, stagedStore, uwf::app::UwfCapability::Available, continuedCoordinator, uwf::windows::systemCommitTraversalSource()}, nullptr);
  bool continueActionClicked = false;
  bool continueDecisionClicked = false;
  bool commandQueuedDuringPowerApproval = false;
  std::optional<app::ApplicationCommandResult> commandResultAfterPowerApproval;
  QTimer continueDriver;
  continueDriver.setInterval(1);
  connect(&continueDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* details = visiblePowerFailureDetails(dialog);
    auto* shutdown = dialog->findChild<QPushButton*>(QStringLiteral("dangerBtn"));
    if (details && details->isVisible() && shutdown) {
      if (!commandQueuedDuringPowerApproval) {
        commandQueuedDuringPowerApproval = true;
        continuedCoordinator.requestCommit([&](const app::FileStagingBatchResult& result) { commandResultAfterPowerApproval = result.command; });
      }
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
  QVERIFY(commandResultAfterPowerApproval.has_value());
  QCOMPARE(commandResultAfterPowerApproval->outcome, app::ApplicationCommandOutcome::CompletedWithFailures);
  QCOMPARE(continuedWmi.invocations,
           std::vector<QString>({QStringLiteral("CommitFile"), QStringLiteral("CommitFile"), QStringLiteral("CommitFile"), QStringLiteral("ShutdownSystem")}));

  RecordingWmiOperations disabledWmi;
  disabledWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                       {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(false)},
                                       {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(false)}}});
  disabledWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                       {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(false)},
                                       {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(false)}}});
  auto disabledCoordinatorOwner = uwf::bootstrap::createFileStagingCoordinator(disabledWmi, stagedStore, uwf::app::UwfCapability::Available, nullptr);
  auto& disabledCoordinator = *disabledCoordinatorOwner;
  auto disabledController = uwf::bootstrap::createPowerCoordinator(
      {disabledWmi, stagedStore, uwf::app::UwfCapability::Available, disabledCoordinator, uwf::windows::systemCommitTraversalSource()}, nullptr);
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
  QCOMPARE(disabledWmi.invocations, std::vector<QString>{QStringLiteral("ShutdownSystem")});
}

void ApplicationCommitPowerTests::commitBatchUsesAuthoritativeExistenceForEveryOutcome() {
  QStringList categories;
  QStringList reasons;
  QTimer watcher;
  watcher.setInterval(5);
  connect(&watcher, &QTimer::timeout, &watcher, [&watcher, &categories, &reasons] {
    for (const auto widgets = QApplication::topLevelWidgets(); auto* widget : widgets) {
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
  int commits = 0;
  auto displayOf = [](const QString& target) { return target; };
  auto commitOne = [&commits](const QString&) { ++commits; };
  auto exists = [&probes](const QString& target) {
    const int observation = probes[target]++;
    if (target == QStringLiteral("missing")) return false;
    if (target == QStringLiteral("confirmed")) return observation == 0;
    return true;
  };
  const auto batch = app::makeCommitBatchRequest(targets, displayOf, commitOne, exists);
  presentation::CommitBatchViewModel execution(batch);
  execution.run({});
  ui::showCommitReport(nullptr, execution.rows(), batch.operation, {uwf::presentation::CommitCompletion::Cause::Canceled, execution.untouched()});

  QCOMPARE(commits, 2);
  QCOMPARE(categories.size(), 3);
  QCOMPARE(categories[0], QStringLiteral("Skipped"));
  QCOMPARE(categories[1], QStringLiteral("Succeeded"));
  QCOMPARE(categories[2], QStringLiteral("Failed"));
  QVERIFY(reasons[2].contains(QStringLiteral("still exists")));
}

void ApplicationCommitPowerTests::powerButtonVisualStates_data() { addLocaleThemeRows(); }

void ApplicationCommitPowerTests::powerButtonContentAlignment_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<QString>("screenName");
  for (const auto screens = QApplication::screens(); const auto* screen : screens)
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
  for (const auto screens = QApplication::screens(); auto* candidate : screens)
    if (candidate->name() == screenName) screen = candidate;
  QVERIFY(screen);
  QWidget owner;
  owner.setScreen(screen);
  owner.setGeometry(QRect(screen->availableGeometry().center() - QPoint(400, 300), QSize(800, 600)));
  owner.show();
  QVERIFY(QTest::qWaitForWindowExposed(&owner));
  for (const auto action : {app::PowerAction::Shutdown, app::PowerAction::Restart}) {
    bool inspected = false;
    QTimer::singleShot(0, this, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      QVERIFY(dialog);
      const auto close = qScopeGuard([dialog] { dialog->reject(); });
      QVERIFY(QTest::qWaitForWindowExposed(dialog));
      auto* safe = dialog->findChild<QPushButton*>(action == app::PowerAction::Shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn"));
      QVERIFY(safe);
      QCOMPARE(dialog->screen(), screen);
      QSignalSpy clicks(safe, &QPushButton::clicked);
      const auto check = [&](const QString& state) {
        const auto frame = safe->grab().toImage();
        const qreal scale = frame.devicePixelRatio();
        const auto background = frame.pixelColor(frame.width() / 2, qRound(3 * scale));
        auto foreground = ui::DesignSystem::instance().color(ui::design::Color::AccentText);
        if (!safe->isEnabled()) foreground = ui::DesignSystem::mix(background, foreground, .42);
        const auto runs =
            test::horizontalInkRuns(frame, foreground, background,
                                    frame.rect().adjusted(qRound(8 * scale), qRound(6 * scale), -qRound(8 * scale), -qRound(6 * scale)), qRound(5 * scale));
        QVERIFY2(runs.size() >= 2, qUtf8Printable(state));
        const QRect icon = runs.front();
        QRect text;
        for (qsizetype i = 1; i < runs.size(); ++i) text = text.united(runs[i]);
        const qreal iconCenter = (icon.top() + icon.bottom() + 1) / 2.0;
        const qreal textCenter = (text.top() + text.bottom() + 1) / 2.0;
        const auto detail =
            QStringLiteral("%1, DPR %2: icon %3, text %4, button %5").arg(state).arg(scale).arg(iconCenter).arg(textCenter).arg(frame.height() / 2.0);
        QVERIFY2(qAbs(iconCenter - frame.height() / 2.0) <= 1, qUtf8Printable(detail));
        QVERIFY2(qAbs(textCenter - iconCenter) <= 1, qUtf8Printable(detail));
        if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
          QDir output(directory + QStringLiteral("/power-content/") + QString::fromUtf8(QTest::currentDataTag()));
          QVERIFY(output.mkpath(QStringLiteral(".")));
          QVERIFY(frame.save(output.filePath(QStringLiteral("%1-%2.png").arg(static_cast<int>(action)).arg(state))));
        }
      };
      uwf::test::moveMouse(dialog, QPoint(3, dialog->height() / 2));
      QTest::qWait(160);
      check(QStringLiteral("rest"));
      uwf::test::moveMouse(safe, safe->rect().center());
      QTest::qWait(160);
      check(QStringLiteral("hover"));
      QTest::mousePress(safe, Qt::LeftButton);
      QTest::qWait(160);
      check(QStringLiteral("pressed"));
      QTest::mouseRelease(safe, Qt::LeftButton, Qt::NoModifier, QPoint(-5, -5));
      QTest::keyClick(dialog, Qt::Key_Tab);
      safe->setFocus(Qt::OtherFocusReason);
      QTest::qWait(160);
      check(QStringLiteral("keyboard"));
      safe->setEnabled(false);
      QTest::qWait(160);
      check(QStringLiteral("disabled"));
      QCOMPARE(clicks.count(), 0);
      inspected = true;
    });
    QCOMPARE(ui::runPowerActionDialog(&owner, {action}), app::PowerActionDialogOutcome::Canceled);
    QVERIFY(inspected);
  }
}

void ApplicationCommitPowerTests::powerButtonVisualStates() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  const auto language = ui::I18n::instance().lang();
  const auto theme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(language);
    ui::ThemeManager::instance().apply(theme);
  });
  ui::I18n::instance().setLang(locale);
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
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
      const QString prefix = QStringLiteral("interaction-power-%1-").arg(static_cast<int>(action));
      QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + QStringLiteral("default")));
      for (const auto& [name, button] :
           {std::pair{QStringLiteral("safe"), safe}, std::pair{QStringLiteral("direct"), direct}, std::pair{QStringLiteral("cancel"), cancel}}) {
        uwf::test::moveMouse(button, button->rect().center());
        QTRY_VERIFY(button->underMouse());
        QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + name + QStringLiteral("-hover"), true));
        QTest::mousePress(button, Qt::LeftButton);
        QVERIFY(button->isDown());
        QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + name + QStringLiteral("-pressed"), true));
        QTest::mouseRelease(button, Qt::LeftButton, Qt::NoModifier, QPoint(-5, -5));
        QVERIFY(!button->isDown());
        button->setFocus(Qt::TabFocusReason);
        QTRY_VERIFY(button->hasFocus());
        QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + name + QStringLiteral("-focus")));
      }
      safe->setEnabled(false);
      direct->setEnabled(false);
      QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + QStringLiteral("disabled")));
      QTest::mouseClick(cancel, Qt::LeftButton);
      QVERIFY(!dialog->isVisible());
      inspected = true;
    });
    QCOMPARE(ui::runPowerActionDialog(nullptr, {action}), app::PowerActionDialogOutcome::Canceled);
    QVERIFY(inspected);
  }
}

void ApplicationCommitPowerTests::powerFlowVisualStates_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("recordedTargets");
  for (const auto& locale : ui::I18n::languages())
    for (const bool dark : {false, true})
      for (const bool recordedTargets : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(QString(locale.code), dark ? "dark" : "light", recordedTargets ? "files" : "batch")))
            << locale.id << dark << recordedTargets;
}

void ApplicationCommitPowerTests::powerFlowVisualStates() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  QFETCH(bool, recordedTargets);
  const auto language = ui::I18n::instance().lang();
  const auto theme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(language);
    ui::ThemeManager::instance().apply(theme);
  });
  ui::I18n::instance().setLang(locale);
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
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
      QVERIFY(uwf::test::saveAnimatedScreenshot(*dialog, prefix + QStringLiteral("preparing")));
      preparationReady = true;
      QTRY_VERIFY_WITH_TIMEOUT(safe->isEnabled(), 5000);
      QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + QStringLiteral("ready")));
      QTest::mouseClick(safe, Qt::LeftButton);
      QTRY_VERIFY(advances > 0);
      QVERIFY(!safe->isEnabled());
      QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + QStringLiteral("committing")));
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
      QVERIFY(uwf::test::saveScreenshot(*dialog, prefix + QStringLiteral("failure")));
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
    QCOMPARE(ui::runPowerActionDialog(nullptr, std::move(request)), app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
    QVERIFY(inspected);
  }
}

QTEST_MAIN(ApplicationCommitPowerTests)
#include "CommitPowerCases.moc"
