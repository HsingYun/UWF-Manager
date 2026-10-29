// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ExecutableIdentity.h"

#include <algorithm>
#include <iterator>

#include "windows/resources/UniqueHandle.h"
#include "windows/system/WindowsSystemError.h"

namespace uwf::windows {
namespace {
// Includes the NUL terminator within the Win32 extended-length path capacity.
constexpr std::size_t kExtendedPathCapacity = 32768;
}  // namespace
std::wstring processImagePath(const HANDLE process) {
  std::wstring path(kExtendedPathCapacity, L'\0');
  DWORD length = static_cast<DWORD>(path.size());
  if (!QueryFullProcessImageNameW(process, 0, path.data(), &length)) return {};
  path.resize(length);
  return path;
}
std::wstring currentExecutablePath() {
  std::wstring path(kExtendedPathCapacity, L'\0');
  const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
  if (length == 0 || static_cast<std::size_t>(length) >= path.size()) throw WindowsSystemError(GetLastError(), "read current executable path");
  path.resize(length);
  return path;
}

bool sameFile(const std::wstring& leftPath, const std::wstring& rightPath, const FileIdentityFormat format) {
  const DWORD sharing = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  const UniqueHandle left(CreateFileW(leftPath.c_str(), FILE_READ_ATTRIBUTES, sharing, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
  const UniqueHandle right(CreateFileW(rightPath.c_str(), FILE_READ_ATTRIBUTES, sharing, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (!left.valid() || !right.valid()) return false;
  if (format == FileIdentityFormat::FileIndex64) {
    BY_HANDLE_FILE_INFORMATION leftInfo{};
    BY_HANDLE_FILE_INFORMATION rightInfo{};
    if (!GetFileInformationByHandle(left.get(), &leftInfo) || !GetFileInformationByHandle(right.get(), &rightInfo)) return false;
    return leftInfo.dwVolumeSerialNumber == rightInfo.dwVolumeSerialNumber && leftInfo.nFileIndexHigh == rightInfo.nFileIndexHigh &&
           leftInfo.nFileIndexLow == rightInfo.nFileIndexLow;
  }
  FILE_ID_INFO leftInfo{};
  FILE_ID_INFO rightInfo{};
  return GetFileInformationByHandleEx(left.get(), FileIdInfo, &leftInfo, sizeof(leftInfo)) &&
         GetFileInformationByHandleEx(right.get(), FileIdInfo, &rightInfo, sizeof(rightInfo)) && leftInfo.VolumeSerialNumber == rightInfo.VolumeSerialNumber &&
         std::equal(std::begin(leftInfo.FileId.Identifier), std::end(leftInfo.FileId.Identifier), std::begin(rightInfo.FileId.Identifier));
}
}  // namespace uwf::windows
