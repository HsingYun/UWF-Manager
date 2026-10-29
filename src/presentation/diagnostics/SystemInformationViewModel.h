// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QStringList>

#include "app/diagnostics/DiagnosticReportService.h"
#include "presentation/diagnostics/DiagnosticReportFormatter.h"
#include "presentation/diagnostics/DiagnosticReportViewModel.h"
namespace uwf::presentation {
class SystemInformationViewModel final {
 public:
  explicit SystemInformationViewModel(const app::SystemInformationSource& source) : m_source(source), m_reports(source) {}
  [[nodiscard]] uint32_t totalRamMb() const { return m_source.totalRamMb(); }
  [[nodiscard]] QString systemDriveLetter() const { return m_source.systemDriveLetter(); }
  [[nodiscard]] const app::WindowsVersionInfo& windowsVersion() const { return m_source.windowsVersion(); }
  [[nodiscard]] const app::SystemHardwareInfo& hardware() const { return m_source.hardware(); }
  [[nodiscard]] QStringList summaryParts() const;
  [[nodiscard]] QString diagnosticReport(const app::DiagnosticReportContext& context) const {
    return DiagnosticReportFormatter::format(m_reports.collect(context));
  }

  [[nodiscard]] DiagnosticReportViewModel diagnosticDocument(const app::DiagnosticReportContext& context) const {
    return DiagnosticReportViewModel(m_reports.collect(context), m_source.totalRamMb());
  }

 private:
  const app::SystemInformationSource& m_source;
  app::DiagnosticReportService m_reports;
};
}  // namespace uwf::presentation
