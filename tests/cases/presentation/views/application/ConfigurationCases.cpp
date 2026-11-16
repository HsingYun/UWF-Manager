// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTabBar>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTextEdit>
#include <QThreadPool>
#include <QTimer>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/configuration/importing/ConfigurationImporter.h"
#include "application/overlay/contracts/OverlayPolicy.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/uwf/CompositeAllVolumeProtection.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "cases/presentation/views/application/ApplicationUiFixture.h"
#include "cases/presentation/views/application/ApplicationUiTest.h"
#include "composition/desktop/MainWindowComposition.h"
#include "composition/desktop/ViewModelComposition.h"
#include "domain/uwf/contracts/Types.h"
#include "domain/uwf/model/UwfModel.h"
#include "infrastructure/windows/diagnostics/SystemHardwareInfo.h"
#include "infrastructure/windows/filesystem/paths/VolumePaths.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/feedback/ContentState.h"
#include "presentation/controls/inputs/SwitchButton.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/viewmodels/configuration/ImportViewModel.h"
#include "presentation/viewmodels/configuration/VolumeConfigurationViewModel.h"
#include "presentation/viewmodels/shell/MainWindowViewModel.h"
#include "presentation/views/configuration/exclusions/ExclusionListWidget.h"
#include "presentation/views/configuration/global/GlobalStatusPanel.h"
#include "presentation/views/configuration/review/ApplyPlanDialog.h"
#include "presentation/views/configuration/volumes/VolumeTab.h"
#include "presentation/views/shell/MainWindow.h"
#include "presentation/views/staging/FileStagingWidget.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/FileTargetFixture.h"
#include "support/app/ReadOnlyFileBrowserFixture.h"
#include "support/app/WorkerGate.h"
#include "support/ui/DesktopPlatformFixture.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/LocalFileDialogs.h"
#include "support/ui/StagingControls.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/ViewMetrics.h"
#include "support/uwf/FakeUwfBackend.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

class ApplicationConfigurationTests final : public ApplicationUiTest {
  Q_OBJECT
 private slots:
  void configurationVisualStates_data();
  void configurationVisualStates();
  void diskTabsApplyCapabilityBoundariesAndPreserveInnerSelection();
  void knownHormDisablesChangingActionsWithoutDisablingDiagnostics_data();
  void knownHormDisablesChangingActionsWithoutDisablingDiagnostics();
  void registryActionsRequireTheSystemVolumeOverlay_data();
  void registryActionsRequireTheSystemVolumeOverlay();
  void diskHeadingKeepsVolumeMetadataAsLiteralText_data();
  void diskHeadingKeepsVolumeMetadataAsLiteralText();
  void diskTabKeepsExclusionsBoundToEachSessionVolume();
  void overlayCommitScopesStayIndependentOfFactoryOrder_data();
  void overlayCommitScopesStayIndependentOfFactoryOrder();
  void importRoutingAndPendingCollectionCoverInvalidDuplicateAndMissingTargets();
  void importCommandsPreserveOrderAcrossRepeatedTargets();
  void importGuardedCommandsPreserveDependencies();
  void pendingCountAndReviewShareTheCompleteStagingTarget_data();
  void pendingCountAndReviewShareTheCompleteStagingTarget();
  void reviewRequiresAnObservedStagingTarget_data();
  void reviewRequiresAnObservedStagingTarget();
  void reviewReadFailurePreservesTheDraftAndAllowsRetry();
  void userRefreshPublishesSystemStateWhenStagingCannotBeRead_data();
  void userRefreshPublishesSystemStateWhenStagingCannotBeRead();
  void userRefreshDiscardsOrdinaryAndMaintenanceIntentsTogether();
  void offlineStagingEntriesRemainManageableAndReturnToTheirMountedPage_data();
  void offlineStagingEntriesRemainManageableAndReturnToTheirMountedPage();
  void partialApplyPreservesBulkProtectionExceptionsForRetry_data();
  void partialApplyPreservesBulkProtectionExceptionsForRetry();
  void replacingAMountedStagingAliasPersistsTheSelectedDisplayAddress();
  void applyPlanPreviewAndCopyUseTheSameProductionCommandMapping();
  void applyPlanConfirmedWritePublishesReconciliationAndPreventsReplay();
  void applyPlanConnectionFailureRemainsRetryableAndDoesNotRequestReconciliation();
  void applyPlanCloseRequestsRespectTheExecutionPhase_data();
  void applyPlanCloseRequestsRespectTheExecutionPhase();
  void applyPlanOverlayWritesRespectProviderConstraints_data();
  void applyPlanOverlayWritesRespectProviderConstraints();
  void applyPlanDependencyFailureIsLocalized_data();
  void applyPlanDependencyFailureIsLocalized();
};

using namespace uwf;
using namespace uwf::test::application;

void ApplicationConfigurationTests::overlayCommitScopesStayIndependentOfFactoryOrder_data() {
  QTest::addColumn<bool>("reverseCreation");
  QTest::addColumn<bool>("elevated");
  QTest::addColumn<bool>("currentReadOnlyMedia");
  QTest::addColumn<bool>("nextReadOnlyMedia");
  for (const bool reverse : {false, true}) {
    const auto prefix = reverse ? QStringLiteral("other-first/") : QStringLiteral("writable-first/");
    QTest::newRow(qPrintable(prefix + QStringLiteral("privileges"))) << reverse << false << false << false;
    QTest::newRow(qPrintable(prefix + QStringLiteral("current-media"))) << reverse << true << true << false;
    QTest::newRow(qPrintable(prefix + QStringLiteral("next-media"))) << reverse << true << false << true;
    QTest::newRow(qPrintable(prefix + QStringLiteral("current-and-next-media"))) << reverse << true << true << true;
  }
}

void ApplicationConfigurationTests::overlayCommitScopesStayIndependentOfFactoryOrder() {
  QFETCH(bool, reverseCreation);
  QFETCH(bool, elevated);
  QFETCH(bool, currentReadOnlyMedia);
  QFETCH(bool, nextReadOnlyMedia);
  const bool otherCommitsAllowed = elevated && !currentReadOnlyMedia;
  const auto originalLanguage = ui::I18n::instance().lang();
  const auto restoreLanguage = qScopeGuard([&] { ui::I18n::instance().setLang(originalLanguage); });
  ui::I18n::instance().setLang(ui::I18n::Lang::En);
  MutableApplicationStateSource writableSource, restrictedSource;
  writableSource.snapshot = editableSnapshot();
  writableSource.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  restrictedSource.snapshot = writableSource.snapshot;
  restrictedSource.snapshot.elevated = elevated;
  restrictedSource.volumes = writableSource.volumes;
  restrictedSource.policy.transportAvailable = true;
  restrictedSource.policy.current.filterEnabled = restrictedSource.policy.next.filterEnabled = {app::PolicyObservationStatus::Available, true};
  restrictedSource.policy.current.overlayType = restrictedSource.policy.next.overlayType = {app::PolicyObservationStatus::Available, 0};
  restrictedSource.policy.current.flags = {app::PolicyObservationStatus::Available,
                                           app::OverlayFlags{currentReadOnlyMedia ? app::OverlayFlags::ReadOnlyMedia : 0u}};
  restrictedSource.policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{nextReadOnlyMedia ? app::OverlayFlags::ReadOnlyMedia : 0u}};
  const QString path = QStringLiteral("C:\\Data\\state.bin");
  const auto factory = composition::createViewModelFactory(
      std::make_shared<test::FakeUwfBackend>(), uwf::test::localDesktopPorts(),
      [path](const QString&, std::stop_token) { return QVector<app::OverlayFileEntry>{{QStringLiteral("\\Data\\state.bin"), path}}; });
  std::unique_ptr<presentation::MainWindowViewModel> writable, restricted;
  const auto createWritable = [&] { writable = factory->mainWindow(writableSource, app::UwfCapability::Available, {}); };
  const auto createRestricted = [&] { restricted = factory->mainWindow(restrictedSource, app::UwfCapability::Available, {}); };
  if (reverseCreation) {
    createRestricted();
    createWritable();
  } else {
    createWritable();
    createRestricted();
  }
  for (auto* window : {writable.get(), restricted.get()}) {
    window->initializeConfiguration(16384, QStringLiteral("C:"));
    window->refresh();
  }
  QTRY_VERIFY(!writable->refreshing() && !restricted->refreshing());
  const auto writableVolume = writable->configuration()->volume(QStringLiteral("C:"));
  const auto restrictedVolume = restricted->configuration()->volume(QStringLiteral("C:"));
  auto writableFiles = factory->overlayFiles(writableVolume, writable.get());
  auto restrictedFiles = factory->overlayFiles(restrictedVolume, restricted.get());
  writableFiles->startLoading();
  restrictedFiles->startLoading();
  QTRY_VERIFY(writableFiles->loaded() && restrictedFiles->loaded());
  QSignalSpy requests(writableFiles.get(), &presentation::OverlayFilesViewModel::commitFileRequested);
  QSignalSpy otherRequests(restrictedFiles.get(), &presentation::OverlayFilesViewModel::commitFileRequested);
  {
    ui::VolumeTab tab(restrictedVolume, test::localFileDialogs(), nullptr, factory, nullptr, restricted.get());
    const auto commit = tab.findChild<QPushButton*>(QStringLiteral("volumeCommitButton"));
    QVERIFY(commit);
    for (const auto language : {ui::I18n::Lang::En, ui::I18n::Lang::Zh_CN, ui::I18n::Lang::En}) {
      ui::I18n::instance().setLang(language);
      QVERIFY(writableFiles->canCommit(path));
      QVERIFY(writableFiles->commitRestriction(path).isEmpty());
      QCOMPARE(restrictedFiles->canCommit(path), otherCommitsAllowed);
      const auto expectedReason = !elevated              ? ui::I18n::tr("Administrator privileges are required; the overlay configuration is read-only.")
                                  : currentReadOnlyMedia ? ui::I18n::tr("File and registry commits are unavailable while Read-Only Media mode is active.")
                                                         : QString{};
      QCOMPARE(restrictedFiles->commitRestriction(path), expectedReason);
      QCOMPARE(commit->isEnabled(), otherCommitsAllowed);
      const auto expectedHint = !elevated              ? ui::I18n::tr("Administrator privileges are required; the overlay configuration is read-only.")
                                : currentReadOnlyMedia ? ui::I18n::tr("File and registry commits are unavailable while Read-Only Media mode is active.")
                                                       : ui::I18n::tr("Commit changes from the overlay to disk or the registry. This cannot be undone.");
      QCOMPARE(commit->toolTip(), ui::I18n::tr("Review available commit operations and their requirements."));
      for (const auto* source : {"Commit file changes…", "Delete and commit file…", "Commit registry changes…", "Delete and commit registry…"}) {
        const auto action = actionWithText(&tab, ui::I18n::tr(source));
        QVERIFY(action);
        QCOMPARE(action->isEnabled(), otherCommitsAllowed);
        QCOMPARE(action->toolTip(), expectedHint);
      }
      QCOMPARE(restrictedVolume->commitAvailability().overlay, elevated);
    }
    restrictedFiles->requestCommit(path);
    QCOMPARE(otherRequests.count(), otherCommitsAllowed ? 1 : 0);
  }
  // MainWindow rebuilds volume pages after stateCommitted; use the refreshed scope.
  restrictedSource.snapshot.elevated = true;
  restrictedSource.policy.current.flags.value = app::OverlayFlags{};
  restrictedSource.policy.next.flags.value = app::OverlayFlags{app::OverlayFlags::ReadOnlyMedia};
  restricted->refresh();
  QTRY_VERIFY(!restricted->refreshing());
  QVERIFY(!restrictedFiles->canCommit(path));  // The replaced volume editor is retired with its command scope.
  QVERIFY(writableFiles->canCommit(path));
  const auto restoredVolume = restricted->configuration()->volume(QStringLiteral("C:"));
  auto restoredFiles = factory->overlayFiles(restoredVolume, restricted.get());
  restoredFiles->startLoading();
  QTRY_VERIFY(restoredFiles->loaded());
  {
    ui::VolumeTab tab(restoredVolume, test::localFileDialogs(), nullptr, factory, nullptr, restricted.get());
    const auto commit = tab.findChild<QPushButton*>(QStringLiteral("volumeCommitButton"));
    QVERIFY(commit);
    for (const auto language : {ui::I18n::Lang::En, ui::I18n::Lang::Zh_CN, ui::I18n::Lang::En}) {
      ui::I18n::instance().setLang(language);
      QVERIFY(restoredFiles->canCommit(path));
      QVERIFY(restoredFiles->commitRestriction(path).isEmpty());
      QVERIFY(commit->isEnabled());
      QCOMPARE(commit->toolTip(), ui::I18n::tr("Review available commit operations and their requirements."));
    }
  }
  writableFiles->requestCommit(path);
  QCOMPARE(requests.count(), 1);
  restricted.reset();
  QVERIFY(!restrictedFiles->canCommit(path));
  QVERIFY(!restoredFiles->canCommit(path));
  QVERIFY(writableFiles->canCommit(path));
  writable->setExternalOperationActive(true);
  QVERIFY(!writableFiles->canCommit(path));
  QCOMPARE(writableFiles->commitRestriction(path), ui::I18n::tr("Please wait for the operation to finish."));
  writableFiles->requestCommit(path);
  QCOMPARE(requests.count(), 1);
  writable->setExternalOperationActive(false);
  QVERIFY(writableFiles->canCommit(path));
  writable.reset();
  QVERIFY(!writableFiles->canCommit(path));
}

namespace {
enum class OverlayWriteOutcome { Confirmed, Rejected, Uncertain, LostAcknowledgement, Unconfirmed };

// Vary only the first command's acknowledgement to isolate application dependency handling.
class ObservedOverlaySystem final : public compat::Uwf {
 public:
  std::shared_ptr<test::FakeUwfBackend> backend;
  OverlayWriteOutcome outcome = OverlayWriteOutcome::Confirmed;
  mutable std::vector<compat::Request> writes;
  mutable int reads = 0;

  compat::Capability probe(const compat::Request& request) const override { return backend->probe(request); }
  compat::UwfStateSnapshot stateSnapshot() const override { return backend->stateSnapshot(); }
  void updateState(compat::UwfStateFacts state) const override { backend->updateState(std::move(state)); }
  compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
    if (!compat::isWrite(request)) {
      ++reads;
      return backend->execute(request, context);
    }
    writes.push_back(request);
    if (writes.size() == 1) {
      switch (outcome) {
        case OverlayWriteOutcome::Rejected:
          return compat::Response::fail(compat::ErrorKind::AccessDenied, "The provider rejected the command");
        case OverlayWriteOutcome::Uncertain:
          return compat::Response::fail(compat::ErrorKind::Unavailable, "The command acknowledgement is unavailable", compat::Effect::Uncertain);
        case OverlayWriteOutcome::LostAcknowledgement: {
          auto response = backend->execute(request, context);
          if (response.failure) return response;
          return compat::Response::fail(compat::ErrorKind::Unavailable, "The acknowledgement was lost after the command was applied",
                                        compat::Effect::Uncertain);
        }
        case OverlayWriteOutcome::Unconfirmed:
          return compat::Response::fail(compat::ErrorKind::Protocol, "The accepted command could not be confirmed", compat::Effect::Uncertain);
        case OverlayWriteOutcome::Confirmed:
          break;
      }
    }
    return backend->execute(request, context);
  }
};

test::FakeUwfBackend::State overlayState(const core::OverlayConfig& overlay) {
  test::FakeUwfBackend::State state;
  state.current.overlayType = state.next.overlayType = static_cast<std::uint32_t>(overlay.type);
  state.current.maximumOverlaySize = state.next.maximumOverlaySize = overlay.maximumSizeMb;
  state.current.warningThreshold = state.next.warningThreshold = overlay.warningThresholdMb;
  state.current.criticalThreshold = state.next.criticalThreshold = overlay.criticalThresholdMb;
  return state;
}

core::OverlayConfig overlayConfiguration(const test::FakeUwfBackend::State& state) {
  return {static_cast<core::OverlayType>(state.next.overlayType), state.next.maximumOverlaySize, state.current.warningThreshold,
          state.current.criticalThreshold};
}

