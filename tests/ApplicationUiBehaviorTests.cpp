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
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFontInfo>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QScopeGuard>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSpinBox>
#include <QStyle>
#include <QStyleHints>
#include <QStyleOptionButton>
#include <QTabBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextLayout>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QUuid>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "UiTestInput.h"
#include "UiTextChecks.h"
#include "app/FileStagingConflictPolicy.h"
#include "app/FileStagingStore.h"
#include "app/UserConfig.h"
#include "core/UwfModel.h"
#include "service/EnhancedModeAgent.h"
#include "service/EnhancedModeService.h"
#include "ui/AboutDialog.h"
#include "ui/ApplyPlanDialog.h"
#include "ui/CommitBatch.h"
#include "ui/CommitDispatcher.h"
#include "ui/CommitReportDialog.h"
#include "ui/ContentHintBox.h"
#include "ui/DiagnosticReportProvider.h"
#include "ui/Dialogs.h"
#include "ui/DiskTab.h"
#include "ui/EnhancedModeDialog.h"
#include "ui/FileStagingCoordinator.h"
#include "ui/FileStagingWidget.h"
#include "ui/GlobalStatusPanel.h"
#include "ui/I18n.h"
#include "ui/ImportApplier.h"
#include "ui/ImportDialog.h"
#include "ui/LogViewerDialog.h"
#include "ui/MainWindow.h"
#include "ui/OverlayFilesDialog.h"
#include "ui/OverlayHubView.h"
#include "ui/OverlayHudPalette.h"
#include "ui/OverlayUsageBar.h"
#include "ui/PendingCollect.h"
#include "ui/PowerConfirmDialog.h"
#include "ui/PowerController.h"
#include "ui/RegistryPickerDialog.h"
#include "ui/StatusBanner.h"
#include "ui/StatusPanel.h"
#include "ui/SwitchButton.h"
#include "ui/SystemInformationDialog.h"
#include "ui/ThemeManager.h"
#include "ui/UiUtil.h"
#include "util/DriveLetter.h"
#include "util/Log.h"
#include "uwf/FileStagingCommitter.h"
#include "uwf/FileStagingTask.h"
#include "uwf/RegistryTreeCommitter.h"
#include "uwf/UwfSnapshot.h"
#include "uwf/api/UwfmgrCli.h"
#include "uwf/wmi/WmiError.h"
#include "uwf/wmi/WmiException.h"

namespace {

using namespace uwf;

QPushButton* buttonWithText(QWidget* root, const QString& text) {
  for (const auto buttons = root->findChildren<QPushButton*>(); auto* button : buttons) {
    if (button->text() == text) return button;
  }
  return nullptr;
}

QAction* actionWithText(QWidget* root, const QString& text) {
  for (const auto actions = root->findChildren<QAction*>(); auto* action : actions) {
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

service::EnhancedModeStatus completeEnhancedModeStatus() {
  return {.state = service::EnhancedModeState::Enabled,
          .serviceExists = true,
          .ownProcess = true,
          .automaticStart = true,
          .localSystemAccount = true,
          .running = true,
          .executableMatches = true,
          .preshutdownTimeoutConfigured = true,
          .requiredPrivilegesConfigured = true,
          .preshutdownAccepted = true,
          .serviceRegistryPresent = true,
          .agentState = service::EnhancedModeAgentState::Unobserved,
          .detail = {}};
}

class RecordingWmiOperations final : public WmiOperations {
 public:
  mutable std::deque<std::vector<WmiRow>> queryResults;
  mutable std::deque<WmiRow> objectResults;
  mutable std::deque<WmiMethodOutput> readResults;
  mutable std::vector<QString> invocations;
  mutable std::vector<QString> invocationPaths;
  mutable std::vector<WmiRow> invocationInputs;
  mutable std::exception_ptr invocationFailure;
  std::function<void()> invocationObserver;
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
    if (invocationObserver) invocationObserver();
    invocationPaths.push_back(QString::fromStdString(path));
    invocations.push_back(QString::fromStdString(method));
    invocationInputs.push_back(inputs);
    if (invocationFailure) std::rethrow_exception(std::exchange(invocationFailure, std::exception_ptr{}));
  }
  WmiMethodOutput callMethodRead(const std::string&, const std::string&, const WmiRow&) const override {
    if (readResults.empty()) throw std::runtime_error("unexpected callMethodRead");
    auto result = std::move(readResults.front());
    readResults.pop_front();
    return result;
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

class MemoryFileStagingStore final : public app::FileStagingStore {
 public:
  QList<app::FileStagingEntry> entries;
  QString readFailure;
  QString writeFailure;
  int writes = 0;
  mutable int loads = 0;

  [[nodiscard]] QList<app::FileStagingEntry> load() const override {
    ++loads;
    if (!readFailure.isEmpty()) throw std::runtime_error(readFailure.toStdString());
    return entries;
  }

  void replace(const QList<app::FileStagingEntry>& replacement) override {
    if (!writeFailure.isEmpty()) throw std::runtime_error(writeFailure.toStdString());
    entries = replacement;
    ++writes;
  }
};

class MemoryEnhancedModeServiceControl final : public service::EnhancedModeServiceControl {
 public:
  class Barrier final : public DeletionBarrier {
   public:
    explicit Barrier(MemoryEnhancedModeServiceControl& owner) : m_owner(owner) { ++m_owner.activeDeletionBarriers; }
    ~Barrier() override { --m_owner.activeDeletionBarriers; }

   private:
    MemoryEnhancedModeServiceControl& m_owner;
  };

  service::EnhancedModeStatus current;
  QString queryFailure;
  int installs = 0;
  int starts = 0;
  int removals = 0;
  int activeDeletionBarriers = 0;
  mutable int queries = 0;
  mutable bool queriedDuringDeletion = false;
  std::vector<RegistryCommitTarget> installationPlan;
  std::vector<RegistryCommitTarget> deletionPlan;
  QString installedDescription;

  [[nodiscard]] service::EnhancedModeStatus query() const override {
    ++queries;
    if (!queryFailure.isEmpty()) throw std::runtime_error(queryFailure.toStdString());
    if (activeDeletionBarriers != 0) queriedDuringDeletion = true;
    return current;
  }
  void installAndStart(const QString& description) override {
    ++installs;
    installedDescription = description;
    current = completeEnhancedModeStatus();
  }
  void start() override {
    ++starts;
    current.running = true;
    current.preshutdownAccepted = true;
    current.state = current.serviceContractSatisfied() ? service::EnhancedModeState::Enabled : service::EnhancedModeState::RepairRequired;
  }
  [[nodiscard]] std::vector<RegistryCommitTarget> planRegistryCommit() const override { return installationPlan; }
  [[nodiscard]] std::vector<RegistryCommitTarget> planRegistryDeletion() const override { return deletionPlan; }
  void deleteRegistryRemnants() override { current.serviceRegistryPresent = false; }
  [[nodiscard]] Removal stopAndMarkForDeletion() override {
    ++removals;
    current = {};
    return {RemovalState::MarkedForDeletion, std::make_unique<Barrier>(*this)};
  }
};

class MemoryEnhancedModeAgent final : public service::EnhancedModeAgentConnection {
 public:
  int starts = 0;
  int stops = 0;
  std::vector<bool> preshutdownCommitStates;

  void start() override {
    if (m_running) return;
    m_running = true;
    ++starts;
  }

  void stop() override {
    if (!m_running) return;
    m_running = false;
    ++stops;
  }

  [[nodiscard]] bool running() const override { return m_running; }
  [[nodiscard]] bool markPreshutdownCommitHandled() override {
    preshutdownCommitStates.push_back(true);
    return true;
  }
  [[nodiscard]] bool requirePreshutdownCommit() override {
    preshutdownCommitStates.push_back(false);
    return true;
  }

  void publishConnectionState(const bool connected) { emit connectionStateChanged(connected); }

 private:
  bool m_running = false;
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

FileStagingCommitResult completeFileStaging(FileStagingCommitter& committer, const QList<app::FileStagingEntry>& entries) {
  auto operation = committer.beginCommit(committer.prepare(FileStagingCommitter::scan(committer.prepareScan(entries))));
  while (!operation.finished()) static_cast<void>(operation.advance());
  return operation.result();
}

WmiMethodOutput fileExclusionResult(const std::initializer_list<std::string_view> paths = {}) {
  WmiMethodOutput result;
  auto& rows = result.arrays["ExcludedFiles"];
  rows.reserve(paths.size());
  for (const auto path : paths) rows.push_back({{"FileName", WmiValue::fromString(std::string(path))}});
  return result;
}

FileStagingCommitOperation beginFileStagingWithTemporaryCommitter(WmiOperations& wmi, const QList<app::FileStagingEntry>& entries) {
  FileStagingCommitter committer(wmi);
  return committer.beginCommit(committer.prepare(FileStagingCommitter::scan(committer.prepareScan(entries))));
}

class ApplicationUiBehaviorTests final : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void languageAndThemeChangesReachProductionWidgets();
  void qtStandardWidgetsUseSupplementalTranslations_data();
  void qtStandardWidgetsUseSupplementalTranslations();
  void buttonContrastAndSwitchFocusStayVisible_data();
  void buttonContrastAndSwitchFocusStayVisible();
  void reportDetailsPreserveCompleteText_data();
  void reportDetailsPreserveCompleteText();
  void startupPreferencesReachTheRealUiWithoutPersistingToolbarChanges_data();
  void startupPreferencesReachTheRealUiWithoutPersistingToolbarChanges();
  void startupHiddenHubRemainsHiddenAcrossRefreshAndUiRebuild_data();
  void startupHiddenHubRemainsHiddenAcrossRefreshAndUiRebuild();
  void aboutDialogExposesVersionLicenseAndSafeClose();
  void diagnosticReportDistinguishesMissingStateAndBoundsLogs();
  void commonDialogsPreserveSafeDefaultsAndPreviewPagination();
  void logBufferAndViewerPreserveMalformedAndStructuredLines();
  void hintAndUsageWidgetsHandleEmptyOverflowAndThresholdEdges();
  void diskTabsApplyCapabilityBoundariesAndPreserveInnerSelection();
  void diskTabKeepsExclusionsBoundToEachSessionVolume();
  void fileStagingTabPersistsImmediateEditsAndCollapsesCoveredPaths();
  void fileStagingTabPreservesStateAcrossInvalidInputAndStorageFailures();
  void fileStagingAndFileExclusionsRejectOverlapFromEitherEntryPoint();
  void fileStagingCommitterTreatsPersistentAndAbsentTargetsAsIdempotent();
  void fileStagingTaskSkipsUnavailableCapabilityWithoutWmi();
  void fileStagingCommandUiCoalescesRequestsAndCompletesEveryTarget();
  void enhancedModeServiceContractRejectsEveryBrokenInvariant();
  void enhancedModeDialogReflectsDisabledEnabledAndRepairStates();
  void enhancedModeRemovalPreflightsProtectedRegistryPersistence();
  void mainWindowEnhancedModeActionTracksTheServiceLifecycle();
  void mainWindowRetriesTransientEnhancedModeStatusFailures();
  void mainWindowTreatsAuthenticatedServiceConnectionAsStatusOnly();
  void emptyAndMissingRegistryPlansSkipUwfInvocation();
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
  void mainWindowMountsFileStagingOnlyWherePerFileCommitIsSupported();
  void mainWindowQuietStartupInitializesWhileRemainingHidden();
  void mainWindowDistinguishesInitialFailureFromCommittedUnavailableState();
  void mainWindowCompactLayoutKeepsControlsReachable_data();
  void mainWindowCompactLayoutKeepsControlsReachable();
  void mainWindowCompactToolbarKeepsLanguageMenuUsable();
  void dialogLayoutsKeepActionsVisible_data();
  void dialogLayoutsKeepActionsVisible();
  void longMessageDialogsKeepCompleteTextAndReachableActions_data();
  void longMessageDialogsKeepCompleteTextAndReachableActions();
  void typographyUsesNativeFontsAndAntialiasedText();

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
  QCOMPARE(ui::I18n::enhancedModeServiceDescription(), QStringLiteral("UWF Manager 增强模式辅助服务"));
  ui::AboutDialog localizedDialog;
  QVERIFY(buttonWithText(&localizedDialog, QStringLiteral("关闭")));
  QVERIFY(!buttonWithText(&localizedDialog, QStringLiteral("Close")));
  ui::I18n::instance().setLang(ui::I18n::Lang::En);
  QCOMPARE(ui::I18n::enhancedModeServiceDescription(), QStringLiteral("UWF Manager enhanced mode helper service"));

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

void ApplicationUiBehaviorTests::qtStandardWidgetsUseSupplementalTranslations_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  for (const auto locale : {ui::I18n::Lang::Id, ui::I18n::Lang::Vi})
    for (const bool dark : {false, true})
      QTest::newRow(locale == ui::I18n::Lang::Id ? (dark ? "id-dark" : "id-light") : (dark ? "vi-dark" : "vi-light")) << locale << dark;
}

void ApplicationUiBehaviorTests::qtStandardWidgetsUseSupplementalTranslations() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  auto& language = ui::I18n::instance();
  auto& theme = ui::ThemeManager::instance();
  const auto oldLanguage = language.lang();
  const auto oldTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    language.setLang(oldLanguage);
    theme.apply(oldTheme);
  });
  language.setLang(locale);
  theme.apply(dark ? ui::Theme::Dark : ui::Theme::Light);
  const bool indonesian = locale == ui::I18n::Lang::Id;
  const QString copy = indonesian ? QStringLiteral("Salin") : QStringLiteral("Sao chép");
  const QString cancel = indonesian ? QStringLiteral("Batal") : QStringLiteral("Hủy");
  const QString save = indonesian ? QStringLiteral("Simpan") : QStringLiteral("Lưu");
  QCOMPARE(QCoreApplication::translate("QAbstractSpinBox", "&Step up"), indonesian ? QStringLiteral("&Naikkan") : QStringLiteral("&Tăng"));
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  const auto capture = [&](QWidget& widget, const QString& name) {
    QTRY_VERIFY2(test::clippedLabel(widget).isEmpty(), qPrintable(test::clippedLabel(widget)));
    if (!screenshots.isEmpty()) {
      QVERIFY(QDir().mkpath(screenshots));
      QVERIFY(widget.grab().save(QDir(screenshots).filePath(name + "-" + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
    }
  };
  QDialog dialog;
  auto* layout = new QVBoxLayout(&dialog);
  auto* line = new QLineEdit(QStringLiteral("C:\\Example.txt"), &dialog);
  auto* text = new QTextEdit(QStringLiteral("Example"), &dialog);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
  layout->addWidget(line);
  layout->addWidget(text);
  layout->addWidget(buttons);
  dialog.resize(520, 280);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QCOMPARE(buttons->button(QDialogButtonBox::Cancel)->text().remove('&'), cancel);
  QCOMPARE(buttons->button(QDialogButtonBox::Save)->text().remove('&'), save);
  capture(dialog, QStringLiteral("qt-buttons"));
  const auto inspectMenu = [&](std::unique_ptr<QMenu> menu, const QString& name) {
    bool copyPresent = false;
    for (auto* action : menu->actions()) copyPresent = copyPresent || action->text().section('\t', 0, 0).remove('&') == copy;
    QVERIFY(copyPresent);
    menu->popup(dialog.mapToGlobal(QPoint(20, 30)));
    QTRY_VERIFY(menu->isVisible());
    for (auto* action : menu->actions()) {
      if (action->isSeparator()) continue;
      QVERIFY(menu->rect().contains(menu->actionGeometry(action)));
    }
    capture(*menu, name);
    menu->hide();
  };
  inspectMenu(std::unique_ptr<QMenu>(line->createStandardContextMenu()), QStringLiteral("qt-line-menu"));
  inspectMenu(std::unique_ptr<QMenu>(text->createStandardContextMenu()), QStringLiteral("qt-text-menu"));
  dialog.hide();
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFileDialog files(nullptr, {}, directory.path());
  files.setOptions(QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly);
  files.setFileMode(QFileDialog::ExistingFile);
  files.resize(900, 620);
  files.show();
  QVERIFY(QTest::qWaitForWindowExposed(&files));
  QVERIFY(buttonWithText(&files, cancel));
  QStringList fileLabels;
  for (auto* label : files.findChildren<QLabel*>()) fileLabels.append(label->text().remove('&'));
  QVERIFY(fileLabels.contains(indonesian ? QStringLiteral("Lihat di:") : QStringLiteral("Tìm trong:")));
  QVERIFY(fileLabels.contains(indonesian ? QStringLiteral("Jenis berkas:") : QStringLiteral("Loại tệp:")));
  capture(files, QStringLiteral("qt-file-dialog"));
}

void ApplicationUiBehaviorTests::buttonContrastAndSwitchFocusStayVisible_data() {
  QTest::addColumn<bool>("dark");
  QTest::newRow("light") << false;
  QTest::newRow("dark") << true;
}

void ApplicationUiBehaviorTests::buttonContrastAndSwitchFocusStayVisible() {
  QFETCH(bool, dark);
  auto& theme = ui::ThemeManager::instance();
  const auto original = theme.current();
  const auto restore = qScopeGuard([&] { theme.apply(original); });
  theme.apply(dark ? ui::Theme::Dark : ui::Theme::Light);
  QDialog host;
  auto* layout = new QVBoxLayout(&host);
  auto* primary = new QPushButton(QStringLiteral("Apply"), &host);
  primary->setObjectName(QStringLiteral("primaryBtn"));
  auto* danger = new QPushButton(QStringLiteral("Remove"), &host);
  danger->setObjectName(QStringLiteral("dangerBtn"));
  auto* enable = new QPushButton(QStringLiteral("Enable enhanced mode"), &host);
  enable->setObjectName(QStringLiteral("enhancedModeChangeButton"));
  enable->setProperty("actionStyle", "enable");
  auto* repair = new QPushButton(QStringLiteral("Repair enhanced mode"), &host);
  repair->setObjectName(QStringLiteral("enhancedModeChangeButton"));
  repair->setProperty("actionStyle", "repair");
  auto* toggle = new ui::SwitchButton(&host);
  auto* other = new QPushButton(QStringLiteral("Other"), &host);
  layout->addWidget(primary);
  layout->addWidget(danger);
  layout->addWidget(enable);
  layout->addWidget(repair);
  layout->addWidget(toggle);
  layout->addWidget(other);
  QWidget::setTabOrder(other, toggle);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  host.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&host));
  const auto luminance = [](const QColor& color) {
    const auto linear = [](const double value) { return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4); };
    return 0.2126 * linear(color.redF()) + 0.7152 * linear(color.greenF()) + 0.0722 * linear(color.blueF());
  };
  const auto surface = [](QPushButton* button) {
    const auto image = button->grab().toImage();
    return image.pixelColor(image.width() / 2, qRound(5 * image.devicePixelRatio()));
  };
  const auto contrast = [&](QPushButton* button, const QColor& background) {
    const double foreground = luminance(button->palette().color(QPalette::ButtonText));
    const double backdrop = luminance(background);
    return (std::max(foreground, backdrop) + 0.05) / (std::min(foreground, backdrop) + 0.05);
  };
  for (auto* button : {primary, danger, enable, repair}) {
    QTest::mouseMove(host.windowHandle(), other->mapTo(&host, other->rect().center()));
    QTRY_VERIFY(!button->underMouse());
    const QColor normal = surface(button);
    QVERIFY(contrast(button, normal) >= 4.5);
    QTest::mouseMove(host.windowHandle(), button->mapTo(&host, button->rect().center()));
    QTRY_VERIFY(button->underMouse());
    QTRY_VERIFY(surface(button) != normal);
    const QColor hovered = surface(button);
    QVERIFY(hovered != normal);
    QVERIFY(contrast(button, hovered) >= 4.5);
    QTest::mousePress(button, Qt::LeftButton);
    if (button == primary || button == danger) QTRY_VERIFY(surface(button) != hovered);
    const QColor pressed = surface(button);
    QVERIFY(contrast(button, pressed) >= 4.5);
    QTest::mouseRelease(button, Qt::LeftButton);
  }
  other->setFocus();
  QTRY_VERIFY(other->hasFocus());
  const auto before = toggle->grab().toImage();
  const QSize originalSize = toggle->size();
  QTest::keyClick(other, Qt::Key_Tab);
  QTRY_VERIFY(toggle->hasFocus());
  QVERIFY(toggle->grab().toImage() != before);
  QCOMPARE(toggle->size(), originalSize);
  QTest::keyClick(toggle, Qt::Key_Space);
  QVERIFY(toggle->isChecked());
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!screenshots.isEmpty()) {
    QVERIFY(QDir().mkpath(screenshots));
    QVERIFY(host.grab().save(QDir(screenshots).filePath(QStringLiteral("focus-") + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
  }
  QTest::keyClick(toggle, Qt::Key_Tab, Qt::ShiftModifier);
  QTRY_VERIFY(!toggle->hasFocus());
}

void ApplicationUiBehaviorTests::reportDetailsPreserveCompleteText_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  for (const auto& language : ui::I18n::languages()) {
    if (language.id != ui::I18n::Lang::En && language.id != ui::I18n::Lang::Zh_CN && language.id != ui::I18n::Lang::Ar) continue;
    for (const bool dark : {false, true}) QTest::newRow((QString(language.code) + (dark ? "-dark" : "-light")).toLatin1().constData()) << language.id << dark;
  }
}

void ApplicationUiBehaviorTests::reportDetailsPreserveCompleteText() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  auto& language = ui::I18n::instance();
  auto& theme = ui::ThemeManager::instance();
  const auto oldLanguage = language.lang();
  const auto oldTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    language.setLang(oldLanguage);
    theme.apply(oldTheme);
    clearLogLines();
  });
  language.setLang(locale);
  theme.apply(dark ? ui::Theme::Dark : ui::Theme::Light);
  const QString fullText = QStringLiteral("<literal>& data 中文 %1\n") + QStringLiteral("long-details-").repeated(1000);
  const auto inspectDetails = [&](const std::function<void()>& open, const QString& screenshot) {
    bool inspected = false;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, [&] {
      auto* details = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      QVERIFY(details);
      const auto close = qScopeGuard([&] { details->reject(); });
      QCOMPARE(details->objectName(), QStringLiteral("tableRowDetailsDialog"));
      auto* text = details->findChild<QPlainTextEdit*>(QStringLiteral("tableRowDetailsText"));
      QVERIFY(text && text->isReadOnly());
      QVERIFY(text->toPlainText().contains(fullText));
      auto* copy = buttonWithText(details, ui::I18n::tr("Copy all"));
      QVERIFY(copy);
      QTRY_VERIFY_WITH_TIMEOUT((copy->click(), QApplication::clipboard()->text().contains(fullText)), 2000);
      const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
      if (!screenshots.isEmpty()) {
        QVERIFY(QDir().mkpath(screenshots));
        QVERIFY(details->grab().save(QDir(screenshots).filePath(screenshot + "-" + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
      }
      inspected = true;
    });
    timer.start(20);
    open();
    timer.stop();
    QVERIFY(inspected);
  };
  clearLogLines();
  logLine('E', "details-test", fullText.toStdString());
  {
    ui::LogViewerDialog dialog;
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* table = dialog.findChild<QTableWidget*>();
    QVERIFY(table && !table->showGrid());
    QTRY_VERIFY(table->rowCount() > 0);
    QTest::qWait(80);
    QVERIFY(!table->item(0, 1)->icon().isNull());
    QCOMPARE(table->item(0, 1)->text(), QStringLiteral("E"));
    const auto position = table->viewport()->mapTo(&dialog, table->visualItemRect(table->item(0, 3)).center());
    QTest::mouseClick(dialog.windowHandle(), Qt::LeftButton, Qt::NoModifier, position);
    QTRY_VERIFY(!table->selectedItems().isEmpty());
    auto* button = dialog.findChild<QPushButton*>(QStringLiteral("tableDetailsButton"));
    QVERIFY(button && button->isEnabled());
    inspectDetails([&] { button->click(); }, QStringLiteral("log-details"));
    inspectDetails(
        [&] {
          const auto point = table->visualItemRect(table->item(0, 3)).center();
          QTest::mouseClick(table->viewport(), Qt::LeftButton, Qt::NoModifier, point);
          QTest::mouseDClick(table->viewport(), Qt::LeftButton, Qt::NoModifier, point);
          QTest::mouseRelease(table->viewport(), Qt::LeftButton, Qt::NoModifier, point);
        },
        QStringLiteral("log-details-double-click"));
    dialog.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&dialog));
    table->setFocus();
    QTRY_VERIFY(table->hasFocus());
    inspectDetails([&] { QTest::keyClick(table, Qt::Key_Return); }, QStringLiteral("log-details-enter"));
  }
  bool reportInspected = false;
  QTimer::singleShot(0, this, [&] {
    auto* report = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QVERIFY(report);
    const auto close = qScopeGuard([&] { report->reject(); });
    auto* table = report->findChild<QTableWidget*>();
    QVERIFY(table && !table->showGrid());
    QCOMPARE(table->rowCount(), 1);
    QVERIFY(!table->item(0, 0)->icon().isNull());
    const auto position = table->viewport()->mapTo(report, table->visualItemRect(table->item(0, 0)).center());
    QTest::mouseClick(report->windowHandle(), Qt::LeftButton, Qt::NoModifier, position);
    QTRY_VERIFY(!table->selectedItems().isEmpty());
    auto* button = report->findChild<QPushButton*>(QStringLiteral("tableDetailsButton"));
    QVERIFY(button && button->isEnabled());
    inspectDetails([&] { button->click(); }, QStringLiteral("commit-details"));
    reportInspected = true;
  });
  ui::showCommitReport(nullptr, {{ui::I18n::tr("Failed"), QStringLiteral("C:\\Example.txt"), QStringLiteral("0x80041001"), fullText, true, true}},
                       ui::CommitOperation::Persist);
  QVERIFY(reportInspected);
}

