// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <stop_token>

#include "application/overlay/contracts/OverlayMaintenance.h"

namespace uwf::windows {
// Decodes SDK HIBERFILE_TYPE_* values; unrecognized values remain Unknown.
[[nodiscard]] app::OverlayHibernationCapabilities decodeWindowsHibernationCapabilities(bool systemS4, bool hiberFilePresent, std::uint8_t hiberFileType);
// Read-only GetPwrCapabilities observation.
[[nodiscard]] app::OverlayHibernationCapabilities readWindowsHibernationCapabilities(std::stop_token stop = {});
}  // namespace uwf::windows
