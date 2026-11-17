// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/filesystem/paths/VolumePaths.h"

#include <windows.h>

#include <cctype>
#include <string_view>

#include "foundation/paths/DriveLetter.h"
#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::drive {

namespace {

bool startsWithCI(const std::string& s, const char* prefix) {
  for (size_t i = 0; prefix[i] != '\0'; ++i) {
    if (i >= s.size()) return false;
    if (std::tolower(static_cast<unsigned char>(s[i])) != std::tolower(static_cast<unsigned char>(prefix[i]))) return false;
  }
  return true;
}

// Restores the extended volume root, including its required trailing separator.
std::string volumeNameFromGuidBody(const std::string& body) {
  const size_t brace = body.find('}');
  if (brace == std::string::npos) return {};
  if (brace + 1 >= body.size() || body[brace + 1] != '\\') return {};
  return R"(\\?\)" + body.substr(0, brace + 2);
}

}  // namespace

std::vector<std::string> mountedPaths(const std::string& volumeGuidPath) {
  const std::wstring volume = windows::utf8ToWide(volumeGuidPath);
  std::wstring paths(MAX_PATH, L'\0');
  // Bound retries when mount points change during the sizing query.
  constexpr int kMaximumMountQueryAttempts = 3;
  for (int attempt = 0; attempt < kMaximumMountQueryAttempts; ++attempt) {
    DWORD length = 0;
    if (!GetVolumePathNamesForVolumeNameW(volume.c_str(), paths.data(), static_cast<DWORD>(paths.size()), &length)) {
      const DWORD error = GetLastError();
      if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) return {};
      if (error != ERROR_MORE_DATA) throw windows::WindowsSystemError(error, "read volume path names");
      if (length <= paths.size()) throw std::runtime_error("volume path query returned an inconsistent required size");
      paths.resize(length);
      continue;
    }
    if (length == 0 || length > paths.size()) throw std::runtime_error("volume path query returned an invalid result size");
    const std::wstring_view result(paths.data(), length);
    std::vector<std::string> mounts;
    for (size_t offset = 0; offset < result.size();) {
      if (result[offset] == L'\0') return mounts;
      const auto end = result.find(L'\0', offset);
      if (end == std::wstring_view::npos) break;
      const auto path = result.substr(offset, end - offset);
      mounts.push_back(windows::wideToUtf8(path));
      offset = end + 1;
    }
    throw std::runtime_error("volume path query returned a malformed MULTI_SZ");
  }
  throw windows::WindowsSystemError(ERROR_MORE_DATA, "volume mount points kept changing during query");
}

std::vector<std::string> mountedLetters(const std::string& volumeGuidPath) {
  std::vector<std::string> letters;
  for (const auto& path : mountedPaths(volumeGuidPath)) {
    if (path.size() != 3 || path[1] != ':' || path[2] != '\\') continue;
    const auto letter = normalize(path.substr(0, 2));
    if (!letter.empty()) letters.push_back(letter);
  }
  return letters;
}

std::string fromPath(const std::string& path) {
  const PathSplit parts = split(path);
  if (!parts.letter.empty()) return parts.letter;

  const std::string& body = parts.rest;
  if (startsWithCI(body, "Volume{")) {
    const std::string volumeName = volumeNameFromGuidBody(body);
    if (volumeName.empty()) throw DriveLetterResolutionError("malformed volume GUID path: " + path);
    const auto letters = mountedLetters(volumeName);
    if (letters.empty()) throw DriveLetterResolutionError("could not resolve volume to a drive letter: " + volumeName);
    return letters.front();
  }

  return {};
}

std::string systemLetter() {
  // Only the ASCII drive prefix is needed from the ANSI Windows directory.
  char buf[MAX_PATH] = {};
  const UINT n = GetWindowsDirectoryA(buf, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return {};
  try {
    return fromPath(std::string(buf, n));
  } catch (const DriveLetterResolutionError&) {
    return {};
  }
}

}  // namespace uwf::drive
