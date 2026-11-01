// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string_view>

namespace uwf::core {

enum class FileExclusionError { None, Empty, EmbeddedNull, InvalidCharacter, TooLong };

// Validate original Add syntax, not Find/Remove identity or target eligibility.
// The exact \\?\ prefix is excluded from validation and raises the body limit
// from 260 to 32767 UTF-16 units. The body must be nonempty and exclude
// <>:"/|?* and units below U+001F. Preserve separators, dot segments, spaces,
// and U+001F; do not apply this validation when removing observed raw entries.
[[nodiscard]] FileExclusionError fileExclusionError(std::u16string_view path);

}  // namespace uwf::core
