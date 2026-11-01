// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/paths/PathPolicy.h"
#include "application/staging/editing/FileStagingConflictPolicy.h"
#include "application/staging/editing/FileStagingEntries.h"
#include "application/staging/editing/FileStagingPath.h"
#include "domain/uwf/contracts/Types.h"
#include "support/app/FileTargetFixture.h"

namespace {

class StagingAliasSource final : public uwf::app::CommitTraversalSource {
 public:
  bool offline = false;
  std::optional<uwf::app::CommitNode> inspect(const uwf::app::CommitNode&) const override { throw std::logic_error("Unexpected inspection"); }
  bool exists(const uwf::app::CommitNode&) const override { throw std::logic_error("Unexpected existence observation"); }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  uwf::app::FileCommitTarget resolveFileTarget(const QString& path) const override {
    if (offline) throw std::runtime_error("Volume offline");
    const auto address = uwf::app::paths::splitVolumeFilePath(path);
    if (!address) throw std::runtime_error("Unexpected path");
    return {uwf::compat::parseVolumeId(address->volume).value_or(uwf::test::application::fixtureVolumeId), address->relativePath};
  }
  QString enumerateChildren(const uwf::app::CommitNode&, std::stop_token, const uwf::app::CommitNodeVisitor&) const override {
    throw std::logic_error("Unexpected traversal");
  }
};

class FileStagingPolicyTests final : public QObject {
  Q_OBJECT

 private slots:

