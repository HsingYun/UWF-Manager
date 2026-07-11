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

#include <QStringList>
#include <cstdint>

#include "../util/SystemHardwareInfo.h"
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
  return QString("%1 · %2.%3.%4.%5").arg(head).arg(version.major).arg(version.minor).arg(version.build).arg(version.revision);
}

QString memoryCapacityText(const std::uint64_t bytes) {
  if (bytes == 0) return {};
  constexpr std::uint64_t kMiB = 1ULL << 20;
  constexpr std::uint64_t kGiB = 1ULL << 30;
  const std::uint64_t roundedGiB = (bytes + kGiB / 2) / kGiB;
  if (roundedGiB >= 1) return QString("%1 GB").arg(roundedGiB);
  return QString("%1 MB").arg(bytes / kMiB);
}

template <typename Value>
Value commonMemoryValue(const std::vector<PhysicalMemoryModuleInfo>& modules, Value PhysicalMemoryModuleInfo::* member) {
  if (modules.empty()) return {};
  const Value value = modules.front().*member;
  if (value == Value{}) return {};
  for (const PhysicalMemoryModuleInfo& module : modules) {
    if (module.*member != value) return {};
  }
  return value;
}

QString ramText(const SystemHardwareInfo& info) {
  QStringList parts;
  const QString capacity = memoryCapacityText(info.totalMemoryBytes);
  if (!capacity.isEmpty()) parts << capacity;

  const auto memoryType = commonMemoryValue(info.memoryModules, &PhysicalMemoryModuleInfo::memoryType);
  const auto typeName = physicalMemoryTypeName(memoryType);
  if (!typeName.empty()) parts << QString::fromLatin1(typeName.data(), static_cast<qsizetype>(typeName.size()));

  const auto speed = commonMemoryValue(info.memoryModules, &PhysicalMemoryModuleInfo::configuredSpeedMtPerSecond);
  if (speed != 0) parts << QString("%1 MT/s").arg(speed);

  const auto installedModules = static_cast<std::uint64_t>(info.memoryModules.size());
  if (installedModules != 0 && info.totalMemorySlots >= installedModules) {
    parts << QString("%1/%2 slots").arg(installedModules).arg(info.totalMemorySlots);
  }
  return parts.join(QStringLiteral(" · "));
}

QString gpuText(const GraphicsAdapterInfo& adapter) {
  QStringList parts;
  const QString name = QString::fromStdString(adapter.name).trimmed().simplified();
  if (!name.isEmpty()) parts << name;
  const QString memory = memoryCapacityText(adapter.dedicatedVideoMemoryBytes);
  if (!memory.isEmpty()) parts << memory;
  return parts.join(QStringLiteral(" · "));
}

}  // namespace

QString SystemInfoProvider::summaryHtml() {
  const SystemHardwareInfo& hardware = systemHardwareInfo();
  const QString ver = windowsVersionText();
  const QString cpu = QString::fromStdString(hardware.cpuModel).trimmed().simplified();
  const QString ram = ramText(hardware);
  const QString gpu = gpuText(hardware.graphicsAdapter);
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
