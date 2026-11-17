// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/files/contracts/TextFileWriter.h"

namespace uwf::presentation {
// Empty means success. File selection/cancellation and feedback belong to the view.
// Arguments remain owned while host I/O may destroy the originating view.
[[nodiscard]] QString exportTextFile(QString path, QString text, app::TextFileWriter writer);
}  // namespace uwf::presentation
