// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
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
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "application/staging/contracts/FileStagingStore.h"
#include "application/staging/editing/FileStagingConflictPolicy.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/uwf/contracts/UwfCapability.h"
#include "cases/presentation/views/application/ApplicationUiFixture.h"
#include "cases/presentation/views/application/ApplicationUiTest.h"
#include "composition/desktop/MainWindowComposition.h"
#include "composition/desktop/ViewModelComposition.h"
#include "composition/desktop/WorkflowComposition.h"
#include "domain/uwf/model/UwfModel.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "infrastructure/windows/filesystem/paths/VolumePaths.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/models/PageProxyModel.h"
#include "presentation/viewmodels/common/PresentationTiming.h"
#include "presentation/viewmodels/staging/FileStagingResultText.h"
#include "presentation/views/configuration/exclusions/ExclusionListWidget.h"
#include "presentation/views/configuration/review/ApplyPlanDialog.h"
#include "presentation/views/configuration/volumes/VolumeTab.h"
#include "presentation/views/shell/MainWindow.h"
#include "presentation/views/staging/FileStagingWidget.h"
#include "support/app/CommitScenario.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/UwfMaintenanceFixture.h"
#include "support/ui/DesktopPlatformFixture.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/StagingControls.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/ViewMetrics.h"
#include "support/ui/WidgetInput.h"
#include "support/uwf/FakeUwfTrace.h"
#include "support/visual/InlineIconTextProbe.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

namespace {
class ObservedUwf final : public uwf::compat::Uwf {
 public:
  using Observer = std::function<void(const uwf::compat::Request&, const uwf::compat::Response&)>;
  ObservedUwf(std::shared_ptr<uwf::compat::Uwf> system, Observer observer) : m_system(std::move(system)), m_observer(std::move(observer)) {}
  uwf::compat::Capability probe(const uwf::compat::Request& request) const override { return m_system->probe(request); }
  uwf::compat::Response execute(const uwf::compat::Request& request, const uwf::compat::Context& context = {}) const override {
    auto result = m_system->execute(request, context);
    m_observer(request, result);
    return result;
  }
  uwf::compat::UwfStateSnapshot stateSnapshot() const override { return m_system->stateSnapshot(); }
  void updateState(uwf::compat::UwfStateFacts state) const override { m_system->updateState(std::move(state)); }
  void synchronize(const std::function<void()>& work) const override { m_system->synchronize(work); }
  uwf::compat::Response refreshState(const uwf::compat::Context& context = {}) const override { return m_system->refreshState(context); }

