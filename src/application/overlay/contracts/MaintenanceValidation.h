// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <variant>

#include "application/operations/contracts/OperationError.h"
#include "application/overlay/contracts/HormEligibilityTypes.h"

namespace uwf::app {
enum class MaintenancePlanIssue {
  HormActive,
  UnprotectAndHormConflict,
  FullCommitAndHormConflict,
  StorageAndHormConflict,
  StorageAndFullCommitConflict,
  StorageVolumeRequired,
  StorageAndRamConflict,
  BatchProtectionUnsupported,
  StorageUnsupported,
  StorageRequiresFilterDisabled,
  FullCommitUnsupported,
  FullCommitRequiresReadOnlyMedia,
  FullCommitFilterChange,
  StorageRequiresHormDisabled,
  FullCommitRequiresHormDisabled,
  ResultUnconfirmed,
  Canceled,
  UnknownFailure,
  HormStateUnavailable,
  FullCommitStateUnavailable,
  StorageStateUnavailable,
  StorageCapacityUnavailable,
  StorageInventoryUnavailable,
  StorageVolumeUnavailable,
  Count
};

// Validation issues remain language-neutral until presentation.
using MaintenanceValidationIssue = std::variant<MaintenancePlanIssue, HormIssue>;

class MaintenanceValidationError final : public OperationError {
 public:
  explicit MaintenanceValidationError(MaintenanceValidationIssue issue, std::errc code = std::errc::invalid_argument)
      : OperationError(std::make_error_code(code), "Maintenance configuration validation rejected"), m_issue(issue) {}
  [[nodiscard]] const MaintenanceValidationIssue& issue() const { return m_issue; }

 private:
  MaintenanceValidationIssue m_issue;
};
}  // namespace uwf::app
