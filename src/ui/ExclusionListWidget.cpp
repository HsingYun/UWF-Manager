#include "ExclusionListWidget.h"

#include <QAction>
#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QColor>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPolygon>
#include <QPushButton>
#include <QRegion>
#include <QScreen>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <cmath>

#include "../util/DriveLetter.h"
#include "../util/RegistryKey.h"
#include "I18n.h"
#include "MessageDialog.h"
#include "ThemeManager.h"

#include <shlobj.h>
#include <windows.h>

namespace uwf::ui {

namespace {
QString lowerKey(const QString& s) { return s.toLower(); }

// QSet 没有自带的大小写不敏感 contains/remove；用户填进去的 m_added /
// m_removed 通常只有几十条，O(n) 扫描完全够。所有用到这两个集合的地方
// 必须走这两个 helper，否则路径大小写差异会导致状态不一致（"先删 C:\Foo
// 再加 c:\foo" 之类）。
bool setContainsCI(const QSet<QString>& set, const QString& key) {
  for (const auto& s : set) {
    if (QString::compare(s, key, Qt::CaseInsensitive) == 0) return true;
  }
  return false;
}

void setRemoveCI(QSet<QString>& set, const QString& key) {
  for (auto it = set.begin(); it != set.end();) {
    if (QString::compare(*it, key, Qt::CaseInsensitive) == 0)
      it = set.erase(it);
    else
      ++it;
  }
}

// UWF 官方文档明确说不能排除的一组路径。命中其中任何一条就返回
// 面向用户的中文原因；不命中返回空串（允许添加）。
QString forbidExclusionReason(const QString& rawPath, const QString& volumeDl) {
  const QString upper = QDir::toNativeSeparators(rawPath).trimmed().toUpper();
  if (upper.isEmpty()) return {};

  // 卷根本身：match "C:" 或 "C:\"。
  if (upper == volumeDl || upper == volumeDl + "\\") {
    return I18n::tr("Cannot add an entire volume (%1) to the exclusion list.").arg(volumeDl);
  }

  // 剥掉盘符前缀，得到以 "\" 开头的卷内相对路径。
  QString rel = upper;
  if (rel.startsWith(volumeDl)) rel = rel.mid(volumeDl.size());
  if (!rel.startsWith("\\")) rel = "\\" + rel;
  while (rel.size() > 1 && rel.endsWith("\\")) rel.chop(1);

  // 分页/交换/休眠文件——放在卷根上的系统文件；任何卷都禁。
  if (rel == "\\PAGEFILE.SYS" || rel == "\\SWAPFILE.SYS" || rel == "\\HIBERFIL.SYS") {
    return I18n::tr("The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.");
  }

  const bool isSystemVolume = (volumeDl == QString::fromStdString(drive::systemLetter()));
  if (!isSystemVolume) return {};

  // 系统卷：几个关键目录本身不能排除（里面的具体文件可以）。
  if (rel == "\\WINDOWS") return I18n::tr("The entire \\Windows directory cannot be excluded.");
  if (rel == "\\WINDOWS\\SYSTEM32") return I18n::tr("The entire \\Windows\\System32 directory cannot be excluded.");
  if (rel == R"(\WINDOWS\SYSTEM32\DRIVERS)") return I18n::tr(R"(The entire \Windows\System32\Drivers directory cannot be excluded.)");

  // 系统卷：几个关键文件（注册表蜂巢 / 引导统计）。
  static const QStringList fixedBlocklist = {
      R"(\WINDOWS\SYSTEM32\CONFIG\DEFAULT)", R"(\WINDOWS\SYSTEM32\CONFIG\SAM)", R"(\WINDOWS\SYSTEM32\CONFIG\SECURITY)", R"(\WINDOWS\SYSTEM32\CONFIG\SOFTWARE)",
      R"(\WINDOWS\SYSTEM32\CONFIG\SYSTEM)",  "\\WINDOWS\\BOOTSTAT.DAT",         R"(\EFI\MICROSOFT\BOOT\BOOTSTAT.DAT)",  "\\BOOT\\BOOTSTAT.DAT",
  };
  for (const auto& b : fixedBlocklist) {
    if (rel == b) return I18n::tr("This critical system file cannot be excluded: %1").arg(b.toLower());
  }

  // 每个用户的 NTUSER.DAT：\Users\<User Name>\NTUSER.DAT，用户名任意但
  // 只有一层。
  if (rel.startsWith("\\USERS\\") && rel.endsWith("\\NTUSER.DAT")) {
    const auto parts = rel.split('\\', Qt::SkipEmptyParts);
    if (parts.size() == 3 && parts[0] == "USERS" && parts[2] == "NTUSER.DAT") {
      return I18n::tr("The per-user registry file NTUSER.DAT cannot be excluded.");
    }
  }

  return {};
}

// uwf::regkey::normalize 的 QString 包装——把注册表键归一成长写 hive 形式
// （HKLM\... → HKEY_LOCAL_MACHINE\...）。注册表键的存储 / 展示 / 校验全部先过
// 这一步，口径才统一。
QString normRegKey(const QString& key) { return QString::fromStdString(regkey::normalize(key.toStdString())); }

// 注册表排除规则：
//   1. 路径不得包含非法字符（控制字符、正斜杠、`\\` 空段、引导反斜杠等）；
//   2. 必须是以下 6 个允许顶层键的子键：
//        HKEY_LOCAL_MACHINE 下的 BCD00000000 / SYSTEM / SOFTWARE /
//        SAM / SECURITY / COMPONENTS；
//   3. 不得是 HKEY_LOCAL_MACHINE\SECURITY\Policy\Secrets\$MACHINE.ACC。
// 命中返回中文原因，否则返回空串。
QString forbidRegExclusionReason(const QString& rawKey) {
  QString input = rawKey.trimmed();
  // 末尾多余的 `\` 去掉，但若只剩 `\` 一个字符说明是纯非法输入。
  while (input.size() > 1 && input.endsWith('\\')) input.chop(1);
  if (input.isEmpty()) return I18n::tr("Registry key cannot be empty.");

  // 路径非法字符检查（在原始输入上做——这些字符归一化不处理）。
  if (input.startsWith('\\')) return I18n::tr("Registry key cannot start with a backslash.");
  if (input.contains("\\\\")) return I18n::tr("The path contains consecutive backslashes; this is not valid.");
  if (input.contains('/')) return I18n::tr("Registry paths use backslash `\\` as the separator; do not use forward slash `/`.");
  for (QChar c : input) {
    if (c.unicode() < 0x20) return I18n::tr("The path contains invisible control characters; this is not valid.");
  }

  // 归一成长写 hive 后再比对黑 / 白名单——简写 / 长写的口径统一交给 uwf::regkey。
  const QString upper = normRegKey(input).toUpper();

  // 黑名单：$MACHINE.ACC。放在白名单检查之前——它在 SECURITY 下能通过白名单，
  // 必须先单独挡掉。
  if (upper == R"(HKEY_LOCAL_MACHINE\SECURITY\POLICY\SECRETS\$MACHINE.ACC)")
    return I18n::tr(
        "HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly "
        "forbids excluding.");

  // 白名单：必须是 6 个允许顶层键的"子键"（不是顶层键本身，所以前缀末尾
  // 的 "\" 之后还得至少有一个字符）。
  static const QStringList allowedPrefixes = {
      R"(HKEY_LOCAL_MACHINE\BCD00000000\)", R"(HKEY_LOCAL_MACHINE\SYSTEM\)",   R"(HKEY_LOCAL_MACHINE\SOFTWARE\)",
      R"(HKEY_LOCAL_MACHINE\SAM\)",         R"(HKEY_LOCAL_MACHINE\SECURITY\)", R"(HKEY_LOCAL_MACHINE\COMPONENTS\)",
  };
  for (const auto& prefix : allowedPrefixes) {
    if (upper.startsWith(prefix) && upper.size() > prefix.size()) {
      return {};  // 合法
    }
  }

  return I18n::tr(
      "UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:\n  HKLM\\BCD00000000\n  "
      "HKLM\\SYSTEM\n  HKLM\\SOFTWARE\n  HKLM\\SAM\n  HKLM\\SECURITY\n  HKLM\\COMPONENTS");
}

void sortList(QStringList& list) {
  std::sort(list.begin(), list.end(), [](const QString& a, const QString& b) { return a.compare(b, Qt::CaseInsensitive) < 0; });
  list.removeDuplicates();
}

// 文件排除项：磁盘上存在的才能判断是目录还是文件；否则按路径启发式猜。
// 以 `\` 结尾、或没有扩展名 → 当目录，其它 → 当文件。
QIcon iconForFileEntry(const QString& path) {
  auto& tm = ThemeManager::instance();
  QFileInfo fi(path);
  if (fi.exists()) {
    return fi.isDir() ? tm.icon(":/icons/folder.svg") : tm.icon(":/icons/file.svg");
  }
  if (path.endsWith('\\') || path.endsWith('/')) {
    return tm.icon(":/icons/folder.svg");
  }
  return fi.suffix().isEmpty() ? tm.icon(":/icons/folder.svg") : tm.icon(":/icons/file.svg");
}

enum class Badge { None, Add, Remove };

// 把 base icon 和右下角的小徽标（+ / −）合成成一张 icon，
// 这样列表项既能区分类型（文件/目录/注册表），又能一眼看出
// 待添加 / 待删除 状态。逻辑尺寸 16×16，按 DPR 放大渲染以避免模糊。
QIcon composeWithBadge(const QIcon& base, Badge badge) {
  constexpr int kLogical = 16;
  qreal dpr = 1.0;
  if (auto* primary = QApplication::primaryScreen()) {
    dpr = primary->devicePixelRatio();
  }
  if (dpr < 1.0) dpr = 1.0;
  // 再乘 2 做超采样，让 SVG 栅格化后缩到 16 仍然锐利。
  const qreal renderScale = dpr * 2.0;
  const int kPx = static_cast<int>(std::lround(kLogical * renderScale));

  QPixmap pm(kPx, kPx);
  pm.setDevicePixelRatio(renderScale);
  pm.fill(Qt::transparent);

  QPainter p(&pm);
  p.setRenderHint(QPainter::Antialiasing);
  p.setRenderHint(QPainter::SmoothPixmapTransform);
  // 以逻辑坐标绘制；QPainter 会按 DPR 采样。
  const QSize hiRes(kPx, kPx);
  p.drawPixmap(QRect(0, 0, kLogical, kLogical), base.pixmap(hiRes, QIcon::Normal, QIcon::Off));

  if (badge != Badge::None) {
    auto& tm = ThemeManager::instance();
    const QColor bg = badge == Badge::Add ? tm.color(Sem::AddOk) : tm.color(Sem::Danger);
    // 徽标直径 7px，右下角贴边；用浮点坐标保证 +/− 严格居中。
    const qreal kBadge = 7.0;
    const QRectF br(kLogical - kBadge, kLogical - kBadge, kBadge, kBadge);
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawEllipse(br);

    QPen sym(Qt::white, 1.2);
    sym.setCapStyle(Qt::RoundCap);
    p.setPen(sym);
    const qreal cx = br.center().x();
    const qreal cy = br.center().y();
    constexpr qreal half = 1.7;
    p.drawLine(QPointF(cx - half, cy), QPointF(cx + half, cy));
    if (badge == Badge::Add) {
      p.drawLine(QPointF(cx, cy - half), QPointF(cx, cy + half));
    }
  }
  p.end();

  QIcon result;
  result.addPixmap(pm);
  return result;
}
}  // namespace

ExclusionListWidget::ExclusionListWidget(Kind kind, QWidget* parent) : QWidget(parent), m_kind(kind) {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(8);

  auto* header = new QHBoxLayout();
  m_filter = new QLineEdit(this);
  m_filter->setPlaceholderText(kind == Kind::File ? I18n::tr("Search file paths…") : I18n::tr("Search registry keys…"));
  m_filter->setClearButtonEnabled(true);
  m_filter->setObjectName("filterInput");
  header->addWidget(m_filter, 1);

  auto& tm = ThemeManager::instance();
  // primary / danger 这两个按钮在两种主题下都是填充式（蓝 / 红）+ 白字，
  // 对应 +/− 图标也固定用 dark 主题的亮灰前景，让 icon 在彩色背景上清晰。
  // 不跟随主题翻转——亮色下走默认染色会把 icon 染成深色，跟蓝/红底冲突。
  const QColor btnIconFg(0xE8EAED);

  if (kind == Kind::File) {
    // 一个"添加"菜单按钮：弹出菜单里再选文件 / 文件夹；避免两个并排按钮
    // 过多挤占筛选框的空间，也和右侧"持久化"菜单的交互模式保持一致。
    m_addBtn = new QPushButton(tm.iconWithColor(":/icons/add.svg", btnIconFg), I18n::tr("Add"), this);
    m_addBtn->setObjectName("primaryBtn");
    m_addBtn->setToolTip(I18n::tr("Add a file or folder to the exclusion list. Excluded entries are not protected by UWF."));
    auto* addMenu = new QMenu(m_addBtn);
    m_addFileAct = addMenu->addAction(tm.icon(":/icons/file.svg"), I18n::tr("File…"));
    m_addFileAct->setToolTip(I18n::tr("Pick a file to add to the exclusion list."));
    m_addDirAct = addMenu->addAction(tm.icon(":/icons/folder.svg"), I18n::tr("Folder…"));
    m_addDirAct->setToolTip(I18n::tr("Pick a folder (and all of its contents) to add to the exclusion list."));
    m_addBtn->setMenu(addMenu);
    header->addWidget(m_addBtn);
    connect(m_addFileAct, &QAction::triggered, this, &ExclusionListWidget::onAddFile);
    connect(m_addDirAct, &QAction::triggered, this, &ExclusionListWidget::onAddDir);
  } else {
    m_addBtn = new QPushButton(tm.iconWithColor(":/icons/add.svg", btnIconFg), I18n::tr("Add"), this);
    m_addBtn->setObjectName("primaryBtn");
    m_addBtn->setToolTip(I18n::tr("Enter a registry key path to add to the exclusion list."));
    header->addWidget(m_addBtn);
    connect(m_addBtn, &QPushButton::clicked, this, &ExclusionListWidget::onAddRegistry);
  }

  m_rmBtn = new QPushButton(tm.iconWithColor(":/icons/remove.svg", btnIconFg), I18n::tr("Remove selected"), this);
  m_rmBtn->setObjectName("dangerBtn");
  m_rmBtn->setToolTip(I18n::tr("Remove the selected entries from the exclusion list. Takes effect after Apply."));
  header->addWidget(m_rmBtn);
  layout->addLayout(header);

  // 摘要放在按钮行和列表之间，紧贴状态栏方向（顶部），
  // 而不是压在底部离 StatusPanel 太远。
  m_summary = new QLabel(this);
  m_summary->setObjectName("diffSummary");
  m_summary->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(m_summary);

  m_list = new QListWidget(this);
  m_list->setAlternatingRowColors(false);
  m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_list->setUniformItemSizes(true);
  m_list->setIconSize({16, 16});
  m_list->setObjectName("exclusionList");
  m_list->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
  m_list->setMinimumHeight(80);
  m_list->setMinimumWidth(0);
  layout->addWidget(m_list, 1);

  // 容器 border-radius=6，但 Qt QSS 不会把 item 的矩形背景按圆角裁剪，
  // 首尾 item 选中/hover 时矩形会越过圆角边缘。给 viewport 打一个圆角
  // QRegion mask，painter 被硬裁到这个形状里，首尾不再穿帮。
  // 走 eventFilter 监听 viewport 的 Resize，随大小变化重算 mask。
  m_list->viewport()->installEventFilter(this);
  applyViewportMask();

  connect(m_rmBtn, &QPushButton::clicked, this, &ExclusionListWidget::onRemove);
  connect(m_filter, &QLineEdit::textChanged, this, &ExclusionListWidget::onFilterChanged);
  connect(m_list, &QListWidget::itemDoubleClicked, this, &ExclusionListWidget::onItemDoubleClicked);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) { refreshThemedIcons(); });

  // 文件列表上的右键菜单：仅 File kind 提供（注册表条目没有"所在文件夹"
  // 或"提交改动"语义）。
  if (m_kind == Kind::File) {
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_list, &QListWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
      const auto* item = m_list->itemAt(pos);
      if (!item) return;
      const QString rel = item->data(Qt::UserRole).toString();
      if (rel.isEmpty()) return;

      // entry 可能是 "\Users\xxx" 或 "C:\Users\xxx"，统一拼成绝对路径
      // 给后续动作用。
      QString abs = rel;
      if (abs.startsWith('\\')) abs = m_driveLetter + abs;
      abs = QDir::toNativeSeparators(abs);

      auto& tm = ThemeManager::instance();
      QMenu menu(this);

      auto* openAct = menu.addAction(tm.icon(":/icons/folder.svg"), I18n::tr("Open containing folder"));
      connect(openAct, &QAction::triggered, this, [this, rel]() { openContainingFolder(rel); });

      // "提交改动到磁盘"只在两个条件同时满足时出现：
      //   1. 路径在磁盘上真实存在（既存的文件 / 文件夹）；
      //   2. 该条目在当前会话**尚未**被排除（不在 m_current 里）。
      // 已在当前会话被排除的条目，其写入本就绕过覆盖层直接落盘，覆盖层里
      // 没有可提交的改动——若仍提供提交项，点击后会被 MainWindow 以"提交
      // 被拒绝"挡回（见 MainWindow::commitFilePath）。只有下次会话才新增 /
      // 待应用的排除项当前仍受保护，覆盖层里才有改动可提交。
      // 按文件/文件夹分别用不同文案，最终都走 UWF_Volume.CommitFile
      //（MainWindow 那边收到 absPath 自己判断是文件还是目录、是否递归）。
      const QFileInfo fi(abs);
      const bool excludedInCurrentSession = m_current.contains(rel, Qt::CaseInsensitive);
      if (fi.exists() && !excludedInCurrentSession) {
        menu.addSeparator();
        const QString label = fi.isDir() ? I18n::tr("Commit folder changes to disk…") : I18n::tr("Commit file changes to disk…");
        auto* commitAct = menu.addAction(tm.icon(":/icons/commit.svg"), label);
        connect(commitAct, &QAction::triggered, this, [this, abs]() { emit commitFileRequested(abs); });
      }

      menu.exec(m_list->viewport()->mapToGlobal(pos));
    });
  }
}

