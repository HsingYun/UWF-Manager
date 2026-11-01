// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QDir>
#include <QFile>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>
#include <array>
#include <functional>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <utility>

#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/operations/contracts/OperationError.h"
#include "application/staging/execution/FileStagingCommandResult.h"
#include "application/staging/execution/FileStagingTask.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "foundation/logging/Log.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "infrastructure/windows/filesystem/paths/VolumePaths.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/OperationFailureFixture.h"
#include "support/app/UwfMaintenanceFixture.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

class MemoryStagingSource final : public app::CommitTraversalSource {
 public:
  app::CommitTargetKind kind = app::CommitTargetKind::File;
  QString skippedReason;
  QString readError;
  QStringList children;
  std::function<std::optional<app::CommitNode>(const app::CommitNode&, int)> inspectResult;
  std::function<std::string(int)> volumeResult;
  std::function<app::FileCommitTarget(const QString&)> targetResolver;
  mutable int inspections = 0;
  mutable int volumeReads = 0;
  std::string volumeIdentity = fixtureVolumeId;
  bool present = true;
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override {
    ++inspections;
    if (inspectResult) return inspectResult(target, inspections);
    if (!present) return std::nullopt;
    auto node = target;
    node.kind = kind;
    node.skippedReason = skippedReason;
    return node;
  }
  bool exists(const app::CommitNode&) const override { return present; }
  std::string driveLetterForPath(const QString&) const override {
    ++volumeReads;
    return volumeResult ? volumeResult(volumeReads) : "C:";
  }
  app::FileCommitTarget resolveFileTarget(const QString& path) const override {
    if (targetResolver) return targetResolver(path);
    const auto drive = driveLetterForPath(path);
    if (drive.empty()) throw std::runtime_error("No mounted volume identity");
    return memoryFileTarget(path, drive == "D:" ? otherFixtureVolumeId : volumeIdentity);
  }
  QString enumerateChildren(const app::CommitNode&, std::stop_token stop, const app::CommitNodeVisitor& discover) const override {
    for (const auto& path : children)
      if (!stop.stop_requested()) discover({app::CommitTargetKind::File, path, {}, {}});
    return readError;
  }
};

QStringList stagingFailureLogs() {
  QStringList lines;
  for (const auto& line : recentLogLines()) {
    const auto text = QString::fromUtf8(line);
    if (text.contains(QStringLiteral(" E staging] file staging item failed: "))) lines.append(text);
  }
  return lines;
}

class FileStagingCommitterTests final : public QObject {
  Q_OBJECT
 private slots:
  void mountedAliasesCommitOnePhysicalTargetOnce() {
    UwfMaintenanceFixture fixture;
    const auto direct = QStringLiteral("C:\\file.bin");
    const auto alias = QStringLiteral("D:\\MountedC\\file.bin");
    fixture.stageFile(direct);
    auto source = std::make_shared<MemoryStagingSource>();
    source->targetResolver = [=](const QString& path) { return memoryFileTarget(path == alias ? direct : path); };
    app::FileStagingCommitter committer(fixture.repository(), source);
    const auto result = completeFileStaging(committer, {{app::FileStagingKind::File, direct}, {app::FileStagingKind::File, alias}});
    QVERIFY(result.succeeded());
    QCOMPARE(result.discoveredFiles, std::size_t{1});
    QCOMPARE(result.committedFiles, std::size_t{1});
    QCOMPARE(fixture.state->commits.size(), std::size_t{1});
  }

