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
#include <QClipboard>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTabWidget>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextEdit>
#include <QTimer>
#include <QtTest>
#include <deque>
#include <memory>
#include <vector>

#include "core/UwfModel.h"
#include "ui/AboutDialog.h"
#include "ui/ApplyPlanDialog.h"
#include "ui/CommitBatch.h"
#include "ui/CommitDispatcher.h"
#include "ui/Dialogs.h"
#include "ui/DiskTab.h"
#include "ui/GlobalStatusPanel.h"
#include "ui/I18n.h"
#include "ui/ImportApplier.h"
#include "ui/LogViewerDialog.h"
#include "ui/MainWindow.h"
#include "ui/MarqueeHintBox.h"
#include "ui/OverlayHudPalette.h"
#include "ui/OverlayUsageBar.h"
#include "ui/PendingCollect.h"
#include "ui/PowerController.h"
#include "ui/StatusBanner.h"
#include "ui/ThemeManager.h"
#include "ui/UiUtil.h"
#include "util/Log.h"
#include "uwf/api/UwfmgrCli.h"
#include "uwf/wmi/WmiException.h"

namespace {

using namespace uwf;

QPushButton* buttonWithText(QWidget* root, const QString& text) {
  for (auto* button : root->findChildren<QPushButton*>()) {
    if (button->text() == text) return button;
  }
  return nullptr;
}

QAction* actionWithText(QWidget* root, const QString& text) {
  for (auto* action : root->findChildren<QAction*>()) {
    if (action->text() == text) return action;
  }
  return nullptr;
}

core::UwfSnapshot editableSnapshot() {
  core::UwfSnapshot snapshot;
  snapshot.uwfAvailable = true;
  snapshot.elevated = true;
  snapshot.current.filter.enabled = true;
  snapshot.next.filter.enabled = true;
  snapshot.current.overlay = {core::OverlayType::RAM, 4096, 2048, 3072};
  snapshot.next.overlay = {core::OverlayType::Disk, 8192, 4096, 6144};
  snapshot.runtime = {3072, 1024};
  snapshot.current.volumes.push_back({"Volume{c}", "C:", true, true});
  snapshot.next.volumes.push_back({"Volume{c}", "C:", true, true});
  snapshot.current.volumes.push_back({"Volume{d}", "D:", false, true});
  snapshot.next.volumes.push_back({"Volume{d}", "D:", false, true});
  snapshot.next.fileExclusions["Volume{c}"] = {"C:\\Existing"};
  snapshot.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\Existing"};
  return snapshot;
}

class RecordingWmiOperations final : public WmiOperations {
 public:
  mutable std::deque<std::vector<WmiRow>> queryResults;
  mutable std::deque<WmiRow> objectResults;
  mutable std::vector<QString> invocations;
  mutable std::vector<QString> invocationPaths;
  mutable std::vector<WmiRow> invocationInputs;
  QString connectionFailure;

  void ensureConnected() const override {
    if (!connectionFailure.isEmpty()) throw std::runtime_error(connectionFailure.toStdString());
  }
  std::vector<WmiRow> query(const std::string&) const override { return takeQuery(); }
  std::vector<WmiRow> queryInstances(const std::string&) const override { return takeQuery(); }
  WmiClassStatus classStatus(const std::string&) const override { return WmiClassStatus::Present; }
  WmiRow getObject(const std::string&) const override {
    if (objectResults.empty()) throw std::runtime_error("no object result queued");
    auto row = std::move(objectResults.front());
    objectResults.pop_front();
    return row;
  }
  void invokeMethod(const std::string& path, const std::string& method, const WmiRow& inputs) const override {
    invocationPaths.push_back(QString::fromStdString(path));
    invocations.push_back(QString::fromStdString(method));
    invocationInputs.push_back(inputs);
  }
  WmiMethodOutput callMethodRead(const std::string&, const std::string&, const WmiRow&) const override {
    throw std::runtime_error("unexpected callMethodRead");
  }
  WmiMethodOutput callMethodReadCancelable(const std::string&, const std::string&, const WmiRow&, std::stop_token) const override {
    throw std::runtime_error("unexpected callMethodReadCancelable");
  }
  void putInstance(const std::string&, const WmiRow&, WmiPutMode) const override { throw std::runtime_error("unexpected putInstance"); }

 private:
  std::vector<WmiRow> takeQuery() const {
    if (queryResults.empty()) throw std::runtime_error("no query result queued");
    auto result = std::move(queryResults.front());
    queryResults.pop_front();
    return result;
  }
};

class MemoryFileDialogs final : public ui::dialogs::FileDialogProvider {
 public:
  QString openedFile;
  QString selectedDirectory;
  QString savedFile;
  QList<ui::dialogs::FileDialogRequest> requests;

  QStringList openFiles(QWidget*, const ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return openedFile.isEmpty() ? QStringList{} : QStringList{openedFile};
  }
  QString openFile(QWidget*, const ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return openedFile;
  }
  QString selectDirectory(QWidget*, const ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return selectedDirectory;
  }
  QString saveFile(QWidget*, const ui::dialogs::FileDialogRequest& request) override {
    requests.append(request);
    return savedFile;
  }
};

class MutableApplicationStateSource final : public ui::ApplicationStateSource {
 public:
  std::vector<core::DiskInfo> disks;
  core::UwfSnapshot snapshot;
  QString failure;
  int reads = 0;

