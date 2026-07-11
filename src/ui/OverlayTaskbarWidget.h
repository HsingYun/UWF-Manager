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

#include <QWidget>
#include <cstdint>

#include "../core/UwfModel.h"

class QContextMenuEvent;
class QMouseEvent;
class QPaintEvent;
class QTimer;

namespace uwf::ui {

// 嵌入 Windows 主任务栏通知区域左侧的 overlay 用量窗口。窗口只消费控制器
// 已读取的运行时数据，不自行访问 WMI；宿主 Explorer 重启后会自动重新嵌入。
class OverlayTaskbarWidget final : public QWidget {
  Q_OBJECT
 public:
  enum class DisplayState { Unavailable, Attaching, Confirmed };

  explicit OverlayTaskbarWidget(QWidget* parent = nullptr);

  void updateUsage(const core::OverlayRuntime& runtime);
  void setUnavailable();
  void setFilterEnabled(bool enabled);
  void setTaskbarVisible(bool visible);
  [[nodiscard]] DisplayState displayState() const { return m_displayState; }

 signals:
  void showMainWindowRequested();
  void hideTaskbarViewRequested();
  void exitApplicationRequested();
  void displayStateChanged();

 protected:
  void contextMenuEvent(QContextMenuEvent* ev) override;
  void mouseReleaseEvent(QMouseEvent* ev) override;
  void paintEvent(QPaintEvent* ev) override;

 private:
  bool attachAndPosition();
  bool ensureNativeWindow();
  void refreshHost();
  void setDisplayState(DisplayState state);
  void updateAnimationTimer();
  void updateDisplayConfirmation();
  [[nodiscard]] bool verifyDisplayConfirmation() const;
  [[nodiscard]] int desiredLogicalWidth() const;

  QTimer* m_hostTimer = nullptr;
  QTimer* m_animationTimer = nullptr;
  QTimer* m_confirmationTimer = nullptr;
  core::OverlayRuntime m_runtime;
  bool m_requestedVisible = false;
  bool m_hasRuntime = false;
  bool m_filterEnabled = false;
  bool m_unavailable = false;
  bool m_hasPainted = false;
  bool m_attachmentSucceeded = false;
  DisplayState m_displayState = DisplayState::Unavailable;
  qreal m_wavePhase = 0;
};

}  // namespace uwf::ui