compat::SetNumber overlayWrite(const compat::NumberProperty property, const core::OverlayConfig& target) {
  switch (property) {
    case compat::NumberProperty::OverlayType:
      return {property, static_cast<std::uint32_t>(target.type)};
    case compat::NumberProperty::MaximumOverlaySize:
      return {property, target.maximumSizeMb};
    case compat::NumberProperty::WarningThreshold:
      return {property, target.warningThresholdMb};
    case compat::NumberProperty::CriticalThreshold:
      return {property, target.criticalThresholdMb};
    default:
      throw std::logic_error("Unexpected configuration property");
  }
}

void updateOverlay(core::OverlayConfig& overlay, const compat::SetNumber& request) {
  switch (request.property) {
    case compat::NumberProperty::OverlayType:
      overlay.type = static_cast<core::OverlayType>(request.value);
      break;
    case compat::NumberProperty::MaximumOverlaySize:
      overlay.maximumSizeMb = request.value;
      break;
    case compat::NumberProperty::WarningThreshold:
      overlay.warningThresholdMb = request.value;
      break;
    case compat::NumberProperty::CriticalThreshold:
      overlay.criticalThresholdMb = request.value;
      break;
    default:
      throw std::logic_error("Unexpected configuration property");
  }
}
}  // namespace

void ApplicationConfigurationTests::diskTabsApplyCapabilityBoundariesAndPreserveInnerSelection() {
  const core::VolumeInfo supported{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  const core::VolumeInfo limited{"D:", uwf::test::application::fixtureVolumeDName, "exFAT", "Data", 1000, 500, core::VolumeSupport::FileSystemLimited};
  const core::VolumeInfo unsupported{"E:", "Volume{e}", "NTFS", "USB", 1000, 500, core::VolumeSupport::NotFixedLocalDisk};
  ui::VolumeTab c(std::make_shared<presentation::VolumeConfigurationViewModel>(supported, true), uwf::test::localFileDialogs(), nullptr,
                  uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts()));
  ui::VolumeTab d(std::make_shared<presentation::VolumeConfigurationViewModel>(limited, false), uwf::test::localFileDialogs(), nullptr,
                  uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts()));
  ui::VolumeTab e(std::make_shared<presentation::VolumeConfigurationViewModel>(unsupported, false), uwf::test::localFileDialogs(), nullptr,
                  uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts()));
  const auto snapshot = editableSnapshot();
  c.viewModel().applySnapshot(snapshot);
  d.viewModel().applySnapshot(snapshot);
  e.viewModel().applySnapshot(snapshot);

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
  c.setActiveInfoPage(ui::VolumeTab::InfoPage::RegistryExclusions);
  QCOMPARE(c.activeInfoPage(), ui::VolumeTab::InfoPage::RegistryExclusions);
  c.setActiveInfoPage(ui::VolumeTab::InfoPage::FileStaging);
  QCOMPARE(c.activeInfoPage(), ui::VolumeTab::InfoPage::RegistryExclusions);

  FileDialogStub dialogs;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  stagingOwner->entries = {{app::FileStagingKind::File, QStringLiteral("C:\\Data\\saved.bin")}};
  app::FileStagingDraft stagingDraft(stagingOwner);
  const auto factory = composition::createViewModelFactory(std::make_shared<test::FakeUwfBackend>(), uwf::test::localDesktopPorts());
  auto supportedModel = std::make_shared<presentation::VolumeConfigurationViewModel>(supported, false);
  ui::VolumeTab stagedSupported(supportedModel, dialogs, factory->fileStaging(QStringLiteral("C:"), stagingDraft, &supportedModel->fileConflicts()), factory);
  ui::VolumeTab stagedLimited(std::make_shared<presentation::VolumeConfigurationViewModel>(limited, false), dialogs, nullptr, factory);
  ui::VolumeTab stagedUnsupported(std::make_shared<presentation::VolumeConfigurationViewModel>(unsupported, false), dialogs, nullptr, factory);
  QCOMPARE(stagedSupported.findChildren<ui::FileStagingWidget*>().size(), 1);
  QCOMPARE(stagedLimited.findChildren<ui::FileStagingWidget*>().size(), 0);
  QCOMPARE(stagedUnsupported.findChildren<ui::FileStagingWidget*>().size(), 0);
  auto* stagingWidget = stagedSupported.findChild<ui::FileStagingWidget*>();
  auto* stagingTabs = stagedSupported.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(stagingWidget && stagingTabs);
  const int stagingIndex = stagingTabs->count() - 1;
  auto& staging = stagingWidget->viewModel();
  QTRY_VERIFY(!staging.busy());
  const auto indicators = [&] { return stagingTabs->tabBar()->property("designTabIndicators").toMap(); };
  const auto indicatorKey = QString::number(stagingIndex);
  using D = app::AutomaticCommitDisposition;
  app::AutomaticCommitSummary automatic;
  automatic.include(D::SkipRetainedOverlay);
  staging.setAvailability({.enhancedModeAvailable = true, .automaticCommit = automatic});
  QCOMPARE(stagingTabs->tabToolTip(stagingIndex), staging.automaticCommitPresentation().description);
  QVERIFY(indicators().contains(indicatorKey));
  QCOMPARE(staging.index(0).data(presentation::StatusTextRole).toString(), ui::I18n::tr("Paused"));
  automatic.include(D::CheckDurability);
  automatic.include(D::Unavailable);
  staging.setAvailability({.enhancedModeAvailable = true, .automaticCommit = automatic});
  QCOMPARE(stagingTabs->tabToolTip(stagingIndex), staging.automaticCommitPresentation().description);
  QVERIFY(stagingTabs->tabToolTip(stagingIndex).contains(presentation::automaticCommitDescription(D::SkipRetainedOverlay)));
  QVERIFY(stagingTabs->tabToolTip(stagingIndex).contains(presentation::automaticCommitDescription(D::Unavailable)));
  automatic = {};
  automatic.include(D::AlreadyPersistent);
  staging.setAvailability({.enhancedModeAvailable = true, .automaticCommit = automatic});
  QCOMPARE(stagingTabs->tabToolTip(stagingIndex), staging.automaticCommitPresentation().description);
  QVERIFY(!indicators().contains(indicatorKey));
  QCOMPARE(staging.index(0).data(presentation::StatusTextRole).toString(), ui::I18n::tr("No files to commit"));
  staging.setAvailability({.enhancedModeAvailable = true, .automaticCommit = {}});
  QVERIFY(!indicators().contains(indicatorKey));
  QCOMPARE(stagingTabs->tabToolTip(stagingIndex), staging.automaticCommitPresentation().description);
  QVERIFY(!stagingTabs->tabToolTip(stagingIndex).contains(presentation::automaticCommitDescription(D::SkipRetainedOverlay)));
  QCOMPARE(staging.index(0).data(presentation::StatusTextRole).toString(), ui::I18n::tr("Ready"));
  QVERIFY(!presentation::volumeSupportText(core::VolumeSupport::FileSystemLimited, "exFAT").isEmpty());
  QVERIFY(!presentation::volumeSupportText(core::VolumeSupport::ExceedsMaxSize, "NTFS").isEmpty());
}

void ApplicationConfigurationTests::knownHormDisablesChangingActionsWithoutDisablingDiagnostics_data() {
  QTest::addColumn<bool>("hormKnown");
  QTest::addColumn<bool>("hormEnabled");
  QTest::addColumn<bool>("flagsKnown");
  for (const auto& [name, horm] :
       {std::pair{"unknown", std::optional<bool>{}}, std::pair{"off", std::optional<bool>{false}}, std::pair{"on", std::optional<bool>{true}}})
    for (const bool flagsKnown : {false, true})
      QTest::newRow(qPrintable(QStringLiteral("horm-%1/flags-known-%2").arg(QString::fromLatin1(name)).arg(flagsKnown)))
          << horm.has_value() << horm.value_or(false) << flagsKnown;
}
void ApplicationConfigurationTests::knownHormDisablesChangingActionsWithoutDisablingDiagnostics() {
  QFETCH(bool, hormKnown);
  QFETCH(bool, hormEnabled);
  QFETCH(bool, flagsKnown);
  const auto horm = hormKnown ? std::optional{hormEnabled} : std::nullopt;
  const bool allowed = !hormKnown || !hormEnabled;
  const bool configurationAllowed = allowed && !flagsKnown;
  const bool commitsAllowed = allowed && !flagsKnown;
  const auto files = std::make_shared<ReadOnlyFileBrowser>();
  const auto selected = QStringLiteral("C:\\Ready.txt");
  files->entries[selected] = {.path = selected, .name = QStringLiteral("Ready.txt"), .directory = false, .size = 1, .modified = {}, .exists = true};
  presentation::ConfigurationViewModel configuration(16384, {}, QStringLiteral("C:"), {}, files);
  const auto volume =
      configuration.addVolume({"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}, true);
  const auto factory = composition::createViewModelFactory(std::make_shared<test::FakeUwfBackend>(), uwf::test::localDesktopPorts());
  ui::VolumeTab tab(volume, test::localFileDialogs(), nullptr, factory);
  auto state = editableSnapshot();
  state.current.filter.enabled = true;
  state.current.overlay.type = state.next.overlay.type = core::OverlayType::RAM;
  state.current.volumes = state.next.volumes = {{uwf::test::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::VolumeId}};
  state.next.fileExclusions = {{uwf::test::application::fixtureVolumeCName, {"\\Ready.txt"}}};
  app::OverlayPolicySnapshot policy;
  policy.transportAvailable = true;
  policy.current.filterEnabled = policy.next.filterEnabled = {app::PolicyObservationStatus::Available, true};
  policy.current.overlayType = policy.next.overlayType = {app::PolicyObservationStatus::Available, 0};
  policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{app::OverlayFlags::ReadOnlyMedia}};
  policy.current.flags = flagsKnown ? policy.next.flags : app::PolicyObservation<app::OverlayFlags>{app::PolicyObservationStatus::Failed};
  configuration.resetSnapshot(state, policy, horm);
  for (const auto& label : {QStringLiteral("Commit file changes…"), QStringLiteral("Delete and commit file…"), QStringLiteral("Commit registry changes…"),
                            QStringLiteral("Delete and commit registry…")}) {
    const auto action = actionWithText(&tab, label);
    QVERIFY(action);
    QCOMPARE(action->isEnabled(), commitsAllowed);
  }
  const auto protect = tab.findChild<ui::SwitchButton*>(QStringLiteral("volumeProtectionNext"));
  QVERIFY(protect);
  QCOMPARE(protect->isEnabled(), configurationAllowed);
  QCOMPARE(configuration.global()->controlsEnabled(), allowed);
  QCOMPARE(volume->files()->commitEnabled(), commitsAllowed);
  QCOMPARE(volume->files()->readOnly(), !configurationAllowed);
  QCOMPARE(volume->registry()->readOnly(), !configurationAllowed);
  QTRY_COMPARE(volume->files()->fileCommitState(selected).enabled, commitsAllowed);
  if (commitsAllowed) QCOMPARE(volume->files()->fileCommitState(selected).kind, app::ExclusionEditor::FileEntryKind::File);
  QVERIFY(volume->commitAvailability().overlay);
  if (!commitsAllowed) {
    const auto commit = tab.findChild<QPushButton*>(QStringLiteral("volumeCommitButton"));
    QVERIFY(commit);
    QVERIFY(!commit->isEnabled());
    const auto expectedHint = !allowed ? ui::I18n::tr("File and registry commits are unavailable while HORM is active.")
                                       : ui::I18n::tr("File and registry commits are unavailable while Read-Only Media mode is active.");
    for (const auto* source : {"Commit file changes…", "Delete and commit file…", "Commit registry changes…", "Delete and commit registry…"}) {
      const auto action = actionWithText(&tab, ui::I18n::tr(source));
      QVERIFY(action && !action->isEnabled());
      QCOMPARE(action->toolTip(), expectedHint);
    }
  }
}

void ApplicationConfigurationTests::registryActionsRequireTheSystemVolumeOverlay_data() {
  QTest::addColumn<bool>("systemProtected");
  QTest::addColumn<bool>("dataProtected");
  for (const bool system : {false, true})
    for (const bool data : {false, true}) {
      const auto name = QStringLiteral("system-%1/data-%2").arg(system).arg(data);
      QTest::newRow(qPrintable(name)) << system << data;
    }
}
void ApplicationConfigurationTests::registryActionsRequireTheSystemVolumeOverlay() {
  QFETCH(bool, systemProtected);
  QFETCH(bool, dataProtected);
  const auto system = std::make_shared<presentation::VolumeConfigurationViewModel>(
      core::VolumeInfo{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}, true);
  const auto data = std::make_shared<presentation::VolumeConfigurationViewModel>(
      core::VolumeInfo{"D:", uwf::test::application::fixtureVolumeDName, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported}, false);
  const auto factory = uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts());
  ui::VolumeTab systemTab(system, uwf::test::localFileDialogs(), nullptr, factory);
  ui::VolumeTab dataTab(data, uwf::test::localFileDialogs(), nullptr, factory);
  auto state = editableSnapshot();
  state.current.filter.enabled = true;
  state.current.volumes = {{uwf::test::application::fixtureVolumeCName, "C:", systemProtected, core::VolumeBinding::VolumeId},
                           {uwf::test::application::fixtureVolumeDName, "D:", dataProtected, core::VolumeBinding::VolumeId}};
  state.next.volumes = {{uwf::test::application::fixtureVolumeCName, "C:", !systemProtected, core::VolumeBinding::VolumeId},
                        {uwf::test::application::fixtureVolumeDName, "D:", !dataProtected, core::VolumeBinding::VolumeId}};
  system->applySnapshot(state);
  data->applySnapshot(state);
  for (const auto& label : {QStringLiteral("Commit registry changes…"), QStringLiteral("Delete and commit registry…")}) {
    const auto action = actionWithText(&systemTab, label);
    QVERIFY(action);
    QCOMPARE(action->isEnabled(), systemProtected);
    QVERIFY(!actionWithText(&dataTab, label));
    if (!systemProtected)
      QCOMPARE(action->toolTip(), ui::I18n::tr("The system volume is not currently protected by UWF; there are no registry changes to commit."));
  }
  const auto dataCommit = actionWithText(&dataTab, QStringLiteral("Commit file changes…"));
  QVERIFY(dataCommit);
  QCOMPARE(dataCommit->isEnabled(), dataProtected);
  if (!systemProtected) {
    const auto commit = systemTab.findChild<QPushButton*>(QStringLiteral("volumeCommitButton"));
    QVERIFY(commit);
    QVERIFY(!commit->isEnabled());
  }
}

void ApplicationConfigurationTests::diskHeadingKeepsVolumeMetadataAsLiteralText_data() {
  QTest::addColumn<QString>("label");
  QTest::newRow("empty") << QString{};
  QTest::newRow("entity") << QStringLiteral("Work &copy; Archive");
  QTest::newRow("markup") << QStringLiteral("<b>Data & logs</b>");
  QTest::newRow("unicode") << QStringLiteral("资料 &amp; Данные");
}

void ApplicationConfigurationTests::diskHeadingKeepsVolumeMetadataAsLiteralText() {
  QFETCH(QString, label);
  const core::VolumeInfo volume{"C:", "Volume{c&copy;}", "NTFS &amp; metadata", label.toStdString(), 1000, 500, core::VolumeSupport::Supported};
  ui::VolumeTab tab(std::make_shared<presentation::VolumeConfigurationViewModel>(volume, false), uwf::test::localFileDialogs(), nullptr,
                    uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts()));
  auto* heading = tab.findChild<QLabel*>(QStringLiteral("diskHeading"));
  QVERIFY(heading);
  QCOMPARE(heading->textFormat(), Qt::PlainText);
  QCOMPARE(heading->text(), QStringLiteral("C:"));
  const auto* volumeLabel = tab.findChild<QLabel*>(QStringLiteral("diskVolumeLabel"));
  QVERIFY(volumeLabel);
  QCOMPARE(volumeLabel->textFormat(), Qt::PlainText);
  QCOMPARE(volumeLabel->text(), label);
  const auto labels = tab.findChildren<QLabel*>();
  QVERIFY(std::ranges::any_of(
      labels, [](const QLabel* text) { return text->textFormat() == Qt::PlainText && text->text().contains(QStringLiteral("NTFS &amp; metadata")); }));
  QCOMPARE(heading->toolTip(), QStringLiteral("Volume{c&copy;}"));
  auto* overlay = buttonWithText(&tab, ui::I18n::tr("View overlay"));
  auto* commit = tab.findChild<QPushButton*>(QStringLiteral("volumeCommitButton"));
  QVERIFY(overlay && commit);
  for (const int width : {600, 900}) {
    tab.resize(test::boundedWindowSize(tab, QSize(width, 420)));
    tab.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tab));
    QVERIFY(test::saveScreenshot(tab, QStringLiteral("volume-heading-%1").arg(width)));
    // Metadata owns the available width. Its content must never center the
    // drive or stretch the action buttons when the optional label disappears.
    QTRY_VERIFY(std::abs(heading->mapTo(&tab, QPoint()).x()) <= heading->height() * test::kVisualAlignmentRatio);
    for (auto* button : {overlay, commit}) {
      QTRY_VERIFY(button->width() >= button->minimumSizeHint().width());
      QTRY_VERIFY(std::abs(button->width() - button->sizeHint().width()) <= button->sizeHint().width() * test::kVisualAlignmentRatio);
    }
    QTRY_VERIFY(std::abs(commit->mapTo(&tab, QPoint(commit->width(), 0)).x() - tab.width()) <= commit->height() * test::kVisualAlignmentRatio);
  }
}

