// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "app/api/OverlayFilesTypes.h"

namespace uwf::app {

// 合并同一文件的 NTFS 流，保留首个原始名称，并按绝对路径排序。
[[nodiscard]] QVector<OverlayFileEntry> buildOverlayFileCatalog(const QString& driveLetter, QVector<OverlayFileEntry> entries);

[[nodiscard]] qulonglong overlayFileCatalogSize(const QVector<OverlayFileEntry>& entries);

}  // namespace uwf::app