 private:
  std::shared_ptr<uwf::compat::Uwf> m_system;
  Observer m_observer;
};

}  // namespace

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
  const auto factory = composition::createViewModelFactory(std::make_shared<test::FakeUwfBackend>(), uwf::test::localDesktopPorts());
  const auto volume = configuration.addVolume(disk, true);
  ui::VolumeTab tab(volume, dialogs, factory->fileStaging(diskDrive, draft, &volume->fileConflicts()), factory);
  configuration.resetSnapshot(editableSnapshot());

  auto* innerTabs = tab.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(innerTabs);
  QCOMPARE(innerTabs->count(), 3);
  QCOMPARE(innerTabs->tabText(0), QStringLiteral("File exclusions"));
  QCOMPARE(innerTabs->tabText(1), QStringLiteral("Registry exclusions"));
  QCOMPARE(innerTabs->tabText(2), QStringLiteral("File staging"));

  auto* staging = innerTabs->widget(2)->findChild<ui::FileStagingWidget*>();
  QVERIFY(staging);
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(staging->driveLetter(), diskDrive);
  auto* addFiles = staging->commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* addDirectory = staging->commandWidget()->findChild<QAction*>(QStringLiteral("stageDirectoryAction"));
  auto* list = uwf::test::stagingEntryList(*staging);
  auto* filter = staging->commandWidget()->findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(addFiles && addDirectory && list && filter);
  QVERIFY(!staging->findChild<QWidget*>(QStringLiteral("fileStagingEnhancedModeNotice")));
  QVERIFY(addFiles->isEnabled() && addDirectory->isEnabled());
  QCOMPARE(list->model()->rowCount(), 0);

  dialogs.openedFile = filePath;
  dialogs.openedFiles = {dialogs.openedFile};
  addFiles->trigger();
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(store.writes, 0);
  QCOMPARE(draft.load().size(), 2);
  const app::FileStagingEntry currentVolumeFile{app::FileStagingKind::File, filePath};
  QVERIFY(draft.load().contains(otherVolumeEntry));
  QVERIFY(draft.load().contains(currentVolumeFile));
  QCOMPARE(list->model()->rowCount(), 1);

  // Drive-letter casing must not create a duplicate staging entry.
  QString alternateCase = filePath;
  alternateCase[0] = alternateCase[0].toLower();
  dialogs.openedFile = alternateCase;
  dialogs.openedFiles = {dialogs.openedFile};
  addFiles->trigger();
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(store.writes, 0);

  filter->setText(QStringLiteral("not-present"));
  QCOMPARE(list->model()->rowCount(), 0);
  QVERIFY(draft.load().contains(currentVolumeFile));
  filter->setText(QFileInfo(filePath).fileName());
  QCOMPARE(list->model()->rowCount(), 1);
  filter->clear();

  dialogs.selectedDirectory = QDir::toNativeSeparators(directory.path());
  addDirectory->trigger();
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(store.writes, 0);
  QCOMPARE(draft.load().size(), 2);
  const app::FileStagingEntry currentVolumeDirectory{app::FileStagingKind::Directory, dialogs.selectedDirectory};
  QVERIFY(draft.load().contains(otherVolumeEntry));
  QVERIFY(draft.load().contains(currentVolumeDirectory));

  dialogs.openedFile = filePath;
  dialogs.openedFiles = {dialogs.openedFile};
  addFiles->trigger();
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(store.writes, 0);
  QCOMPARE(list->model()->rowCount(), 1);
  list->selectionModel()->select(list->model()->index(0, 0), QItemSelectionModel::Select);
  auto* remove = buttonWithText(staging->commandWidget(), QStringLiteral("Remove selected"));
  QVERIFY(remove);
  QTest::mouseClick(remove, Qt::LeftButton);
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(store.writes, 0);
  QCOMPARE(draft.load(), QList<app::FileStagingEntry>{otherVolumeEntry});
}

