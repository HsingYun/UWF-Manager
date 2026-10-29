// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <windows.h>

#include <QPointer>
#include <QSize>
#include <QWindow>
#include <memory>
#include <optional>
#include <string>

#include "TaskbarLayoutStrategy.h"
#include "Win11TaskbarEnvironment.h"
#include "Win11TaskbarLayoutStrategy.h"

namespace uwf::ui {

constexpr wchar_t kWin11TaskbarAttachmentCookieProperty[] = L"UWF.TaskbarHub.AttachmentCookie";

struct Win11TaskbarLayoutStrategy::Impl {
  struct Attachment {
    win11_taskbar::Environment environment;
    QPointer<QWindow> window;
    std::unique_ptr<QWindow> taskbarWindow;
    WId windowId = 0;
    QPointer<QWindow> originalParent;
    QPointer<QObject> originalObjectParent;
    HWND originalNativeParent = nullptr;
    LONG_PTR originalStyle = 0;
    LONG_PTR originalExStyle = 0;
    HANDLE cookie = nullptr;
    QSize logicalSize;
  };
  std::optional<Attachment> attachment;
  bool hardResetRequired = false;
  // 只有一次完整的 commit + visible finalize 成功后，才证明当前进程支持
  // layered child。此后同一属性丢失属于可恢复的 HWND 状态损坏，而不是能力变化。
  bool layeredChildCapabilityConfirmed = false;
  mutable TaskbarLayoutStrategy::VerificationResult lastDiagnosticResult = TaskbarLayoutStrategy::VerificationResult::Confirmed;
  mutable std::string lastDiagnosticReason;
};

}  // namespace uwf::ui
