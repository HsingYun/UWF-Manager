// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/operations/contracts/OperationError.h"
#include "application/overlay/contracts/MaintenanceValidation.h"
#include "application/staging/contracts/FileStagingTypes.h"
#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {

enum class ConfigurationStepOutcome {
  Succeeded,
  Failed,
  DependencySkipped,
  NotAttempted,
  PreparationFailed,
  AdmissionFailed,
  ExecutionFailed,
  VerificationFailed
};
struct ConfigurationMaintenanceDetail {
  OverlayMaintenanceSnapshot snapshot;
  std::vector<AllVolumeProtectionItemResult> allVolumeItems;
};
struct ConfigurationStepResult {
  std::string description;
  ConfigurationStepOutcome outcome;
  WriteEffect writeEffect = WriteEffect::None;
  std::optional<ConfigurationOperation> operation{};
  // A confirmed no-op may still invalidate the editor's observed baseline.
  bool requiresReconciliation = false;
  std::optional<MaintenanceValidationIssue> validationIssue{};
  std::optional<OverlayPolicyFailure> policyFailure{};
  std::optional<OperationFailure> failure{};
  std::optional<ConfigurationMaintenanceDetail> maintenance{};
  std::optional<FileStagingWriteResult> staging{};
};

struct ConfigurationApplyResult {
  // Operation receipts and batch failures are the sole execution facts. The
  // summaries below are projections; snapshots describe state, not receipts.
  std::vector<ConfigurationStepResult> steps{};
  std::optional<OverlayPolicySnapshot> observedOverlayPolicy{};
  std::optional<OverlayMaintenanceSnapshot> observedOverlayMaintenance{};

  [[nodiscard]] int succeededOperations() const { return count(ConfigurationStepOutcome::Succeeded); }
  [[nodiscard]] int failedOperations() const { return count(ConfigurationStepOutcome::Failed); }
  [[nodiscard]] int dependencySkippedOperations() const { return count(ConfigurationStepOutcome::DependencySkipped); }
  [[nodiscard]] int notAttemptedOperations() const { return count(ConfigurationStepOutcome::NotAttempted); }
  [[nodiscard]] int batchFailures() const {
    return count(ConfigurationStepOutcome::PreparationFailed) + count(ConfigurationStepOutcome::AdmissionFailed) +
           count(ConfigurationStepOutcome::ExecutionFailed) + count(ConfigurationStepOutcome::VerificationFailed);
  }
  [[nodiscard]] bool failed() const {
    return failedOperations() != 0 || dependencySkippedOperations() != 0 || notAttemptedOperations() != 0 || batchFailures() != 0;
  }
  [[nodiscard]] bool anyWriteConfirmed() const {
    return std::ranges::any_of(steps, [](const auto& step) {
      return step.writeEffect == WriteEffect::Confirmed || (step.maintenance && std::ranges::any_of(step.maintenance->allVolumeItems, [](const auto& item) {
                                                              return item.effect == WriteEffect::Confirmed;
                                                            }));
    });
  }
  [[nodiscard]] WriteEffect writeEffect() const {
    auto effect = WriteEffect::None;
    for (const auto& step : steps) {
      effect = combineWriteEffects(effect, step.writeEffect);
      if (step.maintenance)
        for (const auto& item : step.maintenance->allVolumeItems) effect = combineWriteEffects(effect, item.effect);
    }
    return effect;
  }
  [[nodiscard]] bool reconciliationRequired() const {
    return writeEffect() != WriteEffect::None || std::ranges::any_of(steps, [](const auto& step) { return step.requiresReconciliation; });
  }
  [[nodiscard]] const FileStagingWriteResult* staging() const {
    for (const auto& step : steps)
      if (step.staging) return &*step.staging;
    return nullptr;
  }

 private:
  [[nodiscard]] int count(const ConfigurationStepOutcome outcome) const {
    return static_cast<int>(std::ranges::count(steps, outcome, &ConfigurationStepResult::outcome));
  }
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
  [[nodiscard]] ConfigurationApplyResult apply(const ConfigurationPlan& plan) {
    const auto result = prepareApply(plan)();
    completeApply(result);
    return result;
  }
  // Prepare / publish run on the owner thread. The operation owns all worker
  // dependencies, independently of this facade and its QObject editors.
  [[nodiscard]] virtual ApplyOperation prepareApply(const core::PendingChanges& changes, const core::UwfSnapshot& snapshot) = 0;
  [[nodiscard]] virtual ApplyOperation prepareApply(const ConfigurationPlan& plan) {
    if (!plan.overlayPolicy.empty())
      throw OperationError(std::make_error_code(std::errc::not_supported), "This configuration executor does not support overlay policies");
    if (!plan.maintenance.empty())
      throw OperationError(std::make_error_code(std::errc::not_supported), "This configuration executor does not support maintenance operations");
    return prepareApply(plan.changes, plan.snapshot);
  }
  virtual void completeApply(const ConfigurationApplyResult&) {}
};

}  // namespace uwf::app