void ApplicationUiBehaviorTests::startupPreferencesReachTheRealUiWithoutPersistingToolbarChanges_data() {
  QTest::addColumn<QByteArray>("contents");
  QTest::addColumn<std::optional<ui::Theme>>("expectedTheme");
  QTest::addColumn<std::optional<ui::I18n::Lang>>("expectedLanguage");
  struct Case {
    const char* name;
    QByteArray contents;
    std::optional<ui::Theme> theme;
    std::optional<ui::I18n::Lang> language;
  };
  const std::array cases{
      Case{"dark-english", "[UI]\nTheme=dark\nLanguage=en\n", ui::Theme::Dark, ui::I18n::Lang::En},
      Case{"light-chinese", "[UI]\nTheme=light\nLanguage=zh_CN\n", ui::Theme::Light, ui::I18n::Lang::Zh_CN},
      Case{"dark-chinese", "[UI]\nTheme=dark\nLanguage=zh_CN\n", ui::Theme::Dark, ui::I18n::Lang::Zh_CN},
      Case{"dark-french", "[UI]\nTheme=dark\nLanguage=fr\n", ui::Theme::Dark, ui::I18n::Lang::Fr},
      Case{"light-arabic", "[UI]\nTheme=light\nLanguage=ar\n", ui::Theme::Light, ui::I18n::Lang::Ar},
      Case{"light-english", "[UI]\nTheme=light\nLanguage=en\n", ui::Theme::Light, ui::I18n::Lang::En},
      Case{"system", "[UI]\nTheme=system\nLanguage=system\n", {}, {}},
      Case{"empty", {}, {}, {}},
      Case{"invalid", "[UI]\nTheme=unknown\nLanguage=unknown\n", {}, {}},
      Case{"malformed", "[UI]\nTheme=dark\nLanguage=zh_CN\n[unterminated\n", {}, {}},
      Case{"partly-valid", "[UI]\nTheme=bad-value\nLanguage=zh_CN\n", {}, ui::I18n::Lang::Zh_CN},
  };
  for (const auto& entry : cases) QTest::newRow(entry.name) << entry.contents << entry.theme << entry.language;
}

void ApplicationUiBehaviorTests::startupPreferencesReachTheRealUiWithoutPersistingToolbarChanges() {
  QFETCH(QByteArray, contents);
  QFETCH(std::optional<ui::Theme>, expectedTheme);
  QFETCH(std::optional<ui::I18n::Lang>, expectedLanguage);
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFile file(directory.filePath(QStringLiteral("UWF.ini")));
  QVERIFY(file.open(QIODevice::WriteOnly));
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  const auto modifiedBefore = QFileInfo(file).lastModified();

  auto& theme = ui::ThemeManager::instance();
  auto& language = ui::I18n::instance();
  const ui::Theme originalTheme = theme.current();
  const ui::I18n::Lang originalLanguage = language.lang();
  const auto restorePreferences = qScopeGuard([&] {
    theme.apply(originalTheme);
    language.setLang(originalLanguage);
  });
  const auto config = app::readUserConfig(directory.path());
  theme.applyStartupTheme(config.theme);
  language.applyStartupLanguage(config.language);
  const ui::Theme startupTheme = expectedTheme ? *expectedTheme : ui::ThemeManager::detectSystemTheme();
  const ui::I18n::Lang startupLanguage = expectedLanguage ? *expectedLanguage : ui::I18n::detectSystemLang();
  QCOMPARE(theme.current(), startupTheme);
  QCOMPARE(language.lang(), startupLanguage);
  QCOMPARE(QGuiApplication::styleHints()->colorScheme(), startupTheme == ui::Theme::Light ? Qt::ColorScheme::Light : Qt::ColorScheme::Dark);

  {
    RecordingWmiOperations wmi;
    MemoryFileStagingStore staging;
    MutableApplicationStateSource source;
    MemoryEnhancedModeServiceControl serviceControl;
    MemoryEnhancedModeAgent agent;
    source.snapshot.uwfAvailable = false;
    source.snapshot.unavailableReason = "provider unavailable in fixture";
    // 所有系统能力均停留在既有内存边界，真实窗口不会触及 UWF、SCM 或代理管道。
    ui::MainWindow window(
        {wmi, source, staging, &serviceControl, &agent},
        {.uwfCapability = UwfCapability::Unavailable, .compatibilityMode = false, .osProductName = {}, .osEditionId = {}, .hubVisible = config.hubVisible});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 2000);
    QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 2000);
    const QString expectedTitle = startupLanguage == ui::I18n::Lang::En      ? QStringLiteral("Unified Write Filter (UWF) Manager")
                                  : startupLanguage == ui::I18n::Lang::Zh_CN ? QStringLiteral("统一写入筛选器 (UWF) 管理器")
                                                                             : ui::I18n::applicationTitle();
    const QString expectedRefresh = startupLanguage == ui::I18n::Lang::En      ? QStringLiteral("Refresh")
                                    : startupLanguage == ui::I18n::Lang::Zh_CN ? QStringLiteral("刷新")
                                                                               : ui::I18n::tr("Refresh");
    QCOMPARE(window.windowTitle(), expectedTitle);
    QVERIFY(actionWithText(&window, expectedRefresh));
    QCOMPARE(window.palette().color(QPalette::Window), theme.color(ui::Sem::Bg));
    QVERIFY(!QApplication::activeModalWidget());

    QToolButton* themeButton = nullptr;
    for (const auto buttons = window.findChildren<QToolButton*>(); auto* button : buttons) {
      if (button->defaultAction() && button->defaultAction()->toolTip() == ui::I18n::tr("Toggle light / dark theme")) {
        themeButton = button;
      }
    }
    QVERIFY(themeButton && themeButton->isVisible() && themeButton->isEnabled());
    QVERIFY(!themeButton->icon().isNull());
    QSignalSpy themeChanges(&theme, &ui::ThemeManager::themeChanged);
    QTest::mouseClick(themeButton, Qt::LeftButton);
    const ui::Theme toggledTheme = startupTheme == ui::Theme::Dark ? ui::Theme::Light : ui::Theme::Dark;
    QTRY_COMPARE_WITH_TIMEOUT(theme.current(), toggledTheme, 2000);
    QTRY_COMPARE_WITH_TIMEOUT(source.reads, 2, 2000);
    QCOMPARE(language.lang(), startupLanguage);
    QVERIFY(window.updatesEnabled());
    QCOMPARE(window.palette().color(QPalette::Window), theme.color(ui::Sem::Bg));

    const ui::I18n::Lang toggledLanguage = startupLanguage == ui::I18n::Lang::En ? ui::I18n::Lang::Zh_CN : ui::I18n::Lang::En;
    QToolButton* languageButton = nullptr;
    for (const auto buttons = window.findChildren<QToolButton*>(); auto* button : buttons) {
      if (button->defaultAction() && button->defaultAction()->toolTip() == ui::I18n::tr("Switch display language")) languageButton = button;
    }
    QVERIFY(languageButton && languageButton->isVisible() && languageButton->isEnabled() && languageButton->defaultAction()->menu());
    const QPointer<QMenu> menu = languageButton->defaultAction()->menu();
    QPointer<QAction> selectedLanguage;
    for (const auto actions = menu->actions(); auto* action : actions) {
      if (action->text() == (toggledLanguage == ui::I18n::Lang::En ? QStringLiteral("English") : QStringLiteral("简体中文"))) selectedLanguage = action;
    }
    QVERIFY(selectedLanguage && selectedLanguage->isEnabled() && !selectedLanguage->isChecked());
    bool menuOpened = false;
    QTimer menuDriver;
    menuDriver.setInterval(1);
    connect(&menuDriver, &QTimer::timeout, this, [&] {
      if (!menu || !selectedLanguage || !menu->isVisible()) return;
      menuDriver.stop();
      menuOpened = true;
      QTest::mouseClick(menu.data(), Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(selectedLanguage.data()).center());
    });
    // InstantPopup 会进入嵌套事件循环。超时只关闭本进程的弹出菜单，让按钮
    // 接线或选择行为错误表现为断言失败，而不是让整套测试永久等待。
    QTimer menuTimeout;
    menuTimeout.setSingleShot(true);
    connect(&menuTimeout, &QTimer::timeout, this, [] {
      if (auto* popup = QApplication::activePopupWidget()) popup->close();
    });
    menuDriver.start();
    menuTimeout.start(2000);
    // 重建会销毁工具栏按钮，窗口本身保持不变；窗口级事件入口可安全发送整次
    // 点击的按下/释放事件，不会在弹出菜单返回后再访问已销毁的旧按钮。
    const QPoint languageButtonPosition = window.mapFromGlobal(languageButton->mapToGlobal(languageButton->rect().center()));
    QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, languageButtonPosition);
    menuDriver.stop();
    menuTimeout.stop();
    QVERIFY(menuOpened);
    QTRY_COMPARE_WITH_TIMEOUT(language.lang(), toggledLanguage, 2000);
    QTRY_COMPARE_WITH_TIMEOUT(source.reads, 3, 2000);
    QCOMPARE(window.windowTitle(),
             toggledLanguage == ui::I18n::Lang::En ? QStringLiteral("Unified Write Filter (UWF) Manager") : QStringLiteral("统一写入筛选器 (UWF) 管理器"));
    QVERIFY(actionWithText(&window, toggledLanguage == ui::I18n::Lang::En ? QStringLiteral("Refresh") : QStringLiteral("刷新")));
    QCOMPARE(theme.current(), toggledTheme);
    QCOMPARE(themeChanges.count(), 1);
    QCOMPARE(window.palette().color(QPalette::Window), theme.color(ui::Sem::Bg));
    QVERIFY(window.updatesEnabled());
    QVERIFY(wmi.invocations.empty());
    QCOMPARE(staging.writes, 0);
    QCOMPARE(agent.starts, 0);
  }

  QVERIFY(file.open(QIODevice::ReadOnly));
  QCOMPARE(file.readAll(), contents);
  file.close();
  QCOMPARE(QFileInfo(file).lastModified(), modifiedBefore);
  QCOMPARE(QDir(directory.path()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System), QStringList{QStringLiteral("UWF.ini")});
}

void ApplicationUiBehaviorTests::startupHiddenHubRemainsHiddenAcrossRefreshAndUiRebuild_data() {
  QTest::addColumn<bool>("quiet");
  QTest::addColumn<bool>("filterEnabled");
  QTest::newRow("interactive-filter-enabled") << false << true;
  QTest::newRow("interactive-filter-disabled") << false << false;
  QTest::newRow("quiet-filter-enabled") << true << true;
  QTest::newRow("quiet-filter-disabled") << true << false;
}

