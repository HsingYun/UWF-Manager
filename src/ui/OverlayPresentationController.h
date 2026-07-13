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

#include <QObject>
#include <QPointer>

#include "../core/UwfModel.h"
#include "../uwf/api/UwfFilter.h"
#include "../uwf/api/UwfOverlay.h"

class QAction;
class QMainWindow;
class QTimer;

namespace uwf::ui {

class GlobalStatusPanel;
class OverlayHub;
class TrayController;

// Overlay 的展示编排：周期读取运行时用量、Hub 生命周期、工具栏开关状态
// 及托盘刷新。具体视图的互备由 OverlayHub 内部封装，MainWindow 不感知。
class OverlayPresentationController : public QObject {
  Q_OBJECT
 public:
  OverlayPresentationController(WmiSession& session, QMainWindow* ownerWindow, TrayController* tray, QObject* parent = nullptr);
  ~OverlayPresentationController() override;

  void bindUi(GlobalStatusPanel* global, QAction* displaysAction);
  void unbindUi();
  void applySnapshot(const core::UwfSnapshot& snapshot);

  [[nodiscard]] QTimer* usageTimer() const { return m_usageTimer; }
  [[nodiscard]] bool hubEnabled() const;
  [[nodiscard]] bool hubPresented() const;

  void setHubEnabled(bool enabled);
  void hideHubTemporarily();
  void restoreHub();
  void refreshActionIcon();

 signals:
  void activateMainWindowRequested();
  void safeShutdownRequested();
  void safeRestartRequested();
  void exitApplicationRequested();

 private:
  void refreshUsage();
  void syncAvailability();

  QMainWindow* m_ownerWindow;
  TrayController* m_tray;
  OverlayHub* m_hub;
  QTimer* m_usageTimer;
  QPointer<GlobalStatusPanel> m_global;
  QPointer<QAction> m_action;

  api::UwfFilter m_filter;
  api::UwfOverlay m_overlay;
};

}  // namespace uwf::ui
