// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/ports/TextFileWriter.h"

namespace uwf::windows {
[[nodiscard]] app::TextFileWriteResult writeUtf8TextFile(const QString& path, const QString& text);
}  // namespace uwf::windows