  void completedRequestsRetainTheActualCommitAccounting_data() {
    QTest::addColumn<qulonglong>("discovered");
    QTest::addColumn<qulonglong>("committed");
    QTest::addColumn<qulonglong>("skipped");
    QTest::addColumn<qulonglong>("skippedEntries");
    QTest::addColumn<bool>("failed");
    struct Accounting {
      const char* name;
      qulonglong discovered, committed, skipped, skippedEntries;
    };
    constexpr std::array cases{Accounting{"empty", 0, 0, 0, 0}, Accounting{"committed-file", 1, 1, 0, 0}, Accounting{"skipped-file", 1, 0, 1, 0},
                               Accounting{"skipped-entry", 0, 0, 0, 1}, Accounting{"mixed", 5, 3, 2, 7}};
    for (const auto& accounting : cases)
      for (const bool failed : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2").arg(accounting.name, failed ? "with-failures" : "success")))
            << accounting.discovered << accounting.committed << accounting.skipped << accounting.skippedEntries << failed;
  }
  void completedRequestsRetainTheActualCommitAccounting() {
    QFETCH(qulonglong, discovered);
    QFETCH(qulonglong, committed);
    QFETCH(qulonglong, skipped);
    QFETCH(qulonglong, skippedEntries);
    QFETCH(bool, failed);
    app::FileStagingCommitResult result{static_cast<std::size_t>(discovered),
                                        static_cast<std::size_t>(committed),
                                        static_cast<std::size_t>(skipped),
                                        static_cast<std::size_t>(skippedEntries),
                                        {},
                                        app::WriteEffect::None};
    if (failed) {
      // Entry failures need not have contributed a discovered file. Preserve
      // their separate accounting instead of inventing a failed-file total.
      result.failures.append({QStringLiteral("relative.txt"), app::FileStagingCommitFailureKind::InvalidPath, {}});
      result.failures.append({QStringLiteral("C:\\unread-folder"), app::FileStagingCommitFailureKind::DirectoryEnumerationFailed, {}});
    }
    const auto detail = QStringLiteral("retained operation diagnostics");
    const auto summary = app::toFileStagingCommandResult(result, detail);
    QVERIFY(summary.completed());
    QCOMPARE(summary.outcome, failed ? app::FileStagingCommandOutcome::CompletedWithFailures : app::FileStagingCommandOutcome::Succeeded);
    QCOMPARE(summary.discoveredFiles, static_cast<std::size_t>(discovered));
    QCOMPARE(summary.committedFiles, static_cast<std::size_t>(committed));
    QCOMPARE(summary.skippedFiles, static_cast<std::size_t>(skipped));
    QCOMPARE(summary.skippedEntries, static_cast<std::size_t>(skippedEntries));
    QCOMPARE(summary.failedTargets, failed ? std::size_t{2} : std::size_t{0});
    QCOMPARE(summary.detail, failed ? detail : QString{});
  }

  void allSkippedPlansRetainTheirResolvedIdentityUntilBeginCommit() {
    for (const bool protectedTarget : {false, true}) {
      UwfMaintenanceFixture fixture;
      fixture.state->protectedVolume = protectedTarget;
      if (protectedTarget) fixture.state->exclusions = {QStringLiteral("\\skipped.bin")};
      const auto source = std::make_shared<MemoryStagingSource>();
      app::FileStagingCommitter committer(fixture.repository(), source);
      auto plan = committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan({{app::FileStagingKind::File, QStringLiteral("C:\\skipped.bin")}})));
      QCOMPARE(plan.fileCount(), std::size_t{0});
      QCOMPARE(fixture.state->volumeReads, protectedTarget ? 1 : 0);
      source->volumeIdentity = otherFixtureVolumeId;
      try {
        (void)committer.beginCommit(std::move(plan));
        QFAIL("An all-skipped result cannot certify a path rebound to another volume");
      } catch (const app::OperationError& error) {
        QCOMPARE(error.failureKind(), app::OperationFailureKind::Conflict);
        QCOMPARE(error.writeEffect(), app::WriteEffect::None);
      }
      QVERIFY(fixture.state->commits.empty());
      for (const auto& entry : fixture.backend->trace()) QVERIFY(!compat::isWrite(entry.request));
    }
  }
  void aDriveReboundToAnotherGuidCannotCommitTheSameRelativePath() {
    UwfMaintenanceFixture fixture;
    const auto path = QStringLiteral("C:\\same.bin");
    fixture.stageFile(path);
    const auto source = std::make_shared<MemoryStagingSource>();
    app::FileStagingCommitter committer(fixture.repository(), source);
    auto operation = committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan({{app::FileStagingKind::File, path}}))));
    source->volumeIdentity = otherFixtureVolumeId;
    (void)operation.advance();
    const auto& result = operation.result();
    QCOMPARE(result.committedFiles, std::size_t{0});
    QCOMPARE(result.failures.size(), 1);
    QVERIFY(result.failures.front().operation);
    QCOMPARE(result.failures.front().operation->kind, app::OperationFailureKind::Conflict);
    QCOMPARE(result.failures.front().operation->effect, app::WriteEffect::None);
    QCOMPARE(result.writeEffect(), app::WriteEffect::None);
    QVERIFY(fixture.state->commits.empty());
  }
  void stagingResultsPreserveTypedFailureEffectAndDiagnostic() {
    using namespace app;
    for (const auto kind : {OperationFailureKind::NotFound, OperationFailureKind::Unsupported, OperationFailureKind::Protocol})
      for (const auto effect : {WriteEffect::None, WriteEffect::Confirmed, WriteEffect::Uncertain}) {
        UwfMaintenanceFixture fixture;
        fixture.stageFiles({QStringLiteral("C:\\first.bin"), QStringLiteral("C:\\second.bin")});
        int writes = 0;
        fixture.state->beforeCommit = [&] {
          if (++writes == 2) throw DetailedOperationFailure(kind, effect);
        };
        const auto source = std::make_shared<MemoryStagingSource>();
        FileStagingCommitter committer(fixture.repository(), source);
        const auto result = completeFileStaging(
            committer, {{FileStagingKind::File, QStringLiteral("C:\\first.bin")}, {FileStagingKind::File, QStringLiteral("C:\\second.bin")}});
        QCOMPARE(result.committedFiles, std::size_t{1});
        QCOMPARE(result.failures.size(), 1);
        const auto& failure = result.failures.front();
        QVERIFY(failure.operation);
        QCOMPARE(failure.operation->kind, kind);
        QCOMPARE(failure.operation->effect, effect);
        QCOMPARE(failure.operation->diagnostic.code, int32_t{1234});
        QCOMPARE(failure.operation->diagnostic.result, uint32_t{0x80070005u});
        QVERIFY(failure.detail.contains(QStringLiteral("Original provider failure")));
        QCOMPARE(result.writeEffect(), effect == WriteEffect::Uncertain ? WriteEffect::Uncertain : WriteEffect::Confirmed);
      }
  }
  void storedPatternsDoNotInventSuccessfulStaging_data();
  void storedPatternsDoNotInventSuccessfulStaging();
  void fileStagingCommitterOnlyWritesEligibleCurrentTargets();
  void fileStagingTaskRejectsUnavailableCapabilityWithoutRepositoryCalls();
  void everyFailedTargetIsLoggedBeyondTheVisibleFailureLimit();
  void failuresAreLoggedAtTheirOrigin_data();
  void failuresAreLoggedAtTheirOrigin();
  void successfulAndSkippedTargetsDoNotProduceFailureLogs();
  void providerNotFoundIsFailureForEveryWriteEffect_data();
  void providerNotFoundIsFailureForEveryWriteEffect();
  void batchPrerequisiteFailuresPropagateWithoutTargetResults_data();
  void batchPrerequisiteFailuresPropagateWithoutTargetResults();
  void skippedTargetsNeverInvokeCommit();
  void aTargetRemovedAfterTraversalIsNotRecreated();
};

