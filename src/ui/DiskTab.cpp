#include "DiskTab.h"

#include <QAction>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSizePolicy>
#include <QTabWidget>
#include <format>

#include "../util/ByteFormat.h"
#include "../util/DriveLetter.h"
#include "Dialogs.h"
#include "ExclusionListWidget.h"
#include "I18n.h"
#include "OverlayFilesDialog.h"
#include "RegistryPickerDialog.h"
#include "StatusPanel.h"
#include "ThemeManager.h"

namespace uwf::ui {

std::string diskSupportText(core::DiskSupport s, const std::string& fileSystem) {
  switch (s) {
    case core::DiskSupport::Supported:
      return {};
    case core::DiskSupport::NotFixedLocalDisk:
      return I18n::tr("Unsupported drive type (only fixed local disks are supported).").toStdString();
    case core::DiskSupport::ExceedsMaxSize:
      return I18n::tr("Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.").toStdString();
    case core::DiskSupport::FileSystemLimited:
      return I18n::tr("%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.")
          .arg(QString::fromStdString(fileSystem))
          .toStdString();
    case core::DiskSupport::QueryFailed:
      return I18n::tr("Failed to read volume information.").toStdString();
  }
  return {};
}

namespace {

const core::VolumeRecord* findVolume(const core::SessionSnapshot& s, const QString& driveLetter) {
  for (const auto& v : s.volumes) {
    const QString dl = QString::fromStdString(v.driveLetter).toUpper();
    if (dl == driveLetter.toUpper()) return &v;
  }
  return nullptr;
}

QStringList toQList(const std::vector<std::string>& v) {
  QStringList out;
  out.reserve(static_cast<int>(v.size()));
  for (const auto& s : v) out << QString::fromStdString(s);
  return out;
}

// Win32_Volume.DeviceID 形如 "\\?\Volume{GUID}\"，heading 上只想看 GUID，
// 不想看那串前缀；解析失败（格式不匹配）就原样返回，保持可读性。
std::string shortVolumeId(const std::string& vn) {
  const auto open = vn.find('{');
  const auto close = vn.find('}', open);
  if (open != std::string::npos && close != std::string::npos && close > open + 1) {
    return vn.substr(open + 1, close - open - 1);
  }
  return vn;
}

// 把磁盘信息拼成单行 heading：盘符加粗，后面跟能拿到的字段。
// 拿不到的字段直接略过，不展示 "(未知)" 之类占位。
QString renderDiskHeading(const core::DiskInfo& d) {
  std::string parts;
  auto add = [&](const std::string& s) {
    if (s.empty()) return;
    if (!parts.empty()) parts += " · ";
    parts += s;
  };

  if (!d.label.empty()) add(d.label);
  if (!d.fileSystem.empty()) add(d.fileSystem);
  if (d.totalBytes > 0) {
    add(I18n::tr("%1 free / %2").arg(QString::fromStdString(formatBytes(d.freeBytes)), QString::fromStdString(formatBytes(d.totalBytes))).toStdString());
  }
  if (!d.volumeName.empty()) {
    add(std::format("<span style='font-family:Consolas,monospace'>{}</span>", shortVolumeId(d.volumeName)));
  }

  const std::string mutedHex = ThemeManager::instance().color(Sem::FgMuted).name().toStdString();
  const std::string body =
      std::format("<b style='font-size:16pt'>{}</b>{}<span style='color:{}'>{}</span>", d.driveLetter, parts.empty() ? "" : " &nbsp; ", mutedHex, parts);
  return QString::fromStdString(body);
}

}  // namespace

DiskTab::DiskTab(const core::DiskInfo& disk, QWidget* parent) : QWidget(parent), m_disk(disk) {
  auto* layout = new QVBoxLayout(this);
  // 顶部留白走 QTabWidget#innerTabs::pane 的 padding 即可，这里再叠 16px
  // 会造成盘符和 TAB 之间出现一大块空白。底部 4px 是为了让文件列表的下边界
  // 与右侧 hoverHint 的下边界水平对齐：右侧 hoverHint 距 globalWrap 底 12px；
  // 左侧累计 = mainTabs 面板下边距 4 + 此处下边距 + innerTabs 面板下边距 4，
  // 取 4 刚好凑成 12。
  layout->setContentsMargins(16, 4, 16, 4);
  layout->setSpacing(12);

  m_headingLabel = new QLabel(renderDiskHeading(disk), this);
  m_headingLabel->setObjectName("diskHeading");
  m_headingLabel->setTextFormat(Qt::RichText);
  m_headingLabel->setWordWrap(true);
  m_headingLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_headingLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(m_headingLabel);

  // 状态卡（保护 / 绑定） —— 固定高度。
  m_status = new StatusPanel(this);
  m_status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(m_status);

  // 一个"持久化"菜单按钮放在 StatusPanel 右侧：点击后从弹出菜单里选择要持久
  // 化的子类型（文件 / 文件夹 / 注册表），避免三个并排按钮占横向空间。
  // 这里特意不用"提交"二字——上面工具栏的"预览并提交变更"是另一回事
  // （把 UI 上暂存的配置写入 WMI），两者放在一起容易让人搞错。
  // 注册表子项仅在系统盘 TAB 上出现（和注册表排除列表展示口径一致）。
  const QString thisDl = QString::fromStdString(disk.driveLetter).toUpper();
  m_showRegistry = (thisDl == QString::fromStdString(drive::systemLetter()));

  const auto& tm = ThemeManager::instance();

  // "View overlay files" 按钮——只在 NTFS 卷上启用（GetOverlayFiles 文档
  // 限制）。点击弹一个非模态对话框，里面异步加载文件列表。
  // 把它放在 commit 菜单按钮**之前** addTrailingAction，让它显示在 commit
  // 按钮的左边。
  m_overlayBtn = new QPushButton(tm.icon(":/icons/log.svg"), I18n::tr("View overlay files"));
  // 初始一律禁用；等第一次 applySnapshot 带着快照后由 updateCommitEnablement
  // 按 NTFS / 全局开关 / 当前卷保护状态共同决定。
  m_overlayBtn->setEnabled(false);
  connect(m_overlayBtn, &QPushButton::clicked, this, [this]() {
    auto* dlg = new OverlayFilesDialog(driveLetter(), this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    // 转发给 DiskTab 自己的 commitFileRequested 信号 → 最终到
    // MainWindow::commitFilePath，走和右键文件列表一样的提交流程。
    connect(dlg, &OverlayFilesDialog::commitFileRequested, this, &DiskTab::commitFileRequested);
    dlg->show();
  });
  m_status->addTrailingAction(m_overlayBtn);

  m_commitBtn = new QPushButton(tm.icon(":/icons/commit.svg"), I18n::tr("Commit"));
  m_commitBtn->setToolTip(I18n::tr("Commit overlay changes to disk / registry. This action cannot be undone."));
  auto* commitMenu = new QMenu(m_commitBtn);
  // QMenu 默认不显示 action 的 toolTip，须显式打开，下面每项设的 setToolTip 才会出现。
  commitMenu->setToolTipsVisible(true);
  // 先列"提交类"（把覆盖层改动写回磁盘 / 注册表），分隔线之后再列"删除类"。
  m_commitFileAct = commitMenu->addAction(tm.icon(":/icons/file.svg"), I18n::tr("Commit file changes…"));
  m_commitFileAct->setToolTip(I18n::tr("Pick a file and commit its overlay changes to disk."));
  m_commitDirAct = commitMenu->addAction(tm.icon(":/icons/folder.svg"), I18n::tr("Commit folder changes…"));
  m_commitDirAct->setToolTip(I18n::tr("Pick a folder and commit overlay changes for every file inside it to disk."));
  if (m_showRegistry) {
    m_commitRegAct = commitMenu->addAction(tm.icon(":/icons/registry.svg"), I18n::tr("Commit registry changes…"));
    m_commitRegAct->setToolTip(I18n::tr(
        "Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the "
        "key and all its subkeys (recursively)."));
  }
  commitMenu->addSeparator();
  m_commitFileDeleteAct = commitMenu->addAction(tm.icon(":/icons/file.svg"), I18n::tr("Delete and commit file…"));
  m_commitFileDeleteAct->setToolTip(I18n::tr("Pick a file to delete, and commit the deletion to disk."));
  m_commitFolderDeleteAct = commitMenu->addAction(tm.icon(":/icons/folder.svg"), I18n::tr("Delete and commit folder…"));
  m_commitFolderDeleteAct->setToolTip(I18n::tr("Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk."));
  if (m_showRegistry) {
    m_commitRegDeleteAct = commitMenu->addAction(tm.icon(":/icons/registry.svg"), I18n::tr("Delete and commit registry…"));
    m_commitRegDeleteAct->setToolTip(I18n::tr(
        "Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole "
        "subtree (the key plus all its values and subkeys, recursively)."));
  }
  m_commitBtn->setMenu(commitMenu);
  m_status->addTrailingAction(m_commitBtn);
  // 初始一律禁用；等第一次 applySnapshot 带着快照再按实际状态点亮。
  // 此刻 m_editable 仍是默认的 false，updateCommitEnablement 会全禁。
  updateCommitEnablement(false, false);

  // 排除列表 TAB —— 占满剩余空间，但不会被压到看不到内容。
  m_infoTabs = new QTabWidget(this);
  m_infoTabs->setObjectName("innerTabs");
  m_infoTabs->setDocumentMode(true);
  m_infoTabs->setMinimumHeight(120);
  m_files = new ExclusionListWidget(ExclusionListWidget::Kind::File, this);
  m_files->setDriveLetter(QString::fromStdString(disk.driveLetter));
  const int fileIdx = m_infoTabs->addTab(m_files, tm.icon(":/icons/file.svg"), I18n::tr("File exclusions"));
  m_infoTabs->setTabToolTip(fileIdx, I18n::tr("Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path."));

  // 注册表排除在 UWF 里是全局的，和卷无关；只在系统盘这个 TAB 上展示，
  // 避免其它盘看到一份完全相同的"注册表排除"列表而误解。
  if (m_showRegistry) {
    m_regs = new ExclusionListWidget(ExclusionListWidget::Kind::Registry, this);
    const int regIdx = m_infoTabs->addTab(m_regs, tm.icon(":/icons/registry.svg"), I18n::tr("Registry exclusions"));
    m_infoTabs->setTabToolTip(
        regIdx, I18n::tr("Global registry exclusion list (shared across volumes; shown only on the system drive). Double-click an entry to copy its path."));
  }

  layout->addWidget(m_infoTabs, 1);

  connect(m_files, &ExclusionListWidget::pendingChanged, this, &DiskTab::pendingChanged);
  connect(m_status, &StatusPanel::pendingChanged, this, &DiskTab::pendingChanged);
  const auto onCopied = [this](const QString& hint) { emit statusHint(hint, 3000); };
  connect(m_files, &ExclusionListWidget::copiedToClipboard, this, onCopied);
  // 文件列表右键菜单的"提交改动"项 → 直接转发到 DiskTab::commitFileRequested
  // 信号，最终由 MainWindow.commitFilePath 处理（含文件/目录递归判断）。
  connect(m_files, &ExclusionListWidget::commitFileRequested, this, &DiskTab::commitFileRequested);
  if (m_regs) {
    connect(m_regs, &ExclusionListWidget::pendingChanged, this, &DiskTab::pendingChanged);
    connect(m_regs, &ExclusionListWidget::copiedToClipboard, this, onCopied);
  }

  connect(m_commitFileAct, &QAction::triggered, this, &DiskTab::onCommitFile);
  connect(m_commitDirAct, &QAction::triggered, this, &DiskTab::onCommitDir);
  connect(m_commitFileDeleteAct, &QAction::triggered, this, &DiskTab::onCommitFileDelete);
  connect(m_commitFolderDeleteAct, &QAction::triggered, this, &DiskTab::onCommitFolderDelete);
  if (m_commitRegAct) {
    connect(m_commitRegAct, &QAction::triggered, this, &DiskTab::onCommitRegistry);
  }
  if (m_commitRegDeleteAct) {
    connect(m_commitRegDeleteAct, &QAction::triggered, this, &DiskTab::onCommitRegistryDelete);
  }

  if (m_disk.support == core::DiskSupport::Supported) {
    // 完全支持，什么都不做
  } else if (supported()) {
    // FileSystemLimited：可保护，但不能管文件排除 / commit 文件
    markLimitedFileSystem();
  } else {
    markUnsupported();
  }

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) { refreshThemedIcons(); });
}

