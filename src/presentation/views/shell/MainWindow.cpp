// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/shell/MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QStackedWidget>
#include <QTabBar>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextOption>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <optional>
#include <tuple>
#include <utility>

#include "application/settings/UserConfig.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/appearance/WindowAppearance.h"
#include "presentation/controls/feedback/ContentState.h"
#include "presentation/controls/feedback/TransientLabel.h"
#include "presentation/controls/layout/VerticalScrollArea.h"
#include "presentation/controls/navigation/CommandBar.h"
#include "presentation/controls/navigation/CommandMenu.h"
#include "presentation/controls/navigation/TabView.h"
#include "presentation/controls/pagination/PagedWindowLayout.h"
#include "presentation/controls/text/SummaryLabel.h"
#include "presentation/design/surfaces/SurfaceShadow.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/viewmodels/shell/MainWindowViewModel.h"
#include "presentation/views/commit/CommitDispatcher.h"
#include "presentation/views/configuration/global/GlobalStatusPanel.h"
#include "presentation/views/configuration/importing/ImportDialog.h"
#include "presentation/views/configuration/overlay/OverlayAdvancedDialog.h"
#include "presentation/views/configuration/overlay/OverlayMaintenanceDialogs.h"
#include "presentation/views/configuration/review/ApplyPlanDialog.h"
#include "presentation/views/configuration/volumes/VolumeTab.h"
#include "presentation/views/diagnostics/AboutDialog.h"
#include "presentation/views/diagnostics/LogViewerDialog.h"
#include "presentation/views/diagnostics/SystemInformationDialog.h"
#include "presentation/views/enhanced/EnhancedModeDialog.h"
#include "presentation/views/overlay/coordination/OverlayPresentationController.h"
#include "presentation/views/shell/HoverHintController.h"
#include "presentation/views/shell/StatusSurface.h"
#include "presentation/views/shell/TrayController.h"
#include "presentation/views/shell/WindowChromeController.h"
#include "presentation/views/shell/WindowTitleBar.h"
#include "presentation/views/shell/WindowWorkflowFactory.h"

namespace uwf::ui {

MainWindow::MainWindow(MainWindowServices services, MainWindowStartup startup, QWidget* parent)
    : QMainWindow(parent),
      m_viewModelFactory(std::move(services.viewModelFactory)),
      m_workflowFactory(std::move(services.workflows)),
      m_systemInformation(m_viewModelFactory->systemInformation()),
      m_fileDialogs(services.fileDialogs),
      m_viewModel(m_workflowFactory->mainWindow(m_viewModelFactory, startup.uwfCapability, this)),
      m_compatibilityMode(startup.compatibilityMode) {
  connect(m_viewModel.get(), &presentation::MainWindowViewModel::pendingChanged, this, &MainWindow::updatePendingSummary);
  connect(m_viewModel.get(), &presentation::MainWindowViewModel::stateCommitted, this, &MainWindow::applyCommittedState);
  connect(m_viewModel.get(), &presentation::MainWindowViewModel::stateCommitted, this, &MainWindow::completeInitialShow);
  connect(m_viewModel.get(), &presentation::MainWindowViewModel::refreshFailed, this, &MainWindow::completeInitialShow);
  connect(m_viewModel.get(), &presentation::MainWindowViewModel::reviewPrepared, this, [this](std::shared_ptr<presentation::ApplyPlanViewModel> plan) {
    ApplyPlanDialog dlg(std::move(plan), m_fileDialogs, this);
    connect(&dlg, &ApplyPlanDialog::safeRestartRequested, this, &MainWindow::requestSafeRestart);
    connect(&dlg, &ApplyPlanDialog::directRestartRequested, this, &MainWindow::requestDirectRestart);
    dlg.exec();
  });
  connect(m_viewModel.get(), &presentation::MainWindowViewModel::reviewFailed, this,
          [this](const QString& reason) { dialogs::warning(this, I18n::tr("Review and apply"), reason); });
  connect(m_viewModel.get(), &presentation::MainWindowViewModel::interactionAvailabilityChanged, this, &MainWindow::updateInteractionAvailability);
  connect(m_viewModel.get(), &presentation::MainWindowViewModel::enhancedModeStatusChanged, this, [this] {
    refreshThemedUi();
    updateInteractionAvailability();
  });
  connect(m_viewModel.get(), &presentation::MainWindowViewModel::enhancedModeAppearanceChanged, this, &MainWindow::refreshThemedUi);
  // Initial layout and data loading precede the first native show.
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
  prepareWindowBackdrop(this);
  installSurfaceShadow(this, ShadowKind::Window);
  // Density is selected from the available client size. A standard-density
  // minimum must not prevent the resize that selects the compact layout.
  layout()->setSizeConstraint(QLayout::SetNoConstraint);
  setMinimumSize(design::smallMainWidth, design::smallMainHeight);
  setFocusPolicy(Qt::ClickFocus);
  setWindowTitle(I18n::applicationTitle());
  setWindowIcon(QIcon(":/icons/app.svg"));
  m_hoverHints = std::make_unique<HoverHintController>(this);
  m_chrome = std::make_unique<WindowChromeController>(this);

  m_tray = std::make_unique<TrayController>(this);
  connect(m_tray.get(), &TrayController::activateWindowRequested, this, &MainWindow::raiseToFront);
  connect(m_tray.get(), &TrayController::exitApplicationRequested, this, &MainWindow::requestExit);

  m_overlayPresentation = std::make_unique<OverlayPresentationController>(m_viewModel->overlay(), *m_tray);
  // Startup visibility is installed before snapshots; refresh and rebuild retain it.
  m_overlayPresentation->setHubEnabled(startup.hubVisible);
  connect(m_overlayPresentation.get(), &OverlayPresentationController::activateMainWindowRequested, this, &MainWindow::raiseToFront);
  connect(m_overlayPresentation.get(), &OverlayPresentationController::exitApplicationRequested, this, &MainWindow::requestExit);
  connect(m_overlayPresentation.get(), &OverlayPresentationController::safeShutdownRequested, this, &MainWindow::requestSafeShutdown);
  connect(m_overlayPresentation.get(), &OverlayPresentationController::safeRestartRequested, this, &MainWindow::requestSafeRestart);

  // Workflow activity gates window actions independently of usage polling.
  m_commit = m_workflowFactory->commit(
      [owner = QPointer(m_overlayPresentation.get())] { return owner ? owner->pauseObservation() : presentation::ObservationPause{}; },
      [owner = QPointer(m_viewModel.get())](const bool active) {
        if (owner) owner->setManualCommitActive(active);
      },
      this);
  updateInteractionAvailability();
  connect(&I18n::events(), &LanguageEvents::changed, this, &MainWindow::retranslateUi);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](app::Theme) {
    refreshThemedUi();
    m_chrome->applyTitleBarTheme();
  });
}

