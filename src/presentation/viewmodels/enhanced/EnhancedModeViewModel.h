// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <cstdint>
#include <optional>

#include "application/enhanced/contracts/EnhancedModeActions.h"
#include "application/staging/contracts/AutomaticCommitDisposition.h"
#include "application/uwf/contracts/RuntimeWriteRestriction.h"

namespace uwf::presentation {
class EnhancedModeViewModel final : public QObject {
  Q_OBJECT
 public:
  enum class FeedbackTone {
    Neutral,
    Success,
    Warning,
    Error,
  };
  struct OperationFeedback {
    QString detail;
    FeedbackTone tone = FeedbackTone::Neutral;
  };
  struct StatusText {
    QString label, action, actionIcon, service, staging, stagingDescription, account, startup, agent, issues, issueHeading, icon, details;
    FeedbackTone statusTone = FeedbackTone::Neutral;
    FeedbackTone issueTone = FeedbackTone::Neutral;
    bool primaryAction = true;
    bool canApply = false;
    bool canRemove = false;
    bool showFacts = false;
    bool showFallbackCommitHint = false;
  };

  explicit EnhancedModeViewModel(app::EnhancedModeActions& actions);
  [[nodiscard]] bool busy() const { return m_manager.operationInProgress(); }
  [[nodiscard]] const std::optional<app::EnhancedModeStatus>& status() const { return m_status; }
  [[nodiscard]] const std::optional<OperationFeedback>& operationFeedback() const { return m_operationFeedback; }
  [[nodiscard]] StatusText statusText() const;
  void refreshStatus();
  void applyRequestedState();
  void removeService();
  void setWriteRestriction(app::RuntimeWriteRestriction restriction);
  void setAutomaticCommitSummary(app::AutomaticCommitSummary summary);
  void retranslate();
 signals:
  void statusChanged(const uwf::app::EnhancedModeStatus& status);
  void statusUnavailable(const QString& detail);
  void feedbackChanged();
  void operationStarted();

 private:
  void finishOperation(const app::EnhancedModeChangeResult& result, std::exception_ptr failure);
  bool m_removing = false;
  void updateStatus(const app::EnhancedModeStatus& status);
  void updateStatusUnavailable(const QString& detail);
  void publishStatus(std::optional<app::EnhancedModeStatus> status, QString failure, std::optional<OperationFeedback> feedback);
  void recoverAfterOperationFailure(const QString& detail);
  void clearOperationFeedback();
  app::EnhancedModeActions& m_manager;
  std::optional<app::EnhancedModeStatus> m_status;
  QString m_statusFailure;
  app::RuntimeWriteRestriction m_writeRestriction = app::RuntimeWriteRestriction::None;
  app::AutomaticCommitSummary m_automaticCommitSummary{.unavailable = true};
  std::optional<OperationFeedback> m_operationFeedback;
  std::uint64_t m_revision = 0;
};
}  // namespace uwf::presentation
