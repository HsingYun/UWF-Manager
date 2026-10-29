// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/ports/SystemInformationSource.h"
namespace uwf::windows {
class WindowsSystemInformationSource final : public app::SystemInformationSource {
 public:
  [[nodiscard]] uint32_t totalRamMb() const override;
  [[nodiscard]] QString systemDriveLetter() const override;
  [[nodiscard]] const app::WindowsVersionInfo& windowsVersion() const override;
  [[nodiscard]] const app::SystemHardwareInfo& hardware() const override;
  [[nodiscard]] app::SystemDiagnosticFacts diagnostics() const override;
};
[[nodiscard]] const app::SystemInformationSource& systemInformationSource();
}  // namespace uwf::windows
