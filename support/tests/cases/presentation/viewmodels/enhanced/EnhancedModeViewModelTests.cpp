// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <array>
#include <atomic>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <thread>

#include "cases/presentation/viewmodels/enhanced/FakeEnhancedModeActions.h"
#include "presentation/viewmodels/common/AutomaticCommitText.h"
#include "presentation/viewmodels/common/RuntimeWriteText.h"
#include "presentation/viewmodels/enhanced/EnhancedModeViewModel.h"
#include "support/app/WorkerGate.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

app::AutomaticCommitSummary commitSummary(const std::initializer_list<app::AutomaticCommitDisposition> dispositions) {
  app::AutomaticCommitSummary summary;
  for (const auto disposition : dispositions) summary.include(disposition);
  return summary;
}

class EnhancedModeViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void enhancedModeStatusProjection_data();
  void enhancedModeStatusProjection();
  void enhancedModeAutomaticCommitPauseDoesNotDisableServiceControls();
  void enhancedModeReadOnlyMediaPauseRespectsServiceWriteAdmission();
  void enhancedModeDisableAndRemoveRemainSeparate();
  void enhancedModeWorkersRespectReceiverLifetimes_data();
  void enhancedModeWorkersRespectReceiverLifetimes();
  void enhancedModePublicationFailureIsReportedWithoutEscapingTheEventLoop();
  void enhancedModeRetainsFeedbackAcrossAgentChanges();
  void enhancedModePrimaryActionRetriesUnavailableState_data();
  void enhancedModePrimaryActionRetriesUnavailableState();
  void enhancedModeReadOnlyRestrictionAllowsStatusRetryWithoutWrites();
  void enhancedModePublishesCompleteState_data();
  void enhancedModePublishesCompleteState();
  void enhancedModeReadsDiscardObsoleteObservations_data();
  void enhancedModeReadsDiscardObsoleteObservations();
  void enhancedModeActionsReserveAdmissionAcrossHostCallbacks_data();
  void enhancedModeActionsReserveAdmissionAcrossHostCallbacks();
  void enhancedModeForcedPausesDoNotClaimRetainedOverlay_data();
  void enhancedModeForcedPausesDoNotClaimRetainedOverlay();
  void fallbackCommitHintRequiresKnownReadyEligibility();
  void automaticCommitSummaryPublishesAllReasonsAndRecovers();
};

void EnhancedModeViewModelTests::enhancedModeForcedPausesDoNotClaimRetainedOverlay_data() {
  QTest::addColumn<int>("restriction");
  QTest::addColumn<int>("reason");
  QTest::newRow("horm") << int(app::RuntimeWriteRestriction::HormActive) << int(app::AutomaticCommitDisposition::SkipHormActive);
  QTest::newRow("horm-transition") << int(app::RuntimeWriteRestriction::None) << int(app::AutomaticCommitDisposition::SkipHormTransition);
  QTest::newRow("full-commit") << int(app::RuntimeWriteRestriction::None) << int(app::AutomaticCommitDisposition::SkipFullCommit);
}
void EnhancedModeViewModelTests::enhancedModeForcedPausesDoNotClaimRetainedOverlay() {
  QFETCH(int, restriction);
  QFETCH(int, reason);
  using Model = presentation::EnhancedModeViewModel;
  FakeEnhancedModeActions actions;
  actions.current.state = app::EnhancedModeState::Enabled;
  actions.current.serviceExists = actions.current.serviceRegistryPresent = true;
  actions.current.ownProcess = actions.current.automaticStart = actions.current.localSystemAccount = true;
  actions.current.executableMatches = actions.current.preshutdownTimeoutConfigured = actions.current.requiredPrivilegesConfigured = true;
  actions.current.running = actions.current.preshutdownAccepted = true;
  Model model(actions);
  model.setAutomaticCommitSummary(commitSummary({static_cast<app::AutomaticCommitDisposition>(reason)}));
  model.setWriteRestriction(static_cast<app::RuntimeWriteRestriction>(restriction));
  model.refreshStatus();
  QCOMPARE(model.statusText().staging, QStringLiteral("Paused"));
  QVERIFY(model.statusText().stagingDescription.contains(presentation::automaticCommitDescription(static_cast<app::AutomaticCommitDisposition>(reason))));
  QVERIFY(!model.statusText().stagingDescription.contains(QStringLiteral("retained")));
  const bool writable = static_cast<app::RuntimeWriteRestriction>(restriction) == app::RuntimeWriteRestriction::None;
  QCOMPARE(model.statusText().canApply, writable);
  QCOMPARE(model.statusText().canRemove, writable);
  if (writable) QVERIFY(model.statusText().issues.isEmpty());
  model.setWriteRestriction(app::RuntimeWriteRestriction::HormUnknown);
  QCOMPARE(model.statusText().staging, QStringLiteral("Paused"));
  QVERIFY(!model.statusText().canApply);
  QVERIFY(!model.statusText().canRemove);
}

