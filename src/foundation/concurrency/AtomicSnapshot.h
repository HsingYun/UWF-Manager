// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <atomic>
#include <memory>
#include <utility>
#include <version>

namespace uwf::foundation {
template <class T>
class AtomicSnapshot final {
 public:
  using Pointer = std::shared_ptr<const T>;
  AtomicSnapshot() noexcept = default;
  explicit AtomicSnapshot(Pointer value) noexcept : m_value(std::move(value)) {}
  AtomicSnapshot(const AtomicSnapshot&) = delete;
  AtomicSnapshot& operator=(const AtomicSnapshot&) = delete;

  [[nodiscard]] Pointer load() const noexcept {
#if defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr >= 201711L
    return m_value.load(std::memory_order_acquire);
#else
    return std::atomic_load_explicit(&m_value, std::memory_order_acquire);
#endif
  }
  void store(Pointer value) noexcept {
#if defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr >= 201711L
    m_value.store(std::move(value), std::memory_order_release);
#else
    std::atomic_store_explicit(&m_value, std::move(value), std::memory_order_release);
#endif
  }

 private:
#if defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr >= 201711L
  std::atomic<Pointer> m_value;
#else
  // Older libc++ versions do not yet implement atomic<shared_ptr<T>>.
  Pointer m_value;
#endif
};
}  // namespace uwf::foundation
