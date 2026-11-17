// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <ctime>

namespace uwf::detail {
// Reentrant local-time conversion, selected at build time for the host platform.
[[nodiscard]] std::tm localTime(std::time_t time) noexcept;
}  // namespace uwf::detail
