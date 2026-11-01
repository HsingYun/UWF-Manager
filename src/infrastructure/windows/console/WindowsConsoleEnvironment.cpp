// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/console/WindowsConsoleEnvironment.h"

#include <windows.h>

#include <array>
#include <memory>
#include <string_view>

#include "foundation/text/StringUtil.h"
#include "infrastructure/uwf/errors/WindowsUwfError.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::windows {
namespace {
constexpr std::size_t maximumPathUnits = 32768;
bool isAsciiAlpha(const char value) { return (value >= 'A' && value <= 'Z') || (value >= 'a' && value <= 'z'); }
std::optional<std::string> volumeIdentity(const std::string& operand) {
  if (const auto identity = compat::parseVolumeId(operand)) return identity;
  if (toLowerAscii(operand).starts_with(R"(\\?\volume{)") && operand.ends_with('}')) return compat::parseVolumeId(operand + '\\');
  return std::nullopt;
}
[[noreturn]] void invalid(const char* message) { throw compat::Error({compat::ErrorKind::InvalidArgument, compat::Effect::None, message, {}}); }
[[noreturn]] void failWindows(const DWORD code, const char* operation) {
  throw compat::Error({classifyWindowsUwfError(code, WindowsUwfErrorContext::Observation).value_or(compat::ErrorKind::Failed),
                       compat::Effect::None,
                       std::string(operation) + " failed",
                       {{"Windows", operation, static_cast<std::int64_t>(HRESULT_FROM_WIN32(code)), "Win32 error " + std::to_string(code)}}});
}
[[noreturn]] void protocol(const char* message) { throw compat::Error({compat::ErrorKind::Protocol, compat::Effect::None, message, {}}); }
std::wstring checkedPath(const std::string& operand) {
  if (operand.empty() || operand.find('\0') != std::string::npos) invalid("A nonempty path without NUL characters is required");
  try {
    return utf8ToWide(operand);
  } catch (const std::exception&) {
    invalid("The path is not valid UTF-8");
  }
}
std::string driveRoot(std::string operand) {
  if (operand.starts_with(R"(\\?\)") || operand.starts_with(R"(\\.\)")) operand.erase(0, 4);
  if (operand.size() == 1 && isAsciiAlpha(operand[0])) operand += ':';
  if (operand.size() == 3 && operand.back() == '\\') operand.pop_back();
  if (operand.size() != 2 || !isAsciiAlpha(operand[0]) || operand[1] != ':') return {};
  return toUpperAscii(operand);
}
std::string mountedVolume(const std::wstring& root) {
  std::array<wchar_t, MAX_PATH + 1> name{};
  if (!GetVolumeNameForVolumeMountPointW(root.c_str(), name.data(), static_cast<DWORD>(name.size()))) failWindows(GetLastError(), "Resolve volume identity");
  const auto identity = compat::parseVolumeId(wideToUtf8(name.data()));
  if (!identity) protocol("Windows returned an invalid volume identity");
  return *identity;
}
std::string ntDevice(std::string operand) {
  if (toLowerAscii(operand).starts_with(R"(\\?\globalroot\)"))
    operand.erase(0, 14);
  else if (toLowerAscii(operand).starts_with(R"(\\.\harddiskvolume)"))
    operand = R"(\Device\)" + operand.substr(4);
  return operand;
}
std::string deviceVolume(const std::string& operand) {
  auto requested = checkedPath(operand);
  if (!requested.empty() && requested.back() == L'\\') requested.pop_back();
  std::array<wchar_t, MAX_PATH + 1> volume{};
  const auto handle = FindFirstVolumeW(volume.data(), static_cast<DWORD>(volume.size()));
  if (handle == INVALID_HANDLE_VALUE) failWindows(GetLastError(), "Enumerate volume identities");
  const std::unique_ptr<void, decltype(&FindVolumeClose)> enumeration(handle, &FindVolumeClose);
  do {
    const auto identity = compat::parseVolumeId(wideToUtf8(volume.data()));
    if (!identity) protocol("Windows volume enumeration returned an invalid identity");
    auto name = utf8ToWide(compat::volumePath(*identity));
    name = name.substr(4, name.size() - 5);
    std::array<wchar_t, maximumPathUnits> devices{};
    const auto length = QueryDosDeviceW(name.c_str(), devices.data(), static_cast<DWORD>(devices.size()));
    if (!length) failWindows(GetLastError(), "Resolve volume device path");
    // QueryDosDevice can include previous mappings; only the first is current.
    if (CompareStringOrdinal(requested.c_str(), -1, devices.data(), -1, TRUE) == CSTR_EQUAL) return *identity;
  } while (FindNextVolumeW(handle, volume.data(), static_cast<DWORD>(volume.size())));
  const auto error = GetLastError();
  if (error != ERROR_NO_MORE_FILES) failWindows(error, "Enumerate volume identities");
  throw compat::Error({compat::ErrorKind::NotFound, compat::Effect::None, "The device path does not identify a mounted local volume", {}});
}
}  // namespace

app::ConsoleVolumeTarget WindowsConsoleEnvironment::resolveVolume(const std::string& operand) const {
  static_cast<void>(checkedPath(operand));
  if (const auto identity = volumeIdentity(operand)) return {*identity, {}, false};
  if (const auto drive = driveRoot(operand); !drive.empty()) return {mountedVolume(utf8ToWide(drive + '\\')), drive, true};
  const auto device = ntDevice(operand);
  if (toLowerAscii(device).starts_with(R"(\device\)")) return {deviceVolume(device), {}, false};
  invalid("Expected a drive letter, volume GUID path, or local volume device path");
}
app::ConsoleFileTarget WindowsConsoleEnvironment::resolveFile(const std::string& operand) const {
  static_cast<void>(checkedPath(operand));
  // An absent body stays empty. Adding a separator could turn an invalid
  // exclusion target into a different, whole-volume exclusion pattern.
  if (!driveRoot(operand).empty() || volumeIdentity(operand)) return {resolveVolume(operand).id, operand.ends_with('\\') ? "\\" : ""};
  const auto lower = toLowerAscii(operand);
  if (lower.starts_with(R"(\\?\volume{)")) {
    const auto end = operand.find('}');
    if (end == std::string::npos || end + 1 == operand.size() || operand[end + 1] != '\\') invalid("Invalid volume GUID file path");
    const auto volume = resolveVolume(operand.substr(0, end + 2));
    return {volume.id, operand.substr(end + 1)};
  }
  const auto device = ntDevice(operand);
  if (toLowerAscii(device).starts_with(R"(\device\)")) {
    const auto end = device.find('\\', 8);
    if (end == std::string::npos) return {deviceVolume(device), ""};
    return {deviceVolume(device.substr(0, end)), device.substr(end)};
  }
  // Preserve the explicit management volume and literal target body across
  // descendant mount points; do not expand against the working directory.
  auto body = operand;
  if (body.starts_with(R"(\\?\)") || body.starts_with(R"(\\.\)")) body.erase(0, 4);
  if (body.size() < 3 || !isAsciiAlpha(body[0]) || body[1] != ':' || body[2] != '\\') invalid("Expected an absolute local file path");
  return {resolveVolume(body.substr(0, 2)).id, body.substr(2)};
}
}  // namespace uwf::windows
