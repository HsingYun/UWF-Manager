// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/planning/ConfigurationScript.h"
#include "application/paths/PathPolicy.h"
#include "application/uwf/CompositeAllVolumeProtection.h"
#include "application/uwf/adapters/UwfOverlayMaintenance.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "domain/uwf/contracts/Types.h"
#include "simulation/app/ConfigurationState.h"
#include "simulation/uwf/FakeUwfBackend.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/FileTargetFixture.h"

namespace {
QString text(const std::string& value) { return QString::fromUtf8(value); }

QStringList rendered(const std::vector<uwf::app::UwfmgrCommand>& commands) {
  QStringList result;
  result.reserve(static_cast<qsizetype>(commands.size()));
  for (const auto& command : commands) result.append(text(uwf::app::renderUwfmgrCommand(command)));
  return result;
}

uwf::core::UwfSnapshot maintenanceScriptSnapshot() {
  uwf::core::SessionSnapshot session;
  session.filter.enabled = true;
  session.overlay = {uwf::core::OverlayType::RAM, 2048, 512, 1024};
  session.volumes = {{uwf::simulation::application::fixtureVolumeCName, "C:", true, uwf::core::VolumeBinding::VolumeId, std::string("C:")}};
  return uwf::test::application::configurationSnapshot(session, session);
}

class ConfigurationScriptTests final : public QObject {
  Q_OBJECT
 private slots:
  void configurationScriptsOmitUnrepresentableTargetsAndDependencies_data();
  void configurationScriptsOmitUnrepresentableTargetsAndDependencies();
  void uwfmgrRenderingPreservesBusinessOrderAndQuoting();
  void plannedSessionRenderingPreservesFinalTargets();
  void sessionRenderingSkipsVolumesWithoutDriveLetters();
  void exactHiddenGuidProtectionIsExportedAndMergedWithoutADriveAlias();
  void guidOnlyExclusionExportsPreserveTheOwningVolumeAndLiteralSuffix();
  void fileExclusionScriptsHonorNativeSuffixBoundaries_data();
  void fileExclusionScriptsHonorNativeSuffixBoundaries();
  void bulkUnprotectExportsTheCompleteScopeIncludingOfflineRegistrations();
  void bulkUnprotectExportRequiresAnIndependentlyConfirmedValidScope();
  void typedPhasesPreservePolicyAndBulkOrderInBothScriptProducts();
  void fileCommandsRequireAnAddressThatResolvesToTheirConfigurationGuid_data();
  void fileCommandsRequireAnAddressThatResolvesToTheirConfigurationGuid();
  void configurationScriptsDescribeOmittedSettings_data();
  void configurationScriptsDescribeOmittedSettings();
  void configurationScriptsPreserveDependenciesAndArguments();
  void legacyGuardedCommandsPreserveDependenciesAndArguments();
  void nativeMaintenanceCommandsKeepCurrentActionsOutOfNextSnapshots_data();
  void nativeMaintenanceCommandsKeepCurrentActionsOutOfNextSnapshots();
  void maintenanceCommandsPreservePhaseOrderAndNeverInventFullCommit();
  void persistentResetRequestsStayOutOfOfflineConfigurationSnapshots_data();
  void persistentResetRequestsStayOutOfOfflineConfigurationSnapshots();
};

void ConfigurationScriptTests::persistentResetRequestsStayOutOfOfflineConfigurationSnapshots_data() {
  QTest::addColumn<bool>("clear");
  QTest::newRow("schedule-clear") << true;
  QTest::newRow("cancel-clear") << false;
}

void ConfigurationScriptTests::persistentResetRequestsStayOutOfOfflineConfigurationSnapshots() {
  QFETCH(bool, clear);
  using namespace uwf;
  using namespace uwf::app;
  ConfigurationPlan plan;
  plan.snapshot = maintenanceScriptSnapshot();
  plan.snapshot.current.overlay.type = plan.snapshot.next.overlay.type = core::OverlayType::Disk;
  plan.snapshot.runtime.currentOverlayFlags = OverlayFlags::Persistent;
  plan.hasPolicyContext = true;
  auto& policy = plan.policySnapshot;
  policy.transportAvailable = true;
  policy.current.filterEnabled = {PolicyObservationStatus::Available, true};
  policy.current.overlayType = {PolicyObservationStatus::Available, 1};
  policy.current.flags = {PolicyObservationStatus::Available, OverlayFlags{OverlayFlags::Persistent}};
  policy.next = policy.current;
  policy.resetPersistentState = {PolicyObservationStatus::Available, clear ? 0 : 1};
  plan.overlayPolicy.reset = clear ? PersistentReset::Clear : PersistentReset::Preserve;
  plan.changes.addRegistryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\ResetExport"};

  const auto pending = configurationChangeScript(plan);
  QVERIFY(pending.exportSupported);
  QCOMPARE(pending.commands.size(), std::size_t{2});
  const auto reset = std::ranges::find(pending.commands, UwfmgrKind::OverlayResetPersistentState, &UwfmgrCommand::kind);
  QVERIFY(reset != pending.commands.end());
  QCOMPARE(reset->args, std::vector<std::string>{clear ? "on" : "off"});
  // Cancellation must precede other writes; scheduling follows the settings.
  QCOMPARE(reset - pending.commands.begin(), clear ? std::ptrdiff_t{1} : std::ptrdiff_t{0});
  QVERIFY(std::ranges::none_of(pending.omissions, [](const auto& omission) { return omission.kind == ConfigurationScriptOmissionKind::CurrentSessionAction; }));

  const auto next = configurationTargetScript(plan);
  QVERIFY(next.exportSupported);
  QVERIFY(!next.commands.empty());
  QCOMPARE(std::ranges::count(next.commands, UwfmgrKind::OverlayResetPersistentState, &UwfmgrCommand::kind), std::ptrdiff_t{0});
  QCOMPARE(std::ranges::count(next.commands, UwfmgrKind::RegistryAddExclusion, &UwfmgrCommand::kind), std::ptrdiff_t{1});
  QCOMPARE(std::ranges::count(next.commands, UwfmgrKind::OverlaySetType, &UwfmgrCommand::kind), std::ptrdiff_t{1});
  QCOMPARE(std::ranges::count(next.commands, UwfmgrKind::OverlaySetSize, &UwfmgrCommand::kind), std::ptrdiff_t{1});
  QCOMPARE(std::ranges::count(next.omissions, ConfigurationScriptOmissionKind::CurrentSessionAction, &ConfigurationScriptOmission::kind), std::ptrdiff_t{1});
  for (const auto& product : {pending, next}) {
    auto parsed = parseUwfmgrText(renderConfigurationScript(product));
    std::erase_if(parsed, [](const auto& command) { return command.parseError == ParseError::Comment; });
    QCOMPARE(parsed.size(), product.commands.size());
    for (std::size_t index = 0; index < parsed.size(); ++index) {
      QCOMPARE(product.commands[index].requiresPrevious, index != 0);
      QCOMPARE(parsed[index].parseError, ParseError::None);
      QCOMPARE(parsed[index].kind, product.commands[index].kind);
      QCOMPARE(parsed[index].args, product.commands[index].args);
      QCOMPARE(parsed[index].requiresPrevious, index != 0);
    }
  }
  // An observed reset request is not an exportable action without explicit intent.
  plan.overlayPolicy.reset.reset();
  for (const auto& product : {configurationChangeScript(plan), configurationTargetScript(plan)}) {
    QCOMPARE(std::ranges::count(product.commands, UwfmgrKind::OverlayResetPersistentState, &UwfmgrCommand::kind), std::ptrdiff_t{0});
    QVERIFY(
        std::ranges::none_of(product.omissions, [](const auto& omission) { return omission.kind == ConfigurationScriptOmissionKind::CurrentSessionAction; }));
  }
}

void ConfigurationScriptTests::nativeMaintenanceCommandsKeepCurrentActionsOutOfNextSnapshots_data() {
  QTest::addColumn<int>("operation");
  QTest::newRow("disable-HORM") << 0;
  QTest::newRow("enable-HORM") << 1;
  QTest::newRow("FullCommit") << 2;
  QTest::newRow("storage-location") << 3;
}

void ConfigurationScriptTests::nativeMaintenanceCommandsKeepCurrentActionsOutOfNextSnapshots() {
  QFETCH(int, operation);
  using namespace uwf;
  using namespace uwf::app;
  ConfigurationPlan plan;
  plan.snapshot = maintenanceScriptSnapshot();
  plan.snapshot.current.overlay.type = core::OverlayType::RAM;
  plan.snapshot.runtime.currentOverlayFlags = OverlayFlags::ReadOnlyMedia;
  plan.snapshot.runtime.hormEnabled = operation == 0;
  plan.snapshot.runtime.romDisposition = operation == 0 ? 2u : 0u;
  const auto kind = operation == 0   ? UwfmgrKind::FilterDisableHorm
                    : operation == 1 ? UwfmgrKind::FilterEnableHorm
                    : operation == 2 ? UwfmgrKind::OverlayCommit
                                     : UwfmgrKind::VolumeCreateSwapfile;
  if (operation < 2)
    plan.maintenance.hormEnabled = operation == 1;
  else if (operation == 2)
    plan.maintenance.fullCommit = true;
  else {
    plan.snapshot.current.filter.enabled = false;
    plan.snapshot.runtime.currentOverlayFlags = 0;
    plan.maintenance.storageVolume = QString::fromLatin1(uwf::simulation::application::fixtureVolumeDName);
    auto& observed = plan.maintenanceSnapshot.emplace();
    observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
    observed.currentOverlayType = observed.nextOverlayType = static_cast<int>(core::OverlayType::RAM);
    observed.currentFlags = observed.nextFlags = OverlayFlags{0};
    observed.maximumOverlaySize = plan.snapshot.next.overlay.maximumSizeMb;
    observed.volumes.push_back({*plan.maintenance.storageVolume, QStringLiteral("D:"), {}, 0, true, {}, {}});
  }
  const auto pending = configurationChangeScript(plan);
  QVERIFY(pending.exportSupported);
  QCOMPARE(pending.commands.size(), std::size_t{1});
  QCOMPARE(pending.commands.front().kind, kind);
  QVERIFY(!pending.commands.front().requiresPrevious);
  QVERIFY(std::ranges::none_of(pending.omissions, [](const auto& omission) {
    return omission.kind == ConfigurationScriptOmissionKind::IncompleteVolumeScope || omission.kind == ConfigurationScriptOmissionKind::CurrentSessionAction;
  }));
  if (operation == 3) {
    QCOMPARE(pending.commands.front().args.size(), std::size_t{1});
    QCOMPARE(compat::parseVolumeId(pending.commands.front().args.front()), compat::parseVolumeId(uwf::simulation::application::fixtureVolumeDName));
  }
  auto parsed = parseUwfmgrText(renderConfigurationScript(pending));
  std::erase_if(parsed, [](const auto& command) { return command.parseError == ParseError::Comment; });
  QCOMPARE(parsed.size(), pending.commands.size());
  QCOMPARE(parsed.front().parseError, ParseError::None);
  QCOMPARE(parsed.front().kind, kind);
  QCOMPARE(parsed.front().args, pending.commands.front().args);

  const auto next = configurationTargetScript(plan);
  QVERIFY(next.exportSupported);
  QCOMPARE(std::ranges::count(next.commands, kind, &UwfmgrCommand::kind), operation == 3 ? std::ptrdiff_t{1} : std::ptrdiff_t{0});
  QCOMPARE(std::ranges::any_of(next.omissions, [](const auto& omission) { return omission.kind == ConfigurationScriptOmissionKind::CurrentSessionAction; }),
           operation != 3);
  QVERIFY(std::ranges::none_of(next.commands, [](const auto& command) {
    return command.kind == UwfmgrKind::FilterEnableHorm || command.kind == UwfmgrKind::FilterDisableHorm || command.kind == UwfmgrKind::OverlayCommit;
  }));
  QVERIFY(!configurationScriptScope(next).empty());
}

void ConfigurationScriptTests::maintenanceCommandsPreservePhaseOrderAndNeverInventFullCommit() {
  using namespace uwf;
  using namespace uwf::app;
  ConfigurationPlan plan;
  plan.snapshot = maintenanceScriptSnapshot();
  plan.snapshot.current.filter.enabled = false;
  plan.maintenance.storageVolume = QString::fromLatin1(uwf::simulation::application::fixtureVolumeDName);
  plan.maintenance.hormEnabled = false;
  plan.changes.addRegistryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\ExportOrder"};
  auto& observed = plan.maintenanceSnapshot.emplace();
  observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
  observed.currentOverlayType = observed.nextOverlayType = static_cast<int>(core::OverlayType::RAM);
  observed.currentFlags = observed.nextFlags = OverlayFlags{0};
  observed.maximumOverlaySize = plan.snapshot.next.overlay.maximumSizeMb;
  observed.volumes.push_back({*plan.maintenance.storageVolume, QStringLiteral("D:"), {}, 0, true, {}, {}});
  const auto script = configurationChangeScript(plan);
  const auto position = [&](const UwfmgrKind kind) { return std::ranges::find(script.commands, kind, &UwfmgrCommand::kind) - script.commands.begin(); };
  QVERIFY(position(UwfmgrKind::FilterDisableHorm) < static_cast<std::ptrdiff_t>(script.commands.size()));
  QVERIFY(position(UwfmgrKind::RegistryAddExclusion) < position(UwfmgrKind::VolumeCreateSwapfile));
  QVERIFY(position(UwfmgrKind::VolumeCreateSwapfile) < position(UwfmgrKind::FilterDisableHorm));
  QCOMPARE(std::ranges::count(script.commands, UwfmgrKind::OverlayCommit, &UwfmgrCommand::kind), std::ptrdiff_t{0});
  for (std::size_t index = 0; index < script.commands.size(); ++index) QCOMPARE(script.commands[index].requiresPrevious, index != 0);
  auto parsed = parseUwfmgrText(renderConfigurationScript(script));
  std::erase_if(parsed, [](const auto& command) { return command.parseError == ParseError::Comment; });
  QCOMPARE(parsed.size(), script.commands.size());
  for (std::size_t index = 0; index < parsed.size(); ++index) {
    QCOMPARE(parsed[index].parseError, ParseError::None);
    QCOMPARE(parsed[index].kind, script.commands[index].kind);
    QCOMPARE(parsed[index].requiresPrevious, index != 0);
  }
  // A known FullCommit state never creates a one-shot command in either export.
  plan.maintenance = {};
  plan.maintenanceSnapshot.reset();
  plan.snapshot.runtime.fullCommit = true;
  plan.snapshot.runtime.romDisposition = 4;
  for (const auto& product : {configurationChangeScript(plan), configurationTargetScript(plan)})
    QCOMPARE(std::ranges::count(product.commands, UwfmgrKind::OverlayCommit, &UwfmgrCommand::kind), std::ptrdiff_t{0});
}

void ConfigurationScriptTests::fileCommandsRequireAnAddressThatResolvesToTheirConfigurationGuid_data() {
  QTest::addColumn<bool>("loose");
  QTest::addColumn<QString>("physical");
  QTest::addColumn<bool>("fileExportable");
  QTest::newRow("loose-volume-moved-to-new-letter") << true << QStringLiteral("E:") << true;
  QTest::newRow("tight-volume-moved-to-new-letter") << false << QStringLiteral("E:") << true;
  QTest::newRow("loose-letter-still-matches") << true << QStringLiteral("D:") << true;
  QTest::newRow("loose-unmounted-volume") << true << QString{} << true;
  QTest::newRow("tight-unmounted-volume") << false << QString{} << true;
}
void ConfigurationScriptTests::fileCommandsRequireAnAddressThatResolvesToTheirConfigurationGuid() {
  QFETCH(bool, loose);
  QFETCH(QString, physical);
  QFETCH(bool, fileExportable);
  using namespace uwf;
  using namespace uwf::app;
  const std::string id = test::application::fixtureVolumeId;
  const std::string name = uwf::simulation::application::fixtureVolumeCName;
  const auto address = physical.toStdString();
  ConfigurationPlan plan;
  plan.snapshot.next.overlay = {core::OverlayType::RAM, 2048, 512, 1024};
  plan.snapshot.next.volumes = {{name, "D:", true, loose ? core::VolumeBinding::DriveLetter : core::VolumeBinding::VolumeId, physical.toStdString()}};
  plan.snapshot.next.fileExclusions[name] = {"\\Before"};
  plan.changes.volumeProtect[id] = false;
  plan.changes.addFileExclusions[id] = {address + "\\After"};
  for (const auto& script : {configurationChangeScript(plan), configurationTargetScript(plan)}) {
    QVERIFY(script.exportSupported);
    QCOMPARE(std::ranges::count(script.commands, UwfmgrKind::VolumeProtect, &UwfmgrCommand::kind), std::ptrdiff_t{0});
    QCOMPARE(std::ranges::count(script.commands, UwfmgrKind::VolumeUnprotect, &UwfmgrCommand::kind), std::ptrdiff_t{1});
    const auto unprotect = std::ranges::find(script.commands, UwfmgrKind::VolumeUnprotect, &UwfmgrCommand::kind);
    QCOMPARE(unprotect->args, std::vector<std::string>{compat::volumePath(id)});
    const auto files = std::ranges::count(script.commands, UwfmgrKind::FileAddExclusion, &UwfmgrCommand::kind);
    QCOMPARE(files > 0, fileExportable);
    for (const auto& command : script.commands)
      if (command.kind == UwfmgrKind::FileAddExclusion) {
        QVERIFY(!command.args.empty());
        QVERIFY(command.args.front().starts_with(address.empty() ? compat::volumePath(id) : address + "\\"));
      }
    const auto omission =
        std::ranges::find_if(script.omissions, [](const auto& value) { return value.kind == ConfigurationScriptOmissionKind::VolumeWithoutDrive; });
    QCOMPARE(omission == script.omissions.end(), fileExportable);
  }
}

void ConfigurationScriptTests::guidOnlyExclusionExportsPreserveTheOwningVolumeAndLiteralSuffix() {
  using namespace uwf;
  using namespace uwf::app;
  const std::string id = test::application::fixtureVolumeId;
  const auto root = compat::volumePath(id);
  const auto raw = compat::volumePath(test::application::otherFixtureVolumeId) + "Data\\..\\Keep ";
  ConfigurationPlan plan;
  plan.snapshot.next.overlay = {core::OverlayType::RAM, 2048, 512, 1024};
  plan.snapshot.next.volumes = {{uwf::simulation::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::VolumeId, std::string{}}};
  plan.snapshot.next.fileExclusions[uwf::simulation::application::fixtureVolumeCName] = {"\\Data", raw};
  plan.changes.addFileExclusions[id] = {root + "Data"};
  const auto script = configurationTargetScript(plan);
  QStringList suffixes;
  for (const auto& command : script.commands) {
    if (command.kind != UwfmgrKind::FileAddExclusion) continue;
    const auto parsed = paths::splitVolumeFilePath(QString::fromStdString(command.args.front()));
    QVERIFY(parsed);
    QCOMPARE(parsed->volume, id);
    suffixes.append(parsed->relativePath);
  }
  QCOMPARE(suffixes.size(), qsizetype{2});
  QVERIFY(suffixes.contains(QStringLiteral("\\Data")));
  QVERIFY(suffixes.contains(QString::fromStdString(raw)));
  QVERIFY(std::ranges::none_of(script.omissions, [](const auto& value) { return value.kind == ConfigurationScriptOmissionKind::VolumeWithoutDrive; }));
}

void ConfigurationScriptTests::fileExclusionScriptsHonorNativeSuffixBoundaries_data() {
  QTest::addColumn<bool>("provider");
  QTest::addColumn<bool>("guid");
  QTest::addColumn<QString>("body");
  QTest::addColumn<bool>("exportable");
  struct Body {
    const char* name;
    QString text;
    bool exportable;
  };
  const auto astral = QStringLiteral("\U0001F680");
  const std::array bodies{
      Body{"empty", {}, false},
      Body{"rootless", QStringLiteral("Data"), true},
      Body{"260-units", QStringLiteral("\\") + QString(259, u'x'), true},
      Body{"261-units", QStringLiteral("\\") + QString(260, u'x'), false},
      Body{"non-BMP-260-units", QStringLiteral("\\") + astral.repeated(129) + u'x', true},
      Body{"non-BMP-261-units", QStringLiteral("\\") + astral.repeated(130), false},
      Body{"extended-260-units", QStringLiteral(R"(\\?\)") + QString(256, u'x'), true},
      Body{"extended-261-units", QStringLiteral(R"(\\?\)") + QString(257, u'x'), false},
      Body{"literal-guid", QString::fromStdString(uwf::compat::volumePath(uwf::test::application::otherFixtureVolumeId)) + QStringLiteral("Data\\..\\Keep "),
           true}};
  for (const bool provider : {false, true})
    for (const bool guid : {false, true})
      for (const auto& body : bodies) {
        const auto row = QStringLiteral("%1/%2/%3")
                             .arg(provider ? QStringLiteral("provider") : QStringLiteral("pending"), guid ? QStringLiteral("guid") : QStringLiteral("drive"),
                                  QString::fromLatin1(body.name));
        QTest::newRow(qUtf8Printable(row)) << provider << guid << body.text << body.exportable;
      }
}

void ConfigurationScriptTests::fileExclusionScriptsHonorNativeSuffixBoundaries() {
  QFETCH(bool, provider);
  QFETCH(bool, guid);
  QFETCH(QString, body);
  QFETCH(bool, exportable);
  using namespace uwf;
  using namespace uwf::app;
  const std::string id = test::application::fixtureVolumeId;
  const auto root = compat::volumePath(id);
  const auto address = guid ? root.substr(0, root.size() - 1) : std::string("C:");
  const auto argument = address + body.toStdString();
  ConfigurationPlan plan;
  plan.snapshot.next.overlay = {core::OverlayType::RAM, 2048, 512, 1024};
  plan.snapshot.next.volumes = {
      {uwf::simulation::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::VolumeId, guid ? std::string{} : std::string("C:")}};
  if (provider)
    plan.snapshot.next.fileExclusions[uwf::simulation::application::fixtureVolumeCName] = {body.toStdString()};
  else
    plan.changes.addFileExclusions[id] = {argument};
  std::vector<ConfigurationScript> products{configurationTargetScript(plan)};
  if (!provider) products.push_back(configurationChangeScript(plan));
  for (const auto& script : products) {
    QCOMPARE(std::ranges::count(script.commands, UwfmgrKind::FileAddExclusion, &UwfmgrCommand::kind), exportable ? std::ptrdiff_t{1} : std::ptrdiff_t{0});
    const auto omitted =
        std::ranges::any_of(script.omissions, [](const auto& value) { return value.kind == ConfigurationScriptOmissionKind::VolumeWithoutDrive; });
    QCOMPARE(omitted, !exportable);
    if (!exportable) continue;
    const auto command = std::ranges::find(script.commands, UwfmgrKind::FileAddExclusion, &UwfmgrCommand::kind);
    QCOMPARE(command->args, std::vector<std::string>{argument});
    const auto parsedAddress = paths::splitVolumeFilePath(QString::fromStdString(command->args.front()));
    QVERIFY(parsedAddress);
    QCOMPARE(parsedAddress->volume, guid ? id : std::string("C:"));
    QCOMPARE(parsedAddress->relativePath, body);
    const auto parsedScript = parseUwfmgrText(renderConfigurationScript(script));
    const auto roundTripped = std::ranges::find(parsedScript, UwfmgrKind::FileAddExclusion, &UwfmgrCommand::kind);
    QVERIFY(roundTripped != parsedScript.end());
    QCOMPARE(roundTripped->parseError, ParseError::None);
    QCOMPARE(roundTripped->args, command->args);
  }
}

void ConfigurationScriptTests::typedPhasesPreservePolicyAndBulkOrderInBothScriptProducts() {
  using namespace uwf;
  using namespace uwf::app;
  core::UwfSnapshot snapshot;
  snapshot.uwfAvailable = snapshot.elevated = true;
  snapshot.next.overlay = {.type = core::OverlayType::RAM, .maximumSizeMb = 2048, .warningThresholdMb = 512, .criticalThresholdMb = 1024};
  snapshot.next.volumes = {{uwf::simulation::application::fixtureVolumeCName, "C:", false, core::VolumeBinding::VolumeId, std::string("C:")}};
  OverlayPolicySnapshot policy;
  policy.transportAvailable = true;
  policy.current.filterEnabled = {PolicyObservationStatus::Available, false};
  policy.current.overlayType = {PolicyObservationStatus::Available, 0};
  policy.current.flags = {PolicyObservationStatus::Available, OverlayFlags{0}};
  policy.next = policy.current;
  policy.next.flags.value = OverlayFlags{OverlayFlags::ReadOnlyMedia};
  policy.resetPersistentState = {PolicyObservationStatus::Available, 0};
  ConfigurationChanges changes;
  changes.settings.setOverlay.type = core::OverlayType::Disk;
  changes.settings.volumeProtect["C:"] = false;
  changes.overlayPolicy = {.persistent = true, .readOnlyMedia = false};
  changes.maintenance.allVolumesProtected = true;
  const auto plan = planApplicationConfiguration(changes, snapshot, policy);
  for (const auto& script : {configurationChangeScript(plan), configurationTargetScript(plan)}) {
    QVERIFY(script.exportSupported);
    const auto position = [&](const UwfmgrKind kind) { return std::ranges::find(script.commands, kind, &UwfmgrCommand::kind) - script.commands.begin(); };
    QVERIFY(position(UwfmgrKind::OverlaySetReadOnlyMedia) < position(UwfmgrKind::OverlaySetType));
    QVERIFY(position(UwfmgrKind::OverlaySetType) < position(UwfmgrKind::OverlaySetPersistent));
    QVERIFY(position(UwfmgrKind::OverlaySetPersistent) < position(UwfmgrKind::VolumeProtectAll));
    QCOMPARE(script.commands.back().kind, UwfmgrKind::VolumeUnprotect);
    QCOMPARE(script.commands.back().args, std::vector<std::string>{compat::volumePath(test::application::fixtureVolumeId)});
    QVERIFY(position(UwfmgrKind::VolumeProtectAll) < static_cast<std::ptrdiff_t>(script.commands.size() - 1));
    QCOMPARE(std::ranges::count(script.commands, UwfmgrKind::VolumeProtectAll, &UwfmgrCommand::kind), std::ptrdiff_t{1});
    for (std::size_t index = 0; index < script.commands.size(); ++index) QCOMPARE(script.commands[index].requiresPrevious, index != 0);
  }
}

void ConfigurationScriptTests::bulkUnprotectExportsTheCompleteScopeIncludingOfflineRegistrations() {
  using namespace uwf;
  using Fake = uwf::simulation::FakeUwfBackend;
  const std::string mounted = "11111111-2222-3333-4444-555555555555", offline = "66666666-7777-8888-9999-aaaaaaaaaaaa";
  Fake::State state;
  state.next.volumes.emplace(mounted, Fake::VolumeConfiguration{.volume = {mounted, "C:", true, true, false}, .fileExclusions = {}});
  state.next.volumes.emplace(offline, Fake::VolumeConfiguration{.volume = {offline, "C:", true, false, false}, .fileExclusions = {}});
  state.runtime.fileMaintenancePrefixes[mounted] = "C:";
  state.runtime.fileMaintenancePrefixes[offline] = "";
  auto system = std::make_shared<Fake>(state);
  app::ConfigurationPlan plan;
  plan.snapshot = app::publishedUwfSnapshot(system->state(), app::UwfCapability::Available, true, {});
  QCOMPARE(plan.snapshot.next.volumes.size(), std::size_t{2});
  QCOMPARE(compat::parseVolumeId(plan.snapshot.next.volumes.front().volumeName), std::optional<std::string>{mounted});
  const auto complete = app::allVolumeProtectionActions(app::composeAllVolumeProtection(system, {}));
  plan.maintenanceSnapshot = app::uwfOverlayMaintenance(system, {}, {}, complete)->observe();
  QVERIFY(plan.maintenanceSnapshot->unprotectScopeConfirmed);
  QCOMPARE(plan.maintenanceSnapshot->unprotectTargetVolumes.size(), std::size_t{2});
  plan.maintenance.allVolumesProtected = false;
  for (const auto& script : {app::configurationChangeScript(plan), app::configurationTargetScript(plan)}) {
    QVERIFY(script.exportSupported);
    std::vector<std::string> unprotected;
    for (const auto& command : script.commands)
      if (command.kind == app::UwfmgrKind::VolumeUnprotect) unprotected.push_back(command.args.front());
    QCOMPARE(unprotected, (std::vector<std::string>{compat::volumePath(mounted), compat::volumePath(offline)}));
    const auto output = app::renderConfigurationScript(script);
    QVERIFY(output.find(" && ") == std::string::npos);
    auto parsed = app::parseUwfmgrText(output);
    std::erase_if(parsed, [](const auto& command) { return command.parseError == app::ParseError::Comment; });
    QCOMPARE(parsed.size(), script.commands.size());
    for (const auto& command : parsed) QCOMPARE(command.parseError, app::ParseError::None);
  }
}

void ConfigurationScriptTests::bulkUnprotectExportRequiresAnIndependentlyConfirmedValidScope() {
  using namespace uwf;
  app::ConfigurationPlan plan;
  plan.snapshot.uwfAvailable = true;
  plan.snapshot.next.volumes = {{uwf::simulation::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::VolumeId, std::string("C:")}};
  plan.maintenance.allVolumesProtected = false;
  const auto check = [&](const bool supported) {
    for (const auto& script : {app::configurationChangeScript(plan), app::configurationTargetScript(plan)}) {
      QCOMPARE(script.exportSupported, supported);
      if (!supported) {
        QVERIFY(std::ranges::none_of(script.commands, [](const auto& command) { return command.kind == app::UwfmgrKind::VolumeUnprotect; }));
        QVERIFY(std::ranges::any_of(script.omissions,
                                    [](const auto& omission) { return omission.kind == app::ConfigurationScriptOmissionKind::IncompleteVolumeScope; }));
      }
    }
  };
  check(false);  // A readable settings projection is not a complete scope.
  plan.maintenanceSnapshot.emplace();
  plan.maintenanceSnapshot->unprotectTargetVolumes = {QString::fromLatin1(uwf::simulation::application::fixtureVolumeCName)};
  check(false);  // Partial identities do not establish completeness.
  plan.maintenanceSnapshot->unprotectScopeConfirmed = true;
  plan.maintenanceSnapshot->unprotectTargetVolumes.push_back(QStringLiteral("invalid-volume"));
  check(false);
  plan.maintenanceSnapshot->unprotectTargetVolumes.clear();
  plan.snapshot.next.volumes.clear();
  check(true);  // An explicitly complete empty registry is a valid no-op.
}

void ConfigurationScriptTests::exactHiddenGuidProtectionIsExportedAndMergedWithoutADriveAlias() {
  using namespace uwf;
  const std::string first = "11111111-2222-3333-4444-555555555555", second = "66666666-7777-8888-9999-aaaaaaaaaaaa";
  app::ConfigurationPlan plan;
  plan.snapshot.uwfAvailable = true;
  plan.snapshot.next.volumes = {{"Volume{" + first + "}", {}, false, core::VolumeBinding::VolumeId},
                                {"Volume{" + second + "}", {}, true, core::VolumeBinding::VolumeId}};
  plan.changes.volumeProtect[first] = true;
  plan.changes.volumeProtect[second] = false;
  for (const auto& script : {app::configurationChangeScript(plan), app::configurationTargetScript(plan)}) {
    QVERIFY(script.exportSupported);
    std::vector<app::UwfmgrCommand> protection;
    for (const auto& command : script.commands)
      if (command.kind == app::UwfmgrKind::VolumeProtect || command.kind == app::UwfmgrKind::VolumeUnprotect) protection.push_back(command);
    QCOMPARE(protection.size(), std::size_t{2});
    QCOMPARE(protection[0].kind, app::UwfmgrKind::VolumeProtect);
    QCOMPARE(protection[0].args, std::vector<std::string>{compat::volumePath(first)});
    QCOMPARE(protection[1].kind, app::UwfmgrKind::VolumeUnprotect);
    QCOMPARE(protection[1].args, std::vector<std::string>{compat::volumePath(second)});
    for (const auto& command : protection) {
      const auto rendered = app::renderUwfmgrCommand(command);
      QVERIFY(rendered.find("\"\\\\?\\Volume{") != std::string::npos);
      const auto parsed = app::parseUwfmgrText(rendered);
      QCOMPARE(parsed[0].parseError, app::ParseError::None);
      QCOMPARE(parsed[0].args, command.args);
    }
    QVERIFY(
        std::ranges::none_of(script.omissions, [](const auto& omission) { return omission.kind == app::ConfigurationScriptOmissionKind::VolumeWithoutDrive; }));
  }
}

void ConfigurationScriptTests::configurationScriptsOmitUnrepresentableTargetsAndDependencies_data() {
  QTest::addColumn<int>("control");
  QTest::addColumn<bool>("registry");
  QTest::addColumn<bool>("target");
  QTest::addColumn<bool>("chained");
  for (int control = 0; control <= 0x7f; ++control) {
    if (control >= 0x20 && control != 0x7f) continue;
    for (const bool registry : {false, true})
      for (const bool target : {false, true})
        for (const bool chained : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("control-%1-registry-%2-target-%3-chained-%4").arg(control).arg(registry).arg(target).arg(chained)))
              << control << registry << target << chained;
  }
}

