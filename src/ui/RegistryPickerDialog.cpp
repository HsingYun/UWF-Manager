#include "RegistryPickerDialog.h"

#include <windows.h>  // REG_SZ / REG_DWORD / ... 值类型宏，formatValuePreview switch 用

#include <QBrush>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <algorithm>

#include "../core/Config.h"
#include "../util/RegistryKey.h"
#include "I18n.h"
#include "ThemeManager.h"

namespace uwf::ui {

namespace {

// 树节点的 Qt::UserRole 存的是该节点对应的完整长写路径（HKEY_LOCAL_MACHINE\...）。
constexpr int kPathRole = Qt::UserRole;
// 子节点是否已经 loadChildren 过的标记——避免重复 RegEnumKey，也让首次展开时
// 能识别"占位 ..."替换的时机。
constexpr int kLoadedRole = Qt::UserRole + 1;

// 树根的 5 个 hive 占位提示文案占位项——树节点必须先有一个子项 Qt 才会画出
// 展开箭头；真正展开时再用 loadChildren 替换为真实子键。
QString placeholderText() { return QStringLiteral("…"); }

QTreeWidgetItem* makePlaceholderChild() {
  auto* p = new QTreeWidgetItem(QStringList{placeholderText()});
  // disabled 占位行无图标 / 不可选中——视觉上"半透明"，鼠标不会误中。
  p->setFlags(Qt::ItemIsEnabled);
  return p;
}

QIcon iconForKey() { return ThemeManager::instance().icon(":/icons/registry.svg"); }

// 把"完整路径"拆成 hive + 之后的段：("HKEY_LOCAL_MACHINE", ["SOFTWARE", "Microsoft", ...])
// 用于地址栏导航——按段一层一层在树里展开。
std::pair<QString, QStringList> splitKeyPath(const QString& key) {
  const auto parts = key.split('\\', Qt::SkipEmptyParts);
  if (parts.isEmpty()) return {};
  return {parts.front(), parts.mid(1)};
}

// 把值的原始字节按 Win32 类型码格式化成可读单行预览。规则贴近 regedit：
//   REG_SZ / EXPAND_SZ / LINK: 直出字符串（去尾部 null）
//   REG_MULTI_SZ:              多串以 " · " 连接
//   REG_DWORD / QWORD:         "0x%X (%u)"
//   REG_BINARY 等:             空格分隔 hex 字节
// 任一长度都截到 200 字符 + "…"——picker 是预览，不是查看器。
constexpr int kPreviewMaxChars = 200;
QString truncatedAt(QString s, int max = kPreviewMaxChars) {
  if (s.size() > max) {
    s.resize(max);
    s.append(QStringLiteral("…"));
  }
  return s;
}
QString formatValuePreview(uint32_t type, const std::vector<uint8_t>& data) {
  // 空数据：REG_SZ 走 regedit 的"未设置默认值"口径，其他类型显示一般的"空"。
  if (data.empty()) return type == REG_SZ ? I18n::tr("(value not set)") : I18n::tr("(empty)");

  switch (type) {
    case REG_SZ:
    case REG_EXPAND_SZ:
    case REG_LINK: {
      // UTF-16 LE 字节流；末尾的 null terminator（一对 0x00 0x00）要去掉。
      const auto chars = static_cast<qsizetype>(data.size() / 2);
      QString s = QString::fromUtf16(reinterpret_cast<const char16_t*>(data.data()), chars);
      while (s.endsWith(QChar(u'\0'))) s.chop(1);
      return truncatedAt(s);
    }
    case REG_MULTI_SZ: {
      const auto chars = static_cast<qsizetype>(data.size() / 2);
      const QStringView view(reinterpret_cast<const char16_t*>(data.data()), chars);
      QStringList parts;
      qsizetype begin = 0;
      for (qsizetype i = 0; i < view.size(); ++i) {
        if (view[i] == QChar(u'\0')) {
          if (i > begin) parts.append(view.mid(begin, i - begin).toString());
          begin = i + 1;
        }
      }
      return truncatedAt(parts.join(QStringLiteral(" · ")));
    }
    case REG_DWORD: {
      if (data.size() < 4) return I18n::tr("(empty)");
      const uint32_t v = static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) | (static_cast<uint32_t>(data[2]) << 16) |
                         (static_cast<uint32_t>(data[3]) << 24);
      return QString::asprintf("0x%08X (%u)", v, v);
    }
    case REG_DWORD_BIG_ENDIAN: {
      if (data.size() < 4) return I18n::tr("(empty)");
      const uint32_t v = (static_cast<uint32_t>(data[0]) << 24) | (static_cast<uint32_t>(data[1]) << 16) | (static_cast<uint32_t>(data[2]) << 8) |
                         static_cast<uint32_t>(data[3]);
      return QString::asprintf("0x%08X (%u)", v, v);
    }
    case REG_QWORD: {
      if (data.size() < 8) return I18n::tr("(empty)");
      uint64_t v = 0;
      for (size_t i = 0; i < 8; ++i) v |= static_cast<uint64_t>(data[i]) << (i * 8);
      return QString::asprintf("0x%016llX (%llu)", static_cast<unsigned long long>(v), static_cast<unsigned long long>(v));
    }
    default: {
      // REG_BINARY / REG_NONE / REG_RESOURCE_* 等 —— hex 序列；截到前 64 字节
      // 已经远超列宽，加 "…" 提示有更多。
      constexpr size_t kMaxBytes = 64;
      const size_t shown = std::min(data.size(), kMaxBytes);
      QString hex;
      hex.reserve(static_cast<qsizetype>(shown * 3));
      for (size_t i = 0; i < shown; ++i) {
        if (i > 0) hex.append(' ');
        hex.append(QString::asprintf("%02X", data[i]));
      }
      if (data.size() > kMaxBytes) hex.append(QStringLiteral(" …"));
      return hex;
    }
  }
}

}  // namespace

