// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "VolumeTab.h"

#include <QAction>
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
#include <utility>

#include "ExclusionListWidget.h"
#include "VolumeStatusPanel.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/overlay/OverlayFilesDialog.h"
#include "ui/registry/RegistryPickerDialog.h"
#include "ui/staging/FileStagingWidget.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/ContentState.h"
#include "ui/widgets/TabView.h"
#include "ui/widgets/TextLabel.h"

namespace uwf::ui {

VolumeTab::VolumeTab(std::shared_ptr<presentation::VolumeConfigurationViewModel> model, dialogs::FileDialogProvider& fileDialogs,
                     app::FileStagingDraft* stagingDraft, std::shared_ptr<presentation::ViewModelFactory> viewModelFactory, QWidget* parent)
    : QWidget(parent),
      m_viewModelFactory(std::move(viewModelFactory)),
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
  m_volumeLabel->setToolTip(m_volumeLabel->text());
  m_volumeLabel->setVisible(!volume.label.empty());
  titleRow->addWidget(m_volumeLabel, 0, Qt::AlignBaseline);
  titleRow->addStretch(1);
  headingText->addLayout(titleRow);
  auto* capacity = new TextLabel(m_viewModel->capacityText(), TextStyle::Eyebrow, this);
  m_capacityLabel = capacity;
  capacity->setProperty("designTone", "faint");
  headingText->addWidget(capacity);
  heading->addLayout(headingText, 1);
  layout->addLayout(heading);

  m_status = new VolumeStatusPanel(m_viewModel->status(), this);
  m_status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(m_status);

  // 合并持久化操作以节省横向空间；注册表操作只在全局注册表宿主页展示。
  m_showRegistry = showRegistry;

  const auto& tm = ThemeManager::instance();

  // 查看覆盖层文件仅支持 NTFS；放在持久化菜单左侧。
  m_overlayBtn = new QPushButton(tm.icon(":/icons/disk.svg"), QString{});
  I18n::bind(m_overlayBtn, [target = m_overlayBtn] { target->setText(I18n::tr("View overlay")); });
  // 初始禁用，收到快照后根据文件系统及当前会话保护状态更新。
  m_overlayBtn->setEnabled(false);
  connect(m_overlayBtn, &QPushButton::clicked, this, [this]() {
    auto* dlg = new OverlayFilesDialog(m_viewModelFactory->overlayFiles(driveLetter()), m_fileDialogs, this);
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
  I18n::bind(m_commitBtn, [target = m_commitBtn] { target->setToolTip(I18n::tr("Commit overlay changes to disk / registry. This action cannot be undone.")); });
  auto* commitMenu = new CommandMenu(m_commitBtn);
  // QMenu 需显式启用工具提示。
  commitMenu->setToolTipsVisible(true);
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
    target->setToolTip(I18n::tr("Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk."));
  });
  if (m_showRegistry) {
    m_commitRegDeleteAct = commitMenu->addAction(tm.icon(":/icons/registry.svg"), QString{});
    I18n::bind(m_commitRegDeleteAct, [target = m_commitRegDeleteAct] { target->setText(I18n::tr("Delete and commit registry…")); });
    I18n::bind(m_commitRegDeleteAct, [target = m_commitRegDeleteAct] {
      target->setToolTip(I18n::tr(
          "Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole "
          "subtree (the key plus all its values and subkeys, recursively)."));
    });
  }
  for (auto* action : {m_commitFileDeleteAct, m_commitFolderDeleteAct, m_commitRegDeleteAct})
    if (action) action->setProperty("designRole", "destructive");
  m_commitBtn->setMenu(commitMenu);
  m_commitBtn->setFixedHeight(31);
  heading->addWidget(m_commitBtn, 0, Qt::AlignVCenter);
  updateCommitEnablement();

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
  m_files->setDriveLetter(QString::fromStdString(volume.driveLetter));
  const int fileIdx = m_infoTabs->addTab(wrapPage(m_files), tm.icon(":/icons/file.svg"), I18n::tr("File exclusions"));
  m_infoTabs->setTabToolTip(fileIdx, I18n::tr("Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path."));

  // 注册表排除是全局配置，仅在一个宿主页展示。
  if (m_showRegistry) {
    m_regs = new ExclusionListWidget(m_viewModel->registry(), m_fileDialogs, this);
    connect(m_regs, &ExclusionListWidget::registryPickerRequested, this, [this] {
      const auto picked =
          RegistryPickerDialog::pick(m_viewModelFactory->registryPicker(RegistryPickerDialog::Mode::Exclusion), I18n::tr("Add registry exclusion"), this);
      if (picked) m_regs->addPendingEntry(picked->key);
    });
    const int regIdx = m_infoTabs->addTab(wrapPage(m_regs), tm.icon(":/icons/registry.svg"), I18n::tr("Registry exclusions"));
    m_infoTabs->setTabToolTip(
        regIdx,
        I18n::tr("Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is "
                 "shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path."));
  }

