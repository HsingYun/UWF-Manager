// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCursor>
#include <QDir>
#include <QElapsedTimer>
#include <QEnterEvent>
#include <QFile>
#include <QFileInfo>
#include <QHoverEvent>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTableView>
#include <QTextEdit>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QWindow>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>

#include "PreviewFixtures.h"
#include "PreviewHost.h"
#include "application/commit/CommitWorkflow.h"
#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/operations/contracts/OperationError.h"
#include "application/overlay/files/OverlayFilesQueryManager.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "composition/desktop/MainWindowComposition.h"
#include "composition/desktop/ViewModelComposition.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/appearance/WindowAppearance.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/models/PageProxyModel.h"
#include "presentation/viewmodels/files/FilePickerViewModel.h"
#include "presentation/viewmodels/staging/FileStagingResultText.h"
#include "presentation/views/commit/CommitInteractionAdapter.h"
#include "presentation/views/commit/CommitOperationDialog.h"
#include "presentation/views/commit/CommitReportDialog.h"
#include "presentation/views/configuration/importing/ImportDialog.h"
#include "presentation/views/configuration/overlay/HormUsageDialog.h"
#include "presentation/views/configuration/overlay/OverlayAdvancedDetailsDialog.h"
#include "presentation/views/configuration/overlay/OverlayAdvancedDialog.h"
#include "presentation/views/configuration/overlay/OverlayMaintenanceDialogs.h"
#include "presentation/views/configuration/review/ApplyPlanDialog.h"
#include "presentation/views/diagnostics/AboutDialog.h"
#include "presentation/views/diagnostics/LogViewerDialog.h"
#include "presentation/views/diagnostics/SystemInformationDialog.h"
#include "presentation/views/enhanced/EnhancedModeDialog.h"
#include "presentation/views/files/FilePickerDialog.h"
#include "presentation/views/overlay/coordination/OverlayContextMenu.h"
#include "presentation/views/overlay/files/OverlayFilesDialog.h"
#include "presentation/views/power/PowerActionDialog.h"
#include "presentation/views/registry/RegistryPickerDialog.h"
#include "presentation/views/shell/MainWindow.h"
#include "presentation/views/shell/TrayController.h"
#include "simulation/app/ConfigurationState.h"
#include "simulation/app/EnhancedMode.h"
#include "simulation/app/MemoryFileStagingStore.h"
#include "simulation/app/OverlayPolicy.h"
#include "simulation/ui/FileDialogStub.h"
#include "simulation/uwf/FakeUwfBackend.h"
using namespace uwf;
using namespace uwf::preview;

