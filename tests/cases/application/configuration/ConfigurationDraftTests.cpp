// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QPointer>
#include <QTest>
#include <limits>
#include <memory>
#include <optional>
#include <string>

#include "application/configuration/editing/ConfigurationDraft.h"
#include "application/configuration/editing/ConfigurationEditSession.h"
#include "application/configuration/editing/GlobalConfigurationEditor.h"
#include "application/configuration/exclusions/ExclusionDraft.h"
#include "application/configuration/volumes/VolumeConfigurationEditor.h"
#include "application/staging/contracts/FileStagingStore.h"
#include "application/state/ApplicationStateSession.h"

namespace {

class ConfigurationDraftTests final : public QObject {
  Q_OBJECT

 private slots:
  void modeAdmissionKeepsFilterExitSeparateFromOrdinaryConfiguration() {
    using namespace uwf;
    constexpr auto id = "11111111-2222-3333-4444-555555555555";
    const core::VolumeInfo physical{"C:", std::string("Volume{") + id + "}", "NTFS", "System", 1000, 500,
                                    core::VolumeSupport::Supported, {true, true}};
    struct Case { const char* name; bool filter, systemProtected; std::uint32_t disposition; bool fullCommit, ordinary, thresholds; };
    const Case cases[] = {
        {"protected-ROM", true, true, 0, false, false, false},
        {"unprotected-system-ROM", true, false, 0, false, true, true},
        {"filter-off", false, true, 0, false, true, true},
        {"HORM-disabled-same-boot", true, true, 3, false, true, true},
        {"FullCommit-future-writes", true, true, 4, true, true, false},
    };
    for (const auto& item : cases) {
      app::ConfigurationEditSession session(8192, QStringLiteral("C:"));
      const auto volume = session.editVolume(physical, true);
      core::UwfSnapshot state;
      state.uwfAvailable = state.elevated = true;
      state.current.filter.enabled = item.filter;
      state.current.overlay = {.type = core::OverlayType::RAM, .maximumSizeMb = 2048, .warningThresholdMb = 64, .criticalThresholdMb = 512};
      state.current.volumes = {{physical.volumeName, "C:", item.systemProtected, core::VolumeBinding::VolumeId}};
      state.next = state.current;
      state.next.filter.enabled = true;
      state.next.volumes[0].isProtected = true;
      state.runtime.hormEnabled = false;
      state.runtime.currentOverlayFlags = 4;
      state.runtime.systemVolumeProtected = item.systemProtected;
      state.runtime.romDisposition = item.disposition;
      state.runtime.fullCommit = item.fullCommit;
      app::OverlayPolicySnapshot policy;
      policy.transportAvailable = true;
      policy.current.filterEnabled = {app::PolicyObservationStatus::Available, item.filter, {}};
      policy.current.overlayType = {app::PolicyObservationStatus::Available, 0, {}};
      policy.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{4}, {}};
      policy.next = policy.current;
      policy.next.filterEnabled.value = true;
      session.resetSnapshot(state, policy, false);
      const auto global = session.globalEditor();
      QVERIFY2(global->editingAllowed(), item.name);
      global->editFilterEnabled(false);
      QCOMPARE(global->pendingFilterEnabled(), std::optional{false});
      QVERIFY2(volume->status()->controlsEnabled() == item.ordinary, item.name);
      QVERIFY2(volume->files()->readOnly() == !item.ordinary, item.name);
      QVERIFY2(session.registryEditor()->readOnly() == !item.ordinary, item.name);
      QVERIFY2(global->thresholdEditingAllowed() == item.thresholds, item.name);
      global->editWarningThreshold(128);
      QCOMPARE(global->pendingOverlay().warningThresholdMb.has_value(), item.thresholds);
      QCOMPARE(volume->status()->editProtection(false), item.ordinary);
      QCOMPARE(session.registryEditor()->importAdd(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Example")) == app::ExclusionImportOutcome::Applied,
               item.ordinary);
    }
  }
  void editorNotificationsMayReleaseTheOwner_data() {
    QTest::addColumn<QString>("operation");
    for (const auto* operation :
         {"global-reset", "global-edit", "status-reset", "status-unsupported", "status-protection-state", "status-protection-availability", "status-binding",
          "exclusion-baseline", "exclusion-restoration", "exclusion-probe-refresh", "exclusion-probe-remove", "exclusion-probe-import"})
      QTest::newRow(operation) << QString::fromLatin1(operation);
  }
  void editorNotificationsMayReleaseTheOwner() {
    QFETCH(QString, operation);
    using namespace uwf;
    if (operation.startsWith(QStringLiteral("global-"))) {
      auto owner = std::make_unique<app::GlobalConfigurationEditor>();
      owner->reset({});
      if (operation == QStringLiteral("global-reset")) owner->setAvailable(false);
      QPointer lifetime(owner.get());
      const auto release = [&] { owner.reset(); };
      if (operation == QStringLiteral("global-reset"))
        connect(owner.get(), &app::GlobalConfigurationEditor::availabilityChanged, this, release);
      else
        connect(owner.get(), &app::GlobalConfigurationEditor::stateChanged, this, release);
      if (operation == QStringLiteral("global-reset"))
        owner->reset({});
      else
        owner->editFilterEnabled(true);
      QVERIFY(!lifetime);
      return;
    }
    if (operation.startsWith(QStringLiteral("status-"))) {
      auto owner = std::make_unique<app::VolumeStatusEditor>();
      const core::VolumeRecord record{"Volume{11111111-2222-3333-4444-555555555555}", "C:", false, core::VolumeBinding::DriveLetter};
      const core::VolumeInfo physical{"C:", record.volumeName, "NTFS", {}, 1000, 500, core::VolumeSupport::Supported, {true, true}};
      const bool protection = operation.startsWith(QStringLiteral("status-protection"));
      owner->reset(&record, protection ? nullptr : &record, true, &physical);
      QPointer lifetime(owner.get());
      const auto release = [&] { owner.reset(); };
      if (operation == QStringLiteral("status-protection-availability"))
        connect(owner.get(), &app::VolumeStatusEditor::availabilityChanged, this, release);
      else
        connect(owner.get(), &app::VolumeStatusEditor::stateChanged, this, release);
      if (operation == QStringLiteral("status-reset"))
        owner->reset(&record, &record);
      else if (operation == QStringLiteral("status-unsupported"))
        owner->setUnsupported();
      else if (protection)
        QVERIFY(owner->editProtection(true));
      else
        QVERIFY(owner->editBinding(core::VolumeBinding::VolumeId));
      QVERIFY(!lifetime);
      return;
    }
    auto owner = std::make_unique<app::ExclusionEditor>(app::ExclusionEditor::Kind::Registry);
    const auto root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), static_cast<qsizetype>(app::kFileStagingRegistryRoot.size()));
    owner->setBaseline({}, {root});
    QPointer lifetime(owner.get());
    const auto release = [&] { owner.reset(); };
    if (operation == QStringLiteral("exclusion-baseline")) {
      connect(owner.get(), &app::ExclusionEditor::baselineChanged, this, release);
      owner->setBaseline({}, {});
    } else if (operation == QStringLiteral("exclusion-restoration")) {
      QVERIFY(owner->removeSelection({{root, {}}}));
      connect(owner.get(), &app::ExclusionEditor::stateChanged, this, release);
      owner->setFileStagingDataProbe([] { return true; });
    } else {
      bool armed = false;
      owner->setFileStagingDataProbe([&] {
        if (armed) release();
        return false;
      });
      armed = true;
      if (operation == QStringLiteral("exclusion-probe-refresh"))
        owner->refreshFileStagingConstraints();
      else if (operation == QStringLiteral("exclusion-probe-remove"))
        QVERIFY(!owner->removeSelection({{root, {}}}));
      else
        QCOMPARE(owner->importRemove(root), app::ExclusionImportOutcome::RejectedReadOnly);
    }
    QVERIFY(!lifetime);
  }
  void volumePublicationsExposeOneCompleteObservation_data() {
    QTest::addColumn<QString>("operation");
    QTest::addColumn<QString>("notification");
    QTest::addColumn<bool>("releaseOwner");
    for (const auto* operation : {"snapshot", "admission"}) {
      const QStringList notifications =
          std::string_view(operation) == "snapshot"
              ? QStringList{"status-state",   "status-availability", "files-baseline",     "files-state",           "files-pending", "registry-baseline",
                            "registry-state", "registry-pending",    "files-availability", "registry-availability", "facade"}
              : QStringList{"status-availability", "files-availability", "registry-availability", "facade"};
      for (const auto& notification : notifications)
        for (const bool release : {false, true})
          QTest::newRow(qUtf8Printable(
              QStringLiteral("%1/%2/%3").arg(QString::fromLatin1(operation), notification, release ? QStringLiteral("release") : QStringLiteral("reenter"))))
              << QString::fromLatin1(operation) << notification << release;
    }
  }
  void volumePublicationsExposeOneCompleteObservation() {
    QFETCH(QString, operation);
    QFETCH(QString, notification);
    QFETCH(bool, releaseOwner);
    using namespace uwf;
    const std::string volumeName = "Volume{11111111-2222-3333-4444-555555555555}";
    const core::VolumeInfo physical{"C:", volumeName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
    auto owner = std::make_unique<app::VolumeConfigurationEditor>(physical, true);
    QPointer lifetime(owner.get());
    QPointer status(owner->status().get());
    QPointer files(owner->files().get());
    QPointer registry(owner->registry().get());
    core::UwfSnapshot first;
    first.uwfAvailable = true;
    first.elevated = operation == QStringLiteral("admission");
    first.current.volumes = {{volumeName, "C:", true, core::VolumeBinding::DriveLetter}};
    first.next = first.current;
    first.next.fileExclusions[volumeName] = {"\\First"};
    first.next.registryExclusions = {"HKLM\\SOFTWARE\\First"};
    auto latest = first;
    latest.elevated = true;
    latest.next.fileExclusions[volumeName] = {"\\Latest"};
    latest.next.registryExclusions = {"HKLM\\SOFTWARE\\Latest"};
    if (operation == QStringLiteral("admission")) owner->applySnapshot(first, false);
    bool observed = false;
    const auto receive = [&] {
      if (std::exchange(observed, true)) return;
      QCOMPARE(files->draft().next(), QStringList{QStringLiteral("C:\\First")});
      QCOMPARE(registry->draft().next(), QStringList{QStringLiteral("HKLM\\SOFTWARE\\First")});
      QVERIFY(!status->controlsEnabled());
      QVERIFY(files->readOnly());
      QVERIFY(registry->readOnly());
      if (releaseOwner)
        owner.reset();
      else if (operation == QStringLiteral("snapshot"))
        owner->applySnapshot(latest, false);
      else
        owner->setEditingAllowed(true);
    };
    if (notification == QStringLiteral("status-state"))
      connect(status, &app::VolumeStatusEditor::stateChanged, this, receive);
    else if (notification == QStringLiteral("status-availability"))
      connect(status, &app::VolumeStatusEditor::availabilityChanged, this, receive);
    else if (notification == QStringLiteral("files-baseline"))
      connect(files, &app::ExclusionEditor::baselineChanged, this, receive);
    else if (notification == QStringLiteral("files-state"))
      connect(files, &app::ExclusionEditor::stateChanged, this, receive);
    else if (notification == QStringLiteral("files-pending"))
      connect(files, &app::ExclusionEditor::pendingChanged, this, receive);
    else if (notification == QStringLiteral("registry-baseline"))
      connect(registry, &app::ExclusionEditor::baselineChanged, this, receive);
    else if (notification == QStringLiteral("registry-state"))
      connect(registry, &app::ExclusionEditor::stateChanged, this, receive);
    else if (notification == QStringLiteral("registry-pending"))
      connect(registry, &app::ExclusionEditor::pendingChanged, this, receive);
    else if (notification == QStringLiteral("files-availability"))
      connect(files, &app::ExclusionEditor::editingAvailabilityChanged, this, receive);
    else if (notification == QStringLiteral("registry-availability"))
      connect(registry, &app::ExclusionEditor::editingAvailabilityChanged, this, receive);
    else
      connect(owner.get(), &app::VolumeConfigurationEditor::stateChanged, this, receive);
    if (operation == QStringLiteral("snapshot"))
      owner->applySnapshot(first, false);
    else
      owner->setEditingAllowed(false);
    QVERIFY(observed);
    if (releaseOwner) {
      QVERIFY(!lifetime);
      QVERIFY(!status);
      QVERIFY(!files);
      QVERIFY(!registry);
    } else {
      QVERIFY(status->controlsEnabled());
      QVERIFY(!files->readOnly());
      QVERIFY(!registry->readOnly());
      QCOMPARE(files->draft().next(), QStringList{operation == QStringLiteral("snapshot") ? QStringLiteral("C:\\Latest") : QStringLiteral("C:\\First")});
      QCOMPARE(registry->draft().next(),
               QStringList{operation == QStringLiteral("snapshot") ? QStringLiteral("HKLM\\SOFTWARE\\Latest") : QStringLiteral("HKLM\\SOFTWARE\\First")});
    }
  }

  void capacityEditingUsesWritableBoundsWithoutRepairingObservedZero() {
    using namespace uwf;
    app::GlobalConfigurationEditor editor;
    core::SessionSnapshot next;
    next.overlay = {.type = core::OverlayType::RAM, .maximumSizeMb = 0};
    editor.reset(next);
    QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{0});
    QCOMPARE(editor.maximumRange().minimum, uint32_t{0});
    QVERIFY(editor.pendingOverlay().empty());
    editor.editWarningThreshold(0);
    QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{0});
    QVERIFY(editor.pendingOverlay().empty());
    editor.editMaximumSize(1);
    QCOMPARE(editor.pendingOverlay().maximumSizeMb, std::optional<uint32_t>{1});
    next.overlay.maximumSizeMb = 2048;
    editor.reset(next);
    editor.editMaximumSize(0);
    QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{1});
    next.overlay.type = core::OverlayType::Disk;
    editor.reset(next);
    editor.editMaximumSize(0);
    QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{1024});
  }
  void editSessionProjectsFieldIntentAndVolumeBoundaries();
  void editSessionAccumulatesRepeatedVolumeEntries();
  void exclusionDraftReversesEditsWithoutChangingTheBaseline();
  void applicationStateSessionPreservesReferencesAndReconciliationGates();
};

