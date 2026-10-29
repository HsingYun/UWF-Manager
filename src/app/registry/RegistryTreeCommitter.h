// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <vector>

#include "app/api/RegistryCommitTypes.h"
#include "app/uwf/UwfRegistryFilter.h"

namespace uwf::app {

class RegistryTreeCommitter final {
 public:
  explicit RegistryTreeCommitter(wmi::WmiOperations& session);

  [[nodiscard]] RegistryCommitResult commit(const std::vector<RegistryCommitTarget>& targets) const;
  [[nodiscard]] RegistryCommitResult commitDeletion(const std::vector<RegistryCommitTarget>& targets) const;

 private:
  app::provider::UwfRegistryFilter m_registry;
};

}  // namespace uwf::app
