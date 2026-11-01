// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <optional>

#include "domain/uwf/contracts/Types.h"

namespace uwf::windows {
enum class WindowsUwfErrorContext { Operation, Observation };

// Classifies shared Win32/HRESULT meanings; adapters handle unknown and
// provider-specific codes. Invalid-data statuses imply Protocol only for
// observations, since a mutating failure does not establish malformed output.
[[nodiscard]] std::optional<compat::ErrorKind> classifyWindowsUwfError(std::uint32_t code,
                                                                       WindowsUwfErrorContext context = WindowsUwfErrorContext::Operation) noexcept;
}  // namespace uwf::windows