int main(int argc, char** argv) {
  QApplication application(argc, argv);
  const auto arguments = application.arguments();
  const auto value = [&](const QString& name, const QString& fallback = QString{}) {
    const auto position = arguments.indexOf(name);
    return position >= 0 && position + 1 < arguments.size() ? arguments[position + 1] : fallback;
  };
  application.setApplicationName("UWF Manager Preview");
  const auto output = value(QStringLiteral("--output"), value(QStringLiteral("--screenshot")));
  const bool interactive =
      arguments.contains(QStringLiteral("--live")) || arguments.contains(QStringLiteral("--smoke-test")) || (output.isEmpty() && host::defaultInteractive());
  if (output.isEmpty() && !interactive) return 2;
  if (!output.isEmpty()) QDir().mkpath(QFileInfo(output).absolutePath());
  const auto page = value(QStringLiteral("--page"), QStringLiteral("main"));
  const auto scenario = value(QStringLiteral("--scenario"), QStringLiteral("normal"));
  const bool applyProgress = page == QLatin1String("review") && scenario.startsWith(QLatin1String("apply-"));
  const bool docs = arguments.contains(QStringLiteral("--docs"));
  const bool visibleCapture = arguments.contains(QStringLiteral("--visible-capture"));
  if (docs && QGuiApplication::platformName() != QLatin1String("windows")) return 6;
  const bool mainPage = page == QLatin1String("main") || page == QLatin1String("overview");
  const bool stagingScenario = mainPage && scenario.startsWith(QLatin1String("staging-"));
  QElapsedTimer reviewTimer;
  QJsonObject reviewTimings;
  ui::I18n::instance().setLang(
      ui::I18n::parseLanguageCode(
          value(QStringLiteral("--language"), arguments.contains(QStringLiteral("--english")) ? QStringLiteral("en") : QStringLiteral("zh_CN")))
          .value_or(ui::I18n::Lang::En));
  ui::ThemeManager::instance().apply(
      arguments.contains(QStringLiteral("--dark")) || (interactive && !arguments.contains(QStringLiteral("--light"))) ? app::Theme::Dark : app::Theme::Light);
  std::unique_ptr<QWidget> documentBackground;
  if (docs) {
    auto* screen = application.primaryScreen();
    const auto available = screen->availableGeometry();
    documentBackground = std::make_unique<QWidget>(nullptr, Qt::Tool | Qt::FramelessWindowHint);
    documentBackground->setStyleSheet(QStringLiteral("background: white;"));
    documentBackground->setGeometry(available);
    documentBackground->show();
  }
  const auto uwfSystem = std::make_shared<uwf::simulation::FakeUwfBackend>();
  auto stagingOwner = std::make_shared<uwf::simulation::application::MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  PreviewApplicationStateSource source;
  auto serviceOwner = std::make_shared<uwf::simulation::application::MemoryEnhancedModeServiceControl>();
  uwf::simulation::application::MemoryEnhancedModeAgent agent;
  source.snapshot = uwf::simulation::application::editableSnapshot();
  source.snapshot.next = source.snapshot.current;
  source.snapshot.current.overlay = {core::OverlayType::RAM, 4096, 3072, 3800};
  source.snapshot.next.overlay = source.snapshot.current.overlay;
  source.snapshot.runtime = {3072, 1024};
  source.snapshot.current.fileExclusions[uwf::simulation::application::fixtureVolumeCName] = {"\\ProgramData\\MyApp\\Logs", "\\Users\\Public\\Documents",
                                                                                              "\\Windows\\Temp", "\\ProgramData\\MyApp\\settings.json"};
  source.snapshot.next.fileExclusions = source.snapshot.current.fileExclusions;
  source.snapshot.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp", "HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager"};
  source.snapshot.next.registryExclusions = source.snapshot.current.registryExclusions;
  source.volumes = {{"C:", uwf::simulation::application::fixtureVolumeCName, "NTFS", "系统", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported},
                    {"D:", uwf::simulation::application::fixtureVolumeDName, "NTFS", "数据", 1ULL << 40, 608ULL << 30, core::VolumeSupport::Supported}};
  if (scenario == QLatin1String("unsupported-exfat"))
    source.volumes.push_back({"E:",
                              "Volume{bd2f3e85-b66d-40ed-9c1a-191b4e83f34c}",
                              "exFAT",
                              "Ventoy",
                              476ULL << 30,
                              472ULL << 30,
                              core::VolumeSupport::FileSystemLimited,
                              {true, false}});
  if (docs) {
    source.snapshot.current.fileExclusions[uwf::simulation::application::fixtureVolumeCName] = {"\\ProgramData\\MyApp\\Logs",
                                                                                                "\\ProgramData\\MyApp\\settings.json",
                                                                                                "\\ProgramData\\DeviceAgent\\Logs",
                                                                                                "\\ProgramData\\DeviceAgent\\state.json",
                                                                                                "\\ProgramData\\Kiosk\\Cache",
                                                                                                "\\ProgramData\\Kiosk\\Uploads",
                                                                                                "\\ProgramData\\Microsoft\\Windows\\WER",
                                                                                                "\\ProgramData\\Terminal\\Receipts",
                                                                                                "\\ProgramData\\Terminal\\Reports",
                                                                                                "\\Users\\Public\\Documents",
                                                                                                "\\Users\\Public\\Pictures",
                                                                                                "\\Windows\\Temp"};
    source.snapshot.next.fileExclusions = source.snapshot.current.fileExclusions;
    if (ui::I18n::instance().lang() == ui::I18n::Lang::En) {
      source.volumes[0].label = "System";
      source.volumes[1].label = "Data";
    }
    staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\Users\\User\\AppData\\Roaming\\MyApp\\preferences.json")},
                       {app::FileStagingKind::Directory, QStringLiteral("C:\\Projects\\Workspace")},
                       {app::FileStagingKind::Directory, QStringLiteral("C:\\Users\\User\\Documents\\Reports")}};
    serviceOwner->current = uwf::simulation::application::completeEnhancedModeStatus();
    agent.connectOnStart = true;
  }
  if (stagingScenario) {
    staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\ProgramData\\DeviceAgent\\state.json")},
                       {app::FileStagingKind::Directory, QStringLiteral("C:\\ProgramData\\DeviceAgent\\Logs")}};
    serviceOwner->current = uwf::simulation::application::completeEnhancedModeStatus();
    agent.connectOnStart = true;
  }
  const auto state = value(QStringLiteral("--state"));
  if (scenario == QLatin1String("hidden-protected")) {
    const auto current = std::string("Volume{aaaaaaaa-1111-2222-3333-444444444444}");
    const auto next = std::string("Volume{bbbbbbbb-1111-2222-3333-444444444444}");
    source.snapshot.current.volumes.push_back({current, {}, true, core::VolumeBinding::VolumeId});
    source.snapshot.next.volumes.push_back({current, {}, false, core::VolumeBinding::VolumeId});
    source.snapshot.next.volumes.push_back({next, {}, true, core::VolumeBinding::VolumeId});
  }
  if (state == QLatin1String("warning")) source.snapshot.runtime = {902, 3194};
  if (state == QStringLiteral("readonly")) source.snapshot.elevated = false;
  if (state == QStringLiteral("unavailable")) {
    source.snapshot.uwfAvailable = false;
    source.snapshot.unavailableReason = "UWF_Overlay.GetOverlayConfig\n0x80041004 · WBEM_E_PROVIDER_FAILURE";
  }
  if (state == QStringLiteral("filter-off")) source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = false;
  if (page == QLatin1String("overlay-advanced") || scenario.startsWith(QLatin1String("overlay-"))) {
    source.policy = uwf::simulation::application::diskPersistentPolicy();
    source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::Disk;
    if (scenario == QLatin1String("readonly")) source.snapshot.elevated = false;
    if (scenario == QLatin1String("dll-failure")) source.policy.transportAvailable = false;
    if (scenario == QLatin1String("read-failure")) {
      source.policy.current.flags = source.policy.next.flags = {app::PolicyObservationStatus::Failed};
      source.policy.resetPersistentState = {app::PolicyObservationStatus::Failed};
    }
    if (scenario == QLatin1String("unsupported")) source.policy.resetPersistentState = {app::PolicyObservationStatus::Unsupported};
    if (scenario == QLatin1String("saved-selector")) source.policy.resetPersistentState.value = 255;
    if (scenario == QLatin1String("reset-pending")) source.policy.resetPersistentState.value = 1;
    if (scenario == QLatin1String("disk-off") || scenario == QLatin1String("ram")) {
      source.snapshot.current.filter.enabled = false;
      source.policy.current.filterEnabled.value = false;
    }
    if (scenario == QLatin1String("ram") || scenario == QLatin1String("active") || scenario == QLatin1String("rom-active")) {
      source.policy.current.overlayType.value = source.policy.next.overlayType.value = 0;
      source.policy.current.flags.value = source.policy.next.flags.value = app::OverlayFlags{scenario == QLatin1String("rom-active") ? 5U : 1U};
      source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::RAM;
    }
  }
  const auto runtimeMode = state.isEmpty() ? scenario : state;
  const bool maintenancePage =
      page == QLatin1String("maintenance") || page == QLatin1String("full-commit") || page == QLatin1String("swap") || page == QLatin1String("horm");
  const bool romMode = runtimeMode.startsWith(QLatin1String("rom-")) || runtimeMode.startsWith(QLatin1String("horm-")) ||
                       page == QLatin1String("full-commit") || page == QLatin1String("horm");
  const bool swapMode = runtimeMode.startsWith(QLatin1String("swap-")) || page == QLatin1String("swap");
  if (romMode || swapMode || maintenancePage) {
    source.policy = uwf::simulation::application::diskPersistentPolicy();
    source.snapshot.current.filter.enabled = !swapMode;
    source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::RAM;
    source.policy.current.filterEnabled.value = !swapMode;
    source.policy.current.overlayType.value = source.policy.next.overlayType.value = 0;
    source.policy.current.flags.value = app::OverlayFlags{romMode && runtimeMode != QLatin1String("rom-next") ? 4U : 0U};
    source.policy.next.flags.value = app::OverlayFlags{romMode ? 4U : 0U};
    if (runtimeMode.startsWith(QLatin1String("horm-")) || page == QLatin1String("horm")) {
      for (auto& volume : source.snapshot.current.volumes) volume.isProtected = true;
      for (auto& volume : source.snapshot.next.volumes) volume.isProtected = true;
    }
  }
  if (runtimeMode == QLatin1String("horm-filter-off")) {
    source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = false;
    source.policy.current.filterEnabled.value = source.policy.next.filterEnabled.value = false;
  }
  if (stagingScenario) {
    const bool alreadyPersistent = scenario == QLatin1String("staging-already-persistent");
    source.policy = uwf::simulation::application::diskPersistentPolicy();
    source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::Disk;
    source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = !alreadyPersistent;
    source.policy.current.filterEnabled.value = source.policy.next.filterEnabled.value = !alreadyPersistent;
  }
  const auto system = std::make_shared<PreviewSystemInformation>();
  auto ports = host::desktopPorts();
  ports.systemInformation = system;
  ports.volumes = std::make_shared<PreviewVolumeRepository>(source.volumes);
  if (stagingScenario) {
    ports.traversal = std::make_shared<PreviewCommitTraversalSource>();
    ports.files = std::make_shared<PreviewFileBrowser>();
  }
  auto runtimeState = previewUwfState({source.volumes, source.snapshot, source.policy}, system->systemDriveLetter().toStdString());
  runtimeState.runtime.hormEnabled = runtimeMode == QLatin1String("horm-active") || runtimeMode == QLatin1String("horm-disable");
  if (runtimeState.runtime.hormEnabled) runtimeState.runtime.romDisposition = 2;
  if (runtimeMode == QLatin1String("rom-commit-pending")) runtimeState.runtime.romDisposition = 4;
  if (runtimeMode == QLatin1String("horm-no-hiberfile")) runtimeState.runtime.hiberfileExists = false;
  if (runtimeMode == QLatin1String("horm-unprotected"))
    runtimeState.runtime.hormFixedVolumeSafety = uwf::simulation::FakeUwfBackend::SafetyObservation::Rejected;
  if (runtimeMode == QLatin1String("horm-state-changed")) runtimeState.runtime.romDisposition = 3;
  if (runtimeMode == QLatin1String("swap-selected") || runtimeMode == QLatin1String("swap-pending")) {
    runtimeState.next.overlayType = 1;
    runtimeState.next.volumes.at(compat::normalizeVolumeId(uwf::simulation::application::fixtureVolumeDName)).swapfileSize = 1;
  }
  const auto seedPreviewSystem = [uwfSystem, runtimeMode, &source](uwf::simulation::FakeUwfBackend::State state) {
    uwfSystem->seed(std::move(state));
    auto facts = uwfSystem->state();
    std::vector<std::string> targets;
    for (const auto& [id, configured] : facts.next.volumes)
      if (configured.present.value == true) targets.push_back(id);
    // Include an additional partition identity outside the picker inventory.
    targets.push_back("dddddddd-eeee-ffff-aaaa-444444444444");
    if (runtimeMode == QLatin1String("horm-unprotected")) {
      targets.insert(targets.end(), {"66666666-7777-8888-9999-aaaaaaaaaaaa", "22222222-3333-4444-5555-666666666666", "aaaaaaaa-bbbb-cccc-dddd-111111111111",
                                     "bbbbbbbb-cccc-dddd-eeee-222222222222", "cccccccc-dddd-eeee-ffff-333333333333"});
    }
    std::ranges::sort(targets);
    targets.erase(std::unique(targets.begin(), targets.end()), targets.end());
    facts.runtime.protectScope = compat::Fact<std::vector<std::string>>::known(std::move(targets));
    uwfSystem->updateState(std::move(facts));
    // Keep the core UI snapshot and provider facts in agreement through the
    // production projection, including the system-volume GUID/protection.
    source.snapshot.runtime = app::publishedUwfSnapshot(uwfSystem->state(), app::UwfCapability::Available, source.snapshot.elevated,
                                                        {source.snapshot.runtime.currentConsumptionMb, source.snapshot.runtime.availableSpaceMb})
                                  .runtime;
  };
  seedPreviewSystem(std::move(runtimeState));
  // Replace all production maintenance observations with preview fixtures.
  ports.maintenanceObservations = {};

  ports.maintenanceObservations.hibernationCapabilities = [uwfSystem, runtimeMode](const std::stop_token stop) {
    if (stop.stop_requested()) throw std::runtime_error("Preview power-capability observation canceled");
    return app::OverlayHibernationCapabilities{
        runtimeMode != QLatin1String("horm-no-s4"), uwfSystem->snapshot().runtime.hiberfileExists,
        runtimeMode == QLatin1String("horm-reduced-hiberfile") ? app::HibernationFileType::Reduced : app::HibernationFileType::Full};
  };

  ports.maintenanceObservations.hormVolumeProtection = [uwfSystem](const std::stop_token stop) {
    if (stop.stop_requested()) throw std::runtime_error("Preview observation canceled");
    const auto safety = uwfSystem->snapshot().runtime.hormFixedVolumeSafety;
    if (safety == uwf::simulation::FakeUwfBackend::SafetyObservation::Unknown)
      throw std::runtime_error("The preview runtime volume observation is unavailable");
    const bool protectedVolumes = safety == uwf::simulation::FakeUwfBackend::SafetyObservation::Satisfied;
    app::OverlayHormVolumeProtection result{protectedVolumes, {}, {}};
    if (!protectedVolumes) {
      // Runtime blockers are layout fixtures independent of the storage-picker inventory.
      result.blockedVolumes = {{QStringLiteral("66666666-7777-8888-9999-aaaaaaaaaaaa"), QStringLiteral("D:")},
                               {QStringLiteral("22222222-3333-4444-5555-666666666666"), QStringLiteral("F:")},
                               {QStringLiteral("aaaaaaaa-bbbb-cccc-dddd-111111111111"), {}},
                               {QStringLiteral("bbbbbbbb-cccc-dddd-eeee-222222222222"), {}},
                               {QStringLiteral("cccccccc-dddd-eeee-ffff-333333333333"), {}}};
    }
    return result;
  };
  if (runtimeMode == QLatin1String("rom-state-unknown"))
    uwfSystem->setCapability(compat::GetBoolean{compat::BooleanProperty::OverlayCommit}, {compat::Support::Unavailable, "Fixture read failure"});
  if (runtimeMode == QLatin1String("rom-commit-unavailable"))
    uwfSystem->setCapability(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true}, {compat::Support::Unsupported, "Fixture unsupported"});
  if (runtimeMode == QLatin1String("horm-unknown") || runtimeMode == QLatin1String("mode-unknown"))
    uwfSystem->setCapability(compat::GetBoolean{compat::BooleanProperty::HormEnabled}, {compat::Support::Unavailable, "Fixture read failure"});
  if (runtimeMode == QLatin1String("horm-unavailable"))
    uwfSystem->setCapability(compat::SetBoolean{compat::BooleanProperty::HormEnabled, true}, {compat::Support::Unsupported, "Fixture unsupported"});
  source.backend = uwfSystem;
  source.hormObservationUnavailable = runtimeMode == QLatin1String("horm-unknown") || runtimeMode == QLatin1String("mode-unknown");
  auto services = composition::mainWindowServices(uwfSystem, source, stagingOwner, ports, serviceOwner, &agent);
  services.viewModelFactory = composition::createViewModelFactory(uwfSystem, ports);
  const auto factory = services.viewModelFactory;
  ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available,
                                              .compatibilityMode = state == QLatin1String("compatibility"),
                                              .hubVisible = docs && page == QLatin1String("overview")});
  uwf::simulation::FileDialogStub fileDialogs;
  PreviewEnhancedActions enhanced;
  enhanced.current.agentState = app::EnhancedModeAgentState::Connected;
  const auto registry = std::make_shared<PreviewRegistryBrowser>();
  std::unique_ptr<QMenu> menu;
  std::unique_ptr<QDialog> panel;
  std::shared_ptr<presentation::ApplyPlanViewModel> reviewModel;
  std::unique_ptr<QWidget> component;
  if (maintenancePage) {
    const auto maintenance = factory->overlayMaintenance();
    maintenance->setInteractionContext(source.snapshot.elevated, false);
    maintenance->refresh();
    if (page == QLatin1String("full-commit"))
      panel = std::make_unique<ui::FullCommitDialog>(maintenance);
    else if (page == QLatin1String("swap"))
      panel = std::make_unique<ui::SwapDialog>(maintenance);
    else if (page == QLatin1String("horm"))
      panel = std::make_unique<ui::HormDialog>(maintenance);
    else {
      auto editor = std::make_shared<app::OverlayPolicyEditor>();
      editor->reset(source.policy, source.snapshot);
      auto advanced = std::make_shared<presentation::OverlayAdvancedViewModel>(std::move(editor));
      auto dialog = std::make_unique<ui::OverlayAdvancedDialog>(std::move(advanced), maintenance);
      dialog->findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"))->setCurrentIndex(1);
      panel = std::move(dialog);
    }
  } else if (page == QLatin1String("horm-usage")) {
    panel = std::make_unique<ui::HormUsageDialog>();
  } else if (page == QLatin1String("overlay-advanced")) {
    auto editor = std::make_shared<app::OverlayPolicyEditor>();
    editor->reset(source.policy, source.snapshot);
    if (scenario == QLatin1String("conflict-type")) editor->setOverlayChanges({.type = core::OverlayType::RAM});
    if (scenario == QLatin1String("conflict-reset")) {
      editor->importChanges({.persistent = false, .reset = app::PersistentReset::Clear});
    }
    if (scenario == QLatin1String("draft")) {
      editor->importChanges({.passthrough = false});
    }
    auto model = std::make_shared<presentation::OverlayAdvancedViewModel>(std::move(editor));
    const auto detail = value(QStringLiteral("--detail"));
    if (detail.isEmpty()) {
      auto maintenance = factory->overlayMaintenance();
      maintenance->setInteractionContext(source.snapshot.elevated, false);
      maintenance->refresh();
      panel = std::make_unique<ui::OverlayAdvancedDialog>(std::move(model), std::move(maintenance));
    } else {
      using Details = ui::OverlayAdvancedDetailsDialog;
      const auto field = value(QStringLiteral("--field"));
      const auto selected = field == QLatin1String("persistent") ? Details::Field::Persistent
                            : field == QLatin1String("reset")    ? Details::Field::Reset
                            : field == QLatin1String("rom")      ? Details::Field::ReadOnlyMedia
                                                                 : Details::Field::Passthrough;
      const auto purpose = detail == QLatin1String("reason")      ? Details::Purpose::Restriction
                           : detail == QLatin1String("conflicts") ? Details::Purpose::Conflicts
                                                                  : Details::Purpose::Feature;
      panel = std::make_unique<Details>(model, model->beginEdit(), purpose, selected);
    }
  }
  QWidget trayOwner;
  std::unique_ptr<ui::TrayController> trayPreview;
  QMenu* productionTrayMenu = nullptr;
  if (page == QLatin1String("picker")) {
    auto browser = std::make_shared<PreviewFileBrowser>();
    browser->scenario = scenario;
    using Mode = presentation::FilePickerViewModel::Mode;
    const auto mode = scenario == QLatin1String("folder")     ? Mode::Directory
                      : scenario == QLatin1String("mixed")    ? Mode::Mixed
                      : scenario == QLatin1String("save")     ? Mode::SaveFile
                      : scenario == QLatin1String("multiple") ? Mode::OpenFiles
                                                              : Mode::OpenFile;
    auto model = std::make_shared<presentation::FilePickerViewModel>(mode, browser);
    const auto scope = value(QStringLiteral("--scope"));
    if (!scope.isEmpty()) model->setAllowedRoots({scope});
    model->setFilters(ui::I18n::tr("Text and scripts (*.txt *.bat *.ps1 *.cmd);;Text files (*.txt);;All files (*)"));
    panel = std::make_unique<ui::FilePickerDialog>(model, scenario == QLatin1String("folder")     ? ui::I18n::tr("Select folder")
                                                          : scenario == QLatin1String("mixed")    ? ui::I18n::tr("Add files or folders")
                                                          : scenario == QLatin1String("save")     ? ui::I18n::tr("Save file")
                                                          : scenario == QLatin1String("multiple") ? ui::I18n::tr("Select files")
                                                                                                  : ui::I18n::tr("Open configuration file"));
    model->start((scope.startsWith(QLatin1String("D:"), Qt::CaseInsensitive) ? QStringLiteral("D:/") : QStringLiteral("C:/Users/User/Documents")) +
                 (mode == Mode::SaveFile ? QStringLiteral("/uwfmgr-commands.txt") : QString{}));
  }
  if (page == QStringLiteral("enhanced")) {
    enhanced.current.agentState = app::EnhancedModeAgentState::Connected;
    if (scenario == QLatin1String("disabled")) enhanced.current = {};
    if (scenario == QLatin1String("stopped")) {
      enhanced.current.state = app::EnhancedModeState::Stopped;
      enhanced.current.running = false;
    }
    if (scenario == QLatin1String("repair")) {
      enhanced.current.state = app::EnhancedModeState::RepairRequired;
      enhanced.current.executableMatches = false;
    }
    enhanced.unavailable = scenario == QLatin1String("error");
    auto model = factory->enhancedMode(enhanced);
    if (scenario.startsWith(QLatin1String("automatic-"))) {
      const auto kind = scenario.mid(10);
      const auto summary = previewAutomaticCommitSummary(kind == QLatin1String("mixed")           ? QStringLiteral("mixed-retained")
                                                         : kind == QLatin1String("mixed-reasons") ? kind
                                                                                                  : QStringLiteral("skip-") + kind);
      const auto restriction = kind == QLatin1String("horm")                                         ? app::RuntimeWriteRestriction::HormActive
                               : kind == QLatin1String("fullcommit") || kind == QLatin1String("rom") ? app::RuntimeWriteRestriction::ReadOnlyMediaActive
                                                                                                     : app::RuntimeWriteRestriction::None;
      model->setWriteRestriction(restriction);
      model->setAutomaticCommitSummary(summary);
    }
    panel = std::make_unique<ui::EnhancedModeDialog>(std::move(model));
  }
  if (page == QStringLiteral("registry")) {
    registry->failure = scenario == QLatin1String("error");
    const auto mode = scenario != QLatin1String("value") && scenario != QLatin1String("delete") ? ui::RegistryPickerDialog::Mode::Exclusion
                      : scenario == QLatin1String("delete")                                     ? ui::RegistryPickerDialog::Mode::DeleteValue
                                                                                                : ui::RegistryPickerDialog::Mode::CommitValue;
    auto dialog = std::make_unique<ui::RegistryPickerDialog>(std::make_shared<presentation::RegistryPickerViewModel>(mode, registry),
                                                             mode == ui::RegistryPickerDialog::Mode::Exclusion ? ui::I18n::tr("Add registry exclusion")
                                                             : scenario == QLatin1String("delete") ? ui::I18n::tr("Select registry key or value to delete")
                                                                                                   : ui::I18n::tr("Select registry key or value"));
    dialog->preselectKey(QStringLiteral("HKLM\\SOFTWARE\\MyApp"));
    panel = std::move(dialog);
  }
  if (page == QStringLiteral("review")) {
    const bool hormDisable = scenario == QLatin1String("horm-disable");
    const bool hormEnable = scenario == QLatin1String("horm-enable");
    const bool fullCommit = scenario == QLatin1String("full-commit");
    const bool storageChange = scenario == QLatin1String("storage-change");
    const bool resetClear = scenario == QLatin1String("reset-clear");
    const bool resetPreserve = scenario == QLatin1String("reset-preserve");
    const bool resetReview = resetClear || resetPreserve;
    const bool nativeMaintenance = hormDisable || hormEnable || fullCommit || storageChange;
    if (nativeMaintenance) {
      source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = !storageChange;
      source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::RAM;
      source.policy = uwf::simulation::application::diskPersistentPolicy();
      source.policy.current.filterEnabled.value = source.policy.next.filterEnabled.value = !storageChange;
      source.policy.current.overlayType.value = source.policy.next.overlayType.value = 0;
      source.policy.current.flags.value = source.policy.next.flags.value = app::OverlayFlags{storageChange ? 0U : 4U};
      source.snapshot.runtime.hormEnabled = hormDisable;
      source.snapshot.runtime.fullCommit = false;
      source.snapshot.runtime.romDisposition = hormDisable ? 2U : 0U;
      source.snapshot.runtime.currentOverlayFlags = storageChange ? 0U : 4U;
      source.snapshot.runtime.systemVolumeProtected = true;
    } else {
      source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = false;
    }
    if (resetReview) {
      source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = true;
      source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::Disk;
      source.policy = uwf::simulation::application::diskPersistentPolicy();
      source.policy.resetPersistentState.value = resetClear ? 0 : 1;
      source.snapshot.runtime.hormEnabled = source.snapshot.runtime.fullCommit = false;
      source.snapshot.runtime.romDisposition = 0;
      source.snapshot.runtime.currentOverlayFlags = source.policy.current.flags.value->raw;
      source.snapshot.runtime.systemVolumeProtected = true;
    }
    source.snapshot.current.overlay.maximumSizeMb = source.snapshot.next.overlay.maximumSizeMb = 2048;
    source.snapshot.current.overlay.warningThresholdMb = source.snapshot.next.overlay.warningThresholdMb = 1536;
    source.snapshot.current.overlay.criticalThresholdMb = source.snapshot.next.overlay.criticalThresholdMb = 1900;
    core::PendingChanges changes;
    if (!nativeMaintenance && !resetReview && scenario != QLatin1String("empty") && !scenario.startsWith(QLatin1String("staging-")) &&
        !scenario.startsWith(QLatin1String("bulk-"))) {
      changes.setFilterEnabled = true;
      changes.setOverlay.maximumSizeMb = 4096;
      changes.setOverlay.criticalThresholdMb = 3800;
      changes.addFileExclusions["C:"] = {"C:\\ProgramData\\MyApp"};
    }
    std::shared_ptr<app::FileStagingDraft> draft;
    const int reviewEntries = value(QStringLiteral("--review-entries"), QStringLiteral("0")).toInt();
    for (int index = 0; index < reviewEntries; ++index) changes.addFileExclusions["C:"].push_back("C:\\ProgramData\\Preview\\entry-" + std::to_string(index));
    if (scenario.startsWith(QLatin1String("staging-"))) {
      const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\Users\\User\\Documents\\settings.json")},
                                                 {app::FileStagingKind::Directory, QStringLiteral("D:\\Data\\Project")}};
      source.snapshot.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp"};
      source.snapshot.next.registryExclusions = source.snapshot.current.registryExclusions;
      if (scenario != QLatin1String("staging-add")) {
        staging.entries = entries;
        source.snapshot.current.registryExclusions.push_back(std::string(app::kFileStagingRegistryRoot));
        source.snapshot.next.registryExclusions = source.snapshot.current.registryExclusions;
      }
      draft = std::make_shared<app::FileStagingDraft>(stagingOwner);
      static_cast<void>(draft->load());
      draft->replace(scenario == QLatin1String("staging-add")       ? entries
                     : scenario == QLatin1String("staging-partial") ? QList<app::FileStagingEntry>{entries.back()}
                                                                    : QList<app::FileStagingEntry>{});
    }
    reviewTimer.start();
    app::ConfigurationChanges all{changes, {}};
    if (resetReview) all.overlayPolicy.reset = resetClear ? app::PersistentReset::Clear : app::PersistentReset::Preserve;
    std::optional<app::OverlayMaintenanceSnapshot> maintenanceFacts;
    if (nativeMaintenance) {
      if (hormDisable || hormEnable) all.maintenance.hormEnabled = hormEnable;
      if (fullCommit) all.maintenance.fullCommit = true;
      if (storageChange) all.maintenance.storageVolume = QString::fromLatin1(uwf::simulation::application::fixtureVolumeDName);
      maintenanceFacts.emplace();
      maintenanceFacts->filterEnabled = source.snapshot.current.filter.enabled;
      maintenanceFacts->hormEnabled = hormDisable;
      maintenanceFacts->overlayCommit = false;
      maintenanceFacts->systemVolumeProtected = source.snapshot.runtime.systemVolumeProtected;
      maintenanceFacts->currentFlags = maintenanceFacts->nextFlags = app::OverlayFlags{storageChange ? 0U : 4U};
      maintenanceFacts->currentOverlayType = maintenanceFacts->nextOverlayType = 0;
      maintenanceFacts->maximumOverlaySize = 2048;
      maintenanceFacts->romDisposition = hormDisable ? 2U : 0U;
      maintenanceFacts->hibernationAvailable = true;
      maintenanceFacts->allVolumesProtected = std::ranges::all_of(source.snapshot.current.volumes, [](const auto& volume) { return volume.isProtected; });
      maintenanceFacts->volumesReadable = true;
      maintenanceFacts->hormCapability.support = maintenanceFacts->fullCommitCapability.support = maintenanceFacts->swapCapability.support =
          app::OverlayMaintenanceSupport::Supported;
      maintenanceFacts->volumes.push_back(
          {QString::fromLatin1(uwf::simulation::application::fixtureVolumeDName), QStringLiteral("D:"), {}, 608ULL << 30, true, {}, {}});
    }
    if (scenario.startsWith(QLatin1String("bulk-"))) {
      all.maintenance.allVolumesProtected = scenario == QLatin1String("bulk-protect");
      source.snapshot.next.volumes.push_back({"Volume{aaaaaaaa-bbbb-cccc-dddd-111111111111}", {}, true, core::VolumeBinding::VolumeId});
      maintenanceFacts.emplace();
      maintenanceFacts->systemVolumeProtected = source.snapshot.runtime.systemVolumeProtected;
      maintenanceFacts->protectAllCapability.support = maintenanceFacts->unprotectAllCapability.support = app::OverlayMaintenanceSupport::Supported;
      maintenanceFacts->unprotectScopeConfirmed = true;
      for (const auto& volume : source.snapshot.next.volumes) maintenanceFacts->unprotectTargetVolumes.push_back(QString::fromStdString(volume.volumeName));
    }
    if (scenario.startsWith(QLatin1String("overlay-"))) {
      all.overlayPolicy.passthrough = false;
      if (scenario == QLatin1String("overlay-clear")) all.overlayPolicy.reset = app::PersistentReset::Clear;
      if (scenario == QLatin1String("overlay-cancel")) {
        source.policy.resetPersistentState.value = 1;
        all.overlayPolicy.reset = app::PersistentReset::Preserve;
      }
    }
    seedPreviewSystem(previewUwfState({source.volumes, source.snapshot, source.policy}, system->systemDriveLetter().toStdString()));
    auto model = factory->applyPlan(all, source.snapshot, source.policy, std::move(draft), std::move(maintenanceFacts));
    if (applyProgress) {
      auto actions = std::make_shared<PreviewApplyActions>();
      actions->workerDelayMs = scenario == QLatin1String("apply-long") ? 5000 : 0;
      actions->fail = scenario == QLatin1String("apply-failure");
      model = std::make_shared<presentation::ApplyPlanViewModel>(changes, source.snapshot, std::move(actions));
    }
    reviewTimings.insert(QStringLiteral("modelMs"), static_cast<double>(reviewTimer.nsecsElapsed()) / 1000000.0);
    reviewTimer.restart();
    reviewModel = model;
    panel = std::make_unique<ui::ApplyPlanDialog>(std::move(model), fileDialogs);
    reviewTimings.insert(QStringLiteral("constructMs"), static_cast<double>(reviewTimer.nsecsElapsed()) / 1000000.0);
  }
  if (page == QStringLiteral("hub")) {
    const int percent = scenario == QLatin1String("critical") ? 94 : scenario == QLatin1String("warning") ? 78 : 25;
    const auto used = static_cast<uint32_t>(4096 * percent / 100);
    const app::OverlayUsageEnabled usage({4096U - used, used}, {core::OverlayType::RAM, 4096, 3072, 3800});
    component = host::overlayComponent(value(QStringLiteral("--component")), usage);
    auto context = std::make_unique<ui::OverlayContextMenu>();
    context->setOperationAvailability(true, true);
    context->setRestorePositionAvailable(scenario == QLatin1String("floating"));
    context->setHubState(true, true);
    if (scenario != QLatin1String("floating") && scenario != QLatin1String("taskbar")) {
      trayPreview = std::make_unique<ui::TrayController>(&trayOwner);
      trayPreview->applyUsageState(usage);
      trayPreview->setHubState(true, true);
      trayPreview->setOperationAvailability(true, true);
      productionTrayMenu = trayOwner.findChild<ui::OverlayContextMenu*>();
    }
    menu = std::move(context);
  }
  if (page == QStringLiteral("import")) {
    auto model = factory->importCommands();
    if (scenario != QLatin1String("empty"))
      model->setText(
          QStringLiteral("uwfmgr filter enable\nuwfmgr overlay set-type RAM\nuwfmgr overlay set-size 4096\nuwfmgr volume protect C:\n"
                         "uwfmgr file add-exclusion \"C:\\ProgramData\\MyApp\"\nuwfmgr registry add-exclusion HKLM\\SOFTWARE\\MyApp"));
    if (scenario == QLatin1String("multiline")) {
      QStringList lines;
      for (int line = 1; line <= 24; ++line) lines.append(line % 4 == 0 ? QString{} : QStringLiteral("line %1  ·  中文输入与 English 123").arg(line));
      model->setText(lines.join(u'\n'));
    }
    panel = std::make_unique<ui::ImportDialog>(model, fileDialogs);
    if (scenario == QLatin1String("report")) {
      model->setApplier([](const QList<app::UwfmgrCommand>& commands) {
        QList<app::ImportResult> results;
        for (int index = 0; index < commands.size(); ++index) {
          const auto& command = commands[index];
          app::ImportResult result;
          result.lineNo = command.sourceLineNo;
          result.lineText = QString::fromStdString(command.rawLine);
          result.status = index == 2   ? app::ImportResult::Status::Duplicate
                          : index == 3 ? app::ImportResult::Status::Unsupported
                          : index == 4 ? app::ImportResult::Status::Failed
                                       : app::ImportResult::Status::Success;
          result.detail = index == 2   ? QStringLiteral("已在待应用变更中")
                          : index == 3 ? QStringLiteral("查询命令无需导入")
                          : index == 4 ? QStringLiteral("参数无效：请输入可用的覆盖层大小")
                                       : QStringLiteral("已加入待应用变更");
          results.append(result);
        }
        return results;
      });
      model->setText(
          QStringLiteral("uwfmgr filter enable\nuwfmgr volume protect C:\nuwfmgr filter enable\nuwfmgr overlay get-consumption\nuwfmgr overlay set-size bad"));
      model->importText();
    }
  }
  const auto diagnosticDocument = [&] {
    app::DiagnosticReportData report;
    report.context = app::DiagnosticReportService::capture(app::UwfCapability::Available, &source.snapshot, &enhanced.current, false);
    report.context.uwf.protectedVolumes = {QStringLiteral("C: · NTFS")};
    report.system.windows = system->windowsVersion();
    report.system.architecture = QStringLiteral("x64");
    report.processArchitecture = QStringLiteral("x64");
    report.version = QStringLiteral("1.0.19");
    report.qtRuntime = QStringLiteral("6.12.0");
    report.qtBuild = report.qtRuntime;
    if (scenario == QLatin1String("failure")) report.system.windowsError = QStringLiteral("Partial inventory failure");
    return presentation::DiagnosticReportViewModel(report, scenario == QLatin1String("failure") ? 0 : 65536);
  };
  if (page == QStringLiteral("about")) {
    auto about = std::make_unique<ui::AboutDialog>(diagnosticDocument);
    const auto exportPath = value(QStringLiteral("--export-path"), QStringLiteral("C:/Program Files/UWF Manager/Service/UWF.ini"));
    const auto exportError = value(QStringLiteral("--export-error"), QStringLiteral("Access denied (0x00000005)."));
    about->setConfigurationExporter([scenario, exportPath, exportError] {
      using Outcome = app::TextFileCreationResult::Outcome;
      const auto outcome = scenario == QLatin1String("about-exists")    ? Outcome::AlreadyExists
                           : scenario == QLatin1String("about-failure") ? Outcome::Failed
                                                                        : Outcome::Created;
      return presentation::UserConfigExportViewModel::Result{outcome, exportPath, outcome == Outcome::Failed ? exportError : QString{}};
    });
    panel = std::move(about);
  }
  if (page == QStringLiteral("system")) panel = std::make_unique<ui::SystemInformationDialog>(diagnosticDocument());
  if (page == QStringLiteral("logs"))
    panel = std::make_unique<ui::LogViewerDialog>(std::make_shared<presentation::LogViewModel>(std::make_shared<PreviewLogSource>(scenario)));
  if (page == QStringLiteral("overlay")) {
    auto model = std::make_shared<presentation::OverlayFilesViewModel>(
        QStringLiteral("C:"), QStringLiteral("11111111-2222-3333-4444-555555555555"),
        std::make_shared<app::OverlayFilesQueryManager>([scenario, docs](const QString&, std::stop_token) -> QVector<app::OverlayFileEntry> {
          if (scenario == QLatin1String("failure"))
            throw app::OperationError(std::make_error_code(std::errc::no_such_device), "read overlay files\n0x80010105\npreview provider failure");
          if (scenario == QLatin1String("empty")) return {};
          return {{QStringLiteral("\\ProgramData\\MyApp\\settings.json"), {}, false, false, 2048},
                  {QStringLiteral("\\Users\\User\\AppData\\Roaming\\Code\\User\\settings.json"), {}, false, false, 4096},
                  {docs ? QStringLiteral("\\Users\\User\\Documents\\Reports\\monthly-report.docx") : QStringLiteral("\\Users\\User\\Documents\\工作记录.docx"),
                   {},
                   false,
                   false,
                   285184},
                  {QStringLiteral("\\ProgramData\\MyApp\\Data"), {}, true, false, 0},
                  {QStringLiteral("\\$Mft"), {}, false, true, 25165824},
                  {QStringLiteral("\\Windows\\Temp\\session.log"), {}, false, false, 16384}};
        }),
        [](const QString&, const QString&) { return app::TextFileWriteResult{}; });
    const bool writable = source.snapshot.uwfAvailable && source.snapshot.elevated && source.snapshot.current.filter.enabled;
    model->setCommitAvailability(
        writable, !source.snapshot.uwfAvailable ? ui::I18n::tr("UWF is currently unavailable")
                  : !source.snapshot.elevated   ? ui::I18n::tr("Read-only")
                  : !source.snapshot.current.filter.enabled
                      ? ui::I18n::tr("The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.")
                      : QString{});
    panel = std::make_unique<ui::OverlayFilesDialog>(std::move(model), fileDialogs);
    auto* overlay = static_cast<ui::OverlayFilesDialog*>(panel.get());
    QObject::connect(overlay, &ui::OverlayFilesDialog::commitFileRequested, overlay, [uwfSystem, overlay, ports](const QString& path) {
      app::CommitWorkflow workflow(app::uwfMaintenanceRepository(uwfSystem), ui::commitInteraction(overlay, {}, *ports.fileDialogs), ports.traversal);
      workflow.commitFilePath(path);
    });
  }
  QWidget* root = component            ? component.get()
                  : productionTrayMenu ? productionTrayMenu
                  : menu               ? static_cast<QWidget*>(menu.get())
                  : panel              ? static_cast<QWidget*>(panel.get())
                                       : &window;
  if (arguments.contains(QStringLiteral("--solid"))) root->setProperty("systemBackdropForceSolid", true);
  const bool power = page == QStringLiteral("shutdown") || page == QStringLiteral("restart");
  const bool aboutFeedback = page == QLatin1String("about") && (scenario == QLatin1String("about-save") || scenario == QLatin1String("about-exists") ||
                                                                scenario == QLatin1String("about-failure"));
  const bool blocking = power || page == QStringLiteral("commit") || page == QStringLiteral("dialogs");
  reviewTimer.start();
  if (!blocking) root->show();
  if (!reviewTimings.isEmpty()) {
    reviewTimings.insert(QStringLiteral("showMs"), static_cast<double>(reviewTimer.nsecsElapsed()) / 1000000.0);
    QTimer::singleShot(0, &application, [&] {
      reviewTimings.insert(QStringLiteral("firstEventLoopMs"), static_cast<double>(reviewTimer.nsecsElapsed()) / 1000000.0);
      QFile timings(output + QStringLiteral(".timings.json"));
      if (timings.open(QIODevice::WriteOnly)) timings.write(QJsonDocument(reviewTimings).toJson());
    });
  }
  const auto size = value(QStringLiteral("--size"), QStringLiteral("1200x798")).split(u'x');
  if (mainPage && size.size() == 2) window.resize(size[0].toInt(), size[1].toInt());
  if (panel && arguments.contains(QStringLiteral("--size")) && size.size() == 2) root->resize(size[0].toInt(), size[1].toInt());
  const auto tab = value(QStringLiteral("--tab"));
  const auto selectBusinessTab = [&] {
    if (tab.isEmpty()) return;
    for (auto groups = window.findChildren<QTabWidget*>(QStringLiteral("innerTabs")); auto* tabs : std::as_const(groups))
      if (tabs->isVisibleTo(&window)) tabs->setCurrentIndex(tab == QStringLiteral("registry") ? 1 : tab == QStringLiteral("staging") ? 2 : 0);
  };
  selectBusinessTab();
  const auto stagingTabEvidence = [&] {
    QJsonObject evidence;
    for (auto groups = window.findChildren<QTabWidget*>(QStringLiteral("innerTabs")); auto* tabs : std::as_const(groups)) {
      if (!tabs->isVisibleTo(&window) || tabs->count() < 3) continue;
      evidence.insert(QStringLiteral("tooltip"), tabs->tabToolTip(2));
      evidence.insert(QStringLiteral("indicator"), tabs->tabBar()->property("designTabIndicators").toMap().value(QStringLiteral("2")).toString());
      evidence.insert(QStringLiteral("currentIndex"), tabs->currentIndex());
      break;
    }
    return evidence;
  };
  QJsonObject stagingRecoveryEvidence;
  if (stagingScenario && scenario == QLatin1String("staging-restored"))
    QTimer::singleShot(250, &application, [&] {
      selectBusinessTab();
      stagingRecoveryEvidence.insert(QStringLiteral("before"), stagingTabEvidence());
      auto restored = uwfSystem->snapshot();
      restored.current.overlayFlags = restored.next.overlayFlags = 0;
      restored.current.overlayType = restored.next.overlayType = 0;
      seedPreviewSystem(std::move(restored));
      window.refresh();
      stagingRecoveryEvidence.insert(QStringLiteral("refreshRequested"), true);
    });
  bool captureComplete = false;
  int captureResult = interactive ? 0 : 1;
  if (!power && blocking)
    QTimer::singleShot(0, &application, [&] {
      if (page == QLatin1String("dialogs")) {
        if (scenario == QLatin1String("confirm"))
          ui::dialogs::confirm(nullptr, ui::I18n::tr("Discard pending changes?"),
                               ui::I18n::tr("There are pending changes that have not been applied. Continue and discard them?"));
        else
          ui::dialogs::warning(nullptr, ui::I18n::tr("Unable to complete the operation"), ui::I18n::tr("Access denied. Check permissions and try again."));
      } else if (scenario == QLatin1String("confirm") || scenario == QLatin1String("delete")) {
        auto backend = std::make_shared<PreviewCommitBackend>();
        presentation::CommitOperationViewModel model(
            backend->request(scenario == QLatin1String("delete") ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist), backend);
        ui::CommitOperationDialog dialog(model, *ports.fileDialogs, {}, nullptr);
        QTimer::singleShot(0, &model, &presentation::CommitOperationViewModel::start);
        dialog.exec();
      } else {
        QList<presentation::CommitReportRow> rows{{presentation::CommitRowStatus::Succeeded, QStringLiteral("C:\\ProgramData\\MyApp\\settings.json"),
                                                   QStringLiteral("—"), ui::I18n::tr("Committed to disk"), true, false, QStringLiteral("settings.json")},
                                                  {presentation::CommitRowStatus::Failed, QStringLiteral("C:\\ProgramData\\MyApp\\Data\\cache.db"),
                                                   QStringLiteral("0x80070005"), ui::I18n::tr("Access denied"), true, true, QStringLiteral("cache.db")}};
        if (scenario == QLatin1String("details")) {
          rows = {{presentation::CommitRowStatus::Failed,
                   QStringLiteral("C:\\Users\\User\\Documents\\项目资料\\年度归档\\设备配置与运行记录\\"
                                  "需要在重启前保存的现场诊断数据与说明\\设备配置备份与现场工作记录.json"),
                   QStringLiteral("0x80070020"),
                   QStringLiteral("文件被其他进程占用，无法提交到磁盘。\n请关闭正在编辑此文件的程序后重试。\n"
                                  "失败详情保留完整路径与错误原因，可选择文本或复制当前记录。"),
                   true, true, QStringLiteral("设备配置备份与现场工作记录.json")}};
          for (int index = 1; index < 12; ++index)
            rows.append({presentation::CommitRowStatus::Succeeded,
                         QStringLiteral("C:\\ProgramData\\MyApp\\Data\\配置记录-%1.json").arg(index, 3, 10, QChar('0')), QStringLiteral("—"),
                         ui::I18n::tr("Committed to disk"), true, false, QStringLiteral("配置记录-%1.json").arg(index, 3, 10, QChar('0'))});
        }
        ui::showCommitReport(
            nullptr, rows, app::CommitOperation::Persist,
            {scenario == QLatin1String("canceled") ? presentation::CommitCompletion::Cause::Canceled : presentation::CommitCompletion::Cause::Finished,
             scenario == QLatin1String("canceled") ? 3 : 0});
      }
      application.exit(captureResult);
    });
  if (power)
    QTimer::singleShot(0, &application, [&] {
      const auto action = page == QStringLiteral("shutdown") ? app::PowerAction::Shutdown : app::PowerAction::Restart;
      app::PowerActionDialogRequest request(action);
      if (scenario.startsWith(QLatin1String("skip-")) || scenario == QLatin1String("mixed-retained")) {
        const auto summary = previewAutomaticCommitSummary(scenario);
        request.pollStagingPreparation = [summary]() -> std::optional<app::PowerStagingPreparation> {
          if (!summary.commitRequired) return app::PowerStagingNotRequired{summary};
          return app::PowerStagingCommit{24,
                                         [summary] {
                                           app::PowerStagingProgress progress;
                                           progress.processedFiles = progress.totalFiles = 24;
                                           progress.automaticCommit = summary;
                                           return progress;
                                         },
                                         summary};
        };
      } else if (scenario.startsWith(QLatin1String("failure"))) {
        const int failureCount = scenario == QLatin1String("failure100") ? 100 : scenario == QLatin1String("failure8") ? 8 : 2;
        app::FileStagingCommitResult result;
        result.discoveredFiles = static_cast<std::size_t>(failureCount);
        result.failures = {
            {QStringLiteral("C:\\ProgramData\\MyApp\\cache.db"), app::FileStagingCommitFailureKind::ProviderFailure, QStringLiteral("拒绝访问 · 0x80070005")},
            {QStringLiteral("C:\\Users\\User\\Documents\\工作记录.docx"), app::FileStagingCommitFailureKind::ProviderFailure,
             QStringLiteral("文件被占用 · 0x80070020")}};
        for (int index = 2; index < failureCount; ++index) {
          const auto number = QStringLiteral("%1").arg(index + 1, 3, 10, QChar('0'));
          const auto path = index == 2 ? QStringLiteral(
                                             "C:\\Users\\User\\Documents\\项目资料\\年度归档\\设备配置与运行记录\\"
                                             "需要在重启前保存的现场诊断数据与说明\\配置备份-%1.json")
                                             .arg(number)
                                       : QStringLiteral("C:\\ProgramData\\MyApp\\Data\\配置记录-%1.json").arg(number);
          result.failures.append({path, app::FileStagingCommitFailureKind::ProviderFailure,
                                  index % 2 == 0 ? QStringLiteral("拒绝访问 · 0x80070005") : QStringLiteral("文件被占用 · 0x80070020")});
        }
        request.pollStagingPreparation = [result = std::move(result)]() -> std::optional<app::PowerStagingPreparation> {
          return app::PowerStagingFailure{presentation::renderFileStagingResult(result), result};
        };
      } else if (scenario == QLatin1String("infrastructure-failure")) {
        request.pollStagingPreparation = []() -> std::optional<app::PowerStagingPreparation> {
          return app::PowerStagingFailure{
              QStringLiteral("无法读取文件暂存列表。\n原始错误：拒绝访问 · 0x80070005\nRegistry staging store could not be opened.")};
        };
      } else if (scenario != QStringLiteral("empty"))
        request.pollStagingPreparation = [scenario, interactive, &captureComplete]() -> std::optional<app::PowerStagingPreparation> {
          if (scenario == QStringLiteral("preparing")) return std::nullopt;
          return app::PowerStagingCommit{58543, [interactive, &captureComplete] {
                                           return app::PowerStagingProgress{
                                               (interactive || captureComplete) ? app::PowerStagingState::Completed : app::PowerStagingState::InProgress,
                                               17000,
                                               58543,
                                               QStringLiteral("C:\\Users\\User\\AppData\\Roaming\\Code\\User\\settings.json"),
                                               {}};
                                         }};
        };
      QTimer driver;
      driver.setInterval(20);
      if (!interactive && scenario == QStringLiteral("staging")) {
        QObject::connect(&driver, &QTimer::timeout, &application, [&] {
          auto* modal = QApplication::activeModalWidget();
          auto* button = modal
                             ? modal->findChild<QPushButton*>(action == app::PowerAction::Shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn"))
                             : nullptr;
          if (button && button->isEnabled()) {
            driver.stop();
            button->click();
          }
        });
        driver.start();
      }
      ui::runPowerActionDialog(nullptr, std::move(request));
      application.exit(captureResult);
    });
  // Prepare the same page and control state before either mode enters its event loop.
  if (!interactive) QCursor::setPos(application.primaryScreen()->availableGeometry().topLeft());
  QWidget* target = nullptr;
  const auto control = value(QStringLiteral("--control"));
  const auto controlState = value(QStringLiteral("--control-state"), QStringLiteral("rest"));
  if (applyProgress)
    QTimer::singleShot(100, &application, [&] {
      auto* apply = panel->findChild<QPushButton*>(QStringLiteral("primaryBtn"));
      if (!apply || !apply->isEnabled()) {
        application.exit(9);
        return;
      }
      apply->click();
      auto* confirmation = panel->findChild<QDialog*>(QStringLiteral("applyExecutionConfirmation"));
      if (!confirmation) {
        application.exit(9);
        return;
      }
      confirmation->accept();
    });
  const bool waitForLoad =
      power ? scenario != QStringLiteral("preparing")
            : (page == QStringLiteral("logs") || page == QStringLiteral("overlay") || page == QLatin1String("picker")) && scenario != QStringLiteral("loading");
  const int captureDelay = value(QStringLiteral("--capture-delay"), scenario == QLatin1String("staging-restored") ? QStringLiteral("1100")
                                                                    : waitForLoad                                 ? QStringLiteral("1300")
                                                                                                                  : QStringLiteral("200"))
                               .toInt();
  QTimer::singleShot(captureDelay, &application, [&, root]() mutable {
    if (blocking || applyProgress) root = QApplication::activeModalWidget();
    if (!root) return;
    if (arguments.contains(QStringLiteral("--protect-all-draft"))) {
      if (auto* protect = root->findChild<QPushButton*>(QStringLiteral("hormProtectAll")); protect && protect->isEnabled()) protect->click();
    }
    if (mainPage && arguments.contains(QStringLiteral("--volume-index")))
      if (auto* volumes = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"))) volumes->setCurrentIndex(value(QStringLiteral("--volume-index")).toInt());
    if (mainPage && scenario == QLatin1String("tab-hover-leave")) {
      if (auto* volumes = window.findChild<QTabWidget*>(QStringLiteral("mainTabs")); volumes && volumes->count() > 1) {
        auto* bar = volumes->tabBar();
        const auto position = bar->tabRect(1).center();
        volumes->setCurrentIndex(1);
        const auto global = bar->mapToGlobal(position);
        QCursor::setPos(global);
        QEnterEvent enter(position, bar->mapTo(bar->window(), position), global);
        QApplication::sendEvent(bar, &enter);
        QHoverEvent hovered(QEvent::HoverMove, position, global, {}, Qt::NoModifier);
        QApplication::sendEvent(bar, &hovered);
        QTimer::singleShot(60, bar, [bar, volumes] {
          volumes->setCurrentIndex(0);
          QCursor::setPos(bar->mapToGlobal(QPoint(-20, -20)));
          QEvent leave(QEvent::Leave);
          QApplication::sendEvent(bar, &leave);
          QHoverEvent left(QEvent::HoverLeave, QPointF(-1, -1), QCursor::pos(), {}, Qt::NoModifier);
          QApplication::sendEvent(bar, &left);
        });
      }
    }
    if (page == QLatin1String("review") && arguments.contains(QStringLiteral("--command-preview")))
      if (auto* tabs = root->findChild<QTabWidget*>(QStringLiteral("reviewTabs"))) tabs->setCurrentIndex(1);
    if ((!mainPage || docs) && arguments.contains(QStringLiteral("--size")) && size.size() == 2) root->resize(size[0].toInt(), size[1].toInt());
    if (docs) {
      const auto area = root->screen()->availableGeometry();
      root->move(page == QLatin1String("overview") ? area.bottomRight() - QPoint(root->width() - 1, root->height() - 1)
                                                   : area.center() - QPoint(root->width() / 2, root->height() / 2));
      root->raise();
      root->activateWindow();
      host::activateForCapture(root, documentBackground.get(), visibleCapture);
    }
    const auto captionStyle = value(QStringLiteral("--caption-style"));
    if (!captionStyle.isEmpty()) host::setCaptionStyle(root, captionStyle);
    // Main-window volume tabs are populated by the asynchronous first refresh.
    if (mainPage) selectBusinessTab();
    if (docs && page == QLatin1String("overlay")) {
      if (auto* table = root->findChild<QTableView*>(QStringLiteral("overlayFilesTable"))) table->selectRow(2);
    }
    if (page == QStringLiteral("system") && scenario == QLatin1String("raw"))
      if (auto* tabs = root->findChild<QTabWidget*>(QStringLiteral("systemInformationTabs"))) tabs->setCurrentIndex(1);
    const auto expand = value(QStringLiteral("--expand"));
    for (auto disclosures = root->findChildren<ui::Disclosure*>(); auto* disclosure : std::as_const(disclosures))
      if (expand == QLatin1String("all") || (!expand.isEmpty() && disclosure->objectName().contains(expand, Qt::CaseInsensitive)) ||
          (page == QLatin1String("review") &&
           (scenario == QLatin1String("horm-disable") || scenario == QLatin1String("horm-enable") || scenario == QLatin1String("full-commit") ||
            scenario == QLatin1String("storage-change") || scenario == QLatin1String("reset-clear") || scenario == QLatin1String("reset-preserve")) &&
           disclosure->objectName() == QLatin1String("changeCommandsDisclosure")))
        disclosure->setOpen(true);
    if (aboutFeedback) {
      auto* save = root->findChild<QPushButton*>(QStringLiteral("aboutSaveConfigButton"));
      if (!save) {
        application.exit(3);
        return;
      }
      save->click();
      return;
    }
    const int toggles = value(QStringLiteral("--theme-toggle-count"), QStringLiteral("0")).toInt();
    for (int count = 0; count < toggles; ++count) ui::ThemeManager::instance().toggle();
    if (arguments.contains(QStringLiteral("--scroll-end"))) {
      QTimer::singleShot(100, root, [root] {
        for (auto bars = root->findChildren<QScrollBar*>(); auto* bar : std::as_const(bars))
          if (bar->isVisibleTo(root)) bar->setValue(bar->maximum());
      });
    }
    if (page == QStringLiteral("logs")) {
      if (auto* pages = root->findChild<presentation::PageProxyModel*>()) pages->goFirst();
      if (auto* table = root->findChild<QTableView*>(); table && table->model()->rowCount() > 5) table->selectRow(5);
    }
    if (!value(QStringLiteral("--select-row")).isEmpty()) {
      for (auto lists = window.findChildren<QListView*>(); auto* list : std::as_const(lists))
        if (dynamic_cast<ui::PathEntryListView*>(list) && list->isVisibleTo(&window)) {
          list->setCurrentIndex(list->model()->index(value(QStringLiteral("--select-row")).toInt(), 0));
          break;
        }
    }
    if (control.isEmpty()) return;
    for (auto candidates = root->findChildren<QWidget*>(control); auto* candidate : std::as_const(candidates))
      if (candidate->isVisibleTo(root)) {
        target = candidate;
        break;
      }
    if (!target && control == QLatin1String("captionMaximize"))
      for (auto buttons = root->findChildren<QAbstractButton*>(); auto* button : std::as_const(buttons))
        if (host::isMaximizeButton(button)) {
          target = button;
          break;
        }
    if (!target && control == QLatin1String("languageMenu"))
      for (auto buttons = root->findChildren<QToolButton*>(); auto* button : std::as_const(buttons))
        if (button->defaultAction() && button->defaultAction()->text() == ui::I18n::tr("Language")) {
          target = button;
          break;
        }
    if (!target) {
      application.exit(3);
      return;
    }
    if (controlState == QLatin1String("context") || controlState == QLatin1String("window-menu")) {
      QWidget* receiver = target;
      QPoint local = target->rect().center();
      if (auto* view = qobject_cast<QAbstractItemView*>(target)) {
        receiver = view->viewport();
        const auto index = view->model()->index(value(QStringLiteral("--context-row"), QStringLiteral("0")).toInt(), 0);
        if (index.isValid()) {
          view->scrollTo(index);
          local = view->visualRect(index).center();
        }
      }
      if (!interactive)
        QTimer::singleShot(100, &application, [&, root] {
          auto* popup = qobject_cast<QMenu*>(QApplication::activePopupWidget());
          if (!popup) {
            application.exit(5);
            return;
          }
          captureResult = popup->grab().save(output) ? 0 : 1;
          captureComplete = true;
          QJsonArray commands;
          for (auto actions = popup->actions(); const auto* action : std::as_const(actions))
            if (!action->isSeparator())
              commands.append(QJsonObject{{"text", action->text()}, {"icon", !action->icon().isNull()}, {"enabled", action->isEnabled()}});
          QFile commandFile(output + QStringLiteral(".json"));
          if (commandFile.open(QIODevice::WriteOnly)) commandFile.write(QJsonDocument(commands).toJson());
          popup->close();
          if (blocking) static_cast<QDialog*>(root)->reject();
          application.exit(captureResult);
        });
      if (controlState == QLatin1String("window-menu"))
        ui::showWindowMenu(root, target->mapToGlobal(local));
      else {
        QContextMenuEvent context(QContextMenuEvent::Mouse, local, receiver->mapToGlobal(local));
        QApplication::sendEvent(receiver, &context);
      }
      return;
    }
    if (controlState == QStringLiteral("keyboard")) {
      QKeyEvent key(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
      QApplication::sendEvent(root, &key);
      target->setFocus(Qt::OtherFocusReason);
    }
    if (controlState == QStringLiteral("disabled")) target->setEnabled(false);
    if (controlState == QStringLiteral("select-all")) {
      target->setFocus(Qt::OtherFocusReason);
      if (auto* text = qobject_cast<QTextEdit*>(target))
        text->selectAll();
      else if (auto* plainText = qobject_cast<QPlainTextEdit*>(target))
        plainText->selectAll();
      else if (auto* label = qobject_cast<QLabel*>(target))
        label->setSelection(0, static_cast<int>(label->text().size()));
    }
    if (controlState == QStringLiteral("hover") || controlState == QStringLiteral("pressed")) QCursor::setPos(target->mapToGlobal(target->rect().center()));
    if (controlState == QStringLiteral("pressed")) {
      if (auto* button = qobject_cast<QAbstractButton*>(target)) button->setDown(true);
    }
    if (controlState == QStringLiteral("popup")) {
      const QPointF local = target->rect().center();
      const QPointF global = target->mapToGlobal(local.toPoint());
      QCursor::setPos(global.toPoint());
      QMouseEvent press(QEvent::MouseButtonPress, local, global, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      QApplication::sendEvent(target, &press);
      if (auto* popup = QApplication::activePopupWidget()) {
        QMouseEvent release(QEvent::MouseButtonRelease, popup->mapFromGlobal(global.toPoint()), global, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(popup, &release);
      }
    }
  });
  if (interactive) {
    auto* notice = ui::I18n::text(new QLabel(&window), [] { return ui::I18n::tr("Simulation mode · UWF changes are kept in memory only"); });
    window.statusBar()->addPermanentWidget(notice);
    const auto visiblePreview = [&]() -> QWidget* {
      auto* preview = QApplication::activePopupWidget();
      if (!preview) preview = QApplication::activeModalWidget();
      if (!preview && !blocking) preview = root;
      return preview && preview->isVisible() ? preview : nullptr;
    };
    if (!output.isEmpty())
      QTimer::singleShot(2500, &application, [&] {
        auto* preview = visiblePreview();
        if (!preview || !preview->grab().save(output)) {
          qCritical("The interactive preview screenshot could not be captured.");
          captureResult = 1;
          application.exit(captureResult);
        }
      });
    const bool smokeTest = arguments.contains(QStringLiteral("--smoke-test"));
    bool smokeVerified = false;
    if (smokeTest)
      QTimer::singleShot(4000, &application, [&] {
        auto* preview = visiblePreview();
        if (!preview) {
          qCritical("The preview smoke test found no visible target window.");
          captureResult = 10;
          application.exit(captureResult);
          return;
        }
        QJsonObject tabs;
        for (auto children = preview->findChildren<QTabWidget*>(); const auto* child : std::as_const(children))
          if (!child->objectName().isEmpty()) tabs.insert(child->objectName(), child->currentIndex());
        QJsonObject disclosures;
        for (auto children = preview->findChildren<ui::Disclosure*>(); const auto* child : std::as_const(children))
          if (!child->objectName().isEmpty()) disclosures.insert(child->objectName(), child->isOpen());
        const QJsonObject evidence{{QStringLiteral("visible"), preview->isVisible()}, {QStringLiteral("modal"), preview->isModal()},
                                   {QStringLiteral("title"), preview->windowTitle()}, {QStringLiteral("width"), preview->width()},
                                   {QStringLiteral("height"), preview->height()},     {QStringLiteral("tabs"), tabs},
                                   {QStringLiteral("disclosures"), disclosures}};
        std::fprintf(stdout, "Preview smoke evidence: %s\n", QJsonDocument(evidence).toJson(QJsonDocument::Compact).constData());
        std::fflush(stdout);
        smokeVerified = true;
        if (blocking) {
          if (!preview->close()) {
            captureResult = 11;
            application.exit(captureResult);
          }
        } else {
          application.quit();
        }
      });
    const auto result = application.exec();
    return smokeTest && !smokeVerified ? 10 : result;
  }
  QTimer captureTimer;
  bool foregroundSettled = false;
  captureTimer.setSingleShot(true);
  QObject::connect(&captureTimer, &QTimer::timeout, &application, [&] {
    if (blocking || aboutFeedback || applyProgress) root = QApplication::activeModalWidget();
    if (!root) {
      application.exit(4);
      return;
    }
    if (!visibleCapture && (docs || arguments.contains(QStringLiteral("--require-foreground"))) && !host::isForeground(root)) {
      if (docs && reviewTimer.elapsed() > 10000) {
        application.exit(7);
        return;
      }
      captureTimer.start(250);
      return;
    }
    if (!visibleCapture && (docs || arguments.contains(QStringLiteral("--require-foreground"))) && !foregroundSettled) {
      foregroundSettled = true;
      QCursor::setPos(target && controlState == QLatin1String("hover") ? target->mapToGlobal(target->rect().center())
                                                                       : root->screen()->availableGeometry().topLeft());
      captureTimer.start(value(QStringLiteral("--settle-delay"), QStringLiteral("250")).toInt());
      return;
    }
    const auto metrics = value(QStringLiteral("--metrics"));
    if (!metrics.isEmpty()) {
      QJsonArray widgets;
      for (auto children = root->findChildren<QWidget*>(); const auto* widget : std::as_const(children)) {
        if (!widget->isVisibleTo(root)) continue;
        const auto position = widget->mapTo(root, QPoint());
        QJsonObject record{{"class", QString::fromLatin1(widget->metaObject()->className())},
                           {"name", widget->objectName()},
                           {"enabled", widget->isEnabled()},
                           {"x", position.x()},
                           {"y", position.y()},
                           {"w", widget->width()},
                           {"h", widget->height()},
                           {"minimumW", widget->minimumSizeHint().width()},
                           {"minimumH", widget->minimumSizeHint().height()},
                           {"hintH", widget->sizeHint().height()},
                           {"hasHfw", widget->hasHeightForWidth()},
                           {"hfw", widget->heightForWidth(widget->width())},
                           {"fontSize", widget->font().pixelSize()},
                           {"font", widget->font().family()}};
        if (const auto* layout = widget->layout()) {
          record["layoutHasHfw"] = layout->hasHeightForWidth();
          record["layoutHfw"] = layout->heightForWidth(widget->contentsRect().width());
          record["layoutMinimumHfw"] = layout->minimumHeightForWidth(widget->contentsRect().width());
          record["layoutTotalHfw"] = layout->totalHeightForWidth(widget->width());
        }
        if (const auto* label = qobject_cast<const QLabel*>(widget)) record["text"] = label->text();
        if (const auto* editor = qobject_cast<const QTextEdit*>(widget)) record["text"] = editor->toPlainText();
        if (const auto* button = qobject_cast<const QAbstractButton*>(widget)) record["checked"] = button->isChecked();
        if (const auto* tabs = qobject_cast<const QTabWidget*>(widget)) {
          record["currentIndex"] = tabs->currentIndex();
          QJsonArray tabFacts;
          for (int index = 0; index < tabs->count(); ++index)
            tabFacts.append(QJsonObject{{"text", tabs->tabText(index)},
                                        {"tooltip", tabs->tabToolTip(index)},
                                        {"indicator", tabs->tabBar()->property("designTabIndicators").toMap().value(QString::number(index)).toString()}});
          record["tabs"] = tabFacts;
        }
        if (const auto* view = qobject_cast<const QAbstractItemView*>(widget); view && view->model()) {
          QJsonArray rows;
          for (int row = 0; row < std::min(50, view->model()->rowCount()); ++row) {
            const auto index = view->model()->index(row, 0);
            rows.append(QJsonObject{{"text", index.data(Qt::DisplayRole).toString()},
                                    {"status", index.data(presentation::StatusTextRole).toString()},
                                    {"tooltip", index.data(Qt::ToolTipRole).toString()}});
          }
          record["rows"] = rows;
        }
        if (const auto* scroll = qobject_cast<const QScrollBar*>(widget)) record["range"] = scroll->maximum() - scroll->minimum();
        widgets.append(record);
      }
      if (!stagingRecoveryEvidence.isEmpty()) {
        stagingRecoveryEvidence.insert(QStringLiteral("after"), stagingTabEvidence());
        stagingRecoveryEvidence.insert(QStringLiteral("class"), QStringLiteral("PreviewEvidence"));
        stagingRecoveryEvidence.insert(QStringLiteral("name"), QStringLiteral("stagingRecoveryEvidence"));
        widgets.append(stagingRecoveryEvidence);
      }
      if (reviewModel) {
        using Scope = app::ConfigurationScriptScope;
        const auto* tabs = panel->findChild<QTabWidget*>(QStringLiteral("reviewTabs"));
        const bool next = tabs && tabs->currentIndex() == 1;
        const auto scope = next ? Scope::NextConfiguration : Scope::PendingChanges;
        QJsonArray commands;
        for (const auto& command : reviewModel->exportCommands(scope)) commands.append(QString::fromStdString(app::renderUwfmgrCommand(command)));
        auto* editor = panel->findChild<QTextEdit*>(next ? QStringLiteral("configurationCommandPreview") : QStringLiteral("changeCommandPreview"));
        bool exportEnabled = false;
        const auto exportButtons = panel->findChildren<QPushButton*>();
        for (const auto* button : std::as_const(exportButtons))
          if (button->text() == ui::I18n::tr("Export commands")) exportEnabled = button->isEnabled();
        widgets.append(QJsonObject{{"class", "PreviewEvidence"},
                                   {"name", "reviewExportEvidence"},
                                   {"scope", next ? "NextConfiguration" : "PendingChanges"},
                                   {"commands", commands},
                                   {"commandCount", static_cast<int>(commands.size())},
                                   {"pendingCommandCount", static_cast<int>(reviewModel->changeCommandCount())},
                                   {"exportEnabled", exportEnabled},
                                   {"modelExportEnabled", reviewModel->canExport(scope)},
                                   {"restriction", reviewModel->exportRestrictionText(scope)},
                                   {"previewText", editor ? editor->toPlainText() : QString{}}});
      }
      QDir().mkpath(QFileInfo(metrics).absolutePath());
      QFile metricsFile(metrics);
      if (metricsFile.open(QIODevice::WriteOnly)) metricsFile.write(QJsonDocument(widgets).toJson());
    }
    if (arguments.contains(QStringLiteral("--dump-material"))) {
      const auto material = host::materialProperties(root);
      QFile materialFile(output + QStringLiteral(".material.json"));
      if (materialFile.open(QIODevice::WriteOnly)) materialFile.write(QJsonDocument(material).toJson());
      root->grab().save(output + QStringLiteral(".qt-layer.png"));
    }
    // QWidget::grab paints the widget tree alone and omits DWM materials.
    // Capture this window's composited pixels when reviewing native backdrops.
    QImage capture;
    if (docs) {
      auto* screen = root->screen();
      if (page == QLatin1String("overview")) {
        const auto area = screen->availableGeometry();
        root->move(area.bottomRight() - QPoint(root->width() - 1, root->height() - 1));
        const bool taskbarConfirmed = host::taskbarPresented();
        if (!taskbarConfirmed) {
          application.exit(8);
          return;
        }
      }
      host::flushCompositor();
      QRect documentBounds(root->mapToGlobal(QPoint()), root->size());
      if (page == QLatin1String("overview")) documentBounds.setBottomRight(screen->geometry().bottomRight());
      const auto origin = documentBounds.topLeft() - screen->geometry().topLeft();
      capture = screen->grabWindow(0, origin.x(), origin.y(), documentBounds.width(), documentBounds.height()).toImage();
    } else if (root->property("systemBackdropRequested").toBool() && host::isForeground(root)) {
      auto* screen = root->screen();
      const QPoint origin = root->mapToGlobal(QPoint()) - screen->geometry().topLeft();
      capture = screen->grabWindow(0, origin.x(), origin.y(), root->width(), root->height()).toImage();
    } else {
      capture = root->grab().toImage();
    }
    const qreal scale = docs || arguments.contains(QStringLiteral("--native-scale")) ? capture.devicePixelRatio() : 1.0;
    if (!docs && scale == 1.0) {
      capture = capture.scaled(root->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      capture.setDevicePixelRatio(1);
    }
    QRect bounds = root->rect();
    if (target && !arguments.contains(QStringLiteral("--full-frame"))) {
      bounds = QRect(target->mapTo(root, QPoint()), target->size()).adjusted(-12, -12, 12, 12);
      if (auto* popup = QApplication::activePopupWidget(); popup && controlState == QStringLiteral("popup")) {
        const QRect popupBounds(root->mapFromGlobal(popup->mapToGlobal(QPoint())), popup->size());
        popup->grab().save(QFileInfo(output).absolutePath() + QStringLiteral("/popup-raw-qt.png"));
        QPainter composite(&capture);
        composite.drawImage(popupBounds, popup->grab().toImage());
        bounds = bounds.united(popupBounds.adjusted(-12, -12, 12, 12));
      }
    }
    const QRect crop(qRound(bounds.x() * scale), qRound(bounds.y() * scale), qRound(bounds.width() * scale), qRound(bounds.height() * scale));
    const bool saved = (docs ? capture : capture.copy(crop.intersected(capture.rect()))).save(output);
    if (docs && saved) {
      const auto* screen = root->screen();
      const QJsonObject environment{{"platform", QGuiApplication::platformName()},
                                    {"screen", screen->name()},
                                    {"screenWidth", screen->geometry().width()},
                                    {"screenHeight", screen->geometry().height()},
                                    {"dpr", capture.devicePixelRatio()},
                                    {"pixelWidth", capture.width()},
                                    {"pixelHeight", capture.height()},
                                    {"windowWidth", root->width()},
                                    {"windowHeight", root->height()}};
      QFile screenFile(output + QStringLiteral(".screen.json"));
      if (screenFile.open(QIODevice::WriteOnly)) screenFile.write(QJsonDocument(environment).toJson());
    }
    captureResult = saved ? 0 : 1;
    captureComplete = true;
    if (arguments.contains(QStringLiteral("--interactive"))) {
      if (arguments.contains(QStringLiteral("--repeat-capture"))) captureTimer.start(1000);
      return;
    }
    if (!blocking)
      application.exit(captureResult);
    else if (!power || scenario != QStringLiteral("staging"))
      static_cast<QDialog*>(root)->reject();
  });
  const int progressAge =
      applyProgress ? value(QStringLiteral("--progress-age"), scenario == QLatin1String("apply-long") ? QStringLiteral("1800") : QStringLiteral("400")).toInt()
                    : 0;
  captureTimer.start(std::max({waitForLoad ? 1500 : 500, captureDelay + 250, progressAge + 100}));
  return application.exec();
}
