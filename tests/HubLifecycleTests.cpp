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
#include <windows.h>

#include <QApplication>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QWindow>
#include <algorithm>
#include <array>
#include <cstdio>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "core/UwfModel.h"
#include "ui/OverlayHub.h"
#include "ui/OverlayHubView.h"
#include "ui/TaskbarLayoutCoordinator.h"
#include "ui/TaskbarLayoutStrategy.h"
#include "ui/Win11TaskbarEnvironment.h"
#include "util/Log.h"

namespace uwf::ui {

class TaskbarLayoutCoordinatorTestAccess {
 public:
  enum class TestState {
    Detached,
    Preparing,
    RecreatingForPrepare,
    Prepared,
    Finalizing,
    Repreparing,
    RecreatingForActivation,
    RecreatingForFailure,
    RecreatingForIncompatible,
    Attached,
    Detaching,
    RecreatingAfterDetach
  };
  enum class TestEvent {
    AcquireRequested,
    ActivateRequested,
    DetachRequested,
    HostInvalidated,
    OperationPrepared,
    OperationAttached,
    OperationRetained,
    OperationTemporary,
    OperationIncompatible,
    OperationFailed,
    OperationRetryPrepare,
    OperationRecreatePrepare,
    OperationRecreateActivation,
    OperationRecreateFailure,
    OperationRecreateIncompatible,
    WindowRecreated,
    WindowRecreationFailed,
    OperationNeedsDetach,
    DetachDue,
    DetachBlocked,
    DetachCompleted
  };
  enum class TestAction {
    None,
    AcceptRequest,
    Prepare,
    Finalize,
    CompletePrepared,
    CompleteAttached,
    CompleteRetained,
    CompleteTemporary,
    CompleteIncompatible,
    CompleteFailed,
    CompleteReleaseStatus,
    RecreateWindow,
    RecreateDetachedWindow,
    BeginUserDetach,
    BeginHostDetach,
    EscalateHostDetach,
    ScheduleDetach,
    AdvanceDetach,
    CompleteDetach,
    NotifyDetachedHostRefresh
  };
  struct Result {
    int nextState;
    int action;
  };
  [[nodiscard]] static int stateCount() { return static_cast<int>(TaskbarLayoutCoordinator::State::RecreatingAfterDetach) + 1; }
  [[nodiscard]] static int eventCount() { return static_cast<int>(TaskbarLayoutCoordinator::EventType::DetachCompleted) + 1; }
  [[nodiscard]] static Result reduce(const int state, const int event) {
    const auto transition = TaskbarLayoutCoordinator::reduce(static_cast<TaskbarLayoutCoordinator::State>(state),
                                                             TaskbarLayoutCoordinator::Event{static_cast<TaskbarLayoutCoordinator::EventType>(event)});
    return {static_cast<int>(transition.nextState), static_cast<int>(transition.action)};
  }
  [[nodiscard]] static Result reduceDetachCompleted(const bool nativeWindowDestroyed) {
    const auto transition =
        TaskbarLayoutCoordinator::reduce(TaskbarLayoutCoordinator::State::Detaching,
                                         TaskbarLayoutCoordinator::Event{TaskbarLayoutCoordinator::EventType::DetachCompleted, nativeWindowDestroyed});
    return {static_cast<int>(transition.nextState), static_cast<int>(transition.action)};
  }
  [[nodiscard]] static const char* stateName(const int value) {
    static constexpr std::array names{"detached",
                                      "preparing",
                                      "recreating-for-prepare",
                                      "prepared",
                                      "finalizing",
                                      "repreparing",
                                      "recreating-for-activation",
                                      "recreating-for-failure",
                                      "recreating-for-incompatible",
                                      "attached",
                                      "detaching",
                                      "recreating-after-detach"};
    return names.at(static_cast<std::size_t>(value));
  }
  [[nodiscard]] static const char* eventName(const int value) {
    static constexpr std::array names{"acquire",
                                      "activate",
                                      "detach",
                                      "host-invalidated",
                                      "prepared",
                                      "attached",
                                      "retained",
                                      "temporary",
                                      "incompatible",
                                      "failed",
                                      "retry-prepare",
                                      "recreate-prepare",
                                      "recreate-activation",
                                      "recreate-failure",
                                      "recreate-incompatible",
                                      "window-recreated",
                                      "window-recreate-failed",
                                      "needs-detach",
                                      "detach-due",
                                      "detach-blocked",
                                      "detach-completed"};
    return names.at(static_cast<std::size_t>(value));
  }
  static void invalidateHost(TaskbarLayoutCoordinator& coordinator) {
    coordinator.postEvent(TaskbarLayoutCoordinator::Event{TaskbarLayoutCoordinator::EventType::HostInvalidated});
  }
  static void deliverTaskbarCreated(TaskbarLayoutCoordinator& coordinator) {
    MSG message{};
    message.message = coordinator.m_taskbarCreatedMessage;
    (void)coordinator.nativeEventFilter({}, &message, nullptr);
  }
  static void advanceDetaching(TaskbarLayoutCoordinator& coordinator) {
    coordinator.postEvent(TaskbarLayoutCoordinator::Event{TaskbarLayoutCoordinator::EventType::DetachDue});
  }
  [[nodiscard]] static TaskbarLayoutCoordinator::AttachResult deliverAcquireWithoutPayload(TaskbarLayoutCoordinator& coordinator) {
    coordinator.m_lastAttachResult = TaskbarLayoutCoordinator::AttachResult::Prepared;
    coordinator.postEvent(TaskbarLayoutCoordinator::Event{TaskbarLayoutCoordinator::EventType::AcquireRequested});
    return coordinator.m_lastAttachResult;
  }
  [[nodiscard]] static int retryInterval(const TaskbarLayoutCoordinator& coordinator) { return coordinator.m_asyncTimer->interval(); }
  [[nodiscard]] static bool isDetached(const TaskbarLayoutCoordinator& coordinator) { return coordinator.m_state == TaskbarLayoutCoordinator::State::Detached; }
  [[nodiscard]] static bool hasRequest(const TaskbarLayoutCoordinator& coordinator) { return coordinator.m_request.has_value(); }
  [[nodiscard]] static bool ownsAttachResources(const TaskbarLayoutCoordinator& coordinator) {
    return coordinator.m_attachment.has_value() || coordinator.m_transaction != nullptr;
  }
  [[nodiscard]] static bool isDetaching(const TaskbarLayoutCoordinator& coordinator) {
    return coordinator.m_state == TaskbarLayoutCoordinator::State::Detaching;
  }
};

class OverlayHubViewTestAccess {
 public:
  enum class Variant {
    RequestEnabled,
    RequestDisabled,
    ExternalRefresh,
    RetryDue,
    ConfirmationDue,
    HealthDue,
    PresentationChanged,
    ActivationAuthorized,
    ReleaseBlocked,
    ReleaseCompleted,
    HostReleaseStarted,
    HostReleaseCompleted,
    AttachPrepared,
    AttachAttached,
    AttachRetained,
    AttachTemporarilyUnavailable,
    AttachIncompatible,
    AttachReleasePending,
    AttachReleaseBlocked,
    AttachFailed,
    ActivationPrepared,
    ActivationAttached,
    ActivationRetained,
    ActivationTemporarilyUnavailable,
    ActivationIncompatible,
    ActivationReleasePending,
    ActivationReleaseBlocked,
    ActivationFailed,
    ReleaseComplete,
    ReleasePending,
    VerificationAttachConfirmed,
    VerificationAttachPending,
    VerificationAttachRetained,
    VerificationAttachRefreshRequired,
    VerificationAttachInvalid,
    VerificationConfirmationConfirmed,
    VerificationConfirmationPending,
    VerificationConfirmationRetained,
    VerificationConfirmationRefreshRequired,
    VerificationConfirmationInvalid,
    VerificationHealthConfirmed,
    VerificationHealthPending,
    VerificationHealthRetained,
    VerificationHealthRefreshRequired,
    VerificationHealthInvalid,
    VerificationChangedConfirmed,
    VerificationChangedPending,
    VerificationChangedRetained,
    VerificationChangedRefreshRequired,
    VerificationChangedInvalid
  };
  enum class TestAction {
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

  struct Result {
    int nextState;
    int action;
    bool hasAction;
    bool actionBeforeStateChange;
  };

  [[nodiscard]] static int stateCount() { return static_cast<int>(OverlayHubView::DisplayState::Confirmed) + 1; }
  [[nodiscard]] static int eventCount() { return static_cast<int>(OverlayHubView::EventType::HostReleaseCompleted) + 1; }
  [[nodiscard]] static int variantCount() { return static_cast<int>(Variant::VerificationChangedInvalid) + 1; }
  [[nodiscard]] static int state(const OverlayHubView::DisplayState value) { return static_cast<int>(value); }
  [[nodiscard]] static int action(const TestAction value) { return static_cast<int>(value); }
  [[nodiscard]] static int requestDisabledEvent() { return static_cast<int>(OverlayHubView::EventType::RequestDisabled); }
  [[nodiscard]] static int retryDueEvent() { return static_cast<int>(OverlayHubView::EventType::RetryDue); }
  [[nodiscard]] static int confirmationDueEvent() { return static_cast<int>(OverlayHubView::EventType::ConfirmationDue); }
  [[nodiscard]] static int releaseBlockedEvent() { return static_cast<int>(OverlayHubView::EventType::ReleaseBlocked); }
  [[nodiscard]] static int releaseCompletedEvent() { return static_cast<int>(OverlayHubView::EventType::ReleaseCompleted); }
  [[nodiscard]] static int hostReleaseStartedEvent() { return static_cast<int>(OverlayHubView::EventType::HostReleaseStarted); }

  [[nodiscard]] static Result reduce(const int stateIndex, const int eventIndex) {
    return reduce(static_cast<OverlayHubView::DisplayState>(stateIndex), OverlayHubView::Event::plain(static_cast<OverlayHubView::EventType>(eventIndex)));
  }

  [[nodiscard]] static Result reduceVariant(const int stateIndex, const int variantIndex) {
    return reduce(static_cast<OverlayHubView::DisplayState>(stateIndex), eventFor(static_cast<Variant>(variantIndex)));
  }

  [[nodiscard]] static const char* variantName(const int variantIndex) {
    static constexpr std::array names{"request-enabled",
                                      "request-disabled",
                                      "external-refresh",
                                      "retry-due",
                                      "confirmation-due",
                                      "health-due",
                                      "presentation-changed",
                                      "activation-authorized",
                                      "release-blocked",
                                      "release-completed",
                                      "host-release-started",
                                      "host-release-completed",
                                      "attach-prepared",
                                      "attach-attached",
                                      "attach-retained",
                                      "attach-temporary",
                                      "attach-incompatible",
                                      "attach-release-pending",
                                      "attach-release-blocked",
                                      "attach-failed",
                                      "activation-prepared",
                                      "activation-attached",
                                      "activation-retained",
                                      "activation-temporary",
                                      "activation-incompatible",
                                      "activation-release-pending",
                                      "activation-release-blocked",
                                      "activation-failed",
                                      "release-complete",
                                      "release-pending",
                                      "verify-attach-confirmed",
                                      "verify-attach-pending",
                                      "verify-attach-retained",
                                      "verify-attach-refresh",
                                      "verify-attach-invalid",
                                      "verify-confirm-confirmed",
                                      "verify-confirm-pending",
                                      "verify-confirm-retained",
                                      "verify-confirm-refresh",
                                      "verify-confirm-invalid",
                                      "verify-health-confirmed",
                                      "verify-health-pending",
                                      "verify-health-retained",
                                      "verify-health-refresh",
                                      "verify-health-invalid",
                                      "verify-changed-confirmed",
                                      "verify-changed-pending",
                                      "verify-changed-retained",
                                      "verify-changed-refresh",
                                      "verify-changed-invalid"};
    return names.at(static_cast<std::size_t>(variantIndex));
  }

  [[nodiscard]] static Result attachFinished(const OverlayHubView::DisplayState state, const OverlayHubView::AttachResult result) {
    return reduce(state, OverlayHubView::Event::attachFinished(result));
  }

  [[nodiscard]] static Result verificationObserved(const OverlayHubView::DisplayState state, const OverlayHubView::VerificationResult result,
                                                   const bool confirmation) {
    return reduce(state, OverlayHubView::Event::verificationObserved(
                             result, confirmation ? OverlayHubView::VerificationSource::Confirmation : OverlayHubView::VerificationSource::Attach));
  }

  [[nodiscard]] static Result requestDisabled(const OverlayHubView::DisplayState state) {
    return reduce(state, OverlayHubView::Event::plain(OverlayHubView::EventType::RequestDisabled));
  }
  [[nodiscard]] static Result retryDue(const OverlayHubView::DisplayState state) {
    return reduce(state, OverlayHubView::Event::plain(OverlayHubView::EventType::RetryDue));
  }
  [[nodiscard]] static Result confirmationDue(const OverlayHubView::DisplayState state) {
    return reduce(state, OverlayHubView::Event::plain(OverlayHubView::EventType::ConfirmationDue));
  }
  [[nodiscard]] static Result releaseBlocked(const OverlayHubView::DisplayState state) {
    return reduce(state, OverlayHubView::Event::plain(OverlayHubView::EventType::ReleaseBlocked));
  }
  [[nodiscard]] static Result releaseCompleted(const OverlayHubView::DisplayState state) {
    return reduce(state, OverlayHubView::Event::plain(OverlayHubView::EventType::ReleaseCompleted));
  }
  [[nodiscard]] static Result hostReleaseStarted(const OverlayHubView::DisplayState state) {
    return reduce(state, OverlayHubView::Event::plain(OverlayHubView::EventType::HostReleaseStarted));
  }
  [[nodiscard]] static Result hostReleaseCompleted(const OverlayHubView::DisplayState state) {
    return reduce(state, OverlayHubView::Event::plain(OverlayHubView::EventType::HostReleaseCompleted));
  }
  [[nodiscard]] static Result releasePending(const OverlayHubView::DisplayState state) {
    return reduce(state, OverlayHubView::Event::releaseStarted(OverlayHubView::ReleaseResult::Pending));
  }

  static void fireConfirmationDue(OverlayHubView& view) { view.postEvent(OverlayHubView::Event::plain(OverlayHubView::EventType::ConfirmationDue)); }
  static void fireHealthDue(OverlayHubView& view) { view.postEvent(OverlayHubView::Event::plain(OverlayHubView::EventType::HealthDue)); }
  static void fireExternalRefresh(OverlayHubView& view) { view.postEvent(OverlayHubView::Event::plain(OverlayHubView::EventType::ExternalRefresh)); }

