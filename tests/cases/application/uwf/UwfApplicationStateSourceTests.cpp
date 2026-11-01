// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QTest>
#include <chrono>
#include <future>

#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/UwfStateUpdates.h"
#include "application/uwf/adapters/UwfApplicationStateSource.h"
#include "support/uwf/FakeUwfBackend.h"
namespace {
using namespace uwf;
class RecordingVolumes final : public app::VolumeRepository {
 public:
  std::vector<core::VolumeInfo> read() const override {
    ++reads;
    return {};
  }
  std::string driveForPath(const std::string&) const override {
    ++resolutions;
    return "Z:";
  }
  std::vector<std::string> mountedLetters(const std::string&) const override { return {"Z:"}; }
  std::vector<std::string> mountedPaths(const std::string&) const override { return {"Z:\\"}; }
  std::string systemDrive() const override {
    ++systemDrives;
    return "Z:";
  }
  mutable int reads = 0;
  mutable int resolutions = 0;
  mutable int systemDrives = 0;
};
class UwfApplicationStateSourceTests final : public QObject {
  Q_OBJECT
 private slots:
  void failedGlobalRefreshCannotPublishAnOlderCompleteSnapshot() {
    auto raw = std::make_shared<test::FakeUwfBackend>();
    int refreshes = 0;
    bool fail = true;
    auto system = std::make_shared<app::PrecheckedUwf>(raw, app::UwfFactsSource{}, false, [&](const compat::Uwf& shared, const compat::Context& context) {
      ++refreshes;
      return fail ? compat::Response::fail(compat::ErrorKind::AccessDenied, "Configuration refresh denied") : app::refreshUwfState(shared, context);
    });
    system->updateState(raw->state());
    const auto volumes = std::make_shared<RecordingVolumes>();
    const auto source = app::uwfApplicationStateSource(system, volumes, true);
    const auto read = source->readOperation(app::UwfCapability::Available);
    QVERIFY_THROWS_EXCEPTION(compat::Error, (void)read({}));
    QCOMPARE(refreshes, 1);
    QCOMPARE(volumes->reads, 1);
    QVERIFY(system->state().refreshFailure.has_value());
    const auto published = source->publishedOperation(app::UwfCapability::Available);
    QVERIFY_THROWS_EXCEPTION(compat::Error, (void)published({}));
    QCOMPARE(refreshes, 1);
    QCOMPARE(volumes->reads, 1);
    fail = false;
    const auto recovered = read({});
    QVERIFY(recovered.snapshot.uwfAvailable);
    QVERIFY(!system->state().refreshFailure);
    QCOMPARE(refreshes, 2);
    QCOMPARE(volumes->reads, 2);
    QVERIFY(recovered.overlayUsageSampled);
    raw->clearTrace();
    QVERIFY(published({}).snapshot.uwfAvailable);
    QVERIFY(!published({}).overlayUsageSampled);
    QVERIFY(raw->trace().empty());
    QCOMPARE(refreshes, 2);
    QCOMPARE(volumes->reads, 2);
  }
  void publishedApplicationStateDoesNotWaitForAnInFlightUwfOperation() {
    using namespace std::chrono_literals;
    auto raw = std::make_shared<test::FakeUwfBackend>();
    auto system = std::make_shared<app::PrecheckedUwf>(raw, app::UwfFactsSource{}, false);
    system->updateState(raw->state());
    const auto volumes = std::make_shared<RecordingVolumes>();
    const auto source = app::uwfApplicationStateSource(system, volumes, true);
    QVERIFY(source->readOperation(app::UwfCapability::Available)({}).snapshot.uwfAvailable);
    const auto published = source->publishedOperation(app::UwfCapability::Available);
    raw->clearTrace();
    std::promise<void> entered, release;
    auto enteredFuture = entered.get_future();
    auto releaseFuture = release.get_future().share();
    std::future<void> writer;
    std::future<app::ApplicationState> reader;
    bool released = false;
    const auto finish = qScopeGuard([&] {
      if (!released) release.set_value();
      if (writer.valid()) writer.wait();
      if (reader.valid()) reader.wait();
    });
    writer = std::async(std::launch::async, [&] {
      system->synchronize([&] {
        entered.set_value();
        releaseFuture.wait();
      });
    });
    QVERIFY(enteredFuture.wait_for(2s) == std::future_status::ready);
    reader = std::async(std::launch::async, [&] { return published({}); });
    const auto ready = reader.wait_for(2s);
    release.set_value();
    released = true;
    QVERIFY(ready == std::future_status::ready);
    QVERIFY(reader.get().snapshot.uwfAvailable);
    writer.get();
    QCOMPARE(volumes->reads, 1);
    QVERIFY(raw->trace().empty());
  }
  void unavailableUwfDoesNotRunConfigurationRefresh() {
    auto raw = std::make_shared<test::FakeUwfBackend>();
    int refreshes = 0;
    auto system = std::make_shared<app::PrecheckedUwf>(raw, app::UwfFactsSource{}, false, [&](const compat::Uwf&, const compat::Context&) {
      ++refreshes;
      return compat::Response::fail(compat::ErrorKind::Unsupported, "UWF is not installed");
    });
    const auto volumes = std::make_shared<RecordingVolumes>();
    const auto source = app::uwfApplicationStateSource(system, volumes, true);
    const auto state = source->readOperation(app::UwfCapability::Unavailable)({});
    QVERIFY(!state.snapshot.uwfAvailable);
    QCOMPARE(refreshes, 0);
    QCOMPARE(volumes->reads, 1);
  }
  void preparedReadsOwnTheirDependenciesAndHonorCancellation() {
    auto system = std::make_shared<test::FakeUwfBackend>();
    auto volumes = std::make_shared<RecordingVolumes>();
    auto source = app::uwfApplicationStateSource(system, volumes, true);
    const auto read = source->readOperation(app::UwfCapability::Available);
    source.reset();
    QVERIFY(read({}).snapshot.uwfAvailable);
    system->clearTrace();
    std::stop_source stop;
    stop.request_stop();
    QVERIFY_THROWS_EXCEPTION(compat::Error, (void)read(stop.get_token()));
    QCOMPARE(volumes->reads, 1);
    QVERIFY(system->trace().empty());
  }
};
}  // namespace
QTEST_APPLESS_MAIN(UwfApplicationStateSourceTests)
#include "UwfApplicationStateSourceTests.moc"
