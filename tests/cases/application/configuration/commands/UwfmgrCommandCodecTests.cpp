// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <QVersionNumber>
#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/commands/UwfmgrCommandParser.h"
#include "domain/uwf/contracts/Types.h"

namespace {
QString text(const std::string& value) { return QString::fromUtf8(value); }
QString helpSection(const QString& help, const QString& heading) {
  const QStringList headings{QStringLiteral("Description:"), QStringLiteral("Commands:"), QStringLiteral("Parameters:"), QStringLiteral("Examples:"),
                             QStringLiteral("Notes:")};
  bool inside = false;
  QStringList body;
  for (auto lines = help.split(u'\n'); const auto& line : std::as_const(lines)) {
    if (line == heading) {
      inside = true;
    } else if (inside && headings.contains(line)) {
      break;
    } else if (inside) {
      body.append(line);
    }
  }
  return body.join(u'\n');
}
QStringList exampleCommands(const QString& help) {
  QStringList examples;
  for (auto lines = helpSection(help, QStringLiteral("Examples:")).split(u'\n'); const auto& line : std::as_const(lines)) {
    const auto command = line.trimmed();
    if (command.startsWith(QStringLiteral("UWF.exe "), Qt::CaseInsensitive)) examples.append(command);
  }
  return examples;
}

class UwfmgrCommandCodecTests final : public QObject {
  Q_OBJECT
 private slots:
  void commandRenderersRejectControlCharacters_data();
  void commandRenderersRejectControlCharacters();
  void commandScriptsRoundTripLiteralArguments_data();
  void commandScriptsRoundTripLiteralArguments();
  void commandScriptEnvelopeControlsPercentDecoding();
  void guardedScriptsKeepCommandsSeparateAndSkipOnlyTheFailingChain();
  void guardedScriptDamageCannotRemoveDependenciesAndLegacyFilesRemainReadable();
  void scriptMarkerDiscoveryHandlesBomWithoutDemotingWhitespaceFraming();
  void uwfmgrParserPreservesLineOutcomes();
  void uwfmgrParserRejectsInvalidArgumentsPrecisely();
  void bulkProtectionAndGuidTargetsFollowTheNativeCliSyntax();
  void uwfmgrRendererRoundTripsEveryRepresentableCommand();
  void parsedQuotedWhitespaceDoesNotImplyCmdRepresentability_data();
  void parsedQuotedWhitespaceDoesNotImplyCmdRepresentability();
  void uwfmgrRendererQuotesShellCharactersAndTrailingSeparators();
  void uwfmgrParserHandlesDeterministicAdversarialInput();
  void publicCommandMatrixCoversEveryHelpCapability_data();
  void publicCommandMatrixCoversEveryHelpCapability();
  void publicCommandVariantsKeepScopeAndDefaultValues();
  void knownFamilyEntriesShareHelpSemantics_data();
  void knownFamilyEntriesShareHelpSemantics();
  void registryHelpDistinguishesKeyNamedAndDefaultValueOperations_data();
  void registryHelpDistinguishesKeyNamedAndDefaultValueOperations();
  void defaultHelpRetainsTheOfficialOverviewDensity();
  void familyHelpPreservesItsPublishedSectionsAndFacts_data();
  void familyHelpPreservesItsPublishedSectionsAndFacts();
  void publicCommandsValidateArityAndScalarBoundaries();
  void publicArgumentsAreNotRetokenizedOrDecodedAsCmd();
  void hiddenCommandsAndPrivateStartupFlagsAreRejected();
  void configurationTextKeepsItsDraftOnlyScope();
  void maintenanceCommandsUseNativeSyntaxAndPreserveDraftDependencies();
};

void UwfmgrCommandCodecTests::publicCommandMatrixCoversEveryHelpCapability_data() {
  using namespace uwf::app;
  using enum UwfmgrKind;
  struct Case {
    std::vector<std::string> input;
    UwfmgrKind kind;
    std::vector<std::string> operands;
  };
  // Independent public Usage inventory; aliases and protect-all share existing capabilities.
  // Compiled but unpublished verbs are excluded.
  const std::vector<Case> cases{
      {{"get-config"}, GetConfig, {}},
      {{"filter", "enable"}, FilterEnable, {}},
      {{"filter", "disable"}, FilterDisable, {}},
      {{"filter", "enable-HORM"}, FilterEnableHorm, {}},
      {{"filter", "disable-HORM"}, FilterDisableHorm, {}},
      {{"filter", "reset-settings"}, FilterResetSettings, {}},
      {{"volume", "protect", "c:"}, VolumeProtect, {"C:"}},
      {{"volume", "unprotect", "d:"}, VolumeUnprotect, {"D:"}},
      {{"volume", "create-swapfile", "e:"}, VolumeCreateSwapfile, {"E:"}},
      {{"volume", "get-config"}, VolumeGetConfig, {}},
      {{"file", "add-exclusion", R"(C:\Data)"}, FileAddExclusion, {R"(C:\Data)"}},
      {{"file", "remove-exclusion", R"(C:\Data)"}, FileRemoveExclusion, {R"(C:\Data)"}},
      {{"file", "get-exclusions"}, FileGetExclusions, {}},
      {{"file", "commit", R"(C:\Data\file)"}, FileCommit, {R"(C:\Data\file)"}},
      {{"file", "commit-delete", R"(C:\Data\file)"}, FileCommitDelete, {R"(C:\Data\file)"}},
      {{"overlay", "set-size", "01024"}, OverlaySetSize, {"1024"}},
      {{"overlay", "set-type", "rAm"}, OverlaySetType, {"RAM"}},
      {{"overlay", "set-passthrough", "ON"}, OverlaySetPassthrough, {"on"}},
      {{"overlay", "set-persistent", "Off"}, OverlaySetPersistent, {"off"}},
      {{"overlay", "set-rom-mode", "ON"}, OverlaySetReadOnlyMedia, {"on"}},
      {{"overlay", "reset-persistentstate", "rollback"}, OverlayResetPersistentState, {"on"}},
      {{"overlay", "set-warningthreshold", "0000"}, OverlaySetWarningThreshold, {"0"}},
      {{"overlay", "set-criticalthreshold", "4294967295"}, OverlaySetCriticalThreshold, {"4294967295"}},
      {{"overlay", "commit"}, OverlayCommit, {}},
      {{"overlay", "get-config"}, OverlayGetConfig, {}},
      {{"overlay", "get-availablespace"}, OverlayGetAvailableSpace, {}},
      {{"overlay", "get-consumption"}, OverlayGetConsumption, {}},
      {{"registry", "add-exclusion", R"(HKLM\Software\Test)"}, RegistryAddExclusion, {R"(HKLM\Software\Test)"}},
      {{"registry", "remove-exclusion", R"(HKLM\Software\Test)"}, RegistryRemoveExclusion, {R"(HKLM\Software\Test)"}},
      {{"registry", "get-exclusions"}, RegistryGetExclusions, {}},
      {{"registry", "commit", R"(HKLM\Software\Test)", ""}, RegistryCommit, {R"(HKLM\Software\Test)", ""}},
      {{"registry", "commit-delete", R"(HKLM\Software\Test)"}, RegistryCommitDelete, {R"(HKLM\Software\Test)"}},
      {{"servicing", "enable"}, ServicingEnable, {}},
      {{"servicing", "disable"}, ServicingDisable, {}},
      {{"servicing", "update-windows"}, ServicingUpdateWindows, {}},
      {{"servicing", "get-config"}, ServicingGetConfig, {}},
  };
  QCOMPARE(cases.size(), std::size_t{36});
  QCOMPARE(uwfmgrCommandCatalog().size(), cases.size());
  QTest::addColumn<QStringList>("arguments");
  QTest::addColumn<int>("expectedKind");
  QTest::addColumn<QStringList>("expectedOperands");
  std::vector<UwfmgrKind> visited;
  for (const auto& test : cases) {
    QVERIFY(std::ranges::find(visited, test.kind) == visited.end());
    visited.push_back(test.kind);
    QStringList input, expected;
    for (const auto& argument : test.input) input.append(QString::fromUtf8(argument));
    for (const auto& argument : test.operands) expected.append(QString::fromUtf8(argument));
    const auto label = input[0] + (input.size() > 1 ? QLatin1Char('.') + input[1] : QString{});
    QTest::newRow(qPrintable(label)) << input << static_cast<int>(test.kind) << expected;
  }
}

void UwfmgrCommandCodecTests::publicCommandMatrixCoversEveryHelpCapability() {
  using namespace uwf::app;
  QFETCH(QStringList, arguments);
  QFETCH(int, expectedKind);
  QFETCH(QStringList, expectedOperands);
  std::vector<std::string> input, expected;
  for (const auto& argument : arguments) input.push_back(argument.toUtf8().toStdString());
  for (const auto& argument : expectedOperands) expected.push_back(argument.toUtf8().toStdString());
  const auto parsed = parseUwfmgrArguments(input);
  QVERIFY2(parsed.parseError == ParseError::None, uwfmgrParseErrorText(parsed).c_str());
  QCOMPARE(static_cast<int>(parsed.kind), expectedKind);
  QCOMPARE(parsed.args, expected);
  const auto descriptor = std::ranges::find_if(uwfmgrCommandCatalog(), [&](const auto& value) { return value.kind == parsed.kind; });
  QVERIFY(descriptor != uwfmgrCommandCatalog().end());
  QVERIFY(!descriptor->description.empty());
  QVERIFY(text(uwfmgrHelpText(descriptor->family)).contains(text(std::string(descriptor->verb)), Qt::CaseInsensitive));
}

void UwfmgrCommandCodecTests::publicCommandVariantsKeepScopeAndDefaultValues() {
  using namespace uwf::app;
  const std::string id = "11111111-2222-3333-4444-555555555555";
  const auto guid = parseUwfmgrArguments({"volume", "protect", "\\\\?\\Volume{" + id + "}"});
  QCOMPARE(guid.parseError, ParseError::None);
  QCOMPARE(guid.args, std::vector<std::string>{uwf::compat::volumePath(id)});
  const auto device = R"(\\?\GLOBALROOT\Device\HarddiskVolume12)";
  QCOMPARE(parseUwfmgrArguments({"volume", "create-swapfile", device}).args, std::vector<std::string>{device});
  QCOMPARE(parseUwfmgrArguments({"volume", "protect", "ALL"}).kind, UwfmgrKind::VolumeProtectAll);
  QVERIFY(parseUwfmgrArguments({"volume", "protect", "all"}).args.empty());
  QCOMPARE(parseUwfmgrArguments({"volume", "get-config", "ALL"}).args, std::vector<std::string>{"all"});
  for (const auto& prefix :
       std::vector<std::vector<std::string>>{{"get-config"}, {"overlay", "get-config"}, {"registry", "get-exclusions"}, {"servicing", "get-config"}}) {
    for (const auto* session : {"CURRENT", "NeXt"}) {
      auto input = prefix;
      input.emplace_back(session);
      const auto parsed = parseUwfmgrArguments(input);
      QCOMPARE(parsed.parseError, ParseError::None);
      QCOMPARE(parsed.args, std::vector<std::string>{std::string(session) == "CURRENT" ? "current" : "next"});
    }
  }
  QCOMPARE(parseUwfmgrArguments({"file", "get-exclusions", "ALL", "NeXt"}).args, (std::vector<std::string>{"all", "next"}));
  const auto defaultDeletion = parseUwfmgrArguments({"registry", "commit-delete", R"(HKLM\Software\Test)", ""});
  const auto keyDeletion = parseUwfmgrArguments({"registry", "commit-delete", R"(HKLM\Software\Test)"});
  QCOMPARE(defaultDeletion.args.size(), std::size_t{2});
  QVERIFY(defaultDeletion.args.back().empty());
  QCOMPARE(keyDeletion.args.size(), std::size_t{1});
  for (const auto& [input, normalized] : {std::pair{"none", "off"}, {"off", "off"}, {"rollback", "on"}, {"on", "on"}}) {
    const auto parsed = parseUwfmgrArguments({"overlay", "reset-persistentstate", input});
    QCOMPARE(parsed.parseError, ParseError::None);
    QCOMPARE(parsed.args, std::vector<std::string>{normalized});
  }
  QCOMPARE(parseUwfmgrArguments({}).kind, UwfmgrKind::Help);
  for (const auto* alias : {"help", "?", "HeLP"}) {
    QCOMPARE(parseUwfmgrArguments({alias}).kind, UwfmgrKind::Help);
    QCOMPARE(parseUwfmgrArguments({"FILTER", alias}).args, std::vector<std::string>{"filter"});
  }
}

void UwfmgrCommandCodecTests::knownFamilyEntriesShareHelpSemantics_data() {
  QTest::addColumn<QStringList>("arguments");
  QTest::addColumn<QString>("expectedFamily");
  const std::array families{QStringLiteral("filter"), QStringLiteral("overlay"),  QStringLiteral("volume"),
                            QStringLiteral("file"),   QStringLiteral("registry"), QStringLiteral("servicing")};
  const std::array forms{std::pair{"bare", QString{}}, std::pair{"help", QStringLiteral("help")}, std::pair{"question", QStringLiteral("?")}};
  for (const auto& family : families) {
    auto mixed = family;
    for (qsizetype index = 0; index < mixed.size(); index += 2) mixed[index] = family.at(index).toUpper();
    const std::array casings{std::pair{"lower", family}, std::pair{"upper", family.toUpper()}, std::pair{"mixed", mixed}};
    for (const auto& [casingName, name] : casings)
      for (const auto& [formName, alias] : forms) {
        QStringList arguments{name};
        if (!alias.isEmpty()) arguments.append(alias);
        const auto label = family.toUtf8() + '/' + casingName + '/' + formName;
        QTest::newRow(label.constData()) << arguments << family;
      }
  }
}

void UwfmgrCommandCodecTests::knownFamilyEntriesShareHelpSemantics() {
  using namespace uwf::app;
  QFETCH(QStringList, arguments);
  QFETCH(QString, expectedFamily);
  std::vector<std::string> operands;
  for (const auto& argument : arguments) operands.push_back(argument.toUtf8().toStdString());
  const auto parsed = parseUwfmgrArguments(operands);
  QCOMPARE(parsed.parseError, ParseError::None);
  QCOMPARE(parsed.kind, UwfmgrKind::Help);
  QCOMPARE(parsed.args, std::vector<std::string>{expectedFamily.toUtf8().toStdString()});
  QVERIFY(!isUwfmgrConfigurationCommand(parsed.kind));
  const auto help = uwfmgrHelpText(parsed.args.front());
  QVERIFY(help.find("Usage: UWF.exe " + expectedFamily.toUtf8().toStdString()) != std::string::npos);
  // Family help accepts these spellings even when its captured Usage omits a help row.
  QVERIFY(!helpSection(text(help), QStringLiteral("Commands:")).trimmed().isEmpty());

  // Import requires command framing and rejects help as non-configuration input.
  const auto imported = parseUwfmgrText(arguments.join(QLatin1Char(' ')).toUtf8().toStdString());
  QCOMPARE(imported.size(), std::size_t{1});
  QCOMPARE(imported.front().parseError, arguments.size() == 1 ? ParseError::Incomplete : ParseError::Unsupported);
}

void UwfmgrCommandCodecTests::registryHelpDistinguishesKeyNamedAndDefaultValueOperations_data() {
  QTest::addColumn<QString>("family");
  QTest::newRow("top-level") << QString{};
  QTest::newRow("registry-family") << QStringLiteral("registry");
}

void UwfmgrCommandCodecTests::registryHelpDistinguishesKeyNamedAndDefaultValueOperations() {
  using namespace uwf::app;
  QFETCH(QString, family);
  const auto help = text(uwfmgrHelpText(family.toUtf8().toStdString())).simplified().toLower();
  if (family.isEmpty()) {
    QVERIFY(!help.contains(QStringLiteral("commit-delete")));
    QVERIFY(!help.contains(QStringLiteral("default value")));
    QVERIFY(!help.contains(QStringLiteral("whole-key deletion")));
    return;
  }
  QVERIFY(help.contains(QStringLiteral("commit <key> \"\"")));
  QVERIFY(help.contains(QStringLiteral("default value")));
  QVERIFY(help.contains(QStringLiteral("commit-delete <key>")));
  QVERIFY(help.contains(QStringLiteral("whole key")) || help.contains(QStringLiteral("whole-key")));
  QVERIFY(help.contains(QStringLiteral("named value")));
  QVERIFY(help.contains(QStringLiteral("empty value name")));
  QVERIFY(help.contains(QStringLiteral("rejected")));

  const std::string key = R"(HKLM\Software\Test)";
  const auto ordinaryDefault = parseUwfmgrArguments({"registry", "commit", key, ""});
  QCOMPARE(ordinaryDefault.parseError, ParseError::None);
  QCOMPARE(ordinaryDefault.kind, UwfmgrKind::RegistryCommit);
  QCOMPARE(ordinaryDefault.args, (std::vector<std::string>{key, ""}));
  const auto keyDeletion = parseUwfmgrArguments({"registry", "commit-delete", key});
  QCOMPARE(keyDeletion.parseError, ParseError::None);
  QCOMPARE(keyDeletion.args, std::vector<std::string>{key});
  const auto namedDeletion = parseUwfmgrArguments({"registry", "commit-delete", key, "Value"});
  QCOMPARE(namedDeletion.parseError, ParseError::None);
  QCOMPARE(namedDeletion.args, (std::vector<std::string>{key, "Value"}));
  const auto defaultDeletion = parseUwfmgrArguments({"registry", "commit-delete", key, ""});
  QCOMPARE(defaultDeletion.parseError, ParseError::None);
  QCOMPARE(defaultDeletion.kind, UwfmgrKind::RegistryCommitDelete);
  QCOMPARE(defaultDeletion.args, (std::vector<std::string>{key, ""}));
}

void UwfmgrCommandCodecTests::defaultHelpRetainsTheOfficialOverviewDensity() {
  using namespace uwf::app;
  const auto help = text(uwfmgrHelpText());
  const auto lines = help.split(u'\n');
  QVERIFY(lines.front().startsWith(QStringLiteral("UWF Manager version ")));
  const auto version = lines.front().mid(QStringLiteral("UWF Manager version ").size());
  QVERIFY(!QVersionNumber::fromString(version).isNull());
  QVERIFY(QVersionNumber::fromString(version).segmentCount() >= 3);
  QVERIFY(help.contains(QStringLiteral("Usage: UWF.exe ")));
  const auto description = helpSection(help, QStringLiteral("Description:")).simplified();
  QVERIFY(description.contains(QStringLiteral("Unified Write Filter"), Qt::CaseInsensitive));
  QVERIFY(description.contains(QStringLiteral("UWF")));
  const QStringList parameters{QStringLiteral("filter"),   QStringLiteral("overlay"),   QStringLiteral("volume"),     QStringLiteral("file"),
                               QStringLiteral("registry"), QStringLiteral("servicing"), QStringLiteral("get-config"), QStringLiteral("help")};
  QStringList observed;
  const auto parameterText = helpSection(help, QStringLiteral("Parameters:"));
  for (auto parameterLines = parameterText.split(u'\n'); const auto& line : std::as_const(parameterLines)) {
    const auto name = line.simplified().section(u' ', 0, 0).toLower();
    if (parameters.contains(name)) observed.append(name);
  }
  QCOMPARE(observed, parameters);
  const auto overview = parameterText.simplified();
  QVERIFY(overview.contains(QStringLiteral("current"), Qt::CaseInsensitive));
  QVERIFY(overview.contains(QStringLiteral("next"), Qt::CaseInsensitive));
  const QStringList expectedExamples{QStringLiteral("uwf.exe get-config"), QStringLiteral("uwf.exe filter enable"),
                                     QStringLiteral("uwf.exe volume protect c:")};
  auto examples = exampleCommands(help);
  for (auto& example : examples) example = example.toLower();
  QCOMPARE(examples, expectedExamples);
  QVERIFY(help.contains(QStringLiteral("UWF.exe filter help"), Qt::CaseInsensitive));
  // Root help is a compact overview; operand details belong to family help.
  QVERIFY(lines.size() <= 34);
  for (const auto& line : lines) QVERIFY2(line.size() <= 100, qPrintable(line));
}

void UwfmgrCommandCodecTests::familyHelpPreservesItsPublishedSectionsAndFacts_data() {
  QTest::addColumn<QString>("family");
  QTest::addColumn<QStringList>("commands");
  QTest::addColumn<bool>("parameters");
  QTest::addColumn<QStringList>("facts");
  QTest::newRow("filter") << QStringLiteral("filter") << QStringList{"enable", "disable", "enable-horm", "disable-horm", "reset-settings"} << false
                          << QStringList{"current session", "next session", "restart", "hibernate"};
  QTest::newRow("overlay") << QStringLiteral("overlay") << QStringList{"set-size",
                                                                       "set-type",
                                                                       "set-passthrough",
                                                                       "set-persistent",
                                                                       "set-rom-mode",
                                                                       "reset-persistentstate",
                                                                       "set-warningthreshold",
                                                                       "set-criticalthreshold",
                                                                       "commit",
                                                                       "get-config",
                                                                       "get-availablespace",
                                                                       "get-consumption"}
                           << false
                           << QStringList{"mb", "ram", "disk", "current session", "next session", "protected volumes", "restart", "rom", "none", "rollback"};
  QTest::newRow("volume") << QStringLiteral("volume") << QStringList{"protect", "unprotect", "create-swapfile", "get-config"} << true
                          << QStringList{"drive", "device", "guid", "all", "disk", "current", "next"};
  QTest::newRow("file") << QStringLiteral("file") << QStringList{"add-exclusion", "remove-exclusion", "get-exclusions", "commit", "commit-delete"} << true
                        << QStringList{"file", "path", "all", "directory", "empty", "last restart", "current", "next"};
  QTest::newRow("registry") << QStringLiteral("registry") << QStringList{"add-exclusion", "remove-exclusion", "get-exclusions", "commit", "commit-delete"}
                            << true << QStringList{"key", "value", "qualified", "next session", "default value", "empty value name", "rejected"};
  QTest::newRow("servicing") << QStringLiteral("servicing") << QStringList{"enable", "disable", "update-windows", "get-config"} << false
                             << QStringList{"current", "next", "windows", "restart", "3010"};
}

void UwfmgrCommandCodecTests::familyHelpPreservesItsPublishedSectionsAndFacts() {
  using namespace uwf::app;
  QFETCH(QString, family);
  QFETCH(QStringList, commands);
  QFETCH(bool, parameters);
  QFETCH(QStringList, facts);
  const auto help = text(uwfmgrHelpText(family.toUtf8().toStdString()));
  QVERIFY(help.startsWith(QStringLiteral("UWF Manager version ")));
  QVERIFY(help.contains(QStringLiteral("Usage: UWF.exe ") + family));
  QVERIFY(!helpSection(help, QStringLiteral("Description:")).simplified().isEmpty());
  const auto commandText = helpSection(help, QStringLiteral("Commands:"));
  QVERIFY(!commandText.simplified().isEmpty());
  QCOMPARE(help.contains(QStringLiteral("\nParameters:\n")), parameters);
  if (parameters) QVERIFY(!helpSection(help, QStringLiteral("Parameters:")).simplified().isEmpty());
  QStringList published;
  for (auto commandLines = commandText.split(u'\n'); const auto& line : std::as_const(commandLines)) {
    const auto command = line.simplified().section(u' ', 0, 0).toLower();
    if (commands.contains(command) && !published.contains(command)) published.append(command);
  }
  published.sort();
  commands.sort();
  QCOMPARE(published, commands);
  const auto normalized = help.simplified().toLower();
  for (const auto& fact : facts) QVERIFY2(normalized.contains(fact), qPrintable(family + QStringLiteral(": missing fact ") + fact));
  const auto examples = exampleCommands(help);
  QVERIFY(!examples.empty());
  for (const auto& example : examples) {
    // Change only the executable prefix; preserve the published operands and quoting.
    const auto imported = parseUwfmgrText((QStringLiteral("uwfmgr.exe") + example.mid(QStringLiteral("UWF.exe").size())).toUtf8().toStdString());
    QCOMPARE(imported.size(), std::size_t{1});
    const auto& command = imported.front();
    QVERIFY(command.kind != UwfmgrKind::Unknown);
    QVERIFY(command.parseError == ParseError::None || (command.parseError == ParseError::Unsupported && !isUwfmgrConfigurationCommand(command.kind)));
    if (command.kind == UwfmgrKind::Help) {
      QCOMPARE(command.args, std::vector<std::string>{family.toUtf8().toStdString()});
      continue;
    }
    const auto descriptor = std::ranges::find(uwfmgrCommandCatalog(), command.kind, &UwfmgrCommandDescriptor::kind);
    QVERIFY(descriptor != uwfmgrCommandCatalog().end());
    QCOMPARE(text(std::string(descriptor->family)), family);
  }
  for (auto descriptionLines = helpSection(help, QStringLiteral("Description:")).split(u'\n'); const auto& line : std::as_const(descriptionLines))
    QVERIFY2(line.size() <= 80, qPrintable(line));
}

void UwfmgrCommandCodecTests::publicCommandsValidateArityAndScalarBoundaries() {
  using namespace uwf::app;
  const std::vector<std::pair<std::vector<std::string>, ParseError>> invalid{
      {{"filters"}, ParseError::Unsupported},
      {{"filter", "enable", "extra"}, ParseError::UnexpectedArgument},
      {{"overlay", "set-size"}, ParseError::MissingSizeArg},
      {{"overlay", "set-size", "0"}, ParseError::InvalidSize},
      {{"overlay", "set-size", "-1"}, ParseError::InvalidSize},
      {{"overlay", "set-warningthreshold", "4294967296"}, ParseError::InvalidSize},
      {{"overlay", "set-criticalthreshold", "+1"}, ParseError::InvalidSize},
      {{"overlay", "set-size", "1 ", "extra"}, ParseError::UnexpectedArgument},
      {{"overlay", "set-type"}, ParseError::MissingTypeArg},
      {{"overlay", "set-type", "flash"}, ParseError::UnknownType},
      {{"overlay", "set-persistent", "true"}, ParseError::UnexpectedArgument},
      {{"overlay", "commit", "on"}, ParseError::UnexpectedArgument},
      {{"overlay", "reset-persistentstate", "savedmode"}, ParseError::UnexpectedArgument},
      {{"overlay", "reset-persistentstate", "commit"}, ParseError::UnexpectedArgument},
      {{"overlay", "get-config", "both"}, ParseError::UnexpectedArgument},
      {{"volume", "protect"}, ParseError::MissingVolumeArg},
      {{"volume", "protect", "C:\\Data"}, ParseError::InvalidVolume},
      {{"volume", "protect", "  C:"}, ParseError::InvalidVolume},
      {{"volume", "create-swapfile", "all"}, ParseError::Unsupported},
      {{"volume", "get-config", "all", "current"}, ParseError::UnexpectedArgument},
      {{"volume", "protect", "C:", "-driveletter"}, ParseError::UnexpectedArgument},
      {{"file", "commit", ""}, ParseError::MissingPathArg},
      {{"file", "get-exclusions", "all", "invalid"}, ParseError::UnexpectedArgument},
      {{"file", "get-exclusions", "all", "current", "extra"}, ParseError::UnexpectedArgument},
      {{"registry", "commit", "HKLM\\Software\\Test"}, ParseError::Incomplete},
      {{"registry", "commit", "", ""}, ParseError::MissingRegistryKeyArg},
      {{"registry", "commit-delete", "HKLM\\Software\\Test", "value", "extra"}, ParseError::UnexpectedArgument},
      {{"registry", "get-exclusions", "current", "extra"}, ParseError::UnexpectedArgument},
      {{"help", "filter"}, ParseError::UnexpectedArgument},
      {{"servicing", "update-windows", "extra"}, ParseError::UnexpectedArgument},
  };
  for (const auto& [input, error] : invalid) {
    const auto parsed = parseUwfmgrArguments(input);
    QCOMPARE(parsed.parseError, error);
    QVERIFY(!uwfmgrParseErrorText(parsed).empty());
  }
}

void UwfmgrCommandCodecTests::publicArgumentsAreNotRetokenizedOrDecodedAsCmd() {
  using namespace uwf::app;
  const std::string literal = "C:\\资料 %USERNAME% ^ & | > !name! \\\"quotes\\\" ";
  const auto parsed = parseUwfmgrArguments({"FILE", "COMMIT", literal});
  QCOMPARE(parsed.kind, UwfmgrKind::FileCommit);
  QCOMPARE(parsed.parseError, ParseError::None);
  QCOMPARE(parsed.args, std::vector<std::string>{literal});
  const std::string operand = "--quiet";
  QCOMPARE(parseUwfmgrArguments({"registry", "commit", "HKLM\\Software\\Test", operand}).args.back(), operand);
  const auto invalid = parseUwfmgrArguments({"file", "commit", std::string("C:\\bad\0target", 13)});
  QCOMPARE(invalid.parseError, ParseError::UnexpectedArgument);
  QCOMPARE(parseUwfmgrArguments({"file", "commit", std::string("\xc0\xaf", 2)}).parseError, ParseError::UnexpectedArgument);
}

void UwfmgrCommandCodecTests::hiddenCommandsAndPrivateStartupFlagsAreRejected() {
  using namespace uwf::app;
  for (const auto& input : std::vector<std::vector<std::string>>{{"filter", "get-config"},
                                                                 {"filter", "shutdown"},
                                                                 {"filter", "restart"},
                                                                 {"overlay", "reset-persistentstate-savedmode", "rollback"},
                                                                 {"overlay", "get-files"},
                                                                 {"overlay", "get-filedetails"},
                                                                 {"unattend"},
                                                                 {"--commit-stage"},
                                                                 {"--install"},
                                                                 {"--uninstall"},
                                                                 {"--version"},
                                                                 {"uwfmgr.exe", "filter", "enable"}}) {
    QCOMPARE(parseUwfmgrArguments(input).parseError, ParseError::Unsupported);
  }
}

void UwfmgrCommandCodecTests::configurationTextKeepsItsDraftOnlyScope() {
  using namespace uwf::app;
  const auto commands = parseUwfmgrText(
      "uwfmgr.exe overlay get-config\nuwfmgr file commit C:\\Data\\file\n"
      "uwfmgr servicing enable\n"
      "uwfmgr registry commit HKLM\\Software\\Test Value\n");
  QCOMPARE(commands.size(), std::size_t{4});
  for (const auto& command : commands) QCOMPARE(command.parseError, ParseError::Unsupported);
  const auto reset = parseUwfmgrText("overlay reset-persistentstate none\noverlay reset-persistentstate rollback");
  QCOMPARE(reset[0].args, std::vector<std::string>{"off"});
  QCOMPARE(reset[1].args, std::vector<std::string>{"on"});
  for (const auto& command : reset) QCOMPARE(command.parseError, ParseError::None);
}

void UwfmgrCommandCodecTests::maintenanceCommandsUseNativeSyntaxAndPreserveDraftDependencies() {
  using namespace uwf::app;
  struct Case {
    UwfmgrKind kind;
    std::vector<std::string> arguments;
    std::string nativeCommand;
  };
  const std::vector<Case> cases{
      {UwfmgrKind::FilterEnableHorm, {}, "uwfmgr.exe filter enable-HORM"},
      {UwfmgrKind::FilterDisableHorm, {}, "uwfmgr.exe filter disable-HORM"},
      {UwfmgrKind::OverlayCommit, {}, "uwfmgr.exe overlay commit"},
      {UwfmgrKind::VolumeCreateSwapfile, {"E:"}, "uwfmgr.exe volume create-swapfile E:"},
      {UwfmgrKind::VolumeCreateSwapfile,
       {R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\)"},
       R"(uwfmgr.exe volume create-swapfile "\\?\Volume{11111111-2222-3333-4444-555555555555}\\")"},
  };
  std::vector<UwfmgrCommand> commands;
  // These are independent syntax cases, not a claim that conflicting runtime
  // operations can be applied together. Import still uses each editor's gates.
  for (const auto& item : cases) {
    UwfmgrCommand command;
    command.kind = item.kind;
    command.args = item.arguments;
    command.requiresPrevious = !commands.empty();
    QCOMPARE(renderUwfmgrCommand(command), item.nativeCommand);
    QVERIFY(isUwfmgrConfigurationCommand(command.kind));
    commands.push_back(std::move(command));
  }
  const auto restored = parseUwfmgrText(renderUwfmgrCommandScript(commands));
  QCOMPARE(restored.size(), commands.size());
  for (std::size_t index = 0; index < commands.size(); ++index) {
    QCOMPARE(restored[index].parseError, ParseError::None);
    QCOMPARE(restored[index].kind, commands[index].kind);
    QCOMPARE(restored[index].args, commands[index].args);
    QCOMPARE(restored[index].requiresPrevious, commands[index].requiresPrevious);
  }
  // The native storage command selects a host; it has no capacity operand.
  QCOMPARE(parseUwfmgrText("uwfmgr volume create-swapfile E: 1024").front().parseError, ParseError::UnexpectedArgument);
}

void UwfmgrCommandCodecTests::commandRenderersRejectControlCharacters_data() {
  QTest::addColumn<int>("control");
  QTest::addColumn<bool>("registry");
  for (int control = 0; control <= 0x7f; ++control) {
    if (control >= 0x20 && control != 0x7f) continue;
    for (const bool registry : {false, true})
      QTest::newRow(qPrintable(QStringLiteral("control-%1-registry-%2").arg(control).arg(registry))) << control << registry;
  }
}

void UwfmgrCommandCodecTests::commandRenderersRejectControlCharacters() {
  QFETCH(int, control);
  QFETCH(bool, registry);
  using namespace uwf::app;
  UwfmgrCommand command;
  command.kind = registry ? UwfmgrKind::RegistryAddExclusion : UwfmgrKind::FileAddExclusion;
  command.args = {std::string(registry ? "HKLM\\SOFTWARE\\" : "C:\\Data\\") + static_cast<char>(control) + "UWF_OMITTED_TARGET"};
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)renderUwfmgrCommand(command));
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)renderUwfmgrCommandScript({command}));
}

