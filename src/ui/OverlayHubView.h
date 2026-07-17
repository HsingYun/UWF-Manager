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
#include <deque>

#include "OverlayUsageState.h"

class QMenu;
class QTimer;

namespace uwf::ui {

// Hub 可编排的统一展示端点。实现类负责自身窗口生命周期、展示确认和恢复；
// Hub 只依赖此契约，按 priority() 从高到低选择第一个可用端点。
//
// 展示生命周期（全局一张表，不用旁路布尔改写语义）：
//   Disabled ──冷启动──► Probing → … → Confirmed
//   Confirmed ──用户关──► Withdrawing ─┬─ 重开 ─► Recovering → 再挂载
//                                      └─ 完成 ─► Withdrawn ──重开──► Recovering
//   * ──身份/操作失败──► Failing（正在清理）──完成──► Unavailable
//                                  └─阻塞──► Blocked / BlockedWithdrawn
//   Blocked ◄─用户开/关─► BlockedWithdrawn；清理完成后分别进入 Unavailable / Withdrawn
//   Probing/Activating ──进程能力不兼容──► Incompatible（进程生命周期内终态）
//   Recovering ──Shell 瞬时失败──► 有限次排他重试 ──耗尽──► Failing（让出 fallback）
//   正在清理或已阻塞的状态忽略 HostReleaseStarted（不打断清理），但必须接受
//   HostReleaseCompleted（cleanup 升级为 HostInvalidated 时只发 Host 完成事件）。
class OverlayHubView : public QWidget {
  Q_OBJECT
 public:
  enum class DisplayState {
    Disabled,
    Withdrawn,
    Unavailable,
    Incompatible,
    Probing,
    Activating,
    Attaching,
    Refreshing,
    Withdrawing,
    Failing,
    Blocked,
    BlockedWithdrawn,
    Recovering,
    Confirmed
  };
  enum class AttachResult { Prepared, Attached, Retained, TemporarilyUnavailable, Incompatible, ReleasePending, ReleaseBlocked, Failed };
  // Pending 仅用于首次展示尚未完成；Retained 表示已确认 View 的宿主身份仍然
  // 有效，只是外部系统暂时无法确认其可见性；RefreshRequired 表示身份仍在，
  // 但必须进入 attach 事务重修或重新分类能力。只有 Confirmed/Retained 能维持
  // Hub 的当前优先级，普通隐藏和身份丢失必须返回 Invalid。
  enum class VerificationResult { Confirmed, Pending, Retained, RefreshRequired, Invalid };

  explicit OverlayHubView(QWidget* parent = nullptr, Qt::WindowFlags flags = {});
  ~OverlayHubView() override = default;

  [[nodiscard]] virtual bool isCompatible() const { return true; }
  [[nodiscard]] virtual int priority() const = 0;

  virtual void applyUsageState(const OverlayUsageState&) {}

  void setPresentationRequested(bool requested);
  void authorizePresentationActivation();

  [[nodiscard]] DisplayState displayState() const { return m_displayState; }
  [[nodiscard]] bool presentationConfirmed() const { return m_displayState == DisplayState::Confirmed; }
  [[nodiscard]] bool presentationExclusive() const {
    return m_displayState == DisplayState::Activating || m_displayState == DisplayState::Attaching || m_displayState == DisplayState::Refreshing ||
           m_displayState == DisplayState::Recovering;
  }

 signals:
  void showMainWindowRequested();
  void hideHubRequested();
  void safeShutdownRequested();
  void safeRestartRequested();
  void exitApplicationRequested();
  void displayStateChanged();

 protected:
  enum class ReleaseReason { RequestRevoked, Recovery };
  enum class ReleaseResult { Complete, Pending };

  virtual AttachResult acquirePresentation();
  virtual AttachResult activatePresentation();
  [[nodiscard]] virtual VerificationResult verifyPresentation() const = 0;
  virtual void suspendPresentation();
  virtual ReleaseResult detachPresentation(ReleaseReason reason);
  [[nodiscard]] virtual int healthCheckIntervalMs() const { return 0; }
  [[nodiscard]] virtual int confirmationTimeoutMs() const { return 300; }
  [[nodiscard]] virtual int retryIntervalMs(int consecutiveFailures) const {
    Q_UNUSED(consecutiveFailures);
    return 1000;
  }
  // Recovering 内软失败的最大排他重试次数；超过后进入 Failing 让出 fallback。
  [[nodiscard]] virtual int maxExclusiveRecoverAttempts() const { return 3; }

  [[nodiscard]] bool presentationRequested() const { return m_requested; }
  void addApplicationTitleToMenu(QMenu& menu) const;
  void requestPresentationRefresh();
  void notifyPresentationChanged();
  void notifyHostPresentationReleaseStarted();
  void notifyHostPresentationReleaseCompleted();
  void notifyPresentationReleaseBlocked();
  void notifyPresentationReleaseCompleted();

