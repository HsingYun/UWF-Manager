// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/power/WindowsHibernationCapabilities.h"

#include <Windows.h>
#include <powrprof.h>

#include <system_error>

namespace uwf::windows {
namespace {
void canceled(const std::stop_token stop) {
  if (stop.stop_requested()) throw app::OperationError(std::make_error_code(std::errc::operation_canceled), "Hibernation capability observation canceled");
}
template <class Capabilities>
std::uint8_t fileType(const Capabilities& value) {
  if constexpr (requires { value.HiberFileType; }) {
    return value.HiberFileType;
  } else {
    // Older SDK/MinGW headers retain the pre-Windows-10 spelling. WinNT.h's
    // Windows-10 ABI replaces the first two spare3 bytes with HiberFileType and
    // AoAcConnectivitySupported without changing SYSTEM_POWER_CAPABILITIES.
    return value.spare3[0];
  }
}
}  // namespace
app::OverlayHibernationCapabilities decodeWindowsHibernationCapabilities(const bool systemS4, const bool hiberFilePresent, const std::uint8_t hiberFileType) {
  const auto type = hiberFileType == 0   ? app::HibernationFileType::None
                    : hiberFileType == 1 ? app::HibernationFileType::Reduced
                    : hiberFileType == 2 ? app::HibernationFileType::Full
                                         : app::HibernationFileType::Unknown;
  return {systemS4, hiberFilePresent, type};
}
app::OverlayHibernationCapabilities readWindowsHibernationCapabilities(const std::stop_token stop) {
  canceled(stop);
  SYSTEM_POWER_CAPABILITIES capabilities{};
  if (!GetPwrCapabilities(&capabilities)) {
    const auto error = GetLastError();
    throw std::system_error(static_cast<int>(error == ERROR_SUCCESS ? ERROR_GEN_FAILURE : error), std::system_category(), "GetPwrCapabilities");
  }
  canceled(stop);
  return decodeWindowsHibernationCapabilities(capabilities.SystemS4 != FALSE, capabilities.HiberFilePresent != FALSE, fileType(capabilities));
}
}  // namespace uwf::windows
