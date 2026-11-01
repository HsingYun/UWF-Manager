// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/enhanced/EnhancedModeViewModel.h"

#include <QPointer>
#include <QStringList>
#include <array>
#include <exception>
#include <utility>

#include "presentation/viewmodels/common/AutomaticCommitText.h"
#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/common/RuntimeWriteText.h"

namespace uwf::presentation {
namespace {
using Tone = EnhancedModeViewModel::FeedbackTone;
QStringList statusIssues(const app::EnhancedModeStatus& status) {
  QStringList issues;
  const bool intentionallyDisabled = status.state == app::EnhancedModeState::Disabled && status.startupDisabled && !status.running;
  if (status.serviceExists) {
    struct ContractRule {
      bool app::EnhancedModeStatus::* satisfied;
      const char* message;
    };
    constexpr std::array rules{
        ContractRule{&app::EnhancedModeStatus::ownProcess, QT_TRANSLATE_NOOP("uwf", "The service is not configured as an independent process.")},
        ContractRule{&app::EnhancedModeStatus::automaticStart, QT_TRANSLATE_NOOP("uwf", "The service is not configured for automatic startup.")},
        ContractRule{&app::EnhancedModeStatus::localSystemAccount, QT_TRANSLATE_NOOP("uwf", "The service is not configured to run as LocalSystem.")},
        ContractRule{&app::EnhancedModeStatus::executableMatches,
                     QT_TRANSLATE_NOOP("uwf", "The service executable path or startup arguments do not match this executable.")},
        ContractRule{&app::EnhancedModeStatus::preshutdownTimeoutConfigured,
                     QT_TRANSLATE_NOOP("uwf", "The service preshutdown timeout is not configured correctly.")},
        ContractRule{&app::EnhancedModeStatus::requiredPrivilegesConfigured,
                     QT_TRANSLATE_NOOP("uwf", "The service does not declare the privileges required to start the UI agent.")},
    };
    for (const auto& rule : rules) {
      if (intentionallyDisabled && rule.satisfied == &app::EnhancedModeStatus::automaticStart) continue;
      if (!(status.*(rule.satisfied))) issues.append(PresentationText::tr(rule.message));
    }
    if (!status.running && !intentionallyDisabled) {
      issues.append(status.state == app::EnhancedModeState::Stopped
                        ? PresentationText::tr("The service is installed but stopped. Start it to resume automatic file commits.")
                        : PresentationText::tr("The service is not running."));
    } else if (status.running && !status.preshutdownAccepted) {
      issues.append(PresentationText::tr("The service cannot receive Windows preshutdown notifications."));
    }
  } else if (status.serviceRegistryPresent) {
    issues.append(PresentationText::tr("The service is absent, but its service registry data remains."));
  }

  if (status.running) {
    switch (status.agentState) {
      case app::EnhancedModeAgentState::Unobserved:
        break;
      case app::EnhancedModeAgentState::Connecting:
        issues.append(PresentationText::tr("Waiting for the UI agent to complete authentication."));
        break;
      case app::EnhancedModeAgentState::Disconnected:
        issues.append(PresentationText::tr("The service is running, but no authenticated UI agent is connected."));
        break;
      case app::EnhancedModeAgentState::Connected:
        break;
    }
  }
  if (!status.detail.isEmpty()) issues.append(status.detail);
  return issues;
}

Tone issueTone(const app::EnhancedModeStatus& status) {
  switch (status.state) {
    case app::EnhancedModeState::RepairRequired:
      return Tone::Warning;
    case app::EnhancedModeState::Stopped:
      return Tone::Warning;
    case app::EnhancedModeState::Disabled:
      return status.detail.isEmpty() ? Tone::Neutral : Tone::Error;
    case app::EnhancedModeState::Enabled:
      break;
  }
  if (!status.detail.isEmpty() || status.agentState == app::EnhancedModeAgentState::Disconnected) return Tone::Error;
  return status.agentState == app::EnhancedModeAgentState::Connecting ? Tone::Warning : Tone::Neutral;
}

}  // namespace

EnhancedModeViewModel::EnhancedModeViewModel(app::EnhancedModeActions& actions) : m_manager(actions) {
  connect(&m_manager, &app::EnhancedModeActions::operationStarted, this, [this] {
    const QPointer self(this);
    clearOperationFeedback();
    if (self) emit operationStarted();
  });
  connect(&m_manager, &app::EnhancedModeActions::operationFinished, this, &EnhancedModeViewModel::finishOperation);
  connect(&m_manager, &app::EnhancedModeActions::agentConnectionStateChanged, this, [this](const app::EnhancedModeAgentState state) {
    if (!m_status) {
      refreshStatus();
      return;
    }
    auto status = *m_status;
    status.agentState = state;
    updateStatus(status);
  });
}
EnhancedModeViewModel::StatusText EnhancedModeViewModel::statusText() const {
  StatusText result;
  const auto automatic = automaticCommitPresentation(m_automaticCommitSummary);
  result.canApply = !busy();
  result.stagingDescription = automatic.description;
  if (!m_status) {
    result.label = PresentationText::tr("Status unavailable");
    result.action = PresentationText::tr("Retry");
    result.actionIcon = QStringLiteral(":/icons/refresh.svg");
    result.statusTone = result.issueTone = Tone::Error;
    result.icon = QStringLiteral(":/icons/status_error.svg");
    result.service = result.staging = PresentationText::tr("Unavailable");
    result.issueHeading = PresentationText::tr("Unable to read service status");
    result.issues = PresentationText::tr("The enhanced mode service status could not be read. Check service permissions and retry.");
    result.details = m_statusFailure;
    return result;
  }
  const auto& status = *m_status;
  result.showFacts = true;
  result.showFallbackCommitHint =
      m_writeRestriction == app::RuntimeWriteRestriction::None && !automatic.unavailable && !automatic.paused && !automatic.alreadyPersistent;
  switch (status.state) {
    case app::EnhancedModeState::Disabled:
      result.label = PresentationText::tr("Disabled");
      break;
    case app::EnhancedModeState::Enabled:
      result.label = PresentationText::tr("Enabled");
      result.statusTone = Tone::Success;
      result.icon = QStringLiteral(":/icons/check.svg");
      break;
    case app::EnhancedModeState::Stopped:
      result.label = PresentationText::tr("Service stopped");
      result.statusTone = Tone::Warning;
      break;
    case app::EnhancedModeState::RepairRequired:
      result.label = PresentationText::tr("Repair required");
      result.statusTone = Tone::Error;
      result.icon = QStringLiteral(":/icons/status_error.svg");
      result.issueHeading = PresentationText::tr("Service configuration needs repair");
      break;
  }
  switch (status.availableAction()) {
    case app::EnhancedModeAction::Enable:
      result.action = PresentationText::tr("Enable enhanced mode");
      break;
    case app::EnhancedModeAction::Disable:
      result.action = PresentationText::tr("Disable enhanced mode");
      result.primaryAction = false;
      break;
    case app::EnhancedModeAction::Start:
      result.action = PresentationText::tr("Start service");
      break;
    case app::EnhancedModeAction::Repair:
      result.action = PresentationText::tr("Repair enhanced mode");
      break;
    case app::EnhancedModeAction::Remove:
      result.action = PresentationText::tr("Delete service");
      result.primaryAction = false;
      break;
  }
  if (status.state != app::EnhancedModeState::Enabled) result.actionIcon = QStringLiteral(":/icons/enhanced.svg");
  result.service = status.state == app::EnhancedModeState::RepairRequired ? PresentationText::tr("Configuration error")
                   : status.running                                       ? PresentationText::tr("Running")
                   : status.serviceExists                                 ? PresentationText::tr("Stopped")
                                                                          : PresentationText::tr("Not installed");
  result.staging = status.serviceContractSatisfied() ? automatic.status : PresentationText::tr("Unavailable");
  result.stagingDescription = automatic.description;
  result.account = status.localSystemAccount ? QStringLiteral("LocalSystem") : PresentationText::tr("Not configured");
  result.startup = status.automaticStart    ? PresentationText::tr("Automatic")
                   : status.startupDisabled ? PresentationText::tr("Disabled", "service startup type")
                                            : PresentationText::tr("Not configured");
  result.agent = status.agentState == app::EnhancedModeAgentState::Connected    ? PresentationText::tr("Connected and authenticated")
                 : status.agentState == app::EnhancedModeAgentState::Connecting ? PresentationText::tr("Connecting…")
                                                                                : PresentationText::tr("Not connected");
  result.issues = statusIssues(status).join(u'\n');
  result.issueTone = issueTone(status);
  result.canRemove = !busy() && status.hasRemnants();
  if (m_writeRestriction != app::RuntimeWriteRestriction::None) {
    result.canApply = result.canRemove = false;
    if (m_writeRestriction == app::RuntimeWriteRestriction::ReadOnlyMediaActive) {
      result.issueHeading = PresentationText::tr("Read-only");
      result.issues = PresentationText::tr("File and registry commits are unavailable while Read-Only Media mode is active.");
    } else {
      result.issueHeading = PresentationText::tr("Automatic saving is unavailable in the current mode");
      result.issues = runtimeWriteRestrictionText(m_writeRestriction);
    }
    result.issueTone = Tone::Neutral;
  }
  return result;
}
void EnhancedModeViewModel::setWriteRestriction(const app::RuntimeWriteRestriction restriction) {
  if (m_writeRestriction == restriction) return;
  m_writeRestriction = restriction;
  ++m_revision;
  emit feedbackChanged();
}
void EnhancedModeViewModel::setAutomaticCommitSummary(const app::AutomaticCommitSummary summary) {
  if (m_automaticCommitSummary == summary) return;
  m_automaticCommitSummary = summary;
  ++m_revision;
  emit feedbackChanged();
}
void EnhancedModeViewModel::retranslate() { emit feedbackChanged(); }
void EnhancedModeViewModel::updateStatus(const app::EnhancedModeStatus& status) { publishStatus(status, {}, m_operationFeedback); }
void EnhancedModeViewModel::updateStatusUnavailable(const QString& detail) { publishStatus({}, detail, {}); }
void EnhancedModeViewModel::publishStatus(std::optional<app::EnhancedModeStatus> status, QString failure, std::optional<OperationFeedback> feedback) {
  // Every notification sees the complete presentation state. Keep signal
  // arguments owned across callbacks that may refresh or release the receiver.
  const auto revision = ++m_revision;
  m_status = status;
  m_statusFailure = failure;
  m_operationFeedback = std::move(feedback);
  const QPointer self(this);
  if (status)
    emit statusChanged(*status);
  else
    emit statusUnavailable(failure);
  if (self && m_revision == revision) emit feedbackChanged();
}

void EnhancedModeViewModel::refreshStatus() {
  const auto revision = ++m_revision;
  const QPointer self(this);
  try {
    const auto status = m_manager.status();
    if (self && m_revision == revision) updateStatus(status);
  } catch (const std::exception& error) {
    if (self && m_revision == revision) updateStatusUnavailable(QString::fromUtf8(error.what()));
  } catch (...) {
    if (self && m_revision == revision) updateStatusUnavailable(PresentationText::tr("The enhanced mode status read failed because of an unknown error."));
  }
}

void EnhancedModeViewModel::clearOperationFeedback() {
  ++m_revision;
  m_operationFeedback.reset();
  emit feedbackChanged();
}

void EnhancedModeViewModel::recoverAfterOperationFailure(const QString& detail) {
  const auto revision = ++m_revision;
  const QPointer self(this);
  try {
    const auto status = m_manager.status();
    if (self && m_revision == revision) publishStatus(status, {}, OperationFeedback{detail, FeedbackTone::Error});
  } catch (const std::exception& error) {
    if (self && m_revision == revision)
      updateStatusUnavailable(detail + QLatin1Char('\n') +
                              PresentationText::tr("The current service state could not be read:\n%1").arg(QString::fromUtf8(error.what())));
  } catch (...) {
    if (self && m_revision == revision)
      updateStatusUnavailable(detail + QLatin1Char('\n') + PresentationText::tr("The current service state could not be read because of an unknown error."));
  }
}

void EnhancedModeViewModel::applyRequestedState() {
  if (busy()) return;
  if (!m_status) {
    refreshStatus();
    return;
  }
  if (m_writeRestriction != app::RuntimeWriteRestriction::None) return;
  m_removing = false;
  m_manager.requestChange(m_status->availableAction(), PresentationText::tr("UWF Manager enhanced mode helper service"));
}
void EnhancedModeViewModel::removeService() {
  if (busy() || m_writeRestriction != app::RuntimeWriteRestriction::None || !m_status || !m_status->hasRemnants()) return;
  m_removing = true;
  m_manager.requestChange(app::EnhancedModeAction::Remove);
}
void EnhancedModeViewModel::finishOperation(const app::EnhancedModeChangeResult& result, const std::exception_ptr failure) {
  if (failure) {
    try {
      std::rethrow_exception(failure);
    } catch (const std::exception& error) {
      recoverAfterOperationFailure((m_removing ? PresentationText::tr("The enhanced mode service could not be deleted:\n%1")
                                               : PresentationText::tr("The enhanced mode configuration failed:\n%1"))
                                       .arg(QString::fromUtf8(error.what())));
    } catch (...) {
      recoverAfterOperationFailure(m_removing ? PresentationText::tr("The enhanced mode service could not be deleted because of an unknown error.")
                                              : PresentationText::tr("The enhanced mode configuration failed because of an unknown error."));
    }
    return;
  }
  const QString message =
      m_removing ? (result.persistenceWarning.isEmpty()
                        ? PresentationText::tr("The enhanced mode service and its remaining configuration were deleted.")
                        : PresentationText::tr("The service deletion was requested, but residual cleanup reported:\n%1").arg(result.persistenceWarning))
                 : (result.persistenceWarning.isEmpty()
                        ? PresentationText::tr("The enhanced mode configuration was applied.")
                        : PresentationText::tr("The service configuration was applied, but UWF persistence reported:\n%1").arg(result.persistenceWarning));
  publishStatus(result.status, {}, OperationFeedback{message, result.persistenceWarning.isEmpty() ? FeedbackTone::Success : FeedbackTone::Warning});
}
}  // namespace uwf::presentation

#include "moc_EnhancedModeViewModel.cpp"
