// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "application/operations/contracts/OperationError.h"

namespace uwf::app {

// Semantic overlay properties. Backend diagnostics are opaque to application decisions.
enum class OverlayProperty {
  CurrentFilterEnabled,
  CurrentOverlayType,
  CurrentOverlayFlags,
  NextFilterEnabled,
  NextOverlayType,
  NextOverlayFlags,
  ResetPersistentState
};

[[nodiscard]] constexpr std::string_view overlayPropertyName(OverlayProperty node) {
  switch (node) {
    case OverlayProperty::CurrentFilterEnabled:
      return "CurrentSession/FilterEnabled";
    case OverlayProperty::CurrentOverlayType:
      return "CurrentSession/OverlayType";
    case OverlayProperty::CurrentOverlayFlags:
      return "CurrentSession/OverlayFlags";
    case OverlayProperty::NextFilterEnabled:
      return "NextSession/FilterEnabled";
    case OverlayProperty::NextOverlayType:
      return "NextSession/OverlayType";
    case OverlayProperty::NextOverlayFlags:
      return "NextSession/OverlayFlags";
    case OverlayProperty::ResetPersistentState:
      return "NextSession/ResetPersistentState";
  }
  return "Unknown";
}

enum class OverlayPolicyFailureOrigin { Transport, Provider, Protocol, Policy, Conflict };

struct OverlayPolicyFailure {
  OverlayPolicyFailureOrigin origin = OverlayPolicyFailureOrigin::Protocol;
  OperationFailureKind kind = OperationFailureKind::Protocol;
  OverlayProperty node = OverlayProperty::NextOverlayFlags;
  std::string operation{};
  std::string message{};
  // Backend names, native codes and response details are retained for display
  // and logging only; application decisions use kind and origin.
  std::string diagnostics{};
  WriteEffect effect = WriteEffect::None;
};

class OverlayPolicyError final : public OperationError {
 public:
  explicit OverlayPolicyError(OverlayPolicyFailure failure)
      : OperationError(errorCode(failure.kind), failure.message, failure.effect), m_failure(std::move(failure)) {}
  [[nodiscard]] const OverlayPolicyFailure& failure() const noexcept { return m_failure; }
  [[nodiscard]] OperationFailureKind failureKind() const noexcept override { return m_failure.kind; }

 private:
  static std::error_code errorCode(const OperationFailureKind kind) {
    switch (kind) {
      case OperationFailureKind::Unsupported:
        return std::make_error_code(std::errc::operation_not_supported);
      case OperationFailureKind::Unavailable:
        return std::make_error_code(std::errc::no_such_device);
      case OperationFailureKind::AccessDenied:
        return std::make_error_code(std::errc::permission_denied);
      case OperationFailureKind::NotFound:
        return std::make_error_code(std::errc::no_such_file_or_directory);
      case OperationFailureKind::InvalidArgument:
        return std::make_error_code(std::errc::invalid_argument);
      case OperationFailureKind::Canceled:
        return std::make_error_code(std::errc::operation_canceled);
      case OperationFailureKind::ResourceExhausted:
        return std::make_error_code(std::errc::not_enough_memory);
      case OperationFailureKind::Conflict:
        return std::make_error_code(std::errc::state_not_recoverable);
      case OperationFailureKind::Protocol:
        return std::make_error_code(std::errc::protocol_error);
      default:
        return std::make_error_code(std::errc::io_error);
    }
  }
  OverlayPolicyFailure m_failure;
};

}  // namespace uwf::app
