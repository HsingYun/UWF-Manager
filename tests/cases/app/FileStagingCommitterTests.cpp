// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QDir>
#include <QFile>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>
#include <functional>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <utility>

#include "app/ports/VolumePaths.h"
#include "app/staging/FileStagingCommandResult.h"
#include "app/staging/FileStagingTask.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "shared/Log.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/RecordingWmiOperations.h"
#include "windows/commit/WindowsCommitTraversalSource.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

class MemoryStagingSource final : public app::CommitTraversalSource {
 public:
  app::CommitTargetKind kind = app::CommitTargetKind::File;
  QString skippedReason;
  QString readError;
  std::function<std::optional<app::CommitNode>(const app::CommitNode&, int)> inspectResult;
  std::function<std::string(int)> volumeResult;
  mutable int inspections = 0;
  mutable int volumeReads = 0;
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
  std::string volumeForPath(const QString&) const override {
    ++volumeReads;
    return volumeResult ? volumeResult(volumeReads) : "C:";
  }
  std::vector<std::string> mountedLetters(const std::string&) const override { return {"C:"}; }
  QString read(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override { return readError; }
};

void queueProtectedVolume(RecordingWmiOperations& wmi) {
  using Value = app::wmi::WmiValue;
  wmi.queryResults.push_back({{{"__PATH", Value::fromString("volume")},
                               {"CurrentSession", Value::fromBool(true)},
                               {"DriveLetter", Value::fromString("C:")},
                               {"VolumeName", Value::fromString("Volume{failure-log-fixture}")},
                               {"BindByDriveLetter", Value::fromBool(true)},
                               {"CommitPending", Value::fromBool(false)},
                               {"Protected", Value::fromBool(true)}}});
  wmi.readResults.push_back(fileExclusionResult());
}

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
  void fileStagingCommitterTreatsPersistentAndAbsentTargetsAsIdempotent();
  void fileStagingTaskSkipsUnavailableCapabilityWithoutWmi();
  void everyFailedTargetIsLoggedBeyondTheVisibleFailureLimit();
  void failuresAreLoggedAtTheirOrigin_data();
  void failuresAreLoggedAtTheirOrigin();
  void successfulAndSkippedTargetsDoNotProduceFailureLogs();
  void batchPrerequisiteFailuresPropagateWithoutTargetResults_data();
  void batchPrerequisiteFailuresPropagateWithoutTargetResults();
  void skippedTargetsDoNotResolveTheCommitWorkerTransport();
};

void FileStagingCommitterTests::everyFailedTargetIsLoggedBeyondTheVisibleFailureLimit() {
  clearLogLines();
  const auto clear = qScopeGuard([] { clearLogLines(); });
  RecordingWmiOperations wmi;
  queueProtectedVolume(wmi);
  const QString reason = QStringLiteral("拒绝提交：文件正在使用。\nProvider detail retains every line.\n最后一行诊断");
  wmi.invocationObserver = [&] { wmi.invocationFailure = std::make_exception_ptr(std::runtime_error(reason.toStdString())); };
  QList<app::FileStagingEntry> entries;
  for (int index = 1; index <= 100; ++index)
    entries.append({app::FileStagingKind::File, QStringLiteral("C:\\诊断目录\\target-%1.bin").arg(index, 3, 10, QChar('0'))});
  auto source = std::make_shared<MemoryStagingSource>();
  app::FileStagingCommitter committer(wmi, source);
  auto operation = committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan(entries))));
  QVERIFY(stagingFailureLogs().isEmpty());
  while (!operation.finished()) static_cast<void>(operation.advance());
  QCOMPARE(wmi.invocations.size(), 100);
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
  static_cast<void>(app::toApplicationCommandResult(operation.result(), QStringLiteral("UI summary")));
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
  add("preparation-volume", Kind::MissingDriveLetter, false, "missing-drive-letter");
  add("commit-volume", Kind::MissingDriveLetter, true, "missing-drive-letter");
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
  RecordingWmiOperations wmi;
  queueProtectedVolume(wmi);
  auto source = std::make_shared<MemoryStagingSource>();
  const QString diagnostic = QStringLiteral("诊断阶段失败。\n原始错误详情第二行。\nUnicode 原文保留");
  app::FileStagingEntry entry{app::FileStagingKind::File, QStringLiteral("C:\\诊断目录\\target.bin")};
  QString expectedPath = entry.path;
  QString expectedDetail;
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
    case Kind::MissingDriveLetter:
      source->volumeResult = [duringCommit, diagnostic](int call) -> std::string {
        if (!duringCommit) return {};
        if (call > 1) throw std::runtime_error(diagnostic.toStdString());
        return "C:";
      };
      if (duringCommit) expectedDetail = diagnostic;
      break;
    case Kind::VolumeRoot:
      entry.kind = app::FileStagingKind::Directory;
      entry.path = expectedPath = QStringLiteral("C:\\");
      source->kind = app::CommitTargetKind::Folder;
      break;
    case Kind::ProviderFailure:
      wmi.invocationFailure = std::make_exception_ptr(std::runtime_error(diagnostic.toStdString()));
      expectedDetail = diagnostic;
      break;
    case Kind::UnknownFailure:
      wmi.invocationFailure = std::make_exception_ptr(42);
      break;
  }
  app::FileStagingCommitter committer(wmi, source);
  const auto result = completeFileStaging(committer, {entry});
  QCOMPARE(result.failures.size(), 1);
  QCOMPARE(result.failures.front().kind, kind);
  QCOMPARE(result.failures.front().path, expectedPath);
  QCOMPARE(result.failures.front().detail, expectedDetail);
  const auto logged = stagingFailureLogs();
  QCOMPARE(logged.size(), 1);
  QVERIFY2(logged.front().endsWith(QStringLiteral("path=%1 kind=%2 error=%3").arg(expectedPath, kindName, expectedDetail)), qUtf8Printable(logged.front()));
}

