// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QProgressBar>
#include <QPushButton>
#include <QScopeGuard>
#include <QScreen>
#include <QSet>
#include <QSignalSpy>
#include <QTabBar>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTimer>
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
#include "app/staging/FileStagingCommitter.h"
#include "app/staging/FileStagingConflictPolicy.h"
#include "app/staging/FileStagingDraft.h"
#include "app/staging/FileStagingTask.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "bootstrap/MainWindowComposition.h"
#include "bootstrap/ViewModelComposition.h"
#include "bootstrap/WorkflowComposition.h"
#include "core/UwfModel.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PageProxyModel.h"
#include "presentation/common/PresentationTiming.h"
#include "presentation/staging/FileStagingResultText.h"
#include "shared/Log.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/WidgetCapture.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/configuration/ApplyPlanDialog.h"
#include "ui/configuration/ExclusionListWidget.h"
#include "ui/configuration/VolumeTab.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/overlay/OverlayHudPalette.h"
#include "ui/shell/MainWindow.h"
#include "ui/staging/FileStagingWidget.h"
#include "ui/widgets/EntryListView.h"
#include "ui/widgets/ContentHintBox.h"
#include "windows/ipc/EnhancedModeAgent.h"
#include "windows/system/SystemHardwareInfo.h"

class ApplicationFileStagingTests final : public ApplicationUiTest {
  Q_OBJECT
 private slots:
  void fileStagingTabDefersEditsAndCollapsesCoveredPaths();
  void clearingFileStagingCanBeReviewedCancelledAndRestored();
  void fileStagingIndicatorTracksEnhancedModeWithoutDisablingEdits();
  void fileStagingAvailabilityKeepsListSpace_data();
  void fileStagingAvailabilityKeepsListSpace();
  void fileStagingTabPreservesStateAcrossInvalidInputAndStorageFailures();
  void fileStagingAndFileExclusionsRejectOverlapFromEitherEntryPoint();
  void fileStagingCommandUiCoalescesRequestsAndCompletesEveryTarget();
  void mainWindowMountsFileStagingOnlyWherePerFileCommitIsSupported();
};

using namespace uwf;
using namespace uwf::test::application;

void ApplicationFileStagingTests::fileStagingTabDefersEditsAndCollapsesCoveredPaths() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString filePath = QDir::toNativeSeparators(directory.filePath(QStringLiteral("state.bin")));
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  QVERIFY(file.write("state") > 0);
  file.close();
  const QString diskDrive = QString::fromStdString(drive::fromPath(filePath.toStdString()));
  QVERIFY(!diskDrive.isEmpty());
  const QString otherDrive = diskDrive.compare(QStringLiteral("C:"), Qt::CaseInsensitive) == 0 ? QStringLiteral("D:") : QStringLiteral("C:");
  const app::FileStagingEntry otherVolumeEntry{app::FileStagingKind::File, otherDrive + QStringLiteral("\\Other\\keep.bin")};

  FileDialogStub dialogs;
  auto storeOwner = std::make_shared<MemoryFileStagingStore>();
  auto& store = *storeOwner;
  store.entries = {otherVolumeEntry};
  app::FileStagingDraft draft(storeOwner);
  const core::VolumeInfo disk{diskDrive.toStdString(), "Volume{current}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  presentation::ConfigurationViewModel configuration(16384, [&draft] { return !draft.load().isEmpty(); });
  ui::VolumeTab tab(configuration.addVolume(disk, true), dialogs, &draft, uwf::bootstrap::createViewModelFactory());
  tab.viewModel().applySnapshot(editableSnapshot());

  auto* innerTabs = tab.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(innerTabs);
  QCOMPARE(innerTabs->count(), 3);
  QCOMPARE(innerTabs->tabText(0), QStringLiteral("File exclusions"));
  QCOMPARE(innerTabs->tabText(1), QStringLiteral("Registry exclusions"));
  QCOMPARE(innerTabs->tabText(2), QStringLiteral("File staging"));

  auto* staging = innerTabs->widget(2)->findChild<ui::FileStagingWidget*>();
  QVERIFY(staging);
  QCOMPARE(staging->driveLetter(), diskDrive);
  auto* addFiles = staging->commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* addDirectory = staging->commandWidget()->findChild<QAction*>(QStringLiteral("stageDirectoryAction"));
  auto* list = staging->findChild<QListView*>(QStringLiteral("exclusionList"));
  auto* filter = staging->commandWidget()->findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(addFiles && addDirectory && list && filter);
  QVERIFY(!staging->findChild<QWidget*>(QStringLiteral("fileStagingEnhancedModeNotice")));
  QVERIFY(addFiles->isEnabled() && addDirectory->isEnabled());
  QCOMPARE(list->model()->rowCount(), 0);

  dialogs.openedFile = filePath;
  dialogs.openedFiles = {dialogs.openedFile};
  addFiles->trigger();
  QCOMPARE(store.writes, 0);
  QCOMPARE(draft.load().size(), 2);
  const app::FileStagingEntry currentVolumeFile{app::FileStagingKind::File, filePath};
  QVERIFY(draft.load().contains(otherVolumeEntry));
  QVERIFY(draft.load().contains(currentVolumeFile));
  QCOMPARE(list->model()->rowCount(), 1);

  // Windows 路径大小写不敏感；同一文件的不同盘符大小写不能产生第二条记录。
  QString alternateCase = filePath;
  alternateCase[0] = alternateCase[0].toLower();
  dialogs.openedFile = alternateCase;
  dialogs.openedFiles = {dialogs.openedFile};
  addFiles->trigger();
  QCOMPARE(store.writes, 0);

  filter->setText(QStringLiteral("not-present"));
  QCOMPARE(list->model()->rowCount(), 0);
  QVERIFY(draft.load().contains(currentVolumeFile));
  filter->setText(QFileInfo(filePath).fileName());
  QCOMPARE(list->model()->rowCount(), 1);
  filter->clear();

  dialogs.selectedDirectory = QDir::toNativeSeparators(directory.path());
  addDirectory->trigger();
  QCOMPARE(store.writes, 0);
  QCOMPARE(draft.load().size(), 2);
  const app::FileStagingEntry currentVolumeDirectory{app::FileStagingKind::Directory, dialogs.selectedDirectory};
  QVERIFY(draft.load().contains(otherVolumeEntry));
  QVERIFY(draft.load().contains(currentVolumeDirectory));

  dialogs.openedFile = filePath;
  dialogs.openedFiles = {dialogs.openedFile};
  addFiles->trigger();
  QCOMPARE(store.writes, 0);
  QCOMPARE(list->model()->rowCount(), 1);
  list->selectionModel()->select(list->model()->index(0, 0), QItemSelectionModel::Select);
  auto* remove = buttonWithText(staging->commandWidget(), QStringLiteral("Remove selected"));
  QVERIFY(remove);
  QTest::mouseClick(remove, Qt::LeftButton);
  QCOMPARE(store.writes, 0);
  QCOMPARE(draft.load(), QList<app::FileStagingEntry>{otherVolumeEntry});
}

