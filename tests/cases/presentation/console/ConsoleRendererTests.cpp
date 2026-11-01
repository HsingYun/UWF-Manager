// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>

#include "presentation/console/ConsoleRenderer.h"

namespace {
bool has(const std::string& text, const std::string_view needle) { return text.find(needle) != std::string::npos; }
bool hasPrivateEvidence(const std::string& text) {
  for (const auto needle : {"private-provider-message", "private-backend", "UwfCfgPrivateExport", "SELECT * FROM PrivateClass", "<PrivateXml>", "private-detail"})
    if (has(text, needle)) return true;
  return false;
}
uwf::compat::Failure privateFailure(const uwf::compat::Effect effect = uwf::compat::Effect::None) {
  return {uwf::compat::ErrorKind::AccessDenied,
          effect,
          "private-provider-message",
          {{"private-backend", "UwfCfgPrivateExport", std::int64_t{0x80041003}, "SELECT * FROM PrivateClass"},
           {"private-backend", "UwfCfgPrivateExport", std::int64_t{0x80070005}, "<PrivateXml>private-detail</PrivateXml>"},
           {"private-backend", "UwfCfgPrivateExport", 0, "private-detail-without-a-code"}}};
}
class ConsoleRendererTests final : public QObject {
  Q_OBJECT
 private slots:
  void configurationReportHasExplicitTitleAlignedFieldsAndRestartNote() {
    uwf::app::ConsoleCommandResult result;
    result.title = "Overlay configuration";
    result.sections = {{"Current session",
                        {{"UWF filter", "Enabled"},
                         {"Overlay type", "RAM"},
                         {"Maximum size", "40960 MB"},
                         {"Free-space passthrough", "Disabled"},
                         {"Persistent overlay", "Disabled"},
                         {"Read-only media mode", "Disabled"},
                         {"Warning threshold", "32768 MB"},
                         {"Critical threshold", "36864 MB"}},
                        {}},
                       {"Next session",
                        {{"UWF filter", "Enabled"},
                         {"Overlay type", "Disk"},
                         {"Maximum size", "40960 MB"},
                         {"Free-space passthrough", "Disabled"},
                         {"Persistent overlay", "Enabled"},
                         {"Read-only media mode", "Disabled"},
                         {"Default reset policy", "Preserve"}},
                        {}}};
    result.notes = {"Changes to next-session configuration take effect after restarting Windows."};
    const auto text = uwf::console::renderConsoleResult(result);
    QCOMPARE(text.output,
             "Overlay configuration\n"
             "\n"
             "Current session\n"
             "  UWF filter             : Enabled\n"
             "  Overlay type           : RAM\n"
             "  Maximum size           : 40960 MB\n"
             "  Free-space passthrough : Disabled\n"
             "  Persistent overlay     : Disabled\n"
             "  Read-only media mode   : Disabled\n"
             "  Warning threshold      : 32768 MB\n"
             "  Critical threshold     : 36864 MB\n"
             "\n"
             "Next session\n"
             "  UWF filter             : Enabled\n"
             "  Overlay type           : Disk\n"
             "  Maximum size           : 40960 MB\n"
             "  Free-space passthrough : Disabled\n"
             "  Persistent overlay     : Enabled\n"
             "  Read-only media mode   : Disabled\n"
             "  Default reset policy   : Preserve\n"
             "\n"
             "Changes to next-session configuration take effect after restarting Windows.\n");
    QVERIFY(text.error.empty());
  }
  void reportsHaveIndependentOutputAndDiagnostics() {
    uwf::app::ConsoleCommandResult result;
    result.sections = {{"Overlay - Current", {{"Type", "RAM"}, {"Consumed (MB)", "4"}}, {"C:\\data\\\xE4\xB8\xAD\xE6\x96\x87"}}};
    result.warnings = {"Warning threshold exceeds critical threshold"};
    const auto text = uwf::console::renderConsoleResult(result);
    QCOMPARE(text.output, "Overlay - Current\n  Type          : RAM\n  Consumed (MB) : 4\n  C:\\data\\\xE4\xB8\xAD\xE6\x96\x87\n");
    QVERIFY(!has(text.output, "Warning:"));
    QVERIFY(text.error.starts_with("Warning:"));
  }
  void notApplicableDataDoesNotMaskPartialReadFailures_data() {
    QTest::addColumn<bool>("partialFailure");
    QTest::newRow("not-applicable-is-normal-data") << false;
    QTest::newRow("partial-report-keeps-nonzero-error") << true;
  }
  void notApplicableDataDoesNotMaskPartialReadFailures() {
    QFETCH(bool, partialFailure);
    uwf::app::ConsoleCommandResult result;
    result.title = "Overlay configuration";
    result.sections = {{"Current session", {{"Reset on next boot", "N/A (persistent overlay is disabled)"}}, {}}};
    if (partialFailure) {
      result.failure = uwf::compat::Failure{uwf::compat::ErrorKind::AccessDenied, uwf::compat::Effect::None, "Access denied", {}};
      result.sections.front().fields.push_back({"Overlay options", {}, result.failure});
      result.successfulReads = 1;
      result.failedReads = 1;
      result.exitCode = uwf::app::consoleFailureExitCode(result.failure->kind);
      result.warnings = {"Some configuration fields could not be read"};
    }
    std::string output, error;
    const auto code =
        uwf::console::publishConsoleResult(result, [&](const std::string_view text) { output = text; }, [&](const std::string_view text) { error = text; });
    QCOMPARE(code, partialFailure ? 5 : 0);
    QVERIFY(has(output, "N/A (persistent overlay is disabled)"));
    QCOMPARE(has(output, "Error: Access denied"), partialFailure);
    QVERIFY(!has(output, " : 0\n"));
    if (partialFailure) {
      QVERIFY(error.starts_with("Warning: Some configuration fields could not be read\n"));
      QVERIFY(has(error, "Error: Access denied\n"));
      QVERIFY(has(error, "Effect: None\n"));
    } else {
      QVERIFY(error.empty());
    }
  }
  void failureEffectsAndReadStatesKeepOnlyPublicText_data() {
    QTest::addColumn<int>("effect");
    QTest::addColumn<QByteArray>("name");
    QTest::addColumn<int>("scenario");
    constexpr std::array effectNames{"None", "Confirmed", "Uncertain"};
    constexpr std::array scenarios{"failure-only", "failed-with-section", "partially-read", "all-reads-failed", "read-interrupted"};
    for (int effect = 0; effect < 3; ++effect)
      for (int scenario = 0; scenario < 5; ++scenario) {
        const auto effectName = effectNames[static_cast<std::size_t>(effect)];
        const auto row = QByteArray(effectName) + '/' + scenarios[static_cast<std::size_t>(scenario)];
        QTest::newRow(row.constData()) << effect << QByteArray(effectName) << scenario;
      }
  }
  void failureEffectsAndReadStatesKeepOnlyPublicText() {
    QFETCH(int, effect);
    QFETCH(QByteArray, name);
    QFETCH(int, scenario);
    uwf::app::ConsoleCommandResult result;
    result.effect = static_cast<uwf::compat::Effect>(effect);
    result.failure = privateFailure(result.effect);
    result.failures.push_back({"GetFilterEnabled", *result.failure});
    result.attempts.push_back({"private-backend", {uwf::compat::Support::Unknown, "private-detail"}, result.failure});
    if (scenario != 0) {
      result.title = "Configuration";
      result.sections = {{"Current session", {{"UWF filter", "false", result.failure}}, {}}};
    }
    if (scenario == 2 || scenario == 3) {
      result.failedReads = 3;
      result.successfulReads = scenario == 2 ? 2U : 0U;
    }
    if (scenario == 0) result.interrupted = true;
    if (scenario == 4) {
      result.successfulReads = 2;
      result.interrupted = true;
    }
    if (scenario == 4) result.sections.front().fields = {{"UWF filter", "Enabled"}};
    const auto text = uwf::console::renderConsoleResult(result);
    QCOMPARE(text.output.empty(), scenario == 0);
    QVERIFY(text.error.starts_with("Error: Access denied (0x80070005)\n"));
    QVERIFY(has(text.error, "Effect: " + name.toStdString()));
    QVERIFY(!has(text.error, "Reason:"));
    QVERIFY(!has(text.error, "backend="));
    QVERIFY(!has(text.error, "operation="));
    QVERIFY(!has(text.error, "code="));
    QVERIFY(!hasPrivateEvidence(text.output));
    QVERIFY(!hasPrivateEvidence(text.error));
    QCOMPARE(has(text.error, "Check state before retrying"), effect != 0);
    if (scenario == 2 || scenario == 3) {
      const auto status = scenario == 2 ? "Partially read" : "Read failed";
      QVERIFY(text.output.starts_with("Configuration\nStatus: " + std::string(status) + " (3 failed observations)\n\nCurrent session\n"));
    } else if (scenario == 4) {
      QVERIFY(text.output.starts_with("Configuration\nStatus: Partially read (interrupted)\n\nCurrent session\n"));
      QCOMPARE(result.failedReads, std::size_t{0});
    } else {
      QVERIFY(!has(text.output, "Status:"));
    }
    if (scenario != 0 && scenario != 4) {
      QVERIFY(has(text.output, "UWF filter : Error: Access denied (0x80070005)"));
      QVERIFY(!has(text.output, " : false"));
      QCOMPARE(result.sections.front().fields.front().failure->diagnostics.back().detail, "private-detail-without-a-code");
    }
    if (scenario == 4) {
      QVERIFY(has(text.output, "UWF filter : Enabled"));
      QVERIFY(!has(text.output, "failed observations"));
    }
    QCOMPARE(result.failure->message, "private-provider-message");
    QCOMPARE(result.failure->diagnostics.size(), std::size_t{3});
    QCOMPARE(result.failure->diagnostics.front().backend, "private-backend");
    QCOMPARE(result.failure->diagnostics.front().operation, "UwfCfgPrivateExport");
    QCOMPARE(result.failure->diagnostics.front().detail, "SELECT * FROM PrivateClass");
    QCOMPARE(result.failure->diagnostics[1].code, std::int64_t{0x80070005});
    QCOMPARE(result.failure->diagnostics[1].detail, "<PrivateXml>private-detail</PrivateXml>");
    QCOMPARE(result.attempts.front().failure->message, "private-provider-message");
    QCOMPARE(result.attempts.front().capability.reason, "private-detail");
    QCOMPARE(result.failures.front().operation, "GetFilterEnabled");
    QCOMPARE(result.failures.front().failure.diagnostics.front().detail, "SELECT * FROM PrivateClass");
  }
  void completedTargetAbsenceDoesNotBecomeAnInterruptedRead() {
    uwf::app::ConsoleCommandResult result;
    result.title = "Volume configuration";
    result.sections = {{"Current session", {{"Status", "Not configured"}}, {}}, {"Next session", {{"Status", "Not configured"}}, {}}};
    result.successfulReads = 2;
    result.failure = uwf::compat::Failure{uwf::compat::ErrorKind::NotFound, uwf::compat::Effect::None, "private-provider-message", {}};
    result.exitCode = 1168;
    const auto text = uwf::console::renderConsoleResult(result);
    QCOMPARE(text.output,
             "Volume configuration\n\nCurrent session\n  Status : Not configured\n\nNext session\n  Status : Not configured\n");
    QCOMPARE(text.error, "Error: Target not found\nEffect: None\n");
    QVERIFY(!has(text.output, "Partially read"));
    QVERIFY(!has(text.output, "interrupted"));
    QVERIFY(!result.interrupted);
    QCOMPARE(result.failedReads, std::size_t{0});
  }
  void interruptedReadKeepsItsActualFailureCount_data() {
    QTest::addColumn<bool>("hasSuccessfulReads");
    QTest::newRow("partial-before-interruption") << true;
    QTest::newRow("failed-before-interruption") << false;
  }
  void interruptedReadKeepsItsActualFailureCount() {
    QFETCH(bool, hasSuccessfulReads);
    uwf::app::ConsoleCommandResult result;
    result.title = "UWF configuration";
    result.failure = privateFailure();
    result.sections = {{"Current session", {{"Filter", {}, result.failure}}, {}}};
    result.successfulReads = hasSuccessfulReads ? 1U : 0U;
    result.failedReads = 2;
    result.interrupted = true;
    const auto text = uwf::console::renderConsoleResult(result);
    const auto status = hasSuccessfulReads ? "Partially read" : "Read failed";
    QVERIFY(text.output.starts_with("UWF configuration\nStatus: " + std::string(status) + " (2 failed observations; interrupted)\n\n"));
    QVERIFY(has(text.output, "Filter : Error: Access denied (0x80070005)"));
    QCOMPARE(result.failedReads, std::size_t{2});
    QVERIFY(result.interrupted);
  }
  void nestedReportsKeepSessionVolumeAndFileHierarchyAndGlobalAlignment() {
    uwf::app::ConsoleCommandResult result;
    const auto failure = privateFailure();
    const std::string path = "C:\\Data\\\xE4\xB8\xAD\xE6\x96\x87";
    const uwf::app::ConsoleSection files{"File exclusions", {}, {path, path, "C:\\Data\\last.ini"}};
    const uwf::app::ConsoleSection volume{"Volume C:", {{"Protection", "false", failure}}, {}, {files}};
    const uwf::app::ConsoleSection volumes{"Volumes", {}, {}, {volume}};
    const uwf::app::ConsoleSection overlay{"Overlay", {{"Type", "RAM"}}, {}};
    result.title = "UWF configuration";
    result.sections = {{"Runtime", {{"HORM", "Disabled"}}, {}},
                       {"Current session", {{"Filter", "Enabled"}}, {}, {overlay, volumes}},
                       {"Next session", {{"Filter", "Disabled"}}, {}}};
    result.successfulReads = 4;
    result.failedReads = 1;
    result.failure = failure;
    const auto text = uwf::console::renderConsoleResult(result);
    QCOMPARE(text.output,
             "UWF configuration\n"
             "Status: Partially read (1 failed observations)\n"
             "\n"
             "Runtime\n"
             "  HORM       : Disabled\n"
             "\n"
             "Current session\n"
             "  Filter     : Enabled\n"
             "  Overlay\n"
             "    Type       : RAM\n"
             "  Volumes\n"
             "    Volume C:\n"
             "      Protection : Error: Access denied (0x80070005)\n"
             "      File exclusions\n"
             "        C:\\Data\\\xE4\xB8\xAD\xE6\x96\x87\n"
             "        C:\\Data\\\xE4\xB8\xAD\xE6\x96\x87\n"
             "        C:\\Data\\last.ini\n"
             "\n"
             "Next session\n"
             "  Filter     : Disabled\n");
    QVERIFY(!hasPrivateEvidence(text.output));
    QCOMPARE(result.sections[1].children[1].children.front().children.front().items, files.items);
    QCOMPARE(result.sections[1].children[1].children.front().fields.front().failure->message, "private-provider-message");
  }
  void collectionsKeepEveryEntryInOriginalOrderWithoutTruncatingUnicode() {
    uwf::app::ConsoleCommandResult result;
    uwf::app::ConsoleSection section{"File exclusions", {}, {}};
    std::string expected = "File exclusions\n";
    constexpr int count = 4096;
    for (int index = count; index > 0; --index) {
      const auto item = "C:\\" + std::to_string(index) + "\\" + std::string(256, 'x') + "\\\xE4\xB8\xAD\xE6\x96\x87\xF0\x9F\x92\xBE";
      section.items.push_back(item);
      expected += "  " + item + '\n';
    }
    result.sections.push_back(section);
    const auto text = uwf::console::renderConsoleResult(result);
    QCOMPARE(text.output, expected);
    QCOMPARE(std::count(text.output.begin(), text.output.end(), '\n'), count + 1);
    QCOMPARE(result.sections.front().items, section.items);
    QVERIFY(text.error.empty());
  }
  void actualErrorCodeUsesTheLastNonzeroDiagnostic_data() {
    QTest::addColumn<qint64>("code");
    QTest::addColumn<QByteArray>("suffix");
    QTest::newRow("none") << qint64{0} << QByteArray{};
    QTest::newRow("positive-hresult") << qint64{0x80070005} << QByteArray(" (0x80070005)");
    QTest::newRow("signed-hresult") << qint64{-2147024891} << QByteArray(" (0x80070005)");
    QTest::newRow("win32") << qint64{5} << QByteArray(" (0x00000005)");
    QTest::newRow("wider-native-code") << qint64{0x100000000} << QByteArray(" (0x100000000)");
  }
  void actualErrorCodeUsesTheLastNonzeroDiagnostic() {
    QFETCH(qint64, code);
    QFETCH(QByteArray, suffix);
    uwf::app::ConsoleCommandResult result;
    result.failure = uwf::compat::Failure{uwf::compat::ErrorKind::AccessDenied,
                                          uwf::compat::Effect::None,
                                          "private-provider-message",
                                          {{"private-backend", "UwfCfgPrivateExport", 0, "private-detail"},
                                           {"private-backend", "UwfCfgPrivateExport", code, "private-detail"},
                                           {"private-backend", "UwfCfgPrivateExport", 0, "private-detail"}}};
    result.sections = {{"Current session", {{"Filter", {}, result.failure}}, {}}};
    const auto text = uwf::console::renderConsoleResult(result);
    QVERIFY(text.error.starts_with("Error: Access denied" + suffix.toStdString() + '\n'));
    QVERIFY(has(text.output, "Filter : Error: Access denied" + suffix.toStdString() + '\n'));
    QVERIFY(!has(text.error, "0x00000000"));
    QVERIFY(!has(text.output, "0x00000000"));
  }
  void onlyExplicitPublicReasonIsShownAndControlsAreEscaped() {
    uwf::app::ConsoleCommandResult result;
    result.failure = privateFailure();
    QVERIFY(!has(uwf::console::renderConsoleResult(result).error, "Reason:"));
    result.publicReason = "Invalid operand: C:\\\xE4\xB8\xAD\xE6\x96\x87\n\x1b[2J";
    const auto text = uwf::console::renderConsoleResult(result);
    QVERIFY(has(text.error, "Reason: Invalid operand: C:\\\xE4\xB8\xAD\xE6\x96\x87\\x0A\\x1B[2J\n"));
    QVERIFY(!hasPrivateEvidence(text.output));
    QVERIFY(!hasPrivateEvidence(text.error));
    QVERIFY(text.error.find('\x1b') == std::string::npos);
    QCOMPARE(result.failure->message, "private-provider-message");
  }
  void operandControlsCannotBecomeConsoleInstructions() {
    uwf::app::ConsoleCommandResult result;
    result.title = "Configuration\n\x1b[2J";
    result.sections = {{"Registry", {}, {"value\n\x1b[2J"}}};
    result.notes = {"Restart\tWindows\r", "\xE4\xB8\xAD\xE6\x96\x87"};
    const auto text = uwf::console::renderConsoleResult(result);
    QVERIFY(text.output.starts_with("Configuration\\x0A\\x1B[2J\n\nRegistry\n"));
    QVERIFY(has(text.output, "value\\x0A\\x1B[2J"));
    QVERIFY(text.output.ends_with("\n\nRestart\\x09Windows\\x0D\n\xE4\xB8\xAD\xE6\x96\x87\n"));
    QVERIFY(text.output.find('\x1b') == std::string::npos);
  }
  void outputFailureKeepsTheExecutedEffectAndStillUsesStderr_data() {
    QTest::addColumn<int>("effect");
    QTest::addColumn<bool>("originalFailure");
    for (int effect = 0; effect < 3; ++effect)
      for (const bool failure : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("effect-%1/original-failure-%2").arg(effect).arg(failure))) << effect << failure;
  }
  void outputFailureKeepsTheExecutedEffectAndStillUsesStderr() {
    QFETCH(int, effect);
    QFETCH(bool, originalFailure);
    uwf::app::ConsoleCommandResult result;
    result.sections = {{"Result", {{"Status", "Saved"}}, {}}};
    result.effect = static_cast<uwf::compat::Effect>(effect);
    result.warnings = {"Original warning"};
    if (originalFailure) {
      result.failure = privateFailure(result.effect);
      result.exitCode = 5;
    }
    int writes = 0;
    std::string diagnostics;
    const auto code = uwf::console::publishConsoleResult(
        result,
        [&](std::string_view) {
          ++writes;
          throw std::runtime_error("Broken pipe");
        },
        [&](const std::string_view text) { diagnostics = text; });
    QCOMPARE(code, 1);
    QCOMPARE(writes, 1);
    QVERIFY(has(diagnostics, "Original warning"));
    QVERIFY(has(diagnostics, "Reason: Could not write standard output\n"));
    constexpr std::array effectNames{"None", "Confirmed", "Uncertain"};
    QVERIFY(has(diagnostics, "Effect: " + std::string(effectNames[static_cast<std::size_t>(effect)])));
    QCOMPARE(has(diagnostics, "Check state before retrying"), effect != 0);
    QCOMPARE(has(diagnostics, "Error: Access denied (0x80070005)"), originalFailure);
    QVERIFY(!has(diagnostics, "backend="));
    QVERIFY(!has(diagnostics, "WriteStandardOutput"));
    QVERIFY(!has(diagnostics, "0x00000000"));
    QVERIFY(!has(diagnostics, "Broken pipe"));
    QVERIFY(!hasPrivateEvidence(diagnostics));
    QCOMPARE(result.effect, static_cast<uwf::compat::Effect>(effect));
    if (originalFailure) QCOMPARE(result.failure->diagnostics[1].detail, "<PrivateXml>private-detail</PrivateXml>");
  }
  void stderrFailureCannotReportSuccess_data() {
    QTest::addColumn<int>("effect");
    for (int effect = 0; effect < 3; ++effect) QTest::newRow(qUtf8Printable(QString::number(effect))) << effect;
  }
  void stderrFailureCannotReportSuccess() {
    QFETCH(int, effect);
    uwf::app::ConsoleCommandResult result;
    result.effect = static_cast<uwf::compat::Effect>(effect);
    result.sections = {{"Result", {{"Status", "Saved"}}, {}}};
    result.warnings = {"A warning"};
    std::string output;
    int errorWrites = 0;
    QCOMPARE(uwf::console::publishConsoleResult(
                 result, [&](const std::string_view text) { output = text; },
                 [&](std::string_view) {
                   ++errorWrites;
                   throw std::runtime_error("Closed stderr");
                 }),
             1);
    QCOMPARE(output, "Result\n  Status : Saved\n");
    QCOMPARE(errorWrites, 1);
    QCOMPARE(result.effect, static_cast<uwf::compat::Effect>(effect));
  }
  void providerLocaleDoesNotChooseTheConsoleSummaryLanguage() {
    uwf::app::ConsoleCommandResult result;
    const auto original = QStringLiteral("拒绝访问。").toUtf8().toStdString();
    result.failure = uwf::compat::Failure{
        uwf::compat::ErrorKind::AccessDenied, uwf::compat::Effect::None, original, {{"WMI", "GetFilterEnabled", std::int64_t{0x80041003}, original}}};
    const auto text = uwf::console::renderConsoleResult(result);
    QCOMPARE(text.error, "Error: Access denied (0x80041003)\nEffect: None\n");
    QVERIFY(!has(text.error, original));
    QVERIFY(!has(text.error, "Reason:"));
    QVERIFY(!has(text.error, "WMI"));
    QVERIFY(!has(text.error, "GetFilterEnabled"));
    QCOMPARE(result.failure->message, original);
    QCOMPARE(result.failure->diagnostics.front().detail, original);
  }
  void everyFailureKindHidesRawTransportIdentity_data() {
    using uwf::compat::ErrorKind;
    QTest::addColumn<int>("kind");
    QTest::addColumn<QString>("identity");
    QTest::addColumn<QString>("summary");
    struct FailureCase {
      ErrorKind kind;
      const char* name;
      const char* summary;
    };
    constexpr std::array cases{FailureCase{ErrorKind::Unsupported, "unsupported", "Unsupported operation"},
                               FailureCase{ErrorKind::Unavailable, "unavailable", "UWF operation unavailable"},
                               FailureCase{ErrorKind::AccessDenied, "access-denied", "Access denied"},
                               FailureCase{ErrorKind::NotFound, "not-found", "Target not found"},
                               FailureCase{ErrorKind::InvalidArgument, "invalid-argument", "Invalid argument"},
                               FailureCase{ErrorKind::Canceled, "canceled", "Operation canceled"},
                               FailureCase{ErrorKind::ResourceExhausted, "resource-exhausted", "Insufficient resources"},
                               FailureCase{ErrorKind::Conflict, "conflict", "Operation conflicts with the current state"},
                               FailureCase{ErrorKind::Protocol, "protocol", "Invalid UWF response"},
                               FailureCase{ErrorKind::Failed, "failed", "Operation failed"}};
    for (const auto& item : cases)
      for (const auto* identity : {"WMI", "CSP", "Native", "uwfcfgmgmt.dll"}) {
        const auto row = QByteArray(item.name) + '/' + identity;
        QTest::newRow(row.constData()) << static_cast<int>(item.kind) << QString::fromLatin1(identity) << QString::fromLatin1(item.summary);
      }
  }
  void everyFailureKindHidesRawTransportIdentity() {
    QFETCH(int, kind);
    QFETCH(QString, identity);
    QFETCH(QString, summary);
    const auto source = identity.toStdString();
    const auto original = "backend=" + source + " provider=" + source +
                          " operation=UwfCfgPrivateExport detail=<PrivateXml>SELECT * FROM PrivateClass</PrivateXml> XML WQL";
    const uwf::compat::Failure failure{static_cast<uwf::compat::ErrorKind>(kind),
                                       uwf::compat::Effect::Uncertain,
                                       original,
                                       {{source, "UwfCfgPrivateExport", std::int64_t{0x80070005}, original}}};
    uwf::app::ConsoleCommandResult result;
    result.failure = failure;
    result.effect = failure.effect;
    result.failedReads = 1;
    result.sections = {{"Current session", {{"State", "ignored failure placeholder", failure}}, {}}};
    result.failures.push_back({"UwfCfgPrivateExport", failure});
    result.attempts.push_back({source, {uwf::compat::Support::Unknown, original}, failure});
    QVERIFY(result.publicReason.empty());
    const auto text = uwf::console::renderConsoleResult(result);
    const auto publicSummary = summary.toStdString() + " (0x80070005)";
    QVERIFY(has(text.output, "State : Error: " + publicSummary + '\n'));
    QVERIFY(text.error.starts_with("Error: " + publicSummary + '\n'));
    QVERIFY(has(text.error, "Effect: Uncertain\n"));
    QVERIFY(has(text.error, "Check state before retrying"));
    for (const auto& stream : {text.output, text.error}) {
      const auto visible = QString::fromStdString(stream);
      for (const auto* forbidden : {"backend", "provider", "WMI", "CSP", "Native", "uwfcfgmgmt.dll", ".dll", "WQL", "XML", "SELECT * FROM", "operation=",
                                    "detail=", "UwfCfgPrivateExport", "PrivateClass", "ignored failure placeholder"})
        QVERIFY2(!visible.contains(QString::fromLatin1(forbidden), Qt::CaseInsensitive), forbidden);
      QVERIFY(!has(stream, "Reason:"));
    }
    QCOMPARE(result.failure->kind, failure.kind);
    QCOMPARE(result.failure->effect, uwf::compat::Effect::Uncertain);
    QCOMPARE(result.failure->message, original);
    QCOMPARE(result.failure->diagnostics.front().backend, source);
    QCOMPARE(result.failure->diagnostics.front().operation, "UwfCfgPrivateExport");
    QCOMPARE(result.failure->diagnostics.front().code, std::int64_t{0x80070005});
    QCOMPARE(result.failure->diagnostics.front().detail, original);
    QCOMPARE(result.sections.front().fields.front().failure->message, original);
    QCOMPARE(result.failures.front().failure.diagnostics.front().detail, original);
    QCOMPARE(result.attempts.front().backend, source);
    QCOMPARE(result.attempts.front().capability.reason, original);
    QCOMPARE(result.attempts.front().failure->diagnostics.front().detail, original);
  }
  void transportLikeWordsInBusinessDataRemainVerbatim() {
    uwf::app::ConsoleCommandResult result;
    const std::string path = "C:\\WMI\\CSP\\Native\\uwfcfgmgmt.dll";
    const std::string value = "backend provider operation=detail= XML WQL";
    result.sections = {{"Files", {{"Name", value}}, {path}}};
    const auto text = uwf::console::renderConsoleResult(result);
    QCOMPARE(text.output, "Files\n  Name : " + value + "\n  " + path + '\n');
    QVERIFY(text.error.empty());
  }
};
}  // namespace
QTEST_APPLESS_MAIN(ConsoleRendererTests)
#include "ConsoleRendererTests.moc"
