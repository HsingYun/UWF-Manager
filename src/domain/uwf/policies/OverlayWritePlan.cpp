// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/policies/OverlayWritePlan.h"

#include <stdexcept>

#include "domain/uwf/contracts/Types.h"
#include "domain/uwf/policies/UwfConstraints.h"

namespace uwf::core {

std::uint32_t overlayTypeFlagMask(const OverlayType before, const OverlayType after) {
  if (before == after) return 0;
  if (after == OverlayType::Disk) return compat::flags::passthrough;
  if (after == OverlayType::RAM) return compat::flags::passthrough | compat::flags::persistent;
  return 0;
}
std::uint32_t overlayStorageFlagMask(const OverlayType before) {
  return overlayTypeFlagMask(before, OverlayType::RAM) | overlayTypeFlagMask(OverlayType::RAM, OverlayType::Disk);
}
std::uint32_t overlayFlagsAfterTypeChange(const OverlayType before, const OverlayType after, const std::uint32_t flags) {
  const auto mask = overlayTypeFlagMask(before, after);
  if (after == OverlayType::Disk) return flags | mask;
  if (after == OverlayType::RAM) return flags & ~mask;
  return flags;
}
std::uint32_t overlayFlagsAfterStorage(const OverlayType before, const std::uint32_t flags) {
  return overlayFlagsAfterTypeChange(OverlayType::RAM, OverlayType::Disk, overlayFlagsAfterTypeChange(before, OverlayType::RAM, flags));
}

void validateOverlayTarget(const OverlayConfig& baseline, const OverlayConfigDelta& changes) {
  if (!changes.touchesOverlayConfig()) return;
  if (!validOverlayTarget(changes.type.value_or(baseline.type), changes.maximumSizeMb.value_or(baseline.maximumSizeMb)))
    throw std::invalid_argument("The overlay target requires RAM size greater than zero or Disk size of at least 1024 MB");
  if (changes.type && changes.maximumSizeMb && !validOverlayTarget(*changes.type, baseline.maximumSizeMb) &&
      !validOverlayTarget(baseline.type, *changes.maximumSizeMb))
    throw std::invalid_argument("The observed overlay baseline cannot reach this target through valid type/size steps; repair its capacity first");
}

std::vector<OverlayWriteStep> planOverlayWrites(const OverlayConfig& baseline, const OverlayConfigDelta& changes) {
  validateOverlayTarget(baseline, changes);
  std::vector<OverlayWriteStep> steps;
  if (changes.type && changes.maximumSizeMb) {
    // Select an order whose intermediate type/size remains valid.
    const bool typeFirstAllowed = validOverlayTarget(*changes.type, baseline.maximumSizeMb);
    const bool sizeFirstAllowed = validOverlayTarget(baseline.type, *changes.maximumSizeMb);
    if (sizeFirstAllowed && (*changes.type == OverlayType::Disk || !typeFirstAllowed)) {
      steps.push_back({OverlayField::MaximumSize});
      steps.push_back({OverlayField::Type, true});
    } else {
      steps.push_back({OverlayField::Type});
      steps.push_back({OverlayField::MaximumSize, true});
    }
  } else if (changes.type) {
    steps.push_back({OverlayField::Type});
  } else if (changes.maximumSizeMb) {
    steps.push_back({OverlayField::MaximumSize});
  }

  if (changes.warningThresholdMb && changes.criticalThresholdMb) {
    // Zero disables an event; nonzero warnings may require raising Critical first.
    const bool warningNeedsCritical = baseline.criticalThresholdMb != 0 && *changes.warningThresholdMb >= baseline.criticalThresholdMb;
    const bool enableCriticalFirst = baseline.criticalThresholdMb == 0 && *changes.criticalThresholdMb > baseline.warningThresholdMb;
    if (*changes.warningThresholdMb != 0 && (warningNeedsCritical || enableCriticalFirst)) {
      steps.push_back({OverlayField::CriticalThreshold});
      steps.push_back({OverlayField::WarningThreshold, true});
    } else {
      steps.push_back({OverlayField::WarningThreshold});
      steps.push_back({OverlayField::CriticalThreshold, *changes.criticalThresholdMb <= baseline.warningThresholdMb});
    }
  } else if (changes.warningThresholdMb) {
    steps.push_back({OverlayField::WarningThreshold});
  } else if (changes.criticalThresholdMb) {
    steps.push_back({OverlayField::CriticalThreshold});
  }
  return steps;
}

}  // namespace uwf::core
