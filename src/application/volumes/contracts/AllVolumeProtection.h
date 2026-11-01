// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QString>
#include <optional>
#include <stop_token>
#include <vector>

#include "application/operations/contracts/OperationError.h"
namespace uwf::app {
struct AllVolumeProtectionSnapshot {
  std::optional<bool> nextAllProtected;
  bool protectSupported = false, unprotectSupported = false;
  bool protectScopeConfirmed = false, unprotectScopeConfirmed = false;
  std::optional<bool> nextAllUnprotected;
  QString protectError, unprotectError;
  std::vector<QString> protectTargetVolumes;
  std::vector<QString> unprotectTargetVolumes;
};
struct AllVolumeProtectionItemResult {
  QString volumeId;
  std::optional<bool> before;
  // Value established by the existing snapshot or a successful write receipt.
  std::optional<bool> confirmedProtection;
  QString error;
  std::optional<OperationFailure> failure;
  WriteEffect effect = WriteEffect::None;
  // Confirmed completion (receipt or already satisfied); not an independent readback.
  bool completionConfirmed = false;
};
struct AllVolumeProtectionResult {
  AllVolumeProtectionSnapshot snapshot;
  QString error;
  std::optional<OperationFailure> failure;
  WriteEffect effect = WriteEffect::None;
  // Confirmed completion (receipt or already satisfied); not an independent readback.
  bool completionConfirmed = false;
  std::vector<AllVolumeProtectionItemResult> items;
  [[nodiscard]] bool succeeded() const { return error.isEmpty() && !failure && completionConfirmed; }
};
// A complete global operation, including eligible volumes and the system
// partition. UI lists never supply this operation's target scope.
class AllVolumeProtectionActions {
 public:
  virtual ~AllVolumeProtectionActions() = default;
  [[nodiscard]] virtual AllVolumeProtectionSnapshot observe(std::stop_token stop = {}) = 0;
  // Pure projection of already published state; never refresh or probe here.
  [[nodiscard]] virtual std::optional<AllVolumeProtectionSnapshot> snapshot() const { return {}; }
  [[nodiscard]] virtual AllVolumeProtectionResult setProtected(bool enabled, std::stop_token stop = {}) = 0;
};
}  // namespace uwf::app
