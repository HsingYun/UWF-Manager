/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "Win11TaskbarLayoutStrategy.h"

#include <dwmapi.h>
#include <windows.h>

#include <QPointer>
#include <QWindow>
#include <atomic>
#include <memory>
#include <string>

#include "../util/Log.h"
#include "../util/WindowsVersion.h"
#include "Win11TaskbarEnvironment.h"
#include "Win11TaskbarLayoutStrategyImpl.h"

namespace uwf::ui {

namespace {

constexpr wchar_t kAttachmentCookieProperty[] = L"UWF.TaskbarHub.AttachmentCookie";

using win11_taskbar::calculatePlacement;
using win11_taskbar::Environment;
using win11_taskbar::EnvironmentProbe;
using win11_taskbar::matchesPlacement;
using win11_taskbar::Placement;
using win11_taskbar::probeEnvironment;
using win11_taskbar::refreshEnvironment;
using win11_taskbar::RuntimeAvailability;
using win11_taskbar::sameEnvironment;

bool setWindowLongChecked(const HWND window, const int index, const LONG_PTR value) {
  SetLastError(ERROR_SUCCESS);
  const LONG_PTR previous = SetWindowLongPtrW(window, index, value);
  return previous != 0 || GetLastError() == ERROR_SUCCESS;
}

bool restoreWindowStyles(const HWND window, const LONG_PTR style, const LONG_PTR exStyle) {
  if (GetWindowLongPtrW(window, GWL_STYLE) != style && !setWindowLongChecked(window, GWL_STYLE, style)) return false;
  if (GetWindowLongPtrW(window, GWL_EXSTYLE) != exStyle && !setWindowLongChecked(window, GWL_EXSTYLE, exStyle)) return false;
  return SetWindowPos(window, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER) != FALSE;
}

bool restoreQtParents(QWindow* const window, QWindow* const visualParent, QObject* const objectParent) {
  if (!window) return false;
  window->setParent(visualParent);
  if (window->QObject::parent() != objectParent) window->QObject::setParent(objectParent);
  return window->parent() == visualParent && window->QObject::parent() == objectParent;
}

HANDLE nextAttachmentCookie() {
  static std::atomic<ULONG_PTR> nextCookie{1};
  ULONG_PTR cookie = nextCookie.fetch_add(1, std::memory_order_relaxed);
  if (cookie == 0) cookie = nextCookie.fetch_add(1, std::memory_order_relaxed);
  return reinterpret_cast<HANDLE>(cookie);
}

bool ownsNativeWindow(const HWND window, const HANDLE cookie = nullptr) {
  DWORD processId = 0;
  if (!IsWindow(window)) return false;
  GetWindowThreadProcessId(window, &processId);
  return processId == GetCurrentProcessId() && (!cookie || GetPropW(window, kAttachmentCookieProperty) == cookie);
}

HWND ownedWindow(const WId id, const HANDLE cookie = nullptr) {
  if (!id) return nullptr;
  const HWND window = reinterpret_cast<HWND>(id);
  return ownsNativeWindow(window, cookie) ? window : nullptr;
}

struct InjectedStateObservation {
  bool parentMatches = false;
  bool child = false;
  bool popup = false;
  bool layered = false;
  bool toolWindow = false;
  bool noActivate = false;
  bool appWindow = false;

  enum class Classification { Valid, LayeredChildUnsupported, Invalid };