void FileStagingCommitterTests::batchPrerequisiteFailuresPropagateWithoutTargetResults_data() {
  QTest::addColumn<int>("source");
  QTest::addColumn<bool>("unknown");
  const QStringList sources{QStringLiteral("volume-snapshot"), QStringLiteral("exclusions-read"), QStringLiteral("commit-worker-accessor")};
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
  RecordingWmiOperations wmi;
  queueProtectedVolume(wmi);
  if (source == 0) wmi.queryFailure = failure;
  if (source == 1) wmi.readFailure = failure;
  auto files = std::make_shared<MemoryStagingSource>();
  app::FileStagingCommitter committer(wmi, files);
  const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\Data\\批次失败.bin")}};
  std::optional<app::FileStagingCommitOperation> operation;
  bool producedResult = false;
  int caught = 0;
  try {
    if (source == 2) {
      operation.emplace(committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan(entries)))));
      wmi.workerAccessFailure = failure;
      static_cast<void>(operation->advance());
    } else {
      static_cast<void>(completeFileStaging(committer, entries));
    }
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
  QVERIFY(wmi.invocations.empty());
  if (operation) {
    QVERIFY(operation->result().failures.empty());
    QCOMPARE(operation->result().committedFiles, std::size_t{0});
  }
  // This synchronous primitive propagates to its batch owner. Every production
  // caller is FileStagingTask, whose five entry routes verify the single E log.
  QVERIFY(stagingFailureLogs().isEmpty());
  const auto lines = recentLogLines();
  QVERIFY(std::none_of(lines.begin(), lines.end(), [](const auto& line) { return line.find(" E ") != std::string::npos; }));
}

void FileStagingCommitterTests::skippedTargetsDoNotResolveTheCommitWorkerTransport() {
  RecordingWmiOperations wmi;
  queueProtectedVolume(wmi);
  auto files = std::make_shared<MemoryStagingSource>();
  app::FileStagingCommitter committer(wmi, files);
  auto operation = committer.beginCommit(
      committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan({{app::FileStagingKind::File, QStringLiteral("C:\\Data\\disappeared.bin")}}))));
  files->present = false;
  wmi.workerAccessFailure = std::make_exception_ptr(std::runtime_error("a skipped target must not resolve its commit transport"));
  static_cast<void>(operation.advance());
  QVERIFY(operation.finished());
  QVERIFY(operation.result().succeeded());
  QCOMPARE(operation.result().skippedFiles, std::size_t{1});
  QVERIFY(wmi.invocations.empty());
}

void FileStagingCommitterTests::successfulAndSkippedTargetsDoNotProduceFailureLogs() {
  clearLogLines();
  const auto clear = qScopeGuard([] { clearLogLines(); });
  RecordingWmiOperations wmi;
  queueProtectedVolume(wmi);
  auto source = std::make_shared<MemoryStagingSource>();
  source->inspectResult = [](const app::CommitNode& target, int) -> std::optional<app::CommitNode> {
    return target.path.endsWith(QStringLiteral("missing.bin")) ? std::nullopt : std::optional{target};
  };
  app::FileStagingCommitter committer(wmi, source);
  const auto result = completeFileStaging(
      committer, {{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}, {app::FileStagingKind::File, QStringLiteral("C:\\missing.bin")}});
  QVERIFY(result.succeeded());
  QCOMPARE(result.committedFiles, std::size_t{1});
  QCOMPARE(result.skippedEntries, std::size_t{1});
  QVERIFY(stagingFailureLogs().isEmpty());

  RecordingWmiOperations unchanged;
  queueProtectedVolume(unchanged);
  unchanged.invocationFailure = std::make_exception_ptr(
      app::wmi::WmiProviderError(static_cast<uint32_t>(static_cast<int32_t>(app::wmi::WmiErrorCode::NotFound)), "commit unchanged file"));
  app::FileStagingCommitter unchangedCommitter(unchanged, source);
  const auto alreadyPersistent = completeFileStaging(unchangedCommitter, {{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}});
  QVERIFY(alreadyPersistent.succeeded());
  QCOMPARE(alreadyPersistent.skippedFiles, std::size_t{1});
  QVERIFY(stagingFailureLogs().isEmpty());
}

