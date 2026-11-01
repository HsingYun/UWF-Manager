// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QByteArray>
#include <QScopeGuard>
#include <QTest>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include "foundation/logging/Log.h"
#include "foundation/paths/DriveLetter.h"
#include "foundation/paths/RegistryPath.h"
#include "foundation/text/ByteFormat.h"
#include "foundation/text/StringUtil.h"
#include "foundation/text/Utf8.h"

namespace {

QString text(const std::string& value) { return QString::fromUtf8(value); }

class SharedValueTests final : public QObject {
  Q_OBJECT

 private slots:
  void utf16ValidityPreservesScalarSequences_data();
  void utf16ValidityPreservesScalarSequences();
  void utf8DecodingPreservesOnlyUnicodeScalars_data();
  void utf8DecodingPreservesOnlyUnicodeScalars();
  void utf8ByteBudgetsPreserveScalarsAndCompleteSuffixes_data();
  void utf8ByteBudgetsPreserveScalarsAndCompleteSuffixes();
  void stringAndDriveNormalization();
  void asciiAndDrivePropertiesCoverTheInputSpace();
  void byteFormattingUsesStableUnitBoundaries();
  void logBufferPreservesOrderAndOversizedTail();
  void logByteBudgetsPreserveUtf8_data();
  void logByteBudgetsPreserveUtf8();
};

void SharedValueTests::utf16ValidityPreservesScalarSequences_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<bool>("valid");
  QTest::newRow("empty") << QString{} << true;
  QTest::newRow("nul") << QString(QChar(0)) << true;
  QTest::newRow("bmp-before-surrogates") << QString(QChar(0xd7ff)) << true;
  QTest::newRow("bmp-after-surrogates") << QString(QChar(0xe000)) << true;
  QTest::newRow("maximum-bmp") << QString(QChar(0xffff)) << true;
  QTest::newRow("minimum-supplementary") << QString::fromUcs4(U"\U00010000") << true;
  QTest::newRow("maximum-supplementary") << QString::fromUcs4(U"\U0010ffff") << true;
  QTest::newRow("mixed") << QStringLiteral("A中\U0001F680Z") << true;
  for (const char16_t unit : {char16_t{0xd800}, char16_t{0xdbff}, char16_t{0xdc00}, char16_t{0xdfff}}) {
    const auto name = QString::number(unit, 16).toUtf8();
    QTest::newRow(name.constData()) << QString(QChar(unit)) << false;
    QTest::newRow((name + "-before-bmp").constData()) << (QString(QChar(unit)) + u'A') << false;
  }
  QTest::newRow("reversed-pair") << (QString(QChar(0xdc00)) + QChar(0xd800)) << false;
  QTest::newRow("high-before-pair") << (QString(QChar(0xd800)) + QString::fromUcs4(U"\U00010000")) << false;
}
void SharedValueTests::utf16ValidityPreservesScalarSequences() {
  QFETCH(QString, input);
  QFETCH(bool, valid);
  QCOMPARE(uwf::isValidUtf16(input.toStdU16String()), valid);
  if (valid) {
    const auto roundtrip = uwf::utf8ToUtf16(input.toUtf8().toStdString());
    QVERIFY(roundtrip.has_value());
    QCOMPARE(*roundtrip, input.toStdU16String());
  }
}

