// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "ui/widgets/CommandMenu.h"

class QAction;
class QWidgetAction;

namespace uwf::ui {

// 托盘、任务栏 HUB 与浮窗 HUB 共用的操作和顺序。位置恢复只在浮窗中显示。
class OverlayContextMenu final : public CommandMenu {
  Q_OBJECT
 public:
  explicit OverlayContextMenu(QWidget* parent = nullptr);

  void setRestorePositionAvailable(bool available);
  void setHubState(bool available, bool enabled);
  void setPowerActionsEnabled(bool enabled);
  void setOperationAvailability(bool powerAllowed, bool exitAllowed);
  QWidgetAction* addUsageWidget(QWidget* widget);
  void execAt(const QPoint& position);
  void retranslate();

 signals:
  void showMainWindowRequested();
  void restorePositionRequested();
  void toggleHubRequested();
  void safeShutdownRequested();
  void safeRestartRequested();
  void exitApplicationRequested();

 private:
  void dispatch(QAction* action);
  void updateActionAvailability();
  QAction* m_title = nullptr;
  QAction* m_showMain = nullptr;
  QAction* m_restorePosition = nullptr;
  QAction* m_toggleHub = nullptr;
  QAction* m_shutdown = nullptr;
  QAction* m_restart = nullptr;
  QAction* m_exit = nullptr;
  bool m_hubEnabled = true;
  bool m_deferSignals = false;
  bool m_powerAllowed = true;
  bool m_exitAllowed = true;
  bool m_powerEnabled = true;
};

}  // namespace uwf::ui