void ConfigurationScriptTests::configurationScriptsOmitUnrepresentableTargetsAndDependencies() {
  QFETCH(int, control);
  QFETCH(bool, registry);
  QFETCH(bool, target);
  QFETCH(bool, chained);
  using namespace uwf::app;
  const auto prefix = std::string(registry ? "HKLM\\SOFTWARE\\" : "C:\\Data\\");
  const auto unsafe = prefix + static_cast<char>(control) + "UWF_OMITTED_TARGET";
  const auto safe = prefix + "SafeTarget";
  UwfmgrCommand manualCommand;
  manualCommand.kind = registry ? UwfmgrKind::RegistryAddExclusion : UwfmgrKind::FileAddExclusion;
  manualCommand.args = {unsafe};
  const ConfigurationScript manual{{manualCommand}, {}};
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)renderConfigurationScript(manual));
  ConfigurationPlan plan;
  plan.hasPolicyContext = chained;
  plan.snapshot.next.overlay = {uwf::core::OverlayType::RAM, 1024, 512, 768};
  plan.snapshot.next.volumes = {{uwf::simulation::application::fixtureVolumeCName, "C:", true, uwf::core::VolumeBinding::VolumeId, std::string("C:")}};
  if (target) {
    if (registry)
      plan.snapshot.next.registryExclusions = {unsafe, safe};
    else
      plan.snapshot.next.fileExclusions[uwf::simulation::application::fixtureVolumeCName] = {unsafe.substr(2), safe.substr(2)};
  } else if (registry)
    plan.changes.addRegistryExclusions = {unsafe, safe};
  else
    plan.changes.addFileExclusions["C:"] = {unsafe, safe};
  const auto product = target ? configurationTargetScript(plan) : configurationChangeScript(plan);
  const auto countOmission = [&](ConfigurationScriptOmissionKind kind) {
    return std::ranges::count_if(product.omissions, [kind](const auto& omission) { return omission.kind == kind; });
  };
  QCOMPARE(countOmission(ConfigurationScriptOmissionKind::UnrepresentableArgument), 1);
  QCOMPARE(countOmission(ConfigurationScriptOmissionKind::DependentCommand), chained ? 1 : 0);
  const auto kind = registry ? UwfmgrKind::RegistryAddExclusion : UwfmgrKind::FileAddExclusion;
  QCOMPARE(std::ranges::count_if(product.commands, [kind](const auto& command) { return command.kind == kind; }), chained ? 0 : 1);
  if (!chained) {
    const auto kept = std::ranges::find_if(product.commands, [kind](const auto& command) { return command.kind == kind; });
    QCOMPARE(kept->args, std::vector<std::string>{safe});
    QVERIFY(!kept->requiresPrevious);
  }
  const auto rendered = renderConfigurationScript(product);
  QVERIFY(rendered.find("UWF_OMITTED_TARGET") == std::string::npos);
  const auto scope = configurationScriptScope(product);
  QVERIFY(scope.find("Settings with control characters in paths or names") != std::string::npos);
  QCOMPARE(scope.find("Commands that depend on omitted settings") != std::string::npos, chained);
  auto parsed = parseUwfmgrText(rendered);
  std::erase_if(parsed, [](const auto& command) { return command.parseError == ParseError::Comment; });
  QCOMPARE(parsed.size(), product.commands.size());
  for (std::size_t index = 0; index < parsed.size(); ++index) {
    QCOMPARE(parsed[index].parseError, ParseError::None);
    QCOMPARE(parsed[index].kind, product.commands[index].kind);
    QCOMPARE(parsed[index].args, product.commands[index].args);
    QCOMPARE(parsed[index].requiresPrevious, product.commands[index].requiresPrevious);
  }
}

