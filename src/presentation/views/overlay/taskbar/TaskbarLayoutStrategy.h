// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QtGui/qwindowdefs.h>

#include <QSize>
#include <memory>

class QWindow;

namespace uwf::ui {

// Native attachment contract for a taskbar layout, selected by priority.
class TaskbarLayoutStrategy {
 public:
  enum class AttachReadiness { Ready, TemporarilyUnavailable, Unavailable };
  // Incompatible denotes a process capability failure, not a transient shell condition.
  enum class AttachResult { Attached, TemporarilyUnavailable, Incompatible, Invalid };
  // Retained requires a valid attachment identity during a transient shell condition.
  // Widget visibility or HWND identity failures are Invalid.
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

  // Tests static process capabilities; shell identity and geometry belong to prepare/verify.
  [[nodiscard]] virtual bool isCompatible() const = 0;

  // Higher values take priority; ties retain registration order. Stable after registration.
  [[nodiscard]] virtual int priority() const = 0;

  // Captures one window and shell snapshot for commit, finalize and rollback.
  // Unfinished transactions roll back all partial state without rediscovering the shell.
  // logicalSize uses 96-DPI logical pixels; QWindow parenting synchronizes Qt and Win32.
  [[nodiscard]] virtual std::unique_ptr<AttachTransaction> prepareAttach(QWindow* window, const QSize& logicalSize) = 0;
  // currentWindowId must be obtained without creating a platform window.
  // Verification never recreates a destroyed HWND through QWindow::winId().
  [[nodiscard]] virtual VerificationResult verify(const QWindow* window, WId currentWindowId) const = 0;
  // Releases the captured attachment identity, never a replacement QWindow supplied later.
  virtual DetachResult detach() = 0;

  // Called outside the native message stack after host invalidation. Qt parenting is
  // cleared before releasing the external-host wrapper and attachment snapshot.
  virtual DetachResult invalidate() = 0;
};

}  // namespace uwf::ui