void ApplicationUiBehaviorTests::startupHiddenHubRemainsHiddenAcrossRefreshAndUiRebuild() {
  QFETCH(bool, quiet);
  QFETCH(bool, filterEnabled);
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFile file(directory.filePath(QStringLiteral("UWF.ini")));
  const QByteArray contents("[UI]\nHubVisible=false\n");
  QVERIFY(file.open(QIODevice::WriteOnly));
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  const auto modifiedBefore = QFileInfo(file).lastModified();
  const auto config = app::readUserConfig(directory.path());

  auto& theme = ui::ThemeManager::instance();
  auto& language = ui::I18n::instance();
  const auto originalTheme = theme.current();
  const auto originalLanguage = language.lang();
  const auto restorePreferences = qScopeGuard([&] {
    theme.apply(originalTheme);
    language.setLang(originalLanguage);
  });
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  MutableApplicationStateSource source;
  MemoryEnhancedModeServiceControl serviceControl;
  MemoryEnhancedModeAgent agent;
  source.snapshot = editableSnapshot();
  source.snapshot.current.filter.enabled = filterEnabled;
  ui::MainWindow window(
      {wmi, source, staging, &serviceControl, &agent},
      {.uwfCapability = UwfCapability::Available, .compatibilityMode = false, .osProductName = {}, .osEditionId = {}, .hubVisible = config.hubVisible});
  QList<QPointer<ui::OverlayHubView>> views;
  for (const auto widgets = QApplication::allWidgets(); auto* widget : widgets)
    if (auto* view = qobject_cast<ui::OverlayHubView*>(widget)) views.append(view);
  // 部分系统会静态过滤不兼容的任务栏端点；实际注册的端点均须保持未展示。
  QVERIFY(!views.isEmpty());
  const auto hubIsHidden = [&] {
    auto* action = actionWithText(&window, QStringLiteral("HUB"));
    return action && action->isVisible() == source.snapshot.current.filter.enabled && !action->isChecked() && std::ranges::all_of(views, [](const auto& view) {
             return view && !view->isVisible() && view->displayState() == ui::OverlayHubView::DisplayState::Disabled;
           });
  };
  if (quiet)
    window.startInTray();
  else
    window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 2000);
  QCOMPARE(window.isVisible(), !quiet);
  QVERIFY(hubIsHidden());
  if (quiet) {
    window.raiseToFront();
    QTRY_COMPARE_WITH_TIMEOUT(source.reads, 2, 2000);
  }
  QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 2000);

  // 请求隐藏保存在展示模型中；UWF 保护状态变化和工具栏重建均不得将其覆盖。
  for (const bool enabled : {false, true}) {
    source.snapshot.current.filter.enabled = enabled;
    const int before = source.reads;
    window.refresh();
    QCOMPARE(source.reads, before + 1);
    QVERIFY(hubIsHidden());
  }
  int before = source.reads;
  theme.toggle();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, before + 1, 2000);
  QVERIFY(hubIsHidden());
  auto* languageAction = actionWithText(&window, ui::I18n::tr("Language"));
  QVERIFY(languageAction && languageAction->menu());
  auto* otherLanguage = actionWithText(languageAction->menu(), language.lang() == ui::I18n::Lang::En ? QStringLiteral("简体中文") : QStringLiteral("English"));
  QVERIFY(otherLanguage);
  before = source.reads;
  otherLanguage->trigger();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, before + 1, 2000);
  QVERIFY(hubIsHidden());
  QVERIFY(wmi.invocations.empty());
  QCOMPARE(staging.writes, 0);
  QCOMPARE(agent.starts, 0);
  QVERIFY(file.open(QIODevice::ReadOnly));
  QCOMPARE(file.readAll(), contents);
  QCOMPARE(QFileInfo(file).lastModified(), modifiedBefore);
  QCOMPARE(QDir(directory.path()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System), QStringList{QStringLiteral("UWF.ini")});
}

void ApplicationUiBehaviorTests::aboutDialogExposesVersionLicenseAndSafeClose() {
  ui::AboutDialog dialog;
  QVERIFY(dialog.minimumWidth() >= 500);
  bool hasVersion = false;
  bool hasLicense = false;
  bool hasSourceLink = false;
  for (const auto labels = dialog.findChildren<QLabel*>(); auto* label : labels) {
    hasVersion = hasVersion || label->text().contains(QStringLiteral("Version "));
    hasLicense = hasLicense || label->text().contains(QStringLiteral("GNU General Public License"));
    hasSourceLink = hasSourceLink || label->text().contains(QStringLiteral("github.com/HsingYun/UWF-Manager"));
  }
  QVERIFY(hasVersion);
  QVERIFY(hasLicense);
  QVERIFY(hasSourceLink);
  QVERIFY(buttonWithText(&dialog, QStringLiteral("System information")));
  auto* close = buttonWithText(&dialog, QStringLiteral("Close"));
  QVERIFY(close);
  QSignalSpy finished(&dialog, &QDialog::finished);
  QTest::mouseClick(close, Qt::LeftButton);
  QCOMPARE(finished.count(), 1);
  QCOMPARE(finished.at(0).at(0).toInt(), static_cast<int>(QDialog::Accepted));
}

void ApplicationUiBehaviorTests::diagnosticReportDistinguishesMissingStateAndBoundsLogs() {
  clearLogLines();
  const QString missingReport = ui::DiagnosticReportProvider::diagnosticText({});
  QVERIFY(missingReport.contains(QStringLiteral("snapshot.available = no")));
  QVERIFY(!missingReport.contains(QStringLiteral("user.account = <empty>")));
  QVERIFY(!missingReport.contains(QStringLiteral("user.sid = <empty>")));
  QVERIFY(missingReport.contains(QStringLiteral("user.account = ")) || missingReport.contains(QStringLiteral("user.account.error = ")) ||
          missingReport.contains(QStringLiteral("user.error = ")));
  QVERIFY(missingReport.contains(QStringLiteral("user.sid = ")) || missingReport.contains(QStringLiteral("user.sid.error = ")) ||
          missingReport.contains(QStringLiteral("user.error = ")));
  QVERIFY(missingReport.contains(QStringLiteral("integrity.level = ")) || missingReport.contains(QStringLiteral("integrity.error = ")));

  const core::UwfSnapshot snapshot = editableSnapshot();
  const service::EnhancedModeStatus enhancedMode = completeEnhancedModeStatus();
  const auto context = ui::DiagnosticReportProvider::capture(UwfCapability::Available, &snapshot, &enhancedMode, true);
  QString report = ui::DiagnosticReportProvider::diagnosticText(context);
  QVERIFY(report.contains(QStringLiteral("snapshot.available = yes")));
  QVERIFY(report.contains(QStringLiteral("current.filter.enabled = yes")));
  QVERIFY(report.contains(QStringLiteral("next.overlay.type = disk")));
  QVERIFY(report.contains(QStringLiteral("service.contract_satisfied = yes")));

  clearLogLines();
  logLine('I', "diagnostic-test", "diagnostic-oldest-marker");
  for (int index = 0; index < 68; ++index) logLine('I', "diagnostic-test", "diagnostic-middle-" + std::to_string(index));
  logLine('W', "diagnostic-test", "diagnostic-newest-marker-" + std::string(2048, 'x'));
  report = ui::DiagnosticReportProvider::diagnosticText(context);
  QVERIFY(report.contains(QStringLiteral("lines.count = 64")));
  QVERIFY(!report.contains(QStringLiteral("diagnostic-oldest-marker")));
  QVERIFY(report.contains(QStringLiteral("diagnostic-newest-marker")));
  QVERIFY(report.contains(QStringLiteral("... <truncated>")));
  clearLogLines();
}

void ApplicationUiBehaviorTests::commonDialogsPreserveSafeDefaultsAndPreviewPagination() {
  bool warningSelectable = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    for (const auto labels = dialog->findChildren<ui::ContentHintBox*>(); auto* label : labels) {
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
      for (const auto buttons = box->buttons(); auto* button : buttons) {
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

void ApplicationUiBehaviorTests::hintAndUsageWidgetsHandleEmptyOverflowAndThresholdEdges() {
  QScrollArea host;
  host.setWidgetResizable(true);
  host.resize(240, 160);
  auto* hint = new ui::ContentHintBox(&host);
  host.setWidget(hint);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  hint->setText(QStringLiteral("short"));
  QTRY_COMPARE_WITH_TIMEOUT(hint->verticalScrollBar()->maximum(), 0, 500);
  const int shortHeight = hint->height();
  hint->setText(QStringLiteral("a long information line ").repeated(50));
  QTRY_VERIFY_WITH_TIMEOUT(hint->height() > shortHeight, 1000);
  QTRY_COMPARE_WITH_TIMEOUT(hint->verticalScrollBar()->maximum(), 0, 1000);
  QVERIFY(hint->document()->size().height() <= hint->viewport()->height());
  QVERIFY(host.verticalScrollBar()->maximum() > 0);
  host.verticalScrollBar()->setValue(0);
  test::sendWheel(*host.viewport(), QPoint(0, -120));
  QTRY_VERIFY(host.verticalScrollBar()->value() > 0);
  const int narrowHeight = hint->height();
  host.resize(480, 160);
  QTRY_VERIFY_WITH_TIMEOUT(hint->height() < narrowHeight, 1000);
  QTRY_COMPARE_WITH_TIMEOUT(hint->verticalScrollBar()->maximum(), 0, 1000);
  hint->setText(QStringLiteral("short again"));
  QTRY_COMPARE_WITH_TIMEOUT(host.verticalScrollBar()->maximum(), 0, 1000);
  hint->clear();
  QTRY_COMPARE_WITH_TIMEOUT(hint->verticalScrollBar()->maximum(), 0, 500);

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

  banner.setObjectName(QStringLiteral("statusBanner"));
  const QString longError = QStringLiteral("WMI query failed: <raw error> & code\nC:\\") + QString(180, QLatin1Char('x')) + QStringLiteral("\\data\n");
  for (const int width : {180, 320, 480}) {
    banner.setText(longError + QStringLiteral("Ending AAA"));
    const int required = banner.heightForWidth(width);
    QVERIFY(required > 2 * banner.fontMetrics().height());
    QVERIFY(banner.minimumSizeHint().width() <= width);
    banner.resize(width, required);
    QImage first(banner.size(), QImage::Format_ARGB32_Premultiplied);
    first.fill(Qt::transparent);
    banner.render(&first);
    banner.setText(longError + QStringLiteral("Ending BBB"));
    QCOMPARE(banner.heightForWidth(width), required);
    QImage second(banner.size(), QImage::Format_ARGB32_Premultiplied);
    second.fill(Qt::transparent);
    banner.render(&second);
    // 只改变末行，末行所在区域必须实际绘制变化，不能仅验证控件返回的高度。
    const int lastLineHeight = 2 * banner.fontMetrics().height();
    const QRect lastLine(0, required - lastLineHeight, width, lastLineHeight);
    QVERIFY(first.copy(lastLine) != second.copy(lastLine));
  }
  const auto errorTextColor = banner.palette().color(QPalette::WindowText);
  banner.setProperty("level", "warn");
  QVERIFY(banner.palette().color(QPalette::WindowText) != errorTextColor);
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
  c.setActiveInfoPage(ui::DiskTab::InfoPage::RegistryExclusions);
  QCOMPARE(c.activeInfoPage(), ui::DiskTab::InfoPage::RegistryExclusions);
  c.setActiveInfoPage(ui::DiskTab::InfoPage::FileStaging);
  QCOMPARE(c.activeInfoPage(), ui::DiskTab::InfoPage::RegistryExclusions);

  MemoryFileDialogs dialogs;
  MemoryFileStagingStore staging;
  ui::DiskTab stagedSupported(supported, false, dialogs, staging);
  ui::DiskTab stagedLimited(limited, false, dialogs, staging);
  ui::DiskTab stagedUnsupported(unsupported, false, dialogs, staging);
  QCOMPARE(stagedSupported.findChildren<ui::FileStagingWidget*>().size(), 1);
  QCOMPARE(stagedLimited.findChildren<ui::FileStagingWidget*>().size(), 0);
  QCOMPARE(stagedUnsupported.findChildren<ui::FileStagingWidget*>().size(), 0);
  QVERIFY(!ui::diskSupportText(core::DiskSupport::FileSystemLimited, "exFAT").empty());
  QVERIFY(!ui::diskSupportText(core::DiskSupport::ExceedsMaxSize, "NTFS").empty());
}

void ApplicationUiBehaviorTests::diskTabKeepsExclusionsBoundToEachSessionVolume() {
  const core::DiskInfo disk{"C:", "", "NTFS", "System", 1000, 500, core::DiskSupport::Supported};
  ui::DiskTab tab(disk, false);
  auto snapshot = editableSnapshot();
  snapshot.current.volumes.front().volumeName = "Volume{mounted-guid}";
  snapshot.next.volumes.front().volumeName = "Volume{old}";
  snapshot.next.volumes.front().bindByDriveLetter = true;
  snapshot.current.fileExclusions = {{"Volume{mounted-guid}", {R"(C:\CurrentOnly)"}}, {"Volume{old}", {R"(C:\WrongCurrent)"}}};
  snapshot.next.fileExclusions = {{"Volume{old}", {R"(C:\NextOnly)"}}, {"Volume{mounted-guid}", {R"(C:\WrongNext)"}}};
  tab.applySnapshot(snapshot);
  tab.resize(700, 450);
  tab.show();
  QVERIFY(QTest::qWaitForWindowExposed(&tab));
  auto* tabs = tab.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(tabs);
  auto* files = qobject_cast<ui::ExclusionListWidget*>(tabs->widget(0));
  QVERIFY(files);
  auto* list = files->findChild<QListWidget*>(QStringLiteral("exclusionList"));
  QVERIFY(list);
  QCOMPARE(list->count(), 2);
  bool currentObserved = false;
  bool nextObserved = false;
  for (int index = 0; index < list->count(); ++index) {
    const auto* item = list->item(index);
    const auto path = item->data(Qt::UserRole).toString();
    if (path == QStringLiteral(R"(C:\CurrentOnly)")) {
      currentObserved = true;
      QVERIFY(item->toolTip().contains(QStringLiteral("Current session: Excluded\nNext session: Not excluded")));
    } else if (path == QStringLiteral(R"(C:\NextOnly)")) {
      nextObserved = true;
      QVERIFY(item->toolTip().contains(QStringLiteral("Current session: Not excluded\nNext session: Excluded")));
    } else {
      QFAIL("the displayed exclusions came from the wrong session volume");
    }
  }
  QVERIFY(currentObserved);
  QVERIFY(nextObserved);
  QVERIFY(files->pendingAdded().isEmpty());
  QVERIFY(files->pendingRemoved().isEmpty());
  const auto labels = tab.findChildren<QLabel*>();
  QVERIFY(std::ranges::any_of(labels, [](const QLabel* label) { return label->text().contains(QStringLiteral("mounted-guid")); }));

  const auto nextVolumes = snapshot.next.volumes;
  snapshot.next.volumes.clear();
  tab.applySnapshot(snapshot);
  QCOMPARE(list->count(), 1);
  QCOMPARE(list->item(0)->data(Qt::UserRole).toString(), QStringLiteral(R"(C:\CurrentOnly)"));

  snapshot.next.volumes = nextVolumes;
  snapshot.current.volumes.clear();
  tab.applySnapshot(snapshot);
  QCOMPARE(list->count(), 1);
  QCOMPARE(list->item(0)->data(Qt::UserRole).toString(), QStringLiteral(R"(C:\NextOnly)"));

  snapshot.next.volumes.clear();
  tab.applySnapshot(snapshot);
  QCOMPARE(list->count(), 0);
  QVERIFY(files->pendingAdded().isEmpty());
  QVERIFY(files->pendingRemoved().isEmpty());
}

void ApplicationUiBehaviorTests::fileStagingTabPersistsImmediateEditsAndCollapsesCoveredPaths() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString filePath = QDir::toNativeSeparators(directory.filePath(QStringLiteral("state.bin")));
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  QVERIFY(file.write("state") > 0);
  file.close();
  const QString diskDrive = ui::extractDriveLetter(filePath);
  QVERIFY(!diskDrive.isEmpty());
  const QString otherDrive = diskDrive.compare(QStringLiteral("C:"), Qt::CaseInsensitive) == 0 ? QStringLiteral("D:") : QStringLiteral("C:");
  const app::FileStagingEntry otherVolumeEntry{app::FileStagingKind::File, otherDrive + QStringLiteral("\\Other\\keep.bin")};

  MemoryFileDialogs dialogs;
  MemoryFileStagingStore store;
  store.entries = {otherVolumeEntry};
  const core::DiskInfo disk{diskDrive.toStdString(), "Volume{current}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported};
  ui::DiskTab tab(disk, true, dialogs, store);

  auto* innerTabs = tab.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(innerTabs);
  QCOMPARE(innerTabs->count(), 3);
  QCOMPARE(innerTabs->tabText(0), QStringLiteral("File exclusions"));
  QCOMPARE(innerTabs->tabText(1), QStringLiteral("Registry exclusions"));
  QCOMPARE(innerTabs->tabText(2), QStringLiteral("File staging"));

  auto* staging = qobject_cast<ui::FileStagingWidget*>(innerTabs->widget(2));
  QVERIFY(staging);
  QCOMPARE(staging->driveLetter(), diskDrive);
  auto* addFiles = staging->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* addDirectory = staging->findChild<QAction*>(QStringLiteral("stageDirectoryAction"));
  auto* list = staging->findChild<QListWidget*>(QStringLiteral("exclusionList"));
  auto* filter = staging->findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(addFiles && addDirectory && list && filter);
  QCOMPARE(list->count(), 0);

  dialogs.openedFile = filePath;
  addFiles->trigger();
  QCOMPARE(store.writes, 1);
  QCOMPARE(store.entries.size(), 2);
  const app::FileStagingEntry currentVolumeFile{app::FileStagingKind::File, filePath};
  QVERIFY(store.entries.contains(otherVolumeEntry));
  QVERIFY(store.entries.contains(currentVolumeFile));
  QCOMPARE(list->count(), 1);

  // Windows 路径大小写不敏感；同一文件的不同盘符大小写不能产生第二条记录。
  QString alternateCase = filePath;
  alternateCase[0] = alternateCase[0].toLower();
  dialogs.openedFile = alternateCase;
  addFiles->trigger();
  QCOMPARE(store.writes, 1);

  filter->setText(QStringLiteral("not-present"));
  QVERIFY(list->item(0)->isHidden());
  filter->setText(QFileInfo(filePath).fileName());
  QVERIFY(!list->item(0)->isHidden());
  filter->clear();

  dialogs.selectedDirectory = QDir::toNativeSeparators(directory.path());
  addDirectory->trigger();
  QCOMPARE(store.writes, 2);
  QCOMPARE(store.entries.size(), 2);
  const app::FileStagingEntry currentVolumeDirectory{app::FileStagingKind::Directory, dialogs.selectedDirectory};
  QVERIFY(store.entries.contains(otherVolumeEntry));
  QVERIFY(store.entries.contains(currentVolumeDirectory));

  dialogs.openedFile = filePath;
  addFiles->trigger();
  QCOMPARE(store.writes, 2);
  QCOMPARE(list->count(), 1);
  list->item(0)->setSelected(true);
  auto* remove = buttonWithText(staging, QStringLiteral("Remove selected"));
  QVERIFY(remove);
  QTest::mouseClick(remove, Qt::LeftButton);
  QCOMPARE(store.writes, 3);
  QCOMPARE(store.entries, QList<app::FileStagingEntry>{otherVolumeEntry});
}

void ApplicationUiBehaviorTests::fileStagingTabPreservesStateAcrossInvalidInputAndStorageFailures() {
  QTemporaryFile file;
  QVERIFY(file.open());
  const QString filePath = QDir::toNativeSeparators(file.fileName());
  const QString drive = ui::extractDriveLetter(filePath);
  QVERIFY(!drive.isEmpty());

  MemoryFileDialogs dialogs;
  MemoryFileStagingStore store;
  store.entries = {{app::FileStagingKind::File, filePath}};
  ui::FileStagingWidget staging(drive, store, dialogs);
  auto* list = staging.findChild<QListWidget*>(QStringLiteral("exclusionList"));
  auto* remove = buttonWithText(&staging, QStringLiteral("Remove selected"));
  auto* addDirectory = staging.findChild<QAction*>(QStringLiteral("stageDirectoryAction"));
  QVERIFY(list && remove && addDirectory);
  QCOMPARE(list->count(), 1);

  // 只读与存储故障是两种状态：未提权时仍能筛选、选择和复制现有条目，
  // 但任何程序化或真实按钮触发都不能进入持久化边界。
  auto* addFiles = staging.findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* filter = staging.findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(addFiles && filter);
  staging.setReadOnly(true);
  QVERIFY(!addFiles->isEnabled());
  QVERIFY(!addDirectory->isEnabled());
  QVERIFY(!remove->isEnabled());
  QVERIFY(list->isEnabled());
  QVERIFY(filter->isEnabled());
  dialogs.openedFile = filePath;
  addFiles->trigger();
  QCOMPARE(store.writes, 0);
  staging.setReadOnly(false);
  QVERIFY(addFiles->isEnabled());
  QVERIFY(addDirectory->isEnabled());
  QVERIFY(remove->isEnabled());

  dialogs.selectedDirectory = filePath;
  QTimer::singleShot(0, &staging, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->reject();
  });
  addDirectory->trigger();
  QCOMPARE(store.writes, 0);
  QCOMPARE(list->count(), 1);

  dialogs.openedFile = QStringLiteral("C:relative-state.bin");
  QTimer::singleShot(0, &staging, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->reject();
  });
  addFiles->trigger();
  QCOMPARE(store.writes, 0);
  QCOMPARE(list->count(), 1);

  const QString otherDrive = drive.compare(QStringLiteral("C:"), Qt::CaseInsensitive) == 0 ? QStringLiteral("D:") : QStringLiteral("C:");
  MemoryFileStagingStore crossDriveStore;
  ui::FileStagingWidget crossDrive(otherDrive, crossDriveStore, dialogs);
  auto* crossDriveAction = crossDrive.findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* crossDriveList = crossDrive.findChild<QListWidget*>(QStringLiteral("exclusionList"));
  QVERIFY(crossDriveAction && crossDriveList);
  dialogs.openedFile = filePath;
  QTimer::singleShot(0, &crossDrive, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->reject();
  });
  crossDriveAction->trigger();
  QCOMPARE(crossDriveStore.writes, 0);
  QVERIFY(crossDriveStore.entries.isEmpty());
  QCOMPARE(crossDriveList->count(), 0);
  QCOMPARE(dialogs.requests.back().initialPath, ui::dialogs::dialogBasePath(otherDrive));

  store.writeFailure = QStringLiteral("registry write failed");
  list->item(0)->setSelected(true);
  QTimer::singleShot(0, &staging, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->reject();
  });
  QTest::mouseClick(remove, Qt::LeftButton);
  QCOMPARE(store.writes, 0);
  QCOMPARE(store.entries.size(), 1);
  QCOMPARE(list->count(), 1);

  MemoryFileStagingStore unavailableStore;
  unavailableStore.readFailure = QStringLiteral("registry read failed");
  ui::FileStagingWidget unavailable(drive, unavailableStore, dialogs);
  auto* unavailableList = unavailable.findChild<QListWidget*>(QStringLiteral("exclusionList"));
  auto* unavailableFilter = unavailable.findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(unavailableList && unavailableFilter);
  QVERIFY(!unavailableList->isEnabled());
  QVERIFY(!unavailableFilter->isEnabled());
}

