/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "SystemInfoProvider.h"

#include <windows.h>

#include <QStringList>
#include <algorithm>
#include <cstdint>
#include <string_view>

#include "../core/Config.h"
#include "../util/RegistryKey.h"
#include "ThemeManager.h"

namespace uwf::ui {

namespace {

// RtlGetVersion 是唯一一个在 Windows 8.1+ 上仍返回真实版本号（而不是被
// 应用兼容性"撒谎"成 Windows 8）的接口。动态加载避免对 ntdll 的直接 link。
QString windowsVersionText() {
  using Fn = LONG(WINAPI*)(OSVERSIONINFOW*);
  auto fn = reinterpret_cast<Fn>(reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion")));
  OSVERSIONINFOW v{};
  v.dwOSVersionInfoSize = sizeof(v);
  if (!fn || fn(&v) != 0) return QStringLiteral("Windows");

  constexpr std::string_view kCur = config::kRegPathWindowsCurrentVersion;
  const auto ubr = regkey::readDword(kCur, "UBR");
  const QString productName = QString::fromStdString(regkey::readString(kCur, "ProductName")).trimmed();
  const QString editionId = QString::fromStdString(regkey::readString(kCur, "EditionID")).trimmed();

  QString family = QStringLiteral("Windows");
  if (v.dwMajorVersion == 10) {
    family = v.dwBuildNumber >= static_cast<DWORD>(config::kWindows11MinBuildNumber) ? QStringLiteral("Windows 11") : QStringLiteral("Windows 10");
  }

  QString edition = productName;
  for (const QString& p : {QStringLiteral("Windows 11 "), QStringLiteral("Windows 10 "), QStringLiteral("Windows ")}) {
    if (edition.startsWith(p, Qt::CaseInsensitive)) {
      edition = edition.mid(p.size()).trimmed();
      break;
    }
  }

  const QString ed = editionId.toLower();
  const bool isLtsc =
      std::ranges::any_of(config::kLtscEditionIds, [&ed](std::string_view id) { return ed == QLatin1String(id.data(), static_cast<qsizetype>(id.size())); });
  if (isLtsc && !edition.contains("LTSC", Qt::CaseInsensitive) && !edition.contains("LTSB", Qt::CaseInsensitive)) {
    edition = edition.isEmpty() ? QStringLiteral("LTSC") : (edition + QStringLiteral(" LTSC"));
  }

  const QString head = edition.isEmpty() ? family : (family + ' ' + edition);
  return QString("%1 · %2.%3.%4.%5").arg(head).arg(v.dwMajorVersion).arg(v.dwMinorVersion).arg(v.dwBuildNumber).arg(ubr);
}

QString cpuModelText() {
  const QString name =
      QString::fromStdString(regkey::readString(R"(HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0)", "ProcessorNameString"));
  return name.trimmed().simplified();
}

QString totalRamText() {
  MEMORYSTATUSEX m{};
  m.dwLength = sizeof(m);
  if (!GlobalMemoryStatusEx(&m)) return {};
  const auto gb = static_cast<uint64_t>((m.ullTotalPhys + (512ULL << 20)) / (1ULL << 30));
  if (gb >= 1) return QString("%1 GB").arg(gb);
  return QString("%1 MB").arg(m.ullTotalPhys / (1ULL << 20));
}

QString gpuModelText() {
  DISPLAY_DEVICEW dev{};
  dev.cb = sizeof(dev);
  for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dev, 0); ++i) {
    if (dev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) return QString::fromWCharArray(dev.DeviceString).trimmed();
    dev = {};
    dev.cb = sizeof(dev);
  }
  dev = {};
  dev.cb = sizeof(dev);
  if (EnumDisplayDevicesW(nullptr, 0, &dev, 0)) return QString::fromWCharArray(dev.DeviceString).trimmed();
  return {};
}

}  // namespace

QString SystemInfoProvider::summaryHtml() {
  const QString ver = windowsVersionText();
  const QString cpu = cpuModelText();
  const QString ram = totalRamText();
  const QString gpu = gpuModelText();
  QStringList rows;
  auto addPlain = [&](const QString& v) {
    if (!v.isEmpty()) rows << v.toHtmlEscaped();
  };
  const QString muted = ThemeManager::instance().color(Sem::FgMuted).name();
  auto addWithKey = [&](const QString& k, const QString& v) {
    if (!v.isEmpty()) rows << QString("<span style='color:%1'>%2</span>&nbsp;%3").arg(muted, k.toHtmlEscaped(), v.toHtmlEscaped());
  };
  addPlain(ver);
  addPlain(cpu);
  addWithKey("RAM", ram);
  addPlain(gpu);
  return rows.join("<br>");
}

}  // namespace uwf::ui
