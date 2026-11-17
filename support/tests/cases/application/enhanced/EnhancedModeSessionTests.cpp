// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "application/enhanced/EnhancedModeSession.h"
#include "simulation/app/EnhancedMode.h"

namespace {
using namespace uwf::app;

class MemoryActions final : public EnhancedModeActions {
 public:
  EnhancedModeStatus current;
  bool readFails = false;
  std::function<void()> beforeRead;
  mutable int reads = 0;
  EnhancedModeStatus status() const override {
    ++reads;
    const auto result = current;
    const auto read = beforeRead;
    if (read) read();
    if (readFails) throw std::runtime_error("SCM unavailable");
    return result;
  }
  void setAgentState(const EnhancedModeAgentState state) override {
    if (current.agentState == state) return;
    current.agentState = state;
    emit agentConnectionStateChanged(state);
  }
  ChangeWork prepareChange(const EnhancedModeAction action, const QString&) override {
    auto next = current;
    switch (action) {
      case EnhancedModeAction::Disable:
        next.running = next.preshutdownAccepted = next.automaticStart = false;
        next.startupDisabled = true;
        next.state = next.serviceInstallationSatisfied() ? EnhancedModeState::Disabled : EnhancedModeState::RepairRequired;
        break;
      case EnhancedModeAction::Remove:
        next = {};
        break;
      case EnhancedModeAction::Enable:
      case EnhancedModeAction::Start:
      case EnhancedModeAction::Repair:
        next = uwf::simulation::application::completeEnhancedModeStatus();
        break;
    }
    return [next] { return EnhancedModeChangeResult{next, {}}; };
  }
  void acceptChangeResult(EnhancedModeChangeResult& result) override {
    result.status.agentState = current.agentState;
    current = result.status;
    const QPointer guard(this);
    emit statusChanged(result.status);
    if (guard) result.status = current;
  }
};

class CallbackAgent final : public EnhancedModeAgentConnection {
 public:
  std::function<void(std::string_view)> callback;
  bool connectOnStart = false;
  bool active = false;
  void notify(const std::string_view boundary) {
    const auto invoke = callback;
    if (invoke) invoke(boundary);
  }
  void start() override {
    active = true;
    const bool authenticate = connectOnStart;
    notify("start");
    if (authenticate) emit connectionStateChanged(true);
  }
  void stop() override {
    if (!active) return;
    active = false;
    notify("stop");
    emit connectionStateChanged(false);
  }
  bool running() const override { return active; }
  bool markPreshutdownCommitHandled() override {
    notify("mark-handled");
    return true;
  }
  bool requirePreshutdownCommit() override {
    notify("mark-required");
    return true;
  }
};

class EnhancedModeSessionTests final : public QObject {
  Q_OBJECT
 private slots:
  void sessionBoundariesMayReleaseTheirOwner_data() {
    QTest::addColumn<QString>("boundary");
    QTest::addColumn<bool>("owned");
    for (const auto* boundary : {"read", "connecting", "start", "connection", "stop", "unobserved", "mark-handled", "mark-required"})
      for (const bool owned : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2").arg(QString::fromLatin1(boundary), owned ? QStringLiteral("owned") : QStringLiteral("borrowed"))))
            << QString::fromLatin1(boundary) << owned;
  }
  void sessionBoundariesMayReleaseTheirOwner() {
    QFETCH(QString, boundary);
    QFETCH(bool, owned);
    auto actions = std::make_unique<MemoryActions>();
    auto* source = actions.get();
    source->current = uwf::simulation::application::completeEnhancedModeStatus();
    auto agentOwner = std::make_unique<CallbackAgent>();
    QPointer agent(agentOwner.get());
    agent->connectOnStart = boundary == QStringLiteral("connection");
    std::unique_ptr<EnhancedModeSession> session;
    if (owned)
      session = std::make_unique<EnhancedModeSession>(std::move(actions), std::move(agentOwner));
    else
      session = std::make_unique<EnhancedModeSession>(std::move(actions), *agentOwner);
    const auto controls = session->preshutdownControl();
    const bool initialized = boundary == QStringLiteral("stop") || boundary == QStringLiteral("unobserved") || boundary.startsWith(QStringLiteral("mark-"));
    if (initialized) session->refresh();
    if (boundary == QStringLiteral("stop") || boundary == QStringLiteral("unobserved")) {
      const auto agentState = source->current.agentState;
      source->current = {};
      source->current.agentState = agentState;
    }
    const auto release = [&] { session.reset(); };
    if (boundary == QStringLiteral("read")) source->beforeRead = release;
    agent->callback = [&](const std::string_view observed) {
      if (observed == boundary.toStdString()) release();
    };
    connect(session.get(), &EnhancedModeSession::agentConnectionStateChanged, this, [&](const EnhancedModeAgentState state) {
      if ((boundary == QStringLiteral("connecting") && state == EnhancedModeAgentState::Connecting) ||
          (boundary == QStringLiteral("connection") && state == EnhancedModeAgentState::Connected) ||
          (boundary == QStringLiteral("unobserved") && state == EnhancedModeAgentState::Unobserved))
        release();
    });
    if (boundary == QStringLiteral("mark-handled"))
      QCOMPARE(controls.markHandled(), PreshutdownControlResult::Acknowledged);
    else if (boundary == QStringLiteral("mark-required"))
      QCOMPARE(controls.markRequired(), PreshutdownControlResult::Acknowledged);
    else
      session->refresh();
    QVERIFY(!session);
    QCOMPARE(controls.markHandled(), PreshutdownControlResult::NotApplicable);
    QCOMPARE(controls.markRequired(), PreshutdownControlResult::NotApplicable);
    QCOMPARE(static_cast<bool>(agent), !owned);
    if (agent) QVERIFY(!agent->active);
  }
  void reentrantObservationsSupersedeOlderWork_data() {
    QTest::addColumn<QString>("boundary");
    for (const auto* boundary : {"read", "connecting", "start", "stop"}) QTest::newRow(boundary) << QString::fromLatin1(boundary);
  }
  void reentrantObservationsSupersedeOlderWork() {
    QFETCH(QString, boundary);
    auto actions = std::make_unique<MemoryActions>();
    auto* source = actions.get();
    source->current = uwf::simulation::application::completeEnhancedModeStatus();
    CallbackAgent agent;
    EnhancedModeSession session(std::move(actions), agent);
    if (boundary == QStringLiteral("stop")) {
      session.refresh();
      source->current = {};
    }
    bool reentered = false;
    const bool expectedRunning = boundary == QStringLiteral("stop");
    const auto replace = [&] {
      if (std::exchange(reentered, true)) return;
      source->current = expectedRunning ? uwf::simulation::application::completeEnhancedModeStatus() : EnhancedModeStatus{};
      session.refresh();
    };
    if (boundary == QStringLiteral("read")) source->beforeRead = replace;
    agent.callback = [&](const std::string_view observed) {
      if (observed == boundary.toStdString()) replace();
    };
    connect(&session, &EnhancedModeSession::agentConnectionStateChanged, this, [&](const EnhancedModeAgentState state) {
      if (boundary == QStringLiteral("connecting") && state == EnhancedModeAgentState::Connecting) replace();
    });
    QSignalSpy published(&session, &EnhancedModeSession::statusChanged);
    session.refresh();
    QVERIFY(reentered);
    QCOMPARE(agent.running(), expectedRunning);
    QVERIFY(!published.isEmpty());
    const auto last = qvariant_cast<EnhancedModeStatus>(published.back().front());
    QCOMPARE(last.serviceContractSatisfied(), expectedRunning);
    QCOMPARE(last.agentState, expectedRunning ? EnhancedModeAgentState::Connecting : EnhancedModeAgentState::Unobserved);
    agent.callback = {};
    source->beforeRead = {};
  }

  void completeServiceContractControlsTheConnection_data() {
    QTest::addColumn<EnhancedModeStatus>("status");
    QTest::addColumn<bool>("alreadyRunning");
    const auto row = [](const char* name, const EnhancedModeStatus& status) {
      for (const bool alreadyRunning : {false, true})
        QTest::newRow(qUtf8Printable(
            QStringLiteral("%1/%2").arg(QString::fromLatin1(name), alreadyRunning ? QStringLiteral("active-agent") : QStringLiteral("inactive-agent"))))
            << status << alreadyRunning;
    };
    row("enabled", uwf::simulation::application::completeEnhancedModeStatus());
    row("not-installed", EnhancedModeStatus{});
    auto disabled = uwf::simulation::application::completeEnhancedModeStatus();
    disabled.state = EnhancedModeState::Disabled;
    disabled.running = disabled.preshutdownAccepted = disabled.automaticStart = false;
    disabled.startupDisabled = true;
    row("installed-disabled", disabled);
    auto stopped = uwf::simulation::application::completeEnhancedModeStatus();
    stopped.running = false;
    stopped.state = EnhancedModeState::Stopped;
    row("stopped", stopped);
    auto missingPrivilege = uwf::simulation::application::completeEnhancedModeStatus();
    missingPrivilege.requiredPrivilegesConfigured = false;
    row("incomplete-contract", missingPrivilege);
    auto wrongExecutable = uwf::simulation::application::completeEnhancedModeStatus();
    wrongExecutable.executableMatches = false;
    row("wrong-executable", wrongExecutable);
  }
  void completeServiceContractControlsTheConnection() {
    QFETCH(EnhancedModeStatus, status);
    QFETCH(bool, alreadyRunning);
    auto actions = std::make_unique<MemoryActions>();
    auto* source = actions.get();
    source->current = status;
    uwf::simulation::application::MemoryEnhancedModeAgent agent;
    if (alreadyRunning) agent.start();
    EnhancedModeSession session(std::move(actions), agent);
    QSignalSpy changed(&session, &EnhancedModeSession::statusChanged);
    session.refresh();
    QCOMPARE(agent.running(), status.serviceContractSatisfied());
    const int expectedStarts = alreadyRunning || status.serviceContractSatisfied() ? 1 : 0;
    QCOMPARE(agent.starts, expectedStarts);
    QCOMPARE(agent.stops, alreadyRunning && !status.serviceContractSatisfied() ? 1 : 0);
    QCOMPARE(changed.size(), 1);
    const auto observed = qvariant_cast<EnhancedModeStatus>(changed.back().front());
    const auto expectedAgentState =
        status.serviceContractSatisfied() ? (alreadyRunning ? status.agentState : EnhancedModeAgentState::Connecting) : EnhancedModeAgentState::Unobserved;
    QCOMPARE(observed.agentState, expectedAgentState);
    session.refresh();
    QCOMPARE(agent.starts, expectedStarts);
    source->requestChange(status.serviceExists ? EnhancedModeAction::Disable : EnhancedModeAction::Remove);
    QTRY_VERIFY(!source->operationInProgress());
    QVERIFY(!agent.running());
    QCOMPARE(session.preshutdownControl().markRequired(), PreshutdownControlResult::NotApplicable);
    QVERIFY(agent.preshutdownCommitStates.empty());
  }
  void authenticationDuringStatusPublicationReachesTheCompletionResult() {
    auto actions = std::make_unique<MemoryActions>();
    auto* source = actions.get();
    uwf::simulation::application::MemoryEnhancedModeAgent agent;
    agent.connectOnStart = true;
    EnhancedModeSession session(std::move(actions), agent);
    QSignalSpy changed(&session, &EnhancedModeSession::statusChanged);
    EnhancedModeChangeResult enabled;
    bool completed = false;
    QObject subscriptions;
    connect(source, &EnhancedModeActions::operationFinished, &subscriptions, [&](const auto& result, const auto failure) {
      QVERIFY(!failure);
      enabled = result;
      completed = true;
    });
    source->requestChange(EnhancedModeAction::Enable);
    QTRY_VERIFY(completed);
    QCOMPARE(enabled.status.agentState, EnhancedModeAgentState::Connected);
    QCOMPARE(qvariant_cast<EnhancedModeStatus>(changed.back().front()).agentState, EnhancedModeAgentState::Connected);
    QCOMPARE(source->reads, 0);
  }
  void startupFailureAndDisconnectRequestARecheckWithoutCommitting() {
    auto actions = std::make_unique<MemoryActions>();
    auto* source = actions.get();
    source->current = uwf::simulation::application::completeEnhancedModeStatus();
    uwf::simulation::application::MemoryEnhancedModeAgent agent;
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
  void queryFailureRetainsTheConfirmedConnectionAndHandshake_data() {
    QTest::addColumn<bool>("remove");
    QTest::newRow("disable") << false;
    QTest::newRow("remove") << true;
  }
  void queryFailureRetainsTheConfirmedConnectionAndHandshake() {
    QFETCH(bool, remove);
    auto actions = std::make_unique<MemoryActions>();
    auto* source = actions.get();
    source->current = uwf::simulation::application::completeEnhancedModeStatus();
    uwf::simulation::application::MemoryEnhancedModeAgent agent;
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
    source->requestChange(remove ? EnhancedModeAction::Remove : EnhancedModeAction::Disable);
    QTRY_VERIFY(!source->operationInProgress());
    QVERIFY(!agent.running());
    QCOMPARE(control.markRequired(), PreshutdownControlResult::NotApplicable);
    QCOMPARE(agent.preshutdownCommitStates, std::vector<bool>({true, false}));
  }
  void destructionStopsBorrowedAndDestroysOwnedConnections() {
    uwf::simulation::application::MemoryEnhancedModeAgent borrowed;
    {
      auto actions = std::make_unique<MemoryActions>();
      actions->current = uwf::simulation::application::completeEnhancedModeStatus();
      EnhancedModeSession session(std::move(actions), borrowed);
      session.refresh();
      QVERIFY(borrowed.running());
    }
    QVERIFY(!borrowed.running());
    QCOMPARE(borrowed.stops, 1);
    auto owned = std::make_unique<uwf::simulation::application::MemoryEnhancedModeAgent>();
    QPointer<uwf::simulation::application::MemoryEnhancedModeAgent> lifetime(owned.get());
    {
      EnhancedModeSession session(std::make_unique<MemoryActions>(), std::move(owned));
      QVERIFY(lifetime);
    }
    QVERIFY(!lifetime);
  }
  void invalidDependenciesFailBeforeConnecting() {
    uwf::simulation::application::MemoryEnhancedModeAgent agent;
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, EnhancedModeSession({}, agent));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, EnhancedModeSession(std::make_unique<MemoryActions>(), std::unique_ptr<EnhancedModeAgentConnection>{}));
  }
};
}  // namespace

QTEST_GUILESS_MAIN(EnhancedModeSessionTests)
#include "EnhancedModeSessionTests.moc"