void UwfmgrCommandCodecTests::commandScriptsRoundTripLiteralArguments_data() {
  QTest::addColumn<QString>("argument");
  QTest::newRow("defined-variable") << QStringLiteral("Z:\\%USERNAME%\\Cache");
  QTest::newRow("undefined-variable") << QStringLiteral("Z:\\%UWF_UNDEFINED_VARIABLE%\\Cache");
  QTest::newRow("batch-parameter") << QStringLiteral("Z:\\%1\\%%C\\100%");
  QTest::newRow("delayed-expansion") << QStringLiteral("Z:\\!USERNAME!\\A^B!");
  QTest::newRow("quotes-and-variables") << QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\A\"%USERNAME%&!name!\"B\\");
  QTest::newRow("unicode") << QStringLiteral("Z:\\资料 %USERNAME% !name!\\");
}

void UwfmgrCommandCodecTests::commandScriptsRoundTripLiteralArguments() {
  QFETCH(QString, argument);
  using namespace uwf::app;
  UwfmgrCommand first;
  first.kind = UwfmgrKind::FileAddExclusion;
  first.args = {argument.toStdString()};
  UwfmgrCommand second;
  second.kind = UwfmgrKind::FilterEnable;
  second.requiresPrevious = true;
  const std::vector commands{first, second};
  const auto script = renderUwfmgrCommandScript(commands);
  QVERIFY(script.starts_with(kUwfmgrScriptMarker));
  const auto parsed = parseUwfmgrText(script);
  QCOMPARE(parsed.size(), commands.size());
  for (std::size_t index = 0; index < parsed.size(); ++index) {
    QCOMPARE(parsed[index].parseError, ParseError::None);
    QCOMPARE(parsed[index].kind, commands[index].kind);
    QCOMPARE(parsed[index].args, commands[index].args);
    QCOMPARE(parsed[index].requiresPrevious, commands[index].requiresPrevious);
    QCOMPARE(parsed[index].sourceLineNo, 6 + static_cast<int>(index) * 3);
  }
}

