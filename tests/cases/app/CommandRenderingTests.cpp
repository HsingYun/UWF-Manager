// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/configuration/ConfigurationDraft.h"
#include "app/configuration/UwfmgrCommands.h"
#include "core/OverlayWritePlan.h"

namespace {

QString text(const std::string& value) { return QString::fromUtf8(value); }

QStringList rendered(const std::vector<uwf::app::UwfmgrCommand>& commands) {
  QStringList result;
  result.reserve(static_cast<qsizetype>(commands.size()));
  for (const auto& command : commands) result.append(text(uwf::app::renderCommand(command)));
  return result;
}

class CommandRenderingTests final : public QObject {
  Q_OBJECT

 private slots:
  void uwfmgrParserPreservesLineOutcomes();
  void uwfmgrParserRejectsInvalidArgumentsPrecisely();
  void uwfmgrRenderingPreservesBusinessOrderAndQuoting();
  void uwfmgrRendererRoundTripsEverySupportedCommand();
  void uwfmgrRendererQuotesShellCharactersAndTrailingSeparators();
  void uwfmgrGuardedCommandsPreserveDependenciesAndArguments();
  void commandScriptsRoundTripLiteralArguments_data();
  void commandScriptsRoundTripLiteralArguments();
  void commandScriptEnvelopeControlsPercentDecoding();
  void plannedSessionRenderingPreservesFinalTargets();
  void uwfmgrParserHandlesDeterministicAdversarialInput();
  void sessionRenderingSkipsVolumesWithoutDriveLetters();
};

void CommandRenderingTests::commandScriptsRoundTripLiteralArguments_data() {
  QTest::addColumn<QString>("argument");
  QTest::newRow("defined-variable") << QStringLiteral("Z:\\%USERNAME%\\Cache");
  QTest::newRow("undefined-variable") << QStringLiteral("Z:\\%UWF_UNDEFINED_VARIABLE%\\Cache");
  QTest::newRow("batch-parameter") << QStringLiteral("Z:\\%1\\%%C\\100%");
  QTest::newRow("delayed-expansion") << QStringLiteral("Z:\\!USERNAME!\\A^B!");
  QTest::newRow("quotes-and-variables") << QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\A\"%USERNAME%&!name!\"B\\");
  QTest::newRow("unicode") << QStringLiteral("Z:\\资料 %USERNAME% !name!\\");
}

void CommandRenderingTests::commandScriptsRoundTripLiteralArguments() {
  QFETCH(QString, argument);
  using namespace uwf::app;
  UwfmgrCommand first;
  first.kind = UwfmgrKind::FileAddExclusion;
  first.args = {argument.toStdString()};
  UwfmgrCommand second;
  second.kind = UwfmgrKind::FilterEnable;
  second.requiresPrevious = true;
  const std::vector commands{first, second};
  const auto script = renderCommandScript(commands);
  QVERIFY(script.starts_with(kUwfmgrScriptMarker));
  const auto parsed = parseUwfmgrText(script);
  QCOMPARE(parsed.size(), commands.size());
  for (std::size_t index = 0; index < parsed.size(); ++index) {
    QCOMPARE(parsed[index].parseError, ParseError::None);
    QCOMPARE(parsed[index].kind, commands[index].kind);
    QCOMPARE(parsed[index].args, commands[index].args);
    QCOMPARE(parsed[index].requiresPrevious, commands[index].requiresPrevious);
    QCOMPARE(parsed[index].sourceLineNo, 6);
  }
}

void CommandRenderingTests::commandScriptEnvelopeControlsPercentDecoding() {
  using namespace uwf::app;
  UwfmgrCommand command;
  command.kind = UwfmgrKind::FileAddExclusion;
  command.args = {R"(Z:\%USERNAME%\Cache)"};
  const auto script = renderCommandScript({command});
  const auto plain = parseUwfmgrText(R"(uwfmgr.exe file add-exclusion Z:\%%USERNAME%%\Cache)");
  QCOMPARE(plain.front().args.front(), std::string(R"(Z:\%%USERNAME%%\Cache)"));
  const auto mixed = parseUwfmgrText(script + renderCommand(command));
  QCOMPARE(mixed.size(), std::size_t{2});
  QCOMPARE(mixed[0].args, command.args);
  QCOMPARE(mixed[1].args, command.args);

  auto incomplete = script;
  incomplete.erase(incomplete.find("set \"_uwf_exit="));
  const auto unwrapped = parseUwfmgrText(incomplete);
  const auto body = std::ranges::find_if(unwrapped, [](const auto& value) { return value.kind == UwfmgrKind::FileAddExclusion; });
  QVERIFY(body != unwrapped.end());
  QCOMPARE(body->args.front(), std::string(R"(Z:\%%USERNAME%%\Cache)"));

  auto variable = text(script);
  variable.replace(QStringLiteral("%%USERNAME%%"), QStringLiteral("%USERNAME%"));
  const auto unsupported = parseUwfmgrText(variable.toStdString());
  QCOMPARE(unsupported.size(), std::size_t{1});
  QCOMPARE(unsupported.front().parseError, ParseError::Unsupported);
}

