// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <string>

#include "domain/uwf/contracts/Backend.h"
#include "domain/uwf/contracts/BackendObservations.h"
#include "infrastructure/windows/storage/AllVolumeProtectionScope.h"

namespace uwf::windows {
struct WindowsConfigurationEnvironment {
  std::function<bool()> isAdministrator;
  std::function<bool()> hiberFileExists;
  std::function<std::string()> systemVolumeId{};
  std::function<compat::Response(const compat::Context&)> romDisposition{};
  std::function<compat::Response(const compat::Context&)> hormVolumeProtection{};
  std::function<compat::Response(const compat::Context&)> capturedSettingsAvailable{};
  std::function<compat::Response(const compat::Context&)> shutdownPrivilegeAvailable{};
  std::function<compat::ServicingAdmissionResult(bool, const compat::Context&)> servicingAdmission{};
  // One complete observation owns both bulk scope and per-volume admission.
  std::function<VolumeProtectionInventory(bool, const compat::Context&)> volumeProtectionInventory{};
};
[[nodiscard]] WindowsConfigurationEnvironment windowsConfigurationEnvironment();
}  // namespace uwf::windows