void FileStagingCommitterTests::storedPatternsDoNotInventSuccessfulStaging_data() {
  QTest::addColumn<int>("tails");
  QTest::addColumn<bool>("directory");
  for (const int tails : {0, 1, 2})
    for (const bool directory : {false, true})
      QTest::newRow(qPrintable(QStringLiteral("tails-%1-directory-%2").arg(tails).arg(directory))) << tails << directory;
}
void FileStagingCommitterTests::storedPatternsDoNotInventSuccessfulStaging() {
  QFETCH(int, tails);
  QFETCH(bool, directory);
  constexpr auto volume = UwfMaintenanceFixture::volumeId;
  test::FakeUwfBackend::State initial;
  initial.current.filterEnabled = true;
  initial.runtime.fileMaintenancePrefixes[volume] = "C:";
  const std::string pattern = "\\Coverage" + std::string(static_cast<std::size_t>(tails), '\\');
  initial.current.volumes.emplace(
      volume, test::FakeUwfBackend::VolumeConfiguration{.volume = {volume, "C:", true, false, false}, .fileExclusions = {pattern}, .swapfileSize = 0});
  initial.next = initial.current;
  const auto backend = std::make_shared<test::FakeUwfBackend>(initial);
  backend->seedFile(volume, "\\Coverage\\Child.bin", "base");
  backend->writeFile(volume, "\\Coverage\\Child.bin", "changed");
  auto source = std::make_shared<MemoryStagingSource>();
  source->inspectResult = [](const app::CommitNode& target, int) {
    auto observed = target;
    observed.kind = target.path.endsWith(QStringLiteral(".bin")) ? app::CommitTargetKind::File : app::CommitTargetKind::Folder;
    return std::optional{observed};
  };
  if (directory) source->children = {QStringLiteral("C:\\Coverage\\Child.bin")};
  const QList<app::FileStagingEntry> entries{{directory ? app::FileStagingKind::Directory : app::FileStagingKind::File,
                                              directory ? QStringLiteral("C:\\Coverage") : QStringLiteral("C:\\Coverage\\Child.bin")}};
  app::FileStagingCommitter committer(app::uwfMaintenanceRepository(backend), source);
  auto operation = committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan(entries))));
  while (!operation.finished()) static_cast<void>(operation.advance());
  QCOMPARE(operation.result().committedFiles, tails == 2 ? std::size_t{1} : std::size_t{0});
  QCOMPARE(operation.result().failures.size(), tails == 1 ? 1 : 0);
  if (tails == 1) QVERIFY(operation.result().failures.front().detail.contains(QStringLiteral("reliable commit")));
  backend->simulateRestart();
  QCOMPARE(backend->visibleFile(volume, "\\Coverage\\Child.bin"), std::optional<std::string>{tails == 1 ? "base" : "changed"});
}