void ApplicationConfigurationTests::diskTabKeepsExclusionsBoundToEachSessionVolume() {
  const core::VolumeInfo disk{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  ui::VolumeTab tab(std::make_shared<presentation::VolumeConfigurationViewModel>(disk, false), uwf::test::localFileDialogs(), nullptr,
                    uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts()));
  auto snapshot = editableSnapshot();
  snapshot.current.volumes = {{test::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::VolumeId, std::string("C:")},
                              {test::application::fixtureVolumeDName, "C:", true, core::VolumeBinding::DriveLetter, std::string("D:")}};
  snapshot.next.volumes = {{test::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::DriveLetter, std::string("C:")},
                           {test::application::fixtureVolumeDName, "C:", true, core::VolumeBinding::DriveLetter, std::string("D:")}};
  snapshot.current.fileExclusions = {{test::application::fixtureVolumeCName, {R"(\CurrentOnly)"}},
                                     {test::application::fixtureVolumeDName, {R"(\WrongCurrent)"}}};
  snapshot.next.fileExclusions = {{test::application::fixtureVolumeCName, {R"(\NextOnly)"}}, {test::application::fixtureVolumeDName, {R"(\WrongNext)"}}};
  tab.viewModel().applySnapshot(snapshot);
  tab.resize(test::boundedWindowSize(tab, QSize(700, 450)));
  tab.show();
  QVERIFY(QTest::qWaitForWindowExposed(&tab));
  auto* tabs = tab.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(tabs);
  auto* files = tabs->widget(0)->findChild<ui::ExclusionListWidget*>();
  QVERIFY(files);
  auto* list = files->findChild<QListView*>(QStringLiteral("exclusionList"));
  QVERIFY(list);
  QCOMPARE(list->model()->rowCount(), 2);
  bool currentObserved = false;
  bool nextObserved = false;
  for (int index = 0; index < list->model()->rowCount(); ++index) {
    const auto item = list->model()->index(index, 0);
    const auto path = item.data(Qt::UserRole).toString();
    if (path == QStringLiteral(R"(C:\CurrentOnly)")) {
      currentObserved = true;
      QVERIFY(item.data(Qt::ToolTipRole).toString().contains(QStringLiteral("Current session: Excluded\nNext session: Not excluded")));
    } else if (path == QStringLiteral(R"(C:\NextOnly)")) {
      nextObserved = true;
      QVERIFY(item.data(Qt::ToolTipRole).toString().contains(QStringLiteral("Current session: Not excluded\nNext session: Excluded")));
    } else {
      QFAIL("the displayed exclusions came from the wrong session volume");
    }
  }
  QVERIFY(currentObserved);
  QVERIFY(nextObserved);
  QVERIFY(files->viewModel().pendingAdded().isEmpty());
  QVERIFY(files->viewModel().pendingRemoved().isEmpty());
  const auto labels = tab.findChildren<QLabel*>();
  QVERIFY(std::ranges::any_of(labels, [](const QLabel* label) {
    const auto id = QString::fromStdString(compat::normalizeVolumeId(test::application::fixtureVolumeCName));
    return label->text().contains(id) || label->toolTip().contains(id);
  }));

  const auto nextVolumes = snapshot.next.volumes;
  snapshot.next.volumes.clear();
  tab.viewModel().applySnapshot(snapshot);
  QCOMPARE(list->model()->rowCount(), 1);
  QCOMPARE(list->model()->index(0, 0).data(Qt::UserRole).toString(), QStringLiteral(R"(C:\CurrentOnly)"));

  snapshot.next.volumes = nextVolumes;
  snapshot.current.volumes.clear();
  tab.viewModel().applySnapshot(snapshot);
  QCOMPARE(list->model()->rowCount(), 1);
  QCOMPARE(list->model()->index(0, 0).data(Qt::UserRole).toString(), QStringLiteral(R"(C:\NextOnly)"));

  snapshot.next.volumes.clear();
  tab.viewModel().applySnapshot(snapshot);
  QCOMPARE(list->model()->rowCount(), 0);
  QVERIFY(files->viewModel().pendingAdded().isEmpty());
  QVERIFY(files->viewModel().pendingRemoved().isEmpty());
}

void ApplicationConfigurationTests::importRoutingAndPendingCollectionCoverInvalidDuplicateAndMissingTargets() {
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  const auto snapshot = editableSnapshot();
  configuration.resetSnapshot(snapshot);
  const core::VolumeInfo cInfo{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  const core::VolumeInfo dInfo{"D:", uwf::test::application::fixtureVolumeDName, "exFAT", "Data", 1000, 500, core::VolumeSupport::FileSystemLimited};
  auto cOwner =
      std::make_unique<ui::VolumeTab>(configuration.addVolume(cInfo, true), uwf::test::localFileDialogs(), nullptr,
                                      uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts()));
  auto dOwner =
      std::make_unique<ui::VolumeTab>(configuration.addVolume(dInfo, false), uwf::test::localFileDialogs(), nullptr,
                                      uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts()));
  configuration.resetSnapshot(snapshot);

  const auto parsed = app::parseUwfmgrText(
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
  QList<app::UwfmgrCommand> commands;
  for (const auto& command : parsed) commands.append(command);
  const auto report = configuration.importCommands(commands);
  QCOMPARE(report.size(), commands.size());
  QCOMPARE(report[0].status, app::ImportResult::Status::Success);
  QCOMPARE(report[1].status, app::ImportResult::Status::Duplicate);
  QCOMPARE(report[4].status, app::ImportResult::Status::Failed);
  QCOMPARE(report[6].status, app::ImportResult::Status::Failed);
  QCOMPARE(report[7].status, app::ImportResult::Status::Failed);
  QCOMPARE(report[9].status, app::ImportResult::Status::Failed);
  QCOMPARE(report[10].status, app::ImportResult::Status::Failed);
  QCOMPARE(report[11].status, app::ImportResult::Status::Unsupported);

  const auto pending = configuration.pendingChanges();
  QCOMPARE(pending.setFilterEnabled, std::optional<bool>(false));
  QCOMPARE(pending.volumeProtect.at(compat::normalizeVolumeId(uwf::test::application::fixtureVolumeCName)), false);
  QCOMPARE(pending.addFileExclusions.at(compat::normalizeVolumeId(uwf::test::application::fixtureVolumeCName)).front(), std::string("C:\\Cache"));
  QCOMPARE(pending.addRegistryExclusions.front(), std::string("HKLM\\Software\\Vendor"));
  QVERIFY(!pending.addFileExclusions.contains(compat::normalizeVolumeId(uwf::test::application::fixtureVolumeDName)));

  const auto withoutGlobal = app::applyImportCommands(commands.mid(0, 1), nullptr, {}, nullptr, drive::fromPath);
  QCOMPARE(withoutGlobal.front().status, app::ImportResult::Status::Failed);
}

void ApplicationConfigurationTests::importCommandsPreserveOrderAcrossRepeatedTargets() {
  auto snapshot = editableSnapshot();
  snapshot.current.filter.enabled = false;
  snapshot.next.overlay.type = core::OverlayType::Disk;
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  configuration.resetSnapshot(snapshot);
  const core::VolumeInfo disk{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  ui::VolumeTab tab(configuration.addVolume(disk, true), uwf::test::localFileDialogs(), nullptr,
                    uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts()));
  configuration.resetSnapshot(snapshot);
  const auto parsed = app::parseUwfmgrText(
      "filter disable\nfilter enable\nfilter disable\n"
      "overlay set-size 10240\noverlay set-size 12288\noverlay set-size 10240\n"
      "volume unprotect C:\nvolume protect C:\nvolume unprotect C:\n"
      "file add-exclusion C:\\Cache\nfile remove-exclusion C:\\Cache\nfile add-exclusion C:\\Cache\n"
      "registry add-exclusion HKLM\\SOFTWARE\\Vendor\nregistry remove-exclusion HKLM\\SOFTWARE\\Vendor\n"
      "registry add-exclusion HKLM\\SOFTWARE\\Vendor\n");
  QList<app::UwfmgrCommand> commands;
  for (const auto& command : parsed) commands.append(command);
  const auto report = configuration.importCommands(commands);
  QCOMPARE(report.size(), qsizetype{15});
  for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
  const auto pending = configuration.pendingChanges();
  QCOMPARE(pending.setFilterEnabled, std::optional<bool>(false));
  QCOMPARE(pending.setOverlay.maximumSizeMb, std::optional<uint32_t>(10240));
  QCOMPARE(pending.volumeProtect.at(compat::normalizeVolumeId(uwf::test::application::fixtureVolumeCName)), false);
  QCOMPARE(pending.addFileExclusions.at(compat::normalizeVolumeId(uwf::test::application::fixtureVolumeCName)), std::vector<std::string>{R"(C:\Cache)"});
  QVERIFY(pending.removeFileExclusions.empty());
  QCOMPARE(pending.addRegistryExclusions, std::vector<std::string>{R"(HKLM\SOFTWARE\Vendor)"});
  QVERIFY(pending.removeRegistryExclusions.empty());
}

void ApplicationConfigurationTests::importGuardedCommandsPreserveDependencies() {
  auto snapshot = editableSnapshot();
  snapshot.current.filter.enabled = false;
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  configuration.resetSnapshot(snapshot);
  const auto apply = [&](const std::string& text) {
    QList<app::UwfmgrCommand> commands;
    for (const auto& command : app::parseUwfmgrText(text)) commands.append(command);
    return configuration.importCommands(commands);
  };
  const auto rejected = apply("overlay set-type Flash && overlay set-size 2048 && overlay set-warningthreshold 512");
  QCOMPARE(rejected.size(), 3);
  for (const auto& row : rejected) QCOMPARE(row.status, app::ImportResult::Status::Failed);
  QVERIFY(rejected[1].detail.contains(QStringLiteral("Skipped because a prerequisite operation")));
  QVERIFY(global.viewModel().pendingOverlay().empty());
  const auto unsupported = apply("filter enable && # note && overlay set-size 2048");
  QCOMPARE(unsupported.size(), 3);
  QCOMPARE(unsupported[1].status, app::ImportResult::Status::Unsupported);
  QCOMPARE(unsupported[2].status, app::ImportResult::Status::Failed);
  QVERIFY(global.viewModel().pendingOverlay().empty());
  const auto queued = apply("overlay set-type Disk && overlay set-size 4096\noverlay set-warningthreshold 512 && overlay set-criticalthreshold 768");
  QCOMPARE(queued.size(), 4);
  QCOMPARE(queued[0].status, app::ImportResult::Status::Duplicate);
  for (qsizetype index = 1; index < queued.size(); ++index) QCOMPARE(queued[index].status, app::ImportResult::Status::Success);
  QCOMPARE(global.viewModel().pendingOverlay().maximumSizeMb, std::optional<uint32_t>(4096));
  QCOMPARE(global.viewModel().pendingOverlay().warningThresholdMb, std::optional<uint32_t>(512));
  QCOMPARE(global.viewModel().pendingOverlay().criticalThresholdMb, std::optional<uint32_t>(768));
}

void ApplicationConfigurationTests::pendingCountAndReviewShareTheCompleteStagingTarget_data() {
  QTest::addColumn<bool>("nonempty");
  QTest::addColumn<bool>("ancestor");
  QTest::addColumn<bool>("unknown");
  for (const bool nonempty : {false, true})
    for (const bool ancestor : {false, true})
      for (const bool unknown : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("nonempty-%1-ancestor-%2-unknown-%3").arg(nonempty).arg(ancestor).arg(unknown)))
            << nonempty << ancestor << unknown;
}
void ApplicationConfigurationTests::pendingCountAndReviewShareTheCompleteStagingTarget() {
  QFETCH(bool, nonempty);
  QFETCH(bool, ancestor);
  QFETCH(bool, unknown);
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  const auto root = QString::fromUtf8(app::kFileStagingRegistryRoot);
  const auto removed = ancestor ? QStringLiteral("HKLM\\SOFTWARE\\HsingYun") : root;
  source.snapshot.next.registryExclusions = {removed.toStdString()};
  source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  const auto store = std::make_shared<MemoryFileStagingStore>();
  if (nonempty) store->entries = {{app::FileStagingKind::File, QStringLiteral("C:\\Data\\saved.bin")}};
  const auto original = store->entries;
  const auto draft = std::make_shared<app::FileStagingDraft>(store);
  presentation::MainWindowViewModel model(source, app::UwfCapability::Available, draft);
  model.initializeConfiguration(16384, QStringLiteral("C:"));
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  const auto registry = model.configuration()->volume(QStringLiteral("C:"))->registry();
  QCOMPARE(registry->importRemove(removed), nonempty && !ancestor ? app::ExclusionImportOutcome::RejectedRequired : app::ExclusionImportOutcome::Applied);
  QVERIFY(!draft->changes());
  if (unknown) {
    app::FileStagingWriteResult unconfirmed{{}, std::make_exception_ptr(std::runtime_error("The saved list could not be confirmed")), true};
    unconfirmed.revision = draft->revision();
    draft->acceptWrite(unconfirmed);
    model.requireReconciliation();
    store->readFailure = QStringLiteral("An implicit review/count read must not occur");
  }
  const auto cached = draft->editSnapshot();
  QVERIFY(cached);
  QCOMPARE(cached->before, original);
  QCOMPARE(cached->after, original);
  QCOMPARE(cached->persistencePending, unknown);
  const auto reads = store->loads;
  const auto count = model.pendingChangeCount();
  const std::size_t expected = nonempty ? ancestor ? 3 : unknown ? 1 : 0 : 2;
  QCOMPARE(count, expected);
  QCOMPARE(model.pendingChangeCount(), count);
  QCOMPARE(store->loads, reads);
  const auto system = std::make_shared<test::FakeUwfBackend>();
  const auto factory = composition::createViewModelFactory(system, uwf::test::localDesktopPorts());
  if (unknown) {
    QVERIFY_THROWS_EXCEPTION(std::runtime_error,
                             factory->applyPlan(model.configuration()->pendingConfiguration(), model.state().snapshot(), model.state().overlayPolicy(), draft));
    QVERIFY(draft->reconciliationRequired());
  } else {
    const auto review = factory->applyPlan(model.configuration()->pendingConfiguration(), model.state().snapshot(), model.state().overlayPolicy(), draft);
    QCOMPARE(review->changeCount(), count);
    QCOMPARE(review->hasStagingChanges(), count != 0);
    QVERIFY(!draft->changes());
  }
  QCOMPARE(store->loads, reads);
  QCOMPARE(store->writes, 0);
  QVERIFY(system->trace().empty());
}

