/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "MainWindow.h"

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
#include <QStatusBar>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextOption>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <chrono>
#include <format>
#include <optional>
#include <utility>

#include "../core/Config.h"
#include "../util/DriveLetter.h"
#include "../util/Log.h"
#include "../util/PathMatch.h"
#include "../uwf/UwfSnapshot.h"
#include "../uwf/api/UwfmgrCli.h"
#include "../uwf/wmi/WmiResult.h"
#include "AboutDialog.h"
#include "ApplyPlanDialog.h"
#include "Dialogs.h"
#include "DiskTab.h"
#include "GlobalStatusPanel.h"
#include "HoverHintController.h"
#include "I18n.h"
#include "ImportApplier.h"
#include "ImportDialog.h"
#include "LogViewerDialog.h"
#include "MarqueeHintBox.h"
#include "OverlayPresentationController.h"
#include "PendingCollect.h"
#include "PowerController.h"
#include "SystemInfoProvider.h"
#include "ThemeManager.h"
#include "TransientLabel.h"
#include "TrayController.h"
#include "WindowChromeController.h"

namespace uwf::ui {

namespace {

// 盘符逻辑统一在 uwf::drive（见 src/util/DriveLetter.h）。本函数只做 QString
// ↔ std::string 的边界适配，不含任何盘符逻辑。
QString systemDriveLetter() { return QString::fromStdString(drive::systemLetter()); }

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
  m_hoverHints = new HoverHintController(this, this);
  m_chrome = new WindowChromeController(this, this);

  // 写会话提前连接一次；读快照时会另起一个独立会话。
  std::string err;
  m_writeSession.connect(config::kWmiNamespaceEmbedded, &err);
  // 内容控件与首屏数据统一交给 showEvent 调度的 rebuildUi()——它一次 buildUi()
  // + refresh() 建好。构造期不再 buildUi()/refresh()：那份产出会被 rebuildUi
  // 整个销毁重建，等于白建一遍 UI、白连一次 WMI、白读一份快照。

  // 系统托盘（图标 + 右键菜单）——独立组件，由本窗口编排：接它的"激活窗口"信号。
  m_tray = new TrayController(m_writeSession, this);
  connect(m_tray, &TrayController::activateWindowRequested, this, &MainWindow::raiseToFront);
  connect(m_tray, &TrayController::exitApplicationRequested, this, &MainWindow::requestExit);

  m_overlayPresentation = new OverlayPresentationController(m_writeSession, this, m_tray, this);
  connect(m_overlayPresentation, &OverlayPresentationController::activateMainWindowRequested, this, &MainWindow::raiseToFront);
  connect(m_overlayPresentation, &OverlayPresentationController::exitApplicationRequested, this, &MainWindow::requestExit);
  m_power = new PowerController(m_writeSession, this, this);

  // 4 个 commit 槽的实际工作都在 CommitDispatcher 里跑；提交期间暂停 Overlay
  // 控制器的 usage timer，避免并发读取 WMI。
  m_commit = std::make_unique<CommitDispatcher>(m_writeSession, m_snapshot, m_overlayPresentation->usageTimer(), this);

