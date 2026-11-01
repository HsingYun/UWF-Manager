// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <wbemidl.h>
#include <windows.h>

#include <stop_token>

#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/resources/ComPtr.h"

namespace uwf::windows {

// Connection recovery and completion evidence have different policies. Keep
// the existing reconnectable-transport predicate shared by both boundaries.
[[nodiscard]] inline bool isWmiConnectionFailure(const int32_t hresult) noexcept {
  const auto hr = static_cast<HRESULT>(hresult);
  return hr == RPC_E_DISCONNECTED || hr == RPC_E_SERVER_DIED || hr == RPC_E_SERVER_DIED_DNE || hr == CO_E_SERVER_STOPPING || hr == CO_E_OBJNOTCONNECTED ||
         hr == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) || hr == HRESULT_FROM_WIN32(RPC_S_CALL_FAILED) || hr == HRESULT_FROM_WIN32(RPC_S_CALL_FAILED_DNE) ||
         hr == static_cast<HRESULT>(WBEM_E_TRANSPORT_FAILURE) || hr == static_cast<HRESULT>(WBEM_E_FATAL_TRANSPORT_ERROR) ||
         hr == static_cast<HRESULT>(WBEM_E_CONNECTION_FAILED) || hr == static_cast<HRESULT>(WBEM_E_SHUTTING_DOWN);
}

// WBEM_STATUS_COMPLETE can report a failed operation. Ordinary terminal errors
// establish completion; client cancellation and loss of the observation channel
// do not establish that the provider stopped. Positive timeout/pending statuses
// also cannot be accepted as successful method completion.
[[nodiscard]] inline bool isWmiReadCompletionUnknown(const HRESULT hr) noexcept {
  return isWmiConnectionFailure(static_cast<int32_t>(hr)) || hr == E_ABORT || hr == RPC_E_CALL_CANCELED || hr == HRESULT_FROM_WIN32(ERROR_CANCELLED) ||
         hr == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED) || hr == HRESULT_FROM_WIN32(RPC_S_CALL_CANCELLED) ||
         hr == static_cast<HRESULT>(WBEM_E_CALL_CANCELLED) || hr == static_cast<HRESULT>(WBEM_S_OPERATION_CANCELLED) || hr == RPC_E_TIMEOUT ||
         hr == HRESULT_FROM_WIN32(ERROR_TIMEOUT) || hr == HRESULT_FROM_WIN32(WAIT_TIMEOUT) || hr == HRESULT_FROM_WIN32(ERROR_SEM_TIMEOUT) ||
         hr == static_cast<HRESULT>(WBEM_E_TIMED_OUT) || hr == static_cast<HRESULT>(WBEM_E_PROVIDER_TIMED_OUT) || hr == static_cast<HRESULT>(WBEM_S_TIMEDOUT) ||
         hr == static_cast<HRESULT>(WBEM_S_PENDING) || hr == E_PENDING || hr == RPC_E_CONNECTION_TERMINATED || hr == RPC_E_CLIENT_DIED || hr == RPC_E_FAULT ||
         hr == RPC_E_SERVERFAULT || hr == HRESULT_FROM_WIN32(RPC_S_COMM_FAILURE) || hr == static_cast<HRESULT>(WBEM_E_INVALID_STREAM);
}

class SrwExclusiveLock final {
 public:
  explicit SrwExclusiveLock(SRWLOCK& lock) noexcept : m_lock(&lock) { AcquireSRWLockExclusive(m_lock); }
  ~SrwExclusiveLock() {
    if (m_lock) ReleaseSRWLockExclusive(m_lock);
  }
  SrwExclusiveLock(const SrwExclusiveLock&) = delete;
  SrwExclusiveLock& operator=(const SrwExclusiveLock&) = delete;

  void unlock() noexcept {
    ReleaseSRWLockExclusive(m_lock);
    m_lock = nullptr;
  }
  void lock(SRWLOCK& value) noexcept {
    AcquireSRWLockExclusive(&value);
    m_lock = &value;
  }

 private:
  SRWLOCK* m_lock;
};

// Shared by the WMI callback and its invoking thread. Only completion, stop,
// or a failed native wait ends the wait; slow providers have no local deadline.
struct WmiMethodCompletion {
  SRWLOCK lock = SRWLOCK_INIT;
  CONDITION_VARIABLE changed = CONDITION_VARIABLE_INIT;
  bool completed = false;
  HRESULT status = E_UNEXPECTED;
  ComPtr<IWbemClassObject> output;

  // Inspect while holding lock (or after all callbacks have stopped). Record
  // evidence before constructing an exception, independently of method success.
  // Session and transport tests use this same raw SetStatus conversion.
  void checkReadStatus(wmi::ReadCompletion& evidence, const std::string& operation) const {
    evidence = completed && !isWmiReadCompletionUnknown(status) ? wmi::ReadCompletion::ProviderCompleted : wmi::ReadCompletion::Unknown;
    if (!completed) {
      wmi::WmiProtocolError error(operation, "The asynchronous method has no terminal status");
      error.setReadCompletion(evidence);
      throw error;
    }
    if (SUCCEEDED(status) && evidence == wmi::ReadCompletion::ProviderCompleted) return;
    if (isWmiConnectionFailure(static_cast<int32_t>(status))) {
      wmi::WmiTransportError error(static_cast<int32_t>(status), operation, "asynchronous completion failed");
      error.setReadCompletion(evidence);
      throw error;
    }
    wmi::WmiInfrastructureError error(static_cast<int32_t>(status), operation, "asynchronous completion failed");
    error.setReadCompletion(evidence);
    throw error;
  }

  [[nodiscard]] DWORD wait(const std::stop_token stopToken) noexcept {
    std::stop_callback wakeOnStop(stopToken, [this]() noexcept {
      // Use the predicate's lock so stop cannot lose a wake between the check
      // and SleepConditionVariableSRW atomically releasing the lock.
      SrwExclusiveLock guard(lock);
      WakeAllConditionVariable(&changed);
    });
    // Unlock before unregistering the callback, which may be waiting for lock.
    SrwExclusiveLock guard(lock);
    while (!completed && !stopToken.stop_requested()) {
      if (!SleepConditionVariableSRW(&changed, &lock, INFINITE, 0)) return GetLastError();
    }
    return ERROR_SUCCESS;
  }

  void complete(const HRESULT result) noexcept {
    {
      SrwExclusiveLock guard(lock);
      status = result;
      completed = true;
    }
    WakeAllConditionVariable(&changed);
  }
};

}  // namespace uwf::windows
