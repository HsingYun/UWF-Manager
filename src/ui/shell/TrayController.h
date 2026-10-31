// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 系统托盘图标，共用 HUB 操作菜单并附加覆盖层用量卡片。
//
// "额外的展示与入口"组件，由 MainWindow 创建并编排：
//   - 托盘图标颜色反映最近一次完整提交的 UWF 状态：启用为默认蓝色 logo，禁用
//     / 尚无状态时为红色版本；
//   - 托盘图标被单 / 双击、点击用量卡片 → 发 activateWindowRequested()，
//     MainWindow 接它把主窗口带到前台；
//   - 退出项发出 exitApplicationRequested，由 MainWindow 统一处理退出；
//   - 菜单 aboutToShow 时请求展示控制器刷新；托盘自身不访问 WMI，避免它与
//     Hub 的定时刷新重复读取并提交不一致的半份状态。
//
// 本机无可用系统托盘时不创建任何托盘对象，applyUsageState() 成为空操作。

#include <QAbstractNativeEventFilter>
#include <QIcon>
#include <QObject>
#include <QObjectCleanupHandler>
#include <QPoint>
#include <QPointer>

#include "app/api/OverlayUsage.h"

class QAction;
class QEvent;
class QFrame;
class QLabel;
class QProgressBar;
class QSystemTrayIcon;
class QWidget;
class QTimer;

namespace uwf::ui {

class OverlayContextMenu;

class TrayController : public QObject, public QAbstractNativeEventFilter {
  Q_OBJECT
 public:
  // ownerWindow 只作为右键菜单的 QWidget 父对象；控制器自身由调用方按依赖
  // 顺序拥有，避免同时混用 QObject 父子所有权和 C++ 所有权。
  explicit TrayController(QWidget* ownerWindow, QObject* parent = nullptr);
  ~TrayController() override;

  // 只接收展示控制器已完整读取并提交的状态；读取失败不会调用本函数，旧 UI
  // 因而保持不动。
  void applyUsageState(app::OverlayUsageState state);
  void setHubState(bool available, bool enabled);
  void setOperationAvailability(bool powerAllowed, bool exitAllowed);
 signals:
  // 菜单即将弹出时请求唯一的数据拥有者刷新；本类不自行访问 WMI。
  void refreshRequested();
  // 用户希望把主窗口带到前台（点击托盘图标 / 状态项 / 占用条时发出）。
  void activateWindowRequested();
  void toggleHubRequested();
  void safeShutdownRequested();
  void safeRestartRequested();
  // 用户从托盘菜单请求真正退出应用，MainWindow 统一处理未应用变更与浮窗状态。
  void exitApplicationRequested();

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;
  bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

 private:
  void renderCommittedState();
  void suppressNativeToolTip();
  void showHoverToolTip();

  QIcon m_iconNormal;                 // UWF 启用 / 正常态的托盘图标
  QIcon m_iconAlert;                  // UWF 禁用 / 不可用时的红色托盘图标
  QSystemTrayIcon* m_tray = nullptr;  // 为空 = 本机无可用托盘
  OverlayContextMenu* m_menu = nullptr;
  QAction* m_usageAction = nullptr;
  QWidget* m_usagePane = nullptr;  // 占用条容器；点击 → 展开主窗口
  QWidget* m_usageCard = nullptr;
  QProgressBar* m_usageProgress = nullptr;
  QLabel* m_usageTitle = nullptr;
  QLabel* m_usagePercent = nullptr;
  QLabel* m_usageLabel = nullptr;
  QPointer<QWidget> m_hoverSurface;
  QObjectCleanupHandler m_ownedWidgets;
  QLabel* m_hoverText = nullptr;
  QTimer* m_hoverDelay = nullptr;
  QTimer* m_hoverDismiss = nullptr;
  QPoint m_hoverAnchor;
  quintptr m_callbackWindow = 0;
  unsigned int m_callbackIcon = 0;

  // 托盘是纯展示端：缓存最后一次完整提交的展示模型，语言切换或菜单弹出时
  // 直接从同一模型重绘。WMI 刷新失败不会清空或拼接半份状态。
  app::OverlayUsageState m_usageState{app::OverlayUsageUnavailable{}};
};

}  // namespace uwf::ui
