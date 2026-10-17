/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTableWidget>
#include <QTemporaryFile>
#include <QTest>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <utility>

#include "core/UwfModel.h"
#include "ui/Dialogs.h"
#include "ui/ExclusionListWidget.h"
#include "ui/FlowLayout.h"
#include "ui/GlobalStatusPanel.h"
#include "ui/HoverHintController.h"
#include "ui/I18n.h"
#include "ui/ImportDialog.h"
#include "ui/PowerConfirmDialog.h"
#include "ui/StatusPanel.h"
#include "ui/SwitchButton.h"
#include "ui/TableText.h"
#include "ui/ThemeManager.h"
#include "UiTextChecks.h"
#include "ui/TransientLabel.h"

namespace {

class MemoryFileDialogs final : public uwf::ui::dialogs::FileDialogProvider {
 public:
  QStringList openedFiles;
  QString openedFile;
  QString directory;
  QString savedFile;
  QList<uwf::ui::dialogs::FileDialogRequest> requests;

  QStringList openFiles(QWidget*, const uwf::ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return openedFiles;
  }
  QString openFile(QWidget*, const uwf::ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return openedFile;
  }
  QString selectDirectory(QWidget*, const uwf::ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return directory;
  }
  QString saveFile(QWidget*, const uwf::ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return savedFile;
  }
};

QAction* actionWithText(QWidget* root, const QString& text) {
  for (const auto actions = root->findChildren<QAction*>(); auto* action : actions) {
    if (action->text() == text) return action;
  }
  return nullptr;
}

QPushButton* buttonWithText(QWidget* root, const QString& text) {
  for (const auto buttons = root->findChildren<QPushButton*>(); auto* button : buttons) {
    if (button->text() == text) return button;
  }
  return nullptr;
}

class UiBehaviorTests final : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase() { uwf::ui::I18n::instance().setLang(uwf::ui::I18n::Lang::En); }
  void switchButtonRespondsToMouseKeyboardAndDisabledState();
  void transientLabelPreservesNewBaselineUntilRestore();
  void hoverHintUsesRealWidgetEventsAndRestoresBaseline();
  void powerConfirmationDefaultsToCancel();
  void powerConfirmationEscapeAlwaysCancels();
  void powerConfirmationCanSkipStagingForDirectRestart();
  void powerConfirmationAcceptsOnlyTheExplicitAction();
  void powerDialogShowsProgressAndFailureInPlace();
  void powerDialogCompactLayoutKeepsSafeActionsOutsideScrolling_data();
  void powerDialogCompactLayoutKeepsSafeActionsOutsideScrolling();
  void groupedLayoutReflowsWithoutClippingOrRecreatingControls();
  void tableCopyReflectsTheActualSelectionAndMissingCells();
  void volumeStatusTracksOnlyChangesFromTheDisplayedSnapshot();
  void globalStatusImportProducesConstrainedPendingDelta();
  void unavailableGlobalStatusRejectsProgrammaticChanges();
  void fileExclusionImportMaintainsCaseInsensitiveDeltas();
  void fileExclusionImportRejectsWrongVolumeAndUwfBlacklist();
  void registryExclusionImportUsesTheSamePolicyAsTheUi();
  void exclusionListDoubleClickCopiesTheDisplayedFullPath();
  void exclusionListButtonsFilterAndPersistenceRowsUseDisplayedSelection();
  void exclusionFileActionsUseThePlatformBoundaryAndPreserveValidation();
  void importDialogFiltersCommentsAndAppendsBatchReports();
  void importFileActionLoadsOnlyCommandsFromEverySelectedFile();
  void recommendedConfigurationSelectionAppendsOnlyChosenGroups();
};

void UiBehaviorTests::switchButtonRespondsToMouseKeyboardAndDisabledState() {
  QWidget window;
  QVBoxLayout layout(&window);
  uwf::ui::SwitchButton button;
  layout.addWidget(&button);
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));

  QSignalSpy toggled(&button, &QAbstractButton::toggled);
  QVERIFY(!button.isChecked());
  QTest::mouseClick(&button, Qt::LeftButton);
  QVERIFY(button.isChecked());
  QCOMPARE(toggled.count(), 1);

  QTest::mouseClick(&button, Qt::RightButton);
  QVERIFY(button.isChecked());
  QCOMPARE(toggled.count(), 1);

  button.setFocus();
  QTest::keyClick(&button, Qt::Key_Space);
  QVERIFY(!button.isChecked());
  QCOMPARE(toggled.count(), 2);

  button.setEnabled(false);
  QTest::mouseClick(&button, Qt::LeftButton);
  QTest::keyClick(&button, Qt::Key_Space);
  QVERIFY(!button.isChecked());
  QCOMPARE(toggled.count(), 2);
}

void UiBehaviorTests::transientLabelPreservesNewBaselineUntilRestore() {
  QLabel label;
  uwf::ui::TransientLabel transient(&label, &label);

  transient.setBaseline(QStringLiteral("Ready"));
  QCOMPARE(label.text(), QStringLiteral("Ready"));
  transient.show(QStringLiteral("Working"));
  QVERIFY(transient.isShowing());
  QCOMPARE(label.text(), QStringLiteral("Working"));

  transient.setBaseline(QStringLiteral("2 pending changes"));
  QCOMPARE(label.text(), QStringLiteral("Working"));
  transient.restoreAfter(1);
  QTRY_COMPARE_WITH_TIMEOUT(label.text(), QStringLiteral("2 pending changes"), 100);
  QVERIFY(!transient.isShowing());

  transient.flash(QStringLiteral("Copied"), 1);
  QCOMPARE(label.text(), QStringLiteral("Copied"));
  QTRY_COMPARE_WITH_TIMEOUT(label.text(), QStringLiteral("2 pending changes"), 100);
  transient.show(QStringLiteral("Held"));
  transient.restoreAfter(0);
  QCOMPARE(label.text(), QStringLiteral("2 pending changes"));
}

