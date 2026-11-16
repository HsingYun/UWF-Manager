// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QStringView>
#include <QTest>
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "domain/uwf/contracts/Types.h"
#include "domain/uwf/model/UwfModel.h"
#include "domain/uwf/policies/ExclusionCoverage.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"
#include "domain/uwf/policies/FileExclusionPolicy.h"
#include "domain/uwf/policies/IndividualCommitPolicy.h"
#include "domain/uwf/policies/OverlayWritePlan.h"
#include "domain/uwf/policies/RegistryCommitPolicy.h"
#include "domain/uwf/policies/RegistryExclusionPolicy.h"
#include "domain/uwf/policies/UwfConstraints.h"
#include "foundation/text/StringUtil.h"

namespace {

class DomainPolicyTests final : public QObject {
  Q_OBJECT

 private slots:
  void publicVolumeIdentityRejectsTruncationAndNonGuidText();
  void overlayWritableTargetDomainPreservesAbnormalObservations();
  void individualCommitRestrictionsFollowCurrentProtection();
  void registryWritesSeparateFuturePersistenceFromHistoricalCommits();
  void configurationPersistenceSeparatesOrdinaryWritesThresholdsAndTransitions();
  void configurationPersistenceUsesCurrentFactsAndPreservesUnknownMembership();
  void fileExclusionNativeNames_data();
  void fileExclusionNativeNames();
  void registryExclusionNativeNames_data();
  void registryExclusionNativeNames();
  void registryExclusionEditorTargets_data();
  void registryExclusionEditorTargets();
  void registryCommitTargets_data();
  void registryCommitTargets();
  void registryCommitPreservesExactTargets_data();
  void registryCommitPreservesExactTargets();
  void exclusionManagementAndDriverEffectsRemainDistinct_data();
  void exclusionManagementAndDriverEffectsRemainDistinct();
  void registryDriverResolvesTheSelectedControlSet_data();
  void registryDriverResolvesTheSelectedControlSet();
  void registryObjectNamesResolveOnlyTheCurrentControlSetLink();
  void registryControlSetContextIsRequiredOnlyForRelevantComparisons_data();
  void registryControlSetContextIsRequiredOnlyForRelevantComparisons();
  void pendingChangesCountsEveryBusinessField();
  void exclusionSortingIsCaseInsensitiveAndStable();
  void overlayUsageBoundaries();
};

void DomainPolicyTests::publicVolumeIdentityRejectsTruncationAndNonGuidText() {
  using namespace uwf::compat;
  const std::string guid = "ABCDEF01-2345-6789-ABCD-EF0123456789";
  for (const auto& value : {guid, "{" + guid + "}", "Volume{" + guid + "}", "\\\\?\\Volume{" + guid + "}\\"}) {
    const auto parsed = parseVolumeId(value);
    QVERIFY(parsed);
    QCOMPARE(*parsed, std::string("abcdef01-2345-6789-abcd-ef0123456789"));
  }
  for (const auto& value : std::vector<std::string>{"", "volume", "C:", " " + guid, guid + " ", guid + "suffix", guid + '\0' + "suffix", guid + "\\",
                                                    "\\\\?\\Volume{" + guid + "}\\\\", "Volume{" + guid + "}/", "abcdef01_2345-6789-abcd-ef0123456789",
                                                    "gbcdef01-2345-6789-abcd-ef0123456789", "\\\\?\\{" + guid + "}\\", guid + std::string("\xc0\x80", 2)})
    QVERIFY2(!parseVolumeId(value), "Malformed identity must never become a different C-string target");
}

void DomainPolicyTests::overlayWritableTargetDomainPreservesAbnormalObservations() {
  using namespace uwf::core;
  QVERIFY(!validOverlayTarget(OverlayType::RAM, 0));
  QVERIFY(validOverlayTarget(OverlayType::RAM, 1));
  QVERIFY(!validOverlayTarget(OverlayType::Disk, 1023));
  QVERIFY(validOverlayTarget(OverlayType::Disk, 1024));
  QVERIFY(!validOverlayTarget(OverlayType::Unknown, 1024));
  const OverlayConfig observed{.type = OverlayType::RAM, .maximumSizeMb = 0};
  validateOverlayTarget(observed, {});  // Reading an abnormal value is not a repair.
  OverlayConfigDelta thresholds;
  thresholds.warningThresholdMb = 0;
  validateOverlayTarget(observed, thresholds);
  QCOMPARE(observed.maximumSizeMb, uint32_t{0});
  OverlayConfigDelta zeroSize;
  zeroSize.maximumSizeMb = 0;
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, validateOverlayTarget(observed, zeroSize));
  OverlayConfigDelta sameType;
  sameType.type = OverlayType::RAM;
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)planOverlayWrites(observed, sameType));
  OverlayConfigDelta positiveSize;
  positiveSize.maximumSizeMb = 1;
  validateOverlayTarget(observed, positiveSize);
  const OverlayConfig invalidDisk{.type = OverlayType::Disk, .maximumSizeMb = 0};
  OverlayConfigDelta repairTarget;
  repairTarget.type = OverlayType::RAM;
  repairTarget.maximumSizeMb = 1;
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, validateOverlayTarget(invalidDisk, repairTarget));
  repairTarget.maximumSizeMb = 1024;
  const auto repair = planOverlayWrites(invalidDisk, repairTarget);
  QCOMPARE(repair.front().field, OverlayField::MaximumSize);
  QCOMPARE(repair.back().field, OverlayField::Type);
}

void DomainPolicyTests::individualCommitRestrictionsFollowCurrentProtection() {
  using namespace uwf;
  using enum core::IndividualCommitRestriction;
  struct Case {
    core::IndividualCommitState state;
    core::IndividualCommitRestriction expected;
  };
  const std::array cases{Case{{false, 0, 0, false}, None},
                         Case{{false, 0x80, 0, false}, None},
                         Case{{false, 4, 0, false}, ReadOnlyMediaActive},
                         Case{{false, 0x84, 0, false}, ReadOnlyMediaActive},
                         Case{{false, 4, 1, false}, None},
                         Case{{false, 4, 2, false}, None},
                         Case{{false, 4, 3, false}, None},
                         Case{{false, 4, 4, true}, ReadOnlyMediaActive},
                         Case{{false, 4, 7, true}, ReadOnlyMediaActive},
                         Case{{false, 4, std::nullopt, true}, ReadOnlyMediaActive},
                         Case{{false, 4, std::nullopt, std::nullopt}, None},
                         Case{{true, 0, 0, false}, HormActive},
                         Case{{true, 4, 3, false}, HormActive},
                         Case{{std::nullopt, 4, 0, false}, ReadOnlyMediaActive},
                         Case{{}, None}};
  for (const auto& item : cases)
    for (const bool filter : {false, true})
      for (const bool protectedTarget : {false, true})
        QCOMPARE(core::individualCommitRestriction(filter, protectedTarget, item.state), filter && protectedTarget ? item.expected : None);
  QCOMPARE(core::effectiveReadOnlyMedia({false, 4, 3, false}), std::optional{false});
  QCOMPARE(core::effectiveReadOnlyMedia({false, 4, 4, true}), std::optional{true});
  QCOMPARE(core::individualCommitRestriction(true, true, {false, 0, 0, false}, false), PersistentViewUnavailable);
  QCOMPARE(core::individualCommitRestriction(true, false, {false, 0, 0, false}, false), None);
}

