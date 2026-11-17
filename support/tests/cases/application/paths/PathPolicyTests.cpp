// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "application/configuration/contracts/ConfiguredExclusionIdentity.h"
#include "application/overlay/files/OverlayFileCatalog.h"
#include "application/paths/ExclusionCoverage.h"
#include "application/paths/PathPolicy.h"
#include "application/staging/editing/FileStagingConflictPolicy.h"

namespace {

QString text(const std::string& value) { return QString::fromUtf8(value); }

class PathPolicyTests final : public QObject {
  Q_OBJECT
  enum class ControlSetReader { Missing, Failing, Available };
  Q_ENUM(ControlSetReader)

 private slots:
  void volumeAddressesRetainLiteralSuffixes();
  void localFileSyntaxDoesNotRequireAPhysicalVolumeMapping();
  void fileSystemNavigationPreservesNamespace_data();
  void fileSystemNavigationPreservesNamespace();
  void ordinaryFileSystemNavigationKeepsQtAddressSpelling();
  void volumeAliasesShareCoverageAndConflicts();
  void pathMatchingHonorsSegmentBoundaries();
  void pathMatchingPropertiesPreserveSegmentSemantics();
  void fileNormalizationResolvesEquivalentPaths_data();
  void fileNormalizationResolvesEquivalentPaths();
  void registryContainmentPreservesLiteralDotSegments();
  void registryHiveExpansionPreservesUtf16Identity();
  void registryCoverageResolvesHivesWithoutChangingConfigurationIdentity_data();
  void registryCoverageResolvesHivesWithoutChangingConfigurationIdentity();
  void objectAndConfigurationIdentityRemainDistinct_data();
  void objectAndConfigurationIdentityRemainDistinct();
  void exclusionCoveragePreservesIndependentRawMatches_data();
  void exclusionCoveragePreservesIndependentRawMatches();
  void registryCoverageReadsOnlyNecessaryEnvironmentFacts_data();
  void registryCoverageReadsOnlyNecessaryEnvironmentFacts();
  void registryCoverageAggregatesIndependentEvidenceBeforeReadingContext_data();
  void registryCoverageAggregatesIndependentEvidenceBeforeReadingContext();
  void fileCoveragePreservesStoredSeparatorsAndComponents_data();
  void fileCoveragePreservesStoredSeparatorsAndComponents();
};

void PathPolicyTests::localFileSyntaxDoesNotRequireAPhysicalVolumeMapping() {
  using namespace uwf::app::paths;
  const auto path = QStringLiteral(R"(\\?\Z:\Data\leaf. )");
  const auto syntax = parseLocalFilePath(path);
  QVERIFY(syntax);
  QCOMPARE(syntax->volume, std::string("Z:"));
  QCOMPARE(syntax->relativePath, QStringLiteral("\\Data\\leaf. "));
  QVERIFY(!resolveVolumeFilePath(path));
  const std::string id = "11111111-2222-3333-4444-555555555555";
  const auto bound = resolveVolumeFilePath(path, {{QStringLiteral("Z:"), id}});
  QVERIFY(bound);
  QCOMPARE(bound->volume, id);
  QCOMPARE(bound->relativePath, syntax->relativePath);
  const auto guid = parseLocalFilePath(volumeFilePath(*bound));
  QVERIFY(guid);
  QCOMPARE(guid->volume, id);
  QCOMPARE(guid->relativePath, syntax->relativePath);
}

void PathPolicyTests::fileSystemNavigationPreservesNamespace_data() {
  QTest::addColumn<QString>("root");
  QTest::newRow("volume-guid") << QStringLiteral(R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\)");
  QTest::newRow("extended-drive") << QStringLiteral(R"(\\?\C:\)");
  QTest::newRow("device-drive") << QStringLiteral(R"(\\.\C:\)");
  QTest::newRow("extended-unc") << QStringLiteral(R"(\\?\UNC\server\share\)");
}
void PathPolicyTests::fileSystemNavigationPreservesNamespace() {
  QFETCH(QString, root);
  using namespace uwf::app::paths;
  const auto directory = root + QStringLiteral("Data");
  const auto leaf = QStringLiteral("内容\U0001F680. ");
  const auto object = directory + u'\\' + leaf;
  QCOMPARE(fileSystemPath(object), object);
  auto portable = object;
  portable.replace(u'\\', u'/');
  QCOMPARE(fileSystemPath(portable), object);
  QCOMPARE(absoluteFilePath(object, QStringLiteral("D:/elsewhere")), object);
  QCOMPARE(parentDirectory(object), directory);
  QCOMPARE(parentDirectory(directory), root);
  QCOMPARE(parentDirectory(root), root);
  QCOMPARE(parentDirectory(directory + QStringLiteral("\\\\")), root);
  QCOMPARE(fileName(object), leaf);
  QCOMPARE(fileName(root), QString{});
  QCOMPARE(joinFilePath(directory, leaf), object);
  QCOMPARE(joinFilePath(root, QStringLiteral("Data")), directory);
  QCOMPARE(joinFilePath(directory, QStringLiteral("\\Sibling")), root + QStringLiteral("Sibling"));
  QCOMPARE(absoluteFilePath(QStringLiteral("Child\\item.bin"), directory), directory + QStringLiteral("\\Child\\item.bin"));
  QCOMPARE(absoluteFilePath(QStringLiteral("..\\Sibling"), directory), root + QStringLiteral("Sibling"));
  QCOMPARE(absoluteFilePath(QStringLiteral(".\\Child\\..\\") + leaf, directory), object);
  QCOMPARE(absoluteFilePath(QStringLiteral("..\\..\\.."), directory), root);
  QCOMPARE(absoluteFilePath(QStringLiteral("\\Sibling"), directory), root + QStringLiteral("Sibling"));
  QCOMPARE(absoluteFilePath(QStringLiteral("D:/Other"), directory), QStringLiteral("D:/Other"));
  QCOMPARE(joinFilePath(directory, QStringLiteral("D:/Other")), QStringLiteral("D:/Other"));
  // An explicitly addressed object is not an interactive relative navigation request.
  const auto literal = directory + QStringLiteral("\\.\\Child\\..\\leaf ");
  QCOMPARE(absoluteFilePath(literal, root), literal);
}
void PathPolicyTests::ordinaryFileSystemNavigationKeepsQtAddressSpelling() {
  using namespace uwf::app::paths;
  QCOMPARE(fileSystemPath(QStringLiteral("C:\\Data\\.\\Folder")), QStringLiteral("C:/Data/Folder"));
  QCOMPARE(absoluteFilePath(QStringLiteral("../Sibling"), QStringLiteral("C:/Data/Folder")), QStringLiteral("C:/Data/Sibling"));
  QCOMPARE(parentDirectory(QStringLiteral("C:/Data/leaf ")), QStringLiteral("C:/Data"));
  QCOMPARE(parentDirectory(QStringLiteral("C:/")), QStringLiteral("C:/"));
  QCOMPARE(fileName(QStringLiteral("C:/Data/leaf ")), QStringLiteral("leaf "));
  QCOMPARE(joinFilePath(QStringLiteral("C:/Data"), QStringLiteral("Folder/leaf ")), QStringLiteral("C:/Data/Folder/leaf "));
}

void PathPolicyTests::volumeAddressesRetainLiteralSuffixes() {
  using namespace uwf::app;
  const std::string id = "11111111-2222-3333-4444-555555555555";
  const paths::VolumeIdentities mounts{{QStringLiteral("C:"), id}};
  for (const auto& suffix : {QString{}, QStringLiteral("\\"), QStringLiteral("\\Data\\..\\Data\\"), QStringLiteral("\\Data\\\\leaf "),
                             QStringLiteral("\\\\?\\Volume{66666666-7777-8888-9999-aaaaaaaaaaaa}\\Data")}) {
    const auto drive = paths::resolveVolumeFilePath(QStringLiteral("c:") + suffix, mounts);
    QVERIFY(drive);
    QCOMPARE(drive->volume, id);
    QCOMPARE(drive->relativePath, suffix);
    const auto guid = paths::resolveVolumeFilePath(paths::volumeFilePath(*drive));
    QVERIFY(guid);
    QCOMPARE(guid->volume, id);
    QCOMPARE(guid->relativePath, suffix);
    const auto explicitPath = paths::volumeFilePath(*drive);
    QCOMPARE(paths::normalize(explicitPath), explicitPath);
    auto qtPath = explicitPath;
    qtPath.replace(u'\\', u'/');
    QCOMPARE(paths::normalize(qtPath), explicitPath);
  }
  QVERIFY(!paths::resolveVolumeFilePath(QStringLiteral("Z:\\Data"), mounts));
  QVERIFY(!paths::resolveVolumeFilePath(QStringLiteral("C:\\Data")));
  const auto relative = paths::resolveVolumeFilePath(QStringLiteral("\\Data"), {}, id);
  QVERIFY(relative);
  QCOMPARE(relative->relativePath, QStringLiteral("\\Data"));
}

void PathPolicyTests::volumeAliasesShareCoverageAndConflicts() {
  using namespace uwf::app;
  const std::string id = "11111111-2222-3333-4444-555555555555";
  const std::string other = "66666666-7777-8888-9999-aaaaaaaaaaaa";
  const paths::VolumeIdentities mounts{{QStringLiteral("C:"), id}, {QStringLiteral("D:"), other}};
  const auto guid = [&](const QString& suffix) { return paths::volumeFilePath({id, suffix}); };
  const auto target = QStringLiteral("C:\\Data\\leaf.bin");
  const auto full = guid(QStringLiteral("\\Data\\leaf.bin"));
  for (const auto& pattern : {QStringLiteral("C:\\Data"), guid(QStringLiteral("\\Data"))}) {
    for (const auto& actual : {target, full}) {
      const auto coverage = fileExclusionCoverage(QStringList{pattern}, actual, mounts);
      QVERIFY(coverage.writesBypass());
      QVERIFY(coverage.commitSkipped());
      QCOMPARE(coverage.directWriteExclusion, pattern);
      QCOMPARE(fileCommitSkipExclusion(QStringList{pattern}, actual, mounts), pattern);
      QVERIFY(fileExclusionIntersectsDirectory(pattern, guid(QStringLiteral("\\Data\\Child")), mounts));
    }
  }
  QVERIFY(!fileExclusionCoverage(QStringList{guid(QStringLiteral("\\Data"))}, QStringLiteral("D:\\Data\\leaf.bin"), mounts).preventsCommit());
  for (const auto& suffix : {QStringLiteral("\\Data\\.\\"), QStringLiteral("\\Data\\\\"), QStringLiteral("\\Data\\Other\\..")})
    QVERIFY(!fileExclusionCoverage(QStringList{guid(suffix)}, target, mounts).preventsCommit());
  const auto trailing = fileExclusionCoverage(QStringList{guid(QStringLiteral("\\Data\\"))}, target, mounts);
  QVERIFY(trailing.commitSkipped());
  QVERIFY(!trailing.writesBypass());

  // Provider data is a literal suffix within its owning volume, including an
  // address-looking prefix (native QueryMultiSzValue / PopulatePrefixTable).
  const auto raw = guid(QStringLiteral("\\Data"));
  const auto wrappedRaw = paths::volumeFilePath({id, raw});
  QVERIFY(!fileExclusionCoverage(QStringList{wrappedRaw}, target, mounts).preventsCommit());
  const FileStagingEntry entry{FileStagingKind::File, full, FileCommitTarget{id, QStringLiteral("\\Data\\leaf.bin")}};
  QVERIFY(firstFileStagingExclusionConflict({entry}, {QStringLiteral("C:\\Data")}, mounts));
  QVERIFY(firstFileStagingExclusionConflict({entry}, {guid(QStringLiteral("\\Data"))}, mounts));
  QVERIFY(!firstFileStagingExclusionConflict({entry}, {wrappedRaw}, mounts));
  QVERIFY(!firstFileStagingExclusionConflict({entry}, {QStringLiteral("D:\\Data")}, mounts));
}

void PathPolicyTests::registryHiveExpansionPreservesUtf16Identity() {
  // Hive addressing is a UTF-16 operation, before the separate UTF-8 transport
  // admission. Even names that transport must reject keep their exact identity.
  for (const auto& suffix : {QStringLiteral("数据\\\U0001F680"), QString(QChar(0xd800)), QString(QChar(0xdc00)), QString(QChar(0))}) {
    const auto rest = QStringLiteral("\\Software\\") + suffix;
    QCOMPARE(uwf::app::paths::expandRegistryHive(QStringLiteral("HKLM") + rest), QStringLiteral("HKEY_LOCAL_MACHINE") + rest);
    QCOMPARE(uwf::app::paths::expandRegistryHive(QStringLiteral("Other") + rest), QStringLiteral("Other") + rest);
  }
}

void PathPolicyTests::fileCoveragePreservesStoredSeparatorsAndComponents_data() {
  QTest::addColumn<QString>("pattern");
  QTest::addColumn<bool>("qtTarget");
  QTest::addColumn<bool>("management");
  QTest::addColumn<bool>("driver");
  QTest::addColumn<bool>("intersects");
  struct Pattern {
    const char* name;
    const char16_t* text;
    bool management, driver, intersects;
  };
  constexpr std::array patterns{Pattern{"native", u"C:\\Data\\Cache", true, true, true},
                                Pattern{"one-tail", u"C:\\Data\\Cache\\", true, false, true},
                                Pattern{"two-tails", u"C:\\Data\\Cache\\\\", false, false, false},
                                Pattern{"forward-slashes", u"C:/Data/Cache", false, false, false},
                                Pattern{"mixed-separators", u"C:\\Data/Cache", false, false, false},
                                Pattern{"dot-component", u"C:\\Data\\.\\Cache", false, false, false},
                                Pattern{"parent-component", u"C:\\Data\\Other\\..\\Cache", false, false, false},
                                Pattern{"descendant", u"C:\\Data\\Cache\\Nested", false, false, true},
                                Pattern{"other-volume", u"D:\\Data\\Cache", false, false, false}};
  for (const auto& pattern : patterns)
    for (const bool qtTarget : {false, true})
      QTest::newRow(qPrintable(QStringLiteral("%1/qt-target-%2").arg(QString::fromLatin1(pattern.name)).arg(qtTarget)))
          << QString::fromUtf16(pattern.text) << qtTarget << pattern.management << pattern.driver << pattern.intersects;
}
void PathPolicyTests::fileCoveragePreservesStoredSeparatorsAndComponents() {
  QFETCH(QString, pattern);
  QFETCH(bool, qtTarget);
  QFETCH(bool, management);
  QFETCH(bool, driver);
  QFETCH(bool, intersects);
  const auto target = qtTarget ? QStringLiteral("C:/Data/Cache/Child") : QStringLiteral("C:\\Data\\Cache\\Child");
  const auto directory = qtTarget ? QStringLiteral("C:/Data/Cache") : QStringLiteral("C:\\Data\\Cache");
  const auto check = [&](const auto& effects) {
    QCOMPARE(effects.commitSkipped(), management);
    QCOMPARE(effects.writesBypass(), driver);
    QCOMPARE(effects.commitSkipExclusion, management ? pattern : QString{});
    QCOMPARE(effects.directWriteExclusion, driver ? pattern : QString{});
  };
  check(uwf::app::fileExclusionCoverage(QStringList{pattern}, target));
  check(uwf::app::fileExclusionCoverage(std::vector<std::string>{pattern.toStdString()}, target));
  QCOMPARE(uwf::app::fileCommitSkipExclusion(QStringList{pattern}, target), management ? pattern : QString{});
  QCOMPARE(uwf::app::fileCommitSkipExclusion(std::vector<std::string>{pattern.toStdString()}, target), management ? pattern : QString{});
  QCOMPARE(uwf::app::fileExclusionIntersectsDirectory(pattern, directory), intersects);
}

void PathPolicyTests::registryCoverageAggregatesIndependentEvidenceBeforeReadingContext_data() {
  QTest::addColumn<QStringList>("patterns");
  QTest::addColumn<QString>("target");
  QTest::addColumn<QString>("directWitness");
  QTest::addColumn<QString>("managementWitness");
  QTest::addColumn<bool>("requiresContext");
  QTest::addColumn<ControlSetReader>("readerState");
  for (const bool targetAlias : {false, true}) {
    const auto targetRoot =
        targetAlias ? QStringLiteral("HKLM\\SYSTEM\\CurrentControlSet\\Fixture\\") : QStringLiteral("HKLM\\SYSTEM\\ControlSet001\\Fixture\\");
    const auto otherRoot =
        targetAlias ? QStringLiteral("HKLM\\SYSTEM\\ControlSet001\\Fixture\\") : QStringLiteral("HKLM\\SYSTEM\\CurrentControlSet\\Fixture\\");
    const auto target = targetRoot + QChar(0x00e4) + QStringLiteral("\\Child");
    const auto pending = otherRoot + QChar(0x00e4);
    const auto shared = targetRoot + QChar(0x00e4);
    const auto direct = targetRoot + QChar(0x00c4);
    const auto management = targetRoot + QChar(0x00e4) + u'\\';
    struct Evidence {
      const char* name;
      QStringList patterns;
      QString driver;
      QString management;
      bool requiresContext;
    };
    const std::array evidence{Evidence{"shared-proof", {pending, shared}, shared, shared, false},
                              Evidence{"independent-proofs", {pending, direct, management}, direct, management, false},
                              Evidence{"management-only-proof", {pending, management}, pending, management, true},
                              Evidence{"no-context-free-proof", {pending}, pending, {}, true}};
    for (const auto& row : evidence)
      for (const bool reverse : {false, true}) {
        if (row.patterns.size() == 1 && reverse) continue;
        auto patterns = row.patterns;
        if (reverse) std::reverse(patterns.begin(), patterns.end());
        for (const auto& [readerName, reader] : {std::pair{"missing-fact", ControlSetReader::Missing}, std::pair{"failed-fact", ControlSetReader::Failing},
                                                 std::pair{"available-fact", ControlSetReader::Available}}) {
          const auto name = QStringLiteral("%1/%2/%3/%4")
                                .arg(targetAlias ? QStringLiteral("target-CSS") : QStringLiteral("target-concrete"), QString::fromLatin1(row.name),
                                     reverse ? QStringLiteral("proof-first") : QStringLiteral("pending-first"), QString::fromLatin1(readerName));
          QTest::newRow(qPrintable(name)) << patterns << target << row.driver << row.management << row.requiresContext << reader;
        }
      }
  }
}

void PathPolicyTests::registryCoverageAggregatesIndependentEvidenceBeforeReadingContext() {
  QFETCH(QStringList, patterns);
  QFETCH(QString, target);
  QFETCH(QString, directWitness);
  QFETCH(QString, managementWitness);
  QFETCH(bool, requiresContext);
  QFETCH(ControlSetReader, readerState);
  int reads = 0;
  std::function<std::uint32_t()> reader;
  if (readerState != ControlSetReader::Missing)
    reader = [&]() -> std::uint32_t {
      ++reads;
      if (readerState == ControlSetReader::Failing) throw std::runtime_error("The selected control set cannot be read: original diagnostic");
      return 1;
    };
  const auto verify = [&](const auto& entries) {
    // Explicit commits only need the management skip, even when the independent
    // driver coverage would require an unavailable CurrentControlSet fact.
    QCOMPARE(uwf::app::registryCommitSkipExclusion(entries, target), managementWitness);
    if (requiresContext && readerState != ControlSetReader::Available) {
      try {
        static_cast<void>(uwf::app::registryExclusionCoverage(entries, target, reader));
        QFAIL("Unresolved driver coverage must not succeed without its required environment fact");
      } catch (const std::runtime_error& error) {
        if (readerState == ControlSetReader::Failing)
          QCOMPARE(QString::fromUtf8(error.what()), QStringLiteral("The selected control set cannot be read: original diagnostic"));
      }
      return;
    }
    const auto result = uwf::app::registryExclusionCoverage(entries, target, reader);
    QCOMPARE(result.directWriteExclusion, directWitness);
    QCOMPARE(result.commitSkipExclusion, managementWitness);
    QVERIFY(result.writesBypass());
    QCOMPARE(result.commitSkipped(), !managementWitness.isEmpty());
    // Witnesses need to be real original entries, regardless of provider order.
    QVERIFY(patterns.contains(result.directWriteExclusion));
    if (result.commitSkipped()) QVERIFY(patterns.contains(result.commitSkipExclusion));
  };
  verify(patterns);
  std::vector<std::string> utf8;
  for (const auto& pattern : std::as_const(patterns)) utf8.push_back(pattern.toStdString());
  verify(utf8);
  QCOMPARE(reads, requiresContext && readerState != ControlSetReader::Missing ? 2 : 0);
}

void PathPolicyTests::registryCoverageReadsOnlyNecessaryEnvironmentFacts_data() {
  QTest::addColumn<QString>("pattern");
  QTest::addColumn<QString>("target");
  QTest::addColumn<bool>("management");
  QTest::addColumn<bool>("driver");
  QTest::addColumn<bool>("needsContext");
  QTest::addColumn<bool>("readerFails");
  struct Comparison {
    const char* name;
    const char16_t* pattern;
    const char16_t* target;
    bool management, driver, needsContext;
  };
  constexpr std::array comparisons{
      Comparison{"same-CSS", u"CurrentControlSet\\Vendor", u"CurrentControlSet\\Vendor\\Child", true, true, false},
      Comparison{"same-CSS-one-tail", u"CurrentControlSet\\Vendor\\", u"CurrentControlSet\\Vendor\\Child", true, false, false},
      Comparison{"unrelated-SYSTEM", u"CurrentControlSet\\Vendor", u"Ordinary\\Child", false, false, false},
      Comparison{"unrelated-pattern", u"Ordinary", u"CurrentControlSet\\Vendor\\Child", false, false, false},
      Comparison{"literal-CSS-extra", u"CurrentControlSetExtra", u"CurrentControlSetExtra\\Child", true, false, false},
      Comparison{"CSS-to-concrete", u"CurrentControlSet\\Vendor", u"ControlSet002\\Vendor\\Child", false, true, true},
      Comparison{"concrete-to-CSS", u"ControlSet002\\Vendor", u"CurrentControlSet\\Vendor\\Child", false, true, true},
      Comparison{"CSS-to-ControlSetOther", u"CurrentControlSet\\Vendor", u"ControlSetOther\\Vendor\\Child", false, false, false},
      Comparison{"ControlSetOther-to-CSS", u"ControlSetOther\\Vendor", u"CurrentControlSet\\Vendor\\Child", false, false, false},
      Comparison{"CSS-to-bare-ControlSet", u"CurrentControlSet\\Vendor", u"ControlSet\\Vendor\\Child", false, false, false},
      Comparison{"bare-ControlSet-to-CSS", u"ControlSet\\Vendor", u"CurrentControlSet\\Vendor\\Child", false, false, false},
      Comparison{"CSS-to-short-number", u"CurrentControlSet\\Vendor", u"ControlSet01\\Vendor\\Child", false, false, false},
      Comparison{"short-number-to-CSS", u"ControlSet01\\Vendor", u"CurrentControlSet\\Vendor\\Child", false, false, false},
      Comparison{"CSS-alpha-suffix", u"CurrentControlSetExtra\\Vendor", u"ControlSet002Extra\\Vendor\\Child", false, true, true},
      Comparison{"CSS-numeric-suffix", u"CurrentControlSet1\\Vendor", u"ControlSet0021\\Vendor\\Child", false, true, true},
      Comparison{"CSS-zero-numeric-suffix", u"CurrentControlSet0\\Vendor", u"ControlSet0000\\Vendor\\Child", false, false, true},
      Comparison{"plain-CSS-cannot-match-alpha-suffix", u"CurrentControlSet\\Vendor", u"ControlSet002Extra\\Vendor\\Child", false, false, false},
      Comparison{"alpha-suffix-cannot-match-complete-CSS", u"ControlSet002Extra\\Vendor", u"CurrentControlSet\\Vendor\\Child", false, false, false},
      Comparison{"extra-leading-zero-is-not-a-formatted-id", u"CurrentControlSet\\Vendor", u"ControlSet0011\\Vendor\\Child", false, false, false},
      Comparison{"numeric-suffix-mismatch", u"CurrentControlSet1\\Vendor", u"ControlSet0020\\Vendor\\Child", false, false, false},
      Comparison{"cross-alias-one-tail", u"CurrentControlSet\\Vendor\\", u"ControlSet002\\Vendor\\Child", false, false, false},
  };
  for (const auto& row : comparisons)
    for (const bool readerFails : {false, true})
      QTest::newRow(qPrintable(QStringLiteral("%1/failing-reader-%2").arg(QString::fromLatin1(row.name)).arg(readerFails)))
          << QStringLiteral("HKLM\\SYSTEM\\") + QString::fromUtf16(row.pattern) << QStringLiteral("HKLM\\SYSTEM\\") + QString::fromUtf16(row.target)
          << row.management << row.driver << row.needsContext << readerFails;
  for (const bool readerFails : {false, true})
    QTest::newRow(readerFails ? "known-parent/failing-reader" : "known-parent/available-reader")
        << QStringLiteral("HKLM\\SYSTEM") << QStringLiteral("HKLM\\SYSTEM\\CurrentControlSet\\Vendor\\Child") << true << true << false << readerFails;
  for (const bool readerFails : {false, true})
    QTest::newRow(readerFails ? "parent-pattern-with-one-tail/failing-reader" : "parent-pattern-with-one-tail/available-reader")
        << QStringLiteral("HKLM\\SYSTEM\\") << QStringLiteral("HKLM\\SYSTEM\\CurrentControlSet\\Vendor\\Child") << true << false << false << readerFails;
}
void PathPolicyTests::registryCoverageReadsOnlyNecessaryEnvironmentFacts() {
  QFETCH(QString, pattern);
  QFETCH(QString, target);
  QFETCH(bool, management);
  QFETCH(bool, driver);
  QFETCH(bool, needsContext);
  QFETCH(bool, readerFails);
  int reads = 0;
  const auto reader = [&]() -> std::uint32_t {
    ++reads;
    if (readerFails) throw std::runtime_error("The selected control set cannot be read");
    return 2;
  };
  const QStringList patterns{pattern};
  if (readerFails && needsContext) {
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, (void)uwf::app::registryExclusionCoverage(patterns, target, reader));
    QCOMPARE(reads, 1);
    return;
  }
  const auto coverage = uwf::app::registryExclusionCoverage(patterns, target, reader);
  QCOMPARE(coverage.commitSkipped(), management);
  QCOMPARE(coverage.writesBypass(), driver);
  QCOMPARE(reads, needsContext ? 1 : 0);
  const auto utf8 = uwf::app::registryExclusionCoverage(std::vector<std::string>{pattern.toStdString()}, target, reader);
  QCOMPARE(utf8.commitSkipExclusion, coverage.commitSkipExclusion);
  QCOMPARE(utf8.directWriteExclusion, coverage.directWriteExclusion);
  QCOMPARE(reads, needsContext ? 2 : 0);
}

