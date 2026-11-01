// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string_view>

namespace uwf {
// Host object identity, independently of locale and Unicode case folding.
// The Windows host supplies its ordinal, case-insensitive comparison at link
// composition. This pure string operation does not access files or UWF state.
[[nodiscard]] int compareObjectNames(std::u16string_view left, std::u16string_view right);
}  // namespace uwf