void UiBehaviorTests::hoverHintUsesRealWidgetEventsAndRestoresBaseline() {
  QWidget window;
  window.resize(320, 120);
  auto* layout = new QVBoxLayout(&window);
  auto* hinted = new QPushButton(QStringLiteral("Target"), &window);
  hinted->setToolTip(QStringLiteral("Action description"));
  auto* plain = new QPushButton(QStringLiteral("Plain"), &window);
  auto* hintLabel = new QLabel(&window);
  layout->addWidget(hinted);
  layout->addWidget(plain);
  layout->addWidget(hintLabel);

  uwf::ui::TransientLabel transient(hintLabel, &window);
  transient.setBaseline(QStringLiteral("Baseline"));
  uwf::ui::HoverHintController controller(&window, &window);
  controller.setTarget(&transient);

  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  QTest::mouseMove(hinted, hinted->rect().center());
  QTRY_COMPARE_WITH_TIMEOUT(hintLabel->text(), QStringLiteral("Action description"), 250);

  QTest::mouseMove(plain, plain->rect().center());
  QTRY_COMPARE_WITH_TIMEOUT(hintLabel->text(), QStringLiteral("Baseline"), 500);
}

void UiBehaviorTests::powerConfirmationDefaultsToCancel() {
  bool inspected = false;
  bool cancelWasDefault = false;
  bool enterRejectedDialog = false;
  QString objectName;
  QString actionObjectName;

  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    objectName = dialog->objectName();
    auto* box = dialog->findChild<QDialogButtonBox*>();
    if (!box) {
      dialog->reject();
      return;
    }
    QPushButton* cancel = nullptr;
    for (const auto buttons = box->buttons(); auto* button : buttons) {
      if (box->buttonRole(button) == QDialogButtonBox::RejectRole) cancel = qobject_cast<QPushButton*>(button);
      if (box->buttonRole(button) == QDialogButtonBox::AcceptRole) actionObjectName = button->objectName();
    }
    inspected = cancel != nullptr;
    cancelWasDefault = cancel && cancel->isDefault();
    QSignalSpy rejected(dialog, &QDialog::rejected);
    QTest::keyClick(dialog, Qt::Key_Return);
    enterRejectedDialog = rejected.count() == 1;
    if (dialog->isVisible()) dialog->reject();
  });

  QVERIFY(uwf::ui::runPowerActionDialog(nullptr, {uwf::ui::PowerAction::Shutdown, {}}) == uwf::ui::PowerActionDialogOutcome::Canceled);
  QVERIFY(inspected);
  QVERIFY(cancelWasDefault);
  QVERIFY(enterRejectedDialog);
  QCOMPARE(objectName, QStringLiteral("powerConfirmDialog"));
  QCOMPARE(actionObjectName, QStringLiteral("dangerBtn"));
}

void UiBehaviorTests::powerConfirmationEscapeAlwaysCancels() {
  bool escapeRejectedDialog = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    QSignalSpy rejected(dialog, &QDialog::rejected);
    QTest::keyClick(dialog, Qt::Key_Escape);
    escapeRejectedDialog = rejected.count() == 1;
    if (dialog->isVisible()) dialog->reject();
  });

  QVERIFY(uwf::ui::runPowerActionDialog(nullptr, {uwf::ui::PowerAction::Restart, {}}) == uwf::ui::PowerActionDialogOutcome::Canceled);
  QVERIFY(escapeRejectedDialog);
}

void UiBehaviorTests::powerConfirmationCanSkipStagingForDirectRestart() {
  int preparationPolls = 0;
  bool safeRestartBlocked = false;
  bool directRestartAvailable = false;
  bool restartLabelsCorrect = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* safeRestart = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"));
    auto* directRestart = dialog->findChild<QPushButton*>(QStringLiteral("directRestartBtn"));
    safeRestartBlocked = safeRestart && !safeRestart->isEnabled();
    directRestartAvailable = directRestart && directRestart->isEnabled();
    auto* hint = dialog->findChild<QLabel*>(QStringLiteral("directPowerHint"));
    restartLabelsCorrect = safeRestart && safeRestart->text() == QStringLiteral("Safe restart") && directRestart &&
                           directRestart->text() == QStringLiteral("Direct restart") && hint && hint->isVisible() &&
                           hint->text().startsWith(QStringLiteral("Direct restart"));
    if (directRestart) QTest::mouseClick(directRestart, Qt::LeftButton);
  });

  uwf::ui::PowerActionDialogRequest request{uwf::ui::PowerAction::Restart, [&]() -> std::optional<uwf::ui::PowerStagingPreparation> {
                                              ++preparationPolls;
                                              return uwf::ui::PowerStagingNotRequired{};
                                            }};
  QVERIFY(uwf::ui::runPowerActionDialog(nullptr, std::move(request)) == uwf::ui::PowerActionDialogOutcome::DirectConfirmed);
  QVERIFY(safeRestartBlocked);
  QVERIFY(directRestartAvailable);
  QVERIFY(restartLabelsCorrect);
  QCOMPARE(preparationPolls, 0);

  bool shutdownLabelsCorrect = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* safeShutdown = dialog->findChild<QPushButton*>(QStringLiteral("dangerBtn"));
    auto* directShutdown = dialog->findChild<QPushButton*>(QStringLiteral("directShutdownBtn"));
    auto* hint = dialog->findChild<QLabel*>(QStringLiteral("directPowerHint"));
    shutdownLabelsCorrect = safeShutdown && safeShutdown->text() == QStringLiteral("Safe shutdown") && directShutdown &&
                            directShutdown->text() == QStringLiteral("Direct shutdown") && hint && hint->isVisible() &&
                            hint->text().startsWith(QStringLiteral("Direct shutdown"));
    if (directShutdown) QTest::mouseClick(directShutdown, Qt::LeftButton);
  });
  QVERIFY(uwf::ui::runPowerActionDialog(nullptr, {uwf::ui::PowerAction::Shutdown, {}}) == uwf::ui::PowerActionDialogOutcome::DirectConfirmed);
  QVERIFY(shutdownLabelsCorrect);
}

