#include "MainWindow.h"

#include <windows.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHeaderView>
#include <QHoverEvent>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QProgressDialog>
#include <QPushButton>
#include <QSet>
#include <QShortcut>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <chrono>
#include <format>
#include <memory>

#include "../util/DriveLetter.h"
#include "../util/Log.h"
#include "../uwf/UwfSnapshot.h"
#include "../uwf/api/UwfmgrCli.h"
#include "../uwf/wmi/WmiError.h"
#include "ApplyPlanDialog.h"
#include "DiskTab.h"
#include "GlobalStatusPanel.h"
#include "I18n.h"
#include "ImportDialog.h"
#include "LogViewerDialog.h"
#include "MessageDialog.h"
#include "TableText.h"
#include "ThemeManager.h"
#include "TrayController.h"
#include "uwf_version.h"

namespace uwf::ui {

namespace {

// 旧的 warnSelectable / confirmYesNo helper 已迁移到 ui::dialogs（QDialog 实现，
// 走 app font，避免 QMessageBox 的中文渲染糊问题）。
using uwf::ui::dialogs::confirm;
using uwf::ui::dialogs::information;
using uwf::ui::dialogs::warning;

// 批量 CommitFile 结果里单条文件的记录（非 Ok 才进这里）。
struct CommitReportRow {
  QString category;   // "跳过" / "失败"
  QString path;       // 完整文件路径
  QString errorCode;  // "0x80041001" 之类，或 "-" 表示无 HRESULT
  QString reason;     // 面向普通用户的中文解释
};

// 把 HRESULT / UWF returnValue 翻译成普通用户看得懂的一句话。
// 不在这里暴露 "WBEM_E_*" / "ExecMethod" 这些实现术语——那些进日志。
QString explainCommitFailure(int32_t hresult, uint32_t returnValue) {
  if (hresult != 0) {
    switch (uwf::WmiError(hresult).code()) {
      case uwf::WmiErrorCode::Failed:
        return I18n::tr("The file is in use by another process and cannot be saved right now.");
      case uwf::WmiErrorCode::NotFound:
        return I18n::tr("The file has no pending changes; nothing to save.");
      case uwf::WmiErrorCode::InvalidParameter:
        return I18n::tr("The path is invalid or improperly formatted.");
      default:
        return I18n::tr("System call failed (see log for details).");
    }
  }
  if (returnValue != 0) return I18n::tr("Operation rejected (code %1).").arg(returnValue);
  return I18n::tr("Unknown cause.");
}

QString formatErrorCode(int32_t hresult, uint32_t returnValue) {
  // %08X：大写 8 位定宽十六进制；显示 "0x" 小写前缀更符合 Win32 文档惯例。
  if (hresult != 0) return QString::asprintf("0x%08X", static_cast<uint32_t>(hresult));
  if (returnValue != 0) return QString("rv=%1").arg(returnValue);
  return "-";
}

// 结果对话框：四列表格（类别/路径/错误码/原因），可排序、可选中、
// Ctrl+C 复制选中区、右键菜单"复制选中/复制全部"、底部"复制全部"按钮。
// 当 rows 为空（全成功）时：不渲染表格和"复制全部"按钮，只保留标题+汇总+关闭。
// 用普通 QDialog 而不是 QMessageBox，避免 Windows 的提示音。
void showCommitReport(QWidget* parent, int okCount, const QList<CommitReportRow>& rows, int canceledRemaining = 0) {
  QDialog dlg(parent);
  dlg.setWindowTitle(canceledRemaining > 0 ? I18n::tr("Commit canceled") : I18n::tr("Commit result"));
  const bool hasRows = !rows.isEmpty();
  if (hasRows)
    dlg.resize(1100, 480);
  else
    dlg.resize(420, 140);
  auto* lay = new QVBoxLayout(&dlg);

  const QString skipLabel = I18n::tr("Skipped");
  int skipN = 0, failN = 0;
  for (const auto& r : rows) {
    if (r.category == skipLabel)
      ++skipN;
    else
      ++failN;
  }
  QString summaryText;
  if (hasRows) {
    summaryText = I18n::tr("%1 succeeded; %2 skipped; %3 failed.").arg(okCount).arg(skipN).arg(failN);
  } else {
    summaryText = I18n::tr("%1 files committed successfully.").arg(okCount);
  }
  if (canceledRemaining > 0) {
    summaryText += I18n::tr("\nCanceled by user; %1 entries not processed.").arg(canceledRemaining);
  }
  auto* summary = new QLabel(summaryText);
  summary->setTextInteractionFlags(Qt::TextSelectableByMouse);
  lay->addWidget(summary);

  QTableWidget* table = nullptr;
  if (hasRows) {
    table = new QTableWidget(static_cast<int>(rows.size()), 4, &dlg);
    table->setHorizontalHeaderLabels({I18n::tr("Category"), I18n::tr("Path"), I18n::tr("Error code"), I18n::tr("Reason")});
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setTextElideMode(Qt::ElideMiddle);
    auto* hh = table->horizontalHeader();
    hh->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    hh->setStretchLastSection(false);
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    for (int i = 0; i < rows.size(); ++i) {
      table->setItem(i, 0, new QTableWidgetItem(rows[i].category));
      auto* pathItem = new QTableWidgetItem(rows[i].path);
      pathItem->setToolTip(rows[i].path);
      table->setItem(i, 1, pathItem);
      table->setItem(i, 2, new QTableWidgetItem(rows[i].errorCode));
      auto* reasonItem = new QTableWidgetItem(rows[i].reason);
      reasonItem->setToolTip(rows[i].reason);
      table->setItem(i, 3, reasonItem);
    }
    table->setSortingEnabled(true);
    lay->addWidget(table, 1);

    // Ctrl+C 复制选中
    auto* copyShortcut = new QShortcut(QKeySequence::Copy, table);
    copyShortcut->setContext(Qt::WidgetShortcut);
    QObject::connect(copyShortcut, &QShortcut::activated, [table]() {
      const auto txt = tableSelectionToText(table);
      if (!txt.isEmpty()) QGuiApplication::clipboard()->setText(txt);
    });

    // 右键菜单
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(table, &QWidget::customContextMenuRequested, [table](const QPoint& pos) {
      QMenu menu;
      auto* copySel = menu.addAction(I18n::tr("Copy selected rows"));
      auto* copyAll = menu.addAction(I18n::tr("Copy all"));
      const bool hasSel = !table->selectedRanges().isEmpty();
      copySel->setEnabled(hasSel);
      QObject::connect(copySel, &QAction::triggered, [table]() { QGuiApplication::clipboard()->setText(tableSelectionToText(table)); });
      QObject::connect(copyAll, &QAction::triggered, [table]() { QGuiApplication::clipboard()->setText(tableAllToText(table)); });
      menu.exec(table->viewport()->mapToGlobal(pos));
    });
  }

  auto* btns = new QDialogButtonBox;
  if (table) {
    const auto* copyAllBtn = btns->addButton(I18n::tr("Copy all"), QDialogButtonBox::ActionRole);
    QObject::connect(copyAllBtn, &QPushButton::clicked, [table]() { QGuiApplication::clipboard()->setText(tableAllToText(table)); });
  }
  const auto* closeBtn = btns->addButton(I18n::tr("Close"), QDialogButtonBox::AcceptRole);
  QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
  lay->addWidget(btns);

  dlg.exec();
}

// CommitFile / CommitFileDeletion 操作的是"当前会话"正在使用的 overlay，
// 必须发给 CurrentSession=true 的实例，否则 WMI 直接拒掉。
const api::VolumeRow* findCurrentVolume(const std::vector<api::VolumeRow>& rows, const std::string& driveLetter) {
  for (const auto& r : rows) {
    if (r.currentSession && r.driveLetter == driveLetter) return &r;
  }
  return nullptr;
}

// CommitRegistry / CommitRegistryDeletion 与 UWF_Volume 的 CommitFile 同理，
// 必须发给 CurrentSession=true 的实例。
const api::RegistryFilterRow* findCurrentRegistryFilter(const std::vector<api::RegistryFilterRow>& rows) {
  for (const auto& r : rows) {
    if (r.currentSession) return &r;
  }
  return nullptr;
}

// 盘符逻辑统一在 uwf::drive（见 src/util/DriveLetter.h）。下面两个 MainWindow
// 内多处复用的函数只是 QString ↔ std::string 的边界适配，不含任何盘符逻辑。
// extractDriveLetter 把 fromPath 区分出的"卷 GUID 路径解析失败"写进日志——
// 调用方只关心拿没拿到盘符，但失败原因值得留痕。
QString extractDriveLetter(const QString& path) {
  std::string err;
  const std::string dl = drive::fromPath(path.toStdString(), &err);
  if (dl.empty() && !err.empty()) UWF_LOG_W("ui") << "extractDriveLetter: " << err;
  return QString::fromStdString(dl);
}
QString systemDriveLetter() { return QString::fromStdString(drive::systemLetter()); }

// 小工具：读一个字符串型的 REG_SZ / REG_EXPAND_SZ 注册表值。
QString readRegSz(HKEY root, const wchar_t* subkey, const wchar_t* name) {
  HKEY k = nullptr;
  if (RegOpenKeyExW(root, subkey, 0, KEY_READ | KEY_WOW64_64KEY, &k) != ERROR_SUCCESS) {
    return {};
  }
  wchar_t buf[256] = {};
  DWORD sz = sizeof(buf), type = 0;
  const LONG r = RegQueryValueExW(k, name, nullptr, &type, reinterpret_cast<LPBYTE>(buf), &sz);
  RegCloseKey(k);
  if (r != ERROR_SUCCESS) return {};
  return QString::fromWCharArray(buf).trimmed();
}

DWORD readRegDword(HKEY root, const wchar_t* subkey, const wchar_t* name) {
  HKEY k = nullptr;
  if (RegOpenKeyExW(root, subkey, 0, KEY_READ | KEY_WOW64_64KEY, &k) != ERROR_SUCCESS) {
    return 0;
  }
  DWORD val = 0, sz = sizeof(val), type = 0;
  RegQueryValueExW(k, name, nullptr, &type, reinterpret_cast<LPBYTE>(&val), &sz);
  RegCloseKey(k);
  return val;
}

// RtlGetVersion 是唯一一个在 Windows 8.1+ 上仍返回真实版本号（而不是被
// 应用兼容性"撒谎"成 Windows 8）的接口。动态加载避免对 ntdll 的直接 link。
// 版本型号则从 CurrentVersion\ProductName / EditionID 里取——Windows 11 的
// ProductName 字段至今仍写作 "Windows 10 Xxx"，所以家族名得靠 build 号自己判，
// 型号部分再从 ProductName 去掉前缀拿出来。LTSC 变体 ProductName 里不一定写
// "LTSC" 字样，需要再看 EditionID 末尾有没有 "S"。
QString windowsVersionText() {
  using Fn = LONG(WINAPI*)(OSVERSIONINFOW*);
  auto fn = reinterpret_cast<Fn>(reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion")));
  OSVERSIONINFOW v{};
  v.dwOSVersionInfoSize = sizeof(v);
  if (!fn || fn(&v) != 0) return QStringLiteral("Windows");

  constexpr const wchar_t* kCur = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
  const DWORD ubr = readRegDword(HKEY_LOCAL_MACHINE, kCur, L"UBR");
  const QString productName = readRegSz(HKEY_LOCAL_MACHINE, kCur, L"ProductName");
  const QString editionId = readRegSz(HKEY_LOCAL_MACHINE, kCur, L"EditionID");

  // 家族名（Windows 10 / 11 共享 Major=10，靠 build ≥ 22000 区分）。
  QString family = QStringLiteral("Windows");
  if (v.dwMajorVersion == 10) {
    family = v.dwBuildNumber >= 22000 ? QStringLiteral("Windows 11") : QStringLiteral("Windows 10");
  }

  // 从 ProductName 去掉"Windows 10/11 "前缀拿型号（Pro / Enterprise / Home /
  // ...）。
  QString edition = productName;
  for (const QString& p : {QStringLiteral("Windows 11 "), QStringLiteral("Windows 10 "), QStringLiteral("Windows ")}) {
    if (edition.startsWith(p, Qt::CaseInsensitive)) {
      edition = edition.mid(p.size()).trimmed();
      break;
    }
  }

  // LTSC / LTSB 变体：EditionID = EnterpriseS / EnterpriseSN / IoTEnterpriseS …
  // ProductName 并不总是把 "LTSC" 写出来，需要自己补上。
  const QString ed = editionId.toLower();
  const bool isLtsc = ed == "enterprises" || ed == "enterprisesn" || ed == "iotenterprises" || ed == "iotenterprisesn";
  if (isLtsc && !edition.contains("LTSC", Qt::CaseInsensitive) && !edition.contains("LTSB", Qt::CaseInsensitive)) {
    edition = edition.isEmpty() ? QStringLiteral("LTSC") : (edition + QStringLiteral(" LTSC"));
  }

  QString head = edition.isEmpty() ? family : (family + ' ' + edition);
  return QString("%1 · %2.%3.%4.%5").arg(head).arg(v.dwMajorVersion).arg(v.dwMinorVersion).arg(v.dwBuildNumber).arg(ubr);
}

QString cpuModelText() {
  HKEY k = nullptr;
  if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &k) != ERROR_SUCCESS) {
    return {};
  }
  wchar_t buf[256] = {};
  DWORD sz = sizeof(buf), type = 0;
  const LONG r = RegQueryValueExW(k, L"ProcessorNameString", nullptr, &type, reinterpret_cast<LPBYTE>(buf), &sz);
  RegCloseKey(k);
  if (r != ERROR_SUCCESS) return {};
  QString s = QString::fromWCharArray(buf);
  // BIOS 厂商常常在名字里塞大量尾随空格，去一下。
  return s.trimmed().simplified();
}

QString totalRamText() {
  MEMORYSTATUSEX m{};
  m.dwLength = sizeof(m);
  if (!GlobalMemoryStatusEx(&m)) return {};
  // 实际物理内存常常略少于 N GB（给 GPU、硬件保留）。用就近取整的 GB，
  // 失败再退回 MB。例：16777216 KB ≈ 16 GB。
  const auto gb = static_cast<uint64_t>((m.ullTotalPhys + (512ULL << 20)) / (1ULL << 30));
  if (gb >= 1) return QString("%1 GB").arg(gb);
  return QString("%1 MB").arg(m.ullTotalPhys / (1ULL << 20));
}

QString gpuModelText() {
  // EnumDisplayDevices 的 DeviceString 就是设备厂商/型号字符串。
  // 优先选"挂在桌面上"的主适配器，避免拿到 Remote / Mirror 这类假设备。
  DISPLAY_DEVICEW dev{};
  dev.cb = sizeof(dev);
  for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dev, 0); ++i) {
    if (dev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
      return QString::fromWCharArray(dev.DeviceString).trimmed();
    }
    dev = {};
    dev.cb = sizeof(dev);
  }
  dev = {};
  dev.cb = sizeof(dev);
  if (EnumDisplayDevicesW(nullptr, 0, &dev, 0)) {
    return QString::fromWCharArray(dev.DeviceString).trimmed();
  }
  return {};
}

