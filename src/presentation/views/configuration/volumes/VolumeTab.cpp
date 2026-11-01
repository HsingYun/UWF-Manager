// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/configuration/volumes/VolumeTab.h"

#include <QAction>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStringList>
#include <QTabBar>
#include <QTabWidget>
#include <QWidgetAction>
#include <optional>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentState.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/navigation/CommandMenu.h"
#include "presentation/controls/navigation/TabView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"
#include "presentation/views/configuration/exclusions/ExclusionListWidget.h"
#include "presentation/views/configuration/volumes/VolumeStatusPanel.h"
#include "presentation/views/overlay/files/OverlayFilesDialog.h"
#include "presentation/views/registry/RegistryPickerDialog.h"
#include "presentation/views/staging/FileStagingWidget.h"

namespace uwf::ui {

VolumeTab::VolumeTab(std::shared_ptr<presentation::VolumeConfigurationViewModel> model, dialogs::FileDialogProvider& fileDialogs,
                     std::shared_ptr<presentation::FileStagingViewModel> stagingModel, std::shared_ptr<presentation::ViewModelFactory> viewModelFactory,
                     QWidget* parent, presentation::MainWindowViewModel* windowModel)
    : QWidget(parent),
      m_viewModelFactory(std::move(viewModelFactory)),
      m_windowModel(windowModel),
      m_viewModel(std::move(model)),
      m_volume(m_viewModel->volume()),
      m_fileDialogs(fileDialogs) {
  const auto& volume = m_volume;
  const bool showRegistry = static_cast<bool>(m_viewModel->registry());
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 14, 0, 0);
  layout->setSpacing(14);
  auto* heading = new QHBoxLayout();
  heading->setSpacing(16);
  auto* headingText = new QVBoxLayout();
  headingText->setSpacing(4);

  auto* titleRow = new QHBoxLayout;
  titleRow->setSpacing(8);
  m_headingLabel = new QLabel(QString::fromStdString(volume.driveLetter), this);
  m_headingLabel->setObjectName("diskHeading");
  m_headingLabel->setMinimumHeight(35);
  m_headingLabel->setTextFormat(Qt::PlainText);
  m_headingLabel->setLayoutDirection(Qt::LeftToRight);
  auto titleFont = m_headingLabel->font();
  titleFont.setPixelSize(22);
  titleFont.setWeight(QFont::DemiBold);
  m_headingLabel->setFont(titleFont);
  m_headingLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_headingLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  m_headingLabel->setToolTip(QString::fromStdString(volume.volumeName));
  titleRow->addWidget(m_headingLabel, 0, Qt::AlignBaseline);
  m_volumeLabel = new TextLabel(QString::fromStdString(volume.label), TextStyle::Label, this);
  m_volumeLabel->setObjectName("diskVolumeLabel");
  m_volumeLabel->setColorRole(design::Color::Muted);
  m_volumeLabel->setElideMode(Qt::ElideRight);
  m_volumeLabel->setAlignment(Qt::AlignLeading | Qt::AlignBottom);
  m_volumeLabel->setVerticalReference(TextVerticalReference::GlyphBounds);
  m_volumeLabel->setMinimumHeight(35);
  m_volumeLabel->setToolTip(m_volumeLabel->text());
  m_volumeLabel->setVisible(!volume.label.empty());
  titleRow->addWidget(m_volumeLabel, 0, Qt::AlignBaseline);
  titleRow->addStretch(1);
  headingText->addLayout(titleRow);
  auto* capacity = new TextLabel(m_viewModel->capacityText(), TextStyle::Eyebrow, this);
  m_capacityLabel = capacity;
  capacity->setObjectName(QStringLiteral("volumeMetadata"));
  capacity->setProperty("designTone", "faint");
  capacity->setTextFormat(Qt::PlainText);
  capacity->setLayoutDirection(Qt::LeftToRight);
  capacity->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  capacity->setFocusPolicy(Qt::ClickFocus);
  capacity->setElideMode(Qt::ElideMiddle);
  capacity->setWordWrap(false);
  headingText->addWidget(capacity);
  heading->addLayout(headingText, 1);
  layout->addLayout(heading);