void ApplicationConfigurationTests::reviewRequiresAnObservedStagingTarget_data() {
  QTest::addColumn<bool>("nonempty");
  QTest::addColumn<bool>("unavailable");
  for (const bool nonempty : {false, true})
    for (const bool unavailable : {false, true})
      QTest::newRow(qPrintable(QStringLiteral("nonempty-%1-unavailable-%2").arg(nonempty).arg(unavailable))) << nonempty << unavailable;
}
void ApplicationConfigurationTests::reviewRequiresAnObservedStagingTarget() {
  QFETCH(bool, nonempty);
  QFETCH(bool, unavailable);
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.next.registryExclusions = {"HKLM\\SOFTWARE\\HsingYun"};
  const auto store = std::make_shared<MemoryFileStagingStore>();
  if (nonempty) store->entries = {{app::FileStagingKind::File, QStringLiteral("C:\\Data\\saved.bin")}};
  if (unavailable) store->readFailure = QStringLiteral("The saved list is unavailable");
  const auto draft = std::make_shared<app::FileStagingDraft>(store);
  presentation::MainWindowViewModel model(source, app::UwfCapability::Available, draft);
  // Synchronous plan composition consumes observations; the window's worker
  // owns initial storage reads and projection.
  QVERIFY(!draft->editSnapshot());
  QVERIFY(!draft->changes());
  QCOMPARE(model.pendingChangeCount(), std::size_t{0});
  QCOMPARE(store->loads, 0);
  app::ConfigurationChanges edits;
  edits.settings.removeRegistryExclusions = {"HKLM\\SOFTWARE\\HsingYun"};
  const auto system = std::make_shared<test::FakeUwfBackend>();
  const auto factory = composition::createViewModelFactory(system, uwf::test::localDesktopPorts());
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, factory->applyPlan(edits, source.snapshot, source.policy, draft));
  QCOMPARE(store->loads, 0);
  model.refresh();
  QTRY_VERIFY(!model.refreshing());
  if (unavailable) {
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, factory->applyPlan(edits, source.snapshot, source.policy, draft));
    QVERIFY(!draft->editSnapshot());
    QVERIFY(!draft->changes());
  } else {
    const auto review = factory->applyPlan(edits, source.snapshot, source.policy, draft);
    QCOMPARE(review->changeCount(), nonempty ? std::size_t{3} : std::size_t{2});
    const auto cached = draft->editSnapshot();
    QVERIFY(cached);
    QCOMPARE(cached->before, store->entries);
    QCOMPARE(cached->after, store->entries);
    QVERIFY(!cached->persistencePending);
    QVERIFY(!draft->changes());
  }
  QCOMPARE(store->loads, 1);
  QCOMPARE(model.pendingChangeCount(), std::size_t{0});
  QCOMPARE(store->loads, 1);
  QCOMPARE(store->writes, 0);
  QVERIFY(system->trace().empty());
}

void ApplicationConfigurationTests::userRefreshPublishesSystemStateWhenStagingCannotBeRead_data() {
  QTest::addColumn<bool>("initialStateFailure");
  QTest::newRow("refresh-action") << false;
  QTest::newRow("workspace-read-again") << true;
}