void ConfigurationDraftTests::editSessionProjectsFieldIntentAndVolumeBoundaries() {
  uwf::app::GlobalConfigurationDraft global;
  global.baselineFilter.enabled = true;
  global.filterEnabled = true;
  global.baselineOverlay = {.type = uwf::core::OverlayType::RAM, .maximumSizeMb = 4096, .warningThresholdMb = 1000, .criticalThresholdMb = 3000};
  global.overlay = global.baselineOverlay;
  QVERIFY(!global.pendingFilterEnabled());
  QVERIFY(global.pendingOverlay().empty());
  global.overlay.maximumSizeMb = 8192;
  global.overlay.warningThresholdMb = 0;
  const auto overlay = global.pendingOverlay();
  QVERIFY(!overlay.type);
  QCOMPARE(overlay.maximumSizeMb, std::optional<uint32_t>{8192});
  QCOMPARE(overlay.warningThresholdMb, std::optional<uint32_t>{0});
  QVERIFY(!overlay.criticalThresholdMb);

  uwf::app::ConfigurationEditSession session;
  session.global()->draft = global;
  session.global()->registry.tscal.pendingNext = false;
  uwf::core::VolumeInfo volume;
  volume.driveLetter = "C:";
  const auto first = session.addVolume(volume);
  first->draft.hasTarget = true;
  first->draft.baselineProtected = true;
  first->draft.protectedVolume = false;
  QVERIFY(first->files.add(QStringLiteral("\\Data")));
  volume.driveLetter = "D:";
  volume.support = uwf::core::VolumeSupport::NotFixedLocalDisk;
  const auto unsupported = session.addVolume(volume);
  unsupported->draft = first->draft;
  unsupported->files = first->files;
  volume.driveLetter = "E:";
  volume.support = uwf::core::VolumeSupport::Supported;
  session.addVolume(volume);
  const auto changes = session.pendingChanges();
  QCOMPARE(changes.volumeProtect.size(), std::size_t{1});
  QCOMPARE(changes.volumeProtect.at("C:"), false);
  QCOMPARE(changes.addFileExclusions.size(), std::size_t{1});
  QCOMPARE(changes.addFileExclusions.at("C:"), (std::vector<std::string>{R"(C:\Data)"}));
  QVERIFY(!changes.removeFileExclusions.contains("E:"));
  QCOMPARE(changes.setPersistTSCAL, std::optional<bool>{false});
  QCOMPARE(changes.count(), std::size_t{5});
  session.global()->available = false;
  const auto unavailable = session.pendingChanges();
  QVERIFY(unavailable.setOverlay.empty());
  QVERIFY(!unavailable.setPersistTSCAL);
  QCOMPARE(unavailable.volumeProtect, changes.volumeProtect);
  QCOMPARE(unavailable.addFileExclusions, changes.addFileExclusions);
  const auto ram = uwf::app::overlaySizeBounds(uwf::core::OverlayType::RAM, 8192, std::numeric_limits<uint32_t>::max());
  QCOMPARE(ram.minimum, uint32_t{1});
  QCOMPARE(ram.maximum, uint32_t{8192});
  const auto disk = uwf::app::overlaySizeBounds(uwf::core::OverlayType::Disk, 8192, std::numeric_limits<uint32_t>::max());
  QCOMPARE(disk.minimum, uint32_t{1024});
  QCOMPARE(disk.maximum, std::numeric_limits<uint32_t>::max());
  QCOMPARE(uwf::app::strictThresholdUpper(1), uint32_t{0});
}

