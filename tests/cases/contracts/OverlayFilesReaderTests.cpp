// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <QThreadPool>
#include <exception>
#include <functional>
#include <future>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <utility>

#include "app/wmi/WmiException.h"
#include "windows/uwf/OverlayFilesReader.h"
#include "windows/wmi/WmiRuntime.h"

namespace {
using namespace uwf;
using namespace uwf::app::wmi;

enum class Scenario { Success, ConnectionFailure, InstanceFailure, MethodFailure, MethodCanceled, StopAfterMethod, AlreadyCanceled };

struct Calls {
  int initializations = 0;
  int shutdowns = 0;
  int embeddedAccesses = 0;
  int cimv2Accesses = 0;
  int connections = 0;
  int instanceQueries = 0;
  int synchronousReads = 0;
  int cancelableReads = 0;
  int followUpQueries = 0;
};

struct RuntimeState {
  Scenario scenario = Scenario::Success;
  std::stop_source stop;
  Calls calls;
  bool retired = false;

  void requireActive() const {
    if (retired) throw std::logic_error("WMI thread context was already destroyed");
  }
};

class MemoryWmiOperations final : public WmiOperations {
 public:
  MemoryWmiOperations(RuntimeState& state, std::string name) : m_state(state), m_name(std::move(name)) {}

  void ensureConnected() const override {
    m_state.requireActive();
    ++m_state.calls.connections;
    if (m_state.scenario == Scenario::ConnectionFailure) throw WmiInfrastructureError(-1, "controlled connection failure");
  }

  std::vector<WmiRow> query(const std::string& wql) const override {
    m_state.requireActive();
    if (wql != "SELECT Marker FROM ControlledSource") throw std::logic_error("unexpected follow-up query");
    ++m_state.calls.followUpQueries;
    return {{{"Marker", WmiValue::fromString(m_name)}}};
  }

  std::vector<WmiRow> queryInstances(const std::string& wql) const override {
    m_state.requireActive();
    if (m_name != "embedded" || wql != "SELECT * FROM UWF_Overlay") throw std::logic_error("unexpected overlay instance query");
    ++m_state.calls.instanceQueries;
    if (m_state.scenario == Scenario::InstanceFailure) throw WmiTransportError(-2, "controlled instance failure");
    return {{{"__PATH", WmiValue::fromString("controlled-overlay")},
             {"OverlayConsumption", WmiValue::fromUInt(10)},
             {"AvailableSpace", WmiValue::fromUInt(90)},
             {"CriticalOverlayThreshold", WmiValue::fromUInt(80)},
             {"WarningOverlayThreshold", WmiValue::fromUInt(70)}}};
  }

  WmiMethodOutput callMethodRead(const std::string& path, const std::string& method, const WmiRow& inputs) const override {
    ++m_state.calls.synchronousReads;
    return readFiles(path, method, inputs);
  }

  WmiMethodOutput callMethodReadCancelable(const std::string& path, const std::string& method, const WmiRow& inputs,
                                         std::stop_token token) const override {
    ++m_state.calls.cancelableReads;
    if (!token.stop_possible() || token != m_state.stop.get_token()) throw std::logic_error("reader did not forward its cancellation token");
    return readFiles(path, method, inputs);
  }

  WmiClassStatus classStatus(const std::string&) const override { throw std::logic_error("unexpected class probe"); }
  WmiRow getObject(const std::string&) const override { throw std::logic_error("unexpected object read"); }
  void invokeMethod(const std::string&, const std::string&, const WmiRow&) const override { throw std::logic_error("unexpected write method"); }
  void putInstance(const std::string&, const WmiRow&, WmiPutMode) const override { throw std::logic_error("unexpected instance write"); }

 private:
  WmiMethodOutput readFiles(const std::string& path, const std::string& method, const WmiRow& inputs) const {
    m_state.requireActive();
    if (m_name != "embedded" || path != "controlled-overlay" || method != "GetOverlayFiles" || inputs.size() != 1 ||
        inputs.at("Volume").toString() != "Z:")
      throw std::logic_error("unexpected overlay method request");
    if (m_state.scenario == Scenario::MethodFailure) throw WmiProviderError(87, "controlled method failure");
    if (m_state.scenario == Scenario::MethodCanceled) throw WmiCancelled("controlled method cancellation");
    if (m_state.scenario == Scenario::StopAfterMethod) m_state.stop.request_stop();
    WmiMethodOutput output;
    output.arrays["OverlayFiles"] = {{{"FileName", WmiValue::fromString(R"(\Data\settings.bin)")}, {"FileSize", WmiValue::fromUInt(4097)}},
                                    {{"FileName", WmiValue::fromString(R"(\Data\empty.txt)")}, {"FileSize", WmiValue::fromUInt(0)}}};
    return output;
  }

