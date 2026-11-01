// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <exception>
#include <functional>

#include "application/enhanced/contracts/EnhancedModeStatus.h"

namespace uwf::app {
class EnhancedModeActions : public QObject {
  Q_OBJECT
 public:
  using QObject::QObject;
  [[nodiscard]] virtual EnhancedModeStatus status() const = 0;
  virtual void setAgentState(EnhancedModeAgentState state) = 0;
  void requestChange(EnhancedModeAction action, const QString& serviceDescription = {});
  [[nodiscard]] bool operationInProgress() const { return m_operationInProgress; }
 signals:
  void statusChanged(uwf::app::EnhancedModeStatus status);
  void agentConnectionStateChanged(uwf::app::EnhancedModeAgentState state);
  void operationStarted();
  void operationFinished(const uwf::app::EnhancedModeChangeResult& result, std::exception_ptr failure);

 protected:
  using ChangeWork = std::function<EnhancedModeChangeResult()>;
  // Only requestChange admits work. Prepared work owns its inputs and must not
  // capture the QObject receiver.
  [[nodiscard]] virtual ChangeWork prepareChange(EnhancedModeAction action, const QString& serviceDescription) = 0;
  virtual void acceptChangeResult(EnhancedModeChangeResult& result) { emit statusChanged(result.status); }

 private:
  bool m_operationInProgress = false;
};
}  // namespace uwf::app
