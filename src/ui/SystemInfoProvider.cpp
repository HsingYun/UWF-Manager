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
#include <cstdint>

#include "../util/RegistryKey.h"
#include "../util/WindowsVersion.h"
#include "ThemeManager.h"

namespace uwf::ui {

namespace {

QString windowsVersionText() {
  const WindowsVersionInfo& version = windowsVersionInfo();
  QString head = QString::fromStdString(version.productName).trimmed();
  if (head.isEmpty()) head = QStringLiteral("Windows");

  if (version.longTermServicing && !head.contains("LTSC", Qt::CaseInsensitive) && !head.contains("LTSB", Qt::CaseInsensitive)) {
    head += QStringLiteral(" LTSC");
  }
  const QString displayVersion = QString::fromStdString(version.displayVersion).trimmed();
  if (!displayVersion.isEmpty() && !head.contains(displayVersion, Qt::CaseInsensitive)) {
    head += ' ';
    head += displayVersion;
  }

  if (version.major == 0) return head;
  return QString("%1 · %2.%3.%4.%5")
      .arg(head)
      .arg(version.major)
      .arg(version.minor)
      .arg(version.build)
      .arg(version.revision);
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
