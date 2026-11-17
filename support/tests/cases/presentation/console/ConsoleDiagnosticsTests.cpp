// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QByteArray>
#include <QList>
#include <QString>
#include <QTest>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "application/console/ConsoleCommandExecutor.h"
#include "foundation/logging/Log.h"
#include "presentation/console/ConsoleDiagnostics.h"
#include "presentation/console/ConsoleRenderer.h"

namespace {
using namespace uwf;

bool has(const std::string& text, const std::string_view value) { return text.find(value) != std::string::npos; }

std::vector<std::string> commandLogs(const char level) {
  auto lines = recentLogLines();
  const auto marker = std::string(" ") + level + " command] ";
  std::erase_if(lines, [&](const auto& line) { return !has(line, marker); });
  return lines;
}

compat::Failure providerFailure(const compat::Effect effect, const bool withCode) {
  return {compat::ErrorKind::AccessDenied,
          effect,
          QStringLiteral("raw-reason-sentinel SELECT * FROM UWF_Volume WHERE CurrentSession=TRUE; \"raw quote\"\n拒绝访问资料").toUtf8().toStdString(),
          {{"runtime-fixture", "ActualProviderRead", withCode ? std::int64_t{-2147024891} : std::int64_t{0},
            QStringLiteral("<Result path=\"C:\\Data\">raw-detail-sentinel</Result>\r\nWQL\tSELECT * FROM UWF_Filter; 原始诊断").toUtf8().toStdString()}}};
}

void compareFailure(const compat::Failure& actual, const compat::Failure& expected) {
  QCOMPARE(actual.kind, expected.kind);
  QCOMPARE(actual.effect, expected.effect);
  QCOMPARE(actual.message, expected.message);
  QCOMPARE(actual.diagnostics.size(), expected.diagnostics.size());
  for (std::size_t index = 0; index < actual.diagnostics.size(); ++index) {
    QCOMPARE(actual.diagnostics[index].backend, expected.diagnostics[index].backend);
    QCOMPARE(actual.diagnostics[index].operation, expected.diagnostics[index].operation);
    QCOMPARE(actual.diagnostics[index].code, expected.diagnostics[index].code);
    QCOMPARE(actual.diagnostics[index].detail, expected.diagnostics[index].detail);
  }
}

class ConsoleDiagnosticsTests final : public QObject {
  Q_OBJECT
 private slots:
  void init() { clearLogLines(); }