void EnhancedModeViewModelTests::fallbackCommitHintRequiresKnownReadyEligibility() {
  using Model = presentation::EnhancedModeViewModel;
  FakeEnhancedModeActions actions;
  Model model(actions);
  QVERIFY(!model.statusText().showFallbackCommitHint);
  model.refreshStatus();
  QVERIFY(!model.statusText().showFallbackCommitHint);
  model.setAutomaticCommitSummary({});
  // The application can offer its own eligible commit path even without an
  // installed service, but must not promise that path for any paused subset.
  QVERIFY(model.statusText().showFallbackCommitHint);
  for (const auto disposition :
       {app::AutomaticCommitDisposition::SkipRetainedOverlay, app::AutomaticCommitDisposition::SkipReadOnlyMedia,
        app::AutomaticCommitDisposition::SkipHormActive, app::AutomaticCommitDisposition::SkipHormTransition, app::AutomaticCommitDisposition::SkipFullCommit,
        app::AutomaticCommitDisposition::Unavailable, app::AutomaticCommitDisposition::AlreadyPersistent}) {
    model.setAutomaticCommitSummary(commitSummary({disposition}));
    QVERIFY(!model.statusText().showFallbackCommitHint);
  }
  model.setAutomaticCommitSummary({});
  QVERIFY(model.statusText().showFallbackCommitHint);
  for (const auto restriction :
       {app::RuntimeWriteRestriction::HormActive, app::RuntimeWriteRestriction::HormRestartRequired, app::RuntimeWriteRestriction::HormUnknown,
        app::RuntimeWriteRestriction::ReadOnlyMediaActive, app::RuntimeWriteRestriction::StateUnavailable}) {
    model.setWriteRestriction(restriction);
    QVERIFY(!model.statusText().showFallbackCommitHint);
  }
  model.setWriteRestriction(app::RuntimeWriteRestriction::None);
  QVERIFY(model.statusText().showFallbackCommitHint);
  actions.failRead = true;
  model.refreshStatus();
  QVERIFY(!model.statusText().showFallbackCommitHint);
  actions.failRead = false;
  model.refreshStatus();
  QVERIFY(model.statusText().showFallbackCommitHint);
}

void EnhancedModeViewModelTests::automaticCommitSummaryPublishesAllReasonsAndRecovers() {
  using D = app::AutomaticCommitDisposition;
  FakeEnhancedModeActions actions;
  actions.current.state = app::EnhancedModeState::Enabled;
  actions.current.serviceExists = actions.current.serviceRegistryPresent = true;
  actions.current.ownProcess = actions.current.automaticStart = actions.current.localSystemAccount = true;
  actions.current.executableMatches = actions.current.preshutdownTimeoutConfigured = actions.current.requiredPrivilegesConfigured = true;
  actions.current.running = actions.current.preshutdownAccepted = true;
  presentation::EnhancedModeViewModel model(actions);
  QCOMPARE(model.statusText().stagingDescription, presentation::automaticCommitDescription(D::Unavailable));
  model.refreshStatus();
  QCOMPARE(model.statusText().staging, QStringLiteral("Unavailable"));
  QVERIFY(!model.statusText().showFallbackCommitHint);
  QVERIFY(model.statusText().canApply);

  auto mixed = commitSummary(
      {D::SkipHormActive, D::SkipHormTransition, D::SkipFullCommit, D::SkipReadOnlyMedia, D::SkipRetainedOverlay, D::CheckDurability, D::AlreadyPersistent});
  QSignalSpy changed(&model, &presentation::EnhancedModeViewModel::feedbackChanged);
  model.setAutomaticCommitSummary(mixed);
  QCOMPARE(changed.count(), 1);
  const auto partial = model.statusText();
  QCOMPARE(partial.staging, QStringLiteral("Partially paused"));
  for (const auto reason : {D::SkipHormActive, D::SkipHormTransition, D::SkipFullCommit, D::SkipReadOnlyMedia, D::SkipRetainedOverlay})
    QVERIFY(partial.stagingDescription.contains(presentation::automaticCommitDescription(reason)));
  QVERIFY(partial.canApply);
  QVERIFY(partial.canRemove);
  QVERIFY(!partial.showFallbackCommitHint);
  model.setAutomaticCommitSummary(mixed);
  QCOMPARE(changed.count(), 1);

  mixed.include(D::Unavailable);
  model.setAutomaticCommitSummary(mixed);
  QCOMPARE(changed.count(), 2);
  const auto unknown = model.statusText();
  QCOMPARE(unknown.staging, QStringLiteral("Unavailable"));
  QVERIFY(unknown.stagingDescription.contains(presentation::automaticCommitDescription(D::Unavailable)));
  for (const auto reason : {D::SkipHormActive, D::SkipHormTransition, D::SkipFullCommit, D::SkipReadOnlyMedia, D::SkipRetainedOverlay})
    QVERIFY(unknown.stagingDescription.contains(presentation::automaticCommitDescription(reason)));

  model.setAutomaticCommitSummary(commitSummary({D::AlreadyPersistent}));
  QCOMPARE(model.statusText().staging, QStringLiteral("No files to commit"));
  QCOMPARE(model.statusText().stagingDescription, QStringLiteral("Staged files are already on unprotected storage. No automatic commit is needed."));
  model.setAutomaticCommitSummary({});
  QCOMPARE(model.statusText().staging, QStringLiteral("Ready"));
  QVERIFY(model.statusText().showFallbackCommitHint);
  actions.failRead = true;
  model.refreshStatus();
  QCOMPARE(model.statusText().staging, QStringLiteral("Unavailable"));
  QVERIFY(!model.statusText().showFallbackCommitHint);
  actions.failRead = false;
  model.refreshStatus();
  QCOMPARE(model.statusText().staging, QStringLiteral("Ready"));
  QVERIFY(model.statusText().showFallbackCommitHint);
}

