// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/overlay/OverlayPolicyConstraints.h"

#include <utility>

#include "application/configuration/contracts/ConfigurationPlan.h"

namespace uwf::app {
namespace {
template <typename T>
bool known(const PolicyObservation<T>& field) {
  return field.status == PolicyObservationStatus::Available && field.value.has_value();
}
OverlayPolicyConflict policyConflict(const OverlayPolicyIssue issue, const OverlayProperty node, const char* message) {
  return {issue,
          {.origin = OverlayPolicyFailureOrigin::Policy,
           .kind = OperationFailureKind::InvalidArgument,
           .node = node,
           .operation = "PlanConfiguration",
           .message = message}};
}
template <typename T>
OverlayPolicyConflict unavailable(const PolicyObservation<T>& field, const OverlayProperty node) {
  if (field.failure) return {OverlayPolicyIssue::FlagsUnavailable, *field.failure};
  return policyConflict(OverlayPolicyIssue::FlagsUnavailable, node, "Required overlay baseline has not been observed");
}
std::optional<OverlayPolicyConflict> currentFilterOffConflict(const OverlayPolicySnapshot& baseline) {
  if (!known(baseline.current.filterEnabled)) return unavailable(baseline.current.filterEnabled, OverlayProperty::CurrentFilterEnabled);
  if (*baseline.current.filterEnabled.value)
    return policyConflict(OverlayPolicyIssue::CurrentFilterMustBeOff, OverlayProperty::NextOverlayFlags,
                          "Changing overlay flags requires current-session UWF disabled; a next-session disable is insufficient");
  return {};
}
std::optional<OverlayPolicyConflict> resetCurrentConflict(const OverlayPolicySnapshot& baseline) {
  if (!known(baseline.current.filterEnabled)) return unavailable(baseline.current.filterEnabled, OverlayProperty::CurrentFilterEnabled);
  if (!known(baseline.current.overlayType)) return unavailable(baseline.current.overlayType, OverlayProperty::CurrentOverlayType);
  if (!known(baseline.current.flags)) return unavailable(baseline.current.flags, OverlayProperty::CurrentOverlayFlags);
  if (!*baseline.current.filterEnabled.value || baseline.currentPersistentDisk() != true)
    return policyConflict(OverlayPolicyIssue::CurrentPersistentRequired, OverlayProperty::ResetPersistentState,
                          "Changing the persistent reset request requires UWF enabled with a current persistent Disk overlay");
  return {};
}
OverlayPolicyDelta targetValues(const OverlayPolicySnapshot& baseline, const OverlayPolicyDelta& changes, const std::optional<OverlayFlags> flags) {
  auto values = overlayPolicyValues(baseline, changes);
  if (flags) {
    values.passthrough = flags->passthrough();
    values.persistent = flags->persistent();
    values.readOnlyMedia = flags->readOnlyMedia();
  }
  return values;
}
OverlayPolicyValidation targetValidation(const OverlayPolicySnapshot& baseline, const core::OverlayType type, const OverlayPolicyDelta& changes,
                                        const std::optional<OverlayFlags> flags = std::nullopt, const bool structuralChange = false) {
  OverlayPolicyValidation result;
  if (structuralChange || overlayOptionsRequireCurrentFilterOff(baseline, changes))
    if (auto conflict = currentFilterOffConflict(baseline)) result.conflicts.push_back(std::move(*conflict));
  if (changes.persistent == true && type != core::OverlayType::Disk)
    result.conflicts.push_back(
        policyConflict(OverlayPolicyIssue::PersistentNeedsDisk, OverlayProperty::NextOverlayFlags, "Explicitly enabling Persistent requires a Disk overlay"));
  const auto values = targetValues(baseline, changes, flags);
  if (values.readOnlyMedia == true && type != core::OverlayType::RAM)
    result.conflicts.push_back(policyConflict(OverlayPolicyIssue::MediaNeedsRam, OverlayProperty::NextOverlayFlags,
                                              "Read-Only Media requires RAM; explicitly disable it before targeting Disk"));
  return result;
}
OverlayPolicyValidation draftValidation(const OverlayPolicySnapshot& baseline, const core::OverlayType type, const OverlayPolicyDelta& changes,
                                       const std::optional<OverlayFlags> flags = std::nullopt, const bool structuralChange = false) {
  auto result = targetValidation(baseline, type, changes, flags, structuralChange);
  const auto values = targetValues(baseline, changes, flags);
  const bool resetChanged = changes.reset && changes.reset != baseline.resetMode();
  const auto resetConflict = resetChanged ? resetCurrentConflict(baseline) : std::nullopt;
  if (resetConflict) result.conflicts.push_back(*resetConflict);
  if (values.reset == PersistentReset::Clear) {
    if (baseline.currentPersistentDisk() != true) {
      if (!resetConflict)
        result.conflicts.push_back(policyConflict(OverlayPolicyIssue::CurrentPersistentRequired, OverlayProperty::ResetPersistentState,
                                                  "Changing the persistent reset request requires UWF enabled with a current persistent Disk overlay"));
    } else if (type != core::OverlayType::Disk || values.persistent != true)
      result.conflicts.push_back(policyConflict(OverlayPolicyIssue::NextPersistentRequired, OverlayProperty::ResetPersistentState,
                                                "A scheduled clear requires next-session persistent Disk overlay; explicitly cancel the clear first"));
  }
  return result;
}
}  // namespace

void OverlayPolicyValidation::requireValid() const {
  if (!valid()) throw OverlayPolicyError(conflicts.front().failure);
}

OverlayPolicyDelta overlayPolicyValues(const OverlayPolicySnapshot& baseline, const OverlayPolicyDelta& delta) {
  OverlayPolicyDelta values;
  if (baseline.next.flags.status == PolicyObservationStatus::Available && baseline.next.flags.value) {
    values.passthrough = baseline.next.flags.value->passthrough();
    values.persistent = baseline.next.flags.value->persistent();
    values.readOnlyMedia = baseline.next.flags.value->readOnlyMedia();
  }
  if (baseline.resetPersistentState.status == PolicyObservationStatus::Available) {
    const auto mode = baseline.resetMode();
    if (mode == PersistentReset::Clear || mode == PersistentReset::Preserve) values.reset = mode;
  }
  if (delta.passthrough) values.passthrough = delta.passthrough;
  if (delta.persistent) values.persistent = delta.persistent;
  if (delta.readOnlyMedia) values.readOnlyMedia = delta.readOnlyMedia;
  if (delta.reset) values.reset = delta.reset;
  return values;
}
OverlayPolicyDelta overlayPolicyChanges(const OverlayPolicySnapshot& baseline, const OverlayPolicyDelta& values) {
  auto changes = values;
  const auto original = overlayPolicyValues(baseline);
  if (changes.passthrough == original.passthrough) changes.passthrough.reset();
  if (changes.persistent == original.persistent) changes.persistent.reset();
  if (changes.readOnlyMedia == original.readOnlyMedia) changes.readOnlyMedia.reset();
  if (changes.reset == original.reset) changes.reset.reset();
  return changes;
}
bool overlayOptionsRequireCurrentFilterOff(const OverlayPolicySnapshot& baseline, const OverlayPolicyDelta& values) {
  const auto flags = baseline.next.flags.status == PolicyObservationStatus::Available ? baseline.next.flags.value : std::nullopt;
  const bool mediaChanged = values.readOnlyMedia && (!flags || *values.readOnlyMedia != flags->readOnlyMedia());
  return values.passthrough.has_value() || values.persistent.has_value() || mediaChanged;
}
OverlayPolicyIssue overlayCurrentFilterOffIssue(const OverlayPolicySnapshot& baseline) {
  const auto conflict = currentFilterOffConflict(baseline);
  return conflict ? conflict->issue : OverlayPolicyIssue::None;
}
OverlayPolicyIssue overlayResetCurrentIssue(const OverlayPolicySnapshot& baseline) {
  const auto conflict = resetCurrentConflict(baseline);
  return conflict ? conflict->issue : OverlayPolicyIssue::None;
}
OverlayPolicyValidation validateOverlayPolicyPlan(const ConfigurationPlan& plan) {
  if (!plan.hasPolicyContext && plan.overlayPolicy.empty() && !plan.maintenance.storageVolume) return {};
  const auto target = configurationOverlayTarget(plan);
  return draftValidation(configurationOverlayBaseline(plan), target.overlay.type, plan.overlayPolicy, target.flags,
                         plan.changes.setOverlay.touchesTypeOrSize() || plan.maintenance.storageVolume.has_value());
}
}  // namespace uwf::app