void DiskTab::refreshThemedIcons() {
  auto& tm = ThemeManager::instance();
  if (m_overlayBtn) m_overlayBtn->setIcon(tm.icon(":/icons/log.svg"));
  if (m_commitBtn) m_commitBtn->setIcon(tm.icon(":/icons/commit.svg"));
  if (m_commitFileAct) m_commitFileAct->setIcon(tm.icon(":/icons/file.svg"));
  if (m_commitDirAct) m_commitDirAct->setIcon(tm.icon(":/icons/folder.svg"));
  if (m_commitFileDeleteAct) m_commitFileDeleteAct->setIcon(tm.icon(":/icons/file.svg"));
  if (m_commitFolderDeleteAct) m_commitFolderDeleteAct->setIcon(tm.icon(":/icons/folder.svg"));
  if (m_commitRegAct) m_commitRegAct->setIcon(tm.icon(":/icons/registry.svg"));
  if (m_commitRegDeleteAct) m_commitRegDeleteAct->setIcon(tm.icon(":/icons/registry.svg"));
  if (m_infoTabs) {
    if (m_files) {
      const int idx = m_infoTabs->indexOf(m_files);
      if (idx >= 0) m_infoTabs->setTabIcon(idx, tm.icon(":/icons/file.svg"));
    }
    if (m_regs) {
      const int idx = m_infoTabs->indexOf(m_regs);
      if (idx >= 0) m_infoTabs->setTabIcon(idx, tm.icon(":/icons/registry.svg"));
    }
  }
  // heading 用了 inline 颜色，重新生成一遍 RichText。
  if (m_headingLabel) {
    m_headingLabel->setText(renderDiskHeading(m_disk));
  }
}

