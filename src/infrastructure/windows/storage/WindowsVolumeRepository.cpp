// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/storage/WindowsVolumeRepository.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <exception>
#include <format>
#include <map>
#include <set>

#include "domain/uwf/policies/UwfConstraints.h"
#include "foundation/logging/Log.h"
#include "foundation/text/StringUtil.h"
#include "infrastructure/uwf/windows/PhysicalVolumeObservation.h"
#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/filesystem/paths/VolumePaths.h"
#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/management/wmi/WmiRowUtil.h"
#include "infrastructure/windows/management/wmi/WmiRuntime.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::windows {
namespace {
using core::VolumeSupport;
constexpr unsigned fixedLocalDriveType = DRIVE_FIXED;
VolumeSupport supportFor(const unsigned driveType, const std::uint64_t bytes, const std::string& fileSystem) {
  if (driveType != fixedLocalDriveType) return VolumeSupport::NotFixedLocalDisk;
  if (bytes > core::kMaxProtectedVolumeBytes) return VolumeSupport::ExceedsMaxSize;
  const auto fs = toUpperAscii(fileSystem);
  return std::ranges::find(core::kFullySupportedFileSystems, fs) == core::kFullySupportedFileSystems.end() ? VolumeSupport::FileSystemLimited
                                                                                                           : VolumeSupport::Supported;
}

std::string failureText(std::exception_ptr failure);
core::PhysicalVolumeFacts readPhysical(const std::string& volume) { return readPhysicalVolumeFacts(volume); }

std::vector<core::VolumeInfo> observePhysicalVolumes(std::vector<core::VolumeInfo> volumes, const PhysicalVolumeReader& read) {
  for (auto& volume : volumes) {
    volume.physical = {};
    if (volume.support == VolumeSupport::NotFixedLocalDisk) {
      volume.physical.fixedMedia = false;
      continue;
    }
    if (volume.volumeName.empty()) continue;
    try {
      volume.physical = read(volume.volumeName);
    } catch (...) {
      // Retain inaccessible volumes as Unknown so existing membership remains
      // manageable without granting permission for new registration.
      UWF_LOG_W("volume") << "physical volume admission unavailable: volume=" << volume.volumeName << " error=" << failureText(std::current_exception());
    }
  }
  return volumes;
}

std::vector<core::VolumeInfo> readWin32Inventory() {
  const auto drives = GetLogicalDrives();
  if (!drives) throw WindowsSystemError(GetLastError(), "enumerate logical drives");
  std::vector<core::VolumeInfo> result;
  constexpr unsigned alphabetSize = 26;
  for (unsigned index = 0; index < alphabetSize; ++index) {
    if ((drives & (DWORD{1} << index)) == 0) continue;
    const std::wstring root{static_cast<wchar_t>(L'A' + index), L':', L'\\'};
    const auto type = GetDriveTypeW(root.c_str());
    if (type != DRIVE_FIXED && type != DRIVE_REMOVABLE) continue;
    core::VolumeInfo volume;
    volume.driveLetter = wideToUtf8(std::wstring_view(root).substr(0, 2));
    std::array<wchar_t, MAX_PATH + 1> label{}, fileSystem{}, name{};
    if (!GetVolumeInformationW(root.c_str(), label.data(), static_cast<DWORD>(label.size()), nullptr, nullptr, nullptr, fileSystem.data(),
                               static_cast<DWORD>(fileSystem.size()))) {
      const auto error = GetLastError();
      // Empty removable readers remain visible, like nullable WMI metadata.
      if (type != DRIVE_REMOVABLE || error != ERROR_NOT_READY) throw WindowsSystemError(error, "read volume information");
      volume.support = VolumeSupport::NotFixedLocalDisk;
      result.push_back(std::move(volume));
      continue;
    }
    volume.label = wideToUtf8(label.data());
    volume.fileSystem = wideToUtf8(fileSystem.data());
    ULARGE_INTEGER total{}, free{};
    if (!GetDiskFreeSpaceExW(root.c_str(), nullptr, &total, &free)) throw WindowsSystemError(GetLastError(), "read volume capacity");
    volume.totalBytes = total.QuadPart;
    volume.freeBytes = free.QuadPart;
    if (GetVolumeNameForVolumeMountPointW(root.c_str(), name.data(), static_cast<DWORD>(name.size())))
      volume.volumeName = wideToUtf8(name.data());
    else {
      // Mappings such as SUBST remain visible without a physical GUID.
      const auto error = GetLastError();
      if (error != ERROR_INVALID_PARAMETER && error != ERROR_INVALID_NAME && error != ERROR_INVALID_FUNCTION && error != ERROR_NOT_SUPPORTED &&
          error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
        throw WindowsSystemError(error, "read volume identity");
    }
    volume.support = supportFor(type, volume.totalBytes, volume.fileSystem);
    result.push_back(std::move(volume));
  }
  return result;
}

std::string failureText(const std::exception_ptr failure) {
  try {
    std::rethrow_exception(failure);
  } catch (const std::exception& error) {
    return error.what();
  } catch (...) {
    return "Unknown volume inventory failure";
  }
}
std::string readNullableString(const wmi::WmiRow& row, const char* field) {
  const auto it = row.find(field);
  if (it == row.end()) throw wmi::WmiDecodeError("decode disk inventory", std::format("field '{}' is missing", field));
  return it->second.isValid() ? wmi::row::requireString(row, field) : std::string{};
}

uint64_t readNullableUInt64(const wmi::WmiRow& row, const char* field) {
  const auto it = row.find(field);
  if (it == row.end()) throw wmi::WmiDecodeError("decode disk inventory", std::format("field '{}' is missing", field));
  return it->second.isValid() ? wmi::row::requireUInt64(row, field) : 0;
}

std::vector<core::VolumeInfo> readInventory(wmi::WmiOperations& cim) {
  std::vector<core::VolumeInfo> out;
  cim.ensureConnected();

  const auto rows =
      cim.query("SELECT DeviceID, FileSystem, VolumeName, Size, FreeSpace, DriveType FROM Win32_LogicalDisk WHERE DriveType = 2 OR DriveType = 3");

  // Join display inventory by drive letter. Win32_Volume.DeviceID is an
  // extended volume path; UWF provider instance keys remain independently scoped.
  const auto volRows = cim.query("SELECT DeviceID, DriveLetter FROM Win32_Volume");
  std::map<std::string, std::string> driveToGuid;
  for (const auto& v : volRows) {
    const auto driveIt = v.find("DriveLetter");
    if (driveIt == v.end()) {
      throw wmi::WmiDecodeError("decode Win32_Volume", "field 'DriveLetter' is missing");
    }
    if (!driveIt->second.isValid()) continue;  // Letterless volumes have no logical-drive mapping.
    const auto driveLetter = wmi::row::requireString(v, "DriveLetter", wmi::row::EmptyString::Reject);
    const auto deviceId = wmi::row::requireString(v, "DeviceID", wmi::row::EmptyString::Reject);
    const auto normalizedDrive = drive::normalize(driveLetter);
    if (normalizedDrive.empty()) {
      throw wmi::WmiDecodeError("decode Win32_Volume", std::format("invalid DriveLetter: {}", driveLetter));
    }
    if (!driveToGuid.emplace(normalizedDrive, deviceId).second) {
      throw wmi::WmiProtocolError("decode Win32_Volume", std::format("duplicate volume mapping for {}", normalizedDrive));
    }
  }

  std::set<std::string> logicalDrives;
  for (const auto& r : rows) {
    const auto deviceId = wmi::row::requireString(r, "DeviceID", wmi::row::EmptyString::Reject);
    const auto driveType = wmi::row::requireInt(r, "DriveType");

    core::VolumeInfo d;
    d.driveLetter = drive::normalize(deviceId);
    if (d.driveLetter.empty()) {
      throw wmi::WmiDecodeError("decode Win32_LogicalDisk", std::format("invalid DeviceID: {}", deviceId));
    }
    if (!logicalDrives.emplace(d.driveLetter).second) {
      throw wmi::WmiProtocolError("decode Win32_LogicalDisk", std::format("duplicate logical disk row for {}", d.driveLetter));
    }
    d.fileSystem = readNullableString(r, "FileSystem");
    d.label = readNullableString(r, "VolumeName");
    d.totalBytes = readNullableUInt64(r, "Size");
    d.freeBytes = readNullableUInt64(r, "FreeSpace");
    if (auto it = driveToGuid.find(d.driveLetter); it != driveToGuid.end()) {
      d.volumeName = it->second;
    }

    d.support = supportFor(static_cast<unsigned>(driveType), d.totalBytes, d.fileSystem);
    out.push_back(std::move(d));
  }
  return out;
}

class Win32VolumeRepository final : public app::VolumeRepository {
 public:
  std::vector<core::VolumeInfo> read() const override { return observePhysicalVolumes(readWin32Inventory(), readPhysical); }
  std::string driveForPath(const std::string& path) const override { return drive::fromPath(path); }
  std::vector<std::string> mountedPaths(const std::string& volume) const override { return drive::mountedPaths(volume); }
  std::vector<std::string> mountedLetters(const std::string& volume) const override { return drive::mountedLetters(volume); }
  std::string systemDrive() const override { return drive::systemLetter(); }
};

class WindowsVolumeRepository final : public app::VolumeRepository {
 public:
  explicit WindowsVolumeRepository(std::function<wmi::WmiOperations&()> inventory, std::shared_ptr<const app::VolumeRepository> fallback = {},
                                   PhysicalVolumeReader physical = {})
      : m_inventory(std::move(inventory)), m_fallback(std::move(fallback)), m_physical(physical ? std::move(physical) : PhysicalVolumeReader{readPhysical}) {}
  std::vector<core::VolumeInfo> read() const override {
    try {
      return observePhysicalVolumes(readInventory(m_inventory()), m_physical);
    } catch (...) {
      if (!m_fallback) throw;
      const auto primary = failureText(std::current_exception());
      UWF_LOG_W("volume") << "WMI inventory failed; reading independent volume inventory: error=" << primary;
      try {
        return m_fallback->read();
      } catch (...) {
        throw std::runtime_error("WMI volume inventory: " + primary + "\nFallback volume inventory: " + failureText(std::current_exception()));
      }
    }
  }
  std::string driveForPath(const std::string& path) const override { return drive::fromPath(path); }
  std::vector<std::string> mountedPaths(const std::string& volume) const override { return drive::mountedPaths(volume); }
  std::vector<std::string> mountedLetters(const std::string& volume) const override { return drive::mountedLetters(volume); }
  std::string systemDrive() const override { return drive::systemLetter(); }

 private:
  std::function<wmi::WmiOperations&()> m_inventory;
  std::shared_ptr<const app::VolumeRepository> m_fallback;
  PhysicalVolumeReader m_physical;
};
}  // namespace
std::shared_ptr<app::VolumeRepository> windowsVolumeRepository(wmi::WmiOperations& inventory, std::shared_ptr<const app::VolumeRepository> fallback,
                                                               PhysicalVolumeReader physical) {
  return std::make_shared<WindowsVolumeRepository>(inventory.threadAccessor(), std::move(fallback), std::move(physical));
}
std::shared_ptr<app::VolumeRepository> win32VolumeRepository() { return std::make_shared<Win32VolumeRepository>(); }
std::shared_ptr<app::VolumeRepository> systemVolumeRepository() {
  static const auto repository = std::make_shared<WindowsVolumeRepository>([]() -> wmi::WmiOperations& { return cimv2WmiSession(); }, win32VolumeRepository());
  return repository;
}
}  // namespace uwf::windows
