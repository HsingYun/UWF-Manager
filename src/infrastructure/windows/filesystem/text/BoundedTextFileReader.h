// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "application/files/contracts/TextFileReader.h"
namespace uwf::windows {
[[nodiscard]] app::TextFileReadResult readBoundedTextFile(const QString& path, qint64 maximumBytes);
}  // namespace uwf::windows
