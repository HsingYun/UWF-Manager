// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
// clang-format off
#include <qt_windows.h>
#include <objbase.h>
// clang-format on

#include <QScopeGuard>
#include <QTest>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include "infrastructure/uwf/windows/execution/MtaUwf.h"
#include "infrastructure/windows/execution/MtaExecutor.h"
#include "infrastructure/windows/management/csp/DmCspTransport.h"

namespace {
using namespace uwf;
using windows::MtaExecutor;
using windows::backends::MtaUwf;
constexpr auto waitLimit = std::chrono::seconds{5};
enum class CallerApartment { Uninitialized, Sta, Mta };
enum class Port { Executor, UwfProbe, UwfExecute, CspAvailable, CspProcess };
enum class FailureStage { Factory, Call };
constexpr std::array callers{std::pair{CallerApartment::Uninitialized, "uninitialized"}, std::pair{CallerApartment::Sta, "sta"},
                             std::pair{CallerApartment::Mta, "mta"}};
constexpr std::array ports{std::pair{Port::Executor, "executor"}, std::pair{Port::UwfProbe, "uwf-probe"}, std::pair{Port::UwfExecute, "uwf-execute"},
                           std::pair{Port::CspAvailable, "csp-available"}, std::pair{Port::CspProcess, "csp-process"}};

struct Apartment {
  HRESULT status;
  APTTYPE type = APTTYPE_CURRENT;
  APTTYPEQUALIFIER qualifier = APTTYPEQUALIFIER_NONE;
};
Apartment apartment() {
  Apartment result{};
  result.status = CoGetApartmentType(&result.type, &result.qualifier);
  return result;
}
struct CallerResult {
  HRESULT initialized = S_OK;
  Apartment before{}, after{};
  DWORD thread = 0;
  std::exception_ptr failure;
};
template <typename Work>
CallerResult callFrom(const CallerApartment requested, Work work) {
  CallerResult result;
  std::jthread caller([&] {
    result.thread = GetCurrentThreadId();
    const bool initialize = requested != CallerApartment::Uninitialized;
    if (initialize) result.initialized = CoInitializeEx(nullptr, requested == CallerApartment::Sta ? COINIT_APARTMENTTHREADED : COINIT_MULTITHREADED);
    const auto cleanup = qScopeGuard([&] {
      if (initialize && SUCCEEDED(result.initialized)) CoUninitialize();
    });
    result.before = apartment();
    try {
      if (FAILED(result.initialized)) throw std::system_error(static_cast<int>(result.initialized), std::system_category(), "initialize test caller COM");
      work();
    } catch (...) {
      result.failure = std::current_exception();
    }
    result.after = apartment();
  });
  caller.join();
  return result;
}

class Gate final {
 public:
  void enter() {
    std::unique_lock lock(m_mutex);
    ++m_active;
    m_changed.notify_all();
    m_changed.wait_for(lock, waitLimit, [&] { return m_released; });
    --m_active;
    m_changed.notify_all();
  }
  bool waitFor(const int count) {
    std::unique_lock lock(m_mutex);
    return m_changed.wait_for(lock, waitLimit, [&] { return m_active >= count; });
  }
  void release() {
    const std::scoped_lock lock(m_mutex);
    m_released = true;
    m_changed.notify_all();
  }

 private:
  std::mutex m_mutex;
  std::condition_variable m_changed;
  int m_active = 0;
  bool m_released = false;
};

struct Event {
  std::string name;
  Apartment com;
  DWORD thread;
  bool ownedWorker;
  bool copiedArguments = true;
};
struct Trace {
  std::mutex mutex;
  std::vector<Event> events;
  std::atomic<const compat::Request*> callerRequest{nullptr};
  std::atomic<const compat::Context*> callerContext{nullptr};
  std::atomic<const QString*> callerXml{nullptr};
  std::atomic<const QStringList*> callerFilter{nullptr};
  std::optional<compat::Request> receivedRequest;
  QString receivedXml;
  QStringList receivedFilter;
  std::stop_token expectedStop;
  std::atomic_bool tokenPreserved{true}, canceled{false};
  std::shared_ptr<Gate> gate;