void DomainPolicyTests::registryWritesSeparateFuturePersistenceFromHistoricalCommits() {
  using namespace uwf;
  using D = core::RegistryWriteDisposition;
  struct Case {
    const char* name;
    core::ConfigurationModeState state;
    D expected;
  };
  const std::array cases{
      Case{"ordinary-protected", {true, true, {false, 0, 0, false}}, D::RequiresCommit},
      Case{"ROM-protected", {true, true, {false, 4, 0, false}}, D::ReadOnlyMediaActive},
      Case{"HORM-protected", {true, true, {true, 4, 2, false}}, D::HormActive},
      Case{"FullCommit", {true, true, {false, 4, 4, true}}, D::Direct},
      Case{"FullCommit-bit-receipt", {true, true, {false, 4, 4, {}}}, D::Direct},
      Case{"FullCommit-flags-unavailable", {true, true, {false, {}, 4, true}}, D::Direct},
      Case{"HORM-precedes-FullCommit", {true, true, {true, 4, 4, true}}, D::HormActive},
      Case{"filter-off", {false, {}, {}}, D::Direct},
      Case{"system-unprotected", {true, false, {true, 4, 2, false}}, D::Direct},
      Case{"unprotected-needs-no-filter-observation", {{}, false, {}}, D::Direct},
      Case{"filter-unknown", {{}, true, {false, 0, 0, false}}, D::Unknown},
      Case{"system-protection-unknown", {true, {}, {false, 0, 0, false}}, D::Unknown},
      Case{"HORM-unknown", {true, true, {{}, 4, 4, true}}, D::Unknown},
      Case{"loaded-flags-unknown", {true, true, {false, {}, 0, false}}, D::Unknown},
      Case{"ROM-disposition-unknown", {true, true, {false, 4, {}, false}}, D::Unknown},
      Case{"filter-transition", {true, true, {false, 4, 1, false}}, D::RequiresCommit},
      Case{"HORM-disabled", {true, true, {false, 4, 3, false}}, D::RequiresCommit},
  };
  for (const auto& item : cases) QVERIFY2(core::registryWriteDisposition(item.state) == item.expected, item.name);
  QCOMPARE(core::individualCommitRestriction(true, true, {false, 4, 4, true}), core::IndividualCommitRestriction::ReadOnlyMediaActive);
}

void DomainPolicyTests::configurationPersistenceSeparatesOrdinaryWritesThresholdsAndTransitions() {
  using namespace uwf;
  using R = core::ConfigurationWriteRestriction;
  struct Case {
    const char* name;
    core::ConfigurationModeState state;
    R ordinary, threshold;
  };
  const std::array cases{
      Case{"normal", {true, true, {false, 0, 0, false}}, R::None, R::None},
      Case{"ROM-protected-system", {true, true, {false, 4, 0, false}}, R::RomVolatile, R::RomCommitUnavailable},
      Case{"ROM-unprotected-system", {true, false, {false, 4, 0, false}}, R::None, R::None},
      Case{"filter-off", {false, true, {false, 4, 0, false}}, R::None, R::None},
      Case{"HORM", {true, true, {true, 4, 2, false}}, R::HormActive, R::HormActive},
      Case{"filter-transition", {true, true, {false, 4, 1, false}}, R::None, R::None},
      Case{"HORM-disabled-same-boot", {true, true, {false, 4, 3, false}}, R::None, R::None},
      Case{"FullCommit", {true, true, {false, 4, 4, true}}, R::None, R::RomCommitUnavailable},
      Case{"FullCommit-bit-receipt", {true, true, {false, 4, 4, std::nullopt}}, R::None, R::RomCommitUnavailable},
      Case{"filter-unknown", {{}, true, {false, 4, 0, false}}, R::Unknown, R::Unknown},
      Case{"system-protection-unknown", {true, {}, {false, 4, 0, false}}, R::Unknown, R::Unknown},
      Case{"loaded-flags-unknown", {true, true, {false, {}, 0, false}}, R::Unknown, R::Unknown},
      Case{"disposition-unknown", {true, true, {false, 4, {}, false}}, R::Unknown, R::Unknown},
  };
  for (const auto& item : cases) {
    QVERIFY2(core::configurationWriteRestriction(item.state) == item.ordinary, item.name);
    QVERIFY2(core::configurationWriteRestriction(item.state, core::ConfigurationWrite::Threshold) == item.threshold, item.name);
    QCOMPARE(QString::fromLatin1(core::configurationWriteReason(item.ordinary)).isEmpty(), item.ordinary == R::None);
  }
  const std::vector<compat::Request> ordinary{
      compat::SetVolume{"11111111-1111-1111-1111-111111111111", compat::VolumeProperty::Protected, true},
      compat::AddVolume{"11111111-1111-1111-1111-111111111111", "C:"},
      compat::RemoveVolume{"11111111-1111-1111-1111-111111111111"},
      compat::EditFileExclusion{"11111111-1111-1111-1111-111111111111", "\\Data", compat::ExclusionAction::Add},
      compat::EditRegistryExclusion{"HKEY_LOCAL_MACHINE\\SOFTWARE\\Example", compat::ExclusionAction::Add},
      compat::SetBoolean{compat::BooleanProperty::PersistDomainSecretKey, true},
      compat::SetBoolean{compat::BooleanProperty::PersistTscal, true},
      compat::SetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, 1},
  };
  for (const auto& request : ordinary) QCOMPARE(compat::configurationWrite(request), std::optional{core::ConfigurationWrite::Ordinary});
  for (const auto property : {compat::NumberProperty::WarningThreshold, compat::NumberProperty::CriticalThreshold})
    QCOMPARE(compat::configurationWrite(compat::SetNumber{property, 128}), std::optional{core::ConfigurationWrite::Threshold});
  for (const auto property : {compat::BooleanProperty::FilterEnabled, compat::BooleanProperty::HormEnabled, compat::BooleanProperty::OverlayCommit})
    for (const bool requested : {false, true}) QVERIFY(!compat::configurationWrite(compat::SetBoolean{property, requested}));
}

