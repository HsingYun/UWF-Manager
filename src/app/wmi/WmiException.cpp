// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WmiException.h"

#include <bit>
#include <format>
#include <utility>

#include "WmiError.h"

namespace uwf::app::wmi {

namespace {

class WmiCategory final : public std::error_category {
 public:
  [[nodiscard]] const char* name() const noexcept override { return "wmi"; }

  [[nodiscard]] std::string message(const int value) const override {
    const WmiError status(static_cast<int32_t>(value));
    if (status.isKnown()) {
      if (status.description().empty()) return std::string(status.name());
      return std::format("{}: {}", status.name(), status.description());
    }
    return std::format("WMI error 0x{:08x}", static_cast<uint32_t>(value));
  }
};

std::string composeMessage(const std::string& operation, const std::string& detail) {
  return detail.empty() ? operation : std::format("{}: {}", operation, detail);
}

}  // namespace

const std::error_category& wmiErrorCategory() noexcept {
  static WmiCategory category;
  return category;
}

std::error_code makeWmiErrorCode(const int32_t value) noexcept {
  static_assert(sizeof(int) >= sizeof(int32_t));
  return {static_cast<int>(value), wmiErrorCategory()};
}

WmiException::WmiException(std::error_code code, std::string operation, std::string detail, const WriteEffect effect)
    : OperationError(code, composeMessage(operation, detail), effect), m_operation(std::move(operation)) {}

WmiTransportError::WmiTransportError(const int32_t hresult, std::string operation, std::string detail)
    : WmiException(makeWmiErrorCode(hresult), std::move(operation), std::move(detail)) {}

WmiInfrastructureError::WmiInfrastructureError(const int32_t hresult, std::string operation, std::string detail)
    : WmiException(makeWmiErrorCode(hresult), std::move(operation), std::move(detail)) {}

WmiProviderError::WmiProviderError(const uint32_t returnValue, std::string operation, std::string detail)
    : WmiException(makeWmiErrorCode(std::bit_cast<int32_t>(returnValue)), std::move(operation), std::move(detail)), m_returnValue(returnValue) {}

WmiProtocolError::WmiProtocolError(std::string operation, std::string detail)
    : WmiException(std::make_error_code(std::errc::protocol_error), std::move(operation), std::move(detail)) {}

WmiDecodeError::WmiDecodeError(std::string operation, std::string detail)
    : WmiException(std::make_error_code(std::errc::illegal_byte_sequence), std::move(operation), std::move(detail)) {}

WmiInvocationUncertain::WmiInvocationUncertain(const int32_t hresult, std::string operation, std::string detail)
    : WmiWriteOutcomeError(makeWmiErrorCode(hresult), std::move(operation), std::move(detail)) {}

WmiInvocationUncertain::WmiInvocationUncertain(std::string operation, std::string detail)
    : WmiWriteOutcomeError(std::make_error_code(std::errc::state_not_recoverable), std::move(operation), std::move(detail)) {}

WmiStateVerificationError::WmiStateVerificationError(std::string operation, std::string detail)
    : WmiWriteOutcomeError(std::make_error_code(std::errc::state_not_recoverable), std::move(operation), std::move(detail)) {}

WmiCancelled::WmiCancelled(std::string operation)
    : WmiException(std::make_error_code(std::errc::operation_canceled), std::move(operation), "operation cancelled") {}

bool WmiException::hasCode(const WmiErrorCode status) const noexcept {
  return code().category() == wmiErrorCategory() && WmiError(static_cast<int32_t>(code().value())) == status;
}

}  // namespace uwf::app::wmi
