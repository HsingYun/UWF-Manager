// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
// clang-format off
#include <Windows.h>
#include <OleAuto.h>
// clang-format on

#include <QTest>
#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "application/commit/CommitWorkflow.h"
#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/operations/contracts/OperationError.h"
#include "application/overlay/HormEligibility.h"
#include "application/staging/execution/FileStagingCommitter.h"
#include "application/uwf/CompositeAllVolumeProtection.h"
#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/adapters/UwfApplicationStateSource.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "application/uwf/adapters/UwfOverlayMaintenance.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "composition/uwf/SystemUwf.h"
#include "infrastructure/uwf/routing/FallbackUwf.h"
#include "infrastructure/uwf/windows/native/NativeUwfAbi.h"
#include "infrastructure/uwf/windows/native/NativeUwfBackend.h"
#include "infrastructure/uwf/windows/wmi/WmiUwfBackend.h"
#include "infrastructure/windows/management/wmi/WmiError.h"
#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/storage/WindowsVolumeRepository.h"
#include "infrastructure/windows/text/UnicodeConversion.h"
#include "simulation/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;
constexpr auto memoryVolume = "11111111-2222-3333-4444-555555555555";
class MemoryCommitSource final : public app::CommitTraversalSource {
 public:
  app::CommitTargetKind kind = app::CommitTargetKind::File;
  bool present = true;
  bool unexpectedChild = false;
  mutable int inspections = 0;
  mutable int verifications = 0;
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override {
    ++inspections;
    if (!present) return std::nullopt;
    auto node = target;
    node.kind = kind;
    return node;
  }
  bool exists(const app::CommitNode&) const override {
    ++verifications;
    return present;
  }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  app::FileCommitTarget resolveFileTarget(const QString& path) const override { return {memoryVolume, path.mid(2)}; }
  QString enumerateChildren(const app::CommitNode& target, std::stop_token, const app::CommitNodeVisitor& visitor) const override {
    if (unexpectedChild) visitor({kind, target.path + QStringLiteral("\\new"), {}, {}});
    return {};
  }
};
windows::wmi::WmiRow commitProviderRow(bool registry, const std::string& driveLetter = "C:") {
  using Value = windows::wmi::WmiValue;
  if (registry)
    return {{"__PATH", Value::fromString("registry")},
            {"CurrentSession", Value::fromBool(true)},
            {"PersistDomainSecretKey", Value::fromBool(false)},
            {"PersistTSCAL", Value::fromBool(false)}};
  return {{"__PATH", Value::fromString("volume")},
          {"CurrentSession", Value::fromBool(true)},
          {"DriveLetter", Value::fromString(driveLetter)},
          {"VolumeName", Value::fromString("Volume{" + std::string(memoryVolume) + '}')},
          {"BindByDriveLetter", Value::fromBool(true)},
          {"CommitPending", Value::fromBool(false)},
          {"Protected", Value::fromBool(true)}};
}

class MemoryWmiOperations final : public uwf::windows::wmi::WmiOperations {
 public:
  struct Invocation {
    std::string path;
    std::string method;
    windows::wmi::WmiRow inputs;
  };
  bool currentEnabled = false;
  bool volumeConfigured = false;
  bool wmiMaintenanceAvailable = true;
  std::string driveLetter = "C:";
  std::shared_ptr<MemoryCommitSource> commitSource = std::make_shared<MemoryCommitSource>();
  mutable bool nextEnabled = false;
  mutable int writes = 0;
  mutable std::vector<Invocation> invocations;
  mutable std::vector<Invocation> nativeInvocations;
  mutable std::vector<std::string> queries;
  bool omitVolumeFromEnumeration = false;
  std::function<void()> invocationObserver;
  windows::WindowsConfigurationEnvironment environment() const {
    return {
        .isAdministrator = [] { return true; },
        .hiberFileExists = [] { return true; },
        .systemVolumeId = [] { return std::string(memoryVolume); },
    };
  }
  void ensureConnected() const override {}
  std::vector<uwf::windows::wmi::WmiRow> query(const std::string& query) const override {
    if (query == "SELECT DeviceID, FileSystem, VolumeName, Size, FreeSpace, DriveType FROM Win32_LogicalDisk WHERE DriveType = 2 OR DriveType = 3" ||
        query == "SELECT DeviceID, DriveLetter FROM Win32_Volume")
      return {};
    throw std::logic_error("unexpected inventory query: " + query);
  }
  std::vector<uwf::windows::wmi::WmiRow> queryInstances(const std::string& query) const override {
    queries.push_back(query);
    if (query == "SELECT * FROM UWF_Filter") return {filterRow()};
    if (query == "SELECT * FROM UWF_Volume")
      return volumeConfigured && !omitVolumeFromEnumeration ? std::vector{commitProviderRow(false, driveLetter)} : std::vector<windows::wmi::WmiRow>{};
    if (query == "SELECT * FROM UWF_RegistryFilter") return {commitProviderRow(true)};
    throw std::logic_error("unexpected instance query: " + query);
  }
  uwf::windows::wmi::WmiClassStatus classStatus(const std::string& name) const override {
    if (!wmiMaintenanceAvailable && (name == "UWF_Volume" || name == "UWF_RegistryFilter")) return windows::wmi::WmiClassStatus::Missing;
    return name == "UWF_Filter" || name == "UWF_Volume" || name == "UWF_RegistryFilter" ? windows::wmi::WmiClassStatus::Present
                                                                                        : windows::wmi::WmiClassStatus::Missing;
  }
  uwf::windows::wmi::WmiRow getObject(const std::string& path) const override {
    if (path.starts_with("UWF_Volume.CurrentSession=TRUE,")) {
      if (volumeConfigured) return commitProviderRow(false, driveLetter);
      throw windows::wmi::WmiInfrastructureError(static_cast<std::int32_t>(windows::wmi::WmiErrorCode::NotFound), "exact volume lookup");
    }
    if (path == "filter") return filterRow();
    if (path == "volume" && volumeConfigured) return commitProviderRow(false, driveLetter);
    if (path == "registry") return commitProviderRow(true);
    throw std::logic_error("unexpected object read: " + path);
  }
  uwf::windows::wmi::WmiMethodOutput invokeMethod(const std::string& path, const std::string& method, const uwf::windows::wmi::WmiRow& inputs) const override {
    if (path == "filter" && (method == "Enable" || method == "Disable"))
      nextEnabled = method == "Enable";
    else if (!(path == "volume" && volumeConfigured && (method == "CommitFile" || method == "CommitFileDeletion")) &&
             !(path == "registry" && (method == "CommitRegistry" || method == "CommitRegistryDeletion")))
      throw std::logic_error("unexpected write: " + path + "::" + method);
    invocations.push_back({path, method, inputs});
    ++writes;
    if (invocationObserver) invocationObserver();
    return {};
  }
  uwf::windows::wmi::WmiMethodOutput callMethodRead(const std::string& path, const std::string& method, const uwf::windows::wmi::WmiRow&) const override {
    if (method != "GetExclusions" || (path != "volume" && path != "registry")) throw std::logic_error("unexpected method read: " + path + "::" + method);
    windows::wmi::WmiMethodOutput result;
    result.arrays[path == "registry" ? "ExcludedKeys" : "ExcludedFiles"] = {};
    return result;
  }
  void putInstance(const std::string&, const uwf::windows::wmi::WmiRow&, uwf::windows::wmi::WmiPutMode) const override {
    throw std::logic_error("unexpected instance write");
  }