void DomainPolicyTests::configurationPersistenceUsesCurrentFactsAndPreservesUnknownMembership() {
  using namespace uwf;
  using R = core::ConfigurationWriteRestriction;
  core::UwfSnapshot snapshot;
  snapshot.current.filter.enabled = true;
  snapshot.runtime.systemVolumeProtected = true;
  snapshot.runtime.hormEnabled = false;
  snapshot.runtime.currentOverlayFlags = 4;
  snapshot.runtime.romDisposition = 0;
  snapshot.runtime.fullCommit = false;
  for (const bool nextFilter : {false, true})
    for (const auto nextType : {core::OverlayType::RAM, core::OverlayType::Disk}) {
      snapshot.next.filter.enabled = nextFilter;
      snapshot.next.overlay.type = nextType;
      snapshot.next.volumes = {{"Volume{11111111-1111-1111-1111-111111111111}", "C:", !nextFilter}};
      QCOMPARE(core::configurationWriteRestriction(core::configurationModeState(snapshot)), R::RomVolatile);
    }
  snapshot.runtime.currentOverlayFlags = 0;
  snapshot.next.overlay.type = core::OverlayType::RAM;
  QCOMPARE(core::configurationWriteRestriction(core::configurationModeState(snapshot)), R::None);

  constexpr auto system = "11111111-1111-1111-1111-111111111111";
  compat::UwfStateFacts facts;
  facts.current.filterEnabled = compat::Fact<bool>::known(true);
  facts.current.overlayFlags = compat::BitFacts::known(4);
  facts.runtime.romDisposition = compat::BitFacts::known(0);
  facts.runtime.horm = compat::Fact<bool>::known(false);
  facts.runtime.fullCommit = compat::Fact<bool>::known(false);
  facts.systemVolumeId = compat::Fact<std::string>::known(system);
  auto& volume = facts.current.volumes[system];
  volume.protectedVolume = compat::Fact<bool>::known(true);
  QVERIFY(!compat::configurationModeState(facts).systemVolumeProtected);
  QCOMPARE(core::configurationWriteRestriction(compat::configurationModeState(facts)), R::Unknown);
  volume.present = compat::Fact<bool>::known(true);
  QCOMPARE(core::configurationWriteRestriction(compat::configurationModeState(facts)), R::RomVolatile);
  volume.present = compat::Fact<bool>::known(false);
  QCOMPARE(core::configurationWriteRestriction(compat::configurationModeState(facts)), R::None);
  facts.current.volumes.clear();
  QVERIFY(!compat::configurationModeState(facts).systemVolumeProtected);
  facts.current.volumesObserved = true;
  QCOMPARE(compat::configurationModeState(facts).systemVolumeProtected, std::optional{false});
}

void DomainPolicyTests::overlayUsageBoundaries() {
  using namespace uwf::core;
  using enum OverlayUsageSeverity;
  struct Boundary {
    uint32_t used, warning, critical;
    OverlayUsageSeverity expected;
  };
  const std::array boundaries{Boundary{0, 4, 8, Normal},   Boundary{3, 4, 8, Normal},
                              Boundary{4, 4, 8, Warning},  Boundary{7, 4, 8, Warning},
                              Boundary{8, 4, 8, Critical}, Boundary{9, 4, 8, Critical},
                              Boundary{8, 0, 8, Critical}, Boundary{4, 4, 0, Warning},
                              Boundary{4, 8, 4, Critical}, Boundary{std::numeric_limits<uint32_t>::max(), 0, 0, Normal}};
  for (const auto& boundary : boundaries) QCOMPARE(overlayUsageSeverity(boundary.used, boundary.warning, boundary.critical), boundary.expected);
  const auto maximum = std::numeric_limits<uint32_t>::max();
  for (const uint32_t available : {0u, 1u, maximum})
    for (const uint32_t used : {0u, 1u, maximum}) {
      const auto total = overlayTotalMb({available, used});
      QCOMPARE(total - used, static_cast<uint64_t>(available));
      QVERIFY(total >= used && total >= available);
    }
}

void DomainPolicyTests::fileExclusionNativeNames_data() {
  QTest::addColumn<QString>("path");
  QTest::addColumn<int>("error");
  using enum uwf::core::FileExclusionError;
  const auto add = [](const char* label, const QString& path, const uwf::core::FileExclusionError error) { QTest::newRow(label) << path << int(error); };
  add("empty", {}, Empty);
  add("ordinary", QStringLiteral("\\Data\\Entry.bin"), None);
  add("literal-space-dot-separators", QStringLiteral("\\Data\\\\Entry. \\"), None);
  add("ordinary-260", QStringLiteral("\\") + QString(259, u'X'), None);
  add("ordinary-261", QStringLiteral("\\") + QString(260, u'X'), TooLong);
  const auto pair = QString::fromUcs4(U"\U0001F642");
  add("ordinary-utf16-260", QStringLiteral("\\") + pair.repeated(129) + u'X', None);
  add("ordinary-utf16-261", QStringLiteral("\\") + pair.repeated(130), TooLong);
  const auto extended = QStringLiteral("\\\\?\\");
  add("extended-empty", extended, Empty);
  add("extended-body-32767", extended + QString(32767, u'X'), None);
  add("extended-body-32768", extended + QString(32768, u'X'), TooLong);
  add("extended-utf16-32767", extended + pair.repeated(16383) + u'X', None);
  add("extended-utf16-32768", extended + pair.repeated(16384), TooLong);
  add("device-prefix-260", QStringLiteral("\\\\.\\") + QString(256, u'X'), None);
  add("device-prefix-261", QStringLiteral("\\\\.\\") + QString(257, u'X'), TooLong);
  add("prefix-is-case-and-character-exact", QStringLiteral("\\\\?/") + QString(300, u'X'), InvalidCharacter);
  add("control-001e", QStringLiteral("\\Before") + QChar(0x1e) + QStringLiteral("After"), InvalidCharacter);
  add("control-001f", QStringLiteral("\\Before") + QChar(0x1f) + QStringLiteral("After"), None);
  add("space-0020", QStringLiteral("\\Before After"), None);
  add("delete-007f", QStringLiteral("\\Before") + QChar(0x7f) + QStringLiteral("After"), None);
  add("embedded-null", QStringLiteral("\\Before") + QChar(0) + QStringLiteral("After"), EmbeddedNull);
  for (const QChar character : QStringView(u"<>:\"/|?*")) {
    const auto path = QStringLiteral("\\Before") + character + QStringLiteral("After");
    QTest::newRow(qPrintable(QStringLiteral("forbidden-%1").arg(static_cast<quint32>(character.unicode()), 4, 16, u'0'))) << path << int(InvalidCharacter);
    QTest::newRow(qPrintable(QStringLiteral("extended-forbidden-%1").arg(static_cast<quint32>(character.unicode()), 4, 16, u'0')))
        << extended + path << int(InvalidCharacter);
  }
}
void DomainPolicyTests::fileExclusionNativeNames() {
  QFETCH(QString, path);
  QFETCH(int, error);
  QCOMPARE(int(uwf::core::fileExclusionError({QStringView(path).utf16(), static_cast<std::size_t>(path.size())})), error);
}