 private:
  [[nodiscard]] static OverlayHubView::Event eventFor(const Variant variant) {
    const int value = static_cast<int>(variant);
    if (value <= static_cast<int>(Variant::ActivationAuthorized)) return OverlayHubView::Event::plain(static_cast<OverlayHubView::EventType>(value));
    if (variant == Variant::ReleaseBlocked) return OverlayHubView::Event::plain(OverlayHubView::EventType::ReleaseBlocked);
    if (variant == Variant::ReleaseCompleted) return OverlayHubView::Event::plain(OverlayHubView::EventType::ReleaseCompleted);
    if (variant == Variant::HostReleaseStarted) return OverlayHubView::Event::plain(OverlayHubView::EventType::HostReleaseStarted);
    if (variant == Variant::HostReleaseCompleted) return OverlayHubView::Event::plain(OverlayHubView::EventType::HostReleaseCompleted);
    if (value >= static_cast<int>(Variant::AttachPrepared) && value <= static_cast<int>(Variant::AttachFailed)) {
      const int result = value - static_cast<int>(Variant::AttachPrepared);
      return OverlayHubView::Event::attachFinished(static_cast<OverlayHubView::AttachResult>(result));
    }
    if (value >= static_cast<int>(Variant::ActivationPrepared) && value <= static_cast<int>(Variant::ActivationFailed)) {
      const int result = value - static_cast<int>(Variant::ActivationPrepared);
      return OverlayHubView::Event::activationFinished(static_cast<OverlayHubView::AttachResult>(result));
    }
    if (variant == Variant::ReleaseComplete) return OverlayHubView::Event::releaseStarted(OverlayHubView::ReleaseResult::Complete);
    if (variant == Variant::ReleasePending) return OverlayHubView::Event::releaseStarted(OverlayHubView::ReleaseResult::Pending);

    const int verification = value - static_cast<int>(Variant::VerificationAttachConfirmed);
    const auto source = static_cast<OverlayHubView::VerificationSource>(verification / 5);
    const auto result = static_cast<OverlayHubView::VerificationResult>(verification % 5);
    return OverlayHubView::Event::verificationObserved(result, source);
  }

  [[nodiscard]] static Result reduce(const OverlayHubView::DisplayState state, const OverlayHubView::Event& event) {
    const OverlayHubView::Transition transition = OverlayHubView::reduce(state, event);
    return {static_cast<int>(transition.nextState), static_cast<int>(transition.action), transition.action != OverlayHubView::Action::None,
            transition.actionBeforeStateChange};
  }
};

}  // namespace uwf::ui

namespace {

using uwf::ui::OverlayHub;
using uwf::ui::OverlayHubView;
using uwf::ui::OverlayHubViewTestAccess;
using uwf::ui::TaskbarLayoutCoordinator;
using uwf::ui::TaskbarLayoutCoordinatorTestAccess;
using uwf::ui::TaskbarLayoutStrategy;

using FsmState = OverlayHubView::DisplayState;
using FsmVariant = OverlayHubViewTestAccess::Variant;
using FsmAction = OverlayHubViewTestAccess::TestAction;

struct ExpectedTransition {
  FsmState nextState;
  FsmAction action = FsmAction::None;
  bool actionBeforeStateChange = false;
};

TaskbarLayoutCoordinatorTestAccess::Result expectedCoordinatorTransition(const TaskbarLayoutCoordinatorTestAccess::TestState state,
                                                                         const TaskbarLayoutCoordinatorTestAccess::TestEvent event) {
  using State = TaskbarLayoutCoordinatorTestAccess::TestState;
  using Event = TaskbarLayoutCoordinatorTestAccess::TestEvent;
  using Action = TaskbarLayoutCoordinatorTestAccess::TestAction;
  const auto result = [](const State next, const Action action = Action::None) {
    return TaskbarLayoutCoordinatorTestAccess::Result{static_cast<int>(next), static_cast<int>(action)};
  };

  if (event == Event::AcquireRequested && (state == State::Detached || state == State::Attached)) return result(State::Preparing, Action::AcceptRequest);
  if (event == Event::ActivateRequested && state == State::Prepared) return result(State::Finalizing, Action::Finalize);
  if (event == Event::ActivateRequested && state == State::Attached) return result(State::Attached, Action::CompleteAttached);
  if (event == Event::ActivateRequested && state == State::Detached) return result(State::Detached, Action::CompleteFailed);
  if (event == Event::ActivateRequested && state == State::Detaching) return result(state, Action::CompleteReleaseStatus);

  const auto operationResult = [&](const bool resumeActivation) -> std::optional<TaskbarLayoutCoordinatorTestAccess::Result> {
    if (event == Event::OperationPrepared)
      return resumeActivation ? result(State::Finalizing, Action::Finalize) : result(State::Prepared, Action::CompletePrepared);
    if (event == Event::OperationRetained) return result(State::Attached, Action::CompleteRetained);
    if (event == Event::OperationTemporary) return result(State::Detached, Action::CompleteTemporary);
    if (event == Event::OperationIncompatible) return result(State::Detached, Action::CompleteIncompatible);
    if (event == Event::OperationFailed) return result(State::Detached, Action::CompleteFailed);
    if (event == Event::OperationRecreateFailure) return result(State::RecreatingForFailure, Action::RecreateWindow);
    if (event == Event::OperationRecreateIncompatible) return result(State::RecreatingForIncompatible, Action::RecreateWindow);
    if (event == Event::OperationNeedsDetach) return result(State::Detaching, Action::BeginUserDetach);
    if (!resumeActivation && event == Event::OperationRecreatePrepare) return result(State::RecreatingForPrepare, Action::RecreateWindow);
    if (resumeActivation && event == Event::OperationRecreateActivation) return result(State::RecreatingForActivation, Action::RecreateWindow);
    return std::nullopt;
  };
  if (state == State::Preparing) {
    if (const auto transition = operationResult(false)) return *transition;
  }
  if (state == State::Repreparing) {
    if (const auto transition = operationResult(true)) return *transition;
  }
  if (state == State::Finalizing) {
    if (event == Event::OperationAttached) return result(State::Attached, Action::CompleteAttached);
    if (event == Event::OperationTemporary) return result(State::Detached, Action::CompleteTemporary);
    if (event == Event::OperationIncompatible) return result(State::Detached, Action::CompleteIncompatible);
    if (event == Event::OperationFailed) return result(State::Detached, Action::CompleteFailed);
    if (event == Event::OperationRetryPrepare) return result(State::Repreparing, Action::Prepare);
    if (event == Event::OperationRecreateActivation) return result(State::RecreatingForActivation, Action::RecreateWindow);
    if (event == Event::OperationRecreateFailure) return result(State::RecreatingForFailure, Action::RecreateWindow);
    if (event == Event::OperationRecreateIncompatible) return result(State::RecreatingForIncompatible, Action::RecreateWindow);
    if (event == Event::OperationNeedsDetach) return result(State::Detaching, Action::BeginUserDetach);
  }
  if (state == State::RecreatingForPrepare) {
    if (event == Event::WindowRecreated) return result(State::Preparing, Action::Prepare);
    if (event == Event::WindowRecreationFailed) return result(State::Detached, Action::CompleteFailed);
  }
  if (state == State::RecreatingForActivation) {
    if (event == Event::WindowRecreated) return result(State::Repreparing, Action::Prepare);
    if (event == Event::WindowRecreationFailed) return result(State::Detached, Action::CompleteFailed);
  }
  if (state == State::RecreatingForFailure && (event == Event::WindowRecreated || event == Event::WindowRecreationFailed))
    return result(State::Detached, Action::CompleteFailed);
  if (state == State::RecreatingForIncompatible && event == Event::WindowRecreated) return result(State::Detached, Action::CompleteIncompatible);
  if (state == State::RecreatingForIncompatible && event == Event::WindowRecreationFailed) return result(State::Detached, Action::CompleteFailed);

  const bool activeOperation = state == State::Preparing || state == State::RecreatingForPrepare || state == State::Prepared || state == State::Finalizing ||
                               state == State::Repreparing || state == State::RecreatingForActivation || state == State::RecreatingForFailure ||
                               state == State::RecreatingForIncompatible || state == State::Attached;
  if (activeOperation && event == Event::DetachRequested) return result(State::Detaching, Action::BeginUserDetach);
  if (activeOperation && event == Event::HostInvalidated) return result(State::Detaching, Action::BeginHostDetach);
  if (state == State::Detached && event == Event::HostInvalidated) return result(state, Action::NotifyDetachedHostRefresh);
  if (state == State::Detaching) {
    if (event == Event::AcquireRequested) return result(state, Action::CompleteReleaseStatus);
    if (event == Event::HostInvalidated) return result(state, Action::EscalateHostDetach);
    if (event == Event::DetachDue) return result(state, Action::AdvanceDetach);
    if (event == Event::DetachBlocked) return result(state, Action::ScheduleDetach);
    if (event == Event::DetachCompleted) return result(State::Detached, Action::CompleteDetach);
  }
  if (state == State::RecreatingAfterDetach && (event == Event::AcquireRequested || event == Event::ActivateRequested))
    return result(state, Action::CompleteReleaseStatus);
  if (state == State::RecreatingAfterDetach && (event == Event::WindowRecreated || event == Event::WindowRecreationFailed))
    return result(State::Detached, Action::CompleteDetach);
  return result(state);
}

ExpectedTransition expectedTransition(const FsmState state, const FsmVariant variant) {
  if (variant == FsmVariant::RequestDisabled) {
    if (state == FsmState::Disabled || state == FsmState::Withdrawn || state == FsmState::Incompatible || state == FsmState::Withdrawing) {
      return {state};
    }
    if (state == FsmState::Failing) return {FsmState::Withdrawing};
    return {FsmState::Withdrawing, FsmAction::ReleaseRequest, true};
  }
  if (variant == FsmVariant::RequestEnabled) {
    if (state == FsmState::Disabled) return {FsmState::Probing, FsmAction::AttachAcquire};
    if (state == FsmState::Withdrawn) return {FsmState::Recovering, FsmAction::AttachAcquire};
    if (state == FsmState::Withdrawing) return {FsmState::Recovering};
    return {state};
  }
  if (variant == FsmVariant::ExternalRefresh) {
    if (state == FsmState::Unavailable) return {FsmState::Probing, FsmAction::AttachAcquire};
    if (state == FsmState::Attaching || state == FsmState::Confirmed) return {FsmState::Refreshing, FsmAction::AttachRefresh};
    return {state};
  }
  if (variant == FsmVariant::RetryDue) {
    if (state == FsmState::Unavailable) return {FsmState::Probing, FsmAction::AttachAcquire};
    if (state == FsmState::Recovering) return {state, FsmAction::AttachAcquire};
    return {state};
  }
  if (variant == FsmVariant::ConfirmationDue)
    return state == FsmState::Attaching ? ExpectedTransition{state, FsmAction::VerifyConfirmation} : ExpectedTransition{state};
  if (variant == FsmVariant::HealthDue) return state == FsmState::Confirmed ? ExpectedTransition{state, FsmAction::VerifyHealth} : ExpectedTransition{state};
  if (variant == FsmVariant::PresentationChanged) {
    if (state == FsmState::Attaching || state == FsmState::Refreshing || state == FsmState::Confirmed) return {state, FsmAction::VerifyChanged};
    return {state};
  }
  if (variant == FsmVariant::ActivationAuthorized)
    return state == FsmState::Activating ? ExpectedTransition{state, FsmAction::Activate} : ExpectedTransition{state};
  if (variant == FsmVariant::ReleaseBlocked)
    return state == FsmState::Disabled || state == FsmState::Withdrawn || state == FsmState::Incompatible || state == FsmState::Withdrawing
               ? ExpectedTransition{state}
               : ExpectedTransition{FsmState::Failing};
  if (variant == FsmVariant::ReleaseCompleted) {
    if (state == FsmState::Recovering) return {state, FsmAction::AttachAcquire};
    if (state == FsmState::Withdrawing) return {FsmState::Withdrawn};
    if (state == FsmState::Failing) return {FsmState::Unavailable};
    return {state};
  }
  if (variant == FsmVariant::HostReleaseStarted)
    return state == FsmState::Disabled || state == FsmState::Withdrawn || state == FsmState::Incompatible || state == FsmState::Withdrawing ||
                   state == FsmState::Failing
               ? ExpectedTransition{state}
               : ExpectedTransition{FsmState::Recovering};
  if (variant == FsmVariant::HostReleaseCompleted) {
    if (state == FsmState::Disabled || state == FsmState::Withdrawn || state == FsmState::Incompatible) return {state};
    if (state == FsmState::Withdrawing) return {FsmState::Withdrawn};
    if (state == FsmState::Failing) return {FsmState::Unavailable};
    if (state == FsmState::Recovering) return {state, FsmAction::AttachAcquire};
    return {FsmState::Probing, FsmAction::AttachAcquire};
  }

  const int variantValue = static_cast<int>(variant);
  const int firstAttach = static_cast<int>(FsmVariant::AttachPrepared);
  const int lastAttach = static_cast<int>(FsmVariant::AttachFailed);
  if (variantValue >= firstAttach && variantValue <= lastAttach) {
    if (state != FsmState::Probing && state != FsmState::Refreshing && state != FsmState::Recovering) return {state};
    switch (static_cast<OverlayHubView::AttachResult>(variantValue - firstAttach)) {
      case OverlayHubView::AttachResult::Prepared:
        return state == FsmState::Refreshing ? ExpectedTransition{state, FsmAction::Activate} : ExpectedTransition{FsmState::Activating};
      case OverlayHubView::AttachResult::Attached:
        return {FsmState::Failing, FsmAction::ReleaseRecovery, true};
      case OverlayHubView::AttachResult::Retained:
        return state == FsmState::Refreshing ? ExpectedTransition{FsmState::Confirmed}
                                             : ExpectedTransition{FsmState::Failing, FsmAction::ReleaseRecovery, true};
      case OverlayHubView::AttachResult::TemporarilyUnavailable:
        if (state == FsmState::Recovering) return {state, FsmAction::ScheduleRecoverRetry};
        return {FsmState::Unavailable, FsmAction::Suspend, true};
      case OverlayHubView::AttachResult::Incompatible:
        return {FsmState::Incompatible, FsmAction::Suspend, true};
      case OverlayHubView::AttachResult::ReleasePending:
        return {FsmState::Recovering};
      case OverlayHubView::AttachResult::ReleaseBlocked:
        return {FsmState::Failing};
      case OverlayHubView::AttachResult::Failed:
        if (state == FsmState::Recovering) return {state, FsmAction::ScheduleRecoverRetry};
        return {FsmState::Failing, FsmAction::ReleaseRecovery, true};
    }
  }
  const int firstActivation = static_cast<int>(FsmVariant::ActivationPrepared);
  const int lastActivation = static_cast<int>(FsmVariant::ActivationFailed);
  if (variantValue >= firstActivation && variantValue <= lastActivation) {
    if (state != FsmState::Activating && state != FsmState::Refreshing) return {state};
    switch (static_cast<OverlayHubView::AttachResult>(variantValue - firstActivation)) {
      case OverlayHubView::AttachResult::Attached:
        return state == FsmState::Refreshing ? ExpectedTransition{state, FsmAction::VerifyAttach}
                                             : ExpectedTransition{FsmState::Attaching, FsmAction::VerifyAttach};
      case OverlayHubView::AttachResult::Retained:
        return state == FsmState::Refreshing ? ExpectedTransition{FsmState::Confirmed}
                                             : ExpectedTransition{FsmState::Failing, FsmAction::ReleaseRecovery, true};
      case OverlayHubView::AttachResult::TemporarilyUnavailable:
        return {FsmState::Unavailable, FsmAction::Suspend, true};
      case OverlayHubView::AttachResult::Incompatible:
        return {FsmState::Incompatible, FsmAction::Suspend, true};
      case OverlayHubView::AttachResult::ReleasePending:
        return {FsmState::Recovering};
      case OverlayHubView::AttachResult::ReleaseBlocked:
        return {FsmState::Failing};
      case OverlayHubView::AttachResult::Prepared:
      case OverlayHubView::AttachResult::Failed:
        return {FsmState::Failing, FsmAction::ReleaseRecovery, true};
    }
  }
  if (variant == FsmVariant::ReleaseComplete || variant == FsmVariant::ReleasePending) {
    if (state == FsmState::Disabled || state == FsmState::Withdrawn || state == FsmState::Incompatible) return {state};
    if (state == FsmState::Withdrawing || state == FsmState::Failing)
      return variant == FsmVariant::ReleasePending ? ExpectedTransition{state} : ExpectedTransition{FsmState::Unavailable};
    return variant == FsmVariant::ReleasePending ? ExpectedTransition{FsmState::Failing} : ExpectedTransition{FsmState::Unavailable};
  }

  const int verification = variantValue - static_cast<int>(FsmVariant::VerificationAttachConfirmed);
  const int source = verification / 5;
  const auto result = static_cast<OverlayHubView::VerificationResult>(verification % 5);
  if (state == FsmState::Attaching) {
    if (result == OverlayHubView::VerificationResult::Confirmed) return {FsmState::Confirmed};
    if (result == OverlayHubView::VerificationResult::Invalid) return {FsmState::Failing, FsmAction::ReleaseRecovery, true};
    if (result == OverlayHubView::VerificationResult::RefreshRequired) return {FsmState::Refreshing, FsmAction::AttachRefresh};
    if (source == 1) {
      if (result == OverlayHubView::VerificationResult::Pending || result == OverlayHubView::VerificationResult::Retained)
        return {FsmState::Unavailable, FsmAction::Suspend, true};
      return {state};
    }
    return {state};
  }
  if (state == FsmState::Refreshing) {
    if (result == OverlayHubView::VerificationResult::Confirmed || result == OverlayHubView::VerificationResult::Retained) return {FsmState::Confirmed};
    if (result == OverlayHubView::VerificationResult::Pending) return {FsmState::Attaching};
    return {FsmState::Failing, FsmAction::ReleaseRecovery, true};
  }
  if (state == FsmState::Confirmed) {
    if (result == OverlayHubView::VerificationResult::Confirmed || result == OverlayHubView::VerificationResult::Retained) return {state};
    if (result == OverlayHubView::VerificationResult::RefreshRequired) return {FsmState::Refreshing, FsmAction::AttachRefresh};
    return {FsmState::Failing, FsmAction::ReleaseRecovery, true};
  }
  return {state};
}

bool logContains(const std::string_view text) {
  const auto lines = uwf::recentLogLines();
  return std::ranges::any_of(lines, [text](const std::string& line) { return line.find(text) != std::string::npos; });
}

struct StrategyState {
  bool compatible = true;
  int priority = 0;
  TaskbarLayoutStrategy::AttachReadiness readiness = TaskbarLayoutStrategy::AttachReadiness::Ready;
  TaskbarLayoutStrategy::AttachResult commitResult = TaskbarLayoutStrategy::AttachResult::Attached;
  TaskbarLayoutStrategy::AttachResult finalizeResult = TaskbarLayoutStrategy::AttachResult::Attached;
  TaskbarLayoutStrategy::DetachResult rollbackResult = TaskbarLayoutStrategy::DetachResult::Detached;
  TaskbarLayoutStrategy::VerificationResult verificationResult = TaskbarLayoutStrategy::VerificationResult::Confirmed;
  std::deque<TaskbarLayoutStrategy::DetachResult> detachResults;
  std::string name;
  std::shared_ptr<std::vector<std::string>> trace;
  int prepareCalls = 0;
  int commitCalls = 0;
  int finalizeCalls = 0;
  int rollbackCalls = 0;
  int verifyCalls = 0;
  int detachCalls = 0;
  int invalidateCalls = 0;
  std::function<void()> finalizeHook;
  QPointer<QWindow> preparedWindow;
  WId committedWindowId = 0;
  bool requireVisibleAtFinalize = false;
  bool finalizeObservedStableVisibleWindow = false;
};

class FakeTransaction final : public TaskbarLayoutStrategy::AttachTransaction {
 public:
  explicit FakeTransaction(std::shared_ptr<StrategyState> state) : AttachTransaction(state->readiness), m_state(std::move(state)) {}