  if (stagingDraft && core::supportsFileOverlayOperations(m_volume.support)) {
    m_fileStaging = new FileStagingWidget(
        m_viewModelFactory->fileStaging(QString::fromStdString(volume.driveLetter), *stagingDraft, &m_viewModel->fileConflicts()), m_fileDialogs, this);
    const int stagingIndex = m_infoTabs->addTab(wrapPage(m_fileStaging), tm.icon(":/icons/file_staging.svg"), I18n::tr("File staging"));
    m_infoTabs->setTabToolTip(
        stagingIndex,
        I18n::tr("Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the "
                 "list contains data, UWF Manager excludes HKLM\\SOFTWARE\\HsingYun\\UWF Manager so the list persists across protected sessions."));
    const auto updateIndicator = [this, stagingIndex] {
      const bool available = m_fileStaging->enhancedModeAvailable();
      static_cast<TabView*>(m_infoTabs)
          ->setTabIndicator(
              stagingIndex, available ? QString{} : QStringLiteral(":/icons/enhanced.svg"),
              available ? I18n::tr("File staging") : I18n::tr("Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files."));
    };
    connect(m_fileStaging, &FileStagingWidget::enhancedModeAvailabilityChanged, this, updateIndicator);
    updateIndicator();
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

  connect(m_viewModel.get(), &presentation::VolumeConfigurationViewModel::stateChanged, this, &VolumeTab::renderSnapshot);
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

  if (!supported()) {
    markUnsupported();
  } else if (!canManageExclusions()) {
    markLimitedFileSystem();
  }

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](app::Theme) { refreshThemedIcons(); });
  I18n::bind(this, [this] { retranslateUi(); });
  renderSnapshot();
}