void UiBehaviorTests::powerConfirmationAcceptsOnlyTheExplicitAction() {
  QString heading;
  QString actionObjectName;
  bool clicked = false;
  bool stagingSummaryVisible = false;
  bool actionDisabledDuringPreparation = false;
  qint64 preparationDurationMs = -1;
  int preparationPolls = 0;
  QElapsedTimer preparationElapsed;
  preparationElapsed.start();

  QTimer driver;
  driver.setInterval(1);
  connect(&driver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (auto* label = dialog->findChild<QLabel*>(QStringLiteral("powerActionHeading"))) heading = label->text();
    if (auto* action = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"))) {
      if (!action->isEnabled()) {
        actionDisabledDuringPreparation = true;
        return;
      }
      preparationDurationMs = preparationElapsed.elapsed();
      for (const auto labels = dialog->findChildren<QLabel*>(); const auto* label : labels) {
        if (label->text().contains(QStringLiteral("3 file(s)"))) stagingSummaryVisible = true;
      }
      actionObjectName = action->objectName();
      clicked = true;
      QTest::mouseClick(action, Qt::LeftButton);
      driver.stop();
    } else {
      dialog->reject();
      driver.stop();
    }
  });
  driver.start();

  uwf::ui::PowerActionDialogRequest request{uwf::ui::PowerAction::Restart, [&]() -> std::optional<uwf::ui::PowerStagingPreparation> {
                                              ++preparationPolls;
                                              if (preparationPolls == 1) return std::nullopt;
                                              return uwf::ui::PowerStagingCommit{
                                                  3, [] { return uwf::ui::PowerStagingProgress{uwf::ui::PowerStagingState::Completed, 3, 3, {}, {}}; }};
                                            }};
  QVERIFY(uwf::ui::runPowerActionDialog(nullptr, std::move(request)) == uwf::ui::PowerActionDialogOutcome::Confirmed);
  QVERIFY(clicked);
  QVERIFY(stagingSummaryVisible);
  QVERIFY(actionDisabledDuringPreparation);
  QVERIFY(preparationDurationMs >= 2000);
  QCOMPARE(preparationPolls, 2);
  QCOMPARE(actionObjectName, QStringLiteral("restartBtn"));
  QVERIFY(!heading.isEmpty());
}

void UiBehaviorTests::powerDialogShowsProgressAndFailureInPlace() {
  QDialog* initialDialog = nullptr;
  bool actionClicked = false;
  bool failureObserved = false;
  bool sameDialog = false;
  bool cancelWasDefault = false;
  bool escapeIgnoredDuringStaging = false;
  bool progressObserved = false;
  int steps = 0;

  QTimer driver;
  driver.setInterval(1);
  connect(&driver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (!initialDialog) initialDialog = dialog;

    auto* details = dialog->findChild<QPlainTextEdit*>(QStringLiteral("powerStagingFailureDetails"));
    if (details && details->isVisible()) {
      failureObserved = details->toPlainText().contains(QStringLiteral("C:\\locked.bin"));
      sameDialog = dialog == initialDialog;
      auto* box = dialog->findChild<QDialogButtonBox*>();
      if (box) {
        for (const auto buttons = box->buttons(); auto* button : buttons) {
          if (box->buttonRole(button) != QDialogButtonBox::RejectRole) continue;
          auto* cancel = qobject_cast<QPushButton*>(button);
          cancelWasDefault = cancel && cancel->isDefault();
          if (cancel) QTest::mouseClick(cancel, Qt::LeftButton);
          driver.stop();
          return;
        }
      }
      dialog->reject();
      driver.stop();
      return;
    }

    if (!actionClicked) {
      if (auto* action = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"))) {
        if (!action->isEnabled()) return;
        actionClicked = true;
        QTest::mouseClick(action, Qt::LeftButton);
      }
    }
  });
  driver.start();

  uwf::ui::PowerActionDialogRequest request{
      uwf::ui::PowerAction::Restart, [&]() -> std::optional<uwf::ui::PowerStagingPreparation> {
        return uwf::ui::PowerStagingCommit{2, [&]() {
                                             ++steps;
                                             if (steps == 1) {
                                               auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
                                               if (dialog) {
                                                 QTest::keyClick(dialog, Qt::Key_Escape);
                                                 escapeIgnoredDuringStaging = dialog->isVisible();
                                               }
                                               return uwf::ui::PowerStagingProgress{
                                                   uwf::ui::PowerStagingState::InProgress, 1, 2, QStringLiteral("C:\\first.bin"), {}};
                                             }
                                             auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
                                             auto* progress = dialog ? dialog->findChild<QProgressBar*>(QStringLiteral("powerStagingProgress")) : nullptr;
                                             auto* path = dialog ? dialog->findChild<QLineEdit*>(QStringLiteral("powerStagingCurrentPath")) : nullptr;
                                             progressObserved = progress && progress->isVisible() && progress->value() == 1 && progress->maximum() == 2 &&
                                                                path && path->text() == QStringLiteral("C:\\first.bin");
                                             return uwf::ui::PowerStagingProgress{uwf::ui::PowerStagingState::Failed, 2, 2, QStringLiteral("C:\\locked.bin"),
                                                                                  QStringLiteral("C:\\locked.bin\nProvider rejected the commit.")};
                                           }};
      }};
  QVERIFY(uwf::ui::runPowerActionDialog(nullptr, std::move(request)) == uwf::ui::PowerActionDialogOutcome::CanceledAfterStagingFailure);
  QCOMPARE(steps, 2);
  QVERIFY(actionClicked);
  QVERIFY(failureObserved);
  QVERIFY(sameDialog);
  QVERIFY(cancelWasDefault);
  QVERIFY(escapeIgnoredDuringStaging);
  QVERIFY(progressObserved);
}