  TaskbarLayoutStrategy::AttachResult commit() override {
    ++m_state->commitCalls;
    if (m_state->trace) m_state->trace->push_back("commit:" + m_state->name);
    m_state->committedWindowId = m_state->preparedWindow ? m_state->preparedWindow->winId() : 0;
    return m_state->commitResult;
  }

  TaskbarLayoutStrategy::AttachResult finalize() override {
    ++m_state->finalizeCalls;
    if (m_state->finalizeHook) m_state->finalizeHook();
    if (m_state->requireVisibleAtFinalize) {
      const QWindow* const window = m_state->preparedWindow.data();
      const HWND nativeWindow = window ? reinterpret_cast<HWND>(window->winId()) : nullptr;
      m_state->finalizeObservedStableVisibleWindow =
          window && window->winId() == m_state->committedWindowId && window->isVisible() && IsWindowVisible(nativeWindow);
      if (!m_state->finalizeObservedStableVisibleWindow) return TaskbarLayoutStrategy::AttachResult::Invalid;
    }
    return m_state->finalizeResult;
  }

  TaskbarLayoutStrategy::DetachResult rollback() override {
    ++m_state->rollbackCalls;
    if (m_state->trace) m_state->trace->push_back("rollback:" + m_state->name);
    return m_state->rollbackResult;
  }

 private:
  std::shared_ptr<StrategyState> m_state;
};

class FakeStrategy final : public TaskbarLayoutStrategy {
 public:
  explicit FakeStrategy(std::shared_ptr<StrategyState> state) : m_state(std::move(state)) {}

  [[nodiscard]] bool isCompatible() const override { return m_state->compatible; }
  [[nodiscard]] int priority() const override { return m_state->priority; }

  std::unique_ptr<AttachTransaction> prepareAttach(QWindow* window, const QSize&) override {
    ++m_state->prepareCalls;
    m_state->preparedWindow = window;
    if (m_state->trace) m_state->trace->push_back("prepare:" + m_state->name);
    return std::make_unique<FakeTransaction>(m_state);
  }

  [[nodiscard]] VerificationResult verify(const QWindow*, WId) const override {
    ++m_state->verifyCalls;
    return m_state->verificationResult;
  }

  DetachResult detach() override {
    ++m_state->detachCalls;
    if (m_state->detachResults.empty()) return DetachResult::Detached;
    const DetachResult result = m_state->detachResults.front();
    m_state->detachResults.pop_front();
    return result;
  }

  DetachResult invalidate() override {
    ++m_state->invalidateCalls;
    return detach();
  }

 private:
  std::shared_ptr<StrategyState> m_state;
};

std::unique_ptr<TaskbarLayoutStrategy> fakeStrategy(const std::shared_ptr<StrategyState>& state) { return std::make_unique<FakeStrategy>(state); }

class FakeHubView final : public OverlayHubView {
 public:
  FakeHubView(const int viewPriority, const bool compatible = true) : m_priority(viewPriority), m_compatible(compatible) {}

  [[nodiscard]] bool isCompatible() const override { return m_compatible; }
  [[nodiscard]] int priority() const override { return m_priority; }

  void requestEnabled() {
    setPresentationRequested(true);
    if (displayState() == DisplayState::Activating) authorizePresentationActivation();
  }
  void retryNow() {
    requestPresentationRefresh();
    if (displayState() == DisplayState::Activating) authorizePresentationActivation();
  }
  void verifyNow() { notifyPresentationChanged(); }
  [[nodiscard]] bool requested() const { return presentationRequested(); }

  AttachResult attachResult = AttachResult::Attached;
  VerificationResult verificationResult = VerificationResult::Confirmed;
  std::optional<VerificationResult> verificationAfterAttach;
  mutable int verifyCalls = 0;
  int attachCalls = 0;
  int detachCalls = 0;
  int retryInterval = 0;
  int maxRecoverAttempts = 3;
  bool detachReturnsPending = false;
  bool notifyReleaseCompletedDuringDetach = false;
  std::string traceName;
  std::shared_ptr<std::vector<std::string>> trace;
  std::function<void()> activationHook;

  void hostReleaseStarted() { notifyHostPresentationReleaseStarted(); }
  void hostReleaseCompleted() { notifyHostPresentationReleaseCompleted(); }
  void releaseCompleted() { notifyPresentationReleaseCompleted(); }

 private:
  AttachResult acquirePresentation() override {
    ++attachCalls;
    if (trace) trace->push_back(traceName + ":acquire");
    return attachResult == AttachResult::Attached ? AttachResult::Prepared : attachResult;
  }

  AttachResult activatePresentation() override {
    if (trace) trace->push_back(traceName + ":activate");
    if (activationHook) activationHook();
    if (verificationAfterAttach) {
      verificationResult = *verificationAfterAttach;
      verificationAfterAttach.reset();
    }
    if (attachResult == AttachResult::Attached) (void)OverlayHubView::activatePresentation();
    return attachResult;
  }

  [[nodiscard]] VerificationResult verifyPresentation() const override {
    ++verifyCalls;
    return verificationResult;
  }

  ReleaseResult detachPresentation(ReleaseReason) override {
    ++detachCalls;
    if (trace) trace->push_back(traceName + ":detach");
    (void)OverlayHubView::detachPresentation(ReleaseReason::RequestRevoked);
    // Optional: publish Completed before returning Pending, mirroring coordinator
    // flush that overtakes the caller's ReleaseStarted announcement.
    if (notifyReleaseCompletedDuringDetach) notifyPresentationReleaseCompleted();
    return detachReturnsPending ? ReleaseResult::Pending : ReleaseResult::Complete;
  }
  void suspendPresentation() override {
    if (trace) trace->push_back(traceName + ":suspend");
    OverlayHubView::suspendPresentation();
  }
  [[nodiscard]] int retryIntervalMs(int consecutiveFailures) const override {
    observedFailureCounts.push_back(consecutiveFailures);
    return retryInterval;
  }
  [[nodiscard]] int maxExclusiveRecoverAttempts() const override { return maxRecoverAttempts; }

 public:
  mutable std::vector<int> observedFailureCounts;

  int m_priority;
  bool m_compatible;
};

class CoordinatorBackedHubView final : public OverlayHubView {
 public:
  explicit CoordinatorBackedHubView(const std::shared_ptr<StrategyState>& strategyState) {
    std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
    strategies.push_back(fakeStrategy(strategyState));
    m_coordinator = std::make_unique<TaskbarLayoutCoordinator>(std::move(strategies), [this](const TaskbarLayoutCoordinator::DetachEvent event) {
      if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Started) {
        notifyHostPresentationReleaseStarted();
      } else if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Blocked) {
        notifyPresentationReleaseBlocked();
      } else if (event.reason == TaskbarLayoutCoordinator::DetachReason::HostInvalidated) {
        notifyHostPresentationReleaseCompleted();
      } else {
        notifyPresentationReleaseCompleted();
      }
    });
  }

  [[nodiscard]] int priority() const override { return 200; }
  void verifyNow() { notifyPresentationChanged(); }
  void deliverTaskbarCreated() { TaskbarLayoutCoordinatorTestAccess::deliverTaskbarCreated(*m_coordinator); }
  void advanceDetaching() { TaskbarLayoutCoordinatorTestAccess::advanceDetaching(*m_coordinator); }
  int retryInterval = 1;

 private:
  AttachResult acquirePresentation() override {
    m_windowId = m_window.winId();
    const auto result = m_coordinator->prepareAttach(&m_window, QSize(80, 24), {}, {}, [this]() -> QWindow* {
      m_window.destroy();
      m_windowId = m_window.winId();
      return &m_window;
    });
    switch (result) {
      case TaskbarLayoutCoordinator::AttachResult::Prepared:
        return AttachResult::Prepared;
      case TaskbarLayoutCoordinator::AttachResult::Attached:
        return AttachResult::Failed;
      case TaskbarLayoutCoordinator::AttachResult::Retained:
        return AttachResult::Retained;
      case TaskbarLayoutCoordinator::AttachResult::TemporarilyUnavailable:
        return AttachResult::TemporarilyUnavailable;
      case TaskbarLayoutCoordinator::AttachResult::Incompatible:
        return AttachResult::Incompatible;
      case TaskbarLayoutCoordinator::AttachResult::ReleasePending:
        return AttachResult::ReleasePending;
      case TaskbarLayoutCoordinator::AttachResult::ReleaseBlocked:
        return AttachResult::ReleaseBlocked;
      case TaskbarLayoutCoordinator::AttachResult::Failed:
        return AttachResult::Failed;
    }
    Q_UNREACHABLE_RETURN(AttachResult::Failed);
  }

  AttachResult activatePresentation() override {
    const auto result = m_coordinator->activatePrepared();
    switch (result) {
      case TaskbarLayoutCoordinator::AttachResult::Attached:
        return AttachResult::Attached;
      case TaskbarLayoutCoordinator::AttachResult::Retained:
        return AttachResult::Retained;
      case TaskbarLayoutCoordinator::AttachResult::TemporarilyUnavailable:
        return AttachResult::TemporarilyUnavailable;
      case TaskbarLayoutCoordinator::AttachResult::Incompatible:
        return AttachResult::Incompatible;
      case TaskbarLayoutCoordinator::AttachResult::ReleasePending:
        return AttachResult::ReleasePending;
      case TaskbarLayoutCoordinator::AttachResult::ReleaseBlocked:
        return AttachResult::ReleaseBlocked;
      case TaskbarLayoutCoordinator::AttachResult::Prepared:
      case TaskbarLayoutCoordinator::AttachResult::Failed:
        return AttachResult::Failed;
    }
    Q_UNREACHABLE_RETURN(AttachResult::Failed);
  }

  [[nodiscard]] VerificationResult verifyPresentation() const override {
    switch (m_coordinator->verify(&m_window, m_windowId)) {
      case TaskbarLayoutStrategy::VerificationResult::Confirmed:
        return VerificationResult::Confirmed;
      case TaskbarLayoutStrategy::VerificationResult::Retained:
        return VerificationResult::Retained;
      case TaskbarLayoutStrategy::VerificationResult::RefreshRequired:
        return VerificationResult::RefreshRequired;
      case TaskbarLayoutStrategy::VerificationResult::Invalid:
        return VerificationResult::Invalid;
    }
    Q_UNREACHABLE_RETURN(VerificationResult::Invalid);
  }

  void suspendPresentation() override {}
  ReleaseResult detachPresentation(ReleaseReason) override {
    return m_coordinator->detach() == TaskbarLayoutCoordinator::DetachStatus::Pending ? ReleaseResult::Pending : ReleaseResult::Complete;
  }
  [[nodiscard]] int retryIntervalMs(int) const override { return retryInterval; }

  QWindow m_window;
  WId m_windowId = 0;
  std::unique_ptr<TaskbarLayoutCoordinator> m_coordinator;
};

TaskbarLayoutCoordinator::AttachResult attachCoordinator(TaskbarLayoutCoordinator& coordinator, QWindow* const window, const QSize& size,
                                                         const TaskbarLayoutCoordinator::VisibilityCommit& show = {},
                                                         const TaskbarLayoutCoordinator::VisibilityRollback& hide = {}) {
  const auto prepared = coordinator.prepareAttach(window, size, show, hide, [window]() -> QWindow* {
    if (!window) return nullptr;
    window->destroy();
    (void)window->winId();
    return window;
  });
  return prepared == TaskbarLayoutCoordinator::AttachResult::Prepared ? coordinator.activatePrepared() : prepared;
}

void testStaticCompatibilityFiltering() {
  auto state = std::make_shared<StrategyState>();
  state->compatible = false;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  TaskbarLayoutCoordinator coordinator(std::move(strategies), {});
  QVERIFY2(!coordinator.isCompatible(), "incompatible strategy must be removed at construction");
}