void EnhancedModeViewModelTests::enhancedModeStatusProjection_data() {
  using Status = app::EnhancedModeStatus;
  using State = app::EnhancedModeState;
  using Tone = presentation::EnhancedModeViewModel::FeedbackTone;
  QTest::addColumn<Status>("status");
  QTest::addColumn<QString>("label");
  QTest::addColumn<QString>("action");
  QTest::addColumn<QString>("issue");
  QTest::addColumn<Tone>("issueTone");
  Status enabled;
  enabled.state = State::Enabled;
  enabled.serviceExists = enabled.ownProcess = enabled.automaticStart = enabled.localSystemAccount = enabled.running = true;
  enabled.executableMatches = enabled.preshutdownTimeoutConfigured = enabled.requiredPrivilegesConfigured = enabled.preshutdownAccepted = true;
  enabled.agentState = app::EnhancedModeAgentState::Connected;
  QTest::newRow("enabled") << enabled << QStringLiteral("Enabled") << QStringLiteral("Disable enhanced mode") << QString{} << Tone::Neutral;
  QTest::newRow("absent") << Status{} << QStringLiteral("Disabled") << QStringLiteral("Enable enhanced mode") << QString{} << Tone::Neutral;
  auto disabled = enabled;
  disabled.state = State::Disabled;
  disabled.automaticStart = disabled.running = disabled.preshutdownAccepted = false;
  disabled.startupDisabled = true;
  disabled.agentState = app::EnhancedModeAgentState::Unobserved;
  QTest::newRow("disabled") << disabled << QStringLiteral("Disabled") << QStringLiteral("Enable enhanced mode") << QString{} << Tone::Neutral;
  auto stopped = enabled;
  stopped.running = false;
  stopped.state = State::Stopped;
  QTest::newRow("stopped") << stopped << QStringLiteral("Service stopped") << QStringLiteral("Start service")
                           << QStringLiteral("The service is installed but stopped. Start it to resume automatic file commits.") << Tone::Warning;
  struct Requirement {
    bool Status::* field;
    const char* name;
    const char* issue;
  };
  const std::array requirements{
      Requirement{&Status::ownProcess, "process", "The service is not configured as an independent process."},
      Requirement{&Status::automaticStart, "startup", "The service is not configured for automatic startup."},
      Requirement{&Status::localSystemAccount, "account", "The service is not configured to run as LocalSystem."},
      Requirement{&Status::executableMatches, "executable", "The service executable path or startup arguments do not match this executable."},
      Requirement{&Status::preshutdownTimeoutConfigured, "timeout", "The service preshutdown timeout is not configured correctly."},
      Requirement{&Status::requiredPrivilegesConfigured, "privileges", "The service does not declare the privileges required to start the UI agent."},
      Requirement{&Status::preshutdownAccepted, "notifications", "The service cannot receive Windows preshutdown notifications."},
  };
  for (const auto& requirement : requirements) {
    auto broken = enabled;
    broken.state = State::RepairRequired;
    broken.*(requirement.field) = false;
    QTest::newRow(requirement.name) << broken << QStringLiteral("Repair required") << QStringLiteral("Repair enhanced mode")
                                    << QString::fromLatin1(requirement.issue) << Tone::Warning;
  }
  auto connecting = enabled;
  connecting.agentState = app::EnhancedModeAgentState::Connecting;
  QTest::newRow("agent-connecting") << connecting << QStringLiteral("Enabled") << QStringLiteral("Disable enhanced mode")
                                    << QStringLiteral("Waiting for the UI agent to complete authentication.") << Tone::Warning;
  connecting.agentState = app::EnhancedModeAgentState::Disconnected;
  QTest::newRow("agent-disconnected") << connecting << QStringLiteral("Enabled") << QStringLiteral("Disable enhanced mode")
                                      << QStringLiteral("The service is running, but no authenticated UI agent is connected.") << Tone::Error;
  auto detailed = enabled;
  detailed.detail = QStringLiteral("controlled native detail");
  QTest::newRow("native-detail") << detailed << QStringLiteral("Enabled") << QStringLiteral("Disable enhanced mode") << detailed.detail << Tone::Error;
}

