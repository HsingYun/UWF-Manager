#include "TrayController.h"

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QWidget>
#include <QWidgetAction>
#include <algorithm>

#include "../util/Log.h"
#include "../uwf/api/UwfFilter.h"
#include "../uwf/api/UwfOverlay.h"
#include "../uwf/api/UwfOverlayConfig.h"
#include "../uwf/wmi/WmiClient.h"
#include "I18n.h"
#include "OverlayUsageBar.h"

namespace uwf::ui {

TrayController::TrayController(WmiSession& session, QWidget* ownerWindow) : QObject(ownerWindow), m_session(session) {
  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    UWF_LOG_W("ui") << "system tray unavailable; tray icon will not be created";
    return;
  }

  // 托盘右键菜单：① UWF 状态  ② 覆盖层占用条  ③ 退出。菜单父对象取 ownerWindow。
  m_menu = new QMenu(ownerWindow);

  // 菜单项 1：UWF 启用状态。普通 QAction——自动获得菜单 hover 高亮；文本由
  // updateUsage 按当前状态刷新；点击 → 展开主窗口。
  m_stateAction = m_menu->addAction(QString{});
  connect(m_stateAction, &QAction::triggered, this, &TrayController::activateWindowRequested);

  // 状态项与下方菜单之间的分隔线——随占用条一起显隐，避免占用条隐藏后留下双分隔线。
  m_usageSeparator = m_menu->addSeparator();

  // 菜单项 2：覆盖层占用条。自绘控件须经 QWidgetAction 放进菜单——而 QWidgetAction
  // 的内嵌控件不会自动获得 QMenu 的 hover 高亮，故容器取 objectName=trayUsageItem，
  // 由 QSS 配一条与 QMenu::item:selected 一致的 :hover 规则补上高亮。
  m_usagePane = new QWidget(m_menu);
  m_usagePane->setObjectName("trayUsageItem");
  m_usagePane->setAttribute(Qt::WA_Hover, true);             // 让 QSS :hover 生效
  m_usagePane->setAttribute(Qt::WA_StyledBackground, true);  // 让 QSS 背景生效
  m_usagePane->setCursor(Qt::PointingHandCursor);
  m_usagePane->installEventFilter(this);  // 点击 → eventFilter 里发 activateWindowRequested
  auto* lay = new QVBoxLayout(m_usagePane);
  lay->setContentsMargins(26, 6, 18, 6);  // 左右对齐普通菜单项的文字 padding
  lay->setSpacing(8);                     // 占用条与下方文字之间留一点间隔
  m_usageBar = new OverlayUsageBar(m_usagePane);
  m_usageBar->setFixedWidth(190);
  m_usageLabel = new QLabel(m_usagePane);  // 占用条下方的"已用 / 总计"
  lay->addWidget(m_usageBar);
  lay->addWidget(m_usageLabel);

  auto* usageAction = new QWidgetAction(m_menu);
  usageAction->setDefaultWidget(m_usagePane);
  m_menu->addAction(usageAction);
  m_usageAction = usageAction;

  m_menu->addSeparator();

  // 菜单项 3：退出。
  QAction* exitAction = m_menu->addAction(I18n::tr("Exit"));
  connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

  // 菜单弹出前即时刷新状态与占用。
  connect(m_menu, &QMenu::aboutToShow, this, &TrayController::updateUsage);

  m_tray = new QSystemTrayIcon(QIcon(":/icons/app.svg"), this);
  m_tray->setToolTip(I18n::tr("Unified Write Filter (UWF) Manager"));
  // 不用 setContextMenu——它在 Windows 上按托盘图标 geometry 定位菜单，而该
  // geometry 常不可靠，菜单会弹到奇怪位置。改为右键时自己按光标位置 popup：
  // QMenu::popup 会做屏幕边缘适配，靠近屏幕底部（任务栏在下）时自动向上弹。
  connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
      emit activateWindowRequested();   // 单 / 双击托盘图标 → 还原主窗口
    else if (reason == QSystemTrayIcon::Context)
      m_menu->popup(QCursor::pos());    // 右键 → 在光标处弹出菜单
  });
  m_tray->show();
}

void TrayController::refreshUsage() {
  // 仅当右键菜单正在显示时才重读——菜单没开时占用条不可见，刷新无意义。
  if (m_menu && m_menu->isVisible()) updateUsage();
}

void TrayController::updateUsage() {
  if (!m_tray) return;  // 本机无托盘

  // 占用条与其上方的分隔线一起显隐——避免占用条隐藏后剩下两条相邻分隔线。
  const auto setUsageVisible = [this](bool show) {
    m_usageAction->setVisible(show);
    m_usageSeparator->setVisible(show);
  };

  // 菜单项 1：UWF 当前会话的启用状态。读不到（命名空间不可用）就只显示这一行。
  const auto filter = UwfFilter{m_session}.read();
  if (!filter) {
    m_stateAction->setText(I18n::tr("UWF status unavailable"));
    setUsageVisible(false);
    return;
  }
  const bool enabled = filter->currentEnabled;
  m_stateAction->setText(enabled ? I18n::tr("UWF: Enabled") : I18n::tr("UWF: Disabled"));

  // 菜单项 2：占用条仅在 UWF 已启用时存在；禁用（或读不到 overlay 配置）时隐藏。
  if (!enabled) {
    setUsageVisible(false);
    return;
  }
  const auto overlay = UwfOverlay{m_session}.read();
  const auto cfg = UwfOverlayConfig{m_session}.read(/*currentSession=*/true);
  if (!overlay || !cfg) {
    setUsageVisible(false);
    return;
  }
  setUsageVisible(true);
  const uint32_t used = overlay->overlayConsumption;
  const uint32_t maxMb = static_cast<uint32_t>(std::max(0, cfg->maximumSize));
  const bool isRam = cfg->type == api::OverlayType::RAM;
  m_usageBar->setOverlayData(used, overlay->warningOverlayThreshold, overlay->criticalOverlayThreshold, maxMb, isRam);
  // "已用 / 总计"——总计 = 已用 + 可用，与主面板的 used 标签口径一致。
  m_usageLabel->setText(I18n::tr("Used %1 MB / Total %2 MB").arg(used).arg(overlay->availableSpace + used));
}

bool TrayController::eventFilter(QObject* obj, QEvent* ev) {
  // 点击占用条面板 → 关闭菜单并请求展开主窗口。
  if (obj == m_usagePane && ev->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent*>(ev)->button() == Qt::LeftButton) {
    m_menu->close();
    emit activateWindowRequested();
    return true;
  }
  return QObject::eventFilter(obj, ev);
}

}  // namespace uwf::ui