void testPriorityFallbackAndTemporaryResult() {
  auto high = std::make_shared<StrategyState>();
  const auto trace = std::make_shared<std::vector<std::string>>();
  high->name = "high";
  high->trace = trace;
  high->priority = 200;
  high->readiness = TaskbarLayoutStrategy::AttachReadiness::TemporarilyUnavailable;
  auto low = std::make_shared<StrategyState>();
  low->name = "low";
  low->trace = trace;
  low->priority = 100;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(low));
  strategies.push_back(fakeStrategy(high));
  TaskbarLayoutCoordinator coordinator(std::move(strategies), {});
  QWindow window;
  const auto result = attachCoordinator(coordinator, &window, QSize(80, 24));
  QVERIFY2(result == TaskbarLayoutCoordinator::AttachResult::Attached, "lower priority strategy must provide fallback");
  QVERIFY2(*trace == std::vector<std::string>({"prepare:high", "prepare:low", "commit:low"}), "strategies must be evaluated in priority order");

  auto incompatibleHigh = std::make_shared<StrategyState>();
  auto availableLow = std::make_shared<StrategyState>();
  incompatibleHigh->priority = 200;
  incompatibleHigh->commitResult = TaskbarLayoutStrategy::AttachResult::Incompatible;
  availableLow->priority = 100;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> incompatibleFallbackStrategies;
  incompatibleFallbackStrategies.push_back(fakeStrategy(availableLow));
  incompatibleFallbackStrategies.push_back(fakeStrategy(incompatibleHigh));
  TaskbarLayoutCoordinator incompatibleFallbackCoordinator(std::move(incompatibleFallbackStrategies), {});
  QVERIFY2(attachCoordinator(incompatibleFallbackCoordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::Attached,
           "an incompatible strategy must yield to a lower-priority strategy");
  QCOMPARE(incompatibleHigh->rollbackCalls, 1);
  QCOMPARE(availableLow->commitCalls, 1);

  auto activeIncompatible = std::make_shared<StrategyState>();
  const auto activeTrace = std::make_shared<std::vector<std::string>>();
  activeIncompatible->name = "active";
  activeIncompatible->trace = activeTrace;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> activeStrategies;
  activeStrategies.push_back(fakeStrategy(activeIncompatible));
  TaskbarLayoutCoordinator activeCoordinator(std::move(activeStrategies), {});
  QCOMPARE(attachCoordinator(activeCoordinator, &window, QSize(80, 24)), TaskbarLayoutCoordinator::AttachResult::Attached);
  activeTrace->clear();
  activeIncompatible->commitResult = TaskbarLayoutStrategy::AttachResult::Incompatible;
  const auto activeOutcome =
      activeCoordinator.prepareAttach(&window, QSize(80, 24), {}, [activeTrace]() { activeTrace->push_back("hide:active"); }, [&window]() { return &window; });
  QCOMPARE(activeOutcome, TaskbarLayoutCoordinator::AttachResult::Incompatible);
  QVERIFY2(*activeTrace == std::vector<std::string>({"prepare:active", "commit:active", "hide:active", "rollback:active"}),
           "an active incompatible attachment must be hidden before rollback restores its top-level parent");

  auto incompatibleWithUnavailableHigh = std::make_shared<StrategyState>();
  auto unavailableLow = std::make_shared<StrategyState>();
  incompatibleWithUnavailableHigh->priority = 200;
  incompatibleWithUnavailableHigh->commitResult = TaskbarLayoutStrategy::AttachResult::Incompatible;
  unavailableLow->priority = 100;
  unavailableLow->readiness = TaskbarLayoutStrategy::AttachReadiness::Unavailable;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> unavailableFallbackStrategies;
  unavailableFallbackStrategies.push_back(fakeStrategy(unavailableLow));
  unavailableFallbackStrategies.push_back(fakeStrategy(incompatibleWithUnavailableHigh));
  TaskbarLayoutCoordinator unavailableFallbackCoordinator(std::move(unavailableFallbackStrategies), {});
  QVERIFY2(attachCoordinator(unavailableFallbackCoordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::Failed,
           "one incompatible strategy must not terminalize an endpoint whose remaining strategy is currently unavailable");
  unavailableLow->readiness = TaskbarLayoutStrategy::AttachReadiness::Ready;
  QVERIFY2(attachCoordinator(unavailableFallbackCoordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::Attached &&
               incompatibleWithUnavailableHigh->commitCalls == 1,
           "a remaining strategy must recover without re-probing the process-incompatible strategy");

  auto destructiveIncompatibleHigh = std::make_shared<StrategyState>();
  auto availableAfterReset = std::make_shared<StrategyState>();
  destructiveIncompatibleHigh->priority = 200;
  destructiveIncompatibleHigh->commitResult = TaskbarLayoutStrategy::AttachResult::Incompatible;
  destructiveIncompatibleHigh->rollbackResult = TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed;
  availableAfterReset->priority = 100;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> destructiveFallbackStrategies;
  destructiveFallbackStrategies.push_back(fakeStrategy(availableAfterReset));
  destructiveFallbackStrategies.push_back(fakeStrategy(destructiveIncompatibleHigh));
  TaskbarLayoutCoordinator destructiveFallbackCoordinator(std::move(destructiveFallbackStrategies), {});
  const HWND destroyedWindow = reinterpret_cast<HWND>(window.winId());
  const auto resetOutcome = attachCoordinator(destructiveFallbackCoordinator, &window, QSize(80, 24));
  QVERIFY2(resetOutcome == TaskbarLayoutCoordinator::AttachResult::Attached && !IsWindow(destroyedWindow) && destructiveIncompatibleHigh->commitCalls == 1 &&
               availableAfterReset->commitCalls == 1,
           "the coordinator FSM must recreate a destroyed HWND before the remaining strategy attaches");

  auto finalizeIncompatibleHigh = std::make_shared<StrategyState>();
  auto availableAfterFinalize = std::make_shared<StrategyState>();
  finalizeIncompatibleHigh->priority = 200;
  finalizeIncompatibleHigh->finalizeResult = TaskbarLayoutStrategy::AttachResult::Incompatible;
  availableAfterFinalize->priority = 100;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> finalizeFallbackStrategies;
  finalizeFallbackStrategies.push_back(fakeStrategy(availableAfterFinalize));
  finalizeFallbackStrategies.push_back(fakeStrategy(finalizeIncompatibleHigh));
  TaskbarLayoutCoordinator finalizeFallbackCoordinator(std::move(finalizeFallbackStrategies), {});
  const auto finalizeContinuation = attachCoordinator(finalizeFallbackCoordinator, &window, QSize(80, 24));
  QVERIFY2(finalizeContinuation == TaskbarLayoutCoordinator::AttachResult::Attached && finalizeIncompatibleHigh->finalizeCalls == 1 &&
               availableAfterFinalize->commitCalls == 1 && availableAfterFinalize->finalizeCalls == 1,
           "finalize incompatibility must remain inside the FSM until the lower strategy is fully attached");

  auto temporary = std::make_shared<StrategyState>();
  temporary->readiness = TaskbarLayoutStrategy::AttachReadiness::TemporarilyUnavailable;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> temporaryStrategies;
  temporaryStrategies.push_back(fakeStrategy(temporary));
  TaskbarLayoutCoordinator temporaryCoordinator(std::move(temporaryStrategies), {});
  QVERIFY2(attachCoordinator(temporaryCoordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::TemporarilyUnavailable,
           "temporary readiness must not collapse into a permanent failure");

  auto incompatible = std::make_shared<StrategyState>();
  incompatible->commitResult = TaskbarLayoutStrategy::AttachResult::Incompatible;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> incompatibleStrategies;
  incompatibleStrategies.push_back(fakeStrategy(incompatible));
  int incompatibleHostRefreshes = 0;
  TaskbarLayoutCoordinator incompatibleCoordinator(std::move(incompatibleStrategies),
                                                   [&incompatibleHostRefreshes](const TaskbarLayoutCoordinator::DetachEvent&) { ++incompatibleHostRefreshes; });
  QVERIFY2(attachCoordinator(incompatibleCoordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::Incompatible,
           "an incompatible endpoint must remain distinct from retryable and generic failures");
  TaskbarLayoutCoordinatorTestAccess::deliverTaskbarCreated(incompatibleCoordinator);
  QCoreApplication::processEvents();
  QCOMPARE(incompatibleHostRefreshes, 0);
}

void testRetainedAttachmentIsDistinctFromReleasedTemporaryFailure() {
  auto state = std::make_shared<StrategyState>();
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  TaskbarLayoutCoordinator coordinator(std::move(strategies), {});
  QWindow window;
  QCOMPARE(attachCoordinator(coordinator, &window, QSize(80, 24)), TaskbarLayoutCoordinator::AttachResult::Attached);

  int hideCalls = 0;
  state->readiness = TaskbarLayoutStrategy::AttachReadiness::TemporarilyUnavailable;
  const auto retained = coordinator.prepareAttach(
      &window, QSize(80, 24), {}, [&hideCalls]() { ++hideCalls; },
      [&window]() -> QWindow* {
        window.destroy();
        (void)window.winId();
        return &window;
      });
  QCOMPARE(retained, TaskbarLayoutCoordinator::AttachResult::Retained);
  QCOMPARE(hideCalls, 0);
  QCOMPARE(coordinator.verify(&window, window.winId()), TaskbarLayoutStrategy::VerificationResult::Confirmed);

  state->readiness = TaskbarLayoutStrategy::AttachReadiness::Ready;
  state->commitResult = TaskbarLayoutStrategy::AttachResult::TemporarilyUnavailable;
  const auto released = coordinator.prepareAttach(
      &window, QSize(80, 24), {}, [&hideCalls]() { ++hideCalls; },
      [&window]() -> QWindow* {
        window.destroy();
        (void)window.winId();
        return &window;
      });
  QCOMPARE(released, TaskbarLayoutCoordinator::AttachResult::TemporarilyUnavailable);
  QCOMPARE(hideCalls, 1);
  QCOMPARE(state->rollbackCalls, 1);
  QCOMPARE(coordinator.verify(&window, window.winId()), TaskbarLayoutStrategy::VerificationResult::Invalid);
}

void testAttachDetachLifecycle() {
  auto state = std::make_shared<StrategyState>();
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  int refreshCalls = 0;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), [&refreshCalls](const TaskbarLayoutCoordinator::DetachEvent event) {
    if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Completed) ++refreshCalls;
  });
  QWindow window;
  int showCalls = 0;
  int hideCalls = 0;
  QVERIFY2(attachCoordinator(
               coordinator, &window, QSize(80, 24), [&showCalls]() { ++showCalls; }, [&hideCalls]() { ++hideCalls; }) ==
               TaskbarLayoutCoordinator::AttachResult::Attached,
           "normal attach must reach Attached");
  QVERIFY2(showCalls == 1 && state->finalizeCalls == 1, "visibility and finalize must be committed exactly once");
  QVERIFY2(coordinator.verify(&window, window.winId()) == TaskbarLayoutStrategy::VerificationResult::Confirmed, "attached strategy must own verification");

  coordinator.detach();
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY2(hideCalls >= 1 && state->detachCalls == 1 && refreshCalls == 1, "detach must hide, release, and request one refresh");
  QVERIFY2(coordinator.verify(&window, window.winId()) == TaskbarLayoutStrategy::VerificationResult::Invalid, "detached coordinator must reject verification");
}

void testActivatePreparedReportsStatusForEveryCoordinatorState() {
  auto state = std::make_shared<StrategyState>();
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  TaskbarLayoutCoordinator coordinator(std::move(strategies), {});
  QWindow window;

  QCOMPARE(coordinator.activatePrepared(), TaskbarLayoutCoordinator::AttachResult::Failed);

  QVERIFY2(attachCoordinator(coordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::Attached, "fixture attach must succeed");
  QCOMPARE(state->finalizeCalls, 1);
  QCOMPARE(coordinator.activatePrepared(), TaskbarLayoutCoordinator::AttachResult::Attached);
  QCOMPARE(state->finalizeCalls, 1);
  QCOMPARE(state->detachCalls, 0);

  QVERIFY2(coordinator.detach() == TaskbarLayoutCoordinator::DetachStatus::Pending, "detach must begin an async release");
  QCOMPARE(coordinator.activatePrepared(), TaskbarLayoutCoordinator::AttachResult::ReleasePending);
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QCOMPARE(coordinator.activatePrepared(), TaskbarLayoutCoordinator::AttachResult::Failed);
}

void testIgnoredAcquireDoesNotReplacePreparedRequest() {
  auto state = std::make_shared<StrategyState>();
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  TaskbarLayoutCoordinator coordinator(std::move(strategies), {});
  QWindow firstWindow;
  QWindow secondWindow;
  int firstShowCalls = 0;
  int secondShowCalls = 0;

  QCOMPARE(coordinator.prepareAttach(
               &firstWindow, QSize(80, 24), [&firstShowCalls]() { ++firstShowCalls; }, {}, [&firstWindow]() -> QWindow* { return &firstWindow; }),
           TaskbarLayoutCoordinator::AttachResult::Prepared);
  QCOMPARE(coordinator.prepareAttach(
               &secondWindow, QSize(120, 32), [&secondShowCalls]() { ++secondShowCalls; }, {}, [&secondWindow]() -> QWindow* { return &secondWindow; }),
           TaskbarLayoutCoordinator::AttachResult::Failed);
  QCOMPARE(coordinator.activatePrepared(), TaskbarLayoutCoordinator::AttachResult::Attached);

  QCOMPARE(state->prepareCalls, 1);
  QCOMPARE(state->preparedWindow.data(), &firstWindow);
  QCOMPARE(firstShowCalls, 1);
  QCOMPARE(secondShowCalls, 0);
}

void testMalformedAcquirePayloadConvergesToDetached() {
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), {});
  QCOMPARE(TaskbarLayoutCoordinatorTestAccess::deliverAcquireWithoutPayload(coordinator), TaskbarLayoutCoordinator::AttachResult::Failed);
  QVERIFY(TaskbarLayoutCoordinatorTestAccess::isDetached(coordinator));
}

void testDestroyedPreparedWindowReleasesTransaction() {
  auto state = std::make_shared<StrategyState>();
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  int completed = 0;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), [&completed](const TaskbarLayoutCoordinator::DetachEvent event) {
    completed += event.phase == TaskbarLayoutCoordinator::DetachPhase::Completed;
  });
  auto window = std::make_unique<QWindow>();
  QCOMPARE(coordinator.prepareAttach(window.get(), QSize(80, 24), {}, {}, [&window]() -> QWindow* { return window.get(); }),
           TaskbarLayoutCoordinator::AttachResult::Prepared);
  window.reset();

  QCOMPARE(coordinator.activatePrepared(), TaskbarLayoutCoordinator::AttachResult::ReleasePending);
  QCOMPARE(state->rollbackCalls, 1);
  QCOMPARE(completed, 1);
  QVERIFY(TaskbarLayoutCoordinatorTestAccess::isDetached(coordinator));
  QVERIFY(!TaskbarLayoutCoordinatorTestAccess::ownsAttachResources(coordinator));
  QVERIFY(!TaskbarLayoutCoordinatorTestAccess::hasRequest(coordinator));
}

void testVisibilityIsCommittedBeforeFinalization() {
  auto state = std::make_shared<StrategyState>();
  state->requireVisibleAtFinalize = true;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  QWindow window;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), {});
  const auto result = attachCoordinator(coordinator, &window, QSize(80, 24), [&window]() { window.show(); }, [&window]() { window.hide(); });

  QCOMPARE(result, TaskbarLayoutCoordinator::AttachResult::Attached);
  QVERIFY(state->finalizeObservedStableVisibleWindow);
  QVERIFY(window.isVisible());
  QVERIFY(IsWindowVisible(reinterpret_cast<HWND>(window.winId())));

  coordinator.detach();
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY(!window.isVisible());
}