void FileStagingCommitterTests::everyFailedTargetIsLoggedBeyondTheVisibleFailureLimit() {
  clearLogLines();
  const auto clear = qScopeGuard([] { clearLogLines(); });
  UwfMaintenanceFixture fixture;
  const QString reason = QStringLiteral("拒绝提交：文件正在使用。\nProvider detail retains every line.\n最后一行诊断");
  fixture.state->commitFailure = std::make_exception_ptr(std::runtime_error(reason.toStdString()));
  QList<app::FileStagingEntry> entries;
  for (int index = 1; index <= 100; ++index)
    entries.append({app::FileStagingKind::File, QStringLiteral("C:\\诊断目录\\target-%1.bin").arg(index, 3, 10, QChar('0'))});
  for (const auto& entry : entries) fixture.stageFile(entry.path);
  auto source = std::make_shared<MemoryStagingSource>();
  app::FileStagingCommitter committer(fixture.repository(), source);
  auto operation = committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan(entries))));
  QVERIFY(stagingFailureLogs().isEmpty());
  while (!operation.finished()) static_cast<void>(operation.advance());
  QCOMPARE(fixture.state->commits.size(), 100);
  QCOMPARE(operation.result().failures.size(), 100);
  QCOMPARE(operation.result().committedFiles, std::size_t{0});
  const auto logged = stagingFailureLogs();
  QCOMPARE(logged.size(), 100);
  for (const auto& entry : std::as_const(entries)) {
    const auto expected = QStringLiteral("path=%1 kind=provider-failure error=%2").arg(entry.path, reason);
    QCOMPARE(std::count_if(logged.cbegin(), logged.cend(), [&](const QString& line) { return line.endsWith(expected); }), 1);
  }
  const auto copied = operation.result();
  QCOMPARE(copied.failures.back().path, entries.back().path);
  static_cast<void>(app::toFileStagingCommandResult(operation.result(), QStringLiteral("UI summary")));
  QCOMPARE(stagingFailureLogs(), logged);
}

void FileStagingCommitterTests::failuresAreLoggedAtTheirOrigin_data() {
  using Kind = app::FileStagingCommitFailureKind;
  QTest::addColumn<int>("failureKind");
  QTest::addColumn<bool>("duringCommit");
  QTest::addColumn<QString>("kindName");
  const auto add = [](const char* row, Kind kind, bool commit, const char* name) {
    QTest::newRow(row) << static_cast<int>(kind) << commit << QString::fromLatin1(name);
  };
  add("invalid-path", Kind::InvalidPath, false, "invalid-path");
  add("scan-reparse", Kind::ReparsePoint, false, "reparse-point");
  add("commit-reparse", Kind::ReparsePoint, true, "reparse-point");
  add("scan-inspection", Kind::PathInspectionFailed, false, "path-inspection-failed");
  add("commit-inspection", Kind::PathInspectionFailed, true, "path-inspection-failed");
  add("scan-file-type", Kind::FileTypeChanged, false, "file-type-changed");
  add("commit-file-type", Kind::FileTypeChanged, true, "file-type-changed");
  add("directory-type", Kind::DirectoryTypeChanged, false, "directory-type-changed");
  add("directory-enumeration", Kind::DirectoryEnumerationFailed, false, "directory-enumeration-failed");
  add("preparation-volume", Kind::VolumeResolutionFailed, false, "volume-resolution-failed");
  add("commit-volume", Kind::VolumeResolutionFailed, true, "volume-resolution-failed");
  add("volume-root", Kind::VolumeRoot, false, "volume-root");
  add("commit-provider", Kind::ProviderFailure, true, "provider-failure");
  add("unknown-provider-exception", Kind::UnknownFailure, true, "unknown-failure");
}