  void localPathAdmissionUsesSyntaxWithoutMountObservations_data();
  void localPathAdmissionUsesSyntaxWithoutMountObservations();
  void fileStagingConflictsRespectCaseAndPathSegments();
  void exclusionUpdatesKeepFirstSpellingAndDistinctNonAsciiNames();
  void fileStagingConflictsUseNormalizedCandidates_data();
  void fileStagingConflictsUseNormalizedCandidates();
  void fileStagingConflictsUseTheStoredPatternEffects_data();
  void fileStagingConflictsUseTheStoredPatternEffects();
  void fileStagingConflictsDoNotReinterpretStoredPatterns_data();
  void fileStagingConflictsDoNotReinterpretStoredPatterns();
  void entrySetNormalizationIsOrderIndependent_data();
  void entrySetNormalizationIsOrderIndependent();
  void laterKindChangesRetainOneObject_data();
  void laterKindChangesRetainOneObject();
  void invalidStoredParentsCannotHideValidSelections();
  void mountedAliasesShareObjectSetAndDirectoryCoverage();
  void mountedAliasesShareConflictAndReviewIdentity();
  void conflictObservationsRefreshWithoutChangingStoredPaths();
  void guidStoredAliasesShareOneOrderIndependentObjectSet();
  void offlineGuidIdentitySupportsCoverageWithoutGuessingDriveBindings();
};

void FileStagingPolicyTests::localPathAdmissionUsesSyntaxWithoutMountObservations_data() {
  QTest::addColumn<QString>("path");
  QTest::addColumn<bool>("accepted");
  QTest::newRow("drive") << QStringLiteral(R"(C:\Data\a.txt)") << true;
  QTest::newRow("extended-drive") << QStringLiteral(R"(\\?\C:\Data\a.txt)") << true;
  QTest::newRow("extended-lowercase-drive") << QStringLiteral(R"(\\?\c:\Data\a.txt)") << true;
  QTest::newRow("unobserved-drive") << QStringLiteral(R"(Z:\Unavailable\a.txt)") << true;
  QTest::newRow("unobserved-extended-drive") << QStringLiteral(R"(\\?\Z:\Unavailable\a.txt)") << true;
  QTest::newRow("extended-drive-root") << QStringLiteral(R"(\\?\C:\)") << true;
  QTest::newRow("volume-guid") << QStringLiteral(R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\Data\a.txt)") << true;
  QTest::newRow("relative") << QStringLiteral(R"(Data\a.txt)") << false;
  QTest::newRow("drive-relative") << QStringLiteral(R"(C:Data\a.txt)") << false;
  QTest::newRow("extended-drive-relative") << QStringLiteral(R"(\\?\C:Data\a.txt)") << false;
  QTest::newRow("extended-drive-without-root") << QStringLiteral(R"(\\?\C:)") << false;
  QTest::newRow("extended-invalid-drive") << QStringLiteral(R"(\\?\1:\Data\a.txt)") << false;
  QTest::newRow("extended-double-root") << QStringLiteral(R"(\\?\C:\\Data\a.txt)") << false;
  QTest::newRow("invalid-volume-guid") << QStringLiteral(R"(\\?\Volume{bad}\Data\a.txt)") << false;
  QTest::newRow("unc") << QStringLiteral(R"(\\server\share\a.txt)") << false;
  QTest::newRow("extended-unc") << QStringLiteral(R"(\\?\UNC\server\share\a.txt)") << false;
  QTest::newRow("device-drive") << QStringLiteral(R"(\\.\C:\Data\a.txt)") << false;
  QTest::newRow("device-namespace") << QStringLiteral(R"(\\?\GLOBALROOT\Device\HarddiskVolume1\a.txt)") << false;
}
void FileStagingPolicyTests::localPathAdmissionUsesSyntaxWithoutMountObservations() {
  QFETCH(QString, path);
  QFETCH(bool, accepted);
  QCOMPARE(uwf::app::isAbsoluteLocalFileStagingPath(path), accepted);
}

void FileStagingPolicyTests::guidStoredAliasesShareOneOrderIndependentObjectSet() {
  using namespace uwf::app;
  using namespace uwf::test::application;
  const auto prefix = QString::fromStdString(uwf::compat::volumePath(fixtureVolumeId));
  const FileStagingEntry direct{FileStagingKind::File, QStringLiteral("C:\\Data\\state.bin")};
  const FileStagingEntry guid{FileStagingKind::File, prefix + QStringLiteral("Data\\state.bin")};
  const FileStagingEntry parent{FileStagingKind::Directory, prefix + QStringLiteral("Data")};
  const FileStagingEntry otherVolume{FileStagingKind::File,
                                     QString::fromStdString(uwf::compat::volumePath(otherFixtureVolumeId)) + QStringLiteral("Data\\state.bin")};
  StagingAliasSource source;
  const auto aliases = normalizeFileStagingEntries({direct, guid}, &source);
  QCOMPARE(aliases.size(), 1);
  QCOMPARE(aliases.front().path, direct.path);
  QVERIFY(aliases.front().fileTarget);
  QCOMPARE(normalizeFileStagingEntries({guid, direct}, &source), aliases);
  const auto boundGuid = normalizeFileStagingEntries({guid}, &source);
  const FileStagingChanges addressReplacement{aliases, boundGuid, false};
  QCOMPARE(addressReplacement.added().size(), qsizetype{1});
  QCOMPARE(addressReplacement.removed().size(), qsizetype{1});
  const std::array entries{direct, guid, parent, otherVolume};
  std::array<std::size_t, entries.size()> order{0, 1, 2, 3};
  do {
    QList<FileStagingEntry> input;
    for (const auto index : order) input.append(entries[index]);
    const auto result = normalizeFileStagingEntries(input, &source);
    QCOMPARE(result.size(), 2);
    QVERIFY(result.contains(parent));
    QVERIFY(result.contains(otherVolume));
    QVERIFY(std::ranges::all_of(result, [](const auto& entry) { return entry.fileTarget.has_value(); }));
    QCOMPARE(normalizeFileStagingEntries(result, &source), result);
  } while (std::ranges::next_permutation(order).found);
}

void FileStagingPolicyTests::offlineGuidIdentitySupportsCoverageWithoutGuessingDriveBindings() {
  using namespace uwf::app;
  using namespace uwf::test::application;
  const auto prefix = QString::fromStdString(uwf::compat::volumePath(fixtureVolumeId));
  const FileStagingEntry parent{FileStagingKind::Directory, prefix + QStringLiteral("Data")};
  const FileStagingEntry child{FileStagingKind::File, prefix + QStringLiteral("Data\\state.bin")};
  const FileStagingEntry drive{FileStagingKind::File, QStringLiteral("C:\\Data\\state.bin")};
  StagingAliasSource source;
  source.offline = true;
  const auto result = normalizeFileStagingEntries({child, parent, drive}, &source);
  QCOMPARE(result.size(), 2);
  QVERIFY(result.contains(parent));
  QVERIFY(result.contains(drive));
  const auto boundParent = std::ranges::find_if(result, [&](const auto& entry) { return entry.path == parent.path; });
  QVERIFY(boundParent != result.end());
  QVERIFY(boundParent->fileTarget);
  const auto unboundDrive = std::ranges::find_if(result, [&](const auto& entry) { return entry.path == drive.path; });
  QVERIFY(unboundDrive != result.end());
  QVERIFY(!unboundDrive->fileTarget);
  QCOMPARE(normalizeFileStagingEntries({child, {FileStagingKind::Directory, prefix}}).size(), 2);
}

void FileStagingPolicyTests::mountedAliasesShareObjectSetAndDirectoryCoverage() {
  using namespace uwf::app;
  using namespace uwf::test::application;
  const FileStagingEntry alias{FileStagingKind::File, QStringLiteral("C:\\MountedD\\Data\\state.bin"),
                               FileCommitTarget{otherFixtureVolumeId, QStringLiteral("\\Data\\state.bin")}};
  const FileStagingEntry direct{FileStagingKind::File, QStringLiteral("D:\\Data\\state.bin"), alias.fileTarget};
  const FileStagingEntry parent{FileStagingKind::Directory, QStringLiteral("C:\\MountedD\\Data"),
                                FileCommitTarget{otherFixtureVolumeId, QStringLiteral("\\Data")}};
  const FileStagingEntry otherVolume{FileStagingKind::File, QStringLiteral("C:\\Data\\state.bin"),
                                     FileCommitTarget{fixtureVolumeId, QStringLiteral("\\Data\\state.bin")}};
  const auto aliases = normalizeFileStagingEntries({alias, direct});
  QCOMPARE(aliases, QList<FileStagingEntry>{alias});
  QCOMPARE(aliases.front().fileTarget, alias.fileTarget);
  const auto covered = normalizeFileStagingEntries({alias, direct, parent, otherVolume});
  QCOMPARE(covered.size(), 2);
  QVERIFY(covered.contains(parent));
  QVERIFY(covered.contains(otherVolume));
}

void FileStagingPolicyTests::mountedAliasesShareConflictAndReviewIdentity() {
  using namespace uwf;
  using namespace app;
  using namespace test::application;
  const FileStagingEntry alias{FileStagingKind::File, QStringLiteral("C:\\MountedD\\Data\\state.bin"),
                               FileCommitTarget{otherFixtureVolumeId, QStringLiteral("\\Data\\state.bin")}};
  FileStagingConflictPolicy policy;
  policy.setVolumeIdentity(QStringLiteral("D:"), otherFixtureVolumeId);
  policy.setFileExclusions({QStringLiteral("D:\\Data")});
  policy.setStagedEntries({alias});
  QCOMPARE(policy.conflictingExclusion(alias), std::optional{QStringLiteral("D:\\Data")});
  QCOMPARE(policy.conflictingStagedPath(QStringLiteral("D:\\Data")), std::optional{alias.path});

  ConfigurationPlan plan;
  plan.staging = FileStagingChanges{{}, {alias}, false};
  plan.snapshot.current.volumes = {{fixtureVolumeId, "C:", false, core::VolumeBinding::DriveLetter, std::string("C:")},
                                   {otherFixtureVolumeId, "D:", false, core::VolumeBinding::DriveLetter, std::string("D:")}};
  plan.snapshot.current.fileExclusions[otherFixtureVolumeId] = {"\\Data"};
  // A saved letter does not move A to the physical volume mounted at D:.
  plan.snapshot.next.volumes = {{fixtureVolumeId, "D:", false, core::VolumeBinding::DriveLetter, std::string("C:")}};
  plan.snapshot.next.fileExclusions[fixtureVolumeId] = {"\\Data"};
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, validateFileStagingTarget(plan));
  plan.snapshot.current.fileExclusions.clear();
  validateFileStagingTarget(plan);
  // A management pattern on C retains C's identity and its literal suffix.
  plan.snapshot.current.fileExclusions[fixtureVolumeId] = {"\\MountedD\\Data"};
  validateFileStagingTarget(plan);
}