void ConfigurationDraftTests::editSessionAccumulatesRepeatedVolumeEntries() {
  uwf::app::ConfigurationEditSession session;
  // This fixture has no global snapshot; only volume exclusions are pending.
  session.global()->available = false;
  uwf::core::VolumeInfo volume;
  volume.driveLetter = "C:";
  const auto first = session.addVolume(volume);
  first->files.setBaseline({}, {QStringLiteral("\\OldFirst")});
  QVERIFY(first->files.add(QStringLiteral("\\First")));
  QVERIFY(first->files.remove(QStringLiteral("\\OldFirst")));
  const auto second = session.addVolume(volume);
  second->files.setBaseline({}, {QStringLiteral("\\OldSecond"), QStringLiteral("\\OldFirst")});
  QVERIFY(second->files.add(QStringLiteral("\\Second")));
  QVERIFY(second->files.add(QStringLiteral("\\First")));
  QVERIFY(second->files.remove(QStringLiteral("\\OldSecond")));
  QVERIFY(second->files.remove(QStringLiteral("\\OldFirst")));
  session.addVolume(volume);
  volume.support = uwf::core::VolumeSupport::NotFixedLocalDisk;
  const auto unsupported = session.addVolume(volume);
  unsupported->files = first->files;

  const auto changes = session.pendingChanges();
  QCOMPARE(changes.addFileExclusions.size(), std::size_t{1});
  QCOMPARE(changes.removeFileExclusions.size(), std::size_t{1});
  QCOMPARE(changes.addFileExclusions.at("C:"), (std::vector<std::string>{R"(C:\First)", R"(C:\Second)"}));
  QCOMPARE(changes.removeFileExclusions.at("C:"), (std::vector<std::string>{R"(C:\OldFirst)", R"(C:\OldSecond)"}));
  QCOMPARE(changes.count(), std::size_t{4});
}

