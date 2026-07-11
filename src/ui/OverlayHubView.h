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

#include "../core/UwfModel.h"

class QTimer;

namespace uwf::ui {

// Hub 可编排的统一展示端点。实现类负责自身窗口生命周期、展示确认和恢复；
// Hub 只依赖此契约，按 priority() 从高到低选择第一个可用端点。
class OverlayHubView : public QWidget {
  Q_OBJECT
 public:
  enum class DisplayState { Unavailable, Attaching, Confirmed };

  explicit OverlayHubView(QWidget* parent = nullptr, Qt::WindowFlags flags = {});
  ~OverlayHubView() override = default;

  // 数值越大，展示优先级越高；相同优先级保持注册顺序。注册后优先级必须
  // 保持不变。
  [[nodiscard]] virtual int priority() const = 0;

  // 数据更新必须是被动的：可以更新缓存、隐藏控件的文字和几何，但不得显示
  // View、创建宿主资源或改变 DisplayState。Hub 会向休眠 View 同步最新数据，
  // 让其被选中时无需等待下一次采样。
  virtual void updateUsage(const core::OverlayRuntime&) {}
  virtual void setUsageUnavailable() {}
  virtual void setFilterEnabled(bool) {}

  // 非虚模板方法：基类统一编排 attach、确认超时、重试、健康检查和 detach。
  void setPresentationRequested(bool requested);

  [[nodiscard]] DisplayState displayState() const { return m_displayState; }
  [[nodiscard]] bool presentationVerified() const;

 signals:
  void showMainWindowRequested();
  void hideViewRequested();
  void exitApplicationRequested();
  void displayStateChanged();

 protected:
  // attachPresentation() 必须幂等。返回 false 表示当前展示条件不满足；返回
  // true 后基类会调用 verifyPresentation()，并根据结果进入 Attaching/Confirmed。
  // 普通 QWidget 默认只需 show/hide；宿主型 View 可覆盖三者管理外部资源。
  virtual bool attachPresentation();
  [[nodiscard]] virtual bool verifyPresentation() const = 0;
  virtual void detachPresentation();
  [[nodiscard]] virtual int healthCheckIntervalMs() const { return 0; }
  [[nodiscard]] virtual int confirmationTimeoutMs() const { return 300; }
  [[nodiscard]] virtual int retryIntervalMs() const { return 1000; }

  [[nodiscard]] bool presentationRequested() const { return m_presentationRequested; }
  void requestPresentationRefresh();
  void notifyPresentationChanged();

 private:
  void refreshPresentation();
  void transitionToAttaching();
  void transitionToConfirmed();
  void transitionToUnavailable();
  void setDisplayState(DisplayState state);

  QTimer* m_confirmationTimer;
  QTimer* m_retryTimer;
  QTimer* m_healthTimer;
  bool m_presentationRequested = false;
  DisplayState m_displayState = DisplayState::Unavailable;
};

}  // namespace uwf::ui