void SharedValueTests::utf8DecodingPreservesOnlyUnicodeScalars_data() {
  QTest::addColumn<QByteArray>("bytes");
  QTest::addColumn<QString>("expected");
  QTest::addColumn<bool>("valid");
  const auto valid = [](const char* label, const QByteArray& hex, const QString& expected) {
    QTest::newRow(label) << QByteArray::fromHex(hex) << expected << true;
  };
  valid("empty", "", {});
  valid("ascii-and-null", "4100427f", QStringLiteral("A") + QChar(0) + u'B' + QChar(0x7f));
  valid("two-byte-minimum", "c280", QString(QChar(0x80)));
  valid("two-byte-maximum", "dfbf", QString(QChar(0x7ff)));
  valid("three-byte-minimum", "e0a080", QString(QChar(0x800)));
  valid("before-surrogates", "ed9fbf", QString(QChar(0xd7ff)));
  valid("after-surrogates", "ee8080", QString(QChar(0xe000)));
  valid("bmp-maximum", "efbfbf", QString(QChar(0xffff)));
  valid("non-bmp-minimum", "f0908080", QString::fromUcs4(U"\U00010000"));
  valid("non-bmp-maximum", "f48fbfbf", QString::fromUcs4(U"\U0010ffff"));
  valid("mixed-scalars", "41e4b8adf09f998242", QStringLiteral("A中") + QString::fromUcs4(U"\U0001f642") + u'B');
  for (const auto& hex : {"80", "bf", "c0af", "c1bf", "c2", "e0a0", "f09080", "c241", "e228a1", "e080af", "f08080af", "eda080", "edbfbf", "f4908080",
                          "f5808080", "ff", "41c280e228a1"})
    QTest::newRow(hex) << QByteArray::fromHex(hex) << QString{} << false;
}
void SharedValueTests::utf8DecodingPreservesOnlyUnicodeScalars() {
  QFETCH(QByteArray, bytes);
  QFETCH(QString, expected);
  QFETCH(bool, valid);
  const auto result = uwf::utf8ToUtf16(std::string_view(bytes.constData(), static_cast<std::size_t>(bytes.size())));
  QCOMPARE(result.has_value(), valid);
  if (valid) QCOMPARE(*result, std::u16string(expected.utf16(), expected.utf16() + expected.size()));
}

void SharedValueTests::utf8ByteBudgetsPreserveScalarsAndCompleteSuffixes_data() {
  QTest::addColumn<QByteArray>("input");
  QTest::addColumn<QByteArray>("suffix");
  QTest::addColumn<qulonglong>("budget");
  QTest::addColumn<QByteArray>("expected");
  // Authored scalar boundaries form the oracle independently of byte scanning.
  const std::vector<std::vector<QByteArray>> samples{
      {},
      {"A"},
      {QByteArray::fromHex("c2a2")},
      {QByteArray::fromHex("e4b8ad")},
      {QByteArray::fromHex("f09f9982")},
      {"A", QByteArray::fromHex("c2a2"), QByteArray::fromHex("e4b8ad"), QByteArray::fromHex("f09f9982"), QByteArray(1, '\0'), "Z"},
  };
  const std::array<QByteArray, 5> suffixes{{{}, ".", QByteArray::fromHex("e280a6"), QByteArray::fromHex("f09f998221"), QByteArray(1, '\0')}};
  for (std::size_t sample = 0; sample < samples.size(); ++sample) {
    std::vector<QByteArray> prefixes(1);
    for (const auto& scalar : samples[sample]) prefixes.push_back(prefixes.back() + scalar);
    const auto& input = prefixes.back();
    for (std::size_t marker = 0; marker < suffixes.size(); ++marker) {
      const auto& suffix = suffixes[marker];
      for (qsizetype budget = 0; budget <= input.size() + suffix.size() + 1; ++budget) {
        QByteArray expected = input;
        if (input.size() > budget) {
          const auto retainedSuffix = suffix.size() <= budget ? suffix : QByteArray{};
          const auto prefixBudget = budget - retainedSuffix.size();
          expected.clear();
          for (const auto& prefix : prefixes)
            if (prefix.size() <= prefixBudget) expected = prefix;
          expected += retainedSuffix;
        }
        const auto label = QStringLiteral("sample-%1/suffix-%2/bytes-%3").arg(sample).arg(marker).arg(budget);
        QTest::newRow(qUtf8Printable(label)) << input << suffix << static_cast<qulonglong>(budget) << expected;
      }
    }
  }
  const auto mixed = QByteArray::fromHex("41c2a2e4b8adf09f9982005a");
  QTest::newRow("maximum-size-budget") << mixed << QByteArray::fromHex("e280a6") << static_cast<qulonglong>(std::numeric_limits<std::size_t>::max()) << mixed;
}