void DomainPolicyTests::registryExclusionNativeNames_data() {
  QTest::addColumn<QString>("key");
  QTest::addColumn<int>("error");
  using enum uwf::core::RegistryExclusionError;
  const auto add = [](const char* name, const QString& key, const uwf::core::RegistryExclusionError error) { QTest::newRow(name) << key << int(error); };
  add("empty", {}, MachineSubkeyRequired);
  add("root", QStringLiteral("HKLM"), MachineSubkeyRequired);
  add("empty-components-only", QStringLiteral("HKLM\\\\\\"), MachineSubkeyRequired);
  add("other-hive", QStringLiteral("HKCU\\Software\\Name"), MachineSubkeyRequired);
  add("slash-name", QStringLiteral("hklm\\SOFTWARE\\Slash/Name"), None);
  add("control-name", QStringLiteral("HKLM\\SOFTWARE\\Control") + QChar(1) + QStringLiteral("Name"), None);
  add("newline-name", QStringLiteral("HKLM\\SOFTWARE\\Line\nName"), None);
  add("repeated-separators", QStringLiteral("HKLM\\\\SOFTWARE\\\\Name\\\\"), None);
  add("component-255", QStringLiteral("HKLM\\") + QString(255, u'X'), None);
  add("component-256", QStringLiteral("HKLM\\") + QString(256, u'X'), ComponentTooLong);
  const auto pair = QString::fromUcs4(U"\U0001F642");
  add("utf16-units-255", QStringLiteral("HKLM\\") + pair.repeated(127) + u'X', None);
  add("utf16-units-256", QStringLiteral("HKLM\\") + pair.repeated(128), ComponentTooLong);
  add("components-512", QStringLiteral("HKLM\\") + QStringLiteral("X\\").repeated(511) + u'X', None);
  add("components-513", QStringLiteral("HKLM\\") + QStringLiteral("X\\").repeated(512) + u'X', TooManyComponents);
  add("empty-components-not-counted", QStringLiteral("HKLM\\\\") + QStringLiteral("X\\\\").repeated(511) + QStringLiteral("X\\\\"), None);
  add("embedded-null", QStringLiteral("HKLM\\SOFTWARE\\Before") + QChar(0) + QStringLiteral("After"), EmbeddedNull);
  add("machine-account-is-native-valid", QStringLiteral("HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC"), None);
  for (const auto& relative : {QStringLiteral("SYSTEM"), QStringLiteral("SYSTEM\\CurrentControlSet"), QStringLiteral("SOFTWARE"), QStringLiteral("SAM"),
                               QStringLiteral("SECURITY"), QStringLiteral("COMPONENTS")}) {
    const auto key = QStringLiteral("HKLM\\") + relative;
    QTest::newRow(qUtf8Printable(relative + QStringLiteral("/exact"))) << key << int(Reserved);
    QTest::newRow(qUtf8Printable(relative + QStringLiteral("/child"))) << key + QStringLiteral("\\Child") << int(None);
    QTest::newRow(qUtf8Printable(relative + QStringLiteral("/tail"))) << key + u'\\' << int(None);
  }
}
void DomainPolicyTests::registryExclusionNativeNames() {
  QFETCH(QString, key);
  QFETCH(int, error);
  QCOMPARE(uwf::core::registryExclusionError({QStringView(key).utf16(), static_cast<std::size_t>(key.size())}),
           static_cast<uwf::core::RegistryExclusionError>(error));
}

void DomainPolicyTests::registryExclusionEditorTargets_data() {
  QTest::addColumn<QString>("key");
  QTest::addColumn<int>("classification");
  using enum uwf::core::RegExclusionClass;
  for (const auto& reserved : {QStringLiteral("SYSTEM"), QStringLiteral("SYSTEM\\CurrentControlSet"), QStringLiteral("SOFTWARE"), QStringLiteral("SAM"),
                               QStringLiteral("SECURITY"), QStringLiteral("COMPONENTS")}) {
    const auto key = QStringLiteral("HKLM\\") + reserved;
    QTest::newRow(qUtf8Printable(reserved + QStringLiteral("/reserved"))) << key << int(Container);
    QTest::newRow(qUtf8Printable(reserved + QStringLiteral("/child"))) << key + QStringLiteral("\\Vendor") << int(Allowed);
    QTest::newRow(qUtf8Printable(reserved + QStringLiteral("/sibling"))) << key + QStringLiteral("Other") << int(Allowed);
    QTest::newRow(qUtf8Printable(reserved + QStringLiteral("/literal-space"))) << key + u' ' << int(Allowed);
    QTest::newRow(qUtf8Printable(reserved + QStringLiteral("/original-tail"))) << key + u'\\' << int(Allowed);
  }
  QTest::newRow("hive-root") << QStringLiteral("HKLM") << int(Container);
  QTest::newRow("hive-root-alias") << QStringLiteral("hkey_local_machine\\") << int(Container);
  QTest::newRow("hardware-subkey") << QStringLiteral("HKLM\\HARDWARE\\Vendor") << int(Allowed);
  QTest::newRow("hardware-root") << QStringLiteral("HKLM\\HARDWARE") << int(Allowed);
  QTest::newRow("bcd-root") << QStringLiteral("HKLM\\BCD00000000") << int(Allowed);
  QTest::newRow("custom-root") << QStringLiteral("HKLM\\Custom") << int(Allowed);
  QTest::newRow("short-prefix") << QStringLiteral("HKLM\\S") << int(Allowed);
  QTest::newRow("lowercase-alias") << QStringLiteral("hkey_local_machine\\system\\CurrentControlSet\\Services\\Demo") << int(Allowed);
  QTest::newRow("machine-account-editor-entry") << QStringLiteral("HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC") << int(MachineAccount);
  QTest::newRow("machine-account-descendant") << QStringLiteral("HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC\\Child") << int(Allowed);
  QTest::newRow("machine-account-literal-space") << QStringLiteral("HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC ") << int(Allowed);
  for (const auto& hive : {QStringLiteral("HKCU"), QStringLiteral("HKCR"), QStringLiteral("HKU"), QStringLiteral("HKCC"), QStringLiteral("HKLMOther")})
    QTest::newRow(qUtf8Printable(hive)) << hive + QStringLiteral("\\Software\\Vendor") << int(OutOfScope);
  QTest::newRow("empty") << QString{} << int(OutOfScope);
}

