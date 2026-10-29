// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingCoordinator.h"

#include <exception>
#include <stdexcept>
#include <utility>

#include "app/common/ApplicationText.h"
#include "shared/Log.h"

namespace uwf::app {

namespace {
void notifyCompletion(const FileStagingCoordinator::Completion& completion, const FileStagingBatchResult& result) noexcept {
  try {
    completion(result);
  } catch (const std::exception& error) {
    UWF_LOG_E("staging") << "file staging completion observer failed: error=" << error.what();
  } catch (...) {
    UWF_LOG_E("staging") << "file staging completion observer failed: error=non-standard-exception";
  }
}

void notifyExternalCompletion(FileStagingCoordinator::ExternalCompletion& completion, const FileStagingBatchResult& result,
                              FileStagingCoordinator::ExternalBatch batch) noexcept {
  try {
    completion(result, std::move(batch));
  } catch (const std::exception& error) {
    UWF_LOG_E("staging") << "file staging external completion observer failed: error=" << error.what();
  } catch (...) {
    UWF_LOG_E("staging") << "file staging external completion observer failed: error=non-standard-exception";
  }
}

}  // namespace

FileStagingCoordinator::FileStagingCoordinator(OperationFactory operationFactory, QObject* parent)
    : QObject(parent), m_operationFactory(std::move(operationFactory)) {}

FileStagingCoordinator::~FileStagingCoordinator() = default;

const std::array<FileStagingCoordinator::OwnershipTransition,
                 static_cast<std::size_t>(FileStagingCoordinator::Ownership::Count) * static_cast<std::size_t>(FileStagingCoordinator::OwnershipEvent::Count)>&
FileStagingCoordinator::ownershipTransitions() {
  static constexpr auto table = [] {
    std::array<OwnershipTransition, static_cast<std::size_t>(Ownership::Count) * static_cast<std::size_t>(OwnershipEvent::Count)> result{};
    const auto set = [&result](const Ownership from, const OwnershipEvent event, const Ownership to) {
      result[static_cast<std::size_t>(from) * static_cast<std::size_t>(OwnershipEvent::Count) + static_cast<std::size_t>(event)] = {to, true};
    };
    set(Ownership::Idle, OwnershipEvent::CommandStarted, Ownership::Command);
    set(Ownership::Idle, OwnershipEvent::ExternalReserved, Ownership::External);
    set(Ownership::Command, OwnershipEvent::CommandCompleted, Ownership::Idle);
    set(Ownership::Command, OwnershipEvent::CommandHandedOff, Ownership::External);
    set(Ownership::External, OwnershipEvent::ExternalCompleted, Ownership::Idle);
    set(Ownership::External, OwnershipEvent::ExternalReleased, Ownership::Idle);
    return result;
  }();
  return table;
}

void FileStagingCoordinator::postOwnershipEvent(const OwnershipEvent event) {
  const std::size_t index = static_cast<std::size_t>(m_ownership) * static_cast<std::size_t>(OwnershipEvent::Count) + static_cast<std::size_t>(event);
  const auto& transition = ownershipTransitions().at(index);
  if (!transition.valid) throw std::logic_error("invalid file staging ownership transition");
  const bool wasActive = active();
  m_ownership = transition.next;
  if (active() != wasActive) emit activeChanged(active());
}

FileStagingCoordinator::ExternalBatch::~ExternalBatch() { release(); }

FileStagingCoordinator::ExternalBatch::ExternalBatch(ExternalBatch&& other) noexcept : m_owner(std::exchange(other.m_owner, {})) {}

FileStagingCoordinator::ExternalBatch& FileStagingCoordinator::ExternalBatch::operator=(ExternalBatch&& other) noexcept {
  if (this == &other) return *this;
  release();
  m_owner = std::exchange(other.m_owner, {});
  return *this;
}

void FileStagingCoordinator::ExternalBatch::complete(const FileStagingBatchResult& result) {
  if (!m_owner) return;
  auto owner = std::exchange(m_owner, {});
  owner->completeExternalBatch(result);
}

void FileStagingCoordinator::ExternalBatch::release() {
  if (!m_owner) return;
  auto owner = std::exchange(m_owner, {});
  owner->releaseExternalBatch();
}

void FileStagingCoordinator::requestCommit(Completion completion) {
  if (!completion) throw std::invalid_argument("file staging completion callback is required");
  m_completions.push_back(std::move(completion));
  if (m_ownership != Ownership::Idle) return;
  startCommandBatch();
}
void FileStagingCoordinator::setSuspended(const bool suspended) {
  if (m_suspended == suspended) return;
  m_suspended = suspended;
  if (!suspended) startCommandBatch();
}

std::optional<FileStagingCoordinator::ExternalBatch> FileStagingCoordinator::reserveExternalBatch(ExternalCompletion completedActiveBatch) {
  if (!completedActiveBatch) throw std::invalid_argument("file staging external completion callback is required");
  if (m_suspended) throw std::logic_error("file staging execution is suspended during configuration apply");
  if (m_ownership == Ownership::External) {
    throw std::logic_error("a file staging external batch already owns execution");
  }
  if (m_ownership == Ownership::Command) {
    if (m_externalCompletion) throw std::logic_error("a file staging external batch is already waiting for ownership");
    m_externalCompletion = std::move(completedActiveBatch);
    return std::nullopt;
  }
  const QPointer guard(this);
  postOwnershipEvent(OwnershipEvent::ExternalReserved);
  return guard ? std::optional<ExternalBatch>(ExternalBatch(this)) : std::nullopt;
}

bool FileStagingCoordinator::active() const { return m_ownership != Ownership::Idle; }

void FileStagingCoordinator::startCommandBatch() {
  if (m_suspended || m_ownership != Ownership::Idle || m_completions.empty()) return;
  const auto fail = [this](QString detail) {
    m_operation.reset();
    const FileStagingBatchResult result{app::ApplicationCommandResult::infrastructureFailure(std::move(detail)), std::nullopt};
    finish(result);
  };
  const QPointer guard(this);
  postOwnershipEvent(OwnershipEvent::CommandStarted);
  if (!guard) return;
  try {
    auto operation = m_operationFactory([this](const FileStagingBatchResult& result) { finish(result); },
                                        [this](const std::size_t processed, const std::size_t total) { emit progressChanged(processed, total); });
    if (!operation) throw std::runtime_error("file staging operation factory returned no operation");
    m_operation = std::move(operation);
    m_operation->start();
  } catch (const std::exception& error) {
    UWF_LOG_E("staging") << "file staging batch startup failed: error=" << error.what();
    fail(ApplicationText::tr("Automatic file staging stopped unexpectedly:\n%1").arg(QString::fromUtf8(error.what())));
  } catch (...) {
    UWF_LOG_E("staging") << "file staging batch startup failed: error=non-standard-exception";
    fail(ApplicationText::tr("Automatic file staging stopped because of an unknown error."));
  }
}

void FileStagingCoordinator::completeExternalBatch(const FileStagingBatchResult& result) {
  if (m_ownership != Ownership::External) return;
  auto completions = std::exchange(m_completions, {});
  postOwnershipEvent(OwnershipEvent::ExternalCompleted);
  for (const auto& completion : completions) notifyCompletion(completion, result);
}

void FileStagingCoordinator::releaseExternalBatch() {
  if (m_ownership != Ownership::External) return;
  const QPointer guard(this);
  postOwnershipEvent(OwnershipEvent::ExternalReleased);
  if (guard) startCommandBatch();
}

void FileStagingCoordinator::finish(const FileStagingBatchResult& result) {
  UWF_LOG_I("staging") << "command batch completed: outcome=" << static_cast<int>(result.command.outcome) << " discovered=" << result.command.discoveredFiles
                       << " committed=" << result.command.committedFiles << " skippedFiles=" << result.command.skippedFiles
                       << " skippedEntries=" << result.command.skippedEntries << " failures=" << result.command.failedFiles;
  auto completions = std::exchange(m_completions, {});
  // finish() 由 Operation 自己的完成回调进入，不能在其成员函数尚未返回时
  // 直接 delete this；把所有权交还 Qt 事件循环，在当前调用栈退出后销毁。
  FileStagingCommandOperation* completedOperation = m_operation.release();
  auto externalCompletion = std::exchange(m_externalCompletion, std::nullopt);
  postOwnershipEvent(externalCompletion ? OwnershipEvent::CommandHandedOff : OwnershipEvent::CommandCompleted);
  ExternalBatch batch(externalCompletion ? this : nullptr);
  if (completedOperation) completedOperation->disposeAfterCallback();
  for (const auto& completion : completions) notifyCompletion(completion, result);
  if (externalCompletion) notifyExternalCompletion(*externalCompletion, result, std::move(batch));
}

}  // namespace uwf::app

#include "moc_FileStagingCoordinator.cpp"
