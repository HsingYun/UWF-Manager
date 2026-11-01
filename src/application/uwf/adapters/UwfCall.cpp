// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/adapters/UwfCall.h"

#include <algorithm>
#include <bit>
#include <format>
#include <utility>

#include "application/operations/contracts/OperationError.h"
#include "foundation/logging/Log.h"

namespace uwf::app {
WriteEffect uwfWriteEffect(const compat::Effect effect) {
  switch (effect) {
    case compat::Effect::None:
      return WriteEffect::None;
    case compat::Effect::Confirmed:
      return WriteEffect::Confirmed;
    case compat::Effect::Uncertain:
      return WriteEffect::Uncertain;
  }
  return WriteEffect::Uncertain;
}

OperationFailureKind uwfFailureKind(const compat::ErrorKind kind) {
  switch (kind) {
    case compat::ErrorKind::NotFound:
      return OperationFailureKind::NotFound;
    case compat::ErrorKind::InvalidArgument:
      return OperationFailureKind::InvalidArgument;
    case compat::ErrorKind::Canceled:
      return OperationFailureKind::Canceled;
    case compat::ErrorKind::ResourceExhausted:
      return OperationFailureKind::ResourceExhausted;
    case compat::ErrorKind::Unsupported:
      return OperationFailureKind::Unsupported;
    case compat::ErrorKind::Unavailable:
      return OperationFailureKind::Unavailable;
    case compat::ErrorKind::AccessDenied:
      return OperationFailureKind::AccessDenied;
    case compat::ErrorKind::Conflict:
      return OperationFailureKind::Conflict;
    case compat::ErrorKind::Protocol:
      return OperationFailureKind::Protocol;
    default:
      return OperationFailureKind::Failed;
  }
}

std::string uwfDiagnostics(const compat::Error& error) {
  std::vector<std::string> entries;
  const auto append = [&](const std::string& line) {
    if (!line.empty() && std::ranges::find(entries, line) == entries.end()) entries.push_back(line);
  };
  const auto appendFailure = [&](const compat::Failure& failure) {
    append(failure.message);
    for (const auto& diagnostic : failure.diagnostics) {
      if (!diagnostic.code && diagnostic.detail == failure.message) continue;
      append(std::format("{}: {} (code=0x{:X}) {}", diagnostic.backend, diagnostic.operation, static_cast<std::uint32_t>(diagnostic.code), diagnostic.detail));
    }
  };
  append(error.failure().message);
  for (const auto& attempt : error.attempts()) {
    if (attempt.capability.support == compat::Support::Unsupported || attempt.capability.support == compat::Support::Unavailable)
      append(attempt.backend + ": " + attempt.capability.reason);
    if (attempt.failure) {
      appendFailure(*attempt.failure);
    }
  }
  appendFailure(error.failure());
  std::string result;
  for (const auto& entry : entries) {
    if (!result.empty()) result += '\n';
    result += entry;
  }
  return result;
}

namespace {
std::error_code errorCode(const compat::ErrorKind kind) {
  switch (kind) {
    case compat::ErrorKind::Unsupported:
      return std::make_error_code(std::errc::operation_not_supported);
    case compat::ErrorKind::Unavailable:
      return std::make_error_code(std::errc::no_such_device);
    case compat::ErrorKind::AccessDenied:
      return std::make_error_code(std::errc::permission_denied);
    case compat::ErrorKind::NotFound:
      return std::make_error_code(std::errc::no_such_file_or_directory);
    case compat::ErrorKind::InvalidArgument:
      return std::make_error_code(std::errc::invalid_argument);
    case compat::ErrorKind::Canceled:
      return std::make_error_code(std::errc::operation_canceled);
    case compat::ErrorKind::ResourceExhausted:
      return std::make_error_code(std::errc::not_enough_memory);
    case compat::ErrorKind::Conflict:
      return std::make_error_code(std::errc::device_or_resource_busy);
    case compat::ErrorKind::Protocol:
      return std::make_error_code(std::errc::protocol_error);
    case compat::ErrorKind::Failed:
      return std::make_error_code(std::errc::io_error);
  }
  return std::make_error_code(std::errc::io_error);
}

class UwfOperationError final : public OperationError {
 public:
  explicit UwfOperationError(const compat::Error& error)
      : OperationError(errorCode(error.failure().kind), error.failure().message, uwfWriteEffect(error.failure().effect)),
        m_failure(error.failure()),
        m_details(uwfDiagnostics(error)) {}
  const char* what() const noexcept override { return m_details.c_str(); }
  OperationFailureKind failureKind() const noexcept override { return uwfFailureKind(m_failure.kind); }
  OperationDiagnostic diagnostic() const noexcept override {
    for (auto entry = m_failure.diagnostics.rbegin(); entry != m_failure.diagnostics.rend(); ++entry)
      if (entry->code) return {std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(entry->code)), 0};
    return {};
  }

 private:
  compat::Failure m_failure;
  std::string m_details;
};
}  // namespace

[[noreturn]] void throwUwfOperationError(const compat::Error& error) { throw UwfOperationError(error); }
WriteEffect callUwfWrite(const compat::Uwf& system, const compat::Request& request, const compat::Context& context) {
  auto response = compat::normalizeResponse(request, system.execute(request, context));
  for (const auto& warning : response.warnings) UWF_LOG_W("uwf") << warning;
  if (response.failure) throwUwfOperationError(compat::Error(std::move(*response.failure), std::move(response.attempts)));
  return uwfWriteEffect(response.effect);
}
}  // namespace uwf::app