void FileStagingPolicyTests::conflictObservationsRefreshWithoutChangingStoredPaths() {
  using namespace uwf::app;
  using namespace uwf::test::application;
  FileStagingConflictPolicy policy;
  policy.setVolumeIdentity(QStringLiteral("D:"), otherFixtureVolumeId);
  FileStagingEntry entry{FileStagingKind::File, QStringLiteral("C:\\MountedD\\state.bin"), FileCommitTarget{fixtureVolumeId, QStringLiteral("\\state.bin")}};
  policy.setStagedEntries({entry});
  QVERIFY(!policy.conflictingStagedPath(QStringLiteral("D:\\state.bin")));
  entry.fileTarget->volumeId = otherFixtureVolumeId;
  policy.setStagedEntries({entry});
  QCOMPARE(policy.conflictingStagedPath(QStringLiteral("D:\\state.bin")), std::optional{entry.path});
}

void FileStagingPolicyTests::invalidStoredParentsCannotHideValidSelections() {
  using namespace uwf::app;
  const FileStagingEntry root{FileStagingKind::Directory, QStringLiteral("C:\\")};
  const FileStagingEntry valid{FileStagingKind::File, QStringLiteral("C:\\Data\\saved.bin")};
  const auto result = normalizeFileStagingEntries({valid, root});
  QCOMPARE(result, QList<FileStagingEntry>({root, valid}));
}

