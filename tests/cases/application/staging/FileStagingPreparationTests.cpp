// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTest>
#include <QThreadPool>
#include <algorithm>
#include <atomic>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/operations/contracts/OperationError.h"
#include "application/staging/execution/FileStagingTask.h"
#include "application/uwf/UwfRequestPrecheck.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "support/app/CommitScenario.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/UwfMaintenanceFixture.h"

namespace {
using namespace uwf;
using namespace uwf::app;
using namespace uwf::windows;
using uwf::test::application::UwfMaintenanceFixture;

// The scale fixture records traversal and dispatch; smaller integration cases verify provider admission and durable data.
class RecordingStagingRepository final : public UwfMaintenanceRepository {
 public:
  class Target final : public UwfVolumeTarget {
   public:
    core::VolumeRecord record{test::application::fixtureVolumeId, "C:", true};
    std::vector<std::string> patterns;
    mutable int exclusionReads = 0;
    mutable std::vector<std::string> committed;
    const core::VolumeRecord& volume() const override { return record; }
    std::vector<std::string> exclusions() const override {
      ++exclusionReads;
      return patterns;
    }
    WriteEffect commitFile(const std::string& path) const override {
      committed.push_back(path);
      return WriteEffect::Confirmed;
    }
    WriteEffect commitFileDeletion(const std::string&) const override { throw std::logic_error("unexpected file deletion"); }
  };
  const std::shared_ptr<Target> target = std::make_shared<Target>();
  mutable int admissionReads = 0;
  mutable int volumeReads = 0;
  std::set<std::string> pausedVolumes;
  AutomaticCommitDisposition pausedDisposition = AutomaticCommitDisposition::SkipRetainedOverlay;
  mutable int refreshes = 0;
  std::function<void()> beforeRefresh;
  void refreshAutomaticCommitState() const override {
    ++refreshes;
    if (beforeRefresh) beforeRefresh();
  }
  AutomaticCommitDisposition automaticFileCommitDisposition(const std::string& volumeId) const override {
    return pausedVolumes.contains(volumeId) ? pausedDisposition : AutomaticCommitDisposition::CheckDurability;
  }
  CommitRequirement fileCommitRequirement(const std::string& volumeId) const override {
    if (volumeId != target->record.volumeName) throw std::logic_error("unexpected volume identity");
    ++admissionReads;
    return CommitRequirement::RequiresUwfCommit;
  }
  std::vector<std::shared_ptr<UwfVolumeTarget>> prepareVolumeTargets() const override {
    ++volumeReads;
    return {target};
  }
  CommitRequirement registryCommitRequirement() const override { throw std::logic_error("unexpected registry admission"); }
  CommitRequirement registryWriteRequirement() const override { throw std::logic_error("unexpected registry write admission"); }
  std::shared_ptr<UwfRegistryMaintenanceTarget> prepareRegistryMaintenance() const override { throw std::logic_error("unexpected registry target"); }
  std::shared_ptr<UwfRegistryExclusionConfiguration> prepareRegistryExclusions() const override { throw std::logic_error("unexpected registry configuration"); }
  std::function<void()> preparePowerAction(PowerAction) const override { throw std::logic_error("unexpected power action"); }
};

class BoundStagingSource final : public CommitTraversalSource {
 public:
  const QString root = QStringLiteral("C:\\Staged");
  const int files = 32;
  mutable QStringList resolvedPaths;
  mutable int inspections = 0;
  mutable int enumerations = 0;
  std::function<void(std::stop_token)> beforeEnumeration;
  std::optional<CommitNode> inspect(const CommitNode& target) const override {
    ++inspections;
    auto node = target.fileTarget ? target : bindFileCommitTarget(*this, target);
    node.kind = node.path == root ? CommitTargetKind::Folder : CommitTargetKind::File;
    return node;
  }
  bool exists(const CommitNode&) const override { return true; }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  FileCommitTarget resolveFileTarget(const QString& path) const override {
    resolvedPaths.append(path);
    return test::application::memoryFileTarget(path);
  }
  QString enumerateChildren(const CommitNode& node, std::stop_token stop, const CommitNodeVisitor& discover) const override {
    ++enumerations;
    if (beforeEnumeration) beforeEnumeration(stop);
    if (node.kind != CommitTargetKind::Folder) return {};
    for (int index = 0; index < files && !stop.stop_requested(); ++index) {
      const auto path = root + QStringLiteral("\\%1.bin").arg(index);
      discover({CommitTargetKind::File, path, {}, {}, test::application::memoryFileTarget(path, node.fileTarget->volumeId)});
    }
    return {};
  }
};

QString nativePath(const QString& path) { return QDir::toNativeSeparators(path); }

bool createFile(const QString& path) {
  const qsizetype separator = path.lastIndexOf(u'/');
  if (separator >= 0 && !QDir().mkpath(path.first(separator))) return false;
  QFile file(path);
  return file.open(QIODevice::WriteOnly);
}

QStringList fixtureFiles() {
  return {QStringLiteral("cache/inside.bin"),
          QStringLiteral("cache.child/sibling.bin"),
          QStringLiteral("saved.bin"),
          QStringLiteral("Ärger/Öl.bin"),
          QStringLiteral("Ärger.child/sibling.bin"),
          QStringLiteral("中文文件/inside.bin"),
          QStringLiteral("中文文件.child/sibling.bin")};
}

class FileStagingPreparationTests final : public QObject {
  Q_OBJECT
 private slots:
  void finalRevalidationReusesDiscoveryAndResumesEveryPauseReason_data() {
    QTest::addColumn<int>("reason");
    for (const auto reason : {AutomaticCommitDisposition::SkipHormActive, AutomaticCommitDisposition::SkipHormTransition,
                              AutomaticCommitDisposition::SkipFullCommit, AutomaticCommitDisposition::SkipReadOnlyMedia,
                              AutomaticCommitDisposition::SkipRetainedOverlay})
      QTest::newRow(qPrintable(QString::number(static_cast<int>(reason)))) << static_cast<int>(reason);
  }
  void finalRevalidationReusesDiscoveryAndResumesEveryPauseReason() {
    QFETCH(int, reason);
    auto source = std::make_shared<BoundStagingSource>();
    auto repository = std::make_shared<RecordingStagingRepository>();
    repository->pausedDisposition = static_cast<AutomaticCommitDisposition>(reason);
    repository->pausedVolumes.insert(test::application::fixtureVolumeId);
    FileStagingTask task(repository, {{FileStagingKind::Directory, source->root}}, UwfCapability::Available, source);
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 2000);
    QVERIFY(task.finished());
    task.revalidatePreparation();
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 2000);
    QVERIFY(task.finished());
    QCOMPARE(source->inspections, 0);
    QCOMPARE(source->enumerations, 0);
    QCOMPARE(repository->refreshes, 1);
    repository->beforeRefresh = [target = repository.get()] { target->pausedVolumes.clear(); };
    task.revalidatePreparation();
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 2000);
    QVERIFY(!task.finished());
    QCOMPARE(task.totalFiles(), std::size_t{32});
    QCOMPARE(source->enumerations, 1);
    QVERIFY(repository->target->committed.empty());
    task.revalidatePreparation();
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 2000);
    QCOMPARE(source->enumerations, 1);
    while (!task.finished()) { (void)task.advance(); task.waitForWorker(); }
    QCOMPARE(repository->target->committed.size(), std::size_t{32});
    QVERIFY_THROWS_EXCEPTION(std::logic_error, task.revalidatePreparation());
  }
  void finalRevalidationCanPauseAnAlreadyScannedScopeWithoutLosingIt() {
    auto source = std::make_shared<BoundStagingSource>();
    auto repository = std::make_shared<RecordingStagingRepository>();
    FileStagingTask task(repository, {{FileStagingKind::Directory, source->root}}, UwfCapability::Available, source);
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 2000);
    QCOMPARE(source->enumerations, 1);
    repository->beforeRefresh = [target = repository.get()] { target->pausedVolumes.insert(test::application::fixtureVolumeId); };
    task.revalidatePreparation();
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 2000);
    QVERIFY(task.finished());
    QCOMPARE(task.totalFiles(), std::size_t{0});
    QVERIFY(task.result().automaticCommit.paused());
    QCOMPARE(source->enumerations, 1);
    repository->beforeRefresh = [target = repository.get()] { target->pausedVolumes.clear(); };
    task.revalidatePreparation();
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 2000);
    QCOMPARE(task.totalFiles(), std::size_t{32});
    QCOMPARE(source->enumerations, 1);
    QVERIFY(repository->target->committed.empty());
  }
  void failedFinalObservationCannotReuseAnEarlierSuccessfulSkip() {
    auto source = std::make_shared<BoundStagingSource>();
    auto repository = std::make_shared<RecordingStagingRepository>();
    repository->pausedVolumes.insert(test::application::fixtureVolumeId);
    FileStagingTask task(repository, {{FileStagingKind::Directory, source->root}}, UwfCapability::Available, source);
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 2000);
    repository->beforeRefresh = [] { throw std::runtime_error("Final Current/Next observation failed"); };
    task.revalidatePreparation();
    task.waitForWorker();
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(task.pollPreparation()));
    QVERIFY(task.failed());
    QVERIFY(!task.finished());
    QCOMPARE(source->enumerations, 0);
    QVERIFY(repository->target->committed.empty());
  }
  void cancelFinalRevalidationStopsNewlyRequiredDiscovery() {
    struct Cancellation {
      std::atomic_bool entered{false}, observed{false}, release{false};
    };
    const auto cancellation = std::make_shared<Cancellation>();
    const auto release = qScopeGuard([&] { cancellation->release = true; });
    auto source = std::make_shared<BoundStagingSource>();
    source->beforeEnumeration = [cancellation](const std::stop_token stop) {
      cancellation->entered = true;
      test::commit::waitForRelease([&] { return !cancellation->release && !stop.stop_requested(); });
      cancellation->observed = stop.stop_requested();
    };
    auto repository = std::make_shared<RecordingStagingRepository>();
    repository->pausedVolumes.insert(test::application::fixtureVolumeId);
    auto task = std::make_unique<FileStagingTask>(repository, QList<FileStagingEntry>{{FileStagingKind::Directory, source->root}},
                                                UwfCapability::Available, source);
    QTRY_VERIFY_WITH_TIMEOUT(task->pollPreparation(), 2000);
    QCOMPARE(source->enumerations, 0);
    repository->beforeRefresh = [target = repository.get()] { target->pausedVolumes.clear(); };
    task->revalidatePreparation();
    QTRY_VERIFY_WITH_TIMEOUT(cancellation->entered.load(), 2000);
    task.reset();
    QTRY_VERIFY_WITH_TIMEOUT(cancellation->observed.load(), 2000);
    QVERIFY(repository->target->committed.empty());
  }
  void knownAutomaticPausesDoNotTraverseOrDispatch_data() {
    QTest::addColumn<int>("reason");
    for (const auto reason : {AutomaticCommitDisposition::SkipHormActive, AutomaticCommitDisposition::SkipHormTransition,
                              AutomaticCommitDisposition::SkipFullCommit, AutomaticCommitDisposition::SkipReadOnlyMedia,
                              AutomaticCommitDisposition::SkipRetainedOverlay})
      QTest::newRow(qPrintable(QString::number(static_cast<int>(reason)))) << static_cast<int>(reason);
  }
  void knownAutomaticPausesDoNotTraverseOrDispatch() {
    QFETCH(int, reason);
    auto source = std::make_shared<BoundStagingSource>();
    auto repository = std::make_shared<RecordingStagingRepository>();
    repository->pausedVolumes.insert(test::application::fixtureVolumeId);
    repository->pausedDisposition = static_cast<AutomaticCommitDisposition>(reason);
    FileStagingTask task(repository, {{FileStagingKind::Directory, source->root}}, UwfCapability::Available, source);
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 2000);
    QVERIFY(task.finished());
    const auto& result = task.result();
    QVERIFY(result.succeeded());
    QCOMPARE(result.pausedEntries, std::size_t{1});
    QCOMPARE(result.committedFiles, std::size_t{0});
    QCOMPARE(result.skippedEntries, std::size_t{0});
    QVERIFY(result.automaticCommit.paused());
    QVERIFY(!result.automaticCommit.alreadyPersistent);
    QCOMPARE(result.automaticCommit.hormTransitionPaused, repository->pausedDisposition == AutomaticCommitDisposition::SkipHormTransition);
    QCOMPARE(source->inspections, 0);
    QCOMPARE(source->enumerations, 0);
    QCOMPARE(repository->admissionReads, 0);
    QCOMPARE(repository->volumeReads, 0);
    QVERIFY(repository->target->committed.empty());
  }
  void unavailableAutomaticStateRemainsAnInfrastructureFailure_data() {
    QTest::addColumn<bool>("startupUnavailable");
    QTest::newRow("startup-capability") << true;
    QTest::newRow("published-mode") << false;
  }
  void unavailableAutomaticStateRemainsAnInfrastructureFailure() {
    QFETCH(bool, startupUnavailable);
    auto source = std::make_shared<BoundStagingSource>();
    auto repository = std::make_shared<RecordingStagingRepository>();
    repository->pausedVolumes.insert(test::application::fixtureVolumeId);
    repository->pausedDisposition = AutomaticCommitDisposition::Unavailable;
    FileStagingTask task(repository, {{FileStagingKind::Directory, source->root}},
                         startupUnavailable ? UwfCapability::Unavailable : UwfCapability::Available, source);
    bool failed = false;
    for (int attempt = 0; attempt < 2000 && !failed; ++attempt) {
      try {
        (void)task.pollPreparation();
      } catch (const std::exception&) {
        failed = true;
      }
      if (!failed) QTest::qWait(1);
    }
    QVERIFY(failed);
    QVERIFY(task.failed());
    QCOMPARE(source->inspections, 0);
    QCOMPARE(source->enumerations, 0);
    QVERIFY(repository->target->committed.empty());
  }
  void persistentOverlayPausesBeforeScanningAndResumesWhenRequired() {
    auto source = std::make_shared<BoundStagingSource>();
    auto repository = std::make_shared<RecordingStagingRepository>();
    FileStagingCommitter committer(repository, source);
    const QList<FileStagingEntry> entries{{FileStagingKind::Directory, source->root}};
    repository->pausedVolumes.insert(test::application::fixtureVolumeId);
    auto pausedPlan = committer.prepare(FileStagingCommitter::scan(committer.prepareScan(entries)));
    QCOMPARE(pausedPlan.fileCount(), std::size_t{0});
    QCOMPARE(repository->admissionReads, 0);
    QCOMPARE(repository->volumeReads, 0);
    QCOMPARE(source->resolvedPaths, QStringList{source->root});
    auto paused = committer.beginCommit(std::move(pausedPlan));
    QVERIFY(paused.finished());
    QVERIFY(paused.result().succeeded());
    QCOMPARE(paused.result().pausedEntries, std::size_t{1});
    QCOMPARE(paused.result().committedFiles, std::size_t{0});
    QCOMPARE(paused.result().skippedEntries, std::size_t{0});
    QVERIFY(paused.result().automaticCommit.retainedOverlayPaused);
    QCOMPARE(repository->admissionReads, 0);

    // Revoking retention between discovery and execution must not silently
    // acknowledge the previously paused directory as a completed commit.
    auto resumingPlan = committer.prepare(FileStagingCommitter::scan(committer.prepareScan(entries)));
    repository->pausedVolumes.clear();
    auto resumed = committer.beginCommit(std::move(resumingPlan));
    QCOMPARE(resumed.totalFiles(), static_cast<std::size_t>(source->files));
    while (!resumed.finished()) (void)resumed.advance();
    QVERIFY(resumed.result().succeeded());
    QCOMPARE(resumed.result().pausedEntries, std::size_t{0});
    QCOMPARE(resumed.result().committedFiles, static_cast<std::size_t>(source->files));
    QVERIFY(!resumed.result().automaticCommit.paused());
  }
  void automaticCommitRechecksPauseBeforeEachDispatch() {
    auto source = std::make_shared<BoundStagingSource>();
    auto repository = std::make_shared<RecordingStagingRepository>();
    FileStagingCommitter committer(repository, source);
    auto operation = committer.beginCommit(
        committer.prepare(FileStagingCommitter::scan(committer.prepareScan({{FileStagingKind::File, source->root + QStringLiteral("\\0.bin")}}))));
    repository->pausedVolumes.insert(test::application::fixtureVolumeId);
    (void)operation.advance();
    QVERIFY(operation.finished());
    QVERIFY(operation.result().succeeded());
    QCOMPARE(operation.result().pausedFiles, std::size_t{1});
    QCOMPARE(operation.result().committedFiles, std::size_t{0});
    QVERIFY(operation.result().automaticCommit.retainedOverlayPaused);
    QVERIFY(repository->target->committed.empty());
  }
  void stagingPlansCheckGuidOnlyExclusionsInBothSessions_data() {
    QTest::addColumn<bool>("current");
    QTest::newRow("current") << true;
    QTest::newRow("next") << false;
  }
  void stagingPlansCheckGuidOnlyExclusionsInBothSessions() {
    QFETCH(bool, current);
    const std::string id = test::application::fixtureVolumeId;
    ConfigurationPlan plan;
    const auto path = QString::fromStdString(compat::volumePath(id)) + QStringLiteral("Data\\file.bin");
    plan.staging = FileStagingChanges{{}, {{FileStagingKind::File, path, FileCommitTarget{id, QStringLiteral("\\Data\\file.bin")}}}, false};
    auto& session = current ? plan.snapshot.current : plan.snapshot.next;
    session.volumes = {{test::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::VolumeId, std::string{}}};
    session.fileExclusions[test::application::fixtureVolumeCName] = {"\\Data"};
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, validateFileStagingTarget(plan));
    // A different owning GUID must never lend this exclusion to the file.
    session.fileExclusions.clear();
    session.fileExclusions[test::application::fixtureVolumeDName] = {"\\Data"};
    validateFileStagingTarget(plan);
  }
  void stagingPlanEditsUseOneAddressShellAndPreserveProviderPatterns_data() {
    QTest::addColumn<bool>("guid");
    QTest::newRow("drive-address") << false;
    QTest::newRow("guid-address") << true;
  }
  void stagingPlanEditsUseOneAddressShellAndPreserveProviderPatterns() {
    QFETCH(bool, guid);
    const std::string id = test::application::fixtureVolumeId;
    const auto guidRoot = compat::volumePath(id);
    const auto providerLiteral = guidRoot + "Data";
    const auto shell = guid ? guidRoot.substr(0, guidRoot.size() - 1) : std::string("C:");
    ConfigurationPlan plan;
    const auto path = QString::fromStdString(guidRoot + "Data\\file.bin");
    plan.staging = FileStagingChanges{{}, {{FileStagingKind::File, path, FileCommitTarget{id, QStringLiteral("\\Data\\file.bin")}}}, false};
    plan.snapshot.next.volumes = {{test::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::VolumeId, std::string("C:")}};
    plan.snapshot.next.fileExclusions[test::application::fixtureVolumeCName] = {providerLiteral};
    // The provider literal is not an alternate address of \\Data.
    validateFileStagingTarget(plan);
    plan.snapshot.next.fileExclusions[test::application::fixtureVolumeCName].push_back("\\Data");
    plan.changes.removeFileExclusions[id] = {shell + providerLiteral};
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, validateFileStagingTarget(plan));
    plan.changes.removeFileExclusions[id].push_back(shell + "\\Data");
    validateFileStagingTarget(plan);
    plan.changes.addFileExclusions[id] = {shell + "\\Data"};
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, validateFileStagingTarget(plan));
    // Current remains authoritative until restart despite a Next removal.
    plan.changes.addFileExclusions.clear();
    plan.snapshot.current = plan.snapshot.next;
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, validateFileStagingTarget(plan));
  }
  void stagingPlansRejectAddressesReboundToAnotherConfiguredVolume() {
    const std::string id = test::application::fixtureVolumeId;
    ConfigurationPlan plan;
    plan.staging = FileStagingChanges{
        {}, {{FileStagingKind::File, QStringLiteral("D:\\Data\\file.bin"), FileCommitTarget{id, QStringLiteral("\\Data\\file.bin")}}}, false};
    plan.snapshot.next.volumes = {{test::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::VolumeId, std::string("D:")},
                                  {test::application::fixtureVolumeDName, "D:", true, core::VolumeBinding::DriveLetter, std::string("C:")}};
    plan.changes.addFileExclusions[id] = {"C:\\Unrelated"};
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, validateFileStagingTarget(plan));
    plan.changes.addFileExclusions[id] = {"D:\\Unrelated"};
    validateFileStagingTarget(plan);
    plan.changes.addFileExclusions[id] = {"D:\\Data"};
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, validateFileStagingTarget(plan));
  }
  void stagingPlansRejectAmbiguousPhysicalAddressesRegardlessOfRecordOrder() {
    const std::string id = test::application::fixtureVolumeId;
    ConfigurationPlan plan;
    plan.staging = FileStagingChanges{
        {}, {{FileStagingKind::File, QStringLiteral("D:\\Data\\file.bin"), FileCommitTarget{id, QStringLiteral("\\Data\\file.bin")}}}, false};
    plan.snapshot.next.volumes = {{test::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::VolumeId, std::string("D:")},
                                  {test::application::fixtureVolumeDName, "D:", true, core::VolumeBinding::DriveLetter, std::string("D:")}};
    plan.changes.addFileExclusions[test::application::otherFixtureVolumeId] = {"D:\\Unrelated"};
    for (int order = 0; order < 2; ++order) {
      QVERIFY_THROWS_EXCEPTION(std::runtime_error, validateFileStagingTarget(plan));
      std::ranges::reverse(plan.snapshot.next.volumes);
    }
  }
  void stagedScanReusesEnumeratedBindingsAndExecutionRevalidates_data() {
    QTest::addColumn<bool>("directory");
    QTest::newRow("file") << false;
    QTest::newRow("directory") << true;
  }
  void stagedScanReusesEnumeratedBindingsAndExecutionRevalidates() {
    QFETCH(bool, directory);
    auto source = std::make_shared<BoundStagingSource>();
    auto repository = std::make_shared<RecordingStagingRepository>();
    FileStagingCommitter committer(repository, source);
    const auto selected = directory ? source->root : source->root + QStringLiteral("\\0.bin");
    auto plan =
        committer.prepare(FileStagingCommitter::scan(committer.prepareScan({{directory ? FileStagingKind::Directory : FileStagingKind::File, selected}})));
    const auto count = directory ? static_cast<std::size_t>(source->files) : std::size_t{1};
    QCOMPARE(plan.fileCount(), count);
    // One selection admission, then one worker scan boundary; discovery of
    // already-bound children must not resolve their display paths again.
    QCOMPARE(source->resolvedPaths, (QStringList{selected, selected}));
    auto operation = committer.beginCommit(std::move(plan));
    QCOMPARE(source->resolvedPaths.size(), qsizetype{3});
    while (!operation.finished()) (void)operation.advance();
    QVERIFY(operation.result().succeeded());
    QCOMPARE(operation.result().committedFiles, count);
    QCOMPARE(source->resolvedPaths.size(), static_cast<qsizetype>(count + 3));
    QCOMPARE(repository->target->committed.size(), count);
  }
  void taskNeverBlocksItsOwner_data() {
    QTest::addColumn<bool>("duringCommit");
    QTest::newRow("preparation") << false;
    QTest::newRow("commit") << true;
  }
  void taskNeverBlocksItsOwner() {
    QFETCH(bool, duringCommit);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = nativePath(directory.filePath(QStringLiteral("state.bin")));
    QVERIFY(createFile(path));
    const auto later = nativePath(directory.filePath(QStringLiteral("later.bin")));
    QVERIFY(createFile(later));
    UwfMaintenanceFixture fixture;
    (void)fixture.useFileSystemTarget(*systemCommitTraversalSource(), nativePath(directory.path()));
    fixture.stageFiles({path, later});
    std::atomic_bool entered{false};
    std::atomic_bool release{false};
    std::atomic_bool ranOnOwner{false};
    const auto ownerThread = std::this_thread::get_id();
    const auto unblock = qScopeGuard([&] {
      release = true;
      QThreadPool::globalInstance()->waitForDone();
    });
    const auto block = [&] {
      ranOnOwner = std::this_thread::get_id() == ownerThread;
      entered = true;
      QElapsedTimer emergency;
      emergency.start();
      while (!release && emergency.elapsed() < 2000) std::this_thread::yield();
    };
    if (duringCommit)
      fixture.state->beforeCommit = block;
    else
      fixture.state->beforeFilter = block;
    auto task = std::make_unique<FileStagingTask>(fixture.repository(), QList<FileStagingEntry>{{FileStagingKind::File, path}, {FileStagingKind::File, later}},
                                                  UwfCapability::Available, uwf::windows::systemCommitTraversalSource());
    QElapsedTimer elapsed;
    elapsed.start();
    QVERIFY(!task->pollPreparation());
    QVERIFY(elapsed.elapsed() < 100);
    if (duringCommit) {
      QTRY_VERIFY(task->pollPreparation());
      QCOMPARE(fixture.state->commits.size(), 0);  // Preparation must never authorize a write.
      elapsed.restart();
      static_cast<void>(task->advance());
      QVERIFY(elapsed.elapsed() < 100);
    }
    QTRY_VERIFY(entered.load());
    QVERIFY(!ranOnOwner.load());
    elapsed.restart();
    task.reset();
    QVERIFY(elapsed.elapsed() < 100);
    QVERIFY(!release.load());
    release = true;
    QThreadPool::globalInstance()->waitForDone();
    QCOMPARE(fixture.state->commits.size(), duringCommit ? 1 : 0);
  }
  void exclusionsPreserveFileSelection_data() {
    QTest::addColumn<QStringList>("excluded");
    QTest::addColumn<QStringList>("remaining");
    QTest::addColumn<QStringList>("failed");
    const auto files = fixtureFiles();
    const auto without = [&](const QString& file) {
      auto result = files;
      result.removeAll(file);
      return result;
    };
    const auto add = [&](const char* name, const QStringList& patterns, const QStringList& committed, const QStringList& failed = {}) {
      QTest::newRow(name) << patterns << committed << failed;
    };
    add("no-exclusions", {}, files);
    add("exact-file", {QStringLiteral("saved.bin")}, without(QStringLiteral("saved.bin")));
    add("directory-boundary", {QStringLiteral("cache")}, without(QStringLiteral("cache/inside.bin")));
    add("case-insensitive", {QStringLiteral("CACHE")}, without(QStringLiteral("cache/inside.bin")));
    add("one-tail-is-management-only", {QStringLiteral("cache\\")}, without(QStringLiteral("cache/inside.bin")), {QStringLiteral("cache/inside.bin")});
    add("two-tails-have-no-coverage", {QStringLiteral("cache\\\\")}, files);
    add("unicode-case-insensitive", {QStringLiteral("ÄRGER")}, without(QStringLiteral("Ärger/Öl.bin")));
    add("unicode-directory", {QStringLiteral("中文文件")}, without(QStringLiteral("中文文件/inside.bin")));
    add("literal-parent-segments", {QStringLiteral("cache\\..\\cache")}, files);
    add("literal-dot-segment", {QStringLiteral("cache\\.")}, files);
    add("raw-forward-slash-is-not-a-backslash", {QStringLiteral("cache/inside.bin")}, files);
    add("duplicate-exclusions", {QStringLiteral("cache"), QStringLiteral("CACHE\\"), QStringLiteral("cache\\inside.bin")},
        without(QStringLiteral("cache/inside.bin")));
    add("unrelated-prefix", {QStringLiteral("cach"), QStringLiteral("saved.bin.extra")}, files);
    add("literal-dot-is-not-the-staged-root", {QStringLiteral(".")}, files);
    add("exact-staged-root", {QStringLiteral("@staged")}, {});
    add("volume-root", {QStringLiteral("@drive")}, {});
  }

  void exclusionsPreserveFileSelection() {
    QFETCH(QStringList, excluded);
    QFETCH(QStringList, remaining);
    QFETCH(QStringList, failed);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto files = fixtureFiles();
    for (const auto& file : files) QVERIFY(createFile(directory.filePath(file)));
    UwfMaintenanceFixture fixture;
    (void)fixture.useFileSystemTarget(*systemCommitTraversalSource(), nativePath(directory.path()));
    for (const auto& exclusion : excluded) {
      const auto full = exclusion == QStringLiteral("@staged") ? nativePath(directory.path()) : nativePath(directory.path()) + u'\\' + exclusion;
      fixture.state->exclusions.append(exclusion == QStringLiteral("@drive") ? QStringLiteral("\\") : full);
    }
    const auto repository = fixture.repository();
    // Seed host writes through driver coverage: skipped files must already be durable,
    // while management-only matches retain overlay data.
    for (const auto& file : files) {
      const auto path = systemCommitTraversalSource()->resolveFileTarget(nativePath(directory.filePath(file))).relativePath.toStdString();
      fixture.backend->seedFile(fixture.state->volumeId, path, "base");
      fixture.backend->writeFile(fixture.state->volumeId, path, "changed");
    }
    const QList<FileStagingEntry> entries{{FileStagingKind::Directory, nativePath(directory.path())},
                                          {FileStagingKind::File, nativePath(directory.filePath(QStringLiteral("saved.bin")))}};
    FileStagingCommitter committer(repository, uwf::windows::systemCommitTraversalSource());
    auto operation = committer.beginCommit(committer.prepare(FileStagingCommitter::scan(committer.prepareScan(entries))));
    QStringList actual;
    while (!operation.finished()) actual.append(QDir(directory.path()).relativeFilePath(operation.advance()));
    std::ranges::sort(actual);
    std::ranges::sort(remaining);
    QCOMPARE(actual, remaining);
    QCOMPARE(operation.result().succeeded(), failed.isEmpty());
    QStringList actualFailures;
    for (const auto& failure : operation.result().failures) {
      actualFailures.append(QDir(directory.path()).relativeFilePath(failure.path));
      QVERIFY(failure.detail.contains(QStringLiteral("reliable commit")));
    }
    std::ranges::sort(actualFailures);
    std::ranges::sort(failed);
    QCOMPARE(actualFailures, failed);
    QCOMPARE(operation.result().committedFiles, static_cast<std::size_t>(remaining.size()));
    QCOMPARE(fixture.state->commits.size(), remaining.size());
    QCOMPARE(fixture.state->volumeReads, 1);
    QCOMPARE(fixture.state->exclusionReads, 1);
    for (const auto& file : files) {
      const auto path = systemCommitTraversalSource()->resolveFileTarget(nativePath(directory.filePath(file))).relativePath.toStdString();
      QCOMPARE(fixture.backend->readFile(fixture.state->volumeId, path), std::optional<std::string>{failed.contains(file) ? "base" : "changed"});
      QCOMPARE(fixture.backend->visibleFile(fixture.state->volumeId, path), std::optional<std::string>{"changed"});
      QCOMPARE(fixture.backend->readFile(fixture.state->volumeId, path, test::FakeUwfBackend::Storage::Overlay).has_value(), failed.contains(file));
    }
  }

  void allExcludedPlansRevalidateCurrentAdmissionBeforeReportingSuccess_data() {
    QTest::addColumn<bool>("currentRom");
    QTest::addColumn<bool>("nextRom");
    QTest::addColumn<bool>("readFails");
    for (const bool currentRom : {false, true})
      for (const bool nextRom : {false, true})
        for (const bool readFails : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("current-%1/next-%2/read-failure-%3").arg(currentRom).arg(nextRom).arg(readFails)))
              << currentRom << nextRom << readFails;
  }
  void allExcludedPlansRevalidateCurrentAdmissionBeforeReportingSuccess() {
    QFETCH(bool, currentRom);
    QFETCH(bool, nextRom);
    QFETCH(bool, readFails);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = nativePath(directory.filePath(QStringLiteral("excluded.bin")));
    QVERIFY(createFile(path));
    UwfMaintenanceFixture fixture;
    (void)fixture.useFileSystemTarget(*systemCommitTraversalSource(), nativePath(directory.path()));
    fixture.state->exclusions = {path};
    const auto repository = fixture.repository();
    FileStagingCommitter committer(repository, systemCommitTraversalSource());
    auto plan = committer.prepare(FileStagingCommitter::scan(committer.prepareScan({{FileStagingKind::File, path}})));
    QCOMPARE(plan.fileCount(), std::size_t{0});
    auto live = fixture.backend->snapshot();
    live.current.overlayFlags = currentRom ? compat::flags::readOnlyMedia : 0;
    live.next.overlayFlags = nextRom ? compat::flags::readOnlyMedia : 0;
    fixture.backend->seed(live);
    if (readFails)
      fixture.backend->inject({.request = compat::GetNumber{compat::NumberProperty::OverlayFlags, compat::Session::Current},
                               .operation = {},
                               .timing = test::FakeUwfBackend::FaultTiming::Before,
                               .kind = compat::ErrorKind::Unavailable,
                               .message = "Current flags became unavailable after scanning"});
    fixture.backend->clearTrace();
    const auto operation = committer.beginCommit(std::move(plan));
    QVERIFY(operation.finished());
    QCOMPARE(operation.result().skippedEntries, std::size_t{1});
    QCOMPARE(operation.result().skippedFiles, std::size_t{0});
    QCOMPARE(operation.result().committedFiles, std::size_t{0});
    QVERIFY(operation.result().succeeded());
    QCOMPARE(fixture.backend->snapshot(), live);
    QVERIFY(fixture.backend->trace().empty());
  }
  void unknownPublishedFlagsBlockAnExcludedPlanWithoutQuerying_data() {
    QTest::addColumn<bool>("refreshFailed");
    QTest::newRow("unknown-current-flags") << false;
    QTest::newRow("refresh-failed-with-stale-facts") << true;
  }
  void unknownPublishedFlagsBlockAnExcludedPlanWithoutQuerying() {
    QFETCH(bool, refreshFailed);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = nativePath(directory.filePath(QStringLiteral("excluded.bin")));
    QVERIFY(createFile(path));
    UwfMaintenanceFixture fixture;
    (void)fixture.useFileSystemTarget(*systemCommitTraversalSource(), nativePath(directory.path()));
    fixture.state->exclusions = {path};
    (void)fixture.repository();
    const auto system = std::make_shared<PrecheckedUwf>(fixture.backend, UwfFactsSource{}, true,
                                                     [](const compat::Uwf&, const compat::Context&) {
                                                       return compat::Response::fail(compat::ErrorKind::Unavailable, "Refresh failed");
                                                     });
    FileStagingCommitter committer(uwfMaintenanceRepository(system), systemCommitTraversalSource());
    auto plan = committer.prepare(FileStagingCommitter::scan(committer.prepareScan({{FileStagingKind::File, path}})));
    QCOMPARE(plan.fileCount(), std::size_t{0});
    auto published = system->state();
    if (refreshFailed) {
      QVERIFY(system->refreshState().failure);
    } else {
      published.current.overlayFlags = {};
      system->updateState(std::move(published));
    }
    fixture.backend->clearTrace();
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)committer.beginCommit(std::move(plan)));
    QVERIFY(fixture.backend->trace().empty());
  }
  void largePreparationSnapshotsTargetsOnceAndRevalidatesBeforeExecution() {
    constexpr int fileCount = 4096;
    auto source = std::make_shared<test::commit::Tree>(false);
    source->children.clear();
    auto repository = std::make_shared<RecordingStagingRepository>();
    QList<CommitNode> files;
    std::vector<std::string> expectedCommits;
    for (int index = 0; index < fileCount; ++index) {
      const auto path = source->root.path + u'\\' + QString::number(index) + QStringLiteral(".bin");
      files.append({CommitTargetKind::File, path, {}, {}});
      expectedCommits.push_back(source->resolveFileTarget(path).relativePath.toStdString());
    }
    // Staging traverses a bound volume-relative node; index this fixture's
    // synthetic directory using the same identity as that observed node.
    source->children.insert(bindFileCommitTarget(*source, source->root).identity(), std::move(files));
    for (int index = 0; index < 512; ++index) {
      const auto path = source->root.path + QStringLiteral("\\unrelated\\%1").arg(index);
      repository->target->patterns.push_back(source->resolveFileTarget(path).relativePath.toStdString());
    }
    QList<FileStagingEntry> entries{{FileStagingKind::Directory, source->root.path}};
    FileStagingTask task(repository, entries, uwf::app::UwfCapability::Available, source);
    QVERIFY(!task.pollPreparation());
    entries.clear();
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 20000);
    QCOMPARE(task.totalFiles(), static_cast<std::size_t>(fileCount));
    QVERIFY(task.pollPreparation());
    QCOMPARE(repository->admissionReads, 2);  // Snapshot admission, then execution admission; never per-file discovery.
    QCOMPARE(repository->volumeReads, 1);
    QCOMPARE(repository->target->exclusionReads, 1);
    QVERIFY(repository->target->committed.empty());
    while (!task.finished()) {
      static_cast<void>(task.advance());
      task.waitForWorker();
    }
    QVERIFY(task.result().succeeded());
    QCOMPARE(task.result().committedFiles, static_cast<std::size_t>(fileCount));
    QCOMPARE(repository->admissionReads, fileCount + 2);  // Each dispatch still revalidates live durability.
    auto actualCommits = repository->target->committed;
    std::ranges::sort(actualCommits);
    std::ranges::sort(expectedCommits);
    QCOMPARE(actualCommits, expectedCommits);
  }
};
}  // namespace

QTEST_GUILESS_MAIN(FileStagingPreparationTests)
#include "FileStagingPreparationTests.moc"
