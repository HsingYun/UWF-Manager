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

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QPointer>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "TaskbarLayoutStrategy.h"

class QTimer;
class QWindow;

namespace uwf::ui {

// Shell 布局事务的单一状态机。prepareAttach() 只提交隐藏的父子关系；
// activatePrepared() 在 Hub 已释放 fallback 后才提交可见性并 finalize。
// ActivateRequested 在每个状态下都给出与状态一致的 AttachResult：Prepared 走
// finalize，已 Attached 幂等成功，Detaching 回报释放进度，其余记为失败。
// 所有外部请求和事务结果均进入同一事件队列，原生回调不能越过状态边界。
class TaskbarLayoutCoordinator final : public QObject, private QAbstractNativeEventFilter {
 public:
  enum class DetachReason { PresentationReleased, HostInvalidated };
  enum class DetachPhase { Started, Blocked, Completed };
  struct DetachEvent {
    DetachPhase phase = DetachPhase::Completed;
    DetachReason reason = DetachReason::PresentationReleased;
  };
  using DetachObserver = std::function<void(DetachEvent)>;
  using VisibilityCommit = std::function<void()>;
  using VisibilityRollback = std::function<void()>;
  using NativeWindowRecreate = std::function<QWindow*()>;
  // Incompatible 保留策略层的固定能力判定；Failed 仍表示普通事务失败。
  // 原生窗口是否在 rollback 中被销毁是独立的清理结果，不能编码进失败原因。
  enum class AttachResult { Prepared, Attached, Retained, TemporarilyUnavailable, Incompatible, ReleasePending, ReleaseBlocked, Failed };
  enum class DetachStatus { AlreadyDetached, Pending };

  explicit TaskbarLayoutCoordinator(std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies, DetachObserver detachObserver);
  ~TaskbarLayoutCoordinator() override;

  [[nodiscard]] bool isCompatible() const { return !m_strategies.empty(); }
  AttachResult prepareAttach(QWindow* window, const QSize& logicalSize, const VisibilityCommit& makeVisible, const VisibilityRollback& makeInvisible,
                             const NativeWindowRecreate& recreateNativeWindow);
  AttachResult activatePrepared();
  [[nodiscard]] TaskbarLayoutStrategy::VerificationResult verify(const QWindow* window, WId currentWindowId) const;
  DetachStatus detach();

 private:
  friend class TaskbarLayoutCoordinatorTestAccess;

  enum class State {
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
  struct AttachRequest {
    QPointer<QWindow> window;
    QSize logicalSize;
    VisibilityCommit showWindow;
    VisibilityRollback hideWindow;
    NativeWindowRecreate recreateWindow;
  };
  enum class EventType {
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
  enum class Action {
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
  struct Event {
    EventType type;
    bool nativeWindowDestroyed = false;
    std::optional<AttachRequest> attachRequest = std::nullopt;
  };
  struct Transition {
    State nextState;
    Action action = Action::None;
  };
  using TransitionHandler = Transition (*)(State, const Event&);

  struct ActiveAttachment {
    std::reference_wrapper<TaskbarLayoutStrategy> strategy;
    QPointer<QWindow> window;
    VisibilityRollback hideWindow;
    NativeWindowRecreate recreateWindow;

    [[nodiscard]] bool uses(const TaskbarLayoutStrategy& candidate) const { return &strategy.get() == &candidate; }
  };
  enum class DetachmentCause { UserRequested, HostInvalidated };
  enum class NativeWindowDisposition { Preserved, Destroyed };
  struct DetachingContext {
    DetachmentCause cause = DetachmentCause::UserRequested;
    std::optional<ActiveAttachment> attachment;
    std::vector<std::reference_wrapper<TaskbarLayoutStrategy>> strategies;
    int attempts = 0;
    NativeWindowDisposition disposition = NativeWindowDisposition::Preserved;
    NativeWindowRecreate recreateWindow;
  };

