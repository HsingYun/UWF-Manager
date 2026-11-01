// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <vector>

#include "application/registry/contracts/RegistryCommitTypes.h"

namespace uwf::windows {

// Snapshot targets before SCM marks a service for deletion; execute that
// snapshot while the deletion barrier still retains the registry subtree.
[[nodiscard]] std::vector<app::RegistryCommitTarget> planRegistryTreeCommit(const std::string& key);
[[nodiscard]] std::vector<app::RegistryCommitTarget> planRegistryTreeDeletion(const std::string& key);

}  // namespace uwf::windows
