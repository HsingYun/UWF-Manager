// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowsSystemInformationSource.h"

#include <windows.h>

#include <QLocale>
#include <QSysInfo>
#include <exception>

#include "app/ports/VolumePaths.h"
#include "windows/security/ProcessTokenDiagnostics.h"
#include "windows/system/SystemHardwareInfo.h"
#include "windows/system/WindowsErrorText.h"
#include "windows/system/WindowsVersion.h"
namespace uwf::windows {
namespace {
QString systemTimeZoneText() {
  DYNAMIC_TIME_ZONE_INFORMATION info{};
  const DWORD state = GetDynamicTimeZoneInformation(&info);
  if (state == TIME_ZONE_ID_INVALID) {
    const DWORD error = GetLastError();
    return windows::windowsErrorText(error);
  }

  const LONG seasonalBias = state == TIME_ZONE_ID_DAYLIGHT ? info.DaylightBias : info.StandardBias;
  const LONG offsetMinutes = -(info.Bias + seasonalBias);
  const LONG absoluteMinutes = offsetMinutes < 0 ? -offsetMinutes : offsetMinutes;
  const QString offset = QStringLiteral("UTC%1%2:%3")
                             .arg(offsetMinutes >= 0 ? QChar('+') : QChar('-'))
                             .arg(absoluteMinutes / 60, 2, 10, QChar('0'))
                             .arg(absoluteMinutes % 60, 2, 10, QChar('0'));
  QString name = QString::fromWCharArray(info.TimeZoneKeyName).trimmed();
  if (name.isEmpty()) name = QString::fromWCharArray(state == TIME_ZONE_ID_DAYLIGHT ? info.DaylightName : info.StandardName).trimmed();
  return name.isEmpty() ? offset : QStringLiteral("%1 (%2)").arg(name, offset);
}

}  // namespace
uint32_t WindowsSystemInformationSource::totalRamMb() const { return systemTotalRamMb(); }
QString WindowsSystemInformationSource::systemDriveLetter() const { return QString::fromStdString(drive::systemLetter()); }
const app::WindowsVersionInfo& WindowsSystemInformationSource::windowsVersion() const { return windowsVersionInfo(); }
const app::SystemHardwareInfo& WindowsSystemInformationSource::hardware() const { return systemHardwareInfo(); }
app::SystemDiagnosticFacts WindowsSystemInformationSource::diagnostics() const {
  app::SystemDiagnosticFacts facts;
  facts.processId = GetCurrentProcessId();
  try {
    facts.windows = windowsVersionInfo();
  } catch (const std::exception& error) {
    facts.windowsError = QString::fromUtf8(error.what());
  }
  facts.architecture = QSysInfo::currentCpuArchitecture();
  facts.locale = QLocale::system().name();
  facts.uiLanguages = QLocale::system().uiLanguages();
  facts.timeZone = systemTimeZoneText();
  facts.uptimeMs = GetTickCount64();
  facts.token = collectProcessTokenDiagnostics();
  return facts;
}
const app::SystemInformationSource& systemInformationSource() {
  static const WindowsSystemInformationSource source;
  return source;
}
}  // namespace uwf::windows