void ConfigurationDraftTests::exclusionDraftReversesEditsWithoutChangingTheBaseline() {
  uwf::app::ExclusionDraft draft;
  draft.setBaseline({QStringLiteral("\\Current")}, {QStringLiteral("\\Data")});
  QVERIFY(!draft.add(QStringLiteral("\\data")));
  QVERIFY(draft.remove(QStringLiteral("\\DATA")));
  QVERIFY(draft.add(QStringLiteral("\\data")));
  QVERIFY(draft.removed().isEmpty());
  QVERIFY(draft.add(QStringLiteral("\\New")));
  QVERIFY(draft.remove(QStringLiteral("\\new")));
  QVERIFY(draft.added().isEmpty());
  QCOMPARE(draft.next(), QStringList{QStringLiteral("\\Data")});
  draft.domainSecretKey.baseNext = true;
  draft.togglePersistence(draft.domainSecretKey);
  QCOMPARE(draft.domainSecretKey.pendingNext, std::optional<bool>{false});
  draft.enablePersistence(draft.domainSecretKey);
  QVERIFY(!draft.domainSecretKey.pendingNext);
  draft.enablePersistence(draft.tscal);
  QCOMPARE(draft.tscal.pendingNext, std::optional<bool>{true});
  draft.reset();
  QVERIFY(!draft.tscal.pendingNext);
  QCOMPARE(draft.current(), QStringList{QStringLiteral("\\Current")});
}

