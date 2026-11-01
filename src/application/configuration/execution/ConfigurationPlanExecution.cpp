// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/execution/ConfigurationPlanExecution.h"

#include <algorithm>
#include <exception>
#include <iomanip>
#include <type_traits>

#include "application/configuration/execution/ConfigurationResult.h"
#include "application/configuration/overlay/OverlayPolicyConstraints.h"
#include "application/localization/ApplicationText.h"
#include "domain/uwf/policies/OverlayWritePlan.h"
#include "foundation/logging/Log.h"

namespace uwf::app {
namespace {
template <typename T>
T require(const PolicyObservation<T>& field, OverlayProperty node) {
  if (field.status == PolicyObservationStatus::Available && field.value) return *field.value;
  if (field.failure) throw OverlayPolicyError(*field.failure);
  throw OverlayPolicyError({.origin = OverlayPolicyFailureOrigin::Protocol,
                            .kind = OperationFailureKind::Protocol,
                            .node = node,
                            .operation = "VerifyConfiguration",
                            .message = "Required overlay observation is missing"});
}
void match(bool equal, OverlayProperty node) {
  if (!equal)
    throw OverlayPolicyError({.origin = OverlayPolicyFailureOrigin::Conflict,
                              .kind = OperationFailureKind::Conflict,
                              .node = node,
                              .operation = "VerifyConfiguration",
                              .message = "Configuration differs from the reviewed target or baseline"});
}
void verifyPolicy(const ConfigurationPlan& plan, const OverlayPolicySnapshot& state) {
  const auto& delta = plan.overlayPolicy;
  const auto baseline = configurationOverlayBaseline(plan);
  if (plan.requiresOverlayFlagObservation()) {
    const auto flags = require(state.next.flags, OverlayProperty::NextOverlayFlags).raw;
    match(flags == require(baseline.next.flags, OverlayProperty::NextOverlayFlags).raw, OverlayProperty::NextOverlayFlags);
    match(require(state.next.overlayType, OverlayProperty::NextOverlayType) == require(baseline.next.overlayType, OverlayProperty::NextOverlayType),
          OverlayProperty::NextOverlayType);
  }
  if (plan.maintenance.storageVolume || plan.changes.setOverlay.touchesOverlayConfig() || overlayOptionsRequireCurrentFilterOff(baseline, delta))
    match(!require(state.current.filterEnabled, OverlayProperty::CurrentFilterEnabled), OverlayProperty::CurrentFilterEnabled);
  if (delta.reset) {
    match(require(state.resetPersistentState, OverlayProperty::ResetPersistentState) ==
              require(baseline.resetPersistentState, OverlayProperty::ResetPersistentState),
          OverlayProperty::ResetPersistentState);
  }
}
void verifyTarget(const ConfigurationPlan& plan, const OverlayPolicySnapshot& state) {
  const auto target = configurationOverlayTarget(plan);
  if (plan.maintenance.storageVolume || plan.changes.setOverlay.type)
    match(require(state.next.overlayType, OverlayProperty::NextOverlayType) == static_cast<std::int64_t>(target.overlay.type),
          OverlayProperty::NextOverlayType);
  if (plan.requiresOverlayFlagObservation() && target.flagMask != 0) {
    const auto flags = require(state.next.flags, OverlayProperty::NextOverlayFlags);
    if (target.flags) match((flags.raw & target.flagMask) == (target.flags->raw & target.flagMask), OverlayProperty::NextOverlayFlags);
    if (target.overlay.type == core::OverlayType::Disk) match(!flags.readOnlyMedia(), OverlayProperty::NextOverlayFlags);
  }
  if (plan.overlayPolicy.reset)
    match(require(state.resetPersistentState, OverlayProperty::ResetPersistentState) == *target.reset, OverlayProperty::ResetPersistentState);
}
void verifyStorageTarget(const ConfigurationPlan& plan, const std::optional<OverlayMaintenanceSnapshot>& state) {
  if (!plan.maintenance.storageVolume) return;
  const auto mismatch = [] {
    throw OperationError(std::make_error_code(std::errc::state_not_recoverable),
                         "Published overlay storage differs from the reviewed target or is unavailable");
  };
  const auto target = configurationOverlayTarget(plan);
  if (!state || !state->volumesReadable || state->nextOverlayType != static_cast<std::int64_t>(target.overlay.type) ||
      state->maximumOverlaySize != target.overlay.maximumSizeMb)
    mismatch();
  bool selected = false;
  for (const auto& volume : state->volumes) {
    if (volume.id == *plan.maintenance.storageVolume) {
      if (volume.nextMarker != 1) mismatch();
      selected = true;
    } else if (volume.nextMarker && *volume.nextMarker != 0) {
      mismatch();
    }
    // Physical volumes outside the configured Next inventory have no marker.
  }
  if (!selected) mismatch();
}
bool succeeded(const ConfigurationApplyResult& result, const ConfigurationOperationKind kind) {
  return std::ranges::any_of(
      result.steps, [kind](const auto& step) { return step.operation && step.operation->kind == kind && step.outcome == ConfigurationStepOutcome::Succeeded; });
}
void projectMaintenance(OverlayPolicySnapshot& state, const OverlayMaintenanceSnapshot& receipt) {
  if (receipt.filterEnabled) state.current.filterEnabled = {PolicyObservationStatus::Available, receipt.filterEnabled};
  if (receipt.currentOverlayType) state.current.overlayType = {PolicyObservationStatus::Available, receipt.currentOverlayType};
  if (receipt.nextOverlayType) state.next.overlayType = {PolicyObservationStatus::Available, receipt.nextOverlayType};
  if (receipt.currentFlags) state.current.flags = {PolicyObservationStatus::Available, receipt.currentFlags};
  if (receipt.nextFlags) state.next.flags = {PolicyObservationStatus::Available, receipt.nextFlags};
}
void projectPhase(OverlayPolicySnapshot& state, const ConfigurationPhase& phase, const ConfigurationApplyResult& result) {
  if (result.observedOverlayPolicy) {
    state = *result.observedOverlayPolicy;
    return;
  }
  std::visit(
      [&](const auto& payload) {
        using R = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<R, ConfigurationSettingsPhase>) {
          if (payload.settings.setFilterEnabled && succeeded(result, ConfigurationOperationKind::Filter))
            state.next.filterEnabled = {PolicyObservationStatus::Available, payload.settings.setFilterEnabled};
          if (payload.settings.setOverlay.type && succeeded(result, ConfigurationOperationKind::OverlayType)) {
            const auto type = static_cast<std::int64_t>(*payload.settings.setOverlay.type);
            if (state.next.flags.status == PolicyObservationStatus::Available && state.next.flags.value) {
              auto& flags = state.next.flags.value->raw;
              const auto previous = state.next.overlayType.value ? core::overlayTypeFromValue(*state.next.overlayType.value) : core::OverlayType::Unknown;
              flags = core::overlayFlagsAfterTypeChange(previous, *payload.settings.setOverlay.type, flags);
            }
            state.next.overlayType = {PolicyObservationStatus::Available, type};
          }
        } else if constexpr (std::is_same_v<R, ConfigurationOverlayPolicyPhase>) {
          const auto option = [&](const std::optional<bool> value, const ConfigurationOperationKind kind, const std::uint32_t mask) {
            if (value && succeeded(result, kind) && state.next.flags.status == PolicyObservationStatus::Available && state.next.flags.value) {
              auto& flags = state.next.flags.value->raw;
              flags = *value ? flags | mask : flags & ~mask;
            }
          };
          option(payload.changes.passthrough, ConfigurationOperationKind::OverlayPassthrough, OverlayFlags::Passthrough);
          option(payload.changes.persistent, ConfigurationOperationKind::OverlayPersistent, OverlayFlags::Persistent);
          option(payload.changes.readOnlyMedia, ConfigurationOperationKind::OverlayReadOnlyMedia, OverlayFlags::ReadOnlyMedia);
          if (payload.changes.reset && succeeded(result, ConfigurationOperationKind::ResetPersistentState))
            state.resetPersistentState = {PolicyObservationStatus::Available, static_cast<std::int64_t>(*payload.changes.reset)};
        }
      },
      phase);
  if (result.observedOverlayMaintenance) {
    if (const auto* maintenance = std::get_if<ConfigurationMaintenancePhase>(&phase); maintenance && maintenance->changes.storageVolume) {
      state.next.overlayType = {};
      state.next.flags = {};
    }
    projectMaintenance(state, *result.observedOverlayMaintenance);
  }
}
}  // namespace