RegistryPickerDialog::RegistryPickerDialog(Mode mode, const QString& title, QWidget* parent) : QDialog(parent), m_mode(mode) {
  setWindowTitle(title);
  // 默认尺寸：三种模式都展示 "树 + 值表" 两栏，给 1000 宽够展开常见的注册表键名
  // （HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\... 类）+ 装下值表三列。
  // setMinimumSize 防止用户把对话框拖到 Data 列被压成 "C:..." 的窘境。
  resize(1000, 600);
  setMinimumSize(720, 480);
  buildUi();
  populateRootHives();
}

void RegistryPickerDialog::buildUi() {
  auto* root = new QVBoxLayout(this);
  root->setSpacing(10);

  // 地址栏：用户可输入 / 粘贴完整键路径，回车跳到该路径（树自动展开）。
  // placeholder 也告诉用户支持简写 hive（HKLM / HKCU）。最右侧一个刷新按钮——
  // 重新枚举注册表（外部 regedit / 命令行改过之后，picker 不会自己感知）。
  auto* addrRow = new QHBoxLayout();
  auto* addrLabel = new QLabel(I18n::tr("Key path:"), this);
  m_addressBar = new QLineEdit(this);
  m_addressBar->setPlaceholderText(I18n::tr("HKLM\\Software\\... — type or paste, press Enter to jump"));
  m_addressBar->setClearButtonEnabled(true);
  auto* refreshBtn = new QPushButton(this);
  refreshBtn->setIcon(ThemeManager::instance().icon(":/icons/refresh.svg"));
  refreshBtn->setToolTip(I18n::tr("Refresh"));
  connect(refreshBtn, &QPushButton::clicked, this, &RegistryPickerDialog::onRefresh);
  addrRow->addWidget(addrLabel);
  addrRow->addWidget(m_addressBar, 1);
  addrRow->addWidget(refreshBtn);
  root->addLayout(addrRow);
  connect(m_addressBar, &QLineEdit::returnPressed, this, &RegistryPickerDialog::onAddressBarReturnPressed);
  // 地址栏被清空（X 按钮 / 用户全删 / 任何外部 setText("")）→ 回初态。
  // 树和值表选中都清掉、树折叠回 populateRootHives 后的默认（仅 HKLM 展开）。
  // setText 不为空时 onTreeSelectionChanged 仍会同步地址栏，不会丢失选中。
  connect(m_addressBar, &QLineEdit::textChanged, this, [this](const QString& text) {
    if (text.isEmpty()) {
      m_tree->clearSelection();
      if (m_valueTable) m_valueTable->clearSelection();
      populateRootHives();  // 重建 5 个 hive + 自动展开 HKLM
      refreshSelectionLabels();
      updateOkButton();
    }
  });

  // 主体：三种模式都用水平 splitter——左树 + 右值表。值表的行为按 mode 分两类：
  //   CommitValue / DeleteValue: 值表可选，未选 = 整键递归；
  //   Exclusion: 值表整表禁选，纯展示——让用户看清这个键里有什么值再决定是否
  //              加排除（WMI 的 AddExclusion 本来就不收 ValueName，OK 始终
  //              返回 key 配空值名）。
  m_tree = new QTreeWidget(this);
  m_tree->setHeaderHidden(true);
  m_tree->setUniformRowHeights(true);
  m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(m_tree, &QTreeWidget::itemExpanded, this, &RegistryPickerDialog::onItemExpanded);
  connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &RegistryPickerDialog::onTreeSelectionChanged);

  auto* split = new QSplitter(Qt::Horizontal, this);
  split->addWidget(m_tree);

  m_valueTable = new QTableWidget(0, 3, this);
  m_valueTable->setHorizontalHeaderLabels({I18n::tr("Name"), I18n::tr("Type"), I18n::tr("Data")});
  m_valueTable->verticalHeader()->setVisible(false);
  m_valueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_valueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  // Exclusion 模式整表禁选——AddExclusion 不收 ValueName，值表纯展示。
  m_valueTable->setSelectionMode(supportsValueSelection() ? QAbstractItemView::SingleSelection : QAbstractItemView::NoSelection);
  // mouseTracking 必须开在 viewport 上——QTableView 本体的设置不会自动传播给 viewport，
  // 而 QSS 的 ::item:hover 是在 viewport 内 hover 时才触发。两个都开是稳妥写法。
  m_valueTable->setMouseTracking(true);
  // Name / Type 固定起步宽度（Interactive 允许用户拖拽微调）；Data 走 stretchLastSection
  // 永远吃 viewport 剩余宽度——这样 Data 列永远等于"能填多宽就填多宽"，"…"只在 viewport
  // 真容不下时才出现，避免出现 H scrollbar 把 Data 压成 "C:..." 的尴尬。用户仍可拖
  // Type/Data 边界调整起点。每个 cell 的 tooltip 兜底超长内容。
  m_valueTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  m_valueTable->horizontalHeader()->resizeSection(0, 200);  // Name
  m_valueTable->horizontalHeader()->resizeSection(1, 130);  // Type（REG_RESOURCE_REQUIREMENTS_LIST 是最长的）
  m_valueTable->horizontalHeader()->setStretchLastSection(true);  // Data
  connect(m_valueTable, &QTableWidget::itemSelectionChanged, this, &RegistryPickerDialog::onValueSelectionChanged);
  // 值表 viewport 上单击空白 → 清掉选中（valueName 回到空）。Qt 默认不会清，
  // 这里走 eventFilter 自己处理。仅 m_valueTable 创建之后才能挂——viewport 在
  // QAbstractScrollArea 内部生成，它存在了才可监听。
  m_valueTable->viewport()->installEventFilter(this);
  // 关键：QSS 的 ::item:hover 需要 viewport 自身开 mouseTracking——widget 上设的
  // 不会自动传到 viewport。只设 widget 那条，鼠标按下移动才有事件，纯 hover 无效。
  m_valueTable->viewport()->setMouseTracking(true);

  split->addWidget(m_valueTable);
  // 比例 1:3 + 初始 sizes：树 1/4 + 值表 3/4——dialog 默认 1000，值表起步 ~750，
  // 扣除 Name 200 + Type 130 + V scrollbar 17 + 边框，Data 还有 ~400px 起步宽度。
  split->setStretchFactor(0, 1);
  split->setStretchFactor(1, 3);
  split->setSizes({250, 750});
  root->addWidget(split, 1);

  // 底部"当前选择"展示：键一行，支持值选择的模式额外加值一行。Exclusion 不
  // 展示值——值表纯辅助查看、整表禁选、不参与结果，再写一行徒增噪音。可选中
  // 文本方便用户拷走路径。
  m_keyLabel = new QLabel(this);
  m_keyLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_keyLabel->setWordWrap(true);
  root->addWidget(m_keyLabel);

  if (supportsValueSelection()) {
    m_valueLabel = new QLabel(this);
    m_valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_valueLabel->setWordWrap(true);
    root->addWidget(m_valueLabel);
  }

  // 显式 I18n::tr 的按钮——不走 QDialogButtonBox::Ok / Cancel 标准枚举，那条路径
  // 的文案来自 Qt 内置 qt_zh_CN.qm（我们没加载），会卡在英文。
  auto* btns = new QDialogButtonBox(this);
  m_okBtn = btns->addButton(I18n::tr("OK"), QDialogButtonBox::AcceptRole);
  btns->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  m_okBtn->setEnabled(false);
  connect(btns, &QDialogButtonBox::accepted, this, [this] {
    // valueName 空 = 整键递归（值表无选中或 Exclusion 模式整表禁选）；非空 = 命名值。
    m_result = Result{currentKeyPath(), currentValueName()};
    accept();
  });
  connect(btns, &QDialogButtonBox::rejected, this, [this] {
    m_result.reset();
    reject();
  });
  root->addWidget(btns);

  refreshSelectionLabels();
}

