// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "application/overlay/contracts/OverlayPolicyError.h"

namespace uwf::app {

enum class PersistentReset { Preserve = 0, Clear = 1, UseSavedMode = 255 };
enum class OverlayOption { Passthrough, Persistent, ReadOnlyMedia };
enum class PolicyObservationStatus { Unobserved, Available, Unsupported, Failed };

template <typename T>
struct PolicyObservation {
  PolicyObservationStatus status = PolicyObservationStatus::Unobserved;
  std::optional<T> value{};
  std::optional<OverlayPolicyFailure> failure{};
};

struct OverlayFlags {
  static constexpr std::uint32_t Passthrough = 0x1;
  static constexpr std::uint32_t Persistent = 0x2;
  static constexpr std::uint32_t ReadOnlyMedia = 0x4;
  std::uint32_t raw = 0;
  [[nodiscard]] bool passthrough() const { return (raw & Passthrough) != 0; }
  [[nodiscard]] bool persistent() const { return (raw & Persistent) != 0; }
  [[nodiscard]] bool readOnlyMedia() const { return (raw & ReadOnlyMedia) != 0; }
  bool operator==(const OverlayFlags&) const = default;
};

struct OverlayPolicySession {
  PolicyObservation<bool> filterEnabled;
  // Preserve unknown provider values instead of substituting RAM.
  PolicyObservation<std::int64_t> overlayType;
  PolicyObservation<OverlayFlags> flags;
};

struct OverlayPolicySnapshot {
  bool transportAvailable = false;
  OverlayPolicySession current;
  OverlayPolicySession next;
  PolicyObservation<std::int64_t> resetPersistentState;

  [[nodiscard]] std::optional<PersistentReset> resetMode() const;
  [[nodiscard]] std::optional<bool> currentPersistentDisk() const;
  // Retention across the next restart is distinct from committing to the base
  // volume. Both sessions must retain Disk + Persistent without ROM, the live
  // reset control must say Preserve, and the target must stay protected.
  [[nodiscard]] bool preservesOverlayData() const;
};

struct OverlayPolicyDelta {
  std::optional<bool> passthrough{};
  std::optional<bool> persistent{};
  std::optional<bool> readOnlyMedia{};
  std::optional<PersistentReset> reset{};
  [[nodiscard]] std::uint32_t applyFlags(std::uint32_t baseline) const;
  [[nodiscard]] bool empty() const;
  bool operator==(const OverlayPolicyDelta&) const = default;
};

struct OverlayPolicyUpdate {
  OverlayPolicyDelta changes;
  // Optional optimistic baseline, compared against published state before any write.
  std::optional<std::uint32_t> expectedFlags{};
  std::optional<std::int64_t> expectedReset{};
};

enum class PolicyStepOutcome { NotAttempted, Unchanged, Confirmed, Failed, Uncertain };
struct OverlayPolicyStep {
  OverlayProperty node;
  std::int64_t before;
  std::int64_t requested;
  // Value acknowledged by a successful command. A failed command does not
  // trigger an observation or infer success from matching state.
  std::optional<std::int64_t> observed{};
  PolicyStepOutcome outcome = PolicyStepOutcome::NotAttempted;
  std::optional<OverlayOption> option{};
};

struct OverlayPolicyResult {
  OverlayPolicySnapshot snapshot;
  std::vector<OverlayPolicyStep> steps;
  std::optional<OverlayPolicyFailure> failure;
  WriteEffect effect = WriteEffect::None;
  [[nodiscard]] bool succeeded() const { return !failure; }
  [[nodiscard]] bool reconciliationRequired() const { return effect != WriteEffect::None && failure.has_value(); }
};

}  // namespace uwf::app