 private:
  uwf::windows::wmi::WmiRow filterRow() const {
    return {{"__PATH", uwf::windows::wmi::WmiValue::fromString("filter")},
            {"CurrentEnabled", uwf::windows::wmi::WmiValue::fromBool(currentEnabled)},
            {"HORMEnabled", uwf::windows::wmi::WmiValue::fromBool(false)},
            {"NextEnabled", uwf::windows::wmi::WmiValue::fromBool(nextEnabled)}};
  }
};

// Exercise NativeUwfBackend through injected C ABI exports with matching allocators.
// Native and WMI fixtures share data, not execution paths.
thread_local MemoryWmiOperations* activeNativeTransport = nullptr;
struct NativeFixtureLease {
  explicit NativeFixtureLease(MemoryWmiOperations& transport) : prior(std::exchange(activeNativeTransport, &transport)) {}
  ~NativeFixtureLease() { activeNativeTransport = prior; }
  MemoryWmiOperations* prior;
};
HRESULT __cdecl nativeFilter(const bool current, bool* value) {
  *value = current ? activeNativeTransport->currentEnabled : activeNativeTransport->nextEnabled;
  return S_OK;
}
HRESULT __cdecl nativeHorm(bool* value) {
  *value = false;
  return S_OK;
}
HRESULT __cdecl nativeFlags(bool, ULONG* value) {
  *value = 0;
  return S_OK;
}
HRESULT __cdecl nativeExists(bool, const wchar_t*) { return activeNativeTransport->volumeConfigured ? S_OK : HRESULT_FROM_WIN32(ERROR_NOT_FOUND); }
HRESULT __cdecl nativeProtected(bool, const wchar_t*, bool* value) {
  *value = activeNativeTransport->volumeConfigured;
  return S_OK;
}
HRESULT __cdecl nativeBound(bool, const wchar_t*, bool* value) {
  *value = true;
  return S_OK;
}
void __cdecl nativeRelease(void* value) { HeapFree(GetProcessHeap(), 0, value); }
HRESULT __cdecl nativeVolumes(bool, windows::backends::native::StringArray* output) {
  if (!activeNativeTransport->volumeConfigured) return S_OK;
  output->items = static_cast<BSTR*>(HeapAlloc(GetProcessHeap(), 0, sizeof(BSTR)));
  if (!output->items) return E_OUTOFMEMORY;
  output->capacity = 1;
  const auto id = windows::utf8ToWide(memoryVolume);
  output->items[0] = SysAllocStringLen(id.data(), static_cast<UINT>(id.size()));
  if (!output->items[0]) return E_OUTOFMEMORY;
  output->count = 1;
  return S_OK;
}
HRESULT __cdecl nativeDrive(bool, const wchar_t*, wchar_t** output) {
  const auto drive = windows::utf8ToWide(activeNativeTransport->driveLetter);
  *output = static_cast<wchar_t*>(HeapAlloc(GetProcessHeap(), 0, (drive.size() + 1) * sizeof(wchar_t)));
  if (!*output) return E_OUTOFMEMORY;
  std::copy(drive.c_str(), drive.c_str() + drive.size() + 1, *output);
  return S_OK;
}
HRESULT recordNative(const std::string& method, const std::string& path, windows::wmi::WmiRow inputs) {
  auto& transport = *activeNativeTransport;
  transport.nativeInvocations.push_back({path, method, std::move(inputs)});
  if (transport.invocationObserver) transport.invocationObserver();
  return S_OK;
}
HRESULT __cdecl nativeCommitFile(const wchar_t*, const wchar_t* path) {
  return recordNative("CommitFile", "volume", {{"FileName", windows::wmi::WmiValue::fromString(windows::wideToUtf8(path))}});
}
HRESULT __cdecl nativeDeleteFile(const wchar_t*, const wchar_t* path) {
  return recordNative("CommitFileDeletion", "volume", {{"FileName", windows::wmi::WmiValue::fromString(windows::wideToUtf8(path))}});
}
HRESULT __cdecl nativeDeleteRegistry(const bool hasValue, const wchar_t* key, const wchar_t* value) {
  windows::wmi::WmiRow inputs{{"RegistryKey", windows::wmi::WmiValue::fromString(windows::wideToUtf8(key))}};
  if (hasValue) inputs["ValueName"] = windows::wmi::WmiValue::fromString(windows::wideToUtf8(value));
  return recordNative("CommitRegistryDeletion", "registry", std::move(inputs));
}
std::shared_ptr<compat::Backend> nativeFixture(MemoryWmiOperations& transport) {
  using namespace windows::backends;
  const auto lease = std::make_shared<NativeFixtureLease>(transport);
  NativeUwfBindings bindings;
  bindings.supportsTscalObservation = true;
  bindings.resolveExport = [lease](const std::string_view symbol) -> NativeUwfProcedure {
    (void)lease;
    if (symbol == "UwfCfgGetFilterEnabled") return reinterpret_cast<NativeUwfProcedure>(nativeFilter);
    if (symbol == "UwfCfgGetHormEnabled") return reinterpret_cast<NativeUwfProcedure>(nativeHorm);
    if (symbol == "UwfCfgGetOverlayFlags") return reinterpret_cast<NativeUwfProcedure>(nativeFlags);
    if (symbol == "UwfCfgGetVolumes") return reinterpret_cast<NativeUwfProcedure>(nativeVolumes);
    if (symbol == "UwfCfgGetDriveLetter") return reinterpret_cast<NativeUwfProcedure>(nativeDrive);
    if (symbol == "UwfCfgGetVolumeProtected") return reinterpret_cast<NativeUwfProcedure>(nativeProtected);
    if (symbol == "UwfCfgGetVolumeBindByDriveLetter") return reinterpret_cast<NativeUwfProcedure>(nativeBound);
    if (symbol == "UwfCfgDoesVolumeExist") return reinterpret_cast<NativeUwfProcedure>(nativeExists);
    if (symbol == "UwfCfgCommitFile") return reinterpret_cast<NativeUwfProcedure>(nativeCommitFile);
    if (symbol == "UwfCfgCommitFileDeletion") return reinterpret_cast<NativeUwfProcedure>(nativeDeleteFile);
    if (symbol == "UwfCfgCommitRegistryDeletion") return reinterpret_cast<NativeUwfProcedure>(nativeDeleteRegistry);
    return nullptr;
  };
  bindings.resolveImport = [lease](const std::string_view symbol) -> NativeUwfProcedure {
    (void)lease;
    return symbol == "free" || symbol == "??_V@YAXPEAX@Z" ? reinterpret_cast<NativeUwfProcedure>(nativeRelease) : nullptr;
  };
  return std::make_shared<NativeUwfBackend>(std::move(bindings));
}

