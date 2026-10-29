// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace uwf::app {

struct RegistryCommitTarget {
  std::string key;
  std::string valueName;
};

struct RegistryCommitFailure {
  RegistryCommitTarget target;
  std::string detail;
};

struct RegistryCommitResult {
  std::size_t attempted = 0;
  std::size_t committed = 0;
  std::size_t skipped = 0;
  std::vector<RegistryCommitFailure> failures;

  [[nodiscard]] bool succeeded() const { return failures.empty(); }
};

}  // namespace uwf::app
