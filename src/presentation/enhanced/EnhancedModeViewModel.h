// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QObject>
#include <memory>
#include <optional>

#include "app/api/EnhancedModeActions.h"

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
    QString label, action, service, staging, account, startup, agent, issues, issueHeading, icon;
    FeedbackTone statusTone = FeedbackTone::Neutral;
    FeedbackTone issueTone = FeedbackTone::Neutral;
    bool primaryAction = true;
    bool canRemove = false;
  };

  explicit EnhancedModeViewModel(app::EnhancedModeActions& actions);
  [[nodiscard]] bool busy() const { return m_manager.operationInProgress(); }
  [[nodiscard]] const std::optional<app::EnhancedModeStatus>& status() const { return m_status; }
  [[nodiscard]] app::EnhancedModeAction action() const { return m_action; }
  [[nodiscard]] const std::optional<OperationFeedback>& operationFeedback() const { return m_operationFeedback; }
  [[nodiscard]] StatusText statusText() const;
  void refreshStatus();
  void applyRequestedState();
  void removeService();
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
  void recoverAfterOperationFailure(const QString& detail);
  void setOperationFeedback(QString detail, FeedbackTone tone);
  void clearOperationFeedback();
  app::EnhancedModeActions& m_manager;
  app::EnhancedModeAction m_action = app::EnhancedModeAction::Enable;
  std::optional<app::EnhancedModeStatus> m_status;
  std::optional<OperationFeedback> m_operationFeedback;
};
}  // namespace uwf::presentation