void ApplicationConfigurationTests::userRefreshPublishesSystemStateWhenStagingCannotBeRead() {
  QFETCH(bool, initialStateFailure);
  const auto system = std::make_shared<test::FakeUwfBackend>();
  const auto store = std::make_shared<MemoryFileStagingStore>();
  store->readFailure = QStringLiteral("The saved staging list could not be read");
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.next.filter.enabled = false;
  source.volumes = {{"C:", fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  if (initialStateFailure) source.failure = QStringLiteral("UWF temporarily unavailable");
  auto services = composition::mainWindowServices(system, source, store, test::localDesktopPorts());
  const auto workflows = std::make_shared<CapturingWindowWorkflows>(services.workflows);
  services.workflows = workflows;
  ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  const auto model = workflows->observed;
  QVERIFY(model);
  QTRY_VERIFY(!model->refreshing() && source.reads == 1);
  QCOMPARE(model->state().hasCommittedState(), !initialStateFailure);
  QCOMPARE(model->workspaceDiagnostic().has_value(), initialStateFailure);
  QCOMPARE(model->pendingChangeCount(), std::size_t{0});
  QTRY_VERIFY(refreshCompleted(window));

  // A successful system read remains useful while the independent saved-list
  // error persists. Exercise the same actions a user can actually activate.
  source.failure.clear();
  source.snapshot.next.filter.enabled = true;
  QSignalSpy committed(model.data(), &presentation::MainWindowViewModel::stateCommitted);
  if (initialStateFailure) {
    auto* retry = window.findChild<ui::ContentState*>(QStringLiteral("mainUnavailableState"));
    QVERIFY(retry && retry->isVisibleTo(&window));
    auto* read = buttonWithText(retry, ui::I18n::tr("Read again"));
    QVERIFY(read && read->isVisibleTo(&window) && read->isEnabled());
    QTest::mouseClick(read, Qt::LeftButton);
  } else {
    auto* refresh = actionWithText(&window, ui::I18n::tr("Refresh"));
    QVERIFY(refresh && refresh->isEnabled());
    refresh->trigger();
  }
  QTRY_COMPARE(committed.count(), 1);
  QTRY_VERIFY(!model->refreshing());
  QCOMPARE(source.reads, 2);
  QVERIFY(model->state().snapshot().next.filter.enabled);
  QVERIFY(!model->workspaceDiagnostic());
  QCOMPARE(model->pendingChangeCount(), std::size_t{0});
  auto* global = window.findChild<ui::GlobalStatusPanel*>();
  QVERIFY(global);
  QVERIFY(global->viewModel().draft().filterEnabled);
  auto* nextFilter = global->findChild<ui::SwitchButton*>();
  QVERIFY(nextFilter && nextFilter->isChecked());
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  const auto stagingWidgets = window.findChildren<ui::FileStagingWidget*>();
  QVERIFY(!stagingWidgets.isEmpty());
  for (auto* staging : stagingWidgets) {
    QTRY_VERIFY(!staging->viewModel().busy());
    QVERIFY(!staging->viewModel().storageAvailable());
    QVERIFY(!staging->viewModel().writable());
    QVERIFY(staging->viewModel().storageError().contains(store->readFailure));
  }
  QCOMPARE(store->writes, 0);
  QVERIFY(test::writeRequests(*system).empty());
}

void ApplicationConfigurationTests::userRefreshDiscardsOrdinaryAndMaintenanceIntentsTogether() {
  const auto system = std::make_shared<test::FakeUwfBackend>();
  const auto store = std::make_shared<MemoryFileStagingStore>();
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = false;
  source.snapshot.current.volumes = source.snapshot.next.volumes = {{fixtureVolumeCName, "C:", false, core::VolumeBinding::VolumeId, std::string("C:")}};
  source.snapshot.runtime.systemVolumeProtected = false;
  source.volumes = {{"C:", fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported, {true, true}}};
  auto services = composition::mainWindowServices(system, source, store, test::localDesktopPorts());
  const auto workflows = std::make_shared<CapturingWindowWorkflows>(services.workflows);
  services.workflows = workflows;
  ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  QTRY_VERIFY(refreshCompleted(window));
  const auto model = workflows->observed;
  QVERIFY(model);
  const auto configuration = model->configuration();
  const auto maintenance = configuration->maintenanceEditor();
  QTRY_VERIFY(!maintenance->busy());
  app::OverlayMaintenanceSnapshot observed;
  observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
  observed.systemVolumeProtected = false;
  observed.currentFlags = app::OverlayFlags{0};
  observed.romDisposition = 0;
  observed.allVolumesProtected = observed.allVolumesNextProtected = false;
  observed.protectScopeConfirmed = true;
  observed.protectAllCapability.support = app::OverlayMaintenanceSupport::Supported;
  observed.protectTargetVolumes = {QString::fromStdString(fixtureVolumeCName)};
  const auto generation = maintenance->beginObservation();
  QVERIFY(generation);
  maintenance->finishObservation(*generation, observed);
  const auto parsed = app::parseUwfmgrText("uwfmgr filter enable\nuwfmgr volume protect all");
  const auto imported = configuration->importCommands({parsed.begin(), parsed.end()});
  QCOMPARE(imported.size(), qsizetype{2});
  for (const auto& row : imported) QCOMPARE(row.status, app::ImportResult::Status::Success);
  QCOMPARE(model->pendingChangeCount(), std::size_t{2});
  QCOMPARE(maintenance->draft().allVolumesProtected, std::optional<bool>{true});
  QVERIFY(!source.snapshot.next.volumes.front().isProtected);

  auto* refresh = actionWithText(&window, ui::I18n::tr("Refresh"));
  QVERIFY(refresh && refresh->isEnabled());
  QSignalSpy committed(model.data(), &presentation::MainWindowViewModel::stateCommitted);
  QVERIFY(test::driveModalDialog(
      window, [refresh] { refresh->trigger(); },
      [&](QDialog& confirmation) {
        QCOMPARE(confirmation.windowTitle(), ui::I18n::tr("Discard pending changes?"));
        auto* proceed = buttonWithText(&confirmation, ui::I18n::tr("Continue"));
        QVERIFY(proceed);
        QTest::mouseClick(proceed, Qt::LeftButton);
      }));
  QTRY_COMPARE(committed.count(), 1);
  QTRY_VERIFY(!model->refreshing());
  QCOMPARE(model->pendingChangeCount(), std::size_t{0});
  QVERIFY(configuration->pendingChanges().empty());
  QVERIFY(maintenance->draft().empty());
  const auto review = model->reviewConfiguration();
  QVERIFY(review);
  QCOMPARE(review->changeCount(), std::size_t{0});
  QVERIFY(review->maintenanceChanges().empty());
  QCOMPARE(source.reads, 2);
  QCOMPARE(store->writes, 0);
  QVERIFY(test::writeRequests(*system).empty());
}

void ApplicationConfigurationTests::offlineStagingEntriesRemainManageableAndReturnToTheirMountedPage_data() {
  QTest::addColumn<bool>("systemRegistered");
  QTest::addColumn<bool>("limitedFileSystem");
  QTest::newRow("registered-system") << true << false;
  QTest::newRow("unregistered-system-physical-unknown") << false << false;
  QTest::newRow("unregistered-system-file-operations-unavailable") << false << true;
}

void ApplicationConfigurationTests::offlineStagingEntriesRemainManageableAndReturnToTheirMountedPage() {
  QFETCH(bool, systemRegistered);
  QFETCH(bool, limitedFileSystem);
  class MountableFiles final : public app::CommitTraversalSource {
   public:
    std::atomic_bool dataMounted = true;
    app::FileCommitTarget resolveFileTarget(const QString& path) const override {
      if (path.startsWith(QStringLiteral("D:"), Qt::CaseInsensitive) && !dataMounted) throw std::runtime_error("The data volume is not mounted");
      return memoryFileTarget(path, path.startsWith(QStringLiteral("D:"), Qt::CaseInsensitive) ? otherFixtureVolumeId : fixtureVolumeId);
    }
    std::string driveLetterForPath(const QString& path) const override {
      (void)resolveFileTarget(path);
      return path.left(2).toUpper().toStdString();
    }
    std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override { return app::bindFileCommitTarget(*this, target); }
    bool exists(const app::CommitNode&) const override { throw std::logic_error("Unexpected staging existence read"); }
    QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override {
      throw std::logic_error("Unexpected staging traversal");
    }
  };
  const auto traversal = std::make_shared<MountableFiles>();
  const auto dialogs = std::make_shared<FileDialogStub>();
  const QString savedPath = QStringLiteral("D:\\Data\\saved.bin");
  dialogs->openedFiles = {savedPath};
  const auto store = std::make_shared<MemoryFileStagingStore>();
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  const auto registryRoot = std::string(app::kFileStagingRegistryRoot);
  source.hormEnabled = false;
  source.policy.transportAvailable = true;
  source.policy.current.filterEnabled = source.policy.next.filterEnabled = {app::PolicyObservationStatus::Available, true};
  source.policy.current.overlayType = {app::PolicyObservationStatus::Available, 0};
  source.policy.next.overlayType = {app::PolicyObservationStatus::Available, 1};
  source.policy.current.flags = source.policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{0}};
  source.snapshot.runtime.romDisposition = 0;
  source.snapshot.runtime.fullCommit = false;
  source.snapshot.current.registryExclusions = source.snapshot.next.registryExclusions = {registryRoot};
  for (auto* session : {&source.snapshot.current, &source.snapshot.next}) {
    if (!systemRegistered) {
      std::erase_if(session->volumes, [](const auto& volume) { return volume.driveLetter == "C:"; });
      session->fileExclusions.erase(fixtureVolumeCName);
    }
    for (auto& volume : session->volumes)
      if (volume.driveLetter == "D:") volume.isProtected = true;
  }
  source.snapshot.runtime.systemVolumeProtected = systemRegistered;
  const core::VolumeInfo systemVolume{"C:",
                                      fixtureVolumeCName,
                                      limitedFileSystem ? "" : "NTFS",
                                      "System",
                                      1000,
                                      500,
                                      limitedFileSystem ? core::VolumeSupport::FileSystemLimited : core::VolumeSupport::Supported,
                                      {}};
  const core::VolumeInfo dataVolume{"D:", fixtureVolumeDName, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported};
  source.volumes = {systemVolume, dataVolume};
  test::FakeUwfBackend::State initial;
  initial.current.filterEnabled = initial.next.filterEnabled = true;
  initial.current.registryExclusions = initial.next.registryExclusions = {registryRoot};
  const auto system = std::make_shared<test::FakeUwfBackend>(std::move(initial));
  auto ports = test::localDesktopPorts();
  ports.traversal = traversal;
  ports.fileDialogs = dialogs;
  auto services = composition::mainWindowServices(system, source, store, std::move(ports));
  const auto workflows = std::make_shared<CapturingWindowWorkflows>(services.workflows);
  services.workflows = workflows;
  ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  QTRY_VERIFY(refreshCompleted(window));
  const auto model = workflows->observed;
  QVERIFY(model);
  auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"));
  QVERIFY(tabs);
  ui::VolumeTab* dataPage = nullptr;
  for (int index = 0; index < tabs->count(); ++index) {
    auto* page = qobject_cast<ui::VolumeTab*>(tabs->widget(index));
    if (page && page->driveLetter() == QStringLiteral("D:")) dataPage = page;
  }
  QVERIFY(dataPage);
  tabs->setCurrentWidget(dataPage);
  dataPage->setActiveInfoPage(ui::VolumeTab::InfoPage::FileStaging);
  auto* staging = dataPage->findChild<ui::FileStagingWidget*>();
  QVERIFY(staging);
  QTRY_VERIFY(!staging->viewModel().busy());
  QVERIFY2(model->configurationWritesAllowed(), qUtf8Printable(model->configurationWriteRestriction()));
  QVERIFY2(staging->viewModel().storageAvailable(), qUtf8Printable(staging->viewModel().storageError()));
  QCOMPARE(staging->viewModel().editingRestriction(), app::RuntimeWriteRestriction::None);
  QVERIFY(!staging->viewModel().readOnly());
  QVERIFY(staging->viewModel().writable());
  QVERIFY(staging->viewModel().canAddEntries());
  auto* addFile = staging->commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  QVERIFY(addFile);
  QVERIFY(addFile->isEnabled());
  addFile->trigger();
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(staging->viewModel().entries().size(), qsizetype{1});
  QCOMPARE(staging->viewModel().entries().front().path, savedPath);
  QVERIFY(store->entries.isEmpty());
  QCOMPARE(store->writes, 0);

  const auto applyPending = [&] {
    bool applied = false;
    QVERIFY(test::driveModalDialog(
        window, [&] { window.showPlan(); },
        [&](QDialog& modal) {
          auto* plan = qobject_cast<ui::ApplyPlanDialog*>(&modal);
          QVERIFY(plan);
          auto* apply = buttonWithText(plan, ui::I18n::tr("Apply changes"));
          QVERIFY(apply && apply->isEnabled());
          QVERIFY(test::driveModalDialog(
              *plan, [&] { QTest::mouseClick(apply, Qt::LeftButton); },
              [&](QDialog& confirmation) {
                auto* accept = buttonWithText(&confirmation, ui::I18n::tr("Apply changes"));
                QVERIFY(accept);
                QTest::mouseClick(accept, Qt::LeftButton);
              }));
          QTRY_VERIFY(applyCompleted(*plan));
          applied = true;
        }));
    QVERIFY(applied);
    QTRY_VERIFY(!model->state().reconciliationRequired());
    QTRY_VERIFY(!model->refreshing());
    QTRY_COMPARE(model->pendingChangeCount(), std::size_t{0});
  };
  applyPending();
  QCOMPARE(store->writes, 1);
  QCOMPARE(store->entries.size(), qsizetype{1});
  QCOMPARE(store->entries.front().path, savedPath);

  const auto refreshMount = [&](const bool mounted) {
    traversal->dataMounted = mounted;
    source.volumes = mounted ? std::vector{systemVolume, dataVolume} : std::vector{systemVolume};
    for (auto* session : {&source.snapshot.current, &source.snapshot.next})
      for (auto& volume : session->volumes)
        if (volume.driveLetter == "D:") volume.physicalDriveLetter = mounted ? std::string("D:") : std::string{};
    QSignalSpy committed(model.data(), &presentation::MainWindowViewModel::stateCommitted);
    auto* refresh = actionWithText(&window, ui::I18n::tr("Refresh"));
    QVERIFY(refresh && refresh->isEnabled());
    refresh->trigger();
    QTRY_COMPARE(committed.count(), 1);
    QTRY_VERIFY(!model->refreshing());
    QCOMPARE(model->pendingChangeCount(), std::size_t{0});
  };
  const auto ownerOfSavedPath = [&]() -> ui::FileStagingWidget* {
    ui::FileStagingWidget* owner = nullptr;
    for (int index = 0; index < tabs->count(); ++index) {
      auto* candidate = tabs->widget(index)->findChild<ui::FileStagingWidget*>();
      if (!candidate) continue;
      const auto& entries = candidate->viewModel().entries();
      if (std::ranges::none_of(entries, [&](const auto& entry) { return entry.path == savedPath; })) continue;
      if (owner) return nullptr;
      owner = candidate;
    }
    return owner;
  };
  refreshMount(false);
  auto* offline = ownerOfSavedPath();
  QVERIFY(offline);
  QCOMPARE(offline->driveLetter(), QStringLiteral("C:"));
  QTRY_VERIFY(!offline->viewModel().busy());
  QVERIFY(offline->viewModel().writable());
  QCOMPARE(offline->viewModel().canAddEntries(), !limitedFileSystem);
  const auto systemConfiguration = model->configuration()->volume(QStringLiteral("C:"));
  QVERIFY(systemConfiguration);
  if (!systemRegistered) {
    QVERIFY(!systemConfiguration->status()->draft().hasTarget);
    QVERIFY(!systemConfiguration->status()->controlsEnabled());
  }
  auto* fallbackAdd = buttonWithText(offline->commandWidget(), ui::I18n::tr("Add"));
  auto* fallbackFile = offline->commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  auto* fallbackFolder = offline->commandWidget()->findChild<QAction*>(QStringLiteral("stageDirectoryAction"));
  QVERIFY(fallbackAdd && fallbackFile && fallbackFolder);
  QCOMPARE(fallbackAdd->isEnabled(), !limitedFileSystem);
  QCOMPARE(fallbackFile->isEnabled(), !limitedFileSystem);
  QCOMPARE(fallbackFolder->isEnabled(), !limitedFileSystem);
  if (limitedFileSystem) {
    QCOMPARE(fallbackAdd->toolTip(), ui::I18n::tr("Read-only"));
    QCOMPARE(fallbackFile->toolTip(), fallbackAdd->toolTip());
    QCOMPARE(fallbackFolder->toolTip(), fallbackAdd->toolTip());
    const auto requests = dialogs->requests.size();
    QTest::mouseClick(fallbackAdd, Qt::LeftButton);
    fallbackFile->trigger();
    fallbackFolder->trigger();
    QCOMPARE(dialogs->requests.size(), requests);
  }
  QCOMPARE(offline->viewModel().index(0, 0).data(presentation::StatusTextRole).toString(), ui::I18n::tr("Unavailable"));
  QVERIFY(!offline->viewModel().entries().front().fileTarget);
  QCOMPARE(store->writes, 1);

  refreshMount(true);
  auto* remounted = ownerOfSavedPath();
  QVERIFY(remounted);
  QCOMPARE(remounted->driveLetter(), QStringLiteral("D:"));
  QVERIFY(remounted->viewModel().entries().front().fileTarget);
  QCOMPARE(remounted->viewModel().index(0, 0).data(presentation::StatusTextRole).toString(), ui::I18n::tr("Ready"));
  QCOMPARE(store->writes, 1);

  refreshMount(false);
  offline = ownerOfSavedPath();
  QVERIFY(offline);
  ui::VolumeTab* offlinePage = nullptr;
  for (int index = 0; index < tabs->count(); ++index)
    if (tabs->widget(index)->isAncestorOf(offline)) offlinePage = qobject_cast<ui::VolumeTab*>(tabs->widget(index));
  QVERIFY(offlinePage);
  tabs->setCurrentWidget(offlinePage);
  offlinePage->setActiveInfoPage(ui::VolumeTab::InfoPage::FileStaging);
  auto* list = test::stagingEntryList(*offline);
  auto* remove = buttonWithText(offline->commandWidget(), ui::I18n::tr("Remove selected"));
  QVERIFY(list && remove);
  QTRY_VERIFY(!offline->viewModel().busy());
  QCOMPARE(list->model()->rowCount(), 1);
  list->selectAll();
  QVERIFY(remove->isEnabled());
  QTest::mouseClick(remove, Qt::LeftButton);
  QTRY_VERIFY(!offline->viewModel().busy());
  QCOMPARE(model->pendingChangeCount(), std::size_t{2});  // The saved entry and its now-unneeded registry exclusion.
  QCOMPARE(store->writes, 1);
  QCOMPARE(store->entries.size(), qsizetype{1});
  list->selectAll();
  QCOMPARE(remove->property("accessibleName").toString(), ui::I18n::tr("Undo removal"));
  QCOMPARE(remove->isEnabled(), !limitedFileSystem);
  if (limitedFileSystem) {
    QCOMPARE(remove->toolTip(), ui::I18n::tr("Read-only"));
    QTest::mouseClick(remove, Qt::LeftButton);
    QVERIFY(offline->viewModel().entries().isEmpty());
  }
  applyPending();
  QCOMPARE(store->writes, 2);
  QVERIFY(store->entries.isEmpty());
  QVERIFY(system->snapshot().next.registryExclusions.empty());
}

void ApplicationConfigurationTests::partialApplyPreservesBulkProtectionExceptionsForRetry_data() {
  QTest::addColumn<bool>("bulkFails");
  QTest::addColumn<bool>("discard");
  QTest::newRow("bulk-succeeds-exception-fails/retry") << false << false;
  QTest::newRow("bulk-partially-fails-exception-not-attempted/retry") << true << false;
  QTest::newRow("bulk-succeeds-exception-fails/discard") << false << true;
  QTest::newRow("bulk-partially-fails-exception-not-attempted/discard") << true << true;
}

void ApplicationConfigurationTests::partialApplyPreservesBulkProtectionExceptionsForRetry() {
  QFETCH(bool, bulkFails);
  QFETCH(bool, discard);
  class Volumes final : public app::VolumeRepository {
   public:
    std::vector<core::VolumeInfo> read() const override {
      return {{"C:", fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported, {true, true}},
              {"D:", fixtureVolumeDName, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported, {true, true}}};
    }
    std::string driveForPath(const std::string& path) const override { return path.substr(0, 2); }
    std::vector<std::string> mountedLetters(const std::string& volume) const override {
      const auto id = compat::parseVolumeId(volume);
      if (id == fixtureVolumeId) return {"C:"};
      if (id == otherFixtureVolumeId) return {"D:"};
      return {};
    }
    std::vector<std::string> mountedPaths(const std::string& volume) const override {
      auto paths = mountedLetters(volume);
      for (auto& path : paths) path += '\\';
      return paths;
    }
    std::string systemDrive() const override { return "C:"; }
  };
  class PublishedSource final : public app::ApplicationStateSource {
   public:
    PublishedSource(std::shared_ptr<test::FakeUwfBackend> system, std::shared_ptr<Volumes> volumes)
        : m_system(std::move(system)), m_volumes(std::move(volumes)) {}
    ReadOperation readOperation(app::UwfCapability) override {
      return [system = m_system, volumes = m_volumes](std::stop_token) {
        const auto facts = system->state();
        return app::ApplicationState{volumes->read(), app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {}),
                                     app::UwfOverlayOperations::snapshot(facts), false};
      };
    }

   private:
    std::shared_ptr<test::FakeUwfBackend> m_system;
    std::shared_ptr<Volumes> m_volumes;
  };
  test::FakeUwfBackend::State initial;
  initial.next.volumes.emplace(fixtureVolumeId, test::FakeUwfBackend::VolumeConfiguration{{fixtureVolumeId, "C:", false, true, {}}, {}});
  initial.next.volumes.emplace(otherFixtureVolumeId, test::FakeUwfBackend::VolumeConfiguration{{otherFixtureVolumeId, "D:", false, false, {}}, {}});
  initial.current = initial.next;
  initial.runtime.systemVolumeId = fixtureVolumeId;
  initial.runtime.physicalVolumes[fixtureVolumeId] = initial.runtime.physicalVolumes[otherFixtureVolumeId] = {true, true};
  initial.runtime.fileMaintenancePrefixes[fixtureVolumeId] = "C:";
  initial.runtime.fileMaintenancePrefixes[otherFixtureVolumeId] = "D:";
  const auto system = std::make_shared<test::FakeUwfBackend>(std::move(initial));
  const auto all = app::composeAllVolumeProtection(
      system, {.isAdministrator = [](const compat::Context&) { return true; },
               .protectScope = [](bool, const compat::Context&) { return std::vector<std::string>{fixtureVolumeId, otherFixtureVolumeId}; }});
  const auto protectedState = all->read(compat::GetAllProtected{});
  const auto unprotectedState = all->read(compat::GetAllUnprotected{});
  QVERIFY(!protectedState.failure && protectedState.scopeConfirmed);
  QCOMPARE(protectedState.value, std::optional<bool>{false});
  QVERIFY(!unprotectedState.failure && unprotectedState.scopeConfirmed);
  QCOMPARE(unprotectedState.value, std::optional<bool>{true});
  const auto volumes = std::make_shared<Volumes>();
  PublishedSource source(system, volumes);
  const auto store = std::make_shared<MemoryFileStagingStore>();
  auto ports = test::localDesktopPorts();
  ports.volumes = volumes;
  ports.maintenanceObservations = {};
  auto services = composition::mainWindowServices(system, source, store, std::move(ports));
  const auto workflows = std::make_shared<CapturingWindowWorkflows>(services.workflows);
  services.workflows = workflows;
  ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  QTRY_VERIFY(refreshCompleted(window));
  const auto model = workflows->observed;
  QVERIFY(model);
  const auto importer = model->importModel();
  QVERIFY(importer);
  QTRY_VERIFY(!importer->preparing());
  importer->setText(QStringLiteral("uwfmgr volume protect all && uwfmgr volume unprotect C:"));
  importer->importText();
  QCOMPARE(importer->batches().size(), qsizetype{1});
  QCOMPARE(importer->batches().front().size(), qsizetype{2});
  for (const auto& row : importer->batches().front()) QCOMPARE(row.status, app::ImportResult::Status::Success);
  const auto original = model->configuration()->pendingConfiguration();
  QCOMPARE(original.maintenance.allVolumesProtected, std::optional<bool>{true});
  QVERIFY(original.settings.volumeProtect.contains(fixtureVolumeId));
  QVERIFY(!original.settings.volumeProtect.at(fixtureVolumeId));
  QCOMPARE(model->pendingChangeCount(), std::size_t{2});

  system->inject({.request = compat::SetVolume{bulkFails ? otherFixtureVolumeId : fixtureVolumeId, compat::VolumeProperty::Protected, bulkFails, {}, false},
                  .operation = {},
                  .timing = test::FakeUwfBackend::FaultTiming::Before,
                  .kind = compat::ErrorKind::Failed,
                  .message = "The selected volume is temporarily unavailable"});
  QSignalSpy committed(model.data(), &presentation::MainWindowViewModel::stateCommitted);
  std::shared_ptr<presentation::ApplyPlanViewModel> reviewed;
  connect(model.data(), &presentation::MainWindowViewModel::reviewPrepared, &window, [&](const auto& plan) { reviewed = plan; });
  const auto reviewAndApply = [&] {
    reviewed.reset();
    const auto previousCommits = committed.count();
    bool applied = false;
    QVERIFY(test::driveModalDialog(
        window, [&] { window.showPlan(); },
        [&](QDialog& modal) {
          auto* plan = qobject_cast<ui::ApplyPlanDialog*>(&modal);
          QVERIFY(plan);
          auto* apply = buttonWithText(plan, ui::I18n::tr("Apply changes"));
          QVERIFY(apply && apply->isEnabled());
          QVERIFY(test::driveModalDialog(
              *plan, [&] { QTest::mouseClick(apply, Qt::LeftButton); },
              [&](QDialog& confirmation) {
                auto* accept = buttonWithText(&confirmation, ui::I18n::tr("Apply changes"));
                QVERIFY(accept);
                QTest::mouseClick(accept, Qt::LeftButton);
              }));
          QTRY_VERIFY(applyCompleted(*plan));
          applied = true;
        }));
    QVERIFY(applied && reviewed);
    // This notification comes from the production post-write refresh; the test
    // never issues an observation between either Apply and these assertions.
    QTRY_COMPARE(committed.count(), previousCommits + 1);
    QTRY_VERIFY(!model->refreshing() && !model->state().reconciliationRequired());
    QTRY_VERIFY(!model->overlayMaintenance()->busy());
  };
  reviewAndApply();
  QVERIFY(reviewed);
  QVERIFY(reviewed->resultFailed());
  const auto exception = std::ranges::find_if(reviewed->result().steps, [](const auto& step) {
    return step.operation && step.operation->kind == app::ConfigurationOperationKind::VolumeProtection && step.operation->target == fixtureVolumeId;
  });
  QVERIFY(exception != reviewed->result().steps.end());
  QCOMPARE(exception->outcome, bulkFails ? app::ConfigurationStepOutcome::NotAttempted : app::ConfigurationStepOutcome::Failed);
  const auto bulk = std::ranges::find_if(
      reviewed->result().steps, [](const auto& step) { return step.operation && step.operation->kind == app::ConfigurationOperationKind::ProtectAllVolumes; });
  QVERIFY(bulk != reviewed->result().steps.end());
  QCOMPARE(bulk->outcome, bulkFails ? app::ConfigurationStepOutcome::Failed : app::ConfigurationStepOutcome::Succeeded);
  QVERIFY(system->snapshot().next.volumes.at(fixtureVolumeId).volume.protectedVolume);
  QCOMPARE(system->snapshot().next.volumes.at(otherFixtureVolumeId).volume.protectedVolume, !bulkFails);
  const auto remaining = model->configuration()->pendingConfiguration();
  QVERIFY(remaining.settings.volumeProtect.contains(fixtureVolumeId));
  QVERIFY(!remaining.settings.volumeProtect.at(fixtureVolumeId));
  QVERIFY(model->pendingChangeCount() > 0);
  if (discard) {
    const auto writesBeforeDiscard = test::writeRequests(*system).size();
    const auto previousCommits = committed.count();
    auto* refresh = actionWithText(&window, ui::I18n::tr("Refresh"));
    QVERIFY(refresh && refresh->isEnabled());
    QVERIFY(test::driveModalDialog(
        window, [refresh] { refresh->trigger(); },
        [&](QDialog& confirmation) {
          QCOMPARE(confirmation.windowTitle(), ui::I18n::tr("Discard pending changes?"));
          auto* proceed = buttonWithText(&confirmation, ui::I18n::tr("Continue"));
          QVERIFY(proceed);
          QTest::mouseClick(proceed, Qt::LeftButton);
        }));
    QTRY_COMPARE(committed.count(), previousCommits + 1);
    QTRY_VERIFY(!model->refreshing());
    QCOMPARE(model->pendingChangeCount(), std::size_t{0});
    const auto emptyReview = model->reviewConfiguration();
    QVERIFY(emptyReview);
    QCOMPARE(emptyReview->changeCount(), std::size_t{0});
    QCOMPARE(test::writeRequests(*system).size(), writesBeforeDiscard);
    QVERIFY(system->snapshot().next.volumes.at(fixtureVolumeId).volume.protectedVolume);
    QCOMPARE(system->snapshot().next.volumes.at(otherFixtureVolumeId).volume.protectedVolume, !bulkFails);
  } else {
    reviewAndApply();
    QVERIFY(reviewed);
    QVERIFY(reviewed->changes().volumeProtect.contains(fixtureVolumeId));
    QVERIFY(!reviewed->changes().volumeProtect.at(fixtureVolumeId));
    QVERIFY(!reviewed->resultFailed());
    QVERIFY(!system->snapshot().next.volumes.at(fixtureVolumeId).volume.protectedVolume);
    QVERIFY(system->snapshot().next.volumes.at(otherFixtureVolumeId).volume.protectedVolume);
    QCOMPARE(model->pendingChangeCount(), std::size_t{0});
  }
  QCOMPARE(store->writes, 0);
}