void ApplicationUiBehaviorTests::fileStagingAndFileExclusionsRejectOverlapFromEitherEntryPoint() {
  QTemporaryFile file;
  QVERIFY(file.open());
  const QString filePath = QDir::toNativeSeparators(file.fileName());
  const QString drive = ui::extractDriveLetter(filePath);
  QVERIFY(!drive.isEmpty());

  app::FileStagingConflictPolicy conflicts;
  MemoryFileDialogs dialogs;
  ui::ExclusionListWidget exclusions(ui::ExclusionListWidget::Kind::File, dialogs, conflicts);
  exclusions.setDriveLetter(drive);
  exclusions.setBaseline({filePath}, {filePath});

  MemoryFileStagingStore emptyStore;
  ui::FileStagingWidget staging(drive, emptyStore, dialogs, conflicts);
  dialogs.openedFile = filePath;
  QTimer::singleShot(0, &staging, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->reject();
  });
  auto* addFiles = staging.findChild<QAction*>(QStringLiteral("stageFilesAction"));
  QVERIFY(addFiles);
  addFiles->trigger();
  QCOMPARE(emptyStore.writes, 0);
  QVERIFY(emptyStore.entries.isEmpty());

  MemoryFileStagingStore stagedStore;
  stagedStore.entries = {{app::FileStagingKind::File, filePath}};
  ui::FileStagingWidget staged(drive, stagedStore, dialogs, conflicts);
  QVERIFY(staged.findChild<QListWidget*>(QStringLiteral("exclusionList")));
  exclusions.setBaseline({}, {});
  QCOMPARE(exclusions.importAdd(filePath), ui::ExclusionListWidget::ImportOutcome::RejectedConflict);
  QVERIFY(exclusions.pendingAdded().isEmpty());
}

void ApplicationUiBehaviorTests::fileStagingCommitterTreatsPersistentAndAbsentTargetsAsIdempotent() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString existingPath = QDir::toNativeSeparators(directory.filePath(QStringLiteral("state.bin")));
  QFile file(existingPath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  QVERIFY(file.write("state") > 0);
  file.close();
  const QString drive = ui::extractDriveLetter(existingPath);
  QVERIFY(!drive.isEmpty());

  std::stop_source canceledScan;
  canceledScan.request_stop();
  RecordingWmiOperations canceledWmi;
  canceledWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                       {"CurrentSession", WmiValue::fromBool(true)},
                                       {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                                       {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                       {"BindByDriveLetter", WmiValue::fromBool(true)},
                                       {"CommitPending", WmiValue::fromBool(false)},
                                       {"Protected", WmiValue::fromBool(true)}}});
  canceledWmi.readResults.push_back(fileExclusionResult());
  FileStagingCommitter canceledCommitter(canceledWmi);
  auto canceledOperation = canceledCommitter.beginCommit(canceledCommitter.prepare(
      FileStagingCommitter::scan(canceledCommitter.prepareScan({{app::FileStagingKind::File, existingPath}}), canceledScan.get_token())));
  QVERIFY(canceledOperation.finished());
  QCOMPARE(canceledOperation.result().discoveredFiles, 0);
  QVERIFY(canceledWmi.queryResults.empty());
  QVERIFY(canceledWmi.invocations.empty());

  RecordingWmiOperations absentWmi;
  absentWmi.queryResults.push_back({});
  FileStagingCommitter absentCommitter(absentWmi);
  const auto absentResult =
      completeFileStaging(absentCommitter, {{app::FileStagingKind::File, QDir::toNativeSeparators(directory.filePath(QStringLiteral("missing.bin")))}});
  QCOMPARE(absentResult.skippedEntries, 1);
  QCOMPARE(absentResult.discoveredFiles, 0);
  QVERIFY(absentResult.succeeded());
  QVERIFY(absentWmi.queryResults.empty());
  QVERIFY(absentWmi.invocations.empty());

  RecordingWmiOperations unprotectedWmi;
  unprotectedWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                          {"CurrentSession", WmiValue::fromBool(true)},
                                          {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                                          {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                          {"BindByDriveLetter", WmiValue::fromBool(true)},
                                          {"CommitPending", WmiValue::fromBool(false)},
                                          {"Protected", WmiValue::fromBool(false)}}});
  FileStagingCommitter unprotectedCommitter(unprotectedWmi);
  const auto unprotectedResult = completeFileStaging(unprotectedCommitter, {{app::FileStagingKind::File, existingPath}});
  QCOMPARE(unprotectedResult.discoveredFiles, 0);
  QCOMPARE(unprotectedResult.skippedEntries, 1);
  QVERIFY(unprotectedResult.succeeded());
  QVERIFY(unprotectedWmi.invocations.empty());

  RecordingWmiOperations excludedWmi;
  excludedWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                       {"CurrentSession", WmiValue::fromBool(true)},
                                       {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                                       {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                       {"BindByDriveLetter", WmiValue::fromBool(true)},
                                       {"CommitPending", WmiValue::fromBool(false)},
                                       {"Protected", WmiValue::fromBool(true)}}});
  excludedWmi.readResults.push_back(fileExclusionResult({existingPath.toStdString()}));
  FileStagingCommitter excludedCommitter(excludedWmi);
  const auto excludedResult = completeFileStaging(excludedCommitter, {{app::FileStagingKind::File, existingPath}});
  QCOMPARE(excludedResult.skippedEntries, std::size_t{1});
  QCOMPARE(excludedResult.discoveredFiles, std::size_t{0});
  QVERIFY(excludedResult.succeeded());
  QVERIFY(excludedWmi.invocations.empty());

  RecordingWmiOperations nestedExclusionWmi;
  nestedExclusionWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                              {"CurrentSession", WmiValue::fromBool(true)},
                                              {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                                              {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                              {"BindByDriveLetter", WmiValue::fromBool(true)},
                                              {"CommitPending", WmiValue::fromBool(false)},
                                              {"Protected", WmiValue::fromBool(true)}}});
  nestedExclusionWmi.readResults.push_back(fileExclusionResult({existingPath.toStdString()}));
  FileStagingCommitter nestedExclusionCommitter(nestedExclusionWmi);
  const auto nestedExclusionResult =
      completeFileStaging(nestedExclusionCommitter, {{app::FileStagingKind::Directory, QDir::toNativeSeparators(directory.path())}});
  QCOMPARE(nestedExclusionResult.skippedFiles, std::size_t{1});
  QCOMPARE(nestedExclusionResult.discoveredFiles, std::size_t{0});
  QVERIFY(nestedExclusionResult.succeeded());
  QVERIFY(nestedExclusionWmi.invocations.empty());

  RecordingWmiOperations exclusionReadFailureWmi;
  exclusionReadFailureWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                                   {"CurrentSession", WmiValue::fromBool(true)},
                                                   {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                                                   {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                                   {"BindByDriveLetter", WmiValue::fromBool(true)},
                                                   {"CommitPending", WmiValue::fromBool(false)},
                                                   {"Protected", WmiValue::fromBool(true)}}});
  FileStagingCommitter exclusionReadFailureCommitter(exclusionReadFailureWmi);
  const auto exclusionReadFailureResult = completeFileStaging(exclusionReadFailureCommitter, {{app::FileStagingKind::File, existingPath}});
  QCOMPARE(exclusionReadFailureResult.failures.size(), 1);
  QCOMPARE(exclusionReadFailureResult.failures.front().kind, FileStagingCommitFailureKind::ProviderFailure);
  QVERIFY(exclusionReadFailureWmi.invocations.empty());

  RecordingWmiOperations unchangedWmi;
  unchangedWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                        {"CurrentSession", WmiValue::fromBool(true)},
                                        {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                                        {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                        {"BindByDriveLetter", WmiValue::fromBool(true)},
                                        {"CommitPending", WmiValue::fromBool(false)},
                                        {"Protected", WmiValue::fromBool(true)}}});
  unchangedWmi.readResults.push_back(fileExclusionResult());
  unchangedWmi.invocationFailure =
      std::make_exception_ptr(WmiProviderError(static_cast<uint32_t>(static_cast<int32_t>(WmiErrorCode::NotFound)), "commit staged file"));
  FileStagingCommitter unchangedCommitter(unchangedWmi);
  const auto unchangedResult = completeFileStaging(unchangedCommitter, {{app::FileStagingKind::File, existingPath}});
  QCOMPARE(unchangedResult.discoveredFiles, 1);
  QCOMPARE(unchangedResult.skippedFiles, 1);
  QCOMPARE(unchangedWmi.invocations, std::vector<QString>{QStringLiteral("CommitFile")});
  QVERIFY(unchangedResult.succeeded());

  RecordingWmiOperations detachedOperationWmi;
  detachedOperationWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                                {"CurrentSession", WmiValue::fromBool(true)},
                                                {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                                                {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                                {"BindByDriveLetter", WmiValue::fromBool(true)},
                                                {"CommitPending", WmiValue::fromBool(false)},
                                                {"Protected", WmiValue::fromBool(true)}}});
  detachedOperationWmi.readResults.push_back(fileExclusionResult());
  auto detachedOperation = beginFileStagingWithTemporaryCommitter(detachedOperationWmi, {{app::FileStagingKind::File, existingPath}});
  QCOMPARE(detachedOperation.totalFiles(), std::size_t{1});
  QCOMPARE(detachedOperation.advance(), existingPath);
  QVERIFY(detachedOperation.finished());
  QCOMPARE(detachedOperationWmi.invocations, std::vector<QString>{QStringLiteral("CommitFile")});

  RecordingWmiOperations relativeWmi;
  relativeWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                       {"CurrentSession", WmiValue::fromBool(true)},
                                       {"DriveLetter", WmiValue::fromString("C:")},
                                       {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                       {"BindByDriveLetter", WmiValue::fromBool(true)},
                                       {"CommitPending", WmiValue::fromBool(false)},
                                       {"Protected", WmiValue::fromBool(true)}}});
  FileStagingCommitter relativeCommitter(relativeWmi);
  const auto relativeResult = completeFileStaging(relativeCommitter, {{app::FileStagingKind::File, QStringLiteral("C:relative-state.bin")}});
  QCOMPARE(relativeResult.discoveredFiles, std::size_t{0});
  QCOMPARE(relativeResult.failures.size(), 1);
  QCOMPARE(relativeResult.failures.front().kind, FileStagingCommitFailureKind::InvalidPath);
  QVERIFY(relativeWmi.invocations.empty());
}

void ApplicationUiBehaviorTests::fileStagingTaskSkipsUnavailableCapabilityWithoutWmi() {
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\one.bin")}, {app::FileStagingKind::Directory, QStringLiteral("D:\\State")}};

  FileStagingTask task(wmi, staging, UwfCapability::Unavailable);
  QVERIFY(task.pollPreparation());
  QVERIFY(task.preparationFinished());
  QVERIFY(task.finished());
  QCOMPARE(task.totalFiles(), std::size_t{0});
  QCOMPARE(task.result().skippedEntries, std::size_t{2});
  QVERIFY(task.result().succeeded());
  QVERIFY(wmi.queryResults.empty());
  QVERIFY(wmi.invocations.empty());
}

