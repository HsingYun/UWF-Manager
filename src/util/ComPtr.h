/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <utility>

namespace uwf {

// COM 使用侵入式 AddRef/Release 所有权和 T** 输出参数，
// std::unique_ptr/std::shared_ptr 无法直接表达它的 ABI。把这些约定限制在
// 适配器内，避免应用代码手动调用 Release。
template <typename T>
class ComPtr final {
 public:
  ComPtr() = default;

  // 接管调用方已经拥有的一份引用，不调用 AddRef。显式命名可以避免裸指针
  // 构造时看不出所有权语义。
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

  // put() 仅用于通过 T** 交出新引用的 COM 方法。先释放现有引用，避免它被
  // 输出参数直接覆盖而泄漏。
  [[nodiscard]] T** put() noexcept {
    reset();
    return &m_ptr;
  }

  // QueryInterface 一类 API 会把输出类型擦除成 void**。在此集中转换，避免
  // 原始 COM ABI 扩散到调用方。
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

}  // namespace uwf
