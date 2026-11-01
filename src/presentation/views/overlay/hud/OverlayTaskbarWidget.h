// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPointer>
#include <cstdint>
#include <memory>

#include "presentation/views/overlay/coordination/OverlayHubView.h"
#include "presentation/views/overlay/taskbar/TaskbarLayoutCoordinator.h"

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

// Presents supplied usage data beside the primary taskbar notification area.
// TaskbarLayoutCoordinator owns native attachment and Explorer recovery.
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
  bool m_usageAvailable = false;
  bool m_usageStale = false;
  bool m_hasPainted = false;
  bool m_pointerInside = false;
  qreal m_wavePhase = 0;
};

}  // namespace uwf::ui
