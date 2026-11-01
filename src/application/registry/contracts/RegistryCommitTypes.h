// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "application/operations/contracts/OperationError.h"

namespace uwf::app {

enum class RegistryTargetKind { Value, Key };

struct RegistryCommitTarget {
  std::string key;
  std::string valueName;
  // Empty Value is the exact default value. Key is an explicit whole-key
  // deletion target; an empty name never silently selects it.
  RegistryTargetKind kind = RegistryTargetKind::Value;
  [[nodiscard]] static RegistryCommitTarget wholeKey(std::string key) { return {std::move(key), {}, RegistryTargetKind::Key}; }
  [[nodiscard]] bool isKey() const noexcept { return kind == RegistryTargetKind::Key; }
  [[nodiscard]] std::optional<std::string> deletionValueName() const {
    if (kind == RegistryTargetKind::Key && valueName.empty()) return std::nullopt;
    if (kind == RegistryTargetKind::Value && !valueName.empty()) return valueName;
    throw std::invalid_argument("Registry deletion requires an explicit whole key or a nonempty named value");
  }
};

struct RegistryCommitFailure {
  RegistryCommitTarget target;
  std::string detail;
  std::optional<OperationFailure> operation{};
};

struct RegistryCommitResult {
  std::size_t attempted = 0;
  std::size_t committed = 0;
  std::vector<RegistryCommitFailure> failures;
  WriteEffect effect = WriteEffect::None;

  [[nodiscard]] bool succeeded() const { return failures.empty(); }
  [[nodiscard]] WriteEffect writeEffect() const {
    auto combined = effect;
    for (const auto& failure : failures)
      if (failure.operation) combined = combineWriteEffects(combined, failure.operation->effect);
    return combined;
  }
};

}  // namespace uwf::app
