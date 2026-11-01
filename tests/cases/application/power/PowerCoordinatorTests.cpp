// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

#include "application/power/PowerCoordinator.h"
#include "application/power/contracts/PowerInteraction.h"
#include "foundation/logging/Log.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "support/app/CommitScenario.h"
#include "support/app/UwfMaintenanceFixture.h"

enum class RevocationOutcome { Acknowledged, Unacknowledged, StandardFailure, UnknownFailure };
Q_DECLARE_METATYPE(RevocationOutcome)

namespace {
using namespace uwf;
using namespace uwf::app;
using namespace uwf::windows;

class UnusedStore final : public FileStagingStore {
 public:
  mutable int calls = 0;
  QList<FileStagingEntry> load() const override {
    ++calls;
    throw std::logic_error("unexpected staging read");
  }
  void replace(const QList<FileStagingEntry>&) override {
    ++calls;
    throw std::logic_error("unexpected staging write");
  }
};
class StagingStore final : public FileStagingStore {
 public:
  QList<FileStagingEntry> entries;
  mutable int loads = 0;
  QList<FileStagingEntry> load() const override {
    ++loads;
    return entries;
  }
  void replace(const QList<FileStagingEntry>&) override { throw std::logic_error("unexpected staging write"); }
};

class PowerCoordinatorTests final : public QObject {
  Q_OBJECT
 private slots:
  void confirmationRefreshRevokesRetainedOverlayBeforePower() {
    test::application::UwfMaintenanceFixture fixture;
    const auto repository = fixture.repository();
    auto tree = std::make_shared<test::commit::Tree>(false);
    tree->root.fileTarget = test::application::memoryFileTarget(tree->root.path);
    tree->branch.fileTarget = test::application::memoryFileTarget(tree->branch.path);
    tree->leaf.fileTarget = test::application::memoryFileTarget(tree->leaf.path);
    tree->children.clear();
    tree->children.insert(tree->root.identity(), {tree->leaf});
    fixture.stageFiles({tree->leaf.path});
    auto state = fixture.backend->snapshot();
    state.current.overlayType = state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Disk);
    state.current.overlayFlags = state.next.overlayFlags = compat::flags::persistent;
    state.runtime.resetPersistentState = 0;
    fixture.backend->seed(std::move(state));
    StagingStore store;
    store.entries = {{FileStagingKind::Directory, tree->root.path}};
    int markers = 0;
    QString warning;
    PowerInteraction interaction;
    interaction.confirm = [&](PowerActionDialogRequest request) {
      const auto wait = [&](const auto& poll) {
        std::optional<PowerStagingPreparation> result;
        for (int i = 0; i < 2000 && !result; ++i) { result = poll(); if (!result) QTest::qWait(1); }
        if (!result) throw std::logic_error("preparation timed out");
        return std::move(*result);
      };
      auto initial = wait(request.pollStagingPreparation);
      if (!std::holds_alternative<PowerStagingNotRequired>(initial) || tree->reads.load() != 0)
        throw std::logic_error("retained paths were traversed before confirmation");
      auto changed = fixture.backend->snapshot();
      changed.runtime.resetPersistentState = 1;
      fixture.backend->seed(std::move(changed));  // External observation changes after the dialog is ready.
      auto final = wait(request.pollStagingRevalidation);
      auto* commit = std::get_if<PowerStagingCommit>(&final);
      if (!commit || commit->filesToCommit != 1 || markers != 0 || !fixture.state->commits.empty())
        throw std::logic_error("revoked retention did not prepare a new, unwritten proposal");
      const auto reads = tree->reads.load();
      final = wait(request.pollStagingRevalidation); // User reviews and confirms the new proposal.
      commit = std::get_if<PowerStagingCommit>(&final);
      if (!commit || tree->reads.load() != reads) throw std::logic_error("stable final confirmation rescanned the directory");
      auto progress = commit->advanceStaging();
      for (int i = 0; i < 2000 && progress.state == PowerStagingState::InProgress; ++i) { QTest::qWait(1); progress = commit->advanceStaging(); }
      if (progress.state != PowerStagingState::Completed) throw std::logic_error("newly required commit did not complete");
      return PowerActionDialogOutcome::Confirmed;
    };
    interaction.warning = [&](const QString&, const QString& detail) { warning = detail; };
    interaction.describeStagingResult = [](const auto&) { return QStringLiteral("unexpected failure"); };
    PreshutdownCommitControl control{[&] { ++markers; return PreshutdownControlResult::Acknowledged; }, {}};
    PowerCoordinator power({repository, store, UwfCapability::Available, tree, std::move(control)}, std::move(interaction));
    power.safeRestart();
    QVERIFY2(warning.isEmpty(), qPrintable(warning));
    QCOMPARE(fixture.state->commits.size(), std::size_t{1});
    QCOMPARE(markers, 1);
    QCOMPARE(fixture.backend->snapshot().powerRequests.size(), std::size_t{1});
  }
  void knownUnsupportedModesPauseBeforePowerConfirmation_data() {
    QTest::addColumn<int>("mode");
    QTest::newRow("horm-active") << 0;
    QTest::newRow("horm-disabled-pending-restart") << 1;
    QTest::newRow("full-commit") << 2;
    QTest::newRow("effective-rom") << 3;
  }
  void knownUnsupportedModesPauseBeforePowerConfirmation() {
    QFETCH(int, mode);
    test::application::UwfMaintenanceFixture fixture;
    const auto repository = fixture.repository();
    auto state = fixture.backend->snapshot();
    state.current.overlayFlags = compat::flags::readOnlyMedia;
    state.next = state.current;
    state.runtime.hormEnabled = mode == 0;
    state.runtime.romDisposition = mode == 0 ? 2 : mode == 1 ? 3 : mode == 2 ? 4 : 0;
    fixture.backend->seed(std::move(state));
    auto tree = std::make_shared<test::commit::Tree>(false);
    tree->root.fileTarget = test::application::memoryFileTarget(tree->root.path);
    StagingStore store;
    store.entries = {{FileStagingKind::Directory, tree->root.path}};
    bool observed = false;
    QString warning;
    PowerInteraction interaction;
    interaction.confirm = [&](PowerActionDialogRequest request) {
      std::optional<PowerStagingPreparation> prepared;
      for (int attempt = 0; attempt < 2000 && !prepared; ++attempt) {
        prepared = request.pollStagingPreparation();
        if (!prepared) QTest::qWait(1);
      }
      if (!prepared) throw std::logic_error("automatic pause preparation did not finish");
      const auto* paused = std::get_if<PowerStagingNotRequired>(&*prepared);
      if (!paused || !paused->automaticCommit.paused()) throw std::logic_error("the automatic pause reason was lost");
      observed = true;
      return PowerActionDialogOutcome::Canceled;
    };
    interaction.warning = [&](const QString&, const QString& detail) { warning = detail; };
    PowerCoordinator power({repository, store, UwfCapability::Available, tree}, std::move(interaction));
    power.safeRestart();
    QVERIFY2(warning.isEmpty(), qPrintable(warning));
    QVERIFY(observed);
    QCOMPARE(tree->reads.load(), 0);
    QVERIFY(fixture.state->commits.empty());
    QVERIFY(fixture.backend->snapshot().powerRequests.empty());
  }
  void operationGateBlocksReentryAndRestoresAfterCancellation() {
    test::application::UwfMaintenanceFixture fixture;
    UnusedStore store;
    int confirmations = 0;
    PowerCoordinator* target = nullptr;
    PowerInteraction interaction;
    interaction.confirm = [&](PowerActionDialogRequest) {
      ++confirmations;
      if (!target->active()) throw std::logic_error("confirmation did not hold the power gate");
      target->safeShutdown();
      target->safeRestart();
      target->directRestart();
      return PowerActionDialogOutcome::Canceled;
    };
    PowerCoordinator power({fixture.repository(), store, UwfCapability::Available, systemCommitTraversalSource()}, std::move(interaction));
    target = &power;
    QSignalSpy changes(&power, &PowerCoordinator::activeChanged);
    power.setSuspended(true);
    power.safeShutdown();
    power.safeRestart();
    power.directRestart();
    QCOMPARE(confirmations, 0);
    QVERIFY(changes.empty());
    power.setSuspended(false);
    power.safeRestart();
    QCOMPARE(confirmations, 1);
    QVERIFY(!power.active());
    QCOMPARE(changes.size(), 2);
    QCOMPARE(changes[0][0].toBool(), true);
    QCOMPARE(changes[1][0].toBool(), false);
    power.directRestart();
    QCOMPARE(confirmations, 2);
    QVERIFY(!power.active());
    QCOMPARE(store.calls, 0);
    QVERIFY(fixture.backend->snapshot().powerRequests.empty());
  }
  void activeStateObserverMayDestroyTheCoordinator() {
    test::application::UwfMaintenanceFixture fixture;
    UnusedStore store;
    int confirmations = 0;
    PowerInteraction interaction;
    interaction.confirm = [&](PowerActionDialogRequest) {
      ++confirmations;
      return PowerActionDialogOutcome::Canceled;
    };
    auto power = std::make_unique<PowerCoordinator>(PowerServices{fixture.repository(), store, UwfCapability::Available, systemCommitTraversalSource()},
                                                    std::move(interaction));
    connect(power.get(), &PowerCoordinator::activeChanged, this, [&](const bool active) {
      if (active) power.reset();
    });
    power->safeRestart();
    QVERIFY(!power);
    QCOMPARE(confirmations, 0);
  }
  void confirmationOwnsItsActionState_data() {
    QTest::addColumn<int>("entry");
    QTest::addColumn<bool>("releaseOwner");
    QTest::addColumn<int>("outcome");
    for (int entry = 0; entry < 2; ++entry)
      for (const bool releaseOwner : {false, true})
        for (int outcome = 0; outcome < 4; ++outcome)
          QTest::newRow(qPrintable(QStringLiteral("entry-%1/release-%2/outcome-%3").arg(entry).arg(releaseOwner).arg(outcome)))
              << entry << releaseOwner << outcome;
  }
  void confirmationOwnsItsActionState() {
    QFETCH(int, entry);
    QFETCH(bool, releaseOwner);
    QFETCH(int, outcome);
    test::application::UwfMaintenanceFixture fixture;
    StagingStore store;
    std::unique_ptr<PowerCoordinator> power;
    int warnings = 0, confirmations = 0;
    PowerInteraction interaction;
    interaction.confirm = [&](PowerActionDialogRequest request) -> PowerActionDialogOutcome {
      ++confirmations;
      if (request.pollStagingPreparation) {
        const auto prepared = request.pollStagingPreparation();
        if (!prepared || !std::holds_alternative<PowerStagingNotRequired>(*prepared)) throw std::logic_error("expected empty staging list");
      }
      if (releaseOwner) power.reset();
      if (outcome == 2) throw std::runtime_error("confirmation failed");
      if (outcome == 3) throw 42;
      if (outcome == 1) return PowerActionDialogOutcome::Canceled;
      return entry == 0 ? PowerActionDialogOutcome::DirectConfirmed : PowerActionDialogOutcome::Confirmed;
    };
    interaction.warning = [&](const QString&, const QString&) { ++warnings; };
    power = std::make_unique<PowerCoordinator>(PowerServices{fixture.repository(), store, UwfCapability::Available, systemCommitTraversalSource()},
                                               std::move(interaction));
    fixture.backend->clearTrace();
    if (entry == 0)
      power->directRestart();
    else
      power->safeRestart();
    QCOMPARE(confirmations, 1);
    QCOMPARE(static_cast<bool>(power), !releaseOwner);
    QCOMPARE(warnings, !releaseOwner && outcome >= 2 ? 1 : 0);
    QCOMPARE(fixture.backend->snapshot().powerRequests.size(), !releaseOwner && outcome == 0 ? std::size_t{1} : std::size_t{0});
    if (power) QVERIFY(!power->active());
  }
  void preshutdownAndWarningCallbacksOwnTheirResources_data() {
    QTest::addColumn<bool>("duringWarning");
    QTest::newRow("release-during-marker") << false;
    QTest::newRow("release-during-warning") << true;
  }
  void preshutdownAndWarningCallbacksOwnTheirResources() {
    QFETCH(bool, duringWarning);
    test::application::UwfMaintenanceFixture fixture;
    auto repository = fixture.repository();
    if (duringWarning)
      fixture.backend->inject(
          {compat::Power{compat::PowerAction::Restart}, {}, test::FakeUwfBackend::FaultTiming::Before, compat::ErrorKind::Unavailable, "power failed"});
    UnusedStore store;
    std::unique_ptr<PowerCoordinator> power;
    int warnings = 0;
    std::vector<bool> markers;
    PreshutdownCommitControl control{[&] {
                                       markers.push_back(true);
                                       if (!duringWarning) {
                                         repository.reset();
                                         power.reset();
                                       }
                                       return PreshutdownControlResult::Acknowledged;
                                     },
                                     [&] {
                                       markers.push_back(false);
                                       return PreshutdownControlResult::Acknowledged;
                                     }};
    PowerInteraction interaction;
    interaction.confirm = [](PowerActionDialogRequest) { return PowerActionDialogOutcome::DirectConfirmed; };
    interaction.warning = [&](const QString&, const QString&) {
      ++warnings;
      repository.reset();
      power.reset();
    };
    power = std::make_unique<PowerCoordinator>(PowerServices{repository, store, UwfCapability::Available, systemCommitTraversalSource(), std::move(control)},
                                               std::move(interaction));
    fixture.backend->clearTrace();
    power->directRestart();
    QVERIFY(!power);
    QCOMPARE(markers, (std::vector<bool>{true, false}));
    QCOMPARE(warnings, duringWarning ? 1 : 0);
    QCOMPARE(fixture.backend->trace().size(), duringWarning ? std::size_t{1} : std::size_t{0});
    QVERIFY(fixture.backend->snapshot().powerRequests.empty());
  }
  void dialogCallbacksBecomeInertAfterConfirmation_data() {
    QTest::addColumn<bool>("releaseOwner");
    QTest::addColumn<bool>("prepareFirst");
    for (const bool releaseOwner : {false, true})
      for (const bool prepareFirst : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("release-%1/prepare-%2").arg(releaseOwner).arg(prepareFirst))) << releaseOwner << prepareFirst;
  }
  void dialogCallbacksBecomeInertAfterConfirmation() {
    QFETCH(bool, releaseOwner);
    QFETCH(bool, prepareFirst);
    test::application::UwfMaintenanceFixture fixture;
    auto tree = std::make_shared<test::commit::Tree>(false);
    StagingStore store;
    store.entries = {{FileStagingKind::File, tree->leaf.path}};
    std::function<std::optional<PowerStagingPreparation>()> latePreparation;
    std::function<PowerStagingProgress()> lateAdvance;
    PowerInteraction interaction;
    interaction.confirm = [&](PowerActionDialogRequest request) {
      latePreparation = request.pollStagingPreparation;
      if (prepareFirst) {
        std::optional<PowerStagingPreparation> prepared;
        for (int attempt = 0; attempt < 2000 && !prepared; ++attempt) {
          prepared = request.pollStagingPreparation();
          if (!prepared) QTest::qWait(1);
        }
        if (!prepared || !std::holds_alternative<PowerStagingCommit>(*prepared)) throw std::logic_error("staging preparation did not finish");
        lateAdvance = std::get<PowerStagingCommit>(*prepared).advanceStaging;
      }
      return PowerActionDialogOutcome::Canceled;
    };
    QString warning;
    interaction.warning = [&](const QString&, const QString& detail) { warning = detail; };
    auto power = std::make_unique<PowerCoordinator>(PowerServices{fixture.repository(), store, UwfCapability::Available, tree}, std::move(interaction));
    power->safeRestart();
    QVERIFY2(warning.isEmpty(), qPrintable(warning));
    QCOMPARE(store.loads, prepareFirst ? 1 : 0);
    QVERIFY(latePreparation);
    if (releaseOwner) power.reset();
    const auto result = latePreparation();
    QVERIFY(result && std::holds_alternative<PowerStagingNotRequired>(*result));
    if (lateAdvance) QCOMPARE(lateAdvance().state, PowerStagingState::Failed);
    QCOMPARE(store.loads, prepareFirst ? 1 : 0);
    QVERIFY(fixture.state->commits.empty());
    QVERIFY(fixture.backend->snapshot().powerRequests.empty());
    if (power) QVERIFY(!power->active());
  }
  void persistentOverlayPausesAutomaticCommitsAndClearingResumes_data() {
    QTest::addColumn<bool>("shutdown");
    QTest::newRow("shutdown") << true;
    QTest::newRow("restart") << false;
  }
  void persistentOverlayPausesAutomaticCommitsAndClearingResumes() {
    QFETCH(bool, shutdown);
    test::application::UwfMaintenanceFixture fixture;
    const auto repository = fixture.repository();
    auto tree = std::make_shared<test::commit::Tree>(false);
    const auto nestedPath = tree->branch.path + QStringLiteral("\\two.txt");
    auto nested = tree->leaf;
    nested.path = nestedPath;
    // Staging enumerates bound objects. Keep this fixture's lookup keys in the
    // same volume-relative identity space as its observed nodes.
    for (auto* node : {&tree->root, &tree->branch, &tree->leaf, &nested})
      node->fileTarget = test::application::memoryFileTarget(node->path);
    tree->children.clear();
    tree->children.insert(tree->root.identity(), {tree->branch, tree->leaf});
    tree->children.insert(tree->branch.identity(), {nested});
    fixture.stageFiles({tree->leaf.path, nestedPath});
    auto state = fixture.backend->snapshot();
    state.current.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Disk);
    state.current.overlayFlags = compat::flags::persistent;
    state.next = state.current;
    state.runtime.resetPersistentState = 0;
    fixture.backend->seed(std::move(state));

    StagingStore store;
    store.entries = {{FileStagingKind::Directory, tree->root.path}};
    const auto savedEntries = store.entries;
    bool expectCommits = false;
    int confirmations = 0;
    QString warning;
    std::vector<bool> markers;
    PowerInteraction interaction;
    interaction.confirm = [&](PowerActionDialogRequest request) {
      ++confirmations;
      if (request.mode != PowerActionDialogMode::Safe || !request.pollStagingPreparation)
        throw std::logic_error("expected a safe power action with automatic staging preparation");
      std::optional<PowerStagingPreparation> prepared;
      for (int attempt = 0; attempt < 2000 && !prepared; ++attempt) {
        prepared = request.pollStagingPreparation();
        if (!prepared) QTest::qWait(1);
      }
      if (!prepared) throw std::logic_error("automatic staging preparation did not finish");
      prepared.reset();
      for (int attempt = 0; attempt < 2000 && !prepared; ++attempt) {
        prepared = request.pollStagingRevalidation();
        if (!prepared) QTest::qWait(1);
      }
      if (!prepared) throw std::logic_error("final automatic staging preparation did not finish");
      if (!expectCommits) {
        if (!std::holds_alternative<PowerStagingNotRequired>(*prepared))
          throw std::logic_error("retained persistent overlay should pause automatic staging");
      } else {
        const auto* commit = std::get_if<PowerStagingCommit>(&*prepared);
        if (!commit || commit->filesToCommit != 2) throw std::logic_error("clearing the overlay should resume both staged file commits");
        auto progress = commit->advanceStaging();
        for (int attempt = 0; attempt < 2000 && progress.state == PowerStagingState::InProgress; ++attempt) {
          QTest::qWait(1);
          progress = commit->advanceStaging();
        }
        if (progress.state != PowerStagingState::Completed) throw std::logic_error("resumed automatic staging did not complete successfully");
      }
      return PowerActionDialogOutcome::Confirmed;
    };
    interaction.warning = [&](const QString&, const QString& detail) { warning = detail; };
    interaction.describeStagingResult = [](const auto&) { return QStringLiteral("unexpected staging failure"); };
    PreshutdownCommitControl control{[&] {
                                       markers.push_back(true);
                                       return PreshutdownControlResult::Acknowledged;
                                     },
                                     [&] {
                                       markers.push_back(false);
                                       return PreshutdownControlResult::Acknowledged;
                                     }};
    PowerCoordinator power({repository, store, UwfCapability::Available, tree, std::move(control)}, std::move(interaction));
    const auto invoke = [&] {
      if (shutdown)
        power.safeShutdown();
      else
        power.safeRestart();
    };
    invoke();
    QVERIFY2(warning.isEmpty(), qPrintable(warning));
    QCOMPARE(confirmations, 1);
    QCOMPARE(tree->reads.load(), 0);
    QVERIFY(fixture.state->commits.empty());
    QCOMPARE(store.entries, savedEntries);
    QCOMPARE(markers, (std::vector<bool>{true}));
    QCOMPARE(fixture.backend->snapshot().powerRequests.size(), std::size_t{1});
    QVERIFY(!power.active());

    state = fixture.backend->snapshot();
    state.runtime.resetPersistentState = 1;
    state.runtime.shutdownPending = false;
    fixture.backend->seed(std::move(state));
    expectCommits = true;
    invoke();
    QVERIFY2(warning.isEmpty(), qPrintable(warning));
    QCOMPARE(confirmations, 2);
    QVERIFY(tree->reads.load() > 0);
    QCOMPARE(fixture.state->commits.size(), std::size_t{2});
    QCOMPARE(store.entries, savedEntries);
    QCOMPARE(markers, (std::vector<bool>{true, true}));
    const auto after = fixture.backend->snapshot();
    QCOMPARE(after.powerRequests.size(), std::size_t{2});
    const auto action = shutdown ? compat::PowerAction::Shutdown : compat::PowerAction::Restart;
    QCOMPARE(after.powerRequests, (std::vector<compat::PowerAction>{action, action}));
    QVERIFY(!power.active());
  }
  void romPausesAutomaticCommitsOnlyAfterItIsLoaded_data() {
    QTest::addColumn<bool>("shutdown");
    QTest::addColumn<bool>("currentRom");
    QTest::addColumn<bool>("nextRom");
    for (const bool shutdown : {false, true})
      for (const bool currentRom : {false, true})
        for (const bool nextRom : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("%1/current-rom-%2/next-rom-%3").arg(shutdown ? "shutdown" : "restart").arg(currentRom).arg(nextRom)))
              << shutdown << currentRom << nextRom;
  }
  void romPausesAutomaticCommitsOnlyAfterItIsLoaded() {
    QFETCH(bool, shutdown);
    QFETCH(bool, currentRom);
    QFETCH(bool, nextRom);
    test::application::UwfMaintenanceFixture fixture;
    const auto repository = fixture.repository();
    auto tree = std::make_shared<test::commit::Tree>(false);
    auto nested = tree->leaf;
    nested.path = tree->branch.path + QStringLiteral("\\two.txt");
    for (auto* node : {&tree->root, &tree->branch, &tree->leaf, &nested})
      node->fileTarget = test::application::memoryFileTarget(node->path);
    tree->children.clear();
    tree->children.insert(tree->root.identity(), {tree->branch, tree->leaf});
    tree->children.insert(tree->branch.identity(), {nested});
    fixture.stageFiles({tree->leaf.path, nested.path});
    auto state = fixture.backend->snapshot();
    state.current.overlayType = state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
    state.current.overlayFlags = currentRom ? compat::flags::readOnlyMedia : 0;
    state.next.overlayFlags = nextRom ? compat::flags::readOnlyMedia : 0;
    fixture.backend->seed(std::move(state));

    StagingStore store;
    store.entries = {{FileStagingKind::Directory, tree->root.path}};
    const auto savedEntries = store.entries;
    int confirmations = 0;
    QString warning;
    PowerInteraction interaction;
    interaction.confirm = [&](PowerActionDialogRequest request) {
      ++confirmations;
      if (request.mode != PowerActionDialogMode::Safe || !request.pollStagingPreparation)
        throw std::logic_error("expected safe power staging preparation");
      std::optional<PowerStagingPreparation> prepared;
      for (int attempt = 0; attempt < 2000 && !prepared; ++attempt) {
        prepared = request.pollStagingPreparation();
        if (!prepared) QTest::qWait(1);
      }
      if (!prepared) throw std::logic_error("automatic staging preparation did not finish");
      prepared.reset();
      for (int attempt = 0; attempt < 2000 && !prepared; ++attempt) {
        prepared = request.pollStagingRevalidation();
        if (!prepared) QTest::qWait(1);
      }
      if (!prepared) throw std::logic_error("final automatic staging preparation did not finish");
      if (currentRom) {
        if (!std::holds_alternative<PowerStagingNotRequired>(*prepared)) throw std::logic_error("loaded ROM must skip automatic commits");
      } else {
        const auto* commit = std::get_if<PowerStagingCommit>(&*prepared);
        if (!commit || commit->filesToCommit != 2) throw std::logic_error("next-session ROM must not skip current automatic commits");
        auto progress = commit->advanceStaging();
        for (int attempt = 0; attempt < 2000 && progress.state == PowerStagingState::InProgress; ++attempt) {
          QTest::qWait(1);
          progress = commit->advanceStaging();
        }
        if (progress.state != PowerStagingState::Completed) throw std::logic_error("automatic staging did not complete successfully");
      }
      return PowerActionDialogOutcome::Confirmed;
    };
    interaction.warning = [&](const QString&, const QString& detail) { warning = detail; };
    interaction.describeStagingResult = [](const auto&) { return QStringLiteral("unexpected staging failure"); };
    PowerCoordinator power({repository, store, UwfCapability::Available, tree}, std::move(interaction));
    fixture.backend->clearTrace();
    if (shutdown)
      power.safeShutdown();
    else
      power.safeRestart();
    QVERIFY2(warning.isEmpty(), qPrintable(warning));
    QCOMPARE(confirmations, 1);
    QCOMPARE(fixture.state->commits.size(), currentRom ? std::size_t{0} : std::size_t{2});
    QCOMPARE(tree->reads.load() == 0, currentRom);
    QCOMPARE(store.loads, 1);
    QCOMPARE(store.entries, savedEntries);
    const auto action = shutdown ? compat::PowerAction::Shutdown : compat::PowerAction::Restart;
    QCOMPARE(fixture.backend->snapshot().powerRequests, (std::vector<compat::PowerAction>{action}));
    QVERIFY(!power.active());
  }
  void powerFailuresSurviveBestEffortTokenRevocation_data() {
    QTest::addColumn<bool>("shutdown");
    QTest::addColumn<RevocationOutcome>("revocation");
    QTest::addColumn<test::FakeUwfBackend::FaultTiming>("timing");
    for (const bool shutdown : {false, true})
      for (const auto& [revocation, name] :
           {std::pair{RevocationOutcome::Acknowledged, "acknowledged"}, std::pair{RevocationOutcome::Unacknowledged, "unacknowledged"},
            std::pair{RevocationOutcome::StandardFailure, "standard-exception"}, std::pair{RevocationOutcome::UnknownFailure, "nonstandard-exception"}})
        for (const auto& [timing, stage] : {std::pair{test::FakeUwfBackend::FaultTiming::Before, "before-dispatch"},
                                            std::pair{test::FakeUwfBackend::FaultTiming::AfterEffect, "after-effect"}})
          QTest::newRow(qPrintable(QStringLiteral("shutdown-%1/%2/%3").arg(shutdown).arg(name, stage))) << shutdown << revocation << timing;
  }
  void powerFailuresSurviveBestEffortTokenRevocation() {
    QFETCH(bool, shutdown);
    QFETCH(RevocationOutcome, revocation);
    QFETCH(test::FakeUwfBackend::FaultTiming, timing);
    test::application::UwfMaintenanceFixture fixture;
    const auto repository = fixture.repository();
    const auto action = shutdown ? compat::PowerAction::Shutdown : compat::PowerAction::Restart;
    const compat::Request request = compat::Power{action};
    fixture.backend->inject({request, {}, timing, compat::ErrorKind::Unavailable, "Original power acknowledgement failed"});
    const auto before = fixture.backend->snapshot();
    UnusedStore store;
    PowerInteraction interaction;
    int confirmations = 0;
    interaction.confirm = [&](PowerActionDialogRequest) {
      ++confirmations;
      return PowerActionDialogOutcome::DirectConfirmed;
    };
    QString title;
    QString warning;
    interaction.warning = [&](const QString& heading, const QString& text) {
      title = heading;
      warning = text;
    };
    std::vector<bool> markers;
    PreshutdownCommitControl control{[&] {
                                       markers.push_back(true);
                                       return PreshutdownControlResult::Acknowledged;
                                     },
                                     [&]() -> PreshutdownControlResult {
                                       markers.push_back(false);
                                       if (revocation == RevocationOutcome::StandardFailure) throw std::runtime_error("Marker revocation failed");
                                       if (revocation == RevocationOutcome::UnknownFailure) throw QStringLiteral("Marker revocation failed");
                                       return revocation == RevocationOutcome::Acknowledged ? PreshutdownControlResult::Acknowledged
                                                                                            : PreshutdownControlResult::Unacknowledged;
                                     }};
    PowerCoordinator power({repository, store, UwfCapability::Available, windows::systemCommitTraversalSource(), std::move(control)}, std::move(interaction));
    uwf::clearLogLines();
    fixture.backend->clearTrace();
    if (shutdown)
      power.safeShutdown();
    else
      power.directRestart();
    QCOMPARE(confirmations, 1);
    QCOMPARE(title, shutdown ? QStringLiteral("Direct shutdown failed") : QStringLiteral("Direct restart failed"));
    QVERIFY(warning.contains(QStringLiteral("Original power acknowledgement failed")));
    QVERIFY(!warning.contains(QStringLiteral("Marker revocation failed")));
    QCOMPARE(markers, (std::vector<bool>{true, false}));
    QCOMPARE(store.calls, 0);
    QVERIFY(!power.active());
    auto expected = before;
    if (timing == test::FakeUwfBackend::FaultTiming::AfterEffect) {
      expected.runtime.shutdownPending = true;
      expected.powerRequests.push_back(action);
    }
    QVERIFY(fixture.backend->snapshot() == expected);
    const auto trace = fixture.backend->trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(trace.front().request == request);
    QVERIFY(trace.front().response.failure);
    QCOMPARE(trace.front().response.failure->message, std::string("Original power acknowledgement failed"));
    if (revocation == RevocationOutcome::StandardFailure || revocation == RevocationOutcome::UnknownFailure) {
      const auto lines = uwf::recentLogLines();
      QVERIFY(std::ranges::any_of(lines, [&](const auto& line) {
        return line.find("revocation failed: error=") != std::string::npos &&
               line.find(revocation == RevocationOutcome::StandardFailure ? "Marker revocation failed" : "non-standard-exception") != std::string::npos;
      }));
    }
  }
};
}  // namespace

QTEST_GUILESS_MAIN(PowerCoordinatorTests)
#include "PowerCoordinatorTests.moc"