void RegistryPickerDialog::populateRootHives() {
  m_tree->clear();
  QTreeWidgetItem* hklmItem = nullptr;
  for (const auto& hive : regkey::rootHiveLongNames()) {
    const QString hivePath = QString::fromStdString(hive);
    auto* item = new QTreeWidgetItem(m_tree, QStringList{hivePath});
    item->setIcon(0, iconForKey());
    item->setData(0, kPathRole, hivePath);
    item->setData(0, kLoadedRole, false);
    // 顶层 hive 永远 hasRealSubkeys=true——5 个 hive 在装了系统的机器上都非空。
    // 三种模式默认共用 standardAvailability：HKLM 会被判 ContainerOnly（子树里
    // 有合法 6-prefix），其余 hive（HKCU / HKCR / HKU / HKCC）一律 Pruned → 不加
    // 占位子节点，根本展不开。
    applyAvailability(item, hivePath, /*hasRealSubkeys=*/true);
    if (hive == "HKEY_LOCAL_MACHINE") hklmItem = item;
  }
  // HKLM 默认展开一层——picker 90% 用途（排除 / commit / delete）都是 HKLM 子树；
  // 一打开就让用户看到 SOFTWARE / SYSTEM / SAM 那几个允许的根，省一次点击。
  if (hklmItem) hklmItem->setExpanded(true);  // 触发 onItemExpanded → loadChildren
}