void UiBehaviorTests::tableCopyReflectsTheActualSelectionAndMissingCells() {
  QTableWidget table(3, 3);
  table.setHorizontalHeaderLabels({QStringLiteral("State"), QStringLiteral("Target"), QStringLiteral("Detail")});
  table.setItem(0, 0, new QTableWidgetItem(QStringLiteral("OK")));
  table.setItem(0, 1, new QTableWidgetItem(QStringLiteral("C:")));
  table.setItem(0, 2, new QTableWidgetItem(QStringLiteral("Protected")));
  table.setItem(1, 0, new QTableWidgetItem(QStringLiteral("Failed")));
  table.setItem(1, 2, new QTableWidgetItem(QStringLiteral("Unavailable")));
  table.setItem(2, 1, new QTableWidgetItem(QStringLiteral("D:")));

  table.setRangeSelected(QTableWidgetSelectionRange(0, 1, 1, 2), true);
  QCOMPARE(uwf::ui::tableSelectionToText(&table), QStringLiteral("C:\tProtected\n\tUnavailable\n"));
  QCOMPARE(uwf::ui::tableAllToText(&table), QStringLiteral("State\tTarget\tDetail\nOK\tC:\tProtected\nFailed\t\tUnavailable\n\tD:\t\n"));

  table.clearSelection();
  QVERIFY(uwf::ui::tableSelectionToText(&table).isEmpty());
}

void UiBehaviorTests::volumeStatusTracksOnlyChangesFromTheDisplayedSnapshot() {
  uwf::ui::StatusPanel panel;
  const uwf::core::VolumeRecord current{"Volume{A}", "C:", true, true};
  const uwf::core::VolumeRecord next{"Volume{A}", "C:", false, false};
  panel.setData(&current, &next);

  QVERIFY(!panel.pendingVolumeProtected().has_value());
  QVERIFY(!panel.pendingBindByVolumeName().has_value());
  QVERIFY(!panel.importProtect(false));
  QVERIFY(panel.importProtect(true));
  QCOMPARE(panel.pendingVolumeProtected(), std::optional<bool>(true));
  QVERIFY(panel.importProtect(false));
  QVERIFY(!panel.pendingVolumeProtected().has_value());

  QVERIFY(!panel.importBindByVolumeName(true));
  QVERIFY(panel.importBindByVolumeName(false));
  QCOMPARE(panel.pendingBindByVolumeName(), std::optional<bool>(false));

  panel.setData(nullptr, nullptr);
  QVERIFY(!panel.importProtect(true));
  QVERIFY(!panel.pendingVolumeProtected().has_value());
  QVERIFY(!panel.pendingBindByVolumeName().has_value());
}

void UiBehaviorTests::globalStatusImportProducesConstrainedPendingDelta() {
  uwf::ui::GlobalStatusPanel panel;
  uwf::core::SessionSnapshot current;
  uwf::core::SessionSnapshot next;
  current.filter.enabled = false;
  next.filter.enabled = false;
  next.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  panel.setData(current, next, {4096, 1024});

  QVERIFY(!panel.pendingFilterEnabled().has_value());
  QVERIFY(panel.pendingOverlay().empty());
  QVERIFY(panel.importFilterEnabled(true));
  const auto pendingFilter = panel.pendingFilterEnabled();
  QVERIFY(pendingFilter.has_value());
  QCOMPARE(pendingFilter.value_or(false), true);
  QVERIFY(!panel.importFilterEnabled(true));

  QVERIFY(panel.importOverlayType(uwf::core::OverlayType::Disk));
  QVERIFY(panel.importOverlayMaxMb(2000));
  QVERIFY(panel.importOverlayWarnMb(2500));
  QVERIFY(panel.importOverlayCritMb(2200));
  panel.finishImport();

  const auto delta = panel.pendingOverlay();
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
  QVERIFY(critical < maximum);

  panel.setData(current, next, {4096, 1024});
  QVERIFY(!panel.pendingFilterEnabled().has_value());
  QVERIFY(panel.pendingOverlay().empty());
}

