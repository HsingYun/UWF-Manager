// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>
#include <utility>

#include "application/overlay/files/OverlayFilesQueryManager.h"

namespace uwf::test {
[[nodiscard]] inline QString overlayFilesVolumeGuid(const QString& drive) {
  return QStringLiteral("11111111-2222-3333-4444-%1").arg(drive.isEmpty() ? 0 : drive.front().toUpper().unicode(), 12, 16, QLatin1Char('0'));
}
[[nodiscard]] inline std::shared_ptr<app::OverlayFilesQueryManager> overlayFilesQueryManager(app::OverlayFileLoader loader) {
  return std::make_shared<app::OverlayFilesQueryManager>(std::move(loader));
}
}  // namespace uwf::test