void RegistryPickerDialog::loadChildren(QTreeWidgetItem* item) {
  if (!item) return;
  if (item->data(0, kLoadedRole).toBool()) return;  // 已加载过——拒重复 enum
  item->setData(0, kLoadedRole, true);

  // 清掉占位 "…"——真实子键即将顶上。
  while (item->childCount() > 0) {
    auto* c = item->takeChild(0);
    delete c;
  }

  const QString path = item->data(0, kPathRole).toString();
  // Pruned 节点本不该走到这里（applyAvailability 没给它加占位子节点 → Qt 不画
  // 展开箭头）；但 navigateTo 会强制 loadChildren——这里再兜一层，Pruned 一律
  // 不下钻，避免地址栏拼一个无效路径就枚举出整棵 HKCU。
  if (availabilityOf(path) == KeyAvailability::Pruned) return;
  // HKLM 根下的 PERSISTENT_SOFTWARE / PERSISTENT_SYSTEM 是 UWF 在内核侧挂的
  // 物理盘直通 hive（绕过 overlay，反映真实落盘内容）。它们由 UWF 自己使用，
  // 用户从不会想 commit / exclude；regedit GUI 也过滤掉了，picker 跟齐。
  // 命中条件够窄（仅 HKLM 一层 + 严格 PERSISTENT_ 前缀），不会误伤用户键。
  const bool atHklmRoot = path == QLatin1String("HKEY_LOCAL_MACHINE");
  const auto children = regkey::subkeyNames(path.toStdString());
  for (const auto& name : children) {
    if (atHklmRoot && name.starts_with("PERSISTENT_")) continue;
    auto* child = new QTreeWidgetItem(item, QStringList{QString::fromStdString(name)});
    child->setIcon(0, iconForKey());
    const QString childPath = path + '\\' + QString::fromStdString(name);
    child->setData(0, kPathRole, childPath);
    child->setData(0, kLoadedRole, false);
    // 访问被拒（HKLM\SAM 等）hasSubkeys 返回 false，按叶子处理——和直观一致。
    const bool real = regkey::hasSubkeys(childPath.toStdString());
    applyAvailability(child, childPath, real);
  }
}

