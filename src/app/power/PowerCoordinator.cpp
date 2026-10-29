// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PowerCoordinator.h"

#include <QElapsedTimer>
#include <QPointer>
#include <QScopeGuard>
#include <QTimer>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>

#include "app/common/ApplicationText.h"
#include "app/staging/FileStagingCommandResult.h"
#include "app/staging/FileStagingTask.h"
#include "app/uwf/UwfFilter.h"
#include "shared/Log.h"

namespace uwf::app {

PowerCoordinator::PowerCoordinator(PowerServices services, PowerInteraction interaction, QObject* parent)
    : QObject(parent),
      m_interaction(std::move(interaction)),
      m_session(services.session),
      m_fileStaging(services.fileStaging),
      m_uwfCapability(services.uwfCapability),
      m_stagingCoordinator(services.stagingCoordinator),
      m_traversal(std::move(services.traversal)),
      m_preshutdown(std::move(services.preshutdown)) {}

void PowerCoordinator::safeShutdown() { execute(PowerAction::Shutdown); }

void PowerCoordinator::safeRestart() { execute(PowerAction::Restart); }

void PowerCoordinator::directRestart() { executeDirectRestart(); }

void PowerCoordinator::executeDirectRestart() {
  if (m_actionActive) return;
  m_actionActive = true;
  const auto releaseAction = qScopeGuard([this] { m_actionActive = false; });
  try {
    PowerActionDialogRequest request{PowerAction::Restart, {}, std::nullopt, PowerActionDialogMode::Direct};
    const auto outcome = m_interaction.confirm(std::move(request));
    if (outcome == PowerActionDialogOutcome::Canceled) return;
    if (outcome != PowerActionDialogOutcome::DirectConfirmed) {
      throw std::logic_error("direct restart dialog returned an invalid outcome");
    }
    invokePowerAction(PowerAction::Restart);
  } catch (const std::exception& error) {
    reportPowerFailure(PowerAction::Restart, error, PowerActionDialogMode::Direct);
  } catch (...) {
    reportUnknownPowerFailure(PowerAction::Restart, PowerActionDialogMode::Direct);
  }
}

void PowerCoordinator::execute(const PowerAction action) {
  if (m_actionActive) return;
  m_actionActive = true;
  bool deferred = false;
  const auto releaseAction = qScopeGuard([&] {
    if (!deferred) m_actionActive = false;
  });
  try {
    auto batch = m_stagingCoordinator.reserveExternalBatch(
        [this, self = QPointer<PowerCoordinator>(this), action](const FileStagingBatchResult& result, FileStagingCoordinator::ExternalBatch batch) {
          if (!self) return;
          // 协调器会在发布同一批次的所有完成通知时调用这里。把交互式电源对话框
          // 排到下一轮事件循环，不能在协调器发布其它已等待命令的回调栈中进入
          // 模态事件循环。
          try {
            QTimer::singleShot(0, this, [this, action, result, batch = std::move(batch)]() mutable {
              const auto releaseAction = qScopeGuard([this] { m_actionActive = false; });
              executeWithCompletedStaging(action, result, std::move(batch));
            });
          } catch (...) {
            // 投递失败时 batch 的 RAII 析构会把执行权交还协调器；同时恢复动作门，
            // 否则本次基础设施异常会永久禁用后续安全关机/重启。
            m_actionActive = false;
            throw;
          }
        });
    if (!batch) {
      deferred = true;
      return;
    }
    executeReserved(action, std::move(*batch));
  } catch (const std::exception& error) {
    reportPowerFailure(action, error, PowerActionDialogMode::Safe);
  } catch (...) {
    reportUnknownPowerFailure(action, PowerActionDialogMode::Safe);
  }
}

void PowerCoordinator::executeReserved(const PowerAction action, FileStagingCoordinator::ExternalBatch batch) {
  PowerActionDialogMode failureMode = PowerActionDialogMode::Safe;
  std::optional<QList<app::FileStagingEntry>> stagedEntries;
  std::optional<app::ApplicationCommandResult> stagingResult;
  std::optional<FileStagingCommitResult> stagingCommitResult;
  // 只有形成最终批次结果后才完成租约。用户在提交前取消时，租约析构会把
  // 等待中的第二实例请求交给普通命令批次。
  const auto completeBatch = qScopeGuard([&] {
    if (stagingResult) batch.complete({*stagingResult, stagedEntries, stagingCommitResult});
  });

  std::optional<FileStagingTask> stagingTask;
  try {
    const auto advanceStaging = [&]() -> PowerStagingProgress {
      if (stagingResult && !stagingResult->completed()) return {PowerStagingState::Failed, 0, 0, {}, stagingResult->detail};
      try {
        if (!stagingTask || !stagingTask->preparationFinished()) throw std::logic_error("file staging task is unavailable");
        FileStagingTaskProgress progress;
        if (!stagingTask->finished()) progress = stagingTask->advance();
        if (!stagingTask->finished()) {
          return {PowerStagingState::InProgress, progress.processedFiles, progress.totalFiles, progress.currentPath, {}};
        }

        const auto& result = stagingTask->result();
        stagingResult = app::toApplicationCommandResult(result, result.succeeded() ? QString{} : m_interaction.describeStagingResult(result));
        stagingCommitResult = result;
        UWF_LOG_I("power") << "automatic file staging completed: discovered=" << result.discoveredFiles << " committed=" << result.committedFiles
                           << " skippedFiles=" << result.skippedFiles << " skippedEntries=" << result.skippedEntries << " failures=" << result.failures.size();
        if (result.succeeded()) {
          return {PowerStagingState::Completed, progress.processedFiles, progress.totalFiles, progress.currentPath, {}};
        }

        return {
            PowerStagingState::Failed, progress.processedFiles, progress.totalFiles, progress.currentPath, m_interaction.describeStagingResult(result), result};
      } catch (const std::exception& error) {
        if (!stagingTask || !stagingTask->failed()) UWF_LOG_E("power") << "automatic file staging failed: error=" << error.what();
        const QString detail = ApplicationText::tr("Automatic file staging stopped unexpectedly:\n%1").arg(QString::fromUtf8(error.what()));
        stagingResult = app::ApplicationCommandResult::infrastructureFailure(detail);
        return {PowerStagingState::Failed, 0, 0, {}, detail};
      } catch (...) {
        if (!stagingTask || !stagingTask->failed()) UWF_LOG_E("power") << "automatic file staging failed: error=non-standard-exception";
        const QString detail = ApplicationText::tr("Automatic file staging stopped because of an unknown error.");
        stagingResult = app::ApplicationCommandResult::infrastructureFailure(detail);
        return {PowerStagingState::Failed, 0, 0, {}, detail};
      }
    };

    PowerActionDialogRequest dialogRequest{
        action, [&]() -> std::optional<PowerStagingPreparation> {
          if (stagingResult && !stagingResult->completed()) return PowerStagingFailure{stagingResult->detail};
          try {
            if (!stagedEntries) {
              try {
                QElapsedTimer elapsed;
                elapsed.start();
                UWF_LOG_I("power") << "reading file staging list for safe power action";
                stagedEntries = m_fileStaging.load();
                UWF_LOG_I("power") << "file staging list read: entries=" << stagedEntries->size() << " elapsedMs=" << elapsed.elapsed();
              } catch (const std::exception& error) {
                UWF_LOG_E("power") << "file staging list read failed: error=" << error.what();
                const QString detail = ApplicationText::tr("The file staging list could not be read:\n%1").arg(QString::fromUtf8(error.what()));
                stagingResult = app::ApplicationCommandResult::infrastructureFailure(detail);
                return PowerStagingFailure{detail};
              } catch (...) {
                UWF_LOG_E("power") << "file staging list read failed: error=non-standard-exception";
                const QString detail = ApplicationText::tr("The file staging list could not be read because of an unknown error.");
                stagingResult = app::ApplicationCommandResult::infrastructureFailure(detail);
                return PowerStagingFailure{detail};
              }
              if (stagedEntries->isEmpty()) {
                stagingResult = app::ApplicationCommandResult{};
                return PowerStagingNotRequired{};
              }
              stagingTask.emplace(m_session, *stagedEntries, m_uwfCapability, m_traversal);
            }

            if (!stagingTask) throw std::logic_error("file staging task was not created for a non-empty staging list");
            if (!stagingTask->pollPreparation()) return std::nullopt;
            const std::size_t filesToCommit = stagingTask->totalFiles();
            UWF_LOG_I("power") << "automatic file staging prepared: files=" << filesToCommit;
            if (stagingTask->finished()) {
              const auto status = advanceStaging();
              if (status.state == PowerStagingState::Failed) return PowerStagingFailure{status.failureDetails, status.failureResult};
              return PowerStagingNotRequired{};
            }
            return PowerStagingCommit{filesToCommit, advanceStaging};
          } catch (const std::exception& error) {
            if (!stagingTask || !stagingTask->failed()) UWF_LOG_E("power") << "automatic file staging preparation failed: error=" << error.what();
            const QString detail = ApplicationText::tr("Automatic file staging stopped unexpectedly:\n%1").arg(QString::fromUtf8(error.what()));
            stagingResult = app::ApplicationCommandResult::infrastructureFailure(detail);
            return PowerStagingFailure{detail};
          } catch (...) {
            if (!stagingTask || !stagingTask->failed()) UWF_LOG_E("power") << "automatic file staging preparation failed: error=non-standard-exception";
            const QString detail = ApplicationText::tr("Automatic file staging stopped because of an unknown error.");
            stagingResult = app::ApplicationCommandResult::infrastructureFailure(detail);
            return PowerStagingFailure{detail};
          }
        }};

    const auto dialogOutcome = m_interaction.confirm(std::move(dialogRequest));
    bool continuationApproved = false;
    switch (dialogOutcome) {
      case PowerActionDialogOutcome::Canceled:
        return;
      case PowerActionDialogOutcome::CanceledAfterStagingFailure:
        UWF_LOG_I("power") << "safe power action canceled after file staging failure";
        return;
      case PowerActionDialogOutcome::DirectConfirmed:
        failureMode = PowerActionDialogMode::Direct;
        invokePowerAction(action);
        if (!stagingResult) {
          app::ApplicationCommandResult skipped;
          skipped.outcome = app::ApplicationCommandOutcome::Rejected;
          skipped.detail = ApplicationText::tr("File staging was skipped because a direct power action was selected.");
          batch.complete({skipped, stagedEntries});
        }
        return;
      case PowerActionDialogOutcome::ContinuedAfterStagingFailure:
        UWF_LOG_W("power") << "safe power action continuing after user accepted file staging failures";
        if (!stagingResult) throw std::logic_error("safe power action has no file staging failure to approve");
        if (stagingResult->outcome != ApplicationCommandOutcome::CompletedWithFailures || stagingResult->failedFiles == 0)
          throw std::logic_error("cannot approve a file staging failure without a recorded failed target");
        continuationApproved = true;
        break;
      case PowerActionDialogOutcome::Confirmed:
        break;
    }

    if (!stagingResult) throw std::logic_error("safe power action has no final file staging result");
    if (stagingResult->outcome != app::ApplicationCommandOutcome::Succeeded && !continuationApproved) {
      throw std::logic_error("safe power action cannot continue without a completed staging result or explicit user approval");
    }
    invokePowerAction(action);
  } catch (const std::exception& error) {
    reportPowerFailure(action, error, failureMode);
  } catch (...) {
    reportUnknownPowerFailure(action, failureMode);
  }
}

void PowerCoordinator::executeWithCompletedStaging(const PowerAction action, FileStagingBatchResult stagingResult,
                                                   FileStagingCoordinator::ExternalBatch batch) {
  PowerActionDialogMode failureMode = PowerActionDialogMode::Safe;
  bool completeBatch = false;
  const auto releaseBatch = qScopeGuard([&] {
    if (completeBatch) batch.complete(stagingResult);
  });
  try {
    PowerActionDialogRequest request{action, {}, std::nullopt};
    if (stagingResult.command.outcome != app::ApplicationCommandOutcome::Succeeded) {
      request.completedStagingFailure = PowerStagingFailure{
          stagingResult.command.detail.isEmpty()
              ? (stagingResult.command.completed() ? ApplicationText::tr("All staged targets were processed; one or more operations failed.")
                                                   : ApplicationText::tr("Automatic file staging stopped because of an unknown error."))
              : stagingResult.command.detail,
          stagingResult.result};
    }

    const auto outcome = m_interaction.confirm(std::move(request));
    bool continuationApproved = false;
    if (outcome == PowerActionDialogOutcome::Canceled || outcome == PowerActionDialogOutcome::CanceledAfterStagingFailure) {
      // 已完整处理的批次仍可满足确认期间到达的第二实例命令；基础设施失败
      // 则释放租约，让等待方重新执行，不能把用户取消当成关机放行授权。
      completeBatch = stagingResult.command.completed();
      return;
    }
    if (outcome == PowerActionDialogOutcome::DirectConfirmed) {
      failureMode = PowerActionDialogMode::Direct;
      completeBatch = true;
      invokePowerAction(action);
      return;
    }
    if (outcome == PowerActionDialogOutcome::ContinuedAfterStagingFailure) {
      UWF_LOG_W("power") << "safe power action continuing after user accepted file staging failures";
      if (stagingResult.command.outcome != ApplicationCommandOutcome::CompletedWithFailures || stagingResult.command.failedFiles == 0)
        throw std::logic_error("cannot approve a file staging failure without a recorded failed target");
      continuationApproved = true;
    }
    if (stagingResult.command.outcome != app::ApplicationCommandOutcome::Succeeded && !continuationApproved) {
      throw std::logic_error("safe power action cannot continue without a completed staging result or explicit user approval");
    }
    completeBatch = true;
    invokePowerAction(action);
  } catch (const std::exception& error) {
    reportPowerFailure(action, error, failureMode);
  } catch (...) {
    reportUnknownPowerFailure(action, failureMode);
  }
}

void PowerCoordinator::invokePowerAction(const PowerAction action) {
  using ControlResult = PreshutdownControlResult;
  // 先取得本次电源调用使用的权威对象状态，再把服务端跳过令牌的有效窗口
  // 收窄到真正的关机/重启调用前。读取失败时根本不武装令牌；进程在调用前
  // 异常终止时，服务误跳过自身 PRESHUTDOWN 提交的时间窗口也因此最小。
  provider::UwfFilter filter(m_session);
  const auto row = filter.read();

  const ControlResult skipResult = m_preshutdown.markHandled ? m_preshutdown.markHandled() : ControlResult::NotApplicable;
  if (skipResult == ControlResult::Unacknowledged) {
    UWF_LOG_W("power") << "enhanced mode preshutdown skip token was not acknowledged; service fallback remains authoritative";
  }
  auto revokeSkipOnFailure = qScopeGuard([&] {
    if (skipResult != ControlResult::NotApplicable && m_preshutdown.markRequired && m_preshutdown.markRequired() == ControlResult::Unacknowledged) {
      UWF_LOG_W("power") << "enhanced mode preshutdown skip token revocation was not acknowledged";
    }
  });
  if (action == PowerAction::Shutdown)
    filter.shutdownSystem(row);
  else
    filter.restartSystem(row);
  revokeSkipOnFailure.dismiss();
}

void PowerCoordinator::reportPowerFailure(const PowerAction action, const std::exception& error, const PowerActionDialogMode mode) {
  const bool shutdown = action == PowerAction::Shutdown;
  const bool direct = mode == PowerActionDialogMode::Direct;
  UWF_LOG_E("power") << (direct ? (shutdown ? "direct shutdown failed: error=" : "direct restart failed: error=")
                                : (shutdown ? "safe shutdown failed: error=" : "safe restart failed: error="))
                     << error.what();
  const QString title = direct ? (shutdown ? ApplicationText::tr("Direct shutdown failed") : ApplicationText::tr("Direct restart failed"))
                               : (shutdown ? ApplicationText::tr("Safe shutdown failed") : ApplicationText::tr("Safe restart failed"));
  const QString message = shutdown ? ApplicationText::tr("Shutdown failed: %1").arg(QString::fromUtf8(error.what()))
                                   : ApplicationText::tr("Restart failed: %1").arg(QString::fromUtf8(error.what()));
  m_interaction.warning(title, message);
}

void PowerCoordinator::reportUnknownPowerFailure(const PowerAction action, const PowerActionDialogMode mode) {
  const bool shutdown = action == PowerAction::Shutdown;
  const bool direct = mode == PowerActionDialogMode::Direct;
  UWF_LOG_E("power") << (direct ? (shutdown ? "direct shutdown failed: error=non-standard-exception" : "direct restart failed: error=non-standard-exception")
                                : (shutdown ? "safe shutdown failed: error=non-standard-exception" : "safe restart failed: error=non-standard-exception"));
  m_interaction.warning(direct ? (shutdown ? ApplicationText::tr("Direct shutdown failed") : ApplicationText::tr("Direct restart failed"))
                               : (shutdown ? ApplicationText::tr("Safe shutdown failed") : ApplicationText::tr("Safe restart failed")),
                        ApplicationText::tr("The operation failed with an unknown error."));
}

}  // namespace uwf::app

#include "moc_PowerCoordinator.cpp"