  RuntimeState& m_state;
  std::string m_name;
};

struct MemoryRuntime {
  RuntimeState state;
  MemoryWmiOperations embedded{state, "embedded"};
  MemoryWmiOperations cimv2{state, "cimv2"};
};

MemoryRuntime& runtime() {
  thread_local MemoryRuntime value;
  return value;
}

struct Outcome {
  std::thread::id thread;
  QVector<app::OverlayFileEntry> files;
  QStringList markers;
  std::exception_ptr failure;
  Calls calls;
};

Outcome runTask(QThreadPool& pool, std::function<void(Outcome&)> work) {
  std::promise<Outcome> promise;
  auto result = promise.get_future();
  pool.start([work = std::move(work), promise = std::move(promise)]() mutable {
    Outcome outcome;
    outcome.thread = std::this_thread::get_id();
    try {
      work(outcome);
    } catch (...) {
      outcome.failure = std::current_exception();
    }
    outcome.calls = runtime().state.calls;
    promise.set_value(std::move(outcome));
  });
  return result.get();
}

QString failureText(const std::exception_ptr& failure) {
  if (!failure) return {};
  try {
    std::rethrow_exception(failure);
  } catch (const std::exception& error) {
    return QString::fromUtf8(error.what());
  } catch (...) {
    return QStringLiteral("non-standard exception");
  }
}

void verifyRead(const Outcome& outcome, Scenario scenario) {
  switch (scenario) {
    case Scenario::ConnectionFailure:
      QVERIFY(outcome.failure);
      QVERIFY_THROWS_EXCEPTION(WmiInfrastructureError, std::rethrow_exception(outcome.failure));
      break;
    case Scenario::InstanceFailure:
      QVERIFY(outcome.failure);
      QVERIFY_THROWS_EXCEPTION(WmiTransportError, std::rethrow_exception(outcome.failure));
      break;
    case Scenario::MethodFailure:
      QVERIFY(outcome.failure);
      QVERIFY_THROWS_EXCEPTION(WmiProviderError, std::rethrow_exception(outcome.failure));
      break;
    case Scenario::MethodCanceled:
      QVERIFY(outcome.failure);
      QVERIFY_THROWS_EXCEPTION(WmiCancelled, std::rethrow_exception(outcome.failure));
      break;
    case Scenario::AlreadyCanceled:
    case Scenario::StopAfterMethod:
      QVERIFY2(!outcome.failure, qUtf8Printable(failureText(outcome.failure)));
      QVERIFY(outcome.files.isEmpty());
      break;
    case Scenario::Success:
      QVERIFY2(!outcome.failure, qUtf8Printable(failureText(outcome.failure)));
      QCOMPARE(outcome.files.size(), 2);
      QCOMPARE(outcome.files[0].rawName, QStringLiteral(R"(\Data\settings.bin)"));
      QCOMPARE(outcome.files[0].fileSize, qulonglong{4097});
      QCOMPARE(outcome.files[1].rawName, QStringLiteral(R"(\Data\empty.txt)"));
      QCOMPARE(outcome.files[1].fileSize, qulonglong{0});
      for (const auto& file : outcome.files) {
        QVERIFY(file.absolutePath.isEmpty());
        QVERIFY(!file.isDirectory);
        QVERIFY(!file.isSystemMetadata);
      }
      break;
  }
}

class OverlayFilesReaderTests final : public QObject {
  Q_OBJECT

