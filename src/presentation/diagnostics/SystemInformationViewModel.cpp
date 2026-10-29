// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SystemInformationViewModel.h"

namespace uwf::presentation {
QStringList SystemInformationViewModel::summaryParts() const {
  QStringList parts;
  QString windows = QString::fromStdString(windowsVersion().productName).trimmed();
  parts << (windows.isEmpty() ? QStringLiteral("Windows") : windows);
  const auto& info = hardware();
  constexpr std::uint64_t kBytesPerGiB = 1ULL << 30;
  const auto gib = info.totalMemoryBytes / kBytesPerGiB + (info.totalMemoryBytes % kBytesPerGiB >= kBytesPerGiB / 2 ? 1 : 0);
  if (gib > 0) parts << QStringLiteral("%1G").arg(gib);
  QString gpu = QString::fromStdString(info.graphicsAdapter.name).simplified();
  for (const auto* prefix : {"NVIDIA GeForce ", "NVIDIA ", "AMD Radeon ", "AMD ", "Intel(R) ", "Intel "}) {
    const QString value = QString::fromLatin1(prefix);
    if (gpu.startsWith(value, Qt::CaseInsensitive)) {
      gpu.remove(0, value.size());
      break;
    }
  }
  gpu.remove(QStringLiteral("(TM)"), Qt::CaseInsensitive).remove(QStringLiteral("(R)"), Qt::CaseInsensitive);
  if (!gpu.isEmpty()) parts << gpu.simplified();
  return parts;
}

}  // namespace uwf::presentation
