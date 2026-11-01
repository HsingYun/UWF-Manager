// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QTest>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

#include "application/enhanced/EnhancedModeManager.h"
#include "support/app/EnhancedModeFixture.h"
#include "support/app/UwfMaintenanceFixture.h"
#include "support/app/WorkerGate.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

class EnhancedModeManagerTests final : public QObject {
  Q_OBJECT
 private slots:
  void requestsStaySerializedThroughStatePublication() {
    const auto owner = std::this_thread::get_id();
    auto gate = std::make_shared<test::WorkerGate>();
    auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
    service->beforeInstall = [gate] { gate->wait(); };
    UwfMaintenanceFixture fixture;
    app::EnhancedModeManager manager(service, fixture.repository(), app::UwfCapability::Unavailable);
    bool preparationObserved = false;
    service->beforeQuery = [&, owner] {
      if (std::this_thread::get_id() != owner || preparationObserved) return;
      preparationObserved = true;
      QVERIFY(manager.operationInProgress());
      manager.requestChange(app::EnhancedModeAction::Disable);
    };
    int starts = 0;
    std::vector<app::EnhancedModeStatus> statuses;
    std::vector<app::EnhancedModeChangeResult> results;
    connect(&manager, &app::EnhancedModeActions::operationStarted, this, [&] {
      QCOMPARE(std::this_thread::get_id(), owner);
      QVERIFY(manager.operationInProgress());
      ++starts;
      manager.requestChange(app::EnhancedModeAction::Disable);
    });
    connect(&manager, &app::EnhancedModeActions::statusChanged, this, [&](const auto status) {
      QCOMPARE(std::this_thread::get_id(), owner);
      QVERIFY(manager.operationInProgress());
      QCOMPARE(manager.status().state, status.state);
      statuses.push_back(status);
      manager.requestChange(app::EnhancedModeAction::Disable);
      // Session observers can authenticate synchronously while accepting status.
      manager.setAgentState(status.serviceContractSatisfied() ? app::EnhancedModeAgentState::Connected : app::EnhancedModeAgentState::Unobserved);
    });
    connect(&manager, &app::EnhancedModeActions::operationFinished, this, [&](const auto& result, const auto failure) {
      QCOMPARE(std::this_thread::get_id(), owner);
      QVERIFY(!manager.operationInProgress());
      QVERIFY(!failure);
      QCOMPARE(manager.status().agentState, result.status.agentState);
      results.push_back(result);
    });

    const auto releaseWorker = qScopeGuard([gate] { gate->release(); });
    manager.requestChange(app::EnhancedModeAction::Enable, QStringLiteral("serialized service"));
    QVERIFY(preparationObserved);
    QTRY_COMPARE(gate->started.load(), 1);
    manager.requestChange(app::EnhancedModeAction::Start);
    manager.requestChange(app::EnhancedModeAction::Repair);
    manager.requestChange(app::EnhancedModeAction::Disable);
    const auto queries = service->queries;
    QCOMPARE(manager.status().state, app::EnhancedModeState::Disabled);
    QCOMPARE(service->queries, queries);
    QCOMPARE(starts, 1);
    QVERIFY(statuses.empty());
    gate->release();
    QTRY_COMPARE(results.size(), std::size_t{1});
    QCOMPARE(statuses.size(), std::size_t{1});
    QCOMPARE(results.back().status.state, app::EnhancedModeState::Enabled);
    QCOMPARE(results.back().status.agentState, app::EnhancedModeAgentState::Connected);
    QCOMPARE(service->installedDescription, QStringLiteral("serialized service"));
    QCOMPARE(service->installs, 1);
    QCOMPARE(service->starts, 0);
    QCOMPARE(service->removals, 0);

    manager.requestChange(app::EnhancedModeAction::Start);
    QTRY_COMPARE(results.size(), std::size_t{2});
    QCOMPARE(service->starts, 1);
    manager.requestChange(app::EnhancedModeAction::Repair, QStringLiteral("repaired service"));
    QTRY_COMPARE(results.size(), std::size_t{3});
    QCOMPARE(service->installs, 2);
    QCOMPARE(service->installedDescription, QStringLiteral("repaired service"));
    manager.requestChange(app::EnhancedModeAction::Disable);
    QTRY_COMPARE(results.size(), std::size_t{4});
    QCOMPARE(results.back().status.state, app::EnhancedModeState::Disabled);
    QCOMPARE(results.back().status.agentState, app::EnhancedModeAgentState::Unobserved);
    QVERIFY(results.back().status.serviceExists);
    QVERIFY(results.back().status.startupDisabled);
    QCOMPARE(service->disables, 1);
    QCOMPARE(service->removals, 0);
    manager.requestChange(app::EnhancedModeAction::Remove);
    QTRY_COMPARE(results.size(), std::size_t{5});
    QVERIFY(!results.back().status.hasRemnants());
    QCOMPARE(service->removals, 1);
    QCOMPARE(starts, 5);
    QCOMPARE(statuses.size(), results.size());
  }

  void failuresReopenTheSameAdmissionGate_data() {
    QTest::addColumn<bool>("preparation");
    QTest::newRow("status-preparation") << true;
    QTest::newRow("service-worker") << false;
  }
  void failuresReopenTheSameAdmissionGate() {
    QFETCH(bool, preparation);
    auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
    if (preparation)
      service->queryFailure = QStringLiteral("controlled preparation failure");
    else
      service->beforeInstall = [] { throw std::runtime_error("controlled worker failure"); };
    UwfMaintenanceFixture fixture;
    app::EnhancedModeManager manager(service, fixture.repository(), app::UwfCapability::Unavailable);
    int finished = 0, published = 0;
    std::exception_ptr failure;
    connect(&manager, &app::EnhancedModeActions::statusChanged, this, [&](const auto&) { ++published; });
    connect(&manager, &app::EnhancedModeActions::operationFinished, this, [&](const auto&, const auto error) {
      ++finished;
      failure = error;
    });
    manager.requestChange(app::EnhancedModeAction::Enable);
    QTRY_COMPARE(finished, 1);
    QVERIFY(failure);
    QVERIFY(!manager.operationInProgress());
    QCOMPARE(published, 0);
    QCOMPARE(service->installs, 0);
    service->queryFailure.clear();
    service->beforeInstall = {};
    manager.requestChange(app::EnhancedModeAction::Enable);
    QTRY_COMPARE(finished, 2);
    QVERIFY(!failure);
    QVERIFY(!manager.operationInProgress());
    QCOMPARE(published, 1);
    QCOMPARE(service->installs, 1);
    QCOMPARE(manager.status().state, app::EnhancedModeState::Enabled);
  }

  void workerOwnsItsServiceAfterManagerDestruction() {
    auto gate = std::make_shared<test::WorkerGate>();
    auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
    service->beforeInstall = [gate] { gate->wait(); };
    std::weak_ptr<MemoryEnhancedModeServiceControl> lifetime = service;
    UwfMaintenanceFixture fixture;
    auto manager = std::make_unique<app::EnhancedModeManager>(service, fixture.repository(), app::UwfCapability::Unavailable);
    const auto releaseWorker = qScopeGuard([gate] { gate->release(); });
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
};
}  // namespace

QTEST_GUILESS_MAIN(EnhancedModeManagerTests)
#include "EnhancedModeManagerTests.moc"
