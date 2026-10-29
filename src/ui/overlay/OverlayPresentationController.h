// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QPointer>
#include <memory>

#include "app/api/OverlayUsage.h"
#include "core/UwfModel.h"
#include "presentation/overlay/OverlayViewModel.h"

class QAction;
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
  OverlayPresentationController(std::shared_ptr<presentation::OverlayViewModel> viewModel, TrayController& tray, QObject* parent = nullptr);
  ~OverlayPresentationController() override;

  void bindUi(GlobalStatusPanel* global, QAction* displaysAction);
  void unbindUi();
  void applySnapshot(const core::UwfSnapshot& snapshot);

  [[nodiscard]] QTimer* usageTimer() const { return m_viewModel->usageTimer(); }
  [[nodiscard]] bool hubEnabled() const;
  [[nodiscard]] bool hubRequestedVisible() const;
  [[nodiscard]] bool hubPresented() const;

  void setHubEnabled(bool enabled);
  void setOperationAvailability(bool powerAllowed, bool exitAllowed);
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
  void publishUsageState(const app::OverlayUsageState& state);
  void syncAvailability();

  TrayController& m_tray;
  std::unique_ptr<OverlayHub> m_hub;
  std::shared_ptr<presentation::OverlayViewModel> m_viewModel;
  QPointer<GlobalStatusPanel> m_global;
  QPointer<QAction> m_action;
};

}  // namespace uwf::ui
