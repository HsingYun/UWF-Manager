// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QMap>
#include <QRegularExpression>
#include <QTest>
#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <stop_token>
#include <string_view>

#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/adapters/UwfCall.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "foundation/logging/Log.h"
#include "infrastructure/uwf/routing/FallbackUwf.h"
#include "support/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf::compat;
using uwf::test::FakeUwfBackend;
constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";

struct Invocation {
  std::string backend;
  Request request;
};
using Invocations = std::shared_ptr<std::vector<Invocation>>;

// Protocol-only backend double; stateful scenarios use FakeUwfBackend.
class Endpoint final : public Backend {
 public:
  using Probe = std::function<Capability(const Request&)>;
  using Execute = std::function<Response(const Request&, const Context&)>;
  Endpoint(std::string label, Invocations calls, Execute execute = {}, Probe probe = {})
      : m_name(std::move(label)), m_calls(std::move(calls)), m_execute(std::move(execute)), m_probe(std::move(probe)) {}
  std::string name() const override { return m_name; }
  Capability probe(const Request& request) const override {
    return m_probe ? m_probe(request) : Capability{Support::Supported, "test endpoint supports this request"};
  }
  Response execute(const Request& request, const Context& context) const override {
    m_calls->push_back({m_name, request});
    return m_execute ? m_execute(request, context) : Response::fail(ErrorKind::Unsupported, "test endpoint has no handler");
  }

 private:
  std::string m_name;
  Invocations m_calls;
  Execute m_execute;
  Probe m_probe;
};

QStringList backends(const Invocations& calls, bool writesOnly = false) {
  QStringList result;
  for (const auto& call : *calls)
    if (!writesOnly || isWrite(call.request)) result.push_back(QString::fromStdString(call.backend));
  return result;
}

bool hasDiagnostic(const Response& response, const std::string& backend, std::int64_t code, const std::string& detail) {
  for (const auto& attempt : response.attempts)
    if (attempt.backend == backend && attempt.failure)
      for (const auto& diagnostic : attempt.failure->diagnostics)
        if (diagnostic.code == code && diagnostic.detail.find(detail) != std::string::npos) return true;
  return false;
}

std::size_t writes(const FakeUwfBackend& backend) {
  const auto trace = backend.trace();
  return static_cast<std::size_t>(std::ranges::count_if(trace, [](const auto& entry) { return isWrite(entry.request); }));
}

struct VisibleTarget {
  const char* name;
  Request write;
  Request read;
  Value observed;
};

const auto visibleTargets = std::to_array<VisibleTarget>({
    {"filter-lower-filters", SetBoolean{BooleanProperty::FilterEnabled, true}, GetBoolean{BooleanProperty::FilterEnabled, Session::Next}, true},
    {"horm-bcd", SetBoolean{BooleanProperty::HormEnabled, true}, GetBoolean{BooleanProperty::HormEnabled, Session::Next}, true},
    {"servicing-account", SetBoolean{BooleanProperty::ServicingEnabled, true}, GetBoolean{BooleanProperty::ServicingEnabled, Session::Next}, true},
    {"domain-special-exclusions", SetBoolean{BooleanProperty::PersistDomainSecretKey, true}, GetBoolean{BooleanProperty::PersistDomainSecretKey, Session::Next},
     true},
    {"flags-bcd", SetOverlayOption{OverlayOption::Persistent, true}, GetNumber{NumberProperty::OverlayFlags, Session::Next}, flags::persistent},
    {"warning-runtime", SetNumber{NumberProperty::WarningThreshold, 768}, GetNumber{NumberProperty::WarningThreshold, Session::Current}, std::uint32_t{768}},
    {"critical-runtime", SetNumber{NumberProperty::CriticalThreshold, 896}, GetNumber{NumberProperty::CriticalThreshold, Session::Current}, std::uint32_t{896}},
    {"type-overlay-file", SetNumber{NumberProperty::OverlayType, 1}, GetNumber{NumberProperty::OverlayType, Session::Next}, std::uint32_t{1}},
    {"size-overlay-file", SetNumber{NumberProperty::MaximumOverlaySize, 2048}, GetNumber{NumberProperty::MaximumOverlaySize, Session::Next},
     std::uint32_t{2048}},
    {"binding-index", SetVolume{volumeId, VolumeProperty::BindByDriveLetter, true},
     GetVolumeProperty{volumeId, VolumeProperty::BindByDriveLetter, Session::Next}, true},
    {"reset-saved-selector", SetNumber{NumberProperty::ResetPersistentState, static_cast<std::uint32_t>(ResetMode::Saved)},
     GetNumber{NumberProperty::ResetPersistentState, Session::Next}, std::uint32_t{1}},
});

struct RoutingRecord {
  QChar level;
  QMap<QString, QString> fields;
  QString field(const char* name) const { return fields.value(QString::fromLatin1(name)); }
};

std::vector<RoutingRecord> routingRecords() {
  std::vector<RoutingRecord> result;
  const QString category = QStringLiteral(" uwf-routing] ");
  static const QRegularExpression field(QStringLiteral("(?:^| )([a-z_]+)=(\"(?:\\\\.|[^\"\\\\])*\"|[^ ]+)"));
  for (const auto& raw : uwf::recentLogLines()) {
    const auto line = QString::fromStdString(raw);
    const auto marker = line.indexOf(category);
    if (marker < 1) continue;
    RoutingRecord record{line.at(marker - 1), {}};
    auto matches = field.globalMatch(line.mid(marker + category.size()));
    while (matches.hasNext()) {
      const auto match = matches.next();
      auto value = match.captured(2);
      if (value.startsWith(u'"') && value.endsWith(u'"')) value = value.mid(1, value.size() - 2);
      record.fields.insert(match.captured(1), value);
    }
    result.push_back(std::move(record));
  }
  return result;
}

std::vector<RoutingRecord> matchingRecords(const std::vector<RoutingRecord>& records, const char* field, const QString& value) {
  std::vector<RoutingRecord> result;
  std::ranges::copy_if(records, std::back_inserter(result), [&](const auto& record) { return record.field(field) == value; });
  return result;
}

enum class LoggedRoute { Direct, Unsupported, Unavailable, NoEffect, Exhausted, PreCanceled, BackendCanceled, Confirmed, Uncertain, DataUncertain };

