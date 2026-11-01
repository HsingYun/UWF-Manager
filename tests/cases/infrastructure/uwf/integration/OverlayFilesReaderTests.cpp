// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <QThreadPool>
#include <atomic>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "application/operations/contracts/OperationError.h"
#include "application/overlay/files/OverlayFilesFailure.h"
#include "application/overlay/files/OverlayFilesQueryManager.h"
#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/adapters/UwfOverlayReaders.h"
#include "infrastructure/uwf/windows/wmi/WmiUwfBackend.h"
#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/management/wmi/WmiMethodCompletion.h"

namespace {
using namespace uwf;
using namespace uwf::windows::wmi;
constexpr auto volumeName = "Volume{11111111-2222-3333-4444-555555555555}";

enum class Scenario { Success, ConnectionFailure, InstanceFailure, MethodFailure, MethodCanceled, UntypedMethodFailure, StopAfterMethod, AlreadyCanceled };

struct Calls {
  int embeddedAccesses = 0;
  int cimv2Accesses = 0;
  int connections = 0;
  int filterQueries = 0;
  int volumeQueries = 0;
  int instanceQueries = 0;
  int synchronousReads = 0;
  int singleReads = 0;
  int followUpQueries = 0;
};

struct RuntimeState {
  Scenario scenario = Scenario::Success;
  std::stop_source stop;
  Calls calls;
};

class MemoryWmiOperations final : public WmiOperations {
 public:
  MemoryWmiOperations(RuntimeState& state, std::string name) : m_state(state), m_name(std::move(name)) {}

  void ensureConnected() const override {
    ++m_state.calls.connections;
    if (m_state.scenario == Scenario::ConnectionFailure) throw WmiInfrastructureError(-1, "controlled connection failure");
  }

  std::vector<WmiRow> query(const std::string& wql) const override {
    if (wql != "SELECT Marker FROM ControlledSource") throw std::logic_error("unexpected follow-up query");
    ++m_state.calls.followUpQueries;
    return {{{"Marker", WmiValue::fromString(m_name)}}};
  }

