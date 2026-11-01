// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "application/diagnostics/contracts/DiagnosticReportTypes.h"
#include "application/enhanced/contracts/EnhancedModeStatus.h"
#include "application/uwf/contracts/UwfCapability.h"
#include "application/diagnostics/contracts/SystemInformationSource.h"
#include "domain/uwf/model/UwfModel.h"
namespace uwf::app {
class DiagnosticReportService final {
 public:
  explicit DiagnosticReportService(const SystemInformationSource& source) : m_source(source) {}
  [[nodiscard]] static DiagnosticReportContext capture(UwfCapability capability, const core::UwfSnapshot* snapshot, const EnhancedModeStatus* enhancedMode,
                                                       bool compatibilityMode);
  [[nodiscard]] DiagnosticReportData collect(const DiagnosticReportContext& context) const;

 private:
  const SystemInformationSource& m_source;
};
}  // namespace uwf::app