void UwfmgrCommandCodecTests::commandScriptEnvelopeControlsPercentDecoding() {
  using namespace uwf::app;
  UwfmgrCommand command;
  command.kind = UwfmgrKind::FileAddExclusion;
  command.args = {R"(Z:\%USERNAME%\Cache)"};
  const auto script = renderUwfmgrCommandScript({command});
  const auto plain = parseUwfmgrText(R"(uwfmgr.exe file add-exclusion Z:\%%USERNAME%%\Cache)");
  QCOMPARE(plain.front().args.front(), std::string(R"(Z:\%%USERNAME%%\Cache)"));
  const auto mixed = parseUwfmgrText(script + renderUwfmgrCommand(command));
  QCOMPARE(mixed.size(), std::size_t{2});
  QCOMPARE(mixed[0].args, command.args);
  QCOMPARE(mixed[1].args, command.args);

  auto incomplete = script;
  incomplete.erase(incomplete.find("set \"_uwf_exit="));
  const auto unwrapped = parseUwfmgrText(incomplete);
  QCOMPARE(unwrapped.size(), std::size_t{1});
  QCOMPARE(unwrapped.front().parseError, ParseError::Unsupported);

  auto variable = text(script);
  variable.replace(QStringLiteral("%%USERNAME%%"), QStringLiteral("%USERNAME%"));
  const auto unsupported = parseUwfmgrText(variable.toStdString());
  QCOMPARE(unsupported.size(), std::size_t{1});
  QCOMPARE(unsupported.front().parseError, ParseError::Unsupported);
}

