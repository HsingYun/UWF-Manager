// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QPair>
#include <QTest>
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

#include "application/commit/CommitTarget.h"
#include "application/commit/CommitWorkflow.h"
#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/identity/contracts/ObjectIdentity.h"
#include "application/operations/contracts/OperationError.h"
#include "application/registry/RegistryBatchCommitter.h"
#include "application/staging/execution/FileStagingCommitter.h"
#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "application/uwf/adapters/UwfOverlayMaintenance.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "domain/uwf/solvers/UwfFailureSolver.h"
#include "simulation/uwf/FakeUwfBackend.h"
#include "support/app/FileTargetFixture.h"

namespace {
using namespace uwf;
using namespace uwf::app;

class Volume final : public UwfVolumeTarget {
 public:
  core::VolumeRecord state{"Volume{11111111-2222-3333-4444-555555555555}", "C:", true};
  mutable QStringList committed;
  WriteEffect receipt = WriteEffect::Confirmed;
  const core::VolumeRecord& volume() const override { return state; }
  std::vector<std::string> exclusions() const override { return {}; }
  WriteEffect commitFile(const std::string& path) const override {
    committed.append(QString::fromStdString(path));
    return receipt;
  }
  WriteEffect commitFileDeletion(const std::string&) const override { throw std::logic_error("unexpected deletion"); }
};
class Registry final : public UwfRegistryMaintenanceTarget {
 public:
  mutable int commits = 0;
  WriteEffect receipt = WriteEffect::Confirmed;
  std::uint32_t currentControlSet() const override { throw std::logic_error("unexpected control-set read"); }
  std::vector<std::string> exclusions() const override { return {}; }
  WriteEffect commit(const std::string&, const std::string&) const override {
    ++commits;
    return receipt;
  }
  WriteEffect commitDeletion(const std::string&, const std::optional<std::string>&) const override { throw std::logic_error("unexpected deletion"); }
};
class Repository final : public UwfMaintenanceRepository {
 public:
  std::shared_ptr<Volume> target = std::make_shared<Volume>();
  std::shared_ptr<Registry> keys = std::make_shared<Registry>();
  mutable int volumeReads = 0, registryReads = 0;
  CommitRequirement fileCommitRequirement(const std::string&) const override { return CommitRequirement::RequiresUwfCommit; }
  CommitRequirement registryCommitRequirement() const override { return CommitRequirement::RequiresUwfCommit; }
  AutomaticCommitDisposition automaticFileCommitDisposition(const std::string&) const override { return AutomaticCommitDisposition::CheckDurability; }
  CommitRequirement registryWriteRequirement() const override { return CommitRequirement::RequiresUwfCommit; }
  std::vector<std::shared_ptr<UwfVolumeTarget>> prepareVolumeTargets() const override {
    ++volumeReads;
    return {target};
  }
  std::shared_ptr<UwfRegistryMaintenanceTarget> prepareRegistryMaintenance() const override {
    ++registryReads;
    return keys;
  }
  std::shared_ptr<UwfRegistryExclusionConfiguration> prepareRegistryExclusions() const override {
    throw std::logic_error("unexpected exclusion configuration");
  }
  std::function<void()> preparePowerAction(PowerAction) const override { throw std::logic_error("unexpected power request"); }
};
class Files final : public CommitTraversalSource {
 public:
  std::shared_ptr<uwf::simulation::FakeUwfBackend> system;
  std::string volume = test::application::fixtureVolumeId;
  std::optional<CommitNode> inspect(const CommitNode& node) const override { return exists(node) ? std::optional{node} : std::nullopt; }
  bool exists(const CommitNode& node) const override {
    return !system || system->visibleFile(volume, toUwfFilePath(node.path.toStdString(), "C:")).has_value();
  }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  FileCommitTarget resolveFileTarget(const QString& path) const override { return test::application::memoryFileTarget(path, volume); }
  QString enumerateChildren(const CommitNode&, std::stop_token, const CommitNodeVisitor&) const override { return {}; }
};

class ScanningFiles final : public CommitTraversalSource {
 public:
  const QString root = QStringLiteral("C:\\Scan");
  const QString branch = root + QStringLiteral("\\Nested");
  const QString leaf = root + QStringLiteral("\\one.bin");
  const QString nested = branch + QStringLiteral("\\two.bin");
  std::string volume = test::application::fixtureVolumeId;
  bool failBranch = false;
  bool empty = false;
  mutable int resolutions = 0;
  mutable std::map<QString, int> inspections;
  std::optional<CommitNode> inspect(const CommitNode& target) const override {
    ++inspections[target.path];
    auto node = target.fileTarget ? target : bindFileCommitTarget(*this, target);
    node.kind = node.path == root || node.path == branch ? CommitTargetKind::Folder : CommitTargetKind::File;
    return node;
  }
  bool exists(const CommitNode&) const override { return true; }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  FileCommitTarget resolveFileTarget(const QString& path) const override {
    ++resolutions;
    return test::application::memoryFileTarget(path, volume);
  }
  QString enumerateChildren(const CommitNode& node, std::stop_token stop, const CommitNodeVisitor& discover) const override {
    const auto child = [&](const QString& path, const CommitTargetKind kind) {
      discover({kind, path, {}, {}, test::application::memoryFileTarget(path, node.fileTarget->volumeId)});
    };
    if (stop.stop_requested() || empty) return {};
    if (node.path == root) {
      child(branch, CommitTargetKind::Folder);
      child(leaf, CommitTargetKind::File);
    } else if (node.path == branch) {
      if (failBranch) return QStringLiteral("directory temporarily unavailable");
      child(nested, CommitTargetKind::File);
    }
    return {};
  }
};

// A controlled enumeration over the shared Fake's actual visible data. Changing
// the published spellings models the registry changing after preparation.
class RegistryValues final : public CommitTraversalSource {
 public:
  std::shared_ptr<uwf::simulation::FakeUwfBackend> system;
  QString key;
  QStringList names;
  QStringList subkeys;
  std::map<QString, QStringList, ObjectNameLess> valuesByKey;
  mutable int treeReads = 0;
  std::optional<CommitNode> inspect(const CommitNode& node) const override { return exists(node) ? std::optional{node} : std::nullopt; }
  bool exists(const CommitNode& node) const override {
    return node.kind == CommitTargetKind::RegistryKey ? system->visibleRegistryKey(node.path.toStdString())
                                                      : system->visibleRegistryValue(node.path.toStdString(), node.valueName.toStdString()).has_value();
  }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  FileCommitTarget resolveFileTarget(const QString&) const override { throw std::logic_error("Registry source has no file identity"); }
  QString enumerateChildren(const CommitNode& node, std::stop_token stop, const CommitNodeVisitor& discover) const override {
    if (node.kind != CommitTargetKind::RegistryKey) return {};
    ++treeReads;
    const auto publishValues = [&](const QStringList& values) {
      for (const auto& name : values)
        if (!stop.stop_requested() && system->visibleRegistryValue(node.path.toStdString(), name.toStdString()))
          discover({CommitTargetKind::RegistryValue, node.path, name, {}});
    };
    if (equalObjectNames(node.path, key)) {
      for (const auto& subkey : subkeys)
        if (!stop.stop_requested() && system->visibleRegistryKey(subkey.toStdString())) discover({CommitTargetKind::RegistryKey, subkey, {}, {}});
      publishValues(names);
    } else if (const auto found = valuesByKey.find(node.path); found != valuesByKey.cend()) {
      publishValues(found->second);
    }
    return {};
  }
};

enum class MaintenanceMode { Ordinary, ReadOnlyMedia, Horm };
enum class MaintenanceOperation { FileWorkflow, RegistryWorkflow, RegistryBatch, RegistryDeletion };
enum class AdmissionOperation { PrepareFile, PrepareRegistry, RegistryWrite, CommitFile, DeleteFile, CommitRegistry, DeleteRegistry };

// An application adapter may consume published state, but must never turn a
// preparation or a commit into an implicit provider observation.
class StateOnlyUwf final : public compat::Uwf {
 public:
  mutable compat::UwfStateFacts published;
  mutable std::vector<compat::Request> writes;
  std::function<void()> afterWrite;
  compat::Capability probe(const compat::Request&) const override { return {compat::Support::Supported, {}}; }
  compat::UwfStateSnapshot stateSnapshot() const override { return std::make_shared<const compat::UwfStateFacts>(published); }
  void updateState(compat::UwfStateFacts value) const override { published = std::move(value); }
  compat::Response execute(const compat::Request& request, const compat::Context&) const override {
    if (!compat::isWrite(request)) throw std::logic_error("An application adapter issued an implicit provider observation");
    writes.push_back(request);
    if (afterWrite) afterWrite();
    return compat::Response::success(std::monostate{}, compat::Effect::Confirmed);
  }
};

template <class T>
concept HasRegistryCommit = requires(const T& target) { target.commit(std::string{}, std::string{}); };
template <class T>
concept HasRegistryExclusionEdit = requires(const T& target) { target.addExclusion(std::string{}); };
static_assert(HasRegistryCommit<UwfRegistryMaintenanceTarget> && !HasRegistryExclusionEdit<UwfRegistryMaintenanceTarget>);
static_assert(!HasRegistryCommit<UwfRegistryExclusionConfiguration> && HasRegistryExclusionEdit<UwfRegistryExclusionConfiguration>);

class MaintenanceRepositoryTests final : public QObject {
  Q_OBJECT
 private slots:
  void automaticRomPauseUsesTheCurrentProtectedTarget_data() {
    QTest::addColumn<int>("change");
    QTest::addColumn<AutomaticCommitDisposition>("disposition");
    QTest::addColumn<bool>("explicitBlocked");
    QTest::newRow("loaded-rom") << 0 << AutomaticCommitDisposition::SkipReadOnlyMedia << true;
    QTest::newRow("next-rom-only") << 1 << AutomaticCommitDisposition::CheckDurability << false;
    QTest::newRow("current-filter-off") << 2 << AutomaticCommitDisposition::AlreadyPersistent << false;
    QTest::newRow("current-unprotected") << 3 << AutomaticCommitDisposition::AlreadyPersistent << false;
    QTest::newRow("next-filter-off") << 4 << AutomaticCommitDisposition::SkipReadOnlyMedia << true;
    QTest::newRow("next-rom-off") << 5 << AutomaticCommitDisposition::SkipReadOnlyMedia << true;
    QTest::newRow("next-volume-absent") << 6 << AutomaticCommitDisposition::SkipReadOnlyMedia << true;
    QTest::newRow("filter-change-disposition") << 7 << AutomaticCommitDisposition::CheckDurability << false;
    QTest::newRow("horm-preparation-disposition") << 8 << AutomaticCommitDisposition::SkipHormTransition << false;
    QTest::newRow("full-commit-remains-rom") << 9 << AutomaticCommitDisposition::SkipFullCommit << true;
    QTest::newRow("full-commit-overrides-filter-change") << 10 << AutomaticCommitDisposition::SkipFullCommit << true;
    QTest::newRow("horm-still-rejected") << 11 << AutomaticCommitDisposition::SkipHormActive << true;
    QTest::newRow("current-volume-absent") << 12 << AutomaticCommitDisposition::AlreadyPersistent << false;
    QTest::newRow("horm-disabled-auto-paused-manual-allowed") << 13 << AutomaticCommitDisposition::SkipHormTransition << false;
  }
  void automaticRomPauseUsesTheCurrentProtectedTarget() {
    QFETCH(int, change);
    QFETCH(AutomaticCommitDisposition, disposition);
    QFETCH(bool, explicitBlocked);
    constexpr auto id = test::application::fixtureVolumeId;
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    initial.current.overlayFlags = 0x80 | compat::flags::readOnlyMedia;
    initial.current.volumes.emplace(
        id, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {id, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    initial.runtime.systemVolumeId = id;
    initial.next = initial.current;
    switch (change) {
      case 1:
        initial.current.overlayFlags = 0x80;
        break;
      case 2:
        initial.current.filterEnabled = false;
        break;
      case 3:
        initial.current.volumes[id].volume.protectedVolume = false;
        break;
      case 4:
        initial.next.filterEnabled = false;
        break;
      case 5:
        initial.next.overlayFlags = 0;
        break;
      case 6:
        initial.next.volumes.clear();
        break;
      case 7:
        initial.runtime.romDisposition = compat::disposition::filterChange;
        break;
      case 8:
        initial.runtime.romDisposition = compat::disposition::horm;
        break;
      case 9:
        initial.runtime.romDisposition = compat::disposition::fullCommit;
        break;
      case 10:
        initial.runtime.romDisposition = compat::disposition::fullCommit | compat::disposition::filterChange;
        break;
      case 11:
        initial.runtime.hormEnabled = true;
        break;
      case 12:
        initial.current.volumes.clear();
        break;
      case 13:
        initial.runtime.romDisposition = compat::disposition::filterChange | compat::disposition::horm;
        break;
      default:
        break;
    }
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    const auto repository = uwfMaintenanceRepository(system);
    QCOMPARE(repository->automaticFileCommitDisposition(id), disposition);
    if (explicitBlocked) {
      QVERIFY_THROWS_EXCEPTION(OperationError, (void)repository->fileCommitRequirement(id));
      QVERIFY_THROWS_EXCEPTION(OperationError, (void)repository->registryCommitRequirement());
      if (disposition == AutomaticCommitDisposition::SkipFullCommit)
        QCOMPARE(repository->registryWriteRequirement(), CommitRequirement::AlreadyPersistent);
      else
        QVERIFY_THROWS_EXCEPTION(OperationError, (void)repository->registryWriteRequirement());
    } else {
      const auto requirement =
          disposition == AutomaticCommitDisposition::AlreadyPersistent ? CommitRequirement::AlreadyPersistent : CommitRequirement::RequiresUwfCommit;
      QCOMPARE(repository->fileCommitRequirement(id), requirement);
      QCOMPARE(repository->registryCommitRequirement(), requirement);
    }
    QCOMPARE(system->snapshot(), initial);
    QVERIFY(system->trace().empty());
  }
  void automaticRomPauseDoesNotHideUnavailableAdmission_data() {
    QTest::addColumn<int>("missing");
    QTest::newRow("current-filter") << 0;
    QTest::newRow("current-membership") << 1;
    QTest::newRow("current-protection") << 2;
    QTest::newRow("current-flags") << 3;
    QTest::newRow("rom-disposition") << 4;
    QTest::newRow("refresh-failed") << 5;
  }
  void automaticRomPauseDoesNotHideUnavailableAdmission() {
    QFETCH(int, missing);
    const auto system = std::make_shared<StateOnlyUwf>();
    auto& state = system->published;
    constexpr auto id = test::application::fixtureVolumeId;
    state.current.filterEnabled = compat::Fact<bool>::known(true);
    state.current.overlayType = compat::Fact<std::uint32_t>::known(0);
    state.current.overlayFlags = compat::BitFacts::known(compat::flags::readOnlyMedia);
    state.current.volumes[id].present = state.current.volumes[id].protectedVolume = compat::Fact<bool>::known(true);
    state.runtime.horm = compat::Fact<bool>::known(false);
    state.runtime.fullCommit = compat::Fact<bool>::known(false);
    state.runtime.romDisposition = compat::BitFacts::known(0);
    switch (missing) {
      case 0:
        state.current.filterEnabled = {};
        break;
      case 1:
        state.current.volumes[id].present = {};
        break;
      case 2:
        state.current.volumes[id].protectedVolume = {};
        break;
      case 3:
        state.current.overlayFlags = {};
        break;
      case 4:
        state.runtime.romDisposition = {};
        break;
      case 5:
        state.refreshFailure = compat::Failure{compat::ErrorKind::Unavailable, compat::Effect::None, "unavailable", {}};
        break;
    }
    const auto repository = uwfMaintenanceRepository(system);
    QCOMPARE(repository->automaticFileCommitDisposition(id), AutomaticCommitDisposition::Unavailable);
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)repository->fileCommitRequirement(id));
    QVERIFY(system->writes.empty());
  }
  void automaticStagingRechecksRomAtEachStage_data() {
    QTest::addColumn<int>("transition");
    QTest::addColumn<int>("filesToCommit");
    QTest::addColumn<int>("pausedEntriesAtPreparation");
    QTest::addColumn<int>("pausedFilesAtPreparation");
    QTest::newRow("rom-before-scan") << 0 << 0 << 1 << 0;
    QTest::newRow("rom-before-commit-preparation") << 1 << 0 << 1 << 1;
    QTest::newRow("rom-before-file-commit") << 2 << 1 << 0 << 0;
    QTest::newRow("rom-ended-after-scan") << 3 << 1 << 0 << 0;
  }
  void automaticStagingRechecksRomAtEachStage() {
    QFETCH(int, transition);
    QFETCH(int, filesToCommit);
    QFETCH(int, pausedEntriesAtPreparation);
    QFETCH(int, pausedFilesAtPreparation);
    constexpr auto id = test::application::fixtureVolumeId;
    constexpr auto path = "\\saved.bin";
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    initial.current.volumes.emplace(
        id, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {id, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    initial.runtime.fileMaintenancePrefixes[id] = "C:";
    initial.next = initial.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    system->seedFile(id, path, "base");
    system->writeFile(id, path, "changed");
    const auto setRom = [&](const bool enabled) {
      auto state = system->snapshot();
      state.current.overlayFlags = enabled ? compat::flags::readOnlyMedia : 0;
      system->seed(std::move(state));
    };
    if (transition == 0 || transition == 3) setRom(true);
    const auto repository = uwfMaintenanceRepository(system);
    auto files = std::make_shared<Files>();
    files->system = system;
    FileStagingCommitter committer(repository, files);
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
    auto plan = committer.prepare(FileStagingCommitter::scan(committer.prepareScan(entries)));
    if (transition == 1) setRom(true);
    if (transition == 3) setRom(false);
    auto operation = committer.beginCommit(std::move(plan));
    if (transition == 2) setRom(true);
    // Preparation accounts for entire paused selections and any files already
    // discovered within them. A later ROM change is observed per file instead.
    QCOMPARE(operation.totalFiles(), static_cast<std::size_t>(filesToCommit));
    QCOMPARE(operation.result().pausedEntries, static_cast<std::size_t>(pausedEntriesAtPreparation));
    QCOMPARE(operation.result().pausedFiles, static_cast<std::size_t>(pausedFilesAtPreparation));
    QCOMPARE(operation.result().committedFiles, std::size_t{0});
    QCOMPARE(system->readFile(id, path), std::optional<std::string>{"base"});
    const auto before = system->snapshot();
    system->clearTrace();
    while (!operation.finished()) (void)operation.advance();
    QVERIFY(operation.result().succeeded());
    const bool resumed = transition == 3;
    QCOMPARE(operation.result().committedFiles, resumed ? std::size_t{1} : std::size_t{0});
    QCOMPARE(operation.result().pausedEntries, static_cast<std::size_t>(pausedEntriesAtPreparation));
    QCOMPARE(operation.result().pausedFiles, transition == 1 || transition == 2 ? std::size_t{1} : std::size_t{0});
    QCOMPARE(system->readFile(id, path), std::optional<std::string>{resumed ? "changed" : "base"});
    QCOMPARE(system->visibleFile(id, path), std::optional<std::string>{"changed"});
    if (!resumed) {
      QCOMPARE(system->snapshot().durable, before.durable);
      QCOMPARE(system->snapshot().overlay, before.overlay);
      QVERIFY(system->trace().empty());
    }
  }
  void automaticCommitPauseRequiresConfirmedRetention_data() {
    QTest::addColumn<int>("change");
    QTest::addColumn<bool>("paused");
    QTest::newRow("preserved") << 0 << true;
    QTest::newRow("current-filter-off") << 1 << false;
    QTest::newRow("next-filter-off") << 2 << false;
    QTest::newRow("ram") << 3 << false;
    QTest::newRow("nonpersistent-disk") << 4 << false;
    QTest::newRow("reset-clear") << 5 << false;
    QTest::newRow("reset-unknown") << 6 << false;
    QTest::newRow("next-unprotected") << 7 << false;
    QTest::newRow("next-absent") << 8 << false;
    QTest::newRow("next-unobserved") << 9 << false;
    QTest::newRow("next-protection-unknown") << 10 << false;
    QTest::newRow("current-unprotected") << 11 << false;
    QTest::newRow("current-membership-unknown") << 12 << false;
    QTest::newRow("reset-saved-selector") << 13 << false;
    QTest::newRow("refresh-failed") << 14 << false;
  }
  void automaticCommitPauseRequiresConfirmedRetention() {
    QFETCH(int, change);
    QFETCH(bool, paused);
    const auto system = std::make_shared<StateOnlyUwf>();
    auto& state = system->published;
    constexpr auto id = test::application::fixtureVolumeId;
    state.current.filterEnabled = compat::Fact<bool>::known(true);
    state.current.overlayType = compat::Fact<std::uint32_t>::known(1);
    state.current.overlayFlags = compat::BitFacts::known(compat::flags::persistent);
    state.runtime.resetState = compat::Fact<std::uint32_t>::known(0);
    state.runtime.horm = compat::Fact<bool>::known(false);
    auto& current = state.current.volumes[id];
    current.present = current.protectedVolume = compat::Fact<bool>::known(true);
    state.next = state.current;
    switch (change) {
      case 1:
        state.current.filterEnabled = compat::Fact<bool>::known(false);
        break;
      case 2:
        state.next.filterEnabled = compat::Fact<bool>::known(false);
        break;
      case 3:
        state.current.overlayType = compat::Fact<std::uint32_t>::known(0);
        break;
      case 4:
        state.current.overlayFlags = compat::BitFacts::known(0);
        break;
      case 5:
        state.runtime.resetState = compat::Fact<std::uint32_t>::known(1);
        break;
      case 6:
        state.runtime.resetState = {};
        break;
      case 7:
        state.next.volumes[id].protectedVolume = compat::Fact<bool>::known(false);
        break;
      case 8:
        state.next.volumes[id].present = compat::Fact<bool>::known(false);
        break;
      case 9:
        state.next.volumes.clear();
        break;
      case 10:
        state.next.volumes[id].protectedVolume = {};
        break;
      case 11:
        state.current.volumes[id].protectedVolume = compat::Fact<bool>::known(false);
        break;
      case 12:
        state.current.volumes[id].present = {};
        break;
      case 13:
        state.runtime.resetState = compat::Fact<std::uint32_t>::known(255);
        break;
      case 14:
        state.refreshFailure = compat::Failure{compat::ErrorKind::Unavailable, compat::Effect::None, "unavailable", {}};
        break;
      default:
        break;
    }
    const auto repository = uwfMaintenanceRepository(system);
    const auto disposition = repository->automaticFileCommitDisposition(id);
    QCOMPARE(automaticFileCommitPaused(disposition), paused);
    if (paused) QCOMPARE(disposition, AutomaticCommitDisposition::SkipRetainedOverlay);
    QVERIFY(system->writes.empty());
    if (paused) QCOMPARE(repository->fileCommitRequirement(id), CommitRequirement::RequiresUwfCommit);
  }
  void automaticPauseIsPerVolumeAndDoesNotBlockExplicitCommits() {
    class VolumeFiles final : public CommitTraversalSource {
     public:
      mutable int enumerations = 0;
      std::optional<CommitNode> inspect(const CommitNode& target) const override { return target; }
      bool exists(const CommitNode&) const override { return true; }
      std::string driveLetterForPath(const QString& path) const override { return path.left(2).toStdString(); }
      FileCommitTarget resolveFileTarget(const QString& path) const override {
        return test::application::memoryFileTarget(path, path.startsWith(u'C') ? test::application::fixtureVolumeId : test::application::otherFixtureVolumeId);
      }
      QString enumerateChildren(const CommitNode& target, std::stop_token, const CommitNodeVisitor&) const override {
        if (target.kind != CommitTargetKind::Folder) return {};
        ++enumerations;
        throw std::logic_error("The paused directory must not be enumerated");
      }
    };
    const auto system = std::make_shared<StateOnlyUwf>();
    auto& state = system->published;
    state.current.filterEnabled = compat::Fact<bool>::known(true);
    state.current.overlayType = compat::Fact<std::uint32_t>::known(1);
    state.current.overlayFlags = compat::BitFacts::known(compat::flags::persistent);
    state.current.volumesObserved = true;
    state.runtime.resetState = compat::Fact<std::uint32_t>::known(0);
    state.runtime.horm = compat::Fact<bool>::known(false);
    for (const auto& [id, drive] : std::array{std::pair{test::application::fixtureVolumeId, "C:"}, std::pair{test::application::otherFixtureVolumeId, "D:"}}) {
      auto& volume = state.current.volumes[id];
      volume.present = volume.protectedVolume = compat::Fact<bool>::known(true);
      volume.bindByDriveLetter = compat::Fact<bool>::known(false);
      volume.driveLetter = compat::Fact<std::string>::known(drive);
      volume.fileExclusions = compat::Fact<std::vector<std::string>>::known({});
    }
    state.next = state.current;
    state.next.volumes[test::application::otherFixtureVolumeId].protectedVolume = compat::Fact<bool>::known(false);
    const auto repository = uwfMaintenanceRepository(system);
    const auto source = std::make_shared<VolumeFiles>();
    FileStagingCommitter committer(repository, source);
    auto operation = committer.beginCommit(committer.prepare(FileStagingCommitter::scan(
        committer.prepareScan({{FileStagingKind::Directory, QStringLiteral("C:\\Keep")}, {FileStagingKind::File, QStringLiteral("D:\\Save.bin")}}))));
    while (!operation.finished()) (void)operation.advance();
    QVERIFY(operation.result().succeeded());
    QCOMPARE(operation.result().pausedEntries, std::size_t{1});
    QCOMPARE(operation.result().committedFiles, std::size_t{1});
    QCOMPARE(source->enumerations, 0);
    QCOMPARE(system->writes.size(), std::size_t{1});
    QCOMPARE(std::get<compat::CommitFile>(system->writes.front()).volume, std::string(test::application::otherFixtureVolumeId));

    CommitRequest request;
    std::shared_ptr<CommitBackend> backend;
    CommitWorkflow workflow(repository, {[&](const auto& submitted, auto prepared) {
                              request = submitted;
                              backend = std::move(prepared);
                            }},
                            source);
    workflow.commitFilePath(QStringLiteral("C:\\Manual.bin"));
    QVERIFY(backend);
    const auto plan = backend->prepare(request, {}, {}, {});
    QCOMPARE(plan.targets().size(), qsizetype{1});
    QVERIFY(backend->execute(request.target, CommitOperation::Persist).succeeded);
    QCOMPARE(system->writes.size(), std::size_t{2});
    QCOMPARE(std::get<compat::CommitFile>(system->writes.back()).volume, std::string(test::application::fixtureVolumeId));
  }
  void fullTraversalRefreshesScopeWithoutRebindingTheSelectedVolume() {
    auto repository = std::make_shared<Repository>();
    auto source = std::make_shared<ScanningFiles>();
    source->empty = true;
    CommitRequest request;
    std::shared_ptr<CommitBackend> backend;
    CommitWorkflow workflow(repository, {[&](const auto& submitted, auto prepared) {
                              request = submitted;
                              backend = std::move(prepared);
                            }},
                            source);
    workflow.commitFilePath(source->root);
    const auto empty = backend->prepare(request, {}, {}, {});
    QVERIFY(empty.targets().isEmpty());
    QVERIFY(empty.request.target.fileTarget);
    QCOMPARE(empty.request.target.fileTarget->volumeId, std::string(test::application::fixtureVolumeId));
    source->empty = false;
    request.preparationIntent = CommitPreparationIntent::TraverseScope;
    const auto current = backend->prepare(request, empty, {}, {});
    QCOMPARE(current.targets().size(), qsizetype{2});
    QCOMPARE(current.request.preparationIntent, CommitPreparationIntent::TraverseScope);
    QCOMPARE(current.request.target.fileTarget, empty.request.target.fileTarget);
    QCOMPARE(source->inspections[source->root], 2);
    QVERIFY(repository->target->committed.isEmpty());
    source->volume = test::application::otherFixtureVolumeId;
    QVERIFY_THROWS_EXCEPTION(OperationError, static_cast<void>(backend->prepare(request, current, {}, {})));
    QVERIFY(repository->target->committed.isEmpty());
  }
  void aRawFakeViewLetsTheSharedLayerComposeAnOptionOnce() {
    uwf::simulation::FakeUwfBackend::State initial;
    initial.next.overlayFlags = 0x80;
    auto backend = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    const auto raw = backend->rawBackend();
    const PrecheckedUwf system(raw);
    const auto result = system.execute(compat::SetOverlayOption{compat::OverlayOption::ReadOnlyMedia, true});
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, compat::Effect::Confirmed);
    QCOMPARE(backend->snapshot().next.overlayFlags, std::uint32_t{0x80 | compat::flags::readOnlyMedia});
    const auto primitives = backend->primitiveRequests();
    QCOMPARE(primitives.size(), std::size_t{1});
    QVERIFY(std::holds_alternative<compat::SetNumber>(primitives.front()));
    QCOMPARE(std::get<compat::SetNumber>(primitives.front()).property, compat::NumberProperty::OverlayFlags);
    QCOMPARE(backend->trace().size(), std::size_t{1});
    backend.reset();
    QCOMPARE(raw->name(), std::string{"FakeUWF"});
  }
  void maintenanceActionsAcceptTheirReceiptWithoutObservingOrConfirmingTheHost() {
    const auto backend = std::make_shared<StateOnlyUwf>();
    int hostReads = 0;
    OverlayMaintenanceReadOnlyPorts ports;

    ports.hormVolumeProtection = [&](std::stop_token) {
      ++hostReads;
      return OverlayHormVolumeProtection{};
    };

    ports.hibernationCapabilities = [&](std::stop_token) {
      ++hostReads;
      return OverlayHibernationCapabilities{};
    };
    const auto actions = uwfOverlayMaintenance(backend, {}, std::move(ports));
    // The literal operation port acknowledges its commands without publishing
    // result fields. Missing display observations cannot invalidate that receipt.
    const auto fullCommit = actions->fullCommit();
    QVERIFY(fullCommit.succeeded());
    QCOMPARE(fullCommit.effect, WriteEffect::Confirmed);
    QVERIFY(!fullCommit.snapshot.overlayCommit);
    QVERIFY(actions->setHorm(true).succeeded());
    QVERIFY(actions->createSwap(QString::fromLatin1(test::application::fixtureVolumeId)).succeeded());
    QVERIFY(actions->snapshot().has_value());
    QCOMPARE(hostReads, 0);
    QCOMPARE(backend->writes.size(), std::size_t{3});
    QVERIFY(std::holds_alternative<compat::SetBoolean>(backend->writes[0]));
    QVERIFY(std::holds_alternative<compat::SetBoolean>(backend->writes[1]));
    QVERIFY(std::holds_alternative<compat::CreateSwapfile>(backend->writes[2]));
  }
  void runtimeDispositionAdmissionUsesPublishedFactsAcrossAllMaintenanceEntries() {
    const auto raw = std::make_shared<StateOnlyUwf>();
    auto facts = raw->state();
    facts.administrator = compat::Fact<bool>::known(true);
    facts.current.filterEnabled = compat::Fact<bool>::known(true);
    facts.current.volumesObserved = true;
    facts.current.overlayFlags = compat::BitFacts::known(4);
    facts.runtime.horm = compat::Fact<bool>::known(false);
    facts.runtime.romDisposition = compat::BitFacts::known(3);
    facts.runtime.fullCommit = compat::Fact<bool>::known(false);
    facts.systemVolumeId = compat::Fact<std::string>::known(test::application::fixtureVolumeId);
    auto& volume = facts.current.volumes[test::application::fixtureVolumeId];
    volume.present = volume.protectedVolume = compat::Fact<bool>::known(true);
    volume.driveLetter = volume.physicalDriveLetter = compat::Fact<std::string>::known("C:");
    volume.bindByDriveLetter = compat::Fact<bool>::known(false);
    volume.fileExclusions = compat::Fact<std::vector<std::string>>::known({});
    facts.current.registryExclusions = compat::Fact<std::vector<std::string>>::known({});
    raw->updateState(facts);
    const auto system = std::make_shared<PrecheckedUwf>(raw, UwfFactsSource{}, true);
    const auto repository = uwfMaintenanceRepository(system);
    QCOMPARE(repository->fileCommitRequirement(test::application::fixtureVolumeId), CommitRequirement::RequiresUwfCommit);
    QCOMPARE(repository->registryCommitRequirement(), CommitRequirement::RequiresUwfCommit);
    const std::array requests{compat::Request{compat::CommitFile{test::application::fixtureVolumeId, "\\Exact"}},
                              compat::Request{compat::CommitFileDeletion{test::application::fixtureVolumeId, "\\Exact"}},
                              compat::Request{compat::CommitRegistry{"HKLM\\Software\\Exact", std::string("Value")}},
                              compat::Request{compat::CommitRegistryDeletion{"HKLM\\Software\\Exact", std::string("Value")}}};
    for (const auto& request : requests) {
      facts.requestTarget = request;
      auto& target = facts.targets[test::application::fixtureVolumeId];
      target.fileTarget = compat::Fact<compat::FileMaintenanceFacts>::known({10, compat::FileMaintenanceKind::File, false});
      target.persistentFileViewReady = compat::Fact<bool>::known(true);
      system->updateState(facts);
      QVERIFY(std::holds_alternative<compat::Continue>(compat::solveUwfRequest(request, facts).decision));
      const auto result = system->execute(request);
      QVERIFY2(result.succeeded(), result.failure ? result.failure->message.c_str() : "");
    }
    QCOMPARE(raw->writes.size(), std::size_t{4});
    facts.requestTarget = requests.front();
    facts.targets[test::application::fixtureVolumeId].persistentFileViewReady = compat::Fact<bool>::known(false);
    system->updateState(facts);
    const auto denied = compat::solveUwfRequest(requests.front(), facts);
    QVERIFY(std::holds_alternative<compat::Reject>(denied.decision));
    QCOMPARE(std::get<compat::Reject>(denied.decision).rule, compat::RuleId::PersistentFileViewUnavailable);
    QCOMPARE(system->execute(requests.front()).failure->kind, compat::ErrorKind::Unavailable);
    facts.targets[test::application::fixtureVolumeId].persistentFileViewReady = compat::Fact<bool>::known(true);
    facts.runtime.romDisposition = compat::BitFacts::known(4);
    facts.runtime.fullCommit = compat::Fact<bool>::known(true);
    system->updateState(facts);
    QCOMPARE(system->execute(requests.front()).failure->kind, compat::ErrorKind::AccessDenied);
    QVERIFY_THROWS_EXCEPTION(OperationError, static_cast<void>(repository->registryCommitRequirement()));
    QCOMPARE(raw->writes.size(), std::size_t{4});
  }
  void preparationAndCommitUseOnlyPublishedState() {
    const auto backend = std::make_shared<StateOnlyUwf>();
    auto& state = backend->published;
    state.current.filterEnabled = compat::Fact<bool>::known(true);
    state.current.volumesObserved = true;
    state.runtime.horm = compat::Fact<bool>::known(false);
    state.current.overlayFlags = compat::BitFacts::known(0);
    state.systemVolumeId = compat::Fact<std::string>::known(test::application::fixtureVolumeId);
    state.runtime.currentControlSet = compat::Fact<std::uint32_t>::known(2);
    state.current.registryExclusions = compat::Fact<std::vector<std::string>>::known({});
    state.next.registryExclusions = compat::Fact<std::vector<std::string>>::known({"HKLM\\Software\\NextOnly"});
    auto& volume = state.current.volumes[test::application::fixtureVolumeId];
    volume.present = compat::Fact<bool>::known(true);
    volume.protectedVolume = compat::Fact<bool>::known(true);
    volume.bindByDriveLetter = compat::Fact<bool>::known(false);
    volume.driveLetter = compat::Fact<std::string>::known("");
    volume.physicalDriveLetter = compat::Fact<std::string>::known("C:");
    volume.fileExclusions = compat::Fact<std::vector<std::string>>::known({});
    // Physical, unregistered volumes must not become maintenance targets.
    state.current.volumes["Volume{aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee}"].present = compat::Fact<bool>::known(false);
    const auto repository = uwfMaintenanceRepository(backend);
    QCOMPARE(repository->fileCommitRequirement(test::application::fixtureVolumeId), CommitRequirement::RequiresUwfCommit);
    QCOMPARE(repository->registryCommitRequirement(), CommitRequirement::RequiresUwfCommit);
    const auto targets = repository->prepareVolumeTargets();
    QCOMPARE(targets.size(), std::size_t{1});
    QCOMPARE(targets.front()->volume().driveLetter, std::string{"C:"});
    QVERIFY(targets.front()->exclusions().empty());
    const auto registry = repository->prepareRegistryMaintenance();
    QVERIFY(registry->exclusions().empty());
    QCOMPARE(registry->currentControlSet(), std::uint32_t{2});
    QCOMPARE(repository->prepareRegistryExclusions()->exclusions(), std::vector<std::string>{"HKLM\\Software\\NextOnly"});
    QVERIFY(backend->writes.empty());
    QCOMPARE(targets.front()->commitFile("C:\\Exact"), WriteEffect::Confirmed);
    QCOMPARE(targets.front()->commitFileDeletion("C:\\Gone"), WriteEffect::Confirmed);
    QCOMPARE(registry->commit("HKLM\\Software\\Exact", "Value"), WriteEffect::Confirmed);
    QCOMPARE(registry->commitDeletion("HKLM\\Software\\Gone", std::nullopt), WriteEffect::Confirmed);
    QCOMPARE(backend->writes.size(), std::size_t{4});
    QVERIFY(std::holds_alternative<compat::CommitFile>(backend->writes[0]));
    QVERIFY(std::holds_alternative<compat::CommitFileDeletion>(backend->writes[1]));
    QVERIFY(std::holds_alternative<compat::CommitRegistry>(backend->writes[2]));
    QVERIFY(std::holds_alternative<compat::CommitRegistryDeletion>(backend->writes[3]));
  }
  void stagedCommitsUseFreshSnapshotHandlesWithoutProviderReads() {
    const auto raw = std::make_shared<StateOnlyUwf>();
    auto& facts = raw->published;
    const auto id = test::application::fixtureVolumeId;
    facts.current.filterEnabled = compat::Fact<bool>::known(true);
    facts.current.volumesObserved = true;
    facts.current.overlayFlags = compat::BitFacts::known(0);
    facts.runtime.horm = compat::Fact<bool>::known(false);
    facts.systemVolumeId = compat::Fact<std::string>::known(id);
    facts.current.registryExclusions = compat::Fact<std::vector<std::string>>::known({});
    auto& volume = facts.current.volumes[id];
    volume.present = volume.protectedVolume = compat::Fact<bool>::known(true);
    volume.bindByDriveLetter = compat::Fact<bool>::known(false);
    volume.driveLetter = volume.physicalDriveLetter = compat::Fact<std::string>::known("C:");
    volume.fileExclusions = compat::Fact<std::vector<std::string>>::known({});
    // Unrelated lists belong to the same publication; per-file admission must
    // retain its handle rather than materializing those lists for every item.
    facts.next.registryExclusions = compat::Fact<std::vector<std::string>>::known(std::vector<std::string>(2048, "HKLM\\Software\\Unrelated"));
    const auto system = std::make_shared<PrecheckedUwf>(raw);
    const auto publication = system->stateSnapshot();
    const auto repository = uwfMaintenanceRepository(system);
    FileStagingCommitter committer(repository, std::make_shared<ScanningFiles>());
    auto operation = committer.beginCommit(committer.prepare(FileStagingCommitter::scan(committer.prepareScan({{FileStagingKind::Directory, "C:\\Scan"}}))));
    QCOMPARE(operation.totalFiles(), std::size_t{2});
    (void)operation.advance();
    QCOMPARE(operation.result().committedFiles, std::size_t{1});
    QCOMPARE(system->stateSnapshot().get(), publication.get());
    auto updated = system->state();
    updated.current.filterEnabled = compat::Fact<bool>::known(false);
    system->updateState(std::move(updated));
    (void)operation.advance();
    QVERIFY(operation.finished());
    QCOMPARE(operation.result().committedFiles, std::size_t{1});
    QCOMPARE(operation.result().skippedFiles, std::size_t{1});
    QCOMPARE(raw->writes.size(), std::size_t{1});
    QCOMPARE(publication->current.filterEnabled.value, std::optional{true});
  }
  void registryBatchRechecksTheCurrentSnapshotBetweenValues() {
    const auto raw = std::make_shared<StateOnlyUwf>();
    auto& facts = raw->published;
    const auto id = test::application::fixtureVolumeId;
    facts.current.filterEnabled = compat::Fact<bool>::known(true);
    facts.current.volumesObserved = true;
    facts.current.overlayFlags = compat::BitFacts::known(0);
    facts.current.registryExclusions = compat::Fact<std::vector<std::string>>::known({});
    facts.runtime.horm = compat::Fact<bool>::known(false);
    facts.systemVolumeId = compat::Fact<std::string>::known(id);
    facts.current.volumes[id].present = facts.current.volumes[id].protectedVolume = compat::Fact<bool>::known(true);
    const auto system = std::make_shared<PrecheckedUwf>(raw);
    const auto original = system->stateSnapshot();
    raw->afterWrite = [&] {
      auto changed = system->state();
      changed.runtime.horm = compat::Fact<bool>::known(true);
      system->updateState(std::move(changed));
    };
    const RegistryBatchCommitter committer(uwfMaintenanceRepository(system));
    const auto result = committer.commit({{"HKLM\\Software\\Exact", "first"}, {"HKLM\\Software\\Exact", "second"}});
    QCOMPARE(result.committed, std::size_t{1});
    QCOMPARE(result.failures.size(), std::size_t{1});
    QCOMPARE(raw->writes.size(), std::size_t{1});
    QCOMPARE(original->runtime.horm.value, std::optional{false});
    QCOMPARE(system->stateSnapshot()->runtime.horm.value, std::optional{true});
  }
  void missingPublishedStateIsNotKnownEmptyOrUnprotected() {
    const auto backend = std::make_shared<StateOnlyUwf>();
    const auto repository = uwfMaintenanceRepository(backend);
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)repository->fileCommitRequirement(test::application::fixtureVolumeId));
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)repository->prepareVolumeTargets());
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)repository->prepareRegistryMaintenance()->exclusions());
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)repository->prepareRegistryExclusions()->exclusions());
    backend->published.current.filterEnabled = compat::Fact<bool>::known(true);
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)repository->fileCommitRequirement(test::application::fixtureVolumeId));
    backend->published.current.volumesObserved = true;
    QCOMPARE(repository->fileCommitRequirement(test::application::fixtureVolumeId), CommitRequirement::AlreadyPersistent);
    QVERIFY(repository->prepareVolumeTargets().empty());
    backend->published.current.registryExclusions = compat::Fact<std::vector<std::string>>::known({});
    QVERIFY(repository->prepareRegistryMaintenance()->exclusions().empty());
    QVERIFY(backend->writes.empty());
  }
  void workflowRejectsIdentityLossBeforeObservationOrExecution_data() {
    QTest::addColumn<int>("field");
    QTest::addColumn<bool>("deleting");
    QTest::addColumn<QString>("invalid");
    const std::array fields{"path", "value-name", "bound-relative-path"};
    const std::array invalid{QString(QChar(char16_t{0xd800})), QString(QChar(char16_t{0xdc00})), QString(u'X') + QChar::Null + u'Y'};
    for (std::size_t field = 0; field < fields.size(); ++field)
      for (const bool deleting : {false, true})
        for (std::size_t text = 0; text < invalid.size(); ++text)
          QTest::newRow(qUtf8Printable(QStringLiteral("%1/delete-%2/text-%3").arg(fields[field]).arg(deleting).arg(text)))
              << static_cast<int>(field) << deleting << invalid[text];
  }
  void workflowRejectsIdentityLossBeforeObservationOrExecution() {
    QFETCH(int, field);
    QFETCH(bool, deleting);
    QFETCH(QString, invalid);
    const auto repository = std::make_shared<Repository>();
    CommitRequest request;
    std::shared_ptr<CommitBackend> backend;
    CommitWorkflow workflow(repository, {[&](const auto& submitted, auto prepared) {
                              request = submitted;
                              backend = std::move(prepared);
                            }},
                            std::make_shared<Files>());
    const auto file = QStringLiteral("C:\\Exact") + (field == 0 ? invalid : QString{});
    if (field == 1) {
      const auto name = QStringLiteral("Exact") + invalid;
      if (deleting)
        workflow.commitRegistryDeletionKey(QStringLiteral("HKLM\\Software\\Exact"), name);
      else
        workflow.commitRegistryKey(QStringLiteral("HKLM\\Software\\Exact"), name);
    } else if (deleting) {
      workflow.commitFileDeletionPath(file);
    } else {
      workflow.commitFilePath(file);
    }
    QVERIFY(backend);
    if (field == 2) request.target.fileTarget = FileCommitTarget{test::application::fixtureVolumeId, QStringLiteral("\\Exact") + invalid};
    // The dialog may display the exact selection. Its worker must reject it
    // before preparing provider records, and execution must enforce the same
    // boundary even when given a target independently of preparation.
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)backend->prepare(request, {}, {}, {}));
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)backend->execute(request.target, request.operation));
    QCOMPARE(repository->registryReads, 0);
    QCOMPARE(repository->volumeReads, 0);
    QCOMPARE(repository->keys->commits, 0);
    QVERIFY(repository->target->committed.isEmpty());
  }
  void registryCapabilitiesFixTheirOwnSession() {
    const auto backend = std::make_shared<uwf::simulation::FakeUwfBackend>();
    auto state = backend->snapshot();
    state.current.filterEnabled = true;
    state.runtime.systemVolumeId = test::application::fixtureVolumeId;
    state.current.volumes.emplace(test::application::fixtureVolumeId,
                                  uwf::simulation::FakeUwfBackend::VolumeConfiguration{
                                      .volume = {test::application::fixtureVolumeId, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    state.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\CurrentOnly"};
    state.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\NextOnly"};
    backend->seed(state);
    const auto repository = uwfMaintenanceRepository(backend);
    const auto current = repository->prepareRegistryMaintenance();
    const auto next = repository->prepareRegistryExclusions();
    QCOMPARE(current->exclusions(), state.current.registryExclusions);
    QCOMPARE(next->exclusions(), state.next.registryExclusions);
    const std::string addition = "HKEY_LOCAL_MACHINE\\SOFTWARE\\NewNext";
    next->addExclusion(addition);
    QCOMPARE(current->exclusions(), state.current.registryExclusions);
    const auto configured = next->exclusions();
    QVERIFY(std::ranges::find(configured, addition) != configured.end());
    backend->seedRegistryValue(addition, "", "before");
    backend->writeRegistryValue(addition, "", "after");
    QCOMPARE(backend->readRegistryValue(addition, ""), std::optional<std::string>{"before"});
    QCOMPARE(current->commit(addition, ""), WriteEffect::Confirmed);
    QCOMPARE(backend->readRegistryValue(addition, ""), std::optional<std::string>{"after"});
  }
  void configuredVolumesWithTheSameDriveKeepTheirIdentityAndRejectAStaleMapping() {
    constexpr auto first = test::application::fixtureVolumeId;
    constexpr auto second = test::application::otherFixtureVolumeId;
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    for (const auto id : {first, second})
      initial.current.volumes.emplace(
          id, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {id, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    initial.runtime.fileMaintenancePrefixes = {{first, "C:"}, {second, "D:"}};
    initial.next = initial.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    for (const auto id : {first, second}) {
      system->seedFile(id, "\\same.bin", std::string("base-") + id);
      system->writeFile(id, "\\same.bin", std::string("changed-") + id);
    }
    const auto repository = uwfMaintenanceRepository(system);
    const auto targets = repository->prepareVolumeTargets();
    QCOMPARE(targets.size(), std::size_t{2});
    std::vector<std::string> identities;
    for (const auto& target : targets) {
      const auto id = compat::normalizeVolumeId(target->volume().volumeName);
      identities.push_back(id);
      QCOMPARE(repository->fileCommitRequirement(id), CommitRequirement::RequiresUwfCommit);
      if (id == first) {
        QCOMPARE(target->commitFile("\\same.bin"), WriteEffect::Confirmed);
      } else {
        try {
          (void)target->commitFile("\\same.bin");
          QFAIL("A stale configured drive must not submit another physical target");
        } catch (const OperationError& error) {
          QCOMPARE(error.failureKind(), OperationFailureKind::Conflict);
          QCOMPARE(error.writeEffect(), WriteEffect::None);
        }
      }
    }
    std::ranges::sort(identities);
    auto expected = std::vector<std::string>{first, second};
    std::ranges::sort(expected);
    QCOMPARE(identities, expected);
    QCOMPARE(system->readFile(first, "\\same.bin"), std::optional<std::string>{std::string("changed-") + first});
    QCOMPARE(system->readFile(second, "\\same.bin"), std::optional<std::string>{std::string("base-") + second});
  }
  void successfulBatchCountersDoNotInventSubmittedWriteEffects() {
    for (const auto receipt : {WriteEffect::None, WriteEffect::Confirmed}) {
      const auto repository = std::make_shared<Repository>();
      repository->keys->receipt = receipt;
      const RegistryBatchCommitter registry(repository);
      const auto registryResult = registry.commit({{"HKLM\\SOFTWARE\\Test", "Value"}});
      QCOMPARE(registryResult.committed, std::size_t{1});
      QCOMPARE(registryResult.writeEffect(), receipt);
      repository->target->receipt = receipt;
      FileStagingCommitter staging(repository, std::make_shared<Files>());
      auto operation = staging.beginCommit(staging.prepare(FileStagingCommitter::scan(staging.prepareScan({{FileStagingKind::File, "C:\\receipt.bin"}}))));
      (void)operation.advance();
      QCOMPARE(operation.result().committedFiles, std::size_t{1});
      QCOMPARE(operation.result().writeEffect(), receipt);
    }
  }
  void workflowReusesEnumeratedIdentitiesAndRevalidatesBeforeExecution_data() {
    QTest::addColumn<bool>("directory");
    QTest::newRow("selected-file") << false;
    QTest::newRow("selected-directory") << true;
  }
  void workflowReusesEnumeratedIdentitiesAndRevalidatesBeforeExecution() {
    QFETCH(bool, directory);
    auto repository = std::make_shared<Repository>();
    auto source = std::make_shared<ScanningFiles>();
    CommitRequest request;
    std::shared_ptr<CommitBackend> prepared;
    CommitWorkflow workflow(repository, {[&](const auto& submitted, auto backend) {
                              request = submitted;
                              prepared = std::move(backend);
                            }},
                            source);
    workflow.commitFilePath(directory ? source->root : source->leaf);
    const auto plan = prepared->prepare(request, {}, {}, {});
    const auto targets = plan.targets();
    QCOMPARE(plan.progress().failed, qsizetype{0});
    QCOMPARE(targets.size(), directory ? qsizetype{2} : qsizetype{1});
    QCOMPARE(source->resolutions, 1);
    QCOMPARE(source->inspections.size(), directory ? std::size_t{4} : std::size_t{1});
    for (const auto& [path, count] : source->inspections) {
      Q_UNUSED(path);
      QCOMPARE(count, 1);
    }
    QCOMPARE(repository->volumeReads, 1);
    QVERIFY(repository->target->committed.isEmpty());
    for (const auto& target : targets) {
      QVERIFY(target.fileTarget);
      QCOMPARE(target.fileTarget->volumeId, std::string(test::application::fixtureVolumeId));
      QCOMPARE(target.fileTarget->relativePath, test::application::memoryFileTarget(target.path).relativePath);
    }

    const auto selected = targets.front();
    const auto result = prepared->execute(selected, CommitOperation::Persist);
    QVERIFY(result.succeeded);
    QCOMPARE(source->resolutions, 2);
    QCOMPARE(repository->target->committed, QStringList{selected.fileTarget->relativePath});
    source->volume = test::application::otherFixtureVolumeId;
    const auto rebound = prepared->execute(selected, CommitOperation::Persist);
    QVERIFY(!rebound.succeeded);
    QCOMPARE(rebound.failureKind, OperationFailureKind::Conflict);
    QCOMPARE(rebound.failureEffect, WriteEffect::None);
    QCOMPARE(source->resolutions, 3);
    QCOMPARE(repository->target->committed.size(), qsizetype{1});
  }
  void preparationRetryRevalidatesFailedBindingsWithoutRewalkingCompletedSiblings() {
    auto repository = std::make_shared<Repository>();
    auto source = std::make_shared<ScanningFiles>();
    source->failBranch = true;
    CommitRequest request;
    std::shared_ptr<CommitBackend> prepared;
    CommitWorkflow workflow(repository, {[&](const auto& submitted, auto backend) {
                              request = submitted;
                              prepared = std::move(backend);
                            }},
                            source);
    workflow.commitFilePath(source->root);
    auto plan = prepared->prepare(request, {}, {}, {});
    QCOMPARE(plan.progress().failed, qsizetype{1});
    QCOMPARE(source->resolutions, 1);
    source->failBranch = false;
    plan = prepared->prepare(request, std::move(plan), {}, {});
    QCOMPARE(plan.progress().failed, qsizetype{0});
    QCOMPARE(plan.targets().size(), qsizetype{2});
    QCOMPARE(source->resolutions, 3);  // Selected root and the failed directory get one new boundary check each.
    QCOMPARE(source->inspections[source->root], 1);
    QCOMPARE(source->inspections[source->branch], 2);
    QCOMPARE(source->inspections[source->leaf], 1);
    QCOMPARE(source->inspections[source->nested], 1);
  }
  void aReboundDriveCannotCommitAnotherGuidAtTheSameRelativePath() {
    auto repository = std::make_shared<Repository>();
    auto source = std::make_shared<Files>();
    CommitRequest request;
    std::shared_ptr<CommitBackend> prepared;
    CommitWorkflow workflow(repository, {[&](const auto& submitted, auto backend) {
                              request = submitted;
                              prepared = std::move(backend);
                            }},
                            source);
    workflow.commitFilePath(QStringLiteral("C:\\same.bin"));
    const auto plan = prepared->prepare(request, {}, {}, {});
    QCOMPARE(plan.targets().size(), 1);
    const auto selected = plan.targets().constFirst();
    QVERIFY(selected.fileTarget);
    QCOMPARE(selected.fileTarget->volumeId, std::string(test::application::fixtureVolumeId));
    QCOMPARE(selected.fileTarget->relativePath, QStringLiteral("\\same.bin"));
    source->volume = test::application::otherFixtureVolumeId;
    const auto result = prepared->execute(selected, CommitOperation::Persist);
    QVERIFY(!result.succeeded);
    QCOMPARE(result.failureKind, OperationFailureKind::Conflict);
    QCOMPARE(result.failureEffect, WriteEffect::None);
    QVERIFY(repository->target->committed.empty());
    QCOMPARE(repository->volumeReads, 1);
  }
  void unprotectedRomFileSkipsWithoutGlobalInventoryOrModeReads() {
    for (const bool horm : {false, true}) {
      constexpr auto selectedVolume = test::application::fixtureVolumeId;
      uwf::simulation::FakeUwfBackend::State initial;
      initial.current.filterEnabled = true;
      initial.current.overlayFlags = compat::flags::readOnlyMedia;
      initial.runtime.hormEnabled = horm;
      initial.current.volumes.emplace(selectedVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{
                                                          .volume = {selectedVolume, "C:", false, false, false}, .fileExclusions = {}, .swapfileSize = 0});
      initial.current.volumes.emplace(
          test::application::otherFixtureVolumeId,
          uwf::simulation::FakeUwfBackend::VolumeConfiguration{
              .volume = {test::application::otherFixtureVolumeId, "D:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
      initial.next = initial.current;
      const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
      const auto repository = uwfMaintenanceRepository(system);
      QCOMPARE(repository->fileCommitRequirement(selectedVolume), CommitRequirement::AlreadyPersistent);
      CommitRequest request;
      std::shared_ptr<CommitBackend> prepared;
      CommitWorkflow workflow(repository, {[&](const auto& submitted, auto backend) {
                                request = submitted;
                                prepared = std::move(backend);
                              }},
                              std::make_shared<Files>());
      workflow.commitFilePath(QStringLiteral("C:\\already-persistent.bin"));
      const auto plan = prepared->prepare(request, {}, {}, {});
      QVERIFY(plan.targets().empty());
      QCOMPARE(plan.records.front().status, TraversalStatus::Skipped);
      workflow.commitFileDeletionPath(QStringLiteral("C:\\already-persistent.bin"));
      try {
        (void)prepared->prepare(request, {}, {}, {});
        QFAIL("UWF cannot delete an already persistent file target");
      } catch (const OperationError& error) {
        QCOMPARE(error.failureKind(), OperationFailureKind::Conflict);
        QCOMPARE(error.writeEffect(), WriteEffect::None);
      }
      for (const auto& entry : system->trace()) {
        QVERIFY(!compat::isWrite(entry.request));
        QVERIFY(!std::holds_alternative<compat::GetVolumes>(entry.request));
        QVERIFY(!std::holds_alternative<compat::GetNumber>(entry.request));
        if (const auto* boolean = std::get_if<compat::GetBoolean>(&entry.request)) QVERIFY(boolean->property != compat::BooleanProperty::HormEnabled);
        if (const auto* volume = std::get_if<compat::GetVolumeProperty>(&entry.request)) QCOMPARE(volume->volume, std::string(selectedVolume));
      }
    }
  }
  void defaultRegistryValueIntentDoesNotBecomeWholeKeyTraversal() {
    constexpr auto volume = test::application::fixtureVolumeId;
    const auto key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\DefaultIntent");
    const auto child = key + QStringLiteral("\\Child");
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    initial.runtime.systemVolumeId = volume;
    initial.current.volumes.emplace(
        volume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {volume, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    initial.next = initial.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    system->seedRegistryValue(key.toStdString(), "", "default-base");
    system->seedRegistryValue(key.toStdString(), "Named", "named-base");
    system->seedRegistryValue(child.toStdString(), "ChildValue", "child-base");
    system->writeRegistryValue(key.toStdString(), "", "default-target");
    system->writeRegistryValue(key.toStdString(), "Named", "named-overlay");
    system->writeRegistryValue(child.toStdString(), "ChildValue", "child-overlay");
    auto source = std::make_shared<RegistryValues>();
    source->system = system;
    source->key = key;
    source->names = {QString{}, QStringLiteral("Named")};
    source->subkeys = {child};
    source->valuesByKey[child] = {QStringLiteral("ChildValue")};
    CommitRequest request;
    std::shared_ptr<CommitBackend> prepared;
    CommitWorkflow workflow(uwfMaintenanceRepository(system), {[&](const auto& submitted, auto backend) {
                              request = submitted;
                              prepared = std::move(backend);
                            }},
                            source);
    workflow.commitRegistryKey(key, std::optional<QString>{QString{}});
    QCOMPARE(request.target.kind, CommitTargetKind::RegistryValue);
    QVERIFY(request.target.valueName.isEmpty());
    const auto plan = prepared->prepare(request, {}, {}, {});
    QCOMPARE(source->treeReads, 0);
    QCOMPARE(plan.targets().size(), 1);
    system->clearTrace();
    QVERIFY(prepared->execute(plan.targets().constFirst(), CommitOperation::Persist).succeeded);
    int commits = 0;
    for (const auto& entry : system->trace())
      if (const auto* commit = std::get_if<compat::CommitRegistry>(&entry.request)) {
        ++commits;
        QCOMPARE(commit->key, key.toStdString());
        QCOMPARE(commit->valueName, std::optional<std::string>{std::string{}});
      }
    QCOMPARE(commits, 1);
    workflow.commitRegistryDeletionKey(key, std::optional<QString>{QString{}});
    system->clearTrace();
    try {
      (void)prepared->prepare(request, {}, {}, {});
      QFAIL("A selected default value cannot become whole-key deletion");
    } catch (const OperationError& error) {
      QCOMPARE(error.failureKind(), OperationFailureKind::Unsupported);
      QCOMPARE(error.writeEffect(), WriteEffect::None);
    }
    for (const auto& entry : system->trace()) QVERIFY(!compat::isWrite(entry.request));
    system->simulateRestart();
    QCOMPARE(system->visibleRegistryValue(key.toStdString(), ""), std::optional<std::string>{"default-target"});
    QCOMPARE(system->visibleRegistryValue(key.toStdString(), "Named"), std::optional<std::string>{"named-base"});
    QCOMPARE(system->visibleRegistryValue(child.toStdString(), "ChildValue"), std::optional<std::string>{"child-base"});
  }
  void maintenanceAdmissionUsesLiveCurrentFacts_data() {
    QTest::addColumn<AdmissionOperation>("operation");
    QTest::addColumn<bool>("currentRom");
    QTest::addColumn<bool>("nextRom");
    QTest::addColumn<bool>("horm");
    QTest::addColumn<bool>("fullCommit");
    const std::array operations{
        std::pair{"prepare-file", AdmissionOperation::PrepareFile},      std::pair{"prepare-registry", AdmissionOperation::PrepareRegistry},
        std::pair{"registry-write", AdmissionOperation::RegistryWrite},  std::pair{"commit-file", AdmissionOperation::CommitFile},
        std::pair{"delete-file", AdmissionOperation::DeleteFile},        std::pair{"commit-registry", AdmissionOperation::CommitRegistry},
        std::pair{"delete-registry", AdmissionOperation::DeleteRegistry}};
    for (const auto& [name, operation] : operations)
      for (const bool currentRom : {false, true})
        for (const bool nextRom : {false, true})
          for (const bool horm : {false, true})
            for (const bool fullCommit : {false, true}) {
              if (fullCommit && (!currentRom || horm)) continue;
              const auto row = QStringLiteral("%1/current-rom-%2/next-rom-%3/horm-%4/full-%5")
                                   .arg(QString::fromLatin1(name))
                                   .arg(currentRom)
                                   .arg(nextRom)
                                   .arg(horm)
                                   .arg(fullCommit);
              QTest::newRow(qUtf8Printable(row)) << operation << currentRom << nextRom << horm << fullCommit;
            }
  }
  void maintenanceAdmissionUsesLiveCurrentFacts() {
    QFETCH(AdmissionOperation, operation);
    QFETCH(bool, currentRom);
    QFETCH(bool, nextRom);
    QFETCH(bool, horm);
    QFETCH(bool, fullCommit);
    constexpr auto volume = "11111111-2222-3333-4444-555555555555";
    constexpr auto path = "\\file.txt";
    constexpr auto key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\LiveAdmission";
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    initial.runtime.fileMaintenancePrefixes[volume] = "C:";
    initial.current.volumes.emplace(
        volume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {volume, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    initial.runtime.systemVolumeId = volume;
    initial.next = initial.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    system->seedFile(volume, path, "base");
    system->writeFile(volume, path, "changed");
    system->seedRegistryValue(key, "Setting", "base");
    system->writeRegistryValue(key, "Setting", "changed");
    const auto shared = std::make_shared<PrecheckedUwf>(system);
    const auto repository = uwfMaintenanceRepository(shared);
    QCOMPARE(repository->fileCommitRequirement(test::application::fixtureVolumeId), CommitRequirement::RequiresUwfCommit);
    QCOMPARE(repository->registryCommitRequirement(), CommitRequirement::RequiresUwfCommit);
    const auto fileTarget = repository->prepareVolumeTargets().front();
    const auto registryTarget = repository->prepareRegistryMaintenance();
    // Explicit observation publishes new Current facts after preparation.
    // The shared UWF layer owns operation admission from those facts.
    auto live = system->snapshot();
    constexpr std::uint32_t opaqueFlag = 0x80;
    live.current.overlayFlags = opaqueFlag | (currentRom ? compat::flags::readOnlyMedia : 0);
    live.next.overlayFlags = opaqueFlag | (nextRom ? compat::flags::readOnlyMedia : 0);
    live.runtime.hormEnabled = horm;
    live.runtime.romDisposition = fullCommit ? 4 : 0;
    system->seed(live);
    shared->updateState(system->state());
    system->clearTrace();
    const auto execute = [&] {
      switch (operation) {
        case AdmissionOperation::PrepareFile:
          QCOMPARE(repository->fileCommitRequirement(test::application::fixtureVolumeId), CommitRequirement::RequiresUwfCommit);
          break;
        case AdmissionOperation::PrepareRegistry:
          QCOMPARE(repository->registryCommitRequirement(), CommitRequirement::RequiresUwfCommit);
          break;
        case AdmissionOperation::RegistryWrite:
          QCOMPARE(repository->registryWriteRequirement(), fullCommit ? CommitRequirement::AlreadyPersistent : CommitRequirement::RequiresUwfCommit);
          break;
        case AdmissionOperation::CommitFile:
          fileTarget->commitFile(path);
          break;
        case AdmissionOperation::DeleteFile:
          fileTarget->commitFileDeletion(path);
          break;
        case AdmissionOperation::CommitRegistry:
          registryTarget->commit(key, "Setting");
          break;
        case AdmissionOperation::DeleteRegistry:
          registryTarget->commitDeletion(key, std::string("Setting"));
          break;
      }
    };
    if ((currentRom || horm) && !(operation == AdmissionOperation::RegistryWrite && fullCommit)) {
      QVERIFY_THROWS_EXCEPTION(OperationError, execute());
      QCOMPARE(system->snapshot(), live);
      for (const auto& entry : system->trace()) QVERIFY(!compat::isWrite(entry.request));
    } else {
      execute();
    }
    for (const auto& entry : system->trace())
      if (const auto* read = std::get_if<compat::GetNumber>(&entry.request); read && read->property == compat::NumberProperty::OverlayFlags)
        QCOMPARE(read->session, compat::Session::Current);
  }
  void registryWriteAdmissionKeepsTheGlobalRefreshBarrier() {
    constexpr auto systemVolume = test::application::fixtureVolumeId;
    auto system = std::make_shared<StateOnlyUwf>();
    auto& facts = system->published;
    facts.current.filterEnabled = compat::Fact<bool>::known(true);
    facts.current.overlayFlags = compat::BitFacts::known(compat::flags::readOnlyMedia);
    facts.runtime.horm = compat::Fact<bool>::known(false);
    facts.runtime.fullCommit = compat::Fact<bool>::known(true);
    facts.runtime.romDisposition = compat::BitFacts::known(compat::disposition::fullCommit);
    facts.systemVolumeId = compat::Fact<std::string>::known(systemVolume);
    facts.current.volumes[systemVolume].present = compat::Fact<bool>::known(true);
    facts.current.volumes[systemVolume].protectedVolume = compat::Fact<bool>::known(true);
    const auto repository = uwfMaintenanceRepository(system);
    QCOMPARE(repository->registryWriteRequirement(), CommitRequirement::AlreadyPersistent);
    for (const bool filter : {true, false}) {
      facts.current.filterEnabled = compat::Fact<bool>::known(filter);
      facts.refreshFailure = compat::Failure{compat::ErrorKind::Unavailable, compat::Effect::None, "Core refresh failed", {}};
      try {
        (void)repository->registryWriteRequirement();
        QFAIL("A new host registry write must retain the global refresh barrier");
      } catch (const OperationError& error) {
        QCOMPARE(error.writeEffect(), WriteEffect::None);
      }
      facts.refreshFailure.reset();
      QCOMPARE(repository->registryWriteRequirement(), CommitRequirement::AlreadyPersistent);
    }
    QVERIFY(system->writes.empty());
  }
  void registryWorkflowDistinguishesAlreadyPersistentWritesAndDeletions_data() {
    QTest::addColumn<MaintenanceMode>("mode");
    QTest::addColumn<bool>("deleting");
    QTest::addColumn<bool>("wholeKey");
    for (const auto& [name, mode] :
         {std::pair{"ordinary", MaintenanceMode::Ordinary}, std::pair{"rom", MaintenanceMode::ReadOnlyMedia}, std::pair{"horm", MaintenanceMode::Horm}})
      for (const bool deleting : {false, true})
        for (const bool whole : {false, true})
          QTest::newRow((std::string(name) + (deleting ? "/delete/" : "/persist/") + (whole ? "key" : "value")).c_str()) << mode << deleting << whole;
  }
  void registryWorkflowDistinguishesAlreadyPersistentWritesAndDeletions() {
    QFETCH(MaintenanceMode, mode);
    QFETCH(bool, deleting);
    QFETCH(bool, wholeKey);
    constexpr auto systemVolume = "11111111-2222-3333-4444-555555555555";
    constexpr auto dataVolume = "66666666-7777-8888-9999-aaaaaaaaaaaa";
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    initial.runtime.systemVolumeId = systemVolume;
    initial.current.volumes.emplace(systemVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{
                                                      .volume = {systemVolume, "C:", false, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    initial.current.volumes.emplace(dataVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{
                                                    .volume = {dataVolume, "D:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    if (mode != MaintenanceMode::Ordinary) initial.current.overlayFlags = compat::flags::readOnlyMedia;
    initial.runtime.hormEnabled = mode == MaintenanceMode::Horm;
    initial.next = initial.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    const auto key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\AlreadyPersistentWorkflow");
    const auto value = QStringLiteral("Setting");
    system->seedRegistryValue(key.toStdString(), value.toStdString(), "before");
    system->writeRegistryValue(key.toStdString(), value.toStdString(), "after");
    const auto source = std::make_shared<RegistryValues>();
    source->system = system;
    source->key = key;
    source->names = {value};
    CommitRequest request;
    std::shared_ptr<CommitBackend> prepared;
    CommitWorkflow workflow(uwfMaintenanceRepository(system), {[&](const auto& submitted, auto backend) {
                              request = submitted;
                              prepared = std::move(backend);
                            }},
                            source);
    if (deleting)
      workflow.commitRegistryDeletionKey(key, wholeKey ? std::nullopt : std::optional<QString>{value});
    else
      workflow.commitRegistryKey(key, wholeKey ? std::nullopt : std::optional<QString>{value});
    QVERIFY(prepared);
    const auto before = system->snapshot();
    system->clearTrace();
    if (deleting) {
      try {
        (void)prepared->prepare(request, {}, {}, {});
        QFAIL("An already persistent registry target must not pretend that UWF deleted it");
      } catch (const OperationError& error) {
        QCOMPARE(error.failureKind(), OperationFailureKind::Conflict);
        QCOMPARE(error.writeEffect(), WriteEffect::None);
        QVERIFY(QString::fromUtf8(error.what()).contains(QStringLiteral("already persistent")));
      }
    } else {
      const auto plan = prepared->prepare(request, {}, {}, {});
      QVERIFY(plan.targets().isEmpty());
      QCOMPARE(plan.records.size(), 1);
      QCOMPARE(plan.records.front().status, TraversalStatus::Skipped);
      QVERIFY(plan.records.front().error.contains(QStringLiteral("already persistent")));
    }
    QCOMPARE(system->snapshot(), before);
    const auto trace = system->trace();
    QVERIFY(std::ranges::none_of(trace, [](const auto& entry) {
      return compat::isWrite(entry.request) || std::holds_alternative<compat::GetRegistryExclusions>(entry.request) ||
             (std::get_if<compat::GetBoolean>(&entry.request) && std::get<compat::GetBoolean>(entry.request).property == compat::BooleanProperty::HormEnabled);
    }));
    system->simulateRestart();
    QCOMPARE(system->visibleRegistryValue(key.toStdString(), value.toStdString()), std::optional<std::string>{"after"});
  }
  void registryTraversalRetainsTargetsWhoseIdentityCannotBePreserved_data() {
    QTest::addColumn<bool>("selectParent");
    QTest::addColumn<bool>("deleting");
    for (const bool parent : {false, true})
      for (const bool deleting : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("parent-%1-delete-%2").arg(parent).arg(deleting))) << parent << deleting;
  }
  void registryTraversalRetainsTargetsWhoseIdentityCannotBePreserved() {
    QFETCH(bool, selectParent);
    QFETCH(bool, deleting);
    const auto parent = QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM");
    const auto unsafe = parent + QStringLiteral("\\CurrentControlSetExtra");
    const auto numbered = parent + QStringLiteral("\\ControlSet001Extra");
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    constexpr auto systemVolume = "11111111-2222-3333-4444-555555555555";
    initial.runtime.systemVolumeId = systemVolume;
    initial.current.volumes.emplace(systemVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{
                                                      .volume = {systemVolume, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    initial.next = initial.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    for (const auto& key : {unsafe, numbered}) {
      system->seedRegistryValue(key.toStdString(), "Setting", "base");
      system->writeRegistryValue(key.toStdString(), "Setting", "changed");
    }
    system->seedRegistryValue(parent.toStdString(), "Allowed", "base");
    system->writeRegistryValue(parent.toStdString(), "Allowed", "changed");
    const auto before = system->snapshot();
    const auto source = std::make_shared<RegistryValues>();
    source->system = system;
    source->key = selectParent ? parent : unsafe;
    source->names = selectParent ? QStringList{QStringLiteral("Allowed")} : QStringList{QStringLiteral("Setting")};
    if (selectParent) {
      source->subkeys = {unsafe, numbered};
      source->valuesByKey[numbered] = {QStringLiteral("Setting")};
    }
    CommitRequest submitted;
    std::shared_ptr<CommitBackend> prepared;
    CommitWorkflow workflow(uwfMaintenanceRepository(system), {[&](const auto& request, auto backend) {
                              submitted = request;
                              prepared = std::move(backend);
                            }},
                            source);
    if (deleting)
      workflow.commitRegistryDeletionKey(source->key, {});
    else
      workflow.commitRegistryKey(source->key, {});
    QVERIFY(prepared);
    const auto plan = prepared->prepare(submitted, {}, {}, {});
    QCOMPARE(plan.progress().failed, 1);
    const auto failure = std::ranges::find_if(plan.records, [](const auto& record) { return record.status == TraversalStatus::Failed; });
    QVERIFY(failure != plan.records.cend());
    QCOMPARE(failure->node.path, unsafe);
    QVERIFY(!failure->error.isEmpty());
    const auto targets = plan.targets();
    QCOMPARE(targets.size(), selectParent ? 2 : 0);
    if (selectParent) {
      for (const auto& target : targets) {
        if (target.path == parent) {
          QCOMPARE(target.kind, CommitTargetKind::RegistryValue);
          QCOMPARE(target.valueName, QStringLiteral("Allowed"));
        } else {
          QCOMPARE(target.path, numbered);
          QCOMPARE(target.kind, deleting ? CommitTargetKind::RegistryKey : CommitTargetKind::RegistryValue);
          QCOMPARE(target.valueName, deleting ? QString{} : QStringLiteral("Setting"));
        }
        QVERIFY(prepared->execute(target, submitted.operation).succeeded);
      }
      QVERIFY(system->visibleRegistryKey(parent.toStdString()));
    } else {
      QCOMPARE(system->snapshot(), before);
    }
    QCOMPARE(system->readRegistryValue(unsafe.toStdString(), "Setting"), std::optional<std::string>{"base"});
    QCOMPARE(system->visibleRegistryValue(unsafe.toStdString(), "Setting"), std::optional<std::string>{"changed"});
    const auto numberedBase = !selectParent ? std::optional<std::string>{"base"} : deleting ? std::nullopt : std::optional<std::string>{"changed"};
    QCOMPARE(system->readRegistryValue(numbered.toStdString(), "Setting"), numberedBase);
    QCOMPARE(system->visibleRegistryValue(numbered.toStdString(), "Setting"), selectParent && deleting ? std::nullopt : std::optional<std::string>{"changed"});
    QCOMPARE(system->visibleRegistryKey(numbered.toStdString()), !selectParent || !deleting);
    for (const auto& entry : system->trace()) {
      if (const auto* operation = std::get_if<compat::CommitRegistry>(&entry.request)) {
        QVERIFY(operation->key == parent.toStdString() || operation->key == numbered.toStdString());
        QCOMPARE(operation->valueName, std::optional<std::string>{operation->key == parent.toStdString() ? "Allowed" : "Setting"});
      }
      if (const auto* operation = std::get_if<compat::CommitRegistryDeletion>(&entry.request)) {
        QVERIFY(operation->key == parent.toStdString() || operation->key == numbered.toStdString());
        QCOMPARE(operation->valueName, operation->key == parent.toStdString() ? std::optional<std::string>{"Allowed"} : std::nullopt);
      }
    }
  }
  void exclusionEffectsGovernRealCommitWorkflows_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("deleting");
    QTest::addColumn<MaintenanceMode>("mode");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("target");
    QTest::addColumn<bool>("managementSkips");
    QTest::addColumn<bool>("driverBypasses");
    for (const bool registry : {false, true}) {
      const auto root = registry ? QStringLiteral("HKLM\\SOFTWARE\\Coverage") : QStringLiteral("\\Coverage");
      const auto target = registry ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Coverage\\Child") : QStringLiteral("\\Coverage\\Child.txt");
      for (const bool deleting : {false, true})
        for (const auto& [name, mode] : {std::pair{"ordinary", MaintenanceMode::Ordinary}, std::pair{"rom", MaintenanceMode::ReadOnlyMedia}}) {
          const auto row = QStringLiteral("registry-%1/delete-%2/%3/").arg(registry).arg(deleting).arg(QString::fromLatin1(name));
          for (const int tails : {0, 1, 2})
            QTest::newRow(qUtf8Printable(row + QStringLiteral("tails-%1").arg(tails)))
                << registry << deleting << mode << root + QString(tails, u'\\') << target << (tails < 2) << (tails == 0);
          const auto prefix = registry ? QStringLiteral("HKLM\\SOFTWARE\\") : QStringLiteral("\\");
          const auto actualPrefix = registry ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\") : QStringLiteral("\\");
          const auto suffix = registry ? QStringLiteral("\\Child") : QStringLiteral("\\Child.txt");
          QTest::newRow(qUtf8Printable(row + QStringLiteral("driver-only-unicode")))
              << registry << deleting << mode << prefix + QChar(0x00e4) << actualPrefix + QChar(0x00c4) + suffix << false << true;
          QTest::newRow(qUtf8Printable(row + QStringLiteral("no-exclusion"))) << registry << deleting << mode << QString{} << target << false << false;
          if (registry)
            QTest::newRow(qUtf8Printable(row + QStringLiteral("driver-only-current-control-set")))
                << registry << deleting << mode << QStringLiteral("HKLM\\SYSTEM\\CurrentControlSet\\Coverage")
                << QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Coverage\\Child") << false << true;
        }
    }
  }
  void exclusionEffectsGovernRealCommitWorkflows() {
    QFETCH(bool, registry);
    QFETCH(bool, deleting);
    QFETCH(MaintenanceMode, mode);
    QFETCH(QString, pattern);
    QFETCH(QString, target);
    QFETCH(bool, managementSkips);
    QFETCH(bool, driverBypasses);
    constexpr auto volume = "11111111-2222-3333-4444-555555555555";
    const auto key = target.toStdString();
    const auto relative = target.toStdString();
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    initial.runtime.fileMaintenancePrefixes[volume] = "C:";
    initial.current.overlayFlags = mode == MaintenanceMode::ReadOnlyMedia ? compat::flags::readOnlyMedia : 0;
    initial.runtime.systemVolumeId = volume;
    const auto exclusions = pattern.isEmpty() ? std::vector<std::string>{} : std::vector{pattern.toStdString()};
    initial.current.volumes.emplace(
        volume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{
                    .volume = {volume, "C:", true, false, false}, .fileExclusions = registry ? std::vector<std::string>{} : exclusions, .swapfileSize = 0});
    if (registry) initial.current.registryExclusions = exclusions;
    initial.next = initial.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    if (registry) {
      system->seedRegistryValue(key, "Setting", "base");
      system->writeRegistryValue(key, "Setting", "changed");
    } else {
      system->seedFile(volume, relative, "base");
      system->writeFile(volume, relative, "changed");
    }
    auto files = std::make_shared<Files>();
    files->system = system;
    files->volume = volume;
    auto values = std::make_shared<RegistryValues>();
    values->system = system;
    values->key = target;
    values->names = {QStringLiteral("Setting")};
    const std::shared_ptr<const CommitTraversalSource> source =
        registry ? std::static_pointer_cast<const CommitTraversalSource>(values) : std::static_pointer_cast<const CommitTraversalSource>(files);
    CommitRequest request;
    std::shared_ptr<CommitBackend> prepared;
    CommitWorkflow workflow(uwfMaintenanceRepository(system), {[&](const auto& submitted, auto backend) {
                              request = submitted;
                              prepared = std::move(backend);
                            }},
                            source);
    if (registry) {
      if (deleting)
        workflow.commitRegistryDeletionKey(target, QStringLiteral("Setting"));
      else
        workflow.commitRegistryKey(target, QStringLiteral("Setting"));
    } else {
      if (deleting)
        workflow.commitFileDeletionPath(QStringLiteral("C:") + target);
      else
        workflow.commitFilePath(QStringLiteral("C:") + target);
    }
    QVERIFY(prepared);
    system->clearTrace();
    const bool blockedByRom = mode == MaintenanceMode::ReadOnlyMedia;
    if (managementSkips || blockedByRom) {
      QVERIFY_THROWS_EXCEPTION(std::runtime_error, (void)prepared->prepare(request, {}, {}, {}));
    } else {
      const auto plan = prepared->prepare(request, {}, {}, {});
      QCOMPARE(plan.progress().failed, 0);
      const auto targets = plan.targets();
      QCOMPARE(targets.size(), 1);
      QVERIFY(prepared->execute(targets.constFirst(), request.operation).succeeded);
    }
    for (const auto& entry : system->trace()) {
      if (managementSkips || blockedByRom) QVERIFY(!compat::isWrite(entry.request));
      if (const auto* read = std::get_if<compat::GetNumber>(&entry.request); read && read->property == compat::NumberProperty::OverlayFlags)
        QCOMPARE(read->session, compat::Session::Current);
    }
    system->simulateRestart();
    const auto expected = blockedByRom      ? std::optional<std::string>{"base"}
                          : managementSkips ? std::optional<std::string>{driverBypasses ? "changed" : "base"}
                          : deleting        ? std::optional<std::string>{}
                                            : std::optional<std::string>{"changed"};
    QCOMPARE(registry ? system->visibleRegistryValue(key, "Setting") : system->visibleFile(volume, relative), expected);
  }
  void allCommitBatchesUseCurrentSessionAdmission_data() {
    QTest::addColumn<MaintenanceMode>("mode");
    QTest::addColumn<MaintenanceOperation>("operation");
    for (const auto& [modeName, mode] :
         {std::pair{"ordinary", MaintenanceMode::Ordinary}, std::pair{"rom", MaintenanceMode::ReadOnlyMedia}, std::pair{"horm", MaintenanceMode::Horm}})
      for (const auto& [operationName, operation] :
           {std::pair{"file-workflow", MaintenanceOperation::FileWorkflow}, std::pair{"registry-workflow", MaintenanceOperation::RegistryWorkflow},
            std::pair{"registry-batch", MaintenanceOperation::RegistryBatch}, std::pair{"registry-deletion", MaintenanceOperation::RegistryDeletion}})
        QTest::newRow((std::string(modeName) + '/' + operationName).c_str()) << mode << operation;
  }
  void allCommitBatchesUseCurrentSessionAdmission() {
    QFETCH(MaintenanceMode, mode);
    QFETCH(MaintenanceOperation, operation);
    const auto currentMode = mode;
    const auto action = operation;
    constexpr auto volume = "11111111-2222-3333-4444-555555555555";
    constexpr auto key = "HKEY_LOCAL_MACHINE\\Software\\MaintenanceTest";
    constexpr auto value = "setting";
    uwf::simulation::FakeUwfBackend::State state;
    state.current.filterEnabled = true;
    state.runtime.fileMaintenancePrefixes[volume] = "C:";
    state.runtime.systemVolumeId = volume;
    if (currentMode != MaintenanceMode::Ordinary) state.current.overlayFlags = compat::flags::readOnlyMedia;
    state.runtime.hormEnabled = currentMode == MaintenanceMode::Horm;
    uwf::simulation::FakeUwfBackend::VolumeConfiguration configured;
    configured.volume = {volume, "C:", true, false, false};
    state.current.volumes.emplace(volume, std::move(configured));
    state.next = state.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(std::move(state));
    system->seedFile(volume, "\\file.txt", "base");
    system->writeFile(volume, "\\file.txt", "changed");
    system->seedRegistryValue(key, value, "base");
    system->writeRegistryValue(key, value, "changed");
    const auto before = system->snapshot();
    const auto repository = uwfMaintenanceRepository(system);
    const auto execute = [&] {
      if (action == MaintenanceOperation::FileWorkflow || action == MaintenanceOperation::RegistryWorkflow) {
        CommitRequest request;
        std::shared_ptr<CommitBackend> backend;
        CommitWorkflow workflow(repository, {[&](const auto& submitted, auto prepared) {
                                  request = submitted;
                                  backend = std::move(prepared);
                                }},
                                std::make_shared<Files>());
        if (action == MaintenanceOperation::FileWorkflow)
          workflow.commitFilePath(QStringLiteral("C:\\file.txt"));
        else
          workflow.commitRegistryKey(QString::fromUtf8(key), QString::fromUtf8(value));
        const auto plan = backend->prepare(request, {}, {}, {});
        if (plan.targets().size() != 1) throw std::logic_error("expected one ordinary commit target");
        if (!backend->execute(request.target, CommitOperation::Persist).succeeded) throw std::logic_error("ordinary commit failed");
      } else {
        RegistryBatchCommitter committer(repository);
        const std::vector<RegistryCommitTarget> targets{{key, value}};
        const auto result = action == MaintenanceOperation::RegistryBatch ? committer.commit(targets) : committer.persistDeletions(targets);
        if (!result.succeeded() || result.committed != 1) throw std::logic_error("ordinary registry batch failed");
      }
    };
    const bool allowed = currentMode == MaintenanceMode::Ordinary;
    if (!allowed) {
      QVERIFY_THROWS_EXCEPTION(OperationError, execute());
      const auto after = system->snapshot();
      QCOMPARE(after.durable, before.durable);
      QCOMPARE(after.overlay, before.overlay);
      for (const auto& entry : system->trace()) QVERIFY(!compat::isWrite(entry.request));
    } else {
      execute();
      if (action == MaintenanceOperation::FileWorkflow)
        QCOMPARE(system->readFile(volume, "\\file.txt"), std::optional<std::string>{"changed"});
      else if (action == MaintenanceOperation::RegistryDeletion)
        QVERIFY(!system->readRegistryValue(key, value));
      else
        QCOMPARE(system->readRegistryValue(key, value), std::optional<std::string>{"changed"});
    }
  }
  void commitAdmissionUsesPublishedFactsWithoutRequeryingGetters_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("readFlags");
    QTest::addColumn<compat::ErrorKind>("failureKind");
    for (const bool registry : {false, true})
      for (const bool readFlags : {false, true})
        for (const auto& [name, kind] : {std::pair{"unavailable", compat::ErrorKind::Unavailable}, std::pair{"unsupported", compat::ErrorKind::Unsupported}})
          QTest::newRow(qPrintable(QStringLiteral("registry-%1/flags-%2/%3").arg(registry).arg(readFlags).arg(QString::fromLatin1(name))))
              << registry << readFlags << kind;
  }
  void commitAdmissionUsesPublishedFactsWithoutRequeryingGetters() {
    QFETCH(bool, registry);
    QFETCH(bool, readFlags);
    QFETCH(compat::ErrorKind, failureKind);
    constexpr auto volume = "11111111-2222-3333-4444-555555555555";
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    initial.runtime.systemVolumeId = volume;
    initial.current.volumes.emplace(
        volume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {volume, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    initial.next = initial.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    const compat::Request required = readFlags ? compat::Request{compat::GetNumber{compat::NumberProperty::OverlayFlags, compat::Session::Current}}
                                               : compat::Request{compat::GetBoolean{compat::BooleanProperty::HormEnabled}};
    system->inject({.request = required,
                    .operation = {},
                    .timing = uwf::simulation::FakeUwfBackend::FaultTiming::Before,
                    .kind = failureKind,
                    .message = "Current prerequisite observation failed: original diagnostic"});
    const auto repository = uwfMaintenanceRepository(system);
    QCOMPARE(registry ? repository->registryCommitRequirement() : repository->fileCommitRequirement(test::application::fixtureVolumeId),
             CommitRequirement::RequiresUwfCommit);
    QVERIFY(system->trace().empty());
    QCOMPARE(system->snapshot(), initial);
    for (const auto& entry : system->trace()) QVERIFY(!compat::isWrite(entry.request));
  }
  void interactiveCommitKeepsThePreparedVolume() {
    auto repository = std::make_shared<Repository>();
    const auto original = repository->target;
    CommitRequest request;
    std::shared_ptr<CommitBackend> backend;
    CommitWorkflow workflow(repository, {[&](const auto& r, auto b) {
                              request = r;
                              backend = std::move(b);
                            }},
                            std::make_shared<Files>());
    workflow.commitFilePath(QStringLiteral("C:\\file.txt"));
    const auto plan = backend->prepare(request, {}, {}, {});
    QCOMPARE(plan.targets().size(), 1);
    repository->target = std::make_shared<Volume>();
    QVERIFY(backend->execute(request.target, CommitOperation::Persist).succeeded);
    QCOMPARE(original->committed.size(), 1);
    QVERIFY(repository->target->committed.isEmpty());
    QCOMPARE(repository->volumeReads, 1);
  }
  void registryDeletionRecognizesNewValuesByActualObjectIdentity_data() {
    QTest::addColumn<QString>("originalName");
    QTest::addColumn<QString>("currentName");
    QTest::addColumn<bool>("sameObject");
    QTest::newRow("ascii-alias") << QStringLiteral("Entry") << QStringLiteral("eNTRY") << true;
    QTest::newRow("unicode-alias") << QString(QChar(0x00c4)) << QString(QChar(0x00e4)) << true;
    QTest::newRow("new-greek-mu-is-not-micro-sign") << QString(QChar(0x00b5)) << QString(QChar(0x03bc)) << false;
  }
  void registryDeletionRecognizesNewValuesByActualObjectIdentity() {
    QFETCH(QString, originalName);
    QFETCH(QString, currentName);
    QFETCH(bool, sameObject);
    constexpr auto volume = "11111111-2222-3333-4444-555555555555";
    const auto key = QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\IdentityTest");
    uwf::simulation::FakeUwfBackend::State state;
    state.current.filterEnabled = true;
    state.runtime.systemVolumeId = volume;
    state.current.volumes.emplace(
        volume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {volume, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    state.next = state.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(std::move(state));
    system->seedRegistryValue(key.toStdString(), originalName.toStdString(), "original");
    const auto source = std::make_shared<RegistryValues>();
    source->system = system;
    source->key = key;
    source->names = {originalName};
    const auto repository = uwfMaintenanceRepository(system);
    CommitRequest request;
    std::shared_ptr<CommitBackend> prepared;
    CommitWorkflow workflow(repository, {[&](const auto& submitted, auto backend) {
                              request = submitted;
                              prepared = std::move(backend);
                            }},
                            source);
    workflow.commitRegistryDeletionKey(key, {});
    const auto plan = prepared->prepare(request, {}, {}, {});
    QCOMPARE(plan.targets().size(), 1);
    QCOMPARE(plan.targets().constFirst().kind, CommitTargetKind::RegistryKey);

    system->writeRegistryValue(key.toStdString(), currentName.toStdString(), "arrived after preparation");
    source->names = sameObject ? QStringList{currentName} : QStringList{originalName, currentName};
    system->clearTrace();
    const auto result = prepared->execute(plan.targets().constFirst(), CommitOperation::DeleteAndPersist);
    QCOMPARE(result.succeeded, sameObject);
    QCOMPARE(system->visibleRegistryKey(key.toStdString()), !sameObject);
    const auto trace = system->trace();
    QCOMPARE(std::ranges::count_if(trace, [](const auto& entry) { return std::holds_alternative<compat::CommitRegistryDeletion>(entry.request); }),
             sameObject ? 1 : 0);
    if (!sameObject) {
      QVERIFY(result.failureDetail.contains(QStringLiteral("new values")));
      QCOMPARE(system->visibleRegistryValue(key.toStdString(), originalName.toStdString()), std::optional<std::string>{"original"});
      QCOMPARE(system->visibleRegistryValue(key.toStdString(), currentName.toStdString()), std::optional<std::string>{"arrived after preparation"});
    }
  }
  void registryWorkflowKeepsTheSelectedObject_data() {
    QTest::addColumn<QString>("suffix");
    QTest::addColumn<QString>("counterpart");
    QTest::addColumn<bool>("alias");
    QTest::addColumn<bool>("wholeKey");
    QTest::addColumn<bool>("deleting");
    const QList<QPair<QString, QString>> names{{QStringLiteral("Vendor "), QStringLiteral("Vendor")},
                                               {QStringLiteral("Parent\\..\\Leaf"), QStringLiteral("Leaf")},
                                               {QStringLiteral("Vendor"), QStringLiteral("Sibling")}};
    for (qsizetype index = 0; index < names.size(); ++index)
      for (const bool alias : {false, true})
        for (const bool whole : {false, true})
          for (const bool deleting : {false, true})
            QTest::newRow(qPrintable(QStringLiteral("name-%1-alias-%2-whole-%3-delete-%4").arg(index).arg(alias).arg(whole).arg(deleting)))
                << names.at(index).first << names.at(index).second << alias << whole << deleting;
  }
  void registryWorkflowKeepsTheSelectedObject() {
    QFETCH(QString, suffix);
    QFETCH(QString, counterpart);
    QFETCH(bool, alias);
    QFETCH(bool, wholeKey);
    QFETCH(bool, deleting);
    const auto root = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\ObjectSelection\\");
    const auto selected = root + suffix;
    const auto sibling = root + counterpart;
    const auto value = QStringLiteral("Setting ");
    const auto input = alias ? QStringLiteral("hKlM") + selected.mid(QStringLiteral("HKEY_LOCAL_MACHINE").size()) : selected;
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    constexpr auto systemVolume = "11111111-2222-3333-4444-555555555555";
    initial.runtime.systemVolumeId = systemVolume;
    initial.current.volumes.emplace(systemVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{
                                                      .volume = {systemVolume, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    initial.next = initial.current;
    const auto system = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    system->seedRegistryValue(selected.toStdString(), value.toStdString(), "selected-base");
    system->seedRegistryValue(sibling.toStdString(), value.toStdString(), "sibling-base");
    system->writeRegistryValue(selected.toStdString(), value.toStdString(), "selected-change");
    const auto source = std::make_shared<RegistryValues>();
    source->system = system;
    source->key = selected;
    source->names = {value};
    CommitRequest submitted;
    std::shared_ptr<CommitBackend> backend;
    CommitWorkflow workflow(uwfMaintenanceRepository(system), {[&](const auto& request, auto prepared) {
                              submitted = request;
                              backend = std::move(prepared);
                            }},
                            source);
    const auto namedValue = wholeKey ? std::nullopt : std::optional<QString>{value};
    if (deleting)
      workflow.commitRegistryDeletionKey(input, namedValue);
    else
      workflow.commitRegistryKey(input, namedValue);
    QCOMPARE(submitted.target.path, selected);
    QCOMPARE(submitted.target.valueName, namedValue.value_or(QString{}));
    QVERIFY(backend);
    const auto plan = backend->prepare(submitted, {}, {}, {});
    QCOMPARE(plan.progress().failed, 0);
    const auto targets = plan.targets();
    QCOMPARE(targets.size(), 1);
    for (const auto& target : targets) {
      QCOMPARE(target.path, selected);
      const auto result = backend->execute(target, submitted.operation);
      QVERIFY(result.succeeded);
    }
    system->simulateRestart();
    QCOMPARE(system->visibleRegistryValue(selected.toStdString(), value.toStdString()),
             deleting ? std::optional<std::string>{} : std::optional<std::string>{"selected-change"});
    QCOMPARE(system->visibleRegistryValue(sibling.toStdString(), value.toStdString()), std::optional<std::string>{"sibling-base"});
    if (deleting && wholeKey) QVERIFY(!system->visibleRegistryKey(selected.toStdString()));
    QVERIFY(system->visibleRegistryKey(sibling.toStdString()));
  }
  void stagingPlanOwnsItsPreparedVolumeAcrossCommitterLifetime() {
    auto repository = std::make_shared<Repository>();
    const auto original = repository->target;
    auto operation = [&] {
      FileStagingCommitter committer(repository, std::make_shared<Files>());
      return committer.beginCommit(committer.prepare(FileStagingCommitter::scan(committer.prepareScan({{FileStagingKind::File, "C:\\file.txt"}}))));
    }();
    repository->target = std::make_shared<Volume>();
    (void)operation.advance();
    QCOMPARE(operation.result().committedFiles, std::size_t{1});
    QCOMPARE(original->committed.size(), 1);
    QVERIFY(repository->target->committed.isEmpty());
    QCOMPARE(repository->volumeReads, 1);
  }
  void registryBatchReadsOnceAndCommitsEveryValue() {
    auto repository = std::make_shared<Repository>();
    RegistryBatchCommitter committer(repository);
    const auto result = committer.commit({{"HKLM\\Software\\Test", "one"}, {"HKLM\\Software\\Test", "two"}});
    QCOMPARE(result.committed, std::size_t{2});
    QCOMPARE(repository->registryReads, 1);
    QCOMPARE(repository->keys->commits, 2);
  }
  void explicitCommitOperationChoosesTheVerificationPostcondition_data() {
    QTest::addColumn<bool>("deleting");
    QTest::addColumn<bool>("presentAfter");
    QTest::addColumn<bool>("expectedSuccess");
    QTest::newRow("persist-existing") << false << true << true;
    QTest::newRow("persist-absent") << false << false << true;
    QTest::newRow("delete-absent") << true << false << true;
    QTest::newRow("delete-still-existing") << true << true << false;
  }
  void explicitCommitOperationChoosesTheVerificationPostcondition() {
    QFETCH(bool, deleting);
    QFETCH(bool, presentAfter);
    QFETCH(bool, expectedSuccess);
    int calls = 0;
    int observations = 0;
    auto commit = [&](int) { ++calls; };
    auto exists = [&](int) { return ++observations == 1 || presentAfter; };
    const auto operation = deleting ? CommitOperation::DeleteAndPersist : CommitOperation::Persist;
    const auto result = executeCommitTarget(0, "target", operation, commit, exists);
    QCOMPARE(calls, 1);
    QCOMPARE(observations, 2);
    QCOMPARE(result.succeeded, expectedSuccess);
    QCOMPARE(result.existsAfter, std::optional<bool>{presentAfter});
    QCOMPARE(result.failureEffect, expectedSuccess ? WriteEffect::None : WriteEffect::Uncertain);
  }
  void deletionWithoutAnObserverIsRejectedBeforeMutation() {
    int calls = 0;
    auto commit = [&](int) { ++calls; };
    decltype(nullptr) noObserver = nullptr;
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)executeCommitTarget(0, "target", CommitOperation::DeleteAndPersist, commit, noObserver));
    QCOMPARE(calls, 0);
  }
  void semanticFailureNeedsNoNativeCode() {
    auto commit = [](int) { throw OperationError(std::make_error_code(std::errc::no_such_file_or_directory), "missing"); };
    decltype(nullptr) probe = nullptr;
    const auto result = executeCommitTarget(0, "target", CommitOperation::Persist, commit, probe);
    QVERIFY(!result.succeeded);
    QCOMPARE(result.failureKind, OperationFailureKind::NotFound);
    QCOMPARE(result.diagnostic.code, 0);
    QCOMPARE(result.diagnostic.result, uint32_t{0});
  }
  void visibleAbsenceDoesNotSkipPersistentDeletion() {
    int calls = 0;
    auto commit = [&](int) { ++calls; };
    auto probe = [](int) { return false; };
    const auto result = executeCommitTarget(0, "target", CommitOperation::DeleteAndPersist, commit, probe);
    QVERIFY(result.succeeded);
    QCOMPARE(calls, 1);
  }
  void visibleAbsenceDoesNotConfirmUncertainDurableDeletion() {
    auto commit = [](int) { throw OperationError(std::make_error_code(std::errc::io_error), "unconfirmed", WriteEffect::Uncertain); };
    int reads = 0;
    auto probe = [&](int) { return ++reads == 1; };
    const auto result = executeCommitTarget(0, "target", CommitOperation::DeleteAndPersist, commit, probe);
    QVERIFY(!result.succeeded);
    QCOMPARE(result.failureEffect, WriteEffect::Uncertain);
    QVERIFY(result.existsAfter && !*result.existsAfter);
    QCOMPARE(result.failureDetail, QString::fromUtf8(OperationError(std::make_error_code(std::errc::io_error), "unconfirmed", WriteEffect::Uncertain).what()));
    QCOMPARE(reads, 2);
  }
  void notFoundPreservesItsWriteEffect_data() {
    QTest::addColumn<WriteEffect>("effect");
    QTest::newRow("not-dispatched") << WriteEffect::None;
    QTest::newRow("earlier-step-confirmed") << WriteEffect::Confirmed;
    QTest::newRow("partial-result-unknown") << WriteEffect::Uncertain;
  }
  void notFoundPreservesItsWriteEffect() {
    QFETCH(WriteEffect, effect);
    int attempts = 0;
    auto commit = [&](int) {
      ++attempts;
      throw OperationError(std::make_error_code(std::errc::no_such_file_or_directory), "missing", effect);
    };
    auto probe = [](int) { return false; };
    const auto result = executeCommitTarget(0, "target", CommitOperation::DeleteAndPersist, commit, probe);
    QCOMPARE(attempts, 1);
    QVERIFY(!result.succeeded);
    QCOMPARE(result.failureKind, OperationFailureKind::NotFound);
    QCOMPARE(result.failureEffect, effect);
    QCOMPARE(result.existedBefore, std::optional{false});
    QCOMPARE(result.existsAfter, std::optional{false});
  }
};
}  // namespace
QTEST_APPLESS_MAIN(MaintenanceRepositoryTests)
#include "MaintenanceRepositoryTests.moc"
