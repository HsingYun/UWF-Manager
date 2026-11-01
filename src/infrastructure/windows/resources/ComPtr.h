// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <utility>

namespace uwf::windows {

// Owns one COM reference and supports native interface output parameters.
template <typename T>
class ComPtr final {
 public:
  ComPtr() = default;

  // Adopts an existing reference without calling AddRef.
  [[nodiscard]] static ComPtr adopt(T* value) noexcept { return ComPtr(value, AdoptTag{}); }

  [[nodiscard]] static ComPtr retain(T* value) noexcept {
    if (value) value->AddRef();
    return adopt(value);
  }

  ~ComPtr() { reset(); }

  ComPtr(const ComPtr&) = delete;
  ComPtr& operator=(const ComPtr&) = delete;

  ComPtr(ComPtr&& other) noexcept : m_ptr(std::exchange(other.m_ptr, nullptr)) {}
  ComPtr& operator=(ComPtr&& other) noexcept {
    if (this != &other) {
      reset();
      m_ptr = std::exchange(other.m_ptr, nullptr);
    }
    return *this;
  }

  [[nodiscard]] T* get() const noexcept { return m_ptr; }
  [[nodiscard]] T& operator*() const noexcept { return *m_ptr; }
  [[nodiscard]] T* operator->() const noexcept { return m_ptr; }
  [[nodiscard]] explicit operator bool() const noexcept { return m_ptr != nullptr; }

  // Releases the current reference before receiving a new owned reference.
  [[nodiscard]] T** put() noexcept {
    reset();
    return &m_ptr;
  }

  // For APIs such as QueryInterface that use void** output parameters.
  [[nodiscard]] void** putVoid() noexcept { return reinterpret_cast<void**>(put()); }

  void reset() noexcept {
    if (m_ptr) m_ptr->Release();
    m_ptr = nullptr;
  }

  [[nodiscard]] T* release() noexcept { return std::exchange(m_ptr, nullptr); }

 private:
  struct AdoptTag {};
  explicit ComPtr(T* value, AdoptTag) noexcept : m_ptr(value) {}

  T* m_ptr = nullptr;
};

}  // namespace uwf::windows