  std::vector<WmiRow> queryInstances(const std::string& wql) const override {
    ensureConnected();
    if (m_name != "embedded") throw std::logic_error("unexpected UWF namespace");
    if (wql == "SELECT * FROM UWF_Filter") {
      ++m_state.calls.filterQueries;
      return {{{"__PATH", WmiValue::fromString("controlled-filter")}, {"CurrentEnabled", WmiValue::fromBool(true)}}};
    }
    if (wql == "SELECT * FROM UWF_Volume") {
      ++m_state.calls.volumeQueries;
      return {{{"__PATH", WmiValue::fromString("controlled-current-volume")},
               {"VolumeName", WmiValue::fromString(volumeName)},
               {"CurrentSession", WmiValue::fromBool(true)},
               {"DriveLetter", WmiValue::fromString("Z:")},
               {"Protected", WmiValue::fromBool(true)},
               {"BindByDriveLetter", WmiValue::fromBool(false)}}};
    }
    if (wql != "SELECT * FROM UWF_Overlay") throw std::logic_error("unexpected UWF instance query");
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

  WmiMethodOutput callMethodReadOnce(const std::string& path, const std::string& method, const WmiRow& inputs, std::stop_token token) const override {
    ++m_state.calls.singleReads;
    if (token.stop_possible() && token != m_state.stop.get_token()) throw std::logic_error("reader did not forward its cancellation token");
    windows::WmiMethodCompletion completion;
    const auto status = m_state.scenario == Scenario::MethodCanceled ? static_cast<HRESULT>(WBEM_E_CALL_CANCELLED)
                        : m_state.scenario == Scenario::MethodFailure ? static_cast<HRESULT>(WBEM_E_INVALID_PARAMETER)
                                                                     : S_OK;
    completion.complete(status);
    auto progress = ReadCompletion::Unknown;
    const windows::SrwExclusiveLock lock(completion.lock);
    completion.checkReadStatus(progress, "controlled single-dispatch GetOverlayFiles");
    return readFiles(path, method, inputs);
  }

  WmiClassStatus classStatus(const std::string& name) const override {
    if (m_name != "embedded" || (name != "UWF_Filter" && name != "UWF_Volume" && name != "UWF_Overlay")) throw std::logic_error("unexpected class probe");
    return WmiClassStatus::Present;
  }
  WmiRow getObject(const std::string&) const override { throw std::logic_error("unexpected object read"); }
  WmiMethodOutput invokeMethod(const std::string&, const std::string&, const WmiRow&) const override { throw std::logic_error("unexpected write method"); }
  void putInstance(const std::string&, const WmiRow&, WmiPutMode) const override { throw std::logic_error("unexpected instance write"); }

 private:
  WmiMethodOutput readFiles(const std::string& path, const std::string& method, const WmiRow& inputs) const {
    if (m_name != "embedded" || path != "controlled-overlay" || method != "GetOverlayFiles" || inputs.size() != 1 ||
        inputs.at("Volume").toString() != volumeName)
      throw std::logic_error("unexpected overlay method request");
    if (m_state.scenario == Scenario::MethodFailure) throw WmiProviderError(87, "controlled method failure");
    if (m_state.scenario == Scenario::MethodCanceled) throw WmiCancelled("controlled method cancellation");
    if (m_state.scenario == Scenario::UntypedMethodFailure) throw std::runtime_error("controlled method exception after dispatch");
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

struct TerminalTransportState {
  explicit TerminalTransportState(const std::int32_t terminalStatus) : terminalStatus(terminalStatus) {}
  const std::int32_t terminalStatus;
  std::atomic_int methodCalls{0};
  std::atomic_int firstProgress{static_cast<int>(ReadCompletion::NotDispatched)};
  std::atomic_int firstErrorCompletion{static_cast<int>(ReadCompletion::NotDispatched)};
  std::atomic<std::int32_t> firstNativeCode{0};
  std::atomic<std::uint32_t> firstNativeResult{0};
};

// Only the raw method's terminal HRESULT varies. The same completion helper as
// WmiSession assigns its evidence and native exception before the production
// backend, UWF facade, reader and query manager receive it.
class TerminalWmiOperations final : public WmiOperations {
 public:
  explicit TerminalWmiOperations(std::shared_ptr<TerminalTransportState> state) : m_state(std::move(state)) {}
  void ensureConnected() const override {}
  std::vector<WmiRow> query(const std::string&) const override { throw std::logic_error("unexpected unscoped query"); }
  std::vector<WmiRow> queryInstances(const std::string& wql) const override {
    if (wql != "SELECT * FROM UWF_Overlay") throw std::logic_error("unexpected UWF instance read");
    return {{{"__PATH", WmiValue::fromString("controlled-terminal-overlay")}}};
  }
  WmiClassStatus classStatus(const std::string& name) const override {
    if (name != "UWF_Overlay") throw std::logic_error("unexpected class probe");
    return WmiClassStatus::Present;
  }
  WmiRow getObject(const std::string&) const override { throw std::logic_error("unexpected object read"); }
  WmiMethodOutput invokeMethod(const std::string&, const std::string&, const WmiRow&) const override { throw std::logic_error("unexpected write"); }
  void putInstance(const std::string&, const WmiRow&, WmiPutMode) const override { throw std::logic_error("unexpected instance write"); }
  WmiMethodOutput callMethodRead(const std::string&, const std::string&, const WmiRow&) const override {
    throw std::logic_error("the diagnostic query must dispatch once");
  }
  WmiMethodOutput callMethodReadOnce(const std::string& path, const std::string& method, const WmiRow& inputs, std::stop_token) const override {
    if (path != "controlled-terminal-overlay" || method != "GetOverlayFiles" || inputs.size() != 1 || inputs.at("Volume").toString() != volumeName)
      throw std::logic_error("unexpected overlay enumeration request");
    const int call = ++m_state->methodCalls;
    windows::WmiMethodCompletion completion;
    completion.complete(call == 1 ? static_cast<HRESULT>(m_state->terminalStatus) : S_OK);
    auto progress = ReadCompletion::Unknown;
    const windows::SrwExclusiveLock lock(completion.lock);
    try {
      completion.checkReadStatus(progress, "controlled terminal GetOverlayFiles");
    } catch (const WmiException& error) {
      if (call == 1) {
        m_state->firstProgress = static_cast<int>(progress);
        m_state->firstErrorCompletion = static_cast<int>(error.readCompletion());
        m_state->firstNativeCode = error.diagnostic().code;
        m_state->firstNativeResult = error.diagnostic().result;
      }
      throw;
    }
    WmiMethodOutput output;
    output.arrays["OverlayFiles"] = {{{"FileName", WmiValue::fromString(R"(\Data\fresh.txt)")}, {"FileSize", WmiValue::fromUInt(123)}}};
    return output;
  }

 private:
  std::shared_ptr<TerminalTransportState> m_state;
};

class QueryReceiver final : public QObject {
 public:
  std::vector<app::OverlayFilesQueryManager::Result> results;
  app::OverlayFilesQueryManager::Completion completion() {
    return [this](app::OverlayFilesQueryManager::Result result) { results.push_back(std::move(result)); };
  }
};

std::shared_ptr<app::PrecheckedUwf> terminalReadSystem(const std::shared_ptr<TerminalTransportState>& state) {
  auto transport = std::make_shared<TerminalWmiOperations>(state);
  auto backend = std::make_shared<windows::backends::WmiUwfBackend>([transport]() -> WmiOperations& { return *transport; });
  auto system = std::make_shared<app::PrecheckedUwf>(std::move(backend));
  compat::UwfStateFacts facts;
  facts.administrator = compat::Fact<bool>::known(true);
  facts.current.filterEnabled = compat::Fact<bool>::known(true);
  facts.current.volumesObserved = true;
  auto& volume = facts.current.volumes[compat::normalizeVolumeId(volumeName)];
  volume.present = compat::Fact<bool>::known(true);
  volume.protectedVolume = compat::Fact<bool>::known(true);
  volume.driveLetter = compat::Fact<std::string>::known("");
  volume.physicalDriveLetter = compat::Fact<std::string>::known("");
  system->updateState(std::move(facts));
  return system;
}

MemoryRuntime& runtime() {
  thread_local MemoryRuntime value;
  return value;
}

WmiOperations& controlledSession(const bool embedded) {
  auto& context = runtime();
  if (embedded) {
    ++context.state.calls.embeddedAccesses;
    return context.embedded;
  }
  ++context.state.calls.cimv2Accesses;
  return context.cimv2;
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
    case Scenario::InstanceFailure:
    case Scenario::MethodFailure:
    case Scenario::MethodCanceled:
    case Scenario::UntypedMethodFailure:
      QVERIFY(outcome.failure);
      try {
        std::rethrow_exception(outcome.failure);
      } catch (const app::OperationError& error) {
        QCOMPARE(error.writeEffect(), app::WriteEffect::None);
        if (scenario == Scenario::MethodCanceled || scenario == Scenario::UntypedMethodFailure) {
          QVERIFY(dynamic_cast<const app::OverlayFilesQueryUncertain*>(&error));
          QCOMPARE(error.failureKind(), app::OperationFailureKind::Other);
        }
        QVERIFY(QString::fromUtf8(error.what()).contains(QStringLiteral("controlled")));
      } catch (...) {
        QFAIL("The reader must expose the application error contract");
      }
      break;
    case Scenario::AlreadyCanceled:
    case Scenario::StopAfterMethod:
      QVERIFY2(!outcome.failure, qUtf8Printable(failureText(outcome.failure)));
      QVERIFY(outcome.files.isEmpty());
      break;
    case Scenario::Success:
      QVERIFY2(!outcome.failure, qUtf8Printable(failureText(outcome.failure)));
      QCOMPARE(outcome.files.size(), 2);
      QCOMPARE(outcome.files[0].rawName, QStringLiteral(R"(\Data\empty.txt)"));
      QCOMPARE(outcome.files[0].fileSize, qulonglong{0});
      QCOMPARE(outcome.files[1].rawName, QStringLiteral(R"(\Data\settings.bin)"));
      QCOMPARE(outcome.files[1].fileSize, qulonglong{4097});
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
  void driveLettersCannotReplaceTheCapturedVolumeIdentity() {
    auto backend =
        std::make_shared<windows::backends::WmiUwfBackend>([]() -> WmiOperations& { throw std::logic_error("The mount check must not access a provider"); });
    app::PrecheckedUwf system(backend);
    compat::UwfStateFacts facts;
    facts.current.volumesObserved = true;
    auto& volume = facts.current.volumes[compat::normalizeVolumeId(volumeName)];
    volume.present = compat::Fact<bool>::known(true);
    volume.driveLetter = compat::Fact<std::string>::known("Z:");
    volume.physicalDriveLetter = compat::Fact<std::string>::known("");
    system.updateState(std::move(facts));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)app::loadUwfOverlayFiles(system, QStringLiteral("Z:"), {}));
  }
  void readLeavesReusableThreadContextUsable_data();
  void readLeavesReusableThreadContextUsable();
  void terminalMethodEvidenceControlsManagerReopening_data();
  void terminalMethodEvidenceControlsManagerReopening();
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
  QTest::newRow("method-exception") << static_cast<int>(Scenario::UntypedMethodFailure) << false;
  QTest::newRow("cancelable-method-exception") << static_cast<int>(Scenario::UntypedMethodFailure) << true;
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
        // The selected GUID and prechecks come from an explicit in-memory refresh result.
        // Reading files must only resolve the overlay instance and call its own method.
        auto backend = std::make_shared<windows::backends::WmiUwfBackend>([]() -> WmiOperations& { return controlledSession(true); });
        app::PrecheckedUwf system(backend);
        compat::UwfStateFacts facts;
        facts.administrator = compat::Fact<bool>::known(true);
        facts.current.filterEnabled = compat::Fact<bool>::known(true);
        facts.current.volumesObserved = true;
        auto& volume = facts.current.volumes[compat::normalizeVolumeId(volumeName)];
        volume.present = compat::Fact<bool>::known(true);
        volume.protectedVolume = compat::Fact<bool>::known(true);
        volume.driveLetter = compat::Fact<std::string>::known("");
        volume.physicalDriveLetter = compat::Fact<std::string>::known("");
        system.updateState(std::move(facts));
        result.files = app::loadUwfOverlayFiles(system, QString::fromLatin1(volumeName), cancelable ? state.stop.get_token() : std::stop_token{});
      } else {
        // Resolve both namespaces in a separate task on the same pool thread.
        // A reader that closes its caller's runtime makes this access fail.
        for (auto* session : {&controlledSession(true), &controlledSession(false)}) {
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
        QCOMPARE(outcome.calls.embeddedAccesses > 0, accessed);
        QCOMPARE(outcome.calls.cimv2Accesses, 0);
        QCOMPARE(outcome.calls.connections, accessed ? 1 : 0);
        QCOMPARE(outcome.calls.filterQueries, 0);
        QCOMPARE(outcome.calls.volumeQueries, 0);
        QCOMPARE(outcome.calls.instanceQueries, queried ? 1 : 0);
        QCOMPARE(outcome.calls.synchronousReads, 0);
        QCOMPARE(outcome.calls.singleReads, invoked ? 1 : 0);
      }
    } else {
      QVERIFY2(!outcome.failure, qUtf8Printable(failureText(outcome.failure)));
      QCOMPARE(outcome.markers, QStringList({QStringLiteral("embedded"), QStringLiteral("cimv2")}));
      QCOMPARE(outcome.calls.followUpQueries, task + 1);
    }
  }
}