void EnhancedModeViewModelTests::enhancedModeStatusProjection() {
  QFETCH(app::EnhancedModeStatus, status);
  QFETCH(QString, label);
  QFETCH(QString, action);
  QFETCH(QString, issue);
  QFETCH(presentation::EnhancedModeViewModel::FeedbackTone, issueTone);
  FakeEnhancedModeActions actions;
  actions.current = status;
  presentation::EnhancedModeViewModel model(actions);
  model.setAutomaticCommitSummary({});
  model.refreshStatus();
  const auto text = model.statusText();
  QCOMPARE(text.label, label);
  QCOMPARE(text.action, action);
  QCOMPARE(text.issues, issue);
  QCOMPARE(text.issueTone, issueTone);
  QCOMPARE(text.canRemove, status.hasRemnants());
  QCOMPARE(text.staging, status.serviceContractSatisfied() ? QStringLiteral("Ready") : QStringLiteral("Unavailable"));
  QCOMPARE(text.primaryAction, status.state != app::EnhancedModeState::Enabled);
  QCOMPARE(text.startup, status.automaticStart    ? QStringLiteral("Automatic")
                         : status.startupDisabled ? QStringLiteral("Disabled")
                                                  : QStringLiteral("Not configured"));
}

void EnhancedModeViewModelTests::enhancedModeAutomaticCommitPauseDoesNotDisableServiceControls() {
  using Model = presentation::EnhancedModeViewModel;
  using D = app::AutomaticCommitDisposition;
  FakeEnhancedModeActions actions;
  Model model(actions);
  model.setAutomaticCommitSummary(commitSummary({D::SkipRetainedOverlay}));
  model.refreshStatus();
  QCOMPARE(model.statusText().staging, QStringLiteral("Unavailable"));
  QVERIFY(model.statusText().canApply);
  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.enables, 1);
  const auto paused = model.statusText();
  QCOMPARE(paused.label, QStringLiteral("Enabled"));
  QCOMPARE(paused.service, QStringLiteral("Running"));
  QCOMPARE(paused.staging, QStringLiteral("Paused"));
  QCOMPARE(paused.stagingDescription, QStringLiteral("Persistent overlay data will be retained at the next startup. Automatic commits are paused."));
  QVERIFY(paused.canApply);
  QVERIFY(paused.canRemove);
  QVERIFY(paused.issues.isEmpty());

  model.setAutomaticCommitSummary(commitSummary({D::SkipRetainedOverlay, D::CheckDurability}));
  const auto partial = model.statusText();
  QCOMPARE(partial.staging, QStringLiteral("Partially paused"));
  QVERIFY(partial.stagingDescription.contains(presentation::automaticCommitDescription(D::SkipRetainedOverlay)));
  QVERIFY(partial.stagingDescription.contains(QStringLiteral("Only eligible staged files will be committed.")));
  QCOMPARE(partial.action, paused.action);
  QCOMPARE(partial.canApply, paused.canApply);
  QCOMPARE(partial.canRemove, paused.canRemove);

  model.setAutomaticCommitSummary({});
  QCOMPARE(model.statusText().staging, QStringLiteral("Ready"));
  QCOMPARE(model.statusText().stagingDescription, QStringLiteral("Commits staged files before normal shutdown or restart."));

  model.setAutomaticCommitSummary(commitSummary({D::SkipRetainedOverlay}));
  const auto automaticDescription = model.statusText().stagingDescription;
  model.setWriteRestriction(app::RuntimeWriteRestriction::ReadOnlyMediaActive);
  QCOMPARE(model.statusText().staging, QStringLiteral("Paused"));
  QCOMPARE(model.statusText().stagingDescription, automaticDescription);
  QVERIFY(!model.statusText().canApply);
  QVERIFY(!model.statusText().canRemove);
  model.setWriteRestriction(app::RuntimeWriteRestriction::None);
  QCOMPARE(model.statusText().staging, QStringLiteral("Paused"));

  // Pausing the file commits must leave both service operations executable.
  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.disables, 1);
  QCOMPARE(model.statusText().staging, QStringLiteral("Unavailable"));
  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.enables, 2);
  QCOMPARE(model.statusText().staging, QStringLiteral("Paused"));
  model.removeService();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.removes, 1);
  QCOMPARE(model.statusText().staging, QStringLiteral("Unavailable"));

  actions.failRead = true;
  model.refreshStatus();
  QCOMPARE(model.statusText().staging, QStringLiteral("Unavailable"));
}

