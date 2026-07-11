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
#include "TaskbarLayoutCoordinator.h"

#include <windows.h>

#include <QCoreApplication>
#include <QTimer>
#include <QWindow>
#include <algorithm>
#include <array>
#include <utility>

#include "../util/Log.h"

namespace uwf::ui {

namespace {

constexpr int kFastRetryAttempts = 3;
constexpr int kFastRetryIntervalMs = 1000;
constexpr int kSlowRetryIntervalMs = 5000;

}  // namespace

TaskbarLayoutCoordinator::TaskbarLayoutCoordinator(std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies, DetachObserver detachObserver)
    : m_strategies(std::move(strategies)),
      m_detachObserver(std::move(detachObserver)),
      m_asyncTimer(new QTimer(this)),
      m_taskbarCreatedMessage(RegisterWindowMessageW(L"TaskbarCreated")) {
  std::erase_if(m_strategies, [](const std::unique_ptr<TaskbarLayoutStrategy>& strategy) { return !strategy || !strategy->isCompatible(); });
  std::stable_sort(
      m_strategies.begin(), m_strategies.end(),
      [](const std::unique_ptr<TaskbarLayoutStrategy>& lhs, const std::unique_ptr<TaskbarLayoutStrategy>& rhs) { return lhs->priority() > rhs->priority(); });
  m_asyncTimer->setSingleShot(true);
  m_asyncTimer->setTimerType(Qt::CoarseTimer);
  connect(m_asyncTimer, &QTimer::timeout, this, [this]() { postEvent(Event{EventType::DetachDue}); });
  if (QCoreApplication::instance()) QCoreApplication::instance()->installNativeEventFilter(this);
}

TaskbarLayoutCoordinator::~TaskbarLayoutCoordinator() {
  if (QCoreApplication::instance()) QCoreApplication::instance()->removeNativeEventFilter(this);
  m_asyncTimer->stop();
  if (m_request && m_request->hideWindow) m_request->hideWindow();
  if (m_transaction && m_transaction->rollback() != TaskbarLayoutStrategy::DetachResult::Failed) m_attachment.reset();
  if (m_detaching) {
    if (m_detaching->attachment && m_detaching->attachment->hideWindow) m_detaching->attachment->hideWindow();
    for (TaskbarLayoutStrategy& strategy : m_detaching->strategies) {
      if (m_detaching->cause == DetachmentCause::HostInvalidated)
        (void)strategy.invalidate();
      else
        (void)strategy.detach();
    }
  } else if (m_attachment) {
    (void)m_attachment->strategy.get().detach();
  }
}

TaskbarLayoutCoordinator::AttachResult TaskbarLayoutCoordinator::prepareAttach(QWindow* const window, const QSize& logicalSize,
                                                                               const VisibilityCommit& makeVisible, const VisibilityRollback& makeInvisible) {
  if (!window) return AttachResult::Failed;
  m_request = AttachRequest{window, logicalSize, makeVisible, makeInvisible};
  m_lastAttachResult = AttachResult::Failed;
  postEvent(Event{EventType::AcquireRequested});
  return m_lastAttachResult;
}

TaskbarLayoutCoordinator::AttachResult TaskbarLayoutCoordinator::activatePrepared() {
  m_lastAttachResult = AttachResult::Failed;
  postEvent(Event{EventType::ActivateRequested});
  return m_lastAttachResult;
}

TaskbarLayoutStrategy::VerificationResult TaskbarLayoutCoordinator::verify(const QWindow* const window, const WId currentWindowId) const {
  if (m_state != State::Attached || !m_attachment || m_attachment->window != window) return TaskbarLayoutStrategy::VerificationResult::Invalid;
  return m_attachment->strategy.get().verify(window, currentWindowId);
}

TaskbarLayoutCoordinator::DetachStatus TaskbarLayoutCoordinator::detach() {
  if (m_state == State::Detached) return DetachStatus::AlreadyDetached;
  postEvent(Event{EventType::DetachRequested});
  return DetachStatus::Pending;
}

TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::reduce(const State state, const Event& event) {
  constexpr std::size_t stateCount = static_cast<std::size_t>(State::Detaching) + 1;
  constexpr std::size_t eventCount = static_cast<std::size_t>(EventType::DetachCompleted) + 1;
  using Table = std::array<std::array<TransitionHandler, eventCount>, stateCount>;
  static const Table table = [] {
    Table result{};
    for (auto& row : result) row.fill(&TaskbarLayoutCoordinator::ignoreEvent);
    const auto set = [&result](const State state, const EventType event, const TransitionHandler handler) {
      result[static_cast<std::size_t>(state)][static_cast<std::size_t>(event)] = handler;
    };

    set(State::Detached, EventType::AcquireRequested, &TaskbarLayoutCoordinator::beginPrepare);
    set(State::Attached, EventType::AcquireRequested, &TaskbarLayoutCoordinator::beginPrepare);
    set(State::Prepared, EventType::ActivateRequested, &TaskbarLayoutCoordinator::beginFinalize);
    set(State::Attached, EventType::ActivateRequested, &TaskbarLayoutCoordinator::attached);
    set(State::Detached, EventType::ActivateRequested, &TaskbarLayoutCoordinator::failed);
    set(State::Detaching, EventType::ActivateRequested, &TaskbarLayoutCoordinator::reportReleaseStatus);

    set(State::Preparing, EventType::OperationPrepared, &TaskbarLayoutCoordinator::prepared);
    set(State::Preparing, EventType::OperationRetained, &TaskbarLayoutCoordinator::retained);
    set(State::Preparing, EventType::OperationTemporary, &TaskbarLayoutCoordinator::temporary);
    set(State::Preparing, EventType::OperationFailed, &TaskbarLayoutCoordinator::failed);
    set(State::Preparing, EventType::OperationDestroyed, &TaskbarLayoutCoordinator::destroyed);
    set(State::Preparing, EventType::OperationNeedsDetach, &TaskbarLayoutCoordinator::needsDetach);
    set(State::Finalizing, EventType::OperationAttached, &TaskbarLayoutCoordinator::attached);
    set(State::Finalizing, EventType::OperationTemporary, &TaskbarLayoutCoordinator::temporary);
    set(State::Finalizing, EventType::OperationFailed, &TaskbarLayoutCoordinator::failed);
    set(State::Finalizing, EventType::OperationDestroyed, &TaskbarLayoutCoordinator::destroyed);
    set(State::Finalizing, EventType::OperationNeedsDetach, &TaskbarLayoutCoordinator::needsDetach);

    for (const State active : {State::Preparing, State::Prepared, State::Finalizing, State::Attached}) {
      set(active, EventType::DetachRequested, &TaskbarLayoutCoordinator::beginUserDetach);
      set(active, EventType::HostInvalidated, &TaskbarLayoutCoordinator::beginHostDetach);
    }
    set(State::Detached, EventType::HostInvalidated, &TaskbarLayoutCoordinator::notifyDetachedHostRefresh);
    set(State::Detaching, EventType::HostInvalidated, &TaskbarLayoutCoordinator::escalateHostDetach);
    set(State::Detaching, EventType::AcquireRequested, &TaskbarLayoutCoordinator::reportReleaseStatus);
    set(State::Detaching, EventType::DetachDue, &TaskbarLayoutCoordinator::advanceDetach);
    set(State::Detaching, EventType::DetachBlocked, &TaskbarLayoutCoordinator::repeatDetach);
    set(State::Detaching, EventType::DetachCompleted, &TaskbarLayoutCoordinator::completeDetach);
    return result;
  }();
  return table[static_cast<std::size_t>(state)][static_cast<std::size_t>(event.type)](state, event);
}

TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::ignoreEvent(const State state, const Event&) { return {state}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::beginPrepare(State, const Event&) { return {State::Preparing, Action::Prepare}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::beginFinalize(State, const Event&) { return {State::Finalizing, Action::Finalize}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::prepared(State, const Event&) { return {State::Prepared, Action::CompletePrepared}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::attached(State, const Event&) { return {State::Attached, Action::CompleteAttached}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::retained(State, const Event&) { return {State::Attached, Action::CompleteRetained}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::temporary(State, const Event&) { return {State::Detached, Action::CompleteTemporary}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::failed(State, const Event&) { return {State::Detached, Action::CompleteFailed}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::destroyed(State, const Event&) { return {State::Detached, Action::CompleteDestroyed}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::needsDetach(State, const Event&) { return {State::Detaching, Action::BeginUserDetach}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::beginUserDetach(State, const Event&) { return {State::Detaching, Action::BeginUserDetach}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::beginHostDetach(State, const Event&) { return {State::Detaching, Action::BeginHostDetach}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::escalateHostDetach(const State state, const Event&) {
  return {state, Action::EscalateHostDetach};
}
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::advanceDetach(const State state, const Event&) { return {state, Action::AdvanceDetach}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::repeatDetach(const State state, const Event&) { return {state, Action::ScheduleDetach}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::completeDetach(State, const Event&) { return {State::Detached, Action::CompleteDetach}; }
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::notifyDetachedHostRefresh(const State state, const Event&) {
  return {state, Action::NotifyDetachedHostRefresh};
}
TaskbarLayoutCoordinator::Transition TaskbarLayoutCoordinator::reportReleaseStatus(const State state, const Event&) {
  return {state, Action::CompleteReleaseStatus};
}

void TaskbarLayoutCoordinator::postEvent(const Event event) {
  m_events.push_back(event);
  if (!m_processingEvents) processEvents();
}

void TaskbarLayoutCoordinator::processEvents() {
  m_processingEvents = true;
  while (!m_events.empty()) {
    const Event event = m_events.front();
    m_events.pop_front();
    const Transition transition = reduce(m_state, event);
    m_state = transition.nextState;
    execute(transition.action);
  }
  m_processingEvents = false;
  flushNotifications();
}

void TaskbarLayoutCoordinator::execute(const Action action) {
  switch (action) {
    case Action::None:
      return;
    case Action::Prepare:
      runPrepare();
      return;
    case Action::Finalize:
      runFinalize();
      return;
    case Action::CompletePrepared:
      finishOperation(AttachResult::Prepared);
      return;
    case Action::CompleteAttached:
      m_transaction.reset();
      m_preparedStrategy = nullptr;
      finishOperation(AttachResult::Attached);
      return;
    case Action::CompleteRetained:
      finishOperation(AttachResult::TemporarilyUnavailable);
      return;
    case Action::CompleteTemporary:
      finishOperation(AttachResult::TemporarilyUnavailable);
      return;
    case Action::CompleteFailed:
      finishOperation(AttachResult::Failed);
      return;
    case Action::CompleteDestroyed:
      m_attachment.reset();
      finishOperation(AttachResult::NativeWindowDestroyed);
      return;
    case Action::CompleteReleaseStatus:
      finishOperation(m_detaching && m_detaching->attempts > 0 ? AttachResult::ReleaseBlocked : AttachResult::ReleasePending);
      return;
    case Action::BeginUserDetach:
      beginDetaching(DetachmentCause::UserRequested);
      return;
    case Action::BeginHostDetach:
      beginDetaching(DetachmentCause::HostInvalidated);
      return;
    case Action::EscalateHostDetach:
      if (m_detaching && m_detaching->cause != DetachmentCause::HostInvalidated) {
        m_detaching->cause = DetachmentCause::HostInvalidated;
        // 新宿主失效是一轮新的清理语义。此前用户释放的失败次数不能吞掉
        // HostInvalidated 的首次 Blocked 边沿，否则 View 会停在排他的
        // Recovering，fallback 永远得不到展示资格。
        m_detaching->attempts = 0;
        for (const auto& strategy : m_strategies) {
          const bool present = std::ranges::any_of(m_detaching->strategies, [&strategy](const auto& candidate) { return &candidate.get() == strategy.get(); });
          if (!present) m_detaching->strategies.emplace_back(*strategy);
        }
        publish(DetachEvent{DetachPhase::Started, DetachReason::HostInvalidated, false});
        m_asyncTimer->start(0);
      }
      return;
    case Action::ScheduleDetach:
      m_asyncTimer->start(0);
      return;
    case Action::AdvanceDetach:
      advanceDetaching();
      return;
    case Action::CompleteDetach: {
      const DetachingContext completed = std::move(*m_detaching);
      m_detaching.reset();
      m_attachment.reset();
      m_transaction.reset();
      m_preparedStrategy = nullptr;
      publish(DetachEvent{DetachPhase::Completed,
                          completed.cause == DetachmentCause::HostInvalidated ? DetachReason::HostInvalidated : DetachReason::PresentationReleased,
                          completed.disposition == NativeWindowDisposition::Destroyed});
      return;
    }
    case Action::NotifyDetachedHostRefresh:
      scheduleDetachedHostRefresh();
      return;
  }
}

void TaskbarLayoutCoordinator::runPrepare() {
  if (!m_request || !m_request->window) {
    postEvent(Event{EventType::OperationFailed});
    return;
  }

  bool temporary = false;
  for (const auto& strategyOwner : m_strategies) {
    TaskbarLayoutStrategy& strategy = *strategyOwner;
    auto transaction = strategy.prepareAttach(m_request->window, m_request->logicalSize);
    if (!transaction) continue;
    if (transaction->readiness() == TaskbarLayoutStrategy::AttachReadiness::TemporarilyUnavailable) {
      if (m_attachment && m_attachment->uses(strategy) && m_attachment->window == m_request->window) {
        postEvent(Event{EventType::OperationRetained});
        return;
      }
      temporary = true;
      continue;
    }
    if (transaction->readiness() != TaskbarLayoutStrategy::AttachReadiness::Ready) continue;

    if (m_attachment && (!m_attachment->uses(strategy) || m_attachment->window != m_request->window)) {
      if (m_attachment->hideWindow) m_attachment->hideWindow();
      const auto detachResult = m_attachment->strategy.get().detach();
      if (detachResult == TaskbarLayoutStrategy::DetachResult::Failed) {
        m_preparedStrategy = &m_attachment->strategy.get();
        postEvent(Event{EventType::OperationNeedsDetach});
        return;
      }
      m_attachment.reset();
      if (detachResult == TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed) {
        postEvent(Event{EventType::OperationDestroyed});
        return;
      }
    }

    const auto commitResult = transaction->commit();
    if (commitResult == TaskbarLayoutStrategy::AttachResult::Attached) {
      m_preparedStrategy = &strategy;
      m_transaction = std::move(transaction);
      m_attachment = ActiveAttachment{strategy, m_request->window, m_request->hideWindow};
      postEvent(Event{EventType::OperationPrepared});
      return;
    }

    const auto rollbackResult = transaction->rollback();
    if (rollbackResult == TaskbarLayoutStrategy::DetachResult::Failed) {
      m_preparedStrategy = &strategy;
      m_attachment = ActiveAttachment{strategy, m_request->window, m_request->hideWindow};
      postEvent(Event{EventType::OperationNeedsDetach});
      return;
    }
    if (m_attachment && m_attachment->uses(strategy)) m_attachment.reset();
    if (rollbackResult == TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed) {
      postEvent(Event{EventType::OperationDestroyed});
      return;
    }
    temporary = temporary || commitResult == TaskbarLayoutStrategy::AttachResult::TemporarilyUnavailable;
  }

  if (m_attachment) {
    if (m_attachment->hideWindow) m_attachment->hideWindow();
    const auto detachResult = m_attachment->strategy.get().detach();
    if (detachResult == TaskbarLayoutStrategy::DetachResult::Failed) {
      postEvent(Event{EventType::OperationNeedsDetach});
      return;
    }
    m_attachment.reset();
    if (detachResult == TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed) {
      postEvent(Event{EventType::OperationDestroyed});
      return;
    }
  }
  postEvent(Event{temporary ? EventType::OperationTemporary : EventType::OperationFailed});
}

void TaskbarLayoutCoordinator::runFinalize() {
  if (!m_request || !m_request->window || !m_transaction || !m_attachment) {
    postEvent(Event{EventType::OperationFailed});
    return;
  }
  if (m_request->showWindow) m_request->showWindow();
  const auto result = m_transaction->finalize();
  if (result == TaskbarLayoutStrategy::AttachResult::Attached) {
    postEvent(Event{EventType::OperationAttached});
    return;
  }

  if (m_request->hideWindow) m_request->hideWindow();
  const auto rollbackResult = m_transaction->rollback();
  m_transaction.reset();
  if (rollbackResult == TaskbarLayoutStrategy::DetachResult::Failed) {
    postEvent(Event{EventType::OperationNeedsDetach});
    return;
  }
  m_attachment.reset();
  m_preparedStrategy = nullptr;
  if (rollbackResult == TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed) {
    postEvent(Event{EventType::OperationDestroyed});
    return;
  }
  postEvent(Event{result == TaskbarLayoutStrategy::AttachResult::TemporarilyUnavailable ? EventType::OperationTemporary : EventType::OperationFailed});
}

void TaskbarLayoutCoordinator::beginDetaching(const DetachmentCause cause) {
  m_lastAttachResult = AttachResult::ReleasePending;
  DetachingContext context;
  context.cause = cause;
  context.attachment = std::move(m_attachment);
  if (context.attachment && context.attachment->hideWindow) context.attachment->hideWindow();

  bool preparedRollbackFailed = false;
  if (m_transaction) {
    const auto rollback = m_transaction->rollback();
    m_transaction.reset();
    if (rollback == TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed) {
      context.disposition = NativeWindowDisposition::Destroyed;
      context.attachment.reset();
    } else if (rollback == TaskbarLayoutStrategy::DetachResult::Detached) {
      context.attachment.reset();
    } else {
      preparedRollbackFailed = true;
    }
  }

  if (cause == DetachmentCause::HostInvalidated) {
    context.strategies.reserve(m_strategies.size());
    for (const auto& strategy : m_strategies) context.strategies.emplace_back(*strategy);
  } else if (context.attachment) {
    context.strategies.emplace_back(context.attachment->strategy);
  } else if (preparedRollbackFailed && m_preparedStrategy) {
    context.strategies.emplace_back(*m_preparedStrategy);
  }
  m_preparedStrategy = nullptr;
  m_detaching = std::move(context);

  if (cause == DetachmentCause::HostInvalidated) publish(DetachEvent{DetachPhase::Started, DetachReason::HostInvalidated, false});
  postEvent(Event{m_detaching->strategies.empty() ? EventType::DetachCompleted : EventType::DetachBlocked});
}

void TaskbarLayoutCoordinator::advanceDetaching() {
  if (!m_detaching) return;
  ++m_detaching->attempts;
  for (auto iterator = m_detaching->strategies.begin(); iterator != m_detaching->strategies.end();) {
    TaskbarLayoutStrategy& strategy = iterator->get();
    const auto result = m_detaching->cause == DetachmentCause::HostInvalidated ? strategy.invalidate() : strategy.detach();
    if (result == TaskbarLayoutStrategy::DetachResult::Failed) {
      ++iterator;
      continue;
    }
    if (result == TaskbarLayoutStrategy::DetachResult::NativeWindowDestroyed) m_detaching->disposition = NativeWindowDisposition::Destroyed;
    if (m_detaching->attachment && m_detaching->attachment->uses(strategy)) m_detaching->attachment.reset();
    iterator = m_detaching->strategies.erase(iterator);
  }
  if (m_detaching->strategies.empty()) {
    postEvent(Event{EventType::DetachCompleted});
    return;
  }
  if (m_detaching->attempts == 1)
    publish(DetachEvent{DetachPhase::Blocked,
                        m_detaching->cause == DetachmentCause::HostInvalidated ? DetachReason::HostInvalidated : DetachReason::PresentationReleased, false});
  if (m_detaching->attempts == kFastRetryAttempts) UWF_LOG_E("taskbar") << "taskbar detachment remains pending; strategies=" << m_detaching->strategies.size();
  m_asyncTimer->start(m_detaching->attempts < kFastRetryAttempts ? kFastRetryIntervalMs : kSlowRetryIntervalMs);
}

void TaskbarLayoutCoordinator::finishOperation(const AttachResult result) {
  m_lastAttachResult = result;
  if (result != AttachResult::Prepared) m_request.reset();
}

void TaskbarLayoutCoordinator::scheduleDetachedHostRefresh() {
  // Detached 没有资源可释放，不能伪造 Started/Recovering。只在离开原生消息栈
  // 后通知 View 重新探测新的 Explorer 宿主。
  if (m_hostRefreshScheduled) return;
  m_hostRefreshScheduled = true;
  QTimer::singleShot(0, this, [this]() {
    m_hostRefreshScheduled = false;
    if (m_detachObserver) m_detachObserver(DetachEvent{DetachPhase::Completed, DetachReason::HostInvalidated, false});
  });
}

void TaskbarLayoutCoordinator::publish(const DetachEvent event) { m_notifications.push_back(event); }

void TaskbarLayoutCoordinator::flushNotifications() {
  while (!m_processingEvents && !m_notifications.empty()) {
    const DetachEvent event = m_notifications.front();
    m_notifications.pop_front();
    if (m_detachObserver) m_detachObserver(event);
  }
}

bool TaskbarLayoutCoordinator::nativeEventFilter(const QByteArray&, void* const message, qintptr*) {
  if (!message || m_taskbarCreatedMessage == 0) return false;
  const auto* const nativeMessage = static_cast<const MSG*>(message);
  if (nativeMessage->message == m_taskbarCreatedMessage) postEvent(Event{EventType::HostInvalidated});
  return false;
}

}  // namespace uwf::ui