void UwfmgrCommandCodecTests::guardedScriptsKeepCommandsSeparateAndSkipOnlyTheFailingChain() {
  using namespace uwf::app;
  std::vector<UwfmgrCommand> commands(5);
  commands[0].kind = UwfmgrKind::FilterDisable;
  commands[1].kind = UwfmgrKind::OverlaySetSize;
  commands[1].args = {"4096"};
  commands[1].requiresPrevious = true;
  commands[2].kind = UwfmgrKind::OverlaySetType;
  commands[2].args = {"Disk"};
  commands[2].requiresPrevious = true;
  commands[3].kind = UwfmgrKind::VolumeProtect;
  commands[3].args = {"D:"};
  commands[4].kind = UwfmgrKind::FileAddExclusion;
  commands[4].args = {R"(D:\Data)"};
  commands[4].requiresPrevious = true;
  const auto script = renderUwfmgrCommandScript(commands);
  QVERIFY(script.find("&&") == std::string::npos);
  std::vector<std::string> lines;
  for (std::istringstream stream(script); stream.good();) {
    std::string line;
    std::getline(stream, line);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    lines.push_back(std::move(line));
  }
  QCOMPARE(std::ranges::count_if(lines, [](const auto& line) { return line.starts_with("uwfmgr.exe "); }), 5);
  const auto restored = parseUwfmgrText(script);
  QCOMPARE(restored.size(), commands.size());
  for (std::size_t index = 0; index < commands.size(); ++index) {
    QCOMPARE(restored[index].kind, commands[index].kind);
    QCOMPARE(restored[index].requiresPrevious, commands[index].requiresPrevious);
  }
  // Interpret the emitted control flow without executing a shell; include negative HRESULT exit codes.
  const auto run = [&](const UwfmgrKind failing, const int status) {
    std::vector<UwfmgrKind> executed;
    int exitCode = 0;
    for (std::size_t index = 5; index < lines.size(); ++index) {
      const auto& line = lines[index];
      if (line.starts_with("uwfmgr.exe ")) {
        const auto command = parseUwfmgrText(line).front();
        executed.push_back(command.kind);
        exitCode = command.kind == failing ? status : 0;
      } else if (line.starts_with("if errorlevel 1 goto ") || line.starts_with("if not errorlevel 0 goto ")) {
        const bool jump = line.starts_with("if not ") ? exitCode < 0 : exitCode >= 1;
        if (jump) {
          const auto label = ":" + line.substr(line.find("goto ") + 5);
          const auto found = std::ranges::find(lines, label);
          if (found == lines.end()) return std::vector<UwfmgrKind>{};
          index = static_cast<std::size_t>(std::distance(lines.begin(), found));
        }
      }
    }
    return executed;
  };
  const std::vector all{UwfmgrKind::FilterDisable, UwfmgrKind::OverlaySetSize, UwfmgrKind::OverlaySetType, UwfmgrKind::VolumeProtect,
                        UwfmgrKind::FileAddExclusion};
  QCOMPARE(run(UwfmgrKind::Unknown, 0), all);
  const std::vector firstFails{UwfmgrKind::FilterDisable, UwfmgrKind::VolumeProtect, UwfmgrKind::FileAddExclusion};
  QCOMPARE(run(UwfmgrKind::FilterDisable, 1), firstFails);
  QCOMPARE(run(UwfmgrKind::FilterDisable, -2147024891), firstFails);
  QCOMPARE(run(UwfmgrKind::OverlaySetSize, 5),
           (std::vector{UwfmgrKind::FilterDisable, UwfmgrKind::OverlaySetSize, UwfmgrKind::VolumeProtect, UwfmgrKind::FileAddExclusion}));
}