  ui::ApplicationState read(UwfCapability) override {
    ++reads;
    if (!failure.isEmpty()) throw std::runtime_error(failure.toStdString());
    return {disks, snapshot};
  }
};

class ApplicationUiBehaviorTests final : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void languageAndThemeChangesReachProductionWidgets();
  void aboutDialogExposesVersionLicenseAndSafeClose();
  void commonDialogsPreserveSafeDefaultsAndPreviewPagination();
  void logBufferAndViewerPreserveMalformedAndStructuredLines();
  void marqueeAndUsageWidgetsHandleEmptyOverflowAndThresholdEdges();
  void diskTabsApplyCapabilityBoundariesAndPreserveInnerSelection();
  void diskCommitActionsRouteSelectedTargetsAndHonorCancellation();
  void importRoutingAndPendingCollectionCoverInvalidDuplicateAndMissingTargets();
  void applyPlanPreviewAndCopyUseTheSameProductionCommandMapping();
  void applyPlanConfirmedWritePublishesReconciliationAndPreventsReplay();
  void applyPlanConnectionFailureRemainsRetryableAndDoesNotRequestReconciliation();
  void commitDispatcherRejectsUnaddressablePathsAndRestoresTheUsageTimer();
  void commitDispatcherConfirmsAndReportsARealFileThroughTheTransportBoundary();
  void commitDispatcherRoutesAnExistingRegistryValueThroughTheTransportBoundary();
  void safePowerActionsRequireConfirmationAndUseTheInjectedTransport();
  void commitBatchUsesAuthoritativeExistenceForEveryOutcome();
  void uiUtilitiesPreserveDriveComboAndDirtySemantics();
  void mainWindowDistinguishesInitialFailureFromCommittedUnavailableState();

 private:
  ui::I18n::Lang m_originalLanguage = ui::I18n::Lang::En;
  ui::Theme m_originalTheme = ui::Theme::Dark;
};

void ApplicationUiBehaviorTests::initTestCase() {
  m_originalLanguage = ui::I18n::instance().lang();
  m_originalTheme = ui::ThemeManager::instance().current();
  ui::I18n::instance().setLang(ui::I18n::Lang::En);
}

void ApplicationUiBehaviorTests::cleanupTestCase() {
  ui::I18n::instance().setLang(m_originalLanguage);
  ui::ThemeManager::instance().apply(m_originalTheme);
  clearLogLines();
}

void ApplicationUiBehaviorTests::languageAndThemeChangesReachProductionWidgets() {
  QCOMPARE(ui::I18n::applicationTitle(), QStringLiteral("Unified Write Filter (UWF) Manager"));
  ui::I18n::instance().setLang(ui::I18n::Lang::Zh_CN);
  QCOMPARE(ui::I18n::instance().lang(), ui::I18n::Lang::Zh_CN);
  QVERIFY(!ui::I18n::applicationTitle().isEmpty());
  QVERIFY(ui::I18n::applicationTitle() != QStringLiteral("Unified Write Filter (UWF) Manager"));
  ui::AboutDialog localizedDialog;
  QVERIFY(buttonWithText(&localizedDialog, QStringLiteral("关闭")));
  QVERIFY(!buttonWithText(&localizedDialog, QStringLiteral("Close")));
  ui::I18n::instance().setLang(ui::I18n::Lang::En);

  auto& theme = ui::ThemeManager::instance();
  QSignalSpy changed(&theme, &ui::ThemeManager::themeChanged);
  theme.apply(ui::Theme::Light);
  QCOMPARE(theme.current(), ui::Theme::Light);
  QCOMPARE(qApp->palette().color(QPalette::Window), theme.color(ui::Sem::Bg));
  QCOMPARE(changed.count(), 1);
  const auto lightHud = ui::overlayHudPalette(ui::Theme::Light);
  const auto darkHud = ui::overlayHudPalette(ui::Theme::Dark);
  QVERIFY(lightHud.text != darkHud.text);
  QVERIFY(lightHud.floatingSurface.alpha() < lightHud.taskbarSurface.alpha());
  theme.toggle();
  QCOMPARE(theme.current(), ui::Theme::Dark);
}

void ApplicationUiBehaviorTests::aboutDialogExposesVersionLicenseAndSafeClose() {
  ui::AboutDialog dialog;
  QVERIFY(dialog.minimumWidth() >= 500);
  bool hasVersion = false;
  bool hasLicense = false;
  bool hasSourceLink = false;
  for (auto* label : dialog.findChildren<QLabel*>()) {
    hasVersion = hasVersion || label->text().contains(QStringLiteral("Version "));
    hasLicense = hasLicense || label->text().contains(QStringLiteral("GNU General Public License"));
    hasSourceLink = hasSourceLink || label->text().contains(QStringLiteral("github.com/HsingYun/UWF-Manager"));
  }
  QVERIFY(hasVersion);
  QVERIFY(hasLicense);
  QVERIFY(hasSourceLink);
  auto* close = buttonWithText(&dialog, QStringLiteral("Close"));
  QVERIFY(close);
  QSignalSpy finished(&dialog, &QDialog::finished);
  QTest::mouseClick(close, Qt::LeftButton);
  QCOMPARE(finished.count(), 1);
  QCOMPARE(finished.at(0).at(0).toInt(), static_cast<int>(QDialog::Accepted));
}