  // 首屏 rebuildUi 不在 ctor 里同步触发——widget 此时还没 show，Qt 一些 polish
  // / 几何计算在 widget 真正进入 shown 状态前结果不稳定，会跟后续"切主题 /
  // 切语言时已 shown 状态下的 rebuildUi"产生差异。改放到 showEvent 第一次
  // 触发后用 singleShot 调度，确保首次 rebuild 也在 shown 状态下跑。
}

MainWindow::~MainWindow() {
  // QObject 子对象默认要等 QMainWindow / QObject 基类析构时才删除，而
  // m_writeSession 作为 C++ 成员会更早析构。显式按依赖逆序释放，保证所有
  // 持有 session / timer 引用的对象都在其依赖仍存活时结束生命周期。
  m_commit.reset();

  delete m_power;
  m_power = nullptr;
  delete m_overlayPresentation;  // 先于它引用的 tray
  m_overlayPresentation = nullptr;
  delete m_tray;
  m_tray = nullptr;

  // 两个 qApp 级事件过滤器也在窗口仍完整时移除，避免进入 QWidget / QObject
  // 基类析构后还持有一个正在拆解的 root window。
  delete m_chrome;
  m_chrome = nullptr;
  delete m_hoverHints;
  m_hoverHints = nullptr;
}

void MainWindow::raiseToFront() { m_chrome->raiseToFront(m_firstShowDone); }

void MainWindow::buildUi() {
  // 标题随语言切换重译，故每次 buildUi（含 rebuildUi 路径）都重设一次；
  // 图标与初始尺寸是一次性窗口外壳设置，已在构造函数里完成，这里不再重复。
  setWindowTitle(I18n::tr("Unified Write Filter (UWF) Manager"));
  m_chrome->applyTitleBarTheme();

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
  tb->setContextMenuPolicy(Qt::PreventContextMenu);
  // iconSize 宽度 = 字形 16 + 右侧 6px 透明边，高度仍 16。加宽"图标—文字"间距：Qt 把
  // 该间距硬编码成"图标宽 + 4px"，没法在样式层改；而 QToolBar 又强制所有按钮用工具栏
  // 统一的 iconSize（单独给按钮 setIconSize 会被它覆盖）。所以从工具栏 iconSize 下手——
  // 配合 ThemedSvgIconEngine：带文字按钮的图标字形靠左渲染、右侧留透明边（字形左缘不
  // 动，只把文字往右推 6px），纯图标的语言 / 主题按钮改用居中渲染（见 refreshThemedUi）。
  tb->setIconSize({16 + 6, 16});
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
  connect(m_actRefresh, &QAction::triggered, this, [this]() {
    if (!confirmDiscardPendingChanges()) return;
    refresh();
  });

  m_actPlan = tb->addAction(I18n::tr("Review and apply"));
  m_actPlan->setToolTip(I18n::tr("Review all pending changes and apply them in one batch. Most changes take effect after the next reboot."));
  connect(m_actPlan, &QAction::triggered, this, &MainWindow::showPlan);

  tb->addSeparator();

  m_actShutdown = tb->addAction(I18n::tr("Safe shutdown"));
  m_actShutdown->setToolTip(I18n::tr("Shut down safely, even when the UWF overlay is full."));
  connect(m_actShutdown, &QAction::triggered, m_power, &PowerController::safeShutdown);

  m_actRestart = tb->addAction(I18n::tr("Safe restart"));
  m_actRestart->setToolTip(I18n::tr("Restart safely, even when the UWF overlay is full."));
  connect(m_actRestart, &QAction::triggered, m_power, &PowerController::safeRestart);

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

  auto* hubAction = tb->addAction("");
  hubAction->setCheckable(true);
  hubAction->setToolTip(I18n::tr("Show or hide the overlay hub."));
  connect(hubAction, &QAction::toggled, m_overlayPresentation, &OverlayPresentationController::setHubEnabled);
  if (auto* btn = qobject_cast<QToolButton*>(tb->widgetForAction(hubAction))) {
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
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
      if (I18n::instance().lang() == target) return true;
      if (!confirmDiscardPendingChanges()) return false;
      // 抑制 paint 直到 rebuildUi 结束，避免用户看到 tear-down 中间空白
      // 一帧。配对的 setUpdatesEnabled(true) 在 rebuildUi 末尾。
      setUpdatesEnabled(false);
      I18n::instance().setLang(target);
      QTimer::singleShot(0, this, &MainWindow::rebuildUi);
      return true;
    };

    auto* enAct = menu->addAction("English");
    enAct->setCheckable(true);
    enAct->setActionGroup(langGroup);
    enAct->setChecked(I18n::instance().lang() == I18n::Lang::En);

    auto* zhAct = menu->addAction("简体中文");
    zhAct->setCheckable(true);
    zhAct->setActionGroup(langGroup);
    zhAct->setChecked(I18n::instance().lang() == I18n::Lang::Zh_CN);

    auto restoreLangChecks = [enAct, zhAct]() {
      enAct->setChecked(I18n::instance().lang() == I18n::Lang::En);
      zhAct->setChecked(I18n::instance().lang() == I18n::Lang::Zh_CN);
    };
    connect(enAct, &QAction::triggered, this, [switchTo, restoreLangChecks]() {
      if (!switchTo(I18n::Lang::En)) restoreLangChecks();
    });
    connect(zhAct, &QAction::triggered, this, [switchTo, restoreLangChecks]() {
      if (!switchTo(I18n::Lang::Zh_CN)) restoreLangChecks();
    });

    btn->setMenu(menu);
  }

