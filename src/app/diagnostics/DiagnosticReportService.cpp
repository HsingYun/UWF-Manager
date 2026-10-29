// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "DiagnosticReportService.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QSysInfo>
#include <algorithm>
#include <limits>

#include "shared/Log.h"
#include "uwf_version.h"
namespace uwf::app {
namespace {
QString overlayTypeText(const core::OverlayType type) {
  switch (type) {
    case core::OverlayType::RAM:
      return QStringLiteral("RAM");
    case core::OverlayType::Disk:
      return QStringLiteral("disk");
    case core::OverlayType::Unknown:
      return QStringLiteral("unknown");
  }
  return QStringLiteral("unknown");
}

QString enhancedModeStateText(const app::EnhancedModeState state) {
  switch (state) {
    case app::EnhancedModeState::Disabled:
      return QStringLiteral("disabled");
    case app::EnhancedModeState::Enabled:
      return QStringLiteral("enabled");
    case app::EnhancedModeState::Stopped:
      return QStringLiteral("stopped");
    case app::EnhancedModeState::RepairRequired:
      return QStringLiteral("repair-required");
  }
  return QStringLiteral("unknown");
}

QString enhancedModeAgentStateText(const app::EnhancedModeAgentState state) {
  switch (state) {
    case app::EnhancedModeAgentState::Unobserved:
      return QStringLiteral("unobserved");
    case app::EnhancedModeAgentState::Connecting:
      return QStringLiteral("connecting");
    case app::EnhancedModeAgentState::Disconnected:
      return QStringLiteral("disconnected");
    case app::EnhancedModeAgentState::Connected:
      return QStringLiteral("connected");
  }
  return QStringLiteral("unknown");
}

DiagnosticSessionSummary captureSession(const core::SessionSnapshot& session) {
  return {
      .filterEnabled = session.filter.enabled,
      .overlayType = overlayTypeText(session.overlay.type),
      .maximumSizeMb = session.overlay.maximumSizeMb,
      .warningThresholdMb = session.overlay.warningThresholdMb,
      .criticalThresholdMb = session.overlay.criticalThresholdMb,
      .persistDomainSecretKey = session.persistDomainSecretKey,
      .persistTscal = session.persistTSCAL,
  };
}

}  // namespace
DiagnosticReportContext DiagnosticReportService::capture(const app::UwfCapability capability, const core::UwfSnapshot* const snapshot,
                                                         const app::EnhancedModeStatus* const enhancedMode, const bool compatibilityMode) {
  DiagnosticReportContext context;
  context.compatibilityMode = compatibilityMode;
  context.uwf.capability = capability == app::UwfCapability::Available ? QStringLiteral("available") : QStringLiteral("unavailable");
  if (snapshot) {
    context.uwf.snapshotPresent = true;
    context.uwf.uwfAvailable = snapshot->uwfAvailable;
    context.uwf.elevated = snapshot->elevated;
    context.uwf.unavailableReason = QString::fromStdString(snapshot->unavailableReason);
    context.uwf.overlayConsumptionMb = snapshot->runtime.currentConsumptionMb;
    context.uwf.overlayAvailableMb = snapshot->runtime.availableSpaceMb;
    context.uwf.current = captureSession(snapshot->current);
    context.uwf.next = captureSession(snapshot->next);
    for (const auto& volume : snapshot->current.volumes)
      if (volume.isProtected && !volume.driveLetter.empty()) context.uwf.protectedVolumes.append(QString::fromStdString(volume.driveLetter));
    context.uwf.protectedVolumes.removeDuplicates();
  }
  if (enhancedMode) {
    context.enhancedMode = {
        .present = true,
        .state = enhancedModeStateText(enhancedMode->state),
        .serviceExists = enhancedMode->serviceExists,
        .serviceRegistryPresent = enhancedMode->serviceRegistryPresent,
        .ownProcess = enhancedMode->ownProcess,
        .automaticStart = enhancedMode->automaticStart,
        .localSystemAccount = enhancedMode->localSystemAccount,
        .running = enhancedMode->running,
        .executableMatches = enhancedMode->executableMatches,
        .preshutdownTimeoutConfigured = enhancedMode->preshutdownTimeoutConfigured,
        .requiredPrivilegesConfigured = enhancedMode->requiredPrivilegesConfigured,
        .preshutdownAccepted = enhancedMode->preshutdownAccepted,
        .serviceContractSatisfied = enhancedMode->serviceContractSatisfied(),
        .agentState = enhancedModeAgentStateText(enhancedMode->agentState),
        .detail = enhancedMode->detail,
    };
  }
  return context;
}

DiagnosticReportData DiagnosticReportService::collect(const DiagnosticReportContext& context) const {
  DiagnosticReportData report;
  report.context = context;
  report.generatedLocal = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
  report.generatedUtc = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
  report.version = QString::fromLatin1(UWF_VER_STRING);
  report.qtRuntime = QString::fromLatin1(qVersion());
  report.qtBuild = QString::fromLatin1(QT_VERSION_STR);
#if defined(NDEBUG)
  report.buildConfiguration = QStringLiteral("release");
#else
  report.buildConfiguration = QStringLiteral("debug");
#endif
  report.executable = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
  report.processArchitecture = QSysInfo::buildCpuArchitecture();
  report.system = m_source.diagnostics();
  const quint64 boundedUptime = std::min(report.system.uptimeMs, static_cast<quint64>(std::numeric_limits<qint64>::max()));
  report.bootTimeLocal = QDateTime::currentDateTime().addMSecs(-static_cast<qint64>(boundedUptime)).toString(Qt::ISODateWithMs);
  constexpr std::size_t kDiagnosticLogLines = 64;
  constexpr std::size_t kDiagnosticLogLineBytes = 1024;
  report.logs = recentLogLines(kDiagnosticLogLines, kDiagnosticLogLineBytes);
  return report;
}
}  // namespace uwf::app