compat::UwfStateFacts memoryState(const MemoryWmiOperations& transport, const bool flagsObserved = true) {
  using namespace compat;
  UwfStateFacts state;
  state.administrator = Fact<bool>::known(true);
  state.systemVolumeId = Fact<std::string>::known(memoryVolume);
  state.current.filterEnabled = Fact<bool>::known(transport.currentEnabled);
  state.next.filterEnabled = Fact<bool>::known(transport.nextEnabled);
  state.current.overlayFlags = flagsObserved ? BitFacts::known(0) : BitFacts{};
  state.runtime.horm = Fact<bool>::known(false);
  state.runtime.controlDevicePresent = Fact<bool>::known(true);
  state.runtime.currentControlSet = Fact<std::uint32_t>::known(1);
  state.current.registryExclusions = Fact<std::vector<std::string>>::known({});
  state.current.volumesObserved = true;
  auto& volume = state.current.volumes[memoryVolume];
  volume.present = Fact<bool>::known(transport.volumeConfigured);
  volume.protectedVolume = Fact<bool>::known(transport.volumeConfigured);
  volume.bindByDriveLetter = Fact<bool>::known(true);
  volume.driveLetter = Fact<std::string>::known(transport.driveLetter);
  volume.physicalDriveLetter = Fact<std::string>::known(transport.driveLetter);
  volume.fileExclusions = Fact<std::vector<std::string>>::known({});
  return state;
}

std::shared_ptr<app::UwfMaintenanceRepository> maintenanceRepository(MemoryWmiOperations& transport) {
  const auto wmi = std::make_shared<windows::backends::WmiUwfBackend>(transport);
  auto system =
      std::make_shared<app::PrecheckedUwf>(std::make_shared<compat::FallbackUwf>(std::vector<std::shared_ptr<compat::Backend>>{wmi, nativeFixture(transport)}));
  system->updateState(memoryState(transport));
  return app::uwfMaintenanceRepository(system);
}

class MutablePhysicalVolumes final : public app::VolumeRepository {
 public:
  std::vector<core::VolumeInfo> volumes;
  mutable int reads = 0;
  std::vector<core::VolumeInfo> read() const override {
    ++reads;
    return volumes;
  }
  std::string driveForPath(const std::string&) const override { return {}; }
  std::vector<std::string> mountedPaths(const std::string&) const override { return {}; }
  std::vector<std::string> mountedLetters(const std::string&) const override { return {}; }
  std::string systemDrive() const override { return "C:"; }
};