 private:
  friend class OverlayHubViewTestAccess;

  enum class VerificationSource { Attach, Confirmation, Health, PresentationChanged };
  enum class EventType {
    RequestEnabled,
    RequestDisabled,
    ExternalRefresh,
    RetryDue,
    ConfirmationDue,
    HealthDue,
    PresentationChanged,
    ActivationAuthorized,
    AttachFinished,
    ActivationFinished,
    VerificationObserved,
    ReleaseStarted,
    ReleaseBlocked,
    ReleaseCompleted,
    HostReleaseStarted,
    HostReleaseCompleted
  };
  enum class Action {
    None,
    AttachAcquire,
    AttachRefresh,
    Activate,
    VerifyAttach,
    VerifyConfirmation,
    VerifyHealth,
    VerifyChanged,
    Suspend,
    ReleaseRecovery,
    ReleaseRequest,
    ScheduleRecoverRetry
  };

  struct Event {
    EventType type;
    AttachResult attachResult = AttachResult::Failed;
    VerificationResult verificationResult = VerificationResult::Invalid;
    VerificationSource verificationSource = VerificationSource::Attach;
    ReleaseResult releaseResult = ReleaseResult::Complete;

    [[nodiscard]] static Event plain(EventType type) { return Event{type}; }
    [[nodiscard]] static Event attachFinished(AttachResult result) {
      Event event{EventType::AttachFinished};
      event.attachResult = result;
      return event;
    }
    [[nodiscard]] static Event activationFinished(AttachResult result) {
      Event event{EventType::ActivationFinished};
      event.attachResult = result;
      return event;
    }
    [[nodiscard]] static Event verificationObserved(VerificationResult result, VerificationSource source) {
      Event event{EventType::VerificationObserved};
      event.verificationResult = result;
      event.verificationSource = source;
      return event;
    }
    [[nodiscard]] static Event releaseStarted(ReleaseResult result) {
      Event event{EventType::ReleaseStarted};
      event.releaseResult = result;
      return event;
    }
  };
  struct Transition {
    DisplayState nextState;
    Action action = Action::None;
    bool actionBeforeStateChange = false;
  };
  using TransitionHandler = Transition (*)(DisplayState, const Event&);

  [[nodiscard]] static Transition reduce(DisplayState state, const Event& event);
  [[nodiscard]] static Transition ignoreEvent(DisplayState state, const Event& event);
  [[nodiscard]] static Transition probePresentation(DisplayState state, const Event& event);
  [[nodiscard]] static Transition refreshPresentation(DisplayState state, const Event& event);
  [[nodiscard]] static Transition disablePresentation(DisplayState state, const Event& event);
  [[nodiscard]] static Transition withdrawFailingPresentation(DisplayState state, const Event& event);
  [[nodiscard]] static Transition enableFromWithdrawn(DisplayState state, const Event& event);
  [[nodiscard]] static Transition enableDuringWithdraw(DisplayState state, const Event& event);
  [[nodiscard]] static Transition verifyConfirmation(DisplayState state, const Event& event);
  [[nodiscard]] static Transition verifyHealth(DisplayState state, const Event& event);
  [[nodiscard]] static Transition verifyChanged(DisplayState state, const Event& event);
  [[nodiscard]] static Transition activatePrepared(DisplayState state, const Event& event);
  [[nodiscard]] static Transition handleAttachFinished(DisplayState state, const Event& event);
  [[nodiscard]] static Transition handleActivationFinished(DisplayState state, const Event& event);
  [[nodiscard]] static Transition handleVerificationObserved(DisplayState state, const Event& event);
  [[nodiscard]] static Transition handleReleaseStarted(DisplayState state, const Event& event);
  [[nodiscard]] static Transition handleReleaseBlocked(DisplayState state, const Event& event);
  [[nodiscard]] static Transition requestBlockedPresentation(DisplayState state, const Event& event);
  [[nodiscard]] static Transition withdrawBlockedPresentation(DisplayState state, const Event& event);
  [[nodiscard]] static Transition handleReleaseCompleted(DisplayState state, const Event& event);
  [[nodiscard]] static Transition enterRecovering(DisplayState state, const Event& event);
  [[nodiscard]] static Transition recoverHost(DisplayState state, const Event& event);
  void postEvent(Event event);
  void processEvents();
  void execute(Action action);
  void beginPresentationRelease(ReleaseReason reason);
  void enterState(DisplayState state);
  void observeVerification(VerificationSource source, bool repaintFirst = false);
  void logOutcome(const Event& event) const;

  QTimer* m_confirmationTimer;
  QTimer* m_retryTimer;
  QTimer* m_healthTimer;
  std::deque<Event> m_events;
  bool m_processingEvents = false;
  bool m_requested = false;
  int m_consecutiveFailures = 0;
  DisplayState m_displayState = DisplayState::Disabled;
};

}  // namespace uwf::ui