void ApplicationUiBehaviorTests::commonDialogsPreserveSafeDefaultsAndPreviewPagination() {
  bool warningSelectable = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    for (auto* label : dialog->findChildren<QLabel*>()) {
      warningSelectable = warningSelectable || label->textInteractionFlags().testFlag(Qt::TextSelectableByMouse);
    }
    dialog->accept();
  });
  ui::dialogs::warning(nullptr, QStringLiteral("Diagnostic"), QStringLiteral("A selectable provider error"));
  QVERIFY(warningSelectable);

  bool cancelWasDefault = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (auto* box = dialog->findChild<QDialogButtonBox*>()) {
      for (auto* button : box->buttons()) {
        if (box->buttonRole(button) == QDialogButtonBox::RejectRole) {
          cancelWasDefault = qobject_cast<QPushButton*>(button)->isDefault();
          QTest::mouseClick(button, Qt::LeftButton);
          return;
        }
      }
    }
    dialog->reject();
  });
  QVERIFY(!ui::dialogs::confirm(nullptr, QStringLiteral("Confirm"), QStringLiteral("Dangerous operation")));
  QVERIFY(cancelWasDefault);

  QStringList preview;
  for (int i = 0; i < 23; ++i) preview.append(QStringLiteral("HKLM\\Software\\Item%1").arg(i));
  bool firstPageTen = false;
  bool lastPageThree = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* list = dialog->findChild<QListWidget*>(QStringLiteral("commitPreviewList"));
    firstPageTen = list && list->count() == 10;
    if (auto* last = buttonWithText(dialog, QStringLiteral("»"))) QTest::mouseClick(last, Qt::LeftButton);
    lastPageThree = list && list->count() == 3 && list->item(2)->text().endsWith(QStringLiteral("Item22"));
    dialog->reject();
  });
  QVERIFY(!ui::dialogs::confirmCommit(nullptr, QStringLiteral("Persist"), QStringLiteral("Delete keys"), QStringLiteral("HKLM\\Software"),
                                      QStringLiteral("Recursive"), preview));
  QVERIFY(firstPageTen);
  QVERIFY(lastPageThree);
}

void ApplicationUiBehaviorTests::logBufferAndViewerPreserveMalformedAndStructuredLines() {
  clearLogLines();
  logLine('I', "test", "structured");
  logLine('W', "test", "warning");
  const auto raw = recentLogLines();
  QCOMPARE(raw.size(), std::size_t{2});
  QVERIFY(raw.front().find(" I test]") != std::string::npos);

  ui::LogViewerDialog dialog;
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* table = dialog.findChild<QTableWidget*>();
  QVERIFY(table);
  QTRY_COMPARE_WITH_TIMEOUT(table->rowCount(), 2, 1000);
  QCOMPARE(table->item(0, 1)->text(), QStringLiteral("I"));
  QCOMPARE(table->item(1, 3)->text(), QStringLiteral("warning"));

  auto* copyAll = buttonWithText(&dialog, QStringLiteral("Copy all"));
  QVERIFY(copyAll);
  QTest::mouseClick(copyAll, Qt::LeftButton);
  QVERIFY(QApplication::clipboard()->text().contains(QStringLiteral("structured")));
  QVERIFY(QApplication::clipboard()->text().contains(QStringLiteral("warning")));

  auto* clear = buttonWithText(&dialog, QStringLiteral("Clear"));
  QVERIFY(clear);
  QTest::mouseClick(clear, Qt::LeftButton);
  QTRY_COMPARE_WITH_TIMEOUT(recentLogLines().size(), std::size_t{0}, 1000);
  dialog.close();
}

void ApplicationUiBehaviorTests::marqueeAndUsageWidgetsHandleEmptyOverflowAndThresholdEdges() {
  ui::MarqueeHintBox marquee;
  marquee.resize(180, 48);
  marquee.show();
  QVERIFY(QTest::qWaitForWindowExposed(&marquee));
  marquee.setText(QStringLiteral("short"));
  QTRY_COMPARE_WITH_TIMEOUT(marquee.verticalScrollBar()->maximum(), 0, 250);
  marquee.setText(QStringLiteral("a long overflowing line ").repeated(50));
  QTRY_VERIFY_WITH_TIMEOUT(marquee.verticalScrollBar()->maximum() > 0, 500);
  QCOMPARE(marquee.verticalScrollBar()->value(), 0);
  marquee.setText(QStringLiteral("short again"));
  QTRY_COMPARE_WITH_TIMEOUT(marquee.verticalScrollBar()->maximum(), 0, 500);

  ui::OverlayUsageBar bar;
  QCOMPARE(bar.minimumSizeHint().height(), bar.sizeHint().height());
  bar.resize(420, bar.sizeHint().height());
  bar.setData(0, 0, 0, 0);
  QImage empty(bar.size(), QImage::Format_ARGB32_Premultiplied);
  empty.fill(Qt::transparent);
  bar.render(&empty);
  QVERIFY(!empty.isNull());
  bar.setData(150, 60, 80, 100, 200);
  QImage thresholds(bar.size(), QImage::Format_ARGB32_Premultiplied);
  thresholds.fill(Qt::transparent);
  bar.render(&thresholds);
  QVERIFY(thresholds != empty);

  ui::StatusBanner banner;
  banner.setText(QStringLiteral("Critical warning"));
  banner.setProperty("level", "error");
  banner.resize(320, 50);
  QImage rendered(banner.size(), QImage::Format_ARGB32_Premultiplied);
  rendered.fill(Qt::transparent);
  banner.render(&rendered);
  QVERIFY(!rendered.isNull());
}

