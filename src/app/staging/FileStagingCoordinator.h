// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QPointer>
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "app/api/FileStagingTypes.h"
#include "app/startup/ApplicationCommand.h"

namespace uwf::app {

class FileStagingCommandOperation {
 public:
  virtual ~FileStagingCommandOperation() = default;
  // Called on the coordinator's thread. Complete once, inline or later; a
  // throwing start must not also deliver a completion. Factories only construct.
  virtual void start() = 0;
  // Keep the operation alive until its current completion callback returns.
  virtual void disposeAfterCallback() = 0;
};

struct FileStagingBatchResult {
  app::ApplicationCommandResult command;
  // 已成功读取的原始暂存列表。即使列表为空也保留 engaged 状态；nullopt
  // 明确表示批次在形成可核验快照前失败，不能把空列表误当成它的输入。
  std::optional<QList<app::FileStagingEntry>> sourceEntries;
  std::optional<FileStagingCommitResult> result = std::nullopt;
};

// 把任意来源的暂存提交请求合并成一个进行中的批次。每个请求都收到同一份
// 最终结果；不会为了第二个客户端重建进度窗或重复提交文件。
class FileStagingCoordinator : public QObject {
  Q_OBJECT
 public:
  using Completion = std::function<void(const FileStagingBatchResult&)>;
  using Progress = std::function<void(std::size_t, std::size_t)>;
  using OperationFactory = std::function<std::unique_ptr<FileStagingCommandOperation>(Completion, Progress)>;

  // 安全电源流程需要先计算、等待用户确认，再在自己的对话框中提交。租约把
  // 这段时间纳入同一个单飞域：服务和第二实例到达的请求只加入等待队列，
  // 不会另起一个 FileStagingTask。若租约未产生最终结果（例如用户在提交前
  // 取消），析构时会自动为等待队列启动普通命令批次。
  class ExternalBatch final {
   public:
    ExternalBatch() = default;
    ~ExternalBatch();
    ExternalBatch(const ExternalBatch&) = delete;
    ExternalBatch& operator=(const ExternalBatch&) = delete;
    ExternalBatch(ExternalBatch&& other) noexcept;
    ExternalBatch& operator=(ExternalBatch&& other) noexcept;

    void complete(const FileStagingBatchResult& result);
    [[nodiscard]] explicit operator bool() const { return !m_owner.isNull(); }

   private:
    friend class FileStagingCoordinator;
    explicit ExternalBatch(FileStagingCoordinator* owner) : m_owner(owner) {}
    void release();

    QPointer<FileStagingCoordinator> m_owner;
  };

  using ExternalCompletion = std::function<void(const FileStagingBatchResult&, ExternalBatch)>;

  explicit FileStagingCoordinator(OperationFactory operationFactory, QObject* parent = nullptr);
  ~FileStagingCoordinator() override;

  void requestCommit(Completion completion);
  void setSuspended(bool suspended);
  // 空 optional 表示已有批次拥有执行权；completedActiveBatch 已加入该批次，
  // 最终结果到达后协调器会把外部租约一并移交给调用方，使确认窗口期间仍
  // 保持单飞所有权，不会让同时到达的命令抢先启动第二个提交批次。
  [[nodiscard]] std::optional<ExternalBatch> reserveExternalBatch(ExternalCompletion completedActiveBatch);
  [[nodiscard]] bool active() const;

 signals:
  void activeChanged(bool active);
  void progressChanged(std::size_t processed, std::size_t total);

 private:
  enum class Ownership {
    Idle,
    Command,
    External,
    Count,
  };
  enum class OwnershipEvent {
    CommandStarted,
    CommandCompleted,
    CommandHandedOff,
    ExternalReserved,
    ExternalCompleted,
    ExternalReleased,
    Count,
  };
  struct OwnershipTransition {
    Ownership next = Ownership::Idle;
    bool valid = false;
  };

  [[nodiscard]] static const std::array<OwnershipTransition, static_cast<std::size_t>(Ownership::Count) * static_cast<std::size_t>(OwnershipEvent::Count)>&
  ownershipTransitions();
  void postOwnershipEvent(OwnershipEvent event);
  void startCommandBatch();
  void completeExternalBatch(const FileStagingBatchResult& result);
  void releaseExternalBatch();
  void finish(const FileStagingBatchResult& result);

  OperationFactory m_operationFactory;
  Ownership m_ownership = Ownership::Idle;
  bool m_suspended = false;
  std::unique_ptr<FileStagingCommandOperation> m_operation;
  std::vector<Completion> m_completions;
  std::optional<ExternalCompletion> m_externalCompletion;
};

}  // namespace uwf::app
