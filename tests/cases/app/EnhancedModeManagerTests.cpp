// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QStringList>
#include <QTest>
#include <algorithm>
#include <array>
#include <exception>
#include <optional>
#include <stdexcept>

#include "app/enhanced/EnhancedModeManager.h"
#include "shared/Log.h"
#include "support/app/EnhancedModeFixture.h"
#include "support/app/RecordingWmiOperations.h"
#include "support/app/WorkerGate.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

class EnhancedModeManagerTests final : public QObject {
  Q_OBJECT
 private slots:
  void init() { clearLogLines(); }
  void enhancedModeServiceContractRejectsEveryBrokenInvariant();
  void enhancedModeRemovalPreflightsProtectedRegistryPersistence();
  void workerOwnsItsServiceAfterManagerDestruction();
  void persistenceFailuresKeepOneOriginalBatchLog_data();
  void persistenceFailuresKeepOneOriginalBatchLog();
  void itemPersistenceWarningsDoNotRepeatRegistryFailureLogs();
};

void EnhancedModeManagerTests::enhancedModeServiceContractRejectsEveryBrokenInvariant() {
  const auto complete = completeEnhancedModeStatus();
  QVERIFY(complete.serviceContractSatisfied());

  constexpr std::array requirements{
      &app::EnhancedModeStatus::serviceExists,
      &app::EnhancedModeStatus::ownProcess,
      &app::EnhancedModeStatus::automaticStart,
      &app::EnhancedModeStatus::localSystemAccount,
      &app::EnhancedModeStatus::running,
      &app::EnhancedModeStatus::executableMatches,
      &app::EnhancedModeStatus::preshutdownTimeoutConfigured,
      &app::EnhancedModeStatus::requiredPrivilegesConfigured,
      &app::EnhancedModeStatus::preshutdownAccepted,
  };
  for (const auto requirement : requirements) {
    auto broken = complete;
    broken.*requirement = false;
    QVERIFY(!broken.serviceContractSatisfied());
  }
}

void EnhancedModeManagerTests::enhancedModeRemovalPreflightsProtectedRegistryPersistence() {
  RecordingWmiOperations unavailableWmi;
  auto protectedServiceOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& protectedService = *protectedServiceOwner;
  protectedService.current = completeEnhancedModeStatus();
  app::EnhancedModeManager unavailableManager(protectedServiceOwner, unavailableWmi, uwf::app::UwfCapability::Available);

  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(unavailableManager.disable()));
  QCOMPARE(protectedService.removals, 0);
  QCOMPARE(protectedService.current.state, app::EnhancedModeState::Enabled);

  RecordingWmiOperations availableWmi;
  availableWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                                        {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                                        {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  availableWmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("registry-path")},
                                        {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
                                        {"PersistDomainSecretKey", uwf::app::wmi::WmiValue::fromBool(false)},
                                        {"PersistTSCAL", uwf::app::wmi::WmiValue::fromBool(false)}}});
  auto removableServiceOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& removableService = *removableServiceOwner;
  removableService.current = protectedService.current;
  // 使用必然存在、但只经内存 WMI 传输处理的 hive 根验证时序；测试不会修改
  // 真实注册表。计划在 SCM 变更前冻结，方法调用发生时删除屏障必须仍存活。
  removableService.deletionPlan = {{"HKEY_CURRENT_USER", {}}};
  bool deletionCommittedInsideBarrier = false;
  availableWmi.invocationObserver = [&] { deletionCommittedInsideBarrier = removableService.activeDeletionBarriers == 1; };
  app::EnhancedModeManager availableManager(removableServiceOwner, availableWmi, uwf::app::UwfCapability::Available);
  const auto result = availableManager.disable();
  QCOMPARE(removableService.removals, 1);
  QCOMPARE(removableService.activeDeletionBarriers, 0);
  QVERIFY(deletionCommittedInsideBarrier);
  QCOMPARE(availableWmi.invocations, std::vector<QString>{QStringLiteral("CommitRegistryDeletion")});
  QCOMPARE(result.status.state, app::EnhancedModeState::Disabled);
  QVERIFY(result.persistenceWarning.isEmpty());
}
void EnhancedModeManagerTests::workerOwnsItsServiceAfterManagerDestruction() {
  auto gate = std::make_shared<test::WorkerGate>();
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->beforeInstall = [gate] { gate->wait(); };
  std::weak_ptr<MemoryEnhancedModeServiceControl> lifetime = service;
  RecordingWmiOperations wmi;
  auto manager = std::make_unique<app::EnhancedModeManager>(service, wmi, app::UwfCapability::Unavailable);
  manager->requestChange(app::EnhancedModeAction::Enable);
  QTRY_COMPARE(gate->started.load(), 1);
  service.reset();
  manager.reset();
  QVERIFY(!lifetime.expired());
  QCOMPARE(gate->finished.load(), 0);
  gate->release();
  QTRY_VERIFY(lifetime.expired());
  QCOMPARE(gate->finished.load(), 1);
}

void EnhancedModeManagerTests::persistenceFailuresKeepOneOriginalBatchLog_data() {
  QTest::addColumn<int>("source");
  QTest::addColumn<bool>("unknown");
  const QStringList sources{QStringLiteral("installation-filter"), QStringLiteral("installation-plan"), QStringLiteral("installation-registry-read"),
                            QStringLiteral("removal-filter"),      QStringLiteral("removal-plan"),      QStringLiteral("removal-registry-read"),
                            QStringLiteral("remnant-cleanup")};
  for (int source = 0; source < sources.size(); ++source)
    for (const bool unknown : {false, true}) {
      const auto name = sources[source] + (unknown ? QStringLiteral("-unknown") : QStringLiteral("-multiline-utf8"));
      QTest::newRow(qUtf8Printable(name)) << source << unknown;
    }
}

