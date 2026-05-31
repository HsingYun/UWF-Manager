#include "MainWindow.h"

#include <windows.h>

#include <QAbstractItemView>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCursor>
#include <QDateTime>
#include <QDialog>
#include <QEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHoverEvent>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStatusBar>
#include <QStyle>
#include <QStyleOption>
#include <QSvgRenderer>
#include <QTabBar>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextOption>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <chrono>
#include <format>
#include <string_view>
#include <utility>

#include "../core/Config.h"
#include "../util/DriveLetter.h"
#include "../util/Log.h"
#include "../util/PathMatch.h"
#include "../util/RegistryKey.h"
#include "../uwf/UwfSnapshot.h"
#include "../uwf/api/UwfmgrCli.h"
#include "../uwf/wmi/WmiResult.h"
#include "AboutDialog.h"
#include "ApplyPlanDialog.h"
#include "DiskTab.h"
#include "GlobalStatusPanel.h"
#include "I18n.h"
#include "ImportApplier.h"
#include "ImportDialog.h"
#include "LogViewerDialog.h"
#include "MarqueeHintBox.h"
#include "Dialogs.h"
#include "PendingCollect.h"
#include "ThemeManager.h"
#include "TransientLabel.h"
#include "TrayController.h"
#include "uwf_version.h"

namespace uwf::ui {

namespace {

// 旧的 warnSelectable / confirmYesNo helper 已迁移到 ui::dialogs（QDialog 实现，
// 走 app font，避免 QMessageBox 的中文渲染糊问题）。
using uwf::ui::dialogs::confirm;
using uwf::ui::dialogs::warning;

// QToolBar 溢出时最右侧的扩展按钮（qt_toolbar_ext_button）：被 QSS 的
// `QToolBar QToolButton` 规则命中后转由 QStyleSheetStyle 渲染，后者既不画
// PE_IndicatorToolBarExtension 雪佛龙、又因该按钮自绘 paintEvent 而忽略 setIcon，
// 于是只剩一个空白方块。QSS background-image 能补图标，但 SVG 被位图栅格化、
// 高分屏放大后发糊。这里用事件过滤器接管它的 Paint：先用 PE_Widget 画出 QSS
// 背景（保留 hover），再用 QSvgRenderer 矢量绘制雪佛龙——任意 DPI 都清晰。
class ToolbarExtIcon : public QObject {
 public:
  using QObject::QObject;

  bool eventFilter(QObject* obj, QEvent* ev) override {
    if (ev->type() != QEvent::Paint) return QObject::eventFilter(obj, ev);
    auto* w = qobject_cast<QWidget*>(obj);
    if (!w) return false;
    QPainter p(w);
    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, w);  // QSS 背景（含 hover）
    static QSvgRenderer svg{QStringLiteral(":/icons/arrow_right.svg")};
    constexpr qreal kSide = 14.0;
    svg.render(&p, QRectF((w->width() - kSide) / 2.0, (w->height() - kSide) / 2.0, kSide, kSide));
    return true;  // 自己画完，吃掉默认那次空白绘制
  }
};

// 盘符逻辑统一在 uwf::drive（见 src/util/DriveLetter.h）。本函数只做 QString
// ↔ std::string 的边界适配，不含任何盘符逻辑。
QString systemDriveLetter() { return QString::fromStdString(drive::systemLetter()); }

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

  constexpr std::string_view kCur = config::kRegPathWindowsCurrentVersion;
  const auto ubr = regkey::readDword(kCur, "UBR");
  const QString productName = QString::fromStdString(regkey::readString(kCur, "ProductName")).trimmed();
  const QString editionId = QString::fromStdString(regkey::readString(kCur, "EditionID")).trimmed();