  m_runtimeNotice = new Notice(this);
  m_runtimeNotice->setObjectName(QStringLiteral("volumeRuntimeNotice"));
  m_runtimeNotice->layout()->setContentsMargins(13, 10, 13, 10);
  m_runtimeNotice->setFlat(true);
  m_runtimeNotice->hide();
  layout->addWidget(m_runtimeNotice);
  m_status = new VolumeStatusPanel(m_viewModel->status(), this);
  m_status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(m_status);

  m_showRegistry = showRegistry;

  const auto& tm = ThemeManager::instance();

  m_overlayBtn = new QPushButton(tm.icon(":/icons/disk.svg"), QString{});
  I18n::bind(m_overlayBtn, [target = m_overlayBtn] { target->setText(I18n::tr("View overlay")); });

  m_overlayBtn->setEnabled(false);
  connect(m_overlayBtn, &QPushButton::clicked, this, [this]() {
    auto* dlg = new OverlayFilesDialog(m_viewModelFactory->overlayFiles(m_viewModel, m_windowModel), m_fileDialogs, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &OverlayFilesDialog::commitFileRequested, this, &VolumeTab::commitFileRequested);
    dlg->show();
  });
  m_overlayBtn->setProperty("designRole", "quiet");
  m_overlayBtn->setFixedHeight(31);
  heading->addWidget(m_overlayBtn, 0, Qt::AlignVCenter);

  m_commitBtn = new QPushButton(tm.icon(":/icons/commit.svg"), QString{});
  I18n::bind(m_commitBtn, [target = m_commitBtn] { target->setText(I18n::tr("Commit") + QChar(0x2026)); });
  m_commitBtn->setObjectName(QStringLiteral("volumeCommitButton"));
  m_commitBtn->setProperty("designPaddingX", 9);
  m_overlayBtn->setProperty("designPaddingX", 9);
  auto* commitMenu = new CommandMenu(m_commitBtn);
  commitMenu->setObjectName(QStringLiteral("volumeCommitMenu"));