void ApplicationUiBehaviorTests::fileStagingCommandUiCoalescesRequestsAndCompletesEveryTarget() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QStringList paths;
  for (const QString& name : {QString(180, u'a') + QStringLiteral(".bin"), QString(180, u'b') + QStringLiteral(".bin")}) {
    const QString path = QDir::toNativeSeparators(directory.filePath(name));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write(name.toUtf8()) > 0);
    paths.append(path);
  }
  const QString drive = ui::extractDriveLetter(paths.front());
  QVERIFY(!drive.isEmpty());

  RecordingWmiOperations wmi;
  wmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  wmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                               {"CurrentSession", WmiValue::fromBool(true)},
                               {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                               {"VolumeName", WmiValue::fromString("Volume{staging}")},
                               {"BindByDriveLetter", WmiValue::fromBool(true)},
                               {"CommitPending", WmiValue::fromBool(false)},
                               {"Protected", WmiValue::fromBool(true)}}});
  wmi.readResults.push_back(fileExclusionResult());
  MemoryFileStagingStore store;
  for (const QString& path : paths) store.entries.append({app::FileStagingKind::File, path});

  QWidget parent;
  parent.show();
  ui::FileStagingCoordinator coordinator(wmi, store, UwfCapability::Available, &parent);
  QSignalSpy progress(&coordinator, &ui::FileStagingCoordinator::progressChanged);
  connect(&coordinator, &ui::FileStagingCoordinator::progressChanged, &coordinator,
          [](std::size_t, std::size_t) { throw std::runtime_error("progress observer failure"); });
  int completions = 0;
  app::ApplicationCommandResult firstResult;
  app::ApplicationCommandResult secondResult;
  QElapsedTimer visibleDuration;
  visibleDuration.start();
  coordinator.requestCommit([&](const ui::FileStagingBatchResult& result) {
    firstResult = result.command;
    ++completions;
  });
  coordinator.requestCommit([&](const ui::FileStagingBatchResult& result) {
    secondResult = result.command;
    ++completions;
  });
  bool externalOwnershipTransferred = false;
  bool requestQueuedDuringExternalOwnership = false;
  std::optional<app::ApplicationCommandResult> queuedResult;
  auto immediateBatch = coordinator.reserveExternalBatch([&](const ui::FileStagingBatchResult& result, ui::FileStagingCoordinator::ExternalBatch batch) {
    externalOwnershipTransferred = coordinator.active();
    QCOMPARE(result.sourceEntries, std::optional<QList<app::FileStagingEntry>>{store.entries});
    coordinator.requestCommit([&](const ui::FileStagingBatchResult& queued) {
      requestQueuedDuringExternalOwnership = true;
      queuedResult = queued.command;
    });
    QVERIFY(!requestQueuedDuringExternalOwnership);
    batch.complete(result);
  });
  QVERIFY(!immediateBatch.has_value());

  auto* dialog = parent.findChild<QDialog*>(QStringLiteral("fileStagingProgressDialog"));
  QTRY_VERIFY_WITH_TIMEOUT(dialog && dialog->isVisible(), 1000);
  QVERIFY(dialog->windowFlags().testFlag(Qt::FramelessWindowHint));
  QVERIFY(dialog->findChildren<QPushButton*>().isEmpty());
  QVERIFY(dialog->findChild<QProgressBar*>(QStringLiteral("fileStagingCommandProgress")));
  auto* currentPath = dialog->findChild<QLineEdit*>(QStringLiteral("fileStagingCommandPath"));
  QVERIFY(currentPath && currentPath->isReadOnly());
  QTRY_VERIFY_WITH_TIMEOUT(!currentPath->text().isEmpty(), 1000);
  QVERIFY(dialog->width() <= 760);
  QVERIFY(paths.contains(currentPath->text()));
  QTRY_COMPARE_WITH_TIMEOUT(completions, 2, 5000);
  QVERIFY(visibleDuration.elapsed() >= 1900);
  QCOMPARE(firstResult.outcome, app::ApplicationCommandOutcome::Succeeded);
  QCOMPARE(firstResult.committedFiles, std::size_t{2});
  QCOMPARE(secondResult.committedFiles, firstResult.committedFiles);
  QVERIFY(externalOwnershipTransferred);
  QVERIFY(requestQueuedDuringExternalOwnership);
  QVERIFY(queuedResult.has_value());
  QCOMPARE(queuedResult->committedFiles, firstResult.committedFiles);
  QCOMPARE(wmi.invocations, std::vector<QString>({QStringLiteral("CommitFile"), QStringLiteral("CommitFile")}));
  QVERIFY(!progress.isEmpty());

  auto exclusiveBatch = coordinator.reserveExternalBatch([](const ui::FileStagingBatchResult&, ui::FileStagingCoordinator::ExternalBatch) {});
  QVERIFY(exclusiveBatch.has_value());
  QVERIFY_THROWS_EXCEPTION(
      std::logic_error,
      static_cast<void>(coordinator.reserveExternalBatch([](const ui::FileStagingBatchResult&, ui::FileStagingCoordinator::ExternalBatch) {})));
  bool completionAfterThrowWasDelivered = false;
  coordinator.requestCommit([](const ui::FileStagingBatchResult&) { throw std::runtime_error("observer failure"); });
  coordinator.requestCommit([&](const ui::FileStagingBatchResult&) { completionAfterThrowWasDelivered = true; });
  exclusiveBatch->complete({firstResult, store.entries});
  QVERIFY(completionAfterThrowWasDelivered);
  QVERIFY(!coordinator.active());
}

void ApplicationUiBehaviorTests::enhancedModeDialogReflectsDisabledEnabledAndRepairStates() {
  RecordingWmiOperations wmi;
  MemoryEnhancedModeServiceControl serviceControl;
  service::EnhancedModeManager manager(serviceControl, wmi, UwfCapability::Unavailable);
  ui::EnhancedModeDialog dialog(manager);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  auto* status = dialog.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* change = dialog.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* remove = dialog.findChild<QPushButton*>(QStringLiteral("enhancedModeRemoveButton"));
  QVERIFY(status && change && remove);
  QCOMPARE(status->text(), QStringLiteral("Status: Disabled"));
  QCOMPARE(change->text(), QStringLiteral("Enable enhanced mode"));
  QVERIFY(!remove->isEnabled());
  QTest::mouseClick(change, Qt::LeftButton);
  QCOMPARE(serviceControl.installs, 1);
  QCOMPARE(serviceControl.installedDescription, QStringLiteral("UWF Manager enhanced mode helper service"));
  QCOMPARE(status->text(), QStringLiteral("Status: Enabled"));
  QCOMPARE(change->text(), QStringLiteral("Disable enhanced mode"));
  QVERIFY(remove->isEnabled());

  const int queriesBeforeAgentStateChanges = serviceControl.queries;
  manager.setAgentState(service::EnhancedModeAgentState::Connecting);
  auto* issue = dialog.findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(issue);
  QCOMPARE(issue->toPlainText(), QStringLiteral("Waiting for the UI agent to complete authentication."));

  manager.setAgentState(service::EnhancedModeAgentState::Disconnected);
  QCOMPARE(status->text(), QStringLiteral("Status: Enabled"));
  QCOMPARE(change->text(), QStringLiteral("Disable enhanced mode"));
  QCOMPARE(issue->toPlainText(), QStringLiteral("The service is running, but no authenticated UI agent is connected."));

  manager.setAgentState(service::EnhancedModeAgentState::Connected);
  QVERIFY(!issue->isVisible());
  QCOMPARE(serviceControl.queries, queriesBeforeAgentStateChanges);

  QTest::mouseClick(change, Qt::LeftButton);
  QCOMPARE(serviceControl.removals, 1);
  QCOMPARE(status->text(), QStringLiteral("Status: Disabled"));
  QCOMPARE(serviceControl.activeDeletionBarriers, 0);
  QVERIFY(!serviceControl.queriedDuringDeletion);
  QVERIFY(!remove->isEnabled());

  serviceControl.current = {};
  serviceControl.current.state = service::EnhancedModeState::Stopped;
  serviceControl.current.serviceExists = true;
  serviceControl.current.ownProcess = true;
  serviceControl.current.automaticStart = true;
  serviceControl.current.localSystemAccount = true;
  serviceControl.current.executableMatches = true;
  serviceControl.current.preshutdownTimeoutConfigured = true;
  serviceControl.current.requiredPrivilegesConfigured = true;
  serviceControl.current.serviceRegistryPresent = true;
  ui::EnhancedModeDialog stopped(manager);
  auto* start = stopped.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* stoppedStatus = stopped.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* stoppedIssue = stopped.findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(start && stoppedStatus && stoppedIssue);
  QCOMPARE(stoppedStatus->text(), QStringLiteral("Status: Service stopped"));
  QCOMPARE(start->text(), QStringLiteral("Start service"));
  QVERIFY(!stoppedIssue->isVisible());
  QTest::mouseClick(start, Qt::LeftButton);
  QCOMPARE(serviceControl.starts, 1);
  QCOMPARE(serviceControl.installs, 1);
  QCOMPARE(stoppedStatus->text(), QStringLiteral("Status: Enabled"));

  serviceControl.current.running = true;
  serviceControl.current.state = service::EnhancedModeState::RepairRequired;
  serviceControl.current.executableMatches = false;
  ui::EnhancedModeDialog repairRequired(manager);
  auto* repair = repairRequired.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* repairStatus = repairRequired.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* repairIssue = repairRequired.findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(repair && repairStatus && repairIssue);
  QCOMPARE(repairStatus->text(), QStringLiteral("Status: Repair required"));
  QCOMPARE(repair->text(), QStringLiteral("Repair enhanced mode"));
  QVERIFY(repairIssue->toPlainText().contains(QStringLiteral("executable path or startup arguments")));

  serviceControl.current = {};
  serviceControl.current.state = service::EnhancedModeState::RepairRequired;
  serviceControl.current.serviceRegistryPresent = true;
  ui::EnhancedModeDialog residual(manager);
  auto* residualRemove = residual.findChild<QPushButton*>(QStringLiteral("enhancedModeRemoveButton"));
  auto* residualIssue = residual.findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(residualRemove && residualIssue);
  QVERIFY(residualRemove->isEnabled());
  QCOMPARE(residualIssue->toPlainText(), QStringLiteral("The service is absent, but its service registry data remains."));

  serviceControl.queryFailure = QStringLiteral("SCM unavailable");
  ui::EnhancedModeDialog unavailable(manager);
  auto* unavailableStatus = unavailable.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* unavailableChange = unavailable.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* unavailableRemove = unavailable.findChild<QPushButton*>(QStringLiteral("enhancedModeRemoveButton"));
  auto* unavailableIssue = unavailable.findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(unavailableStatus && unavailableChange && unavailableRemove && unavailableIssue);
  QCOMPARE(unavailableStatus->text(), QStringLiteral("Enhanced mode status could not be read"));
  QCOMPARE(unavailableIssue->toPlainText(), serviceControl.queryFailure);
  QVERIFY(!unavailableChange->isEnabled());
  QVERIFY(!unavailableRemove->isEnabled());
}

void ApplicationUiBehaviorTests::enhancedModeServiceContractRejectsEveryBrokenInvariant() {
  const auto complete = completeEnhancedModeStatus();
  QVERIFY(complete.serviceContractSatisfied());

  constexpr std::array requirements{
      &service::EnhancedModeStatus::serviceExists,
      &service::EnhancedModeStatus::ownProcess,
      &service::EnhancedModeStatus::automaticStart,
      &service::EnhancedModeStatus::localSystemAccount,
      &service::EnhancedModeStatus::running,
      &service::EnhancedModeStatus::executableMatches,
      &service::EnhancedModeStatus::preshutdownTimeoutConfigured,
      &service::EnhancedModeStatus::requiredPrivilegesConfigured,
      &service::EnhancedModeStatus::preshutdownAccepted,
  };
  for (const auto requirement : requirements) {
    auto broken = complete;
    broken.*requirement = false;
    QVERIFY(!broken.serviceContractSatisfied());
  }
}

void ApplicationUiBehaviorTests::enhancedModeRemovalPreflightsProtectedRegistryPersistence() {
  RecordingWmiOperations unavailableWmi;
  MemoryEnhancedModeServiceControl protectedService;
  protectedService.current = completeEnhancedModeStatus();
  service::EnhancedModeManager unavailableManager(protectedService, unavailableWmi, UwfCapability::Available);

  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(unavailableManager.disable()));
  QCOMPARE(protectedService.removals, 0);
  QCOMPARE(protectedService.current.state, service::EnhancedModeState::Enabled);

  RecordingWmiOperations availableWmi;
  availableWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  availableWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("registry-path")},
                                        {"CurrentSession", WmiValue::fromBool(true)},
                                        {"PersistDomainSecretKey", WmiValue::fromBool(false)},
                                        {"PersistTSCAL", WmiValue::fromBool(false)}}});
  MemoryEnhancedModeServiceControl removableService;
  removableService.current = protectedService.current;
  // 使用必然存在、但只经内存 WMI 传输处理的 hive 根验证时序；测试不会修改
  // 真实注册表。计划在 SCM 变更前冻结，方法调用发生时删除屏障必须仍存活。
  removableService.deletionPlan = {{"HKEY_CURRENT_USER", {}}};
  bool deletionCommittedInsideBarrier = false;
  availableWmi.invocationObserver = [&] { deletionCommittedInsideBarrier = removableService.activeDeletionBarriers == 1; };
  service::EnhancedModeManager availableManager(removableService, availableWmi, UwfCapability::Available);
  const auto result = availableManager.disable();
  QCOMPARE(removableService.removals, 1);
  QCOMPARE(removableService.activeDeletionBarriers, 0);
  QVERIFY(deletionCommittedInsideBarrier);
  QCOMPARE(availableWmi.invocations, std::vector<QString>{QStringLiteral("CommitRegistryDeletion")});
  QCOMPARE(result.status.state, service::EnhancedModeState::Disabled);
  QVERIFY(result.persistenceWarning.isEmpty());
}

void ApplicationUiBehaviorTests::mainWindowEnhancedModeActionTracksTheServiceLifecycle() {
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  MemoryEnhancedModeServiceControl serviceControl;
  MemoryEnhancedModeAgent agent;
  MutableApplicationStateSource source;
  source.disks = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;

  ui::MainWindow window({wmi, source, staging, &serviceControl, &agent},
                        {.uwfCapability = UwfCapability::Unavailable, .compatibilityMode = false, .osProductName = {}, .osEditionId = {}});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);
  auto* enhancedAction = actionWithText(&window, QStringLiteral("Enhanced mode"));
  QVERIFY(enhancedAction);
  QVERIFY(!enhancedAction->icon().isNull());
  QCOMPARE(enhancedAction->toolTip(), QStringLiteral("Coordinate automatic file staging with Windows shutdown and restart."));
  const qint64 disabledIcon = enhancedAction->icon().cacheKey();

  bool enableDialogObserved = false;
  QTimer::singleShot(0, &window, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog || dialog->objectName() != QStringLiteral("enhancedModeDialog")) return;
    auto* status = dialog->findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
    auto* issue = dialog->findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeIssue"));
    auto* change = dialog->findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
    if (!status || !issue || !change || status->text() != QStringLiteral("Status: Disabled")) return;
    enableDialogObserved = true;
    change->click();
    QCOMPARE(issue->toPlainText(), QStringLiteral("Waiting for the UI agent to complete authentication."));
    dialog->reject();
  });
  enhancedAction->trigger();
  QVERIFY(enableDialogObserved);
  QCOMPARE(serviceControl.installs, 1);
  QCOMPARE(serviceControl.current.state, service::EnhancedModeState::Enabled);
  QCOMPARE(agent.starts, 1);
  QVERIFY(enhancedAction->icon().cacheKey() != disabledIcon);

  bool disableDialogObserved = false;
  QTimer::singleShot(0, &window, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog || dialog->objectName() != QStringLiteral("enhancedModeDialog")) return;
    auto* status = dialog->findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
    auto* change = dialog->findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
    if (!status || !change || status->text() != QStringLiteral("Status: Enabled")) return;
    disableDialogObserved = true;
    change->click();
    dialog->reject();
  });
  enhancedAction->trigger();
  QVERIFY(disableDialogObserved);
  QCOMPARE(serviceControl.removals, 1);
  QCOMPARE(serviceControl.current.state, service::EnhancedModeState::Disabled);
  QCOMPARE(agent.stops, 1);
}

void ApplicationUiBehaviorTests::mainWindowRetriesTransientEnhancedModeStatusFailures() {
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  MemoryEnhancedModeServiceControl serviceControl;
  MemoryEnhancedModeAgent agent;
  serviceControl.queryFailure = QStringLiteral("SCM temporarily unavailable");
  MutableApplicationStateSource source;
  source.disks = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;

  ui::MainWindow window({wmi, source, staging, &serviceControl, &agent},
                        {.uwfCapability = UwfCapability::Unavailable, .compatibilityMode = false, .osProductName = {}, .osEditionId = {}});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);
  auto* enhancedAction = actionWithText(&window, QStringLiteral("Enhanced mode"));
  QVERIFY(enhancedAction);
  const qint64 failureIcon = enhancedAction->icon().cacheKey();
  QCOMPARE(serviceControl.queries, 1);

  serviceControl.queryFailure.clear();
  QTRY_VERIFY_WITH_TIMEOUT(serviceControl.queries >= 2, 2500);
  QVERIFY(enhancedAction->icon().cacheKey() != failureIcon);
}

void ApplicationUiBehaviorTests::mainWindowTreatsAuthenticatedServiceConnectionAsStatusOnly() {
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\pending.bin")}};
  MemoryEnhancedModeServiceControl serviceControl;
  serviceControl.current = completeEnhancedModeStatus();
  MemoryEnhancedModeAgent agent;
  MutableApplicationStateSource source;
  source.disks = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;

  ui::MainWindow window({wmi, source, staging, &serviceControl, &agent},
                        {.uwfCapability = UwfCapability::Unavailable, .compatibilityMode = false, .osProductName = {}, .osEditionId = {}});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);
  QCOMPARE(agent.starts, 1);

  const int queriesBeforeAuthentication = serviceControl.queries;
  agent.publishConnectionState(true);
  QCoreApplication::processEvents();
  QCOMPARE(serviceControl.queries, queriesBeforeAuthentication);
  QCOMPARE(staging.loads, 0);
  QVERIFY(wmi.queryResults.empty());
  QVERIFY(wmi.invocations.empty());
}