void ExclusionListWidget::openContainingFolder(const QString& entry) const {
  // entry 可能是 "\Users\xxx\foo.txt"（相对卷根，常见）或 "C:\Users\xxx\..."
  // （带盘符）。统一拼成绝对路径再交给 Shell。
  QString abs = entry;
  if (abs.startsWith('\\')) abs = m_driveLetter + abs;
  abs = QDir::toNativeSeparators(abs);

  const QFileInfo fi(abs);
  if (fi.exists()) {
    // 用 Shell API 打开父目录并高亮目标，不走 explorer.exe /select 的命令行
    // 形式——后者在路径含空格时 QProcess 加的引号位置会让 explorer 解析失败，
    // 回退到默认（用户文档）目录。
    const std::wstring wide = abs.toStdWString();
    PIDLIST_ABSOLUTE pidl = ILCreateFromPathW(wide.c_str());
    if (pidl) {
      // 第二个参数为 0 表示选中 pidl 自己（默认行为），不需要额外子项。
      SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
      ILFree(pidl);
      return;
    }
  }

  // 路径不存在或 PIDL 创建失败：向上回溯到第一个还存在的父目录直接打开。
  QString folder = abs;
  while (!folder.isEmpty() && !QFileInfo::exists(folder)) {
    const qsizetype slash = folder.lastIndexOf(QChar('\\'));
    if (slash < 0) break;
    folder = folder.left(slash);
  }
  if (!folder.isEmpty() && QFileInfo::exists(folder)) {
    const std::wstring wf = folder.toStdWString();
    ShellExecuteW(nullptr, L"open", L"explorer.exe", wf.c_str(), nullptr, SW_SHOWNORMAL);
  }
}

