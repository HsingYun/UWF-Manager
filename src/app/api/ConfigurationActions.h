// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "app/api/ConfigurationPlan.h"
#include "app/api/FileStagingTypes.h"
#include "app/api/OperationError.h"
#include "core/UwfModel.h"

namespace uwf::app {

enum class ConfigurationStepOutcome { Succeeded, Failed, Skipped };
struct ConfigurationStepResult {
  std::string description;
  ConfigurationStepOutcome outcome;
  WriteEffect writeEffect = WriteEffect::None;
  std::optional<ConfigurationOperation> operation{};
};

struct ConfigurationApplyResult {
  std::vector<std::string> lines;
  bool anyWriteConfirmed = false;
  bool reconciliationRequired = false;
  bool connectionEstablished = true;
  int succeededOperations = 0;
  int failedOperations = 0;
  std::vector<ConfigurationStepResult> steps{};
  std::optional<FileStagingWriteResult> staging{};
};

class ConfigurationActions {
 public:
  using ApplyOperation = std::function<ConfigurationApplyResult()>;
  virtual ~ConfigurationActions() = default;
  [[nodiscard]] ConfigurationApplyResult apply(const core::PendingChanges& changes, const core::UwfSnapshot& snapshot) {
    const auto result = prepareApply(changes, snapshot)();
    completeApply(result);
    return result;
  }
  // Prepare / publish run on the owner thread. The operation owns all worker
  // dependencies, independently of this facade and its QObject editors.
  [[nodiscard]] virtual ApplyOperation prepareApply(const core::PendingChanges& changes, const core::UwfSnapshot& snapshot) = 0;
  virtual void completeApply(const ConfigurationApplyResult&) {}
};

}  // namespace uwf::app
