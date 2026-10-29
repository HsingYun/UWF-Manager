// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPointer>
#include <cstdint>
#include <memory>

#include "OverlayHubView.h"
#include "ui/overlay/taskbar/TaskbarLayoutCoordinator.h"

class QContextMenuEvent;
class QEnterEvent;
class QEvent;
class QLabel;
class QMenu;
class QMouseEvent;
class QPaintEvent;
class QTimer;
class QWindow;

namespace uwf::ui {
class HoverToolTip;

// 嵌入 Windows 主任务栏通知区域左侧的 overlay 用量窗口。窗口只消费控制器
// 已读取的运行时数据，不自行访问 WMI，也不感知具体任务栏实现；布局选择、
// 原生附着和 Explorer 恢复全部委托给 TaskbarLayoutCoordinator。
class OverlayTaskbarWidget final : public OverlayHubView {
  Q_OBJECT
 public:
  explicit OverlayTaskbarWidget(QWidget* parent = nullptr);
  ~OverlayTaskbarWidget() override;

  [[nodiscard]] bool isCompatible() const override;
  [[nodiscard]] int priority() const override { return 200; }
  void applyUsageState(const app::OverlayUsageState& state) override;

 protected:
  void contextMenuEvent(QContextMenuEvent* ev) override;
  void enterEvent(QEnterEvent* ev) override;
  void leaveEvent(QEvent* ev) override;
  void mouseReleaseEvent(QMouseEvent* ev) override;
  void paintEvent(QPaintEvent* ev) override;

 private:
  AttachResult acquirePresentation() override;
  AttachResult activatePresentation() override;
  [[nodiscard]] VerificationResult verifyPresentation() const override;
  void suspendPresentation() override;
  ReleaseResult detachPresentation(ReleaseReason reason) override;
  [[nodiscard]] int healthCheckIntervalMs() const override { return 1000; }
  [[nodiscard]] int retryIntervalMs(int consecutiveFailures) const override;

  void showToolTip();
  void hideToolTip();
  void closeContextMenu();
  [[nodiscard]] QWindow* recreateNativeWindow();
  void handleLayoutDetachEvent(const TaskbarLayoutCoordinator::DetachEvent& event);
  void synchronizeContentGeometry();
  void updateAnimationTimer();
  [[nodiscard]] int desiredLogicalWidth() const;

  QTimer* m_animationTimer = nullptr;
  QTimer* m_toolTipTimer = nullptr;
  std::unique_ptr<TaskbarLayoutCoordinator> m_layoutCoordinator;
  std::unique_ptr<HoverToolTip> m_toolTipLabel;
  QString m_hoverContent;
  QPointer<QMenu> m_contextMenu;
  core::OverlayRuntime m_runtime;
  bool m_hasRuntime = false;
  bool m_filterEnabled = false;
  bool m_hasPainted = false;
  bool m_pointerInside = false;
  qreal m_wavePhase = 0;
};

}  // namespace uwf::ui