void ApplicationUiBehaviorTests::diskTabsApplyCapabilityBoundariesAndPreserveInnerSelection() {
  const core::DiskInfo supported{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported};
  const core::DiskInfo limited{"D:", "Volume{d}", "exFAT", "Data", 1000, 500, core::DiskSupport::FileSystemLimited};
  const core::DiskInfo unsupported{"E:", "Volume{e}", "NTFS", "USB", 1000, 500, core::DiskSupport::NotFixedLocalDisk};
  ui::DiskTab c(supported, true);
  ui::DiskTab d(limited, false);
  ui::DiskTab e(unsupported, false);
  const auto snapshot = editableSnapshot();
  c.applySnapshot(snapshot);
  d.applySnapshot(snapshot);
  e.applySnapshot(snapshot);

  QVERIFY(c.supported());
  QVERIFY(c.canManageExclusions());
  QVERIFY(d.supported());
  QVERIFY(!d.canManageExclusions());
  QVERIFY(!e.supported());
  for (const QString& text : {QStringLiteral("Commit file changes…"), QStringLiteral("Commit folder changes…"), QStringLiteral("Delete and commit file…"),
                              QStringLiteral("Delete and commit folder…")}) {
    auto* limitedAction = actionWithText(&d, text);
    auto* unsupportedAction = actionWithText(&e, text);
    QVERIFY(limitedAction && !limitedAction->isEnabled());
    QVERIFY(unsupportedAction && !unsupportedAction->isEnabled());
  }
  auto* innerTabs = c.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(innerTabs);
  QCOMPARE(innerTabs->count(), 2);
  c.setActiveInfoTabIndex(1);
  QCOMPARE(c.activeInfoTabIndex(), 1);
  c.setActiveInfoTabIndex(99);
  QCOMPARE(c.activeInfoTabIndex(), 1);
  QVERIFY(!ui::diskSupportText(core::DiskSupport::FileSystemLimited, "exFAT").empty());
  QVERIFY(!ui::diskSupportText(core::DiskSupport::ExceedsMaxSize, "NTFS").empty());
}

void ApplicationUiBehaviorTests::diskCommitActionsRouteSelectedTargetsAndHonorCancellation() {
  MemoryFileDialogs files;
  const core::DiskInfo disk{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported};
  ui::DiskTab tab(disk, true, files);
  tab.applySnapshot(editableSnapshot());
  tab.resize(900, 600);
  tab.show();
  QVERIFY(QTest::qWaitForWindowExposed(&tab));
  QSignalSpy commit(&tab, &ui::DiskTab::commitFileRequested);
  QSignalSpy deletion(&tab, &ui::DiskTab::commitFileDeletionRequested);

  auto* commitFile = actionWithText(&tab, QStringLiteral("Commit file changes…"));
  auto* commitFolder = actionWithText(&tab, QStringLiteral("Commit folder changes…"));
  auto* deleteFile = actionWithText(&tab, QStringLiteral("Delete and commit file…"));
  auto* deleteFolder = actionWithText(&tab, QStringLiteral("Delete and commit folder…"));
  QVERIFY(commitFile && commitFile->isEnabled());
  QVERIFY(commitFolder && commitFolder->isEnabled());
  QVERIFY(deleteFile && deleteFile->isEnabled());
  QVERIFY(deleteFolder && deleteFolder->isEnabled());

  files.openedFile = QStringLiteral("C:/Data/state.bin");
  commitFile->trigger();
  QCOMPARE(commit.count(), 1);
  QCOMPARE(commit.front().front().toString(), QStringLiteral("C:\\Data\\state.bin"));

  files.selectedDirectory = QStringLiteral("C:/Cache");
  commitFolder->trigger();
  QCOMPARE(commit.count(), 2);
  QCOMPARE(commit.back().front().toString(), QStringLiteral("C:\\Cache"));

  files.openedFile = QStringLiteral("C:/Delete/me.bin");
  deleteFile->trigger();
  files.selectedDirectory = QStringLiteral("C:/Delete/tree");
  deleteFolder->trigger();
  QCOMPARE(deletion.count(), 2);
  QCOMPARE(deletion.front().front().toString(), QStringLiteral("C:\\Delete\\me.bin"));
  QCOMPARE(deletion.back().front().toString(), QStringLiteral("C:\\Delete\\tree"));

  files.openedFile.clear();
  commitFile->trigger();
  QCOMPARE(commit.count(), 2);
  QCOMPARE(files.requests.size(), 5);
  QVERIFY(std::ranges::all_of(files.requests, [](const auto& request) { return request.initialPath == QStringLiteral("C:\\"); }));
}

