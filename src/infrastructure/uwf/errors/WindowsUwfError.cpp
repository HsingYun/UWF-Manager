// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/uwf/errors/WindowsUwfError.h"

namespace uwf::windows {
namespace {
// Win32 and COM wire values, verified against the SDK by the taxonomy tests.
// This runtime has no Windows API or GUI dependency.
namespace status {
inline constexpr std::uint32_t errorBadCommand = 22u;
inline constexpr std::uint32_t comObjectNotConnected = 0x800401FDu;
inline constexpr std::uint32_t comServerExecutionFailed = 0x80080005u;
inline constexpr std::uint32_t errorAccessDenied = 5u;
inline constexpr std::uint32_t errorAlreadyExists = 183u;
inline constexpr std::uint32_t errorBusy = 170u;
inline constexpr std::uint32_t errorCallNotImplemented = 120u;
inline constexpr std::uint32_t errorCancelled = 1223u;
inline constexpr std::uint32_t errorDatatypeMismatch = 1629u;
inline constexpr std::uint32_t errorDeviceNotConnected = 1167u;
inline constexpr std::uint32_t errorDiskFull = 112u;
inline constexpr std::uint32_t errorFileExists = 80u;
inline constexpr std::uint32_t errorFileNotFound = 2u;
inline constexpr std::uint32_t errorHandleDiskFull = 39u;
inline constexpr std::uint32_t errorInvalidData = 13u;
inline constexpr std::uint32_t errorInvalidEnvironment = 1805u;
inline constexpr std::uint32_t errorInvalidFunction = 1u;
inline constexpr std::uint32_t errorInvalidParameter = 87u;
inline constexpr std::uint32_t errorLockViolation = 33u;
inline constexpr std::uint32_t errorModNotFound = 126u;
inline constexpr std::uint32_t errorNotEnoughMemory = 8u;
inline constexpr std::uint32_t errorNotEnoughQuota = 1816u;
inline constexpr std::uint32_t errorNotFound = 1168u;
inline constexpr std::uint32_t errorNotReady = 21u;
inline constexpr std::uint32_t errorNotSupported = 50u;
inline constexpr std::uint32_t errorNoUnicodeTranslation = 1113u;
inline constexpr std::uint32_t errorOperationAborted = 995u;
inline constexpr std::uint32_t errorOutOfMemory = 14u;
inline constexpr std::uint32_t errorPathNotFound = 3u;
inline constexpr std::uint32_t errorPrivilegeNotHeld = 1314u;
inline constexpr std::uint32_t errorProcNotFound = 127u;
inline constexpr std::uint32_t errorServiceNotActive = 1062u;
inline constexpr std::uint32_t errorSharingViolation = 32u;
inline constexpr std::uint32_t comAborted = 0x80004004u;
inline constexpr std::uint32_t comNoInterface = 0x80004002u;
inline constexpr std::uint32_t comNotImplemented = 0x80004001u;
inline constexpr std::uint32_t comClassNotRegistered = 0x80040154u;
inline constexpr std::uint32_t rpcDisconnected = 0x80010108u;
inline constexpr std::uint32_t rpcServerDied = 0x80010007u;
inline constexpr std::uint32_t rpcServerDiedWithoutExecution = 0x80010012u;
inline constexpr std::uint32_t rpcCallFailed = 1726u;
inline constexpr std::uint32_t rpcCallFailedWithoutExecution = 1727u;
inline constexpr std::uint32_t rpcServerUnavailable = 1722u;
}  // namespace status
}  // namespace
std::optional<compat::ErrorKind> classifyWindowsUwfError(const std::uint32_t code, const WindowsUwfErrorContext context) noexcept {
  using compat::ErrorKind;
  constexpr std::uint32_t deviceNotReadyStatus = 0xc00000a3u;
  constexpr std::uint32_t deviceNotReadyHresult = 0xd00000a3u;
  if (code == deviceNotReadyStatus || code == deviceNotReadyHresult) return ErrorKind::Unavailable;
  constexpr std::uint32_t invalidDeviceStateStatus = 0xc0000184u;
  constexpr std::uint32_t invalidDeviceStateHresult = 0xd0000184u;
  if (code == invalidDeviceStateStatus || code == invalidDeviceStateHresult) return ErrorKind::Conflict;
  if (code == static_cast<std::uint32_t>(status::comNotImplemented) || code == static_cast<std::uint32_t>(status::comNoInterface))
    return ErrorKind::Unsupported;
  if (code == static_cast<std::uint32_t>(status::comAborted)) return ErrorKind::Canceled;
  if (code == static_cast<std::uint32_t>(status::rpcDisconnected) || code == static_cast<std::uint32_t>(status::rpcServerDied) ||
      code == static_cast<std::uint32_t>(status::rpcServerDiedWithoutExecution) || code == static_cast<std::uint32_t>(status::comObjectNotConnected) ||
      code == static_cast<std::uint32_t>(status::comServerExecutionFailed) || code == static_cast<std::uint32_t>(status::comClassNotRegistered))
    return ErrorKind::Unavailable;
  constexpr std::uint32_t facilityMask = 0xffff0000u;
  constexpr std::uint32_t win32FailureFacility = 0x80070000u;
  constexpr std::uint32_t win32CodeMask = 0xffffu;
  const auto native = (code & facilityMask) == win32FailureFacility ? code & win32CodeMask : code;
  switch (native) {
    case status::errorInvalidFunction:
    case status::errorNotSupported:
    case status::errorCallNotImplemented:
    case status::errorProcNotFound:
      return ErrorKind::Unsupported;
    case status::errorAccessDenied:
    case status::errorPrivilegeNotHeld:
      return ErrorKind::AccessDenied;
    case status::errorFileNotFound:
    case status::errorPathNotFound:
    case status::errorNotFound:
      return ErrorKind::NotFound;
    case status::errorInvalidParameter:
      return ErrorKind::InvalidArgument;
    case status::errorCancelled:
    case status::errorOperationAborted:
      return ErrorKind::Canceled;
    case status::errorNotEnoughMemory:
    case status::errorOutOfMemory:
    case status::errorDiskFull:
    case status::errorHandleDiskFull:
    case status::errorNotEnoughQuota:
      return ErrorKind::ResourceExhausted;
    case status::errorBusy:
    case status::errorAlreadyExists:
    case status::errorFileExists:
    case status::errorSharingViolation:
    case status::errorLockViolation:
    case status::errorInvalidEnvironment:
    case status::errorBadCommand:
      return ErrorKind::Conflict;
    case status::errorNotReady:
    case status::errorModNotFound:
    case status::errorDeviceNotConnected:
    case status::errorServiceNotActive:
    case status::rpcServerUnavailable:
    case status::rpcCallFailed:
    case status::rpcCallFailedWithoutExecution:
      return ErrorKind::Unavailable;
    case status::errorInvalidData:
    case status::errorDatatypeMismatch:
    case status::errorNoUnicodeTranslation:
      if (context == WindowsUwfErrorContext::Observation) return ErrorKind::Protocol;
      return std::nullopt;
    default:
      return std::nullopt;
  }
}
}  // namespace uwf::windows
