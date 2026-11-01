// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QWidget>
#include <deque>

#include "application/overlay/contracts/OverlayUsage.h"

class QTimer;

namespace uwf::ui {

class OverlayContextMenu;

// Owns endpoint activation, confirmation, cleanup and recovery. The Hub selects by priority.
// Recovery retains exclusivity for a bounded number of attempts before yielding to fallback.
// Incompatible is terminal for the process. Cleanup accepts host-release completion even
// when detachment was escalated to host invalidation.
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
  // Pending denotes initial confirmation. Retained preserves a confirmed host identity
  // during a shell transition; RefreshRequired requests a new attachment transaction.
  // Hidden endpoints and lost identities are Invalid.
  enum class VerificationResult { Confirmed, Pending, Retained, RefreshRequired, Invalid };

  explicit OverlayHubView(QWidget* parent = nullptr, Qt::WindowFlags flags = {});
  void setOperationAvailability(bool powerAllowed, bool exitAllowed);
  ~OverlayHubView() override = default;

  [[nodiscard]] virtual bool isCompatible() const { return true; }
  [[nodiscard]] virtual int priority() const = 0;

  virtual void applyUsageState(const app::OverlayUsageState&) {}

  void setPresentationRequested(bool requested);
  void authorizePresentationActivation();

  [[nodiscard]] DisplayState displayState() const { return m_displayState; }
  [[nodiscard]] bool presentationConfirmed() const { return m_displayState == DisplayState::Confirmed; }
  [[nodiscard]] bool presentationExclusive() const {
    return m_displayState == DisplayState::Activating || m_displayState == DisplayState::Attaching || m_displayState == DisplayState::Refreshing ||
           m_displayState == DisplayState::Recovering;
  }

 signals:
  void operationAvailabilityChanged(bool powerAllowed, bool exitAllowed);
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
  // Transient recovery attempts before cleanup releases priority to fallback.
  [[nodiscard]] virtual int maxExclusiveRecoverAttempts() const { return 3; }

  [[nodiscard]] bool presentationRequested() const { return m_requested; }
  void connectContextMenu(OverlayContextMenu& menu);
  void requestPresentationRefresh();
  void notifyPresentationChanged();
  void notifyHostPresentationReleaseStarted();
  void notifyHostPresentationReleaseCompleted();
  void notifyPresentationReleaseBlocked();
  void notifyPresentationReleaseCompleted();

 private:
  bool m_powerAllowed = false;
  bool m_exitAllowed = true;
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
