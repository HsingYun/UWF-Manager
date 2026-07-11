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
class OverlayFloatingWidget;
class TrayController;

// Overlay 的展示编排：周期读取运行时用量、悬浮窗生命周期与显示偏好、
// 工具栏开关状态及托盘刷新。MainWindow 只在 UI 重建后绑定面板和 QAction。
class OverlayPresentationController : public QObject {
  Q_OBJECT
 public:
  OverlayPresentationController(WmiSession& session, QMainWindow* ownerWindow, TrayController* tray, QObject* parent = nullptr);
  ~OverlayPresentationController() override;

  void bindUi(GlobalStatusPanel* global, QAction* floatingAction);
  void unbindUi();
  void applySnapshot(const core::UwfSnapshot& snapshot);

  [[nodiscard]] QTimer* usageTimer() const { return m_usageTimer; }
  [[nodiscard]] bool floatingVisible() const;

  void setFloatingVisible(bool visible);
  void hideFloatingTemporarily();
  void refreshActionIcon();

 signals:
  void activateMainWindowRequested();
  void exitApplicationRequested();

 private:
  void refreshUsage();
  void syncAvailability();

  QMainWindow* m_ownerWindow;
  TrayController* m_tray;
  OverlayFloatingWidget* m_floating;
  QTimer* m_usageTimer;
  QPointer<GlobalStatusPanel> m_global;
  QPointer<QAction> m_action;

  api::UwfFilter m_filter;
  api::UwfOverlay m_overlay;
  bool m_floatingAllowed = false;
  bool m_floatingRequested = true;
};

}  // namespace uwf::ui