void ApplicationConfigurationTests::replacingAMountedStagingAliasPersistsTheSelectedDisplayAddress() {
  class MountedFiles final : public app::CommitTraversalSource {
   public:
    std::atomic_bool mounted = false;
    app::FileCommitTarget resolveFileTarget(const QString& path) const override {
      const auto mount = QStringLiteral("C:\\MountedD");
      if (mounted && path.startsWith(mount + u'\\', Qt::CaseInsensitive)) return {otherFixtureVolumeId, path.mid(mount.size())};
      return memoryFileTarget(path, path.startsWith(QStringLiteral("D:"), Qt::CaseInsensitive) ? otherFixtureVolumeId : fixtureVolumeId);
    }
    std::string driveLetterForPath(const QString& path) const override { return resolveFileTarget(path).volumeId == otherFixtureVolumeId ? "D:" : "C:"; }
    std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override { return app::bindFileCommitTarget(*this, target); }
    bool exists(const app::CommitNode&) const override { throw std::logic_error("Unexpected staging existence read"); }
    QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override {
      throw std::logic_error("Unexpected staging traversal");
    }
  };
  const QString oldAddress = QStringLiteral("C:\\MountedD\\state.bin");
  const QString replacement = QStringLiteral("D:\\state.bin");
  const auto traversal = std::make_shared<MountedFiles>();
  const auto dialogs = std::make_shared<FileDialogStub>();
  dialogs->openedFiles = {oldAddress};
  const auto store = std::make_shared<MemoryFileStagingStore>();
  const auto registryRoot = std::string(app::kFileStagingRegistryRoot);
  test::FakeUwfBackend::State initial;
  initial.current.filterEnabled = initial.next.filterEnabled = true;
  initial.current.registryExclusions = initial.next.registryExclusions = {registryRoot};
  const auto system = std::make_shared<test::FakeUwfBackend>(std::move(initial));
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.runtime.romDisposition = 0;
  source.snapshot.runtime.fullCommit = false;
  source.snapshot.current.registryExclusions = source.snapshot.next.registryExclusions = {registryRoot};
  source.hormEnabled = false;
  source.policy.transportAvailable = true;
  source.policy.current.filterEnabled = source.policy.next.filterEnabled = {app::PolicyObservationStatus::Available, true};
  source.policy.current.overlayType = {app::PolicyObservationStatus::Available, 0};
  source.policy.next.overlayType = {app::PolicyObservationStatus::Available, 1};
  source.policy.current.flags = source.policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{0}};
  source.volumes = {{"C:", fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported},
                    {"D:", fixtureVolumeDName, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported}};
  auto ports = test::localDesktopPorts();
  ports.traversal = traversal;
  ports.fileDialogs = dialogs;
  auto services = composition::mainWindowServices(system, source, store, std::move(ports));
  const auto workflows = std::make_shared<CapturingWindowWorkflows>(services.workflows);
  services.workflows = workflows;
  ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  QTRY_VERIFY(refreshCompleted(window));
  const auto model = workflows->observed;
  QVERIFY(model);
  auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"));
  QVERIFY(tabs);
  const auto showStaging = [&](const QString& drive) -> ui::FileStagingWidget* {
    for (int index = 0; index < tabs->count(); ++index) {
      auto* page = qobject_cast<ui::VolumeTab*>(tabs->widget(index));
      if (!page || page->driveLetter() != drive) continue;
      tabs->setCurrentWidget(page);
      page->setActiveInfoPage(ui::VolumeTab::InfoPage::FileStaging);
      return page->findChild<ui::FileStagingWidget*>();
    }
    return nullptr;
  };
  const auto reviewAndApply = [&] {
    bool applied = false;
    QVERIFY(test::driveModalDialog(
        window, [&] { window.showPlan(); },
        [&](QDialog& modal) {
          auto* plan = qobject_cast<ui::ApplyPlanDialog*>(&modal);
          QVERIFY(plan);
          auto* apply = buttonWithText(plan, ui::I18n::tr("Apply changes"));
          QVERIFY(apply && apply->isEnabled());
          QVERIFY(test::driveModalDialog(
              *plan, [&] { QTest::mouseClick(apply, Qt::LeftButton); },
              [&](QDialog& confirmation) {
                auto* accept = buttonWithText(&confirmation, ui::I18n::tr("Apply changes"));
                QVERIFY(accept);
                QTest::mouseClick(accept, Qt::LeftButton);
              }));
          QTRY_VERIFY(applyCompleted(*plan));
          applied = true;
        }));
    QVERIFY(applied);
    QTRY_VERIFY(!model->state().reconciliationRequired() && !model->refreshing());
    QTRY_COMPARE(model->pendingChangeCount(), std::size_t{0});
  };
  const auto refresh = [&] {
    QSignalSpy committed(model.data(), &presentation::MainWindowViewModel::stateCommitted);
    auto* action = actionWithText(&window, ui::I18n::tr("Refresh"));
    QVERIFY(action && action->isEnabled());
    action->trigger();
    QTRY_COMPARE(committed.count(), 1);
    QTRY_VERIFY(!model->refreshing());
    QCOMPARE(model->pendingChangeCount(), std::size_t{0});
  };
  auto* staging = showStaging(QStringLiteral("C:"));
  QVERIFY(staging);
  QTRY_VERIFY(!staging->viewModel().busy());
  auto* add = staging->commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  QVERIFY(add && add->isEnabled());
  add->trigger();
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(staging->viewModel().entries().size(), qsizetype{1});
  QCOMPARE(staging->viewModel().entries().front().path, oldAddress);
  QCOMPARE(store->writes, 0);
  reviewAndApply();
  QCOMPARE(store->writes, 1);
  QCOMPARE(store->entries.size(), qsizetype{1});
  QCOMPARE(store->entries.front().path, oldAddress);

  traversal->mounted = true;
  refresh();
  staging = showStaging(QStringLiteral("D:"));
  QVERIFY(staging);
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(staging->viewModel().entries().size(), qsizetype{1});
  QCOMPARE(staging->viewModel().entries().front().path, oldAddress);
  QVERIFY(staging->viewModel().entries().front().fileTarget);
  QCOMPARE(staging->viewModel().entries().front().fileTarget->volumeId, std::string(otherFixtureVolumeId));
  auto* list = test::stagingEntryList(*staging);
  auto* remove = buttonWithText(staging->commandWidget(), ui::I18n::tr("Remove selected"));
  QVERIFY(list && remove);
  list->selectAll();
  QVERIFY(remove->isEnabled());
  QTest::mouseClick(remove, Qt::LeftButton);
  QTRY_VERIFY(!staging->viewModel().busy());
  QVERIFY(staging->viewModel().entries().isEmpty());
  dialogs->openedFiles = {replacement};
  add = staging->commandWidget()->findChild<QAction*>(QStringLiteral("stageFilesAction"));
  QVERIFY(add && add->isEnabled());
  add->trigger();
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(dialogs->requests.constLast().allowedRoots, QStringList{QStringLiteral("D:\\")});
  QCOMPARE(staging->viewModel().entries().size(), qsizetype{1});
  QCOMPARE(staging->viewModel().entries().front().path, replacement);
  QVERIFY(model->pendingChangeCount() > 0);
  QCOMPARE(store->writes, 1);
  QCOMPARE(store->entries.front().path, oldAddress);
  reviewAndApply();
  QCOMPARE(store->writes, 2);
  QCOMPARE(store->entries.size(), qsizetype{1});
  QCOMPARE(store->entries.front().path, replacement);

  // Remove only the old directory alias. The saved D: selection must survive
  // the next observation without reverting to the former C: display name.
  traversal->mounted = false;
  refresh();
  staging = showStaging(QStringLiteral("D:"));
  QVERIFY(staging);
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(staging->viewModel().entries().size(), qsizetype{1});
  QCOMPARE(staging->viewModel().entries().front().path, replacement);
  auto* systemStaging = showStaging(QStringLiteral("C:"));
  QVERIFY(systemStaging);
  QVERIFY(systemStaging->viewModel().entries().isEmpty());
  QCOMPARE(store->writes, 2);
  QVERIFY(test::writeRequests(*system).empty());
}

void ApplicationConfigurationTests::reviewReadFailurePreservesTheDraftAndAllowsRetry() {
  const auto system = std::make_shared<test::FakeUwfBackend>();
  const auto store = std::make_shared<MemoryFileStagingStore>();
  store->readFailure = QStringLiteral("The saved staging list could not be read");
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.next.registryExclusions.clear();
  source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  ui::MainWindow window(composition::mainWindowServices(system, source, store, uwf::test::localDesktopPorts()),
                        {.uwfCapability = app::UwfCapability::Available, .compatibilityMode = false, .hubVisible = false});
  window.show();
  QTRY_VERIFY(refreshCompleted(window));
  const auto stagingWidgets = window.findChildren<ui::FileStagingWidget*>();
  for (auto* staging : stagingWidgets) QTRY_VERIFY(!staging->viewModel().busy());
  auto* global = window.findChild<ui::GlobalStatusPanel*>();
  auto* reviewAction = actionWithText(&window, ui::I18n::tr("Review and apply"));
  QVERIFY(global && reviewAction && reviewAction->isEnabled());
  const bool targetFilter = !source.snapshot.next.filter.enabled;
  QCOMPARE((global->viewModel().importFilterEnabled(targetFilter)).outcome, uwf::app::GlobalEditOutcome::Applied);
  const auto pendingText = ui::I18n::tr("%1 pending change(s)").arg(1);
  const auto pendingIsVisible = [&] {
    const auto labels = window.findChildren<QLabel*>();
    return std::ranges::any_of(labels, [&](const auto* label) { return label->isVisibleTo(&window) && label->text() == pendingText; });
  };
  QVERIFY(pendingIsVisible());
  bool warningShown = false;
  QVERIFY(test::driveModalDialog(
      window, [reviewAction] { reviewAction->trigger(); },
      [&](QDialog& warning) {
        QVERIFY(!qobject_cast<ui::ApplyPlanDialog*>(&warning));
        QCOMPARE(warning.windowTitle(), ui::I18n::tr("Review and apply"));
        const auto messages = warning.findChildren<ui::ContentHintBox*>();
        QVERIFY(std::ranges::any_of(messages, [&](const auto* text) { return text->toPlainText() == store->readFailure; }));
        warningShown = true;
      }));
  QVERIFY(warningShown);
  QVERIFY(!QApplication::activeModalWidget());
  QVERIFY(reviewAction->isEnabled());
  QCOMPARE(global->viewModel().pendingFilterEnabled(), std::optional<bool>(targetFilter));
  QVERIFY(pendingIsVisible());
  QCOMPARE(store->writes, 0);
  QVERIFY(test::writeRequests(*system).empty());

  store->readFailure.clear();
  bool reviewShown = false;
  QVERIFY(test::driveModalDialog(
      window, [reviewAction] { reviewAction->trigger(); },
      [&](QDialog& dialog) {
        auto* review = qobject_cast<ui::ApplyPlanDialog*>(&dialog);
        QVERIFY(review);
        const auto changes = review->findChild<QListView*>(QStringLiteral("applyChangesList"));
        QVERIFY(changes);
        QCOMPARE(changes->model()->rowCount(), 3);  // One heading, one change, and the command disclosure.
        QVERIFY(pendingIsVisible());
        reviewShown = true;
      }));
  QVERIFY(reviewShown);
  QVERIFY(reviewAction->isEnabled());
  QCOMPARE(global->viewModel().pendingFilterEnabled(), std::optional<bool>(targetFilter));
  QVERIFY(pendingIsVisible());
  QCOMPARE(store->writes, 0);
  QVERIFY(test::writeRequests(*system).empty());
}

void ApplicationConfigurationTests::applyPlanPreviewAndCopyUseTheSameProductionCommandMapping() {
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  auto snapshot = editableSnapshot();
  snapshot.next.overlay.type = core::OverlayType::Disk;
  snapshot.next.overlay.maximumSizeMb = 8192;
  configuration.resetSnapshot(snapshot);
  QCOMPARE((global.viewModel().importFilterEnabled(false)).outcome, uwf::app::GlobalEditOutcome::Applied);
  auto backend = std::make_shared<test::FakeUwfBackend>();
  QTemporaryDir exportDirectory;
  QVERIFY(exportDirectory.isValid());
  const QString exportPath = exportDirectory.filePath(QStringLiteral("commands.cmd"));
  {
    QFile previous(exportPath);
    QVERIFY(previous.open(QIODevice::WriteOnly));
    QCOMPARE(previous.write("stale"), qint64{5});
  }
  FileDialogStub files;
  files.savedFile = exportPath;
  ui::ApplyPlanDialog dialog(
      uwf::composition::createViewModelFactory(backend, uwf::test::localDesktopPorts())->applyPlan(configuration.pendingChanges(), snapshot), files);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  if (auto* disclosure = dialog.findChild<ui::Disclosure*>(QStringLiteral("changeCommandsDisclosure"))) disclosure->setOpen(true);
  QTextEdit* pending = nullptr;
  for (auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : std::as_const(edits)) {
    if (edit->toPlainText().contains(QStringLiteral("uwfmgr.exe filter disable"))) pending = edit;
  }
  QVERIFY(pending);
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(apply);
  QVERIFY(apply->isEnabled());
  if (auto* disclosure = dialog.findChild<ui::Disclosure*>(QStringLiteral("changeCommandsDisclosure"))) disclosure->setOpen(true);
  pending->selectAll();
  pending->setFocus();
  QTest::keyClick(pending, Qt::Key_C, Qt::ControlModifier);
  const auto copied = QApplication::clipboard()->text();
  QCOMPARE(copied.trimmed(), pending->toPlainText().trimmed());
  const auto copiedCommands = app::parseUwfmgrText(copied.toStdString());
  QCOMPARE(copiedCommands.size(), std::size_t{1});
  QCOMPARE(copiedCommands.front().kind, app::UwfmgrKind::FilterDisable);
  QCOMPARE(copiedCommands.front().parseError, app::ParseError::None);
  auto* exportButton = buttonWithText(&dialog, QStringLiteral("Export commands"));
  QVERIFY(exportButton);
  // The copied preview above is Pending changes; this export deliberately
  // verifies the complete Next configuration rather than changing its scope.
  auto* reviewTabs = dialog.findChild<QTabWidget*>(QStringLiteral("reviewTabs"));
  QVERIFY(reviewTabs && reviewTabs->count() == 2);
  reviewTabs->setCurrentIndex(1);
  QTRY_VERIFY(exportButton->isEnabled());
  bool exportSucceeded = false;
  QVERIFY(test::driveModalDialog(
      dialog, [&] { QTest::mouseClick(exportButton, Qt::LeftButton); },
      [&](QDialog& information) {
        QCOMPARE(information.windowTitle(), QStringLiteral("Export finished"));
        exportSucceeded = true;
        information.accept();
      }));
  QVERIFY(exportSucceeded);
  QFile output(exportPath);
  QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
  const QString exportedText = QString::fromUtf8(output.readAll());
  QVERIFY(exportedText.contains(QStringLiteral("uwfmgr.exe filter disable")));
  QVERIFY(!exportedText.contains(QStringLiteral("uwfmgr.exe filter enable")));
  QVERIFY(exportedText.contains(QStringLiteral("uwfmgr.exe overlay set-type Disk")));
  QVERIFY(exportedText.contains(QStringLiteral("uwfmgr.exe overlay set-size 8192")));
  QVERIFY(!exportedText.contains(QStringLiteral(" && ")));
  presentation::ConfigurationViewModel replayConfiguration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel replay(replayConfiguration.global());
  replayConfiguration.resetSnapshot(snapshot);
  QList<app::UwfmgrCommand> exportedCommands;
  for (const auto& command : app::parseUwfmgrText(exportedText.toStdString())) exportedCommands.append(command);
  (void)replayConfiguration.importCommands(exportedCommands);
  QCOMPARE(replay.viewModel().pendingFilterEnabled(), std::optional<bool>(false));
  QVERIFY(replay.viewModel().pendingOverlay().empty());
  QVERIFY(exportedText.count(QChar('\n')) > 1);
  QVERIFY(exportedText.startsWith(QString::fromUtf8(app::kUwfmgrScriptMarker.data(), static_cast<qsizetype>(app::kUwfmgrScriptMarker.size()))));
  QCOMPARE(files.requests.size(), 1);
  QVERIFY(QFileInfo(files.requests.front().initialPath).fileName().startsWith(QStringLiteral("uwfmgr-next-configuration-")));
  QVERIFY(files.requests.front().initialPath.endsWith(QStringLiteral(".cmd")));

  files.savedFile.clear();
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QCOMPARE(files.requests.size(), 2);

  files.savedFile = QDir::tempPath();
  bool failureReported = false;
  QVERIFY(test::driveModalDialog(
      dialog, [&] { QTest::mouseClick(exportButton, Qt::LeftButton); },
      [&](QDialog& warning) {
        for (auto labels = warning.findChildren<ui::ContentHintBox*>(); auto* label : std::as_const(labels)) {
          failureReported = failureReported || label->toPlainText().contains(QStringLiteral("Could not open file for writing"));
        }
        warning.accept();
      }));
  QVERIFY(failureReported);
  QCOMPARE(files.requests.size(), 3);
  QVERIFY(backend->trace().empty());
}

