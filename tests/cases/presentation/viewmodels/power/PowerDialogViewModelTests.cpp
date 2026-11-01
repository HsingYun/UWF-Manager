// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QElapsedTimer>
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>

#include "presentation/viewmodels/common/PresentationTiming.h"
#include "presentation/viewmodels/power/PowerDialogViewModel.h"
#include "presentation/viewmodels/common/AutomaticCommitText.h"

namespace {
using namespace uwf;

class PowerDialogViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void directConfirmationDefersPreparationUntilCanceled();
  void powerPreparationAndCommitRespectTimingAndCancelBoundaries();
  void directPowerDoesNotPrepareStaging();
  void powerFailureContinuationRequiresRecordedTargets_data();
  void powerFailureContinuationRequiresRecordedTargets();
  void powerCallbacksOwnTheirInputsAcrossReceiverRelease_data();
  void powerCallbacksOwnTheirInputsAcrossReceiverRelease();
  void powerPreparationReentryKeepsTheDialogDecision_data();
  void powerPreparationReentryKeepsTheDialogDecision();
  void automaticPauseReasonsSurvivePreparation_data();
  void automaticPauseReasonsSurvivePreparation();
  void changedFinalProposalRequiresAnotherConfirmation_data();
  void changedFinalProposalRequiresAnotherConfirmation();
  void stableSkippedProposalIsRevalidatedWithoutStartingWrites();
  void unknownFinalCapabilityCannotAcceptPower();
  void automaticCommitPresentationUsesActualPreparedFileCount_data();
  void automaticCommitPresentationUsesActualPreparedFileCount();
};

void PowerDialogViewModelTests::automaticCommitPresentationUsesActualPreparedFileCount_data() {
  QTest::addColumn<bool>("hasFiles");
  QTest::addColumn<bool>("unknown");
  for (const bool files : {false, true})
    for (const bool unknown : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("files-%1/unknown-%2").arg(files).arg(unknown))) << files << unknown;
}

void PowerDialogViewModelTests::automaticCommitPresentationUsesActualPreparedFileCount() {
  QFETCH(bool, hasFiles);
  QFETCH(bool, unknown);
  using D = app::AutomaticCommitDisposition;
  app::AutomaticCommitSummary summary;
  for (const auto disposition : {D::SkipHormActive, D::SkipReadOnlyMedia, D::SkipRetainedOverlay, D::CheckDurability, D::AlreadyPersistent})
    summary.include(disposition);
  if (unknown) summary.include(D::Unavailable);
  int writes = 0;
  presentation::PowerDialogViewModel model(
      {app::PowerAction::Restart, [&]() -> std::optional<app::PowerStagingPreparation> {
         if (!hasFiles) return app::PowerStagingNotRequired{summary};
         return app::PowerStagingCommit{3, [&] { ++writes; return app::PowerStagingProgress{}; }, summary};
       }},
      std::chrono::milliseconds::zero());
  QSignalSpy ready(&model, &presentation::PowerDialogViewModel::preparationReady);
  model.start();
  QTRY_COMPARE(ready.count(), 1);
  const auto view = model.automaticCommitPresentation();
  const auto expected = presentation::automaticCommitPresentation(summary, hasFiles ? 3u : 0u);
  QCOMPARE(view.status, expected.status);
  QCOMPARE(view.description, expected.description);
  QCOMPARE(view.unavailable, unknown);
  QVERIFY(view.description.contains(presentation::automaticCommitDescription(D::SkipHormActive)));
  QVERIFY(view.description.contains(presentation::automaticCommitDescription(D::SkipReadOnlyMedia)));
  QVERIFY(view.description.contains(presentation::automaticCommitDescription(D::SkipRetainedOverlay)));
  if (!hasFiles) {
    QVERIFY(!view.description.contains(QStringLiteral("Only eligible staged files will be committed.")));
    QVERIFY(!view.description.contains(QStringLiteral("Other staged files will still be committed.")));
  }
  QCOMPARE(writes, 0);
}