void DomainPolicyTests::registryExclusionEditorTargets() {
  QFETCH(QString, key);
  QFETCH(int, classification);
  QCOMPARE(uwf::core::classifyRegistryExclusion(key.toStdString()), static_cast<uwf::core::RegExclusionClass>(classification));
}

void DomainPolicyTests::registryCommitTargets_data() {
  QTest::addColumn<QString>("key");
  QTest::addColumn<bool>("valueOperation");
  QTest::addColumn<bool>("allowed");
  for (const auto& hive : {QStringLiteral("HKLM"), QStringLiteral("HKCU"), QStringLiteral("HKCR"), QStringLiteral("HKU"), QStringLiteral("HKCC")}) {
    QTest::newRow(qUtf8Printable(hive + QStringLiteral("/key-root"))) << hive << false << false;
    QTest::newRow(qUtf8Printable(hive + QStringLiteral("/value-root"))) << hive << true << (hive == QStringLiteral("HKLM"));
    QTest::newRow(qUtf8Printable(hive + QStringLiteral("/key-child"))) << hive + QStringLiteral("\\Vendor") << false << true;
    QTest::newRow(qUtf8Printable(hive + QStringLiteral("/value-child"))) << hive + QStringLiteral("\\Vendor") << true << (hive == QStringLiteral("HKLM"));
  }
  for (const auto& relative : {QStringLiteral("SYSTEM"), QStringLiteral("SOFTWARE"), QStringLiteral("SAM"), QStringLiteral("SECURITY"),
                               QStringLiteral("COMPONENTS"), QStringLiteral("S"), QStringLiteral("SOFT")}) {
    const auto key = QStringLiteral("HKLM\\") + relative;
    QTest::newRow(qUtf8Printable(relative + QStringLiteral("/key"))) << key << false << false;
    QTest::newRow(qUtf8Printable(relative + QStringLiteral("/value"))) << key << true << true;
  }
  for (const auto& key :
       {QStringLiteral("HKLM\\SYSTEM\\CurrentControlSet"), QStringLiteral("HKLM\\SYSTEM\\Vendor"), QStringLiteral("HKLM\\SYSTEMOther"),
        QStringLiteral("HKLM\\HARDWARE\\Vendor"), QStringLiteral("HKLM\\BCD00000000"), QStringLiteral("HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC")}) {
    QTest::newRow(qUtf8Printable(key + QStringLiteral("/key"))) << key << false << true;
    QTest::newRow(qUtf8Printable(key + QStringLiteral("/value"))) << key << true << true;
  }
  for (const bool valueOperation : {false, true}) {
    QTest::newRow(valueOperation ? "invalid/value" : "invalid/key") << QStringLiteral("invalid\\Vendor") << valueOperation << false;
    QTest::newRow(valueOperation ? "empty/value" : "empty/key") << QString{} << valueOperation << false;
  }
}

void DomainPolicyTests::registryCommitTargets() {
  QFETCH(QString, key);
  QFETCH(bool, valueOperation);
  QFETCH(bool, allowed);
  QCOMPARE(uwf::core::isRegistryCommitTargetAllowed(key.toStdString(), valueOperation), allowed);
}

void DomainPolicyTests::registryCommitPreservesExactTargets_data() {
  QTest::addColumn<QString>("key");
  QTest::addColumn<bool>("preserved");
  for (const auto& hive : {QStringLiteral("HKLM"), QStringLiteral("hkey_local_machine")})
    for (const auto& marker : {QStringLiteral("SYSTEM\\CurrentControlSet"), QStringLiteral("sYsTeM\\cUrReNtCoNtRoLsEt")})
      for (const auto& suffix : {QString{}, QStringLiteral("\\Services\\Demo"), QStringLiteral("Extra"), QStringLiteral("Extra\\Child"),
                                 QStringLiteral("Backup"), QStringLiteral(" "), QStringLiteral(".")}) {
        const auto key = hive + u'\\' + marker + suffix;
        QTest::newRow(qUtf8Printable(key)) << key << (suffix.isEmpty() || suffix.startsWith(u'\\'));
      }
  for (const auto& key : {QStringLiteral("HKLM\\SYSTEM\\ControlSet001\\Services\\Demo"), QStringLiteral("HKLM\\SYSTEM\\ControlSet002"),
                          QStringLiteral("HKLM\\SYSTEMOther\\CurrentControlSetExtra"), QStringLiteral("HKLM\\SOFTWARE\\CurrentControlSetExtra"),
                          QStringLiteral("HKCU\\SYSTEM\\CurrentControlSetExtra"), QStringLiteral("HKLMOther\\SYSTEM\\CurrentControlSetExtra")})
    QTest::newRow(qUtf8Printable(key)) << key << true;
}

void DomainPolicyTests::registryCommitPreservesExactTargets() {
  QFETCH(QString, key);
  QFETCH(bool, preserved);
  QCOMPARE(uwf::core::preservesRegistryCommitTarget(key.toStdString()), preserved);
}