  commitMenu->setToolTipsVisible(true);
  const auto addMenuNote = [commitMenu](const QString& name, QAction*& action, TextLabel*& note) {
    note = new TextLabel({}, TextStyle::Eyebrow, commitMenu);
    note->setObjectName(name);
    note->setColorRole(design::Color::Muted);
    note->setLineHeight(1.7);
    note->setWordWrap(true);
    note->setMaximumWidth(290);
    note->setContentsMargins(12, 6, 12, 6);
    note->setFocusPolicy(Qt::NoFocus);
    auto* widgetAction = new QWidgetAction(commitMenu);
    widgetAction->setDefaultWidget(note);
    widgetAction->setEnabled(false);
    commitMenu->addAction(widgetAction);
    action = widgetAction;
  };
  addMenuNote(QStringLiteral("individualCommitMenuNote"), m_individualCommitNoteAct, m_individualCommitNote);
  m_commitFileAct = commitMenu->addAction(tm.icon(":/icons/file.svg"), QString{});
  I18n::bind(m_commitFileAct, [target = m_commitFileAct] { target->setText(I18n::tr("Commit file changes…")); });
  I18n::bind(m_commitFileAct, [target = m_commitFileAct] { target->setToolTip(I18n::tr("Pick a file and commit its overlay changes to disk.")); });
  m_commitDirAct = commitMenu->addAction(tm.icon(":/icons/folder.svg"), QString{});
  I18n::bind(m_commitDirAct, [target = m_commitDirAct] { target->setText(I18n::tr("Commit folder changes…")); });
  I18n::bind(m_commitDirAct,
             [target = m_commitDirAct] { target->setToolTip(I18n::tr("Pick a folder and commit overlay changes for every file inside it to disk.")); });
  if (m_showRegistry) {
    m_commitRegAct = commitMenu->addAction(tm.icon(":/icons/registry.svg"), QString{});
    I18n::bind(m_commitRegAct, [target = m_commitRegAct] { target->setText(I18n::tr("Commit registry changes…")); });
    I18n::bind(m_commitRegAct, [target = m_commitRegAct] {
      target->setToolTip(I18n::tr(
          "Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the "
          "key and all its subkeys (recursively)."));
    });
  }
  commitMenu->addSeparator();
  m_commitFileDeleteAct = commitMenu->addAction(tm.icon(":/icons/file.svg"), QString{});
  I18n::bind(m_commitFileDeleteAct, [target = m_commitFileDeleteAct] { target->setText(I18n::tr("Delete and commit file…")); });
  I18n::bind(m_commitFileDeleteAct,
             [target = m_commitFileDeleteAct] { target->setToolTip(I18n::tr("Pick a file to delete, and commit the deletion to disk.")); });
  m_commitFolderDeleteAct = commitMenu->addAction(tm.icon(":/icons/folder.svg"), QString{});
  I18n::bind(m_commitFolderDeleteAct, [target = m_commitFolderDeleteAct] { target->setText(I18n::tr("Delete and commit folder…")); });
  I18n::bind(m_commitFolderDeleteAct, [target = m_commitFolderDeleteAct] {
    target->setToolTip(I18n::tr("Select a folder, recursively delete it and all its contents, and commit the deletions to disk."));
  });
  if (m_showRegistry) {
    m_commitRegDeleteAct = commitMenu->addAction(tm.icon(":/icons/registry.svg"), QString{});
    I18n::bind(m_commitRegDeleteAct, [target = m_commitRegDeleteAct] { target->setText(I18n::tr("Delete and commit registry…")); });
    I18n::bind(m_commitRegDeleteAct, [target = m_commitRegDeleteAct] {
      target->setToolTip(
          I18n::tr("Enter a registry key and an optional value name. Leave the value name empty to delete the key and all its subkeys and values, then commit "
                   "the deletions."));
    });
  }
  for (auto* action : {m_commitFileDeleteAct, m_commitFolderDeleteAct, m_commitRegDeleteAct})
    if (action) action->setProperty("designRole", "destructive");
  commitMenu->addSeparator();
  m_fullCommitAct = commitMenu->addAction(tm.icon(":/icons/disk.svg"), QString{});
  m_fullCommitAct->setObjectName(QStringLiteral("volumeFullCommitAction"));
  connect(m_fullCommitAct, &QAction::triggered, this, [this] {
    const auto maintenance = m_windowModel ? m_windowModel->overlayMaintenance() : nullptr;
    if (maintenance && maintenance->fullCommitState().available) emit fullCommitRequested();
  });
  m_removeFullCommitDraftAct = commitMenu->addAction(tm.icon(":/icons/remove.svg"), QString{});
  m_removeFullCommitDraftAct->setObjectName(QStringLiteral("volumeRemoveFullCommitDraftAction"));
  I18n::bind(m_removeFullCommitDraftAct, [target = m_removeFullCommitDraftAct] { target->setText(I18n::tr("Remove full volume commit draft")); });
  connect(m_removeFullCommitDraftAct, &QAction::triggered, this, [this] {
    if (const auto maintenance = m_windowModel ? m_windowModel->overlayMaintenance() : nullptr; maintenance && maintenance->fullCommitState().pending)
      maintenance->unstageFullCommit();
  });
  QAction* fullCommitNoteAction = nullptr;
  addMenuNote(QStringLiteral("fullCommitMenuNote"), fullCommitNoteAction, m_fullCommitNote);
  m_commitBtn->setMenu(commitMenu);
  m_commitBtn->setFixedHeight(31);
  heading->addWidget(m_commitBtn, 0, Qt::AlignVCenter);

