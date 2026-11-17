// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QBuffer>
#include <QTest>
#include <array>
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
    QTest::newRow("encoding-loss-question-marks") << QStringLiteral("Changes cannot be committed") << QStringLiteral("???? HORM ?????") << false;
    QTest::newRow("encoding-loss-accent") << QStringLiteral("Open") << QStringLiteral("?ffnen") << false;
    QTest::newRow("encoding-loss-replacement") << QStringLiteral("Open") << QString(QChar::ReplacementCharacter) + QStringLiteral("ffnen") << false;
    QTest::newRow("legitimate-question") << QStringLiteral("Continue?") << QStringLiteral("Fortfahren?") << true;
    QTest::newRow("changed-localized-placeholder") << QStringLiteral("Value %L1") << QStringLiteral("Wert %1") << false;
    QTest::newRow("unfinished-content") << QStringLiteral("Open") << QString() << false;
    QTest::newRow("unreviewed-source") << QStringLiteral("Open") << QStringLiteral("Open") << false;
    QTest::newRow("reviewed-word") << QStringLiteral("OK") << QStringLiteral("OK") << true;
    QTest::newRow("reviewed-partition-name") << QStringLiteral("Partition") << QStringLiteral("Partition") << true;
    QTest::newRow("reviewed-numbered-partition") << QStringLiteral("Partition %1") << QStringLiteral("Partition %1") << true;
    QTest::newRow("protocol-identifier") << QStringLiteral("HORM") << QStringLiteral("HORM") << true;
    QTest::newRow("capacity-format") << QStringLiteral("%1 MB") << QStringLiteral("%1 MB") << true;
    QTest::newRow("placeholder-composition") << QStringLiteral("%1 → %2") << QStringLiteral("%1 → %2") << true;
    QTest::newRow("placeholder-punctuation") << QStringLiteral("(%2) / %1 · %3") << QStringLiteral("(%2) / %1 · %3") << true;
    QTest::newRow("overlapping-placeholder-numbers") << QStringLiteral("%1 → %10") << QStringLiteral("%1 → %10") << true;
    QTest::newRow("word-inside-composition") << QStringLiteral("%1 → Disk") << QStringLiteral("%1 → Disk") << false;
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
  }
  void validatesTranslationSemantics() {
    QFETCH(QString, source);
    QFETCH(QString, translation);
    QFETCH(bool, valid);
    const TranslationMessage message{QString(), {translation}, false};
    QCOMPARE(checkMessage({QStringLiteral("C"), source, QString()}, message, QStringLiteral("de")).isEmpty(), valid);
  }
  void placeholderCompositionsRespectUnicodeWordBoundaries_data() {
    QTest::addColumn<quint32>("codepoint");
    QTest::addColumn<bool>("word");
    struct Character {
      const char* name;
      char32_t codepoint;
      bool word;
    };
    constexpr std::array characters{Character{"latin-letter", U'A', true},
                                    Character{"bmp-han-letter", U'\u4E00', true},
                                    Character{"bmp-arabic-digit", U'\u0660', true},
                                    Character{"supplementary-han-letter", U'\U00020000', true},
                                    Character{"supplementary-math-letter", U'\U0001D400', true},
                                    Character{"supplementary-math-digit", U'\U0001D7CE', true},
                                    Character{"arrow", U'\u2192', false},
                                    Character{"punctuation", U'\u2014', false},
                                    Character{"supplementary-emoji", U'\U0001F642', false},
                                    Character{"supplementary-music-symbol", U'\U0001D11E', false}};
    for (const auto& character : characters) QTest::newRow(character.name) << static_cast<quint32>(character.codepoint) << character.word;
  }
  void placeholderCompositionsRespectUnicodeWordBoundaries() {
    QFETCH(quint32, codepoint);
    QFETCH(bool, word);
    const auto character = static_cast<char32_t>(codepoint);
    const auto glyph = QString::fromUcs4(&character, 1);
    const std::array compositions{glyph, QStringLiteral("%1 → ") + glyph + QStringLiteral(" / %10"), glyph + QStringLiteral(" · %L1")};
    for (const auto& source : compositions) {
      const TranslationMessage message{QString(), {source}, false};
      const auto problems = checkMessage({QStringLiteral("C"), source, QString()}, message, QStringLiteral("de"));
      QCOMPARE(problems.isEmpty(), !word);
      if (word) QVERIFY(problems.contains(QStringLiteral("unreviewed untranslated source")));
    }
  }
  void richTextPreservesScopeWithoutConstrainingWordOrder_data() {
    QTest::addColumn<QString>("source");
    QTest::addColumn<QString>("translation");
    QTest::addColumn<bool>("valid");
    QTest::newRow("nested-link-scope") << QStringLiteral("<a href='x'><b>X</b> Y</a>") << QStringLiteral("<a href='x'>Wert</a><b>Andere</b>") << false;
    QTest::newRow("formatting-parent-changed") << QStringLiteral("<p><b>X</b><i>Y</i></p>") << QStringLiteral("<p><b><i>Wert</i></b>Andere</p>") << false;
    QTest::newRow("placeholder-moved-out-of-link") << QStringLiteral("<p><a href='x'>Value %1</a> More</p>")
                                                   << QStringLiteral("<p><a href='x'>Wert</a> Mehr %1</p>") << false;
    QTest::newRow("placeholder-moved-between-links") << QStringLiteral("<p><a href='x'>Value %1</a><a href='y'>Other %2</a></p>")
                                                     << QStringLiteral("<p><a href='x'>Wert %2</a><a href='y'>Andere %1</a></p>") << false;
    QTest::newRow("placeholder-moved-from-title") << QStringLiteral("<p><span title='Value %1'>Value</span></p>")
                                                  << QStringLiteral("<p><span title='Wert'>Wert %1</span></p>") << false;
    QTest::newRow("empty-link") << QStringLiteral("<p><a href='x'>Value</a> More</p>") << QStringLiteral("<p><a href='x'></a> Wert Mehr</p>") << false;
    QTest::newRow("markup-added-to-plain-text") << QStringLiteral("Value %1") << QStringLiteral("<a href='x'>Wert %1</a>") << false;
    QTest::newRow("translated-link-text") << QStringLiteral("<a href='x'>Value %1</a>") << QStringLiteral("<a href='x'>Wert %1</a>") << true;
    QTest::newRow("reordered-inline-siblings") << QStringLiteral("<p><b>First %1</b> then <i>Second %2</i></p>")
                                               << QStringLiteral("<p><i>Zweite %2</i> und <b>Erste %1</b></p>") << true;
    QTest::newRow("reordered-links-retain-their-placeholders") << QStringLiteral("<p><a href='x'>Value %1</a><a href='y'>Other %2</a></p>")
                                                               << QStringLiteral("<p><a href='y'>Andere %2</a><a href='x'>Wert %1</a></p>") << true;
    QTest::newRow("sentence-split-around-inline-node")
        << QStringLiteral("<p><b>Value</b> remains</p>") << QStringLiteral("<p>Der <b>Wert</b> bleibt</p>") << true;
    QTest::newRow("attribute-order-and-accessible-text") << QStringLiteral("<p><span title='Value %1' style='color:red'>Value</span></p>")
                                                         << QStringLiteral("<p><span style='color:red' title='Wert %1'>Wert</span></p>") << true;
    QTest::newRow("paragraph-order") << QStringLiteral("<p><b>First %1</b></p><p><i>Second %2</i></p>")
                                     << QStringLiteral("<p><i>Zweite %2</i></p><p><b>Erste %1</b></p>") << true;
    QTest::newRow("unclosed-link") << QStringLiteral("<p><a href='x'>Value</a></p>") << QStringLiteral("<p><a href='x'>Wert</p>") << false;
  }
  void richTextPreservesScopeWithoutConstrainingWordOrder() {
    QFETCH(QString, source);
    QFETCH(QString, translation);
    QFETCH(bool, valid);
    const TranslationMessage message{QString(), {translation}, false};
    const auto problems = checkMessage({QStringLiteral("C"), source, QString()}, message, QStringLiteral("de"));
    QCOMPARE(problems.isEmpty(), valid);
  }
  void technicalLiteralsStayUnchangedInsideTranslatedMessages_data() {
    QTest::addColumn<QString>("literal");
    QTest::addColumn<QString>("changed");
    QTest::newRow("registry-path") << QStringLiteral("HKLM\\SYSTEM") << QStringLiteral("HKCU\\SYSTEM");
    QTest::newRow("executable-path") << QStringLiteral("%ProgramFiles%\\UWF Manager\\Service\\UWF.exe")
                                     << QStringLiteral("%ProgramFiles%\\UWF Manager\\Service\\Other.exe");
    QTest::newRow("setting-identifier") << QStringLiteral("DomainSecretKey") << QStringLiteral("DomainSecret");
    QTest::newRow("data-file") << QStringLiteral("NTUSER.DAT") << QStringLiteral("NTUSER.LOG");
  }
  void technicalLiteralsStayUnchangedInsideTranslatedMessages() {
    QFETCH(QString, literal);
    QFETCH(QString, changed);
    const MessageKey key{QStringLiteral("C"), QStringLiteral("Use ") + literal, QString{}};
    for (const bool preserved : {true, false}) {
      const TranslationMessage message{QString{}, {QStringLiteral("Verwende ") + (preserved ? literal : changed)}, false};
      const auto problems = checkMessage(key, message, QStringLiteral("de"));
      if (preserved)
        QVERIFY2(problems.isEmpty(), qUtf8Printable(problems.join(u'\n')));
      else
        QVERIFY2(problems.join(u'\n').contains(QStringLiteral("modified technical literal")), qUtf8Printable(problems.join(u'\n')));
    }
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
