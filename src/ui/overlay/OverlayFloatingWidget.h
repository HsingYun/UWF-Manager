// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPoint>
#include <cstdint>

#include "OverlayHubView.h"

class QLabel;
class QHideEvent;
class QMoveEvent;
class QPaintEvent;
class QShowEvent;
class QTimer;

namespace uwf::ui {

class OverlayMoveHandle;

// 桌面浮窗只消费展示控制器提供的覆盖层用量，不自行读取 WMI。
class OverlayFloatingWidget final : public OverlayHubView {
  Q_OBJECT
 public:
  explicit OverlayFloatingWidget(QWidget* parent = nullptr);

  [[nodiscard]] int priority() const override { return 100; }
  void applyUsageState(const app::OverlayUsageState& state) override;

 protected:
  void showEvent(QShowEvent* ev) override;
  void hideEvent(QHideEvent* ev) override;
  void moveEvent(QMoveEvent* ev) override;
  void paintEvent(QPaintEvent* ev) override;

 private:
  friend class OverlayMoveHandle;

  void applyHudStyle();
  void refreshText();
  void resizeToContent();
  void updateAnimationTimer();
  [[nodiscard]] VerificationResult verifyPresentation() const override;
  void moveToDefaultPosition();
  void syncHandleGeometry();
  void moveByHandleDrag(const QPoint& globalPos, const QPoint& dragOffset);
  void popupContextMenuAt(const QPoint& globalPos);

  QLabel* m_usage = nullptr;
  OverlayMoveHandle* m_handle = nullptr;
  QTimer* m_animationTimer = nullptr;

  core::OverlayRuntime m_runtime;
  QString m_hoverContent;
  bool m_hasRuntime = false;
  bool m_filterEnabled = false;
  bool m_positionInitialized = false;
  bool m_hasPainted = false;
  qreal m_wavePhase = 0;
};

}  // namespace uwf::ui
