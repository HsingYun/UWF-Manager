// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QByteArray>
#include <QTest>
#include <stdexcept>
#include <string>

#include "application/staging/editing/FileStagingPath.h"
#include "infrastructure/windows/staging/RegistryFileStagingCodec.h"

namespace {
class RegistryFileStagingCodecTests final : public QObject {
  Q_OBJECT
 private slots:
  void fileStagingRegistryCodecPreservesKindsAndRejectsCorruption();
  void extendedDrivePathsRoundTripWithoutMountObservations() {
    using namespace uwf::app;
    using namespace uwf::windows;
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral(R"(\\?\C:\数据\state.bin)")},
                                          {FileStagingKind::Directory, QStringLiteral(R"(\\?\Z:\Unavailable\Cache)")}};
    const std::vector<std::string> expected{R"(F|\\?\C:\数据\state.bin)", R"(D|\\?\Z:\Unavailable\Cache)"};
    QCOMPARE(encodeFileStagingEntries(entries), expected);
    QCOMPARE(decodeFileStagingEntries(expected), entries);
    for (const auto& entry : entries) QVERIFY(!entry.fileTarget);
  }
  void storedVolumeGuidPathsRoundTripWithoutLosingTheirAddress() {
    using namespace uwf::app;
    using namespace uwf::windows;
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral(R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\数据\state.bin)")},
                                          {FileStagingKind::Directory, QStringLiteral(R"(\\?\Volume{66666666-7777-8888-9999-AAAAAAAAAAAA}\Cache)")}};
    QCOMPARE(decodeFileStagingEntries(encodeFileStagingEntries(entries)), entries);
    for (const auto& entry : entries) QVERIFY(isAbsoluteLocalFileStagingPath(entry.path));
    for (const auto* path : {R"(\\?\Volume{bad}\file)", R"(\\?\Volume{11111111-2222-3333-4444-555555555555}file)",
                             R"(\\?\Volume{11111111-2222-3333-4444-555555555555})", R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\\file)"}) {
      QVERIFY(!isAbsoluteLocalFileStagingPath(QString::fromLatin1(path)));
      QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(decodeFileStagingEntries({std::string("F|") + path})));
    }
  }
  void selectedObjectObservationsDoNotChangeTheStoredFormat() {
    using namespace uwf::app;
    using namespace uwf::windows;
    const FileStagingEntry entry{FileStagingKind::File, QStringLiteral("C:\\MountedD\\state.bin"),
                                 FileCommitTarget{"11111111-2222-3333-4444-555555555555", QStringLiteral("\\state.bin")}};
    const auto encoded = encodeFileStagingEntries({entry});
    QCOMPARE(encoded, std::vector<std::string>{"F|C:\\MountedD\\state.bin"});
    const auto decoded = decodeFileStagingEntries(encoded);
    QCOMPARE(decoded, QList<FileStagingEntry>{entry});
    QVERIFY(!decoded.front().fileTarget);
  }
  void stagingCodecPreservesUnicodeWithoutRegistryIo() {
    using namespace uwf::app;
    using namespace uwf::windows;
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral("C:\\数据\\\U0001F680.bin")},
                                          {FileStagingKind::Directory, QStringLiteral("D:\\缓存\\\U0001F4BE")}};
    QCOMPARE(decodeFileStagingEntries(encodeFileStagingEntries(entries)), entries);
  }
  void stagingPathAdmissionAndCodecPreserveUnicodeIdentity_data() {
    QTest::addColumn<QString>("suffix");
    QTest::addColumn<bool>("representable");
    QTest::newRow("bmp") << QStringLiteral("数据") << true;
    QTest::newRow("supplementary-pair") << QStringLiteral("\U0001F680") << true;
    QTest::newRow("literal-replacement-character") << QString(QChar(0xfffd)) << true;
    QTest::newRow("orphan-high") << QString(QChar(0xd800)) << false;
    QTest::newRow("orphan-low") << QString(QChar(0xdc00)) << false;
    QTest::newRow("high-before-bmp") << (QString(QChar(0xd800)) + u'A') << false;
    QTest::newRow("reversed-pair") << (QString(QChar(0xdc00)) + QChar(0xd800)) << false;
    QTest::newRow("repeated-high") << (QString(QChar(0xd800)) + QChar(0xd800) + QChar(0xdc00)) << false;
  }
  void stagingPathAdmissionAndCodecPreserveUnicodeIdentity() {
    QFETCH(QString, suffix);
    QFETCH(bool, representable);
    using namespace uwf::app;
    using namespace uwf::windows;
    for (const auto kind : {FileStagingKind::File, FileStagingKind::Directory})
      for (const auto& prefix : {QStringLiteral("C:\\RoundTrip-"), QStringLiteral(R"(\\?\C:\RoundTrip-)"),
                                 QStringLiteral(R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\RoundTrip-)")}) {
        const QList<FileStagingEntry> entries{{kind, prefix + suffix}};
        QCOMPARE(isAbsoluteLocalFileStagingPath(entries.front().path), representable);
        if (representable)
          QCOMPARE(decodeFileStagingEntries(encodeFileStagingEntries(entries)), entries);
        else
          QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(encodeFileStagingEntries(entries)));
      }
  }
  void stagingCodecRejectsMalformedUtf8_data() {
    QTest::addColumn<QByteArray>("bytes");
    QTest::newRow("truncated") << QByteArray::fromHex("e4b8");
    QTest::newRow("invalid-continuation") << QByteArray::fromHex("e441ad");
    QTest::newRow("stray-continuation") << QByteArray::fromHex("80");
    QTest::newRow("overlong") << QByteArray::fromHex("c0af");
    QTest::newRow("encoded-surrogate") << QByteArray::fromHex("eda080");
    QTest::newRow("out-of-range") << QByteArray::fromHex("f4908080");
  }
  void stagingCodecRejectsMalformedUtf8() {
    QFETCH(QByteArray, bytes);
    auto record = std::string("F|C:\\state-");
    record.append(bytes.constData(), static_cast<std::size_t>(bytes.size()));
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::windows::decodeFileStagingEntries({record})));
  }
  void stagingCodecDoesNotDiscardAnInitialBom() {
    const auto record = std::string("F|\xef\xbb\xbf") + "C:\\state.bin";
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::windows::decodeFileStagingEntries({record})));
  }
};
void RegistryFileStagingCodecTests::fileStagingRegistryCodecPreservesKindsAndRejectsCorruption() {
  const QList<uwf::app::FileStagingEntry> entries{{uwf::app::FileStagingKind::File, QStringLiteral("C:\\数据|state.bin")},
                                                  {uwf::app::FileStagingKind::Directory, QStringLiteral("D:\\Vendor\\Cache")}};
  const auto encoded = uwf::windows::encodeFileStagingEntries(entries);
  QCOMPARE(encoded, std::vector<std::string>({"F|C:\\数据|state.bin", "D|D:\\Vendor\\Cache"}));
  QCOMPARE(uwf::windows::decodeFileStagingEntries(encoded), entries);
  QVERIFY(uwf::windows::decodeFileStagingEntries({}).isEmpty());

  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::windows::decodeFileStagingEntries({"X|C:\\unknown"})));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::windows::decodeFileStagingEntries({"F|"})));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::windows::decodeFileStagingEntries({"F|C:relative"})));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::windows::decodeFileStagingEntries({R"(D|\\server\share\cache)"})));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::windows::decodeFileStagingEntries({R"(F|\\.\C:\device)"})));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::windows::decodeFileStagingEntries({R"(F|\\?\C:relative)"})));
  std::string encodedNul = "F|C:\\state";
  encodedNul.push_back('\0');
  encodedNul += "hidden";
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::windows::decodeFileStagingEntries({encodedNul})));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::windows::encodeFileStagingEntries({{uwf::app::FileStagingKind::File, {}}})));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument,
                           static_cast<void>(uwf::windows::encodeFileStagingEntries({{uwf::app::FileStagingKind::File, QStringLiteral("C:relative")}})));
  QString nulPath = QStringLiteral("C:\\state");
  nulPath.append(QChar(u'\0'));
  const QList<uwf::app::FileStagingEntry> nulEntry{{uwf::app::FileStagingKind::File, nulPath}};
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::windows::encodeFileStagingEntries(nulEntry)));
  const QList<uwf::app::FileStagingEntry> invalidKind{{static_cast<uwf::app::FileStagingKind>(99), QStringLiteral("C:\\state")}};
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::windows::encodeFileStagingEntries(invalidKind)));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::windows::decodeFileStagingEntries({std::string("F|\xC3", 3)})));
}
}  // namespace
QTEST_APPLESS_MAIN(RegistryFileStagingCodecTests)
#include "RegistryFileStagingCodecTests.moc"