void SharedValueTests::utf8ByteBudgetsPreserveScalarsAndCompleteSuffixes() {
  QFETCH(QByteArray, input);
  QFETCH(QByteArray, suffix);
  QFETCH(qulonglong, budget);
  QFETCH(QByteArray, expected);
  const auto result = uwf::truncateUtf8(std::string_view(input.constData(), static_cast<std::size_t>(input.size())), static_cast<std::size_t>(budget),
                                        std::string_view(suffix.constData(), static_cast<std::size_t>(suffix.size())));
  QCOMPARE(QByteArray::fromStdString(result), expected);
  QVERIFY(result.size() <= budget);
  QVERIFY(uwf::utf8ToUtf16(result).has_value());
}

void SharedValueTests::stringAndDriveNormalization() {
  QCOMPARE(text(uwf::regkey::expandHive(R"(hklm\Software\..\Vendor \\)")), QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\..\\Vendor \\\\"));
  QCOMPARE(text(uwf::regkey::expandHive(R"( hklm\Software\Vendor\\ )")), QStringLiteral(" hklm\\Software\\Vendor\\\\ "));

  QCOMPARE(text(uwf::trim(" \t UWF Manager\r\n")), QStringLiteral("UWF Manager"));
  QCOMPARE(QString::fromLatin1(uwf::toLowerAscii("AbC-\xE4")), QString::fromLatin1("abc-\xE4"));
  QCOMPARE(QString::fromLatin1(uwf::toUpperAscii("aBc-\xE4")), QString::fromLatin1("ABC-\xE4"));
  QVERIFY(uwf::equalAsciiInsensitive("AbC-\xE4", "aBc-\xE4"));
  QVERIFY(!uwf::equalAsciiInsensitive("\xC3\x84", "\xC3\xA4"));
  QVERIFY(!uwf::equalAsciiInsensitive("abc", "abcd"));
  QVERIFY(uwf::equalAsciiInsensitive(std::string("A\0B", 3), std::string("a\0b", 3)));
  QCOMPARE(uwf::toLowerAscii(std::u16string(u"Entry\u00c4")), std::u16string(u"entry\u00c4"));
  QVERIFY(uwf::equalAsciiInsensitive(u"Entry\u00c4", u"eNTRY\u00c4"));
  QVERIFY(!uwf::equalAsciiInsensitive(u"\u00c4", u"\u00e4"));

  QCOMPARE(text(uwf::drive::normalize(" c ")), QStringLiteral("C:"));
  QCOMPARE(text(uwf::drive::normalize(R"(\\?\d:\Data\file.txt)")), QStringLiteral("D:"));
  QVERIFY(uwf::drive::normalize(R"(\\server\share\file.txt)").empty());
  QVERIFY(uwf::drive::normalize(R"(\\?\Volume{1234}\file.txt)").empty());

  const auto absolute = uwf::drive::split(R"(\\?\e:\Users\name)");
  QCOMPARE(text(absolute.letter), QStringLiteral("E:"));
  QCOMPARE(text(absolute.rest), QStringLiteral("\\Users\\name"));

  const auto relative = uwf::drive::split(R"(\Users\name)");
  QVERIFY(relative.letter.empty());
  QCOMPARE(text(relative.rest), QStringLiteral("\\Users\\name"));
}

void SharedValueTests::asciiAndDrivePropertiesCoverTheInputSpace() {
  std::string bytes;
  bytes.reserve(256);
  for (int value = 0; value <= 255; ++value) bytes.push_back(static_cast<char>(value));

  const std::string lower = uwf::toLowerAscii(bytes);
  const std::string upper = uwf::toUpperAscii(bytes);
  QCOMPARE(lower.size(), bytes.size());
  QCOMPARE(upper.size(), bytes.size());
  for (int value = 0; value <= 255; ++value) {
    const auto byte = static_cast<unsigned char>(value);
    const auto expectedLower = byte >= 'A' && byte <= 'Z' ? static_cast<unsigned char>(byte - 'A' + 'a') : byte;
    const auto expectedUpper = byte >= 'a' && byte <= 'z' ? static_cast<unsigned char>(byte - 'a' + 'A') : byte;
    QCOMPARE(static_cast<unsigned char>(lower[static_cast<std::size_t>(value)]), expectedLower);
    QCOMPARE(static_cast<unsigned char>(upper[static_cast<std::size_t>(value)]), expectedUpper);
    QVERIFY(uwf::equalAsciiInsensitive(std::string(1, static_cast<char>(byte)), std::string(1, static_cast<char>(expectedLower))));
    QVERIFY(uwf::equalAsciiInsensitive(std::string(1, static_cast<char>(byte)), std::string(1, static_cast<char>(expectedUpper))));
  }
  QCOMPARE(uwf::trim("\t\n\v\f\r value \t\n\v\f\r"), std::string("value"));
  QCOMPARE(uwf::trim(std::string("\xA0value\xA0", 7)), std::string("\xA0value\xA0", 7));

  for (char letter = 'A'; letter <= 'Z'; ++letter) {
    const char lowerLetter = static_cast<char>(letter - 'A' + 'a');
    const std::string normalized{letter, ':'};
    QCOMPARE(uwf::drive::normalize(std::string(1, letter)), normalized);
    QCOMPARE(uwf::drive::normalize(std::string(1, lowerLetter) + ":\\child"), normalized);
    const auto split = uwf::drive::split(std::string(R"(\\?\)") + lowerLetter + R"(:\child)");
    QCOMPARE(split.letter, normalized);
    QCOMPARE(split.rest, std::string(R"(\child)"));
  }

  const std::array invalid = {std::string{},
                              std::string("1:"),
                              std::string("_:"),
                              std::string("relative"),
                              std::string(R"(\\server\share)"),
                              std::string(R"(\\?\UNC\server\share)"),
                              std::string(R"(\\?\Volume{1234}\path)")};
  for (const auto& value : invalid) QVERIFY2(uwf::drive::normalize(value).empty(), value.c_str());
}

void SharedValueTests::byteFormattingUsesStableUnitBoundaries() {
  QCOMPARE(text(uwf::formatBytes(0)), QStringLiteral("0 B"));
  QCOMPARE(text(uwf::formatBytes(1023)), QStringLiteral("1023 B"));
  QCOMPARE(text(uwf::formatBytes(1024)), QStringLiteral("1.0 KB"));
  QCOMPARE(text(uwf::formatBytes(3ULL * 1024 * 1024 + 512ULL * 1024)), QStringLiteral("3.5 MB"));
  QCOMPARE(text(uwf::formatBytes(5ULL * 1024 * 1024 * 1024)), QStringLiteral("5.00 GB"));

  QCOMPARE(text(uwf::formatMb(0)), QStringLiteral("0 MB"));
  QCOMPARE(text(uwf::formatMb(1024)), QStringLiteral("1 GB"));
  QCOMPARE(text(uwf::formatMb(1536)), QStringLiteral("1.50 GB"));
  QCOMPARE(text(uwf::formatMb(2U * 1024 * 1024)), QStringLiteral("2 TB"));
  QCOMPARE(text(uwf::formatBytes(1024ULL * 1024 * 1024 * 1024)), QStringLiteral("1.00 TB"));
  QCOMPARE(text(uwf::formatMb(1024U * 1024 * 1024)), QStringLiteral("1 PB"));
  QVERIFY(text(uwf::formatBytes(std::numeric_limits<uint64_t>::max())).endsWith(QStringLiteral(" TB")));
}

void SharedValueTests::logBufferPreservesOrderAndOversizedTail() {
  uwf::clearLogLines();
  uwf::logLine('I', "core-test", "first");
  uwf::logLine('W', "core-test", "second");
  auto lines = uwf::recentLogLines();
  QCOMPARE(lines.size(), std::size_t{2});
  QVERIFY(lines[0].ends_with("first"));
  QVERIFY(lines[1].ends_with("second"));

  const std::string oversized(std::size_t{11} * 1024 * 1024, 'x');
  uwf::logLine('E', "oversized", oversized);
  lines = uwf::recentLogLines();
  QCOMPARE(lines.size(), std::size_t{1});
  QVERIFY(lines.front().ends_with(oversized));

  auto bounded = uwf::recentLogLines(64, 1024);
  QCOMPARE(bounded.size(), std::size_t{1});
  QVERIFY(bounded.front().size() <= std::size_t{1024});
  QVERIFY(bounded.front().ends_with("... <truncated>"));

  uwf::clearLogLines();
  for (int index = 0; index < 70; ++index) {
    const std::string message = "bounded-entry-" + std::to_string(index) + "-" + std::string(256, 'x');
    uwf::logLine('I', "bounded", message);
  }
  bounded = uwf::recentLogLines(64, 80);
  QCOMPARE(bounded.size(), std::size_t{64});
  QVERIFY(bounded.front().find("bounded-entry-6-") != std::string::npos);
  QVERIFY(bounded.back().find("bounded-entry-69-") != std::string::npos);
  QVERIFY(std::ranges::all_of(bounded, [](const std::string& line) { return line.size() <= std::size_t{80}; }));
  QVERIFY(std::ranges::all_of(bounded, [](const std::string& line) { return line.ends_with("... <truncated>"); }));

  uwf::clearLogLines();
  QVERIFY(uwf::recentLogLines().empty());
}

void SharedValueTests::logByteBudgetsPreserveUtf8_data() {
  QTest::addColumn<QByteArray>("message");
  QTest::newRow("ascii") << QByteArray(1800, 'a');
  QTest::newRow("two-byte") << QString(900, QChar(0xa2)).toUtf8();
  QTest::newRow("three-byte") << QString(600, QChar(u'中')).toUtf8();
  QTest::newRow("four-byte") << QString::fromUcs4(U"\U0001f642").repeated(450).toUtf8();
  QTest::newRow("mixed-widths-and-null") << (QStringLiteral("A¢中") + QString::fromUcs4(U"\U0001f642") + QChar(0)).repeated(160).toUtf8();
}

void SharedValueTests::logByteBudgetsPreserveUtf8() {
  QFETCH(QByteArray, message);
  uwf::clearLogLines();
  const auto clearOnExit = qScopeGuard([] { uwf::clearLogLines(); });
  uwf::logLine('I', "utf8-budget", std::string_view(message.constData(), static_cast<std::size_t>(message.size())));
  const auto original = uwf::recentLogLines();
  QCOMPARE(original.size(), std::size_t{1});
  const auto& full = original.front();
  constexpr std::string_view suffix = "... <truncated>";
  // Exercise every byte boundary, including budgets too small for the marker
  // and the diagnostic report's 1024-byte budget, against the real log prefix.
  for (std::size_t budget = 0; budget <= full.size() + 1; ++budget) {
    const auto lines = uwf::recentLogLines(1, budget);
    QCOMPARE(lines.size(), std::size_t{1});
    const auto& bounded = lines.front();
    QVERIFY2(bounded.size() <= budget, qUtf8Printable(QString::number(budget)));
    QVERIFY2(uwf::utf8ToUtf16(bounded).has_value(), qUtf8Printable(QString::number(budget)));
    if (full.size() <= budget) {
      QCOMPARE(bounded, full);
    } else {
      std::string_view prefix(bounded);
      if (suffix.size() <= budget) {
        QVERIFY(bounded.ends_with(suffix));
        prefix.remove_suffix(suffix.size());
      }
      QVERIFY(full.starts_with(prefix));
    }
  }
  QCOMPARE(uwf::recentLogLines(), original);
  QVERIFY(uwf::recentLogLines(0, 1024).empty());
}

}  // namespace

QTEST_APPLESS_MAIN(SharedValueTests)

#include "SharedValueTests.moc"
