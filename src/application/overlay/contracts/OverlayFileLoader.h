// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <stop_token>

#include "application/overlay/contracts/OverlayFilesTypes.h"

namespace uwf::app {
// Receives a bound, canonical volume GUID, never a display drive letter.
using OverlayFileLoader = std::function<QVector<OverlayFileEntry>(const QString& volumeGuid, std::stop_token stopToken)>;
}  // namespace uwf::app