void ApplicationConfigurationTests::applyPlanConfirmedWritePublishesReconciliationAndPreventsReplay() {
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  const auto snapshot = editableSnapshot();
  configuration.resetSnapshot(snapshot);
  QCOMPARE((global.viewModel().importFilterEnabled(false)).outcome, uwf::app::GlobalEditOutcome::Applied);

  test::FakeUwfBackend::State state;
  state.current.filterEnabled = snapshot.current.filter.enabled;
  state.next.filterEnabled = snapshot.next.filter.enabled;
  auto backend = std::make_shared<test::FakeUwfBackend>(state);
  auto actions = std::make_shared<app::ConfigurationExecutor>(app::uwfConfigurationRepository(backend));
  auto model = std::make_shared<presentation::ApplyPlanViewModel>(configuration.pendingChanges(), snapshot, actions);
  ui::ApplyPlanDialog dialog(model, uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy reconciliation(&dialog, &ui::ApplyPlanDialog::reconciliationRequired);
  QSignalSpy safeRestartRequested(&dialog, &ui::ApplyPlanDialog::safeRestartRequested);
  QSignalSpy directRestartRequested(&dialog, &ui::ApplyPlanDialog::directRestartRequested);
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  auto* restart = dialog.findChild<QPushButton*>(QStringLiteral("restartBtn"));
  auto* directRestart = dialog.findChild<QPushButton*>(QStringLiteral("directRestartBtn"));
  QVERIFY(apply && apply->isEnabled());
  QVERIFY(restart && restart->isHidden());
  QVERIFY(directRestart && directRestart->isHidden());

  QVERIFY(test::driveModalDialog(dialog, [&] { QTest::mouseClick(apply, Qt::LeftButton); }, [](QDialog& confirmation) { confirmation.accept(); }));
  QTRY_VERIFY(applyCompleted(dialog));

  QCOMPARE(backend->trace().size(), std::size_t{1});
  QCOMPARE(backend->trace().front().request, (compat::Request{compat::SetBoolean{compat::BooleanProperty::FilterEnabled, false}}));
  QVERIFY(backend->snapshot().current.filterEnabled);
  QVERIFY(!backend->snapshot().next.filterEnabled);
  QCOMPARE(reconciliation.count(), 1);
  QVERIFY(!apply->isEnabled());
  QVERIFY(!restart->isHidden());
  QVERIFY(!directRestart->isHidden());
  QCOMPARE(directRestart->toolTip(), ui::I18n::tr("Direct restart skips staged file commits."));
  const auto requested = test::boundedWindowSize(dialog, QSize(820, 560));
  QVERIFY(!requested.isEmpty());
  dialog.resize(requested);
  QTRY_VERIFY(test::similarSize(QRectF(QPointF(), dialog.size()), QRectF(QPointF(), requested)));
  const auto clipping = test::waitForUnclippedText(dialog, 2000);
  QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
  for (auto* button : {apply, restart, directRestart}) QVERIFY(dialog.rect().contains(QRect(button->mapTo(&dialog, QPoint()), button->size())));
  QTest::mouseClick(restart, Qt::LeftButton);
  QTest::mouseClick(directRestart, Qt::LeftButton);
  QCOMPARE(safeRestartRequested.count(), 1);
  QCOMPARE(directRestartRequested.count(), 1);
  bool successReported = false;
  for (auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : std::as_const(edits)) {
    successReported = successReported || edit->toPlainText().contains(QStringLiteral("Filter: Disabled"));
  }
  QVERIFY(successReported);

  QTest::mouseClick(apply, Qt::LeftButton);
  QTRY_VERIFY(applyCompleted(dialog));
  QCOMPARE(backend->trace().size(), std::size_t{1});
}

void ApplicationConfigurationTests::applyPlanConnectionFailureRemainsRetryableAndDoesNotRequestReconciliation() {
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  const auto snapshot = editableSnapshot();
  configuration.resetSnapshot(snapshot);
  QCOMPARE((global.viewModel().importFilterEnabled(false)).outcome, uwf::app::GlobalEditOutcome::Applied);
  test::FakeUwfBackend::State state;
  state.current.filterEnabled = snapshot.current.filter.enabled;
  state.next.filterEnabled = snapshot.next.filter.enabled;
  auto backend = std::make_shared<test::FakeUwfBackend>(state);
  backend->inject({.request = compat::SetBoolean{compat::BooleanProperty::FilterEnabled, false},
                   .operation = {},
                   .kind = compat::ErrorKind::Unavailable,
                   .message = "transport unavailable"});
  auto actions = std::make_shared<app::ConfigurationExecutor>(app::uwfConfigurationRepository(backend));
  auto model = std::make_shared<presentation::ApplyPlanViewModel>(configuration.pendingChanges(), snapshot, actions);
  ui::ApplyPlanDialog dialog(model, uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy reconciliation(&dialog, &ui::ApplyPlanDialog::reconciliationRequired);
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(apply && apply->isEnabled());

  QVERIFY(test::driveModalDialog(dialog, [&] { QTest::mouseClick(apply, Qt::LeftButton); }, [](QDialog& confirmation) { confirmation.accept(); }));
  QTRY_VERIFY(applyCompleted(dialog));
  QVERIFY(apply->isEnabled());
  QCOMPARE(reconciliation.count(), 0);
  bool failureReported = false;
  for (auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : std::as_const(edits)) {
    failureReported = failureReported || (edit->toPlainText().contains(QStringLiteral("transport unavailable")) &&
                                          edit->toPlainText().contains(QStringLiteral("Failed to disable filter")));
  }
  QVERIFY(failureReported);
  QCOMPARE(backend->trace().size(), std::size_t{1});
  QCOMPARE(backend->trace().front().response.effect, compat::Effect::None);
  QVERIFY(backend->snapshot().current.filterEnabled);
  QVERIFY(backend->snapshot().next.filterEnabled);

  QVERIFY(test::driveModalDialog(dialog, [&] { QTest::mouseClick(apply, Qt::LeftButton); }, [](QDialog& confirmation) { confirmation.accept(); }));
  QTRY_VERIFY(applyCompleted(dialog));
  QTRY_COMPARE(reconciliation.count(), 1);
  QVERIFY(!apply->isEnabled());
  QCOMPARE(backend->trace().size(), std::size_t{2});
  QVERIFY(!backend->snapshot().next.filterEnabled);
}

void ApplicationConfigurationTests::applyPlanCloseRequestsRespectTheExecutionPhase_data() {
  QTest::addColumn<QString>("phase");
  QTest::addColumn<QString>("entry");
  for (const QString& phase : {QStringLiteral("ready"), QStringLiteral("applying"), QStringLiteral("complete"), QStringLiteral("failed")})
    for (const QString& entry : {QStringLiteral("title"), QStringLiteral("footer"), QStringLiteral("escape"), QStringLiteral("window")})
      QTest::newRow(qUtf8Printable(phase + '-' + entry)) << phase << entry;
}

void ApplicationConfigurationTests::applyPlanCloseRequestsRespectTheExecutionPhase() {
  QFETCH(QString, phase);
  QFETCH(QString, entry);
  class Actions final : public app::ConfigurationActions {
   public:
    std::shared_ptr<test::WorkerGate> gate;
    bool fail = false;
    int prepares = 0;
    int completions = 0;
    ApplyOperation prepareApply(const core::PendingChanges& changes, const core::UwfSnapshot& snapshot) override {
      ++prepares;
      return [gate = gate, fail = fail, operations = app::planConfiguration(changes, snapshot).operations()] {
        gate->wait({}, false);
        app::ConfigurationApplyResult result;
        for (const auto& operation : operations)
          result.steps.push_back({"Applied", fail ? app::ConfigurationStepOutcome::Failed : app::ConfigurationStepOutcome::Succeeded,
                                  fail ? app::WriteEffect::None : app::WriteEffect::Confirmed, operation});
        return result;
      };
    }
    void completeApply(const app::ConfigurationApplyResult&) override { ++completions; }
  };
  auto gate = std::make_shared<test::WorkerGate>();
  const auto release = qScopeGuard([gate] {
    gate->release();
    QThreadPool::globalInstance()->waitForDone(2000);
  });
  auto actions = std::make_shared<Actions>();
  actions->gate = gate;
  actions->fail = phase == QStringLiteral("failed");
  core::PendingChanges changes;
  changes.setFilterEnabled = false;
  auto model = std::make_shared<presentation::ApplyPlanViewModel>(changes, editableSnapshot(), actions);
  ui::ApplyPlanDialog dialog(model, uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* footer = buttonWithText(&dialog, QStringLiteral("Continue editing"));
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(footer && apply);
  QSignalSpy rejected(&dialog, &QDialog::rejected);
  QSignalSpy finished(&dialog, &QDialog::finished);
  QSignalSpy results(model.get(), &presentation::ApplyPlanViewModel::resultChanged);
  const auto requestClose = [&] {
    if (entry == QStringLiteral("title"))
      QTest::mouseClick(dialog.closeButton(), Qt::LeftButton);
    else if (entry == QStringLiteral("footer"))
      QTest::mouseClick(footer, Qt::LeftButton);
    else if (entry == QStringLiteral("escape"))
      QTest::keyClick(&dialog, Qt::Key_Escape);
    else
      dialog.close();
  };
  QVERIFY(dialog.closeButton()->isEnabled());
  QVERIFY(footer->isEnabled());
  if (phase != QStringLiteral("ready")) {
    QVERIFY(test::driveModalDialog(dialog, [&] { QTest::mouseClick(apply, Qt::LeftButton); }, [](QDialog& confirmation) { confirmation.accept(); }));
    const QPointer execution = dialog.findChild<ui::DialogWindow*>(QStringLiteral("applyExecutionConfirmation"));
    QVERIFY(execution);
    QSignalSpy executionFinished(execution.data(), &QDialog::finished);
    QTRY_COMPARE(gate->started.load(), 1);
    QVERIFY(model->applying());
    QVERIFY(!dialog.closeButton()->isEnabled());
    QVERIFY(!footer->isEnabled());
    QVERIFY(!execution->closeButton()->isEnabled());
    if (phase == QStringLiteral("applying")) {
      requestClose();
      execution->reject();
      QTest::keyClick(execution, Qt::Key_Escape);
      execution->close();
      QVERIFY(execution->isVisible());
      QVERIFY(dialog.isVisible());
      QVERIFY(model->applying());
      QCOMPARE(rejected.count(), 0);
      QCOMPARE(finished.count(), 0);
      QCOMPARE(executionFinished.count(), 0);
      QCOMPARE(actions->completions, 0);
    }
    gate->release();
    QTRY_VERIFY(!model->applying());
    QCOMPARE(actions->prepares, 1);
    QCOMPARE(actions->completions, 1);
    QCOMPARE(results.count(), 1);
    QCOMPARE(model->resultFailed(), actions->fail);
    QCOMPARE(apply->isEnabled(), actions->fail);
    // Completion of the real worker does not end a short busy presentation.
    // The confirmation retains modality/close protection until its >=1 s hold
    // ends, then releases the parent. Neither phase can replay the operation.
    if (execution && execution->isVisible()) {
      QVERIFY(!dialog.closeButton()->isEnabled());
      QVERIFY(!footer->isEnabled());
      requestClose();
      QVERIFY(dialog.isVisible());
      QCOMPARE(rejected.count(), 0);
    }
    QTRY_VERIFY(!execution || !execution->isVisible());
    QCOMPARE(executionFinished.count(), 1);
    QTRY_VERIFY(dialog.closeButton()->isEnabled());
    QTRY_VERIFY(footer->isEnabled());
  }
  requestClose();
  QVERIFY(!dialog.isVisible());
  QCOMPARE(rejected.count(), 1);
  QCOMPARE(finished.count(), 1);
  QCOMPARE(finished.front().front().toInt(), static_cast<int>(QDialog::Rejected));
  QCOMPARE(actions->prepares, phase == QStringLiteral("ready") ? 0 : 1);
  QCOMPARE(actions->completions, actions->prepares);
}

void ApplicationConfigurationTests::applyPlanOverlayWritesRespectProviderConstraints_data() {
  QTest::addColumn<QString>("scenario");
  QTest::addColumn<OverlayWriteOutcome>("outcome");
  for (const QString& scenario :
       {QStringLiteral("threshold-raise"), QStringLiteral("threshold-lower"), QStringLiteral("threshold-equality"), QStringLiteral("threshold-enable"),
        QStringLiteral("threshold-disable"), QStringLiteral("threshold-fresh-state"), QStringLiteral("threshold-enable-critical-with-warning"),
        QStringLiteral("threshold-disable-critical-only"), QStringLiteral("threshold-enable-critical-only"), QStringLiteral("ram-small-to-disk"),
        QStringLiteral("ram-large-to-disk"), QStringLiteral("disk-to-small-ram"), QStringLiteral("disk-to-large-ram")}) {
    for (const auto outcome : {OverlayWriteOutcome::Confirmed, OverlayWriteOutcome::Rejected, OverlayWriteOutcome::Uncertain,
                               OverlayWriteOutcome::LostAcknowledgement, OverlayWriteOutcome::Unconfirmed}) {
      const QByteArray name = scenario.toLatin1() + '-' + QByteArray::number(static_cast<int>(outcome));
      QTest::newRow(name.constData()) << scenario << outcome;
    }
  }
}

void ApplicationConfigurationTests::applyPlanOverlayWritesRespectProviderConstraints() {
  QFETCH(QString, scenario);
  QFETCH(OverlayWriteOutcome, outcome);
  const bool thresholds = scenario.startsWith(QStringLiteral("threshold-"));
  core::OverlayConfig baseline{core::OverlayType::RAM, 4096, 512, 768};
  auto target = baseline;
  using Property = compat::NumberProperty;
  std::vector<Property> order;
  if (thresholds) {
    order = {Property::CriticalThreshold, Property::WarningThreshold};
    target.warningThresholdMb = 1024;
    target.criticalThresholdMb = 1536;
    if (scenario == QStringLiteral("threshold-lower")) {
      baseline.warningThresholdMb = 1024;
      baseline.criticalThresholdMb = 1536;
      target.warningThresholdMb = 256;
      target.criticalThresholdMb = 512;
      order = {Property::WarningThreshold, Property::CriticalThreshold};
    } else if (scenario == QStringLiteral("threshold-equality")) {
      target.warningThresholdMb = 768;
      target.criticalThresholdMb = 1024;
    } else if (scenario == QStringLiteral("threshold-enable")) {
      baseline.warningThresholdMb = 0;
      baseline.criticalThresholdMb = 0;
    } else if (scenario == QStringLiteral("threshold-disable")) {
      target.warningThresholdMb = 0;
      target.criticalThresholdMb = 0;
      order = {Property::WarningThreshold, Property::CriticalThreshold};
    } else if (scenario == QStringLiteral("threshold-fresh-state")) {
      target.warningThresholdMb = 600;
    } else if (scenario == QStringLiteral("threshold-enable-critical-with-warning")) {
      baseline.warningThresholdMb = 1024;
      baseline.criticalThresholdMb = 0;
      target.warningThresholdMb = 256;
      target.criticalThresholdMb = 512;
      order = {Property::WarningThreshold, Property::CriticalThreshold};
    } else if (scenario == QStringLiteral("threshold-disable-critical-only")) {
      target.warningThresholdMb = baseline.warningThresholdMb;
      target.criticalThresholdMb = 0;
      order = {Property::CriticalThreshold};
    } else if (scenario == QStringLiteral("threshold-enable-critical-only")) {
      baseline.criticalThresholdMb = 0;
      target.warningThresholdMb = baseline.warningThresholdMb;
      target.criticalThresholdMb = 768;
      order = {Property::CriticalThreshold};
    }
  } else {
    baseline.warningThresholdMb = target.warningThresholdMb = 128;
    baseline.criticalThresholdMb = target.criticalThresholdMb = 256;
    if (scenario.startsWith(QStringLiteral("ram-"))) {
      baseline.maximumSizeMb = scenario == QStringLiteral("ram-small-to-disk") ? 512 : 4096;
      target.type = core::OverlayType::Disk;
      target.maximumSizeMb = 2048;
      order = {Property::MaximumOverlaySize, Property::OverlayType};
    } else {
      baseline.type = core::OverlayType::Disk;
      baseline.maximumSizeMb = 2048;
      target.type = core::OverlayType::RAM;
      target.maximumSizeMb = scenario == QStringLiteral("disk-to-small-ram") ? 512 : 4096;
      order = {Property::OverlayType, Property::MaximumOverlaySize};
    }
  }
  auto snapshot = editableSnapshot();
  snapshot.current.filter.enabled = snapshot.next.filter.enabled = false;
  snapshot.current.overlay = snapshot.next.overlay = baseline;
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  configuration.resetSnapshot(configurationSnapshot(snapshot.current, snapshot.next));
  global.viewModel().importOverlayType(target.type);
  global.viewModel().importOverlayMaxMb(target.maximumSizeMb);
  global.viewModel().importOverlayWarnMb(target.warningThresholdMb);
  global.viewModel().importOverlayCritMb(target.criticalThresholdMb);
  global.viewModel().finishImport();

  auto initial = baseline;
  if (scenario == QStringLiteral("threshold-fresh-state")) {
    initial.warningThresholdMb = 400;
    initial.criticalThresholdMb = 550;
  }
  auto system = std::make_shared<ObservedOverlaySystem>();
  system->backend = std::make_shared<test::FakeUwfBackend>(overlayState(initial));
  system->outcome = outcome;
  auto actions = std::make_shared<app::ConfigurationExecutor>(app::uwfConfigurationRepository(system));
  auto model = std::make_shared<presentation::ApplyPlanViewModel>(configuration.pendingChanges(), snapshot, actions);
  ui::ApplyPlanDialog dialog(model, uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy reconciliation(&dialog, &ui::ApplyPlanDialog::reconciliationRequired);
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(apply && apply->isEnabled());
  QVERIFY(test::driveModalDialog(dialog, [&] { QTest::mouseClick(apply, Qt::LeftButton); }, [](QDialog& confirmation) { confirmation.accept(); }));
  QTRY_VERIFY(applyCompleted(dialog));
  const bool succeeded = outcome == OverlayWriteOutcome::Confirmed;
  const bool hasDependent = order.size() > 1;
  std::vector<compat::Request> expected;
  for (const auto property : order) expected.emplace_back(overlayWrite(property, target));
  if (!succeeded) expected.resize(1);
  QVERIFY(system->writes == expected);
  QCOMPARE(system->reads, 0);
  for (const auto& entry : system->backend->trace()) QVERIFY(compat::isWrite(entry.request));
  auto final = initial;
  if (succeeded)
    final = target;
  else if (outcome == OverlayWriteOutcome::LostAcknowledgement)
    updateOverlay(final, std::get<compat::SetNumber>(expected.front()));
  const auto actual = overlayConfiguration(system->backend->snapshot());
  QCOMPARE(actual.type, final.type);
  QCOMPARE(actual.maximumSizeMb, final.maximumSizeMb);
  QCOMPARE(actual.warningThresholdMb, final.warningThresholdMb);
  QCOMPARE(actual.criticalThresholdMb, final.criticalThresholdMb);
  const bool rejected = outcome == OverlayWriteOutcome::Rejected;
  QCOMPARE(reconciliation.count(), rejected ? 0 : 1);
  QCOMPARE(apply->isEnabled(), rejected);
  QCOMPARE(model->resultFailed(), !succeeded);
  QCOMPARE(model->result().anyWriteConfirmed(), succeeded);
  QCOMPARE(model->result().failedOperations(), succeeded ? 0 : 1);
  QCOMPARE(model->result().dependencySkippedOperations(), succeeded ? 0 : static_cast<int>(order.size()) - 1);
  QCOMPARE(model->result().notAttemptedOperations(), 0);
  QCOMPARE(model->result().batchFailures(), 0);
  if (!succeeded && hasDependent) {
    bool skipReported = false;
    for (auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : std::as_const(edits))
      skipReported = skipReported || edit->toPlainText().contains(ui::I18n::tr("Skipped because a prerequisite operation failed or could not be confirmed."));
    QVERIFY(skipReported);
  }
}

void ApplicationConfigurationTests::applyPlanDependencyFailureIsLocalized_data() {
  QTest::addColumn<ui::I18n::Lang>("language");
  for (const auto& language : ui::I18n::languages()) QTest::newRow(QString(language.code).toLatin1().constData()) << language.id;
}

void ApplicationConfigurationTests::applyPlanDependencyFailureIsLocalized() {
  QFETCH(ui::I18n::Lang, language);
  ui::I18n::instance().setLang(language);
  const auto restoreLanguage = qScopeGuard([] { ui::I18n::instance().setLang(ui::I18n::Lang::En); });
  auto snapshot = editableSnapshot();
  snapshot.current.filter.enabled = snapshot.next.filter.enabled = false;
  snapshot.current.overlay = snapshot.next.overlay = {core::OverlayType::RAM, 512, 128, 256};
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  configuration.resetSnapshot(configurationSnapshot(snapshot.current, snapshot.next));
  global.viewModel().importOverlayType(core::OverlayType::Disk);
  global.viewModel().importOverlayMaxMb(2048);
  global.viewModel().finishImport();
  auto system = std::make_shared<ObservedOverlaySystem>();
  system->backend = std::make_shared<test::FakeUwfBackend>(overlayState(snapshot.next.overlay));
  system->outcome = OverlayWriteOutcome::Rejected;
  auto actions = std::make_shared<app::ConfigurationExecutor>(app::uwfConfigurationRepository(system));
  auto model = std::make_shared<presentation::ApplyPlanViewModel>(configuration.pendingChanges(), snapshot, actions);
  ui::ApplyPlanDialog dialog(model, uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(apply && apply->isEnabled());
  const auto guarded = QStringLiteral("uwfmgr.exe overlay set-size 2048\nuwfmgr.exe overlay set-type Disk");
  if (auto* disclosure = dialog.findChild<ui::Disclosure*>(QStringLiteral("changeCommandsDisclosure"))) disclosure->setOpen(true);
  QTextEdit* pending = nullptr;
  for (auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : std::as_const(edits))
    if (edit->toPlainText().contains(guarded)) pending = edit;
  QVERIFY(pending);
  if (language == ui::I18n::Lang::En) {
    if (auto* disclosure = dialog.findChild<ui::Disclosure*>(QStringLiteral("changeCommandsDisclosure"))) disclosure->setOpen(true);
    pending->selectAll();
    pending->setFocus();
    QTest::keyClick(pending, Qt::Key_C, Qt::ControlModifier);
    const auto copied = QApplication::clipboard()->text();
    QCOMPARE(copied.trimmed(), pending->toPlainText().trimmed());
    QVERIFY(copied.contains(guarded));
  }
  QVERIFY(test::driveModalDialog(dialog, [&] { QTest::mouseClick(apply, Qt::LeftButton); }, [](QDialog& confirmation) { confirmation.accept(); }));
  QTRY_VERIFY(applyCompleted(dialog));
  const std::vector<compat::Request> expected{compat::SetNumber{compat::NumberProperty::MaximumOverlaySize, 2048}};
  QVERIFY(system->writes == expected);
  QVERIFY(model->resultFailed());
  QCOMPARE(model->result().failedOperations(), 1);
  QCOMPARE(model->result().dependencySkippedOperations(), 1);
  QCOMPARE(model->result().notAttemptedOperations(), 0);
  QCOMPARE(model->result().batchFailures(), 0);
  QCOMPARE(system->backend->snapshot().next.maximumOverlaySize, std::uint32_t{512});
  QCOMPARE(system->backend->snapshot().next.overlayType, static_cast<std::uint32_t>(compat::OverlayType::Ram));
  const auto message = ui::I18n::tr("Skipped because a prerequisite operation failed or could not be confirmed.");
  if (language != ui::I18n::Lang::En) QVERIFY(message != QStringLiteral("Skipped because a prerequisite operation failed or could not be confirmed."));
  bool translatedMessageObserved = false;
  for (auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : std::as_const(edits)) translatedMessageObserved |= edit->toPlainText().contains(message);
  QVERIFY(translatedMessageObserved);
}

void ApplicationConfigurationTests::configurationVisualStates_data() { addLocaleThemeRows(); }

void ApplicationConfigurationTests::configurationVisualStates() {
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
  FileDialogStub files;
  auto storeOwner = std::make_shared<MemoryFileStagingStore>();
  auto& store = *storeOwner;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\Data\\state.bin")},
                   {app::FileStagingKind::Directory, QStringLiteral("C:\\Data\\Settings")}};
  auto snapshot = editableSnapshot();
  snapshot.current.filter.enabled = false;
  QWidget host;
  auto* layout = new QHBoxLayout(&host);
  const core::VolumeInfo info{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported};
  app::FileStagingDraft stagingDraft(storeOwner);
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb(), [&stagingDraft] { return !stagingDraft.load().isEmpty(); });
  const auto factory = composition::createViewModelFactory(std::make_shared<test::FakeUwfBackend>(), uwf::test::localDesktopPorts());
  const auto volume = configuration.addVolume(info, true);
  ui::VolumeTab disk(volume, files, factory->fileStaging(QStringLiteral("C:"), stagingDraft, &volume->fileConflicts()), factory);
  ui::GlobalStatusPanel global(configuration.global());
  layout->addWidget(&disk, 1);
  layout->addWidget(&global);
  configuration.resetSnapshot(snapshot);
  host.resize(test::boundedWindowSize(host, QSize(1440, 900)));
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  QTRY_VERIFY(!stagingDraft.busy());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-initial")));

  auto* filter = global.findChild<ui::SwitchButton*>();
  QVERIFY(filter && filter->isChecked());
  QTest::mouseClick(filter, Qt::LeftButton);
  QVERIFY(!filter->isChecked());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-switch-unchecked")));
  QTest::mouseClick(filter, Qt::LeftButton);
  QVERIFY(filter->isChecked());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-switch-checked")));
  filter->setFocus(Qt::TabFocusReason);
  QTRY_VERIFY(filter->hasFocus());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-switch-focus")));

  auto* type = global.findChild<QComboBox*>();
  QVERIFY(type && type->isEnabled());
  type->setFocus(Qt::TabFocusReason);
  QTest::keyClick(type, Qt::Key_Home);
  QCOMPARE(type->currentData().toInt(), static_cast<int>(core::OverlayType::RAM));
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-overlay-ram")));
  QTest::keyClick(type, Qt::Key_End);
  QCOMPARE(type->currentData().toInt(), static_cast<int>(core::OverlayType::Disk));
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-overlay-disk")));

  auto* tabs = disk.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(tabs && tabs->count() == 3);
  auto* filePage = tabs->widget(0)->findChild<ui::ExclusionListWidget*>();
  QVERIFY(filePage);
  auto* list = filePage->findChild<QListView*>();
  QVERIFY(list && list->model()->rowCount() == 1);
  QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualRect(list->model()->index(0, 0)).center());
  QCOMPARE(list->selectionModel()->selectedIndexes().size(), 1);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-file-selected")));
  QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(list->viewport()->width() - 8, list->viewport()->height() - 8));
  QVERIFY(list->selectionModel()->selectedIndexes().isEmpty());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-file-unselected")));

  auto* add = filePage->commandWidget()->findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  auto* remove = buttonWithText(filePage->commandWidget(), ui::I18n::tr("Remove selected"));
  QVERIFY(add && add->findChild<QMenu*>() && remove);
  files.openedFile = QStringLiteral("C:\\VisualNew.txt");
  files.openedFiles = {files.openedFile};
  bool addMenuInspected = false;
  QTimer::singleShot(20, &host, [&] {
    auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    const auto closeMenu = qScopeGuard([menu] {
      if (menu) menu->close();
    });
    QVERIFY(menu && !menu->actions().isEmpty());
    QVERIFY(uwf::test::saveScreenshot(*menu, QStringLiteral("interaction-file-add-menu")));
    auto* action = menu->actions().constFirst();
    QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(action).center());
    addMenuInspected = true;
  });
  QContextMenuEvent addContext(QContextMenuEvent::Mouse, add->rect().center(), add->mapToGlobal(add->rect().center()));
  QApplication::sendEvent(add, &addContext);
  QTRY_VERIFY(addMenuInspected);
  QCOMPARE(list->model()->rowCount(), 2);
  QVERIFY(disk.viewModel().files()->pendingAdded().contains(files.openedFile));
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-file-added")));
  QModelIndex added;
  for (int row = 0; row < list->model()->rowCount(); ++row) {
    if (list->model()->index(row, 0).data(Qt::UserRole).toString() == files.openedFile) added = list->model()->index(row, 0);
  }
  QVERIFY(added.isValid());
  QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualRect(added).center());
  QTest::mouseClick(remove, Qt::LeftButton);
  QVERIFY(disk.viewModel().files()->pendingAdded().isEmpty());
  QCOMPARE(list->model()->rowCount(), 1);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-file-removed")));

  QTest::mouseClick(tabs->tabBar(), Qt::LeftButton, Qt::NoModifier, tabs->tabBar()->tabRect(1).center());
  QCOMPARE(tabs->currentIndex(), 1);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-registry-tab")));
  QTest::mouseClick(tabs->tabBar(), Qt::LeftButton, Qt::NoModifier, tabs->tabBar()->tabRect(2).center());
  QCOMPARE(tabs->currentIndex(), 2);
  auto* staging = disk.findChild<ui::FileStagingWidget*>();
  auto* staged = staging ? uwf::test::stagingEntryList(*staging) : nullptr;
  QVERIFY(staged && staged->model()->rowCount() == 2);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-staging-unselected")));
  QTest::mouseClick(staged->viewport(), Qt::LeftButton, Qt::NoModifier, staged->visualRect(staged->model()->index(0, 0)).center());
  QCOMPARE(staged->selectionModel()->selectedIndexes().size(), 1);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-staging-selected")));
  auto* removeStaged = buttonWithText(staging->commandWidget(), ui::I18n::tr("Remove selected"));
  QVERIFY(removeStaged);
  QTest::mouseClick(removeStaged, Qt::LeftButton);
  QTRY_VERIFY(!staging->viewModel().busy());
  QCOMPARE(staged->model()->rowCount(), 2);
  QCOMPARE(staged->model()->index(0, 0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Removed));
  QCOMPARE(store.entries.size(), 2);
  QCOMPARE(store.writes, 0);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-staging-removed")));
  auto* search = staging->commandWidget()->findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(search);
  search->setFocus();
  QTest::keyClicks(search, QStringLiteral("missing"));
  QCOMPARE(staged->model()->rowCount(), 0);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-search-empty")));
  QTest::keyClick(search, Qt::Key_A, Qt::ControlModifier);
  QTest::keyClick(search, Qt::Key_Backspace);
  QCOMPARE(staged->model()->rowCount(), 2);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-search-cleared")));
  global.viewModel().setControlsEnabled(false);
  QVERIFY(!filter->isEnabled() && !type->isEnabled());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-disabled")));
}

QTEST_MAIN(ApplicationConfigurationTests)
#include "ConfigurationCases.moc"