void UiBehaviorTests::unavailableGlobalStatusRejectsProgrammaticChanges() {
  uwf::ui::GlobalStatusPanel panel;
  uwf::core::SessionSnapshot current;
  uwf::core::SessionSnapshot next;
  next.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  panel.setData(current, next, {4096, 1024});
  panel.setUnavailable(QStringLiteral("provider unavailable"));

  QVERIFY(!panel.importFilterEnabled(true));
  QVERIFY(!panel.importOverlayType(uwf::core::OverlayType::Disk));
  QVERIFY(!panel.importOverlayMaxMb(8192));
  QVERIFY(!panel.importOverlayWarnMb(4096));
  QVERIFY(!panel.importOverlayCritMb(6144));
  QVERIFY(!panel.pendingFilterEnabled().has_value());
  QVERIFY(panel.pendingOverlay().empty());
}

void UiBehaviorTests::fileExclusionImportMaintainsCaseInsensitiveDeltas() {
  using Outcome = uwf::ui::ExclusionListWidget::ImportOutcome;
  uwf::ui::ExclusionListWidget list(uwf::ui::ExclusionListWidget::Kind::File);
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
}

void UiBehaviorTests::fileExclusionImportRejectsWrongVolumeAndUwfBlacklist() {
  using Outcome = uwf::ui::ExclusionListWidget::ImportOutcome;
  uwf::ui::ExclusionListWidget list(uwf::ui::ExclusionListWidget::Kind::File);
  list.setDriveLetter(QStringLiteral("Z:"));
  list.setBaseline({}, {});

  QCOMPARE(list.importAdd(QStringLiteral("Y:\\Data")), Outcome::RejectedNotOnVolume);
  QCOMPARE(list.importAdd(QStringLiteral("Z:")), Outcome::RejectedForbidden);
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\pagefile.sys")), Outcome::RejectedForbidden);
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\Data\\Cache")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("Z:\\Data\\Cache")}));
}

void UiBehaviorTests::registryExclusionImportUsesTheSamePolicyAsTheUi() {
  using Outcome = uwf::ui::ExclusionListWidget::ImportOutcome;
  uwf::ui::ExclusionListWidget list(uwf::ui::ExclusionListWidget::Kind::Registry);
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

  // 先在空暂存状态排队移除，再添加暂存数据；待应用边界必须
  // 永久丢弃根排除项的移除意图，不能在暂存再次清空后“复活”。
  stagingHasData = false;
  QCOMPARE(list.importRemove(stagingRoot), Outcome::Applied);
  QVERIFY(list.pendingRemoved().contains(stagingRoot, Qt::CaseInsensitive));
  stagingHasData = true;
  list.enforceFileStagingRemovalProtection();
  QVERIFY(!list.pendingRemoved().contains(stagingRoot, Qt::CaseInsensitive));
  stagingHasData = false;
  QVERIFY(!list.pendingRemoved().contains(stagingRoot, Qt::CaseInsensitive));
}

void UiBehaviorTests::exclusionListDoubleClickCopiesTheDisplayedFullPath() {
  uwf::ui::ExclusionListWidget list(uwf::ui::ExclusionListWidget::Kind::File);
  list.setDriveLetter(QStringLiteral("Z:"));
  list.setBaseline({QStringLiteral("\\Data\\State.db")}, {QStringLiteral("\\Data\\State.db")});
  list.resize(500, 260);
  list.show();
  QVERIFY(QTest::qWaitForWindowExposed(&list));

  auto* items = list.findChild<QListWidget*>();
  QVERIFY(items);
  QCOMPARE(items->count(), 1);
  QSignalSpy copied(&list, &uwf::ui::ExclusionListWidget::copiedToClipboard);
  const QRect rect = items->visualItemRect(items->item(0));
  QVERIFY(rect.isValid());
  QTest::mouseClick(items->viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());
  QCOMPARE(items->currentItem(), items->item(0));
  QTest::mouseDClick(items->viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());
  QCOMPARE(copied.count(), 1);
  QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("Z:\\Data\\State.db"));
}