void ExclusionListWidget::refreshThemedIcons() {
  auto& tm = ThemeManager::instance();
  // add / remove 按钮是填充式 fill button（primary 蓝 / danger 红），icon
  // 颜色固定用亮灰，不跟主题翻转。
  const QColor btnIconFg(0xE8EAED);
  if (m_addBtn) {
    m_addBtn->setIcon(tm.iconWithColor(":/icons/add.svg", btnIconFg));
  }
  if (m_addFileAct) m_addFileAct->setIcon(tm.icon(":/icons/file.svg"));
  if (m_addDirAct) m_addDirAct->setIcon(tm.icon(":/icons/folder.svg"));
  if (m_rmBtn) {
    m_rmBtn->setIcon(tm.iconWithColor(":/icons/remove.svg", btnIconFg));
  }
  // 列表项的 icon 在 rebuild() 里逐项重新生成；徽章颜色和文字色都在 rebuild
  // 里取自 ThemeManager，所以一次 rebuild 就完成全部同步。
  rebuild();
}

bool ExclusionListWidget::eventFilter(QObject* obj, QEvent* ev) {
  if (m_list && obj == m_list->viewport() && ev->type() == QEvent::Resize) {
    applyViewportMask();
  }
  return QWidget::eventFilter(obj, ev);
}

