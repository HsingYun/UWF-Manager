// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "DiagnosticReportFormatter.h"

#include <QStringList>
namespace uwf::presentation {
namespace {
QString yesNo(const bool value) { return value ? QStringLiteral("yes") : QStringLiteral("no"); }

void addSection(QStringList& lines, const QString& title) {
  if (!lines.isEmpty()) lines.append(QString{});
  lines.append(QStringLiteral("[%1]").arg(title));
}

void addValue(QStringList& lines, const QString& key, const QString& value) {
  lines.append(QStringLiteral("%1 = %2").arg(key, value.isEmpty() ? QStringLiteral("<empty>") : value));
}

void appendSession(QStringList& lines, const QString& name, const app::DiagnosticSessionSummary& session) {
  addValue(lines, name + QStringLiteral(".filter.enabled"), yesNo(session.filterEnabled));
  addValue(lines, name + QStringLiteral(".overlay.type"), session.overlayType);
  addValue(lines, name + QStringLiteral(".overlay.maximum_mb"), QString::number(session.maximumSizeMb));
  addValue(lines, name + QStringLiteral(".overlay.warning_mb"), QString::number(session.warningThresholdMb));
  addValue(lines, name + QStringLiteral(".overlay.critical_mb"), QString::number(session.criticalThresholdMb));
  addValue(lines, name + QStringLiteral(".registry.persist_domain_secret_key"), yesNo(session.persistDomainSecretKey));
  addValue(lines, name + QStringLiteral(".registry.persist_tscal"), yesNo(session.persistTscal));
}

void appendUwfInformation(QStringList& lines, const app::UwfDiagnosticSummary& summary) {
  addSection(lines, QStringLiteral("Unified Write Filter"));
  addValue(lines, QStringLiteral("capability"), summary.capability);
  addValue(lines, QStringLiteral("snapshot.available"), yesNo(summary.snapshotPresent));
  if (!summary.snapshotPresent) return;
  addValue(lines, QStringLiteral("snapshot.uwf_available"), yesNo(summary.uwfAvailable));
  addValue(lines, QStringLiteral("snapshot.elevated"), yesNo(summary.elevated));
  if (!summary.unavailableReason.isEmpty()) addValue(lines, QStringLiteral("snapshot.unavailable_reason"), summary.unavailableReason);
  if (!summary.uwfAvailable) return;
  addValue(lines, QStringLiteral("runtime.overlay_consumption_mb"), QString::number(summary.overlayConsumptionMb));
  addValue(lines, QStringLiteral("runtime.overlay_available_mb"), QString::number(summary.overlayAvailableMb));
  appendSession(lines, QStringLiteral("current"), summary.current);
  appendSession(lines, QStringLiteral("next"), summary.next);
}

void appendEnhancedModeInformation(QStringList& lines, const app::EnhancedModeDiagnosticSummary& summary) {
  addSection(lines, QStringLiteral("Enhanced mode"));
  if (!summary.present) {
    addValue(lines, QStringLiteral("status"), QStringLiteral("not available in this application instance"));
    return;
  }
  addValue(lines, QStringLiteral("state"), summary.state);
  addValue(lines, QStringLiteral("service.exists"), yesNo(summary.serviceExists));
  addValue(lines, QStringLiteral("service.registry_present"), yesNo(summary.serviceRegistryPresent));
  addValue(lines, QStringLiteral("service.own_process"), yesNo(summary.ownProcess));
  addValue(lines, QStringLiteral("service.automatic_start"), yesNo(summary.automaticStart));
  addValue(lines, QStringLiteral("service.local_system"), yesNo(summary.localSystemAccount));
  addValue(lines, QStringLiteral("service.running"), yesNo(summary.running));
  addValue(lines, QStringLiteral("service.executable_matches"), yesNo(summary.executableMatches));
  addValue(lines, QStringLiteral("service.preshutdown_timeout_configured"), yesNo(summary.preshutdownTimeoutConfigured));
  addValue(lines, QStringLiteral("service.required_privileges_configured"), yesNo(summary.requiredPrivilegesConfigured));
  addValue(lines, QStringLiteral("service.preshutdown_accepted"), yesNo(summary.preshutdownAccepted));
  addValue(lines, QStringLiteral("service.contract_satisfied"), yesNo(summary.serviceContractSatisfied));
  addValue(lines, QStringLiteral("agent.state"), summary.agentState);
  if (!summary.detail.isEmpty()) addValue(lines, QStringLiteral("detail"), summary.detail);
}

}  // namespace
QString DiagnosticReportFormatter::format(const app::DiagnosticReportData& report) {
  const auto& context = report.context;
  QStringList lines;
  lines.append(QStringLiteral("UWF Manager diagnostic report"));
  addValue(lines, QStringLiteral("generated.local"), report.generatedLocal);
  addValue(lines, QStringLiteral("generated.utc"), report.generatedUtc);

  addSection(lines, QStringLiteral("Application"));
  addValue(lines, QStringLiteral("version"), report.version);
  addValue(lines, QStringLiteral("qt.runtime"), report.qtRuntime);
  addValue(lines, QStringLiteral("qt.build"), report.qtBuild);
  addValue(lines, QStringLiteral("build.configuration"), report.buildConfiguration);
  addValue(lines, QStringLiteral("executable"), report.executable);
  addValue(lines, QStringLiteral("process.id"), QString::number(report.system.processId));
  addValue(lines, QStringLiteral("process.architecture"), report.processArchitecture);
  addValue(lines, QStringLiteral("compatibility_mode"), yesNo(context.compatibilityMode));

  addSection(lines, QStringLiteral("Windows"));
  if (report.system.windows) {
    const auto& version = *report.system.windows;
    addValue(lines, QStringLiteral("windows.product"), QString::fromStdString(version.productName));
    addValue(lines, QStringLiteral("windows.edition_id"), QString::fromStdString(version.editionId));
    addValue(lines, QStringLiteral("windows.display_version"), QString::fromStdString(version.displayVersion));
    addValue(lines, QStringLiteral("windows.version"),
             QStringLiteral("%1.%2.%3.%4").arg(version.major).arg(version.minor).arg(version.build).arg(version.revision));
    addValue(lines, QStringLiteral("windows.ltsc"), yesNo(version.longTermServicing));
  } else {
    addValue(lines, QStringLiteral("windows.error"), report.system.windowsError);
  }
  addValue(lines, QStringLiteral("system.architecture"), report.system.architecture);
  addValue(lines, QStringLiteral("system.locale"), report.system.locale);
  addValue(lines, QStringLiteral("system.ui_languages"), report.system.uiLanguages.join(QStringLiteral(", ")));
  addValue(lines, QStringLiteral("system.time_zone"), report.system.timeZone);
  addValue(lines, QStringLiteral("system.uptime_ms"), QString::number(report.system.uptimeMs));
  addValue(lines, QStringLiteral("system.boot_time.local"), report.bootTimeLocal);

  addSection(lines, QStringLiteral("Security and process token"));
  for (const app::DiagnosticField& field : report.system.token) addValue(lines, field.key, field.value);

  appendUwfInformation(lines, context.uwf);
  appendEnhancedModeInformation(lines, context.enhancedMode);

  addSection(lines, QStringLiteral("Recent application log"));
  const auto& logs = report.logs;
  addValue(lines, QStringLiteral("lines.count"), QString::number(logs.size()));
  lines.append(QStringLiteral("--- begin log ---"));
  for (const auto& line : logs) lines.append(QString::fromStdString(line));
  lines.append(QStringLiteral("--- end log ---"));
  return lines.join(QChar('\n'));
}

}  // namespace uwf::presentation
