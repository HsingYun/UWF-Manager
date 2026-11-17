// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTest>
#include <array>
#include <memory>
#include <set>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "infrastructure/uwf/windows/csp/CspUwfBackend.h"
#include "infrastructure/uwf/windows/native/NativeUwfBackend.h"
#include "infrastructure/uwf/windows/wmi/WmiUwfBackend.h"
#include "simulation/uwf/FakeUwfBackend.h"
#include "support/uwf/UwfPublicInterfaceCases.h"

Q_DECLARE_METATYPE(uwf::compat::Request)

namespace {
using namespace uwf::compat;
using namespace uwf::windows;
using namespace uwf::windows::backends;
using uwf::simulation::FakeUwfBackend;
using uwf::test::uwfPublicInterfaceCases;

enum class BackendKind { Native, Wmi, Csp, Fake };
constexpr std::array backendNames{"Native", "WMI", "CSP", "Fake"};

QString text(const std::string_view value) { return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size())); }

struct IoCounter {
  int calls = 0;
  template <class T>
  [[noreturn]] T unexpected() {
    ++calls;
    throw std::logic_error("Public pre-dispatch rejection reached a provider/host IO port");
  }
};

class GuardedWmi final : public wmi::WmiOperations {
 public:
  explicit GuardedWmi(IoCounter& counter) : m_counter(counter) {}
  void ensureConnected() const override { m_counter.unexpected<void>(); }
  std::vector<wmi::WmiRow> query(const std::string&) const override { return m_counter.unexpected<std::vector<wmi::WmiRow>>(); }
  std::vector<wmi::WmiRow> queryInstances(const std::string&) const override { return m_counter.unexpected<std::vector<wmi::WmiRow>>(); }
  wmi::WmiClassStatus classStatus(const std::string&) const override { return m_counter.unexpected<wmi::WmiClassStatus>(); }
  wmi::WmiRow getObject(const std::string&) const override { return m_counter.unexpected<wmi::WmiRow>(); }
  wmi::WmiMethodOutput invokeMethod(const std::string&, const std::string&, const wmi::WmiRow&) const override {
    return m_counter.unexpected<wmi::WmiMethodOutput>();
  }
  wmi::WmiMethodOutput callMethodRead(const std::string&, const std::string&, const wmi::WmiRow&) const override {
    return m_counter.unexpected<wmi::WmiMethodOutput>();
  }
  wmi::WmiMethodOutput callMethodReadOnce(const std::string&, const std::string&, const wmi::WmiRow&, std::stop_token) const override {
    return m_counter.unexpected<wmi::WmiMethodOutput>();
  }
  void putInstance(const std::string&, const wmi::WmiRow&, wmi::WmiPutMode) const override { m_counter.unexpected<void>(); }

 private:
  IoCounter& m_counter;
};

class GuardedCsp final : public DmCspTransport {
 public:
  explicit GuardedCsp(IoCounter& counter) : m_counter(counter) {}
  bool available() override { return m_counter.unexpected<bool>(); }
  DmCspReply process(const QString&, const QStringList&) override { return m_counter.unexpected<DmCspReply>(); }

 private:
  IoCounter& m_counter;
};

uwf::simulation::FakeUwfBackend::State seededState() {
  uwf::simulation::FakeUwfBackend::State state;
  state.current.filterEnabled = true;
  state.current.overlayType = 1;
  state.current.overlayFlags = 6;
  state.next.maximumOverlaySize = 3072;
  state.next.persistDomainSecretKey = true;
  state.current.volumes.emplace(uwf::test::publicInterfaceVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{
                                                                      {uwf::test::publicInterfaceVolume, "C:", true, false, {}}, {"\\CurrentEntry"}, 1});
  state.next.volumes.emplace(uwf::test::publicInterfaceVolume, uwf::simulation::FakeUwfBackend::VolumeConfiguration{
                                                                   {uwf::test::publicInterfaceVolume, "D:", false, true, {}}, {"\\NextEntry"}, 2});
  state.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\Software\\CurrentEntry"};
  state.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\Software\\NextEntry"};
  state.runtime.hormEnabled = true;
  state.runtime.romDisposition = 3;
  state.runtime.systemVolumeId = uwf::test::publicInterfaceVolume;
  state.runtime.windowsDirectory = "C:\\Windows";
  state.runtime.currentControlSet = 2;
  state.runtime.usageOverride = OverlayUsage{17, 93};
  state.durable.files.emplace(uwf::simulation::FakeUwfBackend::FileAddress{uwf::test::publicInterfaceVolume, "\\Witness"}, "durable");
  state.overlay.files.emplace(uwf::simulation::FakeUwfBackend::FileAddress{uwf::test::publicInterfaceVolume, "\\Witness"}, "overlay");
  state.durable.registry.emplace(uwf::simulation::FakeUwfBackend::RegistryAddress{uwf::test::publicInterfaceRegistryKey, "Named"}, "durable-value");
  state.overlay.registry.emplace(uwf::simulation::FakeUwfBackend::RegistryAddress{uwf::test::publicInterfaceRegistryKey, "Named"}, "overlay-value");
  state.powerRequests = {PowerAction::Shutdown};
  state.windowsUpdates = 4;
  state.restarts = 2;
  return state;
}