void ConfigurationDraftTests::applicationStateSessionPreservesReferencesAndReconciliationGates() {
  uwf::app::ApplicationStateSession session;
  const auto* boundSnapshot = &session.snapshot();
  QVERIFY(!session.hasCommittedState());
  QVERIFY(!session.configurationWritesAllowed());
  uwf::app::ApplicationState state;
  state.snapshot.uwfAvailable = true;
  state.snapshot.elevated = true;
  state.snapshot.next.overlay.maximumSizeMb = 4096;
  session.commit(std::move(state));
  QCOMPARE(&session.snapshot(), boundSnapshot);
  QVERIFY(session.configurationWritesAllowed());
  session.requireReconciliation();
  QVERIFY(session.reconciliationRequired());
  QVERIFY(!session.configurationWritesAllowed());
  QCOMPARE(boundSnapshot->next.overlay.maximumSizeMb, uint32_t{4096});
  uwf::app::ApplicationState unavailable;
  session.commit(std::move(unavailable));
  QCOMPARE(&session.snapshot(), boundSnapshot);
  QVERIFY(session.hasCommittedState());
  QVERIFY(!session.reconciliationRequired());
  QVERIFY(!session.configurationWritesAllowed());
}

}  // namespace

QTEST_APPLESS_MAIN(ConfigurationDraftTests)

#include "ConfigurationDraftTests.moc"
