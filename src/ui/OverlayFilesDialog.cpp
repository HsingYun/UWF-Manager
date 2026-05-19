#include "OverlayFilesDialog.h"

#include <combaseapi.h>
#include <shlobj.h>
#include <windows.h>

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHash>
#include <QLabel>
#include <QListWidgetItem>
#include <QMenu>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QSaveFile>
#include <QStringConverter>
#include <QTextStream>
#include <QVBoxLayout>
#include <algorithm>
#include <thread>
#include <utility>

#include "../util/DriveLetter.h"
#include "../uwf/api/UwfOverlay.h"
#include "../uwf/wmi/WmiClient.h"
#include "../uwf/wmi/WmiError.h"
#include "I18n.h"
#include "MessageDialog.h"
#include "ThemeManager.h"

namespace uwf::ui {

namespace {

QString formatSize(qulonglong b) {
  constexpr qulonglong KB = 1024;
  constexpr qulonglong MB = KB * 1024;
  constexpr qulonglong GB = MB * 1024;
  if (b >= GB) return QString::number(static_cast<double>(b) / static_cast<double>(GB), 'f', 2) + " GB";
  if (b >= MB) return QString::number(static_cast<double>(b) / static_cast<double>(MB), 'f', 1) + " MB";
  if (b >= KB) return QString::number(static_cast<double>(b) / static_cast<double>(KB), 'f', 1) + " KB";
  return QString::number(b) + " B";
}

// 把 WMI 返回的原始 fileName（如 "\Users\foo:$DATA"、
// "\$Secure:$SII:$INDEX_ALLOCATION" 或 "\foo:customstream:$DATA"）拼成绝对路径，
// 并把所有 NTFS 流后缀剥掉。
//
// NTFS 路径的完整形式是 "<file_path>:<stream_name>:<stream_type>"。stream_name
// 可能不以 '$' 开头（命名流），所以剥后缀不能查 ":$" 起点——那会把 ":namedstream"
// 当作路径一部分留下来，UI 上看着像路径尾巴多了个 ':' + 字符串。改成"从最后一个
// '\' 之后找第一个 ':'"——NTFS 文件名里禁止 ':'，所以最后一个路径段后的第一个
// ':' 必然是流分隔符；从那刀下去最干净。盘符里的 ':'（如 "C:"）在 '\' 之前，
// 不会被命中。
//
// 目录判定：只要原始字符串里出现 ":$INDEX_ALLOCATION"（NTFS 目录的 B+ 树
// 索引流）就视为目录；普通文件的数据流是 ":$DATA"。
//
// 系统元数据判定：检查 absolutePath 里**所有路径段**（不只是 basename）——
// \$Extend\$Deleted\<id> 这种深层元数据 basename 只是个 ID，但中间段
// $Extend / $Deleted 暴露了它本质上是 NTFS 系统数据。covers $Secure / $MFT /
// $130 / $Extend\$Deleted\... / $RECYCLE.BIN 等。这些条目 CommitFile 走不通，
// UI 保留显示用作诊断，但不给 commit。
void normalizeOverlayEntry(const QString& driveLetter, OverlayFileEntry& e) {
  QString s = e.rawName;

  if (s.contains(QStringLiteral(":$INDEX_ALLOCATION"), Qt::CaseInsensitive)) {
    e.isDirectory = true;
  }

  const qsizetype lastSlash = s.lastIndexOf(QChar('\\'));
  const qsizetype searchFrom = (lastSlash >= 0) ? lastSlash : 0;
  const qsizetype streamColon = s.indexOf(QChar(':'), searchFrom);
  if (streamColon > searchFrom) s = s.left(streamColon);

  if (s.startsWith(QChar('\\'))) s = driveLetter + s;
  e.absolutePath = QDir::toNativeSeparators(s);

  const QStringList segs = e.absolutePath.split(QChar('\\'), Qt::SkipEmptyParts);
  e.isSystemMetadata = false;
  // 跳过第 0 段（盘符 "C:"），只检查后面的目录段 + 文件名。
  for (qsizetype i = 1; i < segs.size(); ++i) {
    if (segs[i].startsWith(QChar('$'))) {
      e.isSystemMetadata = true;
      break;
    }
  }
}

}  // namespace

OverlayFilesDialog::OverlayFilesDialog(const QString& driveLetter, QWidget* parent)
    : QDialog(parent), m_driveLetter(QString::fromStdString(drive::normalize(driveLetter.toStdString()))) {
  setWindowTitle(I18n::tr("Overlay files - %1").arg(m_driveLetter));
  resize(760, 540);

  auto* layout = new QVBoxLayout(this);

  // 简介：UI 风格的精简版（不是把官方文档原文搬过来），只抓三件事——
  // 别拿来决策、慢且可能失败、列表本身就不精确。
  auto* desc = new QLabel(this);
  desc->setWordWrap(true);
  desc->setTextFormat(Qt::RichText);
  desc->setText("<p>" + I18n::tr("<b>Diagnostic snapshot only.</b> Don't use this list to decide what to commit.") + "</p><ul><li>" +
                I18n::tr("NTFS volumes only.") + "</li><li>" +
                I18n::tr("Can be slow or fail when the overlay is large — memory and time grow with overlay size.") + "</li><li>" +
                I18n::tr("The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded "
                         "paths, and files affected by unrelated operations may appear.") +
                "</li></ul>");
  layout->addWidget(desc);

  // 加载行：忙碌进度条 + "Loading…" 标签。完成后两者一起 hide。
  auto* loadingRow = new QHBoxLayout();
  m_progress = new QProgressBar(this);
  m_progress->setRange(0, 0);  // 无确定进度，仅显示忙碌动画
  m_progress->setMaximumHeight(8);
  m_progress->setTextVisible(false);
  loadingRow->addWidget(m_progress, 1);
  m_loadingLabel = new QLabel(I18n::tr("Loading…"), this);
  loadingRow->addWidget(m_loadingLabel);
  layout->addLayout(loadingRow);

  m_list = new QListWidget(this);
  m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_list->setIconSize({16, 16});
  // 复用 ExclusionListWidget 的 QSS class，跟随 dark/light 主题。
  m_list->setObjectName("exclusionList");
  m_list->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_list, &QListWidget::customContextMenuRequested, this, &OverlayFilesDialog::onContextMenu);
  layout->addWidget(m_list, 1);