void PathPolicyTests::exclusionCoveragePreservesIndependentRawMatches_data() {
  QTest::addColumn<bool>("registry");
  QTest::addColumn<QStringList>("patterns");
  QTest::addColumn<QString>("target");
  QTest::addColumn<QString>("direct");
  QTest::addColumn<QString>("skipped");
  for (const bool registry : {false, true}) {
    const auto root = registry ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\..\\Literal \\") : QStringLiteral("C:\\Data\\");
    const auto lower = root + QChar(0x00e4);
    const auto direct = root + QChar(0x00c4);
    const auto skipped = (registry ? QStringLiteral("hKlM\\SOFTWARE\\..\\Literal \\") : root) + QChar(0x00e4) + u'\\';
    const auto unrelated = lower + QStringLiteral("Sibling");
    const auto target = lower + QStringLiteral("\\Child");
    const auto tag = registry ? QStringLiteral("registry/") : QStringLiteral("file/");
    QTest::newRow(qPrintable(tag + QStringLiteral("management-first"))) << registry << QStringList{unrelated, skipped, direct} << target << direct << skipped;
    QTest::newRow(qPrintable(tag + QStringLiteral("driver-first"))) << registry << QStringList{unrelated, direct, skipped} << target << direct << skipped;
    QTest::newRow(qPrintable(tag + QStringLiteral("management-only"))) << registry << QStringList{skipped} << target << QString{} << skipped;
    QTest::newRow(qPrintable(tag + QStringLiteral("driver-only"))) << registry << QStringList{direct} << target << direct << QString{};
    QTest::newRow(qPrintable(tag + QStringLiteral("shared-first"))) << registry << QStringList{lower, skipped, direct} << target << lower << lower;
    QTest::newRow(qPrintable(tag + QStringLiteral("shared-last"))) << registry << QStringList{skipped, direct, lower} << target << direct << skipped;
    QTest::newRow(qPrintable(tag + QStringLiteral("uncovered"))) << registry << QStringList{unrelated} << target << QString{} << QString{};
    QTest::newRow(qPrintable(tag + QStringLiteral("double-tail-uncovered")))
        << registry << QStringList{lower + QStringLiteral("\\\\")} << target << QString{} << QString{};
  }
}
void PathPolicyTests::exclusionCoveragePreservesIndependentRawMatches() {
  QFETCH(bool, registry);
  QFETCH(QStringList, patterns);
  QFETCH(QString, target);
  QFETCH(QString, direct);
  QFETCH(QString, skipped);
  std::vector<std::string> utf8;
  for (const auto& pattern : patterns) utf8.push_back(pattern.toStdString());
  const auto check = [&](const auto& result) {
    QCOMPARE(result.directWriteExclusion, direct);
    QCOMPARE(result.commitSkipExclusion, skipped);
    QCOMPARE(result.writesBypass(), !direct.isEmpty());
    QCOMPARE(result.commitSkipped(), !skipped.isEmpty());
    QCOMPARE(result.preventsCommit(), !direct.isEmpty() || !skipped.isEmpty());
  };
  check(registry ? uwf::app::registryExclusionCoverage(patterns, target) : uwf::app::fileExclusionCoverage(patterns, target));
  check(registry ? uwf::app::registryExclusionCoverage(utf8, target) : uwf::app::fileExclusionCoverage(utf8, target));
}