struct Harness {
  IoCounter providerIo;
  int wmiResolutions = 0;
  GuardedWmi wmiPort{providerIo};
  std::shared_ptr<uwf::simulation::FakeUwfBackend> fake;
  std::shared_ptr<Backend> backend;
  uwf::simulation::FakeUwfBackend::State before;

  explicit Harness(const int kind) {
    switch (static_cast<BackendKind>(kind)) {
      case BackendKind::Native:
        // Explicitly empty injected bindings: never resolve/load a system DLL.
        backend = std::make_shared<NativeUwfBackend>(NativeUwfBindings{});
        break;
      case BackendKind::Wmi:
        backend = std::make_shared<WmiUwfBackend>([this]() -> wmi::WmiOperations& {
          ++wmiResolutions;
          return wmiPort;
        });
        break;
      case BackendKind::Csp:
        backend = std::make_shared<CspUwfBackend>(std::make_shared<GuardedCsp>(providerIo));
        break;
      case BackendKind::Fake:
        fake = std::make_shared<uwf::simulation::FakeUwfBackend>(seededState());
        before = fake->snapshot();
        backend = fake;
        break;
      default:
        throw std::logic_error("Unknown conformance backend");
    }
  }
};

void verifyNoIoOrFakeMutation(const Harness& harness) {
  QCOMPARE(harness.providerIo.calls, 0);
  QCOMPARE(harness.wmiResolutions, 0);
  if (harness.fake) {
    QVERIFY(harness.fake->primitiveRequests().empty());
    QVERIFY(harness.fake->stages().empty());
    QVERIFY(harness.fake->snapshot() == harness.before);
  }
}

void addBackendRows(const QString& name, const Request& request) {
  for (std::size_t backend = 0; backend < backendNames.size(); ++backend)
    QTest::newRow(qUtf8Printable(name + u'/' + QString::fromLatin1(backendNames[backend]))) << request << static_cast<int>(backend);
}
}  // namespace

class UwfBackendEquivalenceTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() {
    QJsonArray interfaces;
    for (const auto& item : uwfPublicInterfaceCases)
      interfaces.append(QJsonObject{{QStringLiteral("slot"), text(item.slot)},
                                    {QStringLiteral("session"), text(item.session)},
                                    {QStringLiteral("write"), item.write},
                                    {QStringLiteral("familyAlternative"), static_cast<int>(item.request.index())}});
    const QJsonObject manifest{
        {QStringLiteral("schema"), QStringLiteral("uwf-public-interface-witnesses/1")},
        {QStringLiteral("signatureCount"), interfaces.size()},
        {QStringLiteral("interfaces"), interfaces},
        {QStringLiteral("scope"),
         QStringLiteral(
             "Literal interface closure and zero-IO rejection/shape checks; positive adapter semantics use separate provider/native/fake witnesses")}};
    QDir directory(QCoreApplication::applicationDirPath());
    QVERIFY(directory.mkpath(QStringLiteral("test-results")));
    QSaveFile output(directory.filePath(QStringLiteral("test-results/backend-equivalence-interfaces.json")));
    QVERIFY(output.open(QIODevice::WriteOnly));
    const auto payload = QJsonDocument(manifest).toJson(QJsonDocument::Indented);
    QCOMPARE(output.write(payload), static_cast<qint64>(payload.size()));
    QVERIFY(output.commit());
  }

  void literalDirectoryClosesEveryPublicSelectorAndRequestAlternative() {
    std::set<std::string_view> slotNames;
    std::set<std::pair<std::string_view, std::string_view>> signatures;
    std::array<bool, std::variant_size_v<Request>> families{};
    std::size_t reads = 0;
    std::size_t writes = 0;
    for (const auto& item : uwfPublicInterfaceCases) {
      slotNames.insert(item.slot);
      QVERIFY(signatures.emplace(item.slot, item.session).second);
      families[item.request.index()] = true;
      QCOMPARE(isWrite(item.request), item.write);
      item.write ? ++writes : ++reads;
    }
    QCOMPARE(slotNames.size(), uwf::test::publicInterfaceSlotCount);
    QCOMPARE(signatures.size(), uwf::test::publicInterfaceSignatureCount);
    QCOMPARE(reads, uwf::test::publicReadSignatureCount);
    QCOMPARE(writes, uwf::test::publicWriteSignatureCount);
    for (const bool present : families) QVERIFY(present);
  }

  void preCanceledRequestsNeverReachProviderOrHostIo_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<int>("backend");
    for (const auto& item : uwfPublicInterfaceCases) addBackendRows(text(item.slot) + u'/' + text(item.session), item.request);
  }
  void preCanceledRequestsNeverReachProviderOrHostIo() {
    QFETCH(Request, request);
    QFETCH(int, backend);
    Harness harness(backend);
    std::stop_source stop;
    stop.request_stop();
    const auto response = harness.backend->execute(request, {stop.get_token()});
    verifyNoIoOrFakeMutation(harness);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Canceled);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(response.failure->effect, Effect::None);
  }

  void invalidSelectorsSessionsAndVolumeIdentitiesNeverReachIo_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<int>("backend");
    const auto add = [](const uwf::test::UwfPublicInterfaceCase& item, const QString& dimension, const int variant, const Request& request) {
      addBackendRows(text(item.slot) + u'/' + text(item.session) + u'/' + dimension + QString::number(variant), request);
    };
    std::string embeddedNul = uwf::test::publicInterfaceVolume;
    embeddedNul.push_back('\0');
    embeddedNul += "suffix";
    const std::array invalidVolumes{std::string{}, std::string("not-a-guid"), std::string(uwf::test::publicInterfaceVolume) + "suffix",
                                    embeddedNul,   std::string("\xc0\xaf"),   std::string("\\\\?\\Volume{") + uwf::test::publicInterfaceVolume + "}\\Child"};
    for (const auto& item : uwfPublicInterfaceCases)
      std::visit(
          [&](const auto& operation) {
            if constexpr (requires { operation.session; }) {
              for (const int invalid : {-1, 2}) {
                auto changed = operation;
                changed.session = static_cast<Session>(invalid);
                add(item, QStringLiteral("session"), invalid, Request{changed});
              }
            }
            if constexpr (requires { operation.property; }) {
              using Selector = std::remove_cvref_t<decltype(operation.property)>;
              for (const int invalid : {-1, static_cast<int>(Selector::Count)}) {
                auto changed = operation;
                changed.property = static_cast<Selector>(invalid);
                add(item, QStringLiteral("property"), invalid, Request{changed});
              }
            }
            if constexpr (requires { operation.option; }) {
              for (const int invalid : {-1, static_cast<int>(OverlayOption::Count)}) {
                auto changed = operation;
                changed.option = static_cast<OverlayOption>(invalid);
                add(item, QStringLiteral("option"), invalid, Request{changed});
              }
            }
            if constexpr (requires { operation.action; }) {
              using Selector = std::remove_cvref_t<decltype(operation.action)>;
              for (const int invalid : {-1, static_cast<int>(Selector::Count)}) {
                auto changed = operation;
                changed.action = static_cast<Selector>(invalid);
                add(item, QStringLiteral("action"), invalid, Request{changed});
              }
            }
            if constexpr (requires { operation.volume; }) {
              for (std::size_t index = 0; index < invalidVolumes.size(); ++index) {
                auto changed = operation;
                changed.volume = invalidVolumes[index];
                add(item, QStringLiteral("volume"), static_cast<int>(index), Request{changed});
              }
            }
          },
          item.request);
  }
  void invalidSelectorsSessionsAndVolumeIdentitiesNeverReachIo() {
    QFETCH(Request, request);
    QFETCH(int, backend);
    Harness harness(backend);
    const auto response = harness.backend->execute(request);
    verifyNoIoOrFakeMutation(harness);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::InvalidArgument);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(response.failure->effect, Effect::None);
  }

  void readOnlyWriterSlotsRejectWithoutProviderDiscovery_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<int>("backend");
    for (const bool value : {false, true}) {
      addBackendRows(QStringLiteral("shutdown-%1").arg(value), SetBoolean{BooleanProperty::ShutdownPending, value});
      addBackendRows(QStringLiteral("registry-persistence-derived-%1").arg(value), SetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted, value});
    }
  }
  void readOnlyWriterSlotsRejectWithoutProviderDiscovery() {
    QFETCH(Request, request);
    QFETCH(int, backend);
    Harness harness(backend);
    const auto response = harness.backend->execute(request);
    verifyNoIoOrFakeMutation(harness);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(response.failure->effect, Effect::None);
  }

  void publicResponseShapeErrorsHaveOneProtocolAndEffectContract_data() {
    QTest::addColumn<Request>("request");
    QTest::addColumn<bool>("writing");
    for (const auto& item : uwfPublicInterfaceCases) QTest::newRow(qUtf8Printable(text(item.slot) + u'/' + text(item.session))) << item.request << item.write;
  }
  void publicResponseShapeErrorsHaveOneProtocolAndEffectContract() {
    QFETCH(Request, request);
    QFETCH(bool, writing);
    // Malformed response shapes exercise normalization without adapter I/O.
    auto malformed = std::holds_alternative<GetOverlayUsage>(request) ? Response::success(false) : Response::success(OverlayUsage{17, 93});
    const auto response = normalizeResponse(request, std::move(malformed));
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Protocol);
    const auto expected = writing ? Effect::Uncertain : Effect::None;
    QCOMPARE(response.effect, expected);
    QCOMPARE(response.failure->effect, expected);
  }
};

QTEST_GUILESS_MAIN(UwfBackendEquivalenceTests)
#include "UwfBackendEquivalenceTests.moc"
