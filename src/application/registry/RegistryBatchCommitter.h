// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <vector>

#include "application/registry/contracts/RegistryCommitTypes.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"

namespace uwf::app {

class RegistryBatchCommitter final {
 public:
  explicit RegistryBatchCommitter(std::shared_ptr<UwfMaintenanceRepository> repository);

  [[nodiscard]] RegistryCommitResult commit(const std::vector<RegistryCommitTarget>& targets) const;
  // Persist a postorder plan after external host deletion. This does not perform
  // interactive deletion. Ordinary driver exclusions or unprotected system storage
  // make prior host deletions durable; management-only exclusions do not.
  [[nodiscard]] RegistryCommitResult persistDeletions(const std::vector<RegistryCommitTarget>& targets) const;

 private:
  std::shared_ptr<UwfMaintenanceRepository> m_registry;
};

}  // namespace uwf::app
