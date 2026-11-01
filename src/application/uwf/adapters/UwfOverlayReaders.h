// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>
#include <stop_token>

#include "application/overlay/contracts/OverlayFilesTypes.h"
#include "application/overlay/contracts/OverlayUsageReader.h"
#include "domain/uwf/contracts/Backend.h"

namespace uwf::app {
[[nodiscard]] std::shared_ptr<OverlayUsageReader> uwfOverlayUsageReader(std::shared_ptr<compat::Uwf> system);
[[nodiscard]] QVector<OverlayFileEntry> loadUwfOverlayFiles(const compat::Uwf& system, const QString& volumeGuid, std::stop_token stopToken);
}  // namespace uwf::app