void testDetachDuringAttachIsSerialized() {
  auto state = std::make_shared<StrategyState>();
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  int refreshCalls = 0;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), [&refreshCalls](const TaskbarLayoutCoordinator::DetachEvent event) {
    if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Completed) ++refreshCalls;
  });
  QWindow window;
  int hideCalls = 0;
  state->finalizeHook = [&coordinator]() { coordinator.detach(); };
  const auto result = attachCoordinator(coordinator, &window, QSize(80, 24), {}, [&hideCalls]() { ++hideCalls; });
  QVERIFY2(result == TaskbarLayoutCoordinator::AttachResult::ReleasePending, "detach during attach must expose the pending release state");
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY2(state->finalizeCalls == 1 && state->rollbackCalls == 1 && state->detachCalls == 0,
           "detach during finalize must roll back the prepared transaction without a second strategy detach");
  QVERIFY2(hideCalls >= 1 && refreshCalls == 1, "serialized detach must finish through the normal completion path");
  QVERIFY2(!TaskbarLayoutCoordinatorTestAccess::hasRequest(coordinator), "release completion must discard the prepared request callbacks");
}

void testRollbackFailureEntersDetaching() {
  auto state = std::make_shared<StrategyState>();
  state->commitResult = TaskbarLayoutStrategy::AttachResult::Invalid;
  state->rollbackResult = TaskbarLayoutStrategy::DetachResult::Failed;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  int refreshCalls = 0;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), [&refreshCalls](const TaskbarLayoutCoordinator::DetachEvent event) {
    if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Completed) ++refreshCalls;
  });
  QWindow window;
  QVERIFY2(attachCoordinator(coordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::ReleasePending,
           "rollback failure must be reported to the caller");
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY2(state->rollbackCalls == 1 && state->detachCalls == 1 && refreshCalls == 1,
           "rollback failure must enter Detaching and complete cleanup asynchronously");
}

void testHostInvalidationIsSerialized() {
  auto state = std::make_shared<StrategyState>();
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  int refreshCalls = 0;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), [&refreshCalls](const TaskbarLayoutCoordinator::DetachEvent event) {
    if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Completed) ++refreshCalls;
  });
  QWindow window;
  state->finalizeHook = [&coordinator]() { TaskbarLayoutCoordinatorTestAccess::invalidateHost(coordinator); };
  const auto result = attachCoordinator(coordinator, &window, QSize(80, 24));
  QVERIFY2(result == TaskbarLayoutCoordinator::AttachResult::ReleasePending, "host invalidation during attach must expose the pending release state");
  QVERIFY2(TaskbarLayoutCoordinatorTestAccess::isDetaching(coordinator), "host invalidation must enter Detaching after the transaction");
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY2(state->invalidateCalls == 1 && refreshCalls == 1, "host invalidation must use the strategy invalidation path exactly once");
}

void testHostInvalidationDominatesUserDetach() {
  auto state = std::make_shared<StrategyState>();
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  int refreshCalls = 0;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), [&refreshCalls](const TaskbarLayoutCoordinator::DetachEvent event) {
    if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Completed) ++refreshCalls;
  });
  QWindow window;
  QVERIFY2(attachCoordinator(coordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::Attached, "setup attach must succeed");

  coordinator.detach();
  TaskbarLayoutCoordinatorTestAccess::deliverTaskbarCreated(coordinator);
  TaskbarLayoutCoordinatorTestAccess::deliverTaskbarCreated(coordinator);
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY2(state->detachCalls == 1 && state->invalidateCalls == 1 && refreshCalls == 1,
           "host invalidation must dominate pending user cleanup and complete through one invalidation pass");
}

void testHostInvalidationRestartsBlockedReleaseContract() {
  auto state = std::make_shared<StrategyState>();
  state->detachResults = {TaskbarLayoutStrategy::DetachResult::Failed, TaskbarLayoutStrategy::DetachResult::Failed,
                          TaskbarLayoutStrategy::DetachResult::Detached};
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  std::vector<TaskbarLayoutCoordinator::DetachEvent> events;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), [&events](const TaskbarLayoutCoordinator::DetachEvent event) { events.push_back(event); });
  QWindow window;
  QCOMPARE(attachCoordinator(coordinator, &window, QSize(80, 24)), TaskbarLayoutCoordinator::AttachResult::Attached);

  QCOMPARE(coordinator.detach(), TaskbarLayoutCoordinator::DetachStatus::Pending);
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QCOMPARE(events.size(), std::size_t{1});
  QCOMPARE(events.back().phase, TaskbarLayoutCoordinator::DetachPhase::Blocked);
  QCOMPARE(events.back().reason, TaskbarLayoutCoordinator::DetachReason::PresentationReleased);

  TaskbarLayoutCoordinatorTestAccess::deliverTaskbarCreated(coordinator);
  QCOMPARE(events.size(), std::size_t{2});
  QCOMPARE(events.back().phase, TaskbarLayoutCoordinator::DetachPhase::Started);
  QCOMPARE(events.back().reason, TaskbarLayoutCoordinator::DetachReason::HostInvalidated);
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QCOMPARE(events.size(), std::size_t{3});
  QCOMPARE(events.back().phase, TaskbarLayoutCoordinator::DetachPhase::Blocked);
  QCOMPARE(events.back().reason, TaskbarLayoutCoordinator::DetachReason::HostInvalidated);

  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QCOMPARE(events.size(), std::size_t{4});
  QCOMPARE(events.back().phase, TaskbarLayoutCoordinator::DetachPhase::Completed);
  QCOMPARE(events.back().reason, TaskbarLayoutCoordinator::DetachReason::HostInvalidated);
}

void testDetachRetryAndNativeWindowReset() {
  auto state = std::make_shared<StrategyState>();
  state->detachResults = {TaskbarLayoutStrategy::DetachResult::Failed, TaskbarLayoutStrategy::DetachResult::Failed, TaskbarLayoutStrategy::DetachResult::Failed,
                          TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed};
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  int refreshCalls = 0;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), [&](const TaskbarLayoutCoordinator::DetachEvent event) {
    if (event.phase != TaskbarLayoutCoordinator::DetachPhase::Completed) return;
    ++refreshCalls;
  });
  QWindow window;
  QVERIFY2(attachCoordinator(coordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::Attached, "setup attach must succeed");
  const WId originalWindow = window.winId();
  coordinator.detach();

  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY2(TaskbarLayoutCoordinatorTestAccess::retryInterval(coordinator) == 1000 && refreshCalls == 0, "first detach failure must schedule a fast retry");
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY2(TaskbarLayoutCoordinatorTestAccess::retryInterval(coordinator) == 1000 && refreshCalls == 0, "second detach failure must remain a fast retry");
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY2(TaskbarLayoutCoordinatorTestAccess::retryInterval(coordinator) == 5000 && refreshCalls == 0, "third detach failure must enter slow retry");
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY2(refreshCalls == 1 && window.winId() != originalWindow,
           "the coordinator must recreate a destroyed native window before publishing detach completion");
}