void PathPolicyTests::pathMatchingHonorsSegmentBoundaries() {
  QCOMPARE(text(uwf::app::paths::stripTrailingSeparators(R"(C:\Data\\/)")), QStringLiteral("C:\\Data"));
  QVERIFY(uwf::app::paths::contains(QStringLiteral(R"(C:\DATA)"), QStringLiteral(R"(c:\data)")));
  QVERIFY(uwf::app::paths::contains(QStringLiteral(R"(c:/data)"), QStringLiteral(R"(C:/Data/Child/file.bin)")));
  QVERIFY(!uwf::app::paths::contains(QStringLiteral(R"(C:\Data)"), QStringLiteral(R"(C:\Database\file.bin)")));
  QVERIFY(!uwf::app::paths::contains(QStringLiteral(R"(C:\Data)"), QStringLiteral(R"(C:\Data2)")));
  QVERIFY(!uwf::app::paths::contains(QString{}, QStringLiteral(R"(C:\Data)")));

  const std::vector<std::string> exclusions = {R"(C:\Program Data\)", R"(D:\Cache)"};
  const auto coverage = uwf::app::fileExclusionCoverage(exclusions, QStringLiteral("c:\\program data\\app\\state.db"));
  QCOMPARE(coverage.commitSkipExclusion, QStringLiteral("C:\\Program Data\\"));
  QVERIFY(!coverage.writesBypass());
  QVERIFY(!uwf::app::fileExclusionCoverage(exclusions, QStringLiteral("C:\\Program")).preventsCommit());
}

