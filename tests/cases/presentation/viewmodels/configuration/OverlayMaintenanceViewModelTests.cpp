// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QPointer>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QTranslator>
#include <array>
#include <atomic>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"
#include "presentation/viewmodels/configuration/VolumeConfigurationViewModel.h"
#include "support/app/WorkerGate.h"

namespace {
using namespace uwf;
class EmptyTranslator final : public QTranslator {
 public:
  bool isEmpty() const override { return false; }
  QString translate(const char*, const char*, const char*, int) const override { return QLatin1String(""); }
};
class MemoryMaintenance final : public app::OverlayMaintenanceActions {
 public:
  app::OverlayMaintenanceSnapshot state;
  std::shared_ptr<test::WorkerGate> gate;
  std::optional<app::OverlayMaintenanceSnapshot> delayedPayload;
  bool snapshotAvailable = true;
  mutable int snapshotCalls = 0;
  std::atomic_int reads{0}, writes{0};
  MemoryMaintenance() {
    state.filterEnabled = true;
    state.overlayCommit = false;
    state.hormEnabled = false;
    state.currentOverlayType = 0;
    state.currentFlags = app::OverlayFlags{4};
    state.nextFlags = app::OverlayFlags{4};
    state.hibernationAvailable = true;
    state.allVolumesProtected = true;
    state.romDisposition = 0;
  }
  app::OverlayMaintenanceSnapshot observe(const std::stop_token stop) override {
    ++reads;
    const auto captured = delayedPayload;
    if (gate) gate->wait(stop);
    return captured.value_or(state);
  }
  std::optional<app::OverlayMaintenanceSnapshot> snapshot() const override {
    ++snapshotCalls;
    return snapshotAvailable ? std::optional{state} : std::nullopt;
  }
  app::OverlayMaintenanceResult fullCommit(std::stop_token) override {
    ++writes;
    state.overlayCommit = true;
    return {state, {}, {}, app::WriteEffect::Confirmed, true};
  }
  app::OverlayMaintenanceResult createSwap(const QString&, std::stop_token) override {
    ++writes;
    return {state, {}, {}, app::WriteEffect::Confirmed, true};
  }
  app::OverlayMaintenanceResult setHorm(const bool enabled, std::stop_token) override {
    ++writes;
    state.hormEnabled = enabled;
    return {state, {}, {}, app::WriteEffect::Confirmed, true};
  }
};
class OverlayMaintenanceViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void publishedAdoptionUsesTheProjectionFromTheFirstNotification() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.hibernationAvailable.reset();
    actions->state.allVolumesProtected.reset();
    presentation::OverlayMaintenanceViewModel maintenance(actions);
    maintenance.setInteractionContext(true, false);
    QSignalSpy finished(&maintenance, &presentation::OverlayMaintenanceViewModel::refreshFinished);
    maintenance.adoptPublishedState();
    QVERIFY(maintenance.hasSnapshot());
    QVERIFY(!maintenance.busy());
    QVERIFY(maintenance.snapshot().readError.isEmpty());
    QVERIFY(!maintenance.snapshot().hibernationCapabilities);
    QVERIFY(!maintenance.snapshot().volumesReadable);
    QVERIFY(maintenance.fullCommitState().available);
    QVERIFY(!maintenance.canSetHorm(true));
    QVERIFY(!maintenance.canCreateSwap());
    QCOMPARE(actions->snapshotCalls, 1);
    QCOMPARE(actions->reads.load(), 0);
    QCOMPARE(finished.count(), 1);
    QVERIFY(finished.front().front().toBool());
    actions->state.currentFlags = app::OverlayFlags{0};
    actions->state.nextOverlayType = 1;
    maintenance.adoptPublishedState();
    QVERIFY(!maintenance.fullCommitState().available);
    QCOMPARE(maintenance.snapshot().nextOverlayType, std::optional<std::int64_t>{1});
    QCOMPARE(actions->snapshotCalls, 2);
    QCOMPARE(finished.count(), 2);
    QVERIFY(finished.back().front().toBool());
    QCoreApplication::processEvents();
    QCOMPARE(actions->reads.load(), 0);
    QCOMPARE(actions->writes.load(), 0);
  }
  void anUnavailablePublishedProjectionDoesNotStartAnObservation() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->snapshotAvailable = false;
    presentation::OverlayMaintenanceViewModel maintenance(actions);
    maintenance.setInteractionContext(true, false);
    QSignalSpy finished(&maintenance, &presentation::OverlayMaintenanceViewModel::refreshFinished);
    maintenance.adoptPublishedState();
    maintenance.adoptPublishedState();
    QCoreApplication::processEvents();
    QCOMPARE(actions->snapshotCalls, 2);
    QCOMPARE(actions->reads.load(), 0);
    QVERIFY(!maintenance.hasSnapshot());
    QVERIFY(!maintenance.busy());
    QVERIFY(!maintenance.fullCommitState().available);
    QVERIFY(finished.isEmpty());
    maintenance.refresh();
    QTRY_VERIFY(maintenance.hasSnapshot() && !maintenance.busy());
    QCOMPARE(actions->reads.load(), 1);
    QCOMPARE(actions->snapshotCalls, 3);
    QVERIFY(maintenance.fullCommitState().available);
    QCOMPARE(finished.count(), 1);
    QVERIFY(finished.front().front().toBool());
    actions->snapshotAvailable = true;
    maintenance.adoptPublishedState();
    QCOMPARE(actions->snapshotCalls, 4);
    QCOMPARE(actions->reads.load(), 1);
    QCOMPARE(actions->writes.load(), 0);
  }
  void globalCommitAvailabilityDoesNotDependOnTheCurrentVolumePage() {
    auto actions = std::make_shared<MemoryMaintenance>();
    presentation::OverlayMaintenanceViewModel maintenance(actions);
    maintenance.setInteractionContext(true, false);
    maintenance.adoptPublishedState();
    QTRY_VERIFY(!maintenance.busy() && maintenance.hasSnapshot());
    QCOMPARE(actions->reads.load(), 0);
    QVERIFY(maintenance.fullCommitState().available);
    presentation::VolumeConfigurationViewModel hidden(
        core::VolumeInfo{.driveLetter = {}, .volumeName = "Volume{11111111-2222-3333-4444-555555555555}", .fileSystem = {}, .label = {}}, false);
    const auto state = hidden.commitAvailability(&maintenance);
    QVERIFY(state.fullCommit.available);
    QVERIFY(state.menu);
    QVERIFY(!state.files);
    QVERIFY(!state.registry);
    QVERIFY(!hidden.commitAvailability(&maintenance, true).menu);
    QVERIFY(maintenance.stageFullCommit());
    QVERIFY(hidden.commitAvailability(&maintenance).removeFullCommitDraft);
    maintenance.unstageFullCommit();
    maintenance.setInteractionContext(true, true);
    QVERIFY(!hidden.commitAvailability(&maintenance).menu);
  }
  void aPublishedAdoptionDuringObservationCannotBeLost_data() {
    QTest::addColumn<bool>("requestAdoption");
    QTest::newRow("published-notification-while-busy") << true;
    QTest::newRow("write-finishes-before-notification") << false;
  }
  void aPublishedAdoptionDuringObservationCannotBeLost() {
    QFETCH(bool, requestAdoption);
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.nextOverlayType = 0;
    actions->state.nextFlags = app::OverlayFlags{4};
    actions->state.hibernationError = QStringLiteral("Host capability detail retained");
    actions->delayedPayload = actions->state;
    auto gate = actions->gate = std::make_shared<test::WorkerGate>();
    const auto release = qScopeGuard([gate] { gate->release(); });
    presentation::OverlayMaintenanceViewModel model(actions);
    model.refresh();
    QTRY_COMPARE(gate->started.load(), 1);
    QVERIFY(model.busy());
    actions->state.nextOverlayType = 1;
    actions->state.nextFlags = app::OverlayFlags{1};
    if (requestAdoption) model.adoptPublishedState();
    QCOMPARE(actions->snapshotCalls, 0);
    gate->release();
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.snapshot().nextOverlayType, std::optional<std::int64_t>{1});
    QCOMPARE(model.snapshot().nextFlags->raw, std::uint32_t{1});
    QCOMPARE(model.snapshot().hibernationError, QStringLiteral("Host capability detail retained"));
    QCOMPARE(actions->snapshotCalls, 1);
    QCOMPARE(actions->reads.load(), 1);
  }
  void aFailedHostRefreshIsNotHiddenByTheLatestPublishedConfiguration() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->delayedPayload = actions->state;
    actions->delayedPayload->readError = QStringLiteral("Physical host detail read failed");
    presentation::OverlayMaintenanceViewModel model(actions);
    QSignalSpy finished(&model, &presentation::OverlayMaintenanceViewModel::refreshFinished);
    model.refresh();
    QTRY_COMPARE(finished.count(), 1);
    QVERIFY(!finished.front().front().toBool());
    QVERIFY(model.snapshot().readError.contains(QStringLiteral("Physical host detail read failed")));
  }
  void translatedReasonsCannotEnableMaintenanceControls() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.volumesReadable = true;
    actions->state.systemVolumeId = QStringLiteral("volume-c");
    actions->state.volumes = {{QStringLiteral("volume-c"), QStringLiteral("C:"), {}, 0, true, 0, 0}};
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(false, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.hasSnapshot());
    QVERIFY(!model.panelState().protectReason.isEmpty());
    EmptyTranslator translator;
    QVERIFY(QCoreApplication::installTranslator(&translator));
    const auto removeTranslator = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });
    const auto panel = model.panelState();
    QVERIFY(panel.protectReason.isEmpty());
    QVERIFY(panel.unprotectReason.isEmpty());
    QVERIFY(!panel.protectEnabled);
    QVERIFY(!panel.unprotectEnabled);
    QVERIFY(!panel.manageProtectionEnabled);
    const auto storage = model.storageDialogState();
    QCOMPARE(storage.selectedIndex, 0);
    QVERIFY(!storage.selectionEnabled);
    QVERIFY(!storage.dialog.submitEnabled);
    const auto horm = model.hormDialogState();
    QVERIFY(!horm.protectEnabled);
    QVERIFY(!horm.dialog.submitEnabled);
    QCOMPARE(actions->writes.load(), 0);
  }
  void storageDialogSelectionUsesTheDraftThenObservedHostThenSystemVolume_data() {
    QTest::addColumn<QString>("draft");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("system");
    QTest::addColumn<QString>("expected");
    QTest::newRow("draft") << QStringLiteral("volume-d") << QStringLiteral("volume-c") << QStringLiteral("volume-c") << QStringLiteral("volume-d");
    QTest::newRow("observed-host") << QString{} << QStringLiteral("volume-d") << QStringLiteral("volume-c") << QStringLiteral("volume-d");
    QTest::newRow("system-identity") << QString{} << QString{} << QStringLiteral("volume-c") << QStringLiteral("volume-c");
    QTest::newRow("system-letter") << QString{} << QString{} << QString{} << QStringLiteral("volume-c");
  }
  void storageDialogSelectionUsesTheDraftThenObservedHostThenSystemVolume() {
    QFETCH(QString, draft);
    QFETCH(QString, host);
    QFETCH(QString, system);
    QFETCH(QString, expected);
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.filterEnabled = false;
    actions->state.volumesReadable = true;
    actions->state.systemDrive = QStringLiteral("C:");
    if (!system.isEmpty()) actions->state.systemVolumeId = system;
    if (!host.isEmpty()) actions->state.nextStorage.volumeId = host;
    actions->state.volumes = {{QStringLiteral("volume-d"), QStringLiteral("D:"), {}, 0, true, 0, 0},
                              {QStringLiteral("volume-c"), QStringLiteral("C:"), {}, 0, true, 0, 0}};
    auto source = std::make_shared<presentation::OverlayMaintenanceViewModel>(actions);
    source->setInteractionContext(true, false);
    source->reconcileAppliedDraft({}, actions->state);
    if (!draft.isEmpty()) QVERIFY(source->stageStorageLocation(draft));
    const auto edit = presentation::OverlayMaintenanceViewModel::editDraft(source);
    const auto state = edit->storageDialogState();
    QVERIFY(state.selectedIndex >= 0);
    QCOMPARE(state.volumes.at(static_cast<std::size_t>(state.selectedIndex)).id, expected);
    QCOMPARE(state.volumes.front().id, QStringLiteral("volume-c"));
    QVERIFY(state.dialog.submitEnabled);
    QCOMPARE(actions->writes.load(), 0);
  }
  void storageSelectionBelongsToOneDialogAndSurvivesNewObservations() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.filterEnabled = false;
    actions->state.volumesReadable = true;
    actions->state.systemVolumeId = QStringLiteral("volume-c");
    actions->state.volumes = {{QStringLiteral("volume-c"), QStringLiteral("C:"), {}, 0, true, 0, 0},
                              {QStringLiteral("volume-d"), QStringLiteral("D:"), {}, 0, true, 0, 0}};
    auto source = std::make_shared<presentation::OverlayMaintenanceViewModel>(actions);
    source->setInteractionContext(true, false);
    source->reconcileAppliedDraft({}, actions->state);
    const auto first = presentation::OverlayMaintenanceViewModel::editDraft(source);
    const auto second = presentation::OverlayMaintenanceViewModel::editDraft(source);
    first->selectStorageVolume(QStringLiteral("volume-d"));
    QCOMPARE(first->storageDialogState().selectedVolume, QStringLiteral("D:"));
    QCOMPARE(second->storageDialogState().selectedVolume, QStringLiteral("C:"));
    QVERIFY(!source->hasPendingChanges());
    auto observed = actions->state;
    observed.volumes.back().eligible = false;
    source->reconcileAppliedDraft({}, observed);
    QCOMPARE(first->storageDialogState().selectedIndex, -1);
    QVERIFY(!first->storageDialogState().dialog.submitEnabled);
    source->reconcileAppliedDraft({}, actions->state);
    QCOMPARE(first->storageDialogState().selectedVolume, QStringLiteral("D:"));
    QVERIFY(first->submitStorageDraft());
    QCOMPARE(source->draft().storageVolume, std::optional{QStringLiteral("volume-d")});
    QCOMPARE(actions->writes.load(), 0);
  }
  void hormDialogCanUndoThePendingIntent_data() {
    QTest::addColumn<bool>("active");
    QTest::newRow("undo-enable") << false;
    QTest::newRow("undo-disable") << true;
  }
  void hormDialogCanUndoThePendingIntent() {
    QFETCH(bool, active);
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.hormEnabled = active;
    auto source = std::make_shared<presentation::OverlayMaintenanceViewModel>(actions);
    source->setInteractionContext(true, false);
    source->reconcileAppliedDraft({}, actions->state);
    QVERIFY(source->stageHorm(!active));
    const auto edit = presentation::OverlayMaintenanceViewModel::editDraft(source);
    QCOMPARE(edit->hormDialogState().disabling, !active);
    QVERIFY(edit->hormDialogState().dialog.submitEnabled);
    QVERIFY(edit->submitHormDraft());
    QVERIFY(!source->draft().hormEnabled);
    QCOMPARE(source->hormActive(), active);
    QCOMPARE(actions->writes.load(), 0);
  }
  void hormDialogAcceptsProtectionWithoutInventingAHormEnableRequest() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.currentFlags = actions->state.nextFlags = app::OverlayFlags{0};
    actions->state.allVolumesProtected = false;
    actions->state.allVolumesNextProtected = false;
    auto source = std::make_shared<presentation::OverlayMaintenanceViewModel>(actions);
    source->setInteractionContext(true, false);
    source->reconcileAppliedDraft({}, actions->state);
    auto edit = presentation::OverlayMaintenanceViewModel::editDraft(source);
    QVERIFY(edit->hormDialogState().protectionVisible);
    QVERIFY(edit->hormDialogState().protectEnabled);
    QVERIFY(!edit->hormDialogState().dialog.submitEnabled);
    QVERIFY(edit->stageAllVolumesProtection(true));
    QVERIFY(edit->hormDialogState().protectionPendingVisible);
    QVERIFY(edit->hormDialogState().dialog.submitEnabled);
    QVERIFY(!edit->hormDialogState().protectEnabled);
    QVERIFY(!source->hasPendingChanges());
    edit.reset();
    QVERIFY(!source->hasPendingChanges());
    edit = presentation::OverlayMaintenanceViewModel::editDraft(source);
    QVERIFY(edit->stageAllVolumesProtection(true));
    QVERIFY(edit->submitHormDraft());
    QCOMPARE(source->draft().allVolumesProtected, std::optional<bool>{true});
    QVERIFY(!source->draft().hormEnabled);
    QVERIFY(!source->canSetHorm(true));
    QCOMPARE(actions->writes.load(), 0);
  }
  void dialogReadFailureRequiresAnotherReadBeforeAcceptingTheSelection() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.filterEnabled = false;
    actions->state.volumesReadable = true;
    actions->state.systemVolumeId = QStringLiteral("volume-c");
    actions->state.volumes = {{QStringLiteral("volume-c"), QStringLiteral("C:"), {}, 0, true, 0, 0}};
    auto source = std::make_shared<presentation::OverlayMaintenanceViewModel>(actions);
    source->setInteractionContext(true, false);
    source->reconcileAppliedDraft({}, actions->state);
    const auto edit = presentation::OverlayMaintenanceViewModel::editDraft(source);
    edit->selectStorageVolume(QStringLiteral("volume-c"));
    const auto ready = actions->state;
    actions->state.filterEnabled.reset();
    actions->state.hormEnabled.reset();
    actions->state.readError = QStringLiteral("Read failed");
    edit->refresh();
    QTRY_VERIFY(!edit->busy());
    QCOMPARE(edit->storageDialogState().dialog.submitLabel, QCoreApplication::translate("uwf", "Read state again"));
    QCOMPARE(edit->storageDialogState().dialog.feedbackTone, presentation::OverlayMaintenanceViewModel::DialogFeedbackTone::Error);
    QVERIFY(!edit->storageDialogState().selectionEnabled);
    actions->state = ready;
    QVERIFY(!edit->submitStorageDraft());
    QVERIFY(!source->hasPendingChanges());
    QTRY_VERIFY(!edit->busy());
    QVERIFY(edit->storageDialogState().selectionEnabled);
    QCOMPARE(edit->storageDialogState().selectedVolume, QStringLiteral("C:"));
    QCOMPARE(actions->writes.load(), 0);
  }
  void blockedVolumeRowsKeepIdentitySeparateFromDisplayNames_data() {
    QTest::addColumn<QString>("name");
    QTest::newRow("no-mount-name") << QString{};
    QTest::newRow("drive-letter") << QStringLiteral("D:");
    QTest::newRow("directory-with-braces") << QStringLiteral("Z:\\Mounted{cache}");
    QTest::newRow("volume-shaped-name") << QStringLiteral("Volume{display-name}");
  }
  void blockedVolumeRowsKeepIdentitySeparateFromDisplayNames() {
    QFETCH(QString, name);
    const auto id = QStringLiteral("11111111-1111-1111-1111-111111111111");
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.blockedVolumes = {{id, name}};
    presentation::OverlayMaintenanceViewModel model(actions);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    const auto rows = model.hormBlockedVolumes();
    QCOMPARE(rows.size(), std::size_t{1});
    QCOMPARE(rows.front().volumeId, id);
    QCOMPARE(rows.front().displayName, name.isEmpty() ? QCoreApplication::translate("uwf", "Partition") : name);
    QCOMPARE(model.snapshot().blockedVolumes.front().name, name);
    QCOMPARE(actions->writes.load(), 0);
  }
  void synchronousNotificationsMayReleaseTheLastOwner_data() {
    QTest::addColumn<int>("boundary");
    QTest::newRow("refresh-busy") << 0;
    QTest::newRow("refresh-state") << 1;
    QTest::newRow("completion-busy") << 2;
    QTest::newRow("completion-state") << 3;
    QTest::newRow("draft-pending") << 4;
    QTest::newRow("reconcile-busy") << 5;
  }
  void synchronousNotificationsMayReleaseTheLastOwner() {
    QFETCH(int, boundary);
    auto actions = std::make_shared<MemoryMaintenance>();
    auto owner = std::make_unique<presentation::OverlayMaintenanceViewModel>(actions);
    owner->setInteractionContext(true, false);
    const QPointer alive(owner.get());
    if (boundary == 4) {
      connect(owner.get(), &presentation::OverlayMaintenanceViewModel::pendingChanged, this, [&] { owner.reset(); });
      app::OverlayMaintenanceChanges draft;
      draft.hormEnabled = true;
      owner->replaceDraft(draft);
    } else if (boundary == 5) {
      actions->gate = std::make_shared<test::WorkerGate>();
      owner->refresh();
      connect(owner.get(), &presentation::OverlayMaintenanceViewModel::busyChanged, this, [&] { owner.reset(); });
      owner->reconcileAppliedDraft({}, actions->state);
    } else {
      if (boundary == 0 || boundary == 2)
        connect(owner.get(), &presentation::OverlayMaintenanceViewModel::busyChanged, this, [&] {
          if (owner && owner->busy() == (boundary == 0)) owner.reset();
        });
      else
        connect(owner.get(), &presentation::OverlayMaintenanceViewModel::stateChanged, this, [&] {
          if (owner && owner->busy() == (boundary == 1)) owner.reset();
        });
      owner->refresh();
    }
    QTRY_VERIFY(!alive);
  }
  void reconciliationCommitsBeforeBusyObserversAndKeepsPendingNotification() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->gate = std::make_shared<test::WorkerGate>();
    presentation::OverlayMaintenanceViewModel model(actions);
    app::OverlayMaintenanceChanges draft;
    draft.hormEnabled = true;
    model.replaceDraft(draft);
    model.refresh();
    QSignalSpy pending(&model, &presentation::OverlayMaintenanceViewModel::pendingChanged);
    QSignalSpy finished(&model, &presentation::OverlayMaintenanceViewModel::refreshFinished);
    bool restarted = false;
    connect(&model, &presentation::OverlayMaintenanceViewModel::busyChanged, &model, [&] {
      if (model.busy() || std::exchange(restarted, true)) return;
      QVERIFY(model.hasSnapshot());
      QCOMPARE(model.snapshot().hormEnabled, std::optional<bool>{true});
      QVERIFY(!model.hasPendingChanges());
      model.refresh();
    });
    auto observed = actions->state;
    observed.hormEnabled = true;
    model.reconcileAppliedDraft(draft, observed);
    QVERIFY(restarted);
    QVERIFY(model.busy());
    QCOMPARE(pending.size(), 1);
    QCOMPARE(finished.size(), 0);
    actions->gate->release();
    QTRY_VERIFY(!model.busy());
    QCOMPARE(finished.size(), 1);
  }
  void hormEnableReportsKnownFailuresBeforeUnavailableRuntimePrerequisites() {
    auto actions = std::make_shared<MemoryMaintenance>();
    auto state = actions->state;
    state.filterEnabled = false;
    state.romDisposition.reset();
    state.allVolumesProtected.reset();
    const auto disabled = app::evaluateHormEligibility(state, true);
    QCOMPARE(disabled.issue, app::HormIssue::FilterDisabled);
    QCOMPARE(disabled.blockingRequirement, std::optional{app::HormRequirement::FilterEnabled});
    QVERIFY(disabled.checklistCoversIssue());
    QVERIFY(app::evaluateHormEligibility(state, false).allowed());
    state.hormEnabled = true;
    QVERIFY(app::evaluateHormEligibility(state, true).allowed());

    state.hormEnabled = false;
    state.filterEnabled = true;
    state.currentOverlayType = 1;
    QCOMPARE(app::evaluateHormEligibility(state, true).issue, app::HormIssue::OverlayNotRam);
    state.currentOverlayType = 0;
    state.currentFlags = app::OverlayFlags{0};
    QCOMPARE(app::evaluateHormEligibility(state, true).issue, app::HormIssue::ConfigurationIncompatible);
    state.currentFlags = app::OverlayFlags{4};
    state.hibernationAvailable = false;
    QCOMPARE(app::evaluateHormEligibility(state, true).issue, app::HormIssue::HibernationUnavailable);
    state.hibernationAvailable = true;
    state.allVolumesProtected = true;
    QCOMPARE(app::evaluateHormEligibility(state, true).issue, app::HormIssue::PrerequisitesUnknown);

    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    state.filterEnabled = false;
    state.allVolumesProtected.reset();
    model.reconcileAppliedDraft({}, state);
    QVERIFY(model.hormDisabledReason(true).contains(QStringLiteral("Enable UWF and restart")));
    QVERIFY(model.hormRestrictionCoveredByChecklist());
  }
  void hormEligibilityAndChecklistShareTheCompleteObservedStateSpace() {
    auto actions = std::make_shared<MemoryMaintenance>();
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    const std::array<std::optional<bool>, 3> booleans{std::nullopt, false, true};
    const std::array<std::optional<std::int64_t>, 4> types{std::nullopt, -1, 0, 1};
    const std::array<std::optional<std::uint32_t>, 3> dispositions{std::nullopt, 0, 1};
    std::vector<std::optional<std::uint32_t>> flagWords{std::nullopt};
    for (std::uint32_t flags = 0; flags != 16; ++flags) flagWords.push_back(flags);
    for (unsigned bit = 4; bit != 32; ++bit) flagWords.push_back(app::OverlayFlags::ReadOnlyMedia | (std::uint32_t{1} << bit));
    for (const auto filter : booleans)
      for (const auto type : types)
        for (const auto flags : flagWords)
          for (const auto hibernation : booleans)
            for (const auto protection : booleans)
              for (const auto disposition : dispositions) {
                auto state = actions->state;
                state.filterEnabled = filter;
                state.currentOverlayType = type;
                state.currentFlags = flags ? std::optional{app::OverlayFlags{*flags}} : std::nullopt;
                state.hibernationAvailable = hibernation;
                state.allVolumesProtected = protection;
                state.romDisposition = disposition;
                const auto eligibility = app::evaluateHormEligibility(state, true);
                const bool expected =
                    filter == true && type == 0 && flags == app::OverlayFlags::ReadOnlyMedia && hibernation == true && protection == true && disposition == 0;
                QCOMPARE(eligibility.allowed(), expected);
                if (eligibility.checklistCoversIssue()) {
                  QVERIFY(!eligibility.allowed());
                  QVERIFY(eligibility.requirements[static_cast<std::size_t>(*eligibility.blockingRequirement)] != true);
                }
                model.reconcileAppliedDraft({}, state);
                QCOMPARE(model.canSetHorm(true), expected);
                QCOMPARE(model.hormDisabledReason(true).isEmpty(), expected);
                QCOMPARE(model.hormRestrictionCoveredByChecklist(), eligibility.checklistCoversIssue());
                QCOMPARE(model.hormEligibility(true).requirements, eligibility.requirements);
                QVERIFY(app::evaluateHormEligibility(state, false).allowed());
                state.hormEnabled = true;
                QVERIFY(app::evaluateHormEligibility(state, true).allowed());
              }
  }

  void storagePresentationPreservesTypedFailuresAndRawDiagnostics() {
    const std::array issues{app::OverlayStorageIssue::ObservationUnavailable, app::OverlayStorageIssue::OverlayTypeUnknown,
                            app::OverlayStorageIssue::MarkerUnavailable, app::OverlayStorageIssue::MultipleHosts,
                            app::OverlayStorageIssue::SystemDriveUnavailable};
    for (const auto issue : issues) {
      app::OverlayStorageLocation location;
      location.issue = issue;
      const auto message = presentation::OverlayMaintenanceViewModel::storageLocationError(location);
      QVERIFY(!message.isEmpty());
      location.diagnostics = QStringLiteral("Provider: 0x80041001 (verbatim)");
      QCOMPARE(presentation::OverlayMaintenanceViewModel::storageLocationError(location), message + u'\n' + location.diagnostics);
    }
  }

  void bulkProtectionRequiresConfirmedConfigurationPersistence() {
    using Issue = app::OverlayMaintenanceEditor::Issue;
    struct Case {
      const char* name;
      std::optional<bool> filter, protectedSystem;
      std::optional<std::uint32_t> flags, disposition;
      bool fullCommit;
      Issue issue;
    };
    const std::array cases{
        Case{"normal", true, true, 0, 0, false, Issue::None},
        Case{"flags-unknown", true, true, {}, 0, false, Issue::ProtectionUnknown},
        Case{"filter-unknown", {}, true, 0, 0, false, Issue::ProtectionUnknown},
        Case{"system-protection-unknown-in-ROM", true, {}, 4, 0, false, Issue::ProtectionUnknown},
        Case{"disposition-unknown-in-ROM", true, true, 4, {}, false, Issue::ProtectionUnknown},
        Case{"filter-off-needs-no-ROM-facts", false, true, {}, {}, false, Issue::None},
        Case{"unprotected-system-needs-no-ROM-facts", true, false, {}, {}, false, Issue::None},
        Case{"ordinary-ROM", true, true, 4, 0, false, Issue::ConfigurationNotPersistent},
        Case{"FullCommit-direct-future-writes", true, true, 4, 4, true, Issue::None},
        Case{"HORM-disabled-disposition-three", true, true, 4, 3, false, Issue::None},
    };
    for (const auto& item : cases) {
      auto actions = std::make_shared<MemoryMaintenance>();
      auto& state = actions->state;
      state.filterEnabled = item.filter;
      state.systemVolumeProtected = item.protectedSystem;
      state.currentFlags = item.flags ? std::optional{app::OverlayFlags{*item.flags}} : std::nullopt;
      state.romDisposition = item.disposition;
      state.overlayCommit = item.fullCommit;
      state.allVolumesNextProtected = state.allVolumesNextUnprotected = false;
      presentation::OverlayMaintenanceViewModel model(actions);
      model.setInteractionContext(true, false);
      model.reconcileAppliedDraft({}, state);
      const bool allowed = item.issue == Issue::None;
      const auto panel = model.panelState();
      QVERIFY2(panel.protectEnabled == allowed, item.name);
      QVERIFY2(panel.unprotectEnabled == allowed, item.name);
      QVERIFY2(panel.manageProtectionEnabled == allowed, item.name);
      QVERIFY2(panel.protectReason.isEmpty() == allowed, item.name);
      QVERIFY2(panel.unprotectReason.isEmpty() == allowed, item.name);
      for (const bool protect : {false, true}) {
        QVERIFY2(model.editor()->allVolumesProtectionRestriction(protect).issue == item.issue, item.name);
        QVERIFY2(model.stageAllVolumesProtection(protect) == allowed, item.name);
        QVERIFY2(model.hasPendingChanges() == allowed, item.name);
        model.discardDraft();
      }
      QCOMPARE(actions->writes.load(), 0);
    }
  }
  void bulkProtectionIsOneReversibleDraftAndDoesNotChangeCurrentReadiness() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.currentFlags = actions->state.nextFlags = app::OverlayFlags{0};
    actions->state.allVolumesProtected = false;
    actions->state.allVolumesNextProtected = false;
    actions->state.allVolumesNextUnprotected = false;
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.stageAllVolumesProtection(true));
    QCOMPARE(model.pendingCount(), std::size_t{1});
    QCOMPARE(model.draft().allVolumesProtected, std::optional<bool>{true});
    QVERIFY(!model.canSetHorm(true));
    const auto origin = model.draft();
    QVERIFY(model.stageAllVolumesProtection(false));
    QCOMPARE(model.pendingCount(), std::size_t{1});
    QCOMPARE(model.draft().allVolumesProtected, std::optional<bool>{false});
    model.replaceDraft(origin);
    QCOMPARE(model.draft().allVolumesProtected, std::optional<bool>{true});
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.draft().allVolumesProtected.has_value());
    auto observed = actions->state;
    observed.allVolumesNextProtected = true;
    observed.allVolumesNextUnprotected = false;
    model.reconcileAppliedDraft({.allVolumesProtected = true}, observed);
    QVERIFY(!model.hasPendingChanges());
    QVERIFY(!model.canSetHorm(true));
    QCOMPARE(model.snapshot().allVolumesProtected, std::optional<bool>{false});
    QCOMPARE(actions->writes.load(), 0);
  }
  void aConfirmedBulkReceiptMayHaveLaterPerVolumeExceptions() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.currentFlags = actions->state.nextFlags = app::OverlayFlags{0};
    actions->state.allVolumesNextProtected = false;
    actions->state.allVolumesNextUnprotected = false;
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.stageAllVolumesProtection(false));
    model.reconcileAppliedDraft({.allVolumesProtected = false}, actions->state);
    QVERIFY(!model.hasPendingChanges());
    auto observed = actions->state;
    observed.allVolumesNextUnprotected = true;
    model.reconcileAppliedDraft({.allVolumesProtected = false}, observed);
    QVERIFY(!model.hasPendingChanges());
    QCOMPARE(actions->writes.load(), 0);
  }
  void anEarlierReadCannotOverwriteAppliedReadback() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.currentFlags = actions->state.nextFlags = app::OverlayFlags{0};
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.stageAllVolumesProtection(true));
    auto gate = actions->gate = std::make_shared<test::WorkerGate>();
    model.refresh();
    QTRY_COMPARE(gate->started.load(), 1);
    auto observed = actions->state;
    observed.allVolumesNextProtected = true;
    observed.allVolumesNextUnprotected = false;
    model.reconcileAppliedDraft({.allVolumesProtected = true}, observed);
    QVERIFY(!model.busy());
    gate->release();
    QTRY_COMPARE(gate->finished.load(), 1);
    QTest::qWait(30);
    QCOMPARE(model.snapshot().allVolumesNextProtected, std::optional<bool>{true});
    QVERIFY(!model.hasPendingChanges());
  }
  void hibernationReasonsDistinguishFullAndReducedFiles() {
    auto actions = std::make_shared<MemoryMaintenance>();
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    const auto verify = [&](app::OverlayHibernationCapabilities capabilities, const QString& expected) {
      actions->state.hibernationCapabilities = capabilities;
      actions->state.hibernationAvailable = capabilities.available();
      model.refresh();
      QTRY_VERIFY(!model.busy());
      QCOMPARE(model.hibernationStateText(), expected);
      QCOMPARE(model.canSetHorm(true), capabilities.available());
    };
    verify({false, true, app::HibernationFileType::Full}, QStringLiteral("Hibernation is not supported"));
    verify({true, false, app::HibernationFileType::None}, QStringLiteral("Hibernation is not enabled"));
    verify({true, true, app::HibernationFileType::Reduced}, QStringLiteral("Fast startup only"));
    verify({true, true, app::HibernationFileType::Full}, QStringLiteral("Met"));
  }
  void fullCommitStaysADraftUntilTheProcessStateIsObserved() {
    auto actions = std::make_shared<MemoryMaintenance>();
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QSignalSpy pending(&model, &presentation::OverlayMaintenanceViewModel::pendingChanged);
    QVERIFY(model.stageFullCommit());
    QVERIFY(model.stageFullCommit());
    QCOMPARE(pending.size(), 1);
    QCOMPARE(model.pendingCount(), std::size_t{1});
    QCOMPARE(model.snapshot().overlayCommit, std::optional<bool>{false});
    QCOMPARE(model.fullCommitLabel(), QStringLiteral("Full volume commit (pending apply)"));
    QCOMPARE(actions->writes.load(), 0);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.draft().fullCommit);
    QVERIFY(model.unstageFullCommit());
    QVERIFY(!model.hasPendingChanges());
    QVERIFY(model.stageFullCommit());

    // A returned task can leave a mode enabled until restart; the bit is not progress.
    auto observed = actions->state;
    observed.overlayCommit = true;
    model.reconcileAppliedDraft({.fullCommit = true}, observed);
    QVERIFY(!model.hasPendingChanges());
    QVERIFY(!model.busy());
    QCOMPARE(model.fullCommitLabel(), QStringLiteral("Full volume commit mode (enabled until restart)"));
    QVERIFY(model.runtimeNotice());
    QCOMPARE(model.runtimeNotice()->text,
             QStringLiteral("Full volume commit mode remains enabled until restart. New changes are saved directly. Individual commits remain unavailable."));
    QVERIFY(!model.fullCommitDialogState().submitVisible);
    QVERIFY(!model.canFullCommit());
    QVERIFY(!model.stageFullCommit());
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QCOMPARE(model.snapshot().overlayCommit, std::optional<bool>{true});
    QCOMPARE(model.fullCommitLabel(), QStringLiteral("Full volume commit mode (enabled until restart)"));
    QVERIFY(model.runtimeNotice());
    QCOMPARE(actions->writes.load(), 0);
  }
  void unknownHormAndPrerequisitesPreventStagingUntilVerified() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.hormEnabled.reset();
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(!model.canSetHorm(true));
    QVERIFY(!model.stageHorm(true));
    QVERIFY(!model.canFullCommit());
    actions->state.hormEnabled = false;
    actions->state.currentFlags.reset();
    actions->state.currentOverlayType.reset();
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(!model.canSetHorm(true));
    QVERIFY(!model.stageHorm(true));
    actions->state.currentFlags = app::OverlayFlags{4};
    actions->state.currentOverlayType = 0;
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.canSetHorm(true));
    QVERIFY(model.stageHorm(true));
    QCOMPARE(model.draft().hormEnabled, std::optional<bool>{true});
    QVERIFY(!model.hormActive());
    QCOMPARE(actions->writes.load(), 0);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.draft().hormEnabled.has_value());
    QVERIFY(model.stageHorm(false));  // Return to the observed value: undo draft.
    QVERIFY(!model.hasPendingChanges());
    QVERIFY(model.stageHorm(true));
    auto observed = actions->state;
    observed.hormEnabled = true;
    model.reconcileAppliedDraft({.hormEnabled = true}, observed);
    QVERIFY(model.hormActive());
    QVERIFY(!model.hasPendingChanges());
    QCOMPARE(actions->writes.load(), 0);
  }
  void disablingHormRemainsPendingAndDoesNotReleaseTheCurrentLock() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.hormEnabled = true;
    actions->state.filterEnabled = false;
    actions->state.currentOverlayType = 1;
    actions->state.currentFlags = app::OverlayFlags{0};
    actions->state.hibernationAvailable = false;
    actions->state.allVolumesProtected = false;
    actions->state.romDisposition.reset();
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.canSetHorm(false));
    QVERIFY(model.stageHorm(false));
    QVERIFY(model.hormActive());
    QVERIFY(!model.restartRequired());
    QCOMPARE(model.draft().hormEnabled, std::optional<bool>{false});
    QCOMPARE(actions->writes.load(), 0);
    auto observed = actions->state;
    observed.hormEnabled = false;
    model.reconcileAppliedDraft({.hormEnabled = false}, observed);
    QVERIFY(!model.hormActive());
    QVERIFY(model.restartRequired());
    QVERIFY(!model.hasPendingChanges());
    actions->state = observed;
    actions->state.filterEnabled = true;
    actions->state.currentOverlayType = 0;
    actions->state.currentFlags = app::OverlayFlags{4};
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(!model.canFullCommit());
    QVERIFY(!model.canSetHorm(true));
  }
  void ordinaryDraftsDoNotPreventStagingAndDiscardNeverWrites() {
    auto actions = std::make_shared<MemoryMaintenance>();
    auto gate = actions->gate = std::make_shared<test::WorkerGate>();
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_COMPARE(gate->started.load(), 1);
    QVERIFY(model.busy());
    model.refresh();
    QVERIFY(!model.stageFullCommit());
    QVERIFY(!model.stageHorm(true));
    QCOMPARE(actions->reads.load(), 1);
    gate->release();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.canFullCommit());
    QVERIFY(model.canSetHorm(true));
    QVERIFY(model.stageFullCommit());
    QVERIFY(model.stageHorm(true));
    QCOMPARE(model.pendingCount(), std::size_t{2});
    const auto saved = model.draft();
    model.discardDraft();
    QVERIFY(!model.hasPendingChanges());
    model.replaceDraft(saved);
    QVERIFY(model.draft() == saved);
    QCOMPARE(model.snapshot().overlayCommit, std::optional<bool>{false});
    QVERIFY(!model.hormActive());
    QCOMPARE(actions->writes.load(), 0);
  }
  void enableRequiresEveryReadOnlyPrerequisiteKnownAndReady_data() {
    QTest::addColumn<int>("condition");
    QTest::addColumn<bool>("unknown");
    QTest::addColumn<quint32>("value");
    const std::array<quint32, 6> incompatible{0, 1, 5, 0, 0, 1};
    for (int condition = 0; condition != 6; ++condition) {
      QTest::newRow(qPrintable(QStringLiteral("condition-%1-unreadable").arg(condition))) << condition << true << quint32{0};
      QTest::newRow(qPrintable(QStringLiteral("condition-%1-not-ready").arg(condition)))
          << condition << false << incompatible[static_cast<std::size_t>(condition)];
    }
    for (const quint32 disposition : {quint32{2}, quint32{3}, quint32{4}, quint32{0x80000000}})
      QTest::newRow(qPrintable(QStringLiteral("nonidle-disposition-%1").arg(disposition))) << 5 << false << disposition;
  }
  void enableRequiresEveryReadOnlyPrerequisiteKnownAndReady() {
    QFETCH(int, condition);
    QFETCH(bool, unknown);
    QFETCH(quint32, value);
    auto actions = std::make_shared<MemoryMaintenance>();
    const auto ready = actions->state;
    switch (condition) {
      case 0:
        actions->state.filterEnabled = unknown ? std::nullopt : std::optional<bool>{value != 0};
        break;
      case 1:
        actions->state.currentOverlayType = unknown ? std::nullopt : std::optional<std::int64_t>{value};
        break;
      case 2:
        actions->state.currentFlags = unknown ? std::nullopt : std::optional<app::OverlayFlags>{app::OverlayFlags{value}};
        break;
      case 3:
        actions->state.hibernationAvailable = unknown ? std::nullopt : std::optional<bool>{value != 0};
        break;
      case 4:
        actions->state.allVolumesProtected = unknown ? std::nullopt : std::optional<bool>{value != 0};
        break;
      case 5:
        actions->state.romDisposition = unknown ? std::nullopt : std::optional<std::uint32_t>{value};
        break;
    }
    // False only excludes Full Commit's bit. Dispositions1/2/3 and opaque
    // nonzero bits still prohibit preparation for HORM.
    QCOMPARE(actions->state.overlayCommit, std::optional<bool>{false});
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(!model.canSetHorm(true));
    QVERIFY(!model.hormDisabledReason(true).isEmpty());
    QVERIFY(!model.stageHorm(true));
    QVERIFY(model.canSetHorm(false));
    QVERIFY(!model.hasPendingChanges());
    QCOMPARE(actions->writes.load(), 0);
    actions->state = ready;
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.canSetHorm(true));
    QVERIFY(model.stageHorm(true));
    QCOMPARE(actions->writes.load(), 0);
  }
  void selectingTheSameStorageHostRemainsAnExplicitIntentUntilConfirmedReadback() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.filterEnabled = false;
    actions->state.nextOverlayType = 1;
    actions->state.maximumOverlaySize = 0;  // Can be corrected in the combined draft.
    actions->state.volumesReadable = true;
    actions->state.volumes = {{QStringLiteral("host-a"), QStringLiteral("C:"), {}, 0, true, 1, 1},
                              {QStringLiteral("host-b"), QStringLiteral("D:"), {}, 0, true, 0, 0}};
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.stageStorageLocation(QStringLiteral("host-a")));
    QCOMPARE(model.pendingCount(), std::size_t{1});
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.draft().storageVolume.has_value());

    auto observed = actions->state;
    model.reconcileAppliedDraft({}, observed);  // An unrelated receipt cannot clear this intent.
    QVERIFY(model.draft().storageVolume.has_value());
    observed.nextOverlayType = 0;
    model.reconcileAppliedDraft({.storageVolume = QStringLiteral("host-a")}, observed);
    QVERIFY(model.draft().storageVolume.has_value());
    observed.nextOverlayType = 1;
    observed.volumes[0].nextMarker = 0;
    model.reconcileAppliedDraft({.storageVolume = QStringLiteral("host-a")}, observed);
    QVERIFY(model.draft().storageVolume.has_value());
    observed.volumes[0].nextMarker = 1;
    model.reconcileAppliedDraft({.storageVolume = QStringLiteral("host-a")}, observed);
    QVERIFY(!model.hasPendingChanges());
    QVERIFY(model.swapRestartRequired());
    QCOMPARE(actions->writes.load(), 0);
  }
  void confirmationDoesNotDiscardANewerStorageIntent() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.filterEnabled = false;
    actions->state.nextOverlayType = 1;
    actions->state.volumesReadable = true;
    actions->state.volumes = {{QStringLiteral("host-a"), QStringLiteral("C:"), {}, 0, true, 0, 0},
                              {QStringLiteral("host-b"), QStringLiteral("D:"), {}, 0, true, 0, 0}};
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.stageStorageLocation(QStringLiteral("host-a")));
    QVERIFY(model.stageStorageLocation(QStringLiteral("host-b")));
    auto observed = actions->state;
    observed.volumes[0].nextMarker = 1;
    model.reconcileAppliedDraft({.storageVolume = QStringLiteral("host-a")}, observed);
    QCOMPARE(model.draft().storageVolume, std::optional<QString>{QStringLiteral("host-b")});
    model.discardDraft();
    QCOMPARE(actions->writes.load(), 0);
  }
  void failedWorkflowReadbackDoesNotClearDraftOnRefresh_data() {
    QTest::addColumn<bool>("horm");
    QTest::newRow("storage-partial-marker-match") << false;
    QTest::newRow("horm-disable-partial-flag-match") << true;
  }
  void failedWorkflowReadbackDoesNotClearDraftOnRefresh() {
    QFETCH(bool, horm);
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.hormEnabled = horm;
    if (!horm) {
      actions->state.filterEnabled = false;
      actions->state.nextOverlayType = 0;
      actions->state.volumesReadable = true;
      actions->state.volumes = {{QStringLiteral("host-a"), QStringLiteral("D:"), {}, 0, true, 0, 0}};
    }
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(horm ? model.stageHorm(false) : model.stageStorageLocation(QStringLiteral("host-a")));
    const auto requested = model.draft();
    auto observed = actions->state;
    if (horm)
      observed.hormEnabled = false;
    else {
      observed.nextOverlayType = 1;
      observed.volumes.front().nextMarker = 1;
    }
    const app::OverlayMaintenanceResult failed{observed, QStringLiteral("Workflow failed after dispatch"),
                                               app::OperationFailure{app::OperationFailureKind::Failed, app::WriteEffect::Uncertain, {}},
                                               app::WriteEffect::Uncertain, false};
    QVERIFY(!failed.succeeded());
    // MainWindow passes only successful operation intents as confirmed. A
    // failed step can still supply its matching, diagnostically useful state.
    model.reconcileAppliedDraft({}, failed.snapshot);
    QCOMPARE(model.draft(), requested);
    actions->state = observed;
    for (int refresh = 0; refresh != 2; ++refresh) {
      model.refresh();
      QTRY_VERIFY(!model.busy());
      QCOMPARE(model.draft(), requested);
      QCOMPARE(model.pendingCount(), std::size_t{1});
    }
    if (horm) QVERIFY(model.restartRequired());
    QCOMPARE(actions->writes.load(), 0);
    model.reconcileAppliedDraft(requested, observed);
    QVERIFY(!model.hasPendingChanges());
  }
  void observingStartedFullCommitStillRemovesItsDraftAndPreventsRepeats() {
    auto actions = std::make_shared<MemoryMaintenance>();
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(model.stageFullCommit());
    actions->state.overlayCommit = true;
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(!model.draft().fullCommit);
    QVERIFY(!model.canFullCommit());
    QVERIFY(!model.stageFullCommit());
    QCOMPARE(actions->writes.load(), 0);
  }
  void nextRomDoesNotBecomeCurrentRom() {
    auto actions = std::make_shared<MemoryMaintenance>();
    actions->state.currentFlags = app::OverlayFlags{0};
    presentation::OverlayMaintenanceViewModel model(actions);
    model.setInteractionContext(true, false);
    model.refresh();
    QTRY_VERIFY(!model.busy());
    QVERIFY(!model.currentRomLoaded());
    QVERIFY(!model.stageFullCommit());
    QCOMPARE(actions->writes.load(), 0);
  }
};
}  // namespace
QTEST_GUILESS_MAIN(OverlayMaintenanceViewModelTests)
#include "OverlayMaintenanceViewModelTests.moc"