void FileStagingPolicyTests::entrySetNormalizationIsOrderIndependent_data() {
  QTest::addColumn<bool>("withParent");
  QTest::newRow("independent-files") << false;
  QTest::newRow("directory-subsumes-descendants") << true;
}
void FileStagingPolicyTests::entrySetNormalizationIsOrderIndependent() {
  QFETCH(bool, withParent);
  using namespace uwf::app;
  const FileStagingEntry root{withParent ? FileStagingKind::Directory : FileStagingKind::File, QStringLiteral("C:\\Data\\Tree")};
  const FileStagingEntry child{FileStagingKind::File, QStringLiteral("C:/Data/Tree/Nested/../child.bin")};
  const FileStagingEntry nested{FileStagingKind::Directory, QStringLiteral("C:\\Data\\Tree\\Nested")};
  const FileStagingEntry sibling{FileStagingKind::File, QStringLiteral("C:\\Data\\Tree!\\sibling.bin")};
  const FileStagingEntry disconnected{FileStagingKind::File, QStringLiteral("Z:\\Offline\\saved.bin")};
  const std::array entries{root, child, nested, sibling, disconnected};
  const QList<FileStagingEntry> expected =
      withParent ? QList<FileStagingEntry>{sibling, root, disconnected} : QList<FileStagingEntry>{sibling, root, child, nested, disconnected};
  std::set<FileStagingEntryKey> expectedKeys;
  for (const auto& entry : expected) expectedKeys.insert(fileStagingEntryKey(entry));
  std::array<std::size_t, entries.size()> order{0, 1, 2, 3, 4};
  do {
    QList<FileStagingEntry> input;
    for (const auto index : order) input.append(entries[index]);
    // Repeating a selection and re-normalizing a result are both idempotent.
    input.append(input);
    const auto result = normalizeFileStagingEntries(input);
    std::set<FileStagingEntryKey> actual;
    for (const auto& entry : result) actual.insert(fileStagingEntryKey(entry));
    QCOMPARE(actual, expectedKeys);
    QCOMPARE(result.size(), static_cast<qsizetype>(expectedKeys.size()));
    QCOMPARE(normalizeFileStagingEntries(result), result);
  } while (std::ranges::next_permutation(order).found);
}
void FileStagingPolicyTests::laterKindChangesRetainOneObject_data() {
  QTest::addColumn<bool>("directoryFirst");
  QTest::newRow("file-to-directory") << false;
  QTest::newRow("directory-to-file") << true;
}
void FileStagingPolicyTests::laterKindChangesRetainOneObject() {
  QFETCH(bool, directoryFirst);
  using namespace uwf::app;
  const FileStagingEntry first{directoryFirst ? FileStagingKind::Directory : FileStagingKind::File, QStringLiteral("C:\\Data\\Object")};
  const FileStagingEntry latest{directoryFirst ? FileStagingKind::File : FileStagingKind::Directory, QStringLiteral("c:/data/object")};
  const auto result = normalizeFileStagingEntries({first, latest});
  QCOMPARE(result.size(), 1);
  QCOMPARE(result.front().kind, latest.kind);
  QCOMPARE(result.front().path, uwf::app::paths::normalize(latest.path));
  const auto repeated = normalizeFileStagingEntries({first, {first.kind, QStringLiteral("c:/data/object")}});
  QCOMPARE(repeated, QList<FileStagingEntry>{first});
}