void ApplicationUiBehaviorTests::importRoutingAndPendingCollectionCoverInvalidDuplicateAndMissingTargets() {
  ui::GlobalStatusPanel global;
  const auto snapshot = editableSnapshot();
  global.setData(snapshot.current, snapshot.next, snapshot.runtime);
  const core::DiskInfo cInfo{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported};
  const core::DiskInfo dInfo{"D:", "Volume{d}", "exFAT", "Data", 1000, 500, core::DiskSupport::FileSystemLimited};
  auto cOwner = std::make_unique<ui::DiskTab>(cInfo, true);
  auto dOwner = std::make_unique<ui::DiskTab>(dInfo, false);
  auto* c = cOwner.get();
  auto* d = dOwner.get();
  c->applySnapshot(snapshot);
  d->applySnapshot(snapshot);
  QVector<QPointer<ui::DiskTab>> tabs{c, QPointer<ui::DiskTab>{}, d};

  const auto parsed = api::parseUwfmgrText(
      "filter disable\n"
      "filter disable\n"
      "overlay set-warningthreshold 5000\n"
      "volume unprotect C:\n"
      "volume protect Z:\n"
      "file add-exclusion C:\\Cache\n"
      "file add-exclusion D:\\Cache\n"
      "file add-exclusion relative\\Cache\n"
      "registry add-exclusion HKLM\\Software\\Vendor\n"
      "file add-exclusion \"C:\\unterminated\n"
      "filter disable extra\n"
      "unknown command\n");
  QList<api::UwfmgrCommand> commands;
  for (const auto& command : parsed) commands.append(command);
  const auto report = ui::applyImportCommands(commands, &global, tabs);
  QCOMPARE(report.size(), commands.size());
  QCOMPARE(report[0].status, ui::ImportReportRow::Status::Success);
  QCOMPARE(report[1].status, ui::ImportReportRow::Status::Duplicate);
  QCOMPARE(report[4].status, ui::ImportReportRow::Status::Failed);
  QCOMPARE(report[6].status, ui::ImportReportRow::Status::Failed);
  QCOMPARE(report[7].status, ui::ImportReportRow::Status::Failed);
  QCOMPARE(report[9].status, ui::ImportReportRow::Status::Failed);
  QCOMPARE(report[10].status, ui::ImportReportRow::Status::Failed);
  QCOMPARE(report[11].status, ui::ImportReportRow::Status::Unsupported);

  const auto pending = ui::collectPending(&global, tabs);
  QCOMPARE(pending.setFilterEnabled, std::optional<bool>(false));
  QCOMPARE(pending.volumeProtect.at("C:"), false);
  QCOMPARE(pending.addFileExclusions.at("C:").front(), std::string("C:\\Cache"));
  QCOMPARE(pending.addRegistryExclusions.front(), std::string("HKEY_LOCAL_MACHINE\\Software\\Vendor"));
  QVERIFY(!pending.addFileExclusions.contains("D:"));

  const auto withoutGlobal = ui::applyImportCommands(commands.mid(0, 1), nullptr, {});
  QCOMPARE(withoutGlobal.front().status, ui::ImportReportRow::Status::Duplicate);
}

void ApplicationUiBehaviorTests::applyPlanPreviewAndCopyUseTheSameProductionCommandMapping() {
  ui::GlobalStatusPanel global;
  const auto snapshot = editableSnapshot();
  global.setData(snapshot.current, snapshot.next, snapshot.runtime);
  QVERIFY(global.importFilterEnabled(false));
  RecordingWmiOperations wmi;
  QTemporaryDir exportDirectory;
  QVERIFY(exportDirectory.isValid());
  const QString exportPath = exportDirectory.filePath(QStringLiteral("commands.txt"));
  {
    QFile previous(exportPath);
    QVERIFY(previous.open(QIODevice::WriteOnly));
    QCOMPARE(previous.write("stale"), qint64{5});
  }
  MemoryFileDialogs files;
  files.savedFile = exportPath;
  ui::ApplyPlanDialog dialog(&global, {}, snapshot, ui::ApplyPlanServices{wmi, files});
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QTextEdit* pending = nullptr;
  for (auto* edit : dialog.findChildren<QTextEdit*>()) {
    if (edit->toPlainText().contains(QStringLiteral("uwfmgr.exe filter disable"))) pending = edit;
  }
  QVERIFY(pending);
  auto* apply = buttonWithText(&dialog, QStringLiteral("Apply"));
  QVERIFY(apply);
  QVERIFY(apply->isEnabled());
  pending->selectAll();
  pending->setFocus();
  QTest::keyClick(pending, Qt::Key_C, Qt::ControlModifier);
  QCOMPARE(QApplication::clipboard()->text().trimmed(), QStringLiteral("uwfmgr.exe filter disable"));
  auto* exportButton = buttonWithText(&dialog, QStringLiteral("Export commands…"));
  QVERIFY(exportButton);
  bool exportSucceeded = false;
  QTimer::singleShot(0, this, [&] {
    if (auto* information = qobject_cast<QDialog*>(QApplication::activeModalWidget())) {
      exportSucceeded = information->windowTitle() == QStringLiteral("Export finished");
      information->accept();
    }
  });
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QVERIFY(exportSucceeded);
  QFile output(exportPath);
  QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
  const QString exportedText = QString::fromUtf8(output.readAll());
  QVERIFY(exportedText.contains(QStringLiteral("uwfmgr.exe filter disable")));
  QVERIFY(exportedText.count(QChar('\n')) > 1);
  QVERIFY(!exportedText.contains(QStringLiteral("::")));
  QCOMPARE(files.requests.size(), 1);
  QVERIFY(files.requests.front().initialPath.contains(QStringLiteral("uwfmgr-commands-")));

  files.savedFile.clear();
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QCOMPARE(files.requests.size(), 2);

  files.savedFile = QDir::tempPath();
  bool failureReported = false;
  QTimer::singleShot(0, this, [&] {
    auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (warning) {
      for (auto* label : warning->findChildren<QLabel*>()) {
        failureReported = failureReported || label->text().contains(QStringLiteral("Could not open file for writing"));
      }
    }
    if (warning) warning->accept();
  });
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QVERIFY(failureReported);
  QCOMPARE(files.requests.size(), 3);
  QVERIFY(wmi.invocations.empty());
}

