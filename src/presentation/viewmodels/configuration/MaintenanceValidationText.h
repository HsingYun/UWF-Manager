// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

#include "application/overlay/contracts/MaintenanceValidation.h"

namespace uwf::presentation {
[[nodiscard]] QString maintenanceValidationText(const app::MaintenanceValidationIssue& issue);
}  // namespace uwf::presentation