  m_infoTabs = new TabView(this);
  m_infoTabs->setElideMode(Qt::ElideNone);
  m_infoTabs->setUsesScrollButtons(true);
  m_infoTabs->setObjectName("innerTabs");
  static_cast<TabView*>(m_infoTabs)->setKind(TabView::Kind::Entries);
  m_infoTabs->setDocumentMode(true);
  m_infoTabs->setMinimumHeight(120);
  const auto wrapPage = [this](QWidget* content) {
    auto* host = new QStackedWidget(m_infoTabs);
    host->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    host->addWidget(content);
    auto* error = new ContentState(host, ContentState::ScrollMode::SelfManaged);
    error->setObjectName(QStringLiteral("volumeUnavailableState"));
    error->setText(I18n::tr("UWF is currently unavailable"), {});
    error->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/enhanced.svg")));
    error->setAction(I18n::tr("Read again"), ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")));
    connect(error, &ContentState::actionRequested, this, &VolumeTab::refreshRequested);
    host->addWidget(error);
    m_pageHosts.insert(content, host);
    return host;
  };
  m_files = new ExclusionListWidget(m_viewModel->files(), m_fileDialogs, this);
  const int fileIdx = m_infoTabs->addTab(wrapPage(m_files), tm.icon(":/icons/file.svg"), I18n::tr("File exclusions"));
  m_infoTabs->setTabToolTip(fileIdx, I18n::tr("Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path."));

  // Global registry exclusions appear on the designated system-volume page.
  if (m_showRegistry) {
    m_regs = new ExclusionListWidget(m_viewModel->registry(), m_fileDialogs, this);
    connect(m_regs, &ExclusionListWidget::registryPickerRequested, this, [this] {
      const auto picked =
          RegistryPickerDialog::pick(m_viewModelFactory->registryPicker(RegistryPickerDialog::Mode::Exclusion), I18n::tr("Add registry exclusion"), this);
      if (picked) m_regs->addSelectedPath(picked->key);
    });
    const int regIdx = m_infoTabs->addTab(wrapPage(m_regs), tm.icon(":/icons/registry.svg"), I18n::tr("Registry exclusions"));
    m_infoTabs->setTabToolTip(regIdx, I18n::tr("Registry exclusions are global and are shown only on the system drive."));
  }

  if (stagingModel) {
    m_fileStaging = new FileStagingWidget(std::move(stagingModel), m_fileDialogs, this);
    const int stagingIndex = m_infoTabs->addTab(wrapPage(m_fileStaging), tm.icon(":/icons/file_staging.svg"), I18n::tr("File staging"));
    m_infoTabs->setTabToolTip(stagingIndex, m_fileStaging->viewModel().automaticCommitPresentation().description);
    connect(m_fileStaging, &FileStagingWidget::enhancedModeAvailabilityChanged, this, &VolumeTab::renderState);
    connect(&m_fileStaging->viewModel(), &presentation::FileStagingViewModel::automaticCommitStateChanged, this, &VolumeTab::renderState);
  }

  m_listCommands = new QStackedWidget(m_infoTabs);
  m_listCommands->setObjectName(QStringLiteral("mainListCommands"));
  m_listCommands->addWidget(m_files->commandWidget());
  connect(m_files, &ExclusionListWidget::selectionDetailChanged, this, [this](const QString& detail) {
    if (activeInfoPage() == InfoPage::FileExclusions) emit selectionDetailChanged(detail);
  });
  if (m_regs) {
    m_listCommands->addWidget(m_regs->commandWidget());
    connect(m_regs, &ExclusionListWidget::selectionDetailChanged, this, [this](const QString& detail) {
      if (activeInfoPage() == InfoPage::RegistryExclusions) emit selectionDetailChanged(detail);
    });
  }
  if (m_fileStaging) {
    m_listCommands->addWidget(m_fileStaging->commandWidget());
    connect(m_fileStaging, &FileStagingWidget::selectionDetailChanged, this, [this](const QString& detail) {
      if (activeInfoPage() == InfoPage::FileStaging) emit selectionDetailChanged(detail);
    });
  }
  m_infoTabs->setCornerWidget(m_listCommands, Qt::TopRightCorner);
  connect(m_infoTabs, &QTabWidget::currentChanged, this, [this](int index) {
    m_listCommands->setCurrentIndex(index);
    emit selectionDetailChanged(QString());
  });
  layout->addWidget(m_infoTabs, 1);
  for (auto* button : {m_overlayBtn, m_commitBtn}) {
    auto type = button->font();
    type.setPixelSize(11);
    type.setWeight(QFont::Medium);
    button->setFont(type);
  }

  connect(m_viewModel.get(), &presentation::VolumeConfigurationViewModel::stateChanged, this, &VolumeTab::renderState);
  if (m_windowModel) {
    connect(m_windowModel, &presentation::MainWindowViewModel::interactionAvailabilityChanged, this, &VolumeTab::renderState);
    if (const auto& maintenance = m_windowModel->overlayMaintenance()) {
      connect(maintenance.get(), &presentation::OverlayMaintenanceViewModel::stateChanged, this, &VolumeTab::renderState);
      connect(maintenance.get(), &presentation::OverlayMaintenanceViewModel::busyChanged, this, &VolumeTab::renderState);
    }
  }
  const auto onCopied = [this] { emit statusHint(I18n::tr("Copied to clipboard"), design::statusNoticeDurationMs); };
  connect(m_files, &ExclusionListWidget::copiedToClipboard, this, onCopied);
  if (m_fileStaging) connect(m_fileStaging, &FileStagingWidget::copiedToClipboard, this, onCopied);
  connect(m_files, &ExclusionListWidget::commitFileRequested, this, &VolumeTab::commitFileRequested);
  if (m_regs) {
    connect(m_regs, &ExclusionListWidget::copiedToClipboard, this, onCopied);
  }

  connect(m_commitFileAct, &QAction::triggered, this, &VolumeTab::onCommitFile);
  connect(m_commitDirAct, &QAction::triggered, this, &VolumeTab::onCommitDir);
  connect(m_commitFileDeleteAct, &QAction::triggered, this, &VolumeTab::onCommitFileDelete);
  connect(m_commitFolderDeleteAct, &QAction::triggered, this, &VolumeTab::onCommitFolderDelete);
  if (m_commitRegAct) {
    connect(m_commitRegAct, &QAction::triggered, this, &VolumeTab::onCommitRegistry);
  }
  if (m_commitRegDeleteAct) {
    connect(m_commitRegDeleteAct, &QAction::triggered, this, &VolumeTab::onCommitRegistryDelete);
  }

  if (!m_volume.driveLetter.empty() && !supported()) {
    markUnsupported();
  } else if (!m_volume.driveLetter.empty() && !canManageExclusions()) {
    markLimitedFileSystem();
  }

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](app::Theme) { refreshThemedIcons(); });
  I18n::bind(this, [this] { renderState(); });
}