void UwfmgrCommandCodecTests::guardedScriptDamageCannotRemoveDependenciesAndLegacyFilesRemainReadable() {
  using namespace uwf::app;
  UwfmgrCommand first, second;
  first.kind = UwfmgrKind::FilterDisable;
  second.kind = UwfmgrKind::FilterEnable;
  second.requiresPrevious = true;
  const auto script = renderUwfmgrCommandScript({first, second});
  const std::vector<std::pair<std::string, std::string>> damage{{"if errorlevel 1 goto _uwf_chain_1_end\r\n", ""},
                                                                {"if not errorlevel 0 goto _uwf_chain_1_end", "if not errorlevel 0 goto _uwf_chain_2_end"},
                                                                {":_uwf_chain_1_end\r\n", ""},
                                                                {"setlocal DisableDelayedExpansion", "setlocal EnableDelayedExpansion"}};
  for (const auto& [from, to] : damage) {
    auto broken = script;
    const auto position = broken.find(from);
    QVERIFY(position != std::string::npos);
    broken.replace(position, from.size(), to);
    const auto parsed = parseUwfmgrText(broken);
    QCOMPARE(parsed.size(), std::size_t{1});
    QCOMPARE(parsed.front().parseError, ParseError::Unsupported);
  }
  const std::string legacy =
      ":: UWF Manager CMD script (UTF-8)\r\n@echo off\r\nsetlocal DisableDelayedExpansion\r\n"
      "for /f \"tokens=2 delims=:\" %%C in ('chcp') do set \"_uwf_codepage=%%C\"\r\nchcp 65001 >nul\r\n"
      "uwfmgr.exe file add-exclusion Z:\\%%USERNAME%%\\Cache && uwfmgr.exe filter enable\r\n"
      "set \"_uwf_exit=%errorlevel%\"\r\nchcp %_uwf_codepage% >nul\r\nendlocal & exit /b %_uwf_exit%\r\n";
  const auto parsed = parseUwfmgrText(legacy);
  QCOMPARE(parsed.size(), std::size_t{2});
  QCOMPARE(parsed[0].args.front(), std::string(R"(Z:\%USERNAME%\Cache)"));
  QVERIFY(!parsed[0].requiresPrevious);
  QVERIFY(parsed[1].requiresPrevious);
  QCOMPARE(parsed[0].parseError, ParseError::None);
  QCOMPARE(parsed[1].parseError, ParseError::None);
  const auto legacyWithBom = parseUwfmgrText(std::string("\xEF\xBB\xBF") + legacy);
  QCOMPARE(legacyWithBom.size(), std::size_t{2});
  QCOMPARE(legacyWithBom[0].args, parsed[0].args);
  QVERIFY(legacyWithBom[1].requiresPrevious);
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)renderUwfmgrCommandScript({second}));
}