void DiskTab::onCommitFile() {
  if (!supported()) return;
  const QString path = QFileDialog::getOpenFileName(this, I18n::tr("Select a file to commit to disk"), dialogs::dialogBasePath(driveLetter()));
  if (path.isEmpty()) return;
  emit commitFileRequested(QDir::toNativeSeparators(path));
}

void DiskTab::onCommitDir() {
  if (!supported()) return;
  const QString path = QFileDialog::getExistingDirectory(this, I18n::tr("Select a folder to commit to disk"), dialogs::dialogBasePath(driveLetter()));
  if (path.isEmpty()) return;
  emit commitFileRequested(QDir::toNativeSeparators(path));
}

void DiskTab::onCommitFileDelete() {
  if (!supported()) return;
  // CommitFileDeletion 删除的是一个**当前仍存在**的文件，所以直接用文件选择框
  // 挑现存文件即可。
  const QString path = QFileDialog::getOpenFileName(this, I18n::tr("Select a file whose deletion you want to commit"), dialogs::dialogBasePath(driveLetter()));
  if (path.isEmpty()) return;
  emit commitFileDeletionRequested(QDir::toNativeSeparators(path));
}

void DiskTab::onCommitFolderDelete() {
  if (!supported()) return;
  // 文件夹删除同样作用于"当前存在"的目录——用文件夹选择器挑现存目录；递归删除
  // （子文件 / 子目录 / 目录本身）交给 MainWindow::commitFileDeletionPath。
  const QString path = QFileDialog::getExistingDirectory(this, I18n::tr("Select a folder whose deletion you want to commit"), dialogs::dialogBasePath(driveLetter()));
  if (path.isEmpty()) return;
  emit commitFileDeletionRequested(QDir::toNativeSeparators(path));
}

