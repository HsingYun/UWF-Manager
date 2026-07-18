/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtTest>
#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "core/RegistryExclusionPolicy.h"
#include "core/UwfModel.h"
#include "ui/Pager.h"
#include "ui/UsageBarGeometry.h"
#include "util/ByteFormat.h"
#include "util/ComPtr.h"
#include "util/DriveLetter.h"
#include "util/Log.h"
#include "util/PathMatch.h"
#include "util/RegistryKey.h"
#include "util/StringUtil.h"
#include "uwf/api/UwfmgrCli.h"
#include "uwf/api/WriteVerification.h"
#include "uwf/wmi/WmiException.h"

namespace {

QString text(const std::string& value) { return QString::fromUtf8(value); }

QStringList rendered(const std::vector<uwf::api::UwfmgrCommand>& commands) {
  QStringList result;
  result.reserve(static_cast<qsizetype>(commands.size()));
  for (const auto& command : commands) result.append(text(uwf::api::renderCommand(command)));
  return result;
}

class CoreBehaviorTests final : public QObject {
  Q_OBJECT

 private slots:
  void stringAndDriveNormalization();
  void asciiAndDrivePropertiesCoverTheInputSpace();
  void byteFormattingUsesStableUnitBoundaries();
  void pathMatchingHonorsSegmentBoundaries();
  void pathMatchingPropertiesPreserveSegmentSemantics();
  void registryExclusionPolicyClassifiesDocumentedBoundaries();
  void exclusionSortingIsCaseInsensitiveAndStable();
  void pendingChangesCountsEveryBusinessField();
  void uwfmgrParserPreservesLineOutcomes();
  void uwfmgrParserRejectsInvalidArgumentsPrecisely();
  void uwfmgrRenderingPreservesBusinessOrderAndQuoting();
  void uwfmgrRendererRoundTripsEverySupportedCommand();
  void uwfmgrParserHandlesDeterministicAdversarialInput();
  void sessionRenderingSkipsVolumesWithoutDriveLetters();
  void writeVerificationConfirmsDefinitiveAndUncertainOutcomes();
  void writeVerificationPreservesRejectionAndRereadFailure();
  void pagerPreservesTheVisibleAnchor();
  void pagerPropertiesRemainBoundedAtIntegerEdges();
  void usageBarGeometryClampsVisualHints();
  void usageBarGeometryPropertiesRemainFiniteAndBounded();
  void logBufferPreservesOrderAndOversizedTail();
  void comPtrModelsAdoptRetainMovePutAndRelease();
  void registryProtocolNamesAndRootsAreStable();
};

struct RefCountedProbe {
  unsigned long refs = 1;
  int releases = 0;
  unsigned long AddRef() { return ++refs; }
  unsigned long Release() {
    ++releases;
    return --refs;
  }
};

void CoreBehaviorTests::stringAndDriveNormalization() {
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

  const std::string utf8 = "UWF \xE8\xA6\x86\xE7\x9B\x96\xE5\xB1\x82";
  QCOMPARE(text(uwf::wideToUtf8(uwf::utf8ToWide(utf8))), text(utf8));
  QVERIFY_THROWS_EXCEPTION(std::system_error, (void)uwf::utf8ToWide(std::string_view("\xC3\x28", 2)));
  QVERIFY(uwf::wideToUtf8(nullptr).empty());
  QVERIFY_THROWS_EXCEPTION(std::system_error, (void)uwf::wideToUtf8(std::wstring(1, static_cast<wchar_t>(0xD800))));
}

void CoreBehaviorTests::asciiAndDrivePropertiesCoverTheInputSpace() {
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
  QVERIFY_THROWS_EXCEPTION(uwf::drive::DriveLetterResolutionError, (void)uwf::drive::fromPath(R"(\\?\Volume{missing-brace\path)"));
}

void CoreBehaviorTests::byteFormattingUsesStableUnitBoundaries() {
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

void CoreBehaviorTests::pathMatchingHonorsSegmentBoundaries() {
  QCOMPARE(text(uwf::stripTrailingSep(R"(C:\Data\\/)")), QStringLiteral("C:\\Data"));
  QVERIFY(uwf::pathIsExcludedBy(R"(c:\data)", R"(C:\DATA)"));
  QVERIFY(uwf::pathIsExcludedBy(R"(C:/Data/Child/file.bin)", R"(c:/data)"));
  QVERIFY(!uwf::pathIsExcludedBy(R"(C:\Database\file.bin)", R"(C:\Data)"));
  QVERIFY(!uwf::pathIsExcludedBy(R"(C:\Data2)", R"(C:\Data)"));
  QVERIFY(!uwf::pathIsExcludedBy(R"(C:\Data)", ""));

  const std::vector<std::string> exclusions = {R"(C:\Program Data\)", R"(D:\Cache)"};
  QCOMPARE(text(uwf::findCoveringExclusion(exclusions, R"(c:/program data/app/state.db)")), QStringLiteral("C:\\Program Data\\"));
  QVERIFY(uwf::findCoveringExclusion(exclusions, R"(C:\Program)").empty());
}

void CoreBehaviorTests::pathMatchingPropertiesPreserveSegmentSemantics() {
  const std::array roots = {std::string(R"(C:\Root\Node)"), std::string(R"(c:/root/node)"), std::string(R"(C:/ROOT\NODE)")};
  for (const auto& root : roots) {
    const std::string canonical = uwf::stripTrailingSep(root + R"(\//)");
    QVERIFY(uwf::pathIsExcludedBy(canonical, uwf::stripTrailingSep(root)));
    QVERIFY(uwf::pathIsExcludedBy(canonical + R"(/child\leaf.bin)", uwf::stripTrailingSep(root)));
    QVERIFY(!uwf::pathIsExcludedBy(canonical + "-sibling", uwf::stripTrailingSep(root)));
    QVERIFY(!uwf::pathIsExcludedBy(canonical.substr(0, canonical.size() - 1), uwf::stripTrailingSep(root)));
  }

  const std::vector<std::string> exclusions{R"(C:\Root\)", R"(C:\Root\Longer\)", R"(D:/Data/)"};
  QCOMPARE(uwf::findCoveringExclusion(exclusions, R"(c:/root/longer/file)"), exclusions.front());
  QVERIFY(uwf::findCoveringExclusion(exclusions, R"(C:\Rooted\file)").empty());
  QVERIFY(uwf::stripTrailingSep("////").empty());
}

void CoreBehaviorTests::registryExclusionPolicyClassifiesDocumentedBoundaries() {
  using enum uwf::core::RegExclusionClass;
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(HKLM\Software\Vendor\Product)"), Allowed);
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(hkey_local_machine\system\CurrentControlSet\Services\Demo)"), Allowed);
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(HKLM\SOFTWARE)"), Container);
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(HKEY_LOCAL_MACHINE)"), Container);
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC)"), MachineAccount);
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(HKCU\Software\Vendor)"), OutOfScope);
  QCOMPARE(uwf::core::classifyRegistryExclusion(""), OutOfScope);

  QCOMPARE(text(uwf::regkey::normalize(R"( hklm\Software\Vendor\\ )")), QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Vendor"));
}

void CoreBehaviorTests::exclusionSortingIsCaseInsensitiveAndStable() {
  std::vector<std::string> items = {R"(C:\zeta)", R"(c:\Alpha)", R"(C:\ALPHA)", R"(C:\beta)"};
  uwf::core::sortExclusions(items);
  QCOMPARE(items.size(), std::size_t{3});
  QCOMPARE(text(items[0]), QStringLiteral("c:\\Alpha"));
  QCOMPARE(text(items[1]), QStringLiteral("C:\\beta"));
  QCOMPARE(text(items[2]), QStringLiteral("C:\\zeta"));

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

void CoreBehaviorTests::pendingChangesCountsEveryBusinessField() {
  uwf::core::PendingChanges changes;
  QVERIFY(changes.empty());
  QCOMPARE(changes.count(), std::size_t{0});

  changes.setFilterEnabled = false;
  changes.setOverlay.type = uwf::core::OverlayType::Disk;
  changes.setOverlay.maximumSizeMb = 4096;
  changes.setOverlay.warningThresholdMb = 2048;
  changes.setOverlay.criticalThresholdMb = 3072;
  changes.volumeProtect["C:"] = true;
  changes.volumeBindByVolumeName["D:"] = false;
  changes.addFileExclusions["C:"] = {R"(C:\A)", R"(C:\B)"};
  changes.removeFileExclusions["D:"] = {R"(D:\Old)"};
  changes.addRegistryExclusions = {R"(HKLM\Software\A)"};
  changes.removeRegistryExclusions = {R"(HKLM\System\B)"};
  changes.setPersistDomainSecretKey = true;
  changes.setPersistTSCAL = false;

  QVERIFY(!changes.empty());
  QCOMPARE(changes.count(), std::size_t{14});
  QVERIFY(changes.setOverlay.touchesOverlayConfig());

  changes.clear();
  QVERIFY(changes.empty());
  QCOMPARE(changes.count(), std::size_t{0});

  changes.addFileExclusions["C:"] = {};
  changes.removeFileExclusions["D:"] = {};
  QVERIFY(changes.empty());
  QCOMPARE(changes.count(), std::size_t{0});
}

void CoreBehaviorTests::uwfmgrParserPreservesLineOutcomes() {
  const auto commands = uwf::api::parseUwfmgrText(
      "# exported rules\r\n"
      "UWFMgr.EXE FILTER enable\r\n"
      "overlay set-type disk\r\n"
      "volume protect c\r\n"
      "file add-exclusion \"C:\\Program Files\\Demo\"\r\n"
      "registry remove-exclusion \"HKLM\\Software\\Demo\"\r\n"
      ":: ignored\r\n");

  QCOMPARE(commands.size(), std::size_t{7});
  QCOMPARE(commands[0].parseError, uwf::api::ParseError::Comment);
  QCOMPARE(commands[1].kind, uwf::api::UwfmgrKind::FilterEnable);
  QCOMPARE(commands[1].sourceLineNo, 2);
  QCOMPARE(commands[2].args.at(0), std::string("Disk"));
  QCOMPARE(commands[3].args.at(0), std::string("C:"));
  QCOMPARE(commands[4].args.at(0), std::string(R"(C:\Program Files\Demo)"));
  QCOMPARE(commands[5].args.at(0), std::string(R"(HKLM\Software\Demo)"));
  QCOMPARE(commands[6].parseError, uwf::api::ParseError::Comment);
}

void CoreBehaviorTests::uwfmgrParserRejectsInvalidArgumentsPrecisely() {
  const auto commands = uwf::api::parseUwfmgrText(
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
      uwf::api::ParseError::Incomplete,       uwf::api::ParseError::MissingSizeArg,        uwf::api::ParseError::InvalidSize,
      uwf::api::ParseError::InvalidSize,      uwf::api::ParseError::UnknownType,           uwf::api::ParseError::Unsupported,
      uwf::api::ParseError::InvalidVolume,    uwf::api::ParseError::MissingPathArg,        uwf::api::ParseError::MissingRegistryKeyArg,
      uwf::api::ParseError::MalformedQuoting, uwf::api::ParseError::UnexpectedArgument,    uwf::api::ParseError::UnexpectedArgument,
      uwf::api::ParseError::MissingPathArg,   uwf::api::ParseError::MissingRegistryKeyArg, uwf::api::ParseError::Unsupported,
  };
  QCOMPARE(commands.size(), expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) QCOMPARE(commands[i].parseError, expected[i]);
  QCOMPARE(text(commands[4].parseErrorContext), QStringLiteral("flash"));
  QCOMPARE(text(commands[6].parseErrorContext), QStringLiteral("Volume{A}"));
  QCOMPARE(text(commands[10].parseErrorContext), QStringLiteral("unexpectedly"));
  QVERIFY(commands[11].parseErrorContext.empty());
}

void CoreBehaviorTests::uwfmgrRenderingPreservesBusinessOrderAndQuoting() {
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

  QCOMPARE(rendered(uwf::api::renderPendingChanges(changes)),
           QStringList({"uwfmgr.exe filter enable", "uwfmgr.exe overlay set-type Disk", "uwfmgr.exe overlay set-size 8192",
                        "uwfmgr.exe overlay set-warningthreshold 4096", "uwfmgr.exe overlay set-criticalthreshold 6144",
                        "uwfmgr.exe volume protect C:", "uwfmgr.exe file add-exclusion \"C:\\Program Files\\Demo\"", "uwfmgr.exe file remove-exclusion C:\\Old",
                        "uwfmgr.exe registry add-exclusion \"HKEY_LOCAL_MACHINE\\Software\\Demo App\"",
                        "uwfmgr.exe registry remove-exclusion HKEY_LOCAL_MACHINE\\System\\Demo"}));
}

void CoreBehaviorTests::uwfmgrRendererRoundTripsEverySupportedCommand() {
  using enum uwf::api::UwfmgrKind;
  const auto command = [](const uwf::api::UwfmgrKind kind, std::vector<std::string> args = {}) {
    uwf::api::UwfmgrCommand value;
    value.kind = kind;
    value.args = std::move(args);
    return value;
  };
  const std::vector<uwf::api::UwfmgrCommand> commands = {
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
    const std::string line = uwf::api::renderCommand(expected);
    const auto parsed = uwf::api::parseUwfmgrText(line);
    QCOMPARE(parsed.size(), std::size_t{1});
    QCOMPARE(parsed.front().parseError, uwf::api::ParseError::None);
    QCOMPARE(parsed.front().kind, expected.kind);
    QCOMPARE(parsed.front().args, expected.args);
    QCOMPARE(parsed.front().sourceLineNo, 1);
  }
}

void CoreBehaviorTests::uwfmgrParserHandlesDeterministicAdversarialInput() {
  struct ExpectedLine {
    uwf::api::UwfmgrKind kind = uwf::api::UwfmgrKind::Unknown;
    uwf::api::ParseError error = uwf::api::ParseError::None;
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
        expected.push_back({uwf::api::UwfmgrKind::FilterEnable, uwf::api::ParseError::None, {}});
        break;
      case 1:
        input += "overlay set-size " + std::to_string(mixed);
        expected.push_back({uwf::api::UwfmgrKind::OverlaySetSize, uwf::api::ParseError::None, {std::to_string(mixed)}});
        break;
      case 2: {
        const char letter = static_cast<char>('a' + index % 26);
        input += "volume protect " + std::string(1, letter);
        expected.push_back({uwf::api::UwfmgrKind::VolumeProtect, uwf::api::ParseError::None, {std::string(1, static_cast<char>(letter - 'a' + 'A')) + ':'}});
      } break;
      case 3:
        input += R"(file add-exclusion "C:\Path With Spaces\item")";
        expected.push_back({uwf::api::UwfmgrKind::FileAddExclusion, uwf::api::ParseError::None, {R"(C:\Path With Spaces\item)"}});
        break;
      case 4:
        input += "registry remove-exclusion HKLM\\Software\\" + std::to_string(index);
        expected.push_back({uwf::api::UwfmgrKind::RegistryRemoveExclusion, uwf::api::ParseError::None, {"HKLM\\Software\\" + std::to_string(index)}});
        break;
      case 5:
        input += "# comment " + std::to_string(index);
        expected.push_back({uwf::api::UwfmgrKind::Unknown, hasExecutablePrefix ? uwf::api::ParseError::Unsupported : uwf::api::ParseError::Comment, {}});
        break;
      case 6:
        input += "unknown-" + std::to_string(mixed);
        expected.push_back({uwf::api::UwfmgrKind::Unknown, uwf::api::ParseError::Incomplete, {}});
        break;
      default:
        input.append(static_cast<std::size_t>(index % 17), ' ');
        expected.push_back({uwf::api::UwfmgrKind::Unknown, hasExecutablePrefix ? uwf::api::ParseError::Incomplete : uwf::api::ParseError::Comment, {}});
        break;
    }
    input += index % 2 == 0 ? "\r\n" : "\n";
  }

  const auto first = uwf::api::parseUwfmgrText(input);
  const auto second = uwf::api::parseUwfmgrText(input);
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

void CoreBehaviorTests::sessionRenderingSkipsVolumesWithoutDriveLetters() {
  uwf::core::SessionSnapshot session;
  session.filter.enabled = false;
  session.overlay = {uwf::core::OverlayType::RAM, 4096, 2048, 3072};
  session.volumes = {{"Volume{A}", "C:", true, true}, {"Volume{Hidden}", "", true, false}, {"Volume{D}", "D:", false, true}};
  session.fileExclusions["Volume{A}"] = {R"(C:\Data)"};
  session.registryExclusions = {R"(HKEY_LOCAL_MACHINE\Software\Demo)"};

  const auto output = rendered(uwf::api::renderSession(session));
  QVERIFY(output.contains(QStringLiteral("uwfmgr.exe volume protect C:")));
  QVERIFY(output.contains(QStringLiteral("uwfmgr.exe volume unprotect D:")));
  for (const auto& line : output) QVERIFY(!line.contains(QStringLiteral("Hidden")));
}

void CoreBehaviorTests::writeVerificationConfirmsDefinitiveAndUncertainOutcomes() {
  int invokes = 0;
  int observes = 0;
  uwf::api::invokeAndConfirm(
      "set filter", [&] { ++invokes; },
      [&] {
        ++observes;
        return true;
      });
  QCOMPARE(invokes, 1);
  QCOMPARE(observes, 1);

  uwf::api::invokeAndConfirm(
      "set filter",
      [&] {
        ++invokes;
        throw uwf::WmiInvocationUncertain("set filter", "connection lost after dispatch");
      },
      [&] {
        ++observes;
        return true;
      });
  QCOMPARE(invokes, 2);
  QCOMPARE(observes, 2);

  QVERIFY_THROWS_EXCEPTION(uwf::WmiStateVerificationError, uwf::api::invokeAndConfirm("set filter", [] {}, [] { return false; }));
  QVERIFY_THROWS_EXCEPTION(uwf::WmiStateVerificationError,
                           uwf::api::invokeAndConfirm("set filter", [] { throw uwf::WmiInvocationUncertain("set filter", "lost"); }, [] { return false; }));
}

void CoreBehaviorTests::writeVerificationPreservesRejectionAndRereadFailure() {
  bool observed = false;
  try {
    uwf::api::invokeAndConfirm(
        "protect volume", [] { throw uwf::WmiProviderError(87, "protect volume", "invalid parameter"); },
        [&] {
          observed = true;
          return true;
        });
    QFAIL("provider rejection must propagate");
  } catch (const uwf::WmiProviderError& error) {
    QCOMPARE(error.returnValue(), uint32_t{87});
  }
  QVERIFY(!observed);

  bool nestedPreserved = false;
  try {
    uwf::api::invokeAndConfirm("protect volume", [] {}, []() -> bool { throw std::runtime_error("reread failed"); });
    QFAIL("reread failure must be wrapped");
  } catch (const uwf::WmiStateVerificationError& error) {
    QCOMPARE(text(error.operation()), QStringLiteral("protect volume"));
    try {
      std::rethrow_if_nested(error);
    } catch (const std::runtime_error& nested) {
      nestedPreserved = std::string(nested.what()) == "reread failed";
    }
  }
  QVERIFY(nestedPreserved);
}

void CoreBehaviorTests::pagerPreservesTheVisibleAnchor() {
  uwf::ui::Pager pager{10, 3};
  QCOMPARE(pager.pageCount(31), 4);
  QCOMPARE(pager.pageStart(), 30);
  QCOMPARE(pager.pageEnd(31), 31);
  QVERIFY(pager.hasPrev());
  QVERIFY(!pager.hasNext(31));

  QVERIFY(pager.setPageSize(7));
  QCOMPARE(pager.currentPage, 4);
  QCOMPARE(pager.pageStart(), 28);
  pager.clamp(20);
  QCOMPARE(pager.currentPage, 2);
  pager.goFirst();
  QVERIFY(!pager.hasPrev());
  pager.goLast(20);
  QCOMPARE(pager.currentPage, 2);
  pager.goNext(20);
  QCOMPARE(pager.currentPage, 2);
}

void CoreBehaviorTests::pagerPropertiesRemainBoundedAtIntegerEdges() {
  constexpr std::array totals = {-1, 0, 1, 2, 7, 31, 1024, std::numeric_limits<int>::max()};
  constexpr std::array sizes = {-7, 0, 1, 2, 7, 1024, std::numeric_limits<int>::max()};
  constexpr std::array pages = {-9, 0, 1, 17, std::numeric_limits<int>::max()};

  for (const int total : totals) {
    for (const int size : sizes) {
      for (const int page : pages) {
        uwf::ui::Pager pager{size, page};
        const int count = pager.pageCount(total);
        QVERIFY(count >= 0);
        pager.clamp(total);
        QVERIFY(pager.currentPage >= 0);
        QVERIFY(count == 0 ? pager.currentPage == 0 : pager.currentPage < count);
        const int start = pager.pageStart();
        const int end = pager.pageEnd(total);
        QVERIFY(start >= 0);
        QVERIFY(end >= 0);
        if (total > 0) {
          QVERIFY(start <= total);
          QVERIFY(end >= start);
          QVERIFY(end <= total);
        } else {
          QCOMPARE(end, 0);
        }
        QVERIFY(!pager.hasPrev() || pager.currentPage > 0);
        QVERIFY(!pager.hasNext(total) || pager.currentPage + 1 < count);
      }
    }
  }
}

void CoreBehaviorTests::usageBarGeometryClampsVisualHints() {
  QCOMPARE(uwf::ui::visibleUsedWidth(0, 100), qreal{0});
  QCOMPARE(uwf::ui::visibleUsedWidth(0.1, 100), qreal{2});
  QCOMPARE(uwf::ui::visibleUsedWidth(0.1, 100, 2), qreal{1});
  QCOMPARE(uwf::ui::visibleUsedWidth(120, 100), qreal{100});

  const QRectF bounds(10, 20, 100, 40);
  const QPainterPath path = uwf::ui::waveProgressPath(bounds, 25, 0);
  QVERIFY(path.contains(QPointF(10.5, 30)));
  QVERIFY(!path.contains(QPointF(50, 30)));
  QVERIFY(path.boundingRect().left() >= bounds.left());
  QVERIFY(path.boundingRect().right() <= bounds.left() + 25 + 1.5);
}

void CoreBehaviorTests::usageBarGeometryPropertiesRemainFiniteAndBounded() {
  constexpr std::array widths = {-10.0, 0.0, 0.01, 1.0, 50.0, 100.0, 1000.0};
  for (const qreal full : widths) {
    for (const qreal natural : widths) {
      const qreal visible = uwf::ui::visibleUsedWidth(natural, full, 20, 40, 80);
      QVERIFY(std::isfinite(visible));
      QVERIFY(visible >= 0);
      if (full > 0) QVERIFY(visible <= full);
      if (natural > 0 && full > 0) QVERIFY(visible >= std::min(natural, full));
    }
  }

  const QRectF bounds(3, 5, 127, 41);
  for (const qreal used : widths) {
    for (const qreal phase : {-100.0, 0.0, 1.0, 100.0}) {
      const QRectF pathBounds = uwf::ui::waveProgressPath(bounds, used, phase).boundingRect();
      QVERIFY(pathBounds.left() >= bounds.left());
      QVERIFY(pathBounds.top() >= bounds.top());
      QVERIFY(pathBounds.bottom() <= bounds.bottom());
      QVERIFY(pathBounds.right() <= bounds.right() + 1.5);
    }
  }
}

void CoreBehaviorTests::logBufferPreservesOrderAndOversizedTail() {
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
  uwf::clearLogLines();
  QVERIFY(uwf::recentLogLines().empty());
}

void CoreBehaviorTests::comPtrModelsAdoptRetainMovePutAndRelease() {
  RefCountedProbe first;
  {
    auto owned = uwf::ComPtr<RefCountedProbe>::adopt(&first);
    QVERIFY(owned);
    QCOMPARE(owned->refs, 1UL);
    auto moved = std::move(owned);
    QCOMPARE(moved.get(), &first);
    QCOMPARE(moved.release(), &first);
    QVERIFY(!moved);
  }
  // release 后作用域退出仍没有 Release，证明移动源不再持有该对象。
  QCOMPARE(first.releases, 0);

  RefCountedProbe retained;
  RefCountedProbe replacement;
  RefCountedProbe erasedReplacement;
  {
    auto pointer = uwf::ComPtr<RefCountedProbe>::retain(&retained);
    QCOMPARE(retained.refs, 2UL);
    *pointer.put() = &replacement;
    QCOMPARE(retained.releases, 1);
    QCOMPARE(replacement.releases, 0);
    *pointer.putVoid() = &erasedReplacement;
    QCOMPARE(replacement.releases, 1);
    pointer.reset();
    QCOMPARE(erasedReplacement.releases, 1);
  }
  QCOMPARE(retained.refs, 1UL);
  QCOMPARE(replacement.releases, 1);
}

void CoreBehaviorTests::registryProtocolNamesAndRootsAreStable() {
  const auto roots = uwf::regkey::rootHiveLongNames();
  QCOMPARE(roots.size(), std::size_t{5});
  QCOMPARE(roots.front(), std::string("HKEY_LOCAL_MACHINE"));
  QCOMPARE(uwf::regkey::valueTypeName(1), std::string("REG_SZ"));
  QCOMPARE(uwf::regkey::valueTypeName(4), std::string("REG_DWORD"));
  QCOMPARE(uwf::regkey::valueTypeName(0xFFFFFFFFu), std::string("UNKNOWN(4294967295)"));
}

}  // namespace

QTEST_APPLESS_MAIN(CoreBehaviorTests)

#include "CoreBehaviorTests.moc"
