// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QTest>
#include <QUuid>
#include <algorithm>
#include <stdexcept>

#include "app/ports/RegistryKey.h"
#include "app/registry/RegistryTreeCommitter.h"
#include "shared/Log.h"
#include "support/app/RecordingWmiOperations.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

app::wmi::WmiRow currentRegistryFilter() {
  using Value = app::wmi::WmiValue;
  return {{"__PATH", Value::fromString("registry-filter")},
          {"CurrentSession", Value::fromBool(true)},
          {"PersistDomainSecretKey", Value::fromBool(false)},
          {"PersistTSCAL", Value::fromBool(false)}};
}

std::vector<std::string> failureLogs() {
  auto lines = recentLogLines();
  std::erase_if(lines, [](const auto& line) { return line.find(" E registry] registry commit item failed:") == std::string::npos; });
  return lines;
}

std::string ownedRegistryKey() {
  return QStringLiteral("HKEY_CURRENT_USER\\Software\\HsingYun\\UWF Manager\\Tests\\%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)).toStdString();
}

void removeOwnedRegistryKey(const std::string& key) noexcept {
  try {
    regkey::deleteTree(key);
  } catch (...) {
  }
}

class RegistryTreeCommitterTests final : public QObject {
  Q_OBJECT
 private slots:
  void init() { clearLogLines(); }
  void emptyAndMissingRegistryPlansSkipUwfInvocation();
  void commitLogsEveryFailureBeforeTheVisibleSummaryLimit_data();
  void commitLogsEveryFailureBeforeTheVisibleSummaryLimit();
  void deletionLogsEveryFailureBeforeTheVisibleSummaryLimit_data();
  void deletionLogsEveryFailureBeforeTheVisibleSummaryLimit();
  void deletionInspectionFailureIsLoggedWithoutUwfInvocation();
  void successfulAndMissingDeletionTargetsDoNotProduceFailureLogs();
  void aFailedBatchReadDoesNotInventTargetFailures_data();
  void aFailedBatchReadDoesNotInventTargetFailures();
};

void RegistryTreeCommitterTests::emptyAndMissingRegistryPlansSkipUwfInvocation() {
  RecordingWmiOperations wmi;
  uwf::app::RegistryTreeCommitter committer(wmi);
  const auto emptyCommit = committer.commit({});
  QCOMPARE(emptyCommit.attempted, std::size_t{0});
  QCOMPARE(emptyCommit.committed, std::size_t{0});
  QVERIFY(emptyCommit.failures.empty());
  QVERIFY(wmi.queryResults.empty());

  const std::string missingKey =
      QStringLiteral("HKEY_CURRENT_USER\\Software\\HsingYun\\UWF Manager\\Tests\\%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)).toStdString();

  const auto result = committer.commitDeletion({{missingKey, {}}});

  QCOMPARE(result.attempted, std::size_t{1});
  QCOMPARE(result.skipped, std::size_t{1});
  QCOMPARE(result.committed, std::size_t{0});
  QVERIFY(result.failures.empty());
  QVERIFY(wmi.queryResults.empty());
  QVERIFY(wmi.invocations.empty());
  QVERIFY(failureLogs().empty());
}

void RegistryTreeCommitterTests::commitLogsEveryFailureBeforeTheVisibleSummaryLimit_data() {
  QTest::addColumn<int>("count");
  QTest::newRow("beyond-four-visible-failures") << 5;
  QTest::newRow("one-hundred-failures") << 100;
}

void RegistryTreeCommitterTests::commitLogsEveryFailureBeforeTheVisibleSummaryLimit() {
  QFETCH(int, count);
  RecordingWmiOperations wmi;
  wmi.queryResults.push_back({currentRegistryFilter()});
  std::vector<app::RegistryCommitTarget> targets;
  std::vector<std::string> reasons;
  for (int index = 0; index < count; ++index) {
    targets.push_back(
        {QStringLiteral("HKEY_CURRENT_USER\\Software\\日志目标-%1").arg(index).toStdString(), QStringLiteral("配置值-%1").arg(index).toStdString()});
    reasons.push_back(index % 2 == 0 ? QStringLiteral("拒绝写入-%1\n原始 provider 多行详情").arg(index).toStdString() : "non-standard exception");
  }
  std::size_t invoked = 0;
  wmi.invocationObserver = [&] {
    const auto index = invoked++;
    if (index >= targets.size()) throw std::logic_error("unexpected registry invocation");
    wmi.invocationFailure = index % 2 == 0 ? std::make_exception_ptr(std::runtime_error(reasons[index])) : std::make_exception_ptr(42);
  };
  app::RegistryTreeCommitter committer(wmi);
  const auto result = committer.commit(targets);
  QCOMPARE(result.attempted, targets.size());
  QCOMPARE(result.committed, std::size_t{0});
  QCOMPARE(result.failures.size(), targets.size());
  QCOMPARE(wmi.invocations.size(), targets.size());
  const auto lines = failureLogs();
  QCOMPARE(lines.size(), targets.size());
  for (std::size_t index = 0; index < targets.size(); ++index) {
    QCOMPARE(result.failures[index].target.key, targets[index].key);
    QCOMPARE(result.failures[index].target.valueName, targets[index].valueName);
    QCOMPARE(result.failures[index].detail, reasons[index]);
    const auto expected = "operation=commit key=" + targets[index].key + " value=" + targets[index].valueName + " error=" + reasons[index];
    QCOMPARE(std::count_if(lines.begin(), lines.end(), [&](const auto& line) { return line.find(expected) != std::string::npos; }), 1);
  }
  const auto copied = result;
  QVERIFY(!copied.succeeded());
  QCOMPARE(failureLogs(), lines);
}

void RegistryTreeCommitterTests::deletionLogsEveryFailureBeforeTheVisibleSummaryLimit_data() { commitLogsEveryFailureBeforeTheVisibleSummaryLimit_data(); }

void RegistryTreeCommitterTests::deletionLogsEveryFailureBeforeTheVisibleSummaryLimit() {
  QFETCH(int, count);
  const auto root = ownedRegistryKey();
  const auto remove = qScopeGuard([&] { removeOwnedRegistryKey(root); });
  std::vector<app::RegistryCommitTarget> targets;
  std::vector<std::string> reasons;
  for (int index = 0; index < count; ++index) {
    const auto key = index % 3 == 0 ? root + QStringLiteral("\\分支-%1").arg(index).toStdString() : root;
    const auto name = index % 3 == 0 ? std::string{} : QStringLiteral("删除值-%1").arg(index).toStdString();
    regkey::writeMultiString(key, name.empty() ? "fixture" : name, {"owned test data"});
    targets.push_back({key, name});
    reasons.push_back(index % 2 == 0 ? QStringLiteral("删除失败-%1\n原始 Unicode 详情").arg(index).toStdString() : "non-standard exception");
  }
  RecordingWmiOperations wmi;
  wmi.queryResults.push_back({currentRegistryFilter()});
  std::size_t invoked = 0;
  wmi.invocationObserver = [&] {
    const auto index = invoked++;
    if (index >= targets.size()) throw std::logic_error("unexpected registry deletion invocation");
    wmi.invocationFailure = index % 2 == 0 ? std::make_exception_ptr(std::runtime_error(reasons[index])) : std::make_exception_ptr(42);
  };
  app::RegistryTreeCommitter committer(wmi);
  const auto result = committer.commitDeletion(targets);
  QCOMPARE(result.attempted, targets.size());
  QCOMPARE(result.skipped, std::size_t{0});
  QCOMPARE(result.committed, std::size_t{0});
  QCOMPARE(result.failures.size(), targets.size());
  QCOMPARE(wmi.invocations.size(), targets.size());
  const auto lines = failureLogs();
  QCOMPARE(lines.size(), targets.size());
  for (std::size_t index = 0; index < targets.size(); ++index) {
    QCOMPARE(result.failures[index].detail, reasons[index]);
    const auto expected = "operation=commit-deletion key=" + targets[index].key + " value=" + targets[index].valueName + " error=" + reasons[index];
    QCOMPARE(std::count_if(lines.begin(), lines.end(), [&](const auto& line) { return line.find(expected) != std::string::npos; }), 1);
    QVERIFY(regkey::keyExists(targets[index].key));
    if (!targets[index].valueName.empty()) QVERIFY(regkey::valueExists(targets[index].key, targets[index].valueName));
  }
  regkey::deleteTree(root);
  QVERIFY(!regkey::keyExists(root));
}

void RegistryTreeCommitterTests::deletionInspectionFailureIsLoggedWithoutUwfInvocation() {
  RecordingWmiOperations wmi;
  app::RegistryTreeCommitter committer(wmi);
  const std::vector<app::RegistryCommitTarget> targets{{"UNKNOWN_HIVE\\diagnostic-key", "value"}, {"OTHER_UNKNOWN_HIVE\\diagnostic-key", {}}};
  const auto result = committer.commitDeletion(targets);
  QCOMPARE(result.failures.size(), targets.size());
  QVERIFY(wmi.invocations.empty());
  const auto lines = failureLogs();
  QCOMPARE(lines.size(), targets.size());
  for (const auto& failure : result.failures) {
    QVERIFY(!failure.detail.empty());
    const auto expected = "operation=inspect-deletion key=" + failure.target.key + " value=" + failure.target.valueName + " error=" + failure.detail;
    QCOMPARE(std::count_if(lines.begin(), lines.end(), [&](const auto& line) { return line.find(expected) != std::string::npos; }), 1);
  }
}

void RegistryTreeCommitterTests::successfulAndMissingDeletionTargetsDoNotProduceFailureLogs() {
  const auto root = ownedRegistryKey();
  const auto remove = qScopeGuard([&] { removeOwnedRegistryKey(root); });
  regkey::writeMultiString(root, "existing-value", {"owned test data"});
  RecordingWmiOperations wmi;
  wmi.queryResults.push_back({currentRegistryFilter()});
  app::RegistryTreeCommitter committer(wmi);
  const auto committed = committer.commit({{root, "existing-value"}});
  QVERIFY(committed.succeeded());
  QCOMPARE(committed.committed, std::size_t{1});
  wmi.queryResults.push_back({currentRegistryFilter()});
  const auto result = committer.commitDeletion({{root, "existing-value"}, {root, "missing-value"}, {root + "\\missing-key", {}}});
  QVERIFY(result.succeeded());
  QCOMPARE(result.committed, std::size_t{1});
  QCOMPARE(result.skipped, std::size_t{2});
  QVERIFY(failureLogs().empty());
  QVERIFY(regkey::valueExists(root, "existing-value"));
  regkey::deleteTree(root);
  QVERIFY(!regkey::keyExists(root));
}

void RegistryTreeCommitterTests::aFailedBatchReadDoesNotInventTargetFailures_data() {
  QTest::addColumn<bool>("unknown");
  QTest::newRow("multiline-utf8") << false;
  QTest::newRow("unknown") << true;
}

void RegistryTreeCommitterTests::aFailedBatchReadDoesNotInventTargetFailures() {
  QFETCH(bool, unknown);
  RecordingWmiOperations wmi;
  const auto detail = QStringLiteral("无法读取注册表批次\n保留 provider 的完整原始 Unicode 错误");
  wmi.queryFailure = unknown ? std::make_exception_ptr(42) : std::make_exception_ptr(std::runtime_error(detail.toStdString()));
  app::RegistryTreeCommitter committer(wmi);
  const std::vector<app::RegistryCommitTarget> targets{{"HKEY_CURRENT_USER\\no-real-access", "first"}, {"HKEY_CURRENT_USER\\no-real-access", "second"}};
  if (unknown)
    QVERIFY_THROWS_EXCEPTION(int, static_cast<void>(committer.commit(targets)));
  else
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(committer.commit(targets)));
  QVERIFY(wmi.invocations.empty());
  QVERIFY(failureLogs().empty());

  const auto root = ownedRegistryKey();
  const auto remove = qScopeGuard([&] { removeOwnedRegistryKey(root); });
  regkey::writeMultiString(root, "existing-value", {"owned test data"});
  if (unknown)
    QVERIFY_THROWS_EXCEPTION(int, static_cast<void>(committer.commitDeletion({{root, "existing-value"}})));
  else
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(committer.commitDeletion({{root, "existing-value"}})));
  QVERIFY(wmi.invocations.empty());
  QVERIFY(failureLogs().empty());
  auto errors = recentLogLines();
  std::erase_if(errors, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(errors.size(), std::size_t{2});
  for (const auto& line : errors) {
    QVERIFY(line.find("registry commit batch read failed:") != std::string::npos);
    QVERIFY(line.find(unknown ? "error=non-standard-exception" : detail.toStdString()) != std::string::npos);
  }
  QVERIFY(errors[0].find("operation=commit ") != std::string::npos);
  QVERIFY(errors[1].find("operation=commit-deletion ") != std::string::npos);
  wmi.queryFailure = {};
  wmi.queryResults.push_back({currentRegistryFilter()});
  const auto retried = committer.commit(targets);
  QVERIFY(retried.succeeded());
  QCOMPARE(retried.committed, targets.size());
  auto afterRetry = recentLogLines();
  std::erase_if(afterRetry, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(afterRetry, errors);
  regkey::deleteTree(root);
  QVERIFY(!regkey::keyExists(root));
}
}  // namespace

QTEST_APPLESS_MAIN(RegistryTreeCommitterTests)
#include "RegistryTreeCommitterTests.moc"