void UiBehaviorTests::exclusionListButtonsFilterAndPersistenceRowsUseDisplayedSelection() {
  uwf::ui::ExclusionListWidget files(uwf::ui::ExclusionListWidget::Kind::File);
  files.setDriveLetter(QStringLiteral("Z:"));
  files.setBaseline({QStringLiteral("Z:\\Alpha"), QStringLiteral("Z:\\Beta"), QStringLiteral("Z:\\Gamma")},
                    {QStringLiteral("Z:\\Alpha"), QStringLiteral("Z:\\Beta"), QStringLiteral("Z:\\Gamma")});
  files.resize(560, 320);
  files.show();
  QVERIFY(QTest::qWaitForWindowExposed(&files));
  auto* list = files.findChild<QListWidget*>(QStringLiteral("exclusionList"));
  auto* filter = files.findChild<QLineEdit*>(QStringLiteral("filterInput"));
  auto* remove = buttonWithText(&files, QStringLiteral("Remove selected"));
  QVERIFY(list);
  QVERIFY(filter);
  QVERIFY(remove);
  QCOMPARE(list->count(), 3);

  filter->setText(QStringLiteral("beta"));
  int visible = 0;
  for (int row = 0; row < list->count(); ++row) visible += list->item(row)->isHidden() ? 0 : 1;
  QCOMPARE(visible, 1);
  filter->clear();
  list->item(0)->setSelected(true);
  list->item(2)->setSelected(true);
  QTest::mouseClick(remove, Qt::LeftButton);
  QCOMPARE(files.pendingRemoved().size(), 2);

  uwf::ui::ExclusionListWidget registry(uwf::ui::ExclusionListWidget::Kind::Registry);
  const QString stagingRegistryRoot = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager");
  bool fileStagingHasData = true;
  registry.setFileStagingDataProbe([&fileStagingHasData] { return fileStagingHasData; });
  registry.setBaseline({stagingRegistryRoot}, {stagingRegistryRoot});
  registry.setPersistBaseline(true, true, false, false);
  registry.resize(620, 320);
  registry.show();
  QVERIFY(QTest::qWaitForWindowExposed(&registry));
  auto* enableTscal = actionWithText(&registry, QStringLiteral("Terminal Services Client Access License (TSCAL)"));
  QVERIFY(enableTscal);
  enableTscal->trigger();
  QCOMPARE(registry.pendingPersistTSCAL(), std::optional<bool>(true));

  auto* registryList = registry.findChild<QListWidget*>(QStringLiteral("exclusionList"));
  auto* registryRemove = buttonWithText(&registry, QStringLiteral("Remove selected"));
  QVERIFY(registryList);
  QVERIFY(registryRemove);
  bool stagingRegistryObserved = false;
  QListWidgetItem* stagingRegistryItem = nullptr;
  for (int row = 0; row < registryList->count(); ++row) {
    auto* item = registryList->item(row);
    if (item->data(Qt::UserRole).toString().compare(stagingRegistryRoot, Qt::CaseInsensitive) != 0) continue;
    stagingRegistryObserved = true;
    stagingRegistryItem = item;
    QVERIFY(item->toolTip().startsWith(QStringLiteral("Required by UWF for File staging.")));
    QVERIFY(!item->toolTip().contains(QStringLiteral("Registry:")));
  }
  QVERIFY(stagingRegistryObserved);
  QVERIFY(stagingRegistryItem);
  stagingRegistryItem->setSelected(true);
  QVERIFY(!registryRemove->isEnabled());
  QTest::mouseClick(registryRemove, Qt::LeftButton);
  QCOMPARE(registry.pendingRemoved().size(), 0);

  fileStagingHasData = false;
  registry.setFileStagingDataProbe([&fileStagingHasData] { return fileStagingHasData; });
  QVERIFY(registryRemove->isEnabled());
  registryList->clearSelection();
  bool selectedTscal = false;
  for (int row = 0; row < registryList->count(); ++row) {
    if (registryList->item(row)->data(Qt::UserRole).toString().contains(QStringLiteral("TSCAL"))) {
      registryList->item(row)->setSelected(true);
      selectedTscal = true;
    }
  }
  QVERIFY(selectedTscal);
  QTest::mouseClick(registryRemove, Qt::LeftButton);
  QVERIFY(!registry.pendingPersistTSCAL().has_value());

  registryList->clearSelection();
  for (int row = 0; row < registryList->count(); ++row) registryList->item(row)->setSelected(true);
  QTest::mouseClick(registryRemove, Qt::LeftButton);
  QCOMPARE(registry.pendingPersistDomainSecretKey(), std::optional<bool>(false));
}

void UiBehaviorTests::exclusionFileActionsUseThePlatformBoundaryAndPreserveValidation() {
  MemoryFileDialogs files;
  files.openedFiles = {QStringLiteral("Z:/Data/one.db"), QStringLiteral("Y:/wrong.db"), QString()};
  files.directory = QStringLiteral("Z:/Cache");
  uwf::ui::ExclusionListWidget list(uwf::ui::ExclusionListWidget::Kind::File, files);
  list.setDriveLetter(QStringLiteral("z:"));
  list.setBaseline({}, {});

  auto* addFile = actionWithText(&list, QStringLiteral("File…"));
  auto* addFolder = actionWithText(&list, QStringLiteral("Folder…"));
  QVERIFY(addFile);
  QVERIFY(addFolder);

  bool warningSeen = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    warningSeen = dialog != nullptr;
    if (dialog) dialog->accept();
  });
  addFile->trigger();
  QVERIFY(warningSeen);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("Z:\\Data\\one.db")}));
  QCOMPARE(files.requests.size(), 1);
  QCOMPARE(files.requests.front().initialPath, QStringLiteral("Z:\\"));

  addFolder->trigger();
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("Z:\\Cache"), QStringLiteral("Z:\\Data\\one.db")}));
  QCOMPARE(files.requests.size(), 2);

  list.setReadOnly(true);
  files.openedFiles = {QStringLiteral("Z:/Blocked")};
  addFile->trigger();
  QCOMPARE(files.requests.size(), 2);
  QCOMPARE(list.pendingAdded().size(), 2);
}