void DomainPolicyTests::exclusionManagementAndDriverEffectsRemainDistinct_data() {
  QTest::addColumn<bool>("registry");
  QTest::addColumn<QString>("pattern");
  QTest::addColumn<QString>("target");
  QTest::addColumn<bool>("managementSkipsCommit");
  QTest::addColumn<bool>("driverBypassesWrites");
  // Expected values follow IsFileExcluded/IsKeyInExclusionList and the driver's original-pattern prefix table.
  struct Coverage {
    const char* name;
    const char16_t* patternSuffix;
    const char16_t* targetSuffix;
    bool fileManagement, registryManagement, driver;
  };
  constexpr std::array rows{
      Coverage{"plain/exact", u"", u"", true, true, true},
      Coverage{"plain/child", u"", u"\\Child", true, true, true},
      Coverage{"plain/sibling", u"", u"Other\\Child", false, false, false},
      Coverage{"one-tail/exact", u"\\", u"", false, true, false},
      Coverage{"one-tail/child", u"\\", u"\\Child", true, true, false},
      Coverage{"one-tail/sibling", u"\\", u"Other\\Child", false, false, false},
      Coverage{"two-tails/exact", u"\\\\", u"", false, false, false},
      Coverage{"two-tails/child", u"\\\\", u"\\Child", false, false, false},
      Coverage{"two-tails/sibling", u"\\\\", u"Other\\Child", false, false, false},
      Coverage{"original-tail-exact", u"\\", u"\\", true, true, true},
      Coverage{"literal-space/same", u" ", u" \\Child", true, true, true},
      Coverage{"literal-space/different", u" ", u"\\Child", false, false, false},
  };
  for (const bool registry : {false, true}) {
    const auto prefix = registry ? QStringLiteral("HKLM\\SOFTWARE\\") : QStringLiteral("\\Data\\");
    const auto root = prefix + QStringLiteral("Vendor");
    const auto add = [&](const char* name, const QString& pattern, const QString& target, const bool management, const bool driver) {
      QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2").arg(registry ? QStringLiteral("registry") : QStringLiteral("file"), QString::fromLatin1(name))))
          << registry << pattern << target << management << driver;
    };
    for (const auto& row : rows)
      add(row.name, root + QString::fromUtf16(row.patternSuffix), root + QString::fromUtf16(row.targetSuffix),
          registry ? row.registryManagement : row.fileManagement, row.driver);
    add("ascii-case", prefix + QStringLiteral("Entry"), prefix + QStringLiteral("eNTRY\\Child"), true, true);
    add("ordinal-latin-case", prefix + QStringLiteral("\u00c4"), prefix + QStringLiteral("\u00e4\\Child"), false, true);
    add("ordinal-distinct-micro-sign", prefix + QStringLiteral("\u00b5"), prefix + QStringLiteral("\u03bc\\Child"), false, false);
  }
  QTest::newRow("file/root-prefix") << false << QStringLiteral("\\") << QStringLiteral("\\Data\\Child") << true << true;
  QTest::newRow("file/root-does-not-match-relative-name") << false << QStringLiteral("\\") << QStringLiteral("Data\\Child") << false << false;
  QTest::newRow("registry/hive-alias") << true << QStringLiteral("HKLM\\SOFTWARE\\Vendor") << QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\Child")
                                       << true << true;
}

void DomainPolicyTests::exclusionManagementAndDriverEffectsRemainDistinct() {
  QFETCH(bool, registry);
  QFETCH(QString, pattern);
  QFETCH(QString, target);
  QFETCH(bool, managementSkipsCommit);
  QFETCH(bool, driverBypassesWrites);
  const auto effects = registry ? uwf::core::registryExclusionEffects(pattern.toStdU16String(), target.toStdU16String())
                                : uwf::core::fileExclusionEffects(pattern.toStdU16String(), target.toStdU16String());
  QCOMPARE(effects.managementSkipsCommit, managementSkipsCommit);
  QCOMPARE(effects.driverBypassesWrites, driverBypassesWrites);
  QCOMPARE(effects.preventsCommit(), managementSkipsCommit || driverBypassesWrites);
  if (registry) QCOMPARE(uwf::core::registryManagementSkipsCommit(pattern.toStdU16String(), target.toStdU16String()), managementSkipsCommit);
}

void DomainPolicyTests::registryDriverResolvesTheSelectedControlSet_data() {
  QTest::addColumn<QString>("pattern");
  QTest::addColumn<QString>("target");
  QTest::addColumn<quint32>("currentControlSet");
  QTest::addColumn<bool>("management");
  QTest::addColumn<bool>("driver");
  struct Selector {
    const char* label;
    const char16_t* pattern;
    const char16_t* target;
  };
  constexpr std::array selectors{Selector{"CSS", u"cUrReNtCoNtRoLsEt", u"CurrentControlSet"}, Selector{"001", u"ControlSet001", u"ControlSet001"},
                                 Selector{"002", u"ControlSet002", u"ControlSet002"}};
  // Independent expected alias classes for Select\\Current=1 and =2. The
  // management precheck never joins CSS with a numbered control set.
  constexpr std::array first{std::array{true, true, false}, std::array{true, true, false}, std::array{false, false, true}};
  constexpr std::array second{std::array{true, false, true}, std::array{false, true, false}, std::array{true, false, true}};
  constexpr std::array driverMatches{first, second};
  // A configured CSSBackup prefix is translated by the driver's raw pattern
  // loader, but an actual CSSBackup object is not the CurrentControlSet link.
  constexpr std::array backupFirst{std::array{false, true, false}, std::array{false, true, false}, std::array{false, false, true}};
  constexpr std::array backupSecond{std::array{false, false, true}, std::array{false, true, false}, std::array{false, false, true}};
  constexpr std::array backupMatches{backupFirst, backupSecond};
  for (std::size_t current = 0; current < driverMatches.size(); ++current)
    for (std::size_t pattern = 0; pattern < selectors.size(); ++pattern)
      for (std::size_t target = 0; target < selectors.size(); ++target)
        for (const bool backup : {false, true}) {
          const auto suffix = backup ? QStringLiteral("Backup\\Fixture") : QStringLiteral("\\Services\\Fixture");
          QTest::newRow(qPrintable(QStringLiteral("current-%1/%2-to-%3/%4")
                                       .arg(current + 1)
                                       .arg(QString::fromLatin1(selectors[pattern].label), QString::fromLatin1(selectors[target].label),
                                            backup ? QStringLiteral("marker-prefix-without-boundary") : QStringLiteral("component"))))
              << QStringLiteral("HKLM\\sYsTeM\\") + QString::fromUtf16(selectors[pattern].pattern) + suffix
              << QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\") + QString::fromUtf16(selectors[target].target) + suffix + QStringLiteral("\\Child")
              << static_cast<quint32>(current + 1) << (pattern == target)
              << (backup ? backupMatches[current][pattern][target] : driverMatches[current][pattern][target]);
        }
}

void DomainPolicyTests::registryDriverResolvesTheSelectedControlSet() {
  QFETCH(QString, pattern);
  QFETCH(QString, target);
  QFETCH(quint32, currentControlSet);
  QFETCH(bool, management);
  QFETCH(bool, driver);
  const auto effects = uwf::core::registryExclusionEffects(pattern.toStdU16String(), target.toStdU16String(), currentControlSet);
  QCOMPARE(effects.managementSkipsCommit, management);
  QCOMPARE(effects.driverBypassesWrites, driver);
  QCOMPARE(effects.preventsCommit(), management || driver);
  QCOMPARE(uwf::core::registryManagementSkipsCommit(pattern.toStdU16String(), target.toStdU16String()), management);
}