void FileStagingCommitterTests::failuresAreLoggedAtTheirOrigin() {
  QFETCH(int, failureKind);
  QFETCH(bool, duringCommit);
  QFETCH(QString, kindName);
  using Kind = app::FileStagingCommitFailureKind;
  const auto kind = static_cast<Kind>(failureKind);
  clearLogLines();
  const auto clear = qScopeGuard([] { clearLogLines(); });
  UwfMaintenanceFixture fixture;
  auto source = std::make_shared<MemoryStagingSource>();
  const QString diagnostic = QStringLiteral("诊断阶段失败。\n原始错误详情第二行。\nUnicode 原文保留");
  app::FileStagingEntry entry{app::FileStagingKind::File, QStringLiteral("C:\\诊断目录\\target.bin")};
  QString expectedPath = entry.path;
  QString expectedDetail;
  bool executing = false;
  switch (kind) {
    case Kind::InvalidPath:
      entry.path = expectedPath = QStringLiteral("relative\\target.bin");
      break;
    case Kind::ReparsePoint:
    case Kind::FileTypeChanged:
    case Kind::PathInspectionFailed:
      source->inspectResult = [kind, duringCommit, diagnostic](const app::CommitNode& target, const int call) -> std::optional<app::CommitNode> {
        auto node = target;
        if (!duringCommit || call > 1) {
          if (kind == Kind::PathInspectionFailed) throw std::runtime_error(diagnostic.toStdString());
          if (kind == Kind::FileTypeChanged) node.kind = app::CommitTargetKind::Folder;
          if (kind == Kind::ReparsePoint) node.skippedReason = QStringLiteral("reparse point");
        }
        return node;
      };
      if (kind == Kind::PathInspectionFailed) expectedDetail = diagnostic;
      break;
    case Kind::DirectoryTypeChanged:
      entry.kind = app::FileStagingKind::Directory;
      break;
    case Kind::DirectoryEnumerationFailed:
      entry.kind = app::FileStagingKind::Directory;
      source->kind = app::CommitTargetKind::Folder;
      source->readError = expectedDetail = diagnostic;
      break;
    case Kind::VolumeResolutionFailed:
      source->volumeResult = [duringCommit, diagnostic, &executing](int) -> std::string {
        if (!duringCommit) return {};
        if (executing) throw std::runtime_error(diagnostic.toStdString());
        return "C:";
      };
      expectedDetail = duringCommit ? diagnostic : QStringLiteral("No mounted volume identity");
      if (duringCommit) kindName = QStringLiteral("path-inspection-failed");
      break;
    case Kind::VolumeRoot:
      entry.kind = app::FileStagingKind::Directory;
      entry.path = expectedPath = QStringLiteral("C:\\");
      source->kind = app::CommitTargetKind::Folder;
      break;
    case Kind::ProviderFailure:
      fixture.stageFile(entry.path);
      fixture.state->commitFailure = std::make_exception_ptr(std::runtime_error(diagnostic.toStdString()));
      expectedDetail = diagnostic;
      break;
    case Kind::UnknownFailure:
      fixture.stageFile(entry.path);
      fixture.state->commitFailure = std::make_exception_ptr(42);
      break;
  }
  app::FileStagingCommitter committer(fixture.repository(), source);
  auto operation = committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan({entry}))));
  executing = true;
  while (!operation.finished()) (void)operation.advance();
  const auto result = operation.result();
  QCOMPARE(result.failures.size(), 1);
  QCOMPARE(result.failures.front().kind, kind == Kind::VolumeResolutionFailed && duringCommit ? Kind::PathInspectionFailed : kind);
  QCOMPARE(result.failures.front().path, expectedPath);
  QCOMPARE(result.failures.front().detail, expectedDetail);
  const auto logged = stagingFailureLogs();
  QCOMPARE(logged.size(), 1);
  QVERIFY2(logged.front().endsWith(QStringLiteral("path=%1 kind=%2 error=%3").arg(expectedPath, kindName, expectedDetail)), qUtf8Printable(logged.front()));
}

void FileStagingCommitterTests::batchPrerequisiteFailuresPropagateWithoutTargetResults_data() {
  QTest::addColumn<int>("source");
  QTest::addColumn<bool>("unknown");
  const QStringList sources{QStringLiteral("volume-snapshot"), QStringLiteral("exclusions-read")};
  for (int source = 0; source < sources.size(); ++source)
    for (const bool unknown : {false, true}) {
      const auto name = sources[source] + (unknown ? QStringLiteral("-unknown") : QStringLiteral("-multiline-utf8"));
      QTest::newRow(qUtf8Printable(name)) << source << unknown;
    }
}