void RegistryPickerDialog::applyAvailability(QTreeWidgetItem* item, const QString& path, bool hasRealSubkeys) {
  // 不依赖 QSS 的 disabled 选择器——某些主题下 disabled 行字色和正常色几乎
  // 看不出差异；FgMuted 是 ThemeManager 给"次文字"用的语义灰，明显比 Fg 暗，
  // 两种主题下都已校好。Pruned 和 ContainerOnly 都用同一灰——靠"有无展开箭头"
  // 区分两者：Pruned 没箭头根本下不去、ContainerOnly 有箭头可继续找合法子键。
  const QBrush mutedBrush(ThemeManager::instance().color(Sem::FgMuted));
  switch (availabilityOf(path)) {
    case KeyAvailability::Pruned:
      // NoItemFlags 让 Qt 不响应点击；不加占位子节点 → Qt 不画展开箭头。
      item->setFlags(Qt::NoItemFlags);
      item->setForeground(0, mutedBrush);
      break;
    case KeyAvailability::ContainerOnly:
      // 可展开但不可选：用户能继续往下找合法 key，但点中本节点 OK 灰。
      item->setFlags(Qt::ItemIsEnabled);
      item->setForeground(0, mutedBrush);
      if (hasRealSubkeys) item->addChild(makePlaceholderChild());
      break;
    case KeyAvailability::Selectable:
      // 默认 flags（可选可展开），字色走 style 默认。
      if (hasRealSubkeys) item->addChild(makePlaceholderChild());
      break;
  }
}

RegistryPickerDialog::KeyAvailability RegistryPickerDialog::availabilityOf(const QString& key) const {
  // 未注入 checker → 跑默认的 standardAvailability（6 前缀白名单 + $MACHINE.ACC 黑名单）。
  return m_availability ? m_availability(key) : standardAvailability(key);
}

