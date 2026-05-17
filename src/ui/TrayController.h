#pragma once

// 系统托盘图标 + 右键菜单（① UWF 状态 ② 覆盖层占用条 ③ 退出）。
//
// "额外的展示与入口"组件，由 MainWindow 创建并编排：
//   - 托盘图标被单 / 双击、点击状态项或占用条 → 发 activateWindowRequested()，
//     MainWindow 接它把主窗口带到前台；
//   - 退出项直接 QApplication::quit()；
//   - 菜单 aboutToShow 时自动重读 UWF 状态刷新菜单；MainWindow 的 5s 定时器
//     另调 refreshUsage() 让菜单显示期间也保持实时。
//
// 本机无可用系统托盘时不创建任何托盘对象，refreshUsage() 成为空操作。

#include <QObject>

class QAction;
class QEvent;
class QLabel;
class QMenu;
class QSystemTrayIcon;
class QWidget;

namespace uwf {
class WmiSession;
}

namespace uwf::ui {

class OverlayUsageBar;

class TrayController : public QObject {
  Q_OBJECT
 public:
  // session 用于只读查询 UWF_Filter / UWF_Overlay / UWF_OverlayConfig；
  // ownerWindow 同时作为本对象与右键菜单的父对象。两者生命周期须覆盖本对象。
  TrayController(WmiSession& session, QWidget* ownerWindow);

  // 重读 UWF 状态与占用刷新菜单——仅在右键菜单正在显示时才真正读取。
  void refreshUsage();

 signals:
  // 用户希望把主窗口带到前台（点击托盘图标 / 状态项 / 占用条时发出）。
  void activateWindowRequested();

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  // 无条件重读 UWF 状态与占用并刷新菜单项（菜单 aboutToShow 时调用）。
  void updateUsage();

  WmiSession& m_session;
  QSystemTrayIcon* m_tray = nullptr;    // 为空 = 本机无可用托盘
  QMenu* m_menu = nullptr;
  QAction* m_stateAction = nullptr;     // 菜单项 1：UWF 启用状态
  QAction* m_usageSeparator = nullptr;  // 状态项与占用条间的分隔线（随占用条显隐）
  QAction* m_usageAction = nullptr;     // 菜单项 2：占用条（UWF 禁用时隐藏）
  QWidget* m_usagePane = nullptr;       // 占用条容器；点击 → 展开主窗口
  OverlayUsageBar* m_usageBar = nullptr;
  QLabel* m_usageLabel = nullptr;       // 占用条下方的"已用 / 总计"文字
};

}  // namespace uwf::ui