void UwfmgrCommandCodecTests::scriptMarkerDiscoveryHandlesBomWithoutDemotingWhitespaceFraming() {
  using namespace uwf::app;
  const std::string bom = "\xEF\xBB\xBF";
  QVERIFY(isUwfmgrScriptMarker(kUwfmgrScriptMarker));
  QVERIFY(isUwfmgrScriptMarker(" \t" + std::string(kUwfmgrScriptMarker) + "\r "));
  QVERIFY(isUwfmgrScriptMarker(bom + std::string(kUwfmgrScriptMarker)));
  QVERIFY(isUwfmgrScriptMarker(":: UWF Manager CMD script (UTF-8)"));
  QVERIFY(isUwfmgrScriptMarker(bom + ":: UWF Manager CMD script (UTF-8)"));
  QVERIFY(!isUwfmgrScriptMarker("uwfmgr.exe filter disable"));
  UwfmgrCommand first, second;
  first.kind = UwfmgrKind::FilterDisable;
  second.kind = UwfmgrKind::FilterEnable;
  second.requiresPrevious = true;
  const auto script = renderUwfmgrCommandScript({first, second});
  for (const auto& prefix : {bom, std::string("# Loaded file\n") + bom}) {
    const auto parsed = parseUwfmgrText(prefix + script);
    const auto dependent = std::ranges::find_if(parsed, [](const auto& command) { return command.kind == UwfmgrKind::FilterEnable; });
    QVERIFY(dependent != parsed.end());
    QVERIFY(dependent->requiresPrevious);
    QCOMPARE(dependent->parseError, ParseError::None);
  }
  std::vector<std::string> damaged{" " + script, "\t" + script, bom + " " + script};
  auto trailing = script;
  trailing.insert(kUwfmgrScriptMarker.size(), " ");
  damaged.push_back(std::move(trailing));
  for (const auto& text : damaged) {
    const auto parsed = parseUwfmgrText(text);
    QCOMPARE(parsed.size(), std::size_t{1});
    QCOMPARE(parsed[0].parseError, ParseError::Unsupported);
    QVERIFY(std::ranges::none_of(parsed, [](const auto& command) { return command.kind == UwfmgrKind::FilterEnable; }));
  }
}

