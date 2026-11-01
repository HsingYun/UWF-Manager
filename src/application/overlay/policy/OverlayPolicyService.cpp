// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/overlay/policy/OverlayPolicyService.h"

#include <stdexcept>
#include <string_view>
#include <utility>

#include "foundation/logging/Log.h"

namespace uwf::app {

namespace {
OverlayPolicyFailure failure(OverlayPolicyFailureOrigin origin, OverlayProperty node, std::string message,
                             OperationFailureKind kind = OperationFailureKind::Protocol) {
  if (origin == OverlayPolicyFailureOrigin::Policy && kind == OperationFailureKind::Protocol) kind = OperationFailureKind::InvalidArgument;
  if (origin == OverlayPolicyFailureOrigin::Conflict) kind = OperationFailureKind::Conflict;
  if (origin == OverlayPolicyFailureOrigin::Transport) kind = OperationFailureKind::Unavailable;
  return {.origin = origin, .kind = kind, .node = node, .operation = "ApplyOverlayPolicy", .message = std::move(message)};
}

template <typename T>
const T& require(const PolicyObservation<T>& observation, OverlayProperty node) {
  if (observation.status == PolicyObservationStatus::Available && observation.value) return *observation.value;
  if (observation.failure) throw OverlayPolicyError(*observation.failure);
  throw OverlayPolicyError(failure(OverlayPolicyFailureOrigin::Policy, node, "Required UWF observation is unavailable"));
}

void recordStep(OverlayPolicySnapshot& snapshot, const OverlayPolicyStep& step) {
  if (step.option) {
    const auto mask = *step.option == OverlayOption::Passthrough  ? OverlayFlags::Passthrough
                      : *step.option == OverlayOption::Persistent ? OverlayFlags::Persistent
                                                                  : OverlayFlags::ReadOnlyMedia;
    auto& flags = snapshot.next.flags;
    flags.value->raw = step.requested != 0 ? flags.value->raw | mask : flags.value->raw & ~mask;
  } else {
    snapshot.resetPersistentState = {.status = PolicyObservationStatus::Available, .value = step.requested};
  }
}

std::string_view effectName(const WriteEffect effect) {
  switch (effect) {
    case WriteEffect::None:
      return "none";
    case WriteEffect::Confirmed:
      return "confirmed";
    case WriteEffect::Uncertain:
      return "uncertain";
  }
  return "unknown";
}

std::string_view outcomeName(const PolicyStepOutcome outcome) {
  switch (outcome) {
    case PolicyStepOutcome::NotAttempted:
      return "not-attempted";
    case PolicyStepOutcome::Unchanged:
      return "unchanged";
    case PolicyStepOutcome::Confirmed:
      return "confirmed";
    case PolicyStepOutcome::Failed:
      return "failed";
    case PolicyStepOutcome::Uncertain:
      return "uncertain";
  }
  return "unknown";
}

std::string_view originName(const OverlayPolicyFailureOrigin origin) {
  switch (origin) {
    case OverlayPolicyFailureOrigin::Transport:
      return "transport";
    case OverlayPolicyFailureOrigin::Provider:
      return "provider";
    case OverlayPolicyFailureOrigin::Protocol:
      return "protocol";
    case OverlayPolicyFailureOrigin::Policy:
      return "policy";
    case OverlayPolicyFailureOrigin::Conflict:
      return "conflict";
  }
  return "unknown";
}

void logFailure(const OverlayPolicyResult& result, const bool writeAttempted) {
  const auto& info = *result.failure;
  const bool blocked = !writeAttempted && result.effect == WriteEffect::None &&
                       (info.origin == OverlayPolicyFailureOrigin::Policy || info.origin == OverlayPolicyFailureOrigin::Conflict);
  LogStream log(blocked ? 'W' : 'E', "uwf-overlay-policy");
  log << (blocked ? "apply blocked: " : "apply failed: ") << "origin=" << originName(info.origin) << " operation=" << info.operation
      << " node=" << overlayPropertyName(info.node) << " writeAttempted=" << writeAttempted << " effect=" << effectName(result.effect)
      << " kind=" << static_cast<int>(info.kind) << " message=" << info.message << " diagnostics=" << info.diagnostics;
  for (const auto& step : result.steps) {
    log << " step[node=" << overlayPropertyName(step.node);
    if (step.option) log << " option=" << static_cast<int>(*step.option);
    log << " before=" << step.before << " requested=" << step.requested << " observed=";
    if (step.observed)
      log << *step.observed;
    else
      log << "unavailable";
    log << " outcome=" << outcomeName(step.outcome) << ']';
  }
}
}  // namespace

OverlayPolicyService::OverlayPolicyService(std::shared_ptr<OverlayPolicyOperations> operations) : m_operations(std::move(operations)) {
  if (!m_operations) throw std::invalid_argument("UWF operations must not be null");
}

OverlayPolicySnapshot OverlayPolicyService::snapshot() const {
  const std::lock_guard lock(m_mutex);
  return m_operations->snapshot();
}

OverlayPolicyResult OverlayPolicyService::apply(const OverlayPolicyUpdate& update) {
  const std::lock_guard lock(m_mutex);
  OverlayPolicyResult result;
  bool writeAttempted = false;
  result.snapshot = m_operations->snapshot();
  const auto& delta = update.changes;
  try {
    if (delta.reset == PersistentReset::UseSavedMode)
      throw OverlayPolicyError(failure(OverlayPolicyFailureOrigin::Policy, OverlayProperty::ResetPersistentState,
                                       "Reset selector 255 is read-only in this application", OperationFailureKind::Unsupported));
    const auto checkBaseline = [](OverlayProperty node, std::int64_t actual, std::optional<std::int64_t> expected) {
      if (expected && actual != *expected)
        throw OverlayPolicyError(failure(OverlayPolicyFailureOrigin::Conflict, node, "UWF value changed since the supplied baseline"));
    };
    const auto plan = [&](OverlayProperty node, std::int64_t before, std::int64_t requested, std::optional<std::int64_t> expected) {
      checkBaseline(node, before, expected);
      result.steps.push_back({.node = node, .before = before, .requested = requested});
    };
    if (delta.passthrough || delta.persistent || delta.readOnlyMedia) {
      const auto before = require(result.snapshot.next.flags, OverlayProperty::NextOverlayFlags);
      checkBaseline(OverlayProperty::NextOverlayFlags, before.raw, update.expectedFlags);
      const bool mediaChanged = delta.readOnlyMedia && *delta.readOnlyMedia != before.readOnlyMedia();
      if ((delta.passthrough.has_value() || delta.persistent.has_value() || mediaChanged) &&
          require(result.snapshot.current.filterEnabled, OverlayProperty::CurrentFilterEnabled))
        throw OverlayPolicyError(failure(OverlayPolicyFailureOrigin::Policy, OverlayProperty::NextOverlayFlags,
                                         "Changing overlay flags requires current-session UWF disabled", OperationFailureKind::InvalidArgument));
      if (delta.persistent == true && require(result.snapshot.next.overlayType, OverlayProperty::NextOverlayType) != 1)
        throw OverlayPolicyError(failure(OverlayPolicyFailureOrigin::Policy, OverlayProperty::NextOverlayFlags,
                                         "Enabling Persistent requires next-session Disk overlay", OperationFailureKind::InvalidArgument));
      if (mediaChanged && delta.readOnlyMedia == true) {
        if (require(result.snapshot.next.overlayType, OverlayProperty::NextOverlayType) != 0)
          throw OverlayPolicyError(failure(OverlayPolicyFailureOrigin::Policy, OverlayProperty::NextOverlayFlags,
                                           "Enabling Read-Only Media requires next-session RAM overlay", OperationFailureKind::InvalidArgument));
      }
      const auto planOption = [&](const OverlayOption option, const std::optional<bool> requested, const bool original) {
        if (requested) result.steps.push_back({.node = OverlayProperty::NextOverlayFlags, .before = original, .requested = *requested, .option = option});
      };
      planOption(OverlayOption::Passthrough, delta.passthrough, before.passthrough());
      planOption(OverlayOption::Persistent, delta.persistent, before.persistent());
      planOption(OverlayOption::ReadOnlyMedia, delta.readOnlyMedia, before.readOnlyMedia());
    }
    if (delta.reset) {
      const auto value = static_cast<std::int64_t>(*delta.reset);
      if (value != 0 && value != 1)
        throw OverlayPolicyError(failure(OverlayPolicyFailureOrigin::Policy, OverlayProperty::ResetPersistentState, "Invalid reset selector",
                                         OperationFailureKind::InvalidArgument));
      plan(OverlayProperty::ResetPersistentState, require(result.snapshot.resetPersistentState, OverlayProperty::ResetPersistentState), value,
           update.expectedReset);
    }
  } catch (const OverlayPolicyError& error) {
    result.failure = error.failure();
    result.steps.clear();  // All validation precedes writes.
    logFailure(result, false);
    return result;
  }

  for (auto& step : result.steps) {
    if (!step.option && step.before == step.requested) {
      step.observed = step.before;
      step.outcome = PolicyStepOutcome::Unchanged;
      continue;
    }
    try {
      writeAttempted = true;
      WriteEffect writeEffect;
      if (step.option)
        writeEffect = m_operations->setOption(*step.option, step.requested != 0);
      else
        writeEffect = m_operations->replace(step.node, step.requested);
      if (writeEffect == WriteEffect::Uncertain)
        throw OverlayPolicyError({.origin = OverlayPolicyFailureOrigin::Protocol,
                                  .kind = OperationFailureKind::Failed,
                                  .node = step.node,
                                  .operation = "ApplyOverlayPolicy",
                                  .message = "The operations contract returned an uncertain success",
                                  .effect = WriteEffect::Uncertain});
      step.observed = step.requested;
      step.outcome = writeEffect == WriteEffect::None ? PolicyStepOutcome::Unchanged : PolicyStepOutcome::Confirmed;
      result.effect = combineWriteEffects(result.effect, writeEffect);
      recordStep(result.snapshot, step);
    } catch (const OverlayPolicyError& error) {
      result.failure = error.failure();
    } catch (const std::exception& error) {
      result.failure = failure(OverlayPolicyFailureOrigin::Transport, step.node, error.what());
      result.failure->effect = WriteEffect::Uncertain;
    }
    if (result.failure) {
      step.outcome = result.failure->effect == WriteEffect::None ? PolicyStepOutcome::Failed : PolicyStepOutcome::Uncertain;
      if (result.failure->effect != WriteEffect::None) {
        result.effect = WriteEffect::Uncertain;
        if (step.option)
          result.snapshot.next.flags = {.status = PolicyObservationStatus::Failed, .failure = result.failure};
        else
          result.snapshot.resetPersistentState = {.status = PolicyObservationStatus::Failed, .failure = result.failure};
      }
      result.failure->effect = result.effect;
      break;
    }
    UWF_LOG_I("uwf-overlay-policy") << "command confirmed: node=" << overlayPropertyName(step.node) << " before=" << step.before
                                    << " requested=" << step.requested << " observed=" << *step.observed;
  }

  if (result.failure) logFailure(result, writeAttempted);
  return result;
}

}  // namespace uwf::app