void ApplicationFileStagingTests::clearingFileStagingCanBeReviewedCancelledAndRestored() {
  const auto originalClipboard = QApplication::clipboard()->text();
  const auto restoreClipboard = qScopeGuard([&] { QApplication::clipboard()->setText(originalClipboard); });
  RecordingWmiOperations wmi;
  auto storeOwner = std::make_shared<MemoryFileStagingStore>();
  auto& store = *storeOwner;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\first.bin")},
                   {app::FileStagingKind::Directory, QStringLiteral("C:\\State\\folder")}};
  const auto saved = store.entries;
  const QString root = QString::fromStdString(std::string(app::kFileStagingRegistryRoot));
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.current.registryExclusions = source.snapshot.next.registryExclusions = {root.toStdString()};
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, storeOwner),
                        {.uwfCapability = app::UwfCapability::Available, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  auto* tab = window.findChild<ui::VolumeTab*>();
  auto* staging = window.findChild<ui::FileStagingWidget*>();
  QVERIFY(tab && staging);
  tab->setActiveInfoPage(ui::VolumeTab::InfoPage::FileStaging);
  auto* list = staging->findChild<QListView*>(QStringLiteral("exclusionList"));
  auto* remove = buttonWithText(staging->commandWidget(), QStringLiteral("Remove selected"));
  QVERIFY(list && remove);
  list->selectAll();
  QTest::mouseClick(remove, Qt::LeftButton);
  QCOMPARE(store.entries, saved);
  QCOMPARE(store.writes, 0);
  QCOMPARE(list->model()->rowCount(), 2);
  for (int row = 0; row < list->model()->rowCount(); ++row)
    QCOMPARE(list->model()->index(row, 0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Removed));
  for (int attempt = 0; attempt < 2; ++attempt) {
    bool inspected = false;
    QTimer::singleShot(0, &window, [&] {
      auto* plan = qobject_cast<ui::ApplyPlanDialog*>(QApplication::activeModalWidget());
      QVERIFY(plan);
      const auto close = qScopeGuard([plan] { plan->reject(); });
      auto* changes = plan->findChild<QListView*>(QStringLiteral("applyChangesList"));
      QVERIFY(changes);
      QCOMPARE(changes->model()->rowCount(), 6);  // Two groups, three changes, and the command disclosure.
      for (const auto& expected : {saved[0].path, saved[1].path, root}) {
        QModelIndex found;
        for (int row = 0; row < changes->model()->rowCount(); ++row) {
          const auto index = changes->model()->index(row, 0);
          if (index.data().toString() == expected) found = index;
        }
        QVERIFY(found.isValid());
        changes->scrollTo(found, QAbstractItemView::PositionAtCenter);
        const auto rendered = [&]() -> ui::ContentHintBox* {
          for (const auto titles = changes->findChildren<ui::ContentHintBox*>(); auto* title : titles)
            if (title->isVisibleTo(changes) && title->toPlainText() == expected) return title;
          return nullptr;
        };
        QTRY_VERIFY(rendered());
        auto* title = rendered();
        QCOMPARE(title->focusPolicy(), Qt::ClickFocus);
        title->setFocus();
        const auto before = title->viewport()->grab().toImage();
        title->selectAll();
        QCOMPARE(title->textCursor().selectedText(), expected);
        QVERIFY(title->viewport()->grab().toImage() != before);
        QTest::keyClick(title, Qt::Key_C, Qt::ControlModifier);
        QCOMPARE(QApplication::clipboard()->text(), expected);
        auto cursor = title->textCursor();
        cursor.clearSelection();
        title->setTextCursor(cursor);
      }
      changes->scrollToTop();
      auto* exportButton = buttonWithText(plan, QStringLiteral("Export commands"));
      QVERIFY(exportButton && !exportButton->isEnabled());
      QCOMPARE(store.entries, saved);
      QCOMPARE(store.writes, 0);
      QVERIFY(wmi.invocations.empty());
      QVERIFY(uwf::test::saveScreenshot(*plan, QStringLiteral("staging-clear-preview")));
      inspected = true;
    });
    window.showPlan();
    QVERIFY(inspected);
    QCOMPARE(store.entries, saved);
    QCOMPARE(store.writes, 0);
  }
  list->selectAll();
  QCOMPARE(remove->property("accessibleName").toString(), QStringLiteral("Undo removal"));
  QTest::mouseClick(remove, Qt::LeftButton);
  QCOMPARE(list->model()->rowCount(), 2);
  for (int row = 0; row < list->model()->rowCount(); ++row)
    QCOMPARE(list->model()->index(row, 0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Normal));
  QCOMPARE(store.entries, saved);
  QCOMPARE(store.writes, 0);
  QVERIFY(wmi.invocations.empty());

  // Recreate the same draft and accept both the preview and its confirmation.
  list->selectAll();
  QTest::mouseClick(remove, Qt::LeftButton);
  wmi.queryResults.push_back({{{"__PATH", app::wmi::WmiValue::fromString("registry-next")},
                               {"CurrentSession", app::wmi::WmiValue::fromBool(false)},
                               {"PersistDomainSecretKey", app::wmi::WmiValue::fromBool(false)},
                               {"PersistTSCAL", app::wmi::WmiValue::fromBool(false)}}});
  app::wmi::WmiMethodOutput present, absent;
  present.values.emplace("bFound", app::wmi::WmiValue::fromBool(true));
  absent.values.emplace("bFound", app::wmi::WmiValue::fromBool(false));
  wmi.readResults = {absent};
  wmi.invocationObserver = [&] { source.snapshot.next.registryExclusions.clear(); };
  bool confirmed = false;
  bool applied = false;
  QTimer::singleShot(0, &window, [&] {
    auto* plan = qobject_cast<ui::ApplyPlanDialog*>(QApplication::activeModalWidget());
    QVERIFY(plan);
    auto* apply = buttonWithText(plan, QStringLiteral("Apply changes"));
    QVERIFY(apply && apply->isEnabled());
    QTimer::singleShot(0, plan, [&, plan] {
      auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      QVERIFY(confirmation && confirmation != plan);
      QCOMPARE(store.writes, 0);
      QCOMPARE(store.entries, saved);
      auto* accept = buttonWithText(confirmation, QStringLiteral("Apply changes"));
      QVERIFY(accept);
      confirmed = true;
      QTest::mouseClick(accept, Qt::LeftButton);
    });
    QTest::mouseClick(apply, Qt::LeftButton);
    QTRY_VERIFY(applyCompleted(*plan));
    const auto close = qScopeGuard([plan] { plan->reject(); });
    QVERIFY(confirmed);
    QCOMPARE(store.writes, 1);
    QVERIFY(store.entries.isEmpty());
    QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("RemoveExclusion")});
    applied = true;
  });
  window.showPlan();
  QVERIFY(applied);
}

