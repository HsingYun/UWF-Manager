// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/registry/RegistryCommitPlan.h"

#include "infrastructure/windows/registry/RegistryKey.h"

namespace uwf::windows {

std::vector<app::RegistryCommitTarget> planRegistryTreeCommit(const std::string& key) {
  std::vector<app::RegistryCommitTarget> targets;
  for (const auto& current : regkey::collectKeyTree(key)) {
    for (const auto& valueName : regkey::valueNames(current)) targets.push_back({current, valueName});
  }
  return targets;
}

std::vector<app::RegistryCommitTarget> planRegistryTreeDeletion(const std::string& key) {
  std::vector<app::RegistryCommitTarget> targets;
  for (const auto& current : regkey::collectKeyTree(key)) targets.push_back(app::RegistryCommitTarget::wholeKey(current));
  return targets;
}

}  // namespace uwf::windows