void UwfmgrCommandCodecTests::uwfmgrParserPreservesLineOutcomes() {
  const auto commands = uwf::app::parseUwfmgrText(
      "# exported rules\r\n"
      "UWFMgr.EXE FILTER enable\r\n"
      "overlay set-type disk\r\n"
      "volume protect c\r\n"
      "file add-exclusion \"C:\\Program Files\\Demo\"\r\n"
      "registry remove-exclusion \"HKLM\\Software\\Demo\"\r\n"
      ":: ignored\r\n");

  QCOMPARE(commands.size(), std::size_t{7});
  QCOMPARE(commands[0].parseError, uwf::app::ParseError::Comment);
  QCOMPARE(commands[1].kind, uwf::app::UwfmgrKind::FilterEnable);
  QCOMPARE(commands[1].sourceLineNo, 2);
  QCOMPARE(commands[2].args.at(0), std::string("Disk"));
  QCOMPARE(commands[3].args.at(0), std::string("C:"));
  QCOMPARE(commands[4].args.at(0), std::string(R"(C:\Program Files\Demo)"));
  QCOMPARE(commands[5].args.at(0), std::string(R"(HKLM\Software\Demo)"));
  QCOMPARE(commands[6].parseError, uwf::app::ParseError::Comment);
}

void UwfmgrCommandCodecTests::uwfmgrParserRejectsInvalidArgumentsPrecisely() {
  const auto commands = uwf::app::parseUwfmgrText(
      "filter\n"
      "overlay set-size\n"
      "overlay set-size -1\n"
      "overlay set-size 18446744073709551616\n"
      "overlay set-type flash\n"
      "volume protect all\n"
      "volume protect Volume{A}\n"
      "file add-exclusion\n"
      "registry remove-exclusion\n"
      "file add-exclusion \"C:\\unterminated\n"
      "filter enable unexpectedly\n"
      "filter disable \"\"\n"
      "file add-exclusion \"\"\n"
      "registry add-exclusion \"\"\n"
      "overlay get-config\n"
      "overlay set-size 0\n");

  const std::vector expected = {
      uwf::app::ParseError::Incomplete,       uwf::app::ParseError::MissingSizeArg,        uwf::app::ParseError::InvalidSize,
      uwf::app::ParseError::InvalidSize,      uwf::app::ParseError::UnknownType,           uwf::app::ParseError::None,
      uwf::app::ParseError::InvalidVolume,    uwf::app::ParseError::MissingPathArg,        uwf::app::ParseError::MissingRegistryKeyArg,
      uwf::app::ParseError::MalformedQuoting, uwf::app::ParseError::UnexpectedArgument,    uwf::app::ParseError::UnexpectedArgument,
      uwf::app::ParseError::MissingPathArg,   uwf::app::ParseError::MissingRegistryKeyArg, uwf::app::ParseError::Unsupported,
      uwf::app::ParseError::InvalidSize,
  };
  QCOMPARE(commands.size(), expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) QCOMPARE(commands[i].parseError, expected[i]);
  QCOMPARE(text(commands[4].parseErrorContext), QStringLiteral("flash"));
  QCOMPARE(text(commands[6].parseErrorContext), QStringLiteral("Volume{A}"));
  QCOMPARE(text(commands[10].parseErrorContext), QStringLiteral("unexpectedly"));
  QVERIFY(commands[11].parseErrorContext.empty());
}

void UwfmgrCommandCodecTests::bulkProtectionAndGuidTargetsFollowTheNativeCliSyntax() {
  using namespace uwf;
  const auto parsed = app::parseUwfmgrText("uwfmgr volume protect all\nuwfmgr volume unprotect all");
  QCOMPARE(parsed[0].kind, app::UwfmgrKind::VolumeProtectAll);
  QCOMPARE(parsed[0].parseError, app::ParseError::None);
  QVERIFY(parsed[0].args.empty());
  QCOMPARE(parsed[1].parseError, app::ParseError::Unsupported);
  const std::string id = "11111111-2222-3333-4444-555555555555";
  app::UwfmgrCommand target;
  target.kind = app::UwfmgrKind::VolumeUnprotect;
  target.args = {compat::volumePath(id)};
  const auto text = app::renderUwfmgrCommand(target);
  const auto roundTrip = app::parseUwfmgrText(text);
  QCOMPARE(roundTrip.size(), std::size_t{1});
  QCOMPARE(roundTrip[0].parseError, app::ParseError::None);
  QCOMPARE(roundTrip[0].args, target.args);
  QVERIFY(text.starts_with("uwfmgr.exe volume unprotect \"\\\\?\\Volume{"));
  // Native CVolumeCommand's GUID branch requires the complete Win32 prefix.
  QCOMPARE(app::parseUwfmgrText("uwfmgr volume unprotect {" + id + "}")[0].parseError, app::ParseError::InvalidVolume);
  QCOMPARE(app::parseUwfmgrText("uwfmgr volume unprotect " + id)[0].parseError, app::ParseError::InvalidVolume);
}

void UwfmgrCommandCodecTests::uwfmgrRendererRoundTripsEveryRepresentableCommand() {
  using enum uwf::app::UwfmgrKind;
  const auto command = [](const uwf::app::UwfmgrKind kind, std::vector<std::string> args = {}) {
    uwf::app::UwfmgrCommand value;
    value.kind = kind;
    value.args = std::move(args);
    return value;
  };
  const std::vector<uwf::app::UwfmgrCommand> commands = {
      command(FilterEnable),
      command(FilterDisable),
      command(FilterEnableHorm),
      command(FilterDisableHorm),
      command(OverlaySetType, {"Disk"}),
      command(OverlaySetSize, {"1"}),
      command(OverlaySetWarningThreshold, {"0"}),
      command(OverlaySetWarningThreshold, {"4294967295"}),
      command(OverlaySetCriticalThreshold, {"4096"}),
      command(OverlaySetCriticalThreshold, {"0"}),
      command(OverlaySetPassthrough, {"on"}),
      command(OverlaySetPassthrough, {"off"}),
      command(OverlaySetPersistent, {"on"}),
      command(OverlaySetPersistent, {"off"}),
      command(OverlaySetReadOnlyMedia, {"on"}),
      command(OverlaySetReadOnlyMedia, {"off"}),
      command(OverlayResetPersistentState, {"on"}),
      command(OverlayResetPersistentState, {"off"}),
      command(OverlayCommit),
      command(VolumeProtect, {"C:"}),
      command(VolumeProtectAll),
      command(VolumeCreateSwapfile, {"D:"}),
      command(VolumeCreateSwapfile, {uwf::compat::volumePath("11111111-2222-3333-4444-555555555555")}),
      command(VolumeUnprotect, {"Z:"}),
      command(VolumeUnprotect, {uwf::compat::volumePath("11111111-2222-3333-4444-555555555555")}),
      command(FileAddExclusion, {R"(C:\Program Files\Demo)"}),
      command(FileRemoveExclusion, {R"(D:\Cache)"}),
      command(RegistryAddExclusion, {R"(HKEY_LOCAL_MACHINE\Software\Demo App)"}),
      command(RegistryRemoveExclusion, {R"(HKEY_LOCAL_MACHINE\System\Demo)"}),
      command(FileAddExclusion, {"C:\\资料\\Literal space "}),
      command(RegistryAddExclusion, {"HKLM\\Software\\Literal space "}),
  };
  // ASCII controls are covered by the renderer rejection matrix above.
  for (const auto& expected : commands) {
    const std::string line = uwf::app::renderUwfmgrCommand(expected);
    const auto parsed = uwf::app::parseUwfmgrText(line);
    QCOMPARE(parsed.size(), std::size_t{1});
    QCOMPARE(parsed.front().parseError, uwf::app::ParseError::None);
    QCOMPARE(parsed.front().kind, expected.kind);
    QCOMPARE(parsed.front().args, expected.args);
    QCOMPARE(parsed.front().sourceLineNo, 1);
  }
}