void ApplicationUiBehaviorTests::applyPlanConfirmedWritePublishesReconciliationAndPreventsReplay() {
  ui::GlobalStatusPanel global;
  const auto snapshot = editableSnapshot();
  global.setData(snapshot.current, snapshot.next, snapshot.runtime);
  QVERIFY(global.importFilterEnabled(false));

  RecordingWmiOperations wmi;
  wmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  wmi.objectResults.push_back(
      {{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(false)}});

  ui::ApplyPlanDialog dialog(&global, {}, snapshot, wmi);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy reconciliation(&dialog, &ui::ApplyPlanDialog::reconciliationRequired);
  auto* apply = buttonWithText(&dialog, QStringLiteral("Apply"));
  auto* restart = dialog.findChild<QPushButton*>(QStringLiteral("restartBtn"));
  QVERIFY(apply && apply->isEnabled());
  QVERIFY(restart && restart->isHidden());

  QTimer::singleShot(0, this, [] {
    if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->accept();
  });
  QTest::mouseClick(apply, Qt::LeftButton);

  QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("Disable")});
  QCOMPARE(reconciliation.count(), 1);
  QVERIFY(!apply->isEnabled());
  QVERIFY(!restart->isHidden());
  bool successReported = false;
  for (auto* edit : dialog.findChildren<QTextEdit*>()) {
    successReported = successReported || edit->toPlainText().contains(QStringLiteral("Filter: Disabled"));
  }
  QVERIFY(successReported);

  QTest::mouseClick(apply, Qt::LeftButton);
  QCOMPARE(wmi.invocations.size(), std::size_t{1});
}

void ApplicationUiBehaviorTests::applyPlanConnectionFailureRemainsRetryableAndDoesNotRequestReconciliation() {
  ui::GlobalStatusPanel global;
  const auto snapshot = editableSnapshot();
  global.setData(snapshot.current, snapshot.next, snapshot.runtime);
  QVERIFY(global.importFilterEnabled(false));
  RecordingWmiOperations wmi;
  wmi.connectionFailure = QStringLiteral("transport unavailable");
  ui::ApplyPlanDialog dialog(&global, {}, snapshot, wmi);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy reconciliation(&dialog, &ui::ApplyPlanDialog::reconciliationRequired);
  auto* apply = buttonWithText(&dialog, QStringLiteral("Apply"));
  QVERIFY(apply && apply->isEnabled());

  QTimer::singleShot(0, this, [] {
    if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->accept();
  });
  QTest::mouseClick(apply, Qt::LeftButton);
  QVERIFY(apply->isEnabled());
  QCOMPARE(reconciliation.count(), 0);
  bool failureReported = false;
  for (auto* edit : dialog.findChildren<QTextEdit*>()) {
    failureReported = failureReported || (edit->toPlainText().contains(QStringLiteral("transport unavailable")) &&
                                          edit->toPlainText().contains(QStringLiteral("Failed to connect")));
  }
  QVERIFY(failureReported);
  QVERIFY(wmi.invocations.empty());
}

void ApplicationUiBehaviorTests::commitDispatcherRejectsUnaddressablePathsAndRestoresTheUsageTimer() {
  RecordingWmiOperations wmi;
  const auto snapshot = editableSnapshot();
  QTimer usage;
  usage.start(60'000);
  ui::CommitDispatcher dispatcher(wmi, snapshot, &usage, nullptr);

  dispatcher.commitFilePath({});
  QVERIFY(usage.isActive());
  QTimer::singleShot(0, this, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->accept();
  });
  dispatcher.commitFilePath(QStringLiteral("relative/path.txt"));
  QVERIFY(usage.isActive());
  QVERIFY(wmi.invocations.empty());
  QVERIFY(wmi.queryResults.empty());
}

void ApplicationUiBehaviorTests::commitDispatcherConfirmsAndReportsARealFileThroughTheTransportBoundary() {
  QTemporaryFile file(QDir::temp().filePath(QStringLiteral("uwf-commit-test-XXXXXX.tmp")));
  QVERIFY(file.open());
  QVERIFY(file.write("committed payload") > 0);
  QVERIFY(file.flush());
  const QString path = QDir::toNativeSeparators(file.fileName());
  const QString drive = ui::extractDriveLetter(path);
  QVERIFY(!drive.isEmpty());

  RecordingWmiOperations wmi;
  wmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                               {"CurrentSession", WmiValue::fromBool(true)},
                               {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                               {"VolumeName", WmiValue::fromString("Volume{test}")},
                               {"BindByDriveLetter", WmiValue::fromBool(true)},
                               {"CommitPending", WmiValue::fromBool(false)},
                               {"Protected", WmiValue::fromBool(true)}}});
  const auto snapshot = editableSnapshot();
  QTimer usage;
  usage.start(60'000);
  ui::CommitDispatcher dispatcher(wmi, snapshot, &usage, nullptr);

  bool confirmationAccepted = false;
  bool reportObserved = false;
  bool reportSucceeded = false;
  QTimer modalDriver;
  modalDriver.setInterval(1);
  connect(&modalDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (auto* table = dialog->findChild<QTableWidget*>(); table && table->columnCount() == 4) {
      reportObserved = true;
      reportSucceeded = table->rowCount() == 1 && table->item(0, 0) && table->item(0, 0)->text() == QStringLiteral("Succeeded") && table->item(0, 1) &&
                        table->item(0, 1)->text() == path;
      dialog->accept();
      return;
    }
    if (auto* box = dialog->findChild<QDialogButtonBox*>()) {
      for (auto* button : box->buttons()) {
        if (box->buttonRole(button) == QDialogButtonBox::AcceptRole) {
          confirmationAccepted = true;
          QTest::mouseClick(button, Qt::LeftButton);
          return;
        }
      }
    }
  });
  modalDriver.start();
  dispatcher.commitFilePath(path);
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

  wmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                               {"CurrentSession", WmiValue::fromBool(true)},
                               {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                               {"VolumeName", WmiValue::fromString("Volume{test}")},
                               {"BindByDriveLetter", WmiValue::fromBool(true)},
                               {"CommitPending", WmiValue::fromBool(false)},
                               {"Protected", WmiValue::fromBool(true)}}});
  QTimer::singleShot(0, this, [] {
    if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->reject();
  });
  dispatcher.commitFilePath(path);
  QCOMPARE(wmi.invocations.size(), std::size_t{1});
  QVERIFY(usage.isActive());
}