void FileStagingCommitterTests::fileStagingCommitterTreatsPersistentAndAbsentTargetsAsIdempotent() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString existingPath = QDir::toNativeSeparators(directory.filePath(QStringLiteral("state.bin")));
  QFile file(existingPath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  QVERIFY(file.write("state") > 0);
  file.close();
  const QString drive = QString::fromStdString(drive::fromPath(existingPath.toStdString()));
  QVERIFY(!drive.isEmpty());

  std::stop_source canceledScan;
  canceledScan.request_stop();
  RecordingWmiOperations canceledWmi;
  canceledWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                       {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                       {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                                       {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                       {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                       {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                       {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  canceledWmi.readResults.push_back(fileExclusionResult());
  app::FileStagingCommitter canceledCommitter(canceledWmi, uwf::windows::systemCommitTraversalSource());
  auto canceledOperation = canceledCommitter.beginCommit(canceledCommitter.prepare(
      app::FileStagingCommitter::scan(canceledCommitter.prepareScan({{app::FileStagingKind::File, existingPath}}), canceledScan.get_token())));
  QVERIFY(canceledOperation.finished());
  QCOMPARE(canceledOperation.result().discoveredFiles, 0);
  QVERIFY(canceledWmi.queryResults.empty());
  QVERIFY(canceledWmi.invocations.empty());

  RecordingWmiOperations absentWmi;
  absentWmi.queryResults.push_back({});
  app::FileStagingCommitter absentCommitter(absentWmi, uwf::windows::systemCommitTraversalSource());
  const auto absentResult =
      completeFileStaging(absentCommitter, {{app::FileStagingKind::File, QDir::toNativeSeparators(directory.filePath(QStringLiteral("missing.bin")))}});
  QCOMPARE(absentResult.skippedEntries, 1);
  QCOMPARE(absentResult.discoveredFiles, 0);
  QVERIFY(absentResult.succeeded());
  QVERIFY(absentWmi.queryResults.empty());
  QVERIFY(absentWmi.invocations.empty());

  RecordingWmiOperations unprotectedWmi;
  unprotectedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                          {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                          {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                                          {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                          {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                          {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                          {"Protected", uwf::app::wmi::WmiValue::fromBool(false)}}});
  app::FileStagingCommitter unprotectedCommitter(unprotectedWmi, uwf::windows::systemCommitTraversalSource());
  const auto unprotectedResult = completeFileStaging(unprotectedCommitter, {{app::FileStagingKind::File, existingPath}});
  QCOMPARE(unprotectedResult.discoveredFiles, 0);
  QCOMPARE(unprotectedResult.skippedEntries, 1);
  QVERIFY(unprotectedResult.succeeded());
  QVERIFY(unprotectedWmi.invocations.empty());

  RecordingWmiOperations excludedWmi;
  excludedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                       {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                       {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                                       {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                       {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                       {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                       {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  excludedWmi.readResults.push_back(fileExclusionResult({existingPath.toStdString()}));
  app::FileStagingCommitter excludedCommitter(excludedWmi, uwf::windows::systemCommitTraversalSource());
  const auto excludedResult = completeFileStaging(excludedCommitter, {{app::FileStagingKind::File, existingPath}});
  QCOMPARE(excludedResult.skippedEntries, std::size_t{1});
  QCOMPARE(excludedResult.discoveredFiles, std::size_t{0});
  QVERIFY(excludedResult.succeeded());
  QVERIFY(excludedWmi.invocations.empty());

  RecordingWmiOperations nestedExclusionWmi;
  nestedExclusionWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                              {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                              {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                                              {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                              {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                              {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                              {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  nestedExclusionWmi.readResults.push_back(fileExclusionResult({existingPath.toStdString()}));
  app::FileStagingCommitter nestedExclusionCommitter(nestedExclusionWmi, uwf::windows::systemCommitTraversalSource());
  const auto nestedExclusionResult =
      completeFileStaging(nestedExclusionCommitter, {{app::FileStagingKind::Directory, QDir::toNativeSeparators(directory.path())},
                                                     {app::FileStagingKind::Directory, QDir::toNativeSeparators(directory.path())}});
  QCOMPARE(nestedExclusionResult.skippedFiles, std::size_t{1});
  QCOMPARE(nestedExclusionResult.discoveredFiles, std::size_t{1});
  QVERIFY(nestedExclusionResult.succeeded());
  QVERIFY(nestedExclusionWmi.invocations.empty());
  const auto wireResult = app::decodeCommandResult(app::encodeCommandResult(1, app::toApplicationCommandResult(nestedExclusionResult, {})));
  QVERIFY(wireResult);
  QCOMPARE(wireResult->first, std::uint64_t{1});
  QCOMPARE(wireResult->second.outcome, app::ApplicationCommandOutcome::Succeeded);
  QCOMPARE(wireResult->second.discoveredFiles, std::size_t{1});
  QCOMPARE(wireResult->second.skippedFiles, std::size_t{1});

  RecordingWmiOperations exclusionReadFailureWmi;
  exclusionReadFailureWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                                   {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                                   {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                                                   {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                                   {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                                   {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                                   {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  app::FileStagingCommitter exclusionReadFailureCommitter(exclusionReadFailureWmi, uwf::windows::systemCommitTraversalSource());
  QVERIFY_THROWS_EXCEPTION(std::runtime_error,
                           static_cast<void>(completeFileStaging(exclusionReadFailureCommitter, {{app::FileStagingKind::File, existingPath}})));
  QVERIFY(exclusionReadFailureWmi.invocations.empty());

  RecordingWmiOperations unchangedWmi;
  unchangedWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                        {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                        {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                                        {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                        {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                        {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                        {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  unchangedWmi.readResults.push_back(fileExclusionResult());
  unchangedWmi.invocationFailure = std::make_exception_ptr(
      uwf::app::wmi::WmiProviderError(static_cast<uint32_t>(static_cast<int32_t>(uwf::app::wmi::WmiErrorCode::NotFound)), "commit staged file"));
  app::FileStagingCommitter unchangedCommitter(unchangedWmi, uwf::windows::systemCommitTraversalSource());
  const auto unchangedResult = completeFileStaging(unchangedCommitter, {{app::FileStagingKind::File, existingPath}});
  QCOMPARE(unchangedResult.discoveredFiles, 1);
  QCOMPARE(unchangedResult.skippedFiles, 1);
  QCOMPARE(unchangedWmi.invocations, std::vector<QString>{QStringLiteral("CommitFile")});
  QVERIFY(unchangedResult.succeeded());

  RecordingWmiOperations detachedOperationWmi;
  detachedOperationWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                                {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                                {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
                                                {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                                {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                                {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                                {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  detachedOperationWmi.readResults.push_back(fileExclusionResult());
  auto detachedOperation = beginFileStagingWithTemporaryCommitter(detachedOperationWmi, {{app::FileStagingKind::File, existingPath}});
  QCOMPARE(detachedOperation.totalFiles(), std::size_t{1});
  QCOMPARE(detachedOperation.advance(), existingPath);
  QVERIFY(detachedOperation.finished());
  QCOMPARE(detachedOperationWmi.invocations, std::vector<QString>{QStringLiteral("CommitFile")});

  RecordingWmiOperations relativeWmi;
  relativeWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume-path")},
                                       {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                       {"DriveLetter", uwf::app::wmi::WmiValue::fromString("C:")},
                                       {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging}")},
                                       {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
                                       {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
                                       {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}});
  app::FileStagingCommitter relativeCommitter(relativeWmi, uwf::windows::systemCommitTraversalSource());
  const auto relativeResult = completeFileStaging(relativeCommitter, {{app::FileStagingKind::File, QStringLiteral("C:relative-state.bin")}});
  QCOMPARE(relativeResult.discoveredFiles, std::size_t{0});
  QCOMPARE(relativeResult.failures.size(), 1);
  QCOMPARE(relativeResult.failures.front().kind, app::FileStagingCommitFailureKind::InvalidPath);
  QVERIFY(relativeWmi.invocations.empty());
}

void FileStagingCommitterTests::fileStagingTaskSkipsUnavailableCapabilityWithoutWmi() {
  RecordingWmiOperations wmi;
  MemoryFileStagingStore staging;
  staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\one.bin")}, {app::FileStagingKind::Directory, QStringLiteral("D:\\State")}};

  app::FileStagingTask task(wmi, staging, uwf::app::UwfCapability::Unavailable, uwf::windows::systemCommitTraversalSource());
  QVERIFY(task.pollPreparation());
  QVERIFY(task.preparationFinished());
  QVERIFY(task.finished());
  QCOMPARE(task.totalFiles(), std::size_t{0});
  QCOMPARE(task.result().skippedEntries, std::size_t{2});
  QVERIFY(task.result().succeeded());
  QVERIFY(wmi.queryResults.empty());
  QVERIFY(wmi.invocations.empty());
}
}  // namespace

QTEST_APPLESS_MAIN(FileStagingCommitterTests)
#include "FileStagingCommitterTests.moc"
