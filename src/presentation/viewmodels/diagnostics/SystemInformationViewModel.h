// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QStringList>
#include <memory>
#include <stdexcept>
#include <utility>

#include "application/diagnostics/DiagnosticReportService.h"
#include "presentation/viewmodels/diagnostics/DiagnosticReportFormatter.h"
#include "presentation/viewmodels/diagnostics/DiagnosticReportViewModel.h"
namespace uwf::presentation {
class SystemInformationViewModel final {
 public:
  explicit SystemInformationViewModel(std::shared_ptr<const app::SystemInformationSource> source)
      : m_source(requireSource(std::move(source))), m_reports(*m_source) {}
  [[nodiscard]] uint32_t totalRamMb() const { return m_source->totalRamMb(); }
  [[nodiscard]] QString systemDriveLetter() const { return m_source->systemDriveLetter(); }
  [[nodiscard]] const app::WindowsVersionInfo& windowsVersion() const { return m_source->windowsVersion(); }
  [[nodiscard]] const app::SystemHardwareInfo& hardware() const { return m_source->hardware(); }
  [[nodiscard]] QStringList summaryParts() const;
  [[nodiscard]] QString diagnosticReport(const app::DiagnosticReportContext& context) const {
    return DiagnosticReportFormatter::format(m_reports.collect(context));
  }

  [[nodiscard]] DiagnosticReportViewModel diagnosticDocument(const app::DiagnosticReportContext& context) const {
    return DiagnosticReportViewModel(m_reports.collect(context), m_source->totalRamMb());
  }

 private:
  static std::shared_ptr<const app::SystemInformationSource> requireSource(std::shared_ptr<const app::SystemInformationSource> source) {
    if (!source) throw std::invalid_argument("System information requires an owned source");
    return source;
  }
  std::shared_ptr<const app::SystemInformationSource> m_source;
  app::DiagnosticReportService m_reports;
};
}  // namespace uwf::presentation