  [[nodiscard]] Classification classify() const {
    if (!parentMatches || !child || popup || !noActivate || appWindow) return Classification::Invalid;
    return layered ? Classification::Valid : Classification::LayeredChildUnsupported;
  }
};

InjectedStateObservation observeInjectedState(const HWND window, const HWND taskbar) {
  const LONG_PTR style = GetWindowLongPtrW(window, GWL_STYLE);
  const LONG_PTR exStyle = GetWindowLongPtrW(window, GWL_EXSTYLE);
  return {GetParent(window) == taskbar,
          (style & static_cast<LONG_PTR>(WS_CHILD)) != 0,
          (style & static_cast<LONG_PTR>(WS_POPUP)) != 0,
          (exStyle & static_cast<LONG_PTR>(WS_EX_LAYERED)) != 0,
          (exStyle & static_cast<LONG_PTR>(WS_EX_TOOLWINDOW)) != 0,
          (exStyle & static_cast<LONG_PTR>(WS_EX_NOACTIVATE)) != 0,
          (exStyle & static_cast<LONG_PTR>(WS_EX_APPWINDOW)) != 0};
}

TaskbarLayoutStrategy::AttachResult classifyInjectedState(const InjectedStateObservation& observation, const bool capabilityConfirmed) {
  switch (observation.classify()) {
    case InjectedStateObservation::Classification::Valid:
      return TaskbarLayoutStrategy::AttachResult::Attached;
    case InjectedStateObservation::Classification::LayeredChildUnsupported:
      if (capabilityConfirmed) return TaskbarLayoutStrategy::AttachResult::Invalid;
      UWF_LOG_W("taskbar") << "taskbar presentation unavailable: reason=layered-child-unsupported";
      return TaskbarLayoutStrategy::AttachResult::Incompatible;
    case InjectedStateObservation::Classification::Invalid:
      return TaskbarLayoutStrategy::AttachResult::Invalid;
  }
  Q_UNREACHABLE_RETURN(TaskbarLayoutStrategy::AttachResult::Invalid);
}

}  // namespace

class Win11TaskbarLayoutStrategy::AttachTransactionImpl final : public TaskbarLayoutStrategy::AttachTransaction {
 public:
  AttachTransactionImpl(Win11TaskbarLayoutStrategy* owner, QWindow* window, AttachReadiness readiness, Environment environment = {}, Placement placement = {},
                        QSize logicalSize = {})
      : AttachTransaction(readiness),
        m_owner(owner),
        m_window(window),
        m_windowId(window ? window->winId() : 0),
        m_environment(environment),
        m_placement(placement),
        m_logicalSize(logicalSize) {}

  AttachResult commit() override;
  AttachResult finalize() override;
  DetachResult rollback() override;

