// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/storage/AllVolumeProtectionScope.h"

#include <Windows.h>
#include <objbase.h>

#include <array>
#include <memory>
#include <set>

#include "infrastructure/uwf/errors/WindowsUwfError.h"
#include "infrastructure/uwf/windows/RuntimeAdmissionFacts.h"
#include "infrastructure/windows/errors/WindowsErrorText.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::windows {
namespace {
void canceled(const compat::Context& context) {
  if (context.stop.stop_requested()) throw compat::Error({compat::ErrorKind::Canceled, compat::Effect::None, "Volume scope observation canceled", {}});
}
[[noreturn]] void failWindows(const DWORD error, const char* operation) {
  const auto detail = windowsErrorText(error).toUtf8().toStdString();
  throw compat::Error({classifyWindowsUwfError(error, WindowsUwfErrorContext::Observation).value_or(compat::ErrorKind::Failed),
                       compat::Effect::None,
                       detail,
                       {{"WindowsVolumeScope", operation, static_cast<std::int64_t>(HRESULT_FROM_WIN32(error)), detail}}});
}
[[noreturn]] void malformed(const char* detail) { throw compat::Error({compat::ErrorKind::Protocol, compat::Effect::None, detail, {}}); }
std::string systemPartition(const compat::Context& context) {
  canceled(context);
  // Native VolumeAddAll queries the boot partition independently of the Windows
  // volume identity using IOCTL_MOUNTDEV_QUERY_STABLE_GUID (0x4d0018).
  UniqueHandle partition(CreateFileW(L"\\\\?\\SystemPartition", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL, nullptr));
  if (!partition.valid()) failWindows(GetLastError(), "open system partition");
  GUID identity{};
  DWORD returned = 0;
  if (!DeviceIoControl(partition.get(), 0x4d0018, nullptr, 0, &identity, sizeof(identity), &returned, nullptr))
    failWindows(GetLastError(), "read system partition stable GUID");
  canceled(context);
  if (returned != sizeof(identity)) malformed("The system partition stable GUID response is incomplete");
  std::array<wchar_t, 39> text{};
  if (StringFromGUID2(identity, text.data(), static_cast<int>(text.size())) != static_cast<int>(text.size()))
    malformed("The system partition stable GUID could not be formatted");
  const auto volume = compat::parseVolumeId(wideToUtf8(text.data()));
  if (!volume || *volume == "00000000-0000-0000-0000-000000000000") malformed("The system partition stable GUID is invalid");
  return *volume;
}
}  // namespace
VolumeProtectionInventory readVolumeProtectionInventory(const bool currentFilter, const compat::Context& context) {
  canceled(context);
  VolumeProtectionInventory result;
  std::set<std::string> scope;
  std::array<wchar_t, MAX_PATH + 50> name{};
  const auto handle = FindFirstVolumeW(name.data(), static_cast<DWORD>(name.size()));
  if (handle == INVALID_HANDLE_VALUE) failWindows(GetLastError(), "enumerate physical volumes for protection");
  const std::unique_ptr<void, decltype(&FindVolumeClose)> enumeration(handle, &FindVolumeClose);
  do {
    canceled(context);
    const auto identity = compat::parseVolumeId(wideToUtf8(name.data()));
    if (!identity) malformed("Physical volume enumeration returned an invalid identity");
    const auto observation = inspectVolumeAdmission(*identity, currentFilter, compat::VolumeAdmissionPurpose::Registration, context);
    if (const auto* failure = std::get_if<compat::Failure>(&observation)) throw compat::Error(*failure);
    const auto admission = std::get<compat::VolumeAdmission>(observation);
    result.admissions.emplace(*identity, admission);
    if (admission.protectionEligible) scope.insert(*identity);
  } while (FindNextVolumeW(handle, name.data(), static_cast<DWORD>(name.size())));
  const auto error = GetLastError();
  if (error != ERROR_NO_MORE_FILES) failWindows(error, "enumerate physical volumes for protection");
  const auto partition = systemPartition(context);
  if (!result.admissions.contains(partition)) {
    const auto observation = inspectVolumeAdmission(partition, currentFilter, compat::VolumeAdmissionPurpose::Registration, context);
    if (const auto* failure = std::get_if<compat::Failure>(&observation)) throw compat::Error(*failure);
    result.admissions.emplace(partition, std::get<compat::VolumeAdmission>(observation));
  }
  scope.insert(partition);
  result.scope.assign(scope.begin(), scope.end());
  return result;
}
std::vector<std::string> readAllVolumeProtectionScope(const bool currentFilter, const compat::Context& context) {
  return readVolumeProtectionInventory(currentFilter, context).scope;
}
}  // namespace uwf::windows
