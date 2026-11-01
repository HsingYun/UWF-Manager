// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/overlay/policy/OverlayPolicyRepository.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "application/configuration/execution/ConfigurationResult.h"
#include "application/localization/ApplicationText.h"
#include "application/overlay/contracts/OverlayPolicyActions.h"

namespace uwf::app {
namespace {
ConfigurationApplyResult applyOverlayPolicy(OverlayPolicyActions& policy, const OverlayPolicyDelta& changes, const OverlayPolicySnapshot& baseline) {
  OverlayPolicyUpdate update{.changes = changes};
  const auto require = [](const auto& field) {
    if (field.status == PolicyObservationStatus::Available && field.value) return *field.value;
    if (field.failure) throw OverlayPolicyError(*field.failure);
    throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "Required overlay observation is unavailable");
  };
  if (changes.passthrough || changes.persistent || changes.readOnlyMedia) update.expectedFlags = require(baseline.next.flags).raw;
  if (changes.reset) update.expectedReset = require(baseline.resetPersistentState);
  auto value = policy.apply(update);
  ConfigurationApplyResult result;
  result.observedOverlayPolicy = value.snapshot;
  bool operationFailed = false;
  const auto add = [&](ConfigurationOperationKind kind, OverlayProperty property, std::optional<OverlayOption> option = {}) {
    const auto found = std::ranges::find_if(value.steps, [&](const auto& step) { return step.node == property && step.option == option; });
    const bool success = found != value.steps.end() && (found->outcome == PolicyStepOutcome::Confirmed || found->outcome == PolicyStepOutcome::Unchanged);
    const bool skipped = found == value.steps.end() || found->outcome == PolicyStepOutcome::NotAttempted;
    const auto outcome = success ? ConfigurationStepOutcome::Succeeded : skipped ? ConfigurationStepOutcome::NotAttempted : ConfigurationStepOutcome::Failed;
    const auto effect = found == value.steps.end()                       ? WriteEffect::None
                        : found->outcome == PolicyStepOutcome::Confirmed ? WriteEffect::Confirmed
                        : found->outcome == PolicyStepOutcome::Uncertain ? WriteEffect::Uncertain
                                                                         : WriteEffect::None;
    const auto name = option == OverlayOption::Passthrough     ? ApplicationText::tr("Freespace Passthrough")
                      : option == OverlayOption::Persistent    ? ApplicationText::tr("Persistent Overlay")
                      : option == OverlayOption::ReadOnlyMedia ? ApplicationText::tr("Read-Only Media mode")
                                                               : ApplicationText::tr("Clear persistent data on next startup");
    const auto status = success ? ApplicationText::tr("Succeeded") : skipped ? ApplicationText::tr("Not applied") : ApplicationText::tr("Failed");
    ConfigurationStepResult receipt{(name + ": " + status).toStdString(), outcome, effect, ConfigurationOperation{kind, {}}};
    if (!success && !skipped && value.failure) {
      operationFailed = true;
      receipt.policyFailure = value.failure;
      receipt.description += ": " + value.failure->message;
    }
    recordConfigurationStep(result, std::move(receipt));
  };
  using K = ConfigurationOperationKind;
  if (changes.passthrough) add(K::OverlayPassthrough, OverlayProperty::NextOverlayFlags, OverlayOption::Passthrough);
  if (changes.persistent) add(K::OverlayPersistent, OverlayProperty::NextOverlayFlags, OverlayOption::Persistent);
  if (changes.readOnlyMedia) add(K::OverlayReadOnlyMedia, OverlayProperty::NextOverlayFlags, OverlayOption::ReadOnlyMedia);
  if (changes.reset) add(K::ResetPersistentState, OverlayProperty::ResetPersistentState);
  if (value.failure && !operationFailed) {
    const bool admitted = !value.steps.empty();
    ConfigurationStepResult failure{value.failure->operation + ": " + value.failure->message,
                                    admitted ? ConfigurationStepOutcome::VerificationFailed : ConfigurationStepOutcome::AdmissionFailed,
                                    admitted ? WriteEffect::None : value.effect};
    failure.policyFailure = value.failure;
    recordConfigurationStep(result, std::move(failure));
  }
  return result;
}

class OverlayPolicyRepository final : public ConfigurationRepository {
 public:
  explicit OverlayPolicyRepository(std::shared_ptr<OverlayPolicyActions> policy) : m_policy(std::move(policy)) {}
  bool supports(ConfigurationArea area) const override { return area == ConfigurationArea::OverlayPolicy; }
  OverlayPolicySnapshot snapshot() const override { return m_policy->snapshot(); }
  WriteOperation prepareWrite(const ConfigurationPhase& phase, const ConfigurationPlan&) override {
    const auto* payload = std::get_if<ConfigurationOverlayPolicyPhase>(&phase);
    if (!payload) throw std::invalid_argument("Overlay policy repository requires a policy phase");
    return [policy = m_policy, changes = payload->changes](const OverlayPolicySnapshot& baseline) { return applyOverlayPolicy(*policy, changes, baseline); };
  }

 private:
  std::shared_ptr<OverlayPolicyActions> m_policy;
};
}  // namespace
std::shared_ptr<ConfigurationRepository> overlayPolicyRepository(std::shared_ptr<OverlayPolicyActions> policy) {
  if (!policy) throw std::invalid_argument("Overlay policy actions must not be null");
  return std::make_shared<OverlayPolicyRepository>(std::move(policy));
}
}  // namespace uwf::app
