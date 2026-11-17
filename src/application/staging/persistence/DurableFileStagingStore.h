// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/staging/contracts/FileStagingStorage.h"
#include "application/staging/contracts/FileStagingStore.h"
#include "application/uwf/contracts/UwfCapability.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"

namespace uwf::app {
// Persist replacements in Current. Startup reconciliation also restores Next
// exclusion coverage through ConfigurationPlan.
class DurableFileStagingStore final : public FileStagingStore {
 public:
  DurableFileStagingStore(std::shared_ptr<FileStagingStorage> storage, std::shared_ptr<UwfMaintenanceRepository> repository, UwfCapability capability);
  [[nodiscard]] QList<FileStagingEntry> load() const override;
  void replace(const QList<FileStagingEntry>& entries) override;
  void reconcileDurability();

 private:
  [[nodiscard]] std::shared_ptr<UwfRegistryMaintenanceTarget> prepareCurrentCommit(CommitRequirement requirement) const;
  std::shared_ptr<FileStagingStorage> m_storage;
  std::shared_ptr<UwfMaintenanceRepository> m_repository;
  UwfCapability m_capability;
};
}  // namespace uwf::app
