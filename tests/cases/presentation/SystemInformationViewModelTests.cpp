// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <limits>

#include "presentation/diagnostics/SystemInformationViewModel.h"
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
    return hardwareInfo;
  }
  uwf::app::SystemDiagnosticFacts diagnostics() const override {
    ++diagnosticReads;
    return facts;
  }
  uwf::app::WindowsVersionInfo version;
  uwf::app::SystemHardwareInfo hardwareInfo;
  uwf::app::SystemDiagnosticFacts facts;
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
    MemorySystemInformationSource source;
    source.hardwareInfo.totalMemoryBytes = bytes;
    source.hardwareInfo.graphicsAdapter.name = gpu.toStdString();
    uwf::presentation::SystemInformationViewModel model(source);
    QCOMPARE(model.summaryParts(), expected);
    source.version.productName.clear();
    QCOMPARE(model.summaryParts().constFirst(), QStringLiteral("Windows"));
  }
  void readsOnlyTheRequestedInjectedFacts() {
    MemorySystemInformationSource source;
    uwf::presentation::SystemInformationViewModel model(source);
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
    MemorySystemInformationSource source;
    source.facts.windows = source.version;
    source.facts.processId = 123;
    source.facts.uptimeMs = 4000;
    source.facts.architecture = QStringLiteral("test-architecture");
    source.facts.locale = QStringLiteral("test-locale");
    source.facts.timeZone = QStringLiteral("Test Zone (UTC+08:00)");
    source.facts.token.push_back({QStringLiteral("token.test"), QStringLiteral("injected")});
    uwf::presentation::SystemInformationViewModel model(source);
    uwf::app::DiagnosticReportContext context;
    context.compatibilityMode = true;
    const QString report = model.diagnosticReport(context);
    QVERIFY(report.contains(QStringLiteral("windows.product = Test Windows")));
    QVERIFY(report.contains(QStringLiteral("process.id = 123")));
    QVERIFY(report.contains(QStringLiteral("system.architecture = test-architecture")));
    QVERIFY(report.contains(QStringLiteral("system.time_zone = Test Zone (UTC+08:00)")));
    QVERIFY(report.contains(QStringLiteral("token.test = injected")));
    QVERIFY(report.contains(QStringLiteral("compatibility_mode = yes")));
    QCOMPARE(source.diagnosticReads, 1);
    QCOMPARE(source.hardwareReads, 0);
  }
  void overviewUsesTypedFactsAndKeepsTheFullReport() {
    MemorySystemInformationSource source;
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
    uwf::presentation::SystemInformationViewModel model(source);
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
    MemorySystemInformationSource source;
    source.facts.windowsError = QStringLiteral("version access denied");
    uwf::presentation::SystemInformationViewModel model(source);
    const QString report = model.diagnosticReport({});
    QVERIFY(report.contains(QStringLiteral("windows.error = version access denied")));
    QVERIFY(!report.contains(QStringLiteral("windows.product =")));
    QVERIFY(report.contains(QStringLiteral("snapshot.available = no")));
  }
};
}  // namespace
QTEST_GUILESS_MAIN(SystemInformationViewModelTests)
#include "SystemInformationViewModelTests.moc"
