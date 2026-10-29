// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>

namespace uwf::windows {
// Keep Win32 error-code semantics while exposing UTF-8 text across application boundaries.
class WindowsSystemError final : public std::system_error {
 public:
  WindowsSystemError(std::uint32_t code, std::string_view operation);
  [[nodiscard]] const char* what() const noexcept override;

 private:
  std::string m_message;
};
}  // namespace uwf::windows