// 紧凑排版：一行一个条目；系统/CPU/显卡本身名字就够识别（"Windows 11"、
// "Intel ..."、"NVIDIA ..."），不再加"系统："之类的 key 标签。
// 只有"XX GB"这种纯数字单位需要 RAM 前缀才知道是内存总量。
QString systemInfoHtml() {
  const QString ver = windowsVersionText();
  const QString cpu = cpuModelText();
  const QString ram = totalRamText();
  const QString gpu = gpuModelText();
  QStringList rows;
  auto addPlain = [&](const QString& v) {
    if (!v.isEmpty()) rows << v.toHtmlEscaped();
  };
  const QString muted = ThemeManager::instance().color(Sem::FgMuted).name();
  auto addWithKey = [&](const QString& k, const QString& v) {
    if (v.isEmpty()) return;
    rows << QString("<span style='color:%1'>%2</span>&nbsp;%3").arg(muted, k.toHtmlEscaped(), v.toHtmlEscaped());
  };
  addPlain(ver);
  addPlain(cpu);
  addWithKey("RAM", ram);
  addPlain(gpu);
  return rows.join("<br>");
}

// 去掉末尾的 \ 和 /；排除项可能写成 "C:\Foo\" 也可能写成 "C:\Foo"，
// 统一后才能做稳定的前缀/相等比较。
std::string stripTrailingSep(std::string s) {
  while (!s.empty() && (s.back() == '\\' || s.back() == '/')) s.pop_back();
  return s;
}

// 判断 target 是否等于 prefix 或者是它的后代。大小写不敏感，
// 分隔符兼容 \ 与 /（注册表固定是 \）。两个参数都应先经 stripTrailingSep。
bool pathIsExcludedBy(const std::string& target, const std::string& prefix) {
  if (prefix.empty() || target.size() < prefix.size()) return false;
  for (size_t i = 0; i < prefix.size(); ++i) {
    auto a = static_cast<unsigned char>(target[i]);
    auto b = static_cast<unsigned char>(prefix[i]);
    if (a >= 'A' && a <= 'Z') a = a - 'A' + 'a';
    if (b >= 'A' && b <= 'Z') b = b - 'A' + 'a';
    if (a != b) return false;
  }
  if (target.size() == prefix.size()) return true;
  const char next = target[prefix.size()];
  return next == '\\' || next == '/';
}

// 在 excls 里找第一条"覆盖"了 target 的排除项；找不到返回空串。
// 用原始字符串返回，便于在提示里原样展示给用户。
std::string findCoveringExclusion(const std::vector<std::string>& excls, const std::string& target) {
  const std::string t = stripTrailingSep(target);
  for (const auto& e : excls) {
    const std::string p = stripTrailingSep(e);
    if (pathIsExcludedBy(t, p)) return e;
  }
  return {};
}

}  // namespace