void UiBehaviorTests::importDialogFiltersCommentsAndAppendsBatchReports() {
  QVERIFY(uwf::ui::parseErrorMessage(uwf::api::ParseError::MalformedQuoting, {}).contains(QStringLiteral("quote"), Qt::CaseInsensitive));
  QVERIFY(uwf::ui::parseErrorMessage(uwf::api::ParseError::UnexpectedArgument, QStringLiteral("extra")).contains(QStringLiteral("extra")));

  uwf::ui::ImportDialog dialog;
  auto* input = dialog.findChild<QPlainTextEdit*>(QStringLiteral("importTextEdit"));
  auto* importButton = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  auto* report = dialog.findChild<QTableWidget*>();
  QVERIFY(input);
  QVERIFY(importButton);
  QVERIFY(report);
  QVERIFY(!importButton->isEnabled());

  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  QList<QList<uwf::api::UwfmgrCommand>> batches;
  dialog.setApplier([&](const QList<uwf::api::UwfmgrCommand>& commands) {
    batches.append(commands);
    QList<uwf::ui::ImportReportRow> rows;
    for (const auto& command : commands) {
      uwf::ui::ImportReportRow row;
      row.lineNo = command.sourceLineNo;
      row.lineText = QString::fromStdString(command.rawLine);
      row.status = command.parseError == uwf::api::ParseError::None ? uwf::ui::ImportReportRow::Status::Success : uwf::ui::ImportReportRow::Status::Failed;
      row.detail = command.parseError == uwf::api::ParseError::None ? QStringLiteral("queued") : QStringLiteral("invalid");
      rows.append(row);
    }
    return rows;
  });

  input->setPlainText(QStringLiteral("# note\nuwfmgr filter enable\noverlay set-size bad\n"));
  QVERIFY(importButton->isEnabled());
  QTest::mouseClick(importButton, Qt::LeftButton);
  QCOMPARE(batches.size(), 1);
  QCOMPARE(batches[0].size(), 2);
  QCOMPARE(batches[0][0].sourceLineNo, 2);
  QCOMPARE(batches[0][1].parseError, uwf::api::ParseError::InvalidSize);
  QVERIFY(input->toPlainText().isEmpty());
  QCOMPARE(report->rowCount(), 2);

  input->setPlainText(QStringLiteral("volume protect d"));
  QTest::mouseClick(importButton, Qt::LeftButton);
  QCOMPARE(batches.size(), 2);
  QCOMPARE(batches[1].size(), 1);
  QCOMPARE(batches[1][0].args.at(0), std::string("D:"));
  QCOMPARE(report->rowCount(), 4);  // 两条首批结果 + 第二批分隔行 + 一条结果
  QVERIFY(report->isVisibleTo(&dialog));
}

void UiBehaviorTests::importFileActionLoadsOnlyCommandsFromEverySelectedFile() {
  QTemporaryFile first;
  QTemporaryFile second;
  QVERIFY(first.open());
  QVERIFY(second.open());
  QVERIFY(first.write("noise\nuwfmgr filter enable\nUWFMgr volume protect C:\\n") > 0);
  QVERIFY(second.write("nothing useful\n") > 0);
  first.flush();
  second.flush();
  first.close();
  second.close();

  MemoryFileDialogs files;
  files.openedFiles = {first.fileName(), second.fileName()};
  uwf::ui::ImportDialog dialog(files);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* load = buttonWithText(&dialog, QStringLiteral("Load from file…"));
  auto* input = dialog.findChild<QPlainTextEdit*>(QStringLiteral("importTextEdit"));
  QVERIFY(load);
  QVERIFY(input);

  QTest::mouseClick(load, Qt::LeftButton);
  QCOMPARE(files.requests.size(), 1);
  QVERIFY(files.requests.front().filter.contains(QStringLiteral("*.ps1")));
  const QString text = input->toPlainText();
  QVERIFY(text.contains(QStringLiteral("uwfmgr filter enable")));
  QVERIFY(text.contains(QStringLiteral("UWFMgr volume protect C:")));
  QVERIFY(!text.contains(QStringLiteral("noise")));
  QVERIFY(text.contains(QStringLiteral("no uwfmgr lines found")));

  files.openedFiles.clear();
  QTest::mouseClick(load, Qt::LeftButton);
  QCOMPARE(files.requests.size(), 2);
  QCOMPARE(input->toPlainText(), text);

  QTemporaryFile missing;
  QVERIFY(missing.open());
  const QString missingPath = missing.fileName();
  QVERIFY(missing.remove());
  files.openedFiles = {missingPath};
  QTimer::singleShot(0, this, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->accept();
  });
  QTest::mouseClick(load, Qt::LeftButton);
  QCOMPARE(input->toPlainText(), text);

  QTemporaryFile oversized;
  QVERIFY(oversized.open());
  const QByteArray payload(5 * 1024 * 1024 + 1, 'x');
  QCOMPARE(oversized.write(payload), static_cast<qint64>(payload.size()));
  oversized.close();
  files.openedFiles = {oversized.fileName()};
  QTimer::singleShot(0, this, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->accept();
  });
  QTest::mouseClick(load, Qt::LeftButton);
  QCOMPARE(input->toPlainText(), text);
}

void UiBehaviorTests::recommendedConfigurationSelectionAppendsOnlyChosenGroups() {
  uwf::ui::ImportDialog dialog;
  auto* input = dialog.findChild<QPlainTextEdit*>(QStringLiteral("importTextEdit"));
  QPushButton* recommended = nullptr;
  for (const auto buttons = dialog.findChildren<QPushButton*>(); auto* button : buttons) {
    if (button->text() == QStringLiteral("Load recommended configuration")) recommended = button;
  }
  QVERIFY(input);
  QVERIFY(recommended);

  bool chooserStructureValid = false;
  QTimer::singleShot(0, this, [&chooserStructureValid] {
    auto* chooser = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!chooser) return;
    const auto checks = chooser->findChildren<QCheckBox*>();
    if (checks.size() != 6 || std::ranges::any_of(checks, [](const QCheckBox* check) { return !check->isChecked(); })) {
      chooser->reject();
      return;
    }
    for (int index = 1; index < checks.size(); ++index) checks[index]->setChecked(false);
    auto* box = chooser->findChild<QDialogButtonBox*>();
    if (!box) {
      chooser->reject();
      return;
    }
    QPushButton* accept = nullptr;
    for (const auto buttons = box->buttons(); auto* button : buttons) {
      if (box->buttonRole(button) == QDialogButtonBox::AcceptRole) accept = qobject_cast<QPushButton*>(button);
    }
    if (!accept || !accept->isEnabled()) {
      chooser->reject();
      return;
    }
    chooserStructureValid = true;
    QTest::mouseClick(accept, Qt::LeftButton);
  });
  QTest::mouseClick(recommended, Qt::LeftButton);

  QVERIFY(chooserStructureValid);
  const QString text = input->toPlainText();
  QVERIFY(text.contains(QStringLiteral("Customer Experience Improvement Program")));
  QVERIFY(text.contains(QStringLiteral("uwfmgr registry add-exclusion")));
  QVERIFY(!text.contains(QStringLiteral("Background Intelligent Transfer Service")));
  auto* import = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(import);
  QVERIFY(import->isEnabled());
}