void VolumeTab::renderState() {
  m_volume = m_viewModel->volume();
  renderVolumeMetadata();
  const auto translatePage = [this](QWidget* page, const QString& title, const QString& hint) {
    if (!page) return;
    const int index = m_infoTabs->indexOf(pageHost(page));
    m_infoTabs->setTabText(index, title);
    m_infoTabs->setTabToolTip(index, hint);
  };
  translatePage(m_files, I18n::tr("File exclusions"),
                I18n::tr("Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path."));
  translatePage(m_regs, I18n::tr("Registry exclusions"), I18n::tr("Registry exclusions are global and are shown only on the system drive."));
  if (m_fileStaging) {
    const int index = m_infoTabs->indexOf(pageHost(m_fileStaging));
    const auto automatic = m_fileStaging->viewModel().automaticCommitPresentation();
    QString indicator;
    auto hint = automatic.description;
    if (automatic.unavailable || automatic.paused)
      indicator = QStringLiteral(":/icons/status_warning.svg");
    else if (!automatic.alreadyPersistent && !m_fileStaging->enhancedModeAvailable()) {
      indicator = QStringLiteral(":/icons/enhanced.svg");
      hint += u'\n' + I18n::tr("Enhanced mode is unavailable. Use Safe shutdown or Safe restart to commit staged files.");
    }
    m_infoTabs->setTabText(index, I18n::tr("File staging"));
    static_cast<TabView*>(m_infoTabs)->setTabIndicator(index, indicator, hint);
  }
  const auto failure = diagnostic();
  m_status->setEnabled(!failure);
  const auto detail = failure.value_or(QString{});
  for (auto* host : std::as_const(m_pageHosts)) {
    auto* error = qobject_cast<ContentState*>(host->widget(1));
    error->setText(
        I18n::tr("UWF is currently unavailable"),
        detail.isEmpty()
            ? QString{}
            : I18n::tr("UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.") + u'\n' + detail);
    error->setAction(I18n::tr("Read again"), ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")));
    host->setCurrentIndex(failure ? 1 : 0);
  }
  m_listCommands->setEnabled(!failure);
  if (!m_volume.driveLetter.empty() && !supported())
    markUnsupported();
  else if (!m_volume.driveLetter.empty() && !canManageExclusions())
    markLimitedFileSystem();
  renderCommitAvailability(failure.has_value());
  renderRuntimeNotice();
}

void VolumeTab::setLayoutSize(const QSize& size) {
  const bool shortWindow = size.height() <= design::shortWindowHeight;
  layout()->setContentsMargins(0, shortWindow ? 10 : 14, 0, 0);
  layout()->setSpacing(shortWindow ? 10 : 14);
  m_status->setLayoutSize(size);
  if (m_infoTabs) static_cast<TabView*>(m_infoTabs)->setCompact(size.width() <= design::compactWindowWidth);
  for (QWidget* page : {static_cast<QWidget*>(m_files), static_cast<QWidget*>(m_regs), static_cast<QWidget*>(m_fileStaging)}) {
    if (!page || !page->layout()) continue;
    page->layout()->setContentsMargins(0, shortWindow ? 10 : 14, 0, 0);
    page->layout()->setSpacing(shortWindow ? 10 : 14);
  }
}

void VolumeTab::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  if (m_listCommands)
    m_listCommands->setFixedWidth(width() < design::narrowListCommandsBreakpoint   ? design::narrowListCommandsWidth
                                  : width() < design::mediumListCommandsBreakpoint ? design::mediumListCommandsWidth
                                                                                   : design::wideListCommandsWidth);
}