void OverlayFilesReaderTests::terminalMethodEvidenceControlsManagerReopening_data() {
  QTest::addColumn<qint32>("terminalStatus");
  QTest::addColumn<bool>("completed");
  QTest::addColumn<int>("expectedKind");
  const auto add = [](const char* name, const HRESULT status, const bool completed, const app::OperationFailureKind kind) {
    QTest::newRow(name) << static_cast<qint32>(status) << completed << static_cast<int>(kind);
  };
  using Kind = app::OperationFailureKind;
  add("completed-wmi-out-of-memory", static_cast<HRESULT>(WBEM_E_OUT_OF_MEMORY), true, Kind::ResourceExhausted);
  add("completed-com-out-of-memory", E_OUTOFMEMORY, true, Kind::ResourceExhausted);
  add("completed-quota-violation", static_cast<HRESULT>(WBEM_E_QUOTA_VIOLATION), true, Kind::ResourceExhausted);
  add("completed-wmi-access-denied", static_cast<HRESULT>(WBEM_E_ACCESS_DENIED), true, Kind::AccessDenied);
  add("completed-com-access-denied", E_ACCESSDENIED, true, Kind::AccessDenied);
  add("completed-business-failure", static_cast<HRESULT>(WBEM_E_INVALID_PARAMETER), true, Kind::InvalidArgument);
  add("completed-ordinary-failure", static_cast<HRESULT>(WBEM_E_FAILED), true, Kind::Failed);
  add("unknown-provider-cancellation", static_cast<HRESULT>(WBEM_E_CALL_CANCELLED), false, Kind::Other);
  add("unknown-client-abort", E_ABORT, false, Kind::Other);
  add("unknown-disconnected", RPC_E_DISCONNECTED, false, Kind::Other);
  add("unknown-transport-failure", static_cast<HRESULT>(WBEM_E_TRANSPORT_FAILURE), false, Kind::Other);
  add("unknown-rpc-server-unavailable", HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE), false, Kind::Other);
  add("unknown-wmi-timeout", static_cast<HRESULT>(WBEM_E_TIMED_OUT), false, Kind::Other);
  add("unknown-positive-timeout", static_cast<HRESULT>(WBEM_S_TIMEDOUT), false, Kind::Other);
  add("unknown-positive-pending", static_cast<HRESULT>(WBEM_S_PENDING), false, Kind::Other);
  add("unknown-positive-cancellation", static_cast<HRESULT>(WBEM_S_OPERATION_CANCELLED), false, Kind::Other);
  add("unknown-com-pending", E_PENDING, false, Kind::Other);
}

