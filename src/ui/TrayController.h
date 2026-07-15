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
#pragma once

// 系统托盘图标 + 右键菜单（① UWF 状态 ② 覆盖层占用条 ③ 退出）。
//
// "额外的展示与入口"组件，由 MainWindow 创建并编排：
//   - 托盘图标颜色反映最近一次完整提交的 UWF 状态：启用为默认蓝色 logo，禁用
//     / 尚无状态时为红色版本；
//   - 托盘图标被单 / 双击、点击状态项或占用条 → 发 activateWindowRequested()，
//     MainWindow 接它把主窗口带到前台；
//   - 退出项发出 exitApplicationRequested，由 MainWindow 统一处理退出；
//   - 菜单 aboutToShow 时请求展示控制器刷新；托盘自身不访问 WMI，避免它与
//     Hub 的定时刷新重复读取并提交不一致的半份状态。
//
// 本机无可用系统托盘时不创建任何托盘对象，applyUsageState() 成为空操作。

#include <QIcon>
#include <QObject>
#include <cstdint>
#include <optional>

#include "../core/UwfModel.h"

class QAction;
class QEvent;
class QLabel;
class QMenu;
class QSystemTrayIcon;
class QWidget;

namespace uwf::ui {

class OverlayUsageBar;

class TrayController : public QObject {
  Q_OBJECT
 public:
  // ownerWindow 同时作为本对象与右键菜单的父对象，生命周期须覆盖本对象。
  explicit TrayController(QWidget* ownerWindow);

  // 只接收展示控制器已完整读取并提交的状态。filterEnabled=true 时 runtime
  // 必须有值；读取失败不会调用本函数，旧 UI 因而保持不动。
  void applyUsageState(bool filterEnabled, const std::optional<core::OverlayRuntime>& runtime, const core::OverlayConfig& config);
 signals:
  // 菜单即将弹出时请求唯一的数据拥有者刷新；本类不自行访问 WMI。
  void refreshRequested();
  // 用户希望把主窗口带到前台（点击托盘图标 / 状态项 / 占用条时发出）。
  void activateWindowRequested();
  // 用户从托盘菜单请求真正退出应用，MainWindow 统一处理未应用变更与浮窗状态。
  void exitApplicationRequested();

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  void renderCommittedState();

  QIcon m_iconNormal;                 // UWF 启用 / 正常态的托盘图标
  QIcon m_iconAlert;                  // UWF 禁用 / 不可用时的红色托盘图标
  QSystemTrayIcon* m_tray = nullptr;  // 为空 = 本机无可用托盘
  QMenu* m_menu = nullptr;
  QAction* m_stateAction = nullptr;     // 菜单项 1：UWF 启用状态
  QAction* m_exitAction = nullptr;      // 菜单项 3：退出（随提交状态 / 菜单弹出重译）
  QAction* m_usageSeparator = nullptr;  // 状态项与占用条间的分隔线（随占用条显隐）
  QAction* m_usageAction = nullptr;     // 菜单项 2：占用条（UWF 禁用时隐藏）
  QWidget* m_usagePane = nullptr;       // 占用条容器；点击 → 展开主窗口
  OverlayUsageBar* m_usageBar = nullptr;
  QLabel* m_usageLabel = nullptr;  // 占用条下方的"已用 / 总计"文字

  // 托盘是纯展示端：缓存最后一次完整提交的展示模型，语言切换或菜单弹出时
  // 直接从同一模型重绘。WMI 刷新失败不会清空或拼接半份状态。
  bool m_hasCommittedState = false;
  bool m_filterEnabled = false;
  std::optional<core::OverlayRuntime> m_runtime;
  core::OverlayConfig m_config;
};

}  // namespace uwf::ui