void ApplicationFileStagingTests::clearingFileStagingCanBeReviewedCancelledAndRestored() {
  const auto originalClipboard = QApplication::clipboard()->text();
  const auto restoreClipboard = qScopeGuard([&] { QApplication::clipboard()->setText(originalClipboard); });
  auto storeOwner = std::make_shared<MemoryFileStagingStore>();
  auto& store = *storeOwner;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\first.bin")},
                   {app::FileStagingKind::Directory, QStringLiteral("C:\\State\\folder")}};
  const auto saved = store.entries;
  const QString root = QString::fromStdString(std::string(app::kFileStagingRegistryRoot));
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.current.registryExclusions = source.snapshot.next.registryExclusions = {root.toStdString()};
  constexpr auto systemVolume = "11111111-2222-3333-4444-555555555555";
  constexpr auto otherVolume = "66666666-7777-8888-9999-000000000000";
  for (auto* session : {&source.snapshot.current, &source.snapshot.next})
    for (auto& configured : session->volumes) configured.volumeName = compat::volumePath(configured.driveLetter == "C:" ? systemVolume : otherVolume);
  const auto previousFileExclusions = source.snapshot.next.fileExclusions.at(uwf::test::application::fixtureVolumeCName);
  source.snapshot.next.fileExclusions.clear();
  source.snapshot.next.fileExclusions.emplace(compat::volumePath(systemVolume), previousFileExclusions);
  source.volumes = {{"C:", compat::volumePath(systemVolume), "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  test::FakeUwfBackend::State initial;
  initial.current.filterEnabled = initial.next.filterEnabled = true;
  initial.current.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
  initial.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Disk);
  initial.current.maximumOverlaySize = 4096;
  initial.next.maximumOverlaySize = 8192;
  initial.current.registryExclusions = initial.next.registryExclusions = {root.toStdString()};
  initial.current.volumes.emplace(
      systemVolume, test::FakeUwfBackend::VolumeConfiguration{.volume = {systemVolume, "C:", true, true, false}, .fileExclusions = {}, .swapfileSize = 0});
  initial.current.volumes.emplace(
      otherVolume, test::FakeUwfBackend::VolumeConfiguration{.volume = {otherVolume, "D:", false, true, false}, .fileExclusions = {}, .swapfileSize = 0});
  initial.next.volumes = initial.current.volumes;
  initial.next.volumes.at(systemVolume).fileExclusions = {"\\Existing"};
  const auto backend = std::make_shared<test::FakeUwfBackend>(std::move(initial));
  std::vector<compat::Request> writes;
  bool cleanupObservedSavedEntries = false;
  const auto system = std::make_shared<ObservedUwf>(backend, [&](const compat::Request& request, const compat::Response& response) {
    if (!compat::isWrite(request)) return;
    writes.push_back(request);
    if (const auto* edit = std::get_if<compat::EditRegistryExclusion>(&request);
        edit && edit->action == compat::ExclusionAction::Remove && edit->key == root.toStdString()) {
      cleanupObservedSavedEntries = store.writes == 1 && store.entries.isEmpty();
      if (response.succeeded()) source.snapshot.next.registryExclusions = backend->snapshot().next.registryExclusions;
    }
  });
  ui::MainWindow window(uwf::composition::mainWindowServices(system, source, storeOwner, uwf::test::localDesktopPorts()),
                        {.uwfCapability = app::UwfCapability::Available, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  auto* tab = window.findChild<ui::VolumeTab*>();
  auto* staging = window.findChild<ui::FileStagingWidget*>();
  QVERIFY(tab && staging);
  QTRY_VERIFY(!staging->viewModel().busy());
  tab->setActiveInfoPage(ui::VolumeTab::InfoPage::FileStaging);
  auto* list = uwf::test::stagingEntryList(*staging);
  auto* remove = buttonWithText(staging->commandWidget(), QStringLiteral("Remove selected"));
  QVERIFY(list && remove);
  list->selectAll();
  QTest::mouseClick(remove, Qt::LeftButton);
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(store.entries, saved);
  QCOMPARE(store.writes, 0);
  QCOMPARE(list->model()->rowCount(), 2);
  for (int row = 0; row < list->model()->rowCount(); ++row)
    QCOMPARE(list->model()->index(row, 0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Removed));
  for (int attempt = 0; attempt < 2; ++attempt) {
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        window, [&] { window.showPlan(); },
        [&](QDialog& modal) {
          auto* plan = qobject_cast<ui::ApplyPlanDialog*>(&modal);
          QVERIFY(plan);
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
              for (auto titles = changes->findChildren<ui::ContentHintBox*>(); auto* title : std::as_const(titles))
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
            const auto highlight = title->palette().color(QPalette::Highlight);
            QVERIFY(test::colorCoverage(title->viewport()->grab().toImage(), highlight, title->viewport()->rect()) >
                    test::colorCoverage(before, highlight, title->viewport()->rect()));
            const auto copied = test::copySelectionToClipboard(*title, expected);
            QVERIFY2(copied, qUtf8Printable(QStringLiteral("Ctrl+C did not transfer the preview selection. Expected: %1; selected: %2; clipboard: %3")
                                                .arg(expected, title->textCursor().selectedText(), QApplication::clipboard()->text())));
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
          QVERIFY(writes.empty());
          QVERIFY(uwf::test::saveScreenshot(*plan, QStringLiteral("staging-clear-preview")));
          inspected = true;
        }));
    QVERIFY(inspected);
    QCOMPARE(store.entries, saved);
    QCOMPARE(store.writes, 0);
  }
  list->selectAll();
  QCOMPARE(remove->property("accessibleName").toString(), QStringLiteral("Undo removal"));
  QTest::mouseClick(remove, Qt::LeftButton);
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(list->model()->rowCount(), 2);
  for (int row = 0; row < list->model()->rowCount(); ++row)
    QCOMPARE(list->model()->index(row, 0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Normal));
  QCOMPARE(store.entries, saved);
  QCOMPARE(store.writes, 0);
  QVERIFY(writes.empty());

  // Recreate the same draft and accept both the preview and its confirmation.
  list->selectAll();
  QTest::mouseClick(remove, Qt::LeftButton);
  QTRY_VERIFY(!staging->viewModel().busy());
  bool confirmed = false;
  bool applied = false;
  QVERIFY(test::driveModalDialog(
      window, [&] { window.showPlan(); },
      [&](QDialog& modal) {
        auto* plan = qobject_cast<ui::ApplyPlanDialog*>(&modal);
        QVERIFY(plan);
        auto* apply = buttonWithText(plan, QStringLiteral("Apply changes"));
        QVERIFY(apply && apply->isEnabled());
        QVERIFY(test::driveModalDialog(
            *plan, [&] { QTest::mouseClick(apply, Qt::LeftButton); },
            [&](QDialog& confirmationDialog) {
              auto* confirmation = &confirmationDialog;
              QVERIFY(confirmation != plan);
              QCOMPARE(store.writes, 0);
              QCOMPARE(store.entries, saved);
              auto* accept = buttonWithText(confirmation, QStringLiteral("Apply changes"));
              QVERIFY(accept);
              confirmed = true;
              QTest::mouseClick(accept, Qt::LeftButton);
            }));
        QTRY_VERIFY(applyCompleted(*plan));
        QVERIFY(confirmed);
        QCOMPARE(store.writes, 1);
        QVERIFY(store.entries.isEmpty());
        const compat::Request cleanup = compat::EditRegistryExclusion{root.toStdString(), compat::ExclusionAction::Remove};
        QCOMPARE(writes.size(), std::size_t{1});
        QVERIFY(writes.front() == cleanup);
        QVERIFY(cleanupObservedSavedEntries);
        QVERIFY(backend->snapshot().next.registryExclusions.empty());
        applied = true;
      }));
  QVERIFY(applied);
}