  m_summary = new QLabel(this);
  m_summary->setTextInteractionFlags(Qt::TextSelectableByMouse);
  layout->addWidget(m_summary);

  // 自己拼按钮行而不是 QDialogButtonBox：Action/Reset 角色在 WindowsLayout
  // 下落点不稳定，直接 [Export] [stretch] [Close] 最直观。
  auto* btnRow = new QHBoxLayout();
  m_exportBtn = new QPushButton(I18n::tr("Export to file…"), this);
  // 加载中 / 加载失败 / 没有任何条目时统统禁用，等 onLoadFinished 成功后再放开。
  m_exportBtn->setEnabled(false);
  connect(m_exportBtn, &QPushButton::clicked, this, &OverlayFilesDialog::onExportClicked);
  btnRow->addWidget(m_exportBtn);
  btnRow->addStretch(1);
  auto* closeBtn = new QPushButton(I18n::tr("Close"), this);
  closeBtn->setDefault(true);
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
  btnRow->addWidget(closeBtn);
  layout->addLayout(btnRow);

  startLoading();
}

void OverlayFilesDialog::startLoading() {
  // worker 线程：自己 CoInitializeEx(MTA) + 起独立 WmiSession（initComOnce
  // 用 std::call_once 全局只跑一次，所以 worker 线程进不去那段，必须自己
  // 把 COM 拉起来）。结果用 QMetaObject::invokeMethod 投回 UI 线程，
  // QPointer 兜底——用户可能在加载完成前关掉 dialog。
  QPointer<OverlayFilesDialog> self(this);
  const std::string dl = m_driveLetter.toStdString();

  std::thread([self, dl]() {
    HRESULT comHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool ownsCom = SUCCEEDED(comHr);

    QString errorOut;
    int32_t errorHr = 0;
    QVector<OverlayFileEntry> entries;

    do {
      WmiSession session;
      std::string err;
      if (!session.connect(api::kWmiNamespace, &err)) {
        errorOut = QString::fromStdString(err);
        break;
      }
      UwfOverlay overlay(session);
      auto row = overlay.read(&err);
      if (!row) {
        errorOut = QString::fromStdString(err);
        break;
      }
      int32_t hr = 0;
      auto files = overlay.getOverlayFiles(*row, dl, &err, &hr);
      if (!err.empty()) {
        errorOut = QString::fromStdString(err);
        errorHr = hr;
        break;
      }
      entries.reserve(static_cast<int>(files.size()));
      for (const auto& f : files) {
        OverlayFileEntry e;
        e.rawName = QString::fromStdString(f.fileName);
        e.fileSize = static_cast<qulonglong>(f.fileSize);
        entries.append(std::move(e));
      }
    } while (false);

    if (ownsCom) CoUninitialize();

    // self 是 QPointer——dialog 已被销毁则 lambda 内 self 为 null，直接 no-op。
    QMetaObject::invokeMethod(
        qApp,
        [self, entries = std::move(entries), errorOut, errorHr]() mutable {
          if (!self) return;
          self->onLoadFinished(std::move(entries), errorOut, errorHr);
        },
        Qt::QueuedConnection);
  }).detach();
}