 private slots:
  void readLeavesReusableThreadContextUsable_data();
  void readLeavesReusableThreadContextUsable();
};

void OverlayFilesReaderTests::readLeavesReusableThreadContextUsable_data() {
  QTest::addColumn<int>("scenarioValue");
  QTest::addColumn<bool>("cancelable");
  QTest::newRow("success") << static_cast<int>(Scenario::Success) << false;
  QTest::newRow("cancelable-success") << static_cast<int>(Scenario::Success) << true;
  QTest::newRow("connection-failure") << static_cast<int>(Scenario::ConnectionFailure) << true;
  QTest::newRow("instance-failure") << static_cast<int>(Scenario::InstanceFailure) << true;
  QTest::newRow("method-failure") << static_cast<int>(Scenario::MethodFailure) << false;
  QTest::newRow("cancelable-method-failure") << static_cast<int>(Scenario::MethodFailure) << true;
  QTest::newRow("method-canceled") << static_cast<int>(Scenario::MethodCanceled) << true;
  QTest::newRow("stop-after-method") << static_cast<int>(Scenario::StopAfterMethod) << true;
  QTest::newRow("already-canceled") << static_cast<int>(Scenario::AlreadyCanceled) << true;
}

void OverlayFilesReaderTests::readLeavesReusableThreadContextUsable() {
  QFETCH(int, scenarioValue);
  QFETCH(bool, cancelable);
  const auto scenario = static_cast<Scenario>(scenarioValue);
  QThreadPool pool;
  pool.setMaxThreadCount(1);
  pool.setExpiryTimeout(-1);
  std::thread::id workerThread;
  for (int task = 0; task < 4; ++task) {
    const auto outcome = runTask(pool, [task, scenario, cancelable](Outcome& result) {
      if (task % 2 == 0) {
        auto& state = runtime().state;
        state.scenario = task == 0 ? scenario : Scenario::Success;
        state.stop = std::stop_source{};
        if (state.scenario == Scenario::AlreadyCanceled) state.stop.request_stop();
        result.files = windows::loadOverlayFilesFromSystem(QStringLiteral("Z:"), cancelable ? state.stop.get_token() : std::stop_token{});
      } else {
        // Resolve both namespaces in a separate task on the same pool thread.
        // A reader that closes its caller's runtime makes this access fail.
        for (auto* session : {&windows::embeddedWmiSession(), &windows::cimv2WmiSession()}) {
          const auto rows = session->query("SELECT Marker FROM ControlledSource");
          if (rows.size() != 1) throw std::logic_error("follow-up query returned no marker");
          result.markers.append(QString::fromStdString(rows.front().at("Marker").toString()));
        }
      }
    });
    if (workerThread == std::thread::id{}) workerThread = outcome.thread;
    QVERIFY2(outcome.thread == workerThread, "test must reuse the same pool thread");
    if (task % 2 == 0) {
      verifyRead(outcome, task == 0 ? scenario : Scenario::Success);
      if (task == 0) {
        const bool accessed = scenario != Scenario::AlreadyCanceled;
        const bool queried = accessed && scenario != Scenario::ConnectionFailure;
        const bool invoked = queried && scenario != Scenario::InstanceFailure;
        QCOMPARE(outcome.calls.embeddedAccesses, accessed ? 1 : 0);
        QCOMPARE(outcome.calls.cimv2Accesses, 0);
        QCOMPARE(outcome.calls.connections, accessed ? 1 : 0);
        QCOMPARE(outcome.calls.instanceQueries, queried ? 1 : 0);
        QCOMPARE(outcome.calls.synchronousReads, invoked && !cancelable ? 1 : 0);
        QCOMPARE(outcome.calls.cancelableReads, invoked && cancelable ? 1 : 0);
        if (!accessed) QCOMPARE(outcome.calls.initializations, 0);
      }
    } else {
      QVERIFY2(!outcome.failure, qUtf8Printable(failureText(outcome.failure)));
      QCOMPARE(outcome.markers, QStringList({QStringLiteral("embedded"), QStringLiteral("cimv2")}));
      QCOMPARE(outcome.calls.followUpQueries, task + 1);
      QCOMPARE(outcome.calls.shutdowns, 0);
    }
  }
}

}  // namespace

// This target links the production reader and these controlled runtime entry
// points, without UwfWindows or COM. Closing a runtime retires both namespaces,
// exactly the public lifetime contract that the reader must respect.
namespace uwf::windows {
app::wmi::WmiOperations& embeddedWmiSession() {
  auto& context = runtime();
  context.state.requireActive();
  ++context.state.calls.embeddedAccesses;
  return context.embedded;
}
app::wmi::WmiOperations& cimv2WmiSession() {
  auto& context = runtime();
  context.state.requireActive();
  ++context.state.calls.cimv2Accesses;
  return context.cimv2;
}
void initializeWmiRuntime() {
  auto& state = runtime().state;
  state.requireActive();
  ++state.calls.initializations;
}
void shutdownWmiRuntime() noexcept {
  auto& state = runtime().state;
  ++state.calls.shutdowns;
  state.retired = true;
}
}  // namespace uwf::windows

QTEST_APPLESS_MAIN(OverlayFilesReaderTests)

#include "OverlayFilesReaderTests.moc"