void ExclusionListWidget::applyViewportMask() {
  auto* vp = m_list ? m_list->viewport() : nullptr;
  if (!vp) return;
  const QRect r = vp->rect();
  if (r.isEmpty()) return;
  // 容器 border-radius=6、border=1px，border 内圈半径 = 6 − 1 = 5。
  // viewport 贴在 border 内侧，mask 得用内圈半径才能贴合容器曲率，
  // 否则四角会露出一条细窄的背景。
  QPainterPath path;
  path.addRoundedRect(r, 5, 5);
  vp->setMask(QRegion(path.toFillPolygon().toPolygon()));
}

void ExclusionListWidget::onItemDoubleClicked(QListWidgetItem* item) {
  if (!item) return;
  const QString path = item->data(Qt::UserRole).toString();
  if (path.isEmpty()) return;
  QApplication::clipboard()->setText(path);
  emit copiedToClipboard(I18n::tr("Copied to clipboard: ") + path);
}

void ExclusionListWidget::setDriveLetter(const QString& dl) {
  // 规范化交给 uwf::drive：trim + 大写 + 单个结尾冒号；非法输入（含空串）→ 空串。
  m_driveLetter = QString::fromStdString(drive::normalize(dl.toStdString()));
}

void ExclusionListWidget::setBaseline(const QStringList& current, const QStringList& next) {
  m_current = current;
  m_next = next;
  // 注册表 baseline 从 UWF 回读，可能简写 / 长写混杂；统一归一成长写，与用户
  // 新增项口径一致——展示统一、去重也不会把同一个键的两种写法漏成两条。
  if (m_kind == Kind::Registry) {
    for (QString& k : m_current) k = normRegKey(k);
    for (QString& k : m_next) k = normRegKey(k);
  }
  sortList(m_current);
  sortList(m_next);
  m_added.clear();
  m_removed.clear();
  rebuild();
  emit pendingChanged();
}