  // 家族名（Windows 10 / 11 共享 Major=10，靠 build ≥ 22000 区分）。
  QString family = QStringLiteral("Windows");
  if (v.dwMajorVersion == 10) {
    family = v.dwBuildNumber >= static_cast<DWORD>(config::kWindows11MinBuildNumber) ? QStringLiteral("Windows 11") : QStringLiteral("Windows 10");
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
  const bool isLtsc =
      std::ranges::any_of(config::kLtscEditionIds, [&ed](std::string_view id) { return ed == QLatin1String(id.data(), static_cast<qsizetype>(id.size())); });
  if (isLtsc && !edition.contains("LTSC", Qt::CaseInsensitive) && !edition.contains("LTSB", Qt::CaseInsensitive)) {
    edition = edition.isEmpty() ? QStringLiteral("LTSC") : (edition + QStringLiteral(" LTSC"));
  }

  QString head = edition.isEmpty() ? family : (family + ' ' + edition);
  return QString("%1 · %2.%3.%4.%5").arg(head).arg(v.dwMajorVersion).arg(v.dwMinorVersion).arg(v.dwBuildNumber).arg(ubr);
}

QString cpuModelText() {
  const QString name =
      QString::fromStdString(regkey::readString(R"(HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0)", "ProcessorNameString"));
  // BIOS 厂商常常在名字里塞大量尾随空格，去一下。
  return name.trimmed().simplified();
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

}  // namespace

MainWindow::MainWindow(bool compatibilityMode, const QString& osProductName, const QString& osEditionId, QWidget* parent)
    : QMainWindow(parent), m_compatibilityMode(compatibilityMode), m_osProductName(osProductName), m_osEditionId(osEditionId) {
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
  m_writeSession.connect(config::kWmiNamespaceEmbedded, &err);
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

  // 4 个 commit{File,FileDeletion,Registry,RegistryDeletion}Path 槽的实际工作都
  // 在 CommitDispatcher 里跑——这里只需把 4 个槽变成代理。注意构造时点：要在
  // m_usageTimer 创建之后，因为 dispatcher 会把它当 ScopedTimerPause 的对象。
  m_commit = std::make_unique<CommitDispatcher>(m_writeSession, m_snapshot, m_usageTimer, this);

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

  // 给工具栏溢出扩展按钮装上雪佛龙绘制器（见 ToolbarExtIcon 注释）。过滤器对象
  // parent 设为按钮本身——rebuildUi 重建 toolbar 时随按钮一并回收。
  if (auto* ext = tb->findChild<QToolButton*>(QStringLiteral("qt_toolbar_ext_button"))) {
    ext->installEventFilter(new ToolbarExtIcon(ext));
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
  // 系统版本未通过校验时，把兼容模式提示常驻在面板信息框里。提示文案在此
  // 现翻译——切语言会重跑 buildUi，文案随之跟着切；rebuildUi 重建 m_global
  // 后也连带重新灌入。
  if (m_compatibilityMode) {
    m_global->setCompatibilityNotice(
        I18n::tr("The current system \"%1\" (%2) is not a recognized supported edition. UWF Manager is running in compatibility mode "
                 "and some features may be unavailable.")
            .arg(m_osProductName, m_osEditionId));
  }
  // 顶部全局设置拿走所有可拉伸空间，里面的 QScrollArea 会在高度不足时
  // 自己滚动；tips 区用固定高度贴底。
  globalLayout->addWidget(m_global, 1);
  // 用 QTextBrowser 而非 QLabel：QLabel 的自动换行只能在词边界断，遇到没有空格
  // 的长串（注册表键 HKEY_LOCAL_MACHINE\... 整串无断点）只能在中文前缀"注册表："
  // 处断一次、余下整段溢出裁掉。改用 QTextBrowser + WrapAnywhere（任意字符断行）。
  // 别用 WrapAtWordBoundaryOrAnywhere——它"优先词边界"，会把超长的键整体先甩到下一
  // 行再硬断，第一行照样空着；WrapAnywhere 才是逐字符填满每行、键紧接"注册表："往下
  // 排。配成只读 / 无边框 / 关滚动条 / 不可交互，外观仍由 QSS #hoverHintBox 提供
  // （圆角灰底）。
  // MarqueeHintBox（QTextBrowser 子类）：英文下个别提示（域机密密钥 / TSCAL 的默认
  // 排除说明）超过这 110px 放不下，本类在溢出时自动循环来回滚动让全文都能看到；
  // 放得下时静止。其余配置照旧——它对外仍是个普通 QTextBrowser。
  m_hoverHint = new MarqueeHintBox(this);
  m_hoverHint->setObjectName("hoverHintBox");
  m_hoverHint->setWordWrapMode(QTextOption::WrapAnywhere);
  m_hoverHint->setFrameShape(QFrame::NoFrame);
  m_hoverHint->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_hoverHint->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_hoverHint->setTextInteractionFlags(Qt::NoTextInteraction);
  m_hoverHint->setFocusPolicy(Qt::NoFocus);
  // document margin 归零，内边距完全交给 QSS 的 padding，与原 QLabel 视觉对齐。
  m_hoverHint->document()->setDocumentMargin(0);
  // viewport 透明，让 QSS 画在 frame 上的圆角灰底透出来（否则文本区方角会盖角）。
  m_hoverHint->viewport()->setAutoFillBackground(false);
  m_hoverHint->setFixedHeight(110);
  m_hoverHint->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  // 默认文案是机器基本信息（OS / CPU / RAM / GPU）的 HTML，悬停事件临时覆盖成
  // 纯文本 tooltip。QTextBrowser::setText 走 Qt::mightBeRichText 自动判别：HTML
  // 基线按富文本渲染，纯文本 tooltip 按纯文本渲染，里面的 & / < 不会被解析走样。
  m_hoverCtl = new TransientLabel(m_hoverHint, m_hoverHint);
  m_hoverCtl->setBaseline(systemInfoHtml());
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

  // 状态栏走 TransientLabel：setBaseline = refresh / updatePendingSummary 的
  // 常驻文案，flash() = DiskTab 临时提示（覆盖几秒后自动回基线）。直接用
  // statusBar()->showMessage 走不通——stretch=1 的 permanent widget 把它挤没
  // 显示空间。
  m_statusCtl = new TransientLabel(m_statusText, m_statusText);

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
  // m_statusCtl / m_hoverCtl 的 QObject parent 设为对应 label，label 上面
  // deleteLater 时它们一起 deleteLater——不需要单独管。

  // 重置所有指针成员；buildUi 会重新填充。
  m_actRefresh = m_actImport = m_actPlan = m_actShutdown = m_actRestart = nullptr;
  m_actLog = m_actAbout = m_actLang = m_actTheme = nullptr;
  m_tabs = nullptr;
  m_global = nullptr;
  m_hoverHint = nullptr;
  m_statusText = nullptr;
  m_statusCtl = m_hoverCtl = nullptr;
  m_diskTabs.clear();

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
  if (m_statusCtl) m_statusCtl->flash(text, msec);
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
  // hoverHint 默认 HTML 含 inline color，主题切换后重新生成；处于 hover
  // transient 时 TransientLabel 内部会自动延后到回基线时再刷。
  if (m_hoverCtl) m_hoverCtl->setBaseline(systemInfoHtml());
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev) {
  // hover 到任意带 hoverHint 属性（或 toolTip）的控件，就把说明塞到右侧面板的
  // 提示框里；离开时延迟回基线。走 qApp 级事件过滤器才能捕获所有子控件。
  // restoreAfter 用 120ms：光标在相邻控件之间移动时下一次 enter 会立刻 show，
  // 取消未到期的恢复，避免文字闪烁。
  constexpr int kHoverRestoreMs = 120;
  if (!m_hoverCtl) return QMainWindow::eventFilter(obj, ev);
  // QDialog 子控件原样放行——对话框走原生 QToolTip 矩形气泡，不复用主窗口
  // 右下角的 hint 面板：模态对话框遮住面板的话面板里的字根本看不到，且每个
  // 对话框上下文独立、和主窗口的 baseline 没关系。RegistryPickerDialog 的
  // Name/Type/Data cell 就指望这条放行让原生 tooltip 弹出来。
  // 走 QObject::parent() 链而不是 window()——QMenu 之类的 popup 自身是 top-level，
  // 但它们的 QObject 父链仍指回触发它们的 widget，能正确判断是不是属于对话框。
  for (QObject* p = obj; p; p = p->parent()) {
    if (qobject_cast<QDialog*>(p)) return QMainWindow::eventFilter(obj, ev);
  }
  const auto type = ev->type();
  // 屏蔽原生 tooltip 气泡：截停 ToolTip 事件，说明文字只在右下角面板里。
  if (type == QEvent::ToolTip) return true;

  // QMenu 上的 QAction 不是 QWidget——下面的 parent-toolTip 链拿不到它；落到
  // QMenu 自身上又只会走出菜单按钮的 toolTip（不是当前悬停的那一项）。这里专门
  // 拦菜单的悬停事件：用 menu->activeAction() 反查当前选中项，把它显式设置过的
  // toolTip 推到提示框。判定"显式设置"= toolTip() != text()——Qt 默认把未设置的
  // toolTip 回落到 text()，相同时把同一句话再推一次没有价值。
  if (auto* menu = qobject_cast<QMenu*>(obj)) {
    if (type == QEvent::MouseMove || type == QEvent::HoverMove || type == QEvent::Enter || type == QEvent::HoverEnter) {
      const QAction* act = menu->activeAction();
      const QString tip = act ? act->toolTip() : QString();
      if (act && !tip.isEmpty() && tip != act->text()) {
        m_hoverCtl->show(tip);
      } else {
        // 当前项没有专门 toolTip：让面板回到默认（用 delay 抗闪烁）。
        m_hoverCtl->restoreAfter(kHoverRestoreMs);
      }
    } else if (type == QEvent::Leave || type == QEvent::HoverLeave || type == QEvent::Hide) {
      m_hoverCtl->restoreAfter(kHoverRestoreMs);
    }
    return QMainWindow::eventFilter(obj, ev);
  }

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
          m_hoverCtl->show(tip);
          return QMainWindow::eventFilter(obj, ev);
        }
      }
    }
    // QListWidget / QTableWidget / QTreeWidget 的 item tooltip 存在
    // QListWidgetItem / QTableWidgetItem 上，view 自身的 toolTip() 是空的；
    // 走下方通用的 parent 链拿不到。事件落在 viewport 上（hover/move 都是），
    // 这里按命中坐标反查 index，把 Qt::ToolTipRole 推到提示框。命中空白或
    // item 没设 tooltip 时不 return，落到 parent 链兜底（让 view 自身的
    // toolTip——若有——仍能展示）。
    if (auto* view = qobject_cast<QAbstractItemView*>(w->parentWidget()); view && view->viewport() == w) {
      QPoint pos;
      if (auto* me = dynamic_cast<QMouseEvent*>(ev))
        pos = me->pos();
      else if (auto* he = dynamic_cast<QHoverEvent*>(ev))
        pos = he->position().toPoint();
      else
        pos = w->mapFromGlobal(QCursor::pos());
      const QModelIndex idx = view->indexAt(pos);
      if (idx.isValid()) {
        const QString tip = idx.data(Qt::ToolTipRole).toString();
        if (!tip.isEmpty()) {
          m_hoverCtl->show(tip);
          return QMainWindow::eventFilter(obj, ev);
        }
      }
    }
    QWidget* cur = w;
    while (cur && cur->toolTip().isEmpty() && cur != this) cur = cur->parentWidget();
    if (cur && !cur->toolTip().isEmpty()) {
      m_hoverCtl->show(cur->toolTip());
    }
  } else if (type == QEvent::Leave || type == QEvent::HoverLeave) {
    m_hoverCtl->restoreAfter(kHoverRestoreMs);
  }
  return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::updatePendingSummary() {
  // 与 ApplyPlanDialog 共用同一次遍历（collectPending）+ 同一计数口径
  // （PendingChanges::count），避免状态栏摘要和预览标题各算各的、容易漂。
  const std::size_t pending = collectPending(m_global, m_diskTabs).count();
  const QString msg = pending > 0 ? I18n::tr("%1 pending change(s) (not yet written to the system)").arg(pending) : I18n::tr("No pending changes");
  if (m_statusCtl) m_statusCtl->setBaseline(msg);
}

