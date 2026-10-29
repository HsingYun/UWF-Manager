// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "app/api/OverlayFilesTypes.h"

namespace uwf::windows {

[[nodiscard]] QVector<app::OverlayFileEntry> loadOverlayFilesFromSystem(const QString& driveLetter, std::stop_token stopToken);

}  // namespace uwf::windows