void FileStagingCommitterTests::batchPrerequisiteFailuresPropagateWithoutTargetResults() {
  QFETCH(int, source);
  QFETCH(bool, unknown);
  clearLogLines();
  const auto clear = qScopeGuard([] { clearLogLines(); });
  const auto detail = QStringLiteral("批次前置读取/提交线程初始化失败\n不能包装成空路径的目标失败");
  const auto failure = unknown ? std::make_exception_ptr(42) : std::make_exception_ptr(std::runtime_error(detail.toStdString()));
  UwfMaintenanceFixture fixture;
  if (source == 0) fixture.state->volumeFailure = failure;
  if (source == 1) fixture.state->exclusionFailure = failure;
  auto files = std::make_shared<MemoryStagingSource>();
  app::FileStagingCommitter committer(fixture.repository(), files);
  const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\Data\\批次失败.bin")}};
  bool producedResult = false;
  int caught = 0;
  try {
    static_cast<void>(completeFileStaging(committer, entries));
    producedResult = true;
  } catch (const std::exception& error) {
    QVERIFY(!unknown);
    QCOMPARE(QString::fromUtf8(error.what()), detail);
    ++caught;
  } catch (int marker) {
    QVERIFY(unknown);
    QCOMPARE(marker, 42);
    ++caught;
  }
  QCOMPARE(caught, 1);
  QVERIFY(!producedResult);
  QVERIFY(fixture.state->commits.empty());
  // FileStagingTask owns failure logging; the synchronous primitive only propagates the exception.
  QVERIFY(stagingFailureLogs().isEmpty());
  const auto lines = recentLogLines();
  QVERIFY(std::none_of(lines.begin(), lines.end(), [](const auto& line) { return line.find(" E ") != std::string::npos; }));
}

void FileStagingCommitterTests::skippedTargetsNeverInvokeCommit() {
  UwfMaintenanceFixture fixture;
  auto files = std::make_shared<MemoryStagingSource>();
  app::FileStagingCommitter committer(fixture.repository(), files);
  auto operation = committer.beginCommit(
      committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan({{app::FileStagingKind::File, QStringLiteral("C:\\Data\\disappeared.bin")}}))));
  files->present = false;
  fixture.state->commitFailure = std::make_exception_ptr(std::runtime_error("a skipped target must not submit a commit"));
  static_cast<void>(operation.advance());
  QVERIFY(operation.finished());
  QVERIFY(operation.result().succeeded());
  QCOMPARE(operation.result().skippedFiles, std::size_t{1});
  QVERIFY(fixture.state->commits.empty());
}

void FileStagingCommitterTests::successfulAndSkippedTargetsDoNotProduceFailureLogs() {
  clearLogLines();
  const auto clear = qScopeGuard([] { clearLogLines(); });
  UwfMaintenanceFixture fixture;
  fixture.stageFile(QStringLiteral("C:\\saved.bin"));
  auto source = std::make_shared<MemoryStagingSource>();
  source->inspectResult = [](const app::CommitNode& target, int) -> std::optional<app::CommitNode> {
    return target.path.endsWith(QStringLiteral("missing.bin")) ? std::nullopt : std::optional{target};
  };
  app::FileStagingCommitter committer(fixture.repository(), source);
  const auto result = completeFileStaging(
      committer, {{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}, {app::FileStagingKind::File, QStringLiteral("C:\\missing.bin")}});
  QVERIFY(result.succeeded());
  QCOMPARE(result.committedFiles, std::size_t{1});
  QCOMPARE(result.skippedEntries, std::size_t{1});
  QVERIFY(stagingFailureLogs().isEmpty());
}

void FileStagingCommitterTests::providerNotFoundIsFailureForEveryWriteEffect_data() {
  QTest::addColumn<int>("effect");
  for (const auto& [name, effect] :
       {std::pair{"none", app::WriteEffect::None}, std::pair{"confirmed", app::WriteEffect::Confirmed}, std::pair{"uncertain", app::WriteEffect::Uncertain}})
    QTest::newRow(name) << static_cast<int>(effect);
}
void FileStagingCommitterTests::providerNotFoundIsFailureForEveryWriteEffect() {
  QFETCH(int, effect);
  clearLogLines();
  const auto clear = qScopeGuard([] { clearLogLines(); });
  UwfMaintenanceFixture fixture;
  fixture.stageFile(QStringLiteral("C:\\saved.bin"));
  fixture.state->commitFailure = std::make_exception_ptr(
      app::OperationError(std::make_error_code(std::errc::no_such_file_or_directory), "Commit target was not found", static_cast<app::WriteEffect>(effect)));
  const auto source = std::make_shared<MemoryStagingSource>();
  app::FileStagingCommitter committer(fixture.repository(), source);
  const auto result = completeFileStaging(committer, {{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}});
  QVERIFY(!result.succeeded());
  QCOMPARE(result.failures.size(), std::size_t{1});
  QCOMPARE(result.failures.front().kind, app::FileStagingCommitFailureKind::ProviderFailure);
  QVERIFY(result.failures.front().operation);
  QCOMPARE(result.failures.front().operation->kind, app::OperationFailureKind::NotFound);
  QCOMPARE(result.failures.front().operation->effect, static_cast<app::WriteEffect>(effect));
  QCOMPARE(result.writeEffect(), static_cast<app::WriteEffect>(effect));
  QCOMPARE(result.committedFiles, std::size_t{0});
  QCOMPARE(result.skippedFiles, std::size_t{0});
  QCOMPARE(fixture.state->commits.size(), std::size_t{1});
  QCOMPARE(stagingFailureLogs().size(), 1);
}