void ApplicationFileStagingTests::fileStagingIndicatorTracksEnhancedModeWithoutDisablingEdits() {
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\pending.bin")}};
  const auto originalEntries = staging.entries;
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  MemoryEnhancedModeAgent agent;
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.current.filter.enabled = false;
  source.snapshot.next.filter.enabled = false;
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner, serviceControlOwner, &agent),
                        {.uwfCapability = uwf::app::UwfCapability::Available, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  auto* disk = window.findChild<ui::VolumeTab*>();
  auto* widget = window.findChild<ui::FileStagingWidget*>();
  auto* enhancedAction = actionWithText(&window, QStringLiteral("Enhanced mode"));
  QVERIFY(disk && widget && enhancedAction);
  disk->setActiveInfoPage(ui::VolumeTab::InfoPage::FileStaging);
  auto* tabs = disk->findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  auto* list = widget->findChild<QListView*>(QStringLiteral("exclusionList"));
  auto* filter = widget->commandWidget()->findChild<QLineEdit*>(QStringLiteral("filterInput"));
  auto* add = widget->commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* remove = buttonWithText(widget->commandWidget(), QStringLiteral("Remove selected"));
  QVERIFY(tabs && list && filter && add && remove);
  const auto hasIndicator = [&] { return tabs->tabBar()->property("designTabIndicators").toMap().contains(QString::number(tabs->currentIndex())); };
  QVERIFY(hasIndicator());
  QVERIFY(!widget->findChild<QWidget*>(QStringLiteral("fileStagingEnhancedModeNotice")));
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  const QRect listGeometry = list->geometry();
  QVERIFY(list->isEnabled() && filter->isEnabled() && add->isEnabled());
  QVERIFY(!remove->isEnabled());
  QCOMPARE(list->model()->rowCount(), 1);
  filter->setText(QStringLiteral("pending"));
  list->selectionModel()->select(list->model()->index(0, 0), QItemSelectionModel::Select);
  QVERIFY(remove->isEnabled());

  for (const auto state : {app::EnhancedModeState::Enabled, app::EnhancedModeState::Stopped, app::EnhancedModeState::RepairRequired,
                           app::EnhancedModeState::Disabled, app::EnhancedModeState::Enabled}) {
    serviceControl.current = completeEnhancedModeStatus();
    serviceControl.current.state = state;
    if (state == app::EnhancedModeState::Stopped) serviceControl.current.running = false;
    if (state == app::EnhancedModeState::RepairRequired) serviceControl.current.executableMatches = false;
    if (state == app::EnhancedModeState::Disabled) serviceControl.current = {};
    bool dialogObserved = false;
    QTimer::singleShot(0, &window, [&dialogObserved] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      if (!dialog || dialog->objectName() != QStringLiteral("enhancedModeDialog")) return;
      dialogObserved = true;
      dialog->reject();
    });
    enhancedAction->trigger();
    QVERIFY(dialogObserved);
    QCOMPARE(hasIndicator(), state != app::EnhancedModeState::Enabled);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QCOMPARE(list->geometry(), listGeometry);
    QVERIFY(list->isEnabled() && filter->isEnabled() && add->isEnabled() && remove->isEnabled());
    QCOMPARE(filter->text(), QStringLiteral("pending"));
    QVERIFY(list->selectionModel()->isSelected(list->model()->index(0, 0)));
    QCOMPARE(staging.entries, originalEntries);
    QCOMPARE(staging.writes, 0);
  }
  window.refresh();
  QTRY_VERIFY(refreshCompleted(window));
  auto* refreshedTabs = window.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(refreshedTabs && refreshedTabs->tabBar()->property("designTabIndicators").toMap().isEmpty());
  QCOMPARE(staging.entries, originalEntries);
  QCOMPARE(staging.writes, 0);
  QVERIFY(wmi.invocations.empty());
}

