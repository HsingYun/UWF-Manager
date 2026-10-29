// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/ports/TextFileWriter.h"

namespace uwf::presentation {
// Empty means success. File selection/cancellation and feedback belong to the view.
[[nodiscard]] QString exportTextFile(const QString& path, const QString& text, const app::TextFileWriter& writer);
}  // namespace uwf::presentation
