// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "infrastructure/uwf/routing/FallbackUwf.h"
#include "infrastructure/uwf/windows/wmi/WmiUwfBackend.h"
#include "infrastructure/windows/management/wmi/WmiError.h"
#include "infrastructure/windows/management/wmi/WmiException.h"
#include "support/uwf/CapturedUwfContractVectors.h"

Q_DECLARE_METATYPE(uwf::compat::Request)
Q_DECLARE_METATYPE(uwf::compat::Value)

namespace {
using namespace uwf;
using namespace uwf::compat;
using windows::backends::WmiUwfBackend;
using windows::wmi::WmiMethodOutput;
using windows::wmi::WmiOperations;
using windows::wmi::WmiRow;
using WireValue = windows::wmi::WmiValue;
constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";
constexpr auto volumeName = "Volume{11111111-2222-3333-4444-555555555555}";
constexpr auto registryKey = "HKEY_LOCAL_MACHINE\\Software\\Fixture";

WmiRow volumeRow(bool current, bool protectedVolume = false) {
  return {{"__PATH", WireValue::fromString(current ? "volume-current" : "volume-next")},
          {"CurrentSession", WireValue::fromBool(current)},
          {"DriveLetter", WireValue::fromString("C:")},
          {"VolumeName", WireValue::fromString(volumeName)},
          {"Protected", WireValue::fromBool(protectedVolume)},
          {"BindByDriveLetter", WireValue::fromBool(false)},
          {"CommitPending", WireValue::fromBool(false)}};
}
WmiRow registryRow(bool current) {
  return {{"__PATH", WireValue::fromString(current ? "registry-current" : "registry-next")},
          {"CurrentSession", WireValue::fromBool(current)},
          {"PersistDomainSecretKey", WireValue::fromBool(false)},
          {"PersistTSCAL", WireValue::fromBool(false)}};
}

// Record WMI rows and methods; shared command code owns admission, and uwf::simulation::FakeUwfBackend owns stateful simulation.
class Provider final : public WmiOperations {
 public:
  Provider() {
    tables["UWF_Filter"] = {{{"__PATH", WireValue::fromString("filter")},
                             {"CurrentEnabled", WireValue::fromBool(false)},
                             {"NextEnabled", WireValue::fromBool(true)},
                             {"HORMEnabled", WireValue::fromBool(false)},
                             {"ShutdownPending", WireValue::fromBool(false)}}};
    for (bool current : {true, false}) {
      tables["UWF_OverlayConfig"].push_back({{"__PATH", WireValue::fromString(current ? "config-current" : "config-next")},
                                             {"CurrentSession", WireValue::fromBool(current)},
                                             {"Type", WireValue::fromUInt(0)},
                                             {"MaximumSize", WireValue::fromUInt(2048)}});
      tables["UWF_RegistryFilter"].push_back(registryRow(current));
      tables["UWF_Servicing"].push_back({{"__PATH", WireValue::fromString(current ? "servicing-current" : "servicing-next")},
                                         {"CurrentSession", WireValue::fromBool(current)},
                                         {"ServicingEnabled", WireValue::fromBool(false)}});
      tables["UWF_Volume"].push_back(volumeRow(current, current));
    }
    tables["UWF_Overlay"] = {{{"__PATH", WireValue::fromString("overlay")},
                              {"OverlayConsumption", WireValue::fromUInt(7)},
                              {"AvailableSpace", WireValue::fromUInt(93)},
                              {"WarningOverlayThreshold", WireValue::fromUInt(64)},
                              {"CriticalOverlayThreshold", WireValue::fromUInt(128)}}};
  }
  mutable std::map<std::string, std::vector<WmiRow>> tables;
  mutable std::map<std::string, std::vector<std::string>> exclusions;
  mutable std::vector<std::string> methods, paths;
  mutable std::vector<std::string> readMethods;
  mutable std::vector<WmiRow> arguments;
  mutable std::vector<std::string> queries;
  mutable std::vector<windows::wmi::WmiPutMode> putModes;
  mutable int inventoryReads = 0;
  mutable int reads = 0;
  mutable int connections = 0, classProbes = 0;
  std::set<std::string> missingClasses;
  mutable int postWriteReads = 0;
  bool mutate = true;
  mutable ServicingAdmissionFacts servicingState{0, std::nullopt};
  bool servicingRestoreSucceeds = true;
  mutable std::exception_ptr queryFailure, invocationFailure;
  std::exception_ptr exactFailure;
  std::map<std::string, WmiRow> exactRows;
  std::set<std::string> omittedVolumePaths;
  mutable std::vector<std::string> objectReads;
  std::function<void()> afterWrite;
  std::function<void()> afterQuery;
  WmiMethodOutput writeOutput, filesOutput;
  std::exception_ptr singleReadFailure;
  mutable int singleReads = 0;

  WmiRow& row(const std::string& path) const {
    for (auto& [name, values] : tables) {
      (void)name;
      for (auto& value : values)
        if (value.at("__PATH").toString() == path) return value;
    }
    throw windows::wmi::WmiInfrastructureError(static_cast<std::int32_t>(windows::wmi::WmiErrorCode::NotFound), "find fixture row");
  }
  WmiUwfBackend backend() { return WmiUwfBackend(*this); }
  void ensureConnected() const override { ++connections; }
  windows::wmi::WmiClassStatus classStatus(const std::string& name) const override {
    ++classProbes;
    return missingClasses.contains(name) ? windows::wmi::WmiClassStatus::Missing : windows::wmi::WmiClassStatus::Present;
  }
  std::vector<WmiRow> query(const std::string& query) const override { return queryInstances(query); }
  std::vector<WmiRow> queryInstances(const std::string& query) const override {
    ++reads;
    queries.push_back(query);
    if (!methods.empty()) ++postWriteReads;
    if (queryFailure) std::rethrow_exception(queryFailure);
    const auto name = query.substr(std::string("SELECT * FROM ").size());
    if (name == "UWF_Volume") ++inventoryReads;
    auto rows = tables.at(name);
    if (name == "UWF_Volume") std::erase_if(rows, [this](const WmiRow& row) { return omittedVolumePaths.contains(row.at("__PATH").toString()); });
    if (afterQuery) afterQuery();
    return rows;
  }
  WmiRow getObject(const std::string& path) const override {
    objectReads.push_back(path);
    if (!methods.empty()) ++postWriteReads;
    if (queryFailure) std::rethrow_exception(queryFailure);
    if (path.starts_with("UWF_Volume.")) {
      if (exactFailure) std::rethrow_exception(exactFailure);
      if (const auto found = exactRows.find(path); found != exactRows.end()) return found->second;
      const bool current = path.find("CurrentSession=TRUE,") != std::string::npos;
      const auto expected = std::string("UWF_Volume.CurrentSession=") + (current ? "TRUE" : "FALSE") + ",DriveLetter=\"\",VolumeName=\"" + volumeName + '"';
      if (path != expected) throw std::logic_error("Unexpected exact volume lookup: " + path);
      return row(current ? "volume-current" : "volume-next");
    }
    return row(path);
  }
  WmiMethodOutput invokeMethod(const std::string& path, const std::string& method, const WmiRow& inputs) const override {
    paths.push_back(path);
    methods.push_back(method);
    arguments.push_back(inputs);
    if (invocationFailure) std::rethrow_exception(std::exchange(invocationFailure, std::exception_ptr{}));
    if (mutate) {
      if (method == "PutInstance" && path == "UWF_Volume") {
        auto created = volumeRow(false, true);  // Native WMI default differs unless explicitly supplied.
        for (const auto& [key, value] : inputs) created[key] = value;
        tables["UWF_Volume"].push_back(std::move(created));
      } else if (method == "PutInstance") {
        auto& target = row("registry-next");
        for (const auto& [key, value] : inputs) target[key] = value;
      } else if (method == "Enable" || method == "Disable") {
        row(path)[path == "filter" ? "NextEnabled" : "ServicingEnabled"] = WireValue::fromBool(method == "Enable");
        if (path.starts_with("servicing")) {
          const auto before = servicingState.status;
          if (method == "Enable" && (before == 0 || before == 3)) {
            servicingState.savedFilter = row("filter").at("NextEnabled").toBool();
            row("filter")["NextEnabled"] = WireValue::fromBool(false);
            servicingState.status = 1;
          } else if (method == "Disable" && (before == 1 || before == 3)) {
            if (servicingRestoreSucceeds && servicingState.savedFilter) {
              if (*servicingState.savedFilter) row("filter")["NextEnabled"] = WireValue::fromBool(true);
              servicingState.status = 2;
            } else {
              // The captured final SetStatusEx receipt can be S_OK even when
              // restoration failed. Record the actual Failed runtime state.
              servicingState.status = 3;
            }
          }
        }
      } else if (method == "EnableHORM" || method == "DisableHORM") {
        row(path)["HORMEnabled"] = WireValue::fromBool(method == "EnableHORM");
      } else if (method == "Protect" || method == "Unprotect") {
        row(path)["Protected"] = WireValue::fromBool(method == "Protect");
      } else if (method == "SetBindByDriveLetter") {
        row(path)["BindByDriveLetter"] = inputs.at("bBindByDriveLetter");
      } else if (method == "SetType") {
        row(path)["Type"] = inputs.at("type");
      } else if (method == "SetMaximumSize") {
        row(path)["MaximumSize"] = inputs.at("size");
      } else if (method == "SetWarningThreshold" || method == "SetCriticalThreshold") {
        row(path)[method == "SetWarningThreshold" ? "WarningOverlayThreshold" : "CriticalOverlayThreshold"] = inputs.at("size");
      } else if (method == "AddExclusion") {
        exclusions[path].push_back(inputs.at(path.starts_with("registry") ? "RegistryKey" : "FileName").toString());
      } else if (method == "RemoveExclusion") {
        std::erase(exclusions[path], inputs.at(path.starts_with("registry") ? "RegistryKey" : "FileName").toString());
      }
    }
    if (afterWrite) afterWrite();
    return writeOutput;
  }
  WmiMethodOutput callMethodRead(const std::string& path, const std::string& method, const WmiRow&) const override {
    readMethods.push_back(method);
    WmiMethodOutput output;
    if (method == "GetExclusions") {
      const bool registry = path.starts_with("registry");
      auto& values = output.arrays[registry ? "ExcludedKeys" : "ExcludedFiles"];
      for (const auto& entry : exclusions[path]) values.push_back({{registry ? "RegistryKey" : "FileName", WireValue::fromString(entry)}});
    } else if (method == "GetOverlayFiles")
      return filesOutput;
    else
      throw std::logic_error("Unexpected fixture read method");
    return output;
  }
  WmiMethodOutput callMethodReadOnce(const std::string& path, const std::string& method, const WmiRow& inputs, std::stop_token) const override {
    ++singleReads;
    if (singleReadFailure) std::rethrow_exception(singleReadFailure);
    return callMethodRead(path, method, inputs);
  }
  void putInstance(const std::string& name, const WmiRow& props, windows::wmi::WmiPutMode mode) const override {
    putModes.push_back(mode);
    (void)invokeMethod(name, "PutInstance", props);
  }
};

class AcceptingBackend final : public Backend {
 public:
  mutable int writes = 0;
  std::string name() const override { return "fallback-fixture"; }
  Capability probe(const Request&) const override { return {Support::Supported, {}}; }
  Response execute(const Request&, const Context&) const override {
    ++writes;
    return Response::success(std::monostate{}, Effect::Confirmed);
  }
};
enum class Fault { None, BeforeRead, Provider, MissingMethod, LostAfterMutation, UnobservedSuccess, UnknownAfterMutation, Readback };
enum class BooleanObservation { False, Null, Missing, WrongType };
void setBooleanObservation(WmiRow& row, const std::string& field, const BooleanObservation observation) {
  switch (observation) {
    case BooleanObservation::False:
      row[field] = WireValue::fromBool(false);
      break;
    case BooleanObservation::Null:
      row[field] = WireValue{};
      break;
    case BooleanObservation::Missing:
      row.erase(field);
      break;
    case BooleanObservation::WrongType:
      row[field] = WireValue::fromString("false");
      break;
  }
}
}  // namespace