void EnhancedModeViewModelTests::enhancedModeReadOnlyMediaPauseRespectsServiceWriteAdmission() {
  using Model = presentation::EnhancedModeViewModel;
  FakeEnhancedModeActions actions;
  actions.current.state = app::EnhancedModeState::Enabled;
  actions.current.serviceExists = actions.current.serviceRegistryPresent = true;
  actions.current.ownProcess = actions.current.automaticStart = actions.current.localSystemAccount = true;
  actions.current.executableMatches = actions.current.preshutdownTimeoutConfigured = actions.current.requiredPrivilegesConfigured = true;
  actions.current.running = actions.current.preshutdownAccepted = true;
  Model model(actions);
  using D = app::AutomaticCommitDisposition;
  model.setAutomaticCommitSummary(commitSummary({D::SkipReadOnlyMedia}));
  model.setWriteRestriction(app::RuntimeWriteRestriction::ReadOnlyMediaActive);
  model.refreshStatus();
  const auto paused = model.statusText();
  QCOMPARE(paused.staging, QStringLiteral("Paused"));
  QVERIFY(paused.stagingDescription.contains(presentation::automaticCommitDescription(D::SkipReadOnlyMedia)));
  QVERIFY(!paused.stagingDescription.contains(QStringLiteral("Persistent")));
  QVERIFY(!paused.canApply);
  QVERIFY(!paused.canRemove);
  QCOMPARE(paused.issueHeading, QStringLiteral("Read-only"));
  QCOMPARE(paused.issues, QStringLiteral("File and registry commits are unavailable while Read-Only Media mode is active."));
  model.applyRequestedState();
  model.removeService();
  QCOMPARE(actions.enables + actions.disables + actions.starts + actions.removes, 0);
  model.setAutomaticCommitSummary(commitSummary({D::SkipReadOnlyMedia, D::CheckDurability}));
  const auto partial = model.statusText();
  QCOMPARE(partial.staging, QStringLiteral("Partially paused"));
  QVERIFY(partial.stagingDescription.contains(paused.stagingDescription));
  QVERIFY(!partial.canApply);
  QVERIFY(!partial.canRemove);
  // A paused file workflow is independent of admission for service registry writes.
  model.setWriteRestriction(app::RuntimeWriteRestriction::None);
  QCOMPARE(model.statusText().staging, QStringLiteral("Partially paused"));
  QVERIFY(model.statusText().canApply);
  QVERIFY(model.statusText().canRemove);
  model.setWriteRestriction(app::RuntimeWriteRestriction::HormActive);
  QCOMPARE(model.statusText().staging, QStringLiteral("Partially paused"));
  QCOMPARE(model.statusText().stagingDescription, partial.stagingDescription);
  model.setWriteRestriction(app::RuntimeWriteRestriction::HormUnknown);
  QCOMPARE(model.statusText().staging, QStringLiteral("Partially paused"));
  QCOMPARE(model.statusText().stagingDescription, partial.stagingDescription);
  model.setWriteRestriction(app::RuntimeWriteRestriction::None);
  model.setAutomaticCommitSummary(commitSummary({D::Unavailable}));
  QCOMPARE(model.statusText().staging, QStringLiteral("Unavailable"));
  QCOMPARE(model.statusText().stagingDescription, presentation::runtimeWriteRestrictionText(app::RuntimeWriteRestriction::StateUnavailable));
  QVERIFY(model.statusText().canApply);
  QVERIFY(model.statusText().canRemove);
}

void EnhancedModeViewModelTests::enhancedModeDisableAndRemoveRemainSeparate() {
  FakeEnhancedModeActions actions;
  presentation::EnhancedModeViewModel model(actions);
  model.refreshStatus();
  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.enables, 1);

  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.disables, 1);
  QCOMPARE(actions.removes, 0);
  QVERIFY(model.status()->serviceExists);
  QVERIFY(model.status()->serviceRegistryPresent);
  QVERIFY(model.status()->startupDisabled);
  QVERIFY(!model.status()->automaticStart);
  QVERIFY(!model.status()->running);
  QVERIFY(model.statusText().canRemove);
  QVERIFY(model.statusText().issues.isEmpty());
  QCOMPARE(model.statusText().action, QStringLiteral("Enable enhanced mode"));

  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.enables, 2);
  QVERIFY(model.status()->serviceExists);
  QVERIFY(model.status()->automaticStart);
  QVERIFY(!model.status()->startupDisabled);
  QCOMPARE(model.statusText().action, QStringLiteral("Disable enhanced mode"));

  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QVERIFY(model.statusText().canRemove);
  model.removeService();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.disables, 2);
  QCOMPARE(actions.removes, 1);
  QVERIFY(!model.status()->serviceExists);
  QVERIFY(!model.status()->serviceRegistryPresent);
  QVERIFY(!model.statusText().canRemove);
  QVERIFY(model.operationFeedback()->detail.contains(QStringLiteral("deleted")));
}

void EnhancedModeViewModelTests::enhancedModeWorkersRespectReceiverLifetimes_data() {
  QTest::addColumn<int>("lifetime");
  QTest::newRow("keep-open") << 0;
  QTest::newRow("close-presentation") << 1;
  QTest::newRow("destroy-session") << 2;
  QTest::newRow("destroy-during-status-notification") << 3;
}

