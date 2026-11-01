// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <exception>
#include <limits>
#include <stdexcept>

#include "application/diagnostics/DiagnosticReportService.h"
#include "presentation/viewmodels/diagnostics/SystemInformationViewModel.h"
namespace {
class MemorySystemInformationSource final : public uwf::app::SystemInformationSource {
 public:
  MemorySystemInformationSource() {
    version.family = uwf::app::WindowsFamily::Windows11;
    version.major = 10;
    version.build = 26100;
    version.productName = "Test Windows";
    hardwareInfo.cpuModel = "Injected CPU";
  }
  uint32_t totalRamMb() const override { return 8192; }
  QString systemDriveLetter() const override { return QStringLiteral("X:"); }
  const uwf::app::WindowsVersionInfo& windowsVersion() const override {
    ++versionReads;
    return version;
  }
  const uwf::app::SystemHardwareInfo& hardware() const override {
    ++hardwareReads;
    if (hardwareFailure) std::rethrow_exception(hardwareFailure);
    return hardwareInfo;
  }
  uwf::app::SystemDiagnosticFacts diagnostics() const override {
    ++diagnosticReads;
    return facts;
  }
  uwf::app::WindowsVersionInfo version;
  uwf::app::SystemHardwareInfo hardwareInfo;
  uwf::app::SystemDiagnosticFacts facts;
  std::exception_ptr hardwareFailure;
  mutable int versionReads = 0;
  mutable int hardwareReads = 0;
  mutable int diagnosticReads = 0;
};
class SystemInformationViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void summaryKeepsSystemMemoryAndGraphicsPriority_data() {
    QTest::addColumn<qulonglong>("bytes");
    QTest::addColumn<QString>("gpu");
    QTest::addColumn<QStringList>("expected");
    const QString system = QStringLiteral("Test Windows");
    QTest::newRow("unknown-hardware") << qulonglong{0} << QString{} << QStringList{system};
    QTest::newRow("below-rounding-boundary") << qulonglong{(1ULL << 29) - 1} << QString{} << QStringList{system};
    QTest::newRow("round-up-boundary") << qulonglong{1ULL << 29} << QString{} << QStringList{system, QStringLiteral("1G")};
    QTest::newRow("all-fields") << qulonglong{96ULL << 30} << QStringLiteral("NVIDIA GeForce RTX 5090")
                                << QStringList{system, QStringLiteral("96G"), QStringLiteral("RTX 5090")};
    QTest::newRow("amd") << qulonglong{0} << QStringLiteral("AMD Radeon RX 7900") << QStringList{system, QStringLiteral("RX 7900")};
    QTest::newRow("intel") << qulonglong{0} << QStringLiteral("Intel(R) Arc(TM) A770") << QStringList{system, QStringLiteral("Arc A770")};
    QTest::newRow("literal-adapter") << qulonglong{0} << QStringLiteral("<Custom & GPU>") << QStringList{system, QStringLiteral("<Custom & GPU>")};
    QTest::newRow("capacity-does-not-overflow") << std::numeric_limits<qulonglong>::max() << QString{} << QStringList{system, QStringLiteral("17179869184G")};
  }
  void summaryKeepsSystemMemoryAndGraphicsPriority() {
    QFETCH(qulonglong, bytes);
    QFETCH(QString, gpu);
    QFETCH(QStringList, expected);
    const auto sourceOwner = std::make_shared<MemorySystemInformationSource>();
    auto& source = *sourceOwner;
    source.hardwareInfo.totalMemoryBytes = bytes;
    source.hardwareInfo.graphicsAdapter.name = gpu.toStdString();
    uwf::presentation::SystemInformationViewModel model(sourceOwner);
    QCOMPARE(model.summaryParts(), expected);
    source.version.productName.clear();
    QCOMPARE(model.summaryParts().constFirst(), QStringLiteral("Windows"));
  }
  void readsOnlyTheRequestedInjectedFacts() {
    const auto sourceOwner = std::make_shared<MemorySystemInformationSource>();
    auto& source = *sourceOwner;
    uwf::presentation::SystemInformationViewModel model(sourceOwner);
    QCOMPARE(source.hardwareReads, 0);
    QCOMPARE(source.versionReads, 0);
    QCOMPARE(source.diagnosticReads, 0);
    QCOMPARE(model.totalRamMb(), uint32_t{8192});
    QCOMPARE(model.systemDriveLetter(), QStringLiteral("X:"));
    QCOMPARE(model.hardware().cpuModel, std::string("Injected CPU"));
    QCOMPARE(model.windowsVersion().productName, std::string("Test Windows"));
    QCOMPARE(source.hardwareReads, 1);
    QCOMPARE(source.versionReads, 1);
    QCOMPARE(source.diagnosticReads, 0);
  }
  void diagnosticReportUsesInjectedSystemAndApplicationFacts() {
    const auto sourceOwner = std::make_shared<MemorySystemInformationSource>();
    auto& source = *sourceOwner;
    source.facts.windows = source.version;
    source.facts.processId = 123;
    source.facts.uptimeMs = 4000;
    source.facts.architecture = QStringLiteral("test-architecture");
    source.facts.locale = QStringLiteral("test-locale");
    source.facts.timeZone = QStringLiteral("Test Zone (UTC+08:00)");
    source.facts.token.push_back({QStringLiteral("token.test"), QStringLiteral("injected")});
    source.hardwareInfo.totalMemoryBytes = 48ULL << 30;
    source.hardwareInfo.totalMemorySlots = 4;
    source.hardwareInfo.memoryModules = {{32ULL << 30, 6000, uwf::app::SmbiosMemoryType::DDR5}, {16ULL << 30, 0, static_cast<uwf::app::SmbiosMemoryType>(99)}};
    source.hardwareInfo.graphicsAdapter = {"Injected GPU", 12ULL << 30};
    uwf::presentation::SystemInformationViewModel model(sourceOwner);
    uwf::app::DiagnosticReportContext context;
    context.compatibilityMode = true;
    const QString report = model.diagnosticReport(context);
    QVERIFY(report.contains(QStringLiteral("windows.product = Test Windows")));
    QVERIFY(report.contains(QStringLiteral("process.id = 123")));
    QVERIFY(report.contains(QStringLiteral("system.architecture = test-architecture")));
    QVERIFY(report.contains(QStringLiteral("system.time_zone = Test Zone (UTC+08:00)")));
    QVERIFY(report.contains(QStringLiteral("token.test = injected")));
    QVERIFY(report.contains(QStringLiteral("compatibility_mode = yes")));
    for (const auto* field :
         {"cpu.model = Injected CPU", "memory.total_bytes = 51539607552", "memory.slots.total = 4", "memory.modules.count = 2",
          "memory.module.0.capacity_bytes = 34359738368", "memory.module.0.configured_speed_mt_per_second = 6000", "memory.module.0.type_smbios = 34",
          "memory.module.0.type = DDR5", "memory.module.1.capacity_bytes = 17179869184", "memory.module.1.configured_speed_mt_per_second = 0",
          "memory.module.1.type_smbios = 99", "memory.module.1.type = <empty>", "gpu.name = Injected GPU", "gpu.dedicated_memory_bytes = 12884901888"})
      QVERIFY2(report.contains(QString::fromLatin1(field)), field);
    QCOMPARE(source.diagnosticReads, 1);
    QCOMPARE(source.hardwareReads, 1);
  }
  void hardwareFailurePreservesOtherDiagnosticFacts_data() {
    QTest::addColumn<bool>("standardException");
    QTest::newRow("standard-exception") << true;
    QTest::newRow("unexpected-exception") << false;
  }
  void hardwareFailurePreservesOtherDiagnosticFacts() {
    QFETCH(bool, standardException);
    const auto source = std::make_shared<MemorySystemInformationSource>();
    source->facts.windows = source->version;
    source->hardwareFailure = standardException ? std::make_exception_ptr(std::runtime_error("hardware source unavailable")) : std::make_exception_ptr(0);
    uwf::presentation::SystemInformationViewModel model(source);
    const auto document = model.diagnosticDocument({});
    QVERIFY(document.partial());
    const auto expected =
        standardException ? QStringLiteral("hardware source unavailable") : QStringLiteral("Unexpected error while reading hardware information");
    QVERIFY(document.report().contains(QStringLiteral("hardware.error = ") + expected));
    QVERIFY(document.report().contains(QStringLiteral("windows.product = Test Windows")));
    QVERIFY(document.report().contains(QStringLiteral("snapshot.available = no")));
    QVERIFY(document.report().contains(QStringLiteral("--- end log ---")));
    QVERIFY(!document.report().contains(QStringLiteral("cpu.model =")));
    QCOMPARE(source->hardwareReads, 1);
    QCOMPARE(source->diagnosticReads, 1);
  }
  void capturedProtectionUsesPhysicalAddressesAndKeepsUnletteredIdentities_data() {
    QTest::addColumn<bool>("reusedLetterProtected");
    QTest::newRow("reused-letter-is-unprotected") << false;
    QTest::newRow("both-physical-volumes-are-protected") << true;
  }
  void capturedProtectionUsesPhysicalAddressesAndKeepsUnletteredIdentities() {
    QFETCH(bool, reusedLetterProtected);
    uwf::core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current.filter.enabled = true;
    const std::string first = "Volume{11111111-2222-3333-4444-555555555555}";
    const std::string second = "Volume{66666666-7777-8888-9999-aaaaaaaaaaaa}";
    const std::string unlettered = "Volume{bbbbbbbb-cccc-dddd-eeee-ffffffffffff}";
    snapshot.current.volumes = {{.volumeName = first, .driveLetter = "D:", .isProtected = true, .physicalDriveLetter = "E:"},
                                {.volumeName = second, .driveLetter = "D:", .isProtected = reusedLetterProtected, .physicalDriveLetter = "D:"},
                                {.volumeName = unlettered, .driveLetter = "F:", .isProtected = true, .physicalDriveLetter = std::string{}}};
    const auto context = uwf::app::DiagnosticReportService::capture(uwf::app::UwfCapability::Available, &snapshot, nullptr, false);
    QStringList expected{QStringLiteral("E:")};
    if (reusedLetterProtected) expected.append(QStringLiteral("D:"));
    expected.append(QString::fromStdString(unlettered));
    QCOMPARE(context.uwf.protectedVolumes, expected);
    const auto source = std::make_shared<MemorySystemInformationSource>();
    source->facts.windows = source->version;
    uwf::presentation::SystemInformationViewModel model(source);
    const auto document = model.diagnosticDocument(context);
    QCOMPARE(document.sections()[2].fields[2].value, expected.join(QStringLiteral(" · ")));
  }
  void overviewUsesTypedFactsAndKeepsTheFullReport() {
    const auto sourceOwner = std::make_shared<MemorySystemInformationSource>();
    auto& source = *sourceOwner;
    source.version.productName = "<b>Literal system & name</b>";
    source.facts.windows = source.version;
    source.facts.architecture = QStringLiteral("x64");
    uwf::app::DiagnosticReportContext context;
    context.uwf.snapshotPresent = context.uwf.uwfAvailable = context.uwf.elevated = true;
    context.uwf.current.filterEnabled = true;
    context.uwf.current.overlayType = QStringLiteral("ram");
    context.uwf.overlayConsumptionMb = 1024;
    context.uwf.overlayAvailableMb = 3072;
    context.uwf.protectedVolumes = {QStringLiteral("X:")};
    context.enhancedMode.present = context.enhancedMode.serviceExists = context.enhancedMode.running = true;
    context.enhancedMode.localSystemAccount = true;
    context.enhancedMode.agentState = QStringLiteral("connected");
    uwf::presentation::SystemInformationViewModel model(sourceOwner);
    const auto document = model.diagnosticDocument(context);
    QVERIFY(!document.partial());
    QCOMPARE(document.sections().size(), 4);
    QCOMPARE(document.sections()[0].fields[0].value, QStringLiteral("<b>Literal system & name</b>"));
    QCOMPARE(document.sections()[0].fields[2].value, QStringLiteral("8 GB"));
    QCOMPARE(document.sections()[2].fields[1].value, QStringLiteral("1,024 / 4,096 MB"));
    QCOMPARE(document.sections()[2].fields[2].value, QStringLiteral("X:"));
    QCOMPARE(document.sections()[3].fields[1].value, QStringLiteral("LocalSystem"));
    QVERIFY(document.report().contains(QStringLiteral("current.filter.enabled = yes")));
    QVERIFY(document.report().contains(QStringLiteral("windows.product = <b>Literal system & name</b>")));
    QCOMPARE(source.diagnosticReads, 1);
    source.facts.windows.reset();
    source.facts.windowsError = QStringLiteral("diagnostic source failure");
    const auto partial = model.diagnosticDocument({});
    QVERIFY(partial.partial());
    QCOMPARE(partial.sections()[0].fields[0].value, QStringLiteral("Unavailable"));
    QVERIFY(partial.report().contains(source.facts.windowsError));
  }
  void unavailableWindowsVersionKeepsTheReportedFailure() {
    const auto sourceOwner = std::make_shared<MemorySystemInformationSource>();
    auto& source = *sourceOwner;
    source.facts.windowsError = QStringLiteral("version access denied");
    uwf::presentation::SystemInformationViewModel model(sourceOwner);
    const QString report = model.diagnosticReport({});
    QVERIFY(report.contains(QStringLiteral("windows.error = version access denied")));
    QVERIFY(!report.contains(QStringLiteral("windows.product =")));
    QVERIFY(report.contains(QStringLiteral("snapshot.available = no")));
  }
};
}  // namespace
QTEST_GUILESS_MAIN(SystemInformationViewModelTests)
#include "SystemInformationViewModelTests.moc"
