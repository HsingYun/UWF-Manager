// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <array>
#include <optional>

namespace uwf::app {
enum class HormRequirement { FilterEnabled, ReadOnlyMedia, Hibernation, VolumeProtection };
enum class HormIssue {
  None,
  Unsupported,
  StateUnknown,
  FullCommitActive,
  ProtectionStateChanged,
  PrerequisitesUnknown,
  FilterDisabled,
  OverlayNotRam,
  ConfigurationIncompatible,
  HibernationUnavailable,
  VolumesUnprotected
};
struct HormEligibility {
  std::array<std::optional<bool>, 4> requirements;
  HormIssue issue = HormIssue::None;
  std::optional<HormRequirement> blockingRequirement{};
  [[nodiscard]] bool allowed() const { return issue == HormIssue::None; }
  [[nodiscard]] bool checklistCoversIssue() const { return blockingRequirement.has_value(); }
};
}  // namespace uwf::app