void EnhancedModeViewModelTests::enhancedModeWorkersRespectReceiverLifetimes() {
  QFETCH(int, lifetime);
  auto gate = std::make_shared<test::WorkerGate>();
  auto actions = std::make_unique<FakeEnhancedModeActions>();
  actions->gate = gate;
  auto model = std::make_unique<presentation::EnhancedModeViewModel>(*actions);
  model->refreshStatus();
  int completions = 0;
  const auto owner = std::this_thread::get_id();
  connect(actions.get(), &app::EnhancedModeActions::operationFinished, this, [&](const auto&, const auto failure) {
    QCOMPARE(std::this_thread::get_id(), owner);
    QVERIFY(!failure);
    ++completions;
  });
  model->applyRequestedState();
  QTRY_COMPARE(gate->started.load(), 1);
  QCOMPARE(gate->finished.load(), 0);
  QVERIFY(model->busy());
  model->applyRequestedState();
  model->removeService();
  QCOMPARE(actions->enables, 1);
  QCOMPARE(actions->disables, 0);
  QCOMPARE(actions->removes, 0);
  bool eventDelivered = false;
  QTimer::singleShot(0, this, [&] { eventDelivered = true; });
  QTRY_VERIFY(eventDelivered);
  QCOMPARE(gate->finished.load(), 0);
  if (lifetime != 0) model.reset();
  if (lifetime == 2) actions.reset();
  if (lifetime == 3) connect(actions.get(), &app::EnhancedModeActions::statusChanged, this, [&](const auto&) { actions.reset(); });
  gate->release();
  QTRY_COMPARE(gate->finished.load(), 1);
  if (lifetime < 2) {
    QTRY_COMPARE(completions, 1);
    QVERIFY(!actions->operationInProgress());
    QCOMPARE(actions->current.state, app::EnhancedModeState::Enabled);
  } else {
    QTRY_VERIFY(!actions);
    QCOMPARE(completions, 0);
  }
}

void EnhancedModeViewModelTests::enhancedModePublicationFailureIsReportedWithoutEscapingTheEventLoop() {
  FakeEnhancedModeActions actions;
  actions.failPublish = true;
  presentation::EnhancedModeViewModel model(actions);
  model.refreshStatus();
  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QVERIFY(model.operationFeedback());
  QCOMPARE(model.operationFeedback()->tone, presentation::EnhancedModeViewModel::FeedbackTone::Error);
  QVERIFY(model.operationFeedback()->detail.contains(QStringLiteral("agent startup failed")));
  QCOMPARE(model.status()->state, app::EnhancedModeState::Enabled);
  QCOMPARE(actions.enables, 1);
}

void EnhancedModeViewModelTests::enhancedModeRetainsFeedbackAcrossAgentChanges() {
  FakeEnhancedModeActions actions;
  presentation::EnhancedModeViewModel model(actions);
  model.refreshStatus();
  QCOMPARE(model.statusText().action, QStringLiteral("Enable enhanced mode"));
  actions.persistenceWarning = QStringLiteral("registry commit deferred");
  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.enables, 1);
  QCOMPARE(actions.lastDescription, QStringLiteral("UWF Manager enhanced mode helper service"));
  QCOMPARE(model.operationFeedback()->tone, presentation::EnhancedModeViewModel::FeedbackTone::Warning);
  emit actions.agentConnectionStateChanged(app::EnhancedModeAgentState::Connected);
  QCOMPARE(model.status()->agentState, app::EnhancedModeAgentState::Connected);
  QVERIFY(model.operationFeedback()->detail.contains(actions.persistenceWarning));
  actions.failWrite = true;
  model.removeService();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(model.operationFeedback()->tone, presentation::EnhancedModeViewModel::FeedbackTone::Error);
  actions.failRead = true;
  model.refreshStatus();
  QVERIFY(!model.status());
  QVERIFY(!model.operationFeedback());
}

void EnhancedModeViewModelTests::enhancedModePrimaryActionRetriesUnavailableState_data() {
  QTest::addColumn<app::EnhancedModeState>("state");
  QTest::newRow("enable") << app::EnhancedModeState::Disabled;
  QTest::newRow("disable") << app::EnhancedModeState::Enabled;
  QTest::newRow("start") << app::EnhancedModeState::Stopped;
  QTest::newRow("repair") << app::EnhancedModeState::RepairRequired;
}

void EnhancedModeViewModelTests::enhancedModePrimaryActionRetriesUnavailableState() {
  QFETCH(app::EnhancedModeState, state);
  FakeEnhancedModeActions actions;
  actions.current.state = state;
  actions.current.serviceExists = state != app::EnhancedModeState::Disabled;
  presentation::EnhancedModeViewModel model(actions);
  model.refreshStatus();
  actions.failRead = true;
  model.refreshStatus();
  QVERIFY(!model.status());
  const auto unavailable = model.statusText();
  QCOMPARE(unavailable.action, QStringLiteral("Retry"));
  QVERIFY(unavailable.canApply);
  QVERIFY(!unavailable.canRemove);
  QVERIFY(!unavailable.showFacts);
  QVERIFY(unavailable.details.contains(QStringLiteral("SCM unavailable")));
  model.applyRequestedState();
  model.removeService();
  QVERIFY(!model.status());
  QCOMPARE(actions.enables + actions.disables + actions.starts + actions.removes, 0);
  actions.failRead = false;
  model.applyRequestedState();
  QVERIFY(model.status());
  QCOMPARE(model.status()->state, state);
  QCOMPARE(actions.enables + actions.disables + actions.starts + actions.removes, 0);
  model.applyRequestedState();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(actions.enables + actions.disables + actions.starts + actions.removes, 1);
}