void ExclusionListWidget::resetPending() {
  m_added.clear();
  m_removed.clear();
  rebuild();
  emit pendingChanged();
}

QStringList ExclusionListWidget::pendingAdded() const {
  auto l = m_added.values();
  sortList(l);
  return l;
}

QStringList ExclusionListWidget::pendingRemoved() const {
  auto l = m_removed.values();
  sortList(l);
  return l;
}

void ExclusionListWidget::setReadOnly(bool ro) {
  m_readOnly = ro;
  m_filter->setEnabled(!ro);
  m_list->setEnabled(!ro);
  for (auto* btn : findChildren<QPushButton*>()) btn->setEnabled(!ro);
}

void ExclusionListWidget::onFilterChanged(const QString& text) {
  const QString needle = text.trimmed().toLower();
  for (int i = 0; i < m_list->count(); ++i) {
    auto* it = m_list->item(i);
    it->setHidden(!needle.isEmpty() && !it->text().toLower().contains(needle));
  }
}

void ExclusionListWidget::onAddFile() {
  if (m_readOnly) return;
  const QString base = m_driveLetter.isEmpty() ? QDir::homePath() : m_driveLetter + "\\";
  const QStringList paths = QFileDialog::getOpenFileNames(this, I18n::tr("Select files to add to the exclusion list (multiple selection allowed)"), base);
  for (const auto& p : paths) addPendingEntry(p);
}