void ConfigurationScriptTests::uwfmgrRenderingPreservesBusinessOrderAndQuoting() {
  uwf::core::PendingChanges changes;
  changes.setFilterEnabled = true;
  changes.setOverlay.type = uwf::core::OverlayType::Disk;
  changes.setOverlay.maximumSizeMb = 8192;
  changes.setOverlay.warningThresholdMb = 4096;
  changes.setOverlay.criticalThresholdMb = 6144;
  changes.volumeProtect["C:"] = true;
  changes.volumeBindings["C:"] = uwf::core::VolumeBinding::VolumeId;
  changes.addFileExclusions["C:"] = {R"(C:\Program Files\Demo)"};
  changes.removeFileExclusions["C:"] = {R"(C:\Old)"};
  changes.addRegistryExclusions = {R"(HKEY_LOCAL_MACHINE\Software\Demo App)"};
  changes.removeRegistryExclusions = {R"(HKEY_LOCAL_MACHINE\System\Demo)"};
  changes.setPersistDomainSecretKey = true;

  const uwf::core::OverlayConfig baseline{uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  uwf::app::ConfigurationPlan plan;
  plan.changes = changes;
  plan.snapshot.next.overlay = baseline;
  plan.snapshot.next.volumes = {{uwf::simulation::application::fixtureVolumeCName, "C:", false, uwf::core::VolumeBinding::DriveLetter, std::string("C:")}};
  uwf::app::UwfmgrCommand protection;
  protection.kind = uwf::app::UwfmgrKind::VolumeProtect;
  protection.args = {uwf::compat::volumePath(uwf::test::application::fixtureVolumeId)};
  QCOMPARE(rendered(uwf::app::configurationChangeScript(plan).commands),
           QStringList({"uwfmgr.exe filter enable", "uwfmgr.exe overlay set-size 8192", "uwfmgr.exe overlay set-type Disk",
                        "uwfmgr.exe overlay set-criticalthreshold 6144", "uwfmgr.exe overlay set-warningthreshold 4096",
                        text(uwf::app::renderUwfmgrCommand(protection)), "uwfmgr.exe file add-exclusion \"C:\\Program Files\\Demo\"",
                        "uwfmgr.exe file remove-exclusion C:\\Old", "uwfmgr.exe registry add-exclusion \"HKEY_LOCAL_MACHINE\\Software\\Demo App\"",
                        "uwfmgr.exe registry remove-exclusion HKEY_LOCAL_MACHINE\\System\\Demo"}));
}

void ConfigurationScriptTests::plannedSessionRenderingPreservesFinalTargets() {
  using enum uwf::app::UwfmgrKind;
  uwf::core::SessionSnapshot next;
  next.filter.enabled = true;
  next.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  next.volumes = {{uwf::simulation::application::fixtureVolumeCName, "C:", true, uwf::core::VolumeBinding::DriveLetter, std::string("C:")},
                  {uwf::simulation::application::fixtureVolumeDName, "D:", false, uwf::core::VolumeBinding::DriveLetter, std::string("D:")}};
  next.fileExclusions[uwf::simulation::application::fixtureVolumeCName] = {R"(\Keep)", R"(\Old)"};
  next.registryExclusions = {R"(HKEY_LOCAL_MACHINE\SOFTWARE\Keep)", R"(HKEY_LOCAL_MACHINE\SOFTWARE\Old)"};

  uwf::core::PendingChanges changes;
  changes.setFilterEnabled = false;
  changes.setOverlay.type = uwf::core::OverlayType::Disk;
  changes.setOverlay.maximumSizeMb = 8192;
  changes.setOverlay.warningThresholdMb = 4096;
  changes.setOverlay.criticalThresholdMb = 6144;
  changes.volumeProtect["C:"] = false;
  changes.volumeProtect["E:"] = true;
  changes.addFileExclusions["C:"] = {R"(C:\New)"};
  changes.removeFileExclusions["C:"] = {R"(c:\old)"};
  changes.addRegistryExclusions = {R"(HKLM\SOFTWARE\New)"};
  changes.removeRegistryExclusions = {R"(hkey_local_machine\software\old)"};

  uwf::app::ConfigurationPlan plan;
  plan.changes = changes;
  plan.snapshot.next = next;
  const auto commands = uwf::app::configurationTargetScript(plan).commands;
  const auto count = [&](const uwf::app::UwfmgrKind kind) {
    return std::ranges::count_if(commands, [kind](const auto& command) { return command.kind == kind; });
  };
  QCOMPARE(count(FilterEnable), 0);
  QCOMPARE(count(FilterDisable), 1);
  QCOMPARE(count(OverlaySetSize), 1);
  QCOMPARE(count(VolumeProtect), 1);
  QCOMPARE(count(VolumeUnprotect), 2);
  QCOMPARE(count(FileAddExclusion), 2);
  QCOMPARE(count(FileRemoveExclusion), 1);
  QCOMPARE(count(RegistryAddExclusion), 2);
  QCOMPARE(count(RegistryRemoveExclusion), 1);
  const auto lines = rendered(commands);
  QVERIFY(lines.contains(QStringLiteral("uwfmgr.exe overlay set-type Disk")));
  QVERIFY(lines.contains(QStringLiteral("uwfmgr.exe overlay set-size 8192")));
  QVERIFY(lines.contains(QStringLiteral("uwfmgr.exe overlay set-warningthreshold 4096")));
  QVERIFY(lines.contains(QStringLiteral("uwfmgr.exe overlay set-criticalthreshold 6144")));
  QVERIFY(lines.contains(QStringLiteral("uwfmgr.exe volume protect E:")));
  QVERIFY(lines.contains(QStringLiteral("uwfmgr.exe file add-exclusion C:\\Keep")));
  QVERIFY(lines.contains(QStringLiteral("uwfmgr.exe file remove-exclusion c:\\old")));
  QVERIFY(lines.contains(QStringLiteral("uwfmgr.exe registry remove-exclusion hkey_local_machine\\software\\old")));
  QVERIFY(next.filter.enabled);
  QCOMPARE(next.overlay.maximumSizeMb, uint32_t{4096});
  QCOMPARE(next.fileExclusions.at(uwf::simulation::application::fixtureVolumeCName).size(), std::size_t{2});
}

void ConfigurationScriptTests::sessionRenderingSkipsVolumesWithoutDriveLetters() {
  uwf::core::SessionSnapshot session;
  session.filter.enabled = false;
  session.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  session.volumes = {{"Volume{A}", "C:", true, uwf::core::VolumeBinding::DriveLetter, std::string("C:")},
                     {"Volume{Hidden}", "", true, uwf::core::VolumeBinding::VolumeId},
                     {"Volume{D}", "D:", false, uwf::core::VolumeBinding::DriveLetter, std::string("D:")}};
  session.fileExclusions["Volume{A}"] = {R"(\Data)"};
  session.fileExclusions["Volume{Hidden}"] = {R"(\HiddenCache)", R"(C:\DetachedVolumeCache)"};
  session.registryExclusions = {R"(HKEY_LOCAL_MACHINE\Software\Demo)"};

  uwf::app::ConfigurationPlan plan;
  plan.snapshot.next = session;
  const auto script = uwf::app::configurationTargetScript(plan);
  const auto output = rendered(script.commands);
  QVERIFY(output.contains(QStringLiteral("uwfmgr.exe volume protect C:")));
  QVERIFY(output.contains(QStringLiteral("uwfmgr.exe volume unprotect D:")));
  for (const auto& line : output) QVERIFY(!line.contains(QStringLiteral("Hidden")));
  for (const auto& line : output) QVERIFY(!line.contains(QStringLiteral("DetachedVolumeCache")));
  const uwf::app::ConfigurationScriptOmission missing{uwf::app::ConfigurationScriptOmissionKind::VolumeWithoutDrive, "Volume{Hidden}"};
  QVERIFY(std::ranges::find(script.omissions, missing) != script.omissions.end());
}

void ConfigurationScriptTests::configurationScriptsDescribeOmittedSettings_data() {
  QTest::addColumn<bool>("target");
  QTest::addColumn<bool>("pending");
  for (const bool target : {false, true})
    for (const bool pending : {false, true}) QTest::newRow(qPrintable(QStringLiteral("target-%1-pending-%2").arg(target).arg(pending))) << target << pending;
}

void ConfigurationScriptTests::configurationScriptsDescribeOmittedSettings() {
  QFETCH(bool, target);
  QFETCH(bool, pending);
  using namespace uwf::app;
  ConfigurationPlan plan;
  plan.snapshot.next.overlay = {uwf::core::OverlayType::RAM, 1024, 512, 768};
  plan.snapshot.next.volumes = {{"Volume{A}", "C:", true, uwf::core::VolumeBinding::VolumeId, std::string("C:")}};
  plan.changes.setFilterEnabled = true;
  if (pending) {
    plan.changes.volumeBindings["C:"] = uwf::core::VolumeBinding::DriveLetter;
    plan.changes.setPersistDomainSecretKey = true;
    plan.changes.setPersistTSCAL = false;
  }
  auto product = target ? configurationTargetScript(plan) : configurationChangeScript(plan);
  QCOMPARE(product.omissions.size(), target || pending ? std::size_t{3} : std::size_t{0});
  const auto scope = configurationScriptScope(product);
  QCOMPARE(scope.empty(), !target && !pending);
  // Annotation target names never enter executable text. The scope records
  // categories, including every omitted change, without interpolating paths.
  for (auto& omission : product.omissions) omission.target = "C:%USERNAME% & echo unsafe|^>file\r\nfilter disable";
  const auto script = renderConfigurationScript(product);
  QVERIFY(script.find("echo unsafe") == std::string::npos);
  QVERIFY(script.find("%USERNAME%") == std::string::npos);
  auto parsed = parseUwfmgrText(script);
  if (!scope.empty()) {
    std::size_t comments = 0;
    for (const auto& row : parsed)
      if (row.parseError == ParseError::Comment) ++comments;
    std::size_t scopeLines = 0;
    std::istringstream lines(scope);
    for (std::string line; std::getline(lines, line);) {
      ++scopeLines;
      QVERIFY(script.find(":: " + line + "\r\n") != std::string::npos);
    }
    QCOMPARE(comments, scopeLines);
  }
  std::erase_if(parsed, [](const auto& row) { return row.parseError == ParseError::Comment; });
  QCOMPARE(parsed.size(), product.commands.size());
  for (std::size_t index = 0; index < parsed.size(); ++index) {
    QCOMPARE(parsed[index].parseError, ParseError::None);
    QCOMPARE(parsed[index].kind, product.commands[index].kind);
    QCOMPARE(parsed[index].args, product.commands[index].args);
    QCOMPARE(parsed[index].requiresPrevious, product.commands[index].requiresPrevious);
  }
}

void ConfigurationScriptTests::configurationScriptsPreserveDependenciesAndArguments() {
  using enum uwf::app::UwfmgrKind;
  uwf::core::PendingChanges changes;
  changes.setOverlay.type = uwf::core::OverlayType::Disk;
  changes.setOverlay.maximumSizeMb = 2048;
  changes.setOverlay.warningThresholdMb = 1024;
  changes.setOverlay.criticalThresholdMb = 1536;
  changes.volumeProtect["C:"] = true;
  const uwf::core::OverlayConfig baseline{uwf::core::OverlayType::RAM, 512, 256, 384};
  uwf::app::ConfigurationPlan plan;
  plan.changes = changes;
  plan.snapshot.next.overlay = baseline;
  const auto product = uwf::app::configurationChangeScript(plan);
  auto parsed = uwf::app::parseUwfmgrText(uwf::app::renderConfigurationScript(product));
  std::erase_if(parsed, [](const auto& row) { return row.parseError == uwf::app::ParseError::Comment; });
  const std::array kinds{OverlaySetSize, OverlaySetType, OverlaySetCriticalThreshold, OverlaySetWarningThreshold, VolumeProtect};
  const std::array<std::string, 5> arguments{"2048", "Disk", "1536", "1024", "C:"};
  const std::array dependent{false, true, false, true, false};
  QCOMPARE(parsed.size(), kinds.size());
  for (std::size_t index = 0; index < parsed.size(); ++index) {
    QCOMPARE(parsed[index].parseError, uwf::app::ParseError::None);
    QCOMPARE(parsed[index].kind, kinds[index]);
    QCOMPARE(parsed[index].args, std::vector<std::string>{arguments[index]});
    QCOMPARE(parsed[index].requiresPrevious, dependent[index]);
  }
}

void ConfigurationScriptTests::legacyGuardedCommandsPreserveDependenciesAndArguments() {
  using enum uwf::app::UwfmgrKind;
  const auto parsed = uwf::app::parseUwfmgrText(
      "uwfmgr.exe overlay set-size 2048 && uwfmgr.exe overlay set-type Disk\n"
      "uwfmgr.exe volume protect C:");
  QCOMPARE(parsed.size(), std::size_t{3});
  QCOMPARE(parsed[0].kind, OverlaySetSize);
  QCOMPARE(parsed[0].args, std::vector<std::string>{"2048"});
  QVERIFY(!parsed[0].requiresPrevious);
  QCOMPARE(parsed[1].kind, OverlaySetType);
  QCOMPARE(parsed[1].args, std::vector<std::string>{"Disk"});
  QVERIFY(parsed[1].requiresPrevious);
  QCOMPARE(parsed[2].kind, VolumeProtect);
  QCOMPARE(parsed[2].args, std::vector<std::string>{"C:"});
  QVERIFY(!parsed[2].requiresPrevious);
  for (const auto& command : parsed) QCOMPARE(command.parseError, uwf::app::ParseError::None);
  QCOMPARE(parsed[0].sourceLineNo, 1);
  QCOMPARE(parsed[1].sourceLineNo, 1);
  QCOMPARE(parsed[2].sourceLineNo, 2);
  const auto commentInChain = uwf::app::parseUwfmgrText("filter enable && # note && overlay set-size 2048");
  QCOMPARE(commentInChain.size(), std::size_t{3});
  QCOMPARE(commentInChain[1].parseError, uwf::app::ParseError::Unsupported);
  QVERIFY(commentInChain[1].requiresPrevious);
  QVERIFY(commentInChain[2].requiresPrevious);

  for (const auto& path : {R"(C:\A&&B)", R"(C:\A"&&B)", R"(C:\A^&&B)"}) {
    uwf::app::UwfmgrCommand command;
    command.kind = FileAddExclusion;
    command.args = {path};
    const auto roundTrip = uwf::app::parseUwfmgrText(uwf::app::renderUwfmgrCommand(command));
    QCOMPARE(roundTrip.size(), std::size_t{1});
    QCOMPARE(roundTrip[0].parseError, uwf::app::ParseError::None);
    QCOMPARE(roundTrip[0].args, command.args);
    QVERIFY(!roundTrip[0].requiresPrevious);
  }
  const auto escaped = uwf::app::parseUwfmgrText("file add-exclusion C:\\A^&^&B");
  QCOMPARE(escaped.size(), std::size_t{1});
  QCOMPARE(escaped[0].args, std::vector<std::string>{R"(C:\A&&B)"});
  for (const auto& text :
       {"&& filter disable", "filter disable &&", "filter disable && && volume protect C:", "filter disable && file add-exclusion \"unterminated"}) {
    const auto invalid = uwf::app::parseUwfmgrText(text);
    QCOMPARE(invalid.size(), std::size_t{1});
    QVERIFY(invalid[0].parseError != uwf::app::ParseError::None);
  }
}

}  // namespace
QTEST_APPLESS_MAIN(ConfigurationScriptTests)
#include "ConfigurationScriptTests.moc"