void OverlayFilesReaderTests::terminalMethodEvidenceControlsManagerReopening() {
  QFETCH(qint32, terminalStatus);
  QFETCH(bool, completed);
  QFETCH(int, expectedKind);
  auto state = std::make_shared<TerminalTransportState>(terminalStatus);
  const auto system = terminalReadSystem(state);
  app::OverlayFilesQueryManager manager([system](const QString& volume, const std::stop_token stop) { return app::loadUwfOverlayFiles(*system, volume, stop); });
  auto first = std::make_unique<QueryReceiver>();
  auto firstSubscription = manager.subscribe(QString::fromLatin1(volumeName), first.get(), first->completion());
  QTRY_COMPARE(first->results.size(), std::size_t{1});
  QVERIFY(std::holds_alternative<std::exception_ptr>(first->results.front()));
  const auto failure = std::get<std::exception_ptr>(first->results.front());
  const auto evidence = completed ? ReadCompletion::ProviderCompleted : ReadCompletion::Unknown;
  QCOMPARE(state->firstProgress.load(), static_cast<int>(evidence));
  QCOMPARE(state->firstErrorCompletion.load(), static_cast<int>(evidence));
  QCOMPARE(state->firstNativeCode.load(), terminalStatus);
  QCOMPARE(state->firstNativeResult.load(), std::uint32_t{0});
  try {
    std::rethrow_exception(failure);
  } catch (const app::OperationError& error) {
    QCOMPARE(error.writeEffect(), app::WriteEffect::None);
    QCOMPARE(error.failureKind(), static_cast<app::OperationFailureKind>(expectedKind));
    QCOMPARE(dynamic_cast<const app::OverlayFilesQueryUncertain*>(&error) != nullptr, !completed);
    if (completed) {
      QCOMPARE(error.diagnostic().code, terminalStatus);
      QCOMPARE(error.diagnostic().result, std::uint32_t{0});
    }
    QVERIFY(QString::fromUtf8(error.what()).contains(QStringLiteral("controlled terminal GetOverlayFiles")));
  } catch (...) {
    QFAIL("The production reader lost the application failure contract");
  }
  QCOMPARE(state->methodCalls.load(), 1);
  firstSubscription.cancel();
  first.reset();

  QueryReceiver reopened;
  auto reopenedSubscription = manager.subscribe(QStringLiteral("\\\\?\\Volume{11111111-2222-3333-4444-555555555555}\\"), &reopened, reopened.completion());
  QVERIFY(reopened.results.empty());
  QTRY_COMPARE(reopened.results.size(), std::size_t{1});
  if (completed) {
    QVERIFY(std::holds_alternative<app::OverlayFilesQueryManager::Entries>(reopened.results.front()));
    const auto& entries = std::get<app::OverlayFilesQueryManager::Entries>(reopened.results.front());
    QCOMPARE(entries->size(), 1);
    QCOMPARE(entries->constFirst().rawName, QStringLiteral(R"(\Data\fresh.txt)"));
    QCOMPARE(entries->constFirst().fileSize, qulonglong{123});
    QCOMPARE(state->methodCalls.load(), 2);
  } else {
    QVERIFY(std::holds_alternative<std::exception_ptr>(reopened.results.front()));
    QVERIFY(std::get<std::exception_ptr>(reopened.results.front()) == failure);
    QCOMPARE(state->methodCalls.load(), 1);
  }
}

}  // namespace

QTEST_GUILESS_MAIN(OverlayFilesReaderTests)

#include "OverlayFilesReaderTests.moc"