void ExclusionListWidget::onAddDir() {
  if (m_readOnly) return;
  // Windows 原生目录选择框不支持多选，这里用原生框（好看），
  // 需要添加多个目录就多次点击"添加文件夹"。
  const QString base = m_driveLetter.isEmpty() ? QDir::homePath() : m_driveLetter + "\\";
  const QString path = QFileDialog::getExistingDirectory(this, I18n::tr("Select a folder to add to the exclusion list"), base);
  if (path.isEmpty()) return;
  addPendingEntry(path);
}

void ExclusionListWidget::onAddRegistry() {
  if (m_readOnly) return;
  bool ok = false;
  const QString input = QInputDialog::getText(this, I18n::tr("Add registry exclusion"), I18n::tr("Full registry key (e.g. HKLM\\Software\\MyApp):"),
                                              QLineEdit::Normal, QString(), &ok);
  if (!ok) return;
  addPendingEntry(input);
}

void ExclusionListWidget::addPendingEntry(const QString& raw) {
  QString p = raw.trimmed();
  if (p.isEmpty()) return;

  if (m_kind == Kind::File) {
    p = QDir::toNativeSeparators(p);
    if (!m_driveLetter.isEmpty() && !p.toUpper().startsWith(m_driveLetter + "\\") && p.toUpper() != m_driveLetter) {
      const QString body =
          I18n::tr("The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.").arg(p, m_driveLetter);
      dialogs::warning(this, I18n::tr("Path is not on this volume"), body);
      return;
    }
    // UWF 明确不允许的黑名单（卷根、Windows/System32/config 下的注册表蜂巢、
    // NTUSER.DAT、BOOTSTAT.DAT、分页文件等）——命中就直接拒绝并告诉用户原因。
    const QString reason = forbidExclusionReason(p, m_driveLetter);
    if (!reason.isEmpty()) {
      dialogs::warning(this, I18n::tr("Cannot add this exclusion"), reason);
      return;
    }
  } else {
    // Kind::Registry：先归一成长写（HKEY_*），保证存储 / 展示 / 去重口径统一；
    // 再做 UWF 文档明确禁止排除项的拦截。
    p = normRegKey(p);
    const QString regReason = forbidRegExclusionReason(p);
    if (!regReason.isEmpty()) {
      dialogs::warning(this, I18n::tr("Cannot add this exclusion"), regReason);
      return;
    }
  }

  if (setContainsCI(m_removed, p)) {
    setRemoveCI(m_removed, p);
  } else if (!m_next.contains(p, Qt::CaseInsensitive) && !m_current.contains(p, Qt::CaseInsensitive) && !setContainsCI(m_added, p)) {
    m_added.insert(p);
  }
  rebuild();
  emit pendingChanged();
}