MainWindow::~MainWindow() {
  m_commit.reset();
  disconnect(m_viewModel.get(), nullptr, this, nullptr);
  m_viewModel.reset();
  // Controllers remove application event filters before child-widget destruction.
}

void MainWindow::startInTray() {
  // Quiet startup initializes data without showing the main window.
  rebuildUi();
}

void MainWindow::raiseToFront() { m_chrome->raiseToFront(); }

void MainWindow::setVisible(const bool visible) {
  const bool initial = visible && !m_initialUiPrepared;
  if (initial) {
    m_initialUiPrepared = true;
    ensurePolished();
    (void)winId();
    // Initial width is established before measuring wrapped content while hidden.
    m_chrome->initializeGeometry();
    rebuildUi();
    m_initialGeometryPending = true;
  }
  if (visible && m_initialGeometryPending && m_viewModel->refreshing()) {
    m_showAfterRefresh = true;
    return;
  }
  m_showAfterRefresh = false;
  const bool firstShow = visible && m_initialGeometryPending;
  QMainWindow::setVisible(visible);
  if (firstShow) setFocus(Qt::OtherFocusReason);
}
void MainWindow::completeInitialShow() {
  if (m_showAfterRefresh) setVisible(true);
}

void MainWindow::buildUi() {
  setWindowTitle(I18n::applicationTitle());
  m_chrome->applyTitleBarTheme();

  auto* central = new Surface(design::Color::Window, this);
  central->setBackdropOpacity(0);
  central->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, design::windowRadius);
  auto* centralLayout = new QVBoxLayout(central);
  centralLayout->setContentsMargins(1, 1, 1, 1);
  centralLayout->setSpacing(0);
  auto* titleBar = new WindowTitleBar(central);
  titleBar->setBackdropOpacity(0);
  centralLayout->addWidget(titleBar);
  auto* commands = new CommandBar(central);
  commands->setBackdropOpacity(0);
  m_commands = commands;
  commands->setObjectName(QStringLiteral("mainCommands"));
  centralLayout->addWidget(commands);

  m_actImport = commands->addCommand(QString{});
  I18n::bind(m_actImport, [target = m_actImport] { target->setText(I18n::tr("Import")); });
  connect(m_actImport, &QAction::triggered, this, &MainWindow::showImport);
  m_actRefresh = commands->addCommand(QString{});
  I18n::bind(m_actRefresh, [target = m_actRefresh] { target->setText(I18n::tr("Refresh")); });
  m_actRefresh->setShortcut(QKeySequence::Refresh);
  addAction(m_actRefresh);
  I18n::bind(m_actRefresh,
             [target = m_actRefresh] { target->setToolTip(I18n::tr("Re-read the current session state and next-session configuration of UWF.")); });
  connect(m_actRefresh, &QAction::triggered, this, &MainWindow::refresh);
  m_actPlan = commands->addCommand(QString{});
  I18n::bind(m_actPlan, [target = m_actPlan] { target->setText(I18n::tr("Review and apply")); });
  connect(m_actPlan, &QAction::triggered, this, &MainWindow::showPlan);
  commands->addSeparator();
  m_actShutdown = commands->addCommand(QString{});
  I18n::bind(m_actShutdown, [target = m_actShutdown] { target->setText(I18n::tr("Safe shutdown")); });
  I18n::bind(m_actShutdown, [target = m_actShutdown] { target->setToolTip(I18n::tr("Shut down safely, even when the UWF overlay is full.")); });
  connect(m_actShutdown, &QAction::triggered, this, &MainWindow::requestSafeShutdown);
  m_actRestart = commands->addCommand(QString{});
  I18n::bind(m_actRestart, [target = m_actRestart] { target->setText(I18n::tr("Safe restart")); });
  I18n::bind(m_actRestart, [target = m_actRestart] { target->setToolTip(I18n::tr("Restart safely, even when the UWF overlay is full.")); });
  connect(m_actRestart, &QAction::triggered, this, &MainWindow::requestSafeRestart);
  commands->addStretch();
  if (m_viewModel->hasEnhancedMode()) {
    m_actEnhancedMode = commands->addCommand(QString{}, true);
    I18n::bind(m_actEnhancedMode, [target = m_actEnhancedMode] { target->setText(I18n::tr("Enhanced mode")); });
    I18n::bind(m_actEnhancedMode,
               [target = m_actEnhancedMode] { target->setToolTip(I18n::tr("Coordinate automatic file staging with Windows shutdown and restart.")); });
    connect(m_actEnhancedMode, &QAction::triggered, this, &MainWindow::showEnhancedMode);
  }
  m_actLog = commands->addCommand(QString{}, true);
  I18n::bind(m_actLog, [target = m_actLog] { target->setText(I18n::tr("Log")); });
  I18n::bind(m_actLog, [target = m_actLog] { target->setToolTip(I18n::tr("View the internal log accumulated during this session, for troubleshooting.")); });
  connect(m_actLog, &QAction::triggered, this, &MainWindow::showLogs);
  m_actAbout = new QAction(QString{}, commands);
  I18n::bind(m_actAbout, [target = m_actAbout] { target->setText(I18n::tr("About")); });
  connect(m_actAbout, &QAction::triggered, this, &MainWindow::showAbout);
  auto* more = commands->addCommand(QString{});
  I18n::bind(more, [target = more] { target->setText(I18n::tr("More")); });
  more->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/more.svg")));
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, more,
          [more] { more->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/more.svg"))); });
  auto* moreMenu = new CommandMenu(commands);
  moreMenu->setAnchor(commands->buttonForAction(more));
  auto* systemInformation = moreMenu->addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/service.svg")), QString{});
  I18n::bind(systemInformation, [target = systemInformation] { target->setText(I18n::tr("System information")); });
  systemInformation->setObjectName(QStringLiteral("systemInformationAction"));
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, systemInformation,
          [systemInformation] { systemInformation->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/service.svg"))); });
  connect(systemInformation, &QAction::triggered, this, &MainWindow::showSystemInformation);
  moreMenu->addAction(m_actAbout);
  more->setMenu(moreMenu);
  commands->buttonForAction(more)->setPopupMode(QToolButton::InstantPopup);

  auto* hubAction = new QAction(QStringLiteral("HUB"), titleBar);
  hubAction->setCheckable(true);
  I18n::bind(hubAction, [target = hubAction] { target->setToolTip(I18n::tr("Show or hide the overlay widget.")); });
  connect(hubAction, &QAction::toggled, m_overlayPresentation.get(), &OverlayPresentationController::setHubEnabled);
  m_actLang = new QAction(QString{}, titleBar);
  I18n::bind(m_actLang, [target = m_actLang] { target->setText(I18n::tr("Language")); });
  I18n::bind(m_actLang, [target = m_actLang] { target->setToolTip(I18n::tr("Switch display language")); });
  auto* menu = new CommandMenu(titleBar);
  menu->setMaximumVisibleHeight(520);
  auto* languageHeading = menu->addAction(QString{});
  I18n::bind(languageHeading, [target = languageHeading] { target->setText(I18n::tr("Display language")); });
  languageHeading->setEnabled(false);
  languageHeading->setProperty("designMenuHeading", true);
  auto languageHeadingFont = languageHeading->font();
  languageHeadingFont.setPixelSize(11);
  languageHeading->setFont(languageHeadingFont);
  // Native language names remain recognizable after a language switch.
  auto* langGroup = new QActionGroup(menu);
  langGroup->setExclusive(true);
  for (const auto& language : I18n::languages()) {
    auto* action = menu->addAction(language.nativeName.toString());
    action->setCheckable(true);
    action->setActionGroup(langGroup);
    action->setData(static_cast<int>(language.id));
    action->setChecked(I18n::instance().lang() == language.id);
  }
  connect(langGroup, &QActionGroup::triggered, this, [](const QAction* action) {
    const auto target = static_cast<I18n::Lang>(action->data().toInt());
    I18n::instance().requestLang(target);
  });
  I18n::bind(langGroup, [langGroup] {
    for (auto actions = langGroup->actions(); auto* item : std::as_const(actions))
      item->setChecked(item->data().toInt() == static_cast<int>(I18n::instance().lang()));
  });

  m_actLang->setMenu(menu);
  m_actTheme = new QAction(QString{}, titleBar);
  I18n::bind(m_actTheme, [target = m_actTheme] { target->setText(I18n::tr("Theme")); });
  I18n::bind(m_actTheme, [target = m_actTheme] { target->setToolTip(I18n::tr("Toggle light / dark theme")); });
  connect(m_actTheme, &QAction::triggered, this, [] { ThemeManager::instance().requestToggle(); });
  titleBar->addUtility(hubAction);
  titleBar->addUtility(m_actLang);
  titleBar->addUtility(m_actTheme);

  auto* mainRow = new QHBoxLayout();
  mainRow->setContentsMargins(0, 0, 0, 0);
  mainRow->setSpacing(0);
  m_tabs = new TabView(central);
  m_tabs->setObjectName(QStringLiteral("mainTabs"));
  static_cast<TabView*>(m_tabs)->setKind(TabView::Kind::Volumes);
  m_tabs->setDocumentMode(true);
  m_tabs->setElideMode(Qt::ElideNone);
  m_tabs->setUsesScrollButtons(true);
  connect(m_tabs, &QTabWidget::currentChanged, this, [this] { setStatusDetail({}); });
  m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto* workspace = new Surface(design::Color::Window, central);
  auto* workspaceLayout = new QVBoxLayout(workspace);
  workspaceLayout->setContentsMargins(24, 0, 24, 20);
  workspaceLayout->setSpacing(0);
  m_workspaceStack = new QStackedWidget(workspace);
  m_workspaceState = new ContentState(m_workspaceStack, ContentState::ScrollMode::SelfManaged);
  m_workspaceState->setObjectName(QStringLiteral("mainUnavailableState"));
  m_workspaceState->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/enhanced.svg")));
  m_workspaceState->setAction(I18n::tr("Read again"), ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")));
  connect(m_workspaceState, &ContentState::actionRequested, this, &MainWindow::refresh);
  m_workspaceStack->addWidget(m_tabs);
  m_workspaceStack->addWidget(m_workspaceState);
  workspaceLayout->addWidget(m_workspaceStack);
  m_tabs->tabBar()->setFixedHeight(47);
  mainRow->addWidget(workspace, 1);

  auto* globalWrap = new Surface(design::Color::SurfaceSoft, central);
  globalWrap->setBackdropOpacity(50);
  m_settingsSurface = globalWrap;
  globalWrap->setObjectName(QStringLiteral("globalWrap"));
  globalWrap->setBorder(Qt::LeftEdge);
  globalWrap->setFixedWidth(width() <= design::compactWindowWidth ? design::compactSettingsWidth : design::settingsWidth);
  auto* globalLayout = new QVBoxLayout(globalWrap);
  globalLayout->setContentsMargins(0, 0, 0, 0);
  auto* settingsScroll = m_settingsScroll = new VerticalScrollArea(globalWrap);
  settingsScroll->setObjectName(QStringLiteral("globalSettingsScroll"));
  auto* settingsContent = new QWidget(settingsScroll);
  auto* settingsLayout = new QVBoxLayout(settingsContent);
  settingsLayout->setContentsMargins(22, 23, 21, 17);
  settingsLayout->setSpacing(18);
  globalLayout->addWidget(settingsScroll);
  m_viewModel->initializeConfiguration(m_systemInformation->totalRamMb(), m_systemInformation->systemDriveLetter());
  m_global = new GlobalStatusPanel(m_viewModel->configuration()->global(), m_viewModel->configuration()->overlayPolicy(), globalWrap);
  connect(m_viewModel->configuration()->global().get(), &presentation::GlobalConfigurationViewModel::stateChanged, this, &MainWindow::updateLayoutDensity,
          Qt::UniqueConnection);
  connect(m_global, &GlobalStatusPanel::advancedSettingsRequested, this, [this] {
    OverlayAdvancedDialog dialog(m_viewModel->configuration()->overlayPolicy(), m_viewModel->overlayMaintenance(), this);
    dialog.exec();
  });
  m_overlayPresentation->bindAction(hubAction);
  connect(m_global, &GlobalStatusPanel::usageWarningChanged, this, [this] { updateStatusBaseline(); });
  m_global->setLayoutSize(size());
  settingsLayout->addWidget(m_global);
  settingsLayout->addStretch();
  m_systemBrief = new SummaryLabel(m_systemInformation->summaryParts(), globalWrap);
  m_systemBrief->setObjectName(QStringLiteral("systemBrief"));
  m_systemBrief->setProperty("designTone", "muted");
  m_systemBrief->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  connect(m_systemBrief, &SummaryLabel::activated, this, &MainWindow::setStatusDetail);
  settingsLayout->addWidget(m_systemBrief);
  settingsScroll->setWidget(settingsContent);
  settingsContent->setAutoFillBackground(false);
  mainRow->addWidget(globalWrap);
  centralLayout->addLayout(mainRow, 1);

  m_statusSurface = new StatusSurface(central);
  m_statusSurface->setBackdropOpacity(50);
  m_statusCtl = new TransientLabel(m_statusSurface, m_statusSurface);
  m_statusCtl->setBaseline(I18n::tr("Ready"));
  m_hoverHints->setTarget(m_statusCtl);
  connect(m_statusSurface, &StatusSurface::copiedToClipboard, this,
          [this] { showTransientHint(I18n::tr("Copied to clipboard"), design::statusNoticeDurationMs); });
  centralLayout->addWidget(m_statusSurface);
  connect(m_statusSurface, &StatusSurface::discardRequested, this, [this] { confirmDiscardPendingChanges(); });
  setCentralWidget(central);
  refreshThemedUi();
  updateLayoutDensity();
}

void MainWindow::rebuildUi() {
  m_overlayPresentation->bindAction(nullptr);
  // Detach the old shortcut immediately; the old widget tree is released later.
  // Quiet startup can rebuild the view before its first native show.
  if (m_actRefresh) removeAction(m_actRefresh);

  m_hoverHints->setTarget(nullptr);
  if (auto* central = takeCentralWidget()) {
    central->hide();
    central->deleteLater();
  }

  m_actRefresh = m_actImport = m_actPlan = m_actShutdown = m_actRestart = nullptr;
  m_actEnhancedMode = nullptr;
  m_actLog = m_actAbout = m_actLang = m_actTheme = nullptr;
  m_tabs = nullptr;
  m_global = nullptr;
  m_statusSurface = nullptr;
  m_settingsSurface = nullptr;
  m_settingsScroll = nullptr;
  m_commands = nullptr;
  m_systemBrief = nullptr;
  m_statusCtl = nullptr;
  m_volumeTabs.clear();

  buildUi();
  // The committed snapshot is restored before a replacement is requested.
  applyCommittedState();
  // Initialization has no user refresh intent. Internal post-write reconciliation
  // is owned by the ViewModel and also reads without a discard confirmation.
  m_viewModel->refresh();

  setUpdatesEnabled(true);

  // Pending layout requests are delivered before repainting the rebuilt view.
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  repaint();

  refreshWindowBackdrop(this);
}

void MainWindow::retranslateUi() {
  setWindowTitle(I18n::applicationTitle());
  if (!m_global || !m_tabs) return;
  refreshThemedUi();
  updatePendingSummary();
  updateStatusBaseline();
  updateInteractionAvailability();
  if (!m_viewModel->state().hasCommittedState()) return;
  for (const auto& page : m_viewModel->volumePages(m_systemInformation->systemDriveLetter())) {
    for (const auto& tab : std::as_const(m_volumeTabs))
      if (tab && tab->volumeIdentity() == page.identity) {
        m_tabs->setTabText(m_tabs->indexOf(tab), page.title);
        m_tabs->setTabToolTip(m_tabs->indexOf(tab), page.tooltip);
      }
  }
}

void MainWindow::showTransientHint(const QString& text, const int msec) const {
  if (m_statusCtl) m_statusCtl->flash(text, msec);
}

void MainWindow::requestExit() {
  if (!m_viewModel->exitAllowed()) return;
  m_exitRequested = true;

  const auto finishExit = [this]() {
    if (!close()) {
      m_exitRequested = false;
      m_overlayPresentation->restoreHub();
      return;
    }
    QCoreApplication::quit();
  };

  if (m_global && pendingChangeCount() > 0 && !isVisible()) {
    m_overlayPresentation->hideHubTemporarily();
    raiseToFront();
    QTimer::singleShot(0, this, [this, finishExit]() { QTimer::singleShot(0, this, finishExit); });
    return;
  }

  m_overlayPresentation->hideHubTemporarily();
  finishExit();
}

bool MainWindow::confirmPendingChangesDiscard() {
  if (m_viewModel->applyingConfiguration()) return false;
  if (!m_global) return true;
  if (QWidget* fw = QApplication::focusWidget()) fw->clearFocus();

  const std::size_t pending = pendingChangeCount();
  if (pending == 0) return true;

  if (isMinimized()) {
    showNormal();
    raise();
    activateWindow();
  }

  if (!dialogs::confirm(this, I18n::tr("Discard pending changes?"), I18n::tr("%1 change(s) have not been applied.\n\nDiscard them and continue?").arg(pending)))
    return false;
  return true;
}
bool MainWindow::confirmDiscardPendingChanges() {
  if (!confirmPendingChangesDiscard()) return false;
  if (pendingChangeCount() == 0) return true;
  try {
    if (!m_viewModel->discardPendingChanges()) return false;
  } catch (const std::exception& error) {
    dialogs::warning(this, I18n::tr("File staging could not be saved"), QString::fromUtf8(error.what()));
    return false;
  }
  return true;
}

void MainWindow::changeEvent(QEvent* ev) {
  QMainWindow::changeEvent(ev);
  if (ev->type() != QEvent::WindowStateChange || !isMinimized() || !m_overlayPresentation->hubPresented()) return;

  // Deferred handling rechecks minimization without reentering window-state delivery.
  QTimer::singleShot(0, this, [this]() {
    if (isMinimized() && m_overlayPresentation->hubPresented()) hide();
  });
}

void MainWindow::closeEvent(QCloseEvent* ev) {
  if (!m_viewModel->exitAllowed()) {
    ev->ignore();
    return;
  }
  if (!m_exitRequested && m_overlayPresentation->hubPresented()) {
    hide();
    ev->ignore();
    return;
  }
  if (!confirmDiscardPendingChanges()) {
    m_exitRequested = false;
    ev->ignore();
    return;
  }
  QMainWindow::closeEvent(ev);
}

void MainWindow::showEvent(QShowEvent* ev) {
  QMainWindow::showEvent(ev);
  m_chrome->applyTitleBarTheme();
  if (m_initialGeometryPending) {
    m_initialGeometryPending = false;
    // The first show uses measured child geometry for final sizing and placement.
    m_chrome->initializeGeometry();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    PagedWindowLayout::forWindow(this).fit(true);
  }
}

void MainWindow::resizeEvent(QResizeEvent* event) {
  QMainWindow::resizeEvent(event);
  const auto density = [](const QSize& size) {
    return std::tuple(size.width() <= design::compactWindowWidth, size.height() <= design::shortWindowHeight, size.height() <= design::smallWindowHeight);
  };
  PagedWindowLayout::forWindow(this).observeResize(event->size());
  const auto requested = PagedWindowLayout::forWindow(this).requestedSize();
  // Layout managers handle ordinary pixel changes. Reapplying all row sizes,
  // margins and spacers here invalidates the full layout on every native resize
  // step, including a top-edge resize that also changes the client origin.
  if (density(m_layoutDensitySize.isValid() ? m_layoutDensitySize : event->oldSize()) != density(requested)) updateLayoutDensity();
}
void MainWindow::updateLayoutDensity() {
  const auto requested = PagedWindowLayout::forWindow(this).requestedSize();
  const auto layoutSize = requested.isValid() ? requested : size();
  m_layoutDensitySize = layoutSize;
  const bool shortWindow = layoutSize.height() <= design::shortWindowHeight;
  const bool compact = layoutSize.width() <= design::compactWindowWidth;
  if (m_tabs) {
    m_tabs->tabBar()->setFixedHeight(shortWindow ? 38 : 47);
    static_cast<TabView*>(m_tabs)->setCompact(shortWindow);
    if (auto* workspace = m_workspaceStack ? m_workspaceStack->parentWidget() : m_tabs->parentWidget(); workspace && workspace->layout())
      workspace->layout()->setContentsMargins(compact ? 18 : 24, 0, compact ? 18 : 24, design::workspaceBottomPadding);
  }
  for (auto& tab : m_volumeTabs)
    if (tab) tab->setLayoutSize(layoutSize);
  if (m_global) m_global->setLayoutSize(layoutSize);
  if (auto* settings = m_settingsSurface) {
    const bool small = layoutSize.height() <= design::smallWindowHeight;
    const QMargins margins(compact ? 16 : 22, small ? 10 : shortWindow ? 13 : 23, compact ? 15 : 21, small ? 10 : shortWindow ? 13 : 17);
    auto* scroll = m_settingsScroll;
    if (auto* layout = scroll && scroll->widget() ? scroll->widget()->layout() : nullptr) {
      layout->setContentsMargins(margins);
      layout->setSpacing(shortWindow ? 10 : 18);
    }
    const int preferred = compact ? design::compactSettingsWidth : design::settingsWidth;
    const int contentMinimum = m_global ? m_global->minimumSizeHint().width() : 0;
    const int minimum = scroll ? scroll->minimumSizeHint().width() : contentMinimum + margins.left() + margins.right();
    settings->setFixedWidth(std::max(preferred, minimum));
  }
}

bool MainWindow::nativeEvent(const QByteArray& type, void* message, qintptr* result) {
  return m_chrome->nativeEvent(message, result) || QMainWindow::nativeEvent(type, message, result);
}

void MainWindow::refreshThemedUi() {
  auto& tm = ThemeManager::instance();
  if (m_actImport) m_actImport->setIcon(tm.icon(":/icons/import.svg"));
  if (m_actRefresh) m_actRefresh->setIcon(tm.icon(":/icons/refresh.svg"));
  if (m_actPlan) m_actPlan->setIcon(tm.icon(":/icons/apply.svg"));
  if (m_actShutdown) m_actShutdown->setIcon(tm.icon(":/icons/shutdown.svg"));
  if (m_actRestart) m_actRestart->setIcon(tm.icon(":/icons/restart.svg"));
  if (m_actEnhancedMode) {
    m_actEnhancedMode->setIcon(tm.icon(":/icons/enhanced.svg"));
    QColor iconColor;
    switch (m_viewModel->enhancedModeStatus().state) {
      case app::EnhancedModeState::Enabled:
        iconColor = m_viewModel->enhancedModeStatus().agentState == app::EnhancedModeAgentState::Connected ? tm.color(Sem::AddOk) : tm.color(Sem::Accent);
        break;
      case app::EnhancedModeState::Stopped:
      case app::EnhancedModeState::RepairRequired:
        iconColor = tm.color(Sem::Warn);
        break;
      case app::EnhancedModeState::Disabled:
        break;
    }
    if (auto* button = m_commands->buttonForAction(m_actEnhancedMode)) {
      button->setProperty("designIconColor", iconColor);
      button->update();
    }
  }
  if (m_actLog) m_actLog->setIcon(tm.icon(":/icons/log.svg"));
  if (m_actAbout) m_actAbout->setIcon(tm.icon(":/icons/info.svg"));
  constexpr Qt::Alignment kCenter = Qt::AlignHCenter | Qt::AlignVCenter;
  m_overlayPresentation->refreshActionIcon();
  if (m_actLang) m_actLang->setIcon(tm.icon(":/icons/language.svg", kCenter));
  if (m_actTheme) {
    const bool isDark = tm.current() == app::Theme::Dark;
    m_actTheme->setIcon(tm.icon(isDark ? ":/icons/theme_sun.svg" : ":/icons/theme_moon.svg", kCenter));
  }
  if (m_tabs) {
    for (const auto& page : m_viewModel->volumePages(m_systemInformation->systemDriveLetter())) {
      for (int index = 0; index < m_tabs->count(); ++index) {
        if (m_tabs->tabBar()->tabData(index).toString() != page.identity) continue;
        m_tabs->setTabIcon(index, tm.icon(page.icon));
        break;
      }
    }
  }
  if (m_systemBrief) m_systemBrief->setParts(m_systemInformation->summaryParts());
}

std::size_t MainWindow::pendingChangeCount() const { return m_viewModel->pendingChangeCount(); }

void MainWindow::updatePendingSummary() {
  const std::size_t pending = pendingChangeCount();
  if (m_statusSurface) m_statusSurface->setPending(pending);
  if (auto* commands = m_commands; commands && m_actPlan) {
    auto* button = commands->buttonForAction(m_actPlan);
    button->setProperty("designRole", pending > 0 ? "primary" : "quiet");
    button->update();
  }
}

void MainWindow::rebuildTabs() {
  // Stable volume identities preserve hidden-partition selections separately.
  const QString prevIdentity = m_tabs->currentIndex() >= 0 ? m_tabs->tabBar()->tabData(m_tabs->currentIndex()).toString() : QString();
  QMap<QString, VolumeTab::InfoPage> prevInfoTab;
  for (const auto& t : std::as_const(m_volumeTabs))
    if (t) prevInfoTab.insert(t->volumeIdentity(), t->activeInfoPage());

  // QTabWidget::clear() does not destroy pages; deferred deletion preserves active signal senders.
  for (const auto& t : std::as_const(m_volumeTabs))
    if (t) t->deleteLater();
  m_tabs->clear();
  m_volumeTabs.clear();
  const QString sysDl = m_systemInformation->systemDriveLetter();

  for (const auto& page : m_viewModel->volumePages(sysDl)) {
    auto model = page.model;
    auto staging = page.hasStaging ? m_viewModel->fileStagingModel(model) : nullptr;
    auto* tab = new VolumeTab(std::move(model), m_fileDialogs, std::move(staging), m_viewModelFactory, this, m_viewModel.get());
    auto& tm = ThemeManager::instance();
    const int idx = m_tabs->addTab(tab, tm.icon(page.icon), page.title);
    if (page.system) m_tabs->tabBar()->setProperty("designAccentTab", idx);
    m_tabs->tabBar()->setTabData(idx, page.identity);
    m_tabs->setTabToolTip(idx, page.tooltip);
    if (const auto it = prevInfoTab.constFind(page.identity); it != prevInfoTab.constEnd()) {
      tab->setActiveInfoPage(it.value());
    }
    m_volumeTabs.push_back(tab);
    connect(tab, &VolumeTab::refreshRequested, this, &MainWindow::refresh);
    connect(tab, &VolumeTab::statusHint, this, &MainWindow::showTransientHint);
    connect(tab, &VolumeTab::selectionDetailChanged, this, [this, tab](const QString& detail) {
      if (m_tabs->currentWidget() == tab) setStatusDetail(detail);
    });
    connect(tab, &VolumeTab::commitFileRequested, this, &MainWindow::commitFilePath);
    connect(tab, &VolumeTab::commitFileDeletionRequested, this, &MainWindow::commitFileDeletionPath);
    connect(tab, &VolumeTab::commitRegistryRequested, this, &MainWindow::commitRegistryKey);
    connect(tab, &VolumeTab::commitRegistryDeletionRequested, this, &MainWindow::commitRegistryDeletionKey);
    connect(tab, &VolumeTab::fullCommitRequested, this, [this] {
      if (!m_viewModel->overlayMaintenance()) return;
      FullCommitDialog dialog(m_viewModel->overlayMaintenance(), this);
      dialog.exec();
    });
  }

  if (!prevIdentity.isEmpty()) {
    for (int i = 0; i < m_tabs->count(); ++i) {
      if (m_tabs->tabBar()->tabData(i).toString() == prevIdentity) {
        m_tabs->setCurrentIndex(i);
        break;
      }
    }
  }
}

void MainWindow::refresh() {
  if (!m_viewModel->interactionState().refreshEnabled || !confirmPendingChangesDiscard()) return;
  m_viewModel->refreshDiscardingChanges();
}

void MainWindow::applyCommittedState() {
  if (!m_viewModel->state().hasCommittedState() || !m_global || !m_tabs) return;

  // An unavailable UWF capability is still a committed snapshot with volume inventory.
  rebuildTabs();
  updateInteractionAvailability();
  updateLayoutDensity();
  updateStatusBaseline();
}

void MainWindow::setStatusDetail(const QString& text) {
  if (m_statusSurface) m_statusSurface->setDetail(text);
  updateStatusBaseline();
  if (m_statusCtl) m_statusCtl->restoreAfter(0);
}

void MainWindow::updateStatusBaseline() {
  if (!m_statusCtl) return;
  if (m_statusSurface && !m_statusSurface->detail().isEmpty()) {
    m_statusCtl->setBaseline(m_statusSurface->detail());
  } else if (m_viewModel->state().hasCommittedState() && !m_viewModel->state().snapshot().uwfAvailable) {
    m_statusCtl->setBaseline(I18n::tr("UWF is not available"));
  } else if (const auto restriction = m_viewModel->configurationWriteRestriction(); !restriction.isEmpty()) {
    m_statusCtl->setBaseline(restriction);
  } else {
    const QString warning = m_global ? m_global->usageWarningText() : QString();
    m_statusCtl->setBaseline(warning.isEmpty() ? I18n::tr("Ready") : warning);
  }
}

void MainWindow::updateInteractionAvailability() {
  const auto view = m_viewModel->interactionState();
  if (m_statusSurface) m_statusSurface->setRuntimeModes(m_viewModel->runtimeModesText(m_compatibilityMode));
  if (m_overlayPresentation) m_overlayPresentation->setOperationAvailability(view.powerEnabled, view.exitEnabled);
  if (!m_global) return;
  renderWorkspace();

  m_global->setToolTip(view.configurationHint);
  if (m_statusSurface && view.accessKnown) m_statusSurface->setAccess(view.elevated);
  if (m_actRefresh) m_actRefresh->setEnabled(view.refreshEnabled);
  if (m_actImport) {
    m_actImport->setEnabled(view.configurationEnabled);
    m_actImport->setToolTip(view.importHint);
  }
  if (m_actPlan) {
    m_actPlan->setEnabled(view.reviewEnabled);
    m_actPlan->setToolTip(view.reviewHint);
  }
  for (QAction* action : {m_actShutdown, m_actRestart})
    if (action) {
      action->setEnabled(view.powerEnabled);
      action->setToolTip(view.powerHint);
    }
  // An unavailable UWF snapshot does not establish the process's access level.
  // Keep the independent service settings reachable until access is known.
  if (m_actEnhancedMode) m_actEnhancedMode->setEnabled(view.enhancedModeEnabled);

  // Page-level admission also blocks writes while reconciliation is required.
  // Keep the diagnostic's Read again button reachable; editors and commands
  // inside the page retain their separate failed-state admission.
  const bool retryAvailable = view.refreshEnabled && m_viewModel->workspaceDiagnostic().has_value();
  for (auto& tab : m_volumeTabs) {
    if (!tab) continue;
    tab->setToolTip(view.configurationHint);
    tab->setEnabled(view.volumePagesEnabled || retryAvailable);
  }
}

void MainWindow::requestSafeShutdown() { m_viewModel->safeShutdown(); }
void MainWindow::requestSafeRestart() { m_viewModel->safeRestart(); }
void MainWindow::requestDirectRestart() { m_viewModel->directRestart(); }

void MainWindow::renderWorkspace() {
  const auto diagnostic = m_viewModel->workspaceDiagnostic();
  m_workspaceState->setAction(I18n::tr("Read again"), ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")));
  m_workspaceState->setText(I18n::tr("UWF is currently unavailable"),
                            I18n::tr("UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.") +
                                u'\n' + diagnostic.value_or(QString{}));
  m_workspaceStack->setCurrentWidget(diagnostic && m_volumeTabs.isEmpty() ? static_cast<QWidget*>(m_workspaceState) : m_tabs);
}

void MainWindow::showPlan() {
  if (!m_viewModel->configurationReviewAllowed()) return;
  // Toolbar clicks do not move focus; clearing it commits spinbox edits before plan collection.
  if (QWidget* fw = QApplication::focusWidget()) fw->clearFocus();

  m_viewModel->requestReview();
}

void MainWindow::showImport() {
  auto model = m_viewModel->importModel();
  if (!model) return;
  ImportDialog dlg(std::move(model), m_fileDialogs, this);
  dlg.exec();
  if (dlg.reviewRequested()) showPlan();
}

void MainWindow::showAbout() {
  AboutDialog dlg([this] { return m_systemInformation->diagnosticDocument(m_viewModel->diagnosticContext(m_compatibilityMode)); }, this);
  dlg.setConfigurationExporter([this, exporter = m_viewModelFactory->userConfigExport()] {
    app::UserConfig config;
    config.theme = ThemeManager::instance().current();
    config.language = I18n::instance().lang();
    config.hubVisible = m_overlayPresentation->hubRequestedVisible();
    return exporter->exportConfig(config);
  });
  dlg.exec();
}

void MainWindow::showLogs() {
  LogViewerDialog dlg(m_viewModelFactory->log(), this);
  dlg.exec();
}

void MainWindow::showSystemInformation() {
  SystemInformationDialog dialog(m_systemInformation->diagnosticDocument(m_viewModel->diagnosticContext(m_compatibilityMode)), this);
  dialog.exec();
}

void MainWindow::showEnhancedMode() {
  auto model = m_viewModel->enhancedModeModel();
  if (!model) return;
  EnhancedModeDialog dialog(std::move(model), this);
  dialog.exec();
  m_viewModel->refreshEnhancedModeStatus();
}

void MainWindow::commitFilePath(const QString& path) {
  if (m_viewModel->maintenanceActionsAllowed()) m_commit->commitFilePath(path);
}
void MainWindow::commitFileDeletionPath(const QString& path) {
  if (m_viewModel->maintenanceActionsAllowed()) m_commit->commitFileDeletionPath(path);
}
void MainWindow::commitRegistryKey(const QString& key, const std::optional<QString>& valueName) {
  if (m_viewModel->maintenanceActionsAllowed()) m_commit->commitRegistryKey(key, valueName);
}
void MainWindow::commitRegistryDeletionKey(const QString& key, const std::optional<QString>& valueName) {
  if (m_viewModel->maintenanceActionsAllowed()) m_commit->commitRegistryDeletionKey(key, valueName);
}

}  // namespace uwf::ui

#include "moc_MainWindow.cpp"
