// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace uwf {

// Strict UTF-8 decoding without replacement characters. Empty input and U+0000
// are valid; callers enforce their own NUL restrictions.
[[nodiscard]] std::optional<std::u16string> utf8ToUtf16(std::string_view text);

// Validate surrogate pairing, not object-name syntax or NUL restrictions.
[[nodiscard]] bool isValidUtf16(std::u16string_view text) noexcept;

// Truncate valid UTF-8 at a scalar boundary within the byte budget. Include the
// complete suffix only when truncation is needed and it fits. Inputs are not validated.
[[nodiscard]] std::string truncateUtf8(std::string_view text, std::size_t maxBytes, std::string_view suffix = {});

}  // namespace uwf