  m_actTheme = tb->addAction("");
  m_actTheme->setToolTip(I18n::tr("Toggle light / dark theme. Follows the system setting on startup."));
  // 在 toggle() 之前禁 paint：toggle 内部会 setPalette + setStyleSheet("") +
  // setStyleSheet(sheet)，中间 unstyled 那一帧会闪。先压 paint 再切，再让
  // 后续 themeChanged → singleShot → rebuildUi → setUpdatesEnabled(true)
  // 一次性把最终态画出来。
  connect(m_actTheme, &QAction::triggered, this, [this]() {
    if (!confirmDiscardPendingChanges()) return;
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

  m_chrome->decorateToolbar(tb);

  // 主题切换走和语言切换完全相同的入口：rebuildUi 整体重建 toolbar + 中央
  // widget。两套刷新走同一条路径，避免之前"主题切换只刷 icon、语言切换全
  // 重建"两套机制各自的几何跳变。切换前会先确认是否丢弃 pending changes。
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
  // 不再钉死一个固定的 minimumWidth。Qt 的 qSmartMinSize 里 "minSize.width()>0 →
  // 直接覆盖" 的规则会让任何显式 setMinimumWidth 盖掉内容真实需求——之前这里
  // 写死 220，磁盘页那行（保护状态 + 两张会话卡片 + 绑定下拉框 + 右侧两个按钮，
  // 都不可压缩）实际要 ~目标宽度，却被上报成 220，窗口于是能缩到把整行裁掉。
  // 留空让磁盘页内容如实上报最小宽度，QMainWindow 自动把窗口下限钉到
  // 「左区内容 + 右侧 globalWrap」之和，低于此不再允许缩小。
  m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  mainRow->addWidget(m_tabs, 1);

  auto* globalWrap = new QWidget(this);
  // 纯 QWidget 默认不会应用 QSS 的 background；必须打开 WA_StyledBackground，
  // 否则 QWidget#globalWrap
  // 规则里的背景色根本不生效，看上去就和主窗口底色有差。
  globalWrap->setAttribute(Qt::WA_StyledBackground, true);
  auto* globalLayout = new QVBoxLayout(globalWrap);
  globalLayout->setContentsMargins(18, 12, 18, 12);
  globalLayout->setSpacing(10);
  m_global = new GlobalStatusPanel(this);
  m_overlayPresentation->bindUi(m_global, hubAction);
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
  auto* hoverHint = new MarqueeHintBox(this);
  hoverHint->setObjectName("hoverHintBox");
  hoverHint->setWordWrapMode(QTextOption::WrapAnywhere);
  hoverHint->setFrameShape(QFrame::NoFrame);
  hoverHint->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  hoverHint->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  hoverHint->setTextInteractionFlags(Qt::NoTextInteraction);
  hoverHint->setFocusPolicy(Qt::NoFocus);
  // document margin 归零，内边距完全交给 QSS 的 padding，与原 QLabel 视觉对齐。
  hoverHint->document()->setDocumentMargin(0);
  // viewport 透明，让 QSS 画在 frame 上的圆角灰底透出来（否则文本区方角会盖角）。
  hoverHint->viewport()->setAutoFillBackground(false);
  hoverHint->setFixedHeight(110);
  hoverHint->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  // 默认文案是机器基本信息（OS / CPU / RAM / GPU）的 HTML，悬停事件临时覆盖成
  // 纯文本 tooltip。QTextBrowser::setText 走 Qt::mightBeRichText 自动判别：HTML
  // 基线按富文本渲染，纯文本 tooltip 按纯文本渲染，里面的 & / < 不会被解析走样。
  m_hoverCtl = new TransientLabel(hoverHint, hoverHint);
  m_hoverCtl->setBaseline(SystemInfoProvider::summaryHtml());
  m_hoverHints->setTarget(m_hoverCtl);
  globalLayout->addWidget(hoverHint, 0);
  globalWrap->setObjectName("globalWrap");
  globalWrap->setFixedWidth(420);
  // 右侧面板整体最小高度 = 外壳最小高 + 滚动内容完整高。GlobalStatusPanel 的滚动区
  // 最小高恒为 0（永远可收缩、保证有横幅时滚动表现一致），所以 globalLayout->minimumSize()
  // 只含外壳（标题 / 横幅 / tips / 内边距）、不含两张卡片的高度；这里把内容完整高
  // （preferredContentHeight）显式加回，窗口下限才恰好容得下整块内容、无横幅时不滚动，
  // 也不会因为漏算内容而能把窗口缩得过小。二者解耦：抬窗口下限不影响滚动一致性。
  globalWrap->setMinimumHeight(globalLayout->minimumSize().height() + m_global->preferredContentHeight());
  mainRow->addWidget(globalWrap, 0);

  // 让 MainWindow 也跟随到这个最小高度（加上工具栏和状态栏的大致高度）。
  setMinimumHeight(globalWrap->minimumHeight() + 80);

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
  // 默认文案都按当前主题色生成（构造期间 connect 时 m_hoverCtl 还是 null，
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
  m_overlayPresentation->unbindUi();

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
  m_hoverHints->setTarget(nullptr);

  // 重置所有指针成员；buildUi 会重新填充。
  m_actRefresh = m_actImport = m_actPlan = m_actShutdown = m_actRestart = nullptr;
  m_actLog = m_actAbout = m_actLang = m_actTheme = nullptr;
  m_tabs = nullptr;
  m_global = nullptr;
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

void MainWindow::requestExit() {
  m_exitRequested = true;

  const auto finishExit = [this]() {
    if (!close()) {
      m_exitRequested = false;
      m_overlayPresentation->restoreHub();
      return;
    }
    QCoreApplication::quit();
  };

  if (m_global && collectPending(m_global, m_diskTabs).count() > 0 && !isVisible()) {
    m_overlayPresentation->hideHubTemporarily();
    raiseToFront();
    QTimer::singleShot(0, this, [this, finishExit]() { QTimer::singleShot(0, this, finishExit); });
    return;
  }

  m_overlayPresentation->hideHubTemporarily();
  finishExit();
}

bool MainWindow::confirmDiscardPendingChanges() {
  if (!m_global) return true;
  if (QWidget* fw = QApplication::focusWidget()) fw->clearFocus();

  const std::size_t pending = collectPending(m_global, m_diskTabs).count();
  if (pending == 0) return true;

  if (isMinimized()) {
    showNormal();
    raise();
    activateWindow();
  }

  return dialogs::confirm(this, I18n::tr("Discard pending changes?"),
                          I18n::tr("There are %1 pending change(s) that have not been applied.\n\nContinue and discard them?").arg(pending));
}

void MainWindow::changeEvent(QEvent* ev) {
  QMainWindow::changeEvent(ev);
  if (ev->type() != QEvent::WindowStateChange || !isMinimized() || !m_overlayPresentation->hubPresented()) return;

  // 等原生最小化状态切换完成后再隐藏，避免在 WindowStateChange 分发过程中
  // 重入 show/hide。回调执行前重新检查：用户若已立即还原，或两个 overlay
  // 恰好都被关闭，就保留当前窗口状态。
  QTimer::singleShot(0, this, [this]() {
    if (isMinimized() && m_overlayPresentation->hubPresented()) hide();
  });
}

void MainWindow::closeEvent(QCloseEvent* ev) {
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
  // 带文字的按钮：图标用默认靠左渲染——工具栏 iconSize 宽于高，靠左即在图标右侧留出
  // 透明边，把文字往右推、加宽"图标—文字"间距（见 buildUi 里 setIconSize 的说明）。
  if (m_actImport) m_actImport->setIcon(tm.icon(":/icons/add.svg"));
  if (m_actRefresh) m_actRefresh->setIcon(tm.icon(":/icons/refresh.svg"));
  if (m_actPlan) m_actPlan->setIcon(tm.icon(":/icons/apply.svg"));
  if (m_actShutdown) m_actShutdown->setIcon(tm.icon(":/icons/shutdown.svg"));
  if (m_actRestart) m_actRestart->setIcon(tm.icon(":/icons/restart.svg"));
  if (m_actLog) m_actLog->setIcon(tm.icon(":/icons/log.svg"));
  if (m_actAbout) m_actAbout->setIcon(tm.icon(":/icons/info.svg"));
  // 纯图标按钮（语言 / 主题）：没有文字，图标要在按钮里居中，故用 AlignHCenter 渲染，
  // 不受工具栏那 6px 右侧透明边影响。
  constexpr Qt::Alignment kCenter = Qt::AlignHCenter | Qt::AlignVCenter;
  m_overlayPresentation->refreshActionIcon();
  if (m_actLang) m_actLang->setIcon(tm.icon(":/icons/language.svg", kCenter));
  if (m_actTheme) {
    // 当前 dark → 显示太阳图标（点了切到 light）；当前 light → 显示月亮。
    const bool isDark = tm.current() == Theme::Dark;
    m_actTheme->setIcon(tm.icon(isDark ? ":/icons/theme_sun.svg" : ":/icons/theme_moon.svg", kCenter));
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
  if (m_hoverCtl) m_hoverCtl->setBaseline(SystemInfoProvider::summaryHtml());
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

  // 注册表排除是全局的，只挂在一块盘的 TAB 上：系统盘优先，其次第一块可用盘，
  // 最后退回第一块磁盘。这样即使所有卷都不支持文件排除 / 单文件提交，注册表
  // 排除列表和注册表提交入口仍然可见。没有任何磁盘时不挂载注册表 TAB。
  std::optional<std::size_t> registryHostIndex;
  if (!sysDl.isEmpty()) {
    for (std::size_t i = 0; i < disks.size(); ++i) {
      const auto& d = disks[i];
      if (QString::fromStdString(d.driveLetter).toUpper() == sysDl) {
        registryHostIndex = i;
        break;
      }
    }
  }
  if (!registryHostIndex) {
    for (std::size_t i = 0; i < disks.size(); ++i) {
      const auto& d = disks[i];
      if (d.support == core::DiskSupport::Supported || d.support == core::DiskSupport::FileSystemLimited) {
        registryHostIndex = i;
        break;
      }
    }
  }
  if (!registryHostIndex && !disks.empty()) {
    registryHostIndex = 0;
  }

  for (std::size_t i = 0; i < disks.size(); ++i) {
    const auto& d = disks[i];
    auto* tab = new DiskTab(d, /*showRegistry=*/registryHostIndex && i == *registryHostIndex, this);
    const QString label = QString::fromStdString(d.driveLetter);
    const bool limited = d.support == core::DiskSupport::FileSystemLimited;
    const bool ok = d.support == core::DiskSupport::Supported || limited;
    const bool isSys = QString::fromStdString(d.driveLetter).toUpper() == sysDl;
    auto& tm = ThemeManager::instance();
    const QIcon icon = !ok ? tm.icon(":/icons/disk_off.svg") : isSys ? tm.icon(":/icons/disk_system.svg") : tm.icon(":/icons/disk.svg");
    const int idx = m_tabs->addTab(tab, icon, label);
    const QString sysExtra = isSys ? I18n::tr(" (System drive: also manages the global registry exclusion list here.)") : QString();
    if (!ok || limited) {
      m_tabs->setTabToolTip(idx, QString::fromStdString(diskSupportText(d.support, d.fileSystem)) + sysExtra);
    } else {
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
  std::string volumeErr;
  auto disks = uwf::enumerateDisks(&volumeErr);
  if (!volumeErr.empty()) {
    UWF_LOG_W("ui") << "enumerateDisks error: " << volumeErr;
  }
  std::string snapshotErr;
  m_snapshot = uwf::readSnapshot(&snapshotErr);
  if (!m_snapshot.uwfAvailable) {
    // 不弹模态框——GlobalStatusPanel::setUnavailable 的横幅已常驻展示这条
    // 错误，模态框只是重复打扰（且每点一次刷新就再弹一次）。
    UWF_LOG_E("ui") << "readSnapshot failed: uwfAvailable=false err=" << snapshotErr;
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

  rebuildTabs(disks);
  if (uwfAvailable) {
    m_global->setData(m_snapshot.current, m_snapshot.next, m_snapshot.runtime);
    // UWF 可读但未提权：补一条红色"需要管理员权限"横幅。UWF 不可用时不补——
    // 那条不可用横幅优先级更高，已由下面的 setUnavailable 占据同一横幅。
    if (!elevated) m_global->showElevationRequired();
    if (!volumeErr.empty()) m_global->showVolumeInfoWarning(QString::fromStdString(volumeErr));
  } else {
    m_global->setUnavailable(snapshotErr.empty() ? I18n::tr("UWF namespace is not available") : QString::fromStdString(snapshotErr));
  }
  m_overlayPresentation->applySnapshot(m_snapshot);
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
  connect(&dlg, &ApplyPlanDialog::safeRestartRequested, m_power, &PowerController::safeRestart);
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

// 4 个 commit 槽自身只是 dispatcher 的代理——拿到 DiskTab / ExclusionListWidget /
// OverlayFilesDialog / 命令行的转发后，所有 batch 提交流程在 CommitDispatcher
// 里跑（含目标枚举、排除冲突预校验、QProgressDialog、结果对话框）。
void MainWindow::commitFilePath(const QString& path) { m_commit->commitFilePath(path); }
void MainWindow::commitFileDeletionPath(const QString& path) { m_commit->commitFileDeletionPath(path); }
void MainWindow::commitRegistryKey(const QString& key, const QString& valueName) { m_commit->commitRegistryKey(key, valueName); }
void MainWindow::commitRegistryDeletionKey(const QString& key, const QString& valueName) { m_commit->commitRegistryDeletionKey(key, valueName); }

}  // namespace uwf::ui
