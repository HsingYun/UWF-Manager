// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowsSystemError.h"

#include "WindowsErrorText.h"

namespace uwf::windows {
WindowsSystemError::WindowsSystemError(const std::uint32_t code, const std::string_view operation)
    : std::system_error(static_cast<int>(code), std::system_category()), m_message(operation) {
  if (!m_message.empty()) m_message += ": ";
  m_message += windowsErrorText(code).toUtf8().toStdString();
}
const char* WindowsSystemError::what() const noexcept { return m_message.c_str(); }
}  // namespace uwf::windows