  [[nodiscard]] static Transition reduce(State state, const Event& event);
  [[nodiscard]] static Transition ignoreEvent(State state, const Event& event);
  [[nodiscard]] static Transition beginPrepare(State state, const Event& event);
  [[nodiscard]] static Transition beginFinalize(State state, const Event& event);
  [[nodiscard]] static Transition prepared(State state, const Event& event);
  [[nodiscard]] static Transition attached(State state, const Event& event);
  [[nodiscard]] static Transition retained(State state, const Event& event);
  [[nodiscard]] static Transition temporary(State state, const Event& event);
  [[nodiscard]] static Transition incompatible(State state, const Event& event);
  [[nodiscard]] static Transition failed(State state, const Event& event);
  [[nodiscard]] static Transition retryPrepare(State state, const Event& event);
  [[nodiscard]] static Transition recreateForPrepare(State state, const Event& event);
  [[nodiscard]] static Transition recreateForActivation(State state, const Event& event);
  [[nodiscard]] static Transition recreateForFailure(State state, const Event& event);
  [[nodiscard]] static Transition recreateForIncompatible(State state, const Event& event);
  [[nodiscard]] static Transition resumePrepare(State state, const Event& event);
  [[nodiscard]] static Transition resumeActivation(State state, const Event& event);
  [[nodiscard]] static Transition completeAfterRecreation(State state, const Event& event);
  [[nodiscard]] static Transition failWindowRecreation(State state, const Event& event);
  [[nodiscard]] static Transition needsDetach(State state, const Event& event);
  [[nodiscard]] static Transition beginUserDetach(State state, const Event& event);
  [[nodiscard]] static Transition beginHostDetach(State state, const Event& event);
  [[nodiscard]] static Transition escalateHostDetach(State state, const Event& event);
  [[nodiscard]] static Transition advanceDetach(State state, const Event& event);
  [[nodiscard]] static Transition repeatDetach(State state, const Event& event);
  [[nodiscard]] static Transition completeDetach(State state, const Event& event);
  [[nodiscard]] static Transition finishDetachedRecreation(State state, const Event& event);
  [[nodiscard]] static Transition notifyDetachedHostRefresh(State state, const Event& event);
  [[nodiscard]] static Transition reportReleaseStatus(State state, const Event& event);

  void postEvent(Event event);
  void processEvents();
  void execute(Action action, const Event& event);
  void runPrepare();
  void runFinalize();
  void recreateWindow();
  void recreateDetachedWindow();
  void beginDetaching(DetachmentCause cause);
  void advanceDetaching();
  void finishOperation(AttachResult result);
  void scheduleDetachedHostRefresh();
  void publish(DetachEvent event);
  void flushNotifications();
  void markProcessIncompatible(TaskbarLayoutStrategy& strategy);
  [[nodiscard]] bool hasProcessCompatibleStrategy() const;
  [[nodiscard]] bool isProcessIncompatible(const TaskbarLayoutStrategy& strategy) const;
  [[nodiscard]] bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> m_strategies;
  std::vector<TaskbarLayoutStrategy*> m_processIncompatibleStrategies;
  State m_state = State::Detached;
  std::deque<Event> m_events;
  std::deque<DetachEvent> m_notifications;
  bool m_processingEvents = false;
  bool m_hostRefreshScheduled = false;
  std::optional<AttachRequest> m_request;
  std::optional<ActiveAttachment> m_attachment;
  TaskbarLayoutStrategy* m_preparedStrategy = nullptr;
  std::unique_ptr<TaskbarLayoutStrategy::AttachTransaction> m_transaction;
  std::optional<DetachingContext> m_detaching;
  AttachResult m_lastAttachResult = AttachResult::Failed;
  DetachObserver m_detachObserver;
  QTimer* m_asyncTimer = nullptr;
  quint32 m_taskbarCreatedMessage = 0;
};

[[nodiscard]] std::unique_ptr<TaskbarLayoutCoordinator> createDefaultTaskbarLayoutCoordinator(TaskbarLayoutCoordinator::DetachObserver detachObserver);

}  // namespace uwf::ui