void DiskTab::onCommitRegistry() {
  // valueName 空（值表无选中）→ MainWindow 递归提交整棵子树；非空 → 命名值
  // 单独 commit。(Default) 已在 picker 里禁选，默认值要走整键递归一并 commit。
  const auto target = RegistryPickerDialog::pick(RegistryPickerDialog::Mode::CommitValue, I18n::tr("Commit registry changes"), this);
  if (!target) return;
  emit commitRegistryRequested(target->key, target->valueName);
}

void DiskTab::onCommitRegistryDelete() {
  const auto target = RegistryPickerDialog::pick(RegistryPickerDialog::Mode::DeleteValue, I18n::tr("Commit registry deletion"), this);
  if (!target) return;
  emit commitRegistryDeletionRequested(target->key, target->valueName);
}

int DiskTab::activeInfoTabIndex() const { return m_infoTabs ? m_infoTabs->currentIndex() : 0; }

void DiskTab::setActiveInfoTabIndex(int index) {
  if (m_infoTabs && index >= 0 && index < m_infoTabs->count()) m_infoTabs->setCurrentIndex(index);
}

void DiskTab::markUnsupported() const {
  const std::string reason = diskSupportText(m_disk.support, m_disk.fileSystem);
  // setUnsupported 会把 card 内的所有子控件（包括"持久化"按钮）置灰；
  // 持久化按钮的可用性由 updateCommitEnablement 统一接管。
  m_status->setUnsupported(QString::fromStdString(reason));
  m_files->setReadOnly(true);
  if (m_regs) m_regs->setReadOnly(true);
}