ExclusionListWidget::ImportOutcome ExclusionListWidget::importAdd(const QString& raw) {
  QString p = raw.trimmed();
  if (p.isEmpty()) return ImportOutcome::NoOp;

  if (m_kind == Kind::File) {
    p = QDir::toNativeSeparators(p);
    if (!m_driveLetter.isEmpty() && !p.toUpper().startsWith(m_driveLetter + "\\") && p.toUpper() != m_driveLetter) {
      return ImportOutcome::RejectedNotOnVolume;
    }
    if (!forbidExclusionReason(p, m_driveLetter).isEmpty()) {
      return ImportOutcome::RejectedForbidden;
    }
  } else {
    // Kind::Registry：归一成长写后再校验（与 addPendingEntry 同口径）。
    p = normRegKey(p);
    if (!forbidRegExclusionReason(p).isEmpty()) {
      return ImportOutcome::RejectedForbidden;
    }
  }

  // 复用 addPendingEntry 内部的状态机：在 m_removed 里 → 撤销移除；
  // 否则若不在基线和 m_added 里 → 新增到 m_added。
  bool changed = false;
  if (setContainsCI(m_removed, p)) {
    setRemoveCI(m_removed, p);
    changed = true;
  } else if (!m_next.contains(p, Qt::CaseInsensitive) && !m_current.contains(p, Qt::CaseInsensitive) && !setContainsCI(m_added, p)) {
    m_added.insert(p);
    changed = true;
  }
  if (!changed) return ImportOutcome::NoOp;
  rebuild();
  emit pendingChanged();
  return ImportOutcome::Applied;
}

ExclusionListWidget::ImportOutcome ExclusionListWidget::importRemove(const QString& raw) {
  QString p = raw.trimmed();
  if (p.isEmpty()) return ImportOutcome::NoOp;
  // 与录入侧同口径：文件归一分隔符，注册表归一成长写 hive。
  if (m_kind == Kind::File)
    p = QDir::toNativeSeparators(p);
  else
    p = normRegKey(p);

  // 镜像 onRemove 的状态机：若条目还在 m_added → 撤销 add；
  // 否则若它在 m_next 基线里 → 标 removed。其余情况都属于 NoOp。
  bool changed = false;
  if (setContainsCI(m_added, p)) {
    setRemoveCI(m_added, p);
    changed = true;
  } else if (m_next.contains(p, Qt::CaseInsensitive) && !setContainsCI(m_removed, p)) {
    m_removed.insert(p);
    changed = true;
  }
  if (!changed) return ImportOutcome::NoOp;
  rebuild();
  emit pendingChanged();
  return ImportOutcome::Applied;
}

void ExclusionListWidget::onRemove() {
  if (m_readOnly) return;
  for (auto* it : m_list->selectedItems()) {
    const QString text = it->data(Qt::UserRole).toString();
    if (setContainsCI(m_added, text))
      setRemoveCI(m_added, text);
    else if (m_next.contains(text, Qt::CaseInsensitive))
      m_removed.insert(text);
  }
  rebuild();
  emit pendingChanged();
}