void testAttachNativeWindowResetRecovery() {
  auto state = std::make_shared<StrategyState>();
  state->commitResult = TaskbarLayoutStrategy::AttachResult::Invalid;
  state->rollbackResult = TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  TaskbarLayoutCoordinator coordinator(std::move(strategies), {});
  QWindow window;

  const WId rejectedWindow = window.winId();
  const auto failedOutcome = attachCoordinator(coordinator, &window, QSize(80, 24));
  QVERIFY2(failedOutcome == TaskbarLayoutCoordinator::AttachResult::Failed && window.winId() != rejectedWindow,
           "a rejected native parent must be recreated before the failed operation completes");
  state->commitResult = TaskbarLayoutStrategy::AttachResult::Attached;
  state->rollbackResult = TaskbarLayoutStrategy::DetachResult::Detached;
  QVERIFY2(attachCoordinator(coordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::Attached,
           "coordinator must permit a clean attach immediately after native-window recreation");

  auto incompatibleState = std::make_shared<StrategyState>();
  incompatibleState->commitResult = TaskbarLayoutStrategy::AttachResult::Incompatible;
  incompatibleState->rollbackResult = TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> incompatibleStrategies;
  incompatibleStrategies.push_back(fakeStrategy(incompatibleState));
  TaskbarLayoutCoordinator incompatibleCoordinator(std::move(incompatibleStrategies), {});
  QWindow incompatibleWindow;
  const WId incompatibleNativeWindow = incompatibleWindow.winId();
  const auto incompatibleOutcome = attachCoordinator(incompatibleCoordinator, &incompatibleWindow, QSize(80, 24));
  QVERIFY2(incompatibleOutcome == TaskbarLayoutCoordinator::AttachResult::Incompatible && incompatibleWindow.winId() != incompatibleNativeWindow,
           "terminal incompatibility must still finish native-window recreation before returning");

  auto failedRecreationState = std::make_shared<StrategyState>();
  failedRecreationState->commitResult = TaskbarLayoutStrategy::AttachResult::Incompatible;
  failedRecreationState->rollbackResult = TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> failedRecreationStrategies;
  failedRecreationStrategies.push_back(fakeStrategy(failedRecreationState));
  TaskbarLayoutCoordinator failedRecreationCoordinator(std::move(failedRecreationStrategies), {});
  QWindow failedRecreationWindow;
  QCOMPARE(failedRecreationCoordinator.prepareAttach(&failedRecreationWindow, QSize(80, 24), {}, {}, []() -> QWindow* { return nullptr; }),
           TaskbarLayoutCoordinator::AttachResult::Failed);
}

void testRapidToggleConverges() {
  auto state = std::make_shared<StrategyState>();
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  TaskbarLayoutCoordinator coordinator(std::move(strategies), {});
  QWindow window;
  QVERIFY2(attachCoordinator(coordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::Attached, "setup attach must succeed");
  coordinator.detach();
  QVERIFY2(attachCoordinator(coordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::ReleasePending,
           "attach during cleanup must report release-pending instead of relying on a retry delay");
  coordinator.detach();
  TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
  QVERIFY2(attachCoordinator(coordinator, &window, QSize(80, 24)) == TaskbarLayoutCoordinator::AttachResult::Attached,
           "rapid off/on/off must converge and permit a clean attach");
}

void testCoordinatorDestructorCompletesPendingCleanup() {
  auto state = std::make_shared<StrategyState>();
  state->detachResults = {TaskbarLayoutStrategy::DetachResult::Failed, TaskbarLayoutStrategy::DetachResult::Detached};
  QWindow window;
  {
    std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
    strategies.push_back(fakeStrategy(state));
    TaskbarLayoutCoordinator coordinator(std::move(strategies), {});
    QCOMPARE(attachCoordinator(coordinator, &window, QSize(80, 24)), TaskbarLayoutCoordinator::AttachResult::Attached);
    coordinator.detach();
    TaskbarLayoutCoordinatorTestAccess::advanceDetaching(coordinator);
    QVERIFY(TaskbarLayoutCoordinatorTestAccess::isDetaching(coordinator));
  }
  QCOMPARE(state->detachCalls, 2);
}

void testEnvironmentClassification() {
  using namespace uwf::ui::win11_taskbar;
  uwf::clearLogLines();
  detail::EnvironmentObservation observation;
  observation.taskbarAvailable = true;
  observation.notifyAvailable = true;
  observation.hierarchyValid = true;
  observation.processIdentityValid = true;
  observation.geometryAvailable = true;

  QVERIFY2(detail::classifyEnvironmentObservation(observation).availability == RuntimeAvailability::Available,
           "Taskbar/Notify structural identity and geometry must be sufficient regardless of transient Shell presentation");
  observation.verticalLayout = true;
  QVERIFY2(detail::classifyEnvironmentObservation(observation).availability == RuntimeAvailability::IncompatibleLayout,
           "vertical runtime layout must be incompatible, not system unsupported");
  observation.verticalLayout = false;
  observation.notifyAvailable = false;
  QVERIFY2(detail::classifyEnvironmentObservation(observation).availability == RuntimeAvailability::TemporarilyUnavailable,
           "missing Notify window must be temporary");
  QVERIFY2(logContains("environment incompatible: vertical taskbar") && logContains("environment unavailable: taskbar=true notify=false"),
           "environment diagnostics must identify the exact incompatible and unavailable observations");

  const EnvironmentProbe transient;
  const EnvironmentProbe current{RuntimeAvailability::Available, Environment{}};
  QVERIFY2(detail::resolveRetainedProbe(transient, transient).availability == RuntimeAvailability::TemporarilyUnavailable,
           "two transient observations must retain the existing attachment");
  QVERIFY2(detail::resolveRetainedProbe(transient, current).availability == RuntimeAvailability::Available,
           "a cold current environment must replace an unobservable retained snapshot");

  QVERIFY2(detail::classifyPlacementObservation(false, false) == detail::PlacementObservation::Retained,
           "uncalculable transient geometry must retain the attachment");
  QVERIFY2(detail::classifyPlacementObservation(true, false) == detail::PlacementObservation::RefreshRequired,
           "changed geometry must request an in-place refresh");
  QVERIFY2(detail::classifyPlacementObservation(true, true) == detail::PlacementObservation::Confirmed, "matching geometry must remain confirmed");
}

void testScopedDpiAwarenessRestoration() {
  using uwf::ui::win11_taskbar::ScopedThreadDpiAwareness;
  QWindow host;
  const HWND hostWindow = reinterpret_cast<HWND>(host.winId());
  const DPI_AWARENESS_CONTEXT originalContext = GetThreadDpiAwarenessContext();
  {
    const ScopedThreadDpiAwareness scope(hostWindow);
    QVERIFY(scope.active());
    QVERIFY(AreDpiAwarenessContextsEqual(GetThreadDpiAwarenessContext(), GetWindowDpiAwarenessContext(hostWindow)));
  }
  QVERIFY(AreDpiAwarenessContextsEqual(GetThreadDpiAwarenessContext(), originalContext));
}

void testHubLifecycleDiagnostics() {
  uwf::clearLogLines();
  FakeHubView view(200);
  view.attachResult = OverlayHubView::AttachResult::TemporarilyUnavailable;
  view.retryInterval = 1000;
  view.requestEnabled();

  QVERIFY2(view.displayState() == OverlayHubView::DisplayState::Unavailable, "diagnostic attach must preserve the unavailable state result");
  QVERIFY2(logContains("display state: view=") && logContains("attach outcome: view=") && logContains("result=temporarily-unavailable") &&
               logContains("retry scheduled: view="),
           "Hub diagnostics must retain one failed outcome and its retry decision");

  view.setPresentationRequested(false);
  QVERIFY2(logContains("to=withdrawn requested=false"), "Hub diagnostics must include the committed presentation release state");
}

void testOverlayViewStateMachineMatrix() {
  const int stateCount = OverlayHubViewTestAccess::stateCount();
  for (int state = 0; state < stateCount; ++state) {
    for (int event = 0; event < OverlayHubViewTestAccess::eventCount(); ++event) {
      const auto result = OverlayHubViewTestAccess::reduce(state, event);
      QVERIFY2(result.nextState >= 0 && result.nextState < stateCount, "every state/event pair must produce a valid deterministic state");
      QVERIFY2(!result.actionBeforeStateChange || result.hasAction, "a pre-transition action flag must always name a concrete action");
    }
  }

  using State = OverlayHubView::DisplayState;
  const auto unavailableRetry = OverlayHubViewTestAccess::retryDue(State::Unavailable);
  QVERIFY2(unavailableRetry.nextState == OverlayHubViewTestAccess::state(State::Probing) && unavailableRetry.hasAction,
           "Unavailable must leave through an explicit retry event and attach action");
  const auto attachingConfirmation = OverlayHubViewTestAccess::confirmationDue(State::Attaching);
  QVERIFY2(attachingConfirmation.nextState == OverlayHubViewTestAccess::state(State::Attaching) && attachingConfirmation.hasAction,
           "Attaching must have an explicit confirmation event");
  const auto recoveringComplete = OverlayHubViewTestAccess::releaseCompleted(State::Recovering);
  QVERIFY2(recoveringComplete.nextState == OverlayHubViewTestAccess::state(State::Recovering) && recoveringComplete.hasAction,
           "Recovering must reattach immediately on release completion");
  QVERIFY2(OverlayHubViewTestAccess::releaseBlocked(State::Recovering).nextState == OverlayHubViewTestAccess::state(State::Failing),
           "a concrete cleanup failure must leave exclusive recovery and expose fallback ownership");
  QVERIFY2(OverlayHubViewTestAccess::releaseCompleted(State::Withdrawing).nextState == OverlayHubViewTestAccess::state(State::Withdrawn),
           "user-revoked release completion must enter Withdrawn when presentation is not re-requested");
  QVERIFY2(OverlayHubViewTestAccess::releaseBlocked(State::Withdrawing).nextState == OverlayHubViewTestAccess::state(State::Withdrawing),
           "blocked cleanup must not erase an outstanding user withdrawal");
  QVERIFY2(OverlayHubViewTestAccess::releaseCompleted(State::Failing).nextState == OverlayHubViewTestAccess::state(State::Unavailable),
           "recovery release completion must yield Unavailable for fallback");
  QVERIFY2(OverlayHubViewTestAccess::hostReleaseCompleted(State::Failing).nextState == OverlayHubViewTestAccess::state(State::Unavailable),
           "HostInvalidated completion during Failing must finish cleanup like ReleaseCompleted");
  QVERIFY2(OverlayHubViewTestAccess::hostReleaseCompleted(State::Withdrawing).nextState == OverlayHubViewTestAccess::state(State::Withdrawn),
           "HostInvalidated completion during Withdrawing must finish user withdraw");
  QVERIFY2(OverlayHubViewTestAccess::hostReleaseStarted(State::Failing).nextState == OverlayHubViewTestAccess::state(State::Failing),
           "HostReleaseStarted must not yank Failing back into exclusive Recovering");
  QVERIFY2(OverlayHubViewTestAccess::hostReleaseStarted(State::Confirmed).nextState == OverlayHubViewTestAccess::state(State::Recovering),
           "host invalidation must enter the exclusive recovery state");
  QVERIFY2(OverlayHubViewTestAccess::releasePending(State::Confirmed).nextState == OverlayHubViewTestAccess::state(State::Failing),
           "ordinary pending cleanup without prior withdraw must enter non-exclusive Failing");

  for (int state = 0; state < stateCount; ++state) {
    const auto disabled = OverlayHubViewTestAccess::requestDisabled(static_cast<State>(state));
    if (static_cast<State>(state) == State::Incompatible) {
      QVERIFY2(disabled.nextState == OverlayHubViewTestAccess::state(State::Incompatible), "RequestDisabled must not erase a process-lifetime incompatibility");
      QVERIFY2(!disabled.hasAction, "an incompatible endpoint owns no presentation resource to release");
      continue;
    }
    if (static_cast<State>(state) == State::Disabled || static_cast<State>(state) == State::Withdrawn || static_cast<State>(state) == State::Withdrawing) {
      QVERIFY2(disabled.nextState == state, "RequestDisabled must be idempotent on idle/withdrawn/withdrawing");
    } else if (static_cast<State>(state) == State::Failing) {
      QVERIFY2(disabled.nextState == OverlayHubViewTestAccess::state(State::Withdrawing) && !disabled.hasAction,
               "disabling during failed cleanup must retain the in-flight release and remember user withdrawal");
    } else {
      QVERIFY2(disabled.nextState == OverlayHubViewTestAccess::state(State::Withdrawing),
               "RequestDisabled must enter Withdrawing so an explicit re-enable can resume via Recovering");
      QVERIFY2(disabled.hasAction && disabled.actionBeforeStateChange, "presentation release must execute before Withdrawing becomes externally visible");
    }
  }

  const auto temporaryAttach = OverlayHubViewTestAccess::attachFinished(State::Probing, OverlayHubView::AttachResult::TemporarilyUnavailable);
  QVERIFY2(temporaryAttach.nextState == OverlayHubViewTestAccess::state(State::Unavailable) && temporaryAttach.actionBeforeStateChange,
           "failed acquisition must hide the high-priority view before fallback becomes eligible");
  const auto retainedRefresh = OverlayHubViewTestAccess::attachFinished(State::Refreshing, OverlayHubView::AttachResult::Retained);
  QVERIFY2(retainedRefresh.nextState == OverlayHubViewTestAccess::state(State::Confirmed) && !retainedRefresh.hasAction,
           "only an explicitly retained attachment may return a refresh operation to Confirmed");
  const auto releasedRefresh = OverlayHubViewTestAccess::attachFinished(State::Refreshing, OverlayHubView::AttachResult::TemporarilyUnavailable);
  QVERIFY2(releasedRefresh.nextState == OverlayHubViewTestAccess::state(State::Unavailable) && releasedRefresh.actionBeforeStateChange,
           "a temporary result after releasing attachment ownership must not be promoted back to Confirmed");
  const auto incompatibleAttach = OverlayHubViewTestAccess::attachFinished(State::Probing, OverlayHubView::AttachResult::Incompatible);
  QVERIFY2(incompatibleAttach.nextState == OverlayHubViewTestAccess::state(State::Incompatible) && incompatibleAttach.actionBeforeStateChange,
           "a process-incompatible endpoint must yield fallback ownership without scheduling retries");
  QVERIFY2(OverlayHubViewTestAccess::retryDue(State::Incompatible).nextState == OverlayHubViewTestAccess::state(State::Incompatible),
           "Incompatible must ignore timer retry events");
  QVERIFY2(OverlayHubViewTestAccess::attachFinished(State::Probing, OverlayHubView::AttachResult::ReleasePending).nextState ==
               OverlayHubViewTestAccess::state(State::Recovering),
           "attach during clean release must wait for its completion event");
  QVERIFY2(OverlayHubViewTestAccess::attachFinished(State::Probing, OverlayHubView::AttachResult::ReleaseBlocked).nextState ==
               OverlayHubViewTestAccess::state(State::Failing),
           "attach after a known release failure must keep fallback eligible");

  const auto confirmationPending = OverlayHubViewTestAccess::verificationObserved(State::Attaching, OverlayHubView::VerificationResult::Pending, true);
  QVERIFY2(confirmationPending.nextState == OverlayHubViewTestAccess::state(State::Unavailable) && confirmationPending.actionBeforeStateChange,
           "confirmation exhaustion must revoke exclusive ownership before fallback activation");
  const auto confirmationRefresh = OverlayHubViewTestAccess::verificationObserved(State::Attaching, OverlayHubView::VerificationResult::RefreshRequired, true);
  QVERIFY2(confirmationRefresh.nextState == OverlayHubViewTestAccess::state(State::Refreshing) && confirmationRefresh.hasAction,
           "confirmation must repair refreshable presentation instead of tearing the attachment down");
  QVERIFY2(OverlayHubViewTestAccess::verificationObserved(State::Refreshing, OverlayHubView::VerificationResult::RefreshRequired, false).hasAction,
           "a failed one-shot placement repair must enter cleanup instead of another refresh loop");
}

void testCoordinatorDiagnostics() {
  auto state = std::make_shared<StrategyState>();
  state->priority = 200;
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(fakeStrategy(state));
  int started = 0;
  int completed = 0;
  TaskbarLayoutCoordinator coordinator(std::move(strategies), [&](const TaskbarLayoutCoordinator::DetachEvent event) {
    started += event.phase == TaskbarLayoutCoordinator::DetachPhase::Started;
    completed += event.phase == TaskbarLayoutCoordinator::DetachPhase::Completed;
  });
  TaskbarLayoutCoordinatorTestAccess::deliverTaskbarCreated(coordinator);
  TaskbarLayoutCoordinatorTestAccess::deliverTaskbarCreated(coordinator);
  QCoreApplication::processEvents();
  QCOMPARE(started, 0);
  QCOMPARE(completed, 1);
  QVERIFY2(!TaskbarLayoutCoordinatorTestAccess::isDetaching(coordinator),
           "TaskbarCreated while detached must request a later probe without publishing a fake release lifecycle");
}

void testFailureCounterResetsOnlyAfterConfirmation() {
  FakeHubView view(200);
  view.attachResult = OverlayHubView::AttachResult::Failed;
  view.retryInterval = 0;
  view.requestEnabled();
  view.retryNow();
  QVERIFY2(view.observedFailureCounts == std::vector<int>({1, 2}), "repeated failures must belong to one monotonically increasing recovery episode");

  view.attachResult = OverlayHubView::AttachResult::Attached;
  view.verificationResult = OverlayHubView::VerificationResult::Confirmed;
  view.retryNow();
  view.verificationResult = OverlayHubView::VerificationResult::Invalid;
  view.verifyNow();
  QVERIFY2(view.observedFailureCounts == std::vector<int>({1, 2, 1}), "a confirmed presentation must reset the recovery episode");
}

void testConfirmationTimeoutRepairsRefreshablePresentation() {
  FakeHubView view(100);
  view.verificationResult = OverlayHubView::VerificationResult::Pending;
  view.requestEnabled();
  QVERIFY2(view.displayState() == OverlayHubView::DisplayState::Attaching && view.attachCalls == 1,
           "initial presentation must wait in Attaching until confirmation succeeds");

  view.verificationResult = OverlayHubView::VerificationResult::RefreshRequired;
  view.verificationAfterAttach = OverlayHubView::VerificationResult::Confirmed;
  OverlayHubViewTestAccess::fireConfirmationDue(view);

  QVERIFY2(view.presentationConfirmed() && view.attachCalls == 2 && view.detachCalls == 0,
           "confirmation must rebuild a refreshable presentation in place instead of releasing ownership");
}

void testReleaseAnnouncementPrecedesSyncCompletion() {
  OverlayHub hub;
  auto high = std::make_unique<FakeHubView>(200);
  FakeHubView* const highView = high.get();
  highView->detachReturnsPending = true;
  highView->notifyReleaseCompletedDuringDetach = true;
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(), "high-priority view must own the initial presentation");

  highView->verificationResult = OverlayHubView::VerificationResult::Invalid;
  highView->verifyNow();

  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Unavailable && lowView->presentationConfirmed(),
           "a sync release completion that overtakes detach() must settle on Unavailable for fallback");
  QVERIFY2(highView->detachCalls == 1, "ownership transfer must perform exactly one release");
}

void testDisableDuringFailedCleanupConvergesToWithdrawn() {
  OverlayHub hub;
  auto view = std::make_unique<FakeHubView>(200);
  FakeHubView* const viewPtr = view.get();
  viewPtr->detachReturnsPending = true;
  viewPtr->retryInterval = 0;
  hub.registerView(std::move(view));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY(viewPtr->presentationConfirmed());

  viewPtr->verificationResult = OverlayHubView::VerificationResult::Invalid;
  viewPtr->verifyNow();
  QVERIFY(viewPtr->displayState() == OverlayHubView::DisplayState::Failing);
  hub.setRequestedVisible(false);
  QVERIFY2(viewPtr->displayState() == OverlayHubView::DisplayState::Withdrawing,
           "user withdrawal during recovery cleanup must become explicit state without starting a second detach");
  QCOMPARE(viewPtr->detachCalls, 1);

  viewPtr->releaseCompleted();
  QVERIFY2(viewPtr->displayState() == OverlayHubView::DisplayState::Withdrawn && viewPtr->observedFailureCounts.empty(),
           "cleanup completion after user withdrawal must not enter Unavailable or arm a retry");
}

void testPersistentPlacementMismatchDoesNotSpin() {
  FakeHubView view(200);
  view.retryInterval = 10000;
  view.verificationResult = OverlayHubView::VerificationResult::Pending;
  view.requestEnabled();
  view.verificationResult = OverlayHubView::VerificationResult::RefreshRequired;
  view.verifyNow();
  QVERIFY2(view.attachCalls == 2 && view.displayState() == OverlayHubView::DisplayState::Unavailable,
           "the first placement mismatch must receive one in-place repair attempt");

  QEventLoop eventLoop;
  QTimer::singleShot(600, &eventLoop, &QEventLoop::quit);
  eventLoop.exec();
  QVERIFY2(view.attachCalls == 2 && view.detachCalls == 1 && view.displayState() == OverlayHubView::DisplayState::Unavailable &&
               view.observedFailureCounts == std::vector<int>({1}),
           "a persistent mismatch must leave the fast confirmation path and wait for the normal retry schedule");
}

void testFallbackRemainsStableAfterTaskbarRelease() {
  auto strategyState = std::make_shared<StrategyState>();
  OverlayHub hub;
  auto high = std::make_unique<CoordinatorBackedHubView>(strategyState);
  CoordinatorBackedHubView* const highView = high.get();
  highView->retryInterval = 10000;
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});

  strategyState->verificationResult = TaskbarLayoutStrategy::VerificationResult::Invalid;
  highView->verifyNow();
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Failing && lowView->presentationConfirmed(),
           "a synchronously hidden taskbar may release presentation ownership while native cleanup continues");
  const int prepareCalls = strategyState->prepareCalls;
  highView->advanceDetaching();
  QVERIFY2(
      lowView->presentationConfirmed() && highView->displayState() == OverlayHubView::DisplayState::Unavailable && strategyState->prepareCalls == prepareCalls,
      "presentation release completion must not immediately steal ownership back from the fallback");
}

void testExplicitReenableResumesWhenCleanupCompletes() {
  auto strategyState = std::make_shared<StrategyState>();
  OverlayHub hub;
  auto high = std::make_unique<CoordinatorBackedHubView>(strategyState);
  CoordinatorBackedHubView* const highView = high.get();
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});

  hub.setRequestedVisible(false);
  hub.setRequestedVisible(true);
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Recovering && !lowView->requested(),
           "explicit re-enable must wait on release completion without a timer or fallback flash");
  highView->advanceDetaching();
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(), "an explicit re-enable must resume immediately when its preceding cleanup completes");
}

void testBlockedReleaseHandsOwnershipToFallback() {
  auto strategyState = std::make_shared<StrategyState>();
  OverlayHub hub;
  auto high = std::make_unique<CoordinatorBackedHubView>(strategyState);
  CoordinatorBackedHubView* const highView = high.get();
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});

  strategyState->detachResults = {TaskbarLayoutStrategy::DetachResult::Failed, TaskbarLayoutStrategy::DetachResult::Detached};
  hub.setRequestedVisible(false);
  hub.setRequestedVisible(true);
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Recovering && !lowView->requested(),
           "a normal pending release may remain exclusive until the first concrete cleanup result");

  highView->advanceDetaching();
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Failing && lowView->presentationConfirmed(),
           "a failed cleanup attempt must hand visible ownership to fallback by event, not timeout");

  hub.setRequestedVisible(false);
  hub.setRequestedVisible(true);
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Failing && lowView->presentationConfirmed(),
           "re-enable after a known cleanup failure must not return to an exclusive state that has no new Blocked event");
  highView->advanceDetaching();
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Unavailable && lowView->presentationConfirmed(),
           "release completion after a blocked cleanup must preserve stable fallback ownership");
}

