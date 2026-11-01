// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/policies/IndividualCommitPolicy.h"

#include <algorithm>

#include "domain/uwf/contracts/UwfStateFacts.h"
#include "domain/uwf/policies/ExclusionCoverage.h"
#include "foundation/text/Utf8.h"

namespace uwf::compat {
core::IndividualCommitState individualCommitState(const UwfStateFacts& state) {
  const auto bits = [](const BitFacts& facts, std::uint32_t required) -> std::optional<std::uint32_t> {
    return facts.valid() && (facts.knownMask & required) == required ? std::optional{facts.valueBits} : std::nullopt;
  };
  return {state.runtime.horm.value, bits(state.current.overlayFlags, flags::readOnlyMedia),
          bits(state.runtime.romDisposition, disposition::filterChange | disposition::horm | disposition::fullCommit), state.runtime.fullCommit.value};
}
bool requiresPersistentFileView(const UwfStateFacts& state, const Request& request, const std::string& volume) {
  if (std::holds_alternative<CommitFileDeletion>(request)) return true;
  const auto* file = std::get_if<CommitFile>(&request);
  if (!file || state.current.filterEnabled.value != true) return false;
  const auto found = state.current.volumes.find(normalizeVolumeId(volume));
  if (found == state.current.volumes.end() || found->second.protectedVolume.value != true) return false;
  const auto& exclusions = found->second.fileExclusions.value;
  if (!exclusions) return true;
  const auto path = utf8ToUtf16(file->path);
  return !path || !std::ranges::any_of(*exclusions, [&](const auto& entry) {
    const auto pattern = utf8ToUtf16(entry);
    return pattern && core::fileExclusionEffects(*pattern, *path).managementSkipsCommit;
  });
}
}  // namespace uwf::compat