void PathPolicyTests::pathMatchingPropertiesPreserveSegmentSemantics() {
  const std::array roots = {std::string(R"(C:\Root\Node)"), std::string(R"(c:/root/node)"), std::string(R"(C:/ROOT\NODE)")};
  for (const auto& root : roots) {
    const std::string canonical = uwf::app::paths::stripTrailingSeparators(root + R"(\//)");
    const auto parent = QString::fromStdString(uwf::app::paths::stripTrailingSeparators(root));
    QVERIFY(uwf::app::paths::contains(parent, QString::fromStdString(canonical)));
    QVERIFY(uwf::app::paths::contains(parent, QString::fromStdString(canonical + R"(/child\leaf.bin)")));
    QVERIFY(!uwf::app::paths::contains(parent, QString::fromStdString(canonical + "-sibling")));
    QVERIFY(!uwf::app::paths::contains(parent, QString::fromStdString(canonical.substr(0, canonical.size() - 1))));
  }

  const std::vector<std::string> exclusions{R"(C:\Root\)", R"(C:\Root\Longer\)", R"(D:\Data\)"};
  QCOMPARE(uwf::app::fileExclusionCoverage(exclusions, QStringLiteral("c:\\root\\longer\\file")).commitSkipExclusion,
           QString::fromStdString(exclusions.front()));
  QVERIFY(!uwf::app::fileExclusionCoverage(exclusions, QStringLiteral("C:\\Rooted\\file")).preventsCommit());
  QVERIFY(uwf::app::paths::stripTrailingSeparators("////").empty());
}