void MainWindow::rebuildTabs(const std::vector<core::DiskInfo>& disks) {
  // 重建前按盘符记下两件事，重建后尽量还原——避免 refresh 把用户的"上下文"
  // 都跳走：
  //   1) 当前选中的卷（外层 TAB 的盘符）；
  //   2) 每个 DiskTab 内层"文件 / 注册表排除"TAB 的索引（仅系统盘有 1=注册表）。
  // 内层索引用 int 而不是 tabText，因为 text 受 i18n 影响（切语言后不稳定）；
  // 索引在所有语言下都稳定。
  const QString prevDriveLetter = m_tabs->currentIndex() >= 0 ? m_tabs->tabText(m_tabs->currentIndex()) : QString();
  QMap<QString, int> prevInfoTab;
  for (const auto& t : std::as_const(m_diskTabs))
    if (t) prevInfoTab.insert(t->driveLetter(), t->activeInfoTabIndex());

  // QTabWidget::clear() 只摘掉标签页、不销毁页面控件——上一轮的 DiskTab 会继续
  // 作为 m_tabs 的子对象存活，每次 refresh 泄漏一组（要到下次 rebuildUi 删掉
  // m_tabs 才被连带回收）。这里先显式 deleteLater 回收旧的一组。用 deleteLater
  // 而非 delete：rebuildTabs 可能经由某个 DiskTab 自己的信号回调间接调进来，
  // 同步 delete 会销毁正在执行回调的对象。
  for (const auto& t : std::as_const(m_diskTabs))
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
    // 还原本卷内层 TAB 的选中索引。原本不在（磁盘新插入）→ 保持默认 0。
    if (const auto it = prevInfoTab.constFind(label); it != prevInfoTab.constEnd()) {
      tab->setActiveInfoTabIndex(it.value());
    }
    m_diskTabs.push_back(tab);
    connect(tab, &DiskTab::pendingChanged, this, &MainWindow::updatePendingSummary);
    connect(tab, &DiskTab::statusHint, this, &MainWindow::showTransientHint);
    connect(tab, &DiskTab::commitFileRequested, this, &MainWindow::commitFilePath);
    connect(tab, &DiskTab::commitFileDeletionRequested, this, &MainWindow::commitFileDeletionPath);
    connect(tab, &DiskTab::commitRegistryRequested, this, &MainWindow::commitRegistryKey);
    connect(tab, &DiskTab::commitRegistryDeletionRequested, this, &MainWindow::commitRegistryDeletionKey);
  }

  // 切回刷新前选中的卷（按盘符匹配）。该卷若已不在（磁盘被移除）则保持默认第 0 个。
  if (!prevDriveLetter.isEmpty()) {
    for (int i = 0; i < m_tabs->count(); ++i) {
      if (m_tabs->tabText(i) == prevDriveLetter) {
        m_tabs->setCurrentIndex(i);
        break;
      }
    }
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
  if (m_statusCtl) m_statusCtl->setBaseline(I18n::tr("Refreshed · %1 volumes").arg(disks.size()));
  const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
  UWF_LOG_I("ui") << std::format("refresh done: disks={} uwfAvailable={} currentVolumes={} nextVolumes={} elapsedMs={}", disks.size(), m_snapshot.uwfAvailable,
                                 m_snapshot.current.volumes.size(), m_snapshot.next.volumes.size(), elapsedMs);
}

