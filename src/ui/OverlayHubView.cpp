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
#include "OverlayHubView.h"

#include <QAction>
#include <QMenu>
#include <QTimer>
#include <array>

#include "../util/Log.h"
#include "I18n.h"

namespace uwf::ui {

namespace {

const char* displayStateName(const OverlayHubView::DisplayState state) {
  switch (state) {
    case OverlayHubView::DisplayState::Disabled:
      return "disabled";
    case OverlayHubView::DisplayState::Withdrawn:
      return "withdrawn";
    case OverlayHubView::DisplayState::Unavailable:
      return "unavailable";
    case OverlayHubView::DisplayState::Probing:
      return "probing";
    case OverlayHubView::DisplayState::Activating:
      return "activating";
    case OverlayHubView::DisplayState::Attaching:
      return "attaching";
    case OverlayHubView::DisplayState::Refreshing:
      return "refreshing";
    case OverlayHubView::DisplayState::Withdrawing:
      return "withdrawing";
    case OverlayHubView::DisplayState::Failing:
      return "failing";
    case OverlayHubView::DisplayState::Recovering:
      return "recovering";
    case OverlayHubView::DisplayState::Confirmed:
      return "confirmed";
  }
  return "unknown";
}

const char* attachResultName(const OverlayHubView::AttachResult result) {
  switch (result) {
    case OverlayHubView::AttachResult::Prepared:
      return "prepared";
    case OverlayHubView::AttachResult::Attached:
      return "attached";
    case OverlayHubView::AttachResult::TemporarilyUnavailable:
      return "temporarily-unavailable";
    case OverlayHubView::AttachResult::ReleasePending:
      return "release-pending";
    case OverlayHubView::AttachResult::ReleaseBlocked:
      return "release-blocked";
    case OverlayHubView::AttachResult::Failed:
      return "failed";
  }
  return "unknown";
}

const char* verificationResultName(const OverlayHubView::VerificationResult result) {
  switch (result) {
    case OverlayHubView::VerificationResult::Confirmed:
      return "confirmed";
    case OverlayHubView::VerificationResult::Pending:
      return "pending";
    case OverlayHubView::VerificationResult::Retained:
      return "retained";
    case OverlayHubView::VerificationResult::RefreshRequired:
      return "refresh-required";
    case OverlayHubView::VerificationResult::Invalid:
      return "invalid";
  }
  return "unknown";
}

}  // namespace

OverlayHubView::OverlayHubView(QWidget* parent, const Qt::WindowFlags flags)
    : QWidget(parent, flags), m_confirmationTimer(new QTimer(this)), m_retryTimer(new QTimer(this)), m_healthTimer(new QTimer(this)) {
  m_confirmationTimer->setSingleShot(true);
  m_confirmationTimer->setTimerType(Qt::CoarseTimer);
  connect(m_confirmationTimer, &QTimer::timeout, this, [this]() { postEvent(Event::plain(EventType::ConfirmationDue)); });

  m_retryTimer->setSingleShot(true);
  m_retryTimer->setTimerType(Qt::CoarseTimer);
  connect(m_retryTimer, &QTimer::timeout, this, [this]() { postEvent(Event::plain(EventType::RetryDue)); });

  m_healthTimer->setTimerType(Qt::CoarseTimer);
  connect(m_healthTimer, &QTimer::timeout, this, [this]() { postEvent(Event::plain(EventType::HealthDue)); });
}

void OverlayHubView::setPresentationRequested(const bool requested) {
  postEvent(Event::plain(requested ? EventType::RequestEnabled : EventType::RequestDisabled));
}

void OverlayHubView::authorizePresentationActivation() { postEvent(Event::plain(EventType::ActivationAuthorized)); }

OverlayHubView::AttachResult OverlayHubView::acquirePresentation() { return AttachResult::Prepared; }

OverlayHubView::AttachResult OverlayHubView::activatePresentation() {
  show();
  return AttachResult::Attached;
}

void OverlayHubView::suspendPresentation() { hide(); }

OverlayHubView::ReleaseResult OverlayHubView::detachPresentation(ReleaseReason) {
  hide();
  return ReleaseResult::Complete;
}

void OverlayHubView::addApplicationTitleToMenu(QMenu& menu) const {
  QAction* const title = menu.addAction(I18n::applicationTitle());
  QFont titleFont = title->font();
  titleFont.setBold(true);
  title->setFont(titleFont);
  title->setEnabled(false);
  menu.addSeparator();
}

void OverlayHubView::requestPresentationRefresh() { postEvent(Event::plain(EventType::ExternalRefresh)); }

void OverlayHubView::notifyPresentationChanged() { postEvent(Event::plain(EventType::PresentationChanged)); }

void OverlayHubView::notifyHostPresentationReleaseStarted() { postEvent(Event::plain(EventType::HostReleaseStarted)); }

void OverlayHubView::notifyHostPresentationReleaseCompleted() { postEvent(Event::plain(EventType::HostReleaseCompleted)); }

void OverlayHubView::notifyPresentationReleaseBlocked() { postEvent(Event::plain(EventType::ReleaseBlocked)); }

void OverlayHubView::notifyPresentationReleaseCompleted() { postEvent(Event::plain(EventType::ReleaseCompleted)); }

OverlayHubView::Transition OverlayHubView::reduce(const DisplayState state, const Event& event) {
  constexpr std::size_t stateCount = static_cast<std::size_t>(DisplayState::Confirmed) + 1;
  constexpr std::size_t eventCount = static_cast<std::size_t>(EventType::HostReleaseCompleted) + 1;
  using TransitionTable = std::array<std::array<TransitionHandler, eventCount>, stateCount>;

  static const TransitionTable table = [] {
    TransitionTable result{};
    for (auto& row : result) row.fill(&OverlayHubView::ignoreEvent);
    const auto set = [&result](const DisplayState state, const EventType event, const TransitionHandler handler) {
      result[static_cast<std::size_t>(state)][static_cast<std::size_t>(event)] = handler;
    };
    const auto setPresenting = [&set](const EventType event, const TransitionHandler handler) {
      for (const DisplayState current :
           {DisplayState::Unavailable, DisplayState::Probing, DisplayState::Activating, DisplayState::Attaching, DisplayState::Refreshing,
            DisplayState::Withdrawing, DisplayState::Failing, DisplayState::Recovering, DisplayState::Confirmed})
        set(current, event, handler);
    };

    set(DisplayState::Disabled, EventType::RequestEnabled, &OverlayHubView::probePresentation);
    set(DisplayState::Withdrawn, EventType::RequestEnabled, &OverlayHubView::enableFromWithdrawn);
    set(DisplayState::Withdrawing, EventType::RequestEnabled, &OverlayHubView::enableDuringWithdraw);
    // Failing 期间忽略 RequestEnabled：Hub reconcile 仍会请求展示，但不能拉回独占。

    setPresenting(EventType::RequestDisabled, &OverlayHubView::disablePresentation);
    set(DisplayState::Withdrawn, EventType::RequestDisabled, &OverlayHubView::disablePresentation);
    set(DisplayState::Withdrawing, EventType::RequestDisabled, &OverlayHubView::disablePresentation);
    set(DisplayState::Failing, EventType::RequestDisabled, &OverlayHubView::disablePresentation);

    set(DisplayState::Unavailable, EventType::ExternalRefresh, &OverlayHubView::probePresentation);
    set(DisplayState::Unavailable, EventType::RetryDue, &OverlayHubView::probePresentation);
    set(DisplayState::Recovering, EventType::RetryDue, &OverlayHubView::recoverHost);
    set(DisplayState::Attaching, EventType::ExternalRefresh, &OverlayHubView::refreshPresentation);
    set(DisplayState::Confirmed, EventType::ExternalRefresh, &OverlayHubView::refreshPresentation);
    set(DisplayState::Attaching, EventType::ConfirmationDue, &OverlayHubView::verifyConfirmation);
    set(DisplayState::Confirmed, EventType::HealthDue, &OverlayHubView::verifyHealth);
    set(DisplayState::Activating, EventType::ActivationAuthorized, &OverlayHubView::activatePrepared);
    for (const DisplayState current : {DisplayState::Attaching, DisplayState::Refreshing, DisplayState::Confirmed}) {
      set(current, EventType::PresentationChanged, &OverlayHubView::verifyChanged);
      set(current, EventType::VerificationObserved, &OverlayHubView::handleVerificationObserved);
    }
    set(DisplayState::Probing, EventType::AttachFinished, &OverlayHubView::handleAttachFinished);
    set(DisplayState::Refreshing, EventType::AttachFinished, &OverlayHubView::handleAttachFinished);
    set(DisplayState::Recovering, EventType::AttachFinished, &OverlayHubView::handleAttachFinished);
    set(DisplayState::Activating, EventType::ActivationFinished, &OverlayHubView::handleActivationFinished);
    set(DisplayState::Refreshing, EventType::ActivationFinished, &OverlayHubView::handleActivationFinished);

    setPresenting(EventType::ReleaseStarted, &OverlayHubView::handleReleaseStarted);
    setPresenting(EventType::ReleaseBlocked, &OverlayHubView::enterFailing);
    set(DisplayState::Recovering, EventType::ReleaseCompleted, &OverlayHubView::handleReleaseCompleted);
    set(DisplayState::Withdrawing, EventType::ReleaseCompleted, &OverlayHubView::handleReleaseCompleted);
    set(DisplayState::Failing, EventType::ReleaseCompleted, &OverlayHubView::handleReleaseCompleted);

    // HostReleaseStarted 不进入 Withdrawing/Failing：清理中途不能被拉回独占
    // Recovering。但 HostReleaseCompleted 必须收敛清理——Coordinator 把普通
    // detach 升级为 HostInvalidated 后只发布 Host 完成事件。
    for (const DisplayState current : {DisplayState::Unavailable, DisplayState::Probing, DisplayState::Activating, DisplayState::Attaching,
                                       DisplayState::Refreshing, DisplayState::Recovering, DisplayState::Confirmed}) {
      set(current, EventType::HostReleaseStarted, &OverlayHubView::enterRecovering);
      set(current, EventType::HostReleaseCompleted, &OverlayHubView::probePresentation);
    }
    set(DisplayState::Recovering, EventType::HostReleaseCompleted, &OverlayHubView::recoverHost);
    set(DisplayState::Withdrawing, EventType::HostReleaseCompleted, &OverlayHubView::handleReleaseCompleted);
    set(DisplayState::Failing, EventType::HostReleaseCompleted, &OverlayHubView::handleReleaseCompleted);
    return result;
  }();

  return table[static_cast<std::size_t>(state)][static_cast<std::size_t>(event.type)](state, event);
}

OverlayHubView::Transition OverlayHubView::ignoreEvent(const DisplayState state, const Event&) { return {state}; }

OverlayHubView::Transition OverlayHubView::probePresentation(DisplayState, const Event&) { return {DisplayState::Probing, Action::AttachAcquire}; }

OverlayHubView::Transition OverlayHubView::refreshPresentation(DisplayState, const Event&) { return {DisplayState::Refreshing, Action::AttachRefresh}; }

OverlayHubView::Transition OverlayHubView::disablePresentation(const DisplayState state, const Event&) {
  if (state == DisplayState::Disabled || state == DisplayState::Withdrawn || state == DisplayState::Withdrawing) return {state};
  if (state == DisplayState::Failing) return {state};
  return {DisplayState::Withdrawing, Action::ReleaseRequest, true};
}

OverlayHubView::Transition OverlayHubView::enableFromWithdrawn(DisplayState, const Event&) { return {DisplayState::Recovering, Action::AttachAcquire}; }

OverlayHubView::Transition OverlayHubView::enableDuringWithdraw(DisplayState, const Event&) { return {DisplayState::Recovering}; }

OverlayHubView::Transition OverlayHubView::verifyConfirmation(const DisplayState state, const Event&) { return {state, Action::VerifyConfirmation}; }

OverlayHubView::Transition OverlayHubView::verifyHealth(const DisplayState state, const Event&) { return {state, Action::VerifyHealth}; }

OverlayHubView::Transition OverlayHubView::verifyChanged(const DisplayState state, const Event&) { return {state, Action::VerifyChanged}; }

OverlayHubView::Transition OverlayHubView::activatePrepared(const DisplayState state, const Event&) { return {state, Action::Activate}; }

OverlayHubView::Transition OverlayHubView::handleAttachFinished(const DisplayState state, const Event& event) {
  switch (event.attachResult) {
    case AttachResult::Prepared:
      return state == DisplayState::Refreshing ? Transition{state, Action::Activate} : Transition{DisplayState::Activating};
    case AttachResult::Attached:
      return {DisplayState::Failing, Action::ReleaseRecovery, true};
    case AttachResult::TemporarilyUnavailable:
      if (state == DisplayState::Recovering) return {state, Action::ScheduleRecoverRetry};
      return state == DisplayState::Refreshing ? Transition{DisplayState::Confirmed} : Transition{DisplayState::Unavailable, Action::Suspend, true};
    case AttachResult::ReleasePending:
      return {DisplayState::Recovering};
    case AttachResult::ReleaseBlocked:
      return {DisplayState::Failing};
    case AttachResult::Failed:
      if (state == DisplayState::Recovering) return {state, Action::ScheduleRecoverRetry};
      return {DisplayState::Failing, Action::ReleaseRecovery, true};
  }
  return {state};
}

OverlayHubView::Transition OverlayHubView::handleActivationFinished(const DisplayState state, const Event& event) {
  switch (event.attachResult) {
    case AttachResult::Attached:
      return state == DisplayState::Refreshing ? Transition{state, Action::VerifyAttach} : Transition{DisplayState::Attaching, Action::VerifyAttach};
    case AttachResult::TemporarilyUnavailable:
      return state == DisplayState::Refreshing ? Transition{DisplayState::Confirmed} : Transition{DisplayState::Unavailable, Action::Suspend, true};
    case AttachResult::ReleasePending:
      return {DisplayState::Recovering};
    case AttachResult::ReleaseBlocked:
      return {DisplayState::Failing};
    case AttachResult::Prepared:
    case AttachResult::Failed:
      return {DisplayState::Failing, Action::ReleaseRecovery, true};
  }
  return {state};
}

OverlayHubView::Transition OverlayHubView::handleVerificationObserved(const DisplayState state, const Event& event) {
  const VerificationResult result = event.verificationResult;
  if (state == DisplayState::Attaching) {
    if (result == VerificationResult::Confirmed) return {DisplayState::Confirmed};
    if (result == VerificationResult::Invalid) return {DisplayState::Failing, Action::ReleaseRecovery, true};
    if (result == VerificationResult::RefreshRequired) return {DisplayState::Refreshing, Action::AttachRefresh};
    if (event.verificationSource == VerificationSource::Confirmation && (result == VerificationResult::Pending || result == VerificationResult::Retained))
      return {DisplayState::Unavailable, Action::Suspend, true};
    return {state};
  }
  if (state == DisplayState::Refreshing) {
    if (result == VerificationResult::Confirmed || result == VerificationResult::Retained) return {DisplayState::Confirmed};
    if (result == VerificationResult::Pending) return {DisplayState::Attaching};
    return {DisplayState::Failing, Action::ReleaseRecovery, true};
  }
  if (result == VerificationResult::Confirmed || result == VerificationResult::Retained) return {state};
  if (result == VerificationResult::RefreshRequired) return {DisplayState::Refreshing, Action::AttachRefresh};
  return {DisplayState::Failing, Action::ReleaseRecovery, true};
}

OverlayHubView::Transition OverlayHubView::handleReleaseStarted(const DisplayState state, const Event& event) {
  // 状态已由 RequestDisabled→Withdrawing 或 Failed→Failing 先行进入；此处只在
  // 旧路径漏进时兜底。Pending 保持当前清理态，Complete 视为失败让出。
  if (event.releaseResult == ReleaseResult::Complete) return {DisplayState::Unavailable};
  if (state == DisplayState::Withdrawing || state == DisplayState::Failing) return {state};
  return {DisplayState::Failing};
}

OverlayHubView::Transition OverlayHubView::enterFailing(DisplayState, const Event&) { return {DisplayState::Failing}; }

OverlayHubView::Transition OverlayHubView::handleReleaseCompleted(const DisplayState state, const Event&) {
  if (state == DisplayState::Recovering) return {state, Action::AttachAcquire};
  if (state == DisplayState::Withdrawing) return {DisplayState::Withdrawn};
  if (state == DisplayState::Failing) return {DisplayState::Unavailable};
  return {state};
}

OverlayHubView::Transition OverlayHubView::enterRecovering(DisplayState, const Event&) { return {DisplayState::Recovering}; }

OverlayHubView::Transition OverlayHubView::recoverHost(const DisplayState state, const Event&) { return {state, Action::AttachAcquire}; }

void OverlayHubView::postEvent(Event event) {
  m_events.push_back(event);
  if (!m_processingEvents) processEvents();
}

void OverlayHubView::processEvents() {
  m_processingEvents = true;
  while (!m_events.empty()) {
    const Event event = m_events.front();
    m_events.pop_front();
    if (event.type == EventType::RequestEnabled) m_requested = true;
    if (event.type == EventType::RequestDisabled) m_requested = false;
    logOutcome(event);
    const Transition transition = reduce(m_displayState, event);
    if (transition.actionBeforeStateChange) execute(transition.action);
    if (transition.nextState != m_displayState) enterState(transition.nextState);
    if (!transition.actionBeforeStateChange) execute(transition.action);
  }
  m_processingEvents = false;
}

void OverlayHubView::execute(const Action action) {
  switch (action) {
    case Action::None:
      return;
    case Action::AttachAcquire:
    case Action::AttachRefresh:
      postEvent(Event::attachFinished(acquirePresentation()));
      return;
    case Action::Activate:
      postEvent(Event::activationFinished(activatePresentation()));
      return;
    case Action::VerifyAttach:
      observeVerification(VerificationSource::Attach);
      return;
    case Action::VerifyConfirmation:
      observeVerification(VerificationSource::Confirmation, true);
      return;
    case Action::VerifyHealth:
      observeVerification(VerificationSource::Health);
      return;
    case Action::VerifyChanged:
      observeVerification(VerificationSource::PresentationChanged);
      return;
    case Action::Suspend:
      suspendPresentation();
      return;
    case Action::ReleaseRecovery:
      beginPresentationRelease(ReleaseReason::Recovery);
      return;
    case Action::ReleaseRequest:
      beginPresentationRelease(ReleaseReason::RequestRevoked);
      return;
    case Action::ScheduleRecoverRetry: {
      ++m_consecutiveFailures;
      if (m_consecutiveFailures > maxExclusiveRecoverAttempts()) {
        UWF_LOG_D("hub") << "recover exhausted: view=" << metaObject()->className() << " failure=" << m_consecutiveFailures << " action=yield-fallback";
        postEvent(Event::plain(EventType::ReleaseBlocked));
        beginPresentationRelease(ReleaseReason::Recovery);
        return;
      }
      const int interval = retryIntervalMs(m_consecutiveFailures);
      if (interval > 0) {
        UWF_LOG_D("hub") << "recover retry scheduled: view=" << metaObject()->className() << " failure=" << m_consecutiveFailures << " delayMs=" << interval;
        m_retryTimer->setInterval(interval);
        m_retryTimer->start();
      } else {
        postEvent(Event::plain(EventType::RetryDue));
      }
      return;
    }
  }
}

void OverlayHubView::beginPresentationRelease(const ReleaseReason reason) {
  postEvent(Event::releaseStarted(ReleaseResult::Pending));
  if (detachPresentation(reason) == ReleaseResult::Complete) postEvent(Event::plain(EventType::ReleaseCompleted));
}

void OverlayHubView::observeVerification(const VerificationSource source, const bool repaintFirst) {
  if (repaintFirst) repaint();
  postEvent(Event::verificationObserved(verifyPresentation(), source));
}

void OverlayHubView::enterState(const DisplayState state) {
  const DisplayState previous = m_displayState;
  m_confirmationTimer->stop();
  m_retryTimer->stop();
  m_healthTimer->stop();
  m_displayState = state;

  if (state == DisplayState::Disabled || state == DisplayState::Withdrawn || state == DisplayState::Confirmed || state == DisplayState::Recovering)
    m_consecutiveFailures = 0;
  if (state == DisplayState::Attaching) {
    m_confirmationTimer->setInterval(confirmationTimeoutMs());
    m_confirmationTimer->start();
  } else if (state == DisplayState::Confirmed) {
    const int interval = healthCheckIntervalMs();
    if (interval > 0) {
      m_healthTimer->setInterval(interval);
      m_healthTimer->start();
    }
  } else if (state == DisplayState::Unavailable) {
    ++m_consecutiveFailures;
    const int interval = retryIntervalMs(m_consecutiveFailures);
    if (interval > 0) {
      UWF_LOG_D("hub") << "retry scheduled: view=" << metaObject()->className() << " failure=" << m_consecutiveFailures << " delayMs=" << interval;
      m_retryTimer->setInterval(interval);
      m_retryTimer->start();
    }
  }

  UWF_LOG_D("hub") << "display state: view=" << metaObject()->className() << " from=" << displayStateName(previous) << " to=" << displayStateName(state)
                   << " requested=" << presentationRequested();
  emit displayStateChanged();
}

void OverlayHubView::logOutcome(const Event& event) const {
  if ((event.type == EventType::AttachFinished && event.attachResult != AttachResult::Prepared) ||
      (event.type == EventType::ActivationFinished && event.attachResult != AttachResult::Attached))
    UWF_LOG_D("hub") << "attach outcome: view=" << metaObject()->className() << " result=" << attachResultName(event.attachResult);
  if (event.type == EventType::VerificationObserved &&
      (event.verificationResult == VerificationResult::RefreshRequired || event.verificationResult == VerificationResult::Invalid))
    UWF_LOG_D("hub") << "verification outcome: view=" << metaObject()->className() << " state=" << displayStateName(m_displayState)
                     << " result=" << verificationResultName(event.verificationResult);
}

}  // namespace uwf::ui
