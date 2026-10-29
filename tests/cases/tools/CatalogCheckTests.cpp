// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QBuffer>
#include <QTest>
#include <stdexcept>

#include "tools/catalogs/CatalogCheck.h"

namespace {
using namespace uwf::test::tools;

TranslationCatalog parse(const QByteArray& bytes) {
  QBuffer buffer;
  buffer.setData(bytes);
  if (!buffer.open(QIODevice::ReadOnly)) throw std::runtime_error("cannot open test input");
  return readCatalog(buffer);
}

class CatalogCheckTests final : public QObject {
  Q_OBJECT

 private slots:
  void rejectsInvalidDocuments_data() {
    QTest::addColumn<QByteArray>("xml");
    QTest::newRow("wrong-document") << QByteArray("<root/>");
    QTest::newRow("empty-catalog") << QByteArray("<TS/>");
    QTest::newRow("truncated") << QByteArray("<TS><context>");
    QTest::newRow("missing-source") << QByteArray("<TS><context><name>C</name><message><translation>x</translation></message></context></TS>");
    QTest::newRow("duplicate-key") << QByteArray(
        "<TS><context><name>C</name><message><source>x</source></message><message><source>x</source></message></context></TS>");
  }
  void rejectsInvalidDocuments() {
    QFETCH(QByteArray, xml);
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, parse(xml));
  }
  void preservesContextAndDisambiguation() {
    const auto catalog = parse(
        "<TS language='de'><context><name>A</name><message><source>X</source><comment>first</comment><translation>Eins</translation></message>"
        "<message><source>X</source><comment>second</comment><translation>Zwei</translation></message></context>"
        "<context><name>B</name><message><source>X</source><translation>Drei</translation></message></context></TS>");
    QCOMPARE(catalog.messages.size(), std::size_t{3});
    QCOMPARE(catalog.messages.at({QStringLiteral("A"), QStringLiteral("X"), QStringLiteral("second")}).forms, QStringList{QStringLiteral("Zwei")});
  }
  void validatesEachPluralForm() {
    auto catalog = parse(
        "<TS language='de'><context><name>C</name><message numerus='yes'><source>%n file</source>"
        "<translation><numerusform>%n Datei</numerusform><numerusform>%n Dateien</numerusform></translation></message></context></TS>");
    const auto key = catalog.messages.begin()->first;
    auto message = catalog.messages.begin()->second;
    QVERIFY(checkMessage(key, message, QStringLiteral("de")).isEmpty());
    message.forms[1] = QStringLiteral("Dateien");
    QVERIFY(!checkMessage(key, message, QStringLiteral("de")).isEmpty());
  }
  void validatesTranslationSemantics_data() {
    QTest::addColumn<QString>("source");
    QTest::addColumn<QString>("translation");
    QTest::addColumn<bool>("valid");
    QTest::newRow("reordered-placeholders") << QStringLiteral("Value %1 %L2 %n %ProgramFiles%") << QStringLiteral("Wert %ProgramFiles% %n %L2 %1") << true;
    QTest::newRow("lost-placeholder") << QStringLiteral("Value %1 %1") << QStringLiteral("Wert %1") << false;
    QTest::newRow("changed-localized-placeholder") << QStringLiteral("Value %L1") << QStringLiteral("Wert %1") << false;
    QTest::newRow("unfinished-content") << QStringLiteral("Open") << QString() << false;
    QTest::newRow("unreviewed-source") << QStringLiteral("Open") << QStringLiteral("Open") << false;
    QTest::newRow("reviewed-word") << QStringLiteral("OK") << QStringLiteral("OK") << true;
    QTest::newRow("same-html") << QStringLiteral("<p><span style='color:red'>Value</span><br></p>")
                               << QStringLiteral("<p><span style='color:red'>Wert</span><br/></p>") << true;
    QTest::newRow("changed-attribute") << QStringLiteral("<p><span style='color:red'>Value</span></p>")
                                       << QStringLiteral("<p><span style='color:blue'>Wert</span></p>") << false;
    QTest::newRow("attribute-order") << QStringLiteral("<p><span title='Value' style='color:red'>Value</span></p>")
                                     << QStringLiteral("<p><span style='color:red' title='Value'>Wert</span></p>") << true;
    QTest::newRow("translated-accessible-text") << QStringLiteral("<p><span title='Value %1'>Value</span></p>")
                                                << QStringLiteral("<p><span title='Wert %1'>Wert</span></p>") << true;
    QTest::newRow("lost-accessible-placeholder") << QStringLiteral("<p><span title='Value %1'>Value</span></p>")
                                                 << QStringLiteral("<p><span title='Wert'>Wert</span></p>") << false;
    QTest::newRow("unclosed-tag") << QStringLiteral("<p><b>Value</b></p>") << QStringLiteral("<p><b>Wert</p>") << false;
    QTest::newRow("implicit-paragraph-end") << QStringLiteral("<b>Value</b><p>More") << QStringLiteral("<b>Wert</b><p>Mehr") << true;
    QTest::newRow("translated-filter") << QStringLiteral("Text files (*.txt);;All files (*)") << QStringLiteral("Textdateien (*.txt);;Alle Dateien (*)")
                                       << true;
    QTest::newRow("changed-filter") << QStringLiteral("Text files (*.txt)") << QStringLiteral("Textdateien (*.log)") << false;
    QTest::newRow("translated-command-filter") << QStringLiteral("Command files (*.cmd);;All files (*)")
                                               << QStringLiteral("Befehlsdateien (*.cmd);;Alle Dateien (*)") << true;
    QTest::newRow("changed-command-filter") << QStringLiteral("Command files (*.cmd);;All files (*)")
                                            << QStringLiteral("Befehlsdateien (*.txt);;Alle Dateien (*)") << false;
    QTest::newRow("translated-filter-description") << QStringLiteral("Text files (UTF-8) (*.txt *.log);;All files (*)")
                                                   << QStringLiteral("Textdateien (Unicode) (*.log  *.txt);;Alle Dateien (*)") << true;
    QTest::newRow("lost-filter-group") << QStringLiteral("Text files (*.txt);;All files (*)") << QStringLiteral("Textdateien (*.txt)") << false;
    QTest::newRow("malformed-filter") << QStringLiteral("Text files (*.txt)") << QStringLiteral("Textdateien (*.txt") << false;
    QTest::newRow("changed-technical-path") << QStringLiteral("Use HKLM\\SYSTEM") << QStringLiteral("Nutze HKCU\\SYSTEM") << false;
  }
  void validatesTranslationSemantics() {
    QFETCH(QString, source);
    QFETCH(QString, translation);
    QFETCH(bool, valid);
    const TranslationMessage message{QString(), {translation}, false};
    QCOMPARE(checkMessage({QStringLiteral("C"), source, QString()}, message, QStringLiteral("de")).isEmpty(), valid);
  }
  void catalogIdentityAndStatus_data() {
    QTest::addColumn<QByteArray>("messages");
    QTest::addColumn<QString>("problem");
    const QByteArray open("<message><source>Open</source><translation>Öffnen</translation></message>");
    const QByteArray close("<message><source>Close</source><translation>Schließen</translation></message>");
    QTest::newRow("exact") << open + close << QString{};
    QTest::newRow("missing") << open << QStringLiteral("missing message");
    QTest::newRow("extra") << open + close + QByteArray("<message><source>Unused</source><translation>Unbenutzt</translation></message>")
                          << QStringLiteral("unexpected message");
    auto disambiguated = open;
    disambiguated.replace("<source>", "<comment>verb</comment><source>");
    QTest::newRow("disambiguation-is-identity") << disambiguated + close << QStringLiteral("missing message");
    for (const auto* status : {"unfinished", "vanished", "obsolete"}) {
      auto inactive = open;
      inactive.replace("<translation>", QByteArray("<translation type='") + status + "'>");
      QTest::newRow(status) << inactive + close << QStringLiteral("missing or unfinished translation");
    }
    auto plural = open;
    plural.replace("<message>", "<message numerus='yes'>");
    plural.replace("Öffnen", "<numerusform>Öffnen</numerusform>");
    QTest::newRow("numerus-is-contract") << plural + close << QStringLiteral("changed numerus");
  }
  void catalogIdentityAndStatus() {
    QFETCH(QByteArray, messages);
    QFETCH(QString, problem);
    const auto reference = parse("<TS><context><name>C</name><message><source>Open</source></message><message><source>Close</source></message></context></TS>");
    const auto catalog = parse("<TS language='de'><context><name>C</name>" + messages + "</context></TS>");
    for (const bool supplement : {false, true}) {
      const auto problems = checkCatalog(catalog, reference, QStringLiteral("de"), supplement);
      if (problem.isEmpty())
        QVERIFY2(problems.isEmpty(), qUtf8Printable(problems.join(u'\n')));
      else
        QVERIFY2(problems.join(u'\n').contains(problem), qUtf8Printable(problems.join(u'\n')));
    }
  }
};
}  // namespace

QTEST_APPLESS_MAIN(CatalogCheckTests)
#include "CatalogCheckTests.moc"