void testHubUsesCommittedStateSnapshot() {
  OverlayHub hub;
  auto high = std::make_unique<FakeHubView>(200);
  FakeHubView* const highView = high.get();
  highView->attachResult = OverlayHubView::AttachResult::Failed;
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});

  QVERIFY2(lowView->presentationConfirmed() && hub.presented(), "confirmed lower priority view must remain the stable fallback");
  QVERIFY2(highView->requested() && lowView->requested(), "unavailable high priority view must remain eligible for recovery");

  highView->attachResult = OverlayHubView::AttachResult::Attached;
  highView->verificationResult = OverlayHubView::VerificationResult::Pending;
  highView->retryNow();
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Attaching && !lowView->requested() && !hub.presented(),
           "an attached higher-priority view must immediately hide the fallback while awaiting confirmation");
  highView->attachResult = OverlayHubView::AttachResult::Failed;
  highView->retryNow();
  QVERIFY2(lowView->presentationConfirmed(), "fallback must return after the exclusive higher-priority attach fails");

  highView->attachResult = OverlayHubView::AttachResult::Attached;
  highView->verificationResult = OverlayHubView::VerificationResult::Confirmed;
  highView->retryNow();
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(), "recovered high priority view must replace the fallback");

  const int attachCallsBeforeRefresh = highView->attachCalls;
  highView->verificationResult = OverlayHubView::VerificationResult::RefreshRequired;
  highView->verificationAfterAttach = OverlayHubView::VerificationResult::Confirmed;
  highView->verifyNow();
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested() && highView->attachCalls == attachCallsBeforeRefresh + 1,
           "refreshable geometry must be repaired in place without activating the fallback");

  highView->verificationResult = OverlayHubView::VerificationResult::Retained;
  highView->verifyNow();
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(), "retained external state must not activate the fallback");

  highView->verificationResult = OverlayHubView::VerificationResult::Invalid;
  highView->verifyNow();
  QVERIFY2(!highView->presentationConfirmed() && lowView->presentationConfirmed(), "invalid local state must activate the fallback");

  highView->verificationResult = OverlayHubView::VerificationResult::Confirmed;
  highView->retryNow();
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(), "high priority view must recover after a real invalidation");

  const int highVerifyCalls = highView->verifyCalls;
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY2(highView->verifyCalls == highVerifyCalls, "Hub reconciliation must not re-verify an already committed view");
}

void testHigherPriorityViewAutomaticallyReplacesFallback() {
  OverlayHub hub;
  auto high = std::make_unique<FakeHubView>(200);
  FakeHubView* const highView = high.get();
  highView->attachResult = OverlayHubView::AttachResult::TemporarilyUnavailable;
  highView->retryInterval = 1;
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY2(lowView->presentationConfirmed(), "fallback must cover a temporarily unavailable taskbar view");

  highView->attachResult = OverlayHubView::AttachResult::Attached;
  highView->verificationResult = OverlayHubView::VerificationResult::Confirmed;
  QEventLoop eventLoop;
  QTimer::singleShot(50, &eventLoop, &QEventLoop::quit);
  eventLoop.exec();

  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(),
           "a recovered taskbar view must automatically replace the floating fallback without an external UI event");
}

void testAttachingViewRepairsChangedPlacement() {
  FakeHubView view(100);
  view.verificationResult = OverlayHubView::VerificationResult::Pending;
  view.requestEnabled();
  QVERIFY2(view.displayState() == OverlayHubView::DisplayState::Attaching && view.attachCalls == 1, "pending initial presentation must enter Attaching");

  view.verificationResult = OverlayHubView::VerificationResult::RefreshRequired;
  view.verificationAfterAttach = OverlayHubView::VerificationResult::Confirmed;
  view.verifyNow();
  QVERIFY2(view.presentationConfirmed() && view.attachCalls == 2,
           "placement drift during initial presentation must run one idempotent repair transaction and confirm its result");
}

void testToolbarToggleDuringTransientShellPresentation() {
  OverlayHub hub;
  auto high = std::make_unique<FakeHubView>(200);
  FakeHubView* const highView = high.get();
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY2(highView->presentationConfirmed(), "taskbar view must initially own the Hub");

  highView->verificationResult = OverlayHubView::VerificationResult::Retained;
  highView->verifyNow();
  hub.setRequestedVisible(false);
  hub.setRequestedVisible(true);
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Attaching && !lowView->requested(),
           "toolbar re-enable during transient Shell presentation must await the structurally valid taskbar instead of showing the fallback");

  highView->verificationResult = OverlayHubView::VerificationResult::Confirmed;
  highView->verifyNow();
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(),
           "restored Shell presentation must confirm the taskbar without another toolbar or Start-menu event");
}

void testInterleavedToolbarAndHostTransitionsConverge() {
  auto strategyState = std::make_shared<StrategyState>();
  OverlayHub hub;
  auto high = std::make_unique<CoordinatorBackedHubView>(strategyState);
  CoordinatorBackedHubView* const highView = high.get();
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY2(highView->presentationConfirmed(), "coordinator-backed taskbar must initially own the Hub");

  strategyState->verificationResult = TaskbarLayoutStrategy::VerificationResult::Retained;
  highView->verifyNow();
  hub.setRequestedVisible(false);
  hub.setRequestedVisible(true);
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Recovering && !lowView->requested(),
           "serialized taskbar detach must remain event-driven and exclusive");

  highView->deliverTaskbarCreated();
  hub.setRequestedVisible(false);
  hub.setRequestedVisible(true);
  strategyState->verificationResult = TaskbarLayoutStrategy::VerificationResult::Confirmed;
  highView->advanceDetaching();
  highView->advanceDetaching();

  QEventLoop eventLoop;
  QTimer::singleShot(50, &eventLoop, &QEventLoop::quit);
  eventLoop.exec();
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(),
           "interleaved toolbar toggles and host transitions must converge to the recovered higher-priority taskbar");
}

void testRefreshOwnershipControlsFallbackEligibility() {
  auto strategyState = std::make_shared<StrategyState>();
  OverlayHub hub;
  auto high = std::make_unique<CoordinatorBackedHubView>(strategyState);
  CoordinatorBackedHubView* const highView = high.get();
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(), "taskbar must own the initial presentation");

  strategyState->verificationResult = TaskbarLayoutStrategy::VerificationResult::RefreshRequired;
  strategyState->readiness = TaskbarLayoutStrategy::AttachReadiness::TemporarilyUnavailable;
  highView->verifyNow();
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(), "a refresh that explicitly retains its attachment must keep fallback ineligible");

  strategyState->readiness = TaskbarLayoutStrategy::AttachReadiness::Ready;
  strategyState->commitResult = TaskbarLayoutStrategy::AttachResult::TemporarilyUnavailable;
  highView->verifyNow();
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Unavailable && lowView->presentationConfirmed(),
           "a refresh that rolled attachment ownership back must yield to fallback instead of claiming confirmation");
}

void testStartMenuJitterThenToolbarToggleKeepsTaskbarExclusive() {
  // Reproduces the product failure mode that motivated the hub lifecycle refactor:
  // repeated Start-menu open/close leaves the taskbar attachment in a transient
  // Shell state; toggling the toolbar hub must keep exclusive recovery on the
  // coordinator-backed taskbar instead of yielding to the floating fallback.
  auto strategyState = std::make_shared<StrategyState>();
  OverlayHub hub;
  auto high = std::make_unique<CoordinatorBackedHubView>(strategyState);
  CoordinatorBackedHubView* const highView = high.get();
  highView->retryInterval = 10000;
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(), "taskbar must own the initial presentation");

  for (int cycle = 0; cycle < 3; ++cycle) {
    strategyState->verificationResult = TaskbarLayoutStrategy::VerificationResult::Retained;
    highView->verifyNow();
    QVERIFY2(highView->presentationConfirmed() && !lowView->requested(),
             "Start-menu cloak (Retained) must keep the confirmed taskbar and must not request the floating fallback");

    // Placement/visibility jitter while Start is animating: require one in-place refresh.
    // Shell becomes steady again by the time finalize commits visibility.
    strategyState->verificationResult = TaskbarLayoutStrategy::VerificationResult::RefreshRequired;
    strategyState->finalizeHook = [strategyState]() { strategyState->verificationResult = TaskbarLayoutStrategy::VerificationResult::Confirmed; };
    highView->verifyNow();
    strategyState->finalizeHook = {};
    QVERIFY2(highView->presentationConfirmed() && !lowView->requested(),
             "Start-menu layout jitter (RefreshRequired) must repair on the taskbar without falling back to the floating hub");
  }

  strategyState->verificationResult = TaskbarLayoutStrategy::VerificationResult::Retained;
  highView->verifyNow();
  hub.setRequestedVisible(false);
  hub.setRequestedVisible(true);
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Recovering && !lowView->requested() && !lowView->presentationConfirmed(),
           "toolbar toggle while Start-menu-retained Shell is still settling must stay in exclusive Recovering");

  // Another Start-menu pulse and a second toolbar toggle while native detach is pending.
  strategyState->verificationResult = TaskbarLayoutStrategy::VerificationResult::Retained;
  highView->verifyNow();
  hub.setRequestedVisible(false);
  hub.setRequestedVisible(true);
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Recovering && !lowView->requested() && !lowView->presentationConfirmed(),
           "repeated Start-menu jitter plus toolbar toggles must not hand ownership to the floating fallback mid-cleanup");

  strategyState->verificationResult = TaskbarLayoutStrategy::VerificationResult::Confirmed;
  highView->advanceDetaching();

  QEventLoop eventLoop;
  QTimer::singleShot(50, &eventLoop, &QEventLoop::quit);
  eventLoop.exec();
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested() && !lowView->presentationConfirmed(),
           "after Start-menu jitter and toolbar toggles the coordinator taskbar must reclaim the Hub without a floating interlude");
}

void testHealthDueRepairsConfirmedPresentationInPlace() {
  FakeHubView view(200);
  view.requestEnabled();
  QVERIFY2(view.presentationConfirmed() && view.attachCalls == 1, "health checks only run after a confirmed presentation");
  const int attachCalls = view.attachCalls;

  view.verificationResult = OverlayHubView::VerificationResult::RefreshRequired;
  view.verificationAfterAttach = OverlayHubView::VerificationResult::Confirmed;
  OverlayHubViewTestAccess::fireHealthDue(view);

  QVERIFY2(view.presentationConfirmed() && view.attachCalls == attachCalls + 1 && view.detachCalls == 0,
           "HealthDue RefreshRequired must rebuild the confirmed presentation in place without releasing ownership");
}

void testFailingHostInvalidatedCompletionDoesNotDeadlock() {
  // Coordinator 可将普通 cleanup 升级为 HostInvalidated，此时 Widget 只发
  // HostReleaseCompleted。Failing 必须把它当作清理完成，否则永久卡死且无 retry。
  OverlayHub hub;
  auto high = std::make_unique<FakeHubView>(200);
  FakeHubView* const highView = high.get();
  highView->retryInterval = 10000;
  highView->detachReturnsPending = true;
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY(highView->presentationConfirmed());

  highView->verificationResult = OverlayHubView::VerificationResult::Invalid;
  OverlayHubViewTestAccess::fireHealthDue(*highView);
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Failing, "identity loss must enter Failing while async cleanup is pending");
  QVERIFY2(lowView->presentationConfirmed(), "Failing is non-exclusive so fallback may present during cleanup");

  highView->hostReleaseCompleted();
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Unavailable,
           "HostReleaseCompleted during Failing must finish cleanup into Unavailable rather than deadlocking");
}

void testRecoveringSoftFailureExhaustionYieldsFallback() {
  OverlayHub hub;
  auto high = std::make_unique<FakeHubView>(200);
  FakeHubView* const highView = high.get();
  highView->retryInterval = 0;
  highView->maxRecoverAttempts = 2;
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY(highView->presentationConfirmed() && !lowView->requested());

  highView->attachResult = OverlayHubView::AttachResult::TemporarilyUnavailable;
  highView->hostReleaseStarted();
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Recovering, "host invalidation must enter exclusive Recovering");
  highView->hostReleaseCompleted();

  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Unavailable && lowView->presentationConfirmed(),
           "Recovering soft-failure exhaustion must yield Failing→Unavailable and allow floating fallback");
  QVERIFY2(highView->detachCalls >= 1, "exhaustion must release the exclusive recover attempt");
}

void testHealthDueInvalidYieldsToFallback() {
  OverlayHub hub;
  auto high = std::make_unique<FakeHubView>(200);
  FakeHubView* const highView = high.get();
  highView->retryInterval = 10000;
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY2(highView->presentationConfirmed() && !lowView->requested(), "taskbar must own the initial presentation");

  highView->verificationResult = OverlayHubView::VerificationResult::Invalid;
  OverlayHubViewTestAccess::fireHealthDue(*highView);

  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Unavailable && lowView->presentationConfirmed(),
           "HealthDue Invalid must release the taskbar and hand presentation to the floating fallback");
  QVERIFY2(highView->detachCalls == 1, "identity loss on the health path must perform exactly one release");
}

void testConfirmationTimeoutExhaustionYieldsFallback() {
  // Symmetric to start-menu jitter tests that insist on NOT falling back: when
  // confirmation expires while Shell still reports Pending/Retained, exclusive
  // Attaching must suspend and allow the lower-priority hub to present.
  OverlayHub hub;
  auto high = std::make_unique<FakeHubView>(200);
  FakeHubView* const highView = high.get();
  highView->verificationResult = OverlayHubView::VerificationResult::Pending;
  highView->retryInterval = 10000;
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});

  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Attaching && highView->presentationExclusive() && !lowView->requested() &&
               !lowView->presentationConfirmed(),
           "unconfirmed taskbar attach must keep exclusive Attaching without requesting floating yet");

  highView->verificationResult = OverlayHubView::VerificationResult::Retained;
  OverlayHubViewTestAccess::fireConfirmationDue(*highView);

  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Unavailable && lowView->presentationConfirmed(),
           "confirmation timeout while still Retained must suspend the taskbar and yield to the floating fallback");
  QVERIFY2(highView->detachCalls == 0, "confirmation exhaustion uses Suspend, not a full recovery detach");
}

void testExternalRefreshDuringAttachingRepairsWithoutFallback() {
  OverlayHub hub;
  auto high = std::make_unique<FakeHubView>(200);
  FakeHubView* const highView = high.get();
  highView->verificationResult = OverlayHubView::VerificationResult::Pending;
  highView->retryInterval = 10000;
  auto low = std::make_unique<FakeHubView>(100);
  FakeHubView* const lowView = low.get();
  hub.registerView(std::move(high));
  hub.registerView(std::move(low));
  hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
  QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Attaching && !lowView->requested(),
           "precondition: taskbar must be exclusive Attaching before ExternalRefresh");

  const int attachCalls = highView->attachCalls;
  highView->verificationResult = OverlayHubView::VerificationResult::RefreshRequired;
  highView->verificationAfterAttach = OverlayHubView::VerificationResult::Confirmed;
  OverlayHubViewTestAccess::fireExternalRefresh(*highView);

  QVERIFY2(highView->presentationConfirmed() && highView->attachCalls == attachCalls + 1 && highView->detachCalls == 0 && !lowView->requested() &&
               !lowView->presentationConfirmed(),
           "ExternalRefresh during Attaching must repair on the taskbar without flashing the floating fallback");
}

class HubLifecycleTests final : public QObject {
  Q_OBJECT

