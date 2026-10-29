// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <string>

#include "shared/ByteFormat.h"
#include "shared/DriveLetter.h"
#include "shared/Log.h"
#include "shared/RegistryPath.h"
#include "shared/StringUtil.h"

namespace {

QString text(const std::string& value) { return QString::fromUtf8(value); }

class SharedValueTests final : public QObject {
  Q_OBJECT

 private slots:
  void stringAndDriveNormalization();
  void asciiAndDrivePropertiesCoverTheInputSpace();
  void byteFormattingUsesStableUnitBoundaries();
  void logBufferPreservesOrderAndOversizedTail();
};

void SharedValueTests::stringAndDriveNormalization() {
  QCOMPARE(text(uwf::regkey::normalize(R"( hklm\Software\Vendor\\ )")), QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Vendor"));

  QCOMPARE(text(uwf::trim(" \t UWF Manager\r\n")), QStringLiteral("UWF Manager"));
  QCOMPARE(QString::fromLatin1(uwf::toLowerAscii("AbC-\xE4")), QString::fromLatin1("abc-\xE4"));
  QCOMPARE(QString::fromLatin1(uwf::toUpperAscii("aBc-\xE4")), QString::fromLatin1("ABC-\xE4"));

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

}  // namespace

QTEST_APPLESS_MAIN(SharedValueTests)

#include "SharedValueTests.moc"