void ApplicationUiBehaviorTests::commitDispatcherRoutesAnExistingRegistryValueThroughTheTransportBoundary() {
  const QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
  const QString valueName = QStringLiteral("ProductName");
  RecordingWmiOperations wmi;
  wmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("registry-path")},
                               {"CurrentSession", WmiValue::fromBool(true)},
                               {"PersistDomainSecretKey", WmiValue::fromBool(false)},
                               {"PersistTSCAL", WmiValue::fromBool(false)}}});
  const auto snapshot = editableSnapshot();
  QTimer usage;
  usage.start(60'000);
  ui::CommitDispatcher dispatcher(wmi, snapshot, &usage, nullptr);

  bool confirmationAccepted = false;
  bool reportSucceeded = false;
  QTimer modalDriver;
  modalDriver.setInterval(1);
  connect(&modalDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (auto* table = dialog->findChild<QTableWidget*>(); table && table->columnCount() == 4) {
      reportSucceeded = table->rowCount() == 1 && table->item(0, 0) && table->item(0, 0)->text() == QStringLiteral("Succeeded") && table->item(0, 1) &&
                        table->item(0, 1)->text().contains(valueName);
      dialog->accept();
      return;
    }
    if (auto* box = dialog->findChild<QDialogButtonBox*>()) {
      for (auto* button : box->buttons()) {
        if (box->buttonRole(button) == QDialogButtonBox::AcceptRole) {
          confirmationAccepted = true;
          QTest::mouseClick(button, Qt::LeftButton);
          return;
        }
      }
    }
  });
  modalDriver.start();
  dispatcher.commitRegistryKey(key, valueName);
  modalDriver.stop();

  QVERIFY(confirmationAccepted);
  QVERIFY(reportSucceeded);
  QVERIFY(usage.isActive());
  QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("CommitRegistry")});
  QCOMPARE(wmi.invocationInputs.size(), std::size_t{1});
  QCOMPARE(QString::fromStdString(wmi.invocationInputs.front().at("RegistryKey").toString()), key);
  QCOMPARE(QString::fromStdString(wmi.invocationInputs.front().at("ValueName").toString()), valueName);
}

void ApplicationUiBehaviorTests::safePowerActionsRequireConfirmationAndUseTheInjectedTransport() {
  RecordingWmiOperations wmi;
  ui::PowerController controller(wmi, nullptr);

  QTimer::singleShot(0, this, [] {
    if (auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget())) dialog->reject();
  });
  controller.safeShutdown();
  QVERIFY(wmi.invocations.empty());
  QVERIFY(wmi.queryResults.empty());

  wmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  QTimer::singleShot(0, this, [] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (auto* restart = dialog->findChild<QPushButton*>(QStringLiteral("restartBtn"))) QTest::mouseClick(restart, Qt::LeftButton);
  });
  controller.safeRestart();
  QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("RestartSystem")});
}

void ApplicationUiBehaviorTests::commitBatchUsesAuthoritativeExistenceForEveryOutcome() {
  QStringList categories;
  QStringList reasons;
  auto* watcher = new QTimer(this);
  watcher->setInterval(5);
  connect(watcher, &QTimer::timeout, this, [watcher, &categories, &reasons] {
    for (auto* widget : QApplication::topLevelWidgets()) {
      auto* dialog = qobject_cast<QDialog*>(widget);
      auto* table = dialog ? dialog->findChild<QTableWidget*>() : nullptr;
      if (!table || table->columnCount() != 6 || table->rowCount() != 3) continue;
      for (int row = 0; row < table->rowCount(); ++row) {
        categories.append(table->item(row, 0)->text());
        reasons.append(table->item(row, 5)->text());
      }
      watcher->stop();
      dialog->accept();
      return;
    }
  });
  watcher->start();

  const QList<QString> targets{QStringLiteral("missing"), QStringLiteral("confirmed"), QStringLiteral("still-present")};
  QMap<QString, int> probes;
  int commits = 0;
  ui::runCommitBatch(
      nullptr, QStringLiteral("Delete"), targets, [](const QString& target) { return target; }, [&commits](const QString&) { ++commits; },
      [&probes](const QString& target) {
        const int observation = probes[target]++;
        if (target == QStringLiteral("missing")) return false;
        if (target == QStringLiteral("confirmed")) return observation == 0;
        return true;
      });

  QCOMPARE(commits, 2);
  QCOMPARE(categories.size(), 3);
  QCOMPARE(categories[0], QStringLiteral("Skipped"));
  QCOMPARE(categories[1], QStringLiteral("Succeeded"));
  QCOMPARE(categories[2], QStringLiteral("Failed"));
  QVERIFY(reasons[2].contains(QStringLiteral("still exists")));
}

