// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <optional>
#include <stdexcept>
#include <vector>

#include "app/staging/FileStagingCodec.h"
#include "app/staging/FileStagingConflictPolicy.h"

namespace {

class FileStagingPolicyTests final : public QObject {
  Q_OBJECT

 private slots:
  void fileStagingRegistryCodecPreservesKindsAndRejectsCorruption();
  void fileStagingConflictsRespectCaseAndPathSegments();
  void fileStagingConflictsUseNormalizedCandidates_data();
  void fileStagingConflictsUseNormalizedCandidates();
};

void FileStagingPolicyTests::fileStagingRegistryCodecPreservesKindsAndRejectsCorruption() {
  const QList<uwf::app::FileStagingEntry> entries{{uwf::app::FileStagingKind::File, QStringLiteral("C:\\数据|state.bin")},
                                                  {uwf::app::FileStagingKind::Directory, QStringLiteral("D:\\Vendor\\Cache")}};
  const auto encoded = uwf::app::encodeFileStagingEntries(entries);
  QCOMPARE(encoded, std::vector<std::string>({"F|C:\\数据|state.bin", "D|D:\\Vendor\\Cache"}));
  QCOMPARE(uwf::app::decodeFileStagingEntries(encoded), entries);
  QVERIFY(uwf::app::decodeFileStagingEntries({}).isEmpty());

  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::app::decodeFileStagingEntries({"X|C:\\unknown"})));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::app::decodeFileStagingEntries({"F|"})));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::app::decodeFileStagingEntries({"F|C:relative"})));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::app::decodeFileStagingEntries({R"(D|\\server\share\cache)"})));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::app::decodeFileStagingEntries({R"(F|\\.\C:\device)"})));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::app::decodeFileStagingEntries({R"(F|\\?\C:\extended)"})));
  std::string encodedNul = "F|C:\\state";
  encodedNul.push_back('\0');
  encodedNul += "hidden";
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(uwf::app::decodeFileStagingEntries({encodedNul})));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::app::encodeFileStagingEntries({{uwf::app::FileStagingKind::File, {}}})));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument,
                           static_cast<void>(uwf::app::encodeFileStagingEntries({{uwf::app::FileStagingKind::File, QStringLiteral("C:relative")}})));
  QString nulPath = QStringLiteral("C:\\state");
  nulPath.append(QChar(u'\0'));
  const QList<uwf::app::FileStagingEntry> nulEntry{{uwf::app::FileStagingKind::File, nulPath}};
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::app::encodeFileStagingEntries(nulEntry)));
  const QList<uwf::app::FileStagingEntry> invalidKind{{static_cast<uwf::app::FileStagingKind>(99), QStringLiteral("C:\\state")}};
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::app::encodeFileStagingEntries(invalidKind)));
  QVERIFY_THROWS_EXCEPTION(std::system_error, static_cast<void>(uwf::app::decodeFileStagingEntries({std::string("F|\xC3", 3)})));
}

void FileStagingPolicyTests::fileStagingConflictsRespectCaseAndPathSegments() {
  uwf::app::FileStagingConflictPolicy policy;
  policy.setFileExclusions({QStringLiteral("C:/Program Data/Vendor/"), QStringLiteral("c:\\program data\\vendor"), QStringLiteral("D:\\Shared")});
  policy.setStagedEntries(
      {{uwf::app::FileStagingKind::File, QStringLiteral("C:\\Cache\\state.bin")}, {uwf::app::FileStagingKind::Directory, QStringLiteral("D:/Work/Tree")}});

  const auto vendorConflict = policy.conflictingExclusion(QStringLiteral("c:\\PROGRAM DATA\\Vendor\\state.db"));
  QVERIFY(vendorConflict.has_value());
  QCOMPARE(vendorConflict->compare(QStringLiteral("C:\\Program Data\\Vendor"), Qt::CaseInsensitive), 0);
  QCOMPARE(policy.conflictingExclusion(QStringLiteral("D:\\Shared")), std::optional{QStringLiteral("D:\\Shared")});
  QVERIFY(!policy.conflictingExclusion(QStringLiteral("C:\\Program Database\\Vendor")).has_value());
  QVERIFY(!policy.conflictingExclusion(QStringLiteral("E:\\Shared")).has_value());

  QCOMPARE(policy.conflictingStagedPath(QStringLiteral("C:\\Cache")), std::optional{QStringLiteral("C:\\Cache\\state.bin")});
  QCOMPARE(policy.conflictingStagedPath(QStringLiteral("d:\\work\\tree\\child")), std::optional{QStringLiteral("D:\\Work\\Tree")});
  QVERIFY(!policy.conflictingStagedPath(QStringLiteral("C:\\Caches")).has_value());
}

void FileStagingPolicyTests::fileStagingConflictsUseNormalizedCandidates_data() {
  QTest::addColumn<QString>("candidate");
  QTest::addColumn<bool>("conflict");
  QTest::newRow("same-path") << QStringLiteral("c:/root/protected") << true;
  QTest::newRow("child-dot") << QStringLiteral("C:/Root/Protected/./state.bin") << true;
  QTest::newRow("parent-enters") << QStringLiteral("C:/Root/Other/../Protected/state.bin") << true;
  QTest::newRow("parent-leaves") << QStringLiteral("C:/Root/Protected/../Other/state.bin") << false;
  QTest::newRow("ancestor") << QStringLiteral("C:/Root/Child/..") << true;
  QTest::newRow("sibling-prefix") << QStringLiteral("C:/Root/ProtectedElsewhere/state.bin") << false;
  QTest::newRow("other-volume") << QStringLiteral("D:/Root/Protected/state.bin") << false;
}

void FileStagingPolicyTests::fileStagingConflictsUseNormalizedCandidates() {
  QFETCH(QString, candidate);
  QFETCH(bool, conflict);
  uwf::app::FileStagingConflictPolicy policy;
  const QString stored = QStringLiteral("C:/Root/Nested/../Protected/");
  const QString canonical = QStringLiteral("C:\\Root\\Protected");
  policy.setFileExclusions({stored});
  policy.setStagedEntries({{uwf::app::FileStagingKind::Directory, stored}});
  const auto exclusion = policy.conflictingExclusion(candidate);
  const auto staged = policy.conflictingStagedPath(candidate);
  QCOMPARE(exclusion.has_value(), conflict);
  QCOMPARE(staged.has_value(), conflict);
  if (conflict) {
    QCOMPARE(*exclusion, canonical);
    QCOMPARE(*staged, canonical);
  }
}

}  // namespace

QTEST_APPLESS_MAIN(FileStagingPolicyTests)

#include "FileStagingPolicyTests.moc"