void UiBehaviorTests::powerDialogCompactLayoutKeepsSafeActionsOutsideScrolling_data() {
  QTest::addColumn<bool>("chinese");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("failure");
  for (const bool chinese : {false, true})
    for (const bool dark : {false, true})
      for (const bool failure : {false, true}) {
        const QByteArray name = QStringLiteral("%1-%2-%3").arg(chinese ? "zh" : "en", dark ? "dark" : "light", failure ? "failure" : "confirmation").toLatin1();
        QTest::newRow(name.constData()) << chinese << dark << failure;
      }
}

void UiBehaviorTests::powerDialogCompactLayoutKeepsSafeActionsOutsideScrolling() {
  using namespace uwf::ui;
  QFETCH(bool, chinese);
  QFETCH(bool, dark);
  QFETCH(bool, failure);
  const auto language = I18n::instance().lang();
  const auto theme = ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    I18n::instance().setLang(language);
    ThemeManager::instance().apply(theme);
  });
  I18n::instance().setLang(chinese ? I18n::Lang::Zh_CN : I18n::Lang::En);
  ThemeManager::instance().apply(dark ? Theme::Dark : Theme::Light);
  bool inspected = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    const auto reject = qScopeGuard([&] { dialog->reject(); });
    dialog->resize(760, 420);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QCOMPARE(dialog->size(), QSize(760, 420));
    QTRY_VERIFY2_WITH_TIMEOUT(uwf::test::clippedLabel(*dialog).isEmpty(), qPrintable(uwf::test::clippedLabel(*dialog)), 2000);
    auto* scroll = dialog->findChild<QScrollArea*>(QStringLiteral("powerConfirmationScroll"));
    auto* buttons = dialog->findChild<QDialogButtonBox*>();
    QVERIFY(scroll && buttons);
    QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    for (auto* button : buttons->buttons()) {
      QVERIFY(!scroll->isAncestorOf(button));
      QVERIFY(dialog->rect().contains(QRect(button->mapTo(dialog, QPoint()), button->size())));
      QVERIFY(!button->visibleRegion().isEmpty());
    }
    auto* cancel = qobject_cast<QPushButton*>(buttons->button(QDialogButtonBox::Cancel));
    if (!cancel) {
      for (auto* button : buttons->buttons())
        if (buttons->buttonRole(button) == QDialogButtonBox::RejectRole) cancel = qobject_cast<QPushButton*>(button);
    }
    QVERIFY(cancel && cancel->isDefault());
    const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
    if (!screenshots.isEmpty()) {
      QVERIFY(QDir().mkpath(screenshots));
      QVERIFY(dialog->grab().save(QDir(screenshots).filePath("power-" + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
    }
    inspected = true;
  });
  PowerActionDialogRequest request{PowerAction::Restart};
  if (failure) request.completedStagingFailure = PowerStagingFailure{QStringLiteral("Mock failure details:\n").repeated(40)};
  const auto outcome = runPowerActionDialog(nullptr, std::move(request));
  QCOMPARE(outcome, failure ? PowerActionDialogOutcome::CanceledAfterStagingFailure : PowerActionDialogOutcome::Canceled);
  QVERIFY(inspected);
}

void UiBehaviorTests::groupedLayoutReflowsWithoutClippingOrRecreatingControls() {
  QWidget host;
  auto* layout = new uwf::ui::FlowLayout(&host);
  const std::array sizes{QSize(180, 40), QSize(120, 28), QSize(200, 32)};
  std::array<QWidget*, 3> groups{};
  for (size_t i = 0; i < groups.size(); ++i) {
    groups[i] = new QWidget(&host);
    groups[i]->setFixedSize(sizes[i]);
    layout->addWidget(groups[i]);
  }
  layout->setAlignment(groups.back(), Qt::AlignRight);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  for (const auto direction : {Qt::LeftToRight, Qt::RightToLeft}) {
    host.setLayoutDirection(direction);
    for (int width = 200; width <= 1000; width += 13) {
      host.resize(width, layout->heightForWidth(width));
      QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
      for (size_t i = 0; i < groups.size(); ++i) {
        QCOMPARE(groups[i]->size(), sizes[i]);
        QVERIFY(host.rect().contains(groups[i]->geometry()));
        for (size_t j = i + 1; j < groups.size(); ++j) QVERIFY(!groups[i]->geometry().intersects(groups[j]->geometry()));
      }
    }
  }
  const int allVisibleHeight = layout->heightForWidth(200);
  groups[1]->hide();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QVERIFY(layout->heightForWidth(200) < allVisibleHeight);
  groups[1]->show();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QCOMPARE(layout->heightForWidth(200), allVisibleHeight);
  QCOMPARE(layout->count(), 3);
  QVERIFY(!layout->itemAt(-1));
  QVERIFY(!layout->takeAt(3));
  const std::unique_ptr<QLayoutItem> removed(layout->takeAt(1));
  QVERIFY(removed);
  QCOMPARE(removed->widget(), groups[1]);
  QCOMPARE(layout->count(), 2);
}

}  // namespace

QTEST_MAIN(UiBehaviorTests)

#include "UiBehaviorTests.moc"