void VolumeTab::refreshThemedIcons() {
  auto& tm = ThemeManager::instance();
  if (m_overlayBtn) m_overlayBtn->setIcon(tm.icon(":/icons/disk.svg"));
  if (m_commitBtn) m_commitBtn->setIcon(tm.icon(":/icons/commit.svg"));
  if (m_commitFileAct) m_commitFileAct->setIcon(tm.icon(":/icons/file.svg"));
  if (m_commitDirAct) m_commitDirAct->setIcon(tm.icon(":/icons/folder.svg"));
  if (m_commitFileDeleteAct) m_commitFileDeleteAct->setIcon(tm.icon(":/icons/file.svg"));
  if (m_commitFolderDeleteAct) m_commitFolderDeleteAct->setIcon(tm.icon(":/icons/folder.svg"));
  if (m_commitRegAct) m_commitRegAct->setIcon(tm.icon(":/icons/registry.svg"));
  if (m_commitRegDeleteAct) m_commitRegDeleteAct->setIcon(tm.icon(":/icons/registry.svg"));
  if (m_fullCommitAct) m_fullCommitAct->setIcon(tm.icon(":/icons/disk.svg"));
  if (m_removeFullCommitDraftAct) m_removeFullCommitDraftAct->setIcon(tm.icon(":/icons/remove.svg"));
  if (m_infoTabs) {
    if (m_files) {
      const int idx = m_infoTabs->indexOf(pageHost(m_files));
      if (idx >= 0) m_infoTabs->setTabIcon(idx, tm.icon(":/icons/file.svg"));
    }
    if (m_regs) {
      const int idx = m_infoTabs->indexOf(pageHost(m_regs));
      if (idx >= 0) m_infoTabs->setTabIcon(idx, tm.icon(":/icons/registry.svg"));
    }
    if (m_fileStaging) {
      const int idx = m_infoTabs->indexOf(pageHost(m_fileStaging));
      if (idx >= 0) m_infoTabs->setTabIcon(idx, tm.icon(":/icons/file_staging.svg"));
    }
  }
}

void VolumeTab::renderVolumeMetadata() {
  m_headingLabel->setText(m_volume.driveLetter.empty() ? I18n::tr("Partition") : QString::fromStdString(m_volume.driveLetter));
  m_headingLabel->setToolTip(QString::fromStdString(m_volume.volumeName));
  const QFontMetricsF titleMetrics(m_headingLabel->font());
  const qreal titleBaseline = (m_headingLabel->minimumHeight() - titleMetrics.height()) / 2.0 + titleMetrics.ascent();
  const qreal titleInkBottom = titleBaseline + titleMetrics.tightBoundingRect(m_headingLabel->text()).bottom();
  m_volumeLabel->setContentsMargins(0, 0, 0, qMax(0, qRound(m_headingLabel->minimumHeight() - titleInkBottom)));
  m_volumeLabel->setText(QString::fromStdString(m_volume.label));
  m_volumeLabel->setToolTip(m_volumeLabel->text());
  m_volumeLabel->setVisible(!m_volume.label.empty());
  QStringList metadata;
  if (const auto capacity = m_viewModel->capacityText(); !capacity.isEmpty()) metadata << capacity;
  if (!m_volume.volumeName.empty()) metadata << volumeIdentity();
  m_capacityLabel->setText(metadata.join(QStringLiteral(" · ")));
  m_capacityLabel->setToolTip(m_capacityLabel->text());
}