void EnhancedModeManagerTests::persistenceFailuresKeepOneOriginalBatchLog() {
  QFETCH(int, source);
  QFETCH(bool, unknown);
  const auto detail = QStringLiteral("增强模式持久化失败\n保留原始 provider/计划/清理错误的第二行");
  const auto failure = unknown ? std::make_exception_ptr(42) : std::make_exception_ptr(std::runtime_error(detail.toStdString()));
  RecordingWmiOperations wmi;
  using Value = app::wmi::WmiValue;
  wmi.queryResults.push_back({{{"__PATH", Value::fromString("filter")}, {"CurrentEnabled", Value::fromBool(true)}, {"NextEnabled", Value::fromBool(true)}}});
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  service->installationPlan = {{"HKEY_CURRENT_USER\\logging-only", "value"}};
  // Existence comes from the in-memory service boundary; provider writes are recorded.
  service->deletionPlan = {{"HKEY_CURRENT_USER", {}}};
  if (source == 0 || source == 3) wmi.queryFailure = failure;
  if (source == 1) service->beforeCommitPlan = [failure] { std::rethrow_exception(failure); };
  if (source == 2) service->beforeCommitPlan = [&] { wmi.queryFailure = failure; };
  if (source == 4) service->beforeDeletionPlan = [failure] { std::rethrow_exception(failure); };
  if (source == 5) service->beforeDeletionPlan = [&] { wmi.queryFailure = failure; };
  if (source == 6) {
    service->removalState = app::EnhancedModeServiceControl::RemovalState::Absent;
    service->beforeDeleteRemnants = [failure] { std::rethrow_exception(failure); };
  }
  app::EnhancedModeManager manager(service, wmi, source == 6 ? app::UwfCapability::Unavailable : app::UwfCapability::Available);
  const bool preflight = source == 3 || source == 4;
  std::optional<app::EnhancedModeChangeResult> result;
  int caught = 0;
  try {
    result = source <= 2 ? manager.enable(QStringLiteral("logging fixture")) : manager.disable();
  } catch (const std::exception& error) {
    QVERIFY(preflight);
    QVERIFY(!unknown);
    QCOMPARE(QString::fromUtf8(error.what()), detail);
    ++caught;
  } catch (int marker) {
    QVERIFY(preflight);
    QVERIFY(unknown);
    QCOMPARE(marker, 42);
    ++caught;
  }
  QCOMPARE(caught, preflight ? 1 : 0);
  QCOMPARE(service->removals, source <= 2 || preflight ? 0 : 1);
  QCOMPARE(service->activeDeletionBarriers, 0);
  QVERIFY(wmi.invocations.empty());
  if (result) {
    QVERIFY(!result->persistenceWarning.isEmpty());
    if (!unknown) QCOMPARE(result->persistenceWarning, detail);
    const auto copied = *result;
    QCOMPARE(copied.persistenceWarning, result->persistenceWarning);
  }
  auto errors = recentLogLines();
  std::erase_if(errors, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(errors.size(), std::size_t{1});
  QVERIFY(errors.front().find(unknown ? "error=non-standard-exception" : detail.toStdString()) != std::string::npos);
  QVERIFY(errors.front().find(source == 2 || source == 5 ? "registry commit batch read failed:" : " E enhanced]") != std::string::npos);
  QVERIFY(errors.front().find("item failed:") == std::string::npos);
}

void EnhancedModeManagerTests::itemPersistenceWarningsDoNotRepeatRegistryFailureLogs() {
  RecordingWmiOperations wmi;
  using Value = app::wmi::WmiValue;
  wmi.queryResults.push_back({{{"__PATH", Value::fromString("filter")}, {"CurrentEnabled", Value::fromBool(true)}, {"NextEnabled", Value::fromBool(true)}}});
  wmi.queryResults.push_back({{{"__PATH", Value::fromString("registry")},
                               {"CurrentSession", Value::fromBool(true)},
                               {"PersistDomainSecretKey", Value::fromBool(false)},
                               {"PersistTSCAL", Value::fromBool(false)}}});
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  for (int index = 0; index < 100; ++index) service->installationPlan.push_back({"HKEY_CURRENT_USER\\logging-only", "value-" + std::to_string(index)});
  const auto detail = QStringLiteral("逐目标持久化失败\n完整原始详情");
  wmi.invocationObserver = [&] { wmi.invocationFailure = std::make_exception_ptr(std::runtime_error(detail.toStdString())); };
  app::EnhancedModeManager manager(service, wmi, app::UwfCapability::Available);
  const auto result = manager.enable(QStringLiteral("logging fixture"));
  QVERIFY(result.persistenceWarning.contains(QStringLiteral("100")));
  QCOMPARE(wmi.invocations.size(), std::size_t{100});
  auto errors = recentLogLines();
  std::erase_if(errors, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(errors.size(), std::size_t{100});
  for (const auto& line : errors) {
    QVERIFY(line.find("registry commit item failed:") != std::string::npos);
    QVERIFY(line.find(detail.toStdString()) != std::string::npos);
  }
}
}  // namespace

QTEST_GUILESS_MAIN(EnhancedModeManagerTests)
#include "EnhancedModeManagerTests.moc"