void FileStagingPolicyTests::fileStagingConflictsDoNotReinterpretStoredPatterns_data() {
  QTest::addColumn<QString>("pattern");
  QTest::addColumn<bool>("expected");
  QTest::addColumn<uwf::app::FileStagingKind>("kind");
  for (const auto kind : {uwf::app::FileStagingKind::File, uwf::app::FileStagingKind::Directory}) {
    const auto add = [&](const char* name, const QString& pattern, const bool expected) {
      QTest::newRow(qPrintable(QStringLiteral("%1/kind-%2").arg(QString::fromLatin1(name)).arg(int(kind)))) << pattern << expected << kind;
    };
    add("native", QStringLiteral("C:\\Data\\Cache"), true);
    add("forward-slashes", QStringLiteral("C:/Data/Cache"), false);
    add("mixed-separators", QStringLiteral("C:\\Data/Cache"), false);
    add("dot-component", QStringLiteral("C:\\Data\\.\\Cache"), false);
    add("parent-component", QStringLiteral("C:\\Data\\Other\\..\\Cache"), false);
  }
}
void FileStagingPolicyTests::fileStagingConflictsDoNotReinterpretStoredPatterns() {
  QFETCH(QString, pattern);
  QFETCH(bool, expected);
  QFETCH(uwf::app::FileStagingKind, kind);
  const auto path = QStringLiteral("C:\\Data\\Cache\\Child");
  uwf::app::FileStagingConflictPolicy policy;
  policy.setFileExclusions({pattern});
  policy.setStagedEntries({{kind, path}});
  const auto exclusion = policy.conflictingExclusion(path, kind);
  const auto staged = policy.conflictingStagedPath(pattern);
  const auto finalState = uwf::app::firstFileStagingExclusionConflict({{kind, path}}, {pattern});
  QCOMPARE(exclusion.has_value(), expected);
  QCOMPARE(staged.has_value(), expected);
  QCOMPARE(finalState.has_value(), expected);
  if (expected) {
    QCOMPARE(*exclusion, pattern);
    QCOMPARE(*staged, path);
    QCOMPARE(finalState->exclusion, pattern);
    QCOMPARE(finalState->entry.path, path);
  }
}

void FileStagingPolicyTests::fileStagingConflictsUseTheStoredPatternEffects_data() {
  QTest::addColumn<int>("tails");
  QTest::addColumn<int>("kind");
  QTest::addColumn<QString>("path");
  QTest::addColumn<bool>("expected");
  for (const int tails : {0, 1, 2})
    for (const auto kind : {uwf::app::FileStagingKind::File, uwf::app::FileStagingKind::Directory})
      for (int relation = 0; relation < 3; ++relation) {
        const auto path = relation == 0 ? QStringLiteral("C:\\Data") : relation == 1 ? QStringLiteral("C:\\Data\\Child") : QStringLiteral("C:\\Database");
        const bool expected = relation != 2 && (tails == 0 || (tails == 1 && (relation == 1 || kind == uwf::app::FileStagingKind::Directory)));
        QTest::newRow(qPrintable(QStringLiteral("tails-%1-kind-%2-relation-%3").arg(tails).arg(int(kind)).arg(relation)))
            << tails << int(kind) << path << expected;
      }
}
void FileStagingPolicyTests::fileStagingConflictsUseTheStoredPatternEffects() {
  QFETCH(int, tails);
  QFETCH(int, kind);
  QFETCH(QString, path);
  QFETCH(bool, expected);
  const auto entryKind = static_cast<uwf::app::FileStagingKind>(kind);
  const auto pattern = QStringLiteral("C:\\Data") + QString(tails, u'\\');
  uwf::app::FileStagingConflictPolicy policy;
  policy.setFileExclusions({pattern});
  policy.setStagedEntries({{entryKind, path}});
  const auto existing = policy.conflictingExclusion(path, entryKind);
  const auto added = policy.conflictingStagedPath(pattern);
  QCOMPARE(existing.has_value(), expected);
  QCOMPARE(added.has_value(), expected);
  if (expected) {
    QCOMPARE(*existing, pattern);
    QCOMPARE(*added, path);
  }
}