  void record(std::string name, MtaExecutor* executor, const bool copied = true) {
    Event event{std::move(name), apartment(), GetCurrentThreadId(), executor->isCurrentThread(), copied};
    const std::scoped_lock lock(mutex);
    events.push_back(std::move(event));
  }
  std::vector<Event> snapshot() {
    const std::scoped_lock lock(mutex);
    return events;
  }
};

class MemoryUwf final : public compat::Uwf {
 public:
  MemoryUwf(std::shared_ptr<Trace> trace, MtaExecutor* executor, std::exception_ptr failure = {})
      : m_trace(std::move(trace)), m_executor(executor), m_failure(std::move(failure)) {
    m_trace->record("created", m_executor);
  }
  ~MemoryUwf() override { m_trace->record("destroyed", m_executor); }
  compat::UwfStateSnapshot publication = std::make_shared<const compat::UwfStateFacts>();
  compat::UwfStateSnapshot stateSnapshot() const override { return publication; }
  compat::Capability probe(const compat::Request& request) const override {
    record("probe", request);
    if (m_failure) std::rethrow_exception(m_failure);
    return {compat::Support::Supported, "memory-capability"};
  }
  compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
    record("execute", request, &context != m_trace->callerContext.load());
    m_trace->tokenPreserved = context.stop == m_trace->expectedStop;
    if (m_trace->gate) m_trace->gate->enter();
    if (m_failure) std::rethrow_exception(m_failure);
    m_trace->canceled = context.stop.stop_requested();
    return m_trace->canceled ? compat::Response::fail(compat::ErrorKind::Canceled, "memory-canceled") : compat::Response::success(std::uint32_t{4096});
  }
  void synchronize(const std::function<void()>& work) const override {
    m_trace->record("synchronize", m_executor);
    work();
  }
  compat::Response refreshState(const compat::Context&) const override {
    m_trace->record("refresh", m_executor);
    return compat::Response::success();
  }

 private:
  void record(const char* name, const compat::Request& request, const bool copiedContext = true) const {
    m_trace->record(name, m_executor, &request != m_trace->callerRequest.load() && copiedContext);
    const std::scoped_lock lock(m_trace->mutex);
    m_trace->receivedRequest = request;
  }
  std::shared_ptr<Trace> m_trace;
  MtaExecutor* m_executor;
  std::exception_ptr m_failure;
};
class MemoryTransport final : public windows::DmCspTransport {
 public:
  MemoryTransport(std::shared_ptr<Trace> trace, MtaExecutor* executor, std::exception_ptr failure = {})
      : m_trace(std::move(trace)), m_executor(executor), m_failure(std::move(failure)) {
    m_trace->record("created", m_executor);
  }
  ~MemoryTransport() override { m_trace->record("destroyed", m_executor); }
  bool available() override {
    m_trace->record("available", m_executor);
    if (m_failure) std::rethrow_exception(m_failure);
    return true;
  }
  windows::DmCspReply process(const QString& xml, const QStringList& allowed) override {
    m_trace->record("process", m_executor, &xml != m_trace->callerXml.load() && &allowed != m_trace->callerFilter.load());
    {
      const std::scoped_lock lock(m_trace->mutex);
      m_trace->receivedXml = xml;
      m_trace->receivedFilter = allowed;
    }
    if (m_trace->gate) m_trace->gate->enter();
    if (m_failure) std::rethrow_exception(m_failure);
    return {0x80070005, xml, QStringLiteral("original transport detail"), true};
  }

 private:
  std::shared_ptr<Trace> m_trace;
  MtaExecutor* m_executor;
  std::exception_ptr m_failure;
};