class NativeUwfCompositionTests final : public QObject {
  Q_OBJECT
  enum class CommitEntry { PrepareFile, PrepareRegistry, RegistryWrite, CommitFile, DeleteFile, CommitRegistry, DeleteRegistry };
  Q_ENUM(CommitEntry)
 private slots:
  void initialRefreshMatchesConfiguredAliasesFromTheSameInventory_data() {
    QTest::addColumn<bool>("reverse");
    QTest::addColumn<bool>("configuredMounted");
    QTest::addColumn<bool>("initializeFactory");
    for (const bool reverse : {false, true})
      for (const bool configuredMounted : {false, true})
        for (const bool initializeFactory : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("reverse-%1/configured-mounted-%2/factory-%3").arg(reverse).arg(configuredMounted).arg(initializeFactory)))
              << reverse << configuredMounted << initializeFactory;
  }
  void initialRefreshMatchesConfiguredAliasesFromTheSameInventory() {
    QFETCH(bool, reverse);
    QFETCH(bool, configuredMounted);
    QFETCH(bool, initializeFactory);
    using namespace compat;
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    initial.current.volumes.emplace(memoryVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{
                                                      .volume = {memoryVolume, configuredMounted ? "F:" : "G:", true, false, false}, .fileExclusions = {}});
    initial.next = initial.current;
    initial.next.volumes.at(memoryVolume).volume.driveLetter = "E:";
    initial.runtime.fileMaintenancePrefixes[memoryVolume] = "F:";
    const auto native = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    native->seedFile(memoryVolume, "\\Item", "before");
    native->writeFile(memoryVolume, "\\Item", "after");
    // The production router has no published configuration before its first
    // refresh. Do not seed the global state from the native fixture here.
    const auto router = std::make_shared<FallbackUwf>(std::vector<std::shared_ptr<Backend>>{native->rawBackend()});
    const auto physical = std::make_shared<MutablePhysicalVolumes>();
    physical->volumes = {{.driveLetter = "E:", .volumeName = memoryVolume, .fileSystem = "NTFS", .label = {}, .physical = {true, true}},
                         {.driveLetter = "F:", .volumeName = memoryVolume, .fileSystem = "NTFS", .label = {}, .physical = {true, true}}};
    if (reverse) std::ranges::reverse(physical->volumes);
    windows::WindowsConfigurationEnvironment environment;
    environment.isAdministrator = [] { return true; };
    const auto system = composition::uwfWithStateRefresh(router, environment, false, initializeFactory ? physical : nullptr);
    if (!initializeFactory) {
      const auto source = app::uwfApplicationStateSource(system, physical, true);
      const auto observed = source->readOperation(app::UwfCapability::Available)({});
      QVERIFY(observed.snapshot.uwfAvailable);
    }
    QCOMPARE(physical->reads, 1);
    const auto published = system->state();
    QVERIFY(!published.refreshFailure);
    QCOMPARE(std::ranges::count(native->primitiveRequests(), Request{GetBoolean{BooleanProperty::FilterEnabled, Session::Current}}), std::ptrdiff_t{1});
    QCOMPARE(published.current.volumes.at(memoryVolume).driveLetter.value, std::optional<std::string>{configuredMounted ? "F:" : "G:"});
    QCOMPARE(published.next.volumes.at(memoryVolume).physicalDriveLetter.value, std::optional<std::string>{"E:"});
    if (configuredMounted) QCOMPARE(published.current.volumes.at(memoryVolume).physicalDriveLetter.value, std::optional<std::string>{"F:"});
    native->clearTrace();
    native->clearPrimitiveRequests();
    const auto receipt = system->execute(CommitFile{memoryVolume, "\\Item"});
    QCOMPARE(receipt.succeeded(), configuredMounted);
    QCOMPARE(physical->reads, 1);  // Dispatch consumes the published identity.
    if (configuredMounted) {
      QCOMPARE(receipt.effect, Effect::Confirmed);
      QCOMPARE(native->primitiveRequests(), (std::vector<Request>{CommitFile{memoryVolume, "\\Item"}}));
      QCOMPARE(native->readFile(memoryVolume, "\\Item"), std::optional<std::string>{"after"});
    } else {
      QVERIFY(receipt.failure);
      QCOMPARE(receipt.failure->kind, ErrorKind::Conflict);
      QCOMPARE(receipt.effect, Effect::None);
      QVERIFY(native->primitiveRequests().empty());
    }
  }
  void globalRefreshReplacesProtectionScopeAndAdmissionWithThePhysicalInventory_data() {
    QTest::addColumn<bool>("currentFilter");
    QTest::newRow("inactive") << false;
    QTest::newRow("active") << true;
  }
  void globalRefreshReplacesProtectionScopeAndAdmissionWithThePhysicalInventory() {
    QFETCH(bool, currentFilter);
    constexpr auto added = "22222222-2222-2222-2222-222222222222";
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = currentFilter;
    initial.current.volumes.emplace(
        memoryVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {memoryVolume, "C:", false, false, false}, .fileExclusions = {}});
    initial.next = initial.current;
    const auto raw = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    const auto physical = std::make_shared<MutablePhysicalVolumes>();
    physical->volumes = {{.driveLetter = "C:", .volumeName = memoryVolume, .fileSystem = "NTFS", .label = {}, .physical = {true, true}}};
    windows::WindowsConfigurationEnvironment environment;
    environment.isAdministrator = [] { return true; };
    int inventories = 0;
    bool fail = false;
    bool eligible = true;
    environment.volumeProtectionInventory = [&](bool active, const compat::Context&) {
      if (active != currentFilter) throw std::logic_error("Protection inventory used the wrong Current filter state");
      ++inventories;
      if (fail) throw std::runtime_error("Protection inventory unavailable");
      windows::VolumeProtectionInventory inventory;
      for (const auto& volume : physical->volumes) {
        const auto id = *compat::parseVolumeId(volume.volumeName);
        inventory.admissions.emplace(id, compat::VolumeAdmission{true, eligible});
        if (eligible) inventory.scope.push_back(id);
      }
      return inventory;
    };
    const auto system = composition::uwfWithStateRefresh(raw, environment);
    const auto source = app::uwfApplicationStateSource(system, physical, true);
    const auto refresh = source->readOperation(app::UwfCapability::Available);
    QCOMPARE(refresh({}).volumes.size(), std::size_t{1});
    QCOMPARE(system->state().runtime.protectScope.value->size(), std::size_t{1});
    auto configuration = raw->snapshot();
    configuration.current.volumes.emplace(
        added, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {added, "D:", false, false, false}, .fileExclusions = {}});
    configuration.next = configuration.current;
    raw->seed(configuration);
    physical->volumes.push_back({.driveLetter = "D:", .volumeName = added, .fileSystem = "NTFS", .label = {}, .physical = {true, true}});
    QCOMPARE(refresh({}).volumes.size(), std::size_t{2});
    QCOMPARE(inventories, 2);
    QCOMPARE(physical->reads, 2);
    const auto updated = system->state();
    QCOMPARE(updated.runtime.protectScope.value->size(), std::size_t{2});
    QVERIFY(updated.targets.at(added).volumeEligibility.value->protectionEligible);
    const auto operations = app::composeAllVolumeProtection(system, {});
    const auto applied = operations->execute(compat::ProtectAll{});
    QVERIFY2(!applied.failure, applied.failure ? applied.failure->message.c_str() : "");
    QVERIFY(applied.completionConfirmed);
    QCOMPARE(applied.items.size(), std::size_t{2});
    QVERIFY(raw->snapshot().next.volumes.at(added).volume.protectedVolume);
    QCOMPARE(inventories, 2);  // The operation consumes state without rediscovery.
    eligible = false;
    (void)refresh({});
    QVERIFY(system->state().runtime.protectScope.value->empty());
    QVERIFY(!system->state().targets.at(added).volumeEligibility.value->protectionEligible);
    fail = true;
    QVERIFY(refresh({}).snapshot.uwfAvailable);
    QVERIFY(!system->state().refreshFailure);
    QVERIFY(!system->state().runtime.protectScope.isKnown());
    QVERIFY(!system->state().targets.at(added).volumeEligibility.isKnown());
    QVERIFY(operations->execute(compat::ProtectAll{}).failure);
    fail = false;
    eligible = true;
    physical->volumes.erase(physical->volumes.begin() + 1);
    (void)refresh({});
    QCOMPARE(system->state().runtime.protectScope.value->size(), std::size_t{1});
    QVERIFY(!system->state().targets.at(added).volumeEligibility.isKnown());
    QCOMPARE(system->state().next.volumes.at(added).physicalDriveLetter.value, std::optional<std::string>{""});
    QVERIFY(!system->state().refreshFailure);
  }
  void explicitRefreshInvalidatesFailedCapabilityFactsAndPreservesTheCoreBarrier() {
    MemoryWmiOperations transport;
    transport.currentEnabled = true;
    auto environment = transport.environment();
    int romReads = 0, servicingReads = 0, hibernationReads = 0;
    int failureMode = 0;
    environment.hiberFileExists = [&] {
      ++hibernationReads;
      if (failureMode) throw std::runtime_error("Hibernation observation failed");
      return true;
    };
    environment.romDisposition = [&](const compat::Context&) {
      ++romReads;
      return failureMode ? compat::Response::fail(failureMode == 2 ? compat::ErrorKind::NotFound : compat::ErrorKind::AccessDenied, "ROM observation failed")
                         : compat::Response::success(std::uint32_t{3});
    };
    environment.hormVolumeProtection = [&](const compat::Context&) {
      return failureMode ? compat::Response::fail(compat::ErrorKind::Unavailable, "HORM observation failed") : compat::Response::success(true);
    };
    environment.capturedSettingsAvailable = [&](const compat::Context&) {
      return failureMode ? compat::Response::fail(compat::ErrorKind::Unavailable, "Defaults observation failed") : compat::Response::success(true);
    };
    environment.shutdownPrivilegeAvailable = [&](const compat::Context&) {
      return failureMode ? compat::Response::fail(compat::ErrorKind::Unavailable, "Privilege observation failed") : compat::Response::success(false);
    };
    environment.servicingAdmission = [&](const bool enabled, const compat::Context&) -> compat::ServicingAdmissionResult {
      ++servicingReads;
      if (enabled) throw std::logic_error("A complete refresh must include saved servicing state");
      if (failureMode) return compat::Failure{compat::ErrorKind::Unavailable, compat::Effect::None, "Servicing observation failed", {}};
      return compat::ServicingAdmissionFacts{compat::servicingStatus::started, true};
    };
    const auto system = composition::wmiUwf(transport, environment);
    (void)system->refreshState();
    QCOMPARE(romReads, 1);
    QCOMPARE(servicingReads, 1);
    QCOMPARE(hibernationReads, 1);
    auto facts = system->state();
    QCOMPARE(facts.runtime.romDisposition.valueBits, std::uint32_t{3});
    QCOMPARE(facts.runtime.controlDevicePresent.value, std::optional<bool>{true});
    QCOMPARE(facts.runtime.hiberFile.value, std::optional<bool>{true});
    QCOMPARE(facts.runtime.hormVolumesProtected.value, std::optional<bool>{true});
    QCOMPARE(facts.capturedSettingsAvailable.value, std::optional<bool>{true});
    QCOMPARE(facts.runtime.shutdownPrivilegeAvailable.value, std::optional<bool>{false});
    QCOMPARE(facts.runtime.servicingStatus.value, std::optional<std::uint32_t>{compat::servicingStatus::started});
    QVERIFY(facts.runtime.savedServicingFilter.value.has_value());
    QCOMPARE(*facts.runtime.savedServicingFilter.value, std::optional<bool>{true});
    failureMode = 1;
    (void)system->refreshState();
    facts = system->state();
    QCOMPARE(facts.runtime.controlDevicePresent.value, std::optional<bool>{true});
    QCOMPARE(facts.runtime.romDisposition.valueBits, std::uint32_t{3});
    QVERIFY(!facts.runtime.hiberFile.isKnown());
    QVERIFY(!facts.runtime.hormVolumesProtected.isKnown());
    QVERIFY(!facts.capturedSettingsAvailable.isKnown());
    QVERIFY(!facts.runtime.shutdownPrivilegeAvailable.isKnown());
    QVERIFY(!facts.runtime.servicingStatus.isKnown());
    QVERIFY(!facts.runtime.savedServicingFilter.isKnown());
    QVERIFY(facts.refreshFailure);  // ROM disposition remains a core prerequisite.
    failureMode = 2;
    (void)system->refreshState();
    QCOMPARE(system->state().runtime.controlDevicePresent.value, std::optional<bool>{false});
    QCOMPARE(romReads, 3);
    QCOMPARE(servicingReads, 3);
    QCOMPARE(hibernationReads, 3);
    QVERIFY(transport.invocations.empty());
  }
  void capabilityObservationFailuresDoNotBlockIndependentWrites_data() {
    QTest::addColumn<int>("observation");
    const std::array names{"hibernation-file", "horm-volumes", "captured-defaults", "shutdown-privilege", "servicing", "protection-inventory"};
    for (std::size_t index = 0; index < names.size(); ++index) QTest::newRow(names[index]) << static_cast<int>(index);
  }
  void capabilityObservationFailuresDoNotBlockIndependentWrites() {
    QFETCH(int, observation);
    using namespace compat;
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    initial.current.volumes.emplace(
        memoryVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {memoryVolume, "C:", true, false, false}, .fileExclusions = {}});
    initial.next = initial.current;
    const auto raw = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    raw->seedFile(memoryVolume, "\\Item", "before");
    raw->writeFile(memoryVolume, "\\Item", "after");
    const auto physical = std::make_shared<MutablePhysicalVolumes>();
    physical->volumes = {{.driveLetter = "C:", .volumeName = memoryVolume, .fileSystem = "NTFS", .label = {}, .physical = {true, true}}};
    bool fail = false;
    const Failure denied{ErrorKind::AccessDenied, Effect::None, "Capability observation denied", {}};
    const auto boolean = [&](const int field) { return fail && observation == field ? Response::fail(denied.kind, denied.message) : Response::success(true); };
    windows::WindowsConfigurationEnvironment environment;
    environment.isAdministrator = [] { return true; };
    environment.systemVolumeId = [] { return memoryVolume; };
    environment.romDisposition = [](const Context&) { return Response::success(std::uint32_t{0}); };
    environment.hiberFileExists = [&] {
      if (fail && observation == 0) throw Error(denied);
      return true;
    };
    environment.hormVolumeProtection = [&](const Context&) { return boolean(1); };
    environment.capturedSettingsAvailable = [&](const Context&) { return boolean(2); };
    environment.shutdownPrivilegeAvailable = [&](const Context&) { return boolean(3); };
    environment.servicingAdmission = [&](bool, const Context&) -> ServicingAdmissionResult {
      if (fail && observation == 4) return denied;
      return ServicingAdmissionFacts{servicingStatus::notStarted, {}};
    };
    environment.volumeProtectionInventory = [&](bool, const Context&) {
      if (fail && observation == 5) throw Error(denied);
      return windows::VolumeProtectionInventory{{memoryVolume}, {{memoryVolume, {true, true}}}};
    };
    const auto system = composition::uwfWithStateRefresh(raw, environment);
    const auto source = app::uwfApplicationStateSource(system, physical, true);
    const auto refresh = source->readOperation(app::UwfCapability::Available);
    QVERIFY(refresh({}).snapshot.uwfAvailable);
    fail = true;
    const auto receipt = system->refreshState();
    QVERIFY(receipt.succeeded());
    QCOMPARE(receipt.warnings.size(), std::size_t{1});
    const auto facts = system->state();
    QVERIFY(!facts.refreshFailure);
    QCOMPARE(facts.runtime.hiberFile.isKnown(), observation != 0);
    QCOMPARE(facts.runtime.hormVolumesProtected.isKnown(), observation != 1);
    QCOMPARE(facts.capturedSettingsAvailable.isKnown(), observation != 2);
    QCOMPARE(facts.runtime.shutdownPrivilegeAvailable.isKnown(), observation != 3);
    QCOMPARE(facts.runtime.servicingStatus.isKnown(), observation != 4);
    QCOMPARE(facts.runtime.savedServicingFilter.isKnown(), observation != 4);
    QCOMPARE(facts.runtime.protectScope.isKnown(), observation != 5);
    QCOMPARE(facts.targets.at(memoryVolume).volumeEligibility.isKnown(), observation != 5);
    raw->clearPrimitiveRequests();
    QVERIFY(system->execute(SetNumber{NumberProperty::WarningThreshold, 610}).succeeded());
    QVERIFY(system->execute(CommitFile{memoryVolume, "\\Item"}).succeeded());
    QCOMPARE(raw->primitiveRequests(), (std::vector<Request>{SetNumber{NumberProperty::WarningThreshold, 610}, CommitFile{memoryVolume, "\\Item"}}));
    const auto updated = refresh({});
    QVERIFY(updated.snapshot.uwfAvailable);
    QCOMPARE(updated.snapshot.current.overlay.warningThresholdMb, std::uint32_t{610});
    QVERIFY(updated.overlayPolicy.transportAvailable);
    QVERIFY(source->publishedOperation(app::UwfCapability::Available)({}).snapshot.uwfAvailable);
    if (observation == 5) {
      const auto allVolumes = app::composeAllVolumeProtection(system, {});
      raw->clearPrimitiveRequests();
      QVERIFY(allVolumes->execute(ProtectAll{}).failure);
      QVERIFY(raw->primitiveRequests().empty());
      QVERIFY(allVolumes->execute(UnprotectAll{}).succeeded());
    }
    fail = false;
    const auto recovered = system->refreshState();
    QVERIFY(recovered.succeeded());
    QVERIFY(recovered.warnings.empty());
    const auto restored = system->state();
    QVERIFY(restored.runtime.hiberFile.isKnown());
    QVERIFY(restored.runtime.hormVolumesProtected.isKnown());
    QVERIFY(restored.capturedSettingsAvailable.isKnown());
    QVERIFY(restored.runtime.shutdownPrivilegeAvailable.isKnown());
    QVERIFY(restored.runtime.servicingStatus.isKnown());
    QVERIFY(restored.runtime.savedServicingFilter.isKnown());
    QVERIFY(restored.runtime.protectScope.isKnown());
    QVERIFY(restored.targets.at(memoryVolume).volumeEligibility.isKnown());
  }
  void maintenanceObservationRefreshesRecoveredHormPrerequisites_data() {
    QTest::addColumn<int>("observation");
    QTest::addColumn<bool>("readFails");
    QTest::newRow("hibernation-file-unreadable") << 0 << true;
    QTest::newRow("hibernation-file-absent") << 0 << false;
    QTest::newRow("volume-protection-unreadable") << 1 << true;
    QTest::newRow("volume-protection-unmet") << 1 << false;
  }
  void maintenanceObservationRefreshesRecoveredHormPrerequisites() {
    QFETCH(int, observation);
    QFETCH(bool, readFails);
    using namespace compat;
    uwf::simulation::FakeUwfBackend::State initial;
    initial.current.filterEnabled = true;
    initial.current.overlayFlags = flags::readOnlyMedia;
    initial.current.volumes.emplace(
        memoryVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{.volume = {memoryVolume, "C:", true, false, false}, .fileExclusions = {}});
    initial.next = initial.current;
    const auto raw = std::make_shared<uwf::simulation::FakeUwfBackend>(initial);
    const auto physical = std::make_shared<MutablePhysicalVolumes>();
    physical->volumes = {{.driveLetter = "C:", .volumeName = memoryVolume, .fileSystem = "NTFS", .label = {}, .physical = {true, true}}};
    bool recovered = false;
    int hibernationReads = 0, protectionReads = 0, capabilityReads = 0, detailReads = 0;
    windows::WindowsConfigurationEnvironment environment;
    environment.isAdministrator = [] { return true; };
    environment.systemVolumeId = [] { return memoryVolume; };
    environment.romDisposition = [](const Context&) { return Response::success(std::uint32_t{0}); };
    environment.hiberFileExists = [&] {
      ++hibernationReads;
      if (!recovered && observation == 0 && readFails) throw Error({ErrorKind::AccessDenied, Effect::None, "Hibernation fact unavailable", {}});
      return recovered || observation != 0;
    };
    environment.hormVolumeProtection = [&](const Context&) {
      ++protectionReads;
      if (!recovered && observation == 1 && readFails) return Response::fail(ErrorKind::AccessDenied, "Protection fact unavailable");
      return Response::success(recovered || observation != 1);
    };
    const auto system = composition::uwfWithStateRefresh(raw, environment);
    const auto initialRefresh = system->refreshState();
    QVERIFY(initialRefresh.succeeded());
    QVERIFY(!system->state().refreshFailure);
    app::OverlayMaintenanceReadOnlyPorts details{.hormVolumeProtection =
                                                     [&](std::stop_token) {
                                                       ++detailReads;
                                                       return app::OverlayHormVolumeProtection{recovered || observation != 1, {}, {}};
                                                     },
                                                 .hibernationCapabilities =
                                                     [&](std::stop_token) {
                                                       ++capabilityReads;
                                                       return app::OverlayHibernationCapabilities{true, recovered || observation != 0,
                                                                                                  app::HibernationFileType::Full};
                                                     }};
    const auto maintenance = app::uwfOverlayMaintenance(system, physical, std::move(details));
    const auto before = maintenance->observe();
    const auto prerequisite = observation == 0 ? before.hibernationAvailable : before.allVolumesProtected;
    QCOMPARE(prerequisite, readFails ? std::nullopt : std::optional{false});
    const auto eligibility = app::evaluateHormEligibility(before, true);
    QVERIFY(!eligibility.allowed());
    QCOMPARE(eligibility.issue, readFails          ? app::HormIssue::PrerequisitesUnknown
                                : observation == 0 ? app::HormIssue::HibernationUnavailable
                                                   : app::HormIssue::VolumesUnprotected);
    const auto hibernationBefore = hibernationReads, protectionBefore = protectionReads;
    QCOMPARE(physical->reads, 1);
    QCOMPARE(system->state().current.volumes.at(memoryVolume).physicalDriveLetter.value, std::optional<std::string>{"C:"});
    raw->clearTrace();
    recovered = true;
    physical->volumes.front().driveLetter = "D:";
    const auto cached = maintenance->snapshot();
    QVERIFY(cached);
    QCOMPARE(observation == 0 ? cached->hibernationAvailable : cached->allVolumesProtected, prerequisite);
    QCOMPARE(hibernationReads, hibernationBefore);
    QCOMPARE(protectionReads, protectionBefore);
    QCOMPARE(physical->reads, 1);
    QCOMPARE(cached->volumes.front().driveLetter, QStringLiteral("C:"));
    QVERIFY(raw->trace().empty());
    // Reopening the HORM dialog calls this adapter, without a main-window refresh.
    const auto observed = maintenance->observe();
    QCOMPARE(hibernationReads, hibernationBefore + 1);
    QCOMPARE(protectionReads, protectionBefore + 1);
    QCOMPARE(capabilityReads, 2);
    QCOMPARE(detailReads, 2);
    QCOMPARE(physical->reads, 2);
    QCOMPARE(system->state().current.volumes.at(memoryVolume).physicalDriveLetter.value, std::optional<std::string>{"D:"});
    QCOMPARE(system->state().next.volumes.at(memoryVolume).physicalDriveLetter.value, std::optional<std::string>{"D:"});
    QCOMPARE(system->state().current.volumes.at(memoryVolume).driveLetter.value, std::optional<std::string>{"C:"});
    QCOMPARE(observed.volumes.front().driveLetter, QStringLiteral("D:"));
    QCOMPARE(system->state().runtime.hiberFile.value, std::optional{true});
    QCOMPARE(system->state().runtime.hormVolumesProtected.value, std::optional{true});
    QCOMPARE(observed.hibernationAvailable, std::optional{true});
    QCOMPARE(observed.allVolumesProtected, std::optional{true});
    QVERIFY(app::evaluateHormEligibility(observed, true).allowed());
    QVERIFY(!system->state().refreshFailure);
    QVERIFY(std::ranges::none_of(raw->trace(), [](const auto& call) { return isWrite(call.request); }));
    QVERIFY(!raw->snapshot().runtime.hormEnabled);
    raw->clearTrace();
    QVERIFY(app::evaluateHormEligibility(*maintenance->snapshot(), true).allowed());
    QCOMPARE(hibernationReads, hibernationBefore + 1);
    QCOMPARE(protectionReads, protectionBefore + 1);
    QCOMPARE(capabilityReads, 2);
    QCOMPARE(detailReads, 2);
    QCOMPARE(physical->reads, 2);
    QVERIFY(raw->trace().empty());
  }
  void maintenanceObservationKeepsTheCoreRefreshFailureBarrier() {
    using namespace compat;
    const auto raw = std::make_shared<uwf::simulation::FakeUwfBackend>();
    int capabilityReads = 0;
    windows::WindowsConfigurationEnvironment environment;
    environment.isAdministrator = [] { return true; };
    environment.hiberFileExists = [] { return true; };
    const auto system = composition::uwfWithStateRefresh(raw, environment);
    const auto maintenance = app::uwfOverlayMaintenance(system, {}, {.hibernationCapabilities = [&](std::stop_token) {
                                                          ++capabilityReads;
                                                          return app::OverlayHibernationCapabilities{true, true, app::HibernationFileType::Full};
                                                        }});
    QVERIFY(maintenance->observe().readError.isEmpty());
    QCOMPARE(capabilityReads, 1);
    raw->inject({.request = GetNumber{NumberProperty::CriticalThreshold},
                 .operation = {},
                 .kind = ErrorKind::AccessDenied,
                 .message = "Critical threshold read denied"});
    const auto failed = maintenance->observe();
    QVERIFY(system->state().refreshFailure);
    QVERIFY(failed.readError.contains(QStringLiteral("Critical threshold read denied")));
    QCOMPARE(capabilityReads, 1);
    raw->clearTrace();
    const auto blocked = system->execute(SetNumber{NumberProperty::WarningThreshold, 610});
    QVERIFY(blocked.failure);
    QCOMPARE(blocked.effect, Effect::None);
    QVERIFY(raw->trace().empty());
    QVERIFY(maintenance->snapshot()->readError.contains(QStringLiteral("Critical threshold read denied")));
    QVERIFY(raw->trace().empty());
    QVERIFY(maintenance->observe().readError.isEmpty());
    QVERIFY(!system->state().refreshFailure);
    QCOMPARE(capabilityReads, 2);
    QVERIFY(std::ranges::none_of(raw->trace(), [](const auto& call) { return isWrite(call.request); }));
  }
  void disabledSessionDoesNotQueryInapplicableControlState() {
    MemoryWmiOperations transport;
    auto environment = transport.environment();
    int romReads = 0, hormReads = 0;
    environment.romDisposition = [&](const compat::Context&) {
      ++romReads;
      return compat::Response::fail(compat::ErrorKind::NotFound, "No UwfvolControl");
    };
    environment.hormVolumeProtection = [&](const compat::Context&) {
      ++hormReads;
      return compat::Response::fail(compat::ErrorKind::Unsupported, "No active filters");
    };
    const auto system = composition::wmiUwf(transport, environment);
    const auto response = system->refreshState();
    QCOMPARE(romReads, 0);
    QCOMPARE(hormReads, 0);
    QCOMPARE(system->state().runtime.controlDevicePresent.value, std::optional<bool>{false});
    QCOMPARE(system->state().runtime.romDisposition.knownMask, std::uint32_t{0});
    // The injected WMI-only transport lacks other getters. None of its read
    // failures may be the normal absence of an inactive runtime control.
    if (response.failure) {
      QVERIFY(response.failure->message.find("No UwfvolControl") == std::string::npos);
      QVERIFY(response.failure->message.find("No active filters") == std::string::npos);
    }
  }
  void explicitRefreshLoadsDriverAdmissionForPhysicalTargets() {
    MemoryWmiOperations transport;
    transport.currentEnabled = true;
    auto environment = transport.environment();
    int admissions = 0;
    environment.volumeProtectionInventory = [&](const bool active, const compat::Context&) {
      ++admissions;
      if (!active) throw std::logic_error("Incorrect Current filter state");
      return windows::VolumeProtectionInventory{{}, {{memoryVolume, {true, false}}}};
    };
    const auto system = composition::wmiUwf(transport, environment);
    auto facts = system->state();
    facts.targets[memoryVolume] = {};
    system->updateState(std::move(facts));
    (void)system->refreshState();
    QCOMPARE(admissions, 1);
    const auto eligibility = system->state().targets.at(memoryVolume).volumeEligibility;
    QVERIFY(eligibility.isKnown());
    QVERIFY(eligibility.value->fixedMedia);
    QVERIFY(!eligibility.value->protectionEligible);
  }
  void productionCompositionUsesPublishedTargetFactsWithoutRefreshing_data() {
    QTest::addColumn<bool>("observable");
    QTest::newRow("known-missing") << true;
    QTest::newRow("unavailable-continues") << false;
  }
  void productionCompositionUsesPublishedTargetFactsWithoutRefreshing() {
    QFETCH(bool, observable);
    MemoryWmiOperations transport;
    transport.volumeConfigured = true;
    transport.commitSource->present = false;
    auto environment = transport.environment();
    const auto system = composition::wmiUwf(transport, environment);
    auto facts = memoryState(transport);
    const compat::Request request = compat::CommitFile{memoryVolume, "\\missing.txt"};
    if (observable) {
      facts.requestTarget = request;
      facts.targets[memoryVolume].fileTarget = compat::Fact<compat::FileMaintenanceFacts>::known({14, compat::FileMaintenanceKind::Missing, false});
    }
    system->updateState(std::move(facts));
    const auto response = system->execute(request);
#if defined(UWF_ENABLE_FAILURE_SOLVER)
    const bool rejected = observable;
#else
    const bool rejected = false;
#endif
    QCOMPARE(response.failure.has_value(), rejected);
    if (rejected) {
      QCOMPARE(response.failure->kind, compat::ErrorKind::NotFound);
      QCOMPARE(response.effect, compat::Effect::None);
      QVERIFY(!response.failure->diagnostics.empty());
      QCOMPARE(response.failure->diagnostics.front().backend, std::string{"UwfFailureSolver"});
    } else {
      QCOMPARE(response.effect, compat::Effect::Confirmed);
      QCOMPARE(transport.invocations.size(), std::size_t{1});
      QCOMPARE(transport.invocations.front().method, std::string{"CommitFile"});
    }
    QCOMPARE(transport.queries.size(), rejected ? std::size_t{0} : std::size_t{1});
    QVERIFY(transport.nativeInvocations.empty());
  }
  void deletionUsesInjectedStateBeforeAndAfterWrites_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("newContent");
    for (const bool registry : {false, true})
      for (const bool changed : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("registry-%1-new-content-%2").arg(registry).arg(changed))) << registry << changed;
  }
  void deletionUsesInjectedStateBeforeAndAfterWrites() {
    QFETCH(bool, registry);
    QFETCH(bool, newContent);
    MemoryWmiOperations wmi;
    wmi.currentEnabled = wmi.nextEnabled = true;
    wmi.volumeConfigured = true;  // The injected C: system volume protects registry changes as well as files.
    auto source = wmi.commitSource;
    source->kind = registry ? app::CommitTargetKind::RegistryKey : app::CommitTargetKind::Folder;
    wmi.invocationObserver = [source] { source->present = false; };
    bool called = false;
    app::CommitInteraction interaction;
    interaction.run = [&](const app::CommitRequest& request, const std::shared_ptr<app::CommitBackend> backend) {
      called = true;
      const auto plan = backend->prepare(request, {}, {}, {});
      QCOMPARE(plan.progress().failed, 0);
      const auto targets = plan.targets();
      QCOMPARE(targets.size(), 1);
      source->unexpectedChild = newContent;
      const auto result = backend->execute(targets.front(), request.operation);
      QCOMPARE(result.succeeded, !newContent);
      QCOMPARE(wmi.invocations.size(), newContent ? std::size_t{0} : std::size_t{1});
      QVERIFY(wmi.nativeInvocations.empty());
      QCOMPARE(source->verifications, 2);
      if (newContent)
        QVERIFY(!result.failureDetail.isEmpty());
      else {
        QCOMPARE(wmi.invocations.front().path, registry ? std::string{"registry"} : std::string{"volume"});
        QCOMPARE(wmi.invocations.front().method, registry ? std::string{"CommitRegistryDeletion"} : std::string{"CommitFileDeletion"});
      }
    };
    app::CommitWorkflow workflow(maintenanceRepository(wmi), std::move(interaction), source);
    if (registry)
      workflow.commitRegistryDeletionKey(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\VirtualCommitFixture"), std::nullopt);
    else
      workflow.commitFileDeletionPath(QStringLiteral("C:\\VirtualCommitFixture"));
    QVERIFY(called);
  }
  void stagingScanAndCommitUseTheSameInjectedSource() {
    MemoryWmiOperations wmi;
    wmi.currentEnabled = wmi.nextEnabled = true;
    wmi.volumeConfigured = true;
    wmi.wmiMaintenanceAvailable = false;
    auto source = wmi.commitSource;
    app::FileStagingCommitter committer(maintenanceRepository(wmi), source);
    const QString path = QStringLiteral("C:\\VirtualCommitFixture\\file.bin");
    auto operation = committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan({{app::FileStagingKind::File, path}}))));
    QCOMPARE(operation.totalFiles(), std::size_t{1});
    QCOMPARE(operation.advance(), path);
    QCOMPARE(source->inspections, 2);
    QCOMPARE(operation.result().committedFiles, std::size_t{1});
    QCOMPARE(wmi.nativeInvocations.size(), std::size_t{1});
    QVERIFY(wmi.invocations.empty());
    QCOMPARE(wmi.nativeInvocations.front().method, std::string{"CommitFile"});
  }
  void typedRegistryDeletionKeepsDefaultValueDistinctFromTheKey_data() {
    QTest::addColumn<bool>("wholeKey");
    QTest::newRow("default-value") << false;
    QTest::newRow("whole-key") << true;
  }
  void typedRegistryDeletionKeepsDefaultValueDistinctFromTheKey() {
    QFETCH(bool, wholeKey);
    MemoryWmiOperations wmi;
    wmi.currentEnabled = wmi.nextEnabled = true;
    wmi.volumeConfigured = true;
    auto source = wmi.commitSource;
    source->kind = wholeKey ? app::CommitTargetKind::RegistryKey : app::CommitTargetKind::RegistryValue;
    wmi.invocationObserver = [source] { source->present = false; };
    bool executed = false;
    app::CommitInteraction interaction;
    interaction.run = [&](const app::CommitRequest& selected, const std::shared_ptr<app::CommitBackend> backend) {
      auto request = selected;
      request.target.kind = source->kind;
      request.target.valueName.clear();
      if (!wholeKey) {
        try {
          (void)backend->prepare(request, {}, {}, {});
          QFAIL("Default-value deletion must fail before preparing a whole-key target");
        } catch (const app::OperationError& error) {
          QCOMPARE(error.failureKind(), app::OperationFailureKind::Unsupported);
          QCOMPARE(error.writeEffect(), app::WriteEffect::None);
        }
        executed = true;
        return;
      }
      const auto prepared = backend->prepare(request, {}, {}, {});
      QCOMPARE(prepared.targets().size(), 1);
      const auto result = backend->execute(prepared.targets().constFirst(), request.operation);
      QVERIFY(result.succeeded);
      executed = true;
    };
    app::CommitWorkflow workflow(maintenanceRepository(wmi), std::move(interaction), source);
    workflow.commitRegistryDeletionKey(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\TypedFixture"), std::optional<QString>{QStringLiteral("SelectedValue")});
    QVERIFY(executed);
    QCOMPARE(wmi.invocations.size(), wholeKey ? std::size_t{1} : std::size_t{0});
    QVERIFY(wmi.nativeInvocations.empty());
    if (wholeKey) {
      QCOMPARE(wmi.invocations.front().method, std::string{"CommitRegistryDeletion"});
      QVERIFY(!wmi.invocations.front().inputs.contains("ValueName"));
    }
  }
  void nativeAdaptersUseAnInjectedTransportWithoutComCalls() {
    MemoryWmiOperations transport;
    const auto system = composition::wmiUwf(transport, transport.environment());
    QVERIFY(!compat::call(*system, compat::GetBoolean{compat::BooleanProperty::FilterEnabled, compat::Session::Next}));
    const auto written = system->execute(compat::SetBoolean{compat::BooleanProperty::FilterEnabled, true});
    // This intentionally incomplete WMI-only transport acknowledges the write,
    // then cannot complete the required global refresh. Keep both facts.
    QVERIFY(written.failure.has_value());
    QCOMPARE(written.effect, compat::Effect::Confirmed);
    QVERIFY(system->state().refreshFailure.has_value());
    QCOMPARE(transport.writes, 1);
    QVERIFY(compat::call(*system, compat::GetBoolean{compat::BooleanProperty::FilterEnabled, compat::Session::Next}));
    const auto inventory = system->execute(compat::GetVolumes{compat::Session::Current});
    QVERIFY(inventory.failure);
    QCOMPARE(inventory.failure->kind, compat::ErrorKind::Unsupported);
    QCOMPARE(inventory.effect, compat::Effect::None);
    QCOMPARE(uwf::app::probeUwfCapability(*system), uwf::app::UwfCapability::Available);
    QVERIFY(uwf::windows::windowsVolumeRepository(transport)->read().empty());
    const auto unavailable = uwf::app::publishedUwfSnapshot(system->state(), uwf::app::UwfCapability::Unavailable, true, {});
    QVERIFY(!unavailable.uwfAvailable);
    QVERIFY(unavailable.elevated);
  }
  void singleWmiUsesAnExistingSharedOverlayObservation_data() {
    QTest::addColumn<CommitEntry>("entry");
    const std::array entries{std::pair{"prepare-file", CommitEntry::PrepareFile},      std::pair{"prepare-registry", CommitEntry::PrepareRegistry},
                             std::pair{"registry-write", CommitEntry::RegistryWrite},  std::pair{"commit-file", CommitEntry::CommitFile},
                             std::pair{"delete-file", CommitEntry::DeleteFile},        std::pair{"commit-registry", CommitEntry::CommitRegistry},
                             std::pair{"delete-registry", CommitEntry::DeleteRegistry}};
    for (const auto& [name, entry] : entries) QTest::newRow(name) << entry;
  }
  void singleWmiUsesAnExistingSharedOverlayObservation() {
    QFETCH(CommitEntry, entry);
    MemoryWmiOperations transport;
    transport.currentEnabled = true;
    transport.volumeConfigured = true;
    const auto system = composition::wmiUwf(transport, transport.environment());
    QCOMPARE(system->probe(compat::GetNumber{compat::NumberProperty::OverlayFlags, compat::Session::Current}).support, compat::Support::Unsupported);
    system->updateState(memoryState(transport));  // A previous explicit refresh supplied flags from another capability.
    const auto repository = app::uwfMaintenanceRepository(system);
    const auto registry = repository->prepareRegistryMaintenance();
    bool writing = false;
    switch (entry) {
      case CommitEntry::PrepareFile:
        QCOMPARE(repository->fileCommitRequirement(memoryVolume), app::CommitRequirement::RequiresUwfCommit);
        break;
      case CommitEntry::PrepareRegistry:
        QCOMPARE(repository->registryCommitRequirement(), app::CommitRequirement::RequiresUwfCommit);
        break;
      case CommitEntry::RegistryWrite:
        QCOMPARE(repository->registryWriteRequirement(), app::CommitRequirement::RequiresUwfCommit);
        break;
      case CommitEntry::CommitFile:
        repository->prepareVolumeTargets().front()->commitFile("C:\\Fixture.txt");
        writing = true;
        break;
      case CommitEntry::DeleteFile:
        repository->prepareVolumeTargets().front()->commitFileDeletion("C:\\Fixture.txt");
        writing = true;
        break;
      case CommitEntry::CommitRegistry:
        registry->commit("HKEY_LOCAL_MACHINE\\SOFTWARE\\Fixture", "Setting");
        writing = true;
        break;
      case CommitEntry::DeleteRegistry:
        registry->commitDeletion("HKEY_LOCAL_MACHINE\\SOFTWARE\\Fixture", std::string("Setting"));
        writing = true;
        break;
    }
    QCOMPARE(transport.invocations.size(), writing ? std::size_t{1} : std::size_t{0});
    QCOMPARE(transport.queries.size(), writing ? std::size_t{1} : std::size_t{0});
    for (const auto& query : transport.queries) {
      QVERIFY(query != "SELECT * FROM UWF_Filter");
      QVERIFY(query != "SELECT * FROM UWF_OverlayConfig");
    }
    QVERIFY(transport.nativeInvocations.empty());
  }
  void exactLookupWorksThroughTheMtaCompositionAfterAnOmission_data() {
    QTest::addColumn<bool>("present");
    QTest::newRow("configured-but-omitted") << true;
    QTest::newRow("exact-not-found") << false;
  }
  void exactLookupWorksThroughTheMtaCompositionAfterAnOmission() {
    QFETCH(bool, present);
    MemoryWmiOperations transport;
    transport.volumeConfigured = present;
    transport.omitVolumeFromEnumeration = true;
    const auto system = composition::wmiUwf(transport, transport.environment());
    const auto response = system->execute(compat::GetVolumeProperty{memoryVolume, compat::VolumeProperty::Protected, compat::Session::Current});
    QCOMPARE(response.succeeded(), present);
    if (present)
      QVERIFY(std::get<bool>(response.value));
    else {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, compat::ErrorKind::NotFound);
    }
    QCOMPARE(response.effect, compat::Effect::None);
    QVERIFY(transport.invocations.empty());
    QVERIFY(transport.nativeInvocations.empty());
  }
};

}  // namespace

QTEST_APPLESS_MAIN(NativeUwfCompositionTests)
#include "NativeUwfCompositionTests.moc"
