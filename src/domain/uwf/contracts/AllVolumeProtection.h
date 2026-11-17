// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>

#include "domain/uwf/contracts/Backend.h"

namespace uwf::compat {
struct ProtectAll {};
struct UnprotectAll {};
struct GetAllProtected {};
struct GetAllUnprotected {};

struct AllVolumeProtectionEntry {
  std::string volume;
  // Known missing Next registration is false. Other getter failures remain
  // unknown, with their original failure; they must never become absence.
  std::optional<bool> nextProtected;
  std::optional<Failure> failure;
};
struct AllVolumeProtectionObservation {
  bool scopeConfirmed = false;
  std::vector<AllVolumeProtectionEntry> volumes;
  std::optional<bool> value;
  std::optional<Failure> failure;
};
struct AllVolumeProtectionItemResult {
  std::string volume;
  std::optional<bool> before;
  // Value established by the existing snapshot or a successful write receipt.
  std::optional<bool> confirmedProtection;
  std::optional<Failure> failure;
  Effect effect = Effect::None;
  // Confirmed completion (receipt or already satisfied); not an independent readback.
  bool completionConfirmed = false;
  std::vector<Attempt> attempts;
};
struct AllVolumeProtectionResult {
  AllVolumeProtectionObservation observation;
  std::vector<AllVolumeProtectionItemResult> items;
  std::optional<Failure> failure;
  Effect effect = Effect::None;
  // Confirmed completion (receipt or already satisfied); not an independent readback.
  bool completionConfirmed = false;
  [[nodiscard]] bool succeeded() const { return !failure && completionConfirmed; }
};
// A complete read-only physical scope, including the system partition. This
// port never writes or takes its target list from presentation inventory.
struct AllVolumeProtectionEnvironment {
  std::function<bool(const Context&)> isAdministrator;
  std::function<std::vector<std::string>(bool currentFilter, const Context&)> protectScope;
};
// Compose Next protection operations without changing bindings, exclusions,
// or registration membership. ProtectAll uses eligible physical identities plus
// the system partition; UnprotectAll uses all Next registrations, including offline ones.
// The predicates are independent: mixed protection is false for both; unreadable
// members remain unknown. Empty physical scope is unknown, while confirmed empty
// Next inventory proves all unprotected.
// Item failures may permit later work; fatal session/cancellation failures stop it.
// Success requires an acknowledged receipt for every item; later display-state
// projections never repair failures or invalidate acknowledged writes.
class AllVolumeProtection {
 public:
  virtual ~AllVolumeProtection() = default;
  [[nodiscard]] virtual AllVolumeProtectionObservation read(const GetAllProtected&, const Context& context = {}) const = 0;
  [[nodiscard]] virtual AllVolumeProtectionObservation read(const GetAllUnprotected&, const Context& context = {}) const = 0;
  // Pure projections of shared global scopes and volume facts; never refresh.
  [[nodiscard]] virtual AllVolumeProtectionObservation snapshot(const GetAllProtected&) const { return {}; }
  [[nodiscard]] virtual AllVolumeProtectionObservation snapshot(const GetAllUnprotected&) const { return {}; }
  [[nodiscard]] virtual AllVolumeProtectionResult execute(const ProtectAll&, const Context& context = {}) const = 0;
  [[nodiscard]] virtual AllVolumeProtectionResult execute(const UnprotectAll&, const Context& context = {}) const = 0;
};
}  // namespace uwf::compat