void logConfigurationException(const std::string_view context, const std::exception_ptr failure, const ConfigurationApplyResult& result) noexcept {
  if (!failure) return;
  try {
    std::rethrow_exception(failure);
  } catch (const std::exception& error) {
    const auto* operation = dynamic_cast<const OperationError*>(&error);
    const auto* policy = dynamic_cast<const OverlayPolicyError*>(&error);
    const auto* validation = dynamic_cast<const MaintenanceValidationError*>(&error);
    const bool canceled = operation && operation->writeEffect() == WriteEffect::None && operation->failureKind() == OperationFailureKind::Canceled;
    const bool unchanged = !result.anyWriteConfirmed() && !result.reconciliationRequired();
    const bool blocked =
        unchanged &&
        (validation || (policy && policy->writeEffect() == WriteEffect::None &&
                        (policy->failure().origin == OverlayPolicyFailureOrigin::Policy || policy->failure().origin == OverlayPolicyFailureOrigin::Conflict)));
    LogStream log(canceled ? 'I' : blocked ? 'W' : 'E', "apply");
    log << (canceled  ? "configuration apply canceled: "
            : blocked ? "configuration apply blocked: "
                      : "configuration apply failed: ")
        << "context=" << context << " confirmedWrite=" << result.anyWriteConfirmed() << " reconciliationRequired=" << result.reconciliationRequired()
        << " succeeded=" << result.succeededOperations() << " failed=" << result.failedOperations()
        << " dependencySkipped=" << result.dependencySkippedOperations() << " notAttempted=" << result.notAttemptedOperations()
        << " batchFailures=" << result.batchFailures() << " error=" << error.what();
    if (const auto* system = dynamic_cast<const std::system_error*>(&error))
      log << " errorCode=0x" << std::hex << std::setfill('0') << std::setw(8) << static_cast<std::uint32_t>(system->code().value()) << std::dec;
    if (policy) {
      const auto& info = policy->failure();
      log << " origin=" << static_cast<int>(info.origin) << " operation=" << info.operation << " node=" << overlayPropertyName(info.node)
          << " kind=" << static_cast<int>(info.kind) << " message=" << info.message << " diagnostics=" << info.diagnostics;
    }
    if (validation) {
      log << " validationDomain=" << validation->issue().index();
      std::visit([&](const auto issue) { log << " validationIssue=" << static_cast<int>(issue); }, validation->issue());
    }
  } catch (...) {
    UWF_LOG_E("apply") << "configuration apply failed: context=" << context << " confirmedWrite=" << result.anyWriteConfirmed()
                       << " reconciliationRequired=" << result.reconciliationRequired() << " error=non-standard-exception";
  }
}

