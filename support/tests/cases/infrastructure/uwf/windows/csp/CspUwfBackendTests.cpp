// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <array>
#include <deque>
#include <limits>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <type_traits>
#include <utility>

#include "domain/uwf/contracts/Requests.h"
#include "infrastructure/uwf/routing/FallbackUwf.h"
#include "infrastructure/uwf/windows/csp/CspUwfBackend.h"
#include "infrastructure/windows/management/csp/WapProvisioningXml.h"
#include "simulation/uwf/FakeUwfBackend.h"
#include "support/uwf/CapturedUwfContractVectors.h"

Q_DECLARE_METATYPE(uwf::compat::Request)
Q_DECLARE_METATYPE(uwf::compat::Value)
enum class WapAttribute { Parent, Name, Value };
Q_DECLARE_METATYPE(WapAttribute)

namespace {
using namespace uwf::compat;
using namespace uwf::windows;
using uwf::windows::backends::CspUwfBackend;
constexpr auto fixtureVolume = "11111111-2222-3333-4444-555555555555";

QString valueResponse(QStringView session, QStringView name, QStringView datatype, QStringView value) {
  return QString(u"<wap-provisioningdoc><characteristic type='UnifiedWriteFilter'><characteristic type='") + session + u"'><parm name='" + name +
         u"' datatype='" + datatype + u"' value='" + value + u"'/></characteristic></characteristic></wap-provisioningdoc>";
}

QString scopedResponse(const QStringList& path, const QString& leaf = {}, const QString& datatype = {}) {
  QString xml;
  QXmlStreamWriter writer(&xml);
  writer.writeStartElement(u"wap-provisioningdoc");
  for (const auto& name : std::as_const(path)) {
    writer.writeStartElement(u"characteristic");
    writer.writeAttribute(u"type", name);
  }
  if (!leaf.isEmpty()) {
    writer.writeStartElement(u"parm");
    writer.writeAttribute(u"name", leaf);
    writer.writeAttribute(u"datatype", datatype);
    writer.writeAttribute(u"value", u"0");
    writer.writeEndElement();
  }
  for (qsizetype i = 0; i <= path.size(); ++i) writer.writeEndElement();
  return xml;
}

QString scalarResponse(const QStringList& path, const QString& leaf, const QString& datatype, const QString& value) {
  auto xml = scopedResponse(path, leaf, datatype);
  xml.replace(QStringLiteral("value=\"0\""), QStringLiteral("value=\"%1\"").arg(value));
  return xml;
}

QString exclusionsResponse(const QStringList& keys,
                           const QStringList& path = {QStringLiteral("UnifiedWriteFilter"), QStringLiteral("NextSession"),
                                                      QStringLiteral("RegistryExclusions")},
                           const QStringList& names = {}) {
  QString xml;
  QXmlStreamWriter writer(&xml);
  writer.writeStartElement(u"wap-provisioningdoc");
  for (const auto& name : path) {
    writer.writeStartElement(u"characteristic");
    writer.writeAttribute(u"type", name);
  }
  for (qsizetype index = 0; index < keys.size(); ++index) {
    const auto& key = keys[index];
    writer.writeStartElement(u"parm");
    writer.writeAttribute(u"name", names.value(index, QString::fromLatin1(QUrl::toPercentEncoding(key))));
    writer.writeAttribute(u"datatype", u"string");
    writer.writeAttribute(u"value", key);
    writer.writeEndElement();
  }
  for (qsizetype i = 0; i <= path.size(); ++i) writer.writeEndElement();
  return xml;
}

class ScriptedTransport final : public DmCspTransport {
 public:
  bool loaded = true;
  std::size_t availabilityChecks = 0;
  std::deque<DmCspReply> replies;
  std::vector<QString> requests;
  std::vector<QStringList> filters;
  bool available() override {
    ++availabilityChecks;
    return loaded;
  }
  DmCspReply process(const QString& xml, const QStringList& allowed) override {
    requests.push_back(xml);
    filters.push_back(allowed);
    if (replies.empty()) return {};
    auto result = replies.front();
    replies.pop_front();
    return result;
  }
};

class CspUwfBackendTests final : public QObject {
  Q_OBJECT
  enum class RegistryMaintenanceOperation { CommitValue, DeleteKey, DeleteValue };
  Q_ENUM(RegistryMaintenanceOperation)
 private slots:
  void preDispatchTransportErrorsPreserveKnownMeaningsAndProviderPriority_data() {
    QTest::addColumn<std::uint32_t>("hresult");
    QTest::addColumn<std::uint32_t>("providerCode");
    QTest::addColumn<bool>("writing");
    QTest::addColumn<int>("kind");
    struct PreDispatchFailure {
      const char* name;
      std::uint32_t hresult;
      std::uint32_t providerCode;
      ErrorKind observation;
      ErrorKind operation;
    };
    const std::array cases{
        PreDispatchFailure{"access-denied", 0x80070005u, 0, ErrorKind::AccessDenied, ErrorKind::AccessDenied},
        PreDispatchFailure{"out-of-memory", 0x8007000eu, 0, ErrorKind::ResourceExhausted, ErrorKind::ResourceExhausted},
        PreDispatchFailure{"cancelled", 0x800704c7u, 0, ErrorKind::Canceled, ErrorKind::Canceled},
        PreDispatchFailure{"abort", 0x80004004u, 0, ErrorKind::Canceled, ErrorKind::Canceled},
        PreDispatchFailure{"not-implemented", 0x80004001u, 0, ErrorKind::Unsupported, ErrorKind::Unsupported},
        PreDispatchFailure{"no-interface", 0x80004002u, 0, ErrorKind::Unsupported, ErrorKind::Unsupported},
        PreDispatchFailure{"busy", 0x800700aau, 0, ErrorKind::Conflict, ErrorKind::Conflict},
        PreDispatchFailure{"already-exists", 0x800700b7u, 0, ErrorKind::Conflict, ErrorKind::Conflict},
        PreDispatchFailure{"invalid-argument", 0x80070057u, 0, ErrorKind::InvalidArgument, ErrorKind::InvalidArgument},
        PreDispatchFailure{"not-found", 0x80070002u, 0, ErrorKind::NotFound, ErrorKind::NotFound},
        PreDispatchFailure{"rpc-unavailable", 0x800706bau, 0, ErrorKind::Unavailable, ErrorKind::Unavailable},
        PreDispatchFailure{"invalid-data-only-proves-observation-shape", 0x8007000du, 0, ErrorKind::Protocol, ErrorKind::Unavailable},
        PreDispatchFailure{"datatype-only-proves-observation-shape", 0x8007065du, 0, ErrorKind::Protocol, ErrorKind::Unavailable},
        PreDispatchFailure{"unicode-only-proves-observation-shape", 0x80070459u, 0, ErrorKind::Protocol, ErrorKind::Unavailable},
        PreDispatchFailure{"unknown-apartment-failure", 0x80010106u, 0, ErrorKind::Unavailable, ErrorKind::Unavailable},
        PreDispatchFailure{"unknown-transport-failure", 0x80004005u, 0, ErrorKind::Unavailable, ErrorKind::Unavailable},
        PreDispatchFailure{"csp-not-supported", 0x86000011u, 0, ErrorKind::Unsupported, ErrorKind::Unsupported},
        PreDispatchFailure{"csp-not-found", 0x86000002u, 0, ErrorKind::NotFound, ErrorKind::NotFound},
        PreDispatchFailure{"provider-unsupported-overrides-transport-access", 0x80070005u, 0x86000011u, ErrorKind::Unsupported, ErrorKind::Unsupported},
        PreDispatchFailure{"provider-not-found-overrides-transport-access", 0x80070005u, 0x86000002u, ErrorKind::NotFound, ErrorKind::NotFound},
        PreDispatchFailure{"provider-memory-overrides-transport-access", 0x80070005u, 0x8007000eu, ErrorKind::ResourceExhausted, ErrorKind::ResourceExhausted}};
    for (const auto& item : cases)
      for (const bool writing : {false, true})
        QTest::newRow(qUtf8Printable(QString::fromLatin1(item.name) + (writing ? QStringLiteral("/write") : QStringLiteral("/read"))))
            << item.hresult << item.providerCode << writing << int(writing ? item.operation : item.observation);
  }
  void preDispatchTransportErrorsPreserveKnownMeaningsAndProviderPriority() {
    QFETCH(std::uint32_t, hresult);
    QFETCH(std::uint32_t, providerCode);
    QFETCH(bool, writing);
    QFETCH(int, kind);
    const auto transport = std::make_shared<ScriptedTransport>();
    const auto xml = providerCode
                         ? QStringLiteral("<wap-provisioningdoc><parm-error name='OverlayFlags' hresult='%1'/></wap-provisioningdoc>").arg(providerCode)
                         : QString{};
    transport->replies.push_back({hresult, xml, QStringLiteral("pre-dispatch transport diagnostic"), false});
    const Request request =
        writing ? Request{SetNumber{NumberProperty::OverlayFlags, flags::readOnlyMedia}} : Request{GetNumber{NumberProperty::OverlayFlags, Session::Next}};
    const auto response = CspUwfBackend(transport).execute(request);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, static_cast<ErrorKind>(kind));
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(response.failure->effect, Effect::None);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QCOMPARE(response.failure->diagnostics.size(), providerCode ? std::size_t{2} : std::size_t{1});
    QCOMPARE(response.failure->diagnostics.front().code, std::int64_t{hresult});
    QVERIFY(response.failure->diagnostics.front().detail.find("pre-dispatch transport diagnostic") != std::string::npos);
    if (providerCode) QCOMPARE(response.failure->diagnostics.back().code, std::int64_t{providerCode});
  }
  void exclusionCommandsHaveExplicitWireReceipts_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<int>("action");
    for (const bool registry : {false, true})
      for (const auto action : {ExclusionAction::Add, ExclusionAction::Remove})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(registry ? "registry" : "file").arg(int(action)))) << registry << int(action);
  }
  void exclusionCommandsHaveExplicitWireReceipts() {
    QFETCH(bool, registry);
    QFETCH(int, action);
    const auto operation = static_cast<ExclusionAction>(action);
    const auto entry = registry ? QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Wire%20") : QStringLiteral("\\Wire%20");
    const auto stored = entry.toUtf8().toStdString();
    const Request request = registry ? Request{EditRegistryExclusion{stored, operation}} : Request{EditFileExclusion{fixtureVolume, stored, operation}};
    const auto transport = std::make_shared<ScriptedTransport>();
    QStringList path{QStringLiteral("UnifiedWriteFilter"), QStringLiteral("NextSession")};
    if (registry)
      path << QStringLiteral("RegistryExclusions");
    else
      path << QStringLiteral("Volume") << QString::fromLatin1(fixtureVolume) << QStringLiteral("Exclusions");
    const auto response = CspUwfBackend(transport).execute(request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "command failed");
    QCOMPARE(response.effect, Effect::Confirmed);
    QVERIFY(std::holds_alternative<std::monostate>(response.value));
    QCOMPARE(transport->requests.size(), std::size_t{1});
    const auto& wire = transport->requests.front();
    const auto element = operation == ExclusionAction::Add ? QStringLiteral("<parm name=") : QStringLiteral("<noparm name=");
    QVERIFY(wire.contains(element + u'"' + QString::fromLatin1(QUrl::toPercentEncoding(entry)) + u'"'));
    for (const auto& component : std::as_const(path)) QVERIFY(wire.contains(u"type=\"" + component + u'"'));
    if (operation == ExclusionAction::Add) QVERIFY(wire.contains(u"value=\"" + entry + u'"'));
  }
  void platformErrorsRetainTheirContextAndOriginalCode_data() {
    QTest::addColumn<std::uint32_t>("code");
    QTest::addColumn<bool>("writing");
    QTest::addColumn<bool>("providerError");
    QTest::addColumn<int>("kind");
    struct NativeFailure {
      std::uint32_t code;
      ErrorKind observation;
      ErrorKind operation;
    };
    const std::array cases{NativeFailure{13, ErrorKind::Protocol, ErrorKind::Failed},
                           NativeFailure{1629, ErrorKind::Protocol, ErrorKind::Failed},
                           NativeFailure{1113, ErrorKind::Protocol, ErrorKind::Failed},
                           NativeFailure{170, ErrorKind::Conflict, ErrorKind::Conflict},
                           NativeFailure{183, ErrorKind::Conflict, ErrorKind::Conflict},
                           NativeFailure{50, ErrorKind::Unsupported, ErrorKind::Unsupported},
                           NativeFailure{1722, ErrorKind::Unavailable, ErrorKind::Unavailable},
                           NativeFailure{5, ErrorKind::AccessDenied, ErrorKind::AccessDenied},
                           NativeFailure{87, ErrorKind::InvalidArgument, ErrorKind::InvalidArgument},
                           NativeFailure{1805, ErrorKind::Conflict, ErrorKind::Conflict}};
    for (const auto& item : cases)
      for (const bool writing : {false, true})
        for (const bool hresult : {false, true})
          for (const bool providerError : {false, true}) {
            if (!providerError && !hresult) continue;  // Native transport success codes have no failure bit.
            const auto code = hresult ? 0x80070000u | item.code : item.code;
            QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3-%4")
                                             .arg(code)
                                             .arg(writing ? "write" : "read")
                                             .arg(hresult ? "HRESULT" : "Win32")
                                             .arg(providerError ? "provider" : "transport")))
                << code << writing << providerError << int(writing ? item.operation : item.observation);
          }
  }
  void platformErrorsRetainTheirContextAndOriginalCode() {
    QFETCH(std::uint32_t, code);
    QFETCH(bool, writing);
    QFETCH(bool, providerError);
    QFETCH(int, kind);
    const auto transport = std::make_shared<ScriptedTransport>();
    const auto xml =
        providerError ? QStringLiteral("<wap-provisioningdoc><parm-error name='OverlayFlags' hresult='%1'/></wap-provisioningdoc>").arg(code) : QString{};
    transport->replies.push_back({providerError ? 0u : code, xml, {}, true});
    const Request request =
        writing ? Request{SetNumber{NumberProperty::OverlayFlags, flags::readOnlyMedia}} : Request{GetNumber{NumberProperty::OverlayFlags, Session::Next}};
    const auto result = CspUwfBackend(transport).execute(request);
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, static_cast<ErrorKind>(kind));
    QCOMPARE(result.effect, writing ? Effect::Uncertain : Effect::None);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QCOMPARE(result.failure->diagnostics.size(), std::size_t{1});
    QCOMPARE(result.failure->diagnostics.front().code, std::int64_t{code});
  }
  void unsupportedMappingsRejectBeforeTransport_data() {
    QTest::addColumn<Request>("request");
    const std::vector<Request> requests{SetBoolean{BooleanProperty::ShutdownPending, true},
                                        SetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted, true},
                                        SetOverlayOption{OverlayOption::ReadOnlyMedia, true},
                                        FindFileExclusion{fixtureVolume, "\\File", Session::Next},
                                        FindRegistryExclusion{"HKLM\\Software\\Fixture", Session::Next},
                                        EditFileExclusion{fixtureVolume, {}, ExclusionAction::Clear},
                                        EditRegistryExclusion{{}, ExclusionAction::Clear},
                                        CommitFile{fixtureVolume, "\\File"},
                                        CommitFileDeletion{fixtureVolume, "\\File"},
                                        CommitRegistry{"HKEY_LOCAL_MACHINE\\Software\\Fixture", std::string("Value")},
                                        CommitRegistryDeletion{"HKEY_LOCAL_MACHINE\\Software\\Fixture", std::nullopt},
                                        ResetSettings{},
                                        UpdateWindows{},
                                        Power{PowerAction::Shutdown},
                                        Power{PowerAction::Restart}};
    for (std::size_t index = 0; index < requests.size(); ++index)
      QTest::newRow(qUtf8Printable(QStringLiteral("unsupported-%1-%2").arg(index).arg(QString::fromStdString(operationName(requests[index])))))
          << requests[index];
  }
  void unsupportedMappingsRejectBeforeTransport() {
    QFETCH(Request, request);
    const auto transport = std::make_shared<ScriptedTransport>();
    const CspUwfBackend backend(transport);
    QCOMPARE(backend.probe(request).support, Support::Unsupported);
    const auto result = backend.execute(request);
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, ErrorKind::Unsupported);
    QCOMPARE(result.effect, Effect::None);
    QVERIFY(transport->requests.empty());
  }
  void allReadSlotsHaveExplicitSessionAndValueEvidence_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<Value>("expected");
    QTest::addColumn<bool>("supported");
    const auto add = [](const Request& request, Value expected, const char* scope, const bool supported = true) {
      QTest::newRow(qUtf8Printable(QString::fromStdString(operationName(request)) + u'-' + QString::fromLatin1(scope))) << request << expected << supported;
    };
    for (const auto session : {Session::Current, Session::Next}) {
      const bool current = session == Session::Current;
      const auto* scope = current ? "Current" : "Next";
      add(GetBoolean{BooleanProperty::FilterEnabled, session}, Value{!current}, scope);
      add(GetBoolean{BooleanProperty::ShutdownPending, session}, Value{true}, scope);
      add(GetBoolean{BooleanProperty::HormEnabled, session}, Value{false}, scope);
      add(GetBoolean{BooleanProperty::ServicingEnabled, session}, Value{!current}, scope);
      add(GetBoolean{BooleanProperty::PersistDomainSecretKey, session}, Value{false}, scope, current);
      add(GetBoolean{BooleanProperty::PersistTscal, session}, Value{false}, scope, false);
      add(GetBoolean{BooleanProperty::OverlayCommit, session}, Value{false}, scope);
      add(GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted, session}, Value{true}, scope, false);
      add(GetNumber{NumberProperty::OverlayType, session}, Value{current ? std::uint32_t{0} : std::uint32_t{1}}, scope);
      add(GetNumber{NumberProperty::MaximumOverlaySize, session}, Value{current ? std::uint32_t{1024} : std::uint32_t{3072}}, scope);
      add(GetNumber{NumberProperty::WarningThreshold, session}, Value{std::uint32_t{64}}, scope);
      add(GetNumber{NumberProperty::CriticalThreshold, session}, Value{std::uint32_t{128}}, scope);
      add(GetNumber{NumberProperty::OverlayFlags, session}, Value{current ? std::uint32_t{0x80000004} : std::uint32_t{0x40000002}}, scope);
      add(GetNumber{NumberProperty::ResetPersistentState, session}, Value{std::uint32_t{1}}, scope);
      add(GetNumber{NumberProperty::ResetPersistentStateSavedMode, session}, Value{std::uint32_t{1}}, scope, !current);
      add(GetVolumes{session}, Value{std::vector<Volume>{{fixtureVolume, current ? "C:" : "D:", current, !current, {}}}}, scope);
      add(GetVolumeProperty{fixtureVolume, VolumeProperty::Protected, session}, Value{current}, scope);
      add(GetVolumeProperty{fixtureVolume, VolumeProperty::BindByDriveLetter, session}, Value{!current}, scope);
      add(GetFileExclusions{fixtureVolume, session},
          Value{current ? std::vector<std::string>{"\\Current", "\\Literal%20"} : std::vector<std::string>{"\\Next"}}, scope);
      add(FindFileExclusion{fixtureVolume, "\\cUrReNt", session}, Value{current}, scope, false);
      add(GetRegistryExclusions{session},
          Value{current ? std::vector<std::string>{"HKEY_LOCAL_MACHINE\\Software\\Current"} : std::vector<std::string>{"HKEY_LOCAL_MACHINE\\Software\\Next"}},
          scope);
      add(FindRegistryExclusion{"hkey_local_machine\\software\\current", session}, Value{current}, scope, false);
      add(GetSwapfileSize{fixtureVolume, session}, Value{current ? std::uint32_t{1} : std::uint32_t{2}}, scope);
    }
    add(GetOverlayUsage{}, Value{OverlayUsage{0, 0}}, "Current-global");
    add(GetOverlayFiles{fixtureVolume}, Value{std::vector<OverlayFile>{}}, "Current-exact-volume", false);
  }
  void allReadSlotsHaveExplicitSessionAndValueEvidence() {
    QFETCH(Request, request);
    QFETCH(Value, expected);
    QFETCH(bool, supported);
    const auto transport = std::make_shared<ScriptedTransport>();
    const auto scalar = [&](const QStringList& path, const QString& leaf, const QString& datatype, const QString& value) {
      transport->replies.push_back({0, scalarResponse(path, leaf, datatype, value), {}, true});
    };
    const auto boolean = [&](const QStringList& path, const QString& leaf, const bool value) {
      scalar(path, leaf, QStringLiteral("boolean"), value ? QStringLiteral("1") : QStringLiteral("0"));
    };
    const QStringList currentPath{QStringLiteral("UnifiedWriteFilter"), QStringLiteral("CurrentSession")};
    const QStringList nextPath{QStringLiteral("UnifiedWriteFilter"), QStringLiteral("NextSession")};
    if (supported) {
      std::visit(
          [&](const auto& operation) {
            using R = std::decay_t<decltype(operation)>;
            if constexpr (std::is_same_v<R, GetBoolean>) {
              switch (operation.property) {
                case BooleanProperty::FilterEnabled:
                  boolean(operation.session == Session::Current ? currentPath : nextPath, QStringLiteral("FilterEnabled"), operation.session == Session::Next);
                  break;
                case BooleanProperty::ShutdownPending:
                  boolean(currentPath, QStringLiteral("ShutdownPending"), true);
                  break;
                case BooleanProperty::HormEnabled:
                  boolean(nextPath, QStringLiteral("HORMEnabled"), false);
                  break;
                case BooleanProperty::ServicingEnabled:
                  boolean(operation.session == Session::Current ? currentPath : nextPath, QStringLiteral("ServicingEnabled"),
                          operation.session == Session::Next);
                  break;
                case BooleanProperty::PersistDomainSecretKey:
                  boolean(currentPath, QStringLiteral("PersistDomainSecretKey"), false);
                  break;
                case BooleanProperty::OverlayCommit:
                  boolean(currentPath, QStringLiteral("OverlayCommit"), false);
                  break;
                default:
                  QFAIL("Unexpected supported boolean vector");
              }
            } else if constexpr (std::is_same_v<R, GetNumber>) {
              switch (operation.property) {
                case NumberProperty::OverlayType:
                  scalar(operation.session == Session::Current ? currentPath : nextPath, QStringLiteral("OverlayType"), QStringLiteral("integer"),
                         operation.session == Session::Current ? QStringLiteral("0") : QStringLiteral("1"));
                  break;
                case NumberProperty::MaximumOverlaySize:
                  scalar(operation.session == Session::Current ? currentPath : nextPath, QStringLiteral("MaximumOverlaySize"), QStringLiteral("integer"),
                         operation.session == Session::Current ? QStringLiteral("1024") : QStringLiteral("3072"));
                  break;
                case NumberProperty::WarningThreshold:
                  scalar(currentPath, QStringLiteral("WarningOverlayThreshold"), QStringLiteral("integer"), QStringLiteral("64"));
                  break;
                case NumberProperty::CriticalThreshold:
                  scalar(currentPath, QStringLiteral("CriticalOverlayThreshold"), QStringLiteral("integer"), QStringLiteral("128"));
                  break;
                case NumberProperty::OverlayFlags:
                  scalar(operation.session == Session::Current ? currentPath : nextPath, QStringLiteral("OverlayFlags"), QStringLiteral("integer"),
                         operation.session == Session::Current ? QStringLiteral("2147483652") : QStringLiteral("1073741826"));
                  break;
                case NumberProperty::ResetPersistentState:
                  scalar(nextPath, QStringLiteral("ResetPersistentState"), QStringLiteral("integer"), QStringLiteral("1"));
                  break;
                case NumberProperty::ResetPersistentStateSavedMode:
                  scalar(nextPath, QStringLiteral("ResetPersistentStateSavedMode"), QStringLiteral("integer"), QStringLiteral("1"));
                  break;
                default:
                  QFAIL("Unexpected supported numeric vector");
              }
            } else if constexpr (std::is_same_v<R, GetOverlayUsage>) {
              scalar(currentPath, QStringLiteral("OverlayConsumption"), QStringLiteral("integer"), QStringLiteral("0"));
              scalar(currentPath, QStringLiteral("AvailableOverlaySpace"), QStringLiteral("integer"), QStringLiteral("0"));
            } else if constexpr (std::is_same_v<R, GetVolumes>) {
              const bool current = operation.session == Session::Current;
              auto collection = current ? currentPath : nextPath;
              collection << QStringLiteral("Volume");
              const auto xml = QStringLiteral(
                                   "<wap-provisioningdoc><characteristic type=\"UnifiedWriteFilter\"><characteristic type=\"%1\">"
                                   "<characteristic type=\"Volume\"><characteristic type=\"%2\"/></characteristic>"
                                   "</characteristic></characteristic></wap-provisioningdoc>")
                                   .arg(current ? QStringLiteral("CurrentSession") : QStringLiteral("NextSession"), QString::fromLatin1(fixtureVolume));
              transport->replies.push_back({0, xml, {}, true});
              auto path = collection;
              path << QString::fromLatin1(fixtureVolume);
              scalar(path, QStringLiteral("DriveLetter"), QStringLiteral("string"), current ? QStringLiteral("C:") : QStringLiteral("D:"));
              boolean(path, QStringLiteral("Protected"), current);
              boolean(path, QStringLiteral("BindByDriveLetter"), !current);
            } else if constexpr (std::is_same_v<R, GetVolumeProperty>) {
              auto path = operation.session == Session::Current ? currentPath : nextPath;
              path << QStringLiteral("Volume") << QString::fromLatin1(fixtureVolume);
              const bool protectedProperty = operation.property == VolumeProperty::Protected;
              boolean(path, protectedProperty ? QStringLiteral("Protected") : QStringLiteral("BindByDriveLetter"),
                      protectedProperty ? operation.session == Session::Current : operation.session == Session::Next);
            } else if constexpr (std::is_same_v<R, GetFileExclusions> || std::is_same_v<R, FindFileExclusion> || std::is_same_v<R, GetRegistryExclusions> ||
                                 std::is_same_v<R, FindRegistryExclusion>) {
              constexpr bool registry = std::is_same_v<R, GetRegistryExclusions> || std::is_same_v<R, FindRegistryExclusion>;
              const bool current = operation.session == Session::Current;
              auto path = current ? currentPath : nextPath;
              const auto entries = registry  ? current ? QStringList{QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Current")}
                                                       : QStringList{QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Next")}
                                    : current ? QStringList{QStringLiteral("\\Current"), QStringLiteral("\\Literal%20")}
                                             : QStringList{QStringLiteral("\\Next")};
              if (registry)
                path << QStringLiteral("RegistryExclusions");
              else
                path << QStringLiteral("Volume") << QString::fromLatin1(fixtureVolume) << QStringLiteral("Exclusions");
              transport->replies.push_back({0, exclusionsResponse(entries, path), {}, true});
            } else if constexpr (std::is_same_v<R, GetSwapfileSize>) {
              auto path = operation.session == Session::Current ? currentPath : nextPath;
              path << QStringLiteral("Volume") << QString::fromLatin1(fixtureVolume);
              scalar(path, QStringLiteral("SwapfileSize"), QStringLiteral("integer"),
                     operation.session == Session::Current ? QStringLiteral("1") : QStringLiteral("2"));
            } else {
              QFAIL("Unexpected read vector");
            }
          },
          request);
    }
    const auto submittedReads = transport->replies.size();
    const auto result = CspUwfBackend(transport).execute(request);
    QCOMPARE(result.effect, Effect::None);
    QCOMPARE(transport->requests.size(), submittedReads);
    QVERIFY(transport->replies.empty());
    if (supported) {
      QVERIFY2(result.succeeded(), result.failure ? result.failure->message.c_str() : "read failed");
      QCOMPARE(result.value, expected);
    } else {
      QVERIFY(result.failure);
      QCOMPARE(result.failure->kind, ErrorKind::Unsupported);
    }
  }
  void exclusionNodesRetainTheirExactIdentity_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("current");
    QTest::addColumn<bool>("find");
    QTest::addColumn<int>("representation");
    for (const bool registry : {false, true})
      for (const bool current : {false, true})
        for (const bool find : {false})
          for (int representation = 0; representation < 8; ++representation)
            QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3-%4").arg(registry).arg(current).arg(find).arg(representation)))
                << registry << current << find << representation;
  }
  void exclusionNodesRetainTheirExactIdentity() {
    QFETCH(bool, registry);
    QFETCH(bool, current);
    QFETCH(bool, find);
    QFETCH(int, representation);
    const auto session = current ? Session::Current : Session::Next;
    auto entry = registry ? QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\%20\\项") : QStringLiteral("\\Data\\%20\\项");
    if (representation == 4) entry += QChar(QChar::ReplacementCharacter);
    if (representation == 7) entry += QStringLiteral("%GZ");
    auto name = QString::fromLatin1(QUrl::toPercentEncoding(entry));
    switch (representation) {
      case 0:
      case 7:
        name = entry;
        break;
      case 2:
        name = QStringLiteral("wrong-identity");
        break;
      case 3:
        name += QStringLiteral("%GZ");
        break;
      case 4:
        name.replace(QStringLiteral("%EF%BF%BD"), QStringLiteral("%FF"));
        break;
      case 5:
        name += QStringLiteral("%00");
        break;
      case 6:
        name += QStringLiteral("%2");
        break;
      default:
        break;
    }
    QStringList path{QStringLiteral("UnifiedWriteFilter"), current ? QStringLiteral("CurrentSession") : QStringLiteral("NextSession")};
    if (registry)
      path << QStringLiteral("RegistryExclusions");
    else
      path << QStringLiteral("Volume") << QString::fromLatin1(fixtureVolume) << QStringLiteral("Exclusions");
    const auto transport = std::make_shared<ScriptedTransport>();
    transport->replies.push_back({0, exclusionsResponse({entry}, path, {name}), {}, true});
    const Request request = registry ? find ? Request{FindRegistryExclusion{entry.toStdString(), session}} : Request{GetRegistryExclusions{session}}
                            : find ? Request{FindFileExclusion{fixtureVolume, entry.toStdString(), session}}
                                     : Request{GetFileExclusions{fixtureVolume, session}};
    const auto result = CspUwfBackend(transport).execute(request);
    QCOMPARE(result.effect, Effect::None);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    if (representation != 0 && representation != 1 && representation != 7) {
      QVERIFY(result.failure);
      QCOMPARE(result.failure->kind, ErrorKind::Protocol);
    } else {
      QVERIFY(result.succeeded());
      if (find)
        QCOMPARE(std::get<bool>(result.value), true);
      else
        QCOMPARE(std::get<std::vector<std::string>>(result.value), std::vector<std::string>{entry.toStdString()});
    }
  }

  void overlayRuntimeObservationsReadOnlyTheirOwnNodes() {
    for (const bool usage : {false, true}) {
      const auto transport = std::make_shared<ScriptedTransport>();
      if (usage) {
        transport->replies.push_back({0, valueResponse(u"CurrentSession", u"OverlayConsumption", u"integer", u"7"), {}, true});
        transport->replies.push_back({0, valueResponse(u"CurrentSession", u"AvailableOverlaySpace", u"integer", u"93"), {}, true});
      } else
        transport->replies.push_back({0, valueResponse(u"CurrentSession", u"OverlayCommit", u"boolean", u"1"), {}, true});
      const Request request = usage ? Request{GetOverlayUsage{}} : Request{GetBoolean{BooleanProperty::OverlayCommit, Session::Next}};
      const auto response = CspUwfBackend(transport).execute(request);
      QVERIFY(response.succeeded());
      QCOMPARE(response.effect, Effect::None);
      if (usage)
        QCOMPARE(std::get<OverlayUsage>(response.value), (OverlayUsage{7, 93}));
      else
        QCOMPARE(std::get<bool>(response.value), true);
      QCOMPARE(transport->requests.size(), usage ? std::size_t{2} : std::size_t{1});
      QVERIFY(transport->replies.empty());
      for (const auto& xml : transport->requests) QVERIFY(!xml.contains(u"FilterEnabled"));
    }
  }
  void servicingObservationsPreserveTheSessionAndBooleanResult_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<bool>("enabled");
    for (const bool current : {false, true})
      for (const bool enabled : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2").arg(current ? "Current" : "Next", enabled ? "enabled" : "disabled"))) << current << enabled;
  }
  void servicingObservationsPreserveTheSessionAndBooleanResult() {
    QFETCH(bool, current);
    QFETCH(bool, enabled);
    const auto transport = std::make_shared<ScriptedTransport>();
    const auto session = current ? QStringLiteral("CurrentSession") : QStringLiteral("NextSession");
    transport->replies.push_back({0, valueResponse(session, u"ServicingEnabled", u"boolean", enabled ? u"1" : u"0"), {}, true});
    const CspUwfBackend backend(transport);
    const Request request = GetBoolean{BooleanProperty::ServicingEnabled, current ? Session::Current : Session::Next};
    QCOMPARE(backend.probe(request).support, Support::Unknown);
    const auto result = backend.execute(request);
    QVERIFY(result.succeeded());
    QCOMPARE(std::get<bool>(result.value), enabled);
    QCOMPARE(result.effect, Effect::None);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    const auto& wire = transport->requests.front();
    QVERIFY(wire.contains(u"type=\"" + session + u"\""));
    QVERIFY(wire.contains(u"parm-query name=\"ServicingEnabled\""));
    QVERIFY(!wire.contains(current ? u"NextSession" : u"CurrentSession"));
    QVERIFY(transport->replies.empty());
  }
  void servicingReadFailuresAreNotReportedAsDisabled_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<bool>("missingValue");
    for (const bool current : {false, true})
      for (const bool missingValue : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2").arg(current ? "Current" : "Next", missingValue ? "missing-value" : "access-denied")))
            << current << missingValue;
  }
  void servicingReadFailuresAreNotReportedAsDisabled() {
    QFETCH(bool, current);
    QFETCH(bool, missingValue);
    const auto transport = std::make_shared<ScriptedTransport>();
    const auto session = current ? QStringLiteral("CurrentSession") : QStringLiteral("NextSession");
    transport->replies.push_back({missingValue ? 0u : 0x80070005u, valueResponse(session, u"AnotherProperty", u"boolean", u"0"), {}, true});
    const auto result = CspUwfBackend(transport).execute(GetBoolean{BooleanProperty::ServicingEnabled, current ? Session::Current : Session::Next});
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, missingValue ? ErrorKind::Protocol : ErrorKind::AccessDenied);
    QCOMPARE(result.effect, Effect::None);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    if (!missingValue) {
      QVERIFY(!result.failure->diagnostics.empty());
      QCOMPARE(result.failure->diagnostics.front().code, std::int64_t{0x80070005u});
    }
    QVERIFY(transport->replies.empty());
  }
  void disabledServicingIsACompleteCspObservation_data() {
    QTest::addColumn<bool>("current");
    QTest::newRow("Current") << true;
    QTest::newRow("Next") << false;
  }
  void disabledServicingIsACompleteCspObservation() {
    QFETCH(bool, current);
    const Request request = GetBoolean{BooleanProperty::ServicingEnabled, current ? Session::Current : Session::Next};
    const auto transport = std::make_shared<ScriptedTransport>();
    transport->replies.push_back({0, valueResponse(current ? u"CurrentSession" : u"NextSession", u"ServicingEnabled", u"boolean", u"0"), {}, true});
    const auto csp = std::make_shared<CspUwfBackend>(transport);
    using uwf::simulation::FakeUwfBackend;
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.servicingEnabled = initial.next.servicingEnabled = true;
    const auto simulated = uwf::simulation::FakeUwfBackend::createSystem(initial);
    const auto first = std::make_shared<uwf::simulation::FakeUwfBackend>(simulated, "unavailable-first");
    first->setCapability(request, {Support::Unavailable, "The first backend is unavailable"});
    const auto last = std::make_shared<uwf::simulation::FakeUwfBackend>(simulated, "last-backend");
    const FallbackUwf system({first, csp, last});
    const auto result = system.execute(request);
    QVERIFY(result.succeeded());
    QCOMPARE(std::get<bool>(result.value), false);
    QCOMPARE(result.effect, Effect::None);
    QCOMPARE(result.attempts.size(), std::size_t{2});
    QCOMPARE(result.attempts.back().backend, std::string("CSP"));
    QVERIFY(first->trace().empty());
    QVERIFY(last->trace().empty());
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QVERIFY(transport->replies.empty());
  }
  void hormUsesTheCompleteCspSetterWithoutRuntimeMetadata_data() {
    QTest::addColumn<quint32>("rawStatus");
    QTest::newRow("complete-success") << quint32{0};
    QTest::newRow("raw-access-denied") << quint32{0x80070005u};
    QTest::newRow("raw-invalid-state") << quint32{0x8007070du};
  }
  void hormUsesTheCompleteCspSetterWithoutRuntimeMetadata() {
    QFETCH(quint32, rawStatus);
    const auto transport = std::make_shared<ScriptedTransport>();
    transport->replies.push_back({rawStatus, {}, rawStatus ? QStringLiteral("original complete HORM workflow rejection") : QString{}, true});

    const auto result = CspUwfBackend(transport).execute(SetBoolean{BooleanProperty::HormEnabled, true});
    QCOMPARE(result.succeeded(), rawStatus == 0);
    QCOMPARE(result.effect, rawStatus == 0 ? Effect::Confirmed : Effect::Uncertain);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QVERIFY(transport->requests.front().contains(u"name=\"HORMEnabled\" datatype=\"boolean\" value=\"-1\""));
    QVERIFY(transport->replies.empty());
    if (rawStatus) {
      QCOMPARE(result.failure->kind, rawStatus == 0x80070005u ? ErrorKind::AccessDenied : ErrorKind::Conflict);
      QCOMPARE(result.failure->diagnostics.front().code, std::int64_t{rawStatus});
      QVERIFY(result.failure->diagnostics.front().detail.find("original complete HORM workflow rejection") != std::string::npos);
    }
  }
  void capturedSessionVectorsUseTheRealCspBinding_data() {
    QTest::addColumn<int>("index");
    const auto vectors = uwf::test::captured::filterSessionReads();
    for (std::size_t i = 0; i < vectors.size(); ++i) QTest::newRow(vectors[i].id) << static_cast<int>(i);
  }
  void capturedSessionVectorsUseTheRealCspBinding() {
    QFETCH(int, index);
    const auto vector = uwf::test::captured::filterSessionReads().at(static_cast<std::size_t>(index));
    const auto request = std::get<GetBoolean>(vector.request);
    const auto transport = std::make_shared<ScriptedTransport>();
    // Independent wire facts distinguish Current=false from Next=true.
    const bool current = request.session == Session::Current;
    transport->replies.push_back(
        {0, valueResponse(current ? u"CurrentSession" : u"NextSession", u"FilterEnabled", u"boolean", current ? u"0" : u"-1"), {}, true});
    const auto result = CspUwfBackend(transport).execute(vector.request);
    QVERIFY(result.succeeded());
    QVERIFY(vector.expected.value);
    QVERIFY(result.value == *vector.expected.value);
    QCOMPARE(result.effect, vector.expected.effect);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QVERIFY(transport->requests.front().contains(current ? u"type=\"CurrentSession\"" : u"type=\"NextSession\""));
    QVERIFY(transport->requests.front().contains(u"parm-query name=\"FilterEnabled\""));
    QVERIFY(transport->replies.empty());
  }
  void capturedExactTargetRejectionsPrecedeCspTransport() {
    for (const auto& vector : uwf::test::captured::exactTargetRejections()) {
      const auto transport = std::make_shared<ScriptedTransport>();
      const auto result = CspUwfBackend(transport).execute(vector.request);
      QVERIFY2(result.failure.has_value(), vector.id);
      QVERIFY(vector.expected.failure);
      QCOMPARE(result.failure->kind, *vector.expected.failure);
      QCOMPARE(result.effect, vector.expected.effect);
      QVERIFY(transport->requests.empty());
      QCOMPARE(transport->availabilityChecks, std::size_t{0});
    }
  }

  void persistenceObservationsHonorTheCapturedSelector_data() {
    QTest::addColumn<int>("property");
    QTest::addColumn<bool>("current");
    QTest::addColumn<bool>("observed");
    const std::array properties{std::pair{BooleanProperty::PersistDomainSecretKey, "domain-secret"},
                                std::pair{BooleanProperty::PersistTscal, "terminal-license"}};
    for (const auto& [property, name] : properties)
      for (const bool current : {false, true})
        for (const bool observed : {false, true})
          QTest::newRow(
              qPrintable(QStringLiteral("%1/%2/%3").arg(QString::fromLatin1(name), current ? QStringLiteral("current") : QStringLiteral("next")).arg(observed)))
              << static_cast<int>(property) << current << observed;
  }
  void persistenceObservationsHonorTheCapturedSelector() {
    QFETCH(int, property);
    QFETCH(bool, current);
    QFETCH(bool, observed);
    const auto persistence = static_cast<BooleanProperty>(property);
    const bool safeObservation = current && persistence == BooleanProperty::PersistDomainSecretKey;
    const auto leaf = persistence == BooleanProperty::PersistDomainSecretKey ? u"PersistDomainSecretKey" : u"PersistTSCAL";
    const auto transport = std::make_shared<ScriptedTransport>();
    transport->replies.push_back({0, valueResponse(current ? u"CurrentSession" : u"NextSession", leaf, u"boolean", observed ? u"-1" : u"0"), {}, true});
    CspUwfBackend backend(transport);
    const GetBoolean request{persistence, current ? Session::Current : Session::Next};
    QCOMPARE(backend.probe(request).support, safeObservation ? Support::Unknown : Support::Unsupported);
    const auto response = backend.execute(request);
    QCOMPARE(response.succeeded(), safeObservation);
    QCOMPARE(response.effect, Effect::None);
    if (safeObservation) {
      QCOMPARE(std::get<bool>(response.value), observed);
      QCOMPARE(transport->requests.size(), std::size_t{1});
      QVERIFY(transport->requests.front().contains(u"type=\"CurrentSession\""));
      QVERIFY(transport->requests.front().contains(u"parm-query name=\"" + QString(leaf) + u"\""));
    } else {
      QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
      QVERIFY(transport->requests.empty());
      QCOMPARE(transport->availabilityChecks, std::size_t{0});
      QCOMPARE(transport->replies.size(), std::size_t{1});
    }
  }

  void booleanWritesDoNotDependOnObservations_data() {
    QTest::addColumn<int>("property");
    QTest::addColumn<bool>("desired");
    for (const auto property : {BooleanProperty::PersistDomainSecretKey, BooleanProperty::PersistTscal, BooleanProperty::ServicingEnabled})
      for (const bool desired : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("property-%1/desired-%2").arg(int(property)).arg(desired))) << int(property) << desired;
  }
  void booleanWritesDoNotDependOnObservations() {
    QFETCH(int, property);
    QFETCH(bool, desired);
    const auto selected = static_cast<BooleanProperty>(property);
    const auto leaf = selected == BooleanProperty::PersistDomainSecretKey ? u"PersistDomainSecretKey"
                      : selected == BooleanProperty::PersistTscal         ? u"PersistTSCAL"
                                                                          : u"ServicingEnabled";
    const auto transport = std::make_shared<ScriptedTransport>();
    const CspUwfBackend backend(transport);
    QCOMPARE(backend.probe(GetBoolean{selected, Session::Next}).support,
             selected == BooleanProperty::ServicingEnabled ? Support::Unknown : Support::Unsupported);
    QCOMPARE(backend.probe(SetBoolean{selected, desired}).support, Support::Unknown);
    const auto result = backend.execute(SetBoolean{selected, desired});
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, Effect::Confirmed);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    const auto& wire = transport->requests.front();
    QVERIFY(wire.contains(u"type=\"NextSession\""));
    QVERIFY(wire.contains(u"name=\"" + QString(leaf) + u"\" datatype=\"boolean\""));
    QVERIFY(wire.contains(desired ? u"value=\"-1\"" : u"value=\"0\""));
    QVERIFY(!wire.contains(u"parm-query"));
    QVERIFY(!wire.contains(u"CurrentSession"));
  }
  void registryMaintenancePreservesTheRequestedTarget_data() {
    QTest::addColumn<QString>("key");
    QTest::addColumn<RegistryMaintenanceOperation>("operation");
    QTest::addColumn<bool>("preservesTarget");
    QTest::addColumn<bool>("loaded");
    const std::array operations{std::pair{"commit-value", RegistryMaintenanceOperation::CommitValue},
                                std::pair{"delete-key", RegistryMaintenanceOperation::DeleteKey},
                                std::pair{"delete-value", RegistryMaintenanceOperation::DeleteValue}};
    const std::array targets{std::pair{"HKLM\\SYSTEM\\CurrentControlSetExtra", false},
                             std::pair{"hkey_local_machine\\system\\currentcontrolsetBackup", false},
                             std::pair{"hKlM\\SYSTEM\\CurrentControlSet ", false},
                             std::pair{"HKLM\\SYSTEM\\CurrentControlSet.", false},
                             std::pair{"hkey_local_machine\\system\\currentcontrolset", true},
                             std::pair{"HKLM\\SYSTEM\\ControlSet001", true},
                             std::pair{"HKLM\\SYSTEM\\ControlSet002", true}};
    for (const auto& [root, preserved] : targets)
      for (const bool descendant : {false, true})
        for (const auto& [operationName, operation] : operations)
          for (const bool loaded : {false, true}) {
            const auto key = QString::fromLatin1(root) + (descendant ? QStringLiteral("\\Child") : QString{});
            QTest::newRow(qPrintable(key + QStringLiteral("/%1-loaded-%2").arg(QString::fromLatin1(operationName)).arg(loaded)))
                << key << operation << preserved << loaded;
          }
  }
  void registryMaintenancePreservesTheRequestedTarget() {
    QFETCH(QString, key);
    QFETCH(RegistryMaintenanceOperation, operation);
    QFETCH(bool, preservesTarget);
    QFETCH(bool, loaded);
    const auto transport = std::make_shared<ScriptedTransport>();
    transport->loaded = loaded;
    transport->replies.push_back({0, QStringLiteral("<untouched/>"), {}, false});
    CspUwfBackend backend(transport);
    const auto name = operation == RegistryMaintenanceOperation::DeleteKey ? std::nullopt : std::optional<std::string>{"Named"};
    const Request request = operation == RegistryMaintenanceOperation::CommitValue ? Request{CommitRegistry{key.toStdString(), name}}
                                                                                   : Request{CommitRegistryDeletion{key.toStdString(), name}};
    const auto capability = backend.probe(request);
    QCOMPARE(capability.support, Support::Unsupported);
    // A safe key retains the existing CSP Exec limitation. An unsafe key is
    // rejected by the shared exact-target contract before any raw provider check.
    if (preservesTarget)
      QVERIFY(capability.reason.find("WAP transport") != std::string::npos);
    else {
      // Shared target rejection must precede provider availability checks.
      const auto failure = requestInputFailure(request);
      QVERIFY(failure);
      QCOMPARE(capability.reason, failure->message);
    }
    const auto result = backend.execute(request);
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, ErrorKind::Unsupported);
    QCOMPARE(result.failure->message, capability.reason);
    QCOMPARE(result.effect, Effect::None);
    QCOMPARE(result.failure->effect, Effect::None);
    QCOMPARE(transport->availabilityChecks, std::size_t{0});
    QVERIFY(transport->requests.empty());
    QVERIFY(transport->filters.empty());
    QCOMPARE(transport->loaded, loaded);
    QCOMPARE(transport->replies.size(), std::size_t{1});
    QCOMPARE(transport->replies.front().xml, QStringLiteral("<untouched/>"));
    QVERIFY(!transport->replies.front().invoked);
  }
  void probingDoesNotInvokeTheProviderOrPretendExecIsReplace() {
    const auto transport = std::make_shared<ScriptedTransport>();
    CspUwfBackend backend(transport);
    for (const Request& request : {Request{CommitFile{fixtureVolume, "\\file"}}, Request{CommitFileDeletion{fixtureVolume, "\\file"}},
                                   Request{CommitRegistry{"HKEY_LOCAL_MACHINE\\SOFTWARE\\Example", "Value"}},
                                   Request{CommitRegistryDeletion{"HKEY_LOCAL_MACHINE\\SOFTWARE\\Example", std::nullopt}}, Request{ResetSettings{}},
                                   Request{Power{PowerAction::Restart}}, Request{GetOverlayFiles{fixtureVolume}}, Request{UpdateWindows{}}}) {
      QCOMPARE(backend.probe(request).support, Support::Unsupported);
      QCOMPARE(backend.execute(request).failure->kind, ErrorKind::Unsupported);
    }
    QCOMPARE(backend.probe(GetNumber{NumberProperty::OverlayFlags}).support, Support::Unknown);
    transport->loaded = false;
    QCOMPARE(backend.probe(GetNumber{NumberProperty::OverlayFlags}).support, Support::Unavailable);
    QVERIFY(transport->requests.empty());
  }

  void globalPropertiesHaveOnePhysicalScope_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<QString>("session");
    QTest::addColumn<QString>("node");
    QTest::newRow("horm") << Request{GetBoolean{BooleanProperty::HormEnabled, Session::Current}} << QStringLiteral("NextSession")
                          << QStringLiteral("HORMEnabled");
    QTest::newRow("shutdown") << Request{GetBoolean{BooleanProperty::ShutdownPending, Session::Next}} << QStringLiteral("CurrentSession")
                              << QStringLiteral("ShutdownPending");
    QTest::newRow("commit") << Request{GetBoolean{BooleanProperty::OverlayCommit, Session::Next}} << QStringLiteral("CurrentSession")
                            << QStringLiteral("OverlayCommit");
    QTest::newRow("warning") << Request{GetNumber{NumberProperty::WarningThreshold, Session::Next}} << QStringLiteral("CurrentSession")
                             << QStringLiteral("WarningOverlayThreshold");
    QTest::newRow("critical") << Request{GetNumber{NumberProperty::CriticalThreshold, Session::Next}} << QStringLiteral("CurrentSession")
                              << QStringLiteral("CriticalOverlayThreshold");
    QTest::newRow("reset") << Request{GetNumber{NumberProperty::ResetPersistentState, Session::Current}} << QStringLiteral("NextSession")
                           << QStringLiteral("ResetPersistentState");
  }
  void globalPropertiesHaveOnePhysicalScope() {
    QFETCH(Request, request);
    QFETCH(QString, session);
    QFETCH(QString, node);
    const auto transport = std::make_shared<ScriptedTransport>();
    const bool boolean = std::holds_alternative<GetBoolean>(request);
    transport->replies.push_back({0, valueResponse(session, node, boolean ? u"boolean" : u"integer", u"1"), {}, true});
    const auto result = CspUwfBackend(transport).execute(request);
    QVERIFY(result.succeeded());
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QVERIFY(transport->requests.back().contains(u"type=\"" + session + u"\""));
    QVERIFY(transport->requests.back().contains(u"parm-query name=\"" + node + u"\""));
  }

  void wireTypesAreNormalizedWithoutLosingFlagBits_data() {
    QTest::addColumn<bool>("boolean");
    QTest::addColumn<QString>("wire");
    QTest::addColumn<quint32>("expected");
    for (const auto value : {u"-1", u"1", u"true", u"TRUE"}) QTest::newRow(qPrintable(QString(u"true-") + value)) << true << QString(value) << quint32{1};
    for (const auto value : {u"0", u"false", u"FALSE"}) QTest::newRow(qPrintable(QString(u"false-") + value)) << true << QString(value) << quint32{0};
    QTest::newRow("unknown-flags") << false << QStringLiteral("2147483650") << quint32{2147483650U};
    QTest::newRow("signed-flags") << false << QStringLiteral("-2147483646") << quint32{2147483650U};
    QTest::newRow("all-bits") << false << QStringLiteral("-1") << std::numeric_limits<quint32>::max();
  }
  void wireTypesAreNormalizedWithoutLosingFlagBits() {
    QFETCH(bool, boolean);
    QFETCH(QString, wire);
    QFETCH(quint32, expected);
    const auto transport = std::make_shared<ScriptedTransport>();
    transport->replies.push_back(
        {0, valueResponse(u"CurrentSession", boolean ? u"FilterEnabled" : u"OverlayFlags", boolean ? u"boolean" : u"integer", wire), {}, true});
    const Request request = boolean ? Request{GetBoolean{BooleanProperty::FilterEnabled}} : Request{GetNumber{NumberProperty::OverlayFlags}};
    const auto result = CspUwfBackend(transport).execute(request);
    QVERIFY(result.succeeded());
    QCOMPARE(boolean ? std::uint32_t{std::get<bool>(result.value)} : std::get<std::uint32_t>(result.value), expected);
  }

  void invalidBooleanWireValuesCannotBecomeState() {
    for (const auto value : {u"", u"-2", u"2", u" true ", u"yes"}) {
      const auto transport = std::make_shared<ScriptedTransport>();
      transport->replies.push_back({0, valueResponse(u"CurrentSession", u"FilterEnabled", u"boolean", value), {}, true});
      const auto result = CspUwfBackend(transport).execute(GetBoolean{BooleanProperty::FilterEnabled});
      QVERIFY(!result.succeeded());
      QCOMPARE(result.failure->kind, ErrorKind::Protocol);
    }
    const auto transport = std::make_shared<ScriptedTransport>();
    transport->replies.push_back({0, valueResponse(u"CurrentSession", u"FilterEnabled", u"integer", u"-1"), {}, true});
    QCOMPARE(CspUwfBackend(transport).execute(GetBoolean{BooleanProperty::FilterEnabled}).failure->kind, ErrorKind::Protocol);
  }

  void numericDomainsDoNotRewriteObservedResetSelectors() {
    const auto transport = std::make_shared<ScriptedTransport>();
    CspUwfBackend backend(transport);
    for (const auto value : {std::uint32_t{255}, std::uint32_t{99}}) {
      transport->replies.push_back({0, valueResponse(u"NextSession", u"ResetPersistentState", u"integer", QString::number(value)), {}, true});
      const auto result = backend.execute(GetNumber{NumberProperty::ResetPersistentState});
      QVERIFY(result.succeeded());
      QCOMPARE(std::get<std::uint32_t>(result.value), value);
    }
    const auto previousCalls = transport->requests.size();
    for (const Request& invalid :
         {Request{SetNumber{NumberProperty::MaximumOverlaySize, 0}}, Request{SetNumber{NumberProperty::ResetPersistentStateSavedMode, 0}},
          Request{SetNumber{NumberProperty::ResetPersistentStateSavedMode, 2}}, Request{SetNumber{NumberProperty::ResetPersistentStateSavedMode, 255}}}) {
      const auto result = backend.execute(invalid);
      QVERIFY(result.failure);
      QCOMPARE(result.failure->kind, ErrorKind::InvalidArgument);
      QCOMPARE(result.effect, Effect::None);
    }
    QCOMPARE(transport->requests.size(), previousCalls);
    QVERIFY(backend.execute(SetNumber{NumberProperty::OverlayFlags, 0x80000001u}).succeeded());
    QVERIFY(transport->requests.back().contains(u"value=\"2147483649\""));
    QVERIFY(backend.execute(SetNumber{NumberProperty::ResetPersistentStateSavedMode, 1}).succeeded());
    QVERIFY(transport->requests.back().contains(u"value=\"1\""));
    QCOMPARE(backend.execute(SetBoolean{BooleanProperty::ShutdownPending, true}).failure->kind, ErrorKind::Unsupported);
    QCOMPARE(transport->requests.size(), previousCalls + 2);
  }

  void savedResetSelectorIsPassedToTheNativeWorkflowWithoutReadback() {
    const auto transport = std::make_shared<ScriptedTransport>();
    const auto result = CspUwfBackend(transport).execute(SetNumber{NumberProperty::ResetPersistentState, 255});
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, Effect::Confirmed);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QVERIFY(transport->requests.front().contains(u"name=\"ResetPersistentState\" datatype=\"integer\" value=\"255\""));
    QVERIFY(!transport->requests.front().contains(u"ResetPersistentStateSavedMode"));
  }
  void savedModeIsPerSessionAndCspExposesOnlyNext() {
    const auto transport = std::make_shared<ScriptedTransport>();
    CspUwfBackend backend(transport);
    const GetNumber current{NumberProperty::ResetPersistentStateSavedMode, Session::Current};
    QCOMPARE(backend.probe(current).support, Support::Unsupported);
    QCOMPARE(backend.execute(current).failure->kind, ErrorKind::Unsupported);
    QVERIFY(transport->requests.empty());
    transport->replies.push_back({0, valueResponse(u"NextSession", u"ResetPersistentStateSavedMode", u"integer", u"1"), {}, true});
    const auto result = backend.execute(GetNumber{NumberProperty::ResetPersistentStateSavedMode, Session::Next});
    QVERIFY(result.succeeded());
    QCOMPARE(std::get<std::uint32_t>(result.value), std::uint32_t{1});
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QVERIFY(transport->requests.front().contains(u"type=\"NextSession\""));
  }

  void typedWritesEncodeDynamicNamesWithoutObservationQueries() {
    const auto transport = std::make_shared<ScriptedTransport>();
    CspUwfBackend backend(transport);
    QVERIFY(backend.execute(SetBoolean{BooleanProperty::FilterEnabled, true}).succeeded());
    QVERIFY(transport->requests.back().contains(u"datatype=\"boolean\" value=\"-1\""));
    const QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\策略 & 100%/x");
    const auto written = backend.execute(EditRegistryExclusion{key.toUtf8().toStdString(), ExclusionAction::Add});
    QVERIFY(written.succeeded());
    QCOMPARE(written.effect, Effect::Confirmed);
    const auto& write = transport->requests.back();
    QVERIFY(write.contains(QString::fromLatin1(QUrl::toPercentEncoding(key))));
    QVERIFY(write.contains(u"策略 &amp; 100%/x"));
    QCOMPARE(transport->requests.size(), std::size_t{2});
    for (const auto& filter : transport->filters) QCOMPARE(filter, QStringList{QStringLiteral("UnifiedWriteFilter")});
  }
  void overlayOptionCompositionBelongsToTheAbstractLayer_data() {
    QTest::addColumn<int>("option");
    for (const auto option : {OverlayOption::Passthrough, OverlayOption::Persistent, OverlayOption::ReadOnlyMedia})
      QTest::newRow(qPrintable(QString::number(static_cast<int>(option)))) << static_cast<int>(option);
  }
  void overlayOptionCompositionBelongsToTheAbstractLayer() {
    QFETCH(int, option);
    const auto transport = std::make_shared<ScriptedTransport>();
    const CspUwfBackend backend(transport);
    const Request request = SetOverlayOption{static_cast<OverlayOption>(option), true};
    QCOMPARE(backend.probe(request).support, Support::Unsupported);
    const auto result = backend.execute(request);
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, ErrorKind::Unsupported);
    QCOMPARE(result.effect, Effect::None);
    QVERIFY(transport->requests.empty());
    QCOMPARE(transport->availabilityChecks, std::size_t{0});
  }
  void malformedInventoryIdentityStopsBeforePropertyQueries() {
    const auto transport = std::make_shared<ScriptedTransport>();
    const QStringList path{QStringLiteral("UnifiedWriteFilter"), QStringLiteral("CurrentSession"), QStringLiteral("Volume"),
                           QStringLiteral("Volume{11111111-2222-3333-4444-555555555555}\\\\")};
    transport->replies.push_back({0, scopedResponse(path), {}, true});
    const auto response = CspUwfBackend(transport).execute(GetVolumes{Session::Current});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Protocol);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QVERIFY(transport->replies.empty());
  }
  void volumeEnumerationDistinguishesOptionalMetadataFromFailures_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<quint32>("letterStatus");
    QTest::addColumn<QString>("letterType");
    QTest::addColumn<QString>("letter");
    QTest::addColumn<quint32>("protectedStatus");
    QTest::addColumn<bool>("success");
    QTest::addColumn<int>("failureKind");
    struct Observation {
      const char* name;
      quint32 letterStatus;
      QString letterType;
      QString letter;
      quint32 protectedStatus;
      bool success;
      ErrorKind failureKind;
    };
    const std::array observations{Observation{"letter", 0, QStringLiteral("string"), QStringLiteral("C:"), 0, true, ErrorKind::Failed},
                                  Observation{"lowercase-letter", 0, QStringLiteral("string"), QStringLiteral("c:"), 0, true, ErrorKind::Failed},
                                  Observation{"empty-letter", 0, QStringLiteral("string"), {}, 0, true, ErrorKind::Failed},
                                  Observation{"path-instead-of-letter", 0, QStringLiteral("string"), QStringLiteral("C:\\bad"), 0, false, ErrorKind::Protocol},
                                  Observation{"missing-letter", 0x80070002, {}, {}, 0, true, ErrorKind::Failed},
                                  Observation{"denied-letter", 0x80070005, {}, {}, 0, false, ErrorKind::AccessDenied},
                                  Observation{"missing-path", 0x80070003, {}, {}, 0, false, ErrorKind::NotFound},
                                  Observation{"missing-csp-node", 0x86000002, {}, {}, 0, false, ErrorKind::NotFound},
                                  Observation{"invalid-letter-type", 0, QStringLiteral("integer"), QStringLiteral("0"), 0, false, ErrorKind::Protocol},
                                  Observation{"missing-required-field", 0x80070002, {}, {}, 0x80070002, false, ErrorKind::NotFound}};
    for (const bool current : {false, true})
      for (const auto& observation : observations)
        QTest::newRow(
            qPrintable(QStringLiteral("%1/%2").arg(current ? QStringLiteral("current") : QStringLiteral("next"), QString::fromLatin1(observation.name))))
            << current << observation.letterStatus << observation.letterType << observation.letter << observation.protectedStatus << observation.success
            << int(observation.failureKind);
  }
  void volumeEnumerationDistinguishesOptionalMetadataFromFailures() {
    QFETCH(bool, current);
    QFETCH(quint32, letterStatus);
    QFETCH(QString, letterType);
    QFETCH(QString, letter);
    QFETCH(quint32, protectedStatus);
    QFETCH(bool, success);
    QFETCH(int, failureKind);
    const std::string id = "11111111-2222-3333-4444-555555555555";
    const QStringList target{QStringLiteral("UnifiedWriteFilter"), current ? QStringLiteral("CurrentSession") : QStringLiteral("NextSession"),
                             QStringLiteral("Volume"), QString::fromStdString(id)};
    const auto transport = std::make_shared<ScriptedTransport>();
    transport->replies.push_back({0, scopedResponse(target), {}, true});
    transport->replies.push_back(
        {letterStatus, letterStatus ? QString{} : scalarResponse(target, QStringLiteral("DriveLetter"), letterType, letter), {}, true});
    if (success || protectedStatus || (!letterStatus && letterType == QStringLiteral("string"))) {
      transport->replies.push_back(
          {protectedStatus,
           protectedStatus ? QString{} : scalarResponse(target, QStringLiteral("Protected"), QStringLiteral("boolean"), QStringLiteral("1")),
           {},
           true});
      if (!protectedStatus)
        transport->replies.push_back(
            {0, scalarResponse(target, QStringLiteral("BindByDriveLetter"), QStringLiteral("boolean"), QStringLiteral("0")), {}, true});
    }
    const auto response = CspUwfBackend(transport).execute(GetVolumes{current ? Session::Current : Session::Next});
    QCOMPARE(response.succeeded(), success);
    QCOMPARE(response.effect, Effect::None);
    QVERIFY(transport->replies.empty());
    if (success) {
      const auto& volumes = std::get<std::vector<Volume>>(response.value);
      QCOMPARE(volumes.size(), std::size_t{1});
      QCOMPARE(volumes.front().id, id);
      QCOMPARE(volumes.front().driveLetter, letter.toUpper().toStdString());
      QVERIFY(volumes.front().protectedVolume);
      QVERIFY(!volumes.front().bindByDriveLetter);
    } else {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, static_cast<ErrorKind>(failureKind));
    }
  }

  void volumePropertyReadsUseOnlyTheRequestedLeaf_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<quint32>("error");
    QTest::newRow("current-value") << true << quint32{0};
    QTest::newRow("next-value") << false << quint32{0};
    QTest::newRow("missing-csp") << false << quint32{0x86000002U};
    QTest::newRow("missing-element") << false << quint32{0x80070490U};
    QTest::newRow("denied") << false << quint32{0x80070005U};
  }
  void volumePropertyReadsUseOnlyTheRequestedLeaf() {
    QFETCH(bool, current);
    QFETCH(quint32, error);
    const std::string id = "11111111-2222-3333-4444-555555555555";
    const auto session = current ? Session::Current : Session::Next;
    const QStringList path{QStringLiteral("UnifiedWriteFilter"), current ? QStringLiteral("CurrentSession") : QStringLiteral("NextSession"),
                           QStringLiteral("Volume"), QString::fromStdString(id)};
    const auto transport = std::make_shared<ScriptedTransport>();
    transport->replies.push_back({error, error ? QString{} : scopedResponse(path, QStringLiteral("Protected"), QStringLiteral("boolean")), {}, true});
    const auto result = CspUwfBackend(transport).execute(GetVolumeProperty{id, VolumeProperty::Protected, session});
    QCOMPARE(result.succeeded(), error == 0);
    QCOMPARE(result.effect, Effect::None);
    if (error)
      QCOMPARE(result.failure->kind, error == 0x80070005U ? ErrorKind::AccessDenied : ErrorKind::NotFound);
    else
      QCOMPARE(std::get<bool>(result.value), false);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QVERIFY(transport->requests.front().contains(u"parm-query name=\"Protected\""));
    QVERIFY(!transport->requests.front().contains(u"DriveLetter"));
    QVERIFY(!transport->requests.front().contains(u"characteristic-query"));
  }
  void volumePropertyFailuresKeepTheOriginalLeafError_data() {
    QTest::addColumn<bool>("inventory");
    QTest::addColumn<int>("property");
    QTest::addColumn<quint32>("status");
    QTest::addColumn<int>("kind");
    for (const bool inventory : {false, true})
      for (const auto property : {VolumeProperty::Protected, VolumeProperty::BindByDriveLetter})
        for (const auto [status, kind] : {std::pair{0x86000002u, ErrorKind::NotFound}, std::pair{0x80070002u, ErrorKind::NotFound},
                                          std::pair{0x80070005u, ErrorKind::AccessDenied}, std::pair{0x8007000du, ErrorKind::Protocol}})
          QTest::newRow(qPrintable(QStringLiteral("%1/%2/%3").arg(inventory).arg(int(property)).arg(status)))
              << inventory << int(property) << quint32{status} << int(kind);
  }
  void volumePropertyFailuresKeepTheOriginalLeafError() {
    QFETCH(bool, inventory);
    QFETCH(int, property);
    QFETCH(quint32, status);
    QFETCH(int, kind);
    const auto transport = std::make_shared<ScriptedTransport>();
    const QStringList target{QStringLiteral("UnifiedWriteFilter"), QStringLiteral("NextSession"), QStringLiteral("Volume"), QString::fromLatin1(fixtureVolume)};
    if (inventory) {
      transport->replies.push_back({0, scopedResponse(target), {}, true});
      transport->replies.push_back({0, scalarResponse(target, QStringLiteral("DriveLetter"), QStringLiteral("string"), QStringLiteral("C:")), {}, true});
      if (static_cast<VolumeProperty>(property) == VolumeProperty::BindByDriveLetter)
        transport->replies.push_back({0, scalarResponse(target, QStringLiteral("Protected"), QStringLiteral("boolean"), QStringLiteral("1")), {}, true});
    }
    const auto count = transport->replies.size() + 1;
    transport->replies.push_back({status, {}, QStringLiteral("original property lookup failure"), true});
    const Request request =
        inventory ? Request{GetVolumes{Session::Next}} : Request{GetVolumeProperty{fixtureVolume, static_cast<VolumeProperty>(property), Session::Next}};
    const auto response = CspUwfBackend(transport).execute(request);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, static_cast<ErrorKind>(kind));
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(transport->requests.size(), count);
    QVERIFY(transport->replies.empty());
    QVERIFY(transport->requests.back().contains(u"parm-query name="));
    QCOMPARE(response.failure->diagnostics.front().code, std::int64_t{status});
    QVERIFY(response.failure->diagnostics.front().detail.find("original property lookup failure") != std::string::npos);
  }
  void rawMutationsSubmitOnlyTheirOwnCapability_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<QString>("wire");
    const auto add = [](const char* name, Request request, const QString& wire) { QTest::newRow(name) << request << wire; };
    add("filter", SetBoolean{BooleanProperty::FilterEnabled, true}, QStringLiteral("name=\"FilterEnabled\""));
    add("horm", SetBoolean{BooleanProperty::HormEnabled, true}, QStringLiteral("name=\"HORMEnabled\""));
    add("servicing", SetBoolean{BooleanProperty::ServicingEnabled, true}, QStringLiteral("name=\"ServicingEnabled\""));
    add("domain-secret", SetBoolean{BooleanProperty::PersistDomainSecretKey, true}, QStringLiteral("name=\"PersistDomainSecretKey\""));
    add("terminal-license", SetBoolean{BooleanProperty::PersistTscal, true}, QStringLiteral("name=\"PersistTSCAL\""));
    add("full-commit", SetBoolean{BooleanProperty::OverlayCommit, true}, QStringLiteral("name=\"OverlayCommit\""));
    for (const auto property :
         {NumberProperty::OverlayType, NumberProperty::MaximumOverlaySize, NumberProperty::WarningThreshold, NumberProperty::CriticalThreshold,
          NumberProperty::OverlayFlags, NumberProperty::ResetPersistentState, NumberProperty::ResetPersistentStateSavedMode})
      add(qPrintable(QStringLiteral("number-%1").arg(int(property))), SetNumber{property, property == NumberProperty::MaximumOverlaySize ? 2048u : 1u},
          QStringLiteral("<parm name="));
    add("volume-add", AddVolume{fixtureVolume, {}}, QStringLiteral("<characteristic type=\"") + QString::fromLatin1(fixtureVolume));
    add("volume-remove", RemoveVolume{fixtureVolume}, QStringLiteral("<nocharacteristic type=\"") + QString::fromLatin1(fixtureVolume));
    add("protection", SetVolume{fixtureVolume, VolumeProperty::Protected, true}, QStringLiteral("name=\"Protected\""));
    add("binding", SetVolume{fixtureVolume, VolumeProperty::BindByDriveLetter, true}, QStringLiteral("name=\"BindByDriveLetter\""));
    add("file-add", EditFileExclusion{fixtureVolume, "\\Entry", ExclusionAction::Add}, QStringLiteral("<parm name="));
    add("file-remove", EditFileExclusion{fixtureVolume, "\\Entry", ExclusionAction::Remove}, QStringLiteral("<noparm name="));
    add("registry-add", EditRegistryExclusion{"HKLM\\Software\\Fixture", ExclusionAction::Add}, QStringLiteral("<parm name="));
    add("registry-remove", EditRegistryExclusion{"HKLM\\Software\\Fixture", ExclusionAction::Remove}, QStringLiteral("<noparm name="));
    add("swapfile", CreateSwapfile{fixtureVolume}, QStringLiteral("name=\"SwapfileSize\" datatype=\"integer\" value=\"1\""));
  }
  void rawMutationsSubmitOnlyTheirOwnCapability() {
    QFETCH(Request, request);
    QFETCH(QString, wire);
    const auto transport = std::make_shared<ScriptedTransport>();
    const auto result = CspUwfBackend(transport).execute(request);
    QVERIFY2(result.succeeded(), result.failure ? result.failure->message.c_str() : "raw command failed");
    QCOMPARE(result.effect, Effect::Confirmed);
    QCOMPARE(transport->requests.size(), std::size_t{1});
    QVERIFY(transport->requests.front().contains(wire));
    QVERIFY(!transport->requests.front().contains(u"parm-query"));
    QVERIFY(!transport->requests.front().contains(u"characteristic-query"));
  }
  void nativeAndProviderDiagnosticsRemainIndependent() {
    const auto transport = std::make_shared<ScriptedTransport>();
    const QString xml = QStringLiteral(
        "<wap-provisioningdoc><characteristic type='UnifiedWriteFilter'><characteristic type='NextSession'>"
        "<parm-error name='OverlayFlags' hresult='-2147024891'/></characteristic></characteristic></wap-provisioningdoc>");
    transport->replies.push_back({0x80004005, xml, QStringLiteral("provider condition detail"), true});
    auto result = CspUwfBackend(transport).execute(SetNumber{NumberProperty::OverlayFlags, flags::readOnlyMedia});
    QVERIFY(!result.succeeded());
    QCOMPARE(result.failure->kind, ErrorKind::AccessDenied);
    QCOMPARE(result.effect, Effect::Uncertain);
    QCOMPARE(result.failure->diagnostics.size(), std::size_t{2});
    QCOMPARE(result.failure->diagnostics[0].code, std::int64_t{0x80004005});
    QCOMPARE(result.failure->diagnostics[1].code, std::int64_t{0x80070005});
    QVERIFY(QString::fromStdString(result.failure->diagnostics[0].detail).contains(u"provider condition detail"));
    QVERIFY(QString::fromStdString(result.failure->diagnostics[1].detail).contains(xml));
    transport->replies.push_back({0x8007007e, {}, {}, false});
    result = CspUwfBackend(transport).execute(SetNumber{NumberProperty::OverlayFlags, flags::readOnlyMedia});
    QCOMPARE(result.effect, Effect::None);
    QCOMPARE(result.failure->kind, ErrorKind::Unavailable);
  }

  void providerErrorsOverrideEchoesEvenWhenTheNativeCallSucceeded() {
    const auto transport = std::make_shared<ScriptedTransport>();
    const QString xml = QStringLiteral(
        "<wap-provisioningdoc><characteristic type='UnifiedWriteFilter'><characteristic type='NextSession'>"
        "<parm name='OverlayFlags' datatype='integer' value='1'/>"
        "<parm-error name='OverlayFlags' hresult='0x80070005'/></characteristic></characteristic></wap-provisioningdoc>");
    transport->replies.push_back({0, xml, QStringLiteral("write condition detail"), true});
    const auto result = CspUwfBackend(transport).execute(SetNumber{NumberProperty::OverlayFlags, flags::readOnlyMedia});
    QVERIFY(!result.succeeded());
    QCOMPARE(result.failure->kind, ErrorKind::AccessDenied);
    QCOMPARE(result.effect, Effect::Uncertain);
    QCOMPARE(result.failure->diagnostics.front().code, std::int64_t{0});
    QCOMPARE(result.failure->diagnostics.back().code, std::int64_t{0x80070005});
    QVERIFY(QString::fromStdString(result.failure->diagnostics.back().detail).contains(xml));
  }

  void anUnreportedProviderHresultIsNotFabricatedAsEvidence() {
    for (const auto& attribute : {QString{}, QStringLiteral(" hresult='invalid'"), QStringLiteral(" hresult='0'")}) {
      const auto xml = QStringLiteral("<wap-provisioningdoc><parm-error name='OverlayFlags'") + attribute + QStringLiteral("/></wap-provisioningdoc>");
      const auto document = csp::decodeWapDocument(xml);
      QCOMPARE(document.errors.size(), std::size_t{1});
      QVERIFY(!document.errors.front().code);
      for (const auto nativeCode : {std::uint32_t{0}, std::uint32_t{0x80070005}}) {
        const auto transport = std::make_shared<ScriptedTransport>();
        transport->replies.push_back({nativeCode, xml, {}, true});
        const auto result = CspUwfBackend(transport).execute(SetNumber{NumberProperty::OverlayFlags, flags::readOnlyMedia});
        QVERIFY(!result.succeeded());
        QCOMPARE(result.effect, Effect::Uncertain);
        QCOMPARE(result.failure->kind, nativeCode ? ErrorKind::AccessDenied : ErrorKind::Failed);
        QCOMPARE(result.failure->diagnostics.back().code, std::int64_t{0});
        QVERIFY(QString::fromStdString(result.failure->diagnostics.back().detail).contains(u"did not report a valid HRESULT"));
        if (nativeCode) QCOMPARE(result.failure->diagnostics.front().code, std::int64_t{nativeCode});
      }
    }
  }

  void xmlCommandEncodingPreservesEveryRepresentableAttribute_data() {
    QTest::addColumn<WapAttribute>("attribute");
    QTest::addColumn<bool>("writing");
    QTest::addColumn<QString>("text");
    QTest::addColumn<bool>("representable");
    const auto add = [](const QString& name, const QString& fragment, const bool representable) {
      const std::array attributes{std::pair{WapAttribute::Parent, "parent"}, std::pair{WapAttribute::Name, "name"}, std::pair{WapAttribute::Value, "value"}};
      for (const auto& [attribute, label] : attributes)
        for (const bool writing : {false, true}) {
          if (!writing && attribute == WapAttribute::Value) continue;
          QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2/%3").arg(writing ? "write" : "read", label, name)))
              << attribute << writing << (u"Before" + fragment + u"After") << representable;
        }
    };
    for (char16_t character = 0; character < 0x20; ++character)
      add(QStringLiteral("control-%1").arg(static_cast<qulonglong>(character), 2, 16, QLatin1Char('0')), QString(QChar(character)),
          character == u'\t' || character == u'\n' || character == u'\r');
    add(QStringLiteral("escaped-metacharacters"), QStringLiteral("<&\"'>"), true);
    add(QStringLiteral("delete-character"), QString(QChar(u'\x7f')), true);
    add(QStringLiteral("supplementary"), QString::fromUcs4(U"\U0001F600"), true);
    add(QStringLiteral("unpaired-high-surrogate"), QString(QChar(u'\xd800')), false);
    add(QStringLiteral("unpaired-low-surrogate"), QString(QChar(u'\xdc00')), false);
    add(QStringLiteral("noncharacter-fffe"), QString(QChar(u'\xfffe')), false);
    add(QStringLiteral("noncharacter-ffff"), QString(QChar(u'\xffff')), false);
  }
  void xmlCommandEncodingPreservesEveryRepresentableAttribute() {
    QFETCH(WapAttribute, attribute);
    QFETCH(bool, writing);
    QFETCH(QString, text);
    QFETCH(bool, representable);
    csp::WapCommand command{writing ? csp::WapAction::PutValue : csp::WapAction::GetValue,
                            {QStringLiteral("Root")},
                            QStringLiteral("Leaf"),
                            writing ? std::optional{csp::WapValue{QStringLiteral("string"), QStringLiteral("Original")}} : std::nullopt};
    switch (attribute) {
      case WapAttribute::Parent:
        command.parents.front() = text;
        break;
      case WapAttribute::Name:
        command.name = text;
        break;
      case WapAttribute::Value:
        command.value->text = text;
        break;
    }
    if (!representable) {
      QVERIFY_THROWS_EXCEPTION(csp::WapEncodingError, (void)csp::encodeWapCommand(command));
      return;
    }
    const auto xml = csp::encodeWapCommand(command);
    QXmlStreamReader reader(xml);
    QStringList parents;
    QString leafElement, leafName, datatype, value;
    while (!reader.atEnd()) {
      reader.readNext();
      if (!reader.isStartElement()) continue;
      if (reader.name() == u"characteristic")
        parents.append(reader.attributes().value(u"type").toString());
      else if (reader.name() == u"parm" || reader.name() == u"parm-query") {
        leafElement = reader.name().toString();
        leafName = reader.attributes().value(u"name").toString();
        datatype = reader.attributes().value(u"datatype").toString();
        value = reader.attributes().value(u"value").toString();
      }
    }
    QVERIFY(!reader.hasError());
    QCOMPARE(parents, command.parents);
    QCOMPARE(leafElement, writing ? QStringLiteral("parm") : QStringLiteral("parm-query"));
    QCOMPARE(leafName, command.name);
    if (writing) {
      QCOMPARE(datatype, command.value->datatype);
      QCOMPARE(value, command.value->text);
    }
  }

  void xmlInexpressibleRegistryAddFallsBackWithoutChangingTheTarget_data() {
    QTest::addColumn<QString>("key");
    for (const auto character : {u'\x1', u'\xb', u'\x1f', u'\xfffe', u'\xffff'})
      QTest::newRow(qUtf8Printable(QStringLiteral("codepoint-%1").arg(static_cast<qulonglong>(character), 4, 16, QLatin1Char('0'))))
          << (QStringLiteral("HKLM\\SOFTWARE\\Entry") + QChar(character) + QStringLiteral("Suffix"));
  }
  void xmlInexpressibleRegistryAddFallsBackWithoutChangingTheTarget() {
    QFETCH(QString, key);
    const QString sibling = QStringLiteral("HKLM\\SOFTWARE\\Entry");
    const Request request = EditRegistryExclusion{key.toStdString(), ExclusionAction::Add};
    const auto transport = std::make_shared<ScriptedTransport>();
    const auto csp = std::make_shared<CspUwfBackend>(transport);
    using uwf::simulation::FakeUwfBackend;
    uwf::simulation::FakeUwfBackend::State initial;
    initial.next.registryExclusions = {sibling.toStdString()};
    const auto simulated = uwf::simulation::FakeUwfBackend::createSystem(initial);
    const auto first = std::make_shared<uwf::simulation::FakeUwfBackend>(simulated, "unavailable-first");
    first->setCapability(request, {Support::Unavailable, "The first backend cannot execute this operation"});
    const auto nativeCapable = std::make_shared<uwf::simulation::FakeUwfBackend>(simulated, "native-capable");
    const FallbackUwf system({first, csp, nativeCapable});
    const auto response = system.execute(request);
    QVERIFY(response.succeeded());
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(response.attempts.size(), std::size_t{3});
    QCOMPARE(response.attempts[0].capability.support, Support::Unavailable);
    QVERIFY(response.attempts[1].failure);
    QCOMPARE(response.attempts[1].failure->kind, ErrorKind::Unsupported);
    QCOMPARE(response.attempts[1].failure->effect, Effect::None);
    QCOMPARE(response.attempts[2].backend, std::string("native-capable"));
    QVERIFY(first->trace().empty());
    const auto trace = nativeCapable->trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(trace.front().request == request);
    auto expected = initial;
    expected.next.registryExclusions.push_back(key.toStdString());
    QVERIFY(nativeCapable->snapshot() == expected);
    QCOMPARE(call(*nativeCapable, FindRegistryExclusion{key.toStdString(), Session::Next}), true);
    QCOMPARE(call(*nativeCapable, FindRegistryExclusion{sibling.toStdString(), Session::Next}), true);
    QVERIFY(transport->requests.empty());
    QVERIFY(transport->replies.empty());
  }

  void invalidReadResponsesAreNeverUsableState_data() {
    QTest::addColumn<QString>("xml");
    QTest::newRow("empty") << QString{};
    QTest::newRow("malformed") << QStringLiteral("<broken");
    QTest::newRow("wrong-protocol") << QStringLiteral("<SyncML/>");
    QTest::newRow("missing-node") << QStringLiteral("<wap-provisioningdoc/>");
    QTest::newRow("wrong-type") << valueResponse(u"CurrentSession", u"OverlayFlags", u"boolean", u"1");
    QTest::newRow("wrong-session") << valueResponse(u"NextSession", u"OverlayFlags", u"integer", u"1");
    QTest::newRow("wrong-node") << valueResponse(u"CurrentSession", u"Other", u"integer", u"1");
    QTest::newRow("overflow") << valueResponse(u"CurrentSession", u"OverlayFlags", u"integer", u"4294967296");
    QTest::newRow("signed-overflow") << valueResponse(u"CurrentSession", u"OverlayFlags", u"integer", u"-2147483649");
    QTest::newRow("nondecimal") << valueResponse(u"CurrentSession", u"OverlayFlags", u"integer", u"0x1");
    QTest::newRow("trailing-text") << valueResponse(u"CurrentSession", u"OverlayFlags", u"integer", u"1x");
    QTest::newRow("plus-sign") << valueResponse(u"CurrentSession", u"OverlayFlags", u"integer", u"+1");
    QTest::newRow("empty-number") << valueResponse(u"CurrentSession", u"OverlayFlags", u"integer", u"");
    QTest::newRow("whitespace") << valueResponse(u"CurrentSession", u"OverlayFlags", u"integer", u" 1");
    QTest::newRow("entity") << QStringLiteral("<!DOCTYPE wap-provisioningdoc [<!ENTITY x '1'>]>") +
                                   valueResponse(u"CurrentSession", u"OverlayFlags", u"integer", u"1");
    auto duplicate = valueResponse(u"CurrentSession", u"OverlayFlags", u"integer", u"1");
    duplicate.replace(QStringLiteral("</characteristic></characteristic>"),
                      QStringLiteral("<parm name='OverlayFlags' datatype='integer' value='2'/></characteristic></characteristic>"));
    QTest::newRow("duplicate") << duplicate;
  }
  void invalidReadResponsesAreNeverUsableState() {
    QFETCH(QString, xml);
    const auto transport = std::make_shared<ScriptedTransport>();
    transport->replies.push_back({0, xml, {}, true});
    const auto result = CspUwfBackend(transport).execute(GetNumber{NumberProperty::OverlayFlags});
    QVERIFY(!result.succeeded());
    QCOMPARE(result.failure->kind, ErrorKind::Protocol);
    QCOMPARE(result.effect, Effect::None);
  }

  void cancellationBeforeInvocationHasNoEffect() {
    const auto transport = std::make_shared<ScriptedTransport>();
    std::stop_source stop;
    stop.request_stop();
    const auto result = CspUwfBackend(transport).execute(SetNumber{NumberProperty::OverlayFlags, flags::readOnlyMedia}, {stop.get_token()});
    QCOMPARE(result.failure->kind, ErrorKind::Canceled);
    QCOMPARE(result.effect, Effect::None);
    QVERIFY(transport->requests.empty());
  }
};
}  // namespace

QTEST_APPLESS_MAIN(CspUwfBackendTests)
#include "CspUwfBackendTests.moc"
