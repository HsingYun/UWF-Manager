// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <string>
#include <system_error>
#include <utility>

namespace uwf::windows::wmi {
// Transport dispatch evidence, independent of any application's failure model.
enum class WriteEffect { None, Uncertain };
enum class ReadCompletion { NotDispatched, ProviderCompleted, Unknown };
struct WmiDiagnostic {
  int32_t code = 0;
  uint32_t result = 0;
};

enum class WmiErrorCode : int32_t;

const std::error_category& wmiErrorCategory() noexcept;
std::error_code makeWmiErrorCode(int32_t value) noexcept;

// Exceptions carry failure evidence; recovery boundaries own logging and UI.
class WmiException : public std::system_error {
 public:
  [[nodiscard]] const std::string& operation() const noexcept { return m_operation; }
  [[nodiscard]] bool hasCode(WmiErrorCode status) const noexcept;
  [[nodiscard]] WriteEffect writeEffect() const noexcept { return m_writeEffect; }
  [[nodiscard]] ReadCompletion readCompletion() const noexcept { return m_readCompletion; }
  // The single-dispatch boundary records progress without slicing the native
  // failure type or losing provider diagnostics.
  void setReadCompletion(ReadCompletion completion) noexcept { m_readCompletion = completion; }
  [[nodiscard]] virtual WmiDiagnostic diagnostic() const noexcept;

 protected:
  WmiException(std::error_code code, std::string operation, std::string detail = {}, WriteEffect effect = WriteEffect::None);

 private:
  std::string m_operation;
  WriteEffect m_writeEffect;
  ReadCompletion m_readCompletion = ReadCompletion::NotDispatched;
};

class WmiTransportError final : public WmiException {
 public:
  WmiTransportError(int32_t hresult, std::string operation, std::string detail = {});
};

// Request-level COM/WMI failure, distinct from provider ReturnValue and
// reconnectable transport failures.
class WmiInfrastructureError final : public WmiException {
 public:
  WmiInfrastructureError(int32_t hresult, std::string operation, std::string detail = {});
};

class WmiProviderError final : public WmiException {
 public:
  WmiProviderError(uint32_t returnValue, std::string operation, std::string detail = {}, WriteEffect effect = WriteEffect::None);
  [[nodiscard]] uint32_t returnValue() const noexcept { return m_returnValue; }
  [[nodiscard]] WmiDiagnostic diagnostic() const noexcept override { return {0, m_returnValue}; }

 private:
  uint32_t m_returnValue;
};

class WmiProtocolError final : public WmiException {
 public:
  WmiProtocolError(std::string operation, std::string detail);
};

// Non-final so std::throw_with_nested can preserve a local decoding exception.
class WmiDecodeError : public WmiException {
 public:
  WmiDecodeError(std::string operation, std::string detail);
};

// Post-dispatch failures retain Uncertain even when no valid result is decoded.
class WmiWriteOutcomeError : public WmiException {
 protected:
  WmiWriteOutcomeError(std::error_code code, std::string operation, std::string detail)
      : WmiException(code, std::move(operation), std::move(detail), WriteEffect::Uncertain) {}
};

// Non-final so std::throw_with_nested can retain the underlying invocation or
// decoding failure after a potentially mutating dispatch.
class WmiInvocationUncertain : public WmiWriteOutcomeError {
 public:
  WmiInvocationUncertain(int32_t hresult, std::string operation, std::string detail = {});
  WmiInvocationUncertain(std::string operation, std::string detail);
};

class WmiCancelled final : public WmiException {
 public:
  explicit WmiCancelled(std::string operation);
};

}  // namespace uwf::windows::wmi