void ApplicationFileStagingTests::fileStagingIndicatorTracksEnhancedModeWithoutDisablingEdits() {
  const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  staging.entries = {
      {app::FileStagingKind::File, QStringLiteral("C:\\State\\pending.bin"), app::FileCommitTarget{fixtureVolumeId, QStringLiteral("\\State\\pending.bin")}}};
  const auto originalEntries = staging.entries;
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  MemoryEnhancedModeAgent agent;
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = true;
  source.hormEnabled = false;
  source.policy.transportAvailable = true;
  source.policy.current.filterEnabled = source.policy.next.filterEnabled = {app::PolicyObservationStatus::Available, true};
  source.policy.current.overlayType = source.policy.next.overlayType = {app::PolicyObservationStatus::Available, 0};
  source.policy.current.flags = source.policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{0}};
  source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  auto ports = uwf::test::localDesktopPorts();
  ports.traversal = std::make_shared<uwf::test::commit::Tree>(false);
  ui::MainWindow window(uwf::composition::mainWindowServices(system, source, stagingOwner, std::move(ports), serviceControlOwner, &agent),
                        {.uwfCapability = uwf::app::UwfCapability::Available, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  auto* disk = window.findChild<ui::VolumeTab*>();
  auto* widget = window.findChild<ui::FileStagingWidget*>();
  auto* enhancedAction = actionWithText(&window, QStringLiteral("Enhanced mode"));
  QVERIFY(disk && widget && enhancedAction);
  QTRY_VERIFY(!widget->viewModel().busy());
  disk->setActiveInfoPage(ui::VolumeTab::InfoPage::FileStaging);
  auto* tabs = disk->findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  auto* list = uwf::test::stagingEntryList(*widget);
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
    QVERIFY(test::driveModalDialog(
        window, [&] { enhancedAction->trigger(); },
        [&](QDialog& modal) {
          auto* dialog = &modal;
          QCOMPARE(dialog->objectName(), QStringLiteral("enhancedModeDialog"));
          dialogObserved = true;
          dialog->reject();
        }));
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
  QVERIFY(uwf::test::writeRequests(*system).empty());
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
  ui::FileStagingWidget widget(uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())
                                   ->fileStaging(QStringLiteral("C:"), storeDraft),
                               dialogs);
  widget.show();
  QVERIFY(QTest::qWaitForWindowExposed(&widget));
  QTRY_VERIFY(!widget.viewModel().busy());
  const auto requested = test::boundedWindowSize(widget, windowSize);
  QVERIFY(!requested.isEmpty());
  widget.resize(requested);
  QTRY_VERIFY(test::similarSize(QRectF(QPointF(), widget.size()), QRectF(QPointF(), requested)));
  auto* list = uwf::test::stagingEntryList(widget);
  QVERIFY(list && list->isVisible() && list->isEnabled());
  QVERIFY(!widget.findChild<QWidget*>(QStringLiteral("fileStagingEnhancedModeNotice")));
  const QRect listGeometry = list->geometry();
  widget.viewModel().setAvailability({.enhancedModeAvailable = true});
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QCOMPARE(list->geometry(), listGeometry);
  widget.viewModel().setAvailability({});
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QCOMPARE(list->geometry(), listGeometry);
  auto* entries = dynamic_cast<ui::PathEntryListView*>(list);
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
  const int rowHeight = test::displayedRowHeight(list);
  QVERIFY(rowHeight > 0 && list->viewport()->height() >= rowHeight);
  const auto alignmentError = test::probeInlineIconText(widget, QStringLiteral("file-staging"), QString::fromLatin1(QTest::currentDataTag()));
  QVERIFY2(alignmentError.isEmpty(), qUtf8Printable(alignmentError));
  const auto menuError = test::probeInlineMenus(widget, QStringLiteral("file-staging"), QString::fromLatin1(QTest::currentDataTag()));
  QVERIFY2(menuError.isEmpty(), qUtf8Printable(menuError));
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
  ui::FileStagingWidget staging(
      uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())->fileStaging(drive, storeDraft),
      dialogs);
  QTRY_VERIFY(!staging.viewModel().busy());
  auto* list = uwf::test::stagingEntryList(staging);
  auto* remove = buttonWithText(staging.commandWidget(), QStringLiteral("Remove selected"));
  auto* addDirectory = staging.commandWidget()->findChild<QAction*>(QStringLiteral("stageDirectoryAction"));
  QVERIFY(list && remove && addDirectory);
  QCOMPARE(list->model()->rowCount(), 1);

  // Read-only access still permits filtering, selection and copying; all mutation entry points remain blocked.
  auto* addFiles = staging.commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* filter = staging.commandWidget()->findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(addFiles && filter);
  staging.viewModel().setAvailability({.readOnly = true});
  QVERIFY(!addFiles->isEnabled());
  QVERIFY(!addDirectory->isEnabled());
  QVERIFY(!remove->isEnabled());
  QVERIFY(list->isEnabled());
  QVERIFY(filter->isEnabled());
  dialogs.openedFile = filePath;
  dialogs.openedFiles = {dialogs.openedFile};
  addFiles->trigger();
  QCOMPARE(store.writes, 0);
  staging.viewModel().setAvailability({});
  QVERIFY(addFiles->isEnabled());
  QVERIFY(addDirectory->isEnabled());
  QVERIFY(!remove->isEnabled());

  dialogs.selectedDirectory = filePath;
  QVERIFY(test::driveModalDialog(staging, [&] { addDirectory->trigger(); }, [](QDialog& warning) { warning.reject(); }));
  QTRY_VERIFY(!staging.viewModel().busy());
  QCOMPARE(store.writes, 0);
  QCOMPARE(list->model()->rowCount(), 1);

  dialogs.openedFile = QStringLiteral("C:relative-state.bin");
  dialogs.openedFiles = {dialogs.openedFile};
  QVERIFY(test::driveModalDialog(staging, [&] { addFiles->trigger(); }, [](QDialog& warning) { warning.reject(); }));
  QTRY_VERIFY(!staging.viewModel().busy());
  QCOMPARE(store.writes, 0);
  QCOMPARE(list->model()->rowCount(), 1);

  const QString otherDrive = drive.compare(QStringLiteral("C:"), Qt::CaseInsensitive) == 0 ? QStringLiteral("D:") : QStringLiteral("C:");
  auto crossDriveStoreOwner = std::make_shared<MemoryFileStagingStore>();
  auto& crossDriveStore = *crossDriveStoreOwner;
  app::FileStagingDraft crossDriveStoreDraft(crossDriveStoreOwner);
  ui::FileStagingWidget crossDrive(uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())
                                       ->fileStaging(otherDrive, crossDriveStoreDraft),
                                   dialogs);
  QTRY_VERIFY(!crossDrive.viewModel().busy());
  auto* crossDriveAction = crossDrive.commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* crossDriveList = uwf::test::stagingEntryList(crossDrive);
  QVERIFY(crossDriveAction && crossDriveList);
  dialogs.openedFile = filePath;
  dialogs.openedFiles = {dialogs.openedFile};
  QVERIFY(test::driveModalDialog(crossDrive, [&] { crossDriveAction->trigger(); }, [](QDialog& warning) { warning.reject(); }));
  QTRY_VERIFY(!crossDrive.viewModel().busy());
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
  QTRY_VERIFY(!staging.viewModel().busy());
  QCOMPARE(store.writes, 0);
  QCOMPARE(store.entries.size(), 1);
  QCOMPARE(list->model()->rowCount(), 1);

  auto unavailableStoreOwner = std::make_shared<MemoryFileStagingStore>();

  auto& unavailableStore = *unavailableStoreOwner;
  unavailableStore.readFailure = QStringLiteral("registry read failed");
  app::FileStagingDraft unavailableStoreDraft(unavailableStoreOwner);
  ui::FileStagingWidget unavailable(uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())
                                        ->fileStaging(drive, unavailableStoreDraft),
                                    dialogs);
  QTRY_VERIFY(!unavailable.viewModel().busy());
  auto* unavailableList = uwf::test::stagingEntryList(unavailable);
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
  exclusions.viewModel().setDriveLetter(drive);
  exclusions.viewModel().setBaseline({filePath}, {filePath});

  auto emptyStoreOwner = std::make_shared<MemoryFileStagingStore>();

  auto& emptyStore = *emptyStoreOwner;
  app::FileStagingDraft emptyStoreDraft(emptyStoreOwner);
  ui::FileStagingWidget staging(uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())
                                    ->fileStaging(drive, emptyStoreDraft, &conflicts),
                                dialogs);
  QTRY_VERIFY(!staging.viewModel().busy());
  dialogs.openedFile = filePath;
  dialogs.openedFiles = {dialogs.openedFile};
  auto* addFiles = staging.commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  QVERIFY(addFiles);
  QVERIFY(test::driveModalDialog(staging, [&] { addFiles->trigger(); }, [](QDialog& warning) { warning.reject(); }));
  QTRY_VERIFY(!staging.viewModel().busy());
  QCOMPARE(emptyStore.writes, 0);
  QVERIFY(emptyStore.entries.isEmpty());

  auto stagedStoreOwner = std::make_shared<MemoryFileStagingStore>();

  auto& stagedStore = *stagedStoreOwner;
  stagedStore.entries = {{app::FileStagingKind::File, filePath}};
  app::FileStagingDraft stagedStoreDraft(stagedStoreOwner);
  ui::FileStagingWidget staged(uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())
                                   ->fileStaging(drive, stagedStoreDraft, &conflicts),
                               dialogs);
  QTRY_VERIFY(!staged.viewModel().busy());
  QVERIFY(uwf::test::stagingEntryList(staged));
  exclusions.viewModel().setBaseline({}, {});
  bool conflictShown = false;
  QVERIFY(test::driveModalDialog(
      exclusions, [&] { exclusions.addSelectedPath(filePath); },
      [&](QDialog& warning) {
        conflictShown = true;
        warning.reject();
      }));
  QVERIFY(conflictShown);
  QVERIFY(exclusions.viewModel().pendingAdded().isEmpty());
}

void ApplicationFileStagingTests::mainWindowMountsFileStagingOnlyWherePerFileCommitIsSupported() {
  const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
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

  ui::MainWindow window(uwf::composition::mainWindowServices(system, source, stagingOwner, uwf::test::localDesktopPorts()),
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
  auto* systemStagingList = uwf::test::stagingEntryList(*systemStaging);
  QVERIFY(systemStagingList);
  QCOMPARE(systemStagingList->model()->rowCount(), 2);
  for (const auto& saved : std::as_const(staging.entries))
    QVERIFY(std::ranges::any_of(systemStaging->viewModel().entries(), [&](const auto& entry) { return entry.path == saved.path; }));

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
