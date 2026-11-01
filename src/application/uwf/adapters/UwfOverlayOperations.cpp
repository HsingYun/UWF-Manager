// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/adapters/UwfOverlayOperations.h"

#include <limits>

#include "application/overlay/policy/OverlayPolicyService.h"
#include "application/uwf/adapters/UwfCall.h"
#include "foundation/logging/Log.h"

namespace uwf::app {
namespace {
OverlayPolicyFailureOrigin origin(compat::ErrorKind kind) {
  switch (kind) {
    case compat::ErrorKind::Unavailable:
    case compat::ErrorKind::Canceled:
      return OverlayPolicyFailureOrigin::Transport;
    case compat::ErrorKind::Conflict:
      return OverlayPolicyFailureOrigin::Conflict;
    case compat::ErrorKind::Protocol:
      return OverlayPolicyFailureOrigin::Protocol;
    default:
      return OverlayPolicyFailureOrigin::Provider;
  }
}

[[noreturn]] void throwFailure(const compat::Error& error, OverlayProperty node) {
  const auto& failure = error.failure();
  OverlayPolicyFailure info{.origin = origin(failure.kind),
                            .kind = uwfFailureKind(failure.kind),
                            .node = node,
                            .operation = "WriteOverlayProperty",
                            .message = failure.message,
                            .diagnostics = uwfDiagnostics(error),
                            .effect = uwfWriteEffect(failure.effect)};
  throw OverlayPolicyError(std::move(info));
}

[[noreturn]] void invalidNode(OverlayProperty node) {
  throw OverlayPolicyError({.origin = OverlayPolicyFailureOrigin::Policy,
                            .kind = OperationFailureKind::InvalidArgument,
                            .node = node,
                            .operation = "WriteOverlayProperty",
                            .message = "The overlay property does not support this operation"});
}
}  // namespace

UwfOverlayOperations::UwfOverlayOperations(std::shared_ptr<compat::Uwf> system) : m_system(std::move(system)) {
  if (!m_system) throw std::invalid_argument("UWF compatibility service must not be null");
}

OverlayPolicySnapshot UwfOverlayOperations::snapshot() const { return snapshot(m_system->state()); }
OverlayPolicySnapshot UwfOverlayOperations::snapshot(const compat::UwfStateFacts& state) {
  OverlayPolicySnapshot result;
  const auto boolean = [](const compat::Fact<bool>& source) {
    return PolicyObservation<bool>{source.value ? PolicyObservationStatus::Available : PolicyObservationStatus::Unobserved, source.value};
  };
  const auto number = [](const compat::Fact<std::uint32_t>& source) {
    return PolicyObservation<std::int64_t>{source.value ? PolicyObservationStatus::Available : PolicyObservationStatus::Unobserved,
                                           source.value ? std::optional<std::int64_t>{*source.value} : std::nullopt};
  };
  const auto copy = [&](OverlayPolicySession& target, const compat::SessionStateFacts& source) {
    target.filterEnabled = boolean(source.filterEnabled);
    target.overlayType = number(source.overlayType);
    if (source.overlayFlags.valid() && source.overlayFlags.knownMask == std::numeric_limits<std::uint32_t>::max())
      target.flags = {PolicyObservationStatus::Available, OverlayFlags{source.overlayFlags.valueBits}};
  };
  copy(result.current, state.current);
  copy(result.next, state.next);
  result.resetPersistentState = number(state.runtime.resetState);
  result.transportAvailable = !state.refreshFailure && (result.current.filterEnabled.value.has_value() || result.next.filterEnabled.value.has_value() ||
                                                        result.current.flags.value.has_value() || result.next.flags.value.has_value());
  return result;
}

WriteEffect UwfOverlayOperations::setOption(const OverlayOption option, const bool enabled) {
  compat::OverlayOption target;
  switch (option) {
    case OverlayOption::Passthrough:
      target = compat::OverlayOption::Passthrough;
      break;
    case OverlayOption::Persistent:
      target = compat::OverlayOption::Persistent;
      break;
    case OverlayOption::ReadOnlyMedia:
      target = compat::OverlayOption::ReadOnlyMedia;
      break;
    default:
      invalidNode(OverlayProperty::NextOverlayFlags);
  }
  return write(compat::SetOverlayOption{target, enabled}, OverlayProperty::NextOverlayFlags);
}

WriteEffect UwfOverlayOperations::replace(OverlayProperty node, std::int64_t value) {
  if (value < 0 || value > std::numeric_limits<std::uint32_t>::max())
    throw OverlayPolicyError({.origin = OverlayPolicyFailureOrigin::Policy,
                              .kind = OperationFailureKind::InvalidArgument,
                              .node = node,
                              .operation = "WriteOverlayProperty",
                              .message = "The overlay property value is outside its unsigned 32-bit domain"});
  std::optional<compat::NumberProperty> property;
  switch (node) {
    case OverlayProperty::ResetPersistentState:
      property = compat::NumberProperty::ResetPersistentState;
      break;
    default:
      invalidNode(node);
  }
  return write(compat::SetNumber{*property, static_cast<std::uint32_t>(value)}, node);
}

WriteEffect UwfOverlayOperations::write(const compat::Request& request, const OverlayProperty node) {
  try {
    auto response = compat::normalizeResponse(request, m_system->execute(request));
    for (const auto& warning : response.warnings) UWF_LOG_W("uwf-overlay-policy") << warning;
    if (response.failure) throw compat::Error(std::move(*response.failure), std::move(response.attempts));
    return uwfWriteEffect(response.effect);
  } catch (const compat::Error& error) {
    throwFailure(error, node);
  }
}

std::shared_ptr<OverlayPolicyActions> uwfOverlayPolicy(std::shared_ptr<compat::Uwf> system) {
  return std::make_shared<OverlayPolicyService>(std::make_shared<UwfOverlayOperations>(std::move(system)));
}
}  // namespace uwf::app
