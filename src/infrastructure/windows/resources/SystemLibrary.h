// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <Windows.h>

#include <memory>
#include <string_view>
#include <type_traits>

namespace uwf::windows {

// Owns a System32 module. Callers must retain this lease while using its pointers.
class SystemLibrary final {
 public:
  explicit SystemLibrary(const wchar_t* name) noexcept;
  [[nodiscard]] bool loaded() const noexcept { return static_cast<bool>(m_module); }
  [[nodiscard]] DWORD error() const noexcept { return m_error; }
  [[nodiscard]] FARPROC resolve(const char* name) const noexcept;
  // Return the actual bound import, allowing callers to release an output with
  // the allocator used by the producer rather than their own C runtime.
  [[nodiscard]] FARPROC importedSymbol(std::string_view name) const noexcept;
  [[nodiscard]] bool matchesCodeView(const GUID& guid, DWORD age) const noexcept;

 private:
  struct Deleter {
    void operator()(HMODULE module) const noexcept { FreeLibrary(module); }
  };
  std::unique_ptr<std::remove_pointer_t<HMODULE>, Deleter> m_module;
  DWORD m_error = ERROR_SUCCESS;
};

}  // namespace uwf::windows
