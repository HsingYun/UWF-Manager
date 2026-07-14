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
#include <memory>

class QWindow;

namespace uwf::ui {

// Taskbar HubView 内部的原生布局契约。策略负责识别一种任务栏结构并完成
// attach/verify/detach；调用者只按 priority() 选择，不感知具体 Shell 类型。
class TaskbarLayoutStrategy {
 public:
  enum class AttachReadiness { Ready, TemporarilyUnavailable, Unavailable };
  // Incompatible 表示当前进程缺少该策略所需的固定能力；Coordinator 可以继续
  // 尝试低优先级策略，但上层不得把它当作 Shell 瞬态进行定时重试。
  enum class AttachResult { Attached, TemporarilyUnavailable, Incompatible, Invalid };
  // Retained 只表示 attachment 身份仍有效但 Shell 正处于瞬时过渡；它不是
  // attach 失败，也不能用于掩盖本 View 自身的 Qt 可见性或 HWND 身份错误。
  enum class VerificationResult { Confirmed, Retained, RefreshRequired, Invalid };
  enum class DetachResult { Detached, NativeWindowDestroyed, Failed };

  class AttachTransaction {
   public:
    explicit AttachTransaction(const AttachReadiness readiness) : m_readiness(readiness) {}
    virtual ~AttachTransaction() = default;
    [[nodiscard]] AttachReadiness readiness() const { return m_readiness; }
    virtual AttachResult commit() = 0;
    virtual AttachResult finalize() = 0;
    virtual DetachResult rollback() = 0;

   private:
    AttachReadiness m_readiness;
  };

  virtual ~TaskbarLayoutStrategy() = default;

  // 仅判断不随 Explorer 生命周期变化的静态能力，例如系统家族和版本。
  // Shell HWND、窗口树和几何都属于 prepare/verify 的运行时状态，禁止在此
  // 接口中探测。Coordinator 构造后只保留兼容策略。
  [[nodiscard]] virtual bool isCompatible() const = 0;

  // 数值越大优先级越高；相同优先级保持注册顺序。注册后必须保持不变。
  [[nodiscard]] virtual int priority() const = 0;

  // prepareAttach() 创建绑定策略、目标窗口及 Shell 快照的事务；commit()、
  // finalize() 和 rollback() 只能消费该事务，不得重新探测 Shell。commit 或
  // finalize 未完成时，由 Coordinator 调用 rollback()，事务实现必须完整释放
  // 自己留下的半提交状态。
  // logicalSize 使用 96 DPI 逻辑像素，具体策略负责转换和定位。
  // 传 QWindow 而不是裸 WId：策略通过 QWindow::setParent() 同步 Qt 平台关系，
  // 并在原生关系异常时销毁旧平台窗口，禁止继续修补半失效 HWND。
  [[nodiscard]] virtual std::unique_ptr<AttachTransaction> prepareAttach(QWindow* window, const QSize& logicalSize) = 0;
  // currentWindowId 必须由调用方通过不创建平台窗口的方式取得。verify 是纯
  // 健康检查，禁止调用 QWindow::winId() 隐式重建已被宿主销毁的 HWND。
  [[nodiscard]] virtual VerificationResult verify(const QWindow* window, WId currentWindowId) const = 0;
  // 释放本策略拥有的 attachment。身份由策略在 attach 时保存并验证，调用者
  // 不得重新提供窗口，以免将重建后的 QWindow 误代入旧 attachment。
  virtual DetachResult detach() = 0;

  // 宿主已重建。调用者保证不在原生消息栈执行；实现必须先解除自身 QWindow
  // 的 Qt parent，再释放外部宿主包装和恢复快照。
  virtual DetachResult invalidate() = 0;
};

}  // namespace uwf::ui