void VolumeTab::retranslateUi() {
  renderVolumeMetadata();
  const auto translatePage = [this](QWidget* page, const QString& title, const QString& hint) {
    if (!page) return;
    const int index = m_infoTabs->indexOf(pageHost(page));
    m_infoTabs->setTabText(index, title);
    m_infoTabs->setTabToolTip(index, hint);
  };
  translatePage(m_files, I18n::tr("File exclusions"),
                I18n::tr("Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path."));
  translatePage(
      m_regs, I18n::tr("Registry exclusions"),
      I18n::tr("Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is "
               "shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path."));
  translatePage(
      m_fileStaging, I18n::tr("File staging"),
      I18n::tr("Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the "
               "list contains data, UWF Manager excludes HKLM\\SOFTWARE\\HsingYun\\UWF Manager so the list persists across protected sessions."));
  if (m_fileStaging && !m_fileStaging->enhancedModeAvailable()) {
    const int index = m_infoTabs->indexOf(pageHost(m_fileStaging));
    m_infoTabs->setTabToolTip(index, I18n::tr("Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files."));
  }
  for (auto* host : std::as_const(m_pageHosts)) {
    auto* error = qobject_cast<ContentState*>(host->widget(1));
    error->setText(I18n::tr("UWF is currently unavailable"),
                   m_diagnostic.isEmpty()
                       ? QString{}
                       : I18n::tr("UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.") + u'\n' +
                             m_diagnostic);
    error->setAction(I18n::tr("Read again"), ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")));
  }
  if (!supported())
    markUnsupported();
  else if (!canManageExclusions())
    markLimitedFileSystem();
}

void VolumeTab::setLayoutSize(const QSize& size) {
  const bool shortWindow = size.height() <= design::shortWindowHeight;
  layout()->setContentsMargins(0, shortWindow ? 10 : 14, 0, 0);
  layout()->setSpacing(shortWindow ? 10 : 14);
  m_status->setLayoutSize(size);
  static_cast<TabView*>(m_infoTabs)->setCompact(size.width() <= design::compactWindowWidth);
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
  m_headingLabel->setText(QString::fromStdString(m_volume.driveLetter));
  m_headingLabel->setToolTip(QString::fromStdString(m_volume.volumeName));
  m_volumeLabel->setText(QString::fromStdString(m_volume.label));
  m_volumeLabel->setToolTip(m_volumeLabel->text());
  m_volumeLabel->setVisible(!m_volume.label.empty());
  m_capacityLabel->setText(m_viewModel->capacityText());
}

void VolumeTab::onCommitFile() {
  if (!supported()) return;
  const QString path = m_fileDialogs.openFile(
      this, {I18n::tr("Select a file to commit to disk"), dialogs::dialogBasePath(driveLetter()), {}, {dialogs::dialogBasePath(driveLetter())}});
  if (path.isEmpty()) return;
  emit commitFileRequested(QDir::toNativeSeparators(path));
}

void VolumeTab::onCommitDir() {
  if (!supported()) return;
  const QString path = m_fileDialogs.selectDirectory(
      this, {I18n::tr("Select a folder to commit to disk"), dialogs::dialogBasePath(driveLetter()), {}, {dialogs::dialogBasePath(driveLetter())}});
  if (path.isEmpty()) return;
  emit commitFileRequested(QDir::toNativeSeparators(path));
}

void VolumeTab::onCommitFileDelete() {
  if (!supported()) return;
  // 提交删除由 UWF 执行，文件选择器须返回仍存在的目标。
  const QString path = m_fileDialogs.openFile(
      this,
      {I18n::tr("Select a file whose deletion you want to commit"), dialogs::dialogBasePath(driveLetter()), {}, {dialogs::dialogBasePath(driveLetter())}});
  if (path.isEmpty()) return;
  emit commitFileDeletionRequested(QDir::toNativeSeparators(path));
}

void VolumeTab::onCommitFolderDelete() {
  if (!supported()) return;
  const QString path = m_fileDialogs.selectDirectory(
      this,
      {I18n::tr("Select a folder whose deletion you want to commit"), dialogs::dialogBasePath(driveLetter()), {}, {dialogs::dialogBasePath(driveLetter())}});
  if (path.isEmpty()) return;
  emit commitFileDeletionRequested(QDir::toNativeSeparators(path));
}

void VolumeTab::onCommitRegistry() {
  // 空值名表示整键递归；默认值只能随整键提交。
  const auto target =
      RegistryPickerDialog::pick(m_viewModelFactory->registryPicker(RegistryPickerDialog::Mode::CommitValue), I18n::tr("Select registry key or value"), this);
  if (!target) return;
  emit commitRegistryRequested(target->key, target->valueName);
}

void VolumeTab::onCommitRegistryDelete() {
  const auto target = RegistryPickerDialog::pick(m_viewModelFactory->registryPicker(RegistryPickerDialog::Mode::DeleteValue),
                                                 I18n::tr("Select registry key or value to delete"), this);
  if (!target) return;
  emit commitRegistryDeletionRequested(target->key, target->valueName);
}

void VolumeTab::setEnhancedModeAvailable(const bool available) {
  if (m_fileStaging) m_fileStaging->setEnhancedModeAvailable(available);
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
void VolumeTab::setDiagnostic(const QString& message) {
  m_diagnostic = message;
  for (auto* host : std::as_const(m_pageHosts)) {
    auto* error = qobject_cast<ContentState*>(host->widget(1));
    error->setText(
        I18n::tr("UWF is currently unavailable"),
        I18n::tr("UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.") + u'\n' + message);
    host->setCurrentIndex(1);
  }
  if (m_listCommands) m_listCommands->setEnabled(false);
  m_overlayBtn->setEnabled(false);
  m_commitBtn->setEnabled(false);
}

void VolumeTab::markUnsupported() const {
  const auto reason = presentation::volumeSupportText(m_volume.support, m_volume.fileSystem);
  // 持久化按钮可用性统一由 updateCommitEnablement 管理，覆盖状态卡的禁用结果。
  m_status->setUnsupported(reason);
}

void VolumeTab::markLimitedFileSystem() const {
  // 受限文件系统仍可保护和绑定，但文件排除及文件提交不可用。
  const auto reason = presentation::volumeSupportText(m_volume.support, m_volume.fileSystem);
  m_status->setNotice(reason);
}

void VolumeTab::updateCommitEnablement() const {
  const auto view = m_viewModel->commitAvailability();
  if (m_overlayBtn) {
    m_overlayBtn->setEnabled(view.overlay);
    m_overlayBtn->setToolTip(view.overlayHint);
  }
  for (auto* action : {m_commitFileAct, m_commitDirAct, m_commitFileDeleteAct, m_commitFolderDeleteAct})
    if (action) action->setEnabled(view.files);
  for (auto* action : {m_commitRegAct, m_commitRegDeleteAct})
    if (action) action->setEnabled(view.registry);
  if (m_files) m_files->setCommitEnabled(view.files);
  if (m_commitBtn) {
    const bool any = view.files || view.registry;
    m_commitBtn->setEnabled(any);
    if (QMenu* menu = m_commitBtn->menu()) menu->setEnabled(any);
    m_commitBtn->setToolTip(view.commitHint);
  }
}

void VolumeTab::renderSnapshot() {
  const auto& state = m_viewModel->snapshotState();
  if (state.available) {
    for (auto* host : std::as_const(m_pageHosts)) host->setCurrentIndex(0);
    m_listCommands->setEnabled(true);
  } else
    setDiagnostic(state.unavailableReason);
  if (m_fileStaging) m_fileStaging->setReadOnly(!state.editable);
  if (!supported()) {
    m_status->setUnsupported(presentation::volumeSupportText(m_volume.support, m_volume.fileSystem));
    updateCommitEnablement();
    return;
  }
  if (m_volume.volumeName != m_viewModel->volume().volumeName) {
    m_volume.volumeName = m_viewModel->volume().volumeName;
    renderVolumeMetadata();
  }
  updateCommitEnablement();
}

}  // namespace uwf::ui

#include "moc_VolumeTab.cpp"