MainWindow::MainWindow(const QString& compatibilityNotice, QWidget* parent) : QMainWindow(parent), m_compatibilityNotice(compatibilityNotice) {
  // 构造期摆好窗口外壳（标题 / 图标 / 尺寸），并把窗口设为全透明：窗口会以
  // 透明状态 show 出来——showEvent 照常触发，首屏 rebuildUi() 在 shown 状态下
  // 建好全部内容、拉完数据后才把不透明度恢复成 1 一次性揭幕。整个 buildUi +
  // refresh 期间窗口不可见，用户不会看到空窗 / 改尺寸 / 白屏等中间态。
  setWindowTitle(I18n::tr("Unified Write Filter (UWF) Manager"));
  setWindowIcon(QIcon(":/icons/app.svg"));
  resize(1380, 760);
  setWindowOpacity(0.0);

  // 写会话提前连接一次；读快照时会另起一个独立会话。
  std::string err;
  m_writeSession.connect(api::kWmiNamespace, &err);
  // 内容控件与首屏数据统一交给 showEvent 调度的 rebuildUi()——它一次 buildUi()
  // + refresh() 建好。构造期不再 buildUi()/refresh()：那份产出会被 rebuildUi
  // 整个销毁重建，等于白建一遍 UI、白连一次 WMI、白读一份快照。

  // 系统托盘（图标 + 右键菜单）——独立组件，由本窗口编排：接它的"激活窗口"信号。
  m_tray = new TrayController(m_writeSession, this);
  connect(m_tray, &TrayController::activateWindowRequested, this, &MainWindow::raiseToFront);

  // 每 5s 周期刷新 Usage 数据（占用条）——只读 UWF_Overlay，不做整体 refresh。
  m_usageTimer = new QTimer(this);
  m_usageTimer->setInterval(5000);
  connect(m_usageTimer, &QTimer::timeout, this, &MainWindow::refreshUsage);
  m_usageTimer->start();
  // 首屏 rebuildUi 不在 ctor 里同步触发——widget 此时还没 show，Qt 一些 polish
  // / 几何计算在 widget 真正进入 shown 状态前结果不稳定，会跟后续"切主题 /
  // 切语言时已 shown 状态下的 rebuildUi"产生差异。改放到 showEvent 第一次
  // 触发后用 singleShot 调度，确保首次 rebuild 也在 shown 状态下跑。
}

void MainWindow::raiseToFront() {
  // 最小化时先恢复；否则确保可见。再 raise + activate 抢前台——配合启动方
  // 进程调用的 AllowSetForegroundWindow，能真正前置而非只闪任务栏。
  if (isMinimized())
    showNormal();
  else
    show();
  raise();
  activateWindow();
}

void MainWindow::refreshUsage() {
  // 周期更新只动 Usage 数据：主窗口可见时刷新主面板占用条；托盘那半段交给
  // TrayController（它内部判断右键菜单是否正在显示）。
  if (isVisible() && m_global) {
    if (const auto overlay = m_overlay.read()) {
      core::OverlayRuntime rt;
      rt.currentConsumptionMb = overlay->overlayConsumption;
      rt.availableSpaceMb = overlay->availableSpace;
      m_global->updateUsage(rt);
    }
  }
  if (m_tray) m_tray->refreshUsage();
}

void MainWindow::buildUi() {
  // 标题随语言切换重译，故每次 buildUi（含 rebuildUi 路径）都重设一次；
  // 图标与初始尺寸是一次性窗口外壳设置，已在构造函数里完成，这里不再重复。
  setWindowTitle(I18n::tr("Unified Write Filter (UWF) Manager"));

  // QSS 的 `padding` 在 QToolBar 上不可靠（rebuildUi 后 polish 时机问题），
  // 用 spacer widget 给水平方向兜底。垂直方向不再人为加 margin——
  // setContentsMargins 在 QToolBarLayout 上行为不一致（top margin 部分被
  // QToolBarLayout 默认的 AlignTop 吞掉，bottom 算进总高度但 items 不下沉，
  // 导致底部出现空白条）。直接让 toolbar 高度 = button 高度，靠 button 自己
  // 的 QSS padding (`padding: 7px 16px 9px 16px` + `min-height: 22px`) 提供
  // 内部 breathing room，toolbar 自然紧贴。
  auto* tb = new QToolBar(I18n::tr("Main toolbar"), this);
  tb->setObjectName("mainToolbar");
  tb->setMovable(false);
  tb->setIconSize({16, 16});
  tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  addToolBar(tb);

  // 左侧 padding：固定 12px 宽 spacer widget。Qt 要把 widget 加到 QToolBar
  // 必须走 addWidget——addAction 只接 QAction。
  {
    auto* leftPad = new QWidget(tb);
    leftPad->setFixedWidth(12);
    tb->addWidget(leftPad);
  }

  m_actImport = tb->addAction(I18n::tr("Import"));
  m_actImport->setToolTip(
      I18n::tr("Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you "
               "click \"Review and apply\"."));
  connect(m_actImport, &QAction::triggered, this, &MainWindow::showImport);

  m_actRefresh = tb->addAction(I18n::tr("Refresh"));
  m_actRefresh->setShortcut(QKeySequence::Refresh);
  m_actRefresh->setToolTip(I18n::tr("Re-read the current session state and next-session configuration of UWF."));
  connect(m_actRefresh, &QAction::triggered, this, &MainWindow::refresh);

  m_actPlan = tb->addAction(I18n::tr("Review and apply"));
  m_actPlan->setToolTip(I18n::tr("Review all pending changes and apply them in one batch. Most changes take effect after the next reboot."));
  connect(m_actPlan, &QAction::triggered, this, &MainWindow::showPlan);

  tb->addSeparator();

  m_actShutdown = tb->addAction(I18n::tr("Safe shutdown"));
  m_actShutdown->setToolTip(I18n::tr("Shut down safely, even when the UWF overlay is full."));
  connect(m_actShutdown, &QAction::triggered, this, [this]() { safeShutdown(); });

  m_actRestart = tb->addAction(I18n::tr("Safe restart"));
  m_actRestart->setToolTip(I18n::tr("Restart safely, even when the UWF overlay is full."));
  connect(m_actRestart, &QAction::triggered, this, [this]() { safeRestart(); });

  tb->addSeparator();

  m_actLog = tb->addAction(I18n::tr("Log"));
  m_actLog->setToolTip(I18n::tr("View the internal log accumulated during this session, for troubleshooting."));
  connect(m_actLog, &QAction::triggered, this, &MainWindow::showLogs);

  m_actAbout = tb->addAction(I18n::tr("About"));
  m_actAbout->setToolTip(I18n::tr("About this program."));
  connect(m_actAbout, &QAction::triggered, this, &MainWindow::showAbout);

  // QToolBar 没有原生"右对齐"分组——通过塞一个横向 Expanding 的 spacer
  // QWidget 把后续 action 推到最右。
  {
    auto* spacer = new QWidget(tb);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacer);
  }

  // 语言切换按钮（IconOnly，下拉菜单展示支持的语言）。
  m_actLang = tb->addAction("");
  m_actLang->setToolTip(I18n::tr("Switch display language"));
  if (auto* btn = qobject_cast<QToolButton*>(tb->widgetForAction(m_actLang))) {
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setPopupMode(QToolButton::InstantPopup);
    // 去掉 InstantPopup 默认的小下拉箭头——纯 icon 按钮更干净。
    btn->setStyleSheet("QToolButton::menu-indicator{image:none;width:0}");
    // 把 menu 的 parent 设成 btn——btn 是 toolbar 的子项，toolbar 在
    // rebuildUi 时被 deleteLater，menu 跟着一起释放，避免每次重建留下
    // 一个孤立 QMenu 挂在 MainWindow 下。
    auto* menu = new QMenu(btn);
    // QActionGroup 让两个语言项互斥单选；语言名一律用各自语言的本地写法
    // (English / 简体中文)，不参与翻译。
    auto* langGroup = new QActionGroup(menu);
    langGroup->setExclusive(true);

    // 切换语言后用 QTimer::singleShot(0, rebuildUi) 异步触发整体重建：
    // 不能在 action 的 triggered 回调里直接 rebuildUi——rebuildUi 会删掉
    // 当前 toolbar 及其上的 QAction，正在执行的回调随即变成悬空指针。
    // 单次延迟到事件循环下一轮再做，让回调先安全返回。
    auto switchTo = [this](const I18n::Lang target) {
      if (I18n::instance().lang() == target) return;
      // 抑制 paint 直到 rebuildUi 结束，避免用户看到 tear-down 中间空白
      // 一帧。配对的 setUpdatesEnabled(true) 在 rebuildUi 末尾。
      setUpdatesEnabled(false);
      I18n::instance().setLang(target);
      QTimer::singleShot(0, this, &MainWindow::rebuildUi);
    };

    auto* enAct = menu->addAction("English");
    enAct->setCheckable(true);
    enAct->setActionGroup(langGroup);
    enAct->setChecked(I18n::instance().lang() == I18n::Lang::En);
    connect(enAct, &QAction::triggered, this, [switchTo]() { switchTo(I18n::Lang::En); });

    auto* zhAct = menu->addAction("简体中文");
    zhAct->setCheckable(true);
    zhAct->setActionGroup(langGroup);
    zhAct->setChecked(I18n::instance().lang() == I18n::Lang::Zh_CN);
    connect(zhAct, &QAction::triggered, this, [switchTo]() { switchTo(I18n::Lang::Zh_CN); });

    btn->setMenu(menu);
  }

  m_actTheme = tb->addAction("");
  m_actTheme->setToolTip(I18n::tr("Toggle light / dark theme. Follows the system setting on startup."));
  // 在 toggle() 之前禁 paint：toggle 内部会 setPalette + setStyleSheet("") +
  // setStyleSheet(sheet)，中间 unstyled 那一帧会闪。先压 paint 再切，再让
  // 后续 themeChanged → singleShot → rebuildUi → setUpdatesEnabled(true)
  // 一次性把最终态画出来。
  connect(m_actTheme, &QAction::triggered, this, [this]() {
    setUpdatesEnabled(false);
    ThemeManager::instance().toggle();
  });
  // 主题按钮只显示 icon、不显示文字（与其他 toolbar action 不同），
  // 单独把它的 button style 改成 IconOnly。
  if (auto* btn = qobject_cast<QToolButton*>(tb->widgetForAction(m_actTheme))) {
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
  }

  // 右侧 padding：跟左边对称，固定 12px。放在最后让它真的贴最右——之前
  // 的 Expanding spacer 只把 lang/theme 推到右半部分，这个 padding 才是
  // 让最末按钮和窗口右边沿之间留 12px 空白。
  {
    auto* rightPad = new QWidget(tb);
    rightPad->setFixedWidth(12);
    tb->addWidget(rightPad);
  }

  // 主题切换走和语言切换完全相同的入口：rebuildUi 整体重建 toolbar + 中央
  // widget。两套刷新走同一条路径，避免之前"主题切换只刷 icon、语言切换全
  // 重建"两套机制各自的几何跳变。代价跟语言切换一致——会丢 widget 状态里
  // 的 pending changes，所以切主题前应已 apply 过待应用变更。
  // QTimer::singleShot(0, ...) 把 rebuild 推到下一轮事件循环，让发出
  // themeChanged 信号的 ThemeManager::apply 先安全返回。
  // setUpdatesEnabled(false) 已在主题按钮 click handler 里调过（在 toggle()
  // 之前），这里不再重复。配对的 setUpdatesEnabled(true) 在 rebuildUi 末尾。
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) { QTimer::singleShot(0, this, &MainWindow::rebuildUi); });

  auto* central = new QWidget(this);
  auto* centralLayout = new QVBoxLayout(central);
  centralLayout->setContentsMargins(0, 0, 0, 0);
  centralLayout->setSpacing(0);

  auto* mainRow = new QHBoxLayout();
  mainRow->setContentsMargins(0, 0, 0, 0);
  mainRow->setSpacing(0);

  m_tabs = new QTabWidget(this);
  m_tabs->setObjectName("mainTabs");
  m_tabs->setDocumentMode(true);
  m_tabs->setTabPosition(QTabWidget::North);
  m_tabs->setMinimumWidth(220);
  m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  mainRow->addWidget(m_tabs, 1);

  auto* globalWrap = new QWidget(this);
  // 纯 QWidget 默认不会应用 QSS 的 background；必须打开 WA_StyledBackground，
  // 否则 QWidget#globalWrap
  // 规则里的背景色根本不生效，看上去就和主窗口底色有差。
  globalWrap->setAttribute(Qt::WA_StyledBackground, true);
  auto* globalLayout = new QVBoxLayout(globalWrap);
  globalLayout->setContentsMargins(14, 12, 14, 12);
  globalLayout->setSpacing(10);
  m_global = new GlobalStatusPanel(this);
  // 系统版本未通过校验时，把兼容模式提示常驻在面板信息框里。rebuildUi 会
  // 重建 m_global，所以每次 buildUi 都要重新灌一次。
  if (!m_compatibilityNotice.isEmpty()) m_global->setCompatibilityNotice(m_compatibilityNotice);
  // 顶部全局设置拿走所有可拉伸空间，里面的 QScrollArea 会在高度不足时
  // 自己滚动；tips 区用固定高度贴底。
  globalLayout->addWidget(m_global, 1);
  m_hoverHint = new QLabel(this);
  m_hoverHint->setObjectName("hoverHintBox");
  m_hoverHint->setWordWrap(true);
  m_hoverHint->setTextInteractionFlags(Qt::NoTextInteraction);
  m_hoverHint->setFixedHeight(110);
  m_hoverHint->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_hoverHint->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  // 默认文案换成机器基本信息（OS / CPU / RAM / GPU），悬停事件会临时覆盖。
  // 留 AutoText：默认文案里有 HTML 标签会按 RichText 渲染；
  // 普通 tooltip 是纯文本则按 PlainText 渲染，不怕里面的 & / < 被解析走样。
  m_hoverHintDefault = systemInfoHtml();
  m_hoverHint->setText(m_hoverHintDefault);
  globalLayout->addWidget(m_hoverHint, 0);
  globalWrap->setObjectName("globalWrap");
  globalWrap->setFixedWidth(420);
  // 右侧面板整体最小高度：GlobalStatusPanel 自身需要约 360 来容纳筛选器
  // 和覆盖层两张卡片；加上 tips 固定 110 + 内边距。这样即使把主窗口拖到
  // 很矮，这两块也都能完整看见。
  globalWrap->setMinimumHeight(360 + 110 + 24);
  mainRow->addWidget(globalWrap, 0);

  // 让 MainWindow 也跟随到这个最小高度（加上工具栏和状态栏的大致高度）。
  setMinimumHeight(globalWrap->minimumHeight() + 80);

  qApp->installEventFilter(this);

  centralLayout->addLayout(mainRow, 1);
  setCentralWidget(central);

  connect(m_global, &GlobalStatusPanel::pendingChanged, this, &MainWindow::updatePendingSummary);

  m_statusText = new QLabel(this);
  m_statusText->setObjectName("statusBarLabel");
  statusBar()->addPermanentWidget(m_statusText, 1);

  // 临时提示通过一个 singleShot QTimer 覆盖 m_statusText，
  // 到期后恢复 baseline（来自 refresh / updatePendingSummary 的常驻文案）。
  // 必须走这条路径，因为 statusBar()->showMessage() 被 stretch=1 的
  // permanent widget 挤没空间，显示不出来。
  m_hintTimer = new QTimer(this);
  m_hintTimer->setSingleShot(true);
  connect(m_hintTimer, &QTimer::timeout, this, [this]() { m_statusText->setText(m_statusBaseline); });

  // 鼠标离开控件后的"短延迟回到默认提示"定时器 —— 延迟是为了避免光标在
  // 相邻控件之间移动时文字先变空再变回去造成的闪烁。
  m_hoverClearTimer = new QTimer(this);
  m_hoverClearTimer->setSingleShot(true);
  m_hoverClearTimer->setInterval(120);
  connect(m_hoverClearTimer, &QTimer::timeout, this, [this]() {
    if (m_hoverHint) m_hoverHint->setText(m_hoverHintDefault);
  });

  // buildUi 末尾统一应用一次主题：toolbar 图标、disk tab 图标、hover hint
  // 默认文案都按当前主题色生成（构造期间 connect 时 m_hoverHint 还是 null，
  // 这里补一次初始化）。
  refreshThemedUi();
}

