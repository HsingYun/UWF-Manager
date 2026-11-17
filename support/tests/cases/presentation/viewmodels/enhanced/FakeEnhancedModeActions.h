// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPointer>
#include <functional>
#include <stdexcept>

#include "application/enhanced/contracts/EnhancedModeActions.h"
#include "support/app/WorkerGate.h"

namespace uwf::test::viewmodels {
class FakeEnhancedModeActions final : public app::EnhancedModeActions {
 public:
  void setAgentState(app::EnhancedModeAgentState state) override {
    current.agentState = state;
    emit agentConnectionStateChanged(state);
  }

  app::EnhancedModeStatus current;
  bool failRead = false;
  bool failWrite = false;
  bool failPublish = false;
  QString persistenceWarning;
  QString lastDescription;
  int starts = 0;
  int enables = 0;
  int disables = 0;
  int removes = 0;
  std::shared_ptr<test::WorkerGate> gate;
  std::function<void()> onRead;
  std::function<void()> onPrepare;
  std::function<void()> onPublish;
  app::EnhancedModeStatus status() const override {
    const auto observed = current;
    const bool failed = failRead;
    const auto callback = onRead;
    if (callback) callback();
    if (failed) throw std::runtime_error("SCM unavailable");
    return observed;
  }
  ChangeWork prepareChange(const app::EnhancedModeAction action, const QString& description) override {
    if (action == app::EnhancedModeAction::Disable)
      ++disables;
    else if (action == app::EnhancedModeAction::Remove)
      ++removes;
    else if (action == app::EnhancedModeAction::Start)
      ++starts;
    else {
      ++enables;
      lastDescription = description;
    }
    auto next = current;
    if (action == app::EnhancedModeAction::Remove) {
      next = {};
    } else if (action == app::EnhancedModeAction::Disable) {
      next.state = app::EnhancedModeState::Disabled;
      next.automaticStart = false;
      next.startupDisabled = true;
      next.running = next.preshutdownAccepted = false;
    } else {
      next.state = app::EnhancedModeState::Enabled;
      next.serviceExists = next.serviceRegistryPresent = true;
      next.ownProcess = next.automaticStart = next.localSystemAccount = true;
      next.executableMatches = next.preshutdownTimeoutConfigured = next.requiredPrivilegesConfigured = true;
      next.running = next.preshutdownAccepted = true;
      next.startupDisabled = false;
    }
    ChangeWork work = [failed = failWrite, gate = gate, result = app::EnhancedModeChangeResult{next, persistenceWarning}] {
      if (gate) gate->wait();
      if (failed) throw std::runtime_error("write failed");
      return result;
    };
    const auto callback = onPrepare;
    if (callback) callback();
    return work;
  }
  void acceptChangeResult(app::EnhancedModeChangeResult& result) override {
    current = result.status;
    const bool failed = failPublish;
    const auto callback = onPublish;
    const QPointer guard(this);
    if (callback) callback();
    if (failed) throw std::runtime_error("agent startup failed");
    if (!guard) return;
    emit statusChanged(current);
  }
};
}  // namespace uwf::test::viewmodels
