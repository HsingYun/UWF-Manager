// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/uwf/windows/PhysicalVolumeObservation.h"

#include <Windows.h>
#include <winioctl.h>

#include <cstddef>
#include <cstring>
#include <vector>

#include "domain/uwf/contracts/Backend.h"
#include "infrastructure/uwf/errors/WindowsUwfError.h"
#include "infrastructure/windows/errors/WindowsErrorText.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::windows {
namespace {
void canceled(const std::stop_token stop) {
  if (stop.stop_requested()) throw compat::Error({compat::ErrorKind::Canceled, compat::Effect::None, "The physical volume observation was canceled", {}});
}
[[noreturn]] void failWindows(const DWORD error, const char* operation) {
  const auto detail = windowsErrorText(error).toUtf8().toStdString();
  throw compat::Error({classifyWindowsUwfError(error, WindowsUwfErrorContext::Observation).value_or(compat::ErrorKind::Failed),
                       compat::Effect::None,
                       detail,
                       {{"WindowsPhysicalVolume", operation, static_cast<std::int64_t>(HRESULT_FROM_WIN32(error)), detail}}});
}
[[noreturn]] void protocol(const char* operation, const char* detail) {
  throw compat::Error({compat::ErrorKind::Protocol, compat::Effect::None, detail, {{"WindowsPhysicalVolume", operation, 0, detail}}});
}
DWORD readDevice(const HANDLE device, const DWORD code, void* input, const DWORD inputBytes, void* output, const DWORD outputBytes, const DWORD minimumBytes,
                 const char* operation, const std::stop_token stop) {
  canceled(stop);
  DWORD returned = 0;
  if (!DeviceIoControl(device, code, input, inputBytes, output, outputBytes, &returned, nullptr)) failWindows(GetLastError(), operation);
  canceled(stop);
  if (returned < minimumBytes || returned > outputBytes) protocol(operation, "The physical volume returned an incomplete or invalid observation");
  return returned;
}
bool ineligibleMediaError(const compat::Error& error) {
  if (error.failure().diagnostics.empty()) return false;
  const auto code = error.failure().diagnostics.front().code;
  return code == HRESULT_FROM_WIN32(ERROR_NOT_READY) || code == HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
}
}  // namespace

core::PhysicalVolumeFacts readPhysicalVolumeFacts(const std::string& volume, const bool includeBus, const std::stop_token stop) {
  canceled(stop);
  const auto identity = compat::parseVolumeId(volume);
  if (!identity) throw compat::Error({compat::ErrorKind::InvalidArgument, compat::Effect::None, "A physical volume observation requires an exact GUID", {}});
  auto path = utf8ToWide(compat::volumePath(*identity));
  path.pop_back();
  UniqueHandle device;
  try {
    device.reset(
        CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!device.valid()) failWindows(GetLastError(), "open physical volume");
    DISK_GEOMETRY_EX geometry{};
    readDevice(device.get(), IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, nullptr, 0, &geometry, sizeof(geometry), offsetof(DISK_GEOMETRY_EX, Data),
               "read volume media type", stop);
    if (geometry.Geometry.MediaType != FixedMedia) return {false, {}};
  } catch (const compat::Error& error) {
    if (ineligibleMediaError(error)) return {false, {}};
    throw;
  }
  if (!includeBus) return {true, {}};
  try {
    STORAGE_PROPERTY_QUERY query{};
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;
    STORAGE_DESCRIPTOR_HEADER header{};
    readDevice(device.get(), IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &header, sizeof(header), sizeof(header),
               "read volume storage descriptor header", stop);
    constexpr DWORD minimumDescriptor = offsetof(STORAGE_DEVICE_DESCRIPTOR, BusType) + sizeof(STORAGE_BUS_TYPE);
    constexpr DWORD maximumDescriptor = 1024 * 1024;
    if (header.Version < minimumDescriptor || header.Size < minimumDescriptor || header.Size > maximumDescriptor)
      protocol("read volume storage descriptor", "The storage descriptor size is invalid");
    std::vector<std::byte> descriptor(header.Size);
    const auto returned = readDevice(device.get(), IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), descriptor.data(), header.Size, minimumDescriptor,
                                     "read volume storage descriptor", stop);
    STORAGE_DEVICE_DESCRIPTOR fields{};
    std::memcpy(&fields, descriptor.data(), minimumDescriptor);
    if (fields.Version < minimumDescriptor || fields.Size < minimumDescriptor || fields.Size > returned)
      protocol("read volume storage descriptor", "The storage descriptor is truncated");
    // The captured provider excludes only USB and Storage Spaces, including
    // when a successfully observed bus code is unknown or newer.
    return {true, fields.BusType != BusTypeUsb && fields.BusType != BusTypeSpaces};
  } catch (const compat::Error& error) {
    if (ineligibleMediaError(error)) return {true, false};
    throw;
  }
}
}  // namespace uwf::windows