RegistryPickerDialog::KeyAvailability RegistryPickerDialog::standardAvailability(const QString& key) {
  // 树节点路径都来自实机 RegEnumKey，syntax 已干净——不必复查首字符 / 连续反斜杠之类，
  // 只关心"是否在 6 前缀白名单内"和"是否是 $MACHINE.ACC 黑名单"。
  const QString upper = QString::fromStdString(regkey::normalize(key.toStdString())).toUpper();
  if (upper.isEmpty()) return KeyAvailability::Pruned;

  // 黑名单：$MACHINE.ACC（域机器账户密钥）—— UWF 明确禁排，且单独 commit 它
  // 会破坏域信任，所有模式一律 Pruned。
  if (upper == QLatin1String(config::kForbiddenRegistryKeyMachineAccount.data(), static_cast<qsizetype>(config::kForbiddenRegistryKeyMachineAccount.size()))) {
    return KeyAvailability::Pruned;
  }

  // 白名单：必须是 6 前缀之一的真子键（前缀以 "\" 结尾，故还需多一个字符）。
  for (const auto sv : config::kAllowedRegistryRootPrefixes) {
    const QLatin1String prefix(sv.data(), static_cast<qsizetype>(sv.size()));
    if (upper.startsWith(prefix) && upper.size() > prefix.size()) {
      return KeyAvailability::Selectable;
    }
  }

  // 不在白名单，但若 key 是某 prefix 的祖先 / 自身 = prefix 去尾，则子树里有合法 key →
  // ContainerOnly（可展开、不可选）。两种情况：
  //   - key+'\\' 是 prefix 的前缀  ⇒ prefix 在 key 之下，key 是它的祖先
  //   - prefix 是 key+'\\' 的前缀  ⇒ key 等于某 prefix 去掉尾部 '\\'
  const QString withSep = upper + '\\';
  for (const auto sv : config::kAllowedRegistryRootPrefixes) {
    const QLatin1String prefix(sv.data(), static_cast<qsizetype>(sv.size()));
    if (withSep.startsWith(prefix)) return KeyAvailability::ContainerOnly;
    if (QString(prefix).startsWith(withSep)) return KeyAvailability::ContainerOnly;
  }
  return KeyAvailability::Pruned;
}

void RegistryPickerDialog::onItemExpanded(QTreeWidgetItem* item) { loadChildren(item); }

void RegistryPickerDialog::onRefresh() {
  // 整树重建（populateRootHives 会清掉所有节点 + kLoadedRole，下次展开时
  // loadChildren 会重新跑 RegEnumKey）；地址栏不动，重建后再 navigateTo 一次
  // 把用户原来的选中位置恢复出来——感知是"刷新了，但还在原地"。键被外部删了
  // 时 navigateTo 自然会停在最深可达层，与首次输入路径的行为一致。
  const QString currentPath = m_addressBar->text().trimmed();
  populateRootHives();
  if (!currentPath.isEmpty()) navigateTo(currentPath);
}

void RegistryPickerDialog::onTreeSelectionChanged() {
  const QString key = currentKeyPath();
  // 地址栏跟随选中——但不要让 setText 触发我们自己的 returnPressed 回环。
  // QLineEdit::setText 不会触发 returnPressed，安全。
  if (!key.isEmpty()) m_addressBar->setText(key);
  refreshValueTable(key);
  refreshSelectionLabels();
  updateOkButton();
}

void RegistryPickerDialog::onValueSelectionChanged() {
  refreshSelectionLabels();
  updateOkButton();
}

bool RegistryPickerDialog::eventFilter(QObject* obj, QEvent* ev) {
  // Exclusion 模式整表 NoSelection，没选中可清——直接走默认。
  if (supportsValueSelection() && m_valueTable && obj == m_valueTable->viewport() && ev->type() == QEvent::MouseButtonPress) {
    const auto* me = static_cast<QMouseEvent*>(ev);
    if (!m_valueTable->indexAt(me->pos()).isValid()) {
      // 不吃事件——让 QTableWidget 仍能按常规处理（点击空白本来就不会改选中，
      // 这里只是把当前选中清掉，让"值=空"语义表达"整键递归"）。
      m_valueTable->clearSelection();
    }
  }
  return QDialog::eventFilter(obj, ev);
}

