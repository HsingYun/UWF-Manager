// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "OverlayWritePlan.h"

namespace uwf::core {

std::vector<OverlayWriteStep> planOverlayWrites(const OverlayConfig& baseline, const OverlayConfigDelta& changes) {
  std::vector<OverlayWriteStep> steps;
  if (changes.type && changes.maximumSizeMb) {
    // 切入磁盘模式前准备目标容量；切入 RAM 后再解除磁盘容量与可用空间限制。
    if (*changes.type == OverlayType::Disk) {
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
    // 0 关闭相应事件；恢复非零警告值时先建立足够高的严重阈值。
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