void ExclusionListWidget::rebuild() {
  m_list->clear();

  // 先把所有可能出现的条目去重收齐——四个来源：本次会话已加的 m_added、
  // 本次会话标记删除的 m_removed、当前会话基线 m_current、下次会话基线 m_next。
  QSet<QString> seen;
  QStringList all;
  auto pushUnique = [&](const QString& s) {
    const auto k = lowerKey(s);
    if (!seen.contains(k)) {
      seen.insert(k);
      all << s;
    }
  };
  for (const auto& s : m_added) pushUnique(s);
  for (const auto& s : m_removed) pushUnique(s);
  for (const auto& s : m_current) pushUnique(s);
  for (const auto& s : m_next) pushUnique(s);

  // 排序规则：所有"带标记"的条目（新增 / 删除，无论是否已应用）排在前排，
  // 后面是"保持不变"的条目；前后两段各自按字母顺序。
  // 这里的判定必须和下方 badge 的判定完全一致："带标记" = 有用户改动
  // (userAdded / userRemoved) 或基线在当前 / 下次会话间本就有增删差异
  // (inCurrent != inNextBase)。
  //
  // 不能用 inCurrent != inNextFinal：删除一个"仅下次会话存在、当前未生效"的
  // 条目时，inCurrent 与 inNextFinal 同为 false，会被误判成"无变化"沉到
  // rest——可它明明带着红色删除标记，理应和其它待删项一起排在前排。
  QStringList pending;
  QStringList rest;
  for (const auto& entry : all) {
    const bool inCurrent = m_current.contains(entry, Qt::CaseInsensitive);
    const bool inNextBase = m_next.contains(entry, Qt::CaseInsensitive);
    const bool userAdded = m_added.contains(entry);
    const bool userRemoved = m_removed.contains(entry);
    if (userAdded || userRemoved || inCurrent != inNextBase) {
      pending << entry;
    } else {
      rest << entry;
    }
  }
  sortList(pending);
  sortList(rest);
  QStringList display = pending + rest;

  int added = 0, removed = 0, changed = 0;

  for (const auto& entry : display) {
    auto* item = new QListWidgetItem();
    item->setData(Qt::UserRole, entry);

    const bool inCurrent = m_current.contains(entry, Qt::CaseInsensitive);
    const bool inNextBase = m_next.contains(entry, Qt::CaseInsensitive);
    const bool userAdded = m_added.contains(entry);
    const bool userRemoved = m_removed.contains(entry);
    const bool inNextFinal = (inNextBase || userAdded) && !userRemoved;

    Badge badge = Badge::None;
    QBrush fg;

    auto& tm = ThemeManager::instance();
    if (userAdded || userRemoved) {
      ++changed;
      if (userAdded) {
        badge = Badge::Add;
        fg = QBrush(tm.color(Sem::AddOk));
      } else {
        badge = Badge::Remove;
        fg = QBrush(tm.color(Sem::Danger));
      }
    } else if (!inCurrent && inNextBase) {
      badge = Badge::Add;
      fg = QBrush(tm.color(Sem::AddOk));
      ++added;
    } else if (inCurrent && !inNextBase) {
      badge = Badge::Remove;
      fg = QBrush(tm.color(Sem::Danger));
      ++removed;
    } else {
      fg = QBrush(tm.color(Sem::Fg));
    }

    item->setText(entry);
    item->setForeground(fg);
    const QIcon baseIcon = m_kind == Kind::File ? iconForFileEntry(entry) : tm.icon(":/icons/registry.svg");
    item->setIcon(composeWithBadge(baseIcon, badge));

    const QString pendingText =
        userAdded ? I18n::tr("Add") : (userRemoved ? I18n::tr("Remove") : (inNextFinal ? I18n::tr("Keep") : I18n::tr("Already removed")));
    const QString yes = I18n::tr("Yes");
    const QString no = I18n::tr("No");
    const QString tip =
        I18n::tr("Current session: %1\nNext session (saved on disk): %2\nPending change: %3").arg(inCurrent ? yes : no, inNextBase ? yes : no, pendingText);
    item->setToolTip(tip);
    m_list->addItem(item);
  }

  m_summary->setText(I18n::tr("%1 entries · %2 to add · %3 to remove in next session · %4 pending").arg(display.size()).arg(added).arg(removed).arg(changed));

  onFilterChanged(m_filter->text());
}

}  // namespace uwf::ui
