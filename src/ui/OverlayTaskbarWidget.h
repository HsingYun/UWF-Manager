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

#include <cstdint>
#include <memory>

#include "OverlayHubView.h"

class QContextMenuEvent;
class QEnterEvent;
class QEvent;
class QLabel;
class QMouseEvent;
class QPaintEvent;
class QTimer;

namespace uwf::ui {

class TaskbarLayoutCoordinator;

// 嵌入 Windows 主任务栏通知区域左侧的 overlay 用量窗口。窗口只消费控制器
// 已读取的运行时数据，不自行访问 WMI，也不感知具体任务栏实现；布局选择、
// 原生附着和 Explorer 恢复全部委托给 TaskbarLayoutCoordinator。
class OverlayTaskbarWidget final : public OverlayHubView {
  Q_OBJECT
 public:
  explicit OverlayTaskbarWidget(QWidget* parent = nullptr);
  ~OverlayTaskbarWidget() override;

  [[nodiscard]] int priority() const override { return 200; }
  void updateUsage(const core::OverlayRuntime& runtime) override;
  void setUsageUnavailable() override;
  void setFilterEnabled(bool enabled) override;

 protected:
  void contextMenuEvent(QContextMenuEvent* ev) override;
  void enterEvent(QEnterEvent* ev) override;
  void leaveEvent(QEvent* ev) override;
  void mouseReleaseEvent(QMouseEvent* ev) override;
  void paintEvent(QPaintEvent* ev) override;

 private:
  bool attachPresentation() override;
  [[nodiscard]] bool verifyPresentation() const override;
  void detachPresentation() override;
  [[nodiscard]] int healthCheckIntervalMs() const override { return 1000; }

  bool ensureNativeWindow();
  void showToolTip();
  void hideToolTip();
  void releaseInvalidNativeWindow();
  void releaseNativeWindow();
  void updateAnimationTimer();
  [[nodiscard]] int desiredLogicalWidth() const;

  QTimer* m_animationTimer = nullptr;
  QTimer* m_toolTipTimer = nullptr;
  std::unique_ptr<TaskbarLayoutCoordinator> m_layoutCoordinator;
  std::unique_ptr<QLabel> m_toolTipLabel;
  core::OverlayRuntime m_runtime;
  bool m_hasRuntime = false;
  bool m_filterEnabled = false;
  bool m_hasPainted = false;
  bool m_pointerInside = false;
  qreal m_wavePhase = 0;
};

}  // namespace uwf::ui
