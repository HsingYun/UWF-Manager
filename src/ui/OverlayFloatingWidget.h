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

#include <QPoint>
#include <cstdint>

#include "OverlayHubView.h"

class QLabel;
class QContextMenuEvent;
class QHideEvent;
class QMouseEvent;
class QMoveEvent;
class QPaintEvent;
class QShowEvent;
class QTimer;

namespace uwf::ui {

class OverlayMoveHandle;

// 桌面浮窗：按需显示当前 overlay 用量。窗口本身不读取 WMI，
// 只接收 MainWindow 刷新周期喂进来的状态，避免多一条独立读取链。
class OverlayFloatingWidget final : public OverlayHubView {
  Q_OBJECT
 public:
  explicit OverlayFloatingWidget(QWidget* parent = nullptr);

  [[nodiscard]] int priority() const override { return 100; }
  void updateUsage(const core::OverlayRuntime& runtime) override;
  void setUsageUnavailable() override;
  void setFilterEnabled(bool enabled) override;

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
  bool m_hasRuntime = false;
  bool m_filterEnabled = false;
  bool m_positionInitialized = false;
  bool m_hasPainted = false;
  qreal m_wavePhase = 0;
};

}  // namespace uwf::ui