void MainWindow::showPlan() {
  // 工具栏按钮 focusPolicy 是 Qt::NoFocus，点它不会让正在编辑的 spinbox 失焦；
  // 而阈值 / 最大尺寸等 spinbox 关了 keyboardTracking（见 GlobalStatusPanel），
  // 新输入的值要等 editingFinished（失焦或回车）才提交。不先逼当前编辑器提交，
  // 下面 ApplyPlanDialog 构造时 pendingOverlay() 就会读到改动前的旧值——首次预览
  // 漏掉刚输入的阈值，要等 exec() 弹框抢走焦点提交后、第二次点开才正常。读取
  // pending 前主动清掉焦点，触发 editingFinished 把在编辑的值落定（顺带跑一次
  // 约束链钳值，使预览与实际写入口径一致）。
  if (QWidget* fw = QApplication::focusWidget()) fw->clearFocus();

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
  dlg.setApplier([this](const QList<api::UwfmgrCommand>& cmds) { return applyImportCommands(cmds, m_global, m_diskTabs); });
  dlg.exec();
}

void MainWindow::showAbout() {
  AboutDialog dlg(this);
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
  if (const auto r = m_filter.shutdownSystem(*row); !r.ok) {
    warning(this, I18n::tr("Safe shutdown failed"), I18n::tr("Shutdown failed: %1").arg(QString::fromStdString(r.detail)));
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
  if (const auto r = m_filter.restartSystem(*row); !r.ok) {
    warning(this, I18n::tr("Safe restart failed"), I18n::tr("Restart failed: %1").arg(QString::fromStdString(r.detail)));
  }
}

// 4 个 commit 槽自身只是 dispatcher 的代理——拿到 DiskTab / ExclusionListWidget /
// OverlayFilesDialog / 命令行的转发后，所有 batch 提交流程在 CommitDispatcher
// 里跑（含目标枚举、排除冲突预校验、QProgressDialog、结果对话框）。
void MainWindow::commitFilePath(const QString& path) { m_commit->commitFilePath(path); }
void MainWindow::commitFileDeletionPath(const QString& path) { m_commit->commitFileDeletionPath(path); }
void MainWindow::commitRegistryKey(const QString& key, const QString& valueName) { m_commit->commitRegistryKey(key, valueName); }
void MainWindow::commitRegistryDeletionKey(const QString& key, const QString& valueName) { m_commit->commitRegistryDeletionKey(key, valueName); }

}  // namespace uwf::ui