void MainWindow::rebuildUi() {
  // 切换语言 / 切换主题统一入口：把 buildUi 创建的 toolbar / central widget /
  // 状态栏 permanent widget / 两个 timer 全部清掉，重置成员指针，再
  // buildUi + refresh 重新拉数据。WMI 会话和快照保留——它们和 UI 主题 / 语言
  // 都无关。
  //
  // pending changes 会随 m_global / m_diskTabs 一起销毁；切换主题 / 语言都是
  // 低频操作，不值得为了保留 pending state 而维护两套刷新机制。

  // ThemeManager 是单例，buildUi 每次都新增一个 themeChanged 连接，重建前
  // 必须先把上一轮的连接拆掉，否则刷主题时回调会被多次触发。
  disconnect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, nullptr);

  if (auto* tb = findChild<QToolBar*>("mainToolbar")) {
    removeToolBar(tb);
    // 同步 delete 而不是 deleteLater——否则旧 toolbar 还挂在 MainWindow 子
    // 对象列表里到下一个事件循环 tick 才回收，期间如果 buildUi 又通过
    // findChild<QToolBar*>("mainToolbar") 找一次，会拿到旧实例。我们走的是
    // QTimer::singleShot(0) 路径调进来，已经脱离 action 触发的回调，同步
    // delete 安全。
    delete tb;
  }
  if (auto* central = takeCentralWidget()) {
    central->deleteLater();
  }
  if (m_statusText) {
    statusBar()->removeWidget(m_statusText);
    m_statusText->deleteLater();
  }
  if (m_hintTimer) m_hintTimer->deleteLater();
  if (m_hoverClearTimer) m_hoverClearTimer->deleteLater();

  // 重置所有指针成员；buildUi 会重新填充。
  m_actRefresh = m_actImport = m_actPlan = m_actShutdown = m_actRestart = nullptr;
  m_actLog = m_actAbout = m_actLang = m_actTheme = nullptr;
  m_tabs = nullptr;
  m_global = nullptr;
  m_hoverHint = nullptr;
  m_statusText = nullptr;
  m_hintTimer = m_hoverClearTimer = nullptr;
  m_diskTabs.clear();
  m_statusBaseline.clear();
  m_hoverHintDefault.clear();

  buildUi();
  refresh();

  // 配对的 setUpdatesEnabled(true)。切主题 / 切语言两条入口在调度 rebuildUi
  // 前会先 setUpdatesEnabled(false)，把中间过渡态（unstyled / 空白 / 重新
  // layout）的 paint 攒起来不画，这里画完最终态再统一放出来——只有一帧旧到
  // 新的硬切。首屏 showEvent 不压帧（本就 enabled），这里是 no-op，安全。
  setUpdatesEnabled(true);

  // 揭幕前先把挂起的布局请求与绘制同步跑完——确保恢复不透明的那一刻窗口
  // 已是完整绘制好的最终形态，不会闪过一帧尚未绘制完的空窗。
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  repaint();

  // 揭幕：首屏时窗口自构造起就是全透明的（opacity 0），此刻内容 / 数据 / 绘制
  // 均已就位，恢复成不透明 —— 窗口一次性以完整形态出现，没有任何中间态。
  // 切主题 / 切语言时窗口本就不透明，这里是 no-op。
  setWindowOpacity(1.0);
}

void MainWindow::showTransientHint(const QString& text, const int msec) const {
  m_statusText->setText(text);
  m_hintTimer->start(msec);
}

