// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QBuffer>
#include <QTest>
#include <stdexcept>

#include "tools/results/TestResults.h"

namespace {
using namespace uwf::test::tools;

TestResults parse(const QByteArray& body) {
  QBuffer buffer;
  buffer.setData("<TestCase name='Example'><TestFunction name='initTestCase'><Incident type='pass'/></TestFunction>" + body +
                 "<TestFunction name='cleanupTestCase'><Incident type='pass'/></TestFunction></TestCase>");
  if (!buffer.open(QIODevice::ReadOnly)) throw std::runtime_error("cannot open test input");
  return readTestResults(buffer);
}

class TestResultsTests final : public QObject {
  Q_OBJECT

 private slots:
  void recordsActualRowsAndExcludesLifecycleCounts_data() {
    QTest::addColumn<QString>("tag");
    QTest::newRow("ascii") << QStringLiteral("boundary");
    QTest::newRow("unicode") << QStringLiteral("测试");
  }
  void recordsActualRowsAndExcludesLifecycleCounts() {
    QFETCH(QString, tag);
    const auto results = parse(QStringLiteral("<TestFunction name='behavior'><Incident type='pass'><DataTag>%1</DataTag></Incident><Incident "
                                             "type='pass'><DataTag>normal</DataTag></Incident></TestFunction>").arg(tag).toUtf8());
    QCOMPARE(results.rows.size(), qsizetype{2});
    QCOMPARE(results.lifecycleRecords, qsizetype{2});
    QVERIFY(results.problems.isEmpty());
    const auto id = QStringLiteral("Example::behavior(%1)").arg(tag);
    QVERIFY(results.rows.contains(id));
    const QJsonArray expected{QJsonObject{{QStringLiteral("id"), id}}};
    QVERIFY(reconcileTestResults(results, expected).value(QStringLiteral("passed")).toBool());
  }
  void missingRowsCannotBeCompensatedByOtherPasses() {
    const auto results = parse("<TestFunction name='other'><Incident type='pass'/></TestFunction>");
    const QJsonArray expected{QJsonObject{{QStringLiteral("id"), QStringLiteral("Example::required()")}}};
    QVERIFY(!reconcileTestResults(results, expected).value(QStringLiteral("passed")).toBool());
  }
  void failuresSkipsDuplicatesAndEmptyExecutionCannotPass_data() {
    QTest::addColumn<QByteArray>("body");
    QTest::newRow("failure") << QByteArray("<TestFunction name='case'><Incident type='fail'/></TestFunction>");
    QTest::newRow("skip") << QByteArray("<TestFunction name='case'><Message type='skip'><DataTag>unavailable</DataTag></Message></TestFunction>");
    QTest::newRow("duplicate") << QByteArray("<TestFunction name='case'><Incident type='pass'/><Incident type='pass'/></TestFunction>");
    QTest::newRow("empty") << QByteArray();
    QTest::newRow("unfinished-function")
        << QByteArray("<TestFunction name='unfinished'/><TestFunction name='other'><Incident type='pass'/></TestFunction>");
  }
  void failuresSkipsDuplicatesAndEmptyExecutionCannotPass() {
    QFETCH(QByteArray, body);
    QVERIFY(!reconcileTestResults(parse(body), {}).value(QStringLiteral("passed")).toBool());
  }
  void supportsRenamingAndExplicitRemovalWithoutFreezingNames() {
    const auto results = parse("<TestFunction name='newBehavior'><Incident type='pass'/></TestFunction>");
    QJsonArray expected{
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Old::oldBehavior()")}, {QStringLiteral("target"), QStringLiteral("Example::newBehavior()")}},
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Old::implementationDetail()")},
                    {QStringLiteral("target"), QJsonValue::Null},
                    {QStringLiteral("reason"), QStringLiteral("No observable contract.")}}};
    QVERIFY(reconcileTestResults(results, expected).value(QStringLiteral("passed")).toBool());
    auto removal = expected.at(1).toObject();
    removal.remove(QStringLiteral("reason"));
    expected[1] = removal;
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, reconcileTestResults(results, expected));
  }
  void rejectsMalformedXml_data() {
    QTest::addColumn<QByteArray>("body");
    QTest::newRow("syntax") << QByteArray("<TestFunction");
    QTest::newRow("local-code-page") << QByteArray("<TestFunction name='case'><Incident type='pass'><DataTag>") + QByteArray::fromHex("b2e2cad4") +
                                          QByteArray("</DataTag></Incident></TestFunction>");
  }
  void rejectsMalformedXml() {
    QFETCH(QByteArray, body);
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, parse(body));
  }
  void rejectsInvalidTargetTypes() {
    const auto results = parse("<TestFunction name='behavior'><Incident type='pass'/></TestFunction>");
    const QJsonArray expected{QJsonObject{{QStringLiteral("id"), QStringLiteral("Example::behavior()")}, {QStringLiteral("target"), 1}}};
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, reconcileTestResults(results, expected));
  }
};
}  // namespace

QTEST_APPLESS_MAIN(TestResultsTests)
#include "TestResultsTests.moc"