void PowerDialogViewModelTests::stableSkippedProposalIsRevalidatedWithoutStartingWrites() {
  app::AutomaticCommitSummary summary;
  summary.include(app::AutomaticCommitDisposition::SkipHormTransition);
  app::AutomaticCommitProposal proposal;
  proposal.files.push_back(app::CommitNode{app::CommitTargetKind::File, QStringLiteral("C:\\staged.bin"), {}, {}, {}}.identity());
  int preparations = 0, revalidations = 0;
  app::PowerActionDialogRequest request{app::PowerAction::Shutdown, [&]() -> std::optional<app::PowerStagingPreparation> {
    ++preparations;
    return app::PowerStagingNotRequired{summary, proposal};
  }};
  request.pollStagingRevalidation = [&]() -> std::optional<app::PowerStagingPreparation> {
    if (++revalidations == 1) return std::nullopt;
    return app::PowerStagingNotRequired{summary, proposal};
  };
  presentation::PowerDialogViewModel model(std::move(request), std::chrono::milliseconds::zero());
  QSignalSpy ready(&model, &presentation::PowerDialogViewModel::preparationReady);
  QSignalSpy staging(&model, &presentation::PowerDialogViewModel::stagingBegan);
  QSignalSpy accepted(&model, &presentation::PowerDialogViewModel::accepted);
  model.start();
  QTRY_COMPARE(ready.count(), 1);
  model.confirm();
  QCOMPARE(model.state(), presentation::PowerDialogState::State::Preparing);
  QCOMPARE(accepted.count(), 0);
  QTRY_COMPARE(accepted.count(), 1);
  QCOMPARE(preparations, 1);
  QCOMPARE(revalidations, 2);
  QCOMPARE(ready.count(), 2);
  QCOMPARE(staging.count(), 0);
  QVERIFY(!model.stagingStarted());
  QVERIFY(!model.hasStaging());
  QCOMPARE(model.filesToCommit(), std::size_t{0});
  QCOMPARE(model.automaticCommitSummary(), summary);
  QCOMPARE(model.finish(true), app::PowerActionDialogOutcome::Confirmed);
}

