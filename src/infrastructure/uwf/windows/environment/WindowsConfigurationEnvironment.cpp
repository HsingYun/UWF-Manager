// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/uwf/windows/environment/WindowsConfigurationEnvironment.h"

#include <windows.h>

#include <array>
#include <memory>
#include <system_error>

#include "infrastructure/uwf/windows/RuntimeAdmissionFacts.h"
#include "infrastructure/windows/compatibility/SystemCheck.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::windows {
namespace {
bool hasHiberFile() {
  std::array<wchar_t, MAX_PATH> path{};
  const DWORD length = ExpandEnvironmentStringsW(L"%SystemDrive%\\hiberfil.sys", path.data(), static_cast<DWORD>(path.size()));
  if (!length || length > path.size()) return false;
  WIN32_FIND_DATAW data{};
  const HANDLE result = FindFirstFileW(path.data(), &data);
  if (result == INVALID_HANDLE_VALUE) return false;
  const std::unique_ptr<void, decltype(&FindClose)> found(result, &FindClose);
  return true;
}
std::string systemVolumeIdentity() {
  std::array<wchar_t, MAX_PATH> root{};
  const DWORD length = ExpandEnvironmentStringsW(L"%SystemDrive%\\", root.data(), static_cast<DWORD>(root.size()));
  if (!length) throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "Resolve the system drive");
  if (length > root.size()) throw std::system_error(ERROR_INSUFFICIENT_BUFFER, std::system_category(), "Resolve the system drive");
  std::array<wchar_t, MAX_PATH> volume{};
  if (!GetVolumeNameForVolumeMountPointW(root.data(), volume.data(), static_cast<DWORD>(volume.size())))
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "Resolve the system volume identity");
  return wideToUtf8(volume.data());
}
}  // namespace
WindowsConfigurationEnvironment windowsConfigurationEnvironment() {
  return {.isAdministrator = &isElevated,
          .hiberFileExists = &hasHiberFile,
          .systemVolumeId = &systemVolumeIdentity,
          .romDisposition = &readRomDisposition,
          .hormVolumeProtection = &readHormVolumeProtection,
          .capturedSettingsAvailable = &readCapturedSettingsAvailable,
          .shutdownPrivilegeAvailable = &readShutdownPrivilegeAvailable,
          .servicingAdmission = &readServicingAdmission,
          .volumeProtectionInventory = &readVolumeProtectionInventory};
}
}  // namespace uwf::windows
