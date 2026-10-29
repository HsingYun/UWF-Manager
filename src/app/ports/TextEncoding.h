// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <string_view>

namespace uwf::app {

// Decode persisted UTF-8 strictly; malformed input must not become replacement characters.
[[nodiscard]] QString decodeUtf8Text(std::string_view text);

}  // namespace uwf::app
