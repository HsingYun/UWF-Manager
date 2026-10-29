// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <stdexcept>
#include <utility>

#include "app/enhanced/EnhancedModeSession.h"
#include "support/app/EnhancedModeFixture.h"

namespace {
using namespace uwf::app;

using namespace uwf::test::application;

class MemoryActions final : public EnhancedModeActions {
 public:
  EnhancedModeStatus current;
  bool readFails = false;
  mutable int reads = 0;
  EnhancedModeStatus status() const override {
    ++reads;
    if (readFails) throw std::runtime_error("SCM unavailable");
    return current;
  }
  void setAgentState(const EnhancedModeAgentState state) override {
    if (current.agentState == state) return;
    current.agentState = state;
    emit agentConnectionStateChanged(state);
  }
  EnhancedModeChangeResult enable(const QString&) override { return publish(completeEnhancedModeStatus()); }
  EnhancedModeChangeResult start() override { return publish(completeEnhancedModeStatus()); }
  EnhancedModeChangeResult disable() override { return publish({}); }
  ChangeWork prepareChange(const EnhancedModeAction action, const QString&) override {
    return [action] { return EnhancedModeChangeResult{action == EnhancedModeAction::Disable ? EnhancedModeStatus{} : completeEnhancedModeStatus(), {}}; };
  }
  EnhancedModeChangeResult publish(EnhancedModeStatus status) {
    current = std::move(status);
    emit statusChanged(current);
    return {current, {}};
  }
};

class EnhancedModeSessionTests final : public QObject {
  Q_OBJECT
 private slots:
  void completeServiceContractControlsTheConnection_data() {
    QTest::addColumn<EnhancedModeStatus>("status");
    QTest::newRow("enabled") << completeEnhancedModeStatus();
    QTest::newRow("disabled") << EnhancedModeStatus{};
    auto stopped = completeEnhancedModeStatus();
    stopped.running = false;
    stopped.state = EnhancedModeState::Stopped;
    QTest::newRow("stopped") << stopped;
    auto missingPrivilege = completeEnhancedModeStatus();
    missingPrivilege.requiredPrivilegesConfigured = false;
    QTest::newRow("incomplete-contract") << missingPrivilege;
    auto wrongExecutable = completeEnhancedModeStatus();
    wrongExecutable.executableMatches = false;
    QTest::newRow("wrong-executable") << wrongExecutable;
  }
  void completeServiceContractControlsTheConnection() {
    QFETCH(EnhancedModeStatus, status);
    auto actions = std::make_unique<MemoryActions>();
    auto* source = actions.get();
    source->current = status;
    MemoryEnhancedModeAgent agent;
    EnhancedModeSession session(std::move(actions), agent);
    QSignalSpy changed(&session, &EnhancedModeSession::statusChanged);
    session.refresh();
    QCOMPARE(agent.running(), status.serviceContractSatisfied());
    QCOMPARE(agent.starts, status.serviceContractSatisfied() ? 1 : 0);
    QCOMPARE(changed.size(), 1);
    const auto observed = qvariant_cast<EnhancedModeStatus>(changed.back().front());
    QCOMPARE(observed.agentState, status.serviceContractSatisfied() ? EnhancedModeAgentState::Connecting : EnhancedModeAgentState::Unobserved);
    session.refresh();
    QCOMPARE(agent.starts, status.serviceContractSatisfied() ? 1 : 0);
    source->disable();
    QVERIFY(!agent.running());
  }
  void synchronousAuthenticationIsIncludedInThePublishedStatus() {
    auto actions = std::make_unique<MemoryActions>();
    auto* source = actions.get();
    MemoryEnhancedModeAgent agent;
    agent.connectOnStart = true;
    EnhancedModeSession session(std::move(actions), agent);
    QSignalSpy changed(&session, &EnhancedModeSession::statusChanged);
    const auto enabled = source->enable({});
    QCOMPARE(enabled.status.agentState, EnhancedModeAgentState::Connected);
    QCOMPARE(qvariant_cast<EnhancedModeStatus>(changed.back().front()).agentState, EnhancedModeAgentState::Connected);
    QCOMPARE(source->reads, 0);
  }
  void startupFailureAndDisconnectRequestARecheckWithoutCommitting() {
    auto actions = std::make_unique<MemoryActions>();
    auto* source = actions.get();
    source->current = completeEnhancedModeStatus();
    MemoryEnhancedModeAgent agent;
    agent.canStart = false;
    EnhancedModeSession session(std::move(actions), agent);
    session.refresh();
    QCOMPARE(source->current.agentState, EnhancedModeAgentState::Disconnected);
    agent.canStart = true;
    session.refresh();
    QVERIFY(agent.running());
    QSignalSpy requested(&session, &EnhancedModeSession::refreshRequested);
    agent.publishConnectionState(true);
    QCoreApplication::processEvents();
    QCOMPARE(requested.size(), 0);
    agent.publishConnectionState(false);
    QCOMPARE(requested.size(), 0);
    QTRY_COMPARE(requested.size(), 1);
    QCOMPARE(source->reads, 2);
    QVERIFY(agent.preshutdownCommitStates.empty());
  }
  void queryFailureRetainsTheConfirmedConnectionAndHandshake() {
    auto actions = std::make_unique<MemoryActions>();
    auto* source = actions.get();
    source->current = completeEnhancedModeStatus();
    MemoryEnhancedModeAgent agent;
    EnhancedModeSession session(std::move(actions), agent);
    const auto control = session.preshutdownControl();
    QCOMPARE(control.markHandled(), PreshutdownControlResult::NotApplicable);
    session.refresh();
    source->readFails = true;
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, session.refresh());
    QVERIFY(agent.running());
    QCOMPARE(control.markHandled(), PreshutdownControlResult::Acknowledged);
    agent.acknowledge = false;
    QCOMPARE(control.markRequired(), PreshutdownControlResult::Unacknowledged);
    QCOMPARE(agent.preshutdownCommitStates, std::vector<bool>({true, false}));
    source->readFails = false;
    source->disable();
    QCOMPARE(control.markRequired(), PreshutdownControlResult::NotApplicable);
    QCOMPARE(agent.preshutdownCommitStates, std::vector<bool>({true, false}));
  }
  void destructionStopsBorrowedAndDestroysOwnedConnections() {
    MemoryEnhancedModeAgent borrowed;
    {
      auto actions = std::make_unique<MemoryActions>();
      actions->current = completeEnhancedModeStatus();
      EnhancedModeSession session(std::move(actions), borrowed);
      session.refresh();
      QVERIFY(borrowed.running());
    }
    QVERIFY(!borrowed.running());
    QCOMPARE(borrowed.stops, 1);
    auto owned = std::make_unique<MemoryEnhancedModeAgent>();
    QPointer<MemoryEnhancedModeAgent> lifetime(owned.get());
    {
      EnhancedModeSession session(std::make_unique<MemoryActions>(), std::move(owned));
      QVERIFY(lifetime);
    }
    QVERIFY(!lifetime);
  }
  void invalidDependenciesFailBeforeConnecting() {
    MemoryEnhancedModeAgent agent;
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, EnhancedModeSession({}, agent));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, EnhancedModeSession(std::make_unique<MemoryActions>(), std::unique_ptr<EnhancedModeAgentConnection>{}));
  }
};
}  // namespace

QTEST_GUILESS_MAIN(EnhancedModeSessionTests)
#include "EnhancedModeSessionTests.moc"