 private:
  enum class State { Prepared, Committed, NativeWindowDestroyed, Finished };
  AttachResult hardReset(const char* reason, DWORD error = ERROR_SUCCESS);
  Win11TaskbarLayoutStrategy* m_owner = nullptr;
  QPointer<QWindow> m_window;
  WId m_windowId = 0;
  Environment m_environment;
  Placement m_placement;
  QSize m_logicalSize;
  State m_state = State::Prepared;
};

Win11TaskbarLayoutStrategy::Win11TaskbarLayoutStrategy() : m_impl(std::make_unique<Impl>()) {}
Win11TaskbarLayoutStrategy::~Win11TaskbarLayoutStrategy() {
  if (m_impl->attachment) (void)detach();
}

bool Win11TaskbarLayoutStrategy::isCompatible() const { return windowsVersionInfo().family == WindowsFamily::Windows11; }

void Win11TaskbarLayoutStrategy::recordVerificationDiagnostic(const VerificationResult result, const char* const reason) const {
  if (result == VerificationResult::Confirmed) {
    m_impl->lastDiagnosticResult = result;
    m_impl->lastDiagnosticReason.clear();
    return;
  }
  const std::string currentReason = reason ? reason : "unknown";
  if (m_impl->lastDiagnosticResult == result && m_impl->lastDiagnosticReason == currentReason) return;
  m_impl->lastDiagnosticResult = result;
  m_impl->lastDiagnosticReason = currentReason;
  UWF_LOG_D("taskbar") << "verification observed: result="
                       << (result == VerificationResult::Retained          ? "retained"
                           : result == VerificationResult::RefreshRequired ? "refresh-required"
                                                                           : "invalid")
                       << " reason=" << currentReason;
}

std::unique_ptr<TaskbarLayoutStrategy::AttachTransaction> Win11TaskbarLayoutStrategy::prepareAttach(QWindow* window, const QSize& logicalSize) {
  const bool active = m_impl->attachment && m_impl->attachment->window == window;
  const EnvironmentProbe probe = active ? refreshEnvironment(m_impl->attachment->environment) : probeEnvironment();
  if (probe.availability == RuntimeAvailability::TemporarilyUnavailable)
    return std::make_unique<AttachTransactionImpl>(this, window, AttachReadiness::TemporarilyUnavailable);
  if (probe.availability == RuntimeAvailability::IncompatibleLayout || !probe.environment)
    return std::make_unique<AttachTransactionImpl>(this, window, AttachReadiness::Unavailable);
  const auto placement = calculatePlacement(*probe.environment, logicalSize);
  if (!placement) return std::make_unique<AttachTransactionImpl>(this, window, active ? AttachReadiness::TemporarilyUnavailable : AttachReadiness::Unavailable);
  return std::make_unique<AttachTransactionImpl>(this, window, AttachReadiness::Ready, *probe.environment, *placement, logicalSize);
}

TaskbarLayoutStrategy::AttachResult Win11TaskbarLayoutStrategy::AttachTransactionImpl::hardReset(const char* reason, const DWORD error) {
  QWindow* const qtWindow = m_window.data();
  if (m_owner && m_owner->m_impl->attachment) {
    auto& attached = *m_owner->m_impl->attachment;
    const HWND oldWindow = reinterpret_cast<HWND>(attached.windowId);
    if (ownsNativeWindow(oldWindow, attached.cookie)) (void)RemovePropW(oldWindow, kAttachmentCookieProperty);
    const QPointer<QWindow> originalParent = attached.originalParent;
    const QPointer<QObject> originalObjectParent = attached.originalObjectParent;
    (void)restoreQtParents(qtWindow, originalParent.data(), originalObjectParent.data());
    m_owner->m_impl->attachment.reset();
  }
  if (qtWindow) qtWindow->destroy();
  m_state = State::NativeWindowDestroyed;
  if (error == ERROR_SUCCESS)
    UWF_LOG_I("taskbar") << "native window reset: reason=" << reason << " action=destroy-and-recreate";
  else
    UWF_LOG_I("taskbar") << "native window reset: reason=" << reason << " win32Error=" << error << " action=destroy-and-recreate";
  return AttachResult::Invalid;
}

TaskbarLayoutStrategy::AttachResult Win11TaskbarLayoutStrategy::AttachTransactionImpl::commit() {
  QWindow* const qtWindow = m_window.data();
  if (m_state != State::Prepared || !m_owner || readiness() != AttachReadiness::Ready || !qtWindow || qtWindow->winId() != m_windowId)
    return AttachResult::Invalid;
  HWND window = ownedWindow(m_windowId);
  if (!window || !IsWindow(m_environment.taskbar) || !IsWindow(m_environment.notify)) return AttachResult::TemporarilyUnavailable;

  if (m_owner->m_impl->attachment) {
    auto& attached = *m_owner->m_impl->attachment;
    window = ownedWindow(m_windowId, attached.cookie);
    if (!window || attached.window != qtWindow || attached.windowId != m_windowId || !sameEnvironment(attached.environment, m_environment))
      return hardReset("active-attachment-invariant");
    const InjectedStateObservation observation = observeInjectedState(window, m_environment.taskbar);
    const AttachResult injectedState = classifyInjectedState(observation, m_owner->m_impl->layeredChildCapabilityConfirmed);
    if (injectedState == AttachResult::Incompatible) return injectedState;
    if (injectedState != AttachResult::Attached) return hardReset("active-attachment-invariant");
    if (!matchesPlacement(window, m_environment, m_placement) &&
        !SetWindowPos(window, HWND_TOP, m_placement.x, m_placement.y, m_placement.width, m_placement.height, SWP_NOACTIVATE | SWP_NOOWNERZORDER))
      return AttachResult::TemporarilyUnavailable;
    attached.environment = m_environment;
    attached.logicalSize = m_logicalSize;
    m_state = State::Committed;
    return AttachResult::Attached;
  }

  std::unique_ptr<QWindow> taskbarWindow(QWindow::fromWinId(reinterpret_cast<WId>(m_environment.taskbar)));
  if (!taskbarWindow) return AttachResult::TemporarilyUnavailable;
  if (GetPropW(window, kAttachmentCookieProperty)) {
    qtWindow->destroy();
    m_state = State::NativeWindowDestroyed;
    return AttachResult::Invalid;
  }
  const HANDLE cookie = nextAttachmentCookie();
  if (!SetPropW(window, kAttachmentCookieProperty, cookie)) return AttachResult::Invalid;
  m_owner->m_impl->attachment.emplace(Impl::Attachment{m_environment, qtWindow, std::move(taskbarWindow), m_windowId, qtWindow->parent(),
                                                       qtWindow->QObject::parent(), GetParent(window), GetWindowLongPtrW(window, GWL_STYLE),
                                                       GetWindowLongPtrW(window, GWL_EXSTYLE), cookie, m_logicalSize});
  auto& attached = *m_owner->m_impl->attachment;
  LONG_PTR style = attached.originalStyle;
  style &= ~static_cast<LONG_PTR>(WS_POPUP);
  style |= static_cast<LONG_PTR>(WS_CHILD);
  LONG_PTR exStyle = attached.originalExStyle;
  exStyle &= ~static_cast<LONG_PTR>(WS_EX_APPWINDOW);
  exStyle |= static_cast<LONG_PTR>(WS_EX_NOACTIVATE);
  if (!setWindowLongChecked(window, GWL_STYLE, style) || !setWindowLongChecked(window, GWL_EXSTYLE, exStyle))
    return hardReset("native-style-transaction", GetLastError());

  // SetParent 不会调整 WS_CHILD/WS_POPUP。先建立子窗口语义，再由 Qt 完成
  // 唯一一次 parent 事务——禁止再补 Win32 SetParent，否则 Qt 树与 HWND 树
  // 会出现双提交来源，Confirmed 时 Explorer 下可能出现重复或零 attachment。
  qtWindow->setParent(attached.taskbarWindow.get());
  if (qtWindow->winId() != m_windowId) return hardReset("qt-parent-transaction", GetLastError());
  if (qtWindow->parent() != attached.taskbarWindow.get() || GetParent(window) != m_environment.taskbar) {
    const AttachResult result = m_owner->abortIncompleteParentCommit();
    m_state = result == AttachResult::TemporarilyUnavailable ? State::Finished : State::NativeWindowDestroyed;
    return result;
  }

  if (!SetWindowPos(window, HWND_TOP, m_placement.x, m_placement.y, m_placement.width, m_placement.height,
                    SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOOWNERZORDER))
    return hardReset("native-placement-transaction", GetLastError());
  const InjectedStateObservation observation = observeInjectedState(window, m_environment.taskbar);
  const AttachResult injectedState = classifyInjectedState(observation, m_owner->m_impl->layeredChildCapabilityConfirmed);
  if (injectedState == AttachResult::Incompatible) return injectedState;
  if (injectedState != AttachResult::Attached) return hardReset("native-attachment-invariant");
  m_owner->m_impl->hardResetRequired = false;
  m_state = State::Committed;
  return AttachResult::Attached;
}

TaskbarLayoutStrategy::AttachResult Win11TaskbarLayoutStrategy::AttachTransactionImpl::finalize() {
  QWindow* const qtWindow = m_window.data();
  if (m_state != State::Committed || !m_owner || !qtWindow || qtWindow->winId() != m_windowId || !m_owner->m_impl->attachment)
    return hardReset("finalize-identity");
  const auto& attached = *m_owner->m_impl->attachment;
  const HWND window = ownedWindow(m_windowId, attached.cookie);
  if (!window) return hardReset("finalize-identity");
  const InjectedStateObservation observation = observeInjectedState(window, m_environment.taskbar);
  const AttachResult injectedState = classifyInjectedState(observation, m_owner->m_impl->layeredChildCapabilityConfirmed);
  if (injectedState == AttachResult::Incompatible) return injectedState;
  if (injectedState != AttachResult::Attached) {
    UWF_LOG_D("taskbar") << "attachment finalization rejected: parent=" << observation.parentMatches << " child=" << observation.child
                         << " popup=" << observation.popup
                         << " layered=" << observation.layered << " toolWindow=" << observation.toolWindow << " noActivate=" << observation.noActivate
                         << " appWindow=" << observation.appWindow;
    return hardReset("finalize-invariant");
  }
  if (!matchesPlacement(window, m_environment, m_placement) &&
      !SetWindowPos(window, HWND_TOP, m_placement.x, m_placement.y, m_placement.width, m_placement.height, SWP_NOACTIVATE | SWP_NOOWNERZORDER))
    return hardReset("finalize-placement", GetLastError());
  m_owner->m_impl->layeredChildCapabilityConfirmed = true;
  m_state = State::Finished;
  return AttachResult::Attached;
}

TaskbarLayoutStrategy::DetachResult Win11TaskbarLayoutStrategy::AttachTransactionImpl::rollback() {
  if (m_state == State::Finished) return DetachResult::Detached;
  if (m_state == State::NativeWindowDestroyed) {
    m_state = State::Finished;
    return DetachResult::NativeWindowDestroyed;
  }
  const DetachResult result = m_owner ? m_owner->detach() : DetachResult::Failed;
  m_state = State::Finished;
  return result;
}

TaskbarLayoutStrategy::VerificationResult Win11TaskbarLayoutStrategy::verify(const QWindow* qtWindow, const WId currentWindowId) const {
  if (!m_impl->attachment) {
    recordVerificationDiagnostic(VerificationResult::Invalid, "attachment-missing");
    return VerificationResult::Invalid;
  }
  const auto& attached = *m_impl->attachment;
  const HWND window = ownedWindow(currentWindowId, attached.cookie);
  if (!qtWindow || attached.window != qtWindow || currentWindowId != attached.windowId || !window || !qtWindow->isVisible()) {
    m_impl->hardResetRequired = true;
    recordVerificationDiagnostic(VerificationResult::Invalid, "local-attachment-invariant");
    return VerificationResult::Invalid;
  }
  const InjectedStateObservation injectedState = observeInjectedState(window, attached.environment.taskbar);
  const auto injectedStateClassification = injectedState.classify();
  if (injectedStateClassification == InjectedStateObservation::Classification::LayeredChildUnsupported) {
    // 已激活 attachment 必然通过过 finalize，因此进程能力已经得到证明。
    // 此处的 style 丢失只能是运行时状态损坏，必须重建 HWND，不能永久淘汰策略。
    m_impl->hardResetRequired = true;
    recordVerificationDiagnostic(VerificationResult::Invalid, "layered-child-state-lost");
    return VerificationResult::Invalid;
  }
  if (injectedStateClassification == InjectedStateObservation::Classification::Invalid) {
    m_impl->hardResetRequired = true;
    recordVerificationDiagnostic(VerificationResult::Invalid, "local-attachment-invariant");
    return VerificationResult::Invalid;
  }
  auto probe = refreshEnvironment(attached.environment);
  if (probe.availability == RuntimeAvailability::TemporarilyUnavailable) {
    probe = win11_taskbar::detail::resolveRetainedProbe(probe, probeEnvironment());
    if (probe.availability == RuntimeAvailability::TemporarilyUnavailable) {
      recordVerificationDiagnostic(VerificationResult::Retained, "shell-environment-temporarily-unavailable");
      return VerificationResult::Retained;
    }
  }
  if (probe.availability == RuntimeAvailability::IncompatibleLayout || !probe.environment || !sameEnvironment(attached.environment, *probe.environment)) {
    recordVerificationDiagnostic(VerificationResult::Invalid, "shell-environment-invalid");
    return VerificationResult::Invalid;
  }
  const auto placement = calculatePlacement(*probe.environment, attached.logicalSize);
  const auto observation =
      win11_taskbar::detail::classifyPlacementObservation(placement.has_value(), placement && matchesPlacement(window, *probe.environment, *placement));
  if (observation == win11_taskbar::detail::PlacementObservation::Retained) {
    recordVerificationDiagnostic(VerificationResult::Retained, "placement-not-calculable");
    return VerificationResult::Retained;
  }
  if (observation == win11_taskbar::detail::PlacementObservation::RefreshRequired) {
    recordVerificationDiagnostic(VerificationResult::RefreshRequired, "placement-mismatch");
    return VerificationResult::RefreshRequired;
  }
  DWORD cloaked = 0;
  if (SUCCEEDED(DwmGetWindowAttribute(window, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) {
    recordVerificationDiagnostic(VerificationResult::Retained, "dwm-cloaked");
    return VerificationResult::Retained;
  }
  // QWidget::isVisible() 只代表 Qt 请求的可见状态。Explorer 或其他原生操作
  // 可以隐藏 HWND 而不更新 Qt 状态；此时必须走一次原地 refresh，不能继续
  // 向 Hub 宣称 presentation 已确认。
  if (!IsWindowVisible(window)) {
    recordVerificationDiagnostic(VerificationResult::RefreshRequired, "native-window-hidden");
    return VerificationResult::RefreshRequired;
  }
  recordVerificationDiagnostic(VerificationResult::Confirmed, "confirmed");
  return VerificationResult::Confirmed;
}

TaskbarLayoutStrategy::DetachResult Win11TaskbarLayoutStrategy::detach() {
  if (!m_impl->attachment) return DetachResult::Detached;
  auto& attached = *m_impl->attachment;
  QWindow* const qtWindow = attached.window.data();
  const HWND window = reinterpret_cast<HWND>(attached.windowId);
  const auto forceHardReset = [&](const char* reason) {
    if (ownsNativeWindow(window, attached.cookie)) (void)RemovePropW(window, kAttachmentCookieProperty);
    const QPointer<QWindow> originalParent = attached.originalParent;
    const QPointer<QObject> originalObjectParent = attached.originalObjectParent;
    (void)restoreQtParents(qtWindow, originalParent.data(), originalObjectParent.data());
    m_impl->attachment.reset();
    m_impl->hardResetRequired = false;
    if (qtWindow) qtWindow->destroy();
    UWF_LOG_I("taskbar") << "native window reset: reason=" << reason << " action=destroy-and-recreate";
    return DetachResult::NativeWindowDestroyed;
  };
  if (m_impl->hardResetRequired) return forceHardReset("verification-failure");
  if (!qtWindow || !ownsNativeWindow(window, attached.cookie)) {
    m_impl->attachment.reset();
    return DetachResult::NativeWindowDestroyed;
  }
  if (!restoreQtParents(qtWindow, attached.originalParent.data(), attached.originalObjectParent.data())) return forceHardReset("detach-qt-parent");
  if (!ownsNativeWindow(window, attached.cookie)) return forceHardReset("detach-native-identity");
  if (GetParent(window) != attached.originalNativeParent) return forceHardReset("detach-native-parent");
  if (!restoreWindowStyles(window, attached.originalStyle, attached.originalExStyle)) return forceHardReset("detach-native-style");
  if (RemovePropW(window, kAttachmentCookieProperty) != attached.cookie) return forceHardReset("detach-cookie");
  m_impl->attachment.reset();
  return DetachResult::Detached;
}

TaskbarLayoutStrategy::DetachResult Win11TaskbarLayoutStrategy::invalidate() {
  m_impl->hardResetRequired = true;
  return detach();
}

TaskbarLayoutStrategy::AttachResult Win11TaskbarLayoutStrategy::abortIncompleteParentCommit() {
  // Parent 提交不完整：必须走与正常 detach() 相同的逐项校验（Qt parent、
  // HWND parent、style、cookie）。只有全部还原才允许 TemporarilyUnavailable
  // 重试；任一项失败由 detach() hard-reset 销毁窗口，绝不能留下半注入状态。
  const DetachResult detached = detach();
  if (detached == DetachResult::Detached) {
    UWF_LOG_D("taskbar") << "parent transaction deferred: reason=qt-parent-mismatch action=rollback-and-retry";
    return AttachResult::TemporarilyUnavailable;
  }
  UWF_LOG_D("taskbar") << "parent transaction aborted: reason=qt-parent-mismatch action=destroy-and-recreate";
  return AttachResult::Invalid;
}

}  // namespace uwf::ui
