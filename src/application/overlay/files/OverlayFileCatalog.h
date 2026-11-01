// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/overlay/contracts/OverlayFilesTypes.h"

namespace uwf::app {

// Merge NTFS streams per object, retain the first raw name, and sort by absolute path.
[[nodiscard]] QVector<OverlayFileEntry> buildOverlayFileCatalog(const QString& driveLetter, QVector<OverlayFileEntry> entries);

[[nodiscard]] qulonglong overlayFileCatalogSize(const QVector<OverlayFileEntry>& entries);

}  // namespace uwf::app