void MainWindow::showEvent(QShowEvent* ev) {
  QMainWindow::showEvent(ev);
  if (!m_firstShowDone) {
    m_firstShowDone = true;
    // 首次 show（此时窗口是全透明的）后立刻调度一次 rebuildUi——和"切主题 /
    // 切语言"走完全相同的重建路径，首屏最终形态在 shown 状态下 polish。
    // rebuildUi 建完内容、拉完数据后会把不透明度恢复成 1 揭幕。
    QTimer::singleShot(0, this, &MainWindow::rebuildUi);
  }
}

void MainWindow::refreshThemedUi() {
  auto& tm = ThemeManager::instance();
  if (m_actImport) m_actImport->setIcon(tm.icon(":/icons/add.svg"));
  if (m_actRefresh) m_actRefresh->setIcon(tm.icon(":/icons/refresh.svg"));
  if (m_actPlan) m_actPlan->setIcon(tm.icon(":/icons/apply.svg"));
  if (m_actShutdown) m_actShutdown->setIcon(tm.icon(":/icons/shutdown.svg"));
  if (m_actRestart) m_actRestart->setIcon(tm.icon(":/icons/restart.svg"));
  if (m_actLog) m_actLog->setIcon(tm.icon(":/icons/log.svg"));
  if (m_actAbout) m_actAbout->setIcon(tm.icon(":/icons/info.svg"));
  if (m_actLang) m_actLang->setIcon(tm.icon(":/icons/language.svg"));
  if (m_actTheme) {
    // 当前 dark → 显示太阳图标（点了切到 light）；当前 light → 显示月亮。
    const bool isDark = tm.current() == Theme::Dark;
    m_actTheme->setIcon(tm.icon(isDark ? ":/icons/theme_sun.svg" : ":/icons/theme_moon.svg"));
  }
  // 顺手刷一遍磁盘 TAB 上的 icon —— DiskTab 自己会处理内部的 commit / TAB icon。
  if (m_tabs) {
    const QString sysDl = systemDriveLetter();
    for (auto& t : m_diskTabs) {
      if (!t) continue;
      const int idx = m_tabs->indexOf(t);
      if (idx < 0) continue;
      const bool ok = t->supported();
      const bool isSys = t->driveLetter().toUpper() == sysDl;
      const QIcon ic = !ok ? tm.icon(":/icons/disk_off.svg") : isSys ? tm.icon(":/icons/disk_system.svg") : tm.icon(":/icons/disk.svg");
      m_tabs->setTabIcon(idx, ic);
    }
  }
  // hoverHint 默认 HTML 含 inline color，主题切换后重新生成。
  m_hoverHintDefault = systemInfoHtml();
  if (m_hoverHint && (!m_hintTimer || !m_hintTimer->isActive())) {
    m_hoverHint->setText(m_hoverHintDefault);
  }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev) {
  // hover 到任意带 hoverHint 属性（或 toolTip）的控件，就把说明塞到右侧面板的
  // 提示框里；离开时清空。走 qApp 级事件过滤器才能捕获所有子控件。
  if (!m_hoverHint) return QMainWindow::eventFilter(obj, ev);
  const auto type = ev->type();
  // 屏蔽原生 tooltip 气泡：截停 ToolTip 事件，说明文字只在右下角面板里。
  if (type == QEvent::ToolTip) return true;
  if (type == QEvent::Enter || type == QEvent::HoverEnter || type == QEvent::MouseMove || type == QEvent::HoverMove) {
    auto* w = qobject_cast<QWidget*>(obj);
    if (!w) return QMainWindow::eventFilter(obj, ev);
    // QTabBar 需要按坐标查出当前悬停在哪个 tab 上，再取对应的 tooltip。
    if (auto* bar = qobject_cast<QTabBar*>(w)) {
      QPoint pos;
      if (auto* me = dynamic_cast<QMouseEvent*>(ev))
        pos = me->pos();
      else if (auto* he = dynamic_cast<QHoverEvent*>(ev))
        pos = he->position().toPoint();
      else
        pos = bar->mapFromGlobal(QCursor::pos());
      const int idx = bar->tabAt(pos);
      if (idx >= 0) {
        const QString tip = bar->tabToolTip(idx);
        if (!tip.isEmpty()) {
          if (m_hoverClearTimer) m_hoverClearTimer->stop();
          m_hoverHint->setText(tip);
          return QMainWindow::eventFilter(obj, ev);
        }
      }
    }
    QWidget* cur = w;
    while (cur && cur->toolTip().isEmpty() && cur != this) cur = cur->parentWidget();
    if (cur && !cur->toolTip().isEmpty()) {
      if (m_hoverClearTimer) m_hoverClearTimer->stop();
      m_hoverHint->setText(cur->toolTip());
    }
  } else if (type == QEvent::Leave || type == QEvent::HoverLeave) {
    // 离开时延迟恢复默认说明；如果马上移到另一个带 tooltip 的控件上，
    // 下一次 Enter 会 stop() 这个定时器，避免文字反复闪一下。
    if (m_hoverClearTimer) m_hoverClearTimer->start();
  }
  return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::updatePendingSummary() {
  qsizetype pending = 0;
  if (m_global->pendingFilterEnabled()) ++pending;
  {
    const auto [type, maximumSizeMb, warningThresholdMb, criticalThresholdMb] = m_global->pendingOverlay();
    if (type) ++pending;
    if (maximumSizeMb) ++pending;
    if (warningThresholdMb) ++pending;
    if (criticalThresholdMb) ++pending;
  }
  for (const auto& t : m_diskTabs) {
    if (!t) continue;
    if (t->pendingVolumeProtected()) ++pending;
    if (t->pendingBindByVolumeName()) ++pending;
    pending += t->pendingFileAdded().size() + t->pendingFileRemoved().size() + t->pendingRegAdded().size() + t->pendingRegRemoved().size();
  }
  const QString msg = pending > 0 ? I18n::tr("%1 pending change(s) (not yet written to the system)").arg(pending) : I18n::tr("No pending changes");
  m_statusBaseline = msg;
  if (!m_hintTimer->isActive()) m_statusText->setText(m_statusBaseline);
}

void MainWindow::rebuildTabs(const std::vector<core::DiskInfo>& disks) {
  // QTabWidget::clear() 只摘掉标签页、不销毁页面控件——上一轮的 DiskTab 会继续
  // 作为 m_tabs 的子对象存活，每次 refresh 泄漏一组（要到下次 rebuildUi 删掉
  // m_tabs 才被连带回收）。这里先显式 deleteLater 回收旧的一组。用 deleteLater
  // 而非 delete：rebuildTabs 可能经由某个 DiskTab 自己的信号回调间接调进来，
  // 同步 delete 会销毁正在执行回调的对象。
  for (const auto& t : m_diskTabs)
    if (t) t->deleteLater();
  m_tabs->clear();
  m_diskTabs.clear();
  const QString sysDl = systemDriveLetter();
  for (const auto& d : disks) {
    auto* tab = new DiskTab(d, this);
    const QString label = QString::fromStdString(d.driveLetter);
    const bool ok = d.support == core::DiskSupport::Supported;
    const bool isSys = QString::fromStdString(d.driveLetter).toUpper() == sysDl;
    auto& tm = ThemeManager::instance();
    const QIcon icon = !ok ? tm.icon(":/icons/disk_off.svg") : isSys ? tm.icon(":/icons/disk_system.svg") : tm.icon(":/icons/disk.svg");
    const int idx = m_tabs->addTab(tab, icon, label);
    if (!ok) {
      const std::string reason = diskSupportText(d.support, d.fileSystem);
      m_tabs->setTabToolTip(idx, QString::fromStdString(reason));
    } else {
      const QString sysExtra = isSys ? I18n::tr(" (System drive: also manages the global registry exclusion list here.)") : QString();
      m_tabs->setTabToolTip(idx, I18n::tr("Switch to protection settings and file exclusions for volume %1.%2").arg(label, sysExtra));
    }
    m_diskTabs.push_back(tab);
    connect(tab, &DiskTab::pendingChanged, this, &MainWindow::updatePendingSummary);
    connect(tab, &DiskTab::statusHint, this, &MainWindow::showTransientHint);
    connect(tab, &DiskTab::commitFileRequested, this, &MainWindow::commitFilePath);
    connect(tab, &DiskTab::commitFileDeletionRequested, this, &MainWindow::commitFileDeletionPath);
    connect(tab, &DiskTab::commitRegistryRequested, this, &MainWindow::commitRegistryKey);
  }
}

void MainWindow::refresh() {
  UWF_LOG_I("ui") << "refresh start";
  const auto t0 = std::chrono::steady_clock::now();
  std::string err;
  auto disks = uwf::enumerateDisks(&err);
  if (!err.empty()) {
    UWF_LOG_W("ui") << "enumerateDisks error: " << err;
    warning(this, I18n::tr("Failed to read volume information"), QString::fromStdString(err));
  }
  m_snapshot = uwf::readSnapshot(&err);
  if (!m_snapshot.uwfAvailable) {
    // 不弹模态框——GlobalStatusPanel::setUnavailable 的横幅已常驻展示这条
    // 错误，模态框只是重复打扰（且每点一次刷新就再弹一次）。
    UWF_LOG_E("ui") << "readSnapshot failed: uwfAvailable=false err=" << err;
  }

  // uwfAvailable（命名空间可读）与 elevated（进程已提权）外观相近但用途不同，
  // 各处按需分别判断，不合并成单一标志。
  const bool uwfAvailable = m_snapshot.uwfAvailable;
  const bool elevated = m_snapshot.elevated;

  // 工具栏：UWF 不可用时除"日志 / 关于 / 主题 / 语言"外整体禁用。其中"刷新"
  // 只是重新读取、不写入 UWF，故未提权也允许点——只要 UWF 可读就放开；
  // 导入 / 预览并应用 / 安全关机 / 安全重启会写入，需同时已提权。
  if (m_actRefresh) m_actRefresh->setEnabled(uwfAvailable);
  for (QAction* a : {m_actImport, m_actPlan, m_actShutdown, m_actRestart})
    if (a) a->setEnabled(uwfAvailable && elevated);

  // Usage 定时器只跟随 UWF 可读性——未提权不影响读取占用，故不停表；
  // 仅在 UWF 不可用时停掉，避免每 5s 一次徒劳的 UWF_Overlay 读取。
  if (m_usageTimer) {
    if (uwfAvailable)
      m_usageTimer->start();
    else
      m_usageTimer->stop();
  }

  rebuildTabs(disks);
  if (uwfAvailable) {
    m_global->setData(m_snapshot.current, m_snapshot.next, m_snapshot.runtime);
    // UWF 可读但未提权：补一条红色"需要管理员权限"横幅。UWF 不可用时不补——
    // 那条不可用横幅优先级更高，已由下面的 setUnavailable 占据同一横幅。
    if (!elevated) m_global->showElevationRequired();
  } else {
    m_global->setUnavailable(err.empty() ? I18n::tr("UWF namespace is not available") : QString::fromStdString(err));
  }
  // setData 会把滚动区控件全部恢复 enabled——未提权时随即再统一置灰一次。
  m_global->setControlsEnabled(uwfAvailable && elevated);
  for (auto& t : m_diskTabs)
    if (t) t->applySnapshot(m_snapshot);
  m_statusBaseline = I18n::tr("Refreshed · %1 volumes").arg(disks.size());
  if (!m_hintTimer->isActive()) m_statusText->setText(m_statusBaseline);
  const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
  UWF_LOG_I("ui") << std::format("refresh done: disks={} uwfAvailable={} currentVolumes={} nextVolumes={} elapsedMs={}", disks.size(), m_snapshot.uwfAvailable,
                                 m_snapshot.current.volumes.size(), m_snapshot.next.volumes.size(), elapsedMs);
}

void MainWindow::showPlan() {
  // 收集待应用变更、渲染命令预览、二次确认后写入 WMI 都在 ApplyPlanDialog
  // 里完成；这里只负责把变更来源（GlobalStatusPanel + 各 DiskTab）和写会话
  // 交给它。applied() 用 QueuedConnection 接：等对话框这一轮事件循环回落
  // 再 refresh，避免在回调里递归进 refresh 的弹窗 / WMI 读。
  ApplyPlanDialog dlg(m_global, m_diskTabs, m_snapshot, m_writeSession, this);
  connect(&dlg, &ApplyPlanDialog::applied, this, &MainWindow::refresh, Qt::QueuedConnection);
  dlg.exec();
}

void MainWindow::showImport() {
  ImportDialog dlg(this);
  // applier：把每条 UwfmgrCommand 转成对应的 UI 操作（驱动 m_global 或对应
  // 盘符的 DiskTab）。报告里的"重复"分两种：
  //   1) within-batch：同一次 Import 文本里出现两条等价命令，第二条标 Duplicate；
  //   2) state no-op：命令应用后控件值未变（比如 filter 已经处于目标 enable
  //      状态）；importXxx 返回 false 时归到这里。
  // 都归并到 ImportReportRow::Status::Duplicate。
  dlg.setApplier([this](const QList<api::UwfmgrCommand>& cmds) -> QList<ImportReportRow> {
    QList<ImportReportRow> out;
    out.reserve(cmds.size());

    // within-batch 去重的 canonical key：kind + 大小写无关化的 arg0。
    QSet<QString> seen;
    auto canon = [](const api::UwfmgrCommand& c) {
      const QString a0 = c.args.empty() ? QString{} : QString::fromStdString(c.args[0]).toLower();
      return QString::number(static_cast<int>(c.kind)) + QChar('|') + a0;
    };

    auto findTab = [this](const QString& dl) -> DiskTab* {
      for (auto& t : m_diskTabs) {
        if (t && t->driveLetter().compare(dl, Qt::CaseInsensitive) == 0) return t.data();
      }
      return nullptr;
    };

    auto outcomeToRow = [](const api::UwfmgrCommand& c, ExclusionListWidget::ImportOutcome o, const QString& kindLabel) {
      ImportReportRow r;
      r.lineNo = c.sourceLineNo;
      r.lineText = QString::fromStdString(c.rawLine).trimmed();
      switch (o) {
        case ExclusionListWidget::ImportOutcome::Applied:
          r.status = ImportReportRow::Status::Success;
          r.detail = I18n::tr("Queued as a pending %1 change").arg(kindLabel);
          break;
        case ExclusionListWidget::ImportOutcome::NoOp:
          r.status = ImportReportRow::Status::Duplicate;
          r.detail = I18n::tr("Already in the target state — no-op");
          break;
        case ExclusionListWidget::ImportOutcome::RejectedNotOnVolume:
          r.status = ImportReportRow::Status::Failed;
          r.detail = I18n::tr("Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)");
          break;
        case ExclusionListWidget::ImportOutcome::RejectedForbidden:
          r.status = ImportReportRow::Status::Failed;
          r.detail = I18n::tr("Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)");
          break;
      }
      return r;
    };

    for (const auto& c : cmds) {
      ImportReportRow r;
      r.lineNo = c.sourceLineNo;
      r.lineText = QString::fromStdString(c.rawLine).trimmed();

      // 解析阶段失败的命令直接打包：
      // - Unsupported = 整段没识别 → Status::Unsupported；
      // - 其它非 None / Comment = cat/verb 已认出但参数非法 → Status::Failed。
      // parseErrorMessage 把 enum 翻译成中文（来自 ImportDialog.cpp 的 helper）。
      if (c.parseError != api::ParseError::None && c.parseError != api::ParseError::Comment) {
        r.status = c.parseError == api::ParseError::Unsupported ? ImportReportRow::Status::Unsupported : ImportReportRow::Status::Failed;
        r.detail = parseErrorMessage(c.parseError, QString::fromStdString(c.parseErrorContext));
        out.append(r);
        continue;
      }

      // within-batch dedup：第二条等价命令标 Duplicate，跳过 apply。
      const QString key = canon(c);
      if (seen.contains(key)) {
        r.status = ImportReportRow::Status::Duplicate;
        r.detail = I18n::tr("Same command was already issued earlier in this batch");
        out.append(r);
        continue;
      }
      seen.insert(key);

      // 把 args[0] 提到 QString 一次，下面分支统一用。args[0] 永远存在，因为
      // parser 只有在 args 完整时才把 parseError 设回 None；上面的 parseError 检查
      // 已经把缺参数的全部过滤掉了。
      const QString a0 = c.args.empty() ? QString{} : QString::fromStdString(c.args[0]);

      switch (c.kind) {
        case api::UwfmgrKind::FilterEnable:
        case api::UwfmgrKind::FilterDisable: {
          const bool want = c.kind == api::UwfmgrKind::FilterEnable;
          const bool changed = m_global ? m_global->importFilterEnabled(want) : false;
          r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
          r.detail =
              changed ? I18n::tr("Pending filter %1").arg(want ? I18n::tr("enable") : I18n::tr("disable")) : I18n::tr("Filter is already in the target state");
          break;
        }
        case api::UwfmgrKind::OverlaySetType: {
          const auto t = a0 == QStringLiteral("Disk") ? core::OverlayType::Disk : core::OverlayType::RAM;
          const bool changed = m_global ? m_global->importOverlayType(t) : false;
          r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
          r.detail = changed ? I18n::tr("Pending overlay type → %1").arg(a0) : I18n::tr("Overlay type already %1").arg(a0);
          break;
        }
        case api::UwfmgrKind::OverlaySetSize:
        case api::UwfmgrKind::OverlaySetWarningThreshold:
        case api::UwfmgrKind::OverlaySetCriticalThreshold: {
          bool ok = false;
          const auto mb = a0.toUInt(&ok);
          if (!ok) {
            r.status = ImportReportRow::Status::Failed;
            r.detail = I18n::tr("Invalid size value: %1").arg(a0);
            break;
          }
          bool changed = false;
          QString label;
          if (c.kind == api::UwfmgrKind::OverlaySetSize) {
            label = I18n::tr("maximum size");
            changed = m_global ? m_global->importOverlayMaxMb(mb) : false;
          } else if (c.kind == api::UwfmgrKind::OverlaySetWarningThreshold) {
            label = I18n::tr("warning threshold");
            changed = m_global ? m_global->importOverlayWarnMb(mb) : false;
          } else {
            label = I18n::tr("critical threshold");
            changed = m_global ? m_global->importOverlayCritMb(mb) : false;
          }
          r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
          r.detail = changed ? I18n::tr("Pending overlay %1 → %2 MB").arg(label).arg(mb) : I18n::tr("Overlay %1 already %2 MB").arg(label).arg(mb);
          break;
        }
        case api::UwfmgrKind::VolumeProtect:
        case api::UwfmgrKind::VolumeUnprotect: {
          auto* tab = findTab(a0);
          if (!tab) {
            r.status = ImportReportRow::Status::Failed;
            r.detail = I18n::tr("Unknown volume %1 (no UWF-eligible disk with that drive letter)").arg(a0);
            break;
          }
          const bool want = c.kind == api::UwfmgrKind::VolumeProtect;
          const bool changed = tab->importProtect(want);
          r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
          r.detail = changed ? I18n::tr("Pending volume %1 protection %2").arg(a0, want ? I18n::tr("enable") : I18n::tr("disable"))
                             : I18n::tr("Volume %1 is already in the target protection state").arg(a0);
          break;
        }
        case api::UwfmgrKind::FileAddExclusion:
        case api::UwfmgrKind::FileRemoveExclusion: {
          const QString native = QDir::toNativeSeparators(a0);
          // 路径需要 "<盘符>:" 前缀来路由到对应 DiskTab；缺前缀 → 没办法定位。
          const QString dl = extractDriveLetter(native);
          if (dl.isEmpty()) {
            r.status = ImportReportRow::Status::Failed;
            r.detail = I18n::tr("Path %1 has no drive letter; cannot route to a volume tab").arg(native);
            break;
          }
          auto* tab = findTab(dl);
          if (!tab) {
            r.status = ImportReportRow::Status::Failed;
            r.detail = I18n::tr("No UWF-eligible disk for drive letter %1").arg(dl);
            break;
          }
          const auto outcome = c.kind == api::UwfmgrKind::FileAddExclusion ? tab->importAddFileExclusion(native) : tab->importRemoveFileExclusion(native);
          r = outcomeToRow(c, outcome, I18n::tr("file exclusion"));
          break;
        }
        case api::UwfmgrKind::RegistryAddExclusion:
        case api::UwfmgrKind::RegistryRemoveExclusion: {
          // 注册表排除是全局的，只挂在系统盘 TAB 上。其它 TAB 的 import* 在
          // m_regs == null 时直接返回 RejectedNotOnVolume，所以这里依次尝试
          // 每个 TAB——第一个非 RejectedNotOnVolume 的结果即视作系统盘 TAB
          // 的处理结果。所有 TAB 都拒说明压根没有系统盘 TAB。
          bool dispatched = false;
          for (auto& t : m_diskTabs) {
            if (!t) continue;
            const auto outcome = c.kind == api::UwfmgrKind::RegistryAddExclusion ? t->importAddRegistryExclusion(a0) : t->importRemoveRegistryExclusion(a0);
            if (outcome != ExclusionListWidget::ImportOutcome::RejectedNotOnVolume) {
              r = outcomeToRow(c, outcome, I18n::tr("registry exclusion"));
              dispatched = true;
              break;
            }
          }
          if (!dispatched) {
            r.status = ImportReportRow::Status::Failed;
            r.detail = I18n::tr("Registry exclusions are only available on the system drive tab, which is not present");
          }
          break;
        }
        case api::UwfmgrKind::Unknown:
          // 解析阶段 Unknown 已在前面分支处理了；落到这里说明 parseError
          // 是 None 而 kind 又是 Unknown，理论上不会发生，安全兜底。
          r.status = ImportReportRow::Status::Unsupported;
          r.detail = I18n::tr("Unsupported command");
          break;
      }
      out.append(r);
    }
    return out;
  });

  dlg.exec();
}

void MainWindow::showAbout() {
  // 改用普通 QDialog 而非 QMessageBox：QMessageBox 内部 label 走另一条
  // 字体路径，全局 app.setFont() 设置的 hinting / styleStrategy 不会传播过去，
  // 中文渲染会"糊"。QDialog + QLabel 跟其它对话框一样能继承 app font。
  QDialog dlg(this);
  dlg.setWindowTitle(I18n::tr("About UWF Manager"));
  dlg.setMinimumWidth(520);

  auto* layout = new QVBoxLayout(&dlg);
  layout->setContentsMargins(20, 16, 20, 12);
  layout->setSpacing(10);

  // 标题：手动用 QLabel + 大字号 + bold（YaHei 真实字重 700）替代 <h3>，
  // 避免 QTextDocument 的 <h3> 默认合成粗体（同样的 hinting 问题）。
  auto* title = new QLabel(I18n::tr("Unified Write Filter (UWF) Manager"), &dlg);
  QFont titleFont = title->font();
  titleFont.setBold(true);
  titleFont.setPointSizeF(titleFont.pointSizeF() + 3);
  title->setFont(titleFont);
  title->setTextInteractionFlags(Qt::TextSelectableByMouse);
  layout->addWidget(title);

  // 版本号紧贴标题下方、弱化显示。UWF_VER_STRING 由 cmake/GitVersion.cmake
  // 在构建期注入 git 短哈希（无 git 仓库时回退为 "1.0.0.0"）。用内联 color
  // 走富文本，理由同 body：避开 QSS / palette 对 QLabel 文字色的干扰。
  auto* version = new QLabel(&dlg);
  version->setTextFormat(Qt::RichText);
  version->setTextInteractionFlags(Qt::TextSelectableByMouse);
  version->setText(QStringLiteral("<span style=\"color:%1\">%2</span>")
                       .arg(ThemeManager::instance().color(Sem::FgMuted).name(), I18n::tr("Version %1").arg(QString::fromLatin1(UWF_VER_STRING))));
  layout->addWidget(version);

  auto* body = new QLabel(&dlg);
  body->setTextFormat(Qt::RichText);
  body->setTextInteractionFlags(Qt::TextBrowserInteraction);
  body->setOpenExternalLinks(true);
  body->setWordWrap(true);
  // 之前试过 QPalette::Link 设主题 accent，但 Qt 在 light 主题下 QLabel 的
  // 富文本链接颜色经常被 QTextDocument 的默认值覆盖（看着仍是无对比度的浅蓝）。
  // 改用 inline `style="color:..."` 注到每个 <a> 标签，绕开 palette / QSS 的所有
  // 干扰。<code> 标签去掉的原因同上：会切到 Courier New 的中文 fallback 渲染糊。
  QString html = I18n::tr(
                     "<p>A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after "
                     "the next reboot.</p>"
                     "<p>Source code: <a href=\"%3\">%3</a></p>"
                     "<p>Copyright © 2026 HsingYun &lt;<a href=\"mailto:%1\">%1</a>&gt;</p>"
                     "<p>This program is released under the <a href=\"%2\">GNU General Public License v3.0</a>; the full license text is included in the "
                     "LICENSE file shipped with this program.</p>"
                     "<p>This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided \"as is\", "
                     "without any warranty.</p>")
                     .arg("iakext@gmail.com", "https://www.gnu.org/licenses/gpl-3.0.html", "https://github.com/HsingYun/UWF-Manager");
  const QString linkColor = ThemeManager::instance().color(Sem::Accent).name();
  html.replace(QStringLiteral("<a "), QStringLiteral("<a style=\"color:%1\" ").arg(linkColor));
  body->setText(html);
  layout->addWidget(body);

  auto* btns = new QDialogButtonBox(&dlg);
  auto* closeBtn = btns->addButton(I18n::tr("Close"), QDialogButtonBox::AcceptRole);
  connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
  layout->addWidget(btns);

  dlg.exec();
}

void MainWindow::showLogs() {
  LogViewerDialog dlg(this);
  dlg.exec();
}

void MainWindow::safeShutdown() {
  if (!confirm(this, I18n::tr("Safe shutdown"), I18n::tr("The system will shut down safely.\nUncommitted changes in this session will be lost.\n\nContinue?")))
    return;

  std::string err;
  auto row = m_filter.read(&err);
  if (!row) {
    warning(this, I18n::tr("Safe shutdown failed"), I18n::tr("Failed to read filter state: %1").arg(QString::fromStdString(err)));
    return;
  }
  std::string err2;
  if (!m_filter.shutdownSystem(*row, &err2)) {
    warning(this, I18n::tr("Safe shutdown failed"), I18n::tr("Shutdown failed: %1").arg(QString::fromStdString(err2)));
  }
}

void MainWindow::safeRestart() {
  if (!confirm(this, I18n::tr("Safe restart"), I18n::tr("The system will restart safely.\nUncommitted changes in this session will be lost.\n\nContinue?")))
    return;

  std::string err;
  auto row = m_filter.read(&err);
  if (!row) {
    warning(this, I18n::tr("Safe restart failed"), I18n::tr("Failed to read filter state: %1").arg(QString::fromStdString(err)));
    return;
  }
  std::string err2;
  if (!m_filter.restartSystem(*row, &err2)) {
    warning(this, I18n::tr("Safe restart failed"), I18n::tr("Restart failed: %1").arg(QString::fromStdString(err2)));
  }
}

void MainWindow::commitFilePath(const QString& path) {
  if (path.isEmpty()) return;

  // 从路径解析盘符，定位到对应的 next-session VolumeRow。
  const QString dl = extractDriveLetter(path);
  if (dl.isEmpty()) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("The path has no drive letter; cannot identify the target volume."));
    return;
  }

  std::string err;
  auto volumes = m_volume.readAll(&err);
  if (!err.empty()) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("Failed to read volume information: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto* row = findCurrentVolume(volumes, dl.toStdString());
  if (!row) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("No current-session record found for volume %1.").arg(dl));
    return;
  }

  // 排除列表用 volumeName (Win32_Volume.DeviceID)
  // 作键，按当前会话的运行态来判断。
  if (auto it = m_snapshot.current.fileExclusions.find(row->volumeName); it != m_snapshot.current.fileExclusions.end()) {
    const std::string hit = findCoveringExclusion(it->second, path.toStdString());
    if (!hit.empty()) {
      warning(this, I18n::tr("Commit rejected"),
              I18n::tr("This path is in the file exclusion list. UWF does not write it to the overlay, so committing it to disk is neither needed "
                       "nor possible.\n\nTarget: %1\nExclusion: %2")
                  .arg(path, QString::fromStdString(hit)));
      return;
    }
  }

  // UWF_Volume.CommitFile 只认单个文件条目；给目录会返回 WBEM_E_NOT_FOUND。
  // 所以目录提交 = 递归遍历目录下所有文件挨个 commit。
  const QFileInfo fi(path);
  const bool isDir = fi.isDir();
  QStringList targets;
  if (isDir) {
    QDirIterator it(path, QDir::Files | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) targets << QDir::toNativeSeparators(it.next());
  } else {
    targets << path;
  }

  if (isDir && targets.isEmpty()) {
    information(this, I18n::tr("Nothing to commit"), I18n::tr("No files were found under %1.").arg(path));
    return;
  }

  if (!confirm(this, I18n::tr("Commit to disk"),
               isDir ? I18n::tr("Recursively walk the folder and commit %1 files to disk one by one. This action cannot be undone.\n\n%2\n\nContinue?")
                           .arg(targets.size())
                           .arg(path)
                     : I18n::tr("Commit the following path from the overlay to disk. This action cannot be undone.\n\n%1\n\nContinue?").arg(path)))
    return;

  // 进度条只在多文件时用；单文件一两个 WMI 调用就够了，弹进度条反而会
  // 因为 show 计时器和 autoClose 的时序问题残留在屏上。
  //
  // 重入约束：QProgressDialog::setValue 会驱动 processEvents 派发挂起的
  // UI 事件。本 dialog 必须 WindowModal，否则用户在 commit 期间点工具栏的
  // refresh / 其它磁盘 TAB 上的 commit 等动作会嵌套触发同一份 m_writeSession，
  // WMI 不是可重入的 → 行为未定义。下面的 assert 把这条不变量编译期/运行期
  // 都钉死，将来谁如果改成 NonModal 会立刻被发现。
  std::unique_ptr<QProgressDialog> progress;
  if (targets.size() > 1) {
    progress = std::make_unique<QProgressDialog>(I18n::tr("Committing…"), I18n::tr("Cancel"), 0, static_cast<int>(targets.size()), this);
    progress->setWindowTitle(I18n::tr("Commit to disk"));
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(500);
    progress->setAutoClose(true);
    progress->setAutoReset(false);
    Q_ASSERT(progress->windowModality() == Qt::WindowModal);
  }

  int okCount = 0;
  bool canceled = false;
  QList<CommitReportRow> nonOkRows;
  for (int i = 0; i < targets.size(); ++i) {
    if (progress) {
      progress->setValue(i);  // 内部 processEvents — 必须靠 WindowModal 拦住外部触发的 WMI 调用
      if (progress->wasCanceled()) {
        canceled = true;
        break;
      }
      const QString& f = targets[i];
      // label 截断一下，目录结构深时不至于把对话框撑到屏外。
      const QString shown = f.size() > 80 ? ("…" + f.right(79)) : f;
      progress->setLabelText(QString("[%1/%2] %3").arg(i + 1).arg(targets.size()).arg(shown));
    }
    const QString& f = targets[i];

    const auto res = m_volume.commitFile(*row, f.toStdString());
    if (res.outcome == CommitOutcome::Ok) {
      ++okCount;
    } else {
      // detail 仅入日志，不显示给用户；UI 翻成一句中文。
      if (!res.detail.empty()) {
        const char* kind = res.outcome == CommitOutcome::Skipped ? "skipped" : "failed";
        UWF_LOG_W("commit") << std::format("CommitFile {}: file={} hr=0x{:08x} rv={} detail={}", kind, f.toStdString(), static_cast<uint32_t>(res.hresult),
                                           res.returnValue, res.detail);
      }
      nonOkRows.append({res.outcome == CommitOutcome::Skipped ? I18n::tr("Skipped") : I18n::tr("Failed"), f, formatErrorCode(res.hresult, res.returnValue),
                        explainCommitFailure(res.hresult, res.returnValue)});
    }
  }
  if (progress) {
    progress->setValue(static_cast<int>(targets.size()));
    progress->close();  // 确保残留窗口不会跨越到结果弹窗之后。
  }

  const int untouched = canceled ? (static_cast<int>(targets.size()) - okCount - static_cast<int>(nonOkRows.size())) : 0;
  showCommitReport(this, okCount, nonOkRows, untouched);
}