void RegistryPickerDialog::refreshValueTable(const QString& keyPath) {
  if (!m_valueTable) return;
  m_valueTable->setRowCount(0);
  if (keyPath.isEmpty()) return;
  auto items = regkey::values(keyPath.toStdString());
  // (Default) 行恒列：枚举里若已有（name==""）则移到最前；没有就补一行
  // type=REG_SZ(1) 占位——regedit 同款口径，每个键都展示默认值，类型显示为
  // REG_SZ（即便实际未设值）。
  auto def = std::find_if(items.begin(), items.end(), [](const auto& v) { return v.name.empty(); });
  if (def != items.end()) {
    std::rotate(items.begin(), def, def + 1);
  } else {
    items.insert(items.begin(), {std::string{}, 1u, {}});  // 1 = REG_SZ + 空 data → 预览显示"(value not set)"
  }
  // Exclusion 模式整表都不让选；CommitValue / DeleteValue 模式让命名值可选，
  // 但 (Default) 永远禁选——见类注释里"三种模式下 (Default) 行恒禁用"。
  const bool tableSelectable = supportsValueSelection();
  m_valueTable->setRowCount(static_cast<int>(items.size()));
  for (size_t i = 0; i < items.size(); ++i) {
    const auto& v = items[i];
    const int row = static_cast<int>(i);
    const bool isDefault = v.name.empty();
    const bool rowSelectable = tableSelectable && !isDefault;
    const QString displayName = isDefault ? I18n::tr("(Default)") : QString::fromStdString(v.name);
    auto* nameItem = new QTableWidgetItem(displayName);
    // 把真实值名（可能空）藏在 UserRole——显示名是翻译过的 "(默认)"，提交时
    // 要拿原始空串。
    nameItem->setData(Qt::UserRole, QString::fromStdString(v.name));
    nameItem->setToolTip(displayName);
    // NoItemFlags 让 Qt 的默认 style 把行画成灰字 + 不响应点击——用户一眼能看出
    // "这行没法选"，而不是看起来正常却莫名不响应。
    if (!rowSelectable) nameItem->setFlags(Qt::NoItemFlags);
    m_valueTable->setItem(row, 0, nameItem);

    const QString typeText = QString::fromStdString(regkey::valueTypeName(v.type));
    auto* typeItem = new QTableWidgetItem(typeText);
    typeItem->setToolTip(typeText);  // REG_RESOURCE_REQUIREMENTS_LIST 超过列宽时 hover 还原全名
    if (!rowSelectable) typeItem->setFlags(Qt::NoItemFlags);
    m_valueTable->setItem(row, 1, typeItem);

    // Data 列：单行预览（已 truncate 到 200 字符），tooltip 给同样的预览方便长串
    // 鼠标悬停查看——不显示完整数据，picker 不是查看器。
    const QString preview = formatValuePreview(v.type, v.data);
    auto* dataItem = new QTableWidgetItem(preview);
    dataItem->setToolTip(preview);
    if (!rowSelectable) dataItem->setFlags(Qt::NoItemFlags);
    m_valueTable->setItem(row, 2, dataItem);
  }
}

void RegistryPickerDialog::onAddressBarReturnPressed() {
  const QString raw = m_addressBar->text().trimmed();
  if (raw.isEmpty()) return;
  navigateTo(raw);
}

void RegistryPickerDialog::navigateTo(const QString& keyPath) {
  // 归一成长写——树根节点用的是长写名（HKEY_LOCAL_MACHINE 等），简写匹配不上。
  const QString normalized = QString::fromStdString(regkey::normalize(keyPath.toStdString()));
  if (normalized.isEmpty()) return;

  const auto [hive, segments] = splitKeyPath(normalized);
  if (hive.isEmpty()) return;

  // 1) 在根列表里找 hive 节点。
  QTreeWidgetItem* current = nullptr;
  for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
    auto* root = m_tree->topLevelItem(i);
    if (root->text(0).compare(hive, Qt::CaseInsensitive) == 0) {
      current = root;
      break;
    }
  }
  if (!current) return;  // hive 名拼错——沉默：底部"键 / 值"行仍显示之前的选中

  // 2) 逐段展开 + 找子节点。访问被拒 / 不存在时停在最后能到的层，让用户在
  //    树里直观看到"走到哪儿"。错误不走文本提示——三态灰化已经让规则一目了然。
  for (const auto& seg : segments) {
    loadChildren(current);  // 确保 current 下的真实子键已加载
    current->setExpanded(true);
    QTreeWidgetItem* hit = nullptr;
    for (int i = 0; i < current->childCount(); ++i) {
      auto* c = current->child(i);
      if (c->text(0).compare(seg, Qt::CaseInsensitive) == 0) {
        hit = c;
        break;
      }
    }
    if (!hit) break;
    current = hit;
  }
  m_tree->setCurrentItem(current);
  m_tree->scrollToItem(current);
}