void ApplicationUiBehaviorTests::uiUtilitiesPreserveDriveComboAndDirtySemantics() {
  QCOMPARE(ui::extractDriveLetter(QStringLiteral("c:/Users/Test")), QStringLiteral("C:"));
  QVERIFY(ui::extractDriveLetter(QStringLiteral("relative/path")).isEmpty());
  QVERIFY(ui::enabledStateLabel(true).contains(QStringLiteral("Enabled")));
  QVERIFY(ui::enabledStateLabel(false).contains(QStringLiteral("Disabled")));

  QComboBox combo;
  combo.addItem(QStringLiteral("A"), 10);
  combo.addItem(QStringLiteral("B"), 20);
  ui::setComboValue(&combo, 20);
  QCOMPARE(combo.currentIndex(), 1);
  ui::setComboValue(&combo, 30);
  QCOMPARE(combo.currentIndex(), 1);
  ui::markDirty(&combo, true);
  QCOMPARE(combo.property("dirty").toBool(), true);
  ui::markDirty(&combo, false);
  QCOMPARE(combo.property("dirty").toBool(), false);

  auto* value = new QLabel(QStringLiteral("value"));
  std::unique_ptr<QWidget> chip(ui::makeSessionChip(QStringLiteral("Current"), QStringLiteral("Current session"), value));
  QCOMPARE(value->parentWidget(), chip.get());
  QCOMPARE(chip->objectName(), QStringLiteral("statusChip"));
}

void ApplicationUiBehaviorTests::mainWindowDistinguishesInitialFailureFromCommittedUnavailableState() {
  RecordingWmiOperations wmi;

  MutableApplicationStateSource failingSource;
  failingSource.failure = QStringLiteral("initial provider failure");
  {
    ui::MainWindow window({wmi, failingSource},
                          {.uwfCapability = UwfCapability::Available, .compatibilityMode = false, .osProductName = {}, .osEditionId = {}});
    window.show();
    QTRY_COMPARE_WITH_TIMEOUT(failingSource.reads, 1, 1000);
    auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"));
    QVERIFY(tabs);
    QCOMPARE(tabs->count(), 0);
    auto* refresh = actionWithText(&window, QStringLiteral("Refresh"));
    auto* import = actionWithText(&window, QStringLiteral("Import"));
    QVERIFY(refresh && refresh->isEnabled());
    QVERIFY(import && !import->isEnabled());
    bool reasonVisible = false;
    for (auto* label : window.findChildren<QLabel*>()) reasonVisible = reasonVisible || label->text().contains(failingSource.failure);
    QVERIFY(reasonVisible);
    window.close();
  }

  MutableApplicationStateSource source;
  source.disks = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;
  source.snapshot.unavailableReason = "embedded provider absent";
  {
    ui::MainWindow window({wmi, source}, {.uwfCapability = UwfCapability::Unavailable,
                                          .compatibilityMode = true,
                                          .osProductName = QStringLiteral("Compatibility OS"),
                                          .osEditionId = QStringLiteral("Custom")});
    window.show();
    QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);
    auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"));
    QVERIFY(tabs);
    QCOMPARE(tabs->count(), 1);
    QCOMPARE(tabs->tabText(0), QStringLiteral("C:"));
    auto* refresh = actionWithText(&window, QStringLiteral("Refresh"));
    auto* import = actionWithText(&window, QStringLiteral("Import"));
    auto* shutdown = actionWithText(&window, QStringLiteral("Safe shutdown"));
    QVERIFY(refresh && refresh->isEnabled());
    QVERIFY(import && !import->isEnabled());
    QVERIFY(shutdown && !shutdown->isEnabled());

    bool compatibilityVisible = false;
    bool unavailableVisible = false;
    for (auto* label : window.findChildren<QLabel*>()) {
      compatibilityVisible = compatibilityVisible || label->text().contains(QStringLiteral("Compatibility OS"));
      unavailableVisible = unavailableVisible || label->text().contains(QStringLiteral("embedded provider absent"));
    }
    QVERIFY(compatibilityVisible);
    QVERIFY(unavailableVisible);

    source.failure = QStringLiteral("later provider failure");
    window.refresh();
    QCOMPARE(source.reads, 2);
    QCOMPARE(tabs->count(), 1);
    QCOMPARE(tabs->tabText(0), QStringLiteral("C:"));
    bool laterFailureLeakedIntoUi = false;
    for (auto* label : window.findChildren<QLabel*>()) laterFailureLeakedIntoUi = laterFailureLeakedIntoUi || label->text().contains(source.failure);
    QVERIFY(!laterFailureLeakedIntoUi);
    window.close();
  }
}

}  // namespace

QTEST_MAIN(ApplicationUiBehaviorTests)

#include "ApplicationUiBehaviorTests.moc"