void MainWindow::commitFileDeletionPath(const QString& path) {
  if (path.isEmpty()) return;

  // 盘符解析。
  const QString dl = extractDriveLetter(path);
  if (dl.isEmpty()) {
    warning(this, I18n::tr("Commit file deletion failed"), I18n::tr("The path has no drive letter; cannot identify the target volume."));
    return;
  }

  // 核心校验：该文件在当前 OS 视角下**不应存在**；如果还存在，说明 overlay
  // 里并没有删除它，调用 CommitFileDeletion 没意义。
  if (QFileInfo::exists(path)) {
    warning(
        this, I18n::tr("Path still exists"),
        I18n::tr(
            "Commit file deletion requires the path to no longer exist in the current session — that is, the file has already been deleted in this session, "
            "leaving only a deletion marker in the overlay waiting to be written to disk.\n\nHowever, the following path is still visible:\n\n%1\n\nIf you "
            "want to delete a currently visible file and commit the deletion, delete it in File Explorer first, then return here to commit the deletion.")
            .arg(path));
    return;
  }

  std::string err;
  const auto volumes = m_volume.readAll(&err);
  if (!err.empty()) {
    warning(this, I18n::tr("Commit file deletion failed"), I18n::tr("Failed to read volume information: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto* row = findCurrentVolume(volumes, dl.toStdString());
  if (!row) {
    warning(this, I18n::tr("Commit file deletion failed"), I18n::tr("No current-session record found for volume %1.").arg(dl));
    return;
  }

  // 和 CommitFile 一样：如果该路径落在文件排除列表里，UWF 根本不会在
  // overlay 里维护它的删除，提交删除注定失败，早点告诉用户。
  if (auto it = m_snapshot.current.fileExclusions.find(row->volumeName); it != m_snapshot.current.fileExclusions.end()) {
    const std::string hit = findCoveringExclusion(it->second, path.toStdString());
    if (!hit.empty()) {
      warning(this, I18n::tr("Commit rejected"),
              I18n::tr("This path is in the file exclusion list. UWF does not track its deletion in the overlay, so committing the deletion is "
                       "meaningless.\n\nTarget: %1\nExclusion: %2")
                  .arg(path, QString::fromStdString(hit)));
      return;
    }
  }

  if (!confirm(this, I18n::tr("Commit file deletion"),
               I18n::tr("Commit the deletion of the following file to disk. This action cannot be undone.\n\n%1\n\nContinue?").arg(path)))
    return;

  const auto [outcome, hresult, returnValue, detail] = m_volume.commitFileDeletion(*row, path.toStdString());
  if (!detail.empty()) {
    const char* kind = outcome == CommitOutcome::Skipped ? "skipped" : "failed";
    UWF_LOG_W("commit") << std::format("CommitFileDeletion {}: file={} hr=0x{:08x} rv={} detail={}", kind, path.toStdString(), static_cast<uint32_t>(hresult),
                                       returnValue, detail);
  }

  QList<CommitReportRow> nonOkRows;
  int okCount = 0;
  if (outcome == CommitOutcome::Ok) {
    okCount = 1;
  } else {
    nonOkRows.append({outcome == CommitOutcome::Skipped ? I18n::tr("Skipped") : I18n::tr("Failed"), path, formatErrorCode(hresult, returnValue),
                      explainCommitFailure(hresult, returnValue)});
  }
  showCommitReport(this, okCount, nonOkRows);
}

void MainWindow::commitRegistryKey(const QString& key, const QString& valueName) {
  if (key.isEmpty()) return;

  // 注册表排除是全局的，直接比对当前运行会话即可。覆盖 = 键相等或为其祖先。
  const std::string hit = findCoveringExclusion(m_snapshot.current.registryExclusions, key.toStdString());
  if (!hit.empty()) {
    warning(this, I18n::tr("Commit rejected"),
            I18n::tr("This key is in the registry exclusion list. UWF does not write it to the overlay, so committing it to disk is neither "
                     "needed nor possible.\n\nTarget: %1\nExclusion: %2")
                .arg(key, QString::fromStdString(hit)));
    return;
  }

  const QString desc =
      valueName.isEmpty() ? I18n::tr("Key: %1\n(empty value name → commit the entire key)").arg(key) : I18n::tr("Key: %1\nValue: %2").arg(key, valueName);
  if (!confirm(this, I18n::tr("Commit to disk"),
               I18n::tr("Commit the following registry entry to disk. This action cannot be undone.\n\n%1\n\nContinue?").arg(desc)))
    return;

  std::string err;
  auto filters = m_registry.readAll(&err);
  if (!err.empty()) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("Failed to read registry filter: %1").arg(QString::fromStdString(err)));
    return;
  }
  const auto* row = findCurrentRegistryFilter(filters);
  if (!row) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("No current-session registry filter record found."));
    return;
  }
  std::string err2;
  if (!m_registry.commitRegistry(*row, key.toStdString(), valueName.toStdString(), &err2)) {
    warning(this, I18n::tr("Commit failed"), I18n::tr("Failed to write registry: %1").arg(QString::fromStdString(err2)));
    return;
  }
  showTransientHint(I18n::tr("Committed: %1").arg(key), 3000);
}

}  // namespace uwf::ui
