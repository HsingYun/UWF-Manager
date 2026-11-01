// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <system_error>
#include <utility>

namespace uwf::app {
enum class WriteEffect { None, Confirmed, Uncertain };
enum class OperationFailureKind {
  Other,
  Failed,
  Unsupported,
  Unavailable,
  AccessDenied,
  NotFound,
  InvalidArgument,
  Canceled,
  ResourceExhausted,
  Conflict,
  Protocol
};
struct OperationDiagnostic {
  int32_t code = 0;
  uint32_t result = 0;
};

// Failure and side effects are independent facts. Composite operations must
// report effects from earlier steps even when the failing step was a read.
class OperationError : public std::system_error {
 public:
  OperationError(std::error_code code, std::string detail, WriteEffect effect = WriteEffect::None)
      : std::system_error(code, std::move(detail)), m_effect(effect) {}
  // Override the aggregate effect without losing provider diagnostics or
  // mutating the original exception shared with another observer.
  OperationError(const OperationError& cause, WriteEffect effect)
      : std::system_error(cause), m_effect(effect), m_preservedFailure(PreservedFailure{cause.what(), cause.failureKind(), cause.diagnostic()}) {}
  [[nodiscard]] const char* what() const noexcept override { return m_preservedFailure ? m_preservedFailure->message.c_str() : std::system_error::what(); }
  [[nodiscard]] WriteEffect writeEffect() const noexcept { return m_effect; }
  [[nodiscard]] virtual OperationFailureKind failureKind() const noexcept {
    if (m_preservedFailure) return m_preservedFailure->kind;
    if (code() == std::errc::no_such_file_or_directory) return OperationFailureKind::NotFound;
    if (code() == std::errc::invalid_argument) return OperationFailureKind::InvalidArgument;
    if (code() == std::errc::operation_canceled) return OperationFailureKind::Canceled;
    if (code() == std::errc::operation_not_supported) return OperationFailureKind::Unsupported;
    if (code() == std::errc::no_such_device || code() == std::errc::not_connected) return OperationFailureKind::Unavailable;
    if (code() == std::errc::permission_denied) return OperationFailureKind::AccessDenied;
    if (code() == std::errc::not_enough_memory || code() == std::errc::no_space_on_device) return OperationFailureKind::ResourceExhausted;
    if (code() == std::errc::device_or_resource_busy) return OperationFailureKind::Conflict;
    if (code() == std::errc::protocol_error) return OperationFailureKind::Protocol;
    return OperationFailureKind::Other;
  }
  [[nodiscard]] virtual OperationDiagnostic diagnostic() const noexcept { return m_preservedFailure ? m_preservedFailure->diagnostic : OperationDiagnostic{}; }

 private:
  struct PreservedFailure {
    std::string message;
    OperationFailureKind kind;
    OperationDiagnostic diagnostic;
  };
  WriteEffect m_effect;
  std::optional<PreservedFailure> m_preservedFailure;
};

// Value form used by batch results; presentation text never replaces the
// provider's typed failure, submitted-write effect or native diagnostic.
struct OperationFailure {
  OperationFailureKind kind = OperationFailureKind::Failed;
  WriteEffect effect = WriteEffect::None;
  OperationDiagnostic diagnostic{};
};
[[nodiscard]] inline OperationFailure operationFailure(const std::exception& error, WriteEffect unknownEffect = WriteEffect::Uncertain) {
  if (const auto* operation = dynamic_cast<const OperationError*>(&error)) return {operation->failureKind(), operation->writeEffect(), operation->diagnostic()};
  return {OperationFailureKind::Failed, unknownEffect, {}};
}
[[nodiscard]] constexpr WriteEffect combineWriteEffects(WriteEffect a, WriteEffect b) noexcept {
  if (a == WriteEffect::Uncertain || b == WriteEffect::Uncertain) return WriteEffect::Uncertain;
  if (a == WriteEffect::Confirmed || b == WriteEffect::Confirmed) return WriteEffect::Confirmed;
  return WriteEffect::None;
}
}  // namespace uwf::app