void UwfmgrCommandCodecTests::parsedQuotedWhitespaceDoesNotImplyCmdRepresentability_data() {
  QTest::addColumn<bool>("registry");
  QTest::addColumn<QString>("argument");
  for (const auto& [name, control] : {std::pair{"tab", '\t'}, std::pair{"vertical-tab", '\v'}})
    for (const bool registry : {false, true}) {
      const auto prefix = registry ? QStringLiteral("HKLM\\SOFTWARE\\Literal") : QStringLiteral("C:\\Literal");
      const auto label = QString::fromLatin1(name) + (registry ? QStringLiteral("/registry") : QStringLiteral("/file"));
      QTest::newRow(qPrintable(label)) << registry << prefix + QLatin1Char(control) + QStringLiteral("Whitespace");
    }
}

void UwfmgrCommandCodecTests::parsedQuotedWhitespaceDoesNotImplyCmdRepresentability() {
  QFETCH(bool, registry);
  QFETCH(QString, argument);
  const auto prefix = registry ? "uwfmgr registry add-exclusion \"" : "uwfmgr file add-exclusion \"";
  const auto parsed = uwf::app::parseUwfmgrText(prefix + argument.toStdString() + '"');
  QCOMPARE(parsed.size(), std::size_t{1});
  QCOMPARE(parsed.front().parseError, uwf::app::ParseError::None);
  QCOMPARE(parsed.front().kind, registry ? uwf::app::UwfmgrKind::RegistryAddExclusion : uwf::app::UwfmgrKind::FileAddExclusion);
  QCOMPARE(parsed.front().args, std::vector<std::string>{argument.toStdString()});
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)uwf::app::renderUwfmgrCommand(parsed.front()));
}

void UwfmgrCommandCodecTests::uwfmgrRendererQuotesShellCharactersAndTrailingSeparators() {
  const std::vector<std::string> paths = {
      R"(Z:\R&ver)",
      R"(Z:\A (B)\)",
      R"(Z:\A^B)",
      R"(Z:\With space\)",
      R"(HKEY_LOCAL_MACHINE\Software\A"&ver&"B)",
      R"(HKEY_LOCAL_MACHINE\Software\A\"&ver&"B\)",
      R"(HKEY_LOCAL_MACHINE\Software\A"&ver)",
      R"(HKEY_LOCAL_MACHINE\Software\A"^&|<>() B\)",
      R"(HKEY_LOCAL_MACHINE\Software\A""B)",
  };
  for (const auto& path : paths) {
    uwf::app::UwfmgrCommand command;
    command.kind = uwf::app::UwfmgrKind::FileAddExclusion;
    command.args = {path};
    const std::string line = uwf::app::renderUwfmgrCommand(command);
    QVERIFY(line.starts_with("uwfmgr.exe file add-exclusion \""));
    const auto parsed = uwf::app::parseUwfmgrText(line);
    QCOMPARE(parsed.size(), std::size_t{1});
    QCOMPARE(parsed.front().parseError, uwf::app::ParseError::None);
    QCOMPARE(parsed.front().args, command.args);
  }
}

void UwfmgrCommandCodecTests::uwfmgrParserHandlesDeterministicAdversarialInput() {
  struct ExpectedLine {
    uwf::app::UwfmgrKind kind = uwf::app::UwfmgrKind::Unknown;
    uwf::app::ParseError error = uwf::app::ParseError::None;
    std::vector<std::string> args;
  };

  std::string input;
  std::vector<ExpectedLine> expected;
  constexpr int kLineCount = 512;
  expected.reserve(kLineCount);
  for (int index = 0; index < kLineCount; ++index) {
    const uint32_t mixed = static_cast<uint32_t>(index) * 2654435761u;
    const bool hasExecutablePrefix = index % 3 == 0;
    input += hasExecutablePrefix ? "  UWFMgr.ExE  " : "";
    switch (mixed % 8u) {
      case 0:
        input += "filter enable";
        expected.push_back({uwf::app::UwfmgrKind::FilterEnable, uwf::app::ParseError::None, {}});
        break;
      case 1:
        input += "overlay set-size " + std::to_string(mixed);
        expected.push_back({uwf::app::UwfmgrKind::OverlaySetSize, uwf::app::ParseError::None, {std::to_string(mixed)}});
        break;
      case 2: {
        const char letter = static_cast<char>('a' + index % 26);
        input += "volume protect " + std::string(1, letter);
        expected.push_back({uwf::app::UwfmgrKind::VolumeProtect, uwf::app::ParseError::None, {std::string(1, static_cast<char>(letter - 'a' + 'A')) + ':'}});
      } break;
      case 3:
        input += R"(file add-exclusion "C:\Path With Spaces\item")";
        expected.push_back({uwf::app::UwfmgrKind::FileAddExclusion, uwf::app::ParseError::None, {R"(C:\Path With Spaces\item)"}});
        break;
      case 4:
        input += "registry remove-exclusion HKLM\\Software\\" + std::to_string(index);
        expected.push_back({uwf::app::UwfmgrKind::RegistryRemoveExclusion, uwf::app::ParseError::None, {"HKLM\\Software\\" + std::to_string(index)}});
        break;
      case 5:
        input += "# comment " + std::to_string(index);
        expected.push_back({uwf::app::UwfmgrKind::Unknown, hasExecutablePrefix ? uwf::app::ParseError::Unsupported : uwf::app::ParseError::Comment, {}});
        break;
      case 6:
        input += "unknown-" + std::to_string(mixed);
        expected.push_back({uwf::app::UwfmgrKind::Unknown, uwf::app::ParseError::Incomplete, {}});
        break;
      default:
        input.append(static_cast<std::size_t>(index % 17), ' ');
        expected.push_back({uwf::app::UwfmgrKind::Unknown, hasExecutablePrefix ? uwf::app::ParseError::Incomplete : uwf::app::ParseError::Comment, {}});
        break;
    }
    input += index % 2 == 0 ? "\r\n" : "\n";
  }

  const auto first = uwf::app::parseUwfmgrText(input);
  const auto second = uwf::app::parseUwfmgrText(input);
  QCOMPARE(first.size(), std::size_t{kLineCount});
  QCOMPARE(expected.size(), first.size());
  QCOMPARE(second.size(), first.size());
  for (std::size_t index = 0; index < first.size(); ++index) {
    QCOMPARE(first[index].sourceLineNo, static_cast<int>(index) + 1);
    QCOMPARE(first[index].kind, expected[index].kind);
    QCOMPARE(first[index].parseError, expected[index].error);
    QCOMPARE(first[index].args, expected[index].args);
    QCOMPARE(first[index].kind, second[index].kind);
    QCOMPARE(first[index].args, second[index].args);
    QCOMPARE(first[index].parseError, second[index].parseError);
    QCOMPARE(first[index].parseErrorContext, second[index].parseErrorContext);
  }
}

}  // namespace
QTEST_APPLESS_MAIN(UwfmgrCommandCodecTests)
#include "UwfmgrCommandCodecTests.moc"