void FileStagingCommitterTests::fileStagingCommitterOnlyWritesEligibleCurrentTargets() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString existingPath = QDir::toNativeSeparators(directory.filePath(QStringLiteral("state.bin")));
  QFile file(existingPath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  QVERIFY(file.write("state") > 0);
  file.close();
  const QString drive = QString::fromStdString(drive::fromPath(existingPath.toStdString()));
  QVERIFY(!drive.isEmpty());
  const auto files = uwf::windows::systemCommitTraversalSource();
  const auto resolved = files->resolveFileTarget(existingPath);
  const QList<app::FileStagingEntry> existing{{app::FileStagingKind::File, existingPath}};

  UwfMaintenanceFixture canceled;
  (void)canceled.useFileSystemTarget(*files, existingPath);
  std::stop_source canceledScan;
  canceledScan.request_stop();
  app::FileStagingCommitter canceledCommitter(canceled.repository(), files);
  auto canceledOperation = canceledCommitter.beginCommit(
      canceledCommitter.prepare(app::FileStagingCommitter::scan(canceledCommitter.prepareScan(existing), canceledScan.get_token())));
  QVERIFY(canceledOperation.finished());
  QCOMPARE(canceledOperation.result().discoveredFiles, std::size_t{0});
  QCOMPARE(canceled.state->volumeReads, 1);
  QVERIFY(canceled.state->commits.empty());

  UwfMaintenanceFixture absent;
  (void)absent.useFileSystemTarget(*files, existingPath);
  absent.state->configuredVolume = false;
  app::FileStagingCommitter absentCommitter(absent.repository(), files);
  const auto absentResult =
      completeFileStaging(absentCommitter, {{app::FileStagingKind::File, QDir::toNativeSeparators(directory.filePath(QStringLiteral("missing.bin")))}});
  QCOMPARE(absentResult.skippedEntries, std::size_t{1});
  QCOMPARE(absentResult.discoveredFiles, std::size_t{0});
  QVERIFY(absentResult.succeeded());
  QCOMPARE(absent.state->volumeReads, 0);
  QVERIFY(absent.state->commits.empty());

  UwfMaintenanceFixture unprotected;
  (void)unprotected.useFileSystemTarget(*files, existingPath);
  unprotected.state->protectedVolume = false;
  app::FileStagingCommitter unprotectedCommitter(unprotected.repository(), files);
  const auto unprotectedResult = completeFileStaging(unprotectedCommitter, existing);
  QCOMPARE(unprotectedResult.discoveredFiles, std::size_t{0});
  QCOMPARE(unprotectedResult.skippedEntries, std::size_t{1});
  QVERIFY(unprotectedResult.succeeded());
  QCOMPARE(unprotected.state->exclusionReads, 0);
  QCOMPARE(unprotected.state->volumeReads, 0);
  QCOMPARE(unprotectedResult.writeEffect(), app::WriteEffect::None);
  QVERIFY(unprotected.state->commits.empty());

  UwfMaintenanceFixture excluded;
  (void)excluded.useFileSystemTarget(*files, existingPath);
  excluded.state->exclusions = {existingPath};
  app::FileStagingCommitter excludedCommitter(excluded.repository(), files);
  const auto excludedResult = completeFileStaging(excludedCommitter, existing);
  QCOMPARE(excludedResult.skippedEntries, std::size_t{1});
  QCOMPARE(excludedResult.discoveredFiles, std::size_t{0});
  QVERIFY(excludedResult.succeeded());
  QCOMPARE(excludedResult.writeEffect(), app::WriteEffect::None);
  QVERIFY(excluded.state->commits.empty());

  UwfMaintenanceFixture nested;
  (void)nested.useFileSystemTarget(*files, existingPath);
  nested.state->exclusions = {existingPath};
  app::FileStagingCommitter nestedCommitter(nested.repository(), files);
  const auto nestedResult = completeFileStaging(nestedCommitter, {{app::FileStagingKind::Directory, QDir::toNativeSeparators(directory.path())},
                                                                  {app::FileStagingKind::Directory, QDir::toNativeSeparators(directory.path())}});
  QCOMPARE(nestedResult.skippedFiles, std::size_t{1});
  QCOMPARE(nestedResult.discoveredFiles, std::size_t{1});
  QVERIFY(nestedResult.succeeded());
  QVERIFY(nested.state->commits.empty());
  const auto commandResult = app::toFileStagingCommandResult(nestedResult, {});
  QCOMPARE(commandResult.outcome, app::FileStagingCommandOutcome::Succeeded);
  QCOMPARE(commandResult.discoveredFiles, std::size_t{1});
  QCOMPARE(commandResult.skippedFiles, std::size_t{1});

  UwfMaintenanceFixture exclusionFailure;
  (void)exclusionFailure.useFileSystemTarget(*files, existingPath);
  exclusionFailure.state->exclusionFailure = std::make_exception_ptr(std::runtime_error("The exclusion list could not be read"));
  app::FileStagingCommitter failedCommitter(exclusionFailure.repository(), files);
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(completeFileStaging(failedCommitter, existing)));
  QVERIFY(exclusionFailure.state->commits.empty());

  UwfMaintenanceFixture detached;
  (void)detached.useFileSystemTarget(*files, existingPath);
  detached.stageFile(existingPath, "state");
  auto operation = [&] {
    app::FileStagingCommitter temporary(detached.repository(), files);
    return temporary.beginCommit(temporary.prepare(app::FileStagingCommitter::scan(temporary.prepareScan(existing))));
  }();
  QCOMPARE(operation.totalFiles(), std::size_t{1});
  QCOMPARE(operation.advance(), existingPath);
  QVERIFY(operation.finished());
  QCOMPARE(detached.state->commits, std::vector<std::string>{resolved.relativePath.toStdString()});

  UwfMaintenanceFixture relative;
  app::FileStagingCommitter relativeCommitter(relative.repository(), files);
  const auto relativeResult = completeFileStaging(relativeCommitter, {{app::FileStagingKind::File, QStringLiteral("C:relative-state.bin")}});
  QCOMPARE(relativeResult.discoveredFiles, std::size_t{0});
  QCOMPARE(relativeResult.failures.size(), 1);
  QCOMPARE(relativeResult.failures.front().kind, app::FileStagingCommitFailureKind::InvalidPath);
  QCOMPARE(relative.state->volumeReads, 0);
  QVERIFY(relative.state->commits.empty());
}