void ApplicationFileStagingTests::fileStagingAvailabilityKeepsListSpace_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<QSize>("windowSize");
  for (const auto& locale : ui::I18n::languages()) {
    for (const bool dark : {false, true}) {
      for (const auto size : {QSize(560, 320), QSize(920, 400)}) {
        const QByteArray name = QStringLiteral("%1-%2-%3").arg(QString(locale.code), dark ? "dark" : "light").arg(size.width()).toLatin1();
        QTest::newRow(name.constData()) << locale.id << dark << size;
      }
    }
  }
}

void ApplicationFileStagingTests::fileStagingAvailabilityKeepsListSpace() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  QFETCH(QSize, windowSize);
  auto& language = ui::I18n::instance();
  auto& theme = ui::ThemeManager::instance();
  const auto oldLanguage = language.lang();
  const auto oldTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    language.setLang(oldLanguage);
    theme.apply(oldTheme);
  });
  language.setLang(locale);
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  FileDialogStub dialogs;
  auto storeOwner = std::make_shared<MemoryFileStagingStore>();
  auto& store = *storeOwner;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\Data\\state.bin")},
                   {app::FileStagingKind::Directory, QStringLiteral("C:\\Data\\Settings")}};
  app::FileStagingDraft storeDraft(storeOwner);
  ui::FileStagingWidget widget(uwf::bootstrap::createViewModelFactory()->fileStaging(QStringLiteral("C:"), storeDraft), dialogs);
  widget.show();
  QVERIFY(QTest::qWaitForWindowExposed(&widget));
  widget.resize(windowSize);
  QTRY_COMPARE(widget.size(), windowSize);
  auto* list = widget.findChild<QListView*>(QStringLiteral("exclusionList"));
  QVERIFY(list && list->isVisible() && list->isEnabled());
  QVERIFY(!widget.findChild<QWidget*>(QStringLiteral("fileStagingEnhancedModeNotice")));
  const QRect listGeometry = list->geometry();
  widget.setEnhancedModeAvailable(true);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QCOMPARE(list->geometry(), listGeometry);
  widget.setEnhancedModeAvailable(false);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QCOMPARE(list->geometry(), listGeometry);
  auto* entries = dynamic_cast<ui::EntryListView*>(list);
  QVERIFY(entries);
  QCOMPARE(entries->pages().totalRows(), 2);
  QSet<QString> observed;
  entries->pages().goFirst();
  do {
    for (int row = 0; row < list->model()->rowCount(); ++row) observed.insert(list->model()->index(row, 0).data(presentation::PathRole).toString());
    if (!entries->pages().pager().hasNext(entries->pages().totalRows())) break;
    entries->pages().goNext();
  } while (true);
  QCOMPARE(observed, QSet<QString>({QStringLiteral("C:\\Data\\state.bin"), QStringLiteral("C:\\Data\\Settings")}));
  entries->pages().goFirst();
  const QString clipped = test::clippedText(widget);
  QVERIFY2(clipped.isEmpty(), qUtf8Printable(clipped));
  QVERIFY(list->viewport()->height() >= 64);
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!screenshots.isEmpty()) {
    QVERIFY(QDir().mkpath(screenshots));
    QVERIFY(uwf::test::screenshot(widget).save(QDir(screenshots).filePath(QStringLiteral("staging-") + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
  }
}

void ApplicationFileStagingTests::fileStagingTabPreservesStateAcrossInvalidInputAndStorageFailures() {
  QTemporaryFile file;
  QVERIFY(file.open());
  const QString filePath = QDir::toNativeSeparators(file.fileName());
  const QString drive = QString::fromStdString(drive::fromPath(filePath.toStdString()));
  QVERIFY(!drive.isEmpty());

  FileDialogStub dialogs;
  auto storeOwner = std::make_shared<MemoryFileStagingStore>();
  auto& store = *storeOwner;
  store.entries = {{app::FileStagingKind::File, filePath}};
  app::FileStagingDraft storeDraft(storeOwner);
  ui::FileStagingWidget staging(uwf::bootstrap::createViewModelFactory()->fileStaging(drive, storeDraft), dialogs);
  auto* list = staging.findChild<QListView*>(QStringLiteral("exclusionList"));
  auto* remove = buttonWithText(staging.commandWidget(), QStringLiteral("Remove selected"));
  auto* addDirectory = staging.commandWidget()->findChild<QAction*>(QStringLiteral("stageDirectoryAction"));
  QVERIFY(list && remove && addDirectory);
  QCOMPARE(list->model()->rowCount(), 1);

  // 只读与存储故障是两种状态：未提权时仍能筛选、选择和复制现有条目，
  // 但任何程序化或真实按钮触发都不能进入持久化边界。
  auto* addFiles = staging.commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* filter = staging.commandWidget()->findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(addFiles && filter);
  staging.setReadOnly(true);
  QVERIFY(!addFiles->isEnabled());
  QVERIFY(!addDirectory->isEnabled());
  QVERIFY(!remove->isEnabled());
  QVERIFY(list->isEnabled());
  QVERIFY(filter->isEnabled());
  dialogs.openedFile = filePath;
  dialogs.openedFiles = {dialogs.openedFile};
  addFiles->trigger();
  QCOMPARE(store.writes, 0);
  staging.setReadOnly(false);
  QVERIFY(addFiles->isEnabled());
  QVERIFY(addDirectory->isEnabled());
  QVERIFY(!remove->isEnabled());

  dialogs.selectedDirectory = filePath;
  QTimer::singleShot(0, &staging, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->reject();
  });
  addDirectory->trigger();
  QCOMPARE(store.writes, 0);
  QCOMPARE(list->model()->rowCount(), 1);

  dialogs.openedFile = QStringLiteral("C:relative-state.bin");
  dialogs.openedFiles = {dialogs.openedFile};
  QTimer::singleShot(0, &staging, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->reject();
  });
  addFiles->trigger();
  QCOMPARE(store.writes, 0);
  QCOMPARE(list->model()->rowCount(), 1);

  const QString otherDrive = drive.compare(QStringLiteral("C:"), Qt::CaseInsensitive) == 0 ? QStringLiteral("D:") : QStringLiteral("C:");
  auto crossDriveStoreOwner = std::make_shared<MemoryFileStagingStore>();
  auto& crossDriveStore = *crossDriveStoreOwner;
  app::FileStagingDraft crossDriveStoreDraft(crossDriveStoreOwner);
  ui::FileStagingWidget crossDrive(uwf::bootstrap::createViewModelFactory()->fileStaging(otherDrive, crossDriveStoreDraft), dialogs);
  auto* crossDriveAction = crossDrive.commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* crossDriveList = crossDrive.findChild<QListView*>(QStringLiteral("exclusionList"));
  QVERIFY(crossDriveAction && crossDriveList);
  dialogs.openedFile = filePath;
  dialogs.openedFiles = {dialogs.openedFile};
  QTimer::singleShot(0, &crossDrive, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->reject();
  });
  crossDriveAction->trigger();
  QCOMPARE(crossDriveStore.writes, 0);
  QVERIFY(crossDriveStore.entries.isEmpty());
  QCOMPARE(crossDriveList->model()->rowCount(), 0);
  QCOMPARE(dialogs.requests.back().initialPath, ui::dialogs::dialogBasePath(otherDrive));

  store.writeFailure = QStringLiteral("registry write failed");
  list->selectionModel()->select(list->model()->index(0, 0), QItemSelectionModel::Select);
  QTimer::singleShot(0, &staging, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->reject();
  });
  QTest::mouseClick(remove, Qt::LeftButton);
  QCOMPARE(store.writes, 0);
  QCOMPARE(store.entries.size(), 1);
  QCOMPARE(list->model()->rowCount(), 1);

  auto unavailableStoreOwner = std::make_shared<MemoryFileStagingStore>();

  auto& unavailableStore = *unavailableStoreOwner;
  unavailableStore.readFailure = QStringLiteral("registry read failed");
  app::FileStagingDraft unavailableStoreDraft(unavailableStoreOwner);
  ui::FileStagingWidget unavailable(uwf::bootstrap::createViewModelFactory()->fileStaging(drive, unavailableStoreDraft), dialogs);
  auto* unavailableList = unavailable.findChild<QListView*>(QStringLiteral("exclusionList"));
  auto* unavailableFilter = unavailable.commandWidget()->findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(unavailableList && unavailableFilter);
  QVERIFY(!unavailableList->isEnabled());
  QVERIFY(!unavailableFilter->isEnabled());
}