void VolumeTab::onCommitFile() {
  if (!canCommitFiles()) return;
  const QString path = m_fileDialogs.openFile(
      this, {I18n::tr("Select a file to commit to disk"), dialogs::dialogBasePath(driveLetter()), {}, {dialogs::dialogBasePath(driveLetter())}});
  if (path.isEmpty() || !canCommitFiles()) return;
  emit commitFileRequested(QDir::toNativeSeparators(path));
}

void VolumeTab::onCommitDir() {
  if (!canCommitFiles()) return;
  const QString path = m_fileDialogs.selectDirectory(
      this, {I18n::tr("Select a folder to commit to disk"), dialogs::dialogBasePath(driveLetter()), {}, {dialogs::dialogBasePath(driveLetter())}});
  if (path.isEmpty() || !canCommitFiles()) return;
  emit commitFileRequested(QDir::toNativeSeparators(path));
}

void VolumeTab::onCommitFileDelete() {
  if (!canCommitFiles()) return;
  // UWF performs the deletion; the picker selects an existing target.
  const QString path = m_fileDialogs.openFile(
      this,
      {I18n::tr("Select a file whose deletion you want to commit"), dialogs::dialogBasePath(driveLetter()), {}, {dialogs::dialogBasePath(driveLetter())}});
  if (path.isEmpty() || !canCommitFiles()) return;
  emit commitFileDeletionRequested(QDir::toNativeSeparators(path));
}

void VolumeTab::onCommitFolderDelete() {
  if (!canCommitFiles()) return;
  const QString path = m_fileDialogs.selectDirectory(
      this,
      {I18n::tr("Select a folder whose deletion you want to commit"), dialogs::dialogBasePath(driveLetter()), {}, {dialogs::dialogBasePath(driveLetter())}});
  if (path.isEmpty() || !canCommitFiles()) return;
  emit commitFileDeletionRequested(QDir::toNativeSeparators(path));
}

void VolumeTab::onCommitRegistry() {
  if (!canCommitRegistry()) return;

  const auto target =
      RegistryPickerDialog::pick(m_viewModelFactory->registryPicker(RegistryPickerDialog::Mode::CommitValue), I18n::tr("Select registry key or value"), this);
  if (!target || !canCommitRegistry()) return;
  emit commitRegistryRequested(target->key, target->valueName);
}

void VolumeTab::onCommitRegistryDelete() {
  if (!canCommitRegistry()) return;
  const auto target = RegistryPickerDialog::pick(m_viewModelFactory->registryPicker(RegistryPickerDialog::Mode::DeleteValue),
                                                 I18n::tr("Select registry key or value to delete"), this);
  if (!target || !canCommitRegistry()) return;
  emit commitRegistryDeletionRequested(target->key, target->valueName);
}

VolumeTab::InfoPage VolumeTab::activeInfoPage() const {
  if (!m_infoTabs) return InfoPage::FileExclusions;
  const QWidget* current = m_infoTabs->currentWidget();
  if (current == pageHost(m_regs)) return InfoPage::RegistryExclusions;
  if (current == pageHost(m_fileStaging)) return InfoPage::FileStaging;
  return InfoPage::FileExclusions;
}

void VolumeTab::setActiveInfoPage(const InfoPage page) {
  if (!m_infoTabs) return;
  QWidget* target = nullptr;
  switch (page) {
    case InfoPage::FileExclusions:
      target = m_files;
      break;
    case InfoPage::RegistryExclusions:
      target = m_regs;
      break;
    case InfoPage::FileStaging:
      target = m_fileStaging;
      break;
  }
  if (target) m_infoTabs->setCurrentWidget(pageHost(target));
}
QWidget* VolumeTab::pageHost(QWidget* content) const { return m_pageHosts.contains(content) ? m_pageHosts.value(content) : content; }
void VolumeTab::markUnsupported() const {
  const auto reason = presentation::volumeSupportText(m_volume.support, m_volume.fileSystem);

  m_status->setUnsupported(reason);
}