void DiskTab::markLimitedFileSystem() const {
  // exFAT / ReFS：UWF 仍可保护此卷，但不能管理文件排除 / 提交单文件。
  // 这里保持 m_status 控件可用（保护开关、绑定方式还能用），只把文件
  // 排除列表设 readOnly + 显示提示横幅；commit 菜单的文件类项禁用由
  // updateCommitEnablement 兜底。
  const std::string reason = diskSupportText(m_disk.support, m_disk.fileSystem);
  m_status->setNotice(QString::fromStdString(reason));
  m_files->setReadOnly(true);
  // 注册表排除是全局的，不依赖卷的 FS，所以保持可用。
}

void DiskTab::updateCommitEnablement(const bool globalFilterOn, const bool thisVolumeProtected) const {
  // m_editable=false（UWF 不可用 / 未提权）时一切持久化都不可能成功，直接全禁。
  // 文件类提交还要求 FS 支持文件排除——exFAT / ReFS 等 limited 卷上不行。
  const bool fileOK = m_editable && globalFilterOn && thisVolumeProtected && canManageExclusions();
  const bool regOK = m_editable && globalFilterOn;  // 注册表是全局的，不依赖卷 FS

  // "查看覆盖层文件" 受约束：可写 + NTFS（GetOverlayFiles 文档限制）+
  // 全局筛选器开 + 本卷当前会话受保护。任何一项不满足都没有 overlay
  // 可看。
  if (m_overlayBtn) {
    const bool isNtfs = QString::fromStdString(m_disk.fileSystem).compare("NTFS", Qt::CaseInsensitive) == 0;
    const bool overlayOK = m_editable && isNtfs && globalFilterOn && thisVolumeProtected;
    m_overlayBtn->setEnabled(overlayOK);
    if (!isNtfs) {
      m_overlayBtn->setToolTip(I18n::tr("Overlay file listing is only supported on NTFS volumes."));
    } else if (!globalFilterOn) {
      m_overlayBtn->setToolTip(I18n::tr("The UWF filter is currently disabled, so no volume has an overlay to inspect."));
    } else if (!thisVolumeProtected) {
      m_overlayBtn->setToolTip(I18n::tr("This volume is not currently protected by UWF, so it has no overlay to inspect."));
    } else {
      m_overlayBtn->setToolTip(I18n::tr("Open a diagnostic view of files currently cached in the overlay for this volume."));
    }
  }

  if (m_commitFileAct) m_commitFileAct->setEnabled(fileOK);
  if (m_commitDirAct) m_commitDirAct->setEnabled(fileOK);
  if (m_commitFileDeleteAct) m_commitFileDeleteAct->setEnabled(fileOK);
  if (m_commitFolderDeleteAct) m_commitFolderDeleteAct->setEnabled(fileOK);
  if (m_commitRegAct) m_commitRegAct->setEnabled(regOK);
  if (m_commitRegDeleteAct) m_commitRegDeleteAct->setEnabled(regOK);
  // 文件列表右键"提交改动到磁盘"和上面的 commit 菜单同一套门槛：本卷当前会话
  // 有活动覆盖层（fileOK）时才放出该项。m_files 在构造期晚于本函数首次调用，故判空。
  if (m_files) m_files->setCommitEnabled(fileOK);

  if (m_commitBtn) {
    const bool any = fileOK || (regOK && m_commitRegAct != nullptr);
    m_commitBtn->setEnabled(any);
    // tooltip 动态化：让用户把鼠标悬上去就知道为什么不能点。
    if (!globalFilterOn) {
      m_commitBtn->setToolTip(I18n::tr("The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit."));
    } else if (!canManageExclusions() && m_commitRegAct == nullptr) {
      m_commitBtn->setToolTip(I18n::tr("Per-file commit is not supported on the %1 file system.").arg(QString::fromStdString(m_disk.fileSystem)));
    } else if (!canManageExclusions()) {
      m_commitBtn->setToolTip(
          I18n::tr("Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).")
              .arg(QString::fromStdString(m_disk.fileSystem)));
    } else if (!thisVolumeProtected && m_commitRegAct == nullptr) {
      m_commitBtn->setToolTip(I18n::tr("This volume is not currently protected by UWF; there are no file changes to commit."));
    } else if (!thisVolumeProtected) {
      m_commitBtn->setToolTip(I18n::tr("This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global)."));
    } else {
      m_commitBtn->setToolTip(I18n::tr("Commit overlay changes to disk / registry. This action cannot be undone."));
    }
  }
}