void RegistryPickerDialog::refreshSelectionLabels() {
  const QString key = currentKeyPath();
  m_keyLabel->setText(I18n::tr("Key: %1").arg(key.isEmpty() ? I18n::tr("(none)") : key));

  // Exclusion 模式没有 value 行——值不参与结果，再展示一行只是噪音。
  if (!m_valueLabel) return;

  // CommitValue / DeleteValue Value 行展示规则：
  //   未选行（空白点击 / 初始）  → "(整键递归)"，表达 valueName="" 等同整键操作
  //   选了普通值                  → 真实值名
  // (Default) 行在 refreshValueTable 里就 NoItemFlags 禁用了，永远不会进入选中。
  QString valueText;
  if (m_valueTable && !m_valueTable->selectedItems().isEmpty()) {
    const int row = m_valueTable->selectedItems().front()->row();
    if (auto* nameItem = m_valueTable->item(row, 0)) {
      valueText = nameItem->data(Qt::UserRole).toString();
    }
  } else {
    valueText = I18n::tr("(whole key recursive, including subkeys)");
  }
  m_valueLabel->setText(I18n::tr("Value: %1").arg(valueText));
}

void RegistryPickerDialog::updateOkButton() {
  const QString key = currentKeyPath();
  if (key.isEmpty()) {
    m_okBtn->setEnabled(false);
    return;
  }
  if (m_mode == Mode::Exclusion) {
    // 只有 Selectable 才让 OK 亮——ContainerOnly / Pruned 节点本身就已经灰字 +
    // 禁选了，OK 跟着灰只是把"这条不让选"的反馈再补一层。
    m_okBtn->setEnabled(availabilityOf(key) == KeyAvailability::Selectable);
    return;
  }
  // CommitValue / DeleteValue：选了键就 OK——值表选不选都行（空 = 整键递归）。
  m_okBtn->setEnabled(true);
}

QString RegistryPickerDialog::currentKeyPath() const {
  const auto items = m_tree->selectedItems();
  if (items.isEmpty()) return {};
  return items.front()->data(0, kPathRole).toString();
}

QString RegistryPickerDialog::currentValueName() const {
  // Exclusion 模式下值表纯展示——选中行不参与结果。强制返回空，让 accept 的
  // Result 构造拿到 (key, "")，与 caller 协议一致。
  if (!supportsValueSelection() || !m_valueTable) return {};
  const auto items = m_valueTable->selectedItems();
  if (items.isEmpty()) return {};
  // 第 0 列 UserRole 存了真实值名——(Default) 已经在 refreshValueTable 里禁选，
  // 这里拿到的只会是非空命名值。
  const int row = items.front()->row();
  auto* nameItem = m_valueTable->item(row, 0);
  return nameItem ? nameItem->data(Qt::UserRole).toString() : QString{};
}

void RegistryPickerDialog::setAvailabilityChecker(AvailabilityChecker checker) {
  m_availability = std::move(checker);
  // checker 影响树节点 flags + 占位子节点——重建一次树，否则已经建好的节点
  // 仍按旧 checker（或全 Selectable 默认）来；按钮校验也顺带刷新。
  populateRootHives();
  updateOkButton();
}

void RegistryPickerDialog::preselectKey(const QString& key) {
  if (key.isEmpty()) return;
  m_addressBar->setText(key);
  navigateTo(key);
}

std::optional<RegistryPickerDialog::Result> RegistryPickerDialog::pick(Mode mode, const QString& title, QWidget* parent, AvailabilityChecker checker,
                                                                       const QString& preselectKey) {
  RegistryPickerDialog dlg(mode, title, parent);
  if (checker) dlg.setAvailabilityChecker(std::move(checker));
  if (!preselectKey.isEmpty()) dlg.preselectKey(preselectKey);
  if (dlg.exec() != QDialog::Accepted) return std::nullopt;
  return dlg.result();
}

}  // namespace uwf::ui