void FileStagingPolicyTests::fileStagingConflictsRespectCaseAndPathSegments() {
  uwf::app::FileStagingConflictPolicy policy;
  policy.setFileExclusions({QStringLiteral("C:\\Program Data\\Vendor"), QStringLiteral("c:\\program data\\vendor"), QStringLiteral("D:\\Shared")});
  policy.setStagedEntries(
      {{uwf::app::FileStagingKind::File, QStringLiteral("C:\\Cache\\state.bin")}, {uwf::app::FileStagingKind::Directory, QStringLiteral("D:/Work/Tree")}});

  const auto vendorConflict = policy.conflictingExclusion(QStringLiteral("c:\\PROGRAM DATA\\Vendor\\state.db"), uwf::app::FileStagingKind::File);
  QVERIFY(vendorConflict.has_value());
  QCOMPARE(vendorConflict->compare(QStringLiteral("C:\\Program Data\\Vendor"), Qt::CaseInsensitive), 0);
  QCOMPARE(policy.conflictingExclusion(QStringLiteral("D:\\Shared"), uwf::app::FileStagingKind::Directory), std::optional{QStringLiteral("D:\\Shared")});
  QVERIFY(!policy.conflictingExclusion(QStringLiteral("C:\\Program Database\\Vendor"), uwf::app::FileStagingKind::Directory).has_value());
  QVERIFY(!policy.conflictingExclusion(QStringLiteral("E:\\Shared"), uwf::app::FileStagingKind::Directory).has_value());

  QCOMPARE(policy.conflictingStagedPath(QStringLiteral("C:\\Cache")), std::optional{QStringLiteral("C:\\Cache\\state.bin")});
  QCOMPARE(policy.conflictingStagedPath(QStringLiteral("d:\\work\\tree\\child")), std::optional{QStringLiteral("D:\\Work\\Tree")});
  QVERIFY(!policy.conflictingStagedPath(QStringLiteral("C:\\Caches")).has_value());
}

void FileStagingPolicyTests::exclusionUpdatesKeepFirstSpellingAndDistinctNonAsciiNames() {
  using namespace uwf::app;
  const QString first = QStringLiteral("Z:\\Cache\\Mixed");
  const QString sharp = QStringLiteral("Z:\\Cache\\Stra\u00dfe");
  const QString plain = QStringLiteral("Z:\\Cache\\STRASSE");
  FileStagingConflictPolicy policy;
  policy.setFileExclusions({{}, first, first.toLower(), sharp, plain});
  QCOMPARE(policy.conflictingExclusion(first.toLower() + QStringLiteral("\\state.bin"), FileStagingKind::File), std::optional{first});
  QCOMPARE(policy.conflictingExclusion(sharp + QStringLiteral("\\state.bin"), FileStagingKind::File), std::optional{sharp});
  QCOMPARE(policy.conflictingExclusion(plain + QStringLiteral("\\state.bin"), FileStagingKind::File), std::optional{plain});
  policy.setFileExclusions({});
  QVERIFY(!policy.conflictingExclusion(first, FileStagingKind::Directory));
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
  policy.setFileExclusions({canonical});
  policy.setStagedEntries({{uwf::app::FileStagingKind::Directory, stored}});
  const auto exclusion = policy.conflictingExclusion(candidate, uwf::app::FileStagingKind::Directory);
  const auto staged = policy.conflictingStagedPath(uwf::app::paths::normalize(candidate));
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