void EnhancedModeViewModelTests::enhancedModeReadOnlyRestrictionAllowsStatusRetryWithoutWrites() {
  // Exercise each known-state primary action: enable, disable, start and repair.
  for (const auto state :
       {app::EnhancedModeState::Disabled, app::EnhancedModeState::Enabled, app::EnhancedModeState::Stopped, app::EnhancedModeState::RepairRequired}) {
    FakeEnhancedModeActions actions;
    actions.current.state = state;
    actions.current.serviceExists = actions.current.serviceRegistryPresent = state != app::EnhancedModeState::Disabled;
    actions.failRead = true;
    presentation::EnhancedModeViewModel model(actions);
    const auto restriction = app::RuntimeWriteRestriction::ReadOnlyMediaActive;
    model.setWriteRestriction(restriction);
    model.refreshStatus();
    QVERIFY(!model.status());
    QCOMPARE(model.statusText().action, QStringLiteral("Retry"));
    QVERIFY(model.statusText().canApply);
    QVERIFY(!model.statusText().canRemove);

    // A failed retry remains a read-only query and keeps Retry available.
    model.applyRequestedState();
    QVERIFY(!model.status());
    QVERIFY(model.statusText().canApply);
    QCOMPARE(actions.enables + actions.disables + actions.starts + actions.removes, 0);

    actions.failRead = false;
    model.applyRequestedState();
    QVERIFY(model.status());
    QCOMPARE(model.status()->state, state);
    QCOMPARE(model.statusText().issues, QStringLiteral("File and registry commits are unavailable while Read-Only Media mode is active."));
    QVERIFY(!model.statusText().canApply);
    QVERIFY(!model.statusText().canRemove);
    QCOMPARE(actions.enables + actions.disables + actions.starts + actions.removes, 0);

    // Calling the handlers directly must obey the same restriction as the UI.
    model.applyRequestedState();
    model.removeService();
    QVERIFY(!model.busy());
    QCOMPARE(actions.enables + actions.disables + actions.starts + actions.removes, 0);
    QCOMPARE(actions.current.state, state);
  }
}

void EnhancedModeViewModelTests::enhancedModePublishesCompleteState_data() {
  QTest::addColumn<int>("event");
  QTest::addColumn<bool>("feedbackNotification");
  QTest::addColumn<int>("reaction");
  // Every notification must expose complete state, including during observer replacement or destruction.
  const std::array events{"success", "failure-with-status", "failure-unavailable", "refresh-unavailable", "operation-started"};
  const std::array reactions{"observe", "release", "replace"};
  for (std::size_t event = 0; event < events.size(); ++event)
    for (const bool feedback : {false, true}) {
      if (event == 4 && !feedback) continue;
      for (std::size_t reaction = 0; reaction < reactions.size(); ++reaction)
        QTest::newRow(qPrintable(QStringLiteral("%1-%2-%3")
                                     .arg(QString::fromLatin1(events[event]), feedback ? QStringLiteral("feedback") : QStringLiteral("status"),
                                          QString::fromLatin1(reactions[reaction]))))
            << static_cast<int>(event) << feedback << static_cast<int>(reaction);
    }
}

void EnhancedModeViewModelTests::enhancedModePublishesCompleteState() {
  QFETCH(int, event);
  QFETCH(bool, feedbackNotification);
  QFETCH(int, reaction);
  using Model = presentation::EnhancedModeViewModel;
  FakeEnhancedModeActions actions;
  auto model = std::make_unique<Model>(actions);
  model->refreshStatus();
  emit actions.operationFinished({actions.current, QStringLiteral("old feedback")}, {});
  auto expected = actions.current;
  expected.state = app::EnhancedModeState::Enabled;
  actions.current = expected;
  actions.failRead = event == 2 || event == 3;
  const bool unavailable = actions.failRead;
  const bool hasFeedback = !unavailable && event != 4;
  const auto expectedTone = event == 0 ? Model::FeedbackTone::Success : Model::FeedbackTone::Error;
  bool reacted = false;
  int observations = 0;
  auto observe = [&] {
    if (reacted) return;
    ++observations;
    QCOMPARE(model->status().has_value(), !unavailable);
    if (!unavailable) QCOMPARE(model->status()->state, event == 4 ? app::EnhancedModeState::Disabled : expected.state);
    QCOMPARE(model->operationFeedback().has_value(), hasFeedback);
    if (hasFeedback) QCOMPARE(model->operationFeedback()->tone, expectedTone);
    reacted = true;
    if (reaction == 1)
      model.reset();
    else if (reaction == 2) {
      auto newer = expected;
      newer.state = app::EnhancedModeState::RepairRequired;
      emit actions.operationFinished({newer, QStringLiteral("newer feedback")}, {});
    }
  };
  if (feedbackNotification)
    connect(model.get(), &Model::feedbackChanged, this, observe);
  else if (unavailable)
    connect(model.get(), &Model::statusUnavailable, this, observe);
  else
    connect(model.get(), &Model::statusChanged, this, observe);

  const auto failure = std::make_exception_ptr(std::runtime_error("operation failed"));
  if (event == 3)
    model->refreshStatus();
  else if (event == 4)
    emit actions.operationStarted();
  else
    emit actions.operationFinished({expected, {}}, event == 0 ? std::exception_ptr{} : failure);
  QCOMPARE(observations, 1);
  if (reaction == 1)
    QVERIFY(!model);
  else if (reaction == 2) {
    QCOMPARE(model->status()->state, app::EnhancedModeState::RepairRequired);
    QVERIFY(model->operationFeedback()->detail.contains(QStringLiteral("newer feedback")));
  }
}