void OverlayFilesDialog::onLoadFinished(QVector<OverlayFileEntry> entries, const QString& error, int32_t hresult) {
  if (m_progress) m_progress->hide();
  if (m_loadingLabel) m_loadingLabel->hide();

  const auto& tm = ThemeManager::instance();
  if (!error.isEmpty()) {
    // GetOverlayFiles 是 UWF 文档承认不稳的方法——overlay 大、I/O 高负载或
    // NTFS 元数据复杂时 provider 会抛 RPC_E_SERVERFAULT，也可能 timeout
    // 或 OOM。按命名常量分支给用户一个可操作的中文提示，原始错误带在后面
    // 方便排查。
    const WmiError wbem(hresult);
    QString hint;
    if (hresult == static_cast<int32_t>(RPC_E_SERVERFAULT)) {
      hint = I18n::tr(
          "The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay "
          "is large or under I/O pressure. Wait a few seconds and click \"View overlay files\" again, often it succeeds on retry.");
    } else if (wbem == WmiErrorCode::OutOfMemory || wbem == WmiErrorCode::NotSupported) {
      hint = I18n::tr(
          "Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; "
          "try again with a smaller overlay or after closing memory-heavy applications.");
    }
    if (!hint.isEmpty()) {
      m_summary->setText(QStringLiteral("%1\n\n%2").arg(hint, I18n::tr("Raw error: %1").arg(error)));
    } else {
      m_summary->setText(I18n::tr("Failed: %1").arg(error));
    }
    m_summary->setWordWrap(true);
    m_summary->setStyleSheet(QString("color:%1").arg(tm.color(Sem::Danger).name()));
    return;
  }

  // 拼绝对路径 + 剥 NTFS 流后缀；列表 / 右键菜单 / 导出统一用 absolutePath。
  for (auto& e : entries) normalizeOverlayEntry(m_driveLetter, e);

  // 按 absolutePath（大小写无关）去重：同一个文件 / 目录可能因为多条 NTFS
  // 流（$DATA + 命名流；$INDEX_ALLOCATION + $INDEX_ROOT 等）在 WMI 输出里
  // 占好几行，剥掉流后缀就重了。合并规则：
  //   - size  求和——多条流加起来才是该路径在 overlay 里占的总字节
  //   - isDirectory / isSystemMetadata：只要任一条 raw 命中就保留（OR 合并）
  //   - rawName 保留第一条，仅作诊断 tooltip 用
  QHash<QString, int> indexByKey;
  indexByKey.reserve(entries.size());
  QVector<OverlayFileEntry> dedup;
  dedup.reserve(entries.size());
  for (auto& e : entries) {
    const QString key = e.absolutePath.toLower();
    if (auto it = indexByKey.find(key); it != indexByKey.end()) {
      auto& dst = dedup[*it];
      dst.fileSize += e.fileSize;
      if (e.isDirectory) dst.isDirectory = true;
      if (e.isSystemMetadata) dst.isSystemMetadata = true;
    } else {
      indexByKey[key] = static_cast<int>(dedup.size());
      dedup.append(std::move(e));
    }
  }
  entries = std::move(dedup);

  // 路径按字典序排，方便用户定位；同时 size 信息显示到右侧——用
  // QListWidget 的单字段 text，空格对齐够用了。
  std::ranges::sort(entries,
                    [](const OverlayFileEntry& a, const OverlayFileEntry& b) { return a.absolutePath.compare(b.absolutePath, Qt::CaseInsensitive) < 0; });

  m_summary->setText(I18n::tr("%1 file(s) in overlay").arg(entries.size()));

  for (const auto& e : entries) {
    auto* item = new QListWidgetItem();
    // 目录条目（来自 :$INDEX_ALLOCATION）显示 folder 图标，避免剥掉后缀
    // 后用户分不清这条到底是文件还是目录。
    item->setIcon(tm.icon(e.isDirectory ? ":/icons/folder.svg" : ":/icons/file.svg"));
    item->setData(Qt::UserRole, e.absolutePath);
    item->setData(Qt::UserRole + 1, e.isDirectory);
    item->setData(Qt::UserRole + 2, e.isSystemMetadata);
    item->setText(QString("%1    %2").arg(e.absolutePath, formatSize(e.fileSize)));
    item->setToolTip(e.absolutePath);
    m_list->addItem(item);
  }

  m_entries = std::move(entries);
  if (!m_entries.isEmpty()) m_exportBtn->setEnabled(true);
}