void DiskTab::applySnapshot(const core::UwfSnapshot& snap) {
  // 可写 = UWF 可读且进程已提权。缓存供 updateCommitEnablement 复用（它也会
  // 在没有快照的构造期被调到），并决定排除列表 / 状态卡是否可交互。
  m_editable = snap.uwfAvailable && snap.elevated;
  const bool filterOn = snap.current.filter.enabled;
  if (!supported()) {
    markUnsupported();
    updateCommitEnablement(filterOn, /*thisVolumeProtected=*/false);
    return;
  }

  const QString dl = driveLetter();
  const auto* cv = findVolume(snap.current, dl);
  const auto* nv = findVolume(snap.next, dl);
  m_status->setData(cv, nv);
  // setData 按 hasVolume 点亮保护开关 / 绑定方式；这里再叠加可写性统一收口。
  m_status->setControlsEnabled(m_editable);

  const std::string volKey = nv ? nv->volumeName : (cv ? cv->volumeName : std::string{});
  QStringList curFiles, nxtFiles;
  if (!volKey.empty()) {
    const auto it = snap.current.fileExclusions.find(volKey);
    if (it != snap.current.fileExclusions.end()) curFiles = toQList(it->second);
    const auto it2 = snap.next.fileExclusions.find(volKey);
    if (it2 != snap.next.fileExclusions.end()) nxtFiles = toQList(it2->second);
  }
  m_files->setBaseline(curFiles, nxtFiles);

  if (m_regs) {
    m_regs->setBaseline(toQList(snap.current.registryExclusions), toQList(snap.next.registryExclusions));
    m_regs->setPersistBaseline(snap.current.persistDomainSecretKey, snap.next.persistDomainSecretKey, snap.current.persistTSCAL, snap.next.persistTSCAL);
  }

  // 不可写（UWF 不可用 / 未提权）时排除列表没有可落地的目标——整列设只读，
  // 禁掉添加 / 删除按钮；恢复可写后再放开（FS 受限卷的文件列表始终保持只读）。
  m_files->setReadOnly(!m_editable || !canManageExclusions());
  if (m_regs) m_regs->setReadOnly(!m_editable);

  // 快照里可能补充 volumeName 信息，刷一下标题。
  if (nv && !nv->volumeName.empty() && m_disk.volumeName.empty()) {
    m_disk.volumeName = nv->volumeName;
    m_headingLabel->setText(renderDiskHeading(m_disk));
  }

  updateCommitEnablement(filterOn, cv && cv->isProtected);
}