void ApplicationUiBehaviorTests::emptyAndMissingRegistryPlansSkipUwfInvocation() {
  RecordingWmiOperations wmi;
  RegistryTreeCommitter committer(wmi);
  const auto emptyCommit = committer.commit({});
  QCOMPARE(emptyCommit.attempted, std::size_t{0});
  QCOMPARE(emptyCommit.committed, std::size_t{0});
  QVERIFY(emptyCommit.failures.empty());
  QVERIFY(wmi.queryResults.empty());

  const std::string missingKey =
      QStringLiteral("HKEY_CURRENT_USER\\Software\\HsingYun\\UWF Manager\\Tests\\%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)).toStdString();

  const auto result = committer.commitDeletion({{missingKey, {}}});

  QCOMPARE(result.attempted, std::size_t{1});
  QCOMPARE(result.skipped, std::size_t{1});
  QCOMPARE(result.committed, std::size_t{0});
  QVERIFY(result.failures.empty());
  QVERIFY(wmi.queryResults.empty());
  QVERIFY(wmi.invocations.empty());
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
  for (const auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : edits) {
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
      for (const auto labels = warning->findChildren<ui::ContentHintBox*>(); auto* label : labels) {
        failureReported = failureReported || label->toPlainText().contains(QStringLiteral("Could not open file for writing"));
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
  QSignalSpy safeRestartRequested(&dialog, &ui::ApplyPlanDialog::safeRestartRequested);
  QSignalSpy directRestartRequested(&dialog, &ui::ApplyPlanDialog::directRestartRequested);
  auto* apply = buttonWithText(&dialog, QStringLiteral("Apply"));
  auto* restart = dialog.findChild<QPushButton*>(QStringLiteral("restartBtn"));
  auto* directRestart = dialog.findChild<QPushButton*>(QStringLiteral("directRestartBtn"));
  auto* directRestartHint = dialog.findChild<QLabel*>(QStringLiteral("directRestartHint"));
  QVERIFY(apply && apply->isEnabled());
  QVERIFY(restart && restart->isHidden());
  QVERIFY(directRestart && directRestart->isHidden());
  QVERIFY(directRestartHint && directRestartHint->isHidden());

  QTimer::singleShot(0, this, [] {
    if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->accept();
  });
  QTest::mouseClick(apply, Qt::LeftButton);

  QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("Disable")});
  QCOMPARE(reconciliation.count(), 1);
  QVERIFY(!apply->isEnabled());
  QVERIFY(!restart->isHidden());
  QVERIFY(!directRestart->isHidden());
  QVERIFY(!directRestartHint->isHidden());
  QVERIFY(directRestartHint->text().contains(QStringLiteral("skips File staging commits")));
  dialog.resize(820, 560);
  QTRY_COMPARE(dialog.size(), QSize(820, 560));
  QTRY_VERIFY2_WITH_TIMEOUT(test::clippedLabel(dialog).isEmpty(), qPrintable(test::clippedLabel(dialog)), 2000);
  for (auto* button : {apply, restart, directRestart}) QVERIFY(dialog.rect().contains(QRect(button->mapTo(&dialog, QPoint()), button->size())));
  QTest::mouseClick(restart, Qt::LeftButton);
  QTest::mouseClick(directRestart, Qt::LeftButton);
  QCOMPARE(safeRestartRequested.count(), 1);
  QCOMPARE(directRestartRequested.count(), 1);
  bool successReported = false;
  for (const auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : edits) {
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
  for (const auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : edits) {
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
  MemoryFileStagingStore staging;
  ui::FileStagingCoordinator coordinator(wmi, staging, UwfCapability::Available, nullptr);
  ui::PowerController controller({wmi, staging, UwfCapability::Available, coordinator}, nullptr);

  QTimer::singleShot(0, this, [] {
    if (auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget())) dialog->reject();
  });
  controller.safeShutdown();
  QVERIFY(wmi.invocations.empty());
  QVERIFY(wmi.queryResults.empty());

  RecordingWmiOperations unreadableWmi;
  MemoryFileStagingStore unreadableStore;
  unreadableStore.readFailure = QStringLiteral("registry staging data is corrupt");
  ui::FileStagingCoordinator unreadableCoordinator(unreadableWmi, unreadableStore, UwfCapability::Available, nullptr);
  ui::PowerController unreadableController({unreadableWmi, unreadableStore, UwfCapability::Available, unreadableCoordinator}, nullptr);
  bool readFailureStayedInPowerDialog = false;
  bool readFailureDetailsVisible = false;
  QTimer unreadableDriver;
  unreadableDriver.setInterval(1);
  connect(&unreadableDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* details = dialog->findChild<QPlainTextEdit*>(QStringLiteral("powerStagingFailureDetails"));
    if (!details || !details->isVisible()) return;
    readFailureStayedInPowerDialog = dialog->objectName() == QStringLiteral("powerConfirmDialog");
    readFailureDetailsVisible = details->toPlainText().contains(unreadableStore.readFailure);
    dialog->reject();
    unreadableDriver.stop();
  });
  unreadableDriver.start();
  unreadableController.safeShutdown();
  QVERIFY(readFailureStayedInPowerDialog);
  QVERIFY(readFailureDetailsVisible);
  QVERIFY(unreadableWmi.queryResults.empty());
  QVERIFY(unreadableWmi.invocations.empty());

  RecordingWmiOperations approvedUnreadableWmi;
  approvedUnreadableWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  MemoryFileStagingStore approvedUnreadableStore;
  approvedUnreadableStore.readFailure = unreadableStore.readFailure;
  ui::FileStagingCoordinator approvedUnreadableCoordinator(approvedUnreadableWmi, approvedUnreadableStore, UwfCapability::Available, nullptr);
  ui::PowerController approvedUnreadableController({approvedUnreadableWmi, approvedUnreadableStore, UwfCapability::Available, approvedUnreadableCoordinator},
                                                   nullptr);
  bool unreadableContinuationConfirmed = false;
  QTimer approvedUnreadableDriver;
  approvedUnreadableDriver.setInterval(1);
  connect(&approvedUnreadableDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* details = dialog->findChild<QPlainTextEdit*>(QStringLiteral("powerStagingFailureDetails"));
    auto* shutdown = dialog->findChild<QPushButton*>(QStringLiteral("dangerBtn"));
    if (!details || !details->isVisible() || !shutdown || !shutdown->isEnabled()) return;
    unreadableContinuationConfirmed = shutdown->text() == QStringLiteral("Continue shutdown");
    QTest::mouseClick(shutdown, Qt::LeftButton);
    approvedUnreadableDriver.stop();
  });
  approvedUnreadableDriver.start();
  approvedUnreadableController.safeShutdown();
  QVERIFY(unreadableContinuationConfirmed);
  QCOMPARE(approvedUnreadableWmi.invocations, std::vector<QString>{QStringLiteral("ShutdownSystem")});

  RecordingWmiOperations rejectedPowerWmi;
  rejectedPowerWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  rejectedPowerWmi.invocationFailure = std::make_exception_ptr(WmiProviderError(5, "request shutdown", "provider rejected shutdown"));
  ui::FileStagingCoordinator rejectedPowerCoordinator(rejectedPowerWmi, staging, UwfCapability::Available, nullptr);
  std::vector<bool> rejectedPowerPreshutdownStates;
  bool filterReadBeforePreshutdownToken = false;
  using PreshutdownControlResult = ui::PowerControllerServices::PreshutdownControlResult;
  ui::PowerController rejectedPowerController({rejectedPowerWmi,
                                               staging,
                                               UwfCapability::Available,
                                               rejectedPowerCoordinator,
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
  rejectedPowerController.safeShutdown();
  QVERIFY(rejectedPowerConfirmed);
  QVERIFY(rejectedPowerWarningClosed);
  QVERIFY(filterReadBeforePreshutdownToken);
  QCOMPARE(rejectedPowerPreshutdownStates, (std::vector<bool>{true, false}));

  wmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
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
  controller.safeRestart();
  QVERIFY(emptyPreparationBlockedAction);
  QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("RestartSystem")});

  RecordingWmiOperations directFromSafeWmi;
  directFromSafeWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  MemoryFileStagingStore directFromSafeStore;
  directFromSafeStore.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\never-commit.bin")}};
  ui::FileStagingCoordinator directFromSafeCoordinator(directFromSafeWmi, directFromSafeStore, UwfCapability::Available, nullptr);
  int directFromSafePreshutdownMarks = 0;
  ui::PowerController directFromSafeController({directFromSafeWmi,
                                                directFromSafeStore,
                                                UwfCapability::Available,
                                                directFromSafeCoordinator,
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
    directFromSafeCoordinator.requestCommit([&](const ui::FileStagingBatchResult& result) { skippedConcurrentCommit = result.command; });
    QTest::mouseClick(directRestart, Qt::LeftButton);
    directFromSafeDriver.stop();
  });
  directFromSafeDriver.start();
  directFromSafeController.safeRestart();
  QVERIFY(directOptionAvailableDuringPreparation);
  QVERIFY(skippedConcurrentCommit.has_value());
  QCOMPARE(skippedConcurrentCommit->outcome, app::ApplicationCommandOutcome::Rejected);
  QCOMPARE(directFromSafeStore.loads, 0);
  QCOMPARE(directFromSafeWmi.invocations, std::vector<QString>{QStringLiteral("RestartSystem")});
  QCOMPARE(directFromSafePreshutdownMarks, 1);

  RecordingWmiOperations directShutdownWmi;
  directShutdownWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  MemoryFileStagingStore directShutdownStore;
  directShutdownStore.entries = directFromSafeStore.entries;
  ui::FileStagingCoordinator directShutdownCoordinator(directShutdownWmi, directShutdownStore, UwfCapability::Available, nullptr);
  ui::PowerController directShutdownController({directShutdownWmi, directShutdownStore, UwfCapability::Available, directShutdownCoordinator}, nullptr);
  bool directShutdownConfirmed = false;
  QTimer directShutdownDriver;
  directShutdownDriver.setInterval(1);
  connect(&directShutdownDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* directShutdown = dialog->findChild<QPushButton*>(QStringLiteral("directShutdownBtn"));
    if (!directShutdown || !directShutdown->isEnabled()) return;
    directShutdownConfirmed = directShutdown->text() == QStringLiteral("Direct shutdown");
    QTest::mouseClick(directShutdown, Qt::LeftButton);
    directShutdownDriver.stop();
  });
  directShutdownDriver.start();
  directShutdownController.safeShutdown();
  QVERIFY(directShutdownConfirmed);
  QCOMPARE(directShutdownStore.loads, 0);
  QCOMPARE(directShutdownWmi.invocations, std::vector<QString>{QStringLiteral("ShutdownSystem")});

  RecordingWmiOperations directWmi;
  directWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  MemoryFileStagingStore directStore;
  directStore.entries = directFromSafeStore.entries;
  ui::FileStagingCoordinator directCoordinator(directWmi, directStore, UwfCapability::Available, nullptr);
  ui::PowerController directController({directWmi, directStore, UwfCapability::Available, directCoordinator}, nullptr);
  bool directDialogConfirmed = false;
  QTimer directDriver;
  directDriver.setInterval(1);
  connect(&directDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* directRestart = dialog->findChild<QPushButton*>(QStringLiteral("directRestartBtn"));
    if (!directRestart || !directRestart->isEnabled()) return;
    directDialogConfirmed =
        dialog->windowTitle() == QStringLiteral("Direct restart") && dialog->findChild<QPushButton*>(QStringLiteral("restartBtn")) == nullptr;
    QTest::mouseClick(directRestart, Qt::LeftButton);
    directDriver.stop();
  });
  directDriver.start();
  directController.directRestart();
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
  const QString drive = ui::extractDriveLetter(stagedPath);
  QVERIFY(!drive.isEmpty());

  RecordingWmiOperations stagedWmi;
  stagedWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  stagedWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                     {"CurrentSession", WmiValue::fromBool(true)},
                                     {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                                     {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                     {"BindByDriveLetter", WmiValue::fromBool(true)},
                                     {"CommitPending", WmiValue::fromBool(false)},
                                     {"Protected", WmiValue::fromBool(true)}}});
  stagedWmi.readResults.push_back(fileExclusionResult());
  stagedWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  MemoryFileStagingStore stagedStore;
  stagedStore.entries = {{app::FileStagingKind::File, stagedPath}, {app::FileStagingKind::Directory, QDir::toNativeSeparators(directory.path())}};
  ui::FileStagingCoordinator stagedCoordinator(stagedWmi, stagedStore, UwfCapability::Available, nullptr);
  std::vector<bool> stagedPreshutdownStates;
  ui::PowerController stagedController({stagedWmi,
                                        stagedStore,
                                        UwfCapability::Available,
                                        stagedCoordinator,
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
      calculationStateVisible = stagingDetail->isVisible() && stagingDetail->text() == QStringLiteral("Calculating files for automatic commit…");
      return;
    }
    if (stagingDetail->isVisible()) {
      expandedFileCountVisible = stagingDetail->text().contains(QStringLiteral("3 file(s)"));
    }
    if (!concurrentCommandQueued) {
      concurrentCommandQueued = true;
      stagedCoordinator.requestCommit([&](const ui::FileStagingBatchResult& result) { concurrentCommandResult = result.command; });
      QCOMPARE(QApplication::activeModalWidget()->objectName(), QStringLiteral("powerConfirmDialog"));
    }
    QTest::mouseClick(restart, Qt::LeftButton);
    stagedDriver.stop();
  });
  stagedDriver.start();
  stagedController.safeRestart();
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
  failedWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  failedWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                     {"CurrentSession", WmiValue::fromBool(true)},
                                     {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                                     {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                     {"BindByDriveLetter", WmiValue::fromBool(true)},
                                     {"CommitPending", WmiValue::fromBool(false)},
                                     {"Protected", WmiValue::fromBool(true)}}});
  failedWmi.readResults.push_back(fileExclusionResult());
  failedWmi.invocationFailure = std::make_exception_ptr(WmiProviderError(5, "commit staged file", "provider failure"));
  ui::FileStagingCoordinator failedCoordinator(failedWmi, stagedStore, UwfCapability::Available, nullptr);
  ui::PowerController failedController({failedWmi, stagedStore, UwfCapability::Available, failedCoordinator}, nullptr);
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
    if (auto* details = dialog->findChild<QPlainTextEdit*>(QStringLiteral("powerStagingFailureDetails")); details && details->isVisible()) {
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
  failedController.safeShutdown();
  QCOMPARE(failedWmi.invocations, std::vector<QString>({QStringLiteral("CommitFile"), QStringLiteral("CommitFile"), QStringLiteral("CommitFile")}));
  QVERIFY(failureDetailsVisible);
  QVERIFY(failureStayedInDialog);

  RecordingWmiOperations continuedWmi;
  continuedWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  continuedWmi.queryResults.push_back({{{"__PATH", WmiValue::fromString("volume-path")},
                                        {"CurrentSession", WmiValue::fromBool(true)},
                                        {"DriveLetter", WmiValue::fromString(drive.toStdString())},
                                        {"VolumeName", WmiValue::fromString("Volume{staging}")},
                                        {"BindByDriveLetter", WmiValue::fromBool(true)},
                                        {"CommitPending", WmiValue::fromBool(false)},
                                        {"Protected", WmiValue::fromBool(true)}}});
  continuedWmi.readResults.push_back(fileExclusionResult());
  continuedWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
  continuedWmi.invocationFailure = std::make_exception_ptr(WmiProviderError(5, "commit staged file", "provider failure"));
  ui::FileStagingCoordinator continuedCoordinator(continuedWmi, stagedStore, UwfCapability::Available, nullptr);
  ui::PowerController continuedController({continuedWmi, stagedStore, UwfCapability::Available, continuedCoordinator}, nullptr);
  bool continueActionClicked = false;
  bool continueDecisionClicked = false;
  bool commandQueuedDuringPowerApproval = false;
  std::optional<app::ApplicationCommandResult> commandResultAfterPowerApproval;
  QTimer continueDriver;
  continueDriver.setInterval(1);
  connect(&continueDriver, &QTimer::timeout, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    auto* details = dialog->findChild<QPlainTextEdit*>(QStringLiteral("powerStagingFailureDetails"));
    auto* shutdown = dialog->findChild<QPushButton*>(QStringLiteral("dangerBtn"));
    if (details && details->isVisible() && shutdown) {
      if (!commandQueuedDuringPowerApproval) {
        commandQueuedDuringPowerApproval = true;
        continuedCoordinator.requestCommit([&](const ui::FileStagingBatchResult& result) { commandResultAfterPowerApproval = result.command; });
      }
      continueDecisionClicked = shutdown->text() == QStringLiteral("Continue shutdown");
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
  continuedController.safeShutdown();
  QVERIFY(continueDecisionClicked);
  QVERIFY(commandResultAfterPowerApproval.has_value());
  QCOMPARE(commandResultAfterPowerApproval->outcome, app::ApplicationCommandOutcome::CompletedWithFailures);
  QCOMPARE(continuedWmi.invocations,
           std::vector<QString>({QStringLiteral("CommitFile"), QStringLiteral("CommitFile"), QStringLiteral("CommitFile"), QStringLiteral("ShutdownSystem")}));

  RecordingWmiOperations disabledWmi;
  disabledWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(false)}, {"NextEnabled", WmiValue::fromBool(false)}}});
  disabledWmi.queryResults.push_back(
      {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(false)}, {"NextEnabled", WmiValue::fromBool(false)}}});
  ui::FileStagingCoordinator disabledCoordinator(disabledWmi, stagedStore, UwfCapability::Available, nullptr);
  ui::PowerController disabledController({disabledWmi, stagedStore, UwfCapability::Available, disabledCoordinator}, nullptr);
  bool stagingSummaryHidden = false;
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
    stagingSummaryHidden = !stagingDetail->isVisible();
    QTest::mouseClick(shutdown, Qt::LeftButton);
    disabledDriver.stop();
  });
  disabledDriver.start();
  disabledController.safeShutdown();
  QVERIFY(disabledActionInitiallyBlocked);
  QVERIFY(stagingSummaryHidden);
  QCOMPARE(disabledWmi.invocations, std::vector<QString>{QStringLiteral("ShutdownSystem")});
}

