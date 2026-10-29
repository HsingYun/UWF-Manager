// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "app/ports/TextFileCreator.h"

namespace uwf::windows {
[[nodiscard]] app::TextFileCreationResult createUtf8TextFile(const QString& path, const QString& text);
}  // namespace uwf::windows