 private slots:
  void staticCompatibilityFiltering() { testStaticCompatibilityFiltering(); }
  void priorityFallbackAndTemporaryResult() { testPriorityFallbackAndTemporaryResult(); }
  void retainedAttachmentIsDistinctFromReleasedTemporaryFailure() { testRetainedAttachmentIsDistinctFromReleasedTemporaryFailure(); }
  void attachDetachLifecycle() { testAttachDetachLifecycle(); }
  void activatePreparedReportsStatusForEveryCoordinatorState() { testActivatePreparedReportsStatusForEveryCoordinatorState(); }
  void ignoredAcquireDoesNotReplacePreparedRequest() { testIgnoredAcquireDoesNotReplacePreparedRequest(); }
  void malformedAcquirePayloadConvergesToDetached() { testMalformedAcquirePayloadConvergesToDetached(); }
  void destroyedPreparedWindowReleasesTransaction() { testDestroyedPreparedWindowReleasesTransaction(); }
  void visibilityIsCommittedBeforeFinalization() { testVisibilityIsCommittedBeforeFinalization(); }
  void detachDuringAttachIsSerialized() { testDetachDuringAttachIsSerialized(); }
  void rollbackFailureEntersDetaching() { testRollbackFailureEntersDetaching(); }
  void hostInvalidationIsSerialized() { testHostInvalidationIsSerialized(); }
  void hostInvalidationDominatesUserDetach() { testHostInvalidationDominatesUserDetach(); }
  void hostInvalidationRestartsBlockedReleaseContract() { testHostInvalidationRestartsBlockedReleaseContract(); }
  void detachRetryAndNativeWindowReset() { testDetachRetryAndNativeWindowReset(); }
  void attachNativeWindowResetRecovery() { testAttachNativeWindowResetRecovery(); }
  void rapidToggleConverges() { testRapidToggleConverges(); }
  void coordinatorDestructorCompletesPendingCleanup() { testCoordinatorDestructorCompletesPendingCleanup(); }
  void environmentClassification() { testEnvironmentClassification(); }
  void scopedDpiAwarenessRestoration() { testScopedDpiAwarenessRestoration(); }

  void coordinatorTransitionTable_data() {
    QTest::addColumn<int>("currentState");
    QTest::addColumn<int>("event");
    QTest::addColumn<int>("nextState");
    QTest::addColumn<int>("action");
    for (int state = 0; state < TaskbarLayoutCoordinatorTestAccess::stateCount(); ++state) {
      for (int event = 0; event < TaskbarLayoutCoordinatorTestAccess::eventCount(); ++event) {
        const auto expected = expectedCoordinatorTransition(static_cast<TaskbarLayoutCoordinatorTestAccess::TestState>(state),
                                                            static_cast<TaskbarLayoutCoordinatorTestAccess::TestEvent>(event));
        const std::string name = std::string(TaskbarLayoutCoordinatorTestAccess::stateName(state)) + "/" + TaskbarLayoutCoordinatorTestAccess::eventName(event);
        QTest::newRow(name.c_str()) << state << event << expected.nextState << expected.action;
      }
    }
  }

  void coordinatorTransitionTable() {
    QFETCH(int, currentState);
    QFETCH(int, event);
    QFETCH(int, nextState);
    QFETCH(int, action);
    const auto actual = TaskbarLayoutCoordinatorTestAccess::reduce(currentState, event);
    QCOMPARE(actual.nextState, nextState);
    QCOMPARE(actual.action, action);
  }

  void fsmTransitionTable_data() {
    QTest::addColumn<int>("currentState");
    QTest::addColumn<int>("variant");
    QTest::addColumn<int>("nextState");
    QTest::addColumn<int>("action");
    QTest::addColumn<bool>("actionBeforeStateChange");

    static constexpr std::array stateNames{"disabled",  "withdrawn",  "unavailable", "incompatible", "probing",    "activating",
                                           "attaching", "refreshing", "withdrawing", "failing",      "recovering", "confirmed"};
    for (int stateIndex = 0; stateIndex < OverlayHubViewTestAccess::stateCount(); ++stateIndex) {
      for (int variantIndex = 0; variantIndex < OverlayHubViewTestAccess::variantCount(); ++variantIndex) {
        const auto expected = expectedTransition(static_cast<FsmState>(stateIndex), static_cast<FsmVariant>(variantIndex));
        const std::string rowName =
            std::string(stateNames.at(static_cast<std::size_t>(stateIndex))) + "/" + OverlayHubViewTestAccess::variantName(variantIndex);
        QTest::newRow(rowName.c_str()) << stateIndex << variantIndex << OverlayHubViewTestAccess::state(expected.nextState)
                                       << OverlayHubViewTestAccess::action(expected.action) << expected.actionBeforeStateChange;
      }
    }
  }

  void fsmTransitionTable() {
    QFETCH(int, currentState);
    QFETCH(int, variant);
    QFETCH(int, nextState);
    QFETCH(int, action);
    QFETCH(bool, actionBeforeStateChange);
    const auto actual = OverlayHubViewTestAccess::reduceVariant(currentState, variant);
    QCOMPARE(actual.nextState, nextState);
    QCOMPARE(actual.action, action);
    QCOMPARE(actual.actionBeforeStateChange, actionBeforeStateChange);
  }

  void fsmTotalityAndPayloadTransitions() { testOverlayViewStateMachineMatrix(); }

  void coordinatorPayloadTransitions() {
    using State = TaskbarLayoutCoordinatorTestAccess::TestState;
    using Action = TaskbarLayoutCoordinatorTestAccess::TestAction;
    const auto preserved = TaskbarLayoutCoordinatorTestAccess::reduceDetachCompleted(false);
    QCOMPARE(preserved.nextState, static_cast<int>(State::Detached));
    QCOMPARE(preserved.action, static_cast<int>(Action::CompleteDetach));
    const auto destroyed = TaskbarLayoutCoordinatorTestAccess::reduceDetachCompleted(true);
    QCOMPARE(destroyed.nextState, static_cast<int>(State::RecreatingAfterDetach));
    QCOMPARE(destroyed.action, static_cast<int>(Action::RecreateDetachedWindow));
  }

  void fsmPublishesReleaseBeforeFallbackAttach() {
    auto trace = std::make_shared<std::vector<std::string>>();
    OverlayHub hub;
    auto high = std::make_unique<FakeHubView>(200);
    FakeHubView* const highView = high.get();
    highView->attachResult = OverlayHubView::AttachResult::TemporarilyUnavailable;
    highView->trace = trace;
    highView->traceName = "high";
    auto low = std::make_unique<FakeHubView>(100);
    FakeHubView* const lowView = low.get();
    lowView->trace = trace;
    lowView->traceName = "low";
    QSignalSpy highStateChanges(highView, &OverlayHubView::displayStateChanged);
    QSignalSpy lowStateChanges(lowView, &OverlayHubView::displayStateChanged);
    hub.registerView(std::move(high));
    hub.registerView(std::move(low));
    hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});

    const auto highSuspend = std::ranges::find(*trace, "high:suspend");
    const auto lowAttach = std::ranges::find(*trace, "low:acquire");
    QVERIFY(highSuspend != trace->end());
    QVERIFY(lowAttach != trace->end());
    QVERIFY2(highSuspend < lowAttach, "the high-priority endpoint must hide before fallback attach is allowed");
    QVERIFY(highStateChanges.count() >= 2);
    QVERIFY(lowStateChanges.count() >= 2);
    QVERIFY(lowView->presentationConfirmed());
  }

  void failedProbeKeepsConfirmedFallbackStable() {
    auto trace = std::make_shared<std::vector<std::string>>();
    OverlayHub hub;
    auto high = std::make_unique<FakeHubView>(200);
    FakeHubView* const highView = high.get();
    highView->attachResult = OverlayHubView::AttachResult::TemporarilyUnavailable;
    highView->trace = trace;
    highView->traceName = "high";
    auto low = std::make_unique<FakeHubView>(100);
    FakeHubView* const lowView = low.get();
    lowView->trace = trace;
    lowView->traceName = "low";
    hub.registerView(std::move(high));
    hub.registerView(std::move(low));
    hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
    QVERIFY(lowView->presentationConfirmed());

    trace->clear();
    highView->attachResult = OverlayHubView::AttachResult::Failed;
    highView->retryNow();
    QVERIFY(lowView->presentationConfirmed());
    QVERIFY(lowView->requested());
    QVERIFY(std::ranges::find(*trace, "low:detach") == trace->end());
    QVERIFY(std::ranges::find(*trace, "low:acquire") == trace->end());
  }

  void incompatibleProbeKeepsConfirmedFallbackStableWithoutRetry() {
    auto trace = std::make_shared<std::vector<std::string>>();
    OverlayHub hub;
    auto high = std::make_unique<FakeHubView>(200);
    FakeHubView* const highView = high.get();
    highView->attachResult = OverlayHubView::AttachResult::TemporarilyUnavailable;
    highView->trace = trace;
    highView->traceName = "high";
    auto low = std::make_unique<FakeHubView>(100);
    FakeHubView* const lowView = low.get();
    lowView->trace = trace;
    lowView->traceName = "low";
    hub.registerView(std::move(high));
    hub.registerView(std::move(low));
    hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
    QVERIFY(lowView->presentationConfirmed());

    trace->clear();
    highView->observedFailureCounts.clear();
    highView->attachResult = OverlayHubView::AttachResult::Incompatible;
    highView->retryNow();
    const int attachCalls = highView->attachCalls;
    QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Incompatible, "incompatible endpoint must enter the non-retrying state");
    QVERIFY(lowView->presentationConfirmed());
    QVERIFY(lowView->requested());
    QVERIFY(std::ranges::find(*trace, "low:detach") == trace->end());
    QVERIFY2(highView->observedFailureCounts.empty(), "Incompatible must not arm the retry timer");

    highView->retryNow();
    QCOMPARE(highView->attachCalls, attachCalls);
    QVERIFY(lowView->presentationConfirmed());

    highView->hostReleaseCompleted();
    QCOMPARE(highView->attachCalls, attachCalls);
    QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Incompatible,
             "Explorer recreation must not clear a capability fixed for the process lifetime");

    hub.setRequestedVisible(false);
    hub.setRequestedVisible(true);
    QCOMPARE(highView->attachCalls, attachCalls);
    QVERIFY2(highView->displayState() == OverlayHubView::DisplayState::Incompatible && lowView->presentationConfirmed(),
             "turning the hub off and on must not re-probe a process-incompatible endpoint");
  }

  void activationWaitsForFallbackRelease() {
    auto trace = std::make_shared<std::vector<std::string>>();
    OverlayHub hub;
    auto high = std::make_unique<FakeHubView>(200);
    FakeHubView* const highView = high.get();
    highView->attachResult = OverlayHubView::AttachResult::TemporarilyUnavailable;
    highView->trace = trace;
    highView->traceName = "high";
    auto low = std::make_unique<FakeHubView>(100);
    FakeHubView* const lowView = low.get();
    lowView->trace = trace;
    lowView->traceName = "low";
    bool overlapped = false;
    highView->activationHook = [lowView, &overlapped]() { overlapped = overlapped || lowView->isVisible(); };
    hub.registerView(std::move(high));
    hub.registerView(std::move(low));
    hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
    QVERIFY(lowView->isVisible());

    trace->clear();
    highView->attachResult = OverlayHubView::AttachResult::Attached;
    highView->retryNow();
    const auto lowDetach = std::ranges::find(*trace, "low:detach");
    const auto highActivate = std::ranges::find(*trace, "high:activate");
    QVERIFY(lowDetach != trace->end());
    QVERIFY(highActivate != trace->end());
    QVERIFY2(lowDetach < highActivate, "fallback detach must be ordered before high-priority activation");
    QVERIFY2(!overlapped, "activation callback observed a visible fallback");
    QVERIFY2(highView->isVisible() && !lowView->isVisible(), "the handoff must converge to exactly one visible high-priority View");
  }

  void hubInitiatedSelectionAuthorizesActivationAfterRelease() {
    auto trace = std::make_shared<std::vector<std::string>>();
    OverlayHub hub;
    auto low = std::make_unique<FakeHubView>(100);
    FakeHubView* const lowView = low.get();
    lowView->trace = trace;
    lowView->traceName = "low";
    hub.registerView(std::move(low));
    hub.applyUsageState(uwf::ui::OverlayUsageEnabled{uwf::core::OverlayRuntime{}, uwf::core::OverlayConfig{}});
    QVERIFY(lowView->isVisible());

    trace->clear();
    auto high = std::make_unique<FakeHubView>(200);
    FakeHubView* const highView = high.get();
    highView->trace = trace;
    highView->traceName = "high";
    bool lowVisibleAtActivation = false;
    highView->activationHook = [lowView, &lowVisibleAtActivation]() { lowVisibleAtActivation = lowView->isVisible(); };
    hub.registerView(std::move(high));

    const auto lowDetach = std::ranges::find(*trace, "low:detach");
    const auto highActivate = std::ranges::find(*trace, "high:activate");
    QVERIFY(lowDetach != trace->end());
    QVERIFY(highActivate != trace->end());
    QVERIFY2(lowDetach < highActivate && !lowVisibleAtActivation && highView->isVisible() && !lowView->isVisible(),
             "Hub-driven selection must finish fallback release before authorizing prepared taskbar visibility");
  }

  void hubLifecycleDiagnostics() { testHubLifecycleDiagnostics(); }
  void coordinatorDiagnostics() { testCoordinatorDiagnostics(); }
  void failureCounterResetsOnlyAfterConfirmation() { testFailureCounterResetsOnlyAfterConfirmation(); }
  void confirmationTimeoutRepairsRefreshablePresentation() { testConfirmationTimeoutRepairsRefreshablePresentation(); }
  void releaseAnnouncementPrecedesSyncCompletion() { testReleaseAnnouncementPrecedesSyncCompletion(); }
  void disableDuringFailedCleanupConvergesToWithdrawn() { testDisableDuringFailedCleanupConvergesToWithdrawn(); }
  void persistentPlacementMismatchDoesNotSpin() { testPersistentPlacementMismatchDoesNotSpin(); }
  void fallbackRemainsStableAfterTaskbarRelease() { testFallbackRemainsStableAfterTaskbarRelease(); }
  void explicitReenableResumesWhenCleanupCompletes() { testExplicitReenableResumesWhenCleanupCompletes(); }
  void blockedReleaseHandsOwnershipToFallback() { testBlockedReleaseHandsOwnershipToFallback(); }
  void hubUsesCommittedStateSnapshot() { testHubUsesCommittedStateSnapshot(); }
  void higherPriorityViewAutomaticallyReplacesFallback() { testHigherPriorityViewAutomaticallyReplacesFallback(); }
  void attachingViewRepairsChangedPlacement() { testAttachingViewRepairsChangedPlacement(); }
  void toolbarToggleDuringTransientShellPresentation() { testToolbarToggleDuringTransientShellPresentation(); }
  void interleavedToolbarAndHostTransitionsConverge() { testInterleavedToolbarAndHostTransitionsConverge(); }
  void refreshOwnershipControlsFallbackEligibility() { testRefreshOwnershipControlsFallbackEligibility(); }
  void startMenuJitterThenToolbarToggleKeepsTaskbarExclusive() { testStartMenuJitterThenToolbarToggleKeepsTaskbarExclusive(); }
  void healthDueRepairsConfirmedPresentationInPlace() { testHealthDueRepairsConfirmedPresentationInPlace(); }
  void failingHostInvalidatedCompletionDoesNotDeadlock() { testFailingHostInvalidatedCompletionDoesNotDeadlock(); }
  void recoveringSoftFailureExhaustionYieldsFallback() { testRecoveringSoftFailureExhaustionYieldsFallback(); }
  void healthDueInvalidYieldsToFallback() { testHealthDueInvalidYieldsToFallback(); }
  void confirmationTimeoutExhaustionYieldsFallback() { testConfirmationTimeoutExhaustionYieldsFallback(); }
  void externalRefreshDuringAttachingRepairsWithoutFallback() { testExternalRefreshDuringAttachingRepairsWithoutFallback(); }
};

}  // namespace

QTEST_MAIN(HubLifecycleTests)

#include "HubLifecycleTests.moc"