void CommandRenderingTests::uwfmgrParserPreservesLineOutcomes() {
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

void CommandRenderingTests::uwfmgrParserRejectsInvalidArgumentsPrecisely() {
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
      "overlay get-config\n");

  const std::vector expected = {
      uwf::app::ParseError::Incomplete,       uwf::app::ParseError::MissingSizeArg,        uwf::app::ParseError::InvalidSize,
      uwf::app::ParseError::InvalidSize,      uwf::app::ParseError::UnknownType,           uwf::app::ParseError::Unsupported,
      uwf::app::ParseError::InvalidVolume,    uwf::app::ParseError::MissingPathArg,        uwf::app::ParseError::MissingRegistryKeyArg,
      uwf::app::ParseError::MalformedQuoting, uwf::app::ParseError::UnexpectedArgument,    uwf::app::ParseError::UnexpectedArgument,
      uwf::app::ParseError::MissingPathArg,   uwf::app::ParseError::MissingRegistryKeyArg, uwf::app::ParseError::Unsupported,
  };
  QCOMPARE(commands.size(), expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) QCOMPARE(commands[i].parseError, expected[i]);
  QCOMPARE(text(commands[4].parseErrorContext), QStringLiteral("flash"));
  QCOMPARE(text(commands[6].parseErrorContext), QStringLiteral("Volume{A}"));
  QCOMPARE(text(commands[10].parseErrorContext), QStringLiteral("unexpectedly"));
  QVERIFY(commands[11].parseErrorContext.empty());
}

void CommandRenderingTests::uwfmgrRenderingPreservesBusinessOrderAndQuoting() {
  uwf::core::PendingChanges changes;
  changes.setFilterEnabled = true;
  changes.setOverlay.type = uwf::core::OverlayType::Disk;
  changes.setOverlay.maximumSizeMb = 8192;
  changes.setOverlay.warningThresholdMb = 4096;
  changes.setOverlay.criticalThresholdMb = 6144;
  changes.volumeProtect["C:"] = true;
  changes.volumeBindByVolumeName["C:"] = true;
  changes.addFileExclusions["C:"] = {R"(C:\Program Files\Demo)"};
  changes.removeFileExclusions["C:"] = {R"(C:\Old)"};
  changes.addRegistryExclusions = {R"(HKEY_LOCAL_MACHINE\Software\Demo App)"};
  changes.removeRegistryExclusions = {R"(HKEY_LOCAL_MACHINE\System\Demo)"};
  changes.setPersistDomainSecretKey = true;

  const uwf::core::OverlayConfig baseline{uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  QCOMPARE(rendered(uwf::app::renderPendingChanges(changes, baseline)),
           QStringList({"uwfmgr.exe filter enable", "uwfmgr.exe overlay set-size 8192", "uwfmgr.exe overlay set-type Disk",
                        "uwfmgr.exe overlay set-criticalthreshold 6144", "uwfmgr.exe overlay set-warningthreshold 4096",
                        "uwfmgr.exe volume protect C:", "uwfmgr.exe file add-exclusion \"C:\\Program Files\\Demo\"", "uwfmgr.exe file remove-exclusion C:\\Old",
                        "uwfmgr.exe registry add-exclusion \"HKEY_LOCAL_MACHINE\\Software\\Demo App\"",
                        "uwfmgr.exe registry remove-exclusion HKEY_LOCAL_MACHINE\\System\\Demo"}));
}

void CommandRenderingTests::uwfmgrRendererRoundTripsEverySupportedCommand() {
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
      command(OverlaySetType, {"Disk"}),
      command(OverlaySetSize, {"0"}),
      command(OverlaySetWarningThreshold, {"18446744073709551615"}),
      command(OverlaySetCriticalThreshold, {"4096"}),
      command(VolumeProtect, {"C:"}),
      command(VolumeUnprotect, {"Z:"}),
      command(FileAddExclusion, {R"(C:\Program Files\Demo)"}),
      command(FileRemoveExclusion, {R"(D:\Cache)"}),
      command(RegistryAddExclusion, {R"(HKEY_LOCAL_MACHINE\Software\Demo App)"}),
      command(RegistryRemoveExclusion, {R"(HKEY_LOCAL_MACHINE\System\Demo)"}),
      command(FileAddExclusion, {"C:\\Path\tWithTab"}),
      command(RegistryAddExclusion, {"HKEY_LOCAL_MACHINE\\Software\\Line\vSeparated"}),
  };

  for (const auto& expected : commands) {
    const std::string line = uwf::app::renderCommand(expected);
    const auto parsed = uwf::app::parseUwfmgrText(line);
    QCOMPARE(parsed.size(), std::size_t{1});
    QCOMPARE(parsed.front().parseError, uwf::app::ParseError::None);
    QCOMPARE(parsed.front().kind, expected.kind);
    QCOMPARE(parsed.front().args, expected.args);
    QCOMPARE(parsed.front().sourceLineNo, 1);
  }
}

