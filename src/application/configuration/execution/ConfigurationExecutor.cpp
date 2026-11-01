// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/execution/ConfigurationExecutor.h"

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "application/configuration/execution/ConfigurationPlanExecution.h"
#include "application/configuration/execution/ConfigurationResult.h"
#include "application/localization/ApplicationText.h"

namespace uwf::app {
namespace {
bool maintenanceOperation(const ConfigurationOperationKind kind) {
  return kind == ConfigurationOperationKind::ProtectAllVolumes || kind == ConfigurationOperationKind::UnprotectAllVolumes ||
         kind == ConfigurationOperationKind::OverlayStorage || kind == ConfigurationOperationKind::Horm || kind == ConfigurationOperationKind::FullCommit;
}
void validatePublishedMaintenance(const ConfigurationPlan& plan, const OverlayMaintenanceSnapshot& state) {
  auto publishedPlan = plan;
  publishedPlan.maintenanceSnapshot = state;
  validateOverlayMaintenancePlan(publishedPlan);
  const auto missing = [](const MaintenancePlanIssue issue) { throw MaintenanceValidationError(issue, std::errc::state_not_recoverable); };
  if ((plan.maintenance.hormEnabled || plan.maintenance.allVolumesProtected) && !state.hormEnabled) missing(MaintenancePlanIssue::HormStateUnavailable);
  if (plan.maintenance.fullCommit && (!state.filterEnabled || !state.currentFlags || !state.hormEnabled || !state.overlayCommit))
    missing(MaintenancePlanIssue::FullCommitStateUnavailable);
  if (plan.maintenance.storageVolume) {
    if (!state.filterEnabled || !state.hormEnabled) missing(MaintenancePlanIssue::StorageStateUnavailable);
    if (!state.maximumOverlaySize && !plan.changes.setOverlay.maximumSizeMb) missing(MaintenancePlanIssue::StorageCapacityUnavailable);
    if (plan.maintenanceSnapshot && plan.maintenanceSnapshot->maximumOverlaySize && state.maximumOverlaySize &&
        plan.maintenanceSnapshot->maximumOverlaySize != state.maximumOverlaySize)
      throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "Overlay capacity changed since the storage target was reviewed");
    if (!state.volumesReadable) missing(MaintenancePlanIssue::StorageInventoryUnavailable);
    const auto found = std::ranges::find_if(state.volumes, [&](const auto& volume) { return volume.id == *plan.maintenance.storageVolume && volume.eligible; });
    if (found == state.volumes.end()) missing(MaintenancePlanIssue::StorageVolumeUnavailable);
  }
}
void executeMaintenance(ConfigurationApplyResult& result, const ConfigurationPlan& plan, OverlayMaintenanceActions& actions) {
  for (const auto& operation : plan.operations()) {
    if (!maintenanceOperation(operation.kind)) continue;
    if (result.failed()) break;
    OverlayMaintenanceResult observed;
    std::string description;
    try {
      if (operation.kind == ConfigurationOperationKind::ProtectAllVolumes || operation.kind == ConfigurationOperationKind::UnprotectAllVolumes) {
        description = operation.kind == ConfigurationOperationKind::ProtectAllVolumes ? ApplicationText::tr("Protect all").toStdString()
                                                                                      : ApplicationText::tr("Unprotect all").toStdString();
        observed = actions.setAllVolumesProtected(*plan.maintenance.allVolumesProtected);
      } else if (operation.kind == ConfigurationOperationKind::OverlayStorage) {
        description = ApplicationText::tr("Configure storage location").toStdString();
        observed = actions.createSwap(*plan.maintenance.storageVolume);
      } else if (operation.kind == ConfigurationOperationKind::Horm) {
        description = ApplicationText::tr("Hibernate once / resume many").toStdString() + " (HORM)";
        observed = actions.setHorm(*plan.maintenance.hormEnabled);
      } else {
        description = ApplicationText::tr("Full volume commit").toStdString();
        observed = actions.fullCommit();
      }
    } catch (const MaintenanceValidationError& error) {
      observed.validationIssue = error.issue();
      observed.failure = operationFailure(error);
      observed.effect = observed.failure->effect;
    } catch (const std::exception& error) {
      observed.error = QString::fromUtf8(error.what());
      observed.failure = operationFailure(error);
      observed.effect = observed.failure->effect;
    } catch (...) {
      observed.validationIssue = MaintenancePlanIssue::UnknownFailure;
      observed.failure = OperationFailure{OperationFailureKind::Failed, WriteEffect::Uncertain, {}};
      observed.effect = WriteEffect::Uncertain;
    }
    if (!observed.succeeded() && observed.error.isEmpty() && !observed.validationIssue) {
      observed.validationIssue = MaintenancePlanIssue::ResultUnconfirmed;
      if (!observed.failure) observed.failure = OperationFailure{OperationFailureKind::Failed, observed.effect, {}};
    }
    const bool success = observed.succeeded();
    const auto detail = success || observed.validationIssue ? description : description + ": " + observed.error.toStdString();
    ConfigurationStepResult receipt{detail, success ? ConfigurationStepOutcome::Succeeded : ConfigurationStepOutcome::Failed, observed.effect, operation};
    receipt.validationIssue = observed.validationIssue;
    receipt.failure = observed.failure;
    receipt.maintenance = ConfigurationMaintenanceDetail{observed.snapshot, std::move(observed.allVolumeItems)};
    recordConfigurationStep(result, std::move(receipt));
    result.observedOverlayMaintenance = observed.snapshot;
  }
}
}  // namespace
ConfigurationExecutor::ConfigurationExecutor(std::shared_ptr<ConfigurationRepository> repository, std::optional<FileStagingChanges> stagingChanges,
                                             std::shared_ptr<OverlayMaintenanceActions> maintenance)
    : m_repository(std::move(repository)), m_stagingChanges(std::move(stagingChanges)), m_maintenance(std::move(maintenance)) {
  if (!m_repository) throw std::invalid_argument("Configuration repository must not be null");
}
ConfigurationActions::ApplyOperation ConfigurationExecutor::prepareApply(const core::PendingChanges& changes, const core::UwfSnapshot& snapshot) {
  return prepareApply(planConfiguration(changes, snapshot, m_stagingChanges));
}
ConfigurationActions::ApplyOperation ConfigurationExecutor::prepareApply(const ConfigurationPlan& plan) {
  if (!plan.maintenance.empty() && !m_maintenance)
    throw OperationError(std::make_error_code(std::errc::operation_not_supported), "This executor cannot apply maintenance operations");
  std::vector<PreparedConfigurationPhase> prepared;
  for (const auto& phase : configurationPhases(plan)) {
    std::visit(
        [&](const auto& payload) {
          using Payload = std::decay_t<decltype(payload)>;
          if constexpr (std::is_same_v<Payload, ConfigurationBulkProtectionPhase>) {
            auto bulkPlan = plan;
            bulkPlan.maintenance = {.allVolumesProtected = payload.enabled};
            prepared.push_back({phase, [maintenance = m_maintenance, bulkPlan = std::move(bulkPlan)](const OverlayPolicySnapshot&) {
                                  ConfigurationApplyResult result;
                                  executeMaintenance(result, bulkPlan, *maintenance);
                                  return result;
                                }});
          } else if constexpr (std::is_same_v<Payload, ConfigurationMaintenancePhase>) {
            auto maintenancePlan = plan;
            maintenancePlan.maintenance = payload.changes;
            prepared.push_back({phase, [maintenance = m_maintenance, maintenancePlan = std::move(maintenancePlan)](const OverlayPolicySnapshot&) {
                                  ConfigurationApplyResult result;
                                  executeMaintenance(result, maintenancePlan, *maintenance);
                                  return result;
                                }});
          } else {
            static_assert(std::is_same_v<Payload, ConfigurationSettingsPhase> || std::is_same_v<Payload, ConfigurationOverlayPolicyPhase>);
            prepared.push_back({phase, m_repository->prepareWrite(phase, plan)});
          }
        },
        phase);
  }
  return [plan, prepared = std::move(prepared), repository = m_repository, maintenance = m_maintenance] {
    (void)repository;  // Keep the complete repository session alive through execution.
    ConfigurationApplyResult admission;
    if (!plan.maintenance.empty()) {
      try {
        admission.observedOverlayMaintenance = maintenance->snapshot();
        validatePublishedMaintenance(plan, admission.observedOverlayMaintenance.value_or(OverlayMaintenanceSnapshot{}));
      } catch (const std::exception& error) {
        ConfigurationStepResult failure{{}, ConfigurationStepOutcome::AdmissionFailed, WriteEffect::None};
        if (const auto* validation = dynamic_cast<const MaintenanceValidationError*>(&error))
          failure.validationIssue = validation->issue();
        else
          failure.description = error.what();
        recordConfigurationStep(admission, std::move(failure));
        completeConfigurationResult(admission, plan);
        logConfigurationException("maintenance admission", std::current_exception(), admission);
        return admission;
      }
    }
    auto published = plan.requiresPolicyContext() ? repository->snapshot() : plan.policySnapshot;
    if (plan.maintenance.storageVolume && admission.observedOverlayMaintenance)
      published = fillOverlayPolicyObservations(std::move(published), *admission.observedOverlayMaintenance);
    const auto readPublished = [&] {
      ConfigurationObservations observed;
      if (plan.requiresPolicyContext()) observed.policy = repository->snapshot();
      if (!plan.maintenance.empty()) observed.maintenance = maintenance->snapshot();
      if (plan.maintenance.storageVolume && observed.maintenance)
        observed.policy = fillOverlayPolicyObservations(std::move(observed.policy), *observed.maintenance);
      return observed;
    };
    auto result = executeConfigurationPlan(plan, prepared, published, readPublished);
    if (!result.observedOverlayMaintenance) result.observedOverlayMaintenance = std::move(admission.observedOverlayMaintenance);
    return result;
  };
}
void ConfigurationExecutor::completeApply(const ConfigurationApplyResult& result) { m_repository->completeWrite(result); }
}  // namespace uwf::app