void PathPolicyTests::fileNormalizationResolvesEquivalentPaths_data() {
  QTest::addColumn<QString>("path");
  QTest::addColumn<QString>("expected");
  QTest::newRow("absolute") << QStringLiteral("Z:/Data/State.bin") << QStringLiteral("Z:\\Data\\State.bin");
  QTest::newRow("volume-rooted") << QStringLiteral("/Data/State.bin") << QStringLiteral("Z:\\Data\\State.bin");
  QTest::newRow("dot-segments") << QStringLiteral("Z:/Data/./Cache/../State.bin") << QStringLiteral("Z:\\Data\\State.bin");
  QTest::newRow("rooted-dot-segments") << QStringLiteral("/Scratch/../Data/State.bin/") << QStringLiteral("Z:\\Data\\State.bin");
  QTest::newRow("volume-root") << QStringLiteral("/Data/..") << QStringLiteral("Z:\\");
  QTest::newRow("other-volume") << QStringLiteral("Y:/Data/../State.bin") << QStringLiteral("Y:\\State.bin");
  QTest::newRow("unc") << QStringLiteral("//server/share/Data/../State.bin") << QStringLiteral("\\\\server\\share\\State.bin");
  QTest::newRow("empty") << QString{} << QString{};
}

void PathPolicyTests::fileNormalizationResolvesEquivalentPaths() {
  QFETCH(QString, path);
  QFETCH(QString, expected);
  const auto normalized = uwf::app::paths::normalize(path, QStringLiteral("Z:"));
  QCOMPARE(normalized, expected);
  QCOMPARE(uwf::app::paths::normalize(normalized, QStringLiteral("Z:")), normalized);
}

