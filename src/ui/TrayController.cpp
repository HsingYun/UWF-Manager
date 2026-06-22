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
#include "TrayController.h"

#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QCursor>
#include <QEvent>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
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

namespace {

// 把 app.svg 的蓝色渐变替换成红色后渲染成图标——UWF 禁用 / 状态不可用时用它，
// 一眼能看出"没在保护"。保留白色线条与橙点，logo 仍可辨认。失败回退原图标。
QIcon makeAlertIcon() {
  QFile f(QStringLiteral(":/icons/app.svg"));
  if (!f.open(QIODevice::ReadOnly)) return QIcon(QStringLiteral(":/icons/app.svg"));
  QByteArray svg = f.readAll();
  svg.replace("#4C8BF5", "#E15A4C");  // 渐变亮端：蓝 → 红
  svg.replace("#1A5DC7", "#B4271C");  // 渐变暗端：蓝 → 红
  QSvgRenderer renderer(svg);
  if (!renderer.isValid()) return QIcon(QStringLiteral(":/icons/app.svg"));
  QIcon icon;
  for (const int sz : {16, 20, 24, 32, 48, 64}) {
    QPixmap pm(sz, sz);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    renderer.render(&p);
    p.end();
    icon.addPixmap(pm);
  }
  return icon;
}

}  // namespace

TrayController::TrayController(WmiSession& session, QWidget* ownerWindow)
    : QObject(ownerWindow), m_session(session), m_iconNormal(QStringLiteral(":/icons/app.svg")), m_iconAlert(makeAlertIcon()) {
  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    UWF_LOG_W("ui") << "system tray unavailable; tray icon will not be created";
    return;
  }

  // 托盘右键菜单：① UWF 状态  ② 覆盖层占用条  ③ 退出。菜单父对象取 ownerWindow。
  m_menu = new QMenu(ownerWindow);

  // 菜单项 1：UWF 启用状态。普通 QAction——自动获得菜单 hover 高亮；文本由
  // refreshUsage 按当前状态刷新；点击 → 展开主窗口。
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
  m_exitAction = m_menu->addAction(I18n::tr("Exit"));
  connect(m_exitAction, &QAction::triggered, qApp, &QApplication::quit);

  // 菜单弹出前即时刷新状态与占用。
  connect(m_menu, &QMenu::aboutToShow, this, &TrayController::refreshUsage);

  m_tray = new QSystemTrayIcon(m_iconNormal, this);
  // tooltip 文案由构造末尾的 refreshUsage() 统一设置（它每轮刷新都会重译）。
  // 不用 setContextMenu——它在 Windows 上按托盘图标 geometry 定位菜单，而该
  // geometry 常不可靠，菜单会弹到奇怪位置。改为右键时自己按光标位置 popup：
  // QMenu::popup 会做屏幕边缘适配，靠近屏幕底部（任务栏在下）时自动向上弹。
  connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
      emit activateWindowRequested();  // 单 / 双击托盘图标 → 还原主窗口
    else if (reason == QSystemTrayIcon::Context)
      m_menu->popup(QCursor::pos());  // 右键 → 在光标处弹出菜单
  });
  m_tray->show();

  refreshUsage();  // 启动即按当前 UWF 状态摆正图标颜色与菜单
}

void TrayController::refreshUsage() {
  if (!m_tray) return;

  // 退出项与图标 tooltip 是静态文案：切换语言时 MainWindow 只重建主窗口、
  // 不重建托盘，故在这里随刷新一并重译——与状态项 / 占用条共用同一刷新周期，
  // 不另设重译入口。
  m_exitAction->setText(I18n::tr("Exit"));
  m_tray->setToolTip(I18n::tr("Unified Write Filter (UWF) Manager"));

  // 占用条与其上方的分隔线一起显隐——避免占用条隐藏后剩下两条相邻分隔线。
  const auto setUsageVisible = [this](bool show) {
    m_usageAction->setVisible(show);
    m_usageSeparator->setVisible(show);
  };

  // UWF 当前会话的启用状态——决定托盘图标颜色与状态项文字。
  const auto filter = api::UwfFilter{m_session}.read();
  const bool enabled = filter && filter->currentEnabled;
  m_tray->setIcon(enabled ? m_iconNormal : m_iconAlert);  // 禁用 / 读不到 → 红色图标

  if (!filter) {
    m_stateAction->setText(I18n::tr("UWF status unavailable"));
    setUsageVisible(false);
    return;
  }
  m_stateAction->setText(enabled ? I18n::tr("UWF: Enabled") : I18n::tr("UWF: Disabled"));

  // 菜单项 2：占用条仅在 UWF 已启用时存在；禁用（或读不到 overlay 配置）时隐藏。
  if (!enabled) {
    setUsageVisible(false);
    return;
  }
  const auto overlay = api::UwfOverlay{m_session}.read();
  const auto cfg = api::UwfOverlayConfig{m_session}.read(/*currentSession=*/true);
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