class UwfFallbackTests final : public QObject {
  Q_OBJECT
 private slots:
  void failedWritesPreserveReceiptsWithoutReadingOrReplaying_data() {
    QTest::addColumn<int>("effect");
    QTest::newRow("confirmed") << static_cast<int>(Effect::Confirmed);
    QTest::newRow("uncertain") << static_cast<int>(Effect::Uncertain);
  }
  void failedWritesPreserveReceiptsWithoutReadingOrReplaying() {
    QFETCH(int, effect);
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("first", calls, [effect](const Request&, const Context&) {
      return Response::fail(ErrorKind::Failed, "original setter failure", static_cast<Effect>(effect), {{"first", "Write", 61, "original setter diagnostic"}});
    });
    auto second = std::make_shared<Endpoint>("second", calls);
    const auto response = FallbackUwf({first, second}).execute(SetBoolean{BooleanProperty::FilterEnabled, true});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Failed);
    QCOMPARE(response.effect, static_cast<Effect>(effect));
    QCOMPARE(response.failure->effect, response.effect);
    QCOMPARE(backends(calls), QStringList{"first"});
    QVERIFY(hasDiagnostic(response, "first", 61, "original setter diagnostic"));
  }
  void probeMutationEvidenceStopsRouting_data() {
    QTest::addColumn<int>("effectValue");
    for (const auto effect : {Effect::Confirmed, Effect::Uncertain})
      QTest::newRow(effect == Effect::Confirmed ? "confirmed" : "uncertain") << static_cast<int>(effect);
  }
  void probeMutationEvidenceStopsRouting() {
    QFETCH(int, effectValue);
    const auto effect = static_cast<Effect>(effectValue);
    const auto calls = std::make_shared<std::vector<Invocation>>();
    int probes = 0;
    auto first = std::make_shared<Endpoint>("first", calls, Endpoint::Execute{}, [=](const Request&) -> Capability {
      throw Error({ErrorKind::Failed, effect, "probe receipt", {{"first", "Probe", 42, "retained probe evidence"}}});
    });
    auto second = std::make_shared<Endpoint>("second", calls, Endpoint::Execute{}, [&](const Request&) {
      ++probes;
      return Capability{Support::Supported, {}};
    });
    const FallbackUwf system({first, second});
    const auto response = system.execute(SetBoolean{BooleanProperty::FilterEnabled, true});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Protocol);
    QCOMPARE(response.effect, effect);
    QVERIFY(hasDiagnostic(response, "first", 42, "retained probe evidence"));
    QVERIFY(calls->empty());
    QCOMPARE(probes, 0);
    try {
      (void)system.probe(GetBoolean{BooleanProperty::FilterEnabled});
      QFAIL("A mutating capability observation must not become an ordinary unavailable capability");
    } catch (const Error& error) {
      QCOMPARE(error.failure().kind, ErrorKind::Protocol);
      QCOMPARE(error.failure().effect, effect);
      QCOMPARE(error.failure().diagnostics.front().detail, std::string{"retained probe evidence"});
    }
    QCOMPARE(probes, 0);
  }
  void mutationEvidenceFromAReadNeverBecomesSafeFallback_data() {
    QTest::addColumn<bool>("reportedFailure");
    QTest::addColumn<int>("effectValue");
    for (const bool failure : {false, true})
      for (const auto effect : {Effect::Confirmed, Effect::Uncertain})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2").arg(failure ? "failed-read" : "successful-read").arg(static_cast<int>(effect))))
            << failure << static_cast<int>(effect);
  }
  void mutationEvidenceFromAReadNeverBecomesSafeFallback() {
    QFETCH(bool, reportedFailure);
    QFETCH(int, effectValue);
    const auto effect = static_cast<Effect>(effectValue);
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("first", calls, [=](const Request&, const Context&) {
      return reportedFailure ? Response::fail(ErrorKind::Failed, "original read receipt", effect, {{"first", "Read", 41, "retained native evidence"}})
                             : Response::success(true, effect);
    });
    auto second = std::make_shared<Endpoint>("second", calls, [](const Request&, const Context&) { return Response::success(true); });
    const FallbackUwf system({first, second});
    const auto response = system.execute(GetBoolean{BooleanProperty::FilterEnabled});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Protocol);
    QCOMPARE(response.effect, effect);
    QCOMPARE(response.failure->effect, effect);
    QCOMPARE(backends(calls), QStringList{"first"});
    if (reportedFailure) QVERIFY(hasDiagnostic(response, "first", 41, "retained native evidence"));
  }
  void invalidIdentityStopsBeforeEveryBackendProbe() {
    auto calls = std::make_shared<std::vector<Invocation>>();
    int probes = 0;
    auto backend = std::make_shared<Endpoint>("must-not-run", calls, Endpoint::Execute{}, [&](const Request&) {
      ++probes;
      return Capability{Support::Supported, {}};
    });
    FallbackUwf router({backend});
    const Request request = CommitFile{std::string(volumeId) + '\0' + "other-object", "\\file"};
    QCOMPARE(router.probe(request).support, Support::Unsupported);
    const auto response = router.execute(request);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::InvalidArgument);
    QCOMPARE(response.effect, Effect::None);
    QVERIFY(response.attempts.empty());
    QCOMPARE(probes, 0);
    QVERIFY(calls->empty());
  }
  void unsupportedUnrelatedGettersDoNotForceCommitFallback() {
    const Request request = CommitFile{volumeId, "\\file"};
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>(
        "WMI", calls, [](const Request&, const Context&) { return Response::success(std::monostate{}, Effect::Confirmed); },
        [](const Request& candidate) { return Capability{std::holds_alternative<CommitFile>(candidate) ? Support::Supported : Support::Unsupported, {}}; });
    auto second = std::make_shared<Endpoint>("Native", calls);
    const auto response = FallbackUwf({first, second}).execute(request);
    QVERIFY(response.succeeded());
    QCOMPARE(calls->size(), std::size_t{1});
    QCOMPARE(calls->front().backend, std::string("WMI"));
    QCOMPARE(calls->front().request, request);
  }
  void probeLogsConfiguredOrderAndAggregateOutcome_data() {
    QTest::addColumn<Support>("firstSupport");
    QTest::addColumn<QString>("firstResult");
    QTest::addColumn<bool>("secondSupports");
    for (const auto& [support, name] : std::to_array<std::pair<Support, const char*>>(
             {{Support::Supported, "Supported"}, {Support::Unsupported, "Unsupported"}, {Support::Unavailable, "Unavailable"}, {Support::Unknown, "Unknown"}}))
      for (const bool second : {false, true}) {
        if (support == Support::Supported && !second) continue;
        QTest::newRow(qPrintable(QStringLiteral("first-%1/second-supported-%2").arg(QString::fromLatin1(name)).arg(second)))
            << support << QString::fromLatin1(name) << second;
      }
  }
  void probeLogsConfiguredOrderAndAggregateOutcome() {
    QFETCH(Support, firstSupport);
    QFETCH(bool, secondSupports);
    uwf::clearLogLines();
    const auto calls = std::make_shared<std::vector<Invocation>>();
    QStringList probes;
    const std::array rawNames{"Native", "future-cli", "WMI"};
    const QStringList labels{"Native", "future-cli", "WMI"};
    const std::array supports{firstSupport, secondSupports ? Support::Supported : Support::Unsupported, Support::Unsupported};
    std::vector<std::shared_ptr<Backend>> chain;
    for (std::size_t index = 0; index < rawNames.size(); ++index)
      chain.push_back(std::make_shared<Endpoint>(rawNames[index], calls, Endpoint::Execute{}, [&, index](const Request&) {
        probes.append(QString::fromLatin1(rawNames[index]));
        return Capability{supports[index], "probe-reason-" + std::string(rawNames[index])};
      }));
    const FallbackUwf router(std::move(chain));
    const auto response = router.probe(GetBoolean{BooleanProperty::FilterEnabled, Session::Current});
    const bool supported = firstSupport == Support::Supported || secondSupports;
    const int attempted = firstSupport == Support::Supported ? 1 : secondSupports ? 2 : 3;
    QCOMPARE(response.support, supported ? Support::Supported : firstSupport);
    QVERIFY(calls->empty());
    QCOMPARE(probes.size(), attempted);
    for (int index = 0; index < attempted; ++index) QCOMPARE(probes[index], QString::fromLatin1(rawNames[static_cast<std::size_t>(index)]));
    const auto logs = routingRecords();
    QVERIFY(!logs.empty());
    const auto policy = matchingRecords(logs, "event", QStringLiteral("policy"));
    QCOMPARE(policy.size(), std::size_t{1});
    QCOMPARE(policy.front().level, QChar(u'I'));
    QCOMPARE(policy.front().field("policy"), QStringLiteral("ordered-fallback"));
    QVERIFY(!policy.front().field("router").isEmpty());
    QCOMPARE(policy.front().field("order"), labels.join(QStringLiteral(" -> ")));
#if !defined(UWF_DEBUG_LOGGING)
    QCOMPARE(logs.size(), std::size_t{1});
#else
    QFETCH(QString, firstResult);
    const auto starts = matchingRecords(logs, "event", QStringLiteral("started"));
    QCOMPARE(starts.size(), std::size_t{1});
    const auto requestId = starts.front().field("request");
    QVERIFY(!requestId.isEmpty() && requestId != QStringLiteral("0"));
    const auto requestLogs = matchingRecords(logs, "request", requestId);
    QCOMPARE(requestLogs.front().field("event"), QStringLiteral("started"));
    QCOMPARE(requestLogs.back().field("event"), QStringLiteral("completed"));
    for (const auto& record : requestLogs) {
      QCOMPARE(record.level, QChar(u'D'));
      QCOMPARE(record.field("router"), policy.front().field("router"));
      QCOMPARE(record.field("parent"), QStringLiteral("0"));
      QCOMPARE(record.field("phase"), QStringLiteral("probe"));
      QCOMPARE(record.field("operation"), QStringLiteral("GetFilterEnabled"));
      QCOMPARE(record.field("session"), QStringLiteral("Current"));
    }
    const auto attempts = matchingRecords(requestLogs, "event", QStringLiteral("attempt"));
    QCOMPARE(attempts.size(), static_cast<std::size_t>(attempted * 2));
    for (int index = 0; index < attempted; ++index) {
      const auto& started = attempts[static_cast<std::size_t>(index * 2)];
      const auto& completed = attempts[static_cast<std::size_t>(index * 2 + 1)];
      QCOMPARE(started.field("result"), QStringLiteral("started"));
      for (const auto* record : {&started, &completed}) {
        QCOMPARE(record->field("attempt"), QString::number(index + 1));
        QCOMPARE(record->field("backend"), labels[index]);
        QCOMPARE(record->field("action"), QStringLiteral("probe"));
      }
      const auto expected = index == 0 ? firstResult : index == 1 && secondSupports ? QStringLiteral("Supported") : QStringLiteral("Unsupported");
      QCOMPARE(completed.field("result"), expected);
      QCOMPARE(completed.field("capability"), expected);
      QCOMPARE(completed.field("capability_reason"), QStringLiteral("probe-reason-") + QString::fromLatin1(rawNames[static_cast<std::size_t>(index)]));
    }
    const auto fallbacks = matchingRecords(requestLogs, "event", QStringLiteral("fallback"));
    QCOMPARE(fallbacks.size(), static_cast<std::size_t>(attempted - 1));
    for (std::size_t index = 0; index < fallbacks.size(); ++index) {
      QCOMPARE(fallbacks[index].field("from"), labels[static_cast<int>(index)]);
      QCOMPARE(fallbacks[index].field("to"), labels[static_cast<int>(index + 1)]);
      QVERIFY(!fallbacks[index].field("cause").isEmpty());
      QCOMPARE(fallbacks[index].field("reason"),
               index == 0 && firstSupport == Support::Unknown ? QStringLiteral("probe-inconclusive") : QStringLiteral("capability-rejected"));
    }
    const auto& completed = requestLogs.back();
    QCOMPARE(completed.field("result"), supported ? QStringLiteral("Supported") : firstResult);
    QCOMPARE(completed.field("backend"), supported ? labels[attempted - 1] : QStringLiteral("none"));
    QCOMPARE(completed.field("stop"), supported ? QStringLiteral("supported") : QStringLiteral("exhausted"));
#endif
  }

  void executionLogsRoutingDecisionsAndReadbackParentage_data() {
    QTest::addColumn<LoggedRoute>("route");
    for (const auto& [route, name] :
         std::to_array<std::pair<LoggedRoute, const char*>>({{LoggedRoute::Direct, "direct-success"},
                                                             {LoggedRoute::Unsupported, "unsupported-fallback"},
                                                             {LoggedRoute::Unavailable, "unavailable-fallback"},
                                                             {LoggedRoute::NoEffect, "no-effect-fallback"},
                                                             {LoggedRoute::Exhausted, "exhausted"},
                                                             {LoggedRoute::PreCanceled, "pre-canceled"},
                                                             {LoggedRoute::BackendCanceled, "backend-canceled"},
                                                             {LoggedRoute::Confirmed, "confirmed-write-stop"},
                                                             {LoggedRoute::Uncertain, "uncertain-write-stop"},
                                                             {LoggedRoute::DataUncertain, "uncertain-data-write-without-readback"}}))
      QTest::newRow(name) << route;
  }
  void executionLogsRoutingDecisionsAndReadbackParentage() {
    QFETCH(LoggedRoute, route);
    uwf::clearLogLines();
    const bool writeEffect = route == LoggedRoute::Confirmed || route == LoggedRoute::Uncertain || route == LoggedRoute::DataUncertain;
    const bool readback = false;
    const bool writing = writeEffect || route == LoggedRoute::NoEffect;
    const auto calls = std::make_shared<std::vector<Invocation>>();
    const auto probes = std::make_shared<std::vector<Invocation>>();
    const std::array rawNames{"WMI", "CSP", "Native"};
    const QStringList labels{"WMI", "CSP", "Native"};
    std::vector<std::shared_ptr<Backend>> chain;
    for (std::size_t index = 0; index < rawNames.size(); ++index)
      chain.push_back(std::make_shared<Endpoint>(
          rawNames[index], calls,
          [&, index](const Request& request, const Context&) {
            if (route == LoggedRoute::Exhausted) return Response::fail(ErrorKind::Failed, "read-failed-" + std::string(rawNames[index]));
            if (index == 0) {
              if (route == LoggedRoute::BackendCanceled) return Response::fail(ErrorKind::Canceled, "backend-canceled");
              if (route == LoggedRoute::NoEffect)
                return Response::fail(ErrorKind::AccessDenied, "before-dispatch", Effect::None,
                                      {{"WMI", "SetFilterEnabled", 0x80070005, "original provider detail"}});
              if (writeEffect && isWrite(request))
                return Response::fail(ErrorKind::Failed, "write-receipt-lost", route == LoggedRoute::Confirmed ? Effect::Confirmed : Effect::Uncertain);
            }
            return isWrite(request) ? Response::success(std::monostate{}, Effect::Confirmed) : Response::success(true);
          },
          [&, index](const Request& request) {
            probes->push_back({rawNames[index], request});
            if (index == 0) {
              if (readback && !isWrite(request)) return Capability{Support::Unavailable, "readback-owned-by-CSP"};
              if (route == LoggedRoute::Unsupported) return Capability{Support::Unsupported, "entry-not-provided"};
              if (route == LoggedRoute::Unavailable) return Capability{Support::Unavailable, "transport-unavailable"};
            }
            return Capability{Support::Supported, "probe-ready"};
          }));
    const FallbackUwf router(std::move(chain));
    std::stop_source stop;
    if (route == LoggedRoute::PreCanceled) stop.request_stop();
    const Request request = route == LoggedRoute::DataUncertain ? Request{CommitFile{volumeId, "\\file"}}
                            : writing                           ? Request{SetBoolean{BooleanProperty::FilterEnabled, true}}
                                                                : Request{GetBoolean{BooleanProperty::FilterEnabled, Session::Current}};
    const auto response = router.execute(request, {stop.get_token()});
    const bool skipped = route == LoggedRoute::Unsupported || route == LoggedRoute::Unavailable;
    const bool success = route == LoggedRoute::Direct || skipped || route == LoggedRoute::NoEffect;
    QCOMPARE(response.succeeded(), success);
    QCOMPARE(response.effect, writeEffect ? (route == LoggedRoute::Confirmed ? Effect::Confirmed : Effect::Uncertain)
                              : writing   ? Effect::Confirmed
                                          : Effect::None);
    const QStringList expectedExecutions = route == LoggedRoute::PreCanceled            ? QStringList{}
                                           : route == LoggedRoute::Exhausted            ? QStringList{"WMI", "CSP", "Native"}
                                           : skipped                                    ? QStringList{"CSP"}
                                           : readback || route == LoggedRoute::NoEffect ? QStringList{"WMI", "CSP"}
                                                                                        : QStringList{"WMI"};
    QCOMPARE(backends(calls), expectedExecutions);
    const QStringList expectedProbes = route == LoggedRoute::PreCanceled           ? QStringList{}
                                       : route == LoggedRoute::Exhausted           ? QStringList{"WMI", "CSP", "Native"}
                                       : readback                                  ? QStringList{"WMI", "WMI", "CSP"}
                                       : skipped || route == LoggedRoute::NoEffect ? QStringList{"WMI", "CSP"}
                                                                                   : QStringList{"WMI"};
    QCOMPARE(backends(probes), expectedProbes);
    QCOMPARE(backends(calls, true), writing ? (route == LoggedRoute::NoEffect ? QStringList{"WMI", "CSP"} : QStringList{"WMI"}) : QStringList{});
    const auto logs = routingRecords();
    QVERIFY(!logs.empty());
    for (const auto& record : logs) {
      const auto event = record.field("event");
#if defined(UWF_DEBUG_LOGGING)
      const bool info = event == QStringLiteral("policy") || event == QStringLiteral("fallback") || event == QStringLiteral("completed");
      QCOMPARE(record.level, info ? QChar(u'I') : QChar(u'D'));
#else
      QCOMPARE(record.level, QChar(u'I'));
      QVERIFY(event == QStringLiteral("policy") || event == QStringLiteral("fallback") || event == QStringLiteral("completed"));
#endif
    }
    const auto policies = matchingRecords(logs, "event", QStringLiteral("policy"));
    QCOMPARE(policies.size(), std::size_t{1});
    QCOMPARE(policies.front().field("policy"), QStringLiteral("ordered-fallback"));
    QVERIFY(!policies.front().field("router").isEmpty());
    QCOMPARE(policies.front().field("order"), labels.join(QStringLiteral(" -> ")));
    const auto completions = matchingRecords(logs, "event", QStringLiteral("completed"));
    QCOMPARE(completions.size(), readback ? std::size_t{2} : std::size_t{1});
    const auto requestId = completions.back().field("request");
    QVERIFY(!requestId.isEmpty() && requestId != QStringLiteral("0"));
    const auto rootLogs = matchingRecords(logs, "request", requestId);
    for (const auto& record : logs) {
      if (record.field("event") == QStringLiteral("policy")) continue;
      QCOMPARE(record.field("router"), policies.front().field("router"));
      QCOMPARE(record.field("phase"), QStringLiteral("execute"));
      const bool root = record.field("request") == requestId;
      QCOMPARE(record.field("parent"), root ? QStringLiteral("0") : requestId);
      QCOMPARE(record.field("operation"), root && route == LoggedRoute::DataUncertain ? QStringLiteral("CommitFile")
                                          : root && writing                           ? QStringLiteral("SetFilterEnabled")
                                                                                      : QStringLiteral("GetFilterEnabled"));
      QCOMPARE(record.field("session"), root ? (writing ? QStringLiteral("implicit") : QStringLiteral("Current")) : QStringLiteral("Next"));
    }
#if defined(UWF_DEBUG_LOGGING)
    const auto starts = matchingRecords(logs, "event", QStringLiteral("started"));
    QCOMPARE(starts.size(), readback ? std::size_t{2} : std::size_t{1});
    QCOMPARE(starts.front().field("request"), requestId);
    const auto attempts = matchingRecords(logs, "event", QStringLiteral("attempt"));
    const auto probeAttempts = matchingRecords(attempts, "action", QStringLiteral("probe"));
    const auto executionAttempts = matchingRecords(attempts, "action", QStringLiteral("execute"));
    QCOMPARE(probeAttempts.size(), probes->size() * 2);
    QCOMPARE(executionAttempts.size(), calls->size() * 2);
    for (std::size_t index = 0; index < probes->size(); ++index) {
      const auto& call = probes->at(index);
      const auto label = QString::fromStdString(call.backend);
      const auto& started = probeAttempts[index * 2];
      const auto& finished = probeAttempts[index * 2 + 1];
      for (const auto* record : {&started, &finished}) {
        QCOMPARE(record->field("backend"), label);
        QCOMPARE(record->field("attempt"), QString::number(labels.indexOf(label) + 1));
      }
      QCOMPARE(started.field("result"), QStringLiteral("started"));
      const bool skippedReadback = readback && call.backend == "WMI" && !isWrite(call.request);
      const auto capability = skippedReadback || (route == LoggedRoute::Unavailable && call.backend == "WMI") ? QStringLiteral("Unavailable")
                              : route == LoggedRoute::Unsupported && call.backend == "WMI"                    ? QStringLiteral("Unsupported")
                                                                                                              : QStringLiteral("Supported");
      QCOMPARE(finished.field("result"), capability);
      QCOMPARE(finished.field("capability"), capability);
      QVERIFY(!finished.field("capability_reason").isEmpty());
    }
    for (std::size_t index = 0; index < calls->size(); ++index) {
      const auto& call = calls->at(index);
      const auto label = QString::fromStdString(call.backend);
      const auto& started = executionAttempts[index * 2];
      const auto& finished = executionAttempts[index * 2 + 1];
      for (const auto* record : {&started, &finished}) {
        QCOMPARE(record->field("backend"), label);
        QCOMPARE(record->field("attempt"), QString::number(labels.indexOf(label) + 1));
        QCOMPARE(record->field("capability"), QStringLiteral("Supported"));
      }
      QCOMPARE(started.field("result"), QStringLiteral("started"));
      const bool failed = route == LoggedRoute::Exhausted || route == LoggedRoute::BackendCanceled ||
                          (call.backend == "WMI" && (route == LoggedRoute::NoEffect || (writeEffect && isWrite(call.request))));
      QCOMPARE(finished.field("result"), failed ? QStringLiteral("failed") : QStringLiteral("success"));
      if (failed) {
        QCOMPARE(finished.field("kind"), route == LoggedRoute::BackendCanceled ? QStringLiteral("Canceled")
                                         : route == LoggedRoute::NoEffect      ? QStringLiteral("AccessDenied")
                                                                               : QStringLiteral("Failed"));
        QVERIFY(!finished.field("reason").isEmpty());
      }
    }
    QCOMPARE(rootLogs.front().field("event"), QStringLiteral("started"));
#else
    QVERIFY(matchingRecords(logs, "event", QStringLiteral("started")).empty());
    QVERIFY(matchingRecords(logs, "event", QStringLiteral("attempt")).empty());
    QVERIFY(matchingRecords(logs, "event", QStringLiteral("reconcile")).empty());
#endif
    QCOMPARE(rootLogs.back().field("event"), QStringLiteral("completed"));
    const auto& completed = rootLogs.back();
    QCOMPARE(completed.field("result"), success ? QStringLiteral("success") : QStringLiteral("failed"));
    const auto selected = route == LoggedRoute::PreCanceled || route == LoggedRoute::Exhausted ? QStringLiteral("none")
                          : skipped || route == LoggedRoute::NoEffect                          ? QStringLiteral("CSP")
                                                                                               : QStringLiteral("WMI");
    QCOMPARE(completed.field("backend"), selected);
    QCOMPARE(completed.field("executed_backend"), route == LoggedRoute::Exhausted ? QStringLiteral("Native") : selected);
    QCOMPARE(completed.field("stop"), success                           ? QStringLiteral("success")
                                      : writeEffect                     ? QStringLiteral("write-effect")
                                      : route == LoggedRoute::Exhausted ? QStringLiteral("exhausted")
                                                                        : QStringLiteral("canceled"));
    QCOMPARE(completed.field("effect"), writeEffect ? (route == LoggedRoute::Confirmed ? QStringLiteral("Confirmed") : QStringLiteral("Uncertain"))
                                        : writing   ? QStringLiteral("Confirmed")
                                                    : QStringLiteral("None"));
    if (!success) {
      QVERIFY(response.failure);
      QCOMPARE(completed.field("kind"),
               route == LoggedRoute::PreCanceled || route == LoggedRoute::BackendCanceled ? QStringLiteral("Canceled") : QStringLiteral("Failed"));
      QCOMPARE(completed.field("reason"), QString::fromStdString(response.failure->message));
    }
    const auto rootFallbacks = matchingRecords(rootLogs, "event", QStringLiteral("fallback"));
    QCOMPARE(rootFallbacks.size(), skipped || route == LoggedRoute::NoEffect ? std::size_t{1}
                                   : route == LoggedRoute::Exhausted         ? std::size_t{2}
                                                                             : std::size_t{0});
    for (std::size_t index = 0; index < rootFallbacks.size(); ++index) {
      QCOMPARE(rootFallbacks[index].field("attempt"), QString::number(index + 1));
      QCOMPARE(rootFallbacks[index].field("from"), labels[static_cast<int>(index)]);
      QCOMPARE(rootFallbacks[index].field("to"), labels[static_cast<int>(index + 1)]);
      QVERIFY(!rootFallbacks[index].field("cause").isEmpty());
      QCOMPARE(rootFallbacks[index].field("reason"), skipped ? QStringLiteral("capability-rejected") : QStringLiteral("execution-failed"));
      if (route == LoggedRoute::NoEffect) {
        QCOMPARE(rootFallbacks[index].level, QChar(u'I'));
        QCOMPARE(rootFallbacks[index].field("kind"), QStringLiteral("AccessDenied"));
        QCOMPARE(rootFallbacks[index].field("effect"), QStringLiteral("None"));
        QCOMPARE(rootFallbacks[index].field("diagnostic_backend"), QStringLiteral("WMI"));
        QCOMPARE(rootFallbacks[index].field("diagnostic_operation"), QStringLiteral("SetFilterEnabled"));
        QCOMPARE(rootFallbacks[index].field("code"), QStringLiteral("0x80070005"));
        QCOMPARE(rootFallbacks[index].field("detail"), QStringLiteral("original provider detail"));
      }
    }
#if defined(UWF_DEBUG_LOGGING)
    if (writeEffect) {
      const auto reconciliations = matchingRecords(rootLogs, "event", QStringLiteral("reconcile"));
      QCOMPARE(reconciliations.size(), std::size_t{0});
    }
#endif
    if (readback) {
      const auto childId = completions.front().field("request");
      QVERIFY(!childId.isEmpty() && childId != requestId && childId != QStringLiteral("0"));
      const auto childLogs = matchingRecords(logs, "request", childId);
      QCOMPARE(childLogs.back().field("event"), QStringLiteral("completed"));
      QCOMPARE(childLogs.back().field("result"), QStringLiteral("success"));
      QCOMPARE(childLogs.back().field("backend"), QStringLiteral("CSP"));
      QCOMPARE(completions.front().field("request"), childId);
      QCOMPARE(completions.back().field("request"), requestId);
      const auto childFallbacks = matchingRecords(childLogs, "event", QStringLiteral("fallback"));
      QCOMPARE(childFallbacks.size(), std::size_t{1});
      QCOMPARE(childFallbacks.front().field("from"), QStringLiteral("WMI"));
      QCOMPARE(childFallbacks.front().field("to"), QStringLiteral("CSP"));
      QVERIFY(!childFallbacks.front().field("cause").isEmpty());
      QCOMPARE(childFallbacks.front().field("reason"), QStringLiteral("capability-rejected"));
    }
  }

  void successfulWritesDoNotReadConfirmation_data() {
    QTest::addColumn<int>("setting");
    QTest::newRow("overlay-type") << 0;
    QTest::newRow("maximum-size") << 1;
    QTest::newRow("warning-threshold") << 2;
    QTest::newRow("registry-persistence") << 3;
    QTest::newRow("servicing-transition") << 4;
    QTest::newRow("saved-reset-policy") << 5;
  }
  void successfulWritesDoNotReadConfirmation() {
    QFETCH(int, setting);
    struct Setting {
      Request write;
      Request observe;
      Value expected;
    };
    const auto settings = std::to_array<Setting>({
        {SetNumber{NumberProperty::OverlayType, static_cast<std::uint32_t>(OverlayType::Disk)}, GetNumber{NumberProperty::OverlayType, Session::Next},
         static_cast<std::uint32_t>(OverlayType::Disk)},
        {SetNumber{NumberProperty::MaximumOverlaySize, 2048}, GetNumber{NumberProperty::MaximumOverlaySize, Session::Next}, std::uint32_t{2048}},
        {SetNumber{NumberProperty::WarningThreshold, 640}, GetNumber{NumberProperty::WarningThreshold, Session::Current}, std::uint32_t{640}},
        {SetBoolean{BooleanProperty::PersistTscal, true}, GetBoolean{BooleanProperty::PersistTscal, Session::Next}, true},
        {SetBoolean{BooleanProperty::ServicingEnabled, true}, GetBoolean{BooleanProperty::ServicingEnabled, Session::Next}, true},
        {SetNumber{NumberProperty::ResetPersistentStateSavedMode, static_cast<std::uint32_t>(ResetMode::Clear)},
         GetNumber{NumberProperty::ResetPersistentStateSavedMode, Session::Next}, static_cast<std::uint32_t>(ResetMode::Clear)},
    });
    const auto& scenario = settings.at(static_cast<std::size_t>(setting));
    FakeUwfBackend::State initial;
    initial.next.filterEnabled = true;
    const auto system = FakeUwfBackend::createSystem(initial);
    auto first = std::make_shared<FakeUwfBackend>(system, "first");
    auto second = std::make_shared<FakeUwfBackend>(system, "second");
    constexpr std::int64_t confirmationError = 408;
    first->inject({.request = scenario.observe,
                   .operation = {},
                   .timing = FakeUwfBackend::FaultTiming::Before,
                   .kind = ErrorKind::Unavailable,
                   .message = "confirmation read failed after the setting was applied",
                   .code = confirmationError});
    const auto result = FallbackUwf({first, second}).execute(scenario.write);
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, Effect::Confirmed);
    QCOMPARE(writes(*first), std::size_t{1});
    QVERIFY(second->trace().empty());
    const auto failedRead = first->execute(scenario.observe);
    QVERIFY(!failedRead.succeeded());
    QCOMPARE(failedRead.failure->kind, ErrorKind::Unavailable);
    const auto observed = first->execute(scenario.observe);
    QVERIFY(observed.succeeded());
    QVERIFY(observed.value == scenario.expected);
    if (setting == 0) QVERIFY((first->snapshot().next.overlayFlags & flags::passthrough) != 0);
    if (setting == 4) QVERIFY(!first->snapshot().next.filterEnabled);
  }

  void anUncertainSuccessIsAFailureAndNeverReplayed() {
    for (const bool matches : {false, true}) {
      const auto calls = std::make_shared<std::vector<Invocation>>();
      auto first = std::make_shared<Endpoint>("WMI", calls, [matches](const Request& request, const Context&) {
        if (isWrite(request)) return Response::success(std::monostate{}, Effect::Uncertain);
        return Response::success(matches);
      });
      auto fallback =
          std::make_shared<Endpoint>("CSP", calls, [](const Request&, const Context&) { return Response::success(std::monostate{}, Effect::Confirmed); });
      const SetBoolean request{BooleanProperty::FilterEnabled, true};
      const auto response = FallbackUwf({first, fallback}).execute(request);
      QVERIFY(!response.succeeded());
      QCOMPARE(response.effect, Effect::Uncertain);
      QCOMPARE(backends(calls, true), QStringList({"WMI"}));
      QCOMPARE(calls->size(), std::size_t{1});
      QVERIFY(calls->back().request == Request{request});
      try {
        (void)call(*first, request);
        QFAIL("Direct typed calls must also reject uncertain success");
      } catch (const Error& error) {
        QCOMPARE(error.failure().effect, Effect::Uncertain);
      }
      try {
        (void)uwf::app::callUwfWrite(*first, request);
        QFAIL("Application writes must not accept an uncertain receipt");
      } catch (const uwf::app::OperationError& error) {
        QCOMPARE(error.writeEffect(), uwf::app::WriteEffect::Uncertain);
      }
    }
  }

  void applicationFailureKindsDoNotInferMeaningFromOpaqueNativeCodes() {
    using uwf::app::OperationFailureKind;
    const auto kinds = std::to_array<std::pair<ErrorKind, OperationFailureKind>>({{ErrorKind::Unsupported, OperationFailureKind::Unsupported},
                                                                                  {ErrorKind::Unavailable, OperationFailureKind::Unavailable},
                                                                                  {ErrorKind::AccessDenied, OperationFailureKind::AccessDenied},
                                                                                  {ErrorKind::NotFound, OperationFailureKind::NotFound},
                                                                                  {ErrorKind::InvalidArgument, OperationFailureKind::InvalidArgument},
                                                                                  {ErrorKind::Canceled, OperationFailureKind::Canceled},
                                                                                  {ErrorKind::ResourceExhausted, OperationFailureKind::ResourceExhausted},
                                                                                  {ErrorKind::Conflict, OperationFailureKind::Conflict},
                                                                                  {ErrorKind::Protocol, OperationFailureKind::Protocol},
                                                                                  {ErrorKind::Failed, OperationFailureKind::Failed}});
    for (const auto& [kind, expected] : kinds) {
      try {
        uwf::app::throwUwfOperationError(Error({kind, Effect::Uncertain, "original failure", {{"test-backend", "write", 0x80010105, "opaque native detail"}}}));
      } catch (const uwf::app::OperationError& error) {
        QCOMPARE(error.failureKind(), expected);
        QCOMPARE(error.writeEffect(), uwf::app::WriteEffect::Uncertain);
        QVERIFY(QString::fromUtf8(error.what()).contains(u"opaque native detail"));
        QCOMPARE(static_cast<std::uint32_t>(error.diagnostic().code), std::uint32_t{0x80010105});
      }
    }
  }

  void knownNoEffectFailuresFollowConfiguredPriorityAndPreserveDiagnostics() {
    const auto calls = std::make_shared<std::vector<Invocation>>();
    std::vector<std::shared_ptr<Backend>> chain;
    for (const auto name : {"WMI", "CSP", "Native"}) {
      chain.push_back(std::make_shared<Endpoint>(name, calls, [name](const Request& request, const Context&) {
        if (std::string_view(name) == "Native") return Response::success(std::monostate{}, Effect::Confirmed);
        return Response::fail(ErrorKind::AccessDenied, std::string(name) + " rejected before execution", Effect::None,
                              {{name, operationName(request), std::string_view(name) == "WMI" ? 101 : 102, std::string(name) + " condition detail"}});
      }));
    }
    const auto result = FallbackUwf(std::move(chain)).execute(SetBoolean{BooleanProperty::FilterEnabled, true});
    QVERIFY(result.succeeded());
    QCOMPARE(backends(calls), QStringList({"WMI", "CSP", "Native"}));
    QCOMPARE(result.attempts.size(), std::size_t{3});
    QVERIFY(hasDiagnostic(result, "WMI", 101, "WMI condition detail"));
    QVERIFY(hasDiagnostic(result, "CSP", 102, "CSP condition detail"));
    QCOMPARE(result.effect, Effect::Confirmed);
  }

  void capabilityFailuresSkipExecution_data() {
    QTest::addColumn<int>("failure");
    QTest::newRow("unsupported") << 0;
    QTest::newRow("unavailable") << 1;
    QTest::newRow("probe-exception") << 2;
  }
  void capabilityFailuresSkipExecution() {
    QFETCH(int, failure);
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("WMI", calls, Endpoint::Execute{}, [failure](const Request&) -> Capability {
      if (failure == 2) throw std::runtime_error("probe metadata failed");
      return {failure == 0 ? Support::Unsupported : Support::Unavailable, "required entry is absent"};
    });
    auto second = std::make_shared<Endpoint>("CSP", calls, [](const Request&, const Context&) { return Response::success(true); });
    FallbackUwf router({first, second});
    QCOMPARE(router.probe(GetBoolean{BooleanProperty::FilterEnabled}).support, Support::Supported);
    QVERIFY(calls->empty());
    const auto result = router.execute(GetBoolean{BooleanProperty::FilterEnabled});
    QVERIFY(result.succeeded());
    QVERIFY(std::get<bool>(result.value));
    QCOMPARE(backends(calls), QStringList{"CSP"});
    QCOMPARE(result.attempts.size(), std::size_t{2});
    QCOMPARE(result.attempts.front().backend, std::string("WMI"));
    QVERIFY(!result.attempts.front().capability.reason.empty());
  }

  void explicitNoEffectExceptionsCanFallBack() {
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("WMI", calls, [](const Request& request, const Context&) -> Response {
      throw Error({ErrorKind::Unsupported, Effect::None, "method rejected before invocation", {{"WMI", operationName(request), 201, "missing method"}}});
    });
    auto second =
        std::make_shared<Endpoint>("CSP", calls, [](const Request&, const Context&) { return Response::success(std::monostate{}, Effect::Confirmed); });
    const auto result = FallbackUwf({first, second}).execute(SetBoolean{BooleanProperty::FilterEnabled, true});
    QVERIFY(result.succeeded());
    QCOMPARE(backends(calls, true), QStringList({"WMI", "CSP"}));
    QVERIFY(hasDiagnostic(result, "WMI", 201, "missing method"));
  }

  void readFailuresAndMalformedResultsFallBack_data() {
    QTest::addColumn<int>("failure");
    QTest::newRow("provider-failure") << 0;
    QTest::newRow("standard-exception") << 1;
    QTest::newRow("wrong-result-type") << 2;
  }
  void readFailuresAndMalformedResultsFallBack() {
    QFETCH(int, failure);
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("WMI", calls, [failure](const Request&, const Context&) {
      if (failure == 1) throw std::runtime_error("read transport failed");
      if (failure == 2) return Response::success(false);  // GetNumber requires uint32, never bool.
      return Response::fail(ErrorKind::Failed, "provider read failed");
    });
    auto second = std::make_shared<Endpoint>("CSP", calls, [](const Request&, const Context&) { return Response::success(std::uint32_t{4096}); });
    const auto result = FallbackUwf({first, second}).execute(GetNumber{NumberProperty::MaximumOverlaySize});
    QVERIFY(result.succeeded());
    QCOMPARE(std::get<std::uint32_t>(result.value), std::uint32_t{4096});
    QCOMPARE(backends(calls), QStringList({"WMI", "CSP"}));
    QVERIFY(result.attempts.front().failure.has_value());
    QCOMPARE(result.attempts.front().failure->effect, Effect::None);
    if (failure == 2) QCOMPARE(result.attempts.front().failure->kind, ErrorKind::Protocol);
  }

  void afterEffectFailureRemainsUnknownAfterAnotherBackendObservesTheTarget() {
    const auto shared = FakeUwfBackend::createSystem();
    auto wmi = std::make_shared<FakeUwfBackend>(shared, "WMI");
    auto csp = std::make_shared<FakeUwfBackend>(shared, "CSP");
    auto native = std::make_shared<FakeUwfBackend>(shared, "Native");
    const Request write = SetBoolean{BooleanProperty::FilterEnabled, true};
    wmi->inject({.request = write,
                 .operation = {},
                 .timing = FakeUwfBackend::FaultTiming::AfterEffect,
                 .kind = ErrorKind::Failed,
                 .message = "reply was lost after the write",
                 .code = 301});
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>(
        "WMI", calls,
        [wmi, write](const Request& request, const Context& context) {
          auto response = wmi->execute(request, context);
          // The command itself needs the Next value for its CLI precheck.
          // Lose the observation entry point only after that command ran.
          if (request == write)
            wmi->setCapability(GetBoolean{BooleanProperty::FilterEnabled, Session::Next}, {Support::Unavailable, "WMI readback unavailable"});
          return response;
        },
        [wmi](const Request& request) { return wmi->probe(request); });
    const auto result = FallbackUwf({first, csp, native}).execute(write);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.effect, Effect::Uncertain);
    QVERIFY(csp->snapshot().next.filterEnabled);
    QCOMPARE(writes(*wmi), std::size_t{1});
    QCOMPARE(writes(*csp), std::size_t{0});
    QCOMPARE(writes(*native), std::size_t{0});
    QVERIFY(csp->trace().empty());
    QVERIFY(hasDiagnostic(result, "WMI", 301, "reply was lost"));
  }

  void exclusionReadbackDelegatesNameIdentityToTheProvider_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<int>("action");
    QTest::addColumn<bool>("observedPresent");
    QTest::addColumn<bool>("targetMatches");
    for (const bool registry : {false, true}) {
      const auto prefix = registry ? QStringLiteral("registry-") : QStringLiteral("file-");
      QTest::newRow(qPrintable(prefix + u"add-normalized-case")) << registry << static_cast<int>(ExclusionAction::Add) << true << true;
      QTest::newRow(qPrintable(prefix + u"remove-absent")) << registry << static_cast<int>(ExclusionAction::Remove) << false << true;
      QTest::newRow(qPrintable(prefix + u"remove-still-present-normalized-case")) << registry << static_cast<int>(ExclusionAction::Remove) << true << false;
      QTest::newRow(qPrintable(prefix + u"clear-empty-list")) << registry << static_cast<int>(ExclusionAction::Clear) << false << true;
    }
  }

  void visibleTargetNeverReplacesAFailedCompositeApi_data() {
    QTest::addColumn<int>("index");
    for (std::size_t index = 0; index < visibleTargets.size(); ++index) QTest::newRow(visibleTargets[index].name) << static_cast<int>(index);
  }
  void visibleTargetNeverReplacesAFailedCompositeApi() {
    QFETCH(int, index);
    const auto& test = visibleTargets.at(static_cast<std::size_t>(index));
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("WMI", calls, [&test](const Request& request, const Context&) {
      if (isWrite(request)) return Response::fail(ErrorKind::Failed, "a later API step failed after changing visible state", Effect::Uncertain);
      if (request != test.read) return Response::fail(ErrorKind::Protocol, "unexpected observation");
      Response response;
      response.value = test.observed;
      return response;
    });
    auto second = std::make_shared<Endpoint>("CSP", calls);
    const auto result = FallbackUwf({first, second}).execute(test.write);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.failure->message, std::string("a later API step failed after changing visible state"));
    QCOMPARE(result.effect, Effect::Uncertain);
    QCOMPARE(calls->size(), std::size_t{1});
    QVERIFY(calls->back().request == test.write);
    QCOMPARE(backends(calls, true), QStringList{"WMI"});
    QVERIFY(std::ranges::none_of(result.failure->diagnostics, [](const auto& item) { return item.detail.find("Readback") != std::string::npos; }));
  }
  void exclusionReadbackDelegatesNameIdentityToTheProvider() {
    QFETCH(bool, registry);
    QFETCH(int, action);
    QFETCH(bool, observedPresent);
    QFETCH(bool, targetMatches);
    const auto edit = static_cast<ExclusionAction>(action);
    const std::string volume = "11111111-2222-3333-4444-555555555555";
    const std::string requested = registry ? "HKEY_LOCAL_MACHINE\\software\\case" : "\\data\\cache";
    const std::string canonical = registry ? "HKEY_LOCAL_MACHINE\\SOFTWARE\\Case" : "\\Data\\Cache";
    const Request request = registry ? Request{EditRegistryExclusion{requested, edit}} : Request{EditFileExclusion{volume, requested, edit}};
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("WMI", calls, [observedPresent, canonical](const Request& readback, const Context&) {
      if (isWrite(readback)) return Response::fail(ErrorKind::Failed, "write acknowledgement was lost", Effect::Uncertain);
      if (std::holds_alternative<FindFileExclusion>(readback) || std::holds_alternative<FindRegistryExclusion>(readback))
        return Response::success(observedPresent);
      return Response::success(observedPresent ? std::vector<std::string>{canonical} : std::vector<std::string>{});
    });
    auto second =
        std::make_shared<Endpoint>("CSP", calls, [](const Request&, const Context&) { return Response::success(std::monostate{}, Effect::Confirmed); });
    const auto result = FallbackUwf({first, second}).execute(request);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.effect, Effect::Uncertain);
    (void)targetMatches;
    QCOMPARE(backends(calls), QStringList{"WMI"});
    QCOMPARE(calls->size(), std::size_t{1});
  }

  void unresolvedWritesNeverReachLowerPriorityWriters_data() {
    QTest::addColumn<int>("failure");
    QTest::newRow("readback-mismatch") << 0;
    QTest::newRow("readback-unavailable") << 1;
    QTest::newRow("write-standard-exception") << 2;
    QTest::newRow("write-malformed-result") << 3;
  }
  void unresolvedWritesNeverReachLowerPriorityWriters() {
    QFETCH(int, failure);
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("WMI", calls, [failure](const Request& request, const Context&) {
      if (!isWrite(request)) return failure == 1 ? Response::fail(ErrorKind::Unavailable, "readback unavailable") : Response::success(false);
      if (failure == 2) throw std::runtime_error("write may have reached the provider");
      if (failure == 3) return Response::success(true, Effect::Confirmed);  // SetBoolean requires monostate.
      return Response::fail(ErrorKind::Failed, "write result is unknown", Effect::Uncertain);
    });
    auto second = std::make_shared<Endpoint>("CSP", calls, [failure](const Request& request, const Context&) {
      if (isWrite(request)) return Response::success(std::monostate{}, Effect::Confirmed);
      return failure == 1 ? Response::fail(ErrorKind::Unavailable, "second readback unavailable") : Response::success(false);
    });
    const auto result = FallbackUwf({first, second}).execute(SetBoolean{BooleanProperty::FilterEnabled, true});
    QVERIFY(!result.succeeded());
    QCOMPARE(result.effect, Effect::Uncertain);
    QCOMPARE(backends(calls, true), QStringList{"WMI"});
    if (failure == 3) QCOMPARE(result.failure->kind, ErrorKind::Protocol);
  }

  void operationsWithoutAuthoritativeReadbackCannotBeReplayed() {
    const std::vector<Request> requests{CommitFile{volumeId, "\\file"},
                                        CommitFileDeletion{volumeId, "\\file"},
                                        CommitRegistry{"HKEY_LOCAL_MACHINE\\Software\\Test", "Value"},
                                        CommitRegistryDeletion{"HKEY_LOCAL_MACHINE\\Software\\Test", {}},
                                        Power{PowerAction::Shutdown},
                                        Power{PowerAction::Restart},
                                        ResetSettings{},
                                        UpdateWindows{}};
    for (const auto& request : requests) {
      const auto calls = std::make_shared<std::vector<Invocation>>();
      auto first = std::make_shared<Endpoint>("WMI", calls, [](const Request&, const Context&) {
        return Response::fail(ErrorKind::Failed, "operation may already have happened", Effect::Uncertain);
      });
      auto second =
          std::make_shared<Endpoint>("CSP", calls, [](const Request&, const Context&) { return Response::success(std::monostate{}, Effect::Confirmed); });
      const auto result = FallbackUwf({first, second}).execute(request);
      QVERIFY2(!result.succeeded(), operationName(request).c_str());
      QCOMPARE(result.effect, Effect::Uncertain);
      QCOMPARE(backends(calls), QStringList{"WMI"});
    }
  }

  void terminalFailureRetainsEveryAttemptDiagnostic() {
    const auto calls = std::make_shared<std::vector<Invocation>>();
    std::vector<std::shared_ptr<Backend>> chain;
    std::int64_t code = 400;
    for (const auto name : {"WMI", "CSP", "Native"}) {
      ++code;
      chain.push_back(std::make_shared<Endpoint>(name, calls, [name, code](const Request& request, const Context&) {
        return Response::fail(ErrorKind::Failed, "read failed", Effect::None, {{name, operationName(request), code, std::string(name) + " native detail"}});
      }));
    }
    const auto result = FallbackUwf(std::move(chain)).execute(GetBoolean{BooleanProperty::FilterEnabled});
    QVERIFY(!result.succeeded());
    QCOMPARE(result.attempts.size(), std::size_t{3});
    QCOMPARE(backends(calls), QStringList({"WMI", "CSP", "Native"}));
    QVERIFY(hasDiagnostic(result, "WMI", 401, "WMI native detail"));
    QVERIFY(hasDiagnostic(result, "CSP", 402, "CSP native detail"));
    QVERIFY(hasDiagnostic(result, "Native", 403, "Native native detail"));
    QCOMPARE(result.failure->diagnostics.size(), std::size_t{3});
    QCOMPARE(result.failure->diagnostics.back().code, std::int64_t{403});
    const auto formatted = QString::fromStdString(uwf::app::uwfDiagnostics(Error(*result.failure, result.attempts)));
    QCOMPARE(formatted.count(QStringLiteral("WMI native detail")), 1);
    QCOMPARE(formatted.count(QStringLiteral("CSP native detail")), 1);
    QCOMPARE(formatted.count(QStringLiteral("Native native detail")), 1);
  }

  void nestedAttemptsSurviveAnExceptionOrMalformedResponse_data() {
    QTest::addColumn<int>("failure");
    QTest::newRow("returned-failure") << 0;
    QTest::newRow("thrown-failure") << 1;
    QTest::newRow("malformed-response") << 2;
    QTest::newRow("probe-exception") << 3;
  }
  void nestedAttemptsSurviveAnExceptionOrMalformedResponse() {
    QFETCH(int, failure);
    const auto calls = std::make_shared<std::vector<Invocation>>();
    const std::vector<Attempt> nested{{"inner",
                                       {Support::Supported, "nested provider"},
                                       Failure{ErrorKind::Failed, Effect::None, "nested read failed", {{"inner", "Read", 501, "nested native detail"}}}}};
    const auto execute = [failure, nested](const Request&, const Context&) -> Response {
      auto response = failure == 2 ? Response::success(false) : Response::fail(ErrorKind::Failed, "wrapper read failed");
      response.attempts = nested;
      if (failure == 1) throw Error(*response.failure, response.attempts);
      return response;
    };
    const auto probe = [failure, nested](const Request&) -> Capability {
      if (failure == 3) throw Error({ErrorKind::Failed, Effect::None, "nested probe failed", {}}, nested);
      return {Support::Supported, "wrapper supports this request"};
    };
    auto first = std::make_shared<Endpoint>("wrapper", calls, execute, probe);
    auto second = std::make_shared<Endpoint>("CSP", calls, [](const Request&, const Context&) { return Response::success(std::uint32_t{2048}); });
    const auto result = FallbackUwf({first, second}).execute(GetNumber{NumberProperty::MaximumOverlaySize});
    QVERIFY(result.succeeded());
    QVERIFY(hasDiagnostic(result, "inner", 501, "nested native detail"));
    QCOMPARE(std::ranges::count(result.attempts, std::string("inner"), &Attempt::backend), 1);
  }

  void typedCallPreservesMalformedWriteUncertaintyAndNestedAttempts() {
    const auto calls = std::make_shared<std::vector<Invocation>>();
    const Endpoint backend("wrapper", calls, [](const Request&, const Context&) {
      auto response = Response::success(true);  // A setter must return monostate.
      response.attempts.push_back({"inner", {Support::Supported, "nested provider"}, {}});
      return response;
    });
    try {
      static_cast<void>(call(backend, SetBoolean{BooleanProperty::FilterEnabled, true}));
      QFAIL("A malformed write response was accepted");
    } catch (const Error& error) {
      QCOMPARE(error.failure().kind, ErrorKind::Protocol);
      QCOMPARE(error.failure().effect, Effect::Uncertain);
      QCOMPARE(error.attempts().size(), std::size_t{1});
      QCOMPARE(error.attempts().front().backend, std::string("inner"));
    }
  }

  void formattingKeepsTheFinalCancellationReasonWithoutRepeatingNativeEvidence() {
    const Error error(
        {ErrorKind::Canceled, Effect::None, "canceled before the next backend", {}},
        {{"WMI", {Support::Supported, "available"}, Failure{ErrorKind::Failed, Effect::None, "first read failed", {{"WMI", "Read", 502, "native evidence"}}}}});
    const auto formatted = QString::fromStdString(uwf::app::uwfDiagnostics(error));
    QVERIFY(formatted.contains(u"canceled before the next backend"));
    QCOMPARE(formatted.count(QStringLiteral("native evidence")), 1);
  }

  void ordinaryExceptionsKeepTheirOriginalMessageAtTheApplicationBoundary() {
    const std::string message = "original runtime failure\n原始错误详情";
    const Failure failure{ErrorKind::Failed, Effect::None, message, {{"WMI", "Read", 0, message}}};
    const Error error(failure, {{"WMI", {Support::Unknown, "entry point discovered at runtime"}, failure}});
    try {
      uwf::app::throwUwfOperationError(error);
    } catch (const uwf::app::OperationError& operation) {
      QCOMPARE(std::string(operation.what()), message);
      QCOMPARE(operation.failureKind(), uwf::app::OperationFailureKind::Failed);
    }
  }

  void anAfterEffectCancellationStillUsesAvailableReadback() {
    const auto shared = FakeUwfBackend::createSystem();
    auto first = std::make_shared<FakeUwfBackend>(shared, "WMI");
    auto second = std::make_shared<FakeUwfBackend>(shared, "CSP");
    const Request request = SetBoolean{BooleanProperty::FilterEnabled, true};
    first->inject({.request = request,
                   .operation = {},
                   .timing = FakeUwfBackend::FaultTiming::AfterEffect,
                   .kind = ErrorKind::Canceled,
                   .message = "cancellation reported after submission",
                   .code = 503});
    const auto result = FallbackUwf({first, second}).execute(request);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.failure->kind, ErrorKind::Canceled);
    QCOMPARE(result.effect, Effect::Uncertain);
    QCOMPARE(writes(*first), std::size_t{1});
    QCOMPARE(writes(*second), std::size_t{0});
  }

  void expensiveReadCompletionControlsAutomaticFallback_data() {
    QTest::addColumn<int>("completionValue");
    QTest::addColumn<int>("kindValue");
    for (const auto completion : {ReadCompletion::NotDispatched, ReadCompletion::ProviderCompleted, ReadCompletion::Unknown})
      for (const auto kind : {ErrorKind::Unavailable, ErrorKind::Unsupported, ErrorKind::Protocol, ErrorKind::ResourceExhausted})
        QTest::newRow(qPrintable(QStringLiteral("completion-%1/kind-%2").arg(int(completion)).arg(int(kind)))) << int(completion) << int(kind);
  }
  void expensiveReadCompletionControlsAutomaticFallback() {
    QFETCH(int, completionValue);
    QFETCH(int, kindValue);
    const auto completion = static_cast<ReadCompletion>(completionValue);
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("first", calls, [=](const Request&, const Context&) {
      return Response::fail(static_cast<ErrorKind>(kindValue), "single read failed", Effect::None, {}, completion);
    });
    auto second = std::make_shared<Endpoint>("second", calls, [](const Request&, const Context&) { return Response::success(std::vector<OverlayFile>{}); });
    const auto result = FallbackUwf({first, second}).execute(GetOverlayFiles{volumeId});
    if (completion == ReadCompletion::NotDispatched) {
      QVERIFY(result.succeeded());
      QCOMPARE(backends(calls), (QStringList{"first", "second"}));
    } else {
      QVERIFY(result.failure);
      QCOMPARE(result.failure->readCompletion, completion);
      QCOMPARE(backends(calls), QStringList{"first"});
      QCOMPARE(result.attempts.size(), std::size_t{1});
      QCOMPARE(result.attempts.front().failure->readCompletion, completion);
    }
  }
  void anUntypedEnumerationFailureCannotStartAnotherProvider() {
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("first", calls, [](const Request&, const Context&) -> Response { throw std::runtime_error("lost response"); });
    auto second = std::make_shared<Endpoint>("second", calls);
    const auto result = FallbackUwf({first, second}).execute(GetOverlayFiles{volumeId});
    QVERIFY(result.failure);
    QCOMPARE(result.failure->readCompletion, ReadCompletion::Unknown);
    QCOMPARE(backends(calls), QStringList{"first"});
  }
  void aMalformedEnumerationResponseRemainsACompletedRead() {
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("first", calls, [](const Request&, const Context&) { return Response::success(true); });
    auto second = std::make_shared<Endpoint>("second", calls);
    const auto result = FallbackUwf({first, second}).execute(GetOverlayFiles{volumeId});
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, ErrorKind::Protocol);
    QCOMPARE(result.failure->readCompletion, ReadCompletion::ProviderCompleted);
    QCOMPARE(backends(calls), QStringList{"first"});
  }
  void cancellationStopsBeforeTheNextBackend_data() {
    QTest::addColumn<bool>("preCanceled");
    QTest::newRow("already-canceled") << true;
    QTest::newRow("backend-canceled") << false;
  }
  void cancellationStopsBeforeTheNextBackend() {
    QFETCH(bool, preCanceled);
    const auto calls = std::make_shared<std::vector<Invocation>>();
    auto first = std::make_shared<Endpoint>("WMI", calls, [](const Request&, const Context&) { return Response::fail(ErrorKind::Canceled, "canceled"); });
    auto second = std::make_shared<Endpoint>("CSP", calls, [](const Request&, const Context&) { return Response::success(true); });
    std::stop_source stop;
    if (preCanceled) stop.request_stop();
    const auto result = FallbackUwf({first, second}).execute(GetBoolean{BooleanProperty::FilterEnabled}, {stop.get_token()});
    QVERIFY(!result.succeeded());
    QCOMPARE(result.failure->kind, ErrorKind::Canceled);
    QCOMPARE(result.effect, Effect::None);
    QCOMPARE(backends(calls), preCanceled ? QStringList{} : QStringList{"WMI"});
  }

  void backendPluginsAreOrderedDataRatherThanFixedProtocolCases() {
    const auto calls = std::make_shared<std::vector<Invocation>>();
    std::vector<std::shared_ptr<Backend>> chain;
    for (const auto name : {"WMI", "CSP", "Native"})
      chain.push_back(
          std::make_shared<Endpoint>(name, calls, Endpoint::Execute{}, [](const Request&) { return Capability{Support::Unsupported, "not provided"}; }));
    chain.push_back(std::make_shared<Endpoint>("future-cli", calls, [](const Request&, const Context&) { return Response::success(std::uint32_t{8192}); }));
    const auto result = FallbackUwf(std::move(chain)).execute(GetNumber{NumberProperty::MaximumOverlaySize});
    QVERIFY(result.succeeded());
    QCOMPARE(std::get<std::uint32_t>(result.value), std::uint32_t{8192});
    QCOMPARE(backends(calls), QStringList{"future-cli"});
    QCOMPARE(result.attempts.size(), std::size_t{4});
    QCOMPARE(result.attempts.back().backend, std::string("future-cli"));
  }

  void routerPassesOverlayIntentThroughWithoutItsOwnReadModifyWrite() {
    const auto calls = std::make_shared<std::vector<Invocation>>();
    const SetOverlayOption requested{OverlayOption::Persistent, true};
    auto first = std::make_shared<Endpoint>("WMI", calls, [](const Request&, const Context&) { return Response::fail(ErrorKind::Unsupported, "no entry"); });
    auto second =
        std::make_shared<Endpoint>("CSP", calls, [](const Request&, const Context&) { return Response::success(std::monostate{}, Effect::Confirmed); });
    const auto result = FallbackUwf({first, second}).execute(requested);
    QVERIFY(result.succeeded());
    QCOMPARE(calls->size(), std::size_t{2});
    for (const auto& call : *calls) QVERIFY(call.request == Request{requested});
  }

  void backendPreservesUnknownBitsForApplicationIntentAcrossFallback() {
    FakeUwfBackend::State initial;
    initial.current.overlayType = initial.next.overlayType = static_cast<std::uint32_t>(OverlayType::Disk);
    initial.next.filterEnabled = true;
    constexpr std::uint32_t unknownFlags = 0x80000000U;
    initial.current.overlayFlags = initial.next.overlayFlags = unknownFlags | flags::persistent;
    const auto shared = FakeUwfBackend::createSystem(initial);
    auto wmi = std::make_shared<FakeUwfBackend>(shared, "WMI");
    auto csp = std::make_shared<FakeUwfBackend>(shared, "CSP");
    auto native = std::make_shared<FakeUwfBackend>(shared, "Native");
    wmi->setCapability(GetNumber{NumberProperty::OverlayFlags, Session::Current}, {Support::Unsupported, "no overlay-flags property"});
    wmi->setCapability(GetNumber{NumberProperty::OverlayFlags, Session::Next}, {Support::Unsupported, "no overlay-flags property"});
    wmi->setCapability(SetNumber{NumberProperty::OverlayFlags, unknownFlags | flags::persistent | flags::passthrough},
                       {Support::Unsupported, "no overlay-flags setter"});
    auto router = std::make_shared<FallbackUwf>(std::vector<std::shared_ptr<Backend>>{wmi->rawBackend(), csp->rawBackend(), native->rawBackend()});
    auto operations = std::make_shared<uwf::app::PrecheckedUwf>(router);
    operations->updateState(wmi->state());
    const auto policy = uwf::app::uwfOverlayPolicy(operations);
    uwf::app::OverlayPolicyUpdate update;
    update.expectedFlags = initial.next.overlayFlags;
    update.changes.passthrough = true;
    const auto result = policy->apply(update);
    QVERIFY(result.succeeded());
    QCOMPARE(csp->snapshot().next.overlayFlags, unknownFlags | flags::persistent | flags::passthrough);
    QCOMPARE(csp->snapshot().current.overlayFlags, initial.current.overlayFlags);
    QCOMPARE(writes(*wmi), std::size_t{0});
    QCOMPARE(writes(*csp), std::size_t{1});
    QCOMPARE(writes(*native), std::size_t{0});
  }
};
}  // namespace

QTEST_APPLESS_MAIN(UwfFallbackTests)
#include "UwfFallbackTests.moc"