class WmiUwfBackendTests final : public QObject {
  Q_OBJECT
  enum class RegistryMaintenanceOperation { CommitValue, DeleteKey, DeleteValue };
  Q_ENUM(RegistryMaintenanceOperation)
 private slots:
  void mappedCommandReceiptsKeepTheirSpecificWireTarget_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<QString>("target");
    QTest::addColumn<QString>("method");
    QTest::addColumn<QString>("parameter");
    QTest::addColumn<QString>("value");
    QTest::newRow("critical-threshold") << Request{SetNumber{NumberProperty::CriticalThreshold, 256}} << QStringLiteral("overlay")
                                        << QStringLiteral("SetCriticalThreshold") << QStringLiteral("size") << QStringLiteral("256");
    QTest::newRow("reset-settings") << Request{ResetSettings{}} << QStringLiteral("filter") << QStringLiteral("ResetSettings") << QString{} << QString{};
    QTest::newRow("shutdown") << Request{Power{PowerAction::Shutdown}} << QStringLiteral("filter") << QStringLiteral("ShutdownSystem") << QString{}
                              << QString{};
    QTest::newRow("restart") << Request{Power{PowerAction::Restart}} << QStringLiteral("filter") << QStringLiteral("RestartSystem") << QString{} << QString{};
    for (const auto action : {ExclusionAction::Add, ExclusionAction::Remove}) {
      const auto method = action == ExclusionAction::Add ? QStringLiteral("AddExclusion") : QStringLiteral("RemoveExclusion");
      QTest::newRow(qUtf8Printable(QStringLiteral("file-exclusion-%1").arg(int(action))))
          << Request{EditFileExclusion{volumeId, "\\Wire", action}} << QStringLiteral("volume-next") << method << QStringLiteral("FileName")
          << QStringLiteral("\\Wire");
      QTest::newRow(qUtf8Printable(QStringLiteral("registry-exclusion-%1").arg(int(action))))
          << Request{EditRegistryExclusion{registryKey, action}} << QStringLiteral("registry-next") << method << QStringLiteral("RegistryKey")
          << QString::fromLatin1(registryKey);
    }
    QTest::newRow("file-commit-current-off") << Request{CommitFile{volumeId, "\\Wire"}} << QStringLiteral("volume-current") << QStringLiteral("CommitFile")
                                             << QStringLiteral("FileName") << QStringLiteral("\\Wire");
    QTest::newRow("registry-commit-current-off") << Request{CommitRegistry{registryKey, std::string("Named")}} << QStringLiteral("registry-current")
                                                 << QStringLiteral("CommitRegistry") << QStringLiteral("RegistryKey") << QString::fromLatin1(registryKey);
    QTest::newRow("registration") << Request{AddVolume{volumeId, "C:"}} << QStringLiteral("UWF_Volume") << QStringLiteral("PutInstance")
                                  << QStringLiteral("VolumeName") << QString::fromLatin1(volumeName);
  }
  void mappedCommandReceiptsKeepTheirSpecificWireTarget() {
    QFETCH(Request, request);
    QFETCH(QString, target);
    QFETCH(QString, method);
    QFETCH(QString, parameter);
    QFETCH(QString, value);
    Provider provider;
    if (std::holds_alternative<AddVolume>(request))
      std::erase_if(provider.tables["UWF_Volume"], [](const WmiRow& row) { return !row.at("CurrentSession").toBool(); });
    std::visit(
        [&](const auto& operation) {
          using R = std::decay_t<decltype(operation)>;
          if constexpr (std::is_same_v<R, EditFileExclusion> || std::is_same_v<R, EditRegistryExclusion>) {
            if (operation.action != ExclusionAction::Add) provider.exclusions[target.toStdString()] = {value.toStdString()};
          }
        },
        request);
    const auto response = provider.backend().execute(request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "command failed");
    QCOMPARE(response.effect, Effect::Confirmed);
    QVERIFY(std::holds_alternative<std::monostate>(response.value));
    QCOMPARE(provider.paths, std::vector<std::string>{target.toStdString()});
    QCOMPARE(provider.methods, std::vector<std::string>{method.toStdString()});
    const auto& arguments = provider.arguments.front();
    if (parameter.isEmpty()) {
      QVERIFY(arguments.empty());
    } else if (std::holds_alternative<SetNumber>(request)) {
      QCOMPARE(arguments.size(), std::size_t{1});
      QCOMPARE(arguments.at("size").toUInt(), std::uint32_t{256});
      QCOMPARE(provider.row("overlay").at("CriticalOverlayThreshold").toUInt(), std::uint32_t{256});
    } else {
      QCOMPARE(arguments.at(parameter.toStdString()).toString(), value.toStdString());
      if (std::holds_alternative<CommitRegistry>(request)) QCOMPARE(arguments.at("ValueName").toString(), std::string("Named"));
      if (std::holds_alternative<AddVolume>(request)) {
        QVERIFY(!arguments.at("CurrentSession").toBool());
        QVERIFY(!arguments.at("Protected").toBool());
        QVERIFY(!arguments.at("BindByDriveLetter").toBool());
        QCOMPARE(arguments.at("DriveLetter").toString(), std::string("C:"));
        QVERIFY(!provider.row("volume-next").at("Protected").toBool());
        QVERIFY(!provider.row("volume-next").at("BindByDriveLetter").toBool());
      }
    }
  }
  void propertyClassAbsenceRejectsReadsAndWrites_data() {
    QTest::addColumn<Request>("read");
    QTest::addColumn<Request>("write");
    QTest::addColumn<QString>("className");
    QTest::newRow("filter") << Request{GetBoolean{BooleanProperty::FilterEnabled, Session::Next}} << Request{SetBoolean{BooleanProperty::FilterEnabled, true}}
                            << QStringLiteral("UWF_Filter");
    QTest::newRow("horm") << Request{GetBoolean{BooleanProperty::HormEnabled}} << Request{SetBoolean{BooleanProperty::HormEnabled, false}}
                          << QStringLiteral("UWF_Filter");
    QTest::newRow("servicing") << Request{GetBoolean{BooleanProperty::ServicingEnabled, Session::Next}}
                               << Request{SetBoolean{BooleanProperty::ServicingEnabled, true}} << QStringLiteral("UWF_Servicing");
    QTest::newRow("domain-persistence") << Request{GetBoolean{BooleanProperty::PersistDomainSecretKey, Session::Next}}
                                        << Request{SetBoolean{BooleanProperty::PersistDomainSecretKey, true}} << QStringLiteral("UWF_RegistryFilter");
    QTest::newRow("license-persistence") << Request{GetBoolean{BooleanProperty::PersistTscal, Session::Next}}
                                         << Request{SetBoolean{BooleanProperty::PersistTscal, true}} << QStringLiteral("UWF_RegistryFilter");
    QTest::newRow("overlay-type") << Request{GetNumber{NumberProperty::OverlayType, Session::Next}} << Request{SetNumber{NumberProperty::OverlayType, 1}}
                                  << QStringLiteral("UWF_OverlayConfig");
    QTest::newRow("overlay-capacity") << Request{GetNumber{NumberProperty::MaximumOverlaySize, Session::Next}}
                                      << Request{SetNumber{NumberProperty::MaximumOverlaySize, 4096}} << QStringLiteral("UWF_OverlayConfig");
    QTest::newRow("warning") << Request{GetNumber{NumberProperty::WarningThreshold}} << Request{SetNumber{NumberProperty::WarningThreshold, 256}}
                             << QStringLiteral("UWF_Overlay");
    QTest::newRow("critical") << Request{GetNumber{NumberProperty::CriticalThreshold}} << Request{SetNumber{NumberProperty::CriticalThreshold, 512}}
                              << QStringLiteral("UWF_Overlay");
  }
  void propertyClassAbsenceRejectsReadsAndWrites() {
    QFETCH(Request, read);
    QFETCH(Request, write);
    QFETCH(QString, className);
    Provider provider;
    auto backend = provider.backend();
    QCOMPARE(backend.probe(read).support, Support::Unknown);
    QCOMPARE(backend.probe(write).support, Support::Unknown);
    provider.missingClasses.insert(className.toStdString());
    QCOMPARE(backend.probe(read).support, Support::Unsupported);
    QCOMPARE(backend.probe(write).support, Support::Unsupported);
    QCOMPARE(provider.classProbes, 4);
    // Direct raw execution uses the actual API failure without another discovery query.
    provider.queryFailure = std::make_exception_ptr(
        windows::wmi::WmiInfrastructureError(static_cast<std::int32_t>(windows::wmi::WmiErrorCode::InvalidClass), "raw provider operation"));
    provider.invocationFailure = provider.queryFailure;
    for (const auto& request : {read, write}) {
      const auto response = backend.execute(request);
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
      QCOMPARE(response.effect, Effect::None);
    }
    QCOMPARE(provider.classProbes, 4);
  }
  void disablingActiveHormDoesNotRequireHormToBeDisabled() {
    Provider provider;
    provider.row("filter")["CurrentEnabled"] = WireValue::fromBool(true);
    provider.row("filter")["HORMEnabled"] = WireValue::fromBool(true);
    const auto response = provider.backend().execute(SetBoolean{BooleanProperty::HormEnabled, false});
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "HORM disable failed");
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(provider.methods, std::vector<std::string>{"DisableHORM"});
    QCOMPARE(provider.paths, std::vector<std::string>{"filter"});
    QVERIFY(!provider.row("filter").at("HORMEnabled").toBool());
  }
  void numericWritesBindOneExactProviderField_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<QString>("target");
    QTest::addColumn<QString>("method");
    QTest::addColumn<QString>("parameter");
    QTest::addColumn<QString>("field");
    QTest::addColumn<std::uint32_t>("value");
    QTest::newRow("type") << Request{SetNumber{NumberProperty::OverlayType, 1}} << QStringLiteral("config-next") << QStringLiteral("SetType")
                          << QStringLiteral("type") << QStringLiteral("Type") << std::uint32_t{1};
    QTest::newRow("maximum") << Request{SetNumber{NumberProperty::MaximumOverlaySize, 4096}} << QStringLiteral("config-next")
                             << QStringLiteral("SetMaximumSize") << QStringLiteral("size") << QStringLiteral("MaximumSize") << std::uint32_t{4096};
    QTest::newRow("warning") << Request{SetNumber{NumberProperty::WarningThreshold, 256}} << QStringLiteral("overlay") << QStringLiteral("SetWarningThreshold")
                             << QStringLiteral("size") << QStringLiteral("WarningOverlayThreshold") << std::uint32_t{256};
    QTest::newRow("critical") << Request{SetNumber{NumberProperty::CriticalThreshold, 512}} << QStringLiteral("overlay")
                              << QStringLiteral("SetCriticalThreshold") << QStringLiteral("size") << QStringLiteral("CriticalOverlayThreshold")
                              << std::uint32_t{512};
  }
  void numericWritesBindOneExactProviderField() {
    QFETCH(Request, request);
    QFETCH(QString, target);
    QFETCH(QString, method);
    QFETCH(QString, parameter);
    QFETCH(QString, field);
    QFETCH(std::uint32_t, value);
    Provider provider;
    const auto response = provider.backend().execute(request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "numeric command failed");
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(provider.paths, std::vector<std::string>{target.toStdString()});
    QCOMPARE(provider.methods, std::vector<std::string>{method.toStdString()});
    QCOMPARE(provider.arguments.front().size(), std::size_t{1});
    QCOMPARE(provider.arguments.front().at(parameter.toStdString()).toUInt(), value);
    QCOMPARE(provider.row(target.toStdString()).at(field.toStdString()).toUInt(), value);
    QCOMPARE(provider.postWriteReads, 0);
  }
  void platformErrorsRetainTheirContextAndOriginalCode_data() {
    QTest::addColumn<std::uint32_t>("code");
    QTest::addColumn<bool>("writing");
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
        for (const bool hresult : {false, true}) {
          const auto code = hresult ? 0x80070000u | item.code : item.code;
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(code).arg(writing ? "write" : "read").arg(hresult ? "HRESULT" : "Win32")))
              << code << writing << int(writing ? item.operation : item.observation);
        }
  }
  void platformErrorsRetainTheirContextAndOriginalCode() {
    QFETCH(std::uint32_t, code);
    QFETCH(bool, writing);
    QFETCH(int, kind);
    Provider provider;
    const auto failure = std::make_exception_ptr(windows::wmi::WmiProviderError(code, writing ? "Disable" : "GetFilter"));
    if (writing)
      provider.invocationFailure = failure;
    else
      provider.queryFailure = failure;
    const Request request =
        writing ? Request{SetBoolean{BooleanProperty::FilterEnabled, false}} : Request{GetBoolean{BooleanProperty::FilterEnabled, Session::Current}};
    const auto result = provider.backend().execute(request);
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, static_cast<ErrorKind>(kind));
    QCOMPARE(result.effect, writing ? Effect::Uncertain : Effect::None);
    QCOMPARE(provider.methods.size(), writing ? std::size_t{1} : std::size_t{0});
    QCOMPARE(result.failure->diagnostics.size(), std::size_t{1});
    QCOMPARE(result.failure->diagnostics.front().code, std::int64_t{code});
  }
  void unsupportedMappingsRejectBeforeProviderObservation_data() {
    QTest::addColumn<Request>("request");
    const std::vector<Request> requests{SetBoolean{BooleanProperty::ShutdownPending, true},
                                        SetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted, true},
                                        SetBoolean{BooleanProperty::OverlayCommit, true},
                                        SetNumber{NumberProperty::OverlayFlags, 0},
                                        SetNumber{NumberProperty::ResetPersistentState, 0},
                                        SetNumber{NumberProperty::ResetPersistentState, 1},
                                        SetNumber{NumberProperty::ResetPersistentState, 255},
                                        SetNumber{NumberProperty::ResetPersistentStateSavedMode, 1},
                                        SetOverlayOption{OverlayOption::Passthrough, true},
                                        SetOverlayOption{OverlayOption::Persistent, true},
                                        SetOverlayOption{OverlayOption::ReadOnlyMedia, true},
                                        RemoveVolume{volumeId},
                                        CreateSwapfile{volumeId},
                                        FindFileExclusion{volumeId, "\\File", Session::Next},
                                        FindRegistryExclusion{registryKey, Session::Next},
                                        EditFileExclusion{volumeId, {}, ExclusionAction::Clear},
                                        EditRegistryExclusion{{}, ExclusionAction::Clear}};
    for (std::size_t index = 0; index < requests.size(); ++index)
      QTest::newRow(qUtf8Printable(QStringLiteral("unsupported-%1-%2").arg(index).arg(QString::fromStdString(operationName(requests[index])))))
          << requests[index];
  }
  void unsupportedMappingsRejectBeforeProviderObservation() {
    QFETCH(Request, request);
    Provider provider;
    const auto backend = provider.backend();
    QCOMPARE(backend.probe(request).support, Support::Unsupported);
    const auto result = backend.execute(request);
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, ErrorKind::Unsupported);
    QCOMPARE(result.effect, Effect::None);
    QCOMPARE(provider.reads, 0);
    QVERIFY(provider.methods.empty());
    QVERIFY(provider.readMethods.empty());
  }
  void allReadSlotsHaveExplicitSessionAndValueEvidence_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<Value>("expected");
    QTest::addColumn<bool>("supported");
    const auto add = [](const Request& request, Value expected, const char* scope, const bool supported = true) {
      QTest::newRow(qUtf8Printable(QString::fromStdString(operationName(request)) + u'-' + QString::fromLatin1(scope))) << request << expected << supported;
    };
    struct BooleanRead {
      BooleanProperty property;
      bool current;
      bool next;
    };
    const std::array booleans{
        BooleanRead{BooleanProperty::FilterEnabled, false, true},          BooleanRead{BooleanProperty::ShutdownPending, true, true},
        BooleanRead{BooleanProperty::HormEnabled, false, false},           BooleanRead{BooleanProperty::ServicingEnabled, false, true},
        BooleanRead{BooleanProperty::PersistDomainSecretKey, false, true}, BooleanRead{BooleanProperty::PersistTscal, true, false},
        BooleanRead{BooleanProperty::OverlayCommit, false, false},         BooleanRead{BooleanProperty::RegistryChangesAlwaysPersisted, true, true}};
    struct NumberRead {
      NumberProperty property;
      std::uint32_t current;
      std::uint32_t next;
      bool supported;
    };
    const std::array numbers{NumberRead{NumberProperty::OverlayType, 0, 1, true},
                             NumberRead{NumberProperty::MaximumOverlaySize, 1024, 3072, true},
                             NumberRead{NumberProperty::WarningThreshold, 64, 64, true},
                             NumberRead{NumberProperty::CriticalThreshold, 128, 128, true},
                             NumberRead{NumberProperty::OverlayFlags, 0, 0, false},
                             NumberRead{NumberProperty::ResetPersistentState, 0, 0, false},
                             NumberRead{NumberProperty::ResetPersistentStateSavedMode, 0, 0, false}};
    for (const auto session : {Session::Current, Session::Next}) {
      const bool current = session == Session::Current;
      const auto* scope = current ? "Current" : "Next";
      for (const auto& item : booleans)
        add(GetBoolean{item.property, session}, Value{current ? item.current : item.next}, scope,
            item.property != BooleanProperty::OverlayCommit && item.property != BooleanProperty::RegistryChangesAlwaysPersisted);
      for (const auto& item : numbers) add(GetNumber{item.property, session}, Value{current ? item.current : item.next}, scope, item.supported);
      add(GetVolumes{session}, Value{std::vector<Volume>{}}, scope, false);
      add(GetVolumeProperty{volumeId, VolumeProperty::Protected, session}, Value{current}, scope);
      add(GetVolumeProperty{volumeId, VolumeProperty::BindByDriveLetter, session}, Value{!current}, scope);
      add(GetFileExclusions{volumeId, session}, Value{current ? std::vector<std::string>{"\\Current", "\\Literal%20"} : std::vector<std::string>{"\\Next"}},
          scope);
      add(FindFileExclusion{volumeId, "\\cUrReNt", session}, Value{current}, scope, false);
      add(GetRegistryExclusions{session},
          Value{current ? std::vector<std::string>{"HKEY_LOCAL_MACHINE\\Software\\Current"} : std::vector<std::string>{"HKEY_LOCAL_MACHINE\\Software\\Next"}},
          scope);
      add(FindRegistryExclusion{"hkey_local_machine\\software\\current", session}, Value{current}, scope, false);
      add(GetSwapfileSize{volumeId, session}, Value{std::uint32_t{0}}, scope, false);
    }
    add(GetOverlayUsage{}, Value{OverlayUsage{7, 93}}, "Current-global");
    add(GetOverlayFiles{volumeId}, Value{std::vector<OverlayFile>{{"\\Observed", 4096}}}, "Current-exact-volume");
  }
  void allReadSlotsHaveExplicitSessionAndValueEvidence() {
    QFETCH(Request, request);
    QFETCH(Value, expected);
    QFETCH(bool, supported);
    Provider provider;
    provider.row("filter")["ShutdownPending"] = WireValue::fromBool(true);
    provider.row("servicing-next")["ServicingEnabled"] = WireValue::fromBool(true);
    provider.row("registry-next")["PersistDomainSecretKey"] = WireValue::fromBool(true);
    provider.row("registry-current")["PersistTSCAL"] = WireValue::fromBool(true);
    provider.row("config-current")["MaximumSize"] = WireValue::fromUInt(1024);
    provider.row("config-next")["Type"] = WireValue::fromUInt(1);
    provider.row("config-next")["MaximumSize"] = WireValue::fromUInt(3072);
    provider.row("volume-next")["BindByDriveLetter"] = WireValue::fromBool(true);
    provider.exclusions["volume-current"] = {"\\Current", "\\Literal%20"};
    provider.exclusions["volume-next"] = {"\\Next"};
    provider.exclusions["registry-current"] = {"HKEY_LOCAL_MACHINE\\Software\\Current"};
    provider.exclusions["registry-next"] = {"HKEY_LOCAL_MACHINE\\Software\\Next"};
    if (std::holds_alternative<GetOverlayFiles>(request)) {
      provider.row("filter")["CurrentEnabled"] = WireValue::fromBool(true);
      provider.filesOutput.arrays["OverlayFiles"] = {{{"FileName", WireValue::fromString("\\Observed")}, {"FileSize", WireValue::fromUInt(4096)}}};
    }
    const auto result = provider.backend().execute(request);
    QCOMPARE(result.effect, Effect::None);
    QVERIFY(provider.methods.empty());
    if (supported) {
      QVERIFY2(result.succeeded(), result.failure ? result.failure->message.c_str() : "read failed");
      QCOMPARE(result.value, expected);
    } else {
      QVERIFY(result.failure);
      QCOMPARE(result.failure->kind, ErrorKind::Unsupported);
    }
  }

  void aMalformedProviderVolumeIdentityCannotSelectATarget() {
    Provider provider;
    provider.row("volume-current")["VolumeName"] = WireValue::fromString(std::string(volumeName) + "\\\\");
    const auto response = provider.backend().execute(GetFileExclusions{volumeId, Session::Current});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Protocol);
    QCOMPARE(response.effect, Effect::None);
    QVERIFY(provider.readMethods.empty());
    QVERIFY(provider.methods.empty());
  }
  void fileMaintenanceDoesNotReadDriveMetadataOrAdmission() {
    for (const int fault : {0, 1, 2, 3}) {
      Provider provider;
      auto& row = provider.row("volume-current");
      if (fault == 0)
        row.erase("DriveLetter");
      else if (fault == 1)
        row["DriveLetter"] = WireValue{};
      else if (fault == 2)
        row["DriveLetter"] = WireValue::fromString("C:\\bad");
      else
        row["DriveLetter"] = WireValue::fromUInt(1);

      const auto response = WmiUwfBackend(provider).execute(CommitFile{volumeId, "\\File"});
      QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "raw commit failed");
      QCOMPARE(response.effect, Effect::Confirmed);
      QCOMPARE(provider.queries, std::vector<std::string>{"SELECT * FROM UWF_Volume"});
      QCOMPARE(provider.methods, std::vector<std::string>{"CommitFile"});
      QCOMPARE(provider.arguments.front().at("FileName").toString(), std::string("\\File"));
    }
  }
  void hormEnableUsesTheCompleteProviderWithoutPublicFlags_data() {
    QTest::addColumn<quint32>("rawStatus");
    QTest::newRow("complete-success") << quint32{0};
    QTest::newRow("raw-access-denied") << quint32{0x80070005u};
    QTest::newRow("raw-invalid-state") << quint32{0x8007070du};
  }
  void hormEnableUsesTheCompleteProviderWithoutPublicFlags() {
    QFETCH(quint32, rawStatus);
    Provider provider;
    provider.row("filter")["CurrentEnabled"] = WireValue::fromBool(true);
    if (rawStatus)
      provider.invocationFailure =
          std::make_exception_ptr(windows::wmi::WmiProviderError(rawStatus, "EnableHORM", "original complete HORM workflow rejection"));
    const WmiUwfBackend backend(provider);
    QCOMPARE(backend.probe(GetNumber{NumberProperty::OverlayFlags, Session::Current}).support, Support::Unsupported);
    const auto result = backend.execute(SetBoolean{BooleanProperty::HormEnabled, true});
    QCOMPARE(result.succeeded(), rawStatus == 0);
    QCOMPARE(result.effect, rawStatus == 0 ? Effect::Confirmed : Effect::Uncertain);
    QCOMPARE(provider.methods, std::vector<std::string>{"EnableHORM"});
    QCOMPARE(provider.paths, std::vector<std::string>{"filter"});
    QCOMPARE(provider.postWriteReads, 0);
    QCOMPARE(provider.row("filter").at("HORMEnabled").toBool(), rawStatus == 0);
    if (rawStatus) {
      QCOMPARE(result.failure->kind, rawStatus == 0x80070005u ? ErrorKind::AccessDenied : ErrorKind::Conflict);
      // WMI ReturnValue is UInt32; do not sign-extend it as a COM HRESULT.
      QCOMPARE(result.failure->diagnostics.front().code, std::int64_t{rawStatus});
      QVERIFY(result.failure->diagnostics.front().detail.find("original complete HORM workflow rejection") != std::string::npos);
    }
  }
  void rawMaintenanceUsesOneLocatorAndOneMethod_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("deletion");
    for (const bool registry : {false, true})
      for (const bool deletion : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("registry-%1/delete-%2").arg(registry).arg(deletion))) << registry << deletion;
  }
  void rawMaintenanceUsesOneLocatorAndOneMethod() {
    QFETCH(bool, registry);
    QFETCH(bool, deletion);
    Provider provider;
    provider.tables.erase("UWF_Filter");
    provider.tables.erase("UWF_OverlayConfig");
    const Request request =
        registry   ? deletion ? Request{CommitRegistryDeletion{registryKey, std::string("Named")}} : Request{CommitRegistry{registryKey, std::string("Named")}}
          : deletion ? Request{CommitFileDeletion{volumeId, "\\File"}}
                   : Request{CommitFile{volumeId, "\\File"}};
    auto backend = std::make_shared<WmiUwfBackend>(provider);
    const auto response = FallbackUwf({backend}).execute(request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "raw maintenance failed");
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(provider.classProbes, 1);  // Fallback discovery is not repeated during execute.
    QCOMPARE(provider.queries, std::vector<std::string>{registry ? "SELECT * FROM UWF_RegistryFilter" : "SELECT * FROM UWF_Volume"});
    QCOMPARE(provider.methods.size(), std::size_t{1});
    QCOMPARE(provider.methods.front(), std::string(registry   ? deletion ? "CommitRegistryDeletion" : "CommitRegistry"
                                                   : deletion ? "CommitFileDeletion"
                                                              : "CommitFile"));
    QCOMPARE(provider.postWriteReads, 0);
    QVERIFY(provider.readMethods.empty());
  }
  void filterObservationsDistinguishFalseFromUnreadable_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<int>("observation");
    for (const bool current : {false, true})
      for (const auto& [name, observation] : {std::pair{"false", BooleanObservation::False}, std::pair{"null", BooleanObservation::Null},
                                              std::pair{"missing", BooleanObservation::Missing}, std::pair{"wrong-type", BooleanObservation::WrongType}})
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(current ? QStringLiteral("current") : QStringLiteral("next"), QString::fromLatin1(name))))
            << current << int(observation);
  }
  void filterObservationsDistinguishFalseFromUnreadable() {
    QFETCH(bool, current);
    QFETCH(int, observation);
    Provider provider;
    const auto state = static_cast<BooleanObservation>(observation);
    const auto field = current ? "CurrentEnabled" : "NextEnabled";
    setBooleanObservation(provider.row("filter"), field, state);
    const Request request = GetBoolean{BooleanProperty::FilterEnabled, current ? Session::Current : Session::Next};
    auto first = std::make_shared<WmiUwfBackend>(provider);
    const auto response = first->execute(request);
    const bool readable = state == BooleanObservation::False;
    QCOMPARE(response.succeeded(), readable);
    QCOMPARE(response.effect, Effect::None);
    if (readable)
      QVERIFY(!std::get<bool>(response.value));
    else {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::Protocol);
    }
    Provider recovery;
    recovery.row("filter")[field] = WireValue::fromBool(true);
    auto second = std::make_shared<WmiUwfBackend>(recovery);
    const auto routed = FallbackUwf({first, second}).execute(request);
    QVERIFY(routed.succeeded());
    QCOMPARE(std::get<bool>(routed.value), !readable);
    QCOMPARE(routed.attempts.size(), readable ? std::size_t{1} : std::size_t{2});
    QVERIFY(provider.methods.empty());
    QVERIFY(recovery.methods.empty());
  }
  void rawSettersDoNotInterpretUnrelatedFilterState_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<QString>("field");
    QTest::newRow("type/current") << Request{SetNumber{NumberProperty::OverlayType, 1}} << QStringLiteral("CurrentEnabled");
    QTest::newRow("size/current") << Request{SetNumber{NumberProperty::MaximumOverlaySize, 512}} << QStringLiteral("CurrentEnabled");
    QTest::newRow("reset/current") << Request{ResetSettings{}} << QStringLiteral("CurrentEnabled");
    QTest::newRow("enable/next") << Request{SetBoolean{BooleanProperty::FilterEnabled, true}} << QStringLiteral("NextEnabled");
    QTest::newRow("disable/next") << Request{SetBoolean{BooleanProperty::FilterEnabled, false}} << QStringLiteral("NextEnabled");
  }
  void rawSettersDoNotInterpretUnrelatedFilterState() {
    QFETCH(Request, request);
    QFETCH(QString, field);
    Provider provider;
    provider.row("filter")[field.toStdString()] = WireValue{};
    const auto response = provider.backend().execute(request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "raw setter failed");
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(provider.methods.size(), std::size_t{1});
    QCOMPARE(provider.reads, 1);  // Only the requested class instance's __PATH is resolved.
    QCOMPARE(provider.postWriteReads, 0);
  }
  void nextVolumeProtectionNeverDefaultsUnreadableToFalse_data() {
    QTest::addColumn<int>("observation");
    for (const auto& [name, observation] : {std::pair{"false", BooleanObservation::False}, std::pair{"null", BooleanObservation::Null},
                                            std::pair{"missing", BooleanObservation::Missing}, std::pair{"wrong-type", BooleanObservation::WrongType}})
      QTest::newRow(name) << int(observation);
  }
  void nextVolumeProtectionNeverDefaultsUnreadableToFalse() {
    QFETCH(int, observation);
    Provider provider;
    const auto state = static_cast<BooleanObservation>(observation);
    setBooleanObservation(provider.row("volume-next"), "Protected", state);
    const auto response = provider.backend().execute(GetVolumeProperty{volumeId, VolumeProperty::Protected, Session::Next});
    QCOMPARE(response.succeeded(), state == BooleanObservation::False);
    QCOMPARE(response.effect, Effect::None);
    if (response.succeeded()) {
      QVERIFY(!std::get<bool>(response.value));
    } else {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::Protocol);
    }
  }

  void discoveryDoesNotWriteAndCannotInventMembers() {
    Provider provider;
    const auto backend = provider.backend();
    QCOMPARE(backend.probe(SetBoolean{BooleanProperty::FilterEnabled, false}).support, Support::Unknown);
    QCOMPARE(backend.probe(SetNumber{NumberProperty::OverlayFlags, flags::persistent}).support, Support::Unsupported);
    QCOMPARE(backend.probe(CreateSwapfile{volumeId}).support, Support::Unsupported);
    QCOMPARE(provider.reads, 0);
    QVERIFY(provider.methods.empty());
  }
  void lazyInitializationFailureAllowsOtherBackends() {
    int acquired = 0;
    auto first = std::make_shared<WmiUwfBackend>([&]() -> WmiOperations& {
      ++acquired;
      throw std::runtime_error("COM unavailable");
    });
    QCOMPARE(acquired, 0);
    auto second = std::make_shared<AcceptingBackend>();
    QVERIFY(FallbackUwf({first, second}).execute(SetBoolean{BooleanProperty::FilterEnabled, true}).succeeded());
    QCOMPARE(second->writes, 1);
  }
  void writeEffects_data() {
    QTest::addColumn<int>("fault");
    QTest::addColumn<bool>("success");
    QTest::addColumn<int>("effect");
    const auto add = [](const char* name, Fault fault, bool success, Effect effect) { QTest::newRow(name) << int(fault) << success << int(effect); };
    add("accepted", Fault::None, true, Effect::Confirmed);
    add("preparation", Fault::BeforeRead, false, Effect::None);
    add("provider-return-value", Fault::Provider, false, Effect::Uncertain);
    add("missing-method", Fault::MissingMethod, false, Effect::None);
    add("changed-before-lost-ack", Fault::LostAfterMutation, false, Effect::Uncertain);
    add("raw-provider-receipt", Fault::UnobservedSuccess, true, Effect::Confirmed);
    add("unknown-after-mutation", Fault::UnknownAfterMutation, false, Effect::Uncertain);
    add("no-implicit-reread", Fault::Readback, true, Effect::Confirmed);
  }
  void writeEffects() {
    QFETCH(int, fault);
    QFETCH(bool, success);
    QFETCH(int, effect);
    Provider provider;
    switch (static_cast<Fault>(fault)) {
      case Fault::None:
        break;
      case Fault::BeforeRead:
        provider.queryFailure = std::make_exception_ptr(std::runtime_error("read failed"));
        break;
      case Fault::Provider:
        provider.invocationFailure = std::make_exception_ptr(windows::wmi::WmiProviderError(5, "Disable"));
        break;
      case Fault::MissingMethod:
        provider.invocationFailure =
            std::make_exception_ptr(windows::wmi::WmiInfrastructureError(static_cast<std::int32_t>(windows::wmi::WmiErrorCode::InvalidMethod), "Disable"));
        break;
      case Fault::LostAfterMutation:
        provider.afterWrite = [] { throw windows::wmi::WmiInvocationUncertain("Disable", "ack lost"); };
        break;
      case Fault::UnobservedSuccess:
        provider.mutate = false;
        break;
      case Fault::UnknownAfterMutation:
        provider.afterWrite = [] { throw 42; };
        break;
      case Fault::Readback:
        provider.afterWrite = [&] { provider.queryFailure = std::make_exception_ptr(std::runtime_error("reread failed")); };
        break;
    }
    const auto response = provider.backend().execute(SetBoolean{BooleanProperty::FilterEnabled, false});
    QCOMPARE(response.succeeded(), success);
    QCOMPARE(response.effect, static_cast<Effect>(effect));
    QCOMPARE(provider.methods.size(), static_cast<Fault>(fault) == Fault::BeforeRead ? std::size_t{0} : std::size_t{1});
    if (static_cast<Fault>(fault) == Fault::LostAfterMutation) QVERIFY(!provider.row("filter").at("NextEnabled").toBool());
  }
  void onlyKnownPreDispatchFailureCanFallBack() {
    for (bool missing : {true, false}) {
      Provider provider;
      if (missing)
        provider.invocationFailure =
            std::make_exception_ptr(windows::wmi::WmiInfrastructureError(static_cast<std::int32_t>(windows::wmi::WmiErrorCode::InvalidMethod), "Disable"));
      else
        provider.afterWrite = [] { throw windows::wmi::WmiInvocationUncertain("Disable", "lost after change"); };
      auto first = std::make_shared<WmiUwfBackend>(provider);
      auto second = std::make_shared<AcceptingBackend>();
      const auto response = FallbackUwf({first, second}).execute(SetBoolean{BooleanProperty::FilterEnabled, false});
      QCOMPARE(response.succeeded(), missing);
      QCOMPARE(second->writes, missing ? 1 : 0);
    }
  }
  void booleanWritesUseOnlyTheirProviderReceipt_data() {
    QTest::addColumn<int>("property");
    QTest::addColumn<QString>("target");
    QTest::addColumn<QString>("method");
    QTest::addColumn<QString>("field");
    QTest::newRow("filter") << int(BooleanProperty::FilterEnabled) << QStringLiteral("filter") << QStringLiteral("Disable") << QString{};
    QTest::newRow("HORM-complete-transition") << int(BooleanProperty::HormEnabled) << QStringLiteral("filter") << QStringLiteral("EnableHORM") << QString{};
    QTest::newRow("servicing-complete-transition") << int(BooleanProperty::ServicingEnabled) << QStringLiteral("servicing-next") << QStringLiteral("Enable")
                                                   << QString{};
    QTest::newRow("domain-secret") << int(BooleanProperty::PersistDomainSecretKey) << QStringLiteral("UWF_RegistryFilter") << QStringLiteral("PutInstance")
                                   << QStringLiteral("PersistDomainSecretKey");
    QTest::newRow("TSCAL") << int(BooleanProperty::PersistTscal) << QStringLiteral("UWF_RegistryFilter") << QStringLiteral("PutInstance")
                           << QStringLiteral("PersistTSCAL");
  }
  void booleanWritesUseOnlyTheirProviderReceipt() {
    QFETCH(int, property);
    QFETCH(QString, target);
    QFETCH(QString, method);
    QFETCH(QString, field);
    Provider provider;
    const auto selected = static_cast<BooleanProperty>(property);
    const bool desired = selected != BooleanProperty::FilterEnabled;
    if (selected == BooleanProperty::HormEnabled) provider.row("filter")["CurrentEnabled"] = WireValue::fromBool(true);
    if (selected == BooleanProperty::ServicingEnabled)
      provider.row("servicing-next")["ServicingEnabled"] = WireValue::fromBool(true);  // Same flag still requires the complete transition.
    const auto response = provider.backend().execute(SetBoolean{selected, desired});
    QVERIFY(response.succeeded());
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(provider.methods.size(), std::size_t{1});
    QCOMPARE(provider.paths, std::vector<std::string>{target.toStdString()});
    QCOMPARE(provider.methods, std::vector<std::string>{method.toStdString()});
    if (field.isEmpty()) {
      QVERIFY(provider.arguments.front().empty());
    } else {
      QCOMPARE(provider.arguments.front().size(), std::size_t{2});
      QVERIFY(!provider.arguments.front().at("CurrentSession").toBool());
      QVERIFY(provider.arguments.front().at(field.toStdString()).toBool());
      QVERIFY(provider.row("registry-next").at(field.toStdString()).toBool());
      QVERIFY(!provider.row("registry-current").at(field.toStdString()).toBool());
    }
    QCOMPARE(provider.postWriteReads, 0);
    if (selected == BooleanProperty::ServicingEnabled) QVERIFY(!provider.row("filter").at("NextEnabled").toBool());
  }
  void rawConfigurationWritesUseOnlyProviderReceipts_data() {
    QTest::addColumn<int>("operation");
    QTest::addColumn<int>("fault");
    const std::array names{"filter",     "HORM",    "servicing",      "domain-persistence", "type",        "size", "threshold",
                           "protection", "binding", "file-exclusion", "registry-exclusion", "registration"};
    const std::array faults{"accepted", "stale-state", "read-denied", "write-denied-with-matching-state"};
    for (std::size_t operation = 0; operation < names.size(); ++operation) {
      for (std::size_t fault = 0; fault < faults.size(); ++fault)
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(QString::fromLatin1(names[operation]), QString::fromLatin1(faults[fault]))))
            << int(operation) << int(fault);
    }
  }
  void rawConfigurationWritesUseOnlyProviderReceipts() {
    QFETCH(int, operation);
    QFETCH(int, fault);
    Provider provider;
    if (operation == 1) provider.row("filter")["CurrentEnabled"] = WireValue::fromBool(true);
    if (operation == 11) std::erase_if(provider.tables["UWF_Volume"], [](const WmiRow& row) { return !row.at("CurrentSession").toBool(); });
    provider.mutate = fault != 1;
    if (fault == 2)
      provider.afterWrite = [&] {
        provider.queryFailure = std::make_exception_ptr(windows::wmi::WmiInfrastructureError(
            static_cast<std::int32_t>(windows::wmi::WmiErrorCode::AccessDenied), "confirmation read", "original observation rejection"));
      };
    if (fault == 3) provider.afterWrite = [] { throw windows::wmi::WmiProviderError(5, "configuration write", "original write rejection"); };
    const std::array requests{Request{SetBoolean{BooleanProperty::FilterEnabled, false}},
                              Request{SetBoolean{BooleanProperty::HormEnabled, true}},
                              Request{SetBoolean{BooleanProperty::ServicingEnabled, true}},
                              Request{SetBoolean{BooleanProperty::PersistDomainSecretKey, true}},
                              Request{SetNumber{NumberProperty::OverlayType, static_cast<std::uint32_t>(OverlayType::Disk)}},
                              Request{SetNumber{NumberProperty::MaximumOverlaySize, 4096}},
                              Request{SetNumber{NumberProperty::WarningThreshold, 256}},
                              Request{SetVolume{volumeId, VolumeProperty::Protected, true}},
                              Request{SetVolume{volumeId, VolumeProperty::BindByDriveLetter, true}},
                              Request{EditFileExclusion{volumeId, "\\Payload", ExclusionAction::Add}},
                              Request{EditRegistryExclusion{registryKey, ExclusionAction::Add}},
                              Request{AddVolume{volumeId, "C:"}}};
    const auto response = provider.backend().execute(requests.at(static_cast<std::size_t>(operation)));
    QCOMPARE(response.succeeded(), fault != 3);
    QCOMPARE(response.effect, fault != 3 ? Effect::Confirmed : Effect::Uncertain);
    QCOMPARE(provider.methods.size(), std::size_t{1});
    const std::array wireTargets{"filter",  "filter",      "servicing-next", "UWF_RegistryFilter", "config-next",   "config-next",
                                 "overlay", "volume-next", "volume-next",    "volume-next",        "registry-next", "UWF_Volume"};
    const std::array wireMethods{"Disable", "EnableHORM",           "Enable",       "PutInstance",  "SetType",    "SetMaximumSize", "SetWarningThreshold",
                                 "Protect", "SetBindByDriveLetter", "AddExclusion", "AddExclusion", "PutInstance"};
    QCOMPARE(provider.paths.front(), std::string(wireTargets.at(static_cast<std::size_t>(operation))));
    QCOMPARE(provider.methods.front(), std::string(wireMethods.at(static_cast<std::size_t>(operation))));
    if (operation == 8) QVERIFY(provider.arguments.front().at("bBindByDriveLetter").toBool());
    QCOMPARE(provider.postWriteReads, 0);
    QVERIFY(provider.readMethods.empty());
    const std::array classes{"UWF_Filter",  "UWF_Filter", "UWF_Servicing", "",           "UWF_OverlayConfig",  "UWF_OverlayConfig",
                             "UWF_Overlay", "UWF_Volume", "UWF_Volume",    "UWF_Volume", "UWF_RegistryFilter", ""};
    const std::string targetClass = classes.at(static_cast<std::size_t>(operation));
    QCOMPARE(provider.queries, targetClass.empty() ? std::vector<std::string>{} : std::vector<std::string>{"SELECT * FROM " + targetClass});
    if (fault == 3) {
      QCOMPARE(response.failure->diagnostics.front().code, std::int64_t{5});
      QVERIFY(response.failure->diagnostics.front().detail.find("original write rejection") != std::string::npos);
    }
  }

  void nextSessionWireMappings_data() {
    QTest::addColumn<int>("kind");
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("method");
    QTest::newRow("file") << 0 << QStringLiteral("volume-next") << QStringLiteral("AddExclusion");
    QTest::newRow("registry") << 1 << QStringLiteral("registry-next") << QStringLiteral("AddExclusion");
    QTest::newRow("protection") << 2 << QStringLiteral("volume-next") << QStringLiteral("Protect");
    QTest::newRow("type") << 3 << QStringLiteral("config-next") << QStringLiteral("SetType");
    QTest::newRow("persistence") << 4 << QStringLiteral("UWF_RegistryFilter") << QStringLiteral("PutInstance");
  }
  void nextSessionWireMappings() {
    QFETCH(int, kind);
    QFETCH(QString, path);
    QFETCH(QString, method);
    Provider provider;
    const std::vector<Request> requests{EditFileExclusion{volumeId, "\\Fixture", ExclusionAction::Add},
                                        EditRegistryExclusion{registryKey, ExclusionAction::Add}, SetVolume{volumeId, VolumeProperty::Protected, true},
                                        SetNumber{NumberProperty::OverlayType, 1}, SetBoolean{BooleanProperty::PersistDomainSecretKey, true}};
    const auto response = provider.backend().execute(requests.at(static_cast<std::size_t>(kind)));
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "");
    QCOMPARE(provider.methods.size(), std::size_t{1});
    QCOMPARE(QString::fromStdString(provider.methods.front()), method);
    QCOMPARE(QString::fromStdString(provider.paths.front()), path);
    if (kind == 0) QCOMPARE(provider.arguments.front().at("FileName").toString(), std::string("\\Fixture"));
    if (kind == 3) QCOMPARE(provider.arguments.front().at("type").toUInt(), 1u);
    if (kind == 4) {
      QCOMPARE(provider.arguments.front().size(), std::size_t{2});
      QVERIFY(!provider.arguments.front().at("CurrentSession").toBool());
      QVERIFY(provider.arguments.front().at("PersistDomainSecretKey").toBool());
      QVERIFY(!provider.arguments.front().contains("PersistTSCAL"));
    }
    QVERIFY(provider.row("volume-current").at("Protected").toBool());
  }
  void rawProtectionDoesNotComposeRegistrationOrBinding() {
    Provider provider;
    const auto response = provider.backend().execute(SetVolume{volumeId, VolumeProperty::Protected, true, "C:", true});
    QVERIFY(response.succeeded());
    QCOMPARE(provider.methods, std::vector<std::string>{"Protect"});
    QVERIFY(provider.row("volume-next").at("Protected").toBool());
    QVERIFY(!provider.row("volume-next").at("BindByDriveLetter").toBool());
    QVERIFY(provider.putModes.empty());
    QCOMPARE(provider.queries, std::vector<std::string>{"SELECT * FROM UWF_Volume"});
    QCOMPARE(provider.postWriteReads, 0);
  }
  void rawRegistrationUsesCreateOnlyWithoutExistenceOrMountQueries() {
    for (const bool duplicate : {false, true}) {
      Provider provider;
      if (duplicate) provider.invocationFailure = std::make_exception_ptr(windows::wmi::WmiProviderError(183, "CreateOnly"));
      const WmiUwfBackend backend(provider);
      const auto response = backend.execute(AddVolume{volumeId, {}});
      QCOMPARE(response.succeeded(), !duplicate);
      QCOMPARE(provider.putModes, std::vector<windows::wmi::WmiPutMode>{windows::wmi::WmiPutMode::CreateOnly});
      QCOMPARE(provider.methods, std::vector<std::string>{"PutInstance"});
      QVERIFY(provider.queries.empty());
      QVERIFY(provider.objectReads.empty());
      QVERIFY(!provider.arguments.front().contains("DriveLetter"));
      if (duplicate) QCOMPARE(response.failure->kind, ErrorKind::Conflict);
    }
  }
  void inventoryCannotPromiseThatEnumerationWasComplete() {
    Provider provider;
    provider.row("volume-current")["DriveLetter"] = WireValue{};
    auto synthetic = volumeRow(true);
    synthetic["__PATH"] = WireValue::fromString("synthetic");
    synthetic["VolumeName"] = WireValue::fromString("Volume{unconfigured}");
    synthetic["Protected"] = WireValue{};
    provider.tables["UWF_Volume"].push_back(synthetic);
    const auto response = provider.backend().execute(GetVolumes{Session::Current});
    QCOMPARE(provider.backend().probe(GetVolumes{Session::Current}).support, Support::Unsupported);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(provider.inventoryReads, 0);
  }
  void identityCacheDoesNotCacheBusinessState() {
    Provider provider;
    const auto backend = provider.backend();
    QVERIFY(backend.execute(GetFileExclusions{volumeId, Session::Current}).succeeded());
    QCOMPARE(provider.inventoryReads, 1);
    QVERIFY(backend.execute(GetFileExclusions{volumeId, Session::Current}).succeeded());
    QVERIFY(backend.execute(GetFileExclusions{volumeId, Session::Current}).succeeded());
    QCOMPARE(provider.inventoryReads, 1);
    provider.row("volume-current")["Protected"] = WireValue::fromBool(false);
    const auto property = backend.execute(GetVolumeProperty{volumeId, VolumeProperty::Protected, Session::Current});
    QVERIFY(property.succeeded());
    QVERIFY(!std::get<bool>(property.value));
    QCOMPARE(provider.inventoryReads, 1);
    QVERIFY(backend.execute(GetVolumes{Session::Current}).failure);
    QCOMPARE(provider.inventoryReads, 1);
  }
  void volumeQueriesValidateOnlyTheirRequiredFields_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<bool>("cached");
    QTest::addColumn<int>("operation");
    QTest::addColumn<QString>("field");
    QTest::addColumn<int>("fault");
    const std::array operations{"protected", "binding", "list"};
    const std::array fields{"DriveLetter", "BindByDriveLetter", "CommitPending", "Protected"};
    const std::array faults{"missing", "null", "malformed"};
    for (const bool current : {false, true})
      for (const bool cached : {false, true}) {
        if (cached && !current) continue;
        for (std::size_t operation = 0; operation < operations.size(); ++operation)
          for (const auto* field : fields)
            for (std::size_t fault = 0; fault < faults.size(); ++fault)
              QTest::newRow(qPrintable(QStringLiteral("%1/%2/%3/%4/%5")
                                           .arg(current ? QStringLiteral("Current") : QStringLiteral("Next"),
                                                cached ? QStringLiteral("cached") : QStringLiteral("initial"), QString::fromLatin1(operations[operation]),
                                                QString::fromLatin1(field), QString::fromLatin1(faults[fault]))))
                  << current << cached << int(operation) << QString::fromLatin1(field) << int(fault);
      }
  }
  void volumeQueriesValidateOnlyTheirRequiredFields() {
    QFETCH(bool, current);
    QFETCH(bool, cached);
    QFETCH(int, operation);
    QFETCH(QString, field);
    QFETCH(int, fault);
    Provider provider;
    const auto session = current ? Session::Current : Session::Next;
    const auto path = current ? "volume-current" : "volume-next";
    provider.exclusions[path] = {"\\Entry"};
    const auto backend = provider.backend();
    if (cached) QVERIFY(backend.execute(GetFileExclusions{volumeId, session}).succeeded());
    auto& row = provider.row(path);
    const auto name = field.toStdString();
    if (fault == 0)
      row.erase(name);
    else if (fault == 1)
      row[name] = WireValue{};
    else
      row[name] = name == "DriveLetter" ? WireValue::fromString("C:\\bad") : WireValue::fromUInt(1);
    const std::array requests{Request{GetVolumeProperty{volumeId, VolumeProperty::Protected, session}},
                              Request{GetVolumeProperty{volumeId, VolumeProperty::BindByDriveLetter, session}}, Request{GetFileExclusions{volumeId, session}},
                              Request{FindFileExclusion{volumeId, "\\Entry", session}}};
    const auto response = backend.execute(requests.at(static_cast<std::size_t>(operation)));
    std::optional<ErrorKind> expected;
    if (current && name == "Protected" && fault <= 1)
      expected = fault == 1 ? ErrorKind::NotFound : ErrorKind::Protocol;  // MI presence is the Current membership discriminator.
    else if ((operation == 0 && name == "Protected") || (operation == 1 && name == "BindByDriveLetter"))
      expected = ErrorKind::Protocol;
    QCOMPARE(response.failure.has_value(), expected.has_value());
    QCOMPARE(response.effect, Effect::None);
    if (expected)
      QCOMPARE(response.failure->kind, *expected);
    else if (operation == 2)
      QCOMPARE(std::get<std::vector<std::string>>(response.value), std::vector<std::string>{"\\Entry"});
    else
      QCOMPARE(std::get<bool>(response.value), operation == 0 ? current : operation == 3);
    QCOMPARE(provider.inventoryReads, 1);
    QVERIFY(provider.methods.empty());
  }
  void cachedFileMaintenanceResolvesIdentityWithoutReadingBusinessFields() {
    Provider provider;

    const WmiUwfBackend backend(provider);
    QVERIFY(backend.execute(GetFileExclusions{volumeId, Session::Current}).succeeded());
    provider.row("volume-current").erase("DriveLetter");
    provider.row("filter")["CurrentEnabled"] = WireValue{};
    const auto response = backend.execute(CommitFile{volumeId, "\\File"});
    QVERIFY(response.succeeded());
    QCOMPARE(provider.methods, std::vector<std::string>{"CommitFile"});
    QCOMPARE(provider.objectReads, std::vector<std::string>{"volume-current"});
    QCOMPARE(provider.queries, std::vector<std::string>{"SELECT * FROM UWF_Volume"});
    QCOMPARE(provider.postWriteReads, 0);
  }
  void cachedLocatorsRevalidateIdentityAndKeepTheExactFallback() {
    for (const int scenario : {0, 1, 2}) {
      Provider provider;
      const auto backend = provider.backend();
      QVERIFY(backend.execute(GetFileExclusions{volumeId, Session::Current}).succeeded());
      auto& row = provider.row("volume-current");
      const std::string exactPath = std::string("UWF_Volume.CurrentSession=TRUE,DriveLetter=\"\",VolumeName=\"") + volumeName + '"';
      if (scenario == 0) {
        row["__PATH"] = WireValue::fromString("relocated-current");
        provider.omittedVolumePaths.insert("relocated-current");
        provider.exactRows[exactPath] = row;
      } else if (scenario == 1)
        row["VolumeName"] = WireValue::fromString("Volume{66666666-7777-8888-9999-aaaaaaaaaaaa}");
      else
        row["CurrentSession"] = WireValue::fromBool(false);
      const auto response = backend.execute(GetFileExclusions{volumeId, Session::Current});
      QCOMPARE(response.succeeded(), scenario == 0);
      QCOMPARE(response.effect, Effect::None);
      QCOMPARE(provider.inventoryReads, scenario == 0 ? 2 : 1);
      if (scenario == 0)
        QCOMPARE(provider.objectReads, std::vector<std::string>({"volume-current", exactPath}));
      else {
        QCOMPARE(response.failure->kind, ErrorKind::Protocol);
        QCOMPARE(provider.readMethods, std::vector<std::string>{"GetExclusions"});
      }
      QVERIFY(provider.methods.empty());
    }
  }
  void volumePropertyFailuresDistinguishIdentityFromFields_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<int>("property");
    QTest::addColumn<int>("scenario");
    const std::array names{"missing-field", "wrong-type", "missing-identity", "inventory-read-denied"};
    for (const bool current : {false, true})
      for (const auto property : {VolumeProperty::Protected, VolumeProperty::BindByDriveLetter})
        for (std::size_t scenario = 0; scenario < names.size(); ++scenario)
          QTest::newRow(qPrintable(QStringLiteral("%1/%2/%3")
                                       .arg(current ? QStringLiteral("current") : QStringLiteral("next"))
                                       .arg(int(property))
                                       .arg(QString::fromLatin1(names[scenario]))))
              << current << int(property) << int(scenario);
  }
  void volumePropertyFailuresDistinguishIdentityFromFields() {
    QFETCH(bool, current);
    QFETCH(int, property);
    QFETCH(int, scenario);
    Provider provider;
    const auto selected = static_cast<VolumeProperty>(property);
    const auto field = selected == VolumeProperty::Protected ? "Protected" : "BindByDriveLetter";
    auto& target = provider.row(current ? "volume-current" : "volume-next");
    if (scenario == 0) target.erase(field);
    if (scenario == 1) target[field] = WireValue::fromUInt(1);
    if (scenario == 2) std::erase_if(provider.tables["UWF_Volume"], [&](const WmiRow& row) { return row.at("CurrentSession").toBool() == current; });
    if (scenario == 3)
      provider.queryFailure = std::make_exception_ptr(windows::wmi::WmiInfrastructureError(static_cast<std::int32_t>(windows::wmi::WmiErrorCode::AccessDenied),
                                                                                           "read volume identities", "inventory unavailable"));
    const auto response = provider.backend().execute(GetVolumeProperty{volumeId, selected, current ? Session::Current : Session::Next});
    QVERIFY(response.failure);
    const auto expected = scenario == 2 ? ErrorKind::NotFound : scenario == 3 ? ErrorKind::AccessDenied : ErrorKind::Protocol;
    QCOMPARE(response.failure->kind, expected);
    QCOMPARE(response.effect, Effect::None);
    QVERIFY(provider.methods.empty());
    QCOMPARE(provider.reads, 1);
  }
  void registryMaintenancePreservesTheRequestedTarget_data() {
    QTest::addColumn<QString>("key");
    QTest::addColumn<RegistryMaintenanceOperation>("operation");
    QTest::addColumn<bool>("supported");
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
    for (const auto& [root, supported] : targets)
      for (const bool descendant : {false, true})
        for (const auto& [operationName, operation] : operations) {
          const auto key = QString::fromLatin1(root) + (descendant ? QStringLiteral("\\Child") : QString{});
          QTest::newRow(qPrintable(key + QStringLiteral("/%1").arg(QString::fromLatin1(operationName)))) << key << operation << supported;
        }
  }
  void registryMaintenancePreservesTheRequestedTarget() {
    QFETCH(QString, key);
    QFETCH(RegistryMaintenanceOperation, operation);
    QFETCH(bool, supported);
    Provider provider;
    const auto backend = provider.backend();
    const auto name = operation == RegistryMaintenanceOperation::DeleteKey ? std::nullopt : std::optional<std::string>{"Named"};
    const Request request = operation == RegistryMaintenanceOperation::CommitValue ? Request{CommitRegistry{key.toStdString(), name}}
                                                                                   : Request{CommitRegistryDeletion{key.toStdString(), name}};
    QCOMPARE(backend.probe(request).support, supported ? Support::Unknown : Support::Unsupported);
    const auto result = backend.execute(request);
    QCOMPARE(result.succeeded(), supported);
    QCOMPARE(result.effect, supported ? Effect::Confirmed : Effect::None);
    if (supported) {
      QCOMPARE(provider.classProbes, 1);
      QCOMPARE(provider.methods.size(), std::size_t{1});
      QCOMPARE(provider.arguments.front().at("RegistryKey").toString(), key.toStdString());
      QCOMPARE(provider.queries, std::vector<std::string>{"SELECT * FROM UWF_RegistryFilter"});
      QCOMPARE(provider.postWriteReads, 0);
    } else {
      QVERIFY(result.failure);
      QCOMPARE(result.failure->kind, ErrorKind::Unsupported);
      QCOMPARE(provider.classProbes, 0);
      QCOMPARE(provider.reads, 0);
      QVERIFY(provider.methods.empty());
    }
  }
  void registryTargetsPreserveValueIdentity_data() {
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<bool>("wholeKey");
    QTest::addColumn<bool>("named");
    QTest::addColumn<bool>("supported");
    QTest::newRow("commit-key-unavailable") << false << true << false << false;
    QTest::newRow("commit-default") << false << false << false << true;
    QTest::newRow("delete-key") << true << true << false << true;
    QTest::newRow("delete-default-ambiguous") << true << false << false << false;
    QTest::newRow("delete-named-value") << true << false << true << true;
  }
  void registryTargetsPreserveValueIdentity() {
    QFETCH(bool, deletion);
    QFETCH(bool, wholeKey);
    QFETCH(bool, named);
    QFETCH(bool, supported);
    Provider provider;
    const auto name = wholeKey ? std::nullopt : std::optional<std::string>{named ? "Value" : ""};
    const Request request = deletion ? Request{CommitRegistryDeletion{registryKey, name}} : Request{CommitRegistry{registryKey, name}};
    const auto response = provider.backend().execute(request);
    QCOMPARE(provider.backend().probe(request).support, supported ? Support::Unknown : Support::Unsupported);
    QCOMPARE(response.succeeded(), supported);
    QCOMPARE(response.effect, supported ? Effect::Confirmed : Effect::None);
    if (supported) {
      QCOMPARE(provider.methods, std::vector<std::string>{deletion ? "CommitRegistryDeletion" : "CommitRegistry"});
      const auto& args = provider.arguments.front();
      QCOMPARE(args.contains("ValueName"), !wholeKey);
      if (name) QCOMPARE(args.at("ValueName").toString(), *name);
    } else {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
      QVERIFY(provider.methods.empty());
    }
  }
  void overlayReadFailuresRetainCompletionEvidence_data() {
    QTest::addColumn<int>("fault");
    QTest::addColumn<int>("completion");
    QTest::newRow("instance-preparation") << 0 << int(ReadCompletion::NotDispatched);
    QTest::newRow("method-preparation") << 1 << int(ReadCompletion::NotDispatched);
    QTest::newRow("dispatched-transport-loss") << 2 << int(ReadCompletion::Unknown);
    QTest::newRow("client-cancellation") << 3 << int(ReadCompletion::Unknown);
    QTest::newRow("provider-return-value") << 4 << int(ReadCompletion::ProviderCompleted);
    QTest::newRow("completed-malformed-array") << 5 << int(ReadCompletion::ProviderCompleted);
    QTest::newRow("untyped-dispatch-failure") << 6 << int(ReadCompletion::Unknown);
    QTest::newRow("completed-output-transport-loss") << 7 << int(ReadCompletion::ProviderCompleted);
  }
  void overlayReadFailuresRetainCompletionEvidence() {
    QFETCH(int, fault);
    QFETCH(int, completion);
    Provider provider;
    if (fault == 0) {
      provider.queryFailure = std::make_exception_ptr(windows::wmi::WmiTransportError(-1, "prepare overlay instance"));
    } else if (fault == 5) {
      provider.filesOutput.arrays["OverlayFiles"] = {{{"FileName", WireValue::fromString("\\Malformed")}}};
    } else if (fault == 6) {
      provider.singleReadFailure = std::make_exception_ptr(std::runtime_error("untyped transport failure"));
    } else {
      const auto retain = [&](auto failure) {
        failure.setReadCompletion(static_cast<windows::wmi::ReadCompletion>(completion));
        provider.singleReadFailure = std::make_exception_ptr(std::move(failure));
      };
      if (fault == 1)
        retain(windows::wmi::WmiInfrastructureError(-1, "prepare overlay method"));
      else if (fault == 3)
        retain(windows::wmi::WmiCancelled("cancel client while provider runs"));
      else if (fault == 4)
        retain(windows::wmi::WmiProviderError(87, "completed overlay method"));
      else
        retain(windows::wmi::WmiTransportError(-1, "overlay transport failure"));
    }
    const auto result = provider.backend().execute(GetOverlayFiles{volumeId});
    QVERIFY(result.failure);
    QCOMPARE(int(result.failure->readCompletion), completion);
    QCOMPARE(result.effect, Effect::None);
    QCOMPARE(provider.singleReads, fault == 0 ? 0 : 1);
    QCOMPARE(provider.readMethods.size(), fault == 5 ? std::size_t{1} : std::size_t{0});
  }
  void aLegacyTransportCannotSilentlyReplayAnExpensiveRead() {
    Provider provider;
    try {
      (void)provider.WmiOperations::callMethodReadOnce("overlay", "GetOverlayFiles", {}, {});
      QFAIL("An unimplemented single-dispatch transport was accepted");
    } catch (const windows::wmi::WmiException& error) {
      QCOMPARE(error.readCompletion(), windows::wmi::ReadCompletion::NotDispatched);
    }
    QVERIFY(provider.readMethods.empty());
    QCOMPARE(provider.singleReads, 0);
  }
  void fixedScopeAndRuntimeOutputsStayTyped() {
    Provider provider;
    const auto backend = provider.backend();
    provider.row("filter")["CurrentEnabled"] = WireValue::fromBool(true);
    provider.row("filter")["HORMEnabled"] = WireValue::fromBool(true);
    for (Session session : {Session::Current, Session::Next}) {
      QVERIFY(compat::call(backend, GetBoolean{BooleanProperty::HormEnabled, session}));
      QCOMPARE(compat::call(backend, GetNumber{NumberProperty::WarningThreshold, session}), 64u);
      QCOMPARE(compat::call(backend, GetNumber{NumberProperty::CriticalThreshold, session}), 128u);
    }
    QCOMPARE(compat::call(backend, GetOverlayUsage{}), (OverlayUsage{7, 93}));
    provider.filesOutput.arrays["OverlayFiles"] = {{{"FileName", WireValue::fromString("\\File")}, {"FileSize", WireValue::fromUInt(17)}}};
    QCOMPARE(compat::call(backend, GetOverlayFiles{volumeId}), (std::vector<OverlayFile>{{"\\File", 17}}));
  }
  void windowsUpdateKeepsItsIndependentOutput_data() {
    QTest::addColumn<int>("status");
    for (int value : {-1, 0, 1, 2, 3}) QTest::newRow(qPrintable(QString::number(value))) << value;
  }
  void windowsUpdateKeepsItsIndependentOutput() {
    QFETCH(int, status);
    Provider provider;
    if (status >= 0) provider.writeOutput.values["UpdateStatus"] = WireValue::fromUInt(static_cast<std::uint32_t>(status));
    const Request request = UpdateWindows{};
    QVERIFY(isWrite(request));
    const auto response = provider.backend().execute(request);
    QCOMPARE(provider.methods, (std::vector<std::string>{"UpdateWindows"}));
    QCOMPARE(provider.paths, (std::vector<std::string>{"servicing-current"}));
    QVERIFY(provider.arguments.front().empty());
    if (status >= 0 && status <= 2) {
      QVERIFY(response.succeeded());
      QCOMPARE(response.effect, Effect::Confirmed);
      QCOMPARE(std::get<WindowsUpdateOutcome>(response.value), static_cast<WindowsUpdateOutcome>(status));
    } else {
      QVERIFY(!response.succeeded());
      QCOMPARE(response.failure->kind, ErrorKind::Protocol);
      QCOMPARE(response.effect, Effect::Uncertain);
    }
  }
  void rawExecutionDefersPermissionToProviderAndCancellationPrecedesDispatch() {
    Provider provider;
    const auto accepted = provider.backend().execute(SetBoolean{BooleanProperty::FilterEnabled, true});
    QVERIFY(accepted.succeeded());
    QCOMPARE(provider.methods.size(), std::size_t{1});
    const auto reads = provider.reads;
    std::stop_source stop;
    stop.request_stop();
    const auto canceled = provider.backend().execute(SetBoolean{BooleanProperty::FilterEnabled, false}, {stop.get_token()});
    QVERIFY(canceled.failure);
    QCOMPARE(canceled.failure->kind, ErrorKind::Canceled);
    QCOMPARE(canceled.effect, Effect::None);
    QCOMPARE(provider.reads, reads);
    QCOMPARE(provider.methods.size(), std::size_t{1});
  }
  void cancellationAfterLocatorResolutionPrecedesTheWrite() {
    Provider provider;
    std::stop_source stop;
    provider.afterQuery = [&] { stop.request_stop(); };
    const auto response = provider.backend().execute(SetBoolean{BooleanProperty::FilterEnabled, false}, {stop.get_token()});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Canceled);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(provider.reads, 1);
    QVERIFY(provider.methods.empty());
  }
  void malformedDuplicateInstancesAreNotArbitrarilySelected() {
    Provider provider;
    provider.tables["UWF_Filter"].push_back(provider.tables["UWF_Filter"].front());
    const auto response = provider.backend().execute(GetBoolean{BooleanProperty::FilterEnabled});
    QVERIFY(!response.succeeded());
    QCOMPARE(response.failure->kind, ErrorKind::Protocol);
    QCOMPARE(response.effect, Effect::None);
  }
  void anEmptySingletonEnumerationDoesNotProveNotFound() {
    Provider provider;
    provider.tables["UWF_Filter"].clear();
    const auto response = provider.backend().execute(GetBoolean{BooleanProperty::FilterEnabled});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Protocol);
    QCOMPARE(response.effect, Effect::None);
    QVERIFY(provider.methods.empty());
  }
  void capturedFilterSessionVectorsUseRealWmiRows() {
    Provider provider;
    const auto backend = provider.backend();
    for (const auto& vector : test::captured::filterSessionReads()) {
      const auto response = backend.execute(vector.request);
      QVERIFY2(response.succeeded(), vector.id);
      QCOMPARE(response.effect, vector.expected.effect);
      QVERIFY(vector.expected.value);
      QCOMPARE(std::get<bool>(response.value), std::get<bool>(*vector.expected.value));
    }
  }
  void anEnumerationMissRequiresAnExactVolumeObservation_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<int>("scenario");
    const std::array names{"omitted-but-present", "not-found", "denied", "wrong-volume", "wrong-session", "missing-protected", "synthetic-current"};
    for (const bool current : {false, true})
      for (std::size_t scenario = 0; scenario < names.size(); ++scenario) {
        if (!current && scenario == 6) continue;
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(current).arg(QString::fromLatin1(names[scenario])))) << current << int(scenario);
      }
  }
  void anEnumerationMissRequiresAnExactVolumeObservation() {
    QFETCH(bool, current);
    QFETCH(int, scenario);
    Provider provider;
    const std::string path = std::string("UWF_Volume.CurrentSession=") + (current ? "TRUE" : "FALSE") + ",DriveLetter=\"\",VolumeName=\"" + volumeName + '"';
    provider.omittedVolumePaths.insert(current ? "volume-current" : "volume-next");
    auto exact = volumeRow(current, true);
    if (scenario == 1 || scenario == 2)
      provider.exactFailure = std::make_exception_ptr(windows::wmi::WmiInfrastructureError(
          static_cast<std::int32_t>(scenario == 1 ? windows::wmi::WmiErrorCode::NotFound : windows::wmi::WmiErrorCode::AccessDenied), "exact volume lookup"));
    if (scenario == 3) exact["VolumeName"] = WireValue::fromString("Volume{66666666-7777-8888-9999-aaaaaaaaaaaa}");
    if (scenario == 4) exact["CurrentSession"] = WireValue::fromBool(!current);
    if (scenario == 5) exact.erase("Protected");
    if (scenario == 6) exact["Protected"] = WireValue{};
    provider.exactRows[path] = std::move(exact);
    const auto response = provider.backend().execute(GetVolumeProperty{volumeId, VolumeProperty::Protected, current ? Session::Current : Session::Next});
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(provider.objectReads, std::vector<std::string>{path});
    QVERIFY(provider.methods.empty());
    if (scenario == 0) {
      QVERIFY(response.succeeded());
      QVERIFY(std::get<bool>(response.value));
    } else {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, scenario == 1 || scenario == 6 ? ErrorKind::NotFound : scenario == 2 ? ErrorKind::AccessDenied : ErrorKind::Protocol);
    }
  }
  void derivedRegistryPersistenceDoesNotQueryTheProvider() {
    Provider provider;
    const auto response = provider.backend().execute(GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
    QCOMPARE(response.effect, Effect::None);
    QVERIFY(provider.queries.empty());
    QVERIFY(provider.objectReads.empty());
  }
  void registrationDoesNotQueryAnOmittedVolume() {
    Provider provider;
    provider.omittedVolumePaths.insert("volume-next");
    const auto response = provider.backend().execute(AddVolume{volumeId, "C:"});
    QVERIFY(response.succeeded());
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(provider.methods, std::vector<std::string>{"PutInstance"});
    QVERIFY(provider.objectReads.empty());
    QVERIFY(provider.queries.empty());
  }
  void unknownPowerOutcomeIsNeverReplayed() {
    Provider provider;
    provider.afterWrite = [] { throw windows::wmi::WmiInvocationUncertain("Restart", "lost"); };
    auto first = std::make_shared<WmiUwfBackend>(provider);
    auto second = std::make_shared<AcceptingBackend>();
    const auto response = FallbackUwf({first, second}).execute(Power{PowerAction::Restart});
    QVERIFY(!response.succeeded());
    QCOMPARE(response.effect, Effect::Uncertain);
    QCOMPARE(provider.methods.size(), std::size_t{1});
    QCOMPARE(second->writes, 0);
  }
};
QTEST_APPLESS_MAIN(WmiUwfBackendTests)
#include "WmiUwfBackendTests.moc"
