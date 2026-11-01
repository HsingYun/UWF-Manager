// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

#include "application/uwf/contracts/RuntimeWriteRestriction.h"

namespace uwf::presentation {
[[nodiscard]] QString runtimeWriteRestrictionText(app::RuntimeWriteRestriction restriction);
}  // namespace uwf::presentation
