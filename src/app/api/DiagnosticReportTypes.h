// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QString>
#include <QStringList>
#include <optional>
#include <string>
#include <vector>

#include "app/api/SystemInformation.h"
namespace uwf::app {
struct DiagnosticSessionSummary {
  bool filterEnabled = false;
  QString overlayType;
  std::uint32_t maximumSizeMb = 0;
  std::uint32_t warningThresholdMb = 0;
  std::uint32_t criticalThresholdMb = 0;
  bool persistDomainSecretKey = false;
  bool persistTscal = false;
};

struct UwfDiagnosticSummary {
  QString capability = QStringLiteral("unavailable");
  bool snapshotPresent = false;
  bool uwfAvailable = false;
  bool elevated = false;
  QString unavailableReason;
  std::uint32_t overlayConsumptionMb = 0;
  std::uint32_t overlayAvailableMb = 0;
  QStringList protectedVolumes;
  DiagnosticSessionSummary current;
  DiagnosticSessionSummary next;
};

struct EnhancedModeDiagnosticSummary {
  bool present = false;
  QString state;
  bool serviceExists = false;
  bool serviceRegistryPresent = false;
  bool ownProcess = false;
  bool automaticStart = false;
  bool localSystemAccount = false;
  bool running = false;
  bool executableMatches = false;
  bool preshutdownTimeoutConfigured = false;
  bool requiredPrivilegesConfigured = false;
  bool preshutdownAccepted = false;
  bool serviceContractSatisfied = false;
  QString agentState;
  QString detail;
};

// 诊断报告只持有实际展示的标量字段，不复制卷、排除项或 overlay 文件等完整
// UWF 快照。MainWindow 在用户点击“系统信息”时创建该 DTO。
struct DiagnosticReportContext {
  bool compatibilityMode = false;
  UwfDiagnosticSummary uwf;
  EnhancedModeDiagnosticSummary enhancedMode;
};

struct DiagnosticField {
  QString key;
  QString value;
};
struct SystemDiagnosticFacts {
  std::optional<WindowsVersionInfo> windows;
  QString windowsError;
  QString architecture;
  QString locale;
  QStringList uiLanguages;
  QString timeZone;
  quint64 uptimeMs = 0;
  quint32 processId = 0;
  std::vector<DiagnosticField> token;
};
struct DiagnosticReportData {
  DiagnosticReportContext context;
  SystemDiagnosticFacts system;
  QString generatedLocal;
  QString generatedUtc;
  QString bootTimeLocal;
  QString executable;
  QString version;
  QString qtRuntime;
  QString qtBuild;
  QString buildConfiguration;
  QString processArchitecture;
  std::vector<std::string> logs;
};
}  // namespace uwf::app
