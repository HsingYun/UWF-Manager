// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "app/ports/WindowsFileSystem.h"

#include <windows.h>

#include <algorithm>

#include "windows/system/WindowsSystemError.h"

namespace uwf::winfs {
std::filesystem::path extendedPath(const std::filesystem::path& path) {
  auto native = path.native();
  std::replace(native.begin(), native.end(), L'/', L'\\');
  if (native.starts_with(LR"(\\?\)") || native.starts_with(LR"(\\.\)")) return native;
  if (native.starts_with(LR"(\\)")) return LR"(\\?\UNC\)" + native.substr(2);
  if (native.size() >= 3 && native[1] == L':' && native[2] == L'\\') return LR"(\\?\)" + native;
  return native;
}

EntryType inspect(const std::filesystem::path& path) {
  const DWORD attributes = GetFileAttributesW(extendedPath(path).c_str());
  if (attributes != INVALID_FILE_ATTRIBUTES) {
    if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) return EntryType::ReparsePoint;
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? EntryType::Directory : EntryType::File;
  }

  const DWORD error = GetLastError();
  if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) return EntryType::Missing;
  throw windows::WindowsSystemError(error, "inspect Windows file-system path");
}

[[nodiscard]] bool existsForVerification(const std::filesystem::path& path) {
  const std::wstring nativePath = extendedPath(path).native();
  if (GetFileAttributesW(nativePath.c_str()) != INVALID_FILE_ATTRIBUTES) return true;

  const DWORD error = GetLastError();
  if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) return false;
  throw windows::WindowsSystemError(error, "probe file existence");
}

}  // namespace uwf::winfs
