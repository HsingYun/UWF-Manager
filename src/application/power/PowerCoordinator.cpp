// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/power/PowerCoordinator.h"

#include <QElapsedTimer>
#include <QPointer>
#include <QScopeGuard>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>

#include "application/localization/ApplicationText.h"
#include "application/power/contracts/PowerInteraction.h"
#include "application/staging/execution/FileStagingCommandResult.h"
#include "application/staging/execution/FileStagingTask.h"
#include "foundation/logging/Log.h"

namespace uwf::app {

class PowerCoordinator::Operation final : public std::enable_shared_from_this<Operation> {
 public:
  Operation(PowerCoordinator& owner, const PowerServices& services, const PowerInteraction& interaction)
      : m_owner(&owner),
        m_interaction(interaction),
        m_repository(services.repository),
        m_fileStaging(services.fileStaging),
        m_uwfCapability(services.uwfCapability),
        m_traversal(services.traversal),
        m_preshutdown(services.preshutdown) {}

  void executeDirectRestart();
  void executeSafe(PowerAction action);

 private:
  PowerActionDialogOutcome confirm(PowerActionDialogRequest request) {
    // Dialogs may keep their callbacks until deferred destruction. Their state
    // belongs to this action, and becomes inert as soon as confirmation ends.
    const auto finish = qScopeGuard([this] {
      m_closed = true;
      m_stagingTask.reset();
    });
    return m_interaction.confirm(std::move(request));
  }
  std::optional<PowerStagingPreparation> pollStagingPreparation();
  std::optional<PowerStagingPreparation> pollStagingRevalidation();
  PowerStagingProgress advanceStaging();
  void invokePowerAction(PowerAction action);
  void reportPowerFailure(PowerAction action, const std::exception& error, PowerActionDialogMode mode);
  void reportUnknownPowerFailure(PowerAction action, PowerActionDialogMode mode);

