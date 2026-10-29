// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "DiagnosticReportViewModel.h"

#include <QLocale>
#include <utility>

#include "DiagnosticReportFormatter.h"
#include "presentation/common/PresentationText.h"
namespace uwf::presentation {
DiagnosticReportViewModel::DiagnosticReportViewModel(QString report) : m_report(std::move(report)) {}
DiagnosticReportViewModel::DiagnosticReportViewModel(const app::DiagnosticReportData& report, const quint32 totalRamMb)
    : m_report(DiagnosticReportFormatter::format(report)), m_partial(!report.system.windows || !report.system.windowsError.isEmpty() || totalRamMb == 0) {
  const auto unknown = PresentationText::tr("Unavailable");
  const auto& uwf = report.context.uwf;
  const auto& enhanced = report.context.enhancedMode;
  const auto enabled = [&](const bool value) { return value ? PresentationText::tr("Enabled") : PresentationText::tr("Disabled"); };
  const QLocale numbers(QLocale::English, QLocale::UnitedStates);
  const auto capacity = totalRamMb >= 1024 ? numbers.toString(qRound(static_cast<double>(totalRamMb) / 1024.0)) + QStringLiteral(" GB")
                        : totalRamMb       ? numbers.toString(totalRamMb) + QStringLiteral(" MB")
                                           : unknown;
  const QString architecture = report.system.architecture.isEmpty() ? report.processArchitecture : report.system.architecture;
  const QString architectureText = architecture == QLatin1String("x64")   ? PresentationText::tr("64-bit")
                                   : architecture == QLatin1String("x86") ? PresentationText::tr("32-bit")
                                   : architecture.isEmpty()               ? unknown
                                                                          : architecture;
  m_sections.append({PresentationText::tr("System"),
                     QStringLiteral(":/icons/service.svg"),
                     {{PresentationText::tr("Operating system"), report.system.windows ? QString::fromStdString(report.system.windows->productName) : unknown},
                      {PresentationText::tr("Architecture"), architectureText},
                      {PresentationText::tr("System memory"), capacity}}});
  m_sections.append({PresentationText::tr("Permissions and protection"),
                     QStringLiteral(":/icons/enhanced.svg"),
                     {{PresentationText::tr("Privileges"),
                       uwf.snapshotPresent ? uwf.elevated ? PresentationText::tr("Administrator") : PresentationText::tr("Standard user") : unknown},
                      {PresentationText::tr("UWF feature"), uwf.uwfAvailable ? PresentationText::tr("Available") : unknown},
                      {PresentationText::tr("Current filter"), uwf.snapshotPresent && uwf.uwfAvailable ? enabled(uwf.current.filterEnabled) : unknown}}});
  const QString usage =
      uwf.snapshotPresent && uwf.uwfAvailable
          ? QStringLiteral("%1 / %2 MB")
                .arg(numbers.toString(uwf.overlayConsumptionMb), numbers.toString(static_cast<qulonglong>(uwf.overlayConsumptionMb) + uwf.overlayAvailableMb))
          : unknown;
  m_sections.append({PresentationText::tr("Overlay"),
                     QStringLiteral(":/icons/disk.svg"),
                     {{PresentationText::tr("Type"),
                       uwf.snapshotPresent && uwf.uwfAvailable
                           ? uwf.current.overlayType.compare(QStringLiteral("ram"), Qt::CaseInsensitive) == 0 ? QStringLiteral("RAM") : QStringLiteral("Disk")
                           : unknown},
                      {PresentationText::tr("Current usage"), usage},
                      {PresentationText::tr("Protected volumes"),
                       uwf.snapshotPresent && uwf.uwfAvailable
                           ? uwf.protectedVolumes.isEmpty() ? PresentationText::tr("None") : uwf.protectedVolumes.join(QStringLiteral(" · "))
                           : unknown}}});
  const auto agent = enhanced.agentState == QLatin1String("connected")    ? PresentationText::tr("Connected and authenticated")
                     : enhanced.agentState == QLatin1String("connecting") ? PresentationText::tr("Connecting…")
                                                                          : PresentationText::tr("Not connected");
  m_sections.append({PresentationText::tr("Enhanced mode"),
                     QStringLiteral(":/icons/file_staging.svg"),
                     {{PresentationText::tr("Service status"), enhanced.present ? enhanced.running         ? PresentationText::tr("Running")
                                                                                  : enhanced.serviceExists ? PresentationText::tr("Stopped")
                                                                                                           : PresentationText::tr("Not installed")
                                                                                : unknown},
                      {PresentationText::tr("Service account"),
                       enhanced.present && enhanced.serviceExists && enhanced.localSystemAccount ? QStringLiteral("LocalSystem") : unknown},
                      {PresentationText::tr("UI agent"), enhanced.present ? agent : unknown}}});
}
}  // namespace uwf::presentation