void PowerDialogViewModelTests::unknownFinalCapabilityCannotAcceptPower() {
  app::AutomaticCommitSummary summary;
  summary.include(app::AutomaticCommitDisposition::SkipRetainedOverlay);
  int revalidations = 0;
  app::PowerActionDialogRequest request{app::PowerAction::Restart, [summary]() -> std::optional<app::PowerStagingPreparation> {
    return app::PowerStagingNotRequired{summary};
  }};
  request.pollStagingRevalidation = [&]() -> std::optional<app::PowerStagingPreparation> {
    ++revalidations;
    return app::PowerStagingFailure{QStringLiteral("The current automatic commit capability is unknown.")};
  };
  presentation::PowerDialogViewModel model(std::move(request), std::chrono::milliseconds::zero());
  QSignalSpy ready(&model, &presentation::PowerDialogViewModel::preparationReady);
  QSignalSpy failed(&model, &presentation::PowerDialogViewModel::failureObserved);
  QSignalSpy staging(&model, &presentation::PowerDialogViewModel::stagingBegan);
  QSignalSpy accepted(&model, &presentation::PowerDialogViewModel::accepted);
  model.start();
  QTRY_COMPARE(ready.count(), 1);
  model.confirm();
  QCOMPARE(model.state(), presentation::PowerDialogState::State::Preparing);
  QTRY_COMPARE(failed.count(), 1);
  QCOMPARE(model.state(), presentation::PowerDialogState::State::FailureDecision);
  QCOMPARE(revalidations, 1);
  QCOMPARE(ready.count(), 1);
  QCOMPARE(staging.count(), 0);
  QVERIFY(!model.stagingStarted());
  QVERIFY(!model.canContinueAfterStagingFailure());
  model.confirmDirect();
  model.confirm();
  QCOMPARE(accepted.count(), 0);
  QCOMPARE(model.finish(true), app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
}

void PowerDialogViewModelTests::changedFinalProposalRequiresAnotherConfirmation_data() {
  QTest::addColumn<bool>("sameCount");
  QTest::newRow("paused-to-commit") << false;
  QTest::newRow("same-count-different-volume-scope") << true;
}
void PowerDialogViewModelTests::changedFinalProposalRequiresAnotherConfirmation() {
  QFETCH(bool, sameCount);
  int writes = 0;
  int refreshes = 0;
  app::AutomaticCommitSummary paused;
  paused.include(app::AutomaticCommitDisposition::SkipRetainedOverlay);
  app::AutomaticCommitSummary required;
  required.include(app::AutomaticCommitDisposition::CheckDurability);
  const auto proposal = [](const QString& path) {
    app::AutomaticCommitProposal value;
    value.files.push_back(app::CommitNode{app::CommitTargetKind::File, path, {}, {}, {}}.identity());
    return value;
  };
  const auto operation = [&] {
    ++writes;
    return app::PowerStagingProgress{};
  };
  app::PowerActionDialogRequest request{app::PowerAction::Restart, [=]() -> std::optional<app::PowerStagingPreparation> {
    if (sameCount) return app::PowerStagingCommit{1, operation, required, proposal(QStringLiteral("C:\\first.bin"))};
    return app::PowerStagingNotRequired{paused};
  }};
  request.pollStagingRevalidation = [&]() -> std::optional<app::PowerStagingPreparation> {
    ++refreshes;
    return app::PowerStagingCommit{1, operation, required, proposal(QStringLiteral("D:\\second.bin"))};
  };
  presentation::PowerDialogViewModel model(std::move(request), std::chrono::milliseconds::zero());
  QSignalSpy ready(&model, &presentation::PowerDialogViewModel::preparationReady);
  QSignalSpy accepted(&model, &presentation::PowerDialogViewModel::accepted);
  model.start();
  QTRY_COMPARE(ready.count(), 1);
  model.confirm();
  QTRY_COMPARE(ready.count(), 2);
  QCOMPARE(model.state(), presentation::PowerDialogState::State::Confirmation);
  QCOMPARE(model.filesToCommit(), std::size_t{1});
  QCOMPARE(writes, 0);
  QCOMPARE(accepted.count(), 0);
  model.confirm();
  QTRY_COMPARE(accepted.count(), 1);
  QCOMPARE(refreshes, 2);
  QCOMPARE(writes, 1);
  QCOMPARE(model.finish(true), app::PowerActionDialogOutcome::Confirmed);
}

void PowerDialogViewModelTests::automaticPauseReasonsSurvivePreparation_data() {
  QTest::addColumn<int>("reason");
  for (const auto reason : {app::AutomaticCommitDisposition::SkipHormActive, app::AutomaticCommitDisposition::SkipHormTransition,
                            app::AutomaticCommitDisposition::SkipFullCommit, app::AutomaticCommitDisposition::SkipReadOnlyMedia,
                            app::AutomaticCommitDisposition::SkipRetainedOverlay})
    QTest::newRow(qPrintable(QString::number(static_cast<int>(reason)))) << static_cast<int>(reason);
}
void PowerDialogViewModelTests::automaticPauseReasonsSurvivePreparation() {
  QFETCH(int, reason);
  const auto disposition = static_cast<app::AutomaticCommitDisposition>(reason);
  app::AutomaticCommitSummary summary;
  summary.include(disposition);
  presentation::PowerDialogViewModel model(
      {app::PowerAction::Restart, [summary]() -> std::optional<app::PowerStagingPreparation> { return app::PowerStagingNotRequired{summary}; }},
      std::chrono::milliseconds::zero());
  QSignalSpy ready(&model, &presentation::PowerDialogViewModel::preparationReady);
  model.start();
  QTRY_COMPARE(ready.count(), 1);
  QVERIFY(model.automaticCommitSummary().paused());
  const auto reasonText = presentation::automaticCommitDescription(disposition);
  QVERIFY(!reasonText.isEmpty());
  QVERIFY(model.automaticCommitPresentation().description.contains(reasonText));
  QVERIFY(!model.hasStaging());
  QCOMPARE(model.filesToCommit(), std::size_t{0});
  model.confirm();
  QCOMPARE(model.finish(true), app::PowerActionDialogOutcome::Confirmed);
}

void PowerDialogViewModelTests::directConfirmationDefersPreparationUntilCanceled() {
  int reads = 0;
  presentation::PowerDialogViewModel model({app::PowerAction::Shutdown, [&]() -> std::optional<app::PowerStagingPreparation> {
                                              ++reads;
                                              return app::PowerStagingNotRequired{};
                                            }});
  QSignalSpy ready(&model, &presentation::PowerDialogViewModel::preparationReady);
  model.start();
  model.beginDirectConfirmation();
  QTest::qWait(40);
  QCOMPARE(reads, 0);
  model.cancelDirectConfirmation();
  QTRY_COMPARE_WITH_TIMEOUT(ready.count(), 1, 2000);
  QCOMPARE(reads, 1);
  QCOMPARE(model.finish(false), app::PowerActionDialogOutcome::Canceled);
}

void PowerDialogViewModelTests::powerPreparationAndCommitRespectTimingAndCancelBoundaries() {
  int polls = 0;
  int steps = 0;
  presentation::PowerDialogViewModel model({app::PowerAction::Restart, [&]() -> std::optional<app::PowerStagingPreparation> {
                                              ++polls;
                                              return app::PowerStagingCommit{
                                                  2, [&] {
                                                    ++steps;
                                                    return app::PowerStagingProgress{
                                                        steps == 1 ? app::PowerStagingState::InProgress : app::PowerStagingState::Failed,
                                                        static_cast<std::size_t>(steps), 2, QStringLiteral("C:\\file"), QStringLiteral("write failed")};
                                                  }};
                                            }});
  QSignalSpy ready(&model, &presentation::PowerDialogViewModel::preparationReady);
  QSignalSpy failed(&model, &presentation::PowerDialogViewModel::failureObserved);
  QSignalSpy accepted(&model, &presentation::PowerDialogViewModel::accepted);
  QElapsedTimer elapsed;
  elapsed.start();
  model.start();
  model.confirm();
  QCOMPARE(steps, 0);
  QTRY_COMPARE_WITH_TIMEOUT(ready.count(), 1, 3500);
  QVERIFY(elapsed.elapsed() >= 1000);
  QCOMPARE(polls, 1);
  model.confirm();
  QCOMPARE(model.state(), presentation::PowerDialogState::State::Staging);
  model.confirmDirect();
  QCOMPARE(accepted.count(), 0);
  QCOMPARE(model.state(), presentation::PowerDialogState::State::Staging);
  QVERIFY_THROWS_EXCEPTION(std::logic_error, static_cast<void>(model.finish(false)));
  QTRY_COMPARE(failed.count(), 1);
  QVERIFY(model.stagingStarted());
  QCOMPARE(steps, 2);
  QCOMPARE(model.finish(false), app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
}

void PowerDialogViewModelTests::directPowerDoesNotPrepareStaging() {
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument,
                           presentation::PowerDialogViewModel(
                               {app::PowerAction::Shutdown, [] { return std::optional<app::PowerStagingPreparation>{}; }, app::PowerActionDialogMode::Direct}));
  presentation::PowerDialogViewModel direct({app::PowerAction::Shutdown, {}, app::PowerActionDialogMode::Direct});
  direct.start();
  direct.confirm();
  QCOMPARE(direct.finish(true), app::PowerActionDialogOutcome::DirectConfirmed);
}

void PowerDialogViewModelTests::powerFailureContinuationRequiresRecordedTargets_data() {
  QTest::addColumn<int>("phase");
  QTest::addColumn<int>("failureKind");
  for (int phase = 1; phase < 3; ++phase)
    for (int kind = 0; kind < 3; ++kind) QTest::newRow(qUtf8Printable(QStringLiteral("phase-%1-failure-%2").arg(phase).arg(kind))) << phase << kind;
}

void PowerDialogViewModelTests::powerFailureContinuationRequiresRecordedTargets() {
  QFETCH(int, phase);
  QFETCH(int, failureKind);
  app::PowerStagingFailure failure{QStringLiteral("Complete original error\n第二行错误详情")};
  if (failureKind != 0) failure.result.emplace();
  if (failureKind == 2)
    failure.result->failures.append({QStringLiteral("C:\\failed.bin"), app::FileStagingCommitFailureKind::ProviderFailure, failure.details});
  app::PowerActionDialogRequest request(app::PowerAction::Restart);
  request.pollStagingPreparation = [phase, failure]() -> std::optional<app::PowerStagingPreparation> {
    if (phase == 1) return failure;
    return app::PowerStagingCommit{1,
                                   [failure] { return app::PowerStagingProgress{app::PowerStagingState::Failed, 0, 1, {}, failure.details, failure.result}; }};
  };
  presentation::PowerDialogViewModel model(std::move(request));
  QSignalSpy accepted(&model, &presentation::PowerDialogViewModel::accepted);
  QSignalSpy failed(&model, &presentation::PowerDialogViewModel::failureObserved);
  QSignalSpy ready(&model, &presentation::PowerDialogViewModel::preparationReady);
  model.start();
  if (phase == 2) {
    QTRY_COMPARE_WITH_TIMEOUT(ready.count(), 1, 3500);
    model.confirm();
  }
  QTRY_COMPARE_WITH_TIMEOUT(failed.count(), 1, 3500);
  QCOMPARE(model.state(), presentation::PowerDialogState::State::FailureDecision);
  QCOMPARE(model.canContinueAfterStagingFailure(), failureKind == 2);
  model.confirmDirect();
  QCOMPARE(accepted.count(), 0);
  model.confirm();
  QCOMPARE(accepted.count(), failureKind == 2 ? 1 : 0);
  QCOMPARE(model.finish(true),
           failureKind == 2 ? app::PowerActionDialogOutcome::ContinuedAfterStagingFailure : app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
}

void PowerDialogViewModelTests::powerCallbacksOwnTheirInputsAcrossReceiverRelease_data() {
  QTest::addColumn<bool>("staging");
  QTest::addColumn<int>("outcome");
  QTest::addColumn<bool>("release");
  for (const bool staging : {false, true})
    for (int outcome = 0; outcome < 3; ++outcome)
      for (const bool release : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2-%3")
                                     .arg(staging ? QStringLiteral("advance") : QStringLiteral("prepare"))
                                     .arg(outcome)
                                     .arg(release ? QStringLiteral("release") : QStringLiteral("retain"))))
            << staging << outcome << release;
}

void PowerDialogViewModelTests::powerCallbacksOwnTheirInputsAcrossReceiverRelease() {
  QFETCH(bool, staging);
  QFETCH(int, outcome);
  QFETCH(bool, release);
  std::unique_ptr<presentation::PowerDialogViewModel> model;
  auto marker = std::make_shared<int>(1);
  const std::weak_ptr<int> weak = marker;
  bool invoked = false, captureSurvived = false;
  std::function<void()> boundary = [&, marker] {
    if (release) model.reset();
    captureSurvived = !weak.expired() && *marker == 1;
    invoked = true;
    if (outcome == 1) throw std::runtime_error("controlled callback failure");
    if (outcome == 2) throw 1;
  };
  marker.reset();
  app::PowerActionDialogRequest request(app::PowerAction::Restart);
  if (staging)
    request.pollStagingPreparation = [boundary]() -> std::optional<app::PowerStagingPreparation> {
      return app::PowerStagingCommit{1, [boundary] {
                                       boundary();
                                       return app::PowerStagingProgress{app::PowerStagingState::Completed, 1, 1, {}, {}};
                                     }};
    };
  else
    request.pollStagingPreparation = [boundary]() -> std::optional<app::PowerStagingPreparation> {
      boundary();
      return app::PowerStagingNotRequired{};
    };
  boundary = {};
  model = std::make_unique<presentation::PowerDialogViewModel>(std::move(request), std::chrono::milliseconds::zero());
  int notifications = 0;
  const auto notified = [&] { ++notifications; };
  connect(model.get(), &presentation::PowerDialogViewModel::failureObserved, this, notified);
  connect(model.get(), &presentation::PowerDialogViewModel::progressChanged, this, notified);
  connect(model.get(), &presentation::PowerDialogViewModel::accepted, this, notified);
  connect(model.get(), &presentation::PowerDialogViewModel::preparationReady, this, [&] {
    if (staging)
      model->confirm();
    else
      notified();
  });
  model->start();
  QTRY_VERIFY(invoked);
  QVERIFY(captureSurvived);
  QCOMPARE(static_cast<bool>(model), !release);
  QCOMPARE(notifications, release ? 0 : staging ? 2 : 1);
}

void PowerDialogViewModelTests::powerPreparationReentryKeepsTheDialogDecision_data() {
  QTest::addColumn<int>("decision");
  QTest::addColumn<int>("outcome");
  for (int decision = 0; decision < 3; ++decision)
    for (int outcome = 0; outcome < 3; ++outcome)
      QTest::newRow(qPrintable(QStringLiteral("decision-%1-outcome-%2").arg(decision).arg(outcome))) << decision << outcome;
}

void PowerDialogViewModelTests::powerPreparationReentryKeepsTheDialogDecision() {
  QFETCH(int, decision);
  QFETCH(int, outcome);
  std::unique_ptr<presentation::PowerDialogViewModel> model;
  int polls = 0;
  app::PowerActionDialogRequest request(app::PowerAction::Restart, [&]() -> std::optional<app::PowerStagingPreparation> {
    if (++polls == 1) {
      if (decision == 0)
        static_cast<void>(model->finish(false));
      else if (decision == 1)
        model->confirmDirect();
      else {
        // A nested confirmation can resume its timer before the original
        // provider call returns. It must not start a second preparation.
        model->beginDirectConfirmation();
        model->cancelDirectConfirmation();
        QTest::qWait(presentation::timing::kProgressUpdateInterval * 2);
      }
    }
    if (outcome == 1) throw std::runtime_error("controlled callback failure");
    if (outcome == 2) throw 1;
    return app::PowerStagingNotRequired{};
  });
  model = std::make_unique<presentation::PowerDialogViewModel>(std::move(request), std::chrono::milliseconds::zero());
  QSignalSpy ready(model.get(), &presentation::PowerDialogViewModel::preparationReady);
  QSignalSpy failed(model.get(), &presentation::PowerDialogViewModel::failureObserved);
  QSignalSpy accepted(model.get(), &presentation::PowerDialogViewModel::accepted);
  model->start();
  QTRY_COMPARE(polls, 1);
  QTest::qWait(presentation::timing::kProgressUpdateInterval * 2);
  QCOMPARE(polls, 1);
  QCOMPARE(ready.count(), decision == 2 && outcome == 0 ? 1 : 0);
  QCOMPARE(failed.count(), decision == 2 && outcome != 0 ? 1 : 0);
  QCOMPARE(accepted.count(), decision == 1 ? 1 : 0);
  if (decision < 2) QCOMPARE(model->finish(true), decision == 0 ? app::PowerActionDialogOutcome::Canceled : app::PowerActionDialogOutcome::DirectConfirmed);
}

}  // namespace

QTEST_GUILESS_MAIN(PowerDialogViewModelTests)
#include "PowerDialogViewModelTests.moc"