void FileStagingCommitterTests::fileStagingTaskRejectsUnavailableCapabilityWithoutRepositoryCalls() {
  UwfMaintenanceFixture fixture;
  MemoryFileStagingStore staging;
  staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\one.bin")}, {app::FileStagingKind::Directory, QStringLiteral("D:\\State")}};

  app::FileStagingTask task(fixture.repository(), staging.load(), uwf::app::UwfCapability::Unavailable, uwf::windows::systemCommitTraversalSource());
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(task.pollPreparation()));
  QVERIFY(task.failed());
  QVERIFY(!task.finished());
  QVERIFY(fixture.backend->trace().empty());
  QVERIFY(fixture.state->commits.empty());
}
void FileStagingCommitterTests::aTargetRemovedAfterTraversalIsNotRecreated() {
  UwfMaintenanceFixture fixture;
  const QString path = QStringLiteral("C:\\Data\\removed.bin");
  fixture.stageFile(path, "pending contents");
  fixture.state->beforeCommit = [backend = fixture.backend] { backend->deleteFile(UwfMaintenanceFixture::volumeId, "\\Data\\removed.bin"); };
  const auto source = std::make_shared<MemoryStagingSource>();
  app::FileStagingCommitter committer(fixture.repository(), source);
  const auto result = completeFileStaging(committer, {{app::FileStagingKind::File, path}});
  QCOMPARE(result.committedFiles, std::size_t{0});
  QCOMPARE(result.failures.size(), std::size_t{1});
  QCOMPARE(result.failures.front().kind, app::FileStagingCommitFailureKind::ProviderFailure);
  QVERIFY(!fixture.backend->visibleFile(UwfMaintenanceFixture::volumeId, "\\Data\\removed.bin"));
  QVERIFY(!fixture.backend->readFile(UwfMaintenanceFixture::volumeId, "\\Data\\removed.bin"));
}
}  // namespace

QTEST_APPLESS_MAIN(FileStagingCommitterTests)
#include "FileStagingCommitterTests.moc"
