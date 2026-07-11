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

#include <QtGui/qwindowdefs.h>

#include <QSize>

namespace uwf::ui {

// Taskbar HubView 内部的原生布局契约。策略负责识别一种任务栏结构并完成
// attach/verify/detach；调用者只按 priority() 选择，不感知具体 Shell 类型。
class TaskbarLayoutStrategy {
 public:
  virtual ~TaskbarLayoutStrategy() = default;

  // 数值越大优先级越高；相同优先级保持注册顺序。注册后必须保持不变。
  [[nodiscard]] virtual int priority() const = 0;

  // 只探测当前 Shell 是否满足本策略的完整前置条件，不改变任何窗口状态。
  [[nodiscard]] virtual bool available() const = 0;

  // attach() 必须幂等。logicalSize 使用 96 DPI 逻辑像素，具体策略负责按
  // 任务栏 DPI 转换并定位原生窗口。
  virtual bool attach(WId window, const QSize& logicalSize) = 0;
  [[nodiscard]] virtual bool verify(WId window) const = 0;
  virtual void detach(WId window) = 0;

  // 宿主已经重建时只丢弃句柄与恢复快照，不访问旧窗口。与 detach() 分开，
  // 避免未来会修改 Explorer 布局的策略在 TaskbarCreated 后触碰失效句柄。
  virtual void invalidate() = 0;
};

}  // namespace uwf::ui