void VolumeTab::markLimitedFileSystem() const {
  const auto reason = presentation::volumeSupportText(m_volume.support, m_volume.fileSystem);
  m_status->setNotice(reason);
}

void VolumeTab::renderCommitAvailability(const bool diagnostic) const {
  const auto maintenance = m_windowModel ? m_windowModel->overlayMaintenance() : nullptr;
  const auto view = m_viewModel->commitAvailability(maintenance.get(), diagnostic);
  const auto reason = diagnostic ? I18n::tr("UWF is currently unavailable") : QString{};
  const auto individualReason = diagnostic ? reason : view.commitHint;
  if (m_overlayBtn) {
    m_overlayBtn->setEnabled(view.overlay);
    m_overlayBtn->setToolTip(diagnostic ? reason : view.overlayHint);
  }
  for (auto* action : {m_commitFileAct, m_commitDirAct, m_commitFileDeleteAct, m_commitFolderDeleteAct})
    if (action) {
      action->setEnabled(view.files);
      action->setToolTip(individualReason);
    }
  for (auto* action : {m_commitRegAct, m_commitRegDeleteAct})
    if (action) {
      action->setEnabled(view.registry);
      action->setToolTip(individualReason);
    }
  if (m_individualCommitNote && m_individualCommitNoteAct) {
    const auto note = !diagnostic && (view.files || view.registry) ? QString{} : individualReason;
    m_individualCommitNote->setText(note);
    m_individualCommitNoteAct->setVisible(!note.isEmpty());
  }
  if (m_fullCommitAct) {
    const auto& state = view.fullCommit;
    m_fullCommitAct->setText(state.label);
    m_fullCommitAct->setEnabled(state.available);
    m_fullCommitAct->setToolTip(state.hint);
    if (m_removeFullCommitDraftAct) {
      m_removeFullCommitDraftAct->setVisible(state.pending);
      m_removeFullCommitDraftAct->setEnabled(view.removeFullCommitDraft);
      m_removeFullCommitDraftAct->setToolTip(I18n::tr("Remove full volume commit from pending changes."));
    }
    if (m_fullCommitNote) m_fullCommitNote->setText(state.note);
  }
  if (m_commitBtn) {
    m_commitBtn->setEnabled(view.menu);
    if (QMenu* menu = m_commitBtn->menu()) menu->setEnabled(view.menu);
    m_commitBtn->setToolTip(diagnostic ? reason : I18n::tr("Review available commit operations and their requirements."));
  }
}

bool VolumeTab::canCommitFiles() const { return m_viewModel->commitAvailability(nullptr, diagnostic().has_value()).files; }

bool VolumeTab::canCommitRegistry() const { return m_viewModel->commitAvailability(nullptr, diagnostic().has_value()).registry; }

void VolumeTab::renderRuntimeNotice() {
  const auto maintenance = m_windowModel ? m_windowModel->overlayMaintenance() : nullptr;
  const auto state = m_viewModel->runtimePresentation(maintenance.get());
  auto tone = Notice::Tone::Neutral;
  switch (state.tone) {
    case presentation::OverlayMaintenanceViewModel::NoticeTone::Neutral:
      break;
    case presentation::OverlayMaintenanceViewModel::NoticeTone::Accent:
      tone = Notice::Tone::Accent;
      break;
    case presentation::OverlayMaintenanceViewModel::NoticeTone::Warning:
      tone = Notice::Tone::Warning;
      break;
  }
  m_runtimeNotice->setText(state.text);
  m_runtimeNotice->setTone(tone);
  m_runtimeNotice->setHeading(state.heading);
  if (tone == Notice::Tone::Neutral) m_runtimeNotice->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/lock.svg")));
  m_runtimeNotice->setVisible(!state.text.isEmpty() && !diagnostic());
}

std::optional<QString> VolumeTab::diagnostic() const {
  if (m_windowModel) return m_windowModel->workspaceDiagnostic();
  const auto& state = m_viewModel->snapshotState();
  return state.available ? std::nullopt : std::optional{state.unavailableReason};
}

}  // namespace uwf::ui

#include "moc_VolumeTab.cpp"