void PathPolicyTests::registryContainmentPreservesLiteralDotSegments() {
  using uwf::app::paths::contains;
  const QString root = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor");
  QVERIFY(contains(root + QStringLiteral("\\.."), root + QStringLiteral("\\..\\Child")));
  QVERIFY(!contains(root + QStringLiteral("\\Child"), root + QStringLiteral("\\.\\Child")));
  QVERIFY(!contains(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Child"), root + QStringLiteral("\\..\\Child")));
}

void PathPolicyTests::registryCoverageResolvesHivesWithoutChangingConfigurationIdentity_data() {
  QTest::addColumn<QString>("configured");
  QTest::addColumn<QString>("target");
  for (const auto* hive : {"HKLM", "HKCU", "HKCR", "HKU", "HKCC"}) {
    const auto root = QString::fromLatin1(hive) + QStringLiteral("\\Software\\..\\Vendor ");
    const auto expanded = uwf::app::paths::expandRegistryHive(root);
    QTest::newRow(hive) << root << expanded + QStringLiteral("\\Child");
  }
}
void PathPolicyTests::registryCoverageResolvesHivesWithoutChangingConfigurationIdentity() {
  QFETCH(QString, configured);
  QFETCH(QString, target);
  const auto expected = target.left(target.lastIndexOf(u'\\'));
  QCOMPARE(uwf::app::paths::expandRegistryHive(configured), expected);
  QVERIFY(!uwf::app::equalConfiguredExclusions(configured, expected));
  QVERIFY(uwf::app::paths::contains(configured, target));
  QVERIFY(uwf::app::paths::contains(expected, configured + QStringLiteral("\\Child")));
  QVERIFY(uwf::app::registryExclusionCoverage(QStringList{configured}, target).writesBypass());
  QVERIFY(!uwf::app::registryExclusionCoverage(QStringList{configured}, expected + QStringLiteral("Sibling\\Child")).preventsCommit());
  QVERIFY(!uwf::app::registryExclusionCoverage(QStringList{configured}, expected.trimmed() + QStringLiteral("\\Child")).preventsCommit());
}

void PathPolicyTests::objectAndConfigurationIdentityRemainDistinct_data() {
  QTest::addColumn<QString>("left");
  QTest::addColumn<QString>("right");
  QTest::addColumn<bool>("sameObject");
  QTest::addColumn<bool>("sameConfiguration");
  QTest::newRow("ascii") << QStringLiteral("Entry") << QStringLiteral("eNTRY") << true << true;
  QTest::newRow("latin-case") << QString(QChar(0x00c4)) << QString(QChar(0x00e4)) << true << false;
  QTest::newRow("different-utf8-length") << QString(QChar(0x023a)) << QString(QChar(0x2c65)) << true << false;
  QTest::newRow("micro-sign-is-not-greek-mu") << QString(QChar(0x00b5)) << QString(QChar(0x03bc)) << false << false;
  QTest::newRow("sharp-s-is-not-capital-sharp-s") << QString(QChar(0x00df)) << QString(QChar(0x1e9e)) << false << false;
}
void PathPolicyTests::objectAndConfigurationIdentityRemainDistinct() {
  QFETCH(QString, left);
  QFETCH(QString, right);
  QFETCH(bool, sameObject);
  QFETCH(bool, sameConfiguration);
  using namespace uwf::app;
  const ObjectIdentity first(left), second(right);
  QCOMPARE(first == second, sameObject);
  const std::set<ObjectIdentity> objects{first, second};
  QCOMPARE(objects.size(), sameObject ? std::size_t{1} : std::size_t{2});
  QCOMPARE(objects.contains(second), true);
  QCOMPARE(equalConfiguredExclusions(left, right), sameConfiguration);
  const std::set<QString> configured{configuredExclusionKey(left), configuredExclusionKey(right)};
  QCOMPARE(configured.size(), sameConfiguration ? std::size_t{1} : std::size_t{2});

  const auto parent = QStringLiteral("C:\\Data\\") + left;
  const auto alternate = QStringLiteral("C:\\Data\\") + right;
  QCOMPARE(paths::identity(parent) == paths::identity(alternate), sameObject);
  QCOMPARE(paths::contains(parent, alternate + QStringLiteral("\\child")), sameObject);
  QVERIFY(!paths::contains(parent, alternate + QStringLiteral("-sibling\\child")));
  QCOMPARE(fileExclusionCoverage(QStringList{parent}, alternate + QStringLiteral("\\child")).writesBypass(), sameObject);
  QCOMPARE(fileExclusionCoverage(QStringList{parent}, alternate + QStringLiteral("\\child")).commitSkipped(), sameConfiguration);
  const FileStagingChanges change{{{FileStagingKind::File, parent}}, {{FileStagingKind::File, alternate}}};
  QCOMPARE(change.added().size(), sameObject ? 0 : 1);
  QCOMPARE(change.removed().size(), sameObject ? 0 : 1);
  const auto catalog = buildOverlayFileCatalog(QStringLiteral("C:"), {{parent, {}, false, false, 1}, {alternate, {}, false, false, 2}});
  QCOMPARE(catalog.size(), sameObject ? 1 : 2);
  QCOMPARE(overlayFileCatalogSize(catalog), qulonglong{3});
}

}  // namespace

QTEST_APPLESS_MAIN(PathPolicyTests)

#include "PathPolicyTests.moc"