void EnhancedModeViewModelTests::enhancedModeReadsDiscardObsoleteObservations_data() {
  QTest::addColumn<bool>("recovering");
  QTest::addColumn<int>("outcome");
  QTest::addColumn<bool>("release");
  for (const bool recovering : {false, true})
    for (int outcome = 0; outcome < 3; ++outcome)
      for (const bool release : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2-%3")
                                     .arg(recovering ? QStringLiteral("recovery") : QStringLiteral("refresh"))
                                     .arg(outcome)
                                     .arg(release ? QStringLiteral("release") : QStringLiteral("replace"))))
            << recovering << outcome << release;
}

void EnhancedModeViewModelTests::enhancedModeReadsDiscardObsoleteObservations() {
  QFETCH(bool, recovering);
  QFETCH(int, outcome);
  QFETCH(bool, release);
  FakeEnhancedModeActions actions;
  auto model = std::make_unique<presentation::EnhancedModeViewModel>(actions);
  model->refreshStatus();
  actions.onRead = [&] {
    actions.onRead = {};
    if (release)
      model.reset();
    else {
      actions.current.state = app::EnhancedModeState::RepairRequired;
      model->refreshStatus();
    }
    if (outcome == 1) throw std::runtime_error("old observation failed");
    if (outcome == 2) throw 1;
  };
  if (recovering)
    emit actions.operationFinished({}, std::make_exception_ptr(std::runtime_error("operation failed")));
  else
    model->refreshStatus();
  if (release)
    QVERIFY(!model);
  else {
    QVERIFY(model->status());
    QCOMPARE(model->status()->state, app::EnhancedModeState::RepairRequired);
    QVERIFY(!model->operationFeedback());
  }
}

void EnhancedModeViewModelTests::enhancedModeActionsReserveAdmissionAcrossHostCallbacks_data() {
  QTest::addColumn<bool>("publishing");
  QTest::addColumn<int>("outcome");
  QTest::addColumn<bool>("release");
  for (const bool publishing : {false, true})
    for (int outcome = 0; outcome < 3; ++outcome)
      for (const bool release : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2-%3")
                                     .arg(publishing ? QStringLiteral("publish") : QStringLiteral("prepare"))
                                     .arg(outcome)
                                     .arg(release ? QStringLiteral("release") : QStringLiteral("reenter"))))
            << publishing << outcome << release;
}

void EnhancedModeViewModelTests::enhancedModeActionsReserveAdmissionAcrossHostCallbacks() {
  QFETCH(bool, publishing);
  QFETCH(int, outcome);
  QFETCH(bool, release);
  auto actions = std::make_unique<FakeEnhancedModeActions>();
  int completions = 0, starts = 0;
  bool callbackRan = false, reserved = false, failed = false;
  connect(actions.get(), &app::EnhancedModeActions::operationStarted, this, [&] { ++starts; });
  connect(actions.get(), &app::EnhancedModeActions::operationFinished, this, [&](const auto&, const auto failure) {
    ++completions;
    failed = static_cast<bool>(failure);
  });
  const auto callback = [&] {
    callbackRan = true;
    reserved = actions->operationInProgress();
    if (release)
      actions.reset();
    else
      actions->requestChange(app::EnhancedModeAction::Disable);
    if (outcome == 1) throw std::runtime_error("controlled host failure");
    if (outcome == 2) throw 1;
  };
  if (publishing)
    actions->onPublish = callback;
  else
    actions->onPrepare = callback;
  actions->requestChange(app::EnhancedModeAction::Enable);
  QTRY_VERIFY(callbackRan);
  QVERIFY(reserved);
  if (release) {
    QVERIFY(!actions);
    QCOMPARE(completions, 0);
  } else {
    QTRY_COMPARE(completions, 1);
    QVERIFY(!actions->operationInProgress());
    QCOMPARE(actions->enables, 1);
    QCOMPARE(actions->disables, 0);
    QCOMPARE(failed, outcome != 0);
  }
  QCOMPARE(starts, publishing || (!release && outcome == 0) ? 1 : 0);
}

}  // namespace

QTEST_GUILESS_MAIN(EnhancedModeViewModelTests)
#include "EnhancedModeViewModelTests.moc"
