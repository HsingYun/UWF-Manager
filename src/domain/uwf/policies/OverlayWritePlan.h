// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "domain/uwf/model/UwfModel.h"

namespace uwf::core {

enum class OverlayField { Type, MaximumSize, WarningThreshold, CriticalThreshold };

struct OverlayWriteStep {
  OverlayField field;
  bool requiresPrevious = false;
};

// Successful native type transitions normalize flags only when the type
// actually changes. Swapfile placement invokes RAM and then Disk in that order.
[[nodiscard]] std::uint32_t overlayFlagsAfterTypeChange(OverlayType before, OverlayType after, std::uint32_t flags);
[[nodiscard]] std::uint32_t overlayFlagsAfterStorage(OverlayType before, std::uint32_t flags);
[[nodiscard]] std::uint32_t overlayTypeFlagMask(OverlayType before, OverlayType after);
[[nodiscard]] std::uint32_t overlayStorageFlagMask(OverlayType before);

// Validate storage intent and intermediate type/size ordering before any writes.
// Unedited invalid observations and threshold-only edits do not imply storage repair.
void validateOverlayTarget(const OverlayConfig& baseline, const OverlayConfigDelta& changes);

// Dependent steps execute only after the preceding write is confirmed.
[[nodiscard]] std::vector<OverlayWriteStep> planOverlayWrites(const OverlayConfig& baseline, const OverlayConfigDelta& changes);

}  // namespace uwf::core