ConfigurationApplyResult executeConfigurationPlan(const ConfigurationPlan& plan, const std::vector<PreparedConfigurationPhase>& phases,
                                                  const OverlayPolicySnapshot& published,
                                                  const std::function<ConfigurationObservations()>& publishedAfterExecution) {
  ConfigurationApplyResult result;
  bool invoked = false;
  auto failureOutcome = ConfigurationStepOutcome::AdmissionFailed;
  const bool observe = plan.requiresPolicyContext();
  auto baseline = plan.requiresPolicyContext() ? published : plan.policySnapshot;
  try {
    if (observe) {
      result.observedOverlayPolicy = baseline;
      verifyPolicy(plan, baseline);
    }
    for (const auto& prepared : phases) {
      // Each successful receipt advances the reviewed baseline for the next phase.
      invoked = true;
      failureOutcome = ConfigurationStepOutcome::ExecutionFailed;
      auto phaseResult = prepared.write(baseline);
      const bool failed = phaseResult.failed();
      if (observe) projectPhase(baseline, prepared.phase, phaseResult);
      mergeConfigurationResult(result, std::move(phaseResult));
      if (observe) {
        result.observedOverlayPolicy = baseline;
      }
      if (failed) {
        UWF_LOG_I("apply") << "configuration result: succeeded=" << result.succeededOperations() << " failed=" << result.failedOperations()
                           << " dependencySkipped=" << result.dependencySkippedOperations() << " notAttempted=" << result.notAttemptedOperations()
                           << " batchFailures=" << result.batchFailures() << " confirmedWrite=" << result.anyWriteConfirmed()
                           << " reconciliationRequired=" << result.reconciliationRequired();
        break;
      }
    }
    if (!result.failed()) {
      failureOutcome = ConfigurationStepOutcome::VerificationFailed;
      if (publishedAfterExecution) {
        auto observed = publishedAfterExecution();
        if (observe) baseline = std::move(observed.policy);
        if (!plan.maintenance.empty() || observed.maintenance) result.observedOverlayMaintenance = std::move(observed.maintenance);
      }
      if (observe) {
        result.observedOverlayPolicy = baseline;
        verifyTarget(plan, baseline);
      }
      verifyStorageTarget(plan, result.observedOverlayMaintenance);
    }
  } catch (const std::exception& error) {
    auto effect = invoked ? WriteEffect::Uncertain : WriteEffect::None;
    if (const auto* operation = dynamic_cast<const OperationError*>(&error)) effect = operation->writeEffect();
    ConfigurationStepResult failure{error.what(), failureOutcome, effect};
    if (const auto* validation = dynamic_cast<const MaintenanceValidationError*>(&error)) {
      failure.description.clear();
      failure.validationIssue = validation->issue();
    }
    if (const auto* configuration = dynamic_cast<const OverlayPolicyError*>(&error)) {
      failure.policyFailure = configuration->failure();
    }
    recordConfigurationStep(result, std::move(failure));
    logConfigurationException("execute", std::current_exception(), result);
  } catch (...) {
    const auto message = ApplicationText::tr(": unknown error").toStdString();
    recordConfigurationStep(result, {message, failureOutcome, invoked ? WriteEffect::Uncertain : WriteEffect::None});
    logConfigurationException("execute", std::current_exception(), result);
  }
  if (observe && result.observedOverlayPolicy && result.observedOverlayMaintenance) {
    const auto& policy = *result.observedOverlayPolicy;
    auto& maintenance = *result.observedOverlayMaintenance;
    maintenance.filterEnabled = policy.current.filterEnabled.value;
    maintenance.currentOverlayType = policy.current.overlayType.value;
    maintenance.nextOverlayType = policy.next.overlayType.value;
    maintenance.currentFlags = policy.current.flags.status == PolicyObservationStatus::Available ? policy.current.flags.value : std::nullopt;
    maintenance.nextFlags = policy.next.flags.status == PolicyObservationStatus::Available ? policy.next.flags.value : std::nullopt;
  }
  completeConfigurationResult(result, plan);
  return result;
}
}  // namespace uwf::app
