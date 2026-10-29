// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "app/ports/RegistryCommitPlan.h"

#include <exception>
#include <stdexcept>

#include "app/ports/RegistryKey.h"

namespace uwf::app {

std::vector<RegistryCommitTarget> planRegistryTreeCommit(const std::string& key) {
  std::vector<RegistryCommitTarget> targets;
  for (const auto& current : regkey::collectKeyTree(regkey::normalize(key))) {
    for (const auto& valueName : regkey::valueNames(current)) targets.push_back({current, valueName});
  }
  return targets;
}

std::vector<RegistryCommitTarget> planRegistryTreeDeletion(const std::string& key) {
  std::vector<RegistryCommitTarget> targets;
  for (const auto& current : regkey::collectKeyTree(regkey::normalize(key))) targets.push_back({current, {}});
  return targets;
}

}  // namespace uwf::app
