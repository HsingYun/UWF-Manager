// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <windows.h>
#include <wbemidl.h>

#include <stop_token>

#include "windows/resources/ComPtr.h"

namespace uwf::windows {

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
