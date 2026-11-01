// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <optional>

namespace uwf::core {
// Missing observations never grant permission to register a physical volume.
// File-system capabilities and existing UWF membership are separate concerns.
struct PhysicalVolumeFacts {
  std::optional<bool> fixedMedia;
  std::optional<bool> supportedBus;
};

enum class PhysicalVolumeAdmission { Unknown, Eligible, NotFixedMedia, UnsupportedBus };

[[nodiscard]] constexpr PhysicalVolumeAdmission physicalVolumeAdmission(const PhysicalVolumeFacts& facts) {
  if (!facts.fixedMedia) return PhysicalVolumeAdmission::Unknown;
  if (!*facts.fixedMedia) return PhysicalVolumeAdmission::NotFixedMedia;
  if (!facts.supportedBus) return PhysicalVolumeAdmission::Unknown;
  return *facts.supportedBus ? PhysicalVolumeAdmission::Eligible : PhysicalVolumeAdmission::UnsupportedBus;
}
}  // namespace uwf::core