void DomainPolicyTests::registryObjectNamesResolveOnlyTheCurrentControlSetLink() {
  struct Name {
    const char16_t* suffix;
    bool link;
  };
  constexpr std::array names{Name{u"CurrentControlSet", true},        Name{u"CurrentControlSet\\Child", true},
                             Name{u"CurrentControlSetExtra", false},  Name{u"CurrentControlSetExtra\\Child", false},
                             Name{u"CurrentControlSetBackup", false}, Name{u"CurrentControlSetBackup\\Child", false}};
  const std::array numbered{QStringLiteral("ControlSet001"), QStringLiteral("ControlSet002")};
  for (std::size_t current = 0; current < numbered.size(); ++current)
    for (const auto& name : names) {
      const auto suffix = QString::fromUtf16(name.suffix);
      const auto object = QStringLiteral("HKLM\\SYSTEM\\") + suffix;
      auto expectedSuffix = suffix;
      if (name.link) expectedSuffix.replace(0, QStringLiteral("CurrentControlSet").size(), numbered[current]);
      QCOMPARE(uwf::core::registryDriverObjectName(object.toStdU16String(), static_cast<std::uint32_t>(current + 1)),
               (QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\") + expectedSuffix).toStdU16String());
      if (!name.link) {
        const auto effects = uwf::core::registryExclusionEffects(object.toStdU16String(), object.toStdU16String(), static_cast<std::uint32_t>(current + 1));
        QVERIFY(effects.managementSkipsCommit);
        QVERIFY(!effects.driverBypassesWrites);
      }
    }
}

void DomainPolicyTests::registryControlSetContextIsRequiredOnlyForRelevantComparisons_data() {
  QTest::addColumn<QString>("pattern");
  QTest::addColumn<QString>("target");
  QTest::addColumn<bool>("needsContext");
  QTest::addColumn<bool>("managementWithoutContext");
  QTest::addColumn<bool>("driverWithoutContext");
  struct Comparison {
    const char* name;
    const char16_t* pattern;
    const char16_t* target;
    bool needsContext, management, driver;
  };
  constexpr std::array rows{
      Comparison{"CSS-to-CSS", u"SYSTEM\\CurrentControlSet\\Fixture", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, true, true},
      Comparison{"CSS-to-001", u"SYSTEM\\CurrentControlSet\\Fixture", u"SYSTEM\\ControlSet001\\Fixture\\Child", true, false, false},
      Comparison{"001-to-CSS", u"SYSTEM\\ControlSet001\\Fixture", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", true, false, false},
      Comparison{"CSS-backup-prefix", u"SYSTEM\\CurrentControlSetBackup\\Fixture", u"SYSTEM\\ControlSet001Backup\\Fixture\\Child", true, false, false},
      Comparison{"CSS-to-software", u"SYSTEM\\CurrentControlSet\\Fixture", u"SOFTWARE\\Fixture\\Child", false, false, false},
      Comparison{"software-to-CSS", u"SOFTWARE\\Fixture", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, false, false},
      Comparison{"CSS-to-unrelated-system", u"SYSTEM\\CurrentControlSet\\Fixture", u"SYSTEM\\Vendor\\Child", false, false, false},
      Comparison{"unrelated-system-to-CSS", u"SYSTEM\\Vendor", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, false, false},
      Comparison{"system-ancestor-to-CSS", u"SYSTEM", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, true, true},
      Comparison{"system-ancestor-one-tail", u"SYSTEM\\", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, true, false},
      Comparison{"system-ancestor-two-tails", u"SYSTEM\\\\", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, false, false},
      Comparison{"same-CSS-one-tail", u"SYSTEM\\CurrentControlSet\\Fixture\\", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, true, false},
      Comparison{"CSS-extra-is-an-ordinary-object", u"SYSTEM\\CurrentControlSetExtra", u"SYSTEM\\CurrentControlSetExtra\\Child", false, true, false},
      Comparison{"software-marker-is-literal", u"SOFTWARE\\CurrentControlSet", u"SOFTWARE\\CurrentControlSet\\Child", false, true, true},
      Comparison{"numbered-equal", u"SYSTEM\\ControlSet001\\Fixture", u"SYSTEM\\ControlSet001\\Fixture\\Child", false, true, true},
      Comparison{"numbered-distinct", u"SYSTEM\\ControlSet001\\Fixture", u"SYSTEM\\ControlSet002\\Fixture\\Child", false, false, false},
      Comparison{"CSS-to-ControlSetOther", u"SYSTEM\\CurrentControlSet\\Fixture", u"SYSTEM\\ControlSetOther\\Fixture\\Child", false, false, false},
      Comparison{"ControlSetOther-to-CSS", u"SYSTEM\\ControlSetOther\\Fixture", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, false, false},
      Comparison{"CSS-to-bare-ControlSet", u"SYSTEM\\CurrentControlSet\\Fixture", u"SYSTEM\\ControlSet\\Fixture\\Child", false, false, false},
      Comparison{"bare-ControlSet-to-CSS", u"SYSTEM\\ControlSet\\Fixture", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, false, false},
      Comparison{"CSS-to-short-number", u"SYSTEM\\CurrentControlSet\\Fixture", u"SYSTEM\\ControlSet01\\Fixture\\Child", false, false, false},
      Comparison{"short-number-to-CSS", u"SYSTEM\\ControlSet01\\Fixture", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, false, false},
      Comparison{"CSS-numeric-suffix", u"SYSTEM\\CurrentControlSet1\\Fixture", u"SYSTEM\\ControlSet0011\\Fixture\\Child", true, false, false},
      Comparison{"CSS-zero-numeric-suffix", u"SYSTEM\\CurrentControlSet0\\Fixture", u"SYSTEM\\ControlSet0000\\Fixture\\Child", true, false, false},
      Comparison{"CSS-alpha-suffix", u"SYSTEM\\CurrentControlSetExtra\\Fixture", u"SYSTEM\\ControlSet002Extra\\Fixture\\Child", true, false, false},
      Comparison{"plain-CSS-cannot-match-alpha-suffix", u"SYSTEM\\CurrentControlSet\\Fixture", u"SYSTEM\\ControlSet002Extra\\Fixture\\Child", false, false,
                 false},
      Comparison{"alpha-suffix-cannot-match-complete-CSS", u"SYSTEM\\ControlSet002Extra\\Fixture", u"SYSTEM\\CurrentControlSet\\Fixture\\Child", false, false,
                 false},
      Comparison{"extra-leading-zero-is-not-a-formatted-id", u"SYSTEM\\CurrentControlSet\\Fixture", u"SYSTEM\\ControlSet0011\\Fixture\\Child", false, false,
                 false},
      Comparison{"numeric-suffix-mismatch", u"SYSTEM\\CurrentControlSet1\\Fixture", u"SYSTEM\\ControlSet0020\\Fixture\\Child", false, false, false},
      Comparison{"cross-alias-one-tail", u"SYSTEM\\CurrentControlSet\\Fixture\\", u"SYSTEM\\ControlSet002\\Fixture\\Child", false, false, false},
      Comparison{"numbered-backup-to-literal-CSS-backup", u"SYSTEM\\ControlSet001Backup", u"SYSTEM\\CurrentControlSetBackup\\Child", false, false, false},
      Comparison{"system-sibling-marker-is-literal", u"SYSTEMBackup\\CurrentControlSet", u"SYSTEMBackup\\CurrentControlSet\\Child", false, true, true},
      Comparison{"system-root-to-sibling", u"SYSTEM", u"SYSTEMBackup\\Child", false, false, false},
      Comparison{"system-sibling-to-root", u"SYSTEMBackup", u"SYSTEM\\Child", false, false, false},
      Comparison{"CSS-to-system-sibling", u"SYSTEM\\CurrentControlSet", u"SYSTEMBackup\\CurrentControlSet\\Child", false, false, false},
      Comparison{"system-sibling-to-CSS", u"SYSTEMBackup\\CurrentControlSet", u"SYSTEM\\CurrentControlSet\\Child", false, false, false},
  };
  for (const auto& row : rows)
    QTest::newRow(row.name) << QStringLiteral("HKLM\\") + QString::fromUtf16(row.pattern)
                            << QStringLiteral("HKEY_LOCAL_MACHINE\\") + QString::fromUtf16(row.target) << row.needsContext << row.management << row.driver;
  QTest::newRow("machine-ancestor-to-CSS") << QStringLiteral("HKLM") << QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Fixture\\Child") << false
                                           << false << true;
}

void DomainPolicyTests::registryControlSetContextIsRequiredOnlyForRelevantComparisons() {
  QFETCH(QString, pattern);
  QFETCH(QString, target);
  QFETCH(bool, needsContext);
  QFETCH(bool, managementWithoutContext);
  QFETCH(bool, driverWithoutContext);
  const auto stored = pattern.toStdU16String();
  const auto actual = target.toStdU16String();
  QCOMPARE(uwf::core::registryExclusionNeedsCurrentControlSet(stored, actual), needsContext);
  QCOMPARE(uwf::core::registryManagementSkipsCommit(stored, actual), managementWithoutContext);
  if (needsContext) {
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)uwf::core::registryExclusionEffects(stored, actual));
    return;
  }
  uwf::core::ExclusionEffects effects;
  try {
    effects = uwf::core::registryExclusionEffects(stored, actual);
  } catch (const std::exception& error) {
    QFAIL(error.what());
  }
  QCOMPARE(effects.managementSkipsCommit, managementWithoutContext);
  QCOMPARE(effects.driverBypassesWrites, driverWithoutContext);
  for (const std::uint32_t current : {1u, 2u}) {
    const auto explicitContext = uwf::core::registryExclusionEffects(stored, actual, current);
    QCOMPARE(explicitContext.managementSkipsCommit, managementWithoutContext);
    QCOMPARE(explicitContext.driverBypassesWrites, driverWithoutContext);
  }
}

void DomainPolicyTests::pendingChangesCountsEveryBusinessField() {
  uwf::core::PendingChanges changes;
  QVERIFY(changes.empty());
  QCOMPARE(changes.count(), std::size_t{0});

  changes.setFilterEnabled = false;
  changes.setOverlay.type = uwf::core::OverlayType::Disk;
  changes.setOverlay.maximumSizeMb = 4096;
  changes.setOverlay.warningThresholdMb = 2048;
  changes.setOverlay.criticalThresholdMb = 3072;
  changes.volumeProtect["C:"] = true;
  changes.volumeBindings["D:"] = uwf::core::VolumeBinding::DriveLetter;
  changes.addFileExclusions["C:"] = {R"(C:\A)", R"(C:\B)"};
  changes.removeFileExclusions["D:"] = {R"(D:\Old)"};
  changes.addRegistryExclusions = {R"(HKLM\Software\A)"};
  changes.removeRegistryExclusions = {R"(HKLM\System\B)"};
  changes.setPersistDomainSecretKey = true;
  changes.setPersistTSCAL = false;

  QVERIFY(!changes.empty());
  QCOMPARE(changes.count(), std::size_t{14});
  QVERIFY(changes.setOverlay.touchesTypeOrSize());

  changes.clear();
  QVERIFY(changes.empty());
  QCOMPARE(changes.count(), std::size_t{0});

  changes.addFileExclusions["C:"] = {};
  changes.removeFileExclusions["D:"] = {};
  QVERIFY(changes.empty());
  QCOMPARE(changes.count(), std::size_t{0});
}

void DomainPolicyTests::exclusionSortingIsCaseInsensitiveAndStable() {
  std::vector<std::string> items = {R"(C:\zeta)", R"(c:\Alpha)", R"(C:\ALPHA)", R"(C:\beta)"};
  uwf::core::sortExclusions(items);
  QCOMPARE(items.size(), std::size_t{3});
  QCOMPARE(QString::fromUtf8(items[0]), QStringLiteral("c:\\Alpha"));
  QCOMPARE(QString::fromUtf8(items[1]), QStringLiteral("C:\\beta"));
  QCOMPARE(QString::fromUtf8(items[2]), QStringLiteral("C:\\zeta"));

  uwf::core::UwfSnapshot snapshot;
  snapshot.current.fileExclusions["Volume{A}"] = {"B", "a", "A"};
  snapshot.next.registryExclusions = {"z", "Y", "y"};
  uwf::core::sortSnapshot(snapshot);
  QCOMPARE(snapshot.current.fileExclusions.at("Volume{A}").size(), std::size_t{2});
  QCOMPARE(snapshot.next.registryExclusions.size(), std::size_t{2});

  std::vector<std::string> permutation = {"Zulu", "alpha", "ALPHA", "beta", "BETA"};
  std::ranges::sort(permutation);
  do {
    auto sorted = permutation;
    uwf::core::sortExclusions(sorted);
    QCOMPARE(sorted.size(), std::size_t{3});
    QCOMPARE(uwf::toLowerAscii(sorted[0]), std::string("alpha"));
    QCOMPARE(uwf::toLowerAscii(sorted[1]), std::string("beta"));
    QCOMPARE(uwf::toLowerAscii(sorted[2]), std::string("zulu"));
    const auto once = sorted;
    uwf::core::sortExclusions(sorted);
    QCOMPARE(sorted, once);
  } while (std::ranges::next_permutation(permutation).found);
}

}  // namespace

QTEST_APPLESS_MAIN(DomainPolicyTests)

#include "DomainPolicyTests.moc"