  void publicErrorsAndOriginalDiagnosticsHaveIndependentContracts_data() {
    QTest::addColumn<int>("effect");
    QTest::addColumn<QByteArray>("effectName");
    QTest::addColumn<bool>("withCode");
    const QList<QByteArray> names{"None", "Confirmed", "Uncertain"};
    for (int effect = 0; effect < names.size(); ++effect)
      for (const bool withCode : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/code-%2").arg(QString::fromLatin1(names[effect])).arg(withCode)))
            << effect << names[effect] << withCode;
  }

  void publicErrorsAndOriginalDiagnosticsHaveIndependentContracts() {
    QFETCH(int, effect);
    QFETCH(QByteArray, effectName);
    QFETCH(bool, withCode);
    const auto original = providerFailure(static_cast<compat::Effect>(effect), withCode);
    app::ConsoleCommandResult result;
    result.failure = original;
    result.effect = original.effect;
    result.exitCode = 5;
    result.failedReads = 1;
    result.successfulReads = 2;
    result.failures.push_back({"GetOverlayFlags", original});
    result.attempts.push_back({"runtime-fixture", {compat::Support::Supported, "original capability"}, original});

    const auto text = console::renderConsoleResult(result);
    const auto publicText = text.output + text.error;
    QVERIFY(has(text.error, "Access denied"));
    QVERIFY(has(text.error, "Effect: " + effectName.toStdString()));
    QCOMPARE(has(text.error, "0x80070005"), withCode);
    for (const auto* hidden : {"raw-reason-sentinel", "raw-detail-sentinel", "runtime-fixture", "ActualProviderRead", "GetOverlayFlags", "SELECT ", "<Result",
                               "backend=", "operation=", "detail=", "0x00000000"})
      QVERIFY2(!has(publicText, hidden), hidden);

    console::logConsoleResult(result);
    const auto errors = commandLogs('E');
    QCOMPARE(errors.size(), std::size_t{1});
    QVERIFY(has(errors.front(), "event=failed "));
    QVERIFY(has(errors.front(), "exit_code=5"));
    QVERIFY(has(errors.front(), "failed_reads=1"));
    QVERIFY(has(errors.front(), "effect=" + effectName.toStdString()));
    QVERIFY(!has(errors.front(), "raw-reason-sentinel"));
    QVERIFY(!has(errors.front(), "raw-detail-sentinel"));
    const auto debug = commandLogs('D');
#if defined(UWF_DEBUG_LOGGING)
    QCOMPARE(debug.size(), std::size_t{1});
    const auto& line = debug.front();
    QVERIFY(has(line, "event=failure operation=\"GetOverlayFlags\""));
    QVERIFY(has(line, "effect=" + effectName.toStdString()));
    QVERIFY(has(line, "reason=\"raw-reason-sentinel SELECT * FROM UWF_Volume"));
    QVERIFY(has(line, "\\\"raw quote\\\"\\n"));
    QVERIFY(has(line, QStringLiteral("拒绝访问资料").toUtf8().toStdString()));
    QVERIFY(has(line, "diagnostic_backend=\"runtime-fixture\""));
    QVERIFY(has(line, "diagnostic_operation=\"ActualProviderRead\""));
    QVERIFY(has(line, "raw-detail-sentinel</Result>\\r\\nWQL\\tSELECT * FROM UWF_Filter;"));
    QVERIFY(has(line, QStringLiteral("原始诊断").toUtf8().toStdString()));
    QCOMPARE(has(line, "code=0x80070005"), withCode);
    QVERIFY(!has(line, "code=0x00000000"));
    QVERIFY(line.find_first_of("\r\n\t") == std::string::npos);
#else
    QVERIFY(debug.empty());
#endif
    compareFailure(*result.failure, original);
    QCOMPARE(result.failures.size(), std::size_t{1});
    QCOMPARE(result.failures.front().operation, std::string("GetOverlayFlags"));
    compareFailure(result.failures.front().failure, original);
    QCOMPARE(result.attempts.size(), std::size_t{1});
    QCOMPARE(result.attempts.front().backend, std::string("runtime-fixture"));
    QCOMPARE(result.attempts.front().capability.support, compat::Support::Supported);
    QCOMPARE(result.attempts.front().capability.reason, std::string("original capability"));
    QVERIFY(result.attempts.front().failure);
    compareFailure(*result.attempts.front().failure, original);
  }

  void everyOriginalFailureKeepsItsOwnAbstractOperation() {
    const auto first = providerFailure(compat::Effect::None, true);
    auto second = providerFailure(compat::Effect::Uncertain, false);
    second.message = "second-original-failure";
    second.diagnostics.front().backend = "CSP";
    second.diagnostics.front().operation = "SecondProviderRead";
    app::ConsoleCommandResult result;
    result.failure = first;
    result.effect = compat::Effect::Uncertain;
    result.exitCode = 5;
    result.failedReads = 2;
    result.failures = {{"GetFilterEnabled", first}, {"GetOverlayFlags", second}};
    console::logConsoleResult(result);
    const auto errors = commandLogs('E');
    QCOMPARE(errors.size(), std::size_t{1});
    QVERIFY(has(errors.front(), "failed_reads=2"));
    QVERIFY(has(errors.front(), "failures=2"));
    const auto debug = commandLogs('D');
#if defined(UWF_DEBUG_LOGGING)
    QCOMPARE(debug.size(), std::size_t{2});
    QVERIFY(has(debug[0], "operation=\"GetFilterEnabled\""));
    QVERIFY(has(debug[0], "diagnostic_backend=\"runtime-fixture\""));
    QVERIFY(has(debug[1], "operation=\"GetOverlayFlags\""));
    QVERIFY(has(debug[1], "reason=\"second-original-failure\""));
    QVERIFY(has(debug[1], "diagnostic_backend=\"CSP\""));
    QVERIFY(has(debug[1], "diagnostic_operation=\"SecondProviderRead\""));
#else
    QVERIFY(debug.empty());
#endif
    for (const auto& line : recentLogLines()) QVERIFY(!has(line, "ConsoleCommand"));
    compareFailure(*result.failure, first);
    compareFailure(result.failures[0].failure, first);
    compareFailure(result.failures[1].failure, second);
  }

  void primaryFailureWithoutReadEntriesRetainsOnlyItsRealDiagnosticSource() {
    const auto original = providerFailure(compat::Effect::None, true);
    app::ConsoleCommandResult result;
    result.failure = original;
    result.exitCode = 5;
    console::logConsoleResult(result);
    QCOMPARE(commandLogs('E').size(), std::size_t{1});
    const auto debug = commandLogs('D');
#if defined(UWF_DEBUG_LOGGING)
    QCOMPARE(debug.size(), std::size_t{1});
    QVERIFY(!has(debug.front(), " operation="));
    QVERIFY(has(debug.front(), "diagnostic_backend=\"runtime-fixture\""));
    QVERIFY(has(debug.front(), "diagnostic_operation=\"ActualProviderRead\""));
    QVERIFY(has(debug.front(), "code=0x80070005"));
#else
    QVERIFY(debug.empty());
#endif
    compareFailure(*result.failure, original);
    QVERIFY(result.failures.empty());
  }

  void streamFailuresKeepTheirRealOperationWithoutInventingBackendCodes_data() {
    QTest::addColumn<QByteArray>("operation");
    QTest::newRow("stdout") << QByteArray("WriteStandardOutput");
    QTest::newRow("stderr") << QByteArray("WriteStandardError");
  }

  void streamFailuresKeepTheirRealOperationWithoutInventingBackendCodes() {
    QFETCH(QByteArray, operation);
    const auto detail = QStringLiteral("io-failure-sentinel \"closed pipe\"\n原始输出失败").toUtf8().toStdString();
    console::logConsoleOutputFailure(operation.toStdString(), detail);
    const auto errors = commandLogs('E');
    QCOMPARE(errors.size(), std::size_t{1});
    const auto& line = errors.front();
    QVERIFY(has(line, "event=output-failed operation=\"" + operation.toStdString() + '"'));
    QVERIFY(has(line, "detail=\"io-failure-sentinel \\\"closed pipe\\\"\\n"));
    QVERIFY(has(line, QStringLiteral("原始输出失败").toUtf8().toStdString()));
    QVERIFY(line.find_first_of("\r\n") == std::string::npos);
    QVERIFY(!has(line, "backend="));
    QVERIFY(!has(line, "code="));
    QVERIFY(commandLogs('D').empty());
  }
};
}  // namespace
QTEST_APPLESS_MAIN(ConsoleDiagnosticsTests)
#include "ConsoleDiagnosticsTests.moc"