void OverlayFilesDialog::onContextMenu(const QPoint& pos) {
  auto* item = m_list->itemAt(pos);
  if (!item) return;
  const QString abs = item->data(Qt::UserRole).toString();
  if (abs.isEmpty()) return;
  const bool isDir = item->data(Qt::UserRole + 1).toBool();
  const bool isSystemMetadata = item->data(Qt::UserRole + 2).toBool();

  auto& tm = ThemeManager::instance();
  QMenu menu(this);

  auto* openAct = menu.addAction(tm.icon(":/icons/folder.svg"), I18n::tr("Open containing folder"));
  connect(openAct, &QAction::triggered, this, [abs]() { openContainingFolder(abs); });

  // 系统元数据（basename 以 '$' 起头：$Secure / $130 / $RECYCLE.BIN 之类）
  // 直接不出 commit 项——CommitFile 对它们必然返回 NOT_FOUND，菜单出来只是
  // 误导用户。条目本身依然显示在列表里供诊断。
  // 文件 / 目录走同一条 commitFileRequested 信号；MainWindow::commitFilePath
  // 内部会按 QFileInfo::isDir 自动分发单文件 / 递归目录两个分支。这里只让
  // 菜单文案区分，行为路径共用。
  if (!isSystemMetadata) {
    const QString label = isDir ? I18n::tr("Commit folder changes to disk…") : I18n::tr("Commit file changes to disk…");
    auto* commitAct = menu.addAction(tm.icon(":/icons/commit.svg"), label);
    connect(commitAct, &QAction::triggered, this, [this, abs]() { emit commitFileRequested(abs); });
  }

  menu.exec(m_list->viewport()->mapToGlobal(pos));
}