struct PortResult {
  int value = 0;
  bool available = false;
  compat::Response response;
  windows::DmCspReply reply;
};
PortResult usePort(const Port port, const std::shared_ptr<MtaExecutor>& executor, const std::shared_ptr<Trace>& trace,
                   const std::optional<FailureStage> stage = {}, const std::exception_ptr failure = {}, const compat::Context context = {}) {
  PortResult result;
  const auto callFailure = stage == FailureStage::Call ? failure : std::exception_ptr{};
  if (port == Port::Executor) {
    result.value = executor->invoke([&] {
      trace->record("invoke", executor.get());
      if (callFailure) std::rethrow_exception(callFailure);
      return 42;
    });
  } else if (port == Port::UwfProbe || port == Port::UwfExecute) {
    MtaUwf system(
        [&] {
          trace->record("factory", executor.get());
          if (stage == FailureStage::Factory) std::rethrow_exception(failure);
          return std::make_shared<MemoryUwf>(trace, executor.get(), callFailure);
        },
        executor);
    const compat::Request request = compat::GetNumber{compat::NumberProperty::MaximumOverlaySize, compat::Session::Next};
    trace->callerRequest = &request;
    trace->callerContext = &context;
    if (port == Port::UwfProbe)
      result.available = system.probe(request).support == compat::Support::Supported;
    else
      result.response = system.execute(request, context);
  } else {
    auto transport = windows::createMtaDmCspTransport(
        [&] {
          trace->record("factory", executor.get());
          if (stage == FailureStage::Factory) std::rethrow_exception(failure);
          return std::make_shared<MemoryTransport>(trace, executor.get(), callFailure);
        },
        executor);
    const QString xml = QStringLiteral("<memory-request value='owned'/>");
    const QStringList filters{QStringLiteral("UnifiedWriteFilter"), QStringLiteral("memory-filter")};
    trace->callerXml = &xml;
    trace->callerFilter = &filters;
    if (port == Port::CspAvailable)
      result.available = transport->available();
    else
      result.reply = transport->process(xml, filters);
  }
  return result;
}

void checkCaller(const CallerResult& caller, const CallerApartment requested) {
  QVERIFY(SUCCEEDED(caller.initialized));
  if (requested != CallerApartment::Uninitialized) QCOMPARE(caller.before.status, S_OK);
  if (requested == CallerApartment::Sta) QVERIFY(caller.before.type == APTTYPE_STA || caller.before.type == APTTYPE_MAINSTA);
  if (requested == CallerApartment::Mta) QCOMPARE(caller.before.type, APTTYPE_MTA);
  QCOMPARE(caller.after.status, caller.before.status);
  QCOMPARE(caller.after.type, caller.before.type);
  QCOMPARE(caller.after.qualifier, caller.before.qualifier);
}
void checkWorkers(const std::vector<Event>& events, const DWORD callerThread = 0) {
  QVERIFY(!events.empty());
  for (const auto& event : events) {
    QCOMPARE(event.com.status, S_OK);
    QCOMPARE(event.com.type, APTTYPE_MTA);
    QVERIFY(event.ownedWorker);
    QVERIFY(event.copiedArguments);
    if (callerThread) QVERIFY(event.thread != callerThread);
  }
}
std::size_t countEvents(const std::vector<Event>& events, const std::string& name) {
  return static_cast<std::size_t>(std::ranges::count(events, name, &Event::name));
}