void ApplicationFileStagingTests::fileStagingAndFileExclusionsRejectOverlapFromEitherEntryPoint() {
  QTemporaryFile file;
  QVERIFY(file.open());
  const QString filePath = QDir::toNativeSeparators(file.fileName());
  const QString drive = QString::fromStdString(drive::fromPath(filePath.toStdString()));
  QVERIFY(!drive.isEmpty());

  app::FileStagingConflictPolicy conflicts;
  FileDialogStub dialogs;
  ui::ExclusionListWidget exclusions(std::make_shared<uwf::presentation::ExclusionListViewModel>(ui::ExclusionListWidget::Kind::File, &conflicts), dialogs);
  exclusions.setDriveLetter(drive);
  exclusions.setBaseline({filePath}, {filePath});

  auto emptyStoreOwner = std::make_shared<MemoryFileStagingStore>();

  auto& emptyStore = *emptyStoreOwner;
  app::FileStagingDraft emptyStoreDraft(emptyStoreOwner);
  ui::FileStagingWidget staging(uwf::bootstrap::createViewModelFactory()->fileStaging(drive, emptyStoreDraft, &conflicts), dialogs);
  dialogs.openedFile = filePath;
  dialogs.openedFiles = {dialogs.openedFile};
  QTimer::singleShot(0, &staging, [] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) warning->reject();
  });
  auto* addFiles = staging.commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  QVERIFY(addFiles);
  addFiles->trigger();
  QCOMPARE(emptyStore.writes, 0);
  QVERIFY(emptyStore.entries.isEmpty());

  auto stagedStoreOwner = std::make_shared<MemoryFileStagingStore>();

  auto& stagedStore = *stagedStoreOwner;
  stagedStore.entries = {{app::FileStagingKind::File, filePath}};
  app::FileStagingDraft stagedStoreDraft(stagedStoreOwner);
  ui::FileStagingWidget staged(uwf::bootstrap::createViewModelFactory()->fileStaging(drive, stagedStoreDraft, &conflicts), dialogs);
  QVERIFY(staged.findChild<QListView*>(QStringLiteral("exclusionList")));
  exclusions.setBaseline({}, {});
  bool conflictShown = false;
  QTimer::singleShot(0, &exclusions, [&] {
    if (auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget())) {
      conflictShown = true;
      warning->reject();
    }
  });
  exclusions.addPendingEntry(filePath);
  QVERIFY(conflictShown);
  QVERIFY(exclusions.pendingAdded().isEmpty());
}