void OverlayFilesDialog::openContainingFolder(const QString& absolutePath) {
  // absolutePath 已经是规范化后的路径（去掉了 ":$DATA" / ":$INDEX_ALLOCATION"
  // 这类 NTFS 流后缀）。带流后缀的路径喂给 ILCreateFromPathW /
  // SHOpenFolderAndSelectItems 会得到奇怪的行为（有的版本会按文件关联
  // 直接打开文件本身），所以剥后缀是必须前置步骤。
  if (QFileInfo::exists(absolutePath)) {
    const std::wstring wide = absolutePath.toStdWString();
    if (PIDLIST_ABSOLUTE pidl = ILCreateFromPathW(wide.c_str())) {
      // cidl=0 + apidl=NULL 是 SHOpenFolderAndSelectItems 的"打开父目录
      // 并选中 pidl"快捷形式：文件就选中文件，目录就选中目录，都落在
      // 各自的父目录里。
      (void)SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
      ILFree(pidl);
      return;
    }
  }
  // 路径不存在或拿不到 PIDL：沿父目录回溯到第一个真实存在的文件夹直接
  // 打开（不带 select）。
  QString folder = QFileInfo(absolutePath).absolutePath();
  if (folder.isEmpty()) folder = absolutePath;
  while (!folder.isEmpty() && !QFileInfo::exists(folder)) {
    const QString up = QFileInfo(folder).absolutePath();
    if (up == folder) break;
    folder = up;
  }
  if (!folder.isEmpty() && QFileInfo::exists(folder)) {
    const std::wstring wf = QDir::toNativeSeparators(folder).toStdWString();
    // 走 explorer.exe + 文件夹参数，避免 ShellExecute "open" 在某些机器
    // 上落到默认文件管理器替身的问题。
    ShellExecuteW(nullptr, L"open", L"explorer.exe", wf.c_str(), nullptr, SW_SHOWNORMAL);
  }
}

void OverlayFilesDialog::onExportClicked() {
  if (m_entries.isEmpty()) return;

  // 默认文件名：overlay-files-C-20260426-143005.txt（盘符冒号去掉，保证文件
  // 名合法）。
  const QString letter = m_driveLetter.left(1);
  const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
  const QString suggested = QString("overlay-files-%1-%2.txt").arg(letter, stamp);

  const QString path =
      QFileDialog::getSaveFileName(this, I18n::tr("Export overlay file list"), QDir::home().filePath(suggested), I18n::tr("Text files (*.txt);;All files (*)"));
  if (path.isEmpty()) return;

  // 用 QSaveFile：先写到临时文件，commit 时原子改名替换目标，避免半截写入
  // 让用户拿到坏文件。
  QSaveFile out(path);
  if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    dialogs::warning(this, I18n::tr("Export failed"), I18n::tr("Could not open file for writing: %1").arg(out.errorString()));
    return;
  }

  qulonglong total = 0;
  for (const auto& e : m_entries) total += e.fileSize;

  QTextStream ts(&out);
  ts.setEncoding(QStringConverter::Utf8);
  // 头部信息以 '#' 开头，方便用 awk/grep 过滤；正文是 TSV，可以直接贴进
  // Excel/sheets 处理。
  ts << "# UWF overlay files for " << m_driveLetter << '\n';
  ts << "# Generated " << QDateTime::currentDateTime().toString(Qt::ISODate) << '\n';
  ts << "# " << m_entries.size() << " entries, total " << formatSize(total) << '\n';
  ts << "# Note: list is approximate; files smaller than the disk cluster size (typically 4 KB) may be missing.\n";
  ts << "#\n";
  ts << "# type\tsize_bytes\tsize_human\tpath\n";
  for (const auto& e : m_entries) {
    ts << (e.isDirectory ? "dir" : "file") << '\t' << e.fileSize << '\t' << formatSize(e.fileSize) << '\t' << e.absolutePath << '\n';
  }

  if (!out.commit()) {
    dialogs::warning(this, I18n::tr("Export failed"), I18n::tr("Could not write file: %1").arg(out.errorString()));
    return;
  }

  dialogs::information(this, I18n::tr("Export finished"), I18n::tr("Saved %1 entries to:\n%2").arg(m_entries.size()).arg(QDir::toNativeSeparators(path)));
}

}  // namespace uwf::ui