class MtaExecutionTests final : public QObject {
  Q_OBJECT
 private slots:
  void stateReadersReceiveTheSameImmutablePublicationWithoutMtaQueueing() {
    const auto executor = std::make_shared<MtaExecutor>();
    const auto trace = std::make_shared<Trace>();
    compat::UwfStateFacts facts;
    facts.current.registryExclusions = compat::Fact<std::vector<std::string>>::known(std::vector<std::string>(2048, "HKLM\\Software\\Entry"));
    const auto published = std::make_shared<const compat::UwfStateFacts>(std::move(facts));
    MtaUwf system(
        [&] {
          auto port = std::make_shared<MemoryUwf>(trace, executor.get());
          port->publication = published;
          return port;
        },
        executor);
    const auto before = trace->snapshot().size();
    for (int index = 0; index < 100; ++index) QCOMPARE(system.stateSnapshot().get(), published.get());
    QCOMPARE(trace->snapshot().size(), before);
    QCOMPARE(published->current.registryExclusions.value->size(), std::size_t{2048});
  }
  void completeRefreshSessionAndNestedCallsStayOnTheMtaWorker() {
    const auto executor = std::make_shared<MtaExecutor>(2);
    const auto trace = std::make_shared<Trace>();
    MtaUwf system([&] { return std::make_shared<MemoryUwf>(trace, executor.get()); }, executor);
    DWORD sessionThread = 0;
    bool nestedSucceeded = false;
    const auto caller = callFrom(CallerApartment::Sta, [&] {
      system.synchronize([&] {
        sessionThread = GetCurrentThreadId();
        const auto read = system.execute(compat::GetNumber{compat::NumberProperty::MaximumOverlaySize});
        nestedSucceeded = read.succeeded() && system.refreshState().succeeded();
      });
    });
    QVERIFY(!caller.failure);
    QVERIFY(nestedSucceeded);
    QVERIFY(sessionThread != caller.thread);
    const auto events = trace->snapshot();
    QCOMPARE(events.size(), std::size_t{4});
    QCOMPARE(events[1].name, std::string{"synchronize"});
    QCOMPARE(events[2].name, std::string{"execute"});
    QCOMPARE(events[3].name, std::string{"refresh"});
    for (std::size_t index = 1; index < events.size(); ++index) {
      QVERIFY(events[index].ownedWorker);
      QCOMPARE(events[index].com.type, APTTYPE_MTA);
      QCOMPARE(events[index].thread, sessionThread);
    }
  }
  void publicPortsPreserveCallerApartmentsAndOwnTheirResources_data() {
    QTest::addColumn<CallerApartment>("caller");
    QTest::addColumn<Port>("port");
    for (const auto& [caller, callerName] : callers)
      for (const auto& [port, portName] : ports) QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(callerName, portName))) << caller << port;
  }
  void publicPortsPreserveCallerApartmentsAndOwnTheirResources() {
    QFETCH(CallerApartment, caller);
    QFETCH(Port, port);
    auto executor = std::make_shared<MtaExecutor>(2);
    auto trace = std::make_shared<Trace>();
    PortResult result;
    const auto observed = callFrom(caller, [&] { result = usePort(port, executor, trace); });
    checkCaller(observed, caller);
    QVERIFY(!observed.failure);
    const auto events = trace->snapshot();
    checkWorkers(events, observed.thread);
    QCOMPARE(events.size(), port == Port::Executor ? std::size_t{1} : std::size_t{4});
    QCOMPARE(countEvents(events, "created"), port == Port::Executor ? std::size_t{0} : std::size_t{1});
    QCOMPARE(countEvents(events, "destroyed"), port == Port::Executor ? std::size_t{0} : std::size_t{1});
    if (port == Port::Executor) QCOMPARE(result.value, 42);
    if (port == Port::UwfProbe || port == Port::CspAvailable) QVERIFY(result.available);
    if (port == Port::UwfProbe || port == Port::UwfExecute) {
      QVERIFY(trace->receivedRequest);
      QVERIFY((*trace->receivedRequest == compat::Request{compat::GetNumber{compat::NumberProperty::MaximumOverlaySize, compat::Session::Next}}));
      QVERIFY(trace->tokenPreserved);
    }
    if (port == Port::UwfExecute) {
      QVERIFY(result.response.succeeded());
      QCOMPARE(std::get<std::uint32_t>(result.response.value), std::uint32_t{4096});
    }
    if (port == Port::CspProcess) {
      QCOMPARE(trace->receivedXml, QStringLiteral("<memory-request value='owned'/>"));
      QCOMPARE(trace->receivedFilter, QStringList({QStringLiteral("UnifiedWriteFilter"), QStringLiteral("memory-filter")}));
      QCOMPARE(result.reply.xml, trace->receivedXml);
      QCOMPARE(result.reply.hresult, std::uint32_t{0x80070005});
      QCOMPARE(result.reply.extendedError, QStringLiteral("original transport detail"));
      QVERIFY(result.reply.invoked);
    }
  }

  void factoryAndCallExceptionsRetainIdentityWithoutReplay_data() {
    QTest::addColumn<CallerApartment>("caller");
    QTest::addColumn<Port>("port");
    QTest::addColumn<FailureStage>("stage");
    for (const auto& [caller, callerName] : callers)
      for (const auto& [port, portName] : ports)
        for (const auto stage : {FailureStage::Factory, FailureStage::Call}) {
          if (stage == FailureStage::Factory && port != Port::UwfExecute && port != Port::CspProcess) continue;
          QTest::newRow(qPrintable(QStringLiteral("%1/%2/%3").arg(callerName, portName, stage == FailureStage::Factory ? "factory" : "call")))
              << caller << port << stage;
        }
  }
  void factoryAndCallExceptionsRetainIdentityWithoutReplay() {
    QFETCH(CallerApartment, caller);
    QFETCH(Port, port);
    QFETCH(FailureStage, stage);
    auto executor = std::make_shared<MtaExecutor>(2);
    auto trace = std::make_shared<Trace>();
    const auto original = std::make_exception_ptr(std::system_error(std::make_error_code(std::errc::permission_denied), "original-memory-failure"));
    const auto observed = callFrom(caller, [&] { static_cast<void>(usePort(port, executor, trace, stage, original)); });
    checkCaller(observed, caller);
    QVERIFY(observed.failure == original);
    const auto events = trace->snapshot();
    checkWorkers(events, observed.thread);
    const bool created = stage == FailureStage::Call && port != Port::Executor;
    QCOMPARE(countEvents(events, "created"), created ? std::size_t{1} : std::size_t{0});
    QCOMPARE(countEvents(events, "destroyed"), created ? std::size_t{1} : std::size_t{0});
    QCOMPARE(events.size(), stage == FailureStage::Factory || port == Port::Executor ? std::size_t{1} : std::size_t{4});
  }

  void canceledRequestsKeepTheirTokenAndAreNotReplayed_data() {
    QTest::addColumn<CallerApartment>("caller");
    QTest::addColumn<bool>("preCanceled");
    for (const auto& [caller, name] : callers)
      for (const bool before : {false, true}) QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(name, before ? "before" : "during"))) << caller << before;
  }
  void canceledRequestsKeepTheirTokenAndAreNotReplayed() {
    QFETCH(CallerApartment, caller);
    QFETCH(bool, preCanceled);
    auto executor = std::make_shared<MtaExecutor>(2);
    auto trace = std::make_shared<Trace>();
    trace->gate = std::make_shared<Gate>();
    const auto release = qScopeGuard([&] { trace->gate->release(); });
    std::stop_source stop;
    trace->expectedStop = stop.get_token();
    if (preCanceled) stop.request_stop();
    PortResult result;
    auto pending = std::async(std::launch::async,
                              [&] { return callFrom(caller, [&] { result = usePort(Port::UwfExecute, executor, trace, {}, {}, {stop.get_token()}); }); });
    const bool entered = trace->gate->waitFor(1);
    stop.request_stop();
    trace->gate->release();
    QVERIFY(entered);
    QCOMPARE(pending.wait_for(waitLimit), std::future_status::ready);
    const auto observed = pending.get();
    checkCaller(observed, caller);
    QVERIFY(!observed.failure);
    QVERIFY(trace->tokenPreserved);
    QVERIFY(trace->canceled);
    QVERIFY(result.response.failure);
    QCOMPARE(result.response.failure->kind, compat::ErrorKind::Canceled);
    QCOMPARE(result.response.effect, compat::Effect::None);
    const auto events = trace->snapshot();
    checkWorkers(events, observed.thread);
    QCOMPARE(countEvents(events, "execute"), std::size_t{1});
  }

  void independentCallsEnterTheSamePoolConcurrently_data() {
    QTest::addColumn<Port>("port");
    for (const auto& [port, name] : ports)
      if (port == Port::Executor || port == Port::UwfExecute || port == Port::CspProcess) QTest::newRow(name) << port;
  }
  void independentCallsEnterTheSamePoolConcurrently() {
    QFETCH(Port, port);
    auto executor = std::make_shared<MtaExecutor>(2);
    auto trace = std::make_shared<Trace>();
    trace->gate = std::make_shared<Gate>();
    const auto release = qScopeGuard([&] { trace->gate->release(); });
    {
      std::unique_ptr<MtaUwf> system;
      std::shared_ptr<windows::DmCspTransport> transport;
      if (port == Port::UwfExecute) system = std::make_unique<MtaUwf>([&] { return std::make_shared<MemoryUwf>(trace, executor.get()); }, executor);
      if (port == Port::CspProcess)
        transport = windows::createMtaDmCspTransport([&] { return std::make_shared<MemoryTransport>(trace, executor.get()); }, executor);
      const auto call = [&](const int value) {
        if (port == Port::Executor)
          return executor->invoke([&] {
            trace->record("invoke", executor.get());
            trace->gate->enter();
            return value;
          });
        if (system) {
          const auto response = system->execute(compat::GetNumber{compat::NumberProperty::MaximumOverlaySize});
          if (!response.succeeded()) throw std::runtime_error("memory UWF call failed");
        } else {
          const auto reply = transport->process(QString::number(value), {QStringLiteral("memory-filter")});
          if (reply.xml != QString::number(value)) throw std::runtime_error("concurrent transport reply changed callers");
        }
        return value;
      };
      auto first = std::async(std::launch::async, [&] { return call(1); });
      auto second = std::async(std::launch::async, [&] { return call(2); });
      const bool overlapped = trace->gate->waitFor(2);
      trace->gate->release();
      QVERIFY(overlapped);
      QCOMPARE(first.wait_for(waitLimit), std::future_status::ready);
      QCOMPARE(second.wait_for(waitLimit), std::future_status::ready);
      QCOMPARE(first.get(), 1);
      QCOMPARE(second.get(), 2);
    }
    const auto events = trace->snapshot();
    checkWorkers(events);
    QCOMPARE(countEvents(events, port == Port::Executor ? "invoke" : port == Port::UwfExecute ? "execute" : "process"), std::size_t{2});
  }

  void workerReentryRunsInline_data() {
    QTest::addColumn<Port>("port");
    for (const auto& [port, name] : ports) QTest::newRow(name) << port;
  }
  void workerReentryRunsInline() {
    QFETCH(Port, port);
    auto executor = std::make_shared<MtaExecutor>(2);
    auto trace = std::make_shared<Trace>();
    auto pending = executor->submit([&] {
      const auto thread = GetCurrentThreadId();
      static_cast<void>(usePort(port, executor, trace));
      return thread;
    });
    QCOMPARE(pending.wait_for(waitLimit), std::future_status::ready);
    const auto thread = pending.get();
    const auto events = trace->snapshot();
    checkWorkers(events);
    for (const auto& event : events) QCOMPARE(event.thread, thread);
  }

  void shutdownDrainsAcceptedWorkAndReleasesCapturesBeforeFutures() {
    auto executor = std::make_unique<MtaExecutor>(2);
    auto* worker = executor.get();
    auto trace = std::make_shared<Trace>();
    auto gate = std::make_shared<Gate>();
    const auto release = qScopeGuard([&] { gate->release(); });
    struct Capture {
      std::shared_ptr<Trace> trace;
      MtaExecutor* executor;
      Capture(std::shared_ptr<Trace> value, MtaExecutor* owner) : trace(std::move(value)), executor(owner) {}
      ~Capture() { trace->record("capture-released", executor); }
    };
    const auto original = std::make_exception_ptr(std::runtime_error("accepted-work-failure"));
    constexpr int accepted = 6;
    std::vector<std::future<int>> pending;
    for (int value = 0; value < accepted; ++value) {
      auto capture = std::make_unique<Capture>(trace, worker);
      pending.push_back(executor->submit([capture = std::move(capture), value, gate, trace, worker, original] {
        trace->record("accepted", worker);
        gate->enter();
        if (value == 1) std::rethrow_exception(original);
        return value;
      }));
    }
    const bool started = gate->waitFor(2);
    auto shutdown = std::async(std::launch::async, [owner = std::move(executor)]() mutable { owner.reset(); });
    gate->release();
    QVERIFY(started);
    QCOMPARE(shutdown.wait_for(waitLimit), std::future_status::ready);
    shutdown.get();
    const auto events = trace->snapshot();
    checkWorkers(events);
    QCOMPARE(countEvents(events, "accepted"), static_cast<std::size_t>(accepted));
    QCOMPARE(countEvents(events, "capture-released"), static_cast<std::size_t>(accepted));
    for (int value = 0; value < accepted; ++value) {
      auto& result = pending[static_cast<std::size_t>(value)];
      QCOMPARE(result.wait_for(waitLimit), std::future_status::ready);
      if (value == 1) {
        std::exception_ptr observed;
        try {
          static_cast<void>(result.get());
        } catch (...) {
          observed = std::current_exception();
        }
        QVERIFY(observed == original);
      } else {
        QCOMPARE(result.get(), value);
      }
    }
  }
};
}  // namespace

QTEST_APPLESS_MAIN(MtaExecutionTests)
#include "MtaExecutionTests.moc"