void ApplicationFileStagingTests::fileStagingCommandUiCoalescesRequestsAndCompletesEveryTarget() {
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
  const QString drive = QString::fromStdString(drive::fromPath(paths.front().toStdString()));
  QVERIFY(!drive.isEmpty());

  RecordingWmiOperations wmi;
  wmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                               {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  wmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                               {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                               {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                               {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                               {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  wmi.readResults.push_back(fileExclusionResult());
  auto storeOwner = std::make_shared<MemoryFileStagingStore>();
  auto& store = *storeOwner;
  for (const QString& path : paths) store.entries.append({app::FileStagingKind::File, path});

  QWidget parent;
  parent.show();
  auto coordinatorOwner = uwf::bootstrap::createFileStagingCoordinator(wmi, store, uwf::app::UwfCapability::Available, &parent);
  auto& coordinator = *coordinatorOwner;
  QSignalSpy progress(&coordinator, &app::FileStagingCoordinator::progressChanged);
  connect(&coordinator, &app::FileStagingCoordinator::progressChanged, &coordinator,
          [](std::size_t, std::size_t) { throw std::runtime_error("progress observer failure"); });
  int completions = 0;
  app::ApplicationCommandResult firstResult;
  app::ApplicationCommandResult secondResult;
  QElapsedTimer visibleDuration;
  visibleDuration.start();
  coordinator.requestCommit([&](const app::FileStagingBatchResult& result) {
    firstResult = result.command;
    ++completions;
  });
  coordinator.requestCommit([&](const app::FileStagingBatchResult& result) {
    secondResult = result.command;
    ++completions;
  });
  bool externalOwnershipTransferred = false;
  bool requestQueuedDuringExternalOwnership = false;
  std::optional<app::ApplicationCommandResult> queuedResult;
  auto immediateBatch = coordinator.reserveExternalBatch([&](const app::FileStagingBatchResult& result, app::FileStagingCoordinator::ExternalBatch batch) {
    externalOwnershipTransferred = coordinator.active();
    QCOMPARE(result.sourceEntries, std::optional<QList<app::FileStagingEntry>>{store.entries});
    coordinator.requestCommit([&](const app::FileStagingBatchResult& queued) {
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
  QVERIFY(!dialog->windowTitle().isEmpty());
  QTest::keyClick(dialog, Qt::Key_Escape);
  QVERIFY(dialog->isVisible());
  QVERIFY(coordinator.active());
  QVERIFY(!dialog->close());
  QVERIFY(dialog->isVisible());
  QVERIFY(dialog->findChild<QProgressBar*>(QStringLiteral("fileStagingCommandProgress")));
  auto* currentPath = dialog->findChild<QLineEdit*>(QStringLiteral("fileStagingCommandPath"));
  QVERIFY(currentPath && currentPath->isReadOnly());
  QTRY_VERIFY_WITH_TIMEOUT(!currentPath->text().isEmpty(), 1000);
  QVERIFY(dialog->screen()->availableGeometry().contains(dialog->frameGeometry()));
  QVERIFY(paths.contains(currentPath->text()));
  QTRY_COMPARE_WITH_TIMEOUT(completions, 2, 5000);
  QVERIFY(visibleDuration.elapsed() >= std::chrono::duration_cast<std::chrono::milliseconds>(presentation::timing::kMinimumProgressDisplayDuration).count());
  QCOMPARE(firstResult.outcome, app::ApplicationCommandOutcome::Succeeded);
  QCOMPARE(firstResult.committedFiles, std::size_t{2});
  QCOMPARE(secondResult.committedFiles, firstResult.committedFiles);
  QVERIFY(externalOwnershipTransferred);
  QVERIFY(requestQueuedDuringExternalOwnership);
  QVERIFY(queuedResult.has_value());
  QCOMPARE(queuedResult->committedFiles, firstResult.committedFiles);
  QCOMPARE(wmi.invocations, std::vector<QString>({QStringLiteral("CommitFile"), QStringLiteral("CommitFile")}));
  QVERIFY(!progress.isEmpty());

  auto exclusiveBatch = coordinator.reserveExternalBatch([](const app::FileStagingBatchResult&, app::FileStagingCoordinator::ExternalBatch) {});
  QVERIFY(exclusiveBatch.has_value());
  QVERIFY_THROWS_EXCEPTION(
      std::logic_error,
      static_cast<void>(coordinator.reserveExternalBatch([](const app::FileStagingBatchResult&, app::FileStagingCoordinator::ExternalBatch) {})));
  bool completionAfterThrowWasDelivered = false;
  coordinator.requestCommit([](const app::FileStagingBatchResult&) { throw std::runtime_error("observer failure"); });
  coordinator.requestCommit([&](const app::FileStagingBatchResult&) { completionAfterThrowWasDelivered = true; });
  exclusiveBatch->complete({firstResult, store.entries});
  QVERIFY(completionAfterThrowWasDelivered);
  QVERIFY(!coordinator.active());
}

void ApplicationFileStagingTests::mainWindowMountsFileStagingOnlyWherePerFileCommitIsSupported() {
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  MutableApplicationStateSource source;
  const std::string systemDrive = drive::systemLetter();
  QVERIFY(!systemDrive.empty());
  const std::string dataDrive = systemDrive == "C:" ? "D:" : "C:";
  staging.entries = {{app::FileStagingKind::File, QString::fromStdString(dataDrive) + QStringLiteral("\\Data\\state.bin")},
                     {app::FileStagingKind::Directory, QString::fromStdString(systemDrive) + QStringLiteral("\\ProgramData\\Vendor")}};
  source.volumes = {{dataDrive, "Volume{data}", "NTFS", "Removable data", 1000, 500, core::VolumeSupport::NotFixedLocalDisk},
                    {systemDrive, "Volume{system}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot = editableSnapshot();
  const QString stagingRegistryRoot = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager");
  source.snapshot.next.registryExclusions.push_back(stagingRegistryRoot.toStdString());

  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner),
                        {.uwfCapability = uwf::app::UwfCapability::Available, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));

  auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"));
  QVERIFY(tabs);
  QCOMPARE(tabs->count(), 2);
  QCOMPARE(window.findChildren<ui::FileStagingWidget*>().size(), 1);

  auto* dataTab = qobject_cast<ui::VolumeTab*>(tabs->widget(0));
  auto* systemTab = qobject_cast<ui::VolumeTab*>(tabs->widget(1));
  QVERIFY(dataTab && systemTab);
  auto* dataInnerTabs = dataTab->findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  auto* systemInnerTabs = systemTab->findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(dataInnerTabs && systemInnerTabs);
  QCOMPARE(dataInnerTabs->count(), 1);
  QCOMPARE(systemInnerTabs->count(), 3);
  QCOMPARE(systemInnerTabs->tabText(1), QStringLiteral("Registry exclusions"));
  QCOMPARE(systemInnerTabs->tabText(2), QStringLiteral("File staging"));

  auto* registryExclusions = systemInnerTabs->widget(1)->findChild<ui::ExclusionListWidget*>();
  QVERIFY(registryExclusions);
  auto* registryList = registryExclusions->findChild<QListView*>(QStringLiteral("exclusionList"));
  auto* registryRemove = buttonWithText(registryExclusions->commandWidget(), QStringLiteral("Remove selected"));
  QVERIFY(registryList && registryRemove);
  bool selectedStagingRoot = false;
  for (int row = 0; row < registryList->model()->rowCount(); ++row) {
    const auto item = registryList->model()->index(row, 0);
    if (item.data(Qt::UserRole).toString().compare(stagingRegistryRoot, Qt::CaseInsensitive) != 0) continue;
    registryList->selectionModel()->select(item, QItemSelectionModel::Select);
    selectedStagingRoot = true;
  }
  QVERIFY(selectedStagingRoot);
  QVERIFY(!registryRemove->isEnabled());

  auto* systemStaging = systemInnerTabs->widget(2)->findChild<ui::FileStagingWidget*>();
  QVERIFY(systemStaging);
  QCOMPARE(systemStaging->driveLetter(), QString::fromStdString(systemDrive));
  auto* systemStagingList = systemStaging->findChild<QListView*>(QStringLiteral("exclusionList"));
  QVERIFY(systemStagingList);
  QCOMPARE(systemStagingList->model()->rowCount(), 1);

  dataTab->setActiveInfoPage(ui::VolumeTab::InfoPage::FileStaging);
  QCOMPARE(dataTab->activeInfoPage(), ui::VolumeTab::InfoPage::FileExclusions);
  systemTab->setActiveInfoPage(ui::VolumeTab::InfoPage::RegistryExclusions);
  std::ranges::reverse(source.volumes);
  window.refresh();
  QTRY_VERIFY(refreshCompleted(window));
  QCOMPARE(source.reads, 2);
  for (int index = 0; index < tabs->count(); ++index) {
    auto* rebuilt = qobject_cast<ui::VolumeTab*>(tabs->widget(index));
    QVERIFY(rebuilt);
    const auto expectedPage =
        rebuilt->driveLetter() == QString::fromStdString(dataDrive) ? ui::VolumeTab::InfoPage::FileExclusions : ui::VolumeTab::InfoPage::RegistryExclusions;
    QCOMPARE(rebuilt->activeInfoPage(), expectedPage);
  }
}

QTEST_MAIN(ApplicationFileStagingTests)
#include "FileStagingCases.moc"