void ApplicationUiBehaviorTests::commitBatchUsesAuthoritativeExistenceForEveryOutcome() {
  QStringList categories;
  QStringList reasons;
  QTimer watcher;
  watcher.setInterval(5);
  connect(&watcher, &QTimer::timeout, &watcher, [&watcher, &categories, &reasons] {
    for (const auto widgets = QApplication::topLevelWidgets(); auto* widget : widgets) {
      auto* dialog = qobject_cast<QDialog*>(widget);
      auto* table = dialog ? dialog->findChild<QTableWidget*>() : nullptr;
      if (!table || table->columnCount() != 6 || table->rowCount() != 3) continue;
      for (int row = 0; row < table->rowCount(); ++row) {
        categories.append(table->item(row, 0)->text());
        reasons.append(table->item(row, 5)->text());
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

void ApplicationUiBehaviorTests::mainWindowMountsFileStagingOnlyWherePerFileCommitIsSupported() {
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  MutableApplicationStateSource source;
  const std::string systemDrive = drive::systemLetter();
  QVERIFY(!systemDrive.empty());
  const std::string dataDrive = systemDrive == "C:" ? "D:" : "C:";
  staging.entries = {{app::FileStagingKind::File, QString::fromStdString(dataDrive) + QStringLiteral("\\Data\\state.bin")},
                     {app::FileStagingKind::Directory, QString::fromStdString(systemDrive) + QStringLiteral("\\ProgramData\\Vendor")}};
  source.disks = {{dataDrive, "Volume{data}", "NTFS", "Removable data", 1000, 500, core::DiskSupport::NotFixedLocalDisk},
                  {systemDrive, "Volume{system}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported}};
  source.snapshot = editableSnapshot();
  const QString stagingRegistryRoot = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager");
  source.snapshot.next.registryExclusions.push_back(stagingRegistryRoot.toStdString());

  ui::MainWindow window({wmi, source, staging},
                        {.uwfCapability = UwfCapability::Available, .compatibilityMode = false, .osProductName = {}, .osEditionId = {}});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"));
  QVERIFY(tabs);
  QCOMPARE(tabs->count(), 2);
  QCOMPARE(window.findChildren<ui::FileStagingWidget*>().size(), 1);

  auto* dataTab = qobject_cast<ui::DiskTab*>(tabs->widget(0));
  auto* systemTab = qobject_cast<ui::DiskTab*>(tabs->widget(1));
  QVERIFY(dataTab && systemTab);
  auto* dataInnerTabs = dataTab->findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  auto* systemInnerTabs = systemTab->findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(dataInnerTabs && systemInnerTabs);
  QCOMPARE(dataInnerTabs->count(), 1);
  QCOMPARE(systemInnerTabs->count(), 3);
  QCOMPARE(systemInnerTabs->tabText(1), QStringLiteral("Registry exclusions"));
  QCOMPARE(systemInnerTabs->tabText(2), QStringLiteral("File staging"));

  auto* registryExclusions = qobject_cast<ui::ExclusionListWidget*>(systemInnerTabs->widget(1));
  QVERIFY(registryExclusions);
  auto* registryList = registryExclusions->findChild<QListWidget*>(QStringLiteral("exclusionList"));
  auto* registryRemove = buttonWithText(registryExclusions, QStringLiteral("Remove selected"));
  QVERIFY(registryList && registryRemove);
  bool selectedStagingRoot = false;
  for (int row = 0; row < registryList->count(); ++row) {
    auto* item = registryList->item(row);
    if (item->data(Qt::UserRole).toString().compare(stagingRegistryRoot, Qt::CaseInsensitive) != 0) continue;
    item->setSelected(true);
    selectedStagingRoot = true;
  }
  QVERIFY(selectedStagingRoot);
  QVERIFY(!registryRemove->isEnabled());

  auto* systemStaging = qobject_cast<ui::FileStagingWidget*>(systemInnerTabs->widget(2));
  QVERIFY(systemStaging);
  QCOMPARE(systemStaging->driveLetter(), QString::fromStdString(systemDrive));
  auto* systemStagingList = systemStaging->findChild<QListWidget*>(QStringLiteral("exclusionList"));
  QVERIFY(systemStagingList);
  QCOMPARE(systemStagingList->count(), 1);

  dataTab->setActiveInfoPage(ui::DiskTab::InfoPage::FileStaging);
  QCOMPARE(dataTab->activeInfoPage(), ui::DiskTab::InfoPage::FileExclusions);
  systemTab->setActiveInfoPage(ui::DiskTab::InfoPage::RegistryExclusions);
  std::ranges::reverse(source.disks);
  window.refresh();
  QCOMPARE(source.reads, 2);
  for (int index = 0; index < tabs->count(); ++index) {
    auto* rebuilt = qobject_cast<ui::DiskTab*>(tabs->widget(index));
    QVERIFY(rebuilt);
    const auto expectedPage =
        rebuilt->driveLetter() == QString::fromStdString(dataDrive) ? ui::DiskTab::InfoPage::FileExclusions : ui::DiskTab::InfoPage::RegistryExclusions;
    QCOMPARE(rebuilt->activeInfoPage(), expectedPage);
  }
}

void ApplicationUiBehaviorTests::mainWindowQuietStartupInitializesWhileRemainingHidden() {
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  MutableApplicationStateSource source;
  source.disks = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;
  source.snapshot.unavailableReason = "embedded provider absent";

  ui::MainWindow window({wmi, source, staging},
                        {.uwfCapability = UwfCapability::Unavailable, .compatibilityMode = false, .osProductName = {}, .osEditionId = {}});
  window.startInTray();

  QCOMPARE(source.reads, 1);
  QVERIFY(!window.isVisible());
  QVERIFY(!window.isMinimized());
  QVERIFY(window.findChild<QTabWidget*>(QStringLiteral("mainTabs")));
  QCOMPARE(window.windowOpacity(), 0.0);

  // 只有从托盘、HUB 或第二个进程激活时才映射主窗口；可见 UI 仍在 shown
  // 状态下完成一次稳定重建，不复用隐藏状态下尚未 polish 的布局。
  window.raiseToFront();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 2, 1000);
  QTRY_VERIFY_WITH_TIMEOUT(window.isVisible(), 1000);
  QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 1000);
}

void ApplicationUiBehaviorTests::mainWindowDistinguishesInitialFailureFromCommittedUnavailableState() {
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;

  MutableApplicationStateSource failingSource;
  failingSource.failure = QStringLiteral("initial provider failure");
  MemoryEnhancedModeServiceControl serviceControl;
  {
    MemoryEnhancedModeAgent agent;
    ui::MainWindow window({wmi, failingSource, staging, &serviceControl, &agent},
                          {.uwfCapability = UwfCapability::Available, .compatibilityMode = false, .osProductName = {}, .osEditionId = {}});
    window.show();
    QTRY_COMPARE_WITH_TIMEOUT(failingSource.reads, 1, 1000);
    auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"));
    QVERIFY(tabs);
    QCOMPARE(tabs->count(), 0);
    auto* refresh = actionWithText(&window, QStringLiteral("Refresh"));
    auto* import = actionWithText(&window, QStringLiteral("Import"));
    auto* enhanced = actionWithText(&window, QStringLiteral("Enhanced mode"));
    QVERIFY(refresh && refresh->isEnabled());
    QVERIFY(import && !import->isEnabled());
    QVERIFY(enhanced && enhanced->isEnabled());
    bool reasonVisible = false;
    for (const auto labels = window.findChildren<QLabel*>(); auto* label : labels)
      reasonVisible = reasonVisible || label->text().contains(failingSource.failure);
    QVERIFY(reasonVisible);
    window.close();
  }

  MutableApplicationStateSource source;
  source.disks = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::DiskSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;
  source.snapshot.unavailableReason = "embedded provider absent";
  {
    ui::MainWindow window({wmi, source, staging}, {.uwfCapability = UwfCapability::Unavailable,
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
    for (const auto labels = window.findChildren<QLabel*>(); auto* label : labels) {
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
    for (const auto labels = window.findChildren<QLabel*>(); auto* label : labels)
      laterFailureLeakedIntoUi = laterFailureLeakedIntoUi || label->text().contains(source.failure);
    QVERIFY(!laterFailureLeakedIntoUi);
    window.close();
  }
}

void ApplicationUiBehaviorTests::mainWindowCompactLayoutKeepsControlsReachable_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<QSize>("windowSize");
  QTest::addColumn<bool>("warnings");
  for (const auto& locale : ui::I18n::languages()) {
    for (const bool dark : {false, true}) {
      for (const auto size : {QSize(1280, 720), QSize(1120, 680), QSize(1440, 900)}) {
        for (const bool warnings : {false, true}) {
          const QByteArray name = QStringLiteral("%1-%2-%3x%4-%5")
                                      .arg(QString(locale.code), dark ? "dark" : "light")
                                      .arg(size.width())
                                      .arg(size.height())
                                      .arg(warnings ? "warnings" : "normal")
                                      .toLatin1();
          QTest::newRow(name.constData()) << locale.id << dark << size << warnings;
        }
      }
    }
  }
}

void ApplicationUiBehaviorTests::mainWindowCompactLayoutKeepsControlsReachable() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  QFETCH(QSize, windowSize);
  QFETCH(bool, warnings);
  auto& language = ui::I18n::instance();
  auto& theme = ui::ThemeManager::instance();
  const auto oldLanguage = language.lang();
  const auto oldTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    language.setLang(oldLanguage);
    theme.apply(oldTheme);
  });
  language.setLang(locale);
  QCOMPARE(language.lang(), locale);
  theme.apply(dark ? ui::Theme::Dark : ui::Theme::Light);
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  MemoryEnhancedModeServiceControl serviceControl;
  MemoryEnhancedModeAgent agent;
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  // 模拟关闭的筛选器，避免测试把真实窗口嵌入 Explorer 任务栏。
  // snapshot.elevated 只控制 mock 页面的可编辑性，不改变进程令牌。
  source.snapshot.current.filter.enabled = false;
  source.snapshot.next.filter.enabled = false;
  source.disks = {{"C:", R"(\\?\Volume{12345678-1234-1234-1234-123456789abc}\)", "NTFS", "Workstation System Volume", 512ULL << 30, 256ULL << 30,
                   core::DiskSupport::Supported}};
  ui::MainWindow window({wmi, source, staging, &serviceControl, &agent},
                        {.uwfCapability = UwfCapability::Available,
                         .compatibilityMode = warnings,
                         .osProductName = QStringLiteral("Windows 11 Professional Workstation Compatibility Environment"),
                         .osEditionId = QStringLiteral("Professional")});
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 2000);
  QVERIFY(window.screen()->availableGeometry().contains(window.frameGeometry()));
  window.resize(windowSize);
  QTRY_COMPARE(window.size(), windowSize);

  auto* global = window.findChild<ui::GlobalStatusPanel*>();
  auto* scroll = window.findChild<QScrollArea*>(QStringLiteral("globalSettingsScroll"));
  auto* disk = window.findChild<ui::DiskTab*>();
  auto* status = disk ? disk->findChild<ui::StatusPanel*>() : nullptr;
  auto* inner = disk ? disk->findChild<QTabWidget*>(QStringLiteral("innerTabs")) : nullptr;
  QVERIFY(global && scroll && disk && status && inner);
  if (warnings) {
    global->showVolumeInfoWarning(QStringLiteral("Mock inventory warning: provider read failed.\nWQL: SELECT * FROM UWF_Volume\n") +
                                  QStringLiteral("\\\\COMPUTER\\root\\standardcimv2\\embedded:") + QString(160, u'x') + QStringLiteral("\n旧数据保持不变"));
  }
  QTest::qWait(80);
  QTRY_VERIFY(scroll->viewport()->width() > 0 && scroll->viewport()->height() > 0);
  QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  QTRY_VERIFY(!scroll->horizontalScrollBar()->isVisible());
  auto* initialHint = window.findChild<ui::ContentHintBox*>(QStringLiteral("hoverHintBox"));
  QVERIFY(initialHint && !initialHint->toPlainText().isEmpty());
  QTRY_VERIFY_WITH_TIMEOUT(initialHint->viewport()->height() >= initialHint->document()->size().height(), 2000);
  QTRY_VERIFY_WITH_TIMEOUT(initialHint->document()->size().height() > 0, 2000);
  QTRY_VERIFY2_WITH_TIMEOUT(test::clippedLabel(window).isEmpty(), qPrintable(test::clippedLabel(window)), 2000);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!screenshots.isEmpty()) {
    QVERIFY(QDir().mkpath(screenshots));
    QVERIFY(window.grab().save(QDir(screenshots).filePath(QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral(".png"))));
  }
  auto* typeCombo = global->findChild<QComboBox*>();
  QVERIFY(typeCombo);
  const int originalType = typeCombo->currentIndex();
  typeCombo->setCurrentIndex(typeCombo->findData(static_cast<int>(core::OverlayType::RAM)));
  // 切换后先等待布局请求及原生滚动条更新；旧的几何可能暂时仍满足断言。
  QTest::qWait(80);
  QTRY_VERIFY2_WITH_TIMEOUT(test::clippedLabel(window).isEmpty(), qPrintable(test::clippedLabel(window)), 2000);
  QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  QTRY_VERIFY(scroll->viewport()->width() >= scroll->widget()->minimumSizeHint().width());
  bool inspectedRamCaption = false;
  for (const auto labels = global->findChildren<QLabel*>(); const auto* label : labels) {
    if (!label->text().contains(QStringLiteral("RAM"))) continue;
    inspectedRamCaption = true;
    QVERIFY(!label->wordWrap());
    QVERIFY(label->width() >= label->sizeHint().width());
  }
  QVERIFY(inspectedRamCaption);
  const auto ramScreenshot = window.grab();
  QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  QVERIFY(!scroll->horizontalScrollBar()->isVisible());
  for (const auto controls = global->findChildren<QWidget*>(); const auto* control : controls) {
    if (!control->isVisible() || (!qobject_cast<QLabel const*>(control) && !qobject_cast<QAbstractSpinBox const*>(control) &&
                                  !qobject_cast<QComboBox const*>(control) && !qobject_cast<QAbstractButton const*>(control)))
      continue;
    const QRect bounds(control->mapTo(scroll->viewport(), QPoint()), control->size());
    QVERIFY2(bounds.left() >= 0 && bounds.right() < scroll->viewport()->width(), control->metaObject()->className());
  }
  if (!screenshots.isEmpty())
    QVERIFY(ramScreenshot.save(QDir(screenshots).filePath(QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral("-ram.png"))));
  typeCombo->setCurrentIndex(originalType);
  QTest::qWait(80);
  QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  QTRY_COMPARE(window.size(), windowSize);
  qInfo() << "layout" << QTest::currentDataTag() << "dpr" << window.devicePixelRatioF() << "window" << window.size() << "minimum" << window.minimumSizeHint()
          << "status" << status->size() << "right" << scroll->size() << "content" << scroll->widget()->minimumSizeHint() << "horizontal overflow"
          << scroll->horizontalScrollBar()->maximum();
  QCOMPARE(window.findChildren<QScrollArea*>().size(), 1);

  // 所有状态操作必须完整落在卡片内，不能靠压缩父窗口的 min-size 裁掉按钮。
  for (const auto widgets = status->findChildren<QWidget*>(); auto* widget : widgets) {
    if (!widget->isVisible() || (!qobject_cast<QAbstractButton*>(widget) && !qobject_cast<QComboBox*>(widget))) continue;
    const QRect bounds(widget->mapTo(status, QPoint()), widget->size());
    QVERIFY2(status->rect().contains(bounds), qPrintable(QStringLiteral("clipped status control: %1 %2,%3 %4x%5 in %6x%7")
                                                             .arg(widget->metaObject()->className())
                                                             .arg(bounds.x())
                                                             .arg(bounds.y())
                                                             .arg(bounds.width())
                                                             .arg(bounds.height())
                                                             .arg(status->width())
                                                             .arg(status->height())));
    QVERIFY(!widget->visibleRegion().isEmpty());
    QVERIFY(widget->width() >= widget->minimumSizeHint().width());
  }
  const auto protectionButtons = status->findChildren<QAbstractButton*>();
  auto* protection = status->findChild<ui::SwitchButton*>();
  QVERIFY(protection && protection->isEnabled());
  QTest::mouseClick(protection, Qt::LeftButton);
  QCOMPARE(status->pendingVolumeProtected(), std::optional<bool>(false));
  const int heightAtTarget = status->height();
  window.resize(1800, windowSize.height());
  QTRY_VERIFY(status->height() <= heightAtTarget);
  window.resize(windowSize);
  QTRY_COMPARE(window.size(), windowSize);
  QCOMPARE(status->pendingVolumeProtected(), std::optional<bool>(false));
  QCOMPARE(status->findChildren<QAbstractButton*>(), protectionButtons);

  // 真正切换每个业务页，校验搜索、列表及操作区均在窗口内，重排不丢选择。
  for (int index = 0; index < inner->count(); ++index) {
    QTest::mouseClick(inner->tabBar(), Qt::LeftButton, Qt::NoModifier, inner->tabBar()->tabRect(index).center());
    QCOMPARE(inner->currentIndex(), index);
    if (!screenshots.isEmpty())
      QVERIFY(window.grab().save(QDir(screenshots).filePath(QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral("-tab%1.png").arg(index))));
    auto* page = inner->currentWidget();
    for (const auto widgets = page->findChildren<QWidget*>(); auto* widget : widgets) {
      if (!widget->isVisible() || (!qobject_cast<QLineEdit*>(widget) && !qobject_cast<QPushButton*>(widget) && !qobject_cast<QListWidget*>(widget))) continue;
      QVERIFY2(window.rect().contains(QRect(widget->mapTo(&window, QPoint()), widget->size())), widget->metaObject()->className());
      QVERIFY(!widget->visibleRegion().isEmpty());
    }
  }

  // 逐项滚动到真实设置控件，并用鼠标改变开关，确认并非“滚动条存在”假通过。
  auto* filter = global->findChild<ui::SwitchButton*>();
  QVERIFY(filter && filter->isEnabled());
  scroll->ensureWidgetVisible(filter);
  QTRY_VERIFY(scroll->viewport()->rect().contains(QRect(filter->mapTo(scroll->viewport(), QPoint()), filter->size())));
  QTest::mouseClick(filter, Qt::LeftButton);
  QCOMPARE(global->pendingFilterEnabled(), std::optional<bool>(true));
  for (const auto spins = global->findChildren<QSpinBox*>(); auto* spin : spins) {
    // ensureWidgetVisible 对输入控件只保证输入光标可见，而不是整个边框。
    // 这里验证整项设置可触达，必须按控件的完整矩形滚动。
    const QPoint center = spin->mapTo(scroll->widget(), spin->rect().center());
    scroll->ensureVisible(center.x(), center.y(), spin->width() / 2 + 1, spin->height() / 2 + 1);
    QTRY_VERIFY2(scroll->viewport()->rect().contains(QRect(spin->mapTo(scroll->viewport(), QPoint()), spin->size())),
                 qPrintable(QStringLiteral("spin %1 at %2,%3 size %4x%5; viewport %6x%7 scroll %8/%9")
                                .arg(spin->value())
                                .arg(spin->mapTo(scroll->viewport(), QPoint()).x())
                                .arg(spin->mapTo(scroll->viewport(), QPoint()).y())
                                .arg(spin->width())
                                .arg(spin->height())
                                .arg(scroll->viewport()->width())
                                .arg(scroll->viewport()->height())
                                .arg(scroll->verticalScrollBar()->value())
                                .arg(scroll->verticalScrollBar()->maximum())));
    QVERIFY(spin->width() >= spin->minimumSizeHint().width());
  }
  auto* hint = window.findChild<ui::ContentHintBox*>(QStringLiteral("hoverHintBox"));
  QVERIFY(hint);
  QTest::keyClick(scroll->verticalScrollBar(), Qt::Key_End);
  QTRY_VERIFY(scroll->viewport()->rect().contains(QRect(hint->mapTo(scroll->viewport(), QPoint()), hint->size())));
  scroll->verticalScrollBar()->setValue(0);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QVERIFY(wmi.invocations.empty());
  QCOMPARE(staging.writes, 0);
  QCOMPARE(serviceControl.installs + serviceControl.starts + serviceControl.removals, 0);
  QCOMPARE(agent.starts, 0);
}

void ApplicationUiBehaviorTests::mainWindowCompactToolbarKeepsLanguageMenuUsable() {
  auto& language = ui::I18n::instance();
  const auto oldLanguage = language.lang();
  const auto restore = qScopeGuard([&] { language.setLang(oldLanguage); });
  language.setLang(ui::I18n::Lang::En);
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  MutableApplicationStateSource source;
  source.snapshot.uwfAvailable = false;
  ui::MainWindow window({wmi, source, staging},
                        {.uwfCapability = UwfCapability::Unavailable, .compatibilityMode = false, .osProductName = {}, .osEditionId = {}});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 2000);
  window.resize(900, 680);
  auto* toolbar = window.findChild<QToolBar*>(QStringLiteral("mainToolbar"));
  QVERIFY(toolbar);
  auto* extension = toolbar->findChild<QToolButton*>(QStringLiteral("qt_toolbar_ext_button"));
  QVERIFY(extension);
  QTRY_VERIFY(extension->isVisible());
  auto* action = actionWithText(toolbar, QStringLiteral("Language"));
  QVERIFY(action && action->menu());
  const QPointer<QToolButton> languageButton = qobject_cast<QToolButton*>(toolbar->widgetForAction(action));
  QVERIFY(languageButton);
  bool selected = false;
  bool languageRequested = false;
  QTimer driver;
  driver.setInterval(10);
  connect(&driver, &QTimer::timeout, this, [&] {
    auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    if (!menu) {
      // QMainWindow 中 Qt 优先展开第二行；高度不足时才使用溢出菜单。
      // 两种原生呈现都必须保留同一语言入口。
      if (languageButton && languageButton->isVisible() && !languageRequested) {
        languageRequested = true;
        const QPoint position = languageButton->mapTo(&window, languageButton->rect().center());
        QTimer::singleShot(0, &window, [&window, position] { QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, position); });
      }
      return;
    }
    for (auto* candidate : menu->actions()) {
      if (candidate->text() == QStringLiteral("Language") && candidate->menu()) {
        QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(candidate).center());
        return;
      }
      if (candidate->text() == QStringLiteral("简体中文")) {
        driver.stop();
        selected = true;
        QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(candidate).center());
        return;
      }
    }
  });
  QTimer timeout;
  timeout.setSingleShot(true);
  connect(&timeout, &QTimer::timeout, this, [&] {
    driver.stop();
    for (auto* widget : QApplication::topLevelWidgets())
      if (auto* menu = qobject_cast<QMenu*>(widget)) menu->close();
  });
  driver.start();
  timeout.start(3000);
  QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, extension->mapTo(&window, extension->rect().center()));
  QTRY_VERIFY_WITH_TIMEOUT(selected, 3500);
  timeout.stop();
  QTRY_COMPARE(language.lang(), ui::I18n::Lang::Zh_CN);
  QTRY_COMPARE(window.windowTitle(), QStringLiteral("统一写入筛选器 (UWF) 管理器"));
  QVERIFY(wmi.invocations.empty());
  QCOMPARE(staging.writes, 0);
}

