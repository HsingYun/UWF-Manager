// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "presentation/controls/navigation/CommandMenu.h"

class QAction;
class QWidgetAction;

namespace uwf::ui {

// Shared tray and HUD actions; position restoration is available only for the floating HUD.
class OverlayContextMenu final : public CommandMenu {
  Q_OBJECT
 public:
  explicit OverlayContextMenu(QWidget* parent = nullptr);

  void setRestorePositionAvailable(bool available);
  void setHubState(bool available, bool enabled);
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
  bool m_powerAllowed = false;
  bool m_exitAllowed = true;
};

}  // namespace uwf::ui