  QPointer<PowerCoordinator> m_owner;
  PowerInteraction m_interaction;
  std::shared_ptr<UwfMaintenanceRepository> m_repository;
  FileStagingStore& m_fileStaging;
  UwfCapability m_uwfCapability;
  std::shared_ptr<const CommitTraversalSource> m_traversal;
  PreshutdownCommitControl m_preshutdown;
  bool m_closed = false;
  bool m_revalidating = false;
  bool m_finalPreparationValidated = false;
  std::optional<QList<FileStagingEntry>> m_stagedEntries;
  std::optional<FileStagingCommandResult> m_stagingResult;
  std::optional<FileStagingTask> m_stagingTask;
};

PowerCoordinator::PowerCoordinator(PowerServices services, PowerInteraction interaction, QObject* parent)
    : QObject(parent), m_services(std::move(services)), m_interaction(std::move(interaction)) {
  if (!m_services.repository) throw std::invalid_argument("power coordination requires a repository");
}

void PowerCoordinator::safeShutdown() { execute(PowerAction::Shutdown, PowerActionDialogMode::Safe); }

void PowerCoordinator::safeRestart() { execute(PowerAction::Restart, PowerActionDialogMode::Safe); }

void PowerCoordinator::directRestart() { execute(PowerAction::Restart, PowerActionDialogMode::Direct); }

void PowerCoordinator::execute(const PowerAction action, const PowerActionDialogMode mode) {
  if (m_actionActive || m_suspended) return;
  const auto operation = std::make_shared<Operation>(*this, m_services, m_interaction);
  const QPointer self(this);
  m_actionActive = true;
  const auto releaseAction = qScopeGuard([self] {
    if (!self) return;
    self->m_actionActive = false;
    emit self->activeChanged(false);
  });
  emit activeChanged(true);
  if (!self) return;
  if (mode == PowerActionDialogMode::Direct)
    operation->executeDirectRestart();
  else
    operation->executeSafe(action);
}

void PowerCoordinator::Operation::executeDirectRestart() {
  try {
    PowerActionDialogRequest request{PowerAction::Restart, {}, PowerActionDialogMode::Direct};
    const auto outcome = confirm(std::move(request));
    if (!m_owner) return;
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

PowerStagingProgress PowerCoordinator::Operation::advanceStaging() {
  if (m_closed || !m_owner) return {PowerStagingState::Failed, 0, 0, {}, {}};
  if (m_stagingResult && !m_stagingResult->completed()) return {PowerStagingState::Failed, 0, 0, {}, m_stagingResult->detail};
  try {
    if (!m_stagingTask || !m_stagingTask->preparationFinished()) throw std::logic_error("file staging task is unavailable");
    FileStagingTaskProgress progress;
    if (!m_stagingTask->finished()) progress = m_stagingTask->advance();
    if (!m_stagingTask->finished()) {
      return {PowerStagingState::InProgress, progress.processedFiles, progress.totalFiles, progress.currentPath, {}, {},
              m_stagingTask->automaticCommitSummary()};
    }

    const auto& result = m_stagingTask->result();
    m_stagingResult = app::toFileStagingCommandResult(result, result.succeeded() ? QString{} : m_interaction.describeStagingResult(result));
    UWF_LOG_I("power") << "automatic file staging completed: discovered=" << result.discoveredFiles << " committed=" << result.committedFiles
                       << " skippedFiles=" << result.skippedFiles << " skippedEntries=" << result.skippedEntries << " pausedFiles=" << result.pausedFiles
                       << " pausedEntries=" << result.pausedEntries << " failures=" << result.failures.size();
    if (result.succeeded()) {
      return {PowerStagingState::Completed, progress.processedFiles, progress.totalFiles, progress.currentPath, {}, {}, result.automaticCommit};
    }

    return {PowerStagingState::Failed, progress.processedFiles, progress.totalFiles, progress.currentPath, m_interaction.describeStagingResult(result), result};
  } catch (const std::exception& error) {
    if (!m_stagingTask || !m_stagingTask->failed()) UWF_LOG_E("power") << "automatic file staging failed: error=" << error.what();
    const QString detail = ApplicationText::tr("Automatic file staging stopped unexpectedly:\n%1").arg(QString::fromUtf8(error.what()));
    m_stagingResult = app::FileStagingCommandResult::infrastructureFailure(detail);
    return {PowerStagingState::Failed, 0, 0, {}, detail};
  } catch (...) {
    if (!m_stagingTask || !m_stagingTask->failed()) UWF_LOG_E("power") << "automatic file staging failed: error=non-standard-exception";
    const QString detail = ApplicationText::tr("Automatic file staging stopped because of an unknown error.");
    m_stagingResult = app::FileStagingCommandResult::infrastructureFailure(detail);
    return {PowerStagingState::Failed, 0, 0, {}, detail};
  }
}

std::optional<PowerStagingPreparation> PowerCoordinator::Operation::pollStagingPreparation() {
  if (m_closed || !m_owner) return PowerStagingNotRequired{};
  if (m_stagingResult && !m_stagingResult->completed()) return PowerStagingFailure{m_stagingResult->detail};
  try {
    if (!m_stagedEntries) {
      try {
        QElapsedTimer elapsed;
        elapsed.start();
        UWF_LOG_I("power") << "reading file staging list for safe power action";
        m_stagedEntries = m_fileStaging.load();
        UWF_LOG_I("power") << "file staging list read: entries=" << m_stagedEntries->size() << " elapsedMs=" << elapsed.elapsed();
      } catch (const std::exception& error) {
        UWF_LOG_E("power") << "file staging list read failed: error=" << error.what();
        const QString detail = ApplicationText::tr("The file staging list could not be read:\n%1").arg(QString::fromUtf8(error.what()));
        m_stagingResult = app::FileStagingCommandResult::infrastructureFailure(detail);
        return PowerStagingFailure{detail};
      } catch (...) {
        UWF_LOG_E("power") << "file staging list read failed: error=non-standard-exception";
        const QString detail = ApplicationText::tr("The file staging list could not be read because of an unknown error.");
        m_stagingResult = app::FileStagingCommandResult::infrastructureFailure(detail);
        return PowerStagingFailure{detail};
      }
      if (!m_owner) return PowerStagingNotRequired{};
      if (m_stagedEntries->isEmpty()) {
        m_stagingResult = app::FileStagingCommandResult{};
        return PowerStagingNotRequired{};
      }
      m_stagingTask.emplace(m_repository, *m_stagedEntries, m_uwfCapability, m_traversal);
    }

    if (!m_stagingTask) throw std::logic_error("file staging task was not created for a non-empty staging list");
    if (!m_stagingTask->pollPreparation()) return std::nullopt;
    const std::size_t filesToCommit = m_stagingTask->totalFiles();
    UWF_LOG_I("power") << "automatic file staging prepared: files=" << filesToCommit;
    if (m_stagingTask->finished()) {
      const auto status = advanceStaging();
      if (status.state == PowerStagingState::Failed) return PowerStagingFailure{status.failureDetails, status.failureResult};
      return PowerStagingNotRequired{m_stagingTask->automaticCommitSummary(), m_stagingTask->proposal()};
    }
    return PowerStagingCommit{filesToCommit, [operation = shared_from_this()] { return operation->advanceStaging(); },
                              m_stagingTask->automaticCommitSummary(), m_stagingTask->proposal()};
  } catch (const std::exception& error) {
    if (!m_stagingTask || !m_stagingTask->failed()) UWF_LOG_E("power") << "automatic file staging preparation failed: error=" << error.what();
    const QString detail = ApplicationText::tr("Automatic file staging stopped unexpectedly:\n%1").arg(QString::fromUtf8(error.what()));
    m_stagingResult = app::FileStagingCommandResult::infrastructureFailure(detail);
    return PowerStagingFailure{detail};
  } catch (...) {
    if (!m_stagingTask || !m_stagingTask->failed()) UWF_LOG_E("power") << "automatic file staging preparation failed: error=non-standard-exception";
    const QString detail = ApplicationText::tr("Automatic file staging stopped because of an unknown error.");
    m_stagingResult = app::FileStagingCommandResult::infrastructureFailure(detail);
    return PowerStagingFailure{detail};
  }
}
std::optional<PowerStagingPreparation> PowerCoordinator::Operation::pollStagingRevalidation() {
  if (m_closed || !m_owner) return PowerStagingNotRequired{};
  if (!m_revalidating) {
    m_finalPreparationValidated = false;
    if (!m_stagingTask) {
      m_finalPreparationValidated = true;
      return PowerStagingNotRequired{};
    }
    m_stagingTask->revalidatePreparation();
    m_stagingResult.reset();
    m_revalidating = true;
  }
  auto prepared = pollStagingPreparation();
  if (prepared) {
    m_revalidating = false;
    m_finalPreparationValidated = !std::holds_alternative<PowerStagingFailure>(*prepared);
  }
  return prepared;
}

void PowerCoordinator::Operation::executeSafe(const PowerAction action) {
  PowerActionDialogMode failureMode = PowerActionDialogMode::Safe;

  try {
    PowerActionDialogRequest dialogRequest{action, [operation = shared_from_this()] { return operation->pollStagingPreparation(); }};
    dialogRequest.pollStagingRevalidation = [operation = shared_from_this()] { return operation->pollStagingRevalidation(); };

    const auto dialogOutcome = confirm(std::move(dialogRequest));
    if (!m_owner) return;
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
        return;
      case PowerActionDialogOutcome::ContinuedAfterStagingFailure:
        UWF_LOG_W("power") << "safe power action continuing after user accepted file staging failures";
        if (!m_stagingResult) throw std::logic_error("safe power action has no file staging failure to approve");
        if (m_stagingResult->outcome != FileStagingCommandOutcome::CompletedWithFailures || m_stagingResult->failedTargets == 0)
          throw std::logic_error("cannot approve a file staging failure without a recorded failed target");
        continuationApproved = true;
        break;
      case PowerActionDialogOutcome::Confirmed:
        break;
    }

    if (!m_stagingResult) throw std::logic_error("safe power action has no final file staging result");
    if (!m_finalPreparationValidated && m_stagedEntries && !m_stagedEntries->isEmpty() && !continuationApproved)
      throw std::logic_error("safe power action requires a final staging revalidation");
    if (m_stagingResult->outcome != app::FileStagingCommandOutcome::Succeeded && !continuationApproved) {
      throw std::logic_error("safe power action cannot continue without a completed staging result or explicit user approval");
    }
    invokePowerAction(action);
  } catch (const std::exception& error) {
    reportPowerFailure(action, error, failureMode);
  } catch (...) {
    reportUnknownPowerFailure(action, failureMode);
  }
}

void PowerCoordinator::Operation::invokePowerAction(const PowerAction action) {
  using ControlResult = PreshutdownControlResult;
  // Prepare power admission before arming the service skip token to minimize the
  // interval in which an interrupted client could suppress the service fallback.
  const auto invokePower = m_repository->preparePowerAction(action);
  if (!m_owner) return;

  const ControlResult skipResult = m_preshutdown.markHandled ? m_preshutdown.markHandled() : ControlResult::NotApplicable;
  if (skipResult == ControlResult::Unacknowledged) {
    UWF_LOG_W("power") << "enhanced mode preshutdown skip token was not acknowledged; service fallback remains authoritative";
  }
  auto revokeSkipOnFailure = qScopeGuard([&]() noexcept {
    try {
      if (skipResult != ControlResult::NotApplicable && m_preshutdown.markRequired && m_preshutdown.markRequired() == ControlResult::Unacknowledged)
        UWF_LOG_W("power") << "enhanced mode preshutdown skip token revocation was not acknowledged";
    } catch (const std::exception& error) {
      UWF_LOG_E("power") << "enhanced mode preshutdown skip token revocation failed: error=" << error.what();
    } catch (...) {
      UWF_LOG_E("power") << "enhanced mode preshutdown skip token revocation failed: error=non-standard-exception";
    }
  });
  if (!m_owner) return;
  invokePower();
  revokeSkipOnFailure.dismiss();
}

void PowerCoordinator::Operation::reportPowerFailure(const PowerAction action, const std::exception& error, const PowerActionDialogMode mode) {
  const bool shutdown = action == PowerAction::Shutdown;
  const bool direct = mode == PowerActionDialogMode::Direct;
  UWF_LOG_E("power") << (direct ? (shutdown ? "direct shutdown failed: error=" : "direct restart failed: error=")
                                : (shutdown ? "safe shutdown failed: error=" : "safe restart failed: error="))
                     << error.what();
  const QString title = direct ? (shutdown ? ApplicationText::tr("Direct shutdown failed") : ApplicationText::tr("Direct restart failed"))
                               : (shutdown ? ApplicationText::tr("Safe shutdown failed") : ApplicationText::tr("Safe restart failed"));
  const QString message = shutdown ? ApplicationText::tr("Shutdown failed: %1").arg(QString::fromUtf8(error.what()))
                                   : ApplicationText::tr("Restart failed: %1").arg(QString::fromUtf8(error.what()));
  if (m_owner) m_interaction.warning(title, message);
}

void PowerCoordinator::Operation::reportUnknownPowerFailure(const PowerAction action, const PowerActionDialogMode mode) {
  const bool shutdown = action == PowerAction::Shutdown;
  const bool direct = mode == PowerActionDialogMode::Direct;
  UWF_LOG_E("power") << (direct ? (shutdown ? "direct shutdown failed: error=non-standard-exception" : "direct restart failed: error=non-standard-exception")
                                : (shutdown ? "safe shutdown failed: error=non-standard-exception" : "safe restart failed: error=non-standard-exception"));
  if (m_owner)
    m_interaction.warning(direct ? (shutdown ? ApplicationText::tr("Direct shutdown failed") : ApplicationText::tr("Direct restart failed"))
                                 : (shutdown ? ApplicationText::tr("Safe shutdown failed") : ApplicationText::tr("Safe restart failed")),
                          ApplicationText::tr("The operation failed with an unknown error."));
}

}  // namespace uwf::app

#include "moc_PowerCoordinator.cpp"
