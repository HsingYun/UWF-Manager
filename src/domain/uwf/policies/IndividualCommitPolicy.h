// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <cstdint>
#include <optional>
#include <string>

#include "domain/uwf/contracts/Requests.h"

namespace uwf::core {
enum class IndividualCommitRestriction { None, HormActive, ReadOnlyMediaActive, PersistentViewUnavailable };
inline constexpr std::uint32_t kLoadedReadOnlyMediaFlag = 0x4;
struct IndividualCommitState {
  std::optional<bool> hormEnabled{};
  std::optional<std::uint32_t> overlayFlags{};
  std::optional<std::uint32_t> romDisposition{};
  std::optional<bool> fullCommit{};
};
// Native IsReadOnlyMediaUnsafe: bit4 remains ROM; bits1/2 suppress ROM only
// without bit4 and without HORM. FullCommit never creates a persistent view.
[[nodiscard]] constexpr std::optional<bool> effectiveReadOnlyMedia(const IndividualCommitState& state) {
  if (state.hormEnabled == true) return true;
  if (!state.overlayFlags) return {};
  if ((*state.overlayFlags & kLoadedReadOnlyMediaFlag) == 0) return state.hormEnabled ? std::optional{false} : std::nullopt;
  if (state.fullCommit == true) return true;
  if (!state.romDisposition) return {};
  if ((*state.romDisposition & compat::disposition::fullCommit) != 0) return true;
  if ((*state.romDisposition & (compat::disposition::filterChange | compat::disposition::horm)) != 0)
    return state.hormEnabled ? std::optional{false} : std::nullopt;
  return true;
}
// A restriction is returned only from known facts. Preparation can require
// complete state; execution retains the provider's own final capability checks.
[[nodiscard]] constexpr IndividualCommitRestriction individualCommitRestriction(bool currentFilterEnabled, bool targetProtected,
                                                                                const IndividualCommitState& state,
                                                                                std::optional<bool> persistentFileViewReady = {}) {
  if (!currentFilterEnabled || !targetProtected) return IndividualCommitRestriction::None;
  if (state.hormEnabled == true) return IndividualCommitRestriction::HormActive;
  if (effectiveReadOnlyMedia(state) == true) return IndividualCommitRestriction::ReadOnlyMediaActive;
  if (persistentFileViewReady == false) return IndividualCommitRestriction::PersistentViewUnavailable;
  return IndividualCommitRestriction::None;
}
}  // namespace uwf::core
namespace uwf::compat {
struct UwfStateFacts;
[[nodiscard]] core::IndividualCommitState individualCommitState(const UwfStateFacts& state);
[[nodiscard]] bool requiresPersistentFileView(const UwfStateFacts& state, const Request& request, const std::string& volume);
}  // namespace uwf::compat
