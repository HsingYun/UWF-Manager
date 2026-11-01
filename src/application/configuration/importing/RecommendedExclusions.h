// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string_view>
#include <vector>

#include "application/configuration/commands/UwfmgrCommand.h"

namespace uwf::app {
enum RecommendedExclusionSection {
  Ceip = 1 << 0,
  Bits = 1 << 1,
  Network = 1 << 2,
  Dst = 1 << 3,
  Defender = 1 << 4,
  Scep = 1 << 5,
  AllRecommendedSections = Ceip | Bits | Network | Dst | Defender | Scep
};

struct RecommendedExclusion {
  const char* description;
  UwfmgrCommand command;
};
struct RecommendedExclusionGroup {
  RecommendedExclusionSection section;
  const char* title;
  std::vector<RecommendedExclusion> entries;
  const char* note = nullptr;
};

// The catalog owns policy paths and semantic commands. Consumers choose groups
// and translate their source text; command spelling belongs to the CLI codec.
[[nodiscard]] std::vector<RecommendedExclusionGroup> recommendedExclusions(std::string_view systemDrive);
}  // namespace uwf::app