QStringList DiskTab::pendingFileAdded() const { return m_files->pendingAdded(); }
QStringList DiskTab::pendingFileRemoved() const { return m_files->pendingRemoved(); }
QStringList DiskTab::pendingRegAdded() const { return m_regs ? m_regs->pendingAdded() : QStringList(); }
QStringList DiskTab::pendingRegRemoved() const { return m_regs ? m_regs->pendingRemoved() : QStringList(); }
std::optional<bool> DiskTab::pendingPersistDomainSecretKey() const { return m_regs ? m_regs->pendingPersistDomainSecretKey() : std::nullopt; }
std::optional<bool> DiskTab::pendingPersistTSCAL() const { return m_regs ? m_regs->pendingPersistTSCAL() : std::nullopt; }

std::optional<bool> DiskTab::pendingVolumeProtected() const { return supported() ? m_status->pendingVolumeProtected() : std::nullopt; }

std::optional<bool> DiskTab::pendingBindByVolumeName() const { return supported() ? m_status->pendingBindByVolumeName() : std::nullopt; }

bool DiskTab::importProtect(bool v) {
  if (!supported()) return false;
  return m_status->importProtect(v);
}

ExclusionListWidget::ImportOutcome DiskTab::importAddFileExclusion(const QString& path) {
  // 卷未受 UWF 支持 / 文件系统不支持文件排除（exFAT 等）：当作"不在本卷"处理，
  // 让导入报告里给出明确"被拒"理由而不是无声成功。
  if (!supported() || !canManageExclusions()) return ExclusionListWidget::ImportOutcome::RejectedNotOnVolume;
  return m_files->importAdd(path);
}

ExclusionListWidget::ImportOutcome DiskTab::importRemoveFileExclusion(const QString& path) {
  if (!supported() || !canManageExclusions()) return ExclusionListWidget::ImportOutcome::RejectedNotOnVolume;
  return m_files->importRemove(path);
}

ExclusionListWidget::ImportOutcome DiskTab::importAddRegistryExclusion(const QString& key) {
  if (!m_regs) return ExclusionListWidget::ImportOutcome::RejectedNotOnVolume;
  return m_regs->importAdd(key);
}

ExclusionListWidget::ImportOutcome DiskTab::importRemoveRegistryExclusion(const QString& key) {
  if (!m_regs) return ExclusionListWidget::ImportOutcome::RejectedNotOnVolume;
  return m_regs->importRemove(key);
}

}  // namespace uwf::ui
