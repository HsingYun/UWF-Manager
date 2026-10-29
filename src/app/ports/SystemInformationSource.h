// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/api/DiagnosticReportTypes.h"
namespace uwf::app {
class SystemInformationSource {
 public:
  virtual ~SystemInformationSource() = default;
  [[nodiscard]] virtual uint32_t totalRamMb() const = 0;
  [[nodiscard]] virtual QString systemDriveLetter() const = 0;
  [[nodiscard]] virtual const WindowsVersionInfo& windowsVersion() const = 0;
  [[nodiscard]] virtual const SystemHardwareInfo& hardware() const = 0;
  [[nodiscard]] virtual SystemDiagnosticFacts diagnostics() const = 0;
};
}  // namespace uwf::app