void ApplicationUiBehaviorTests::dialogLayoutsKeepActionsVisible_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  for (const auto& locale : ui::I18n::languages())
    for (const bool dark : {false, true}) {
      const QByteArray name = QStringLiteral("%1-%2").arg(QString(locale.code), dark ? "dark" : "light").toLatin1();
      QTest::newRow(name.constData()) << locale.id << dark;
    }
}

void ApplicationUiBehaviorTests::dialogLayoutsKeepActionsVisible() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  auto& language = ui::I18n::instance();
  auto& theme = ui::ThemeManager::instance();
  const auto oldLanguage = language.lang();
  const auto oldTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    language.setLang(oldLanguage);
    theme.apply(oldTheme);
  });
  language.setLang(locale);
  QCOMPARE(language.lang(), locale);
  theme.apply(dark ? ui::Theme::Dark : ui::Theme::Light);
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  const auto inspect = [&](QDialog& dialog, const QString& name) {
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    const QSize workArea(1200, 700);
    dialog.resize(dialog.size().boundedTo(workArea));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QTRY_VERIFY2_WITH_TIMEOUT(test::clippedLabel(dialog).isEmpty(), qPrintable(test::clippedLabel(dialog)), 2000);
    if (!screenshots.isEmpty()) {
      QVERIFY(QDir().mkpath(screenshots));
      QVERIFY(dialog.grab().save(QDir(screenshots).filePath(name + "-" + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
    }
    QVERIFY2(dialog.width() <= workArea.width() && dialog.height() <= workArea.height(),
             qPrintable(QStringLiteral("%1 exceeds compact work area: %2x%3").arg(name).arg(dialog.width()).arg(dialog.height())));
    for (const auto buttons = dialog.findChildren<QPushButton*>(); auto* button : buttons) {
      if (!button->isVisible()) continue;
      QVERIFY2(dialog.rect().contains(QRect(button->mapTo(&dialog, QPoint()), button->size())), qPrintable(name + ": " + button->text()));
      QVERIFY2(!button->visibleRegion().isEmpty(), qPrintable(name + ": " + button->text()));
      QStyleOptionButton option;
      option.initFrom(button);
      const QRect contents = button->style()->subElementRect(QStyle::SE_PushButtonContents, &option, button);
      const int textWidth = button->fontMetrics().horizontalAdvance(button->text().remove('&'));
      QVERIFY2(contents.width() >= textWidth, qPrintable(name + ": " + button->text()));
    }
    dialog.hide();
  };
  const auto inspectModal = [&](const QString& name, const std::function<void()>& open) {
    bool inspected = false;
    QTimer::singleShot(0, this, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      QVERIFY(dialog);
      const auto close = qScopeGuard([dialog] { dialog->reject(); });
      inspect(*dialog, name);
      inspected = true;
    });
    open();
    QVERIFY2(inspected, qPrintable(name));
  };
  {
    ui::AboutDialog dialog([] { return QStringLiteral("Mock report, no machine data"); }, nullptr);
    inspect(dialog, QStringLiteral("about"));
  }
  {
    MemoryFileDialogs files;
    ui::ImportDialog dialog(files);
    inspect(dialog, QStringLiteral("import"));
    auto* recommended = buttonWithText(&dialog, ui::I18n::tr("Load recommended configuration"));
    QVERIFY(recommended);
    inspectModal(QStringLiteral("recommended-configuration"), [&] { recommended->click(); });
  }
  {
    RecordingWmiOperations wmi;
    MemoryFileStagingStore store;
    QWidget parent;
    ui::FileStagingCoordinator coordinator(wmi, store, UwfCapability::Unavailable, &parent);
    coordinator.requestCommit([](const ui::FileStagingBatchResult&) {});
    auto* progress = parent.findChild<QDialog*>(QStringLiteral("fileStagingProgressDialog"));
    QVERIFY(progress);
    inspect(*progress, QStringLiteral("file-staging-progress"));
    QVERIFY(wmi.invocations.empty());
    QCOMPARE(store.writes, 0);
  }
  {
    ui::SystemInformationDialog dialog(QStringLiteral("[Mock system]\nos=Windows 11\n[Mock token]\nelevated=false\n"));
    inspect(dialog, QStringLiteral("system-information"));
  }
  {
    RecordingWmiOperations wmi;
    MemoryFileDialogs files;
    auto snapshot = editableSnapshot();
    ui::GlobalStatusPanel global;
    global.setData(snapshot.current, snapshot.next, snapshot.runtime);
    QVERIFY(global.importFilterEnabled(false));
    ui::ApplyPlanDialog dialog(&global, {}, snapshot, ui::ApplyPlanServices{wmi, files});
    inspect(dialog, QStringLiteral("apply-plan"));
    QVERIFY(wmi.invocations.empty());
  }
  {
    ui::LogViewerDialog dialog;
    inspect(dialog, QStringLiteral("log"));
  }
  for (const auto state : {service::EnhancedModeState::Disabled, service::EnhancedModeState::Enabled, service::EnhancedModeState::Stopped,
                           service::EnhancedModeState::RepairRequired}) {
    RecordingWmiOperations wmi;
    MemoryEnhancedModeServiceControl control;
    if (state != service::EnhancedModeState::Disabled) control.current = completeEnhancedModeStatus();
    control.current.state = state;
    if (state == service::EnhancedModeState::Stopped) control.current.running = false;
    if (state == service::EnhancedModeState::RepairRequired) {
      control.current.executableMatches = false;
      control.current.detail =
          QStringLiteral("Mock service contract mismatch:\nC:\\") + QString(180, QLatin1Char('x')) + QStringLiteral("\\UWF.exe\n最后一行错误详情");
    }
    service::EnhancedModeManager manager(control, wmi, UwfCapability::Unavailable);
    ui::EnhancedModeDialog dialog(manager);
    inspect(dialog, QStringLiteral("enhanced-%1").arg(static_cast<int>(state)));
    QVERIFY(wmi.invocations.empty());
    QCOMPARE(control.installs + control.starts + control.removals, 0);
  }
  class OfflineRegistryBrowser final : public ui::RegistryBrowser {
   public:
    std::vector<std::string> rootHiveLongNames() const override { return {"HKEY_LOCAL_MACHINE"}; }
    std::vector<std::string> subkeyNames(std::string_view key) const override {
      return key == "HKEY_LOCAL_MACHINE" ? std::vector<std::string>{"SOFTWARE"} : std::vector<std::string>{};
    }
    bool hasSubkeys(std::string_view key) const override { return key == "HKEY_LOCAL_MACHINE"; }
    std::vector<regkey::RegValueInfo> values(std::string_view) const override { return {{"MockValue", REG_DWORD, std::vector<uint8_t>{1, 0, 0, 0}}}; }
  } browser;
  for (const auto mode :
       {ui::RegistryPickerDialog::Mode::CommitValue, ui::RegistryPickerDialog::Mode::DeleteValue, ui::RegistryPickerDialog::Mode::Exclusion}) {
    ui::RegistryPickerDialog dialog(mode, ui::I18n::tr("Commit registry changes"), browser);
    dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE"));
    inspect(dialog, QStringLiteral("registry-%1").arg(static_cast<int>(mode)));
  }
  for (const bool failure : {false, true}) {
    MemoryFileDialogs files;
    ui::OverlayFilesDialog dialog(QStringLiteral("C:"), {[failure](const QString&, std::stop_token) -> QVector<ui::OverlayFileEntry> {
                                                           if (failure) throw std::runtime_error("Mock provider error");
                                                           return {{QStringLiteral("\\Mock.txt"), {}, false, false, 1024}};
                                                         },
                                                         files});
    dialog.show();
    QTRY_VERIFY(!dialog.findChildren<QListWidget*>().isEmpty());
    inspect(dialog, failure ? QStringLiteral("overlay-error") : QStringLiteral("overlay-files"));
  }
  for (const auto operation : {ui::CommitOperation::Persist, ui::CommitOperation::DeleteAndPersist}) {
    inspectModal(QStringLiteral("commit-report-%1").arg(static_cast<int>(operation)), [&] {
      ui::showCommitReport(nullptr,
                           {{ui::I18n::tr("Succeeded"), QStringLiteral("C:\\Mock.txt"), QStringLiteral("-"), QStringLiteral("-"), true, false},
                            {ui::I18n::tr("Failed"), QStringLiteral("C:\\Blocked.txt"), QStringLiteral("0x80041001"),
                             ui::I18n::tr("The operation failed (see log for details)."), true, true}},
                           operation, 2);
    });
  }
  for (const auto action : {ui::PowerAction::Shutdown, ui::PowerAction::Restart}) {
    for (const auto mode : {ui::PowerActionDialogMode::Safe, ui::PowerActionDialogMode::Direct}) {
      inspectModal(QStringLiteral("power-%1-%2").arg(static_cast<int>(action)).arg(static_cast<int>(mode)),
                   [&] { QCOMPARE(ui::runPowerActionDialog(nullptr, {action, {}, {}, mode}), ui::PowerActionDialogOutcome::Canceled); });
    }
    inspectModal(QStringLiteral("power-failure-%1").arg(static_cast<int>(action)), [&] {
      const auto outcome = ui::runPowerActionDialog(nullptr, {action, {}, ui::PowerStagingFailure{QStringLiteral("Mock file staging failure")}});
      QVERIFY(outcome == ui::PowerActionDialogOutcome::Canceled || outcome == ui::PowerActionDialogOutcome::CanceledAfterStagingFailure);
    });
  }
}

void ApplicationUiBehaviorTests::longMessageDialogsKeepCompleteTextAndReachableActions_data() { dialogLayoutsKeepActionsVisible_data(); }

void ApplicationUiBehaviorTests::longMessageDialogsKeepCompleteTextAndReachableActions() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  const auto language = ui::I18n::instance().lang();
  const auto theme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(language);
    ui::ThemeManager::instance().apply(theme);
  });
  ui::I18n::instance().setLang(locale);
  ui::ThemeManager::instance().apply(dark ? ui::Theme::Dark : ui::Theme::Light);
  const QString target = QStringLiteral("C:\\") + QString(180, u'x') + QStringLiteral("\\<literal>&data.txt");
  const QString details = (QStringLiteral("WMI: 0x80041003 — mock failure; no operation performed.\n") + target + "\n").repeated(25);
  for (int purpose = 0; purpose < 4; ++purpose) {
    bool inspected = false;
    QTimer::singleShot(0, this, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      if (!dialog) return;
      const auto close = qScopeGuard([dialog] { dialog->reject(); });
      dialog->resize(800, 500);
      QTRY_COMPARE(dialog->size(), QSize(800, 500));
      QTRY_VERIFY2_WITH_TIMEOUT(test::clippedLabel(*dialog).isEmpty(), qPrintable(test::clippedLabel(*dialog)), 2000);
      auto* scroll = dialog->findChild<QScrollArea*>();
      QVERIFY(scroll);
      QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
      QTRY_VERIFY(scroll->verticalScrollBar()->maximum() > 0);
      const auto texts = dialog->findChildren<ui::ContentHintBox*>();
      QVERIFY(!texts.isEmpty());
      QVERIFY(std::ranges::any_of(texts, [&](const auto* text) { return text->toPlainText().contains(target); }));
      if (purpose >= 2) QVERIFY(std::ranges::any_of(texts, [&](const auto* text) { return text->toPlainText() == target; }));
      for (const auto boxes = dialog->findChildren<QDialogButtonBox*>(); auto* box : boxes) {
        QVERIFY(dialog->rect().contains(QRect(box->mapTo(dialog, QPoint()), box->size())));
        QVERIFY(!box->visibleRegion().isEmpty());
      }
      inspected = true;
    });
    switch (purpose) {
      case 0:
        ui::dialogs::warning(nullptr, ui::I18n::tr("Error"), details);
        break;
      case 1:
        QVERIFY(!ui::dialogs::confirm(nullptr, ui::I18n::tr("Confirm"), details));
        break;
      case 2:
        QVERIFY(!ui::dialogs::confirmCommit(nullptr, ui::I18n::tr("Commit"), QStringLiteral("Mock confirmation"), target, details));
        break;
      case 3:
        ui::dialogs::showCommitBlocker(nullptr, ui::I18n::tr("Error"), QStringLiteral("Mock unavailable operation"), target, details);
        break;
    }
    QVERIFY(inspected);
  }
}

void ApplicationUiBehaviorTests::typographyUsesNativeFontsAndAntialiasedText() {
  auto& theme = ui::ThemeManager::instance();
  theme.apply(theme.current());
  const QFont systemFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  QLabel label(QStringLiteral("UWF Manager · 文件暂存 · 0123456789"));
  label.setStyleSheet("QLabel { background: white; color: black; padding: 0; border: 0; }");
  label.resize(600, 80);
  label.show();
  QVERIFY(QTest::qWaitForWindowExposed(&label));
  QCOMPARE(label.font().family(), systemFont.family());
  QCOMPARE(label.font().hintingPreference(), QFont::PreferNoHinting);
  QVERIFY(label.font().styleStrategy() & QFont::PreferAntialias);
  QVERIFY(!(label.font().styleStrategy() & (QFont::NoAntialias | QFont::NoSubpixelAntialias)));
  for (const auto weight : {QFont::Normal, QFont::Medium, QFont::DemiBold}) {
    for (const auto& text : {QStringLiteral("UWF Manager · 0123456789"), QStringLiteral("全局设置 · 筛选器 · 覆盖层")}) {
      label.setText(text);
      QFont font = label.font();
      font.setWeight(weight);
      label.setFont(font);
      const auto image = label.grab().toImage().convertToFormat(QImage::Format_RGB32);
      int intermediatePixels = 0;
      for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x) {
          const QRgb pixel = image.pixel(x, y);
          if (pixel != qRgb(255, 255, 255) && pixel != qRgb(0, 0, 0)) ++intermediatePixels;
        }
      QVERIFY2(intermediatePixels > 100, "real QLabel rendering must contain antialiased edges, including semibold text");
      QTextLayout layout(label.text(), font);
      layout.beginLayout();
      auto line = layout.createLine();
      QVERIFY(line.isValid());
      line.setLineWidth(label.width());
      layout.endLayout();
      const auto runs = layout.glyphRuns();
      QVERIFY(!runs.isEmpty());
      for (const auto& run : runs) {
        QVERIFY(run.rawFont().isValid());
        qInfo() << "typography" << label.devicePixelRatioF() << QFontInfo(font).family() << weight << run.rawFont().familyName() << run.rawFont().styleName();
      }
      const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
      if (!screenshots.isEmpty()) {
        QVERIFY(QDir().mkpath(screenshots));
        const QString name = QStringLiteral("typography-%1-%2.png").arg(static_cast<int>(weight)).arg(text.startsWith("UWF") ? "en" : "zh");
        QVERIFY(label.grab().save(QDir(screenshots).filePath(name)));
        label.repaint();
        QTest::qWait(100);
        QVERIFY(label.screen()->grabWindow(label.winId()).save(QDir(screenshots).filePath("native-" + name)));
      }
    }
  }
  ui::GlobalStatusPanel panel;
  panel.setData(editableSnapshot().current, editableSnapshot().next, editableSnapshot().runtime);
  panel.show();
  QVERIFY(QTest::qWaitForWindowExposed(&panel));
  auto* title = panel.findChild<QLabel*>(QStringLiteral("panelTitle"));
  QVERIFY(title);
  QCOMPARE(title->font().weight(), QFont::Medium);
  QCOMPARE(title->font().hintingPreference(), QFont::PreferNoHinting);
  QVERIFY(!(title->font().styleStrategy() & QFont::NoAntialias));
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!screenshots.isEmpty()) {
    QTest::qWait(100);
    QVERIFY(panel.screen()->grabWindow(panel.winId()).save(QDir(screenshots).filePath(QStringLiteral("native-settings.png"))));
  }
}

}  // namespace

QTEST_MAIN(ApplicationUiBehaviorTests)

#include "ApplicationUiBehaviorTests.moc"