void CommandRenderingTests::uwfmgrRendererQuotesShellCharactersAndTrailingSeparators() {
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
    const std::string line = uwf::app::renderCommand(command);
    QVERIFY(line.starts_with("uwfmgr.exe file add-exclusion \""));
    const auto parsed = uwf::app::parseUwfmgrText(line);
    QCOMPARE(parsed.size(), std::size_t{1});
    QCOMPARE(parsed.front().parseError, uwf::app::ParseError::None);
    QCOMPARE(parsed.front().args, command.args);
  }
}

void CommandRenderingTests::uwfmgrGuardedCommandsPreserveDependenciesAndArguments() {
  using enum uwf::app::UwfmgrKind;
  uwf::core::PendingChanges changes;
  changes.setOverlay.type = uwf::core::OverlayType::Disk;
  changes.setOverlay.maximumSizeMb = 2048;
  changes.setOverlay.warningThresholdMb = 1024;
  changes.setOverlay.criticalThresholdMb = 1536;
  changes.volumeProtect["C:"] = true;
  const uwf::core::OverlayConfig baseline{uwf::core::OverlayType::RAM, 512, 256, 384};
  const auto commands = uwf::app::renderPendingChanges(changes, baseline);
  const auto lines = uwf::app::renderCommandLines(commands);
  QCOMPARE(lines.size(), std::size_t{3});
  QCOMPARE(lines[0], std::string("uwfmgr.exe overlay set-size 2048 && uwfmgr.exe overlay set-type Disk"));
  QCOMPARE(lines[1], std::string("uwfmgr.exe overlay set-criticalthreshold 1536 && uwfmgr.exe overlay set-warningthreshold 1024"));
  QCOMPARE(lines[2], std::string("uwfmgr.exe volume protect C:"));
  const auto parsed = uwf::app::parseUwfmgrText(lines[0] + '\n' + lines[1] + '\n' + lines[2]);
  QCOMPARE(parsed.size(), commands.size());
  for (std::size_t index = 0; index < parsed.size(); ++index) {
    QCOMPARE(parsed[index].parseError, uwf::app::ParseError::None);
    QCOMPARE(parsed[index].kind, commands[index].kind);
    QCOMPARE(parsed[index].args, commands[index].args);
    QCOMPARE(parsed[index].requiresPrevious, commands[index].requiresPrevious);
  }
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
    const auto roundTrip = uwf::app::parseUwfmgrText(uwf::app::renderCommand(command));
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
  auto orphan = commands.back();
  orphan.requiresPrevious = true;
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::app::renderCommandLines({orphan})));
}

void CommandRenderingTests::plannedSessionRenderingPreservesFinalTargets() {
  using enum uwf::app::UwfmgrKind;
  uwf::core::SessionSnapshot next;
  next.filter.enabled = true;
  next.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  next.volumes = {{"Volume{c}", "C:", true, true}, {"Volume{d}", "D:", false, true}};
  next.fileExclusions["Volume{c}"] = {R"(C:\Keep)", R"(C:\Old)"};
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
  changes.removeFileExclusions["C:"] = {R"(c:/old)"};
  changes.addRegistryExclusions = {R"(HKLM\SOFTWARE\New)"};
  changes.removeRegistryExclusions = {R"(hklm\software\old)"};

  const auto commands = uwf::app::renderPlannedSession(next, changes);
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
  QVERIFY(lines.contains(QStringLiteral("uwfmgr.exe file remove-exclusion c:/old")));
  QVERIFY(lines.contains(QStringLiteral("uwfmgr.exe registry remove-exclusion hklm\\software\\old")));
  QVERIFY(next.filter.enabled);
  QCOMPARE(next.overlay.maximumSizeMb, uint32_t{4096});
  QCOMPARE(next.fileExclusions.at("Volume{c}").size(), std::size_t{2});
}

void CommandRenderingTests::uwfmgrParserHandlesDeterministicAdversarialInput() {
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

void CommandRenderingTests::sessionRenderingSkipsVolumesWithoutDriveLetters() {
  uwf::core::SessionSnapshot session;
  session.filter.enabled = false;
  session.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  session.volumes = {{"Volume{A}", "C:", true, true}, {"Volume{Hidden}", "", true, false}, {"Volume{D}", "D:", false, true}};
  session.fileExclusions["Volume{A}"] = {R"(C:\Data)"};
  session.registryExclusions = {R"(HKEY_LOCAL_MACHINE\Software\Demo)"};

  const auto output = rendered(uwf::app::renderSession(session));
  QVERIFY(output.contains(QStringLiteral("uwfmgr.exe volume protect C:")));
  QVERIFY(output.contains(QStringLiteral("uwfmgr.exe volume unprotect D:")));
  for (const auto& line : output) QVERIFY(!line.contains(QStringLiteral("Hidden")));
}

}  // namespace

QTEST_APPLESS_MAIN(CommandRenderingTests)

#include "CommandRenderingTests.moc"
