// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/configuration/contracts/ConfigurationActions.h"

namespace uwf::app {

// Append one execution fact; all result summaries derive from these records.
void recordConfigurationStep(ConfigurationApplyResult& result, ConfigurationStepResult step);

// Preserve receipts and the latest supplied observations across semantic phases.
void mergeConfigurationResult(ConfigurationApplyResult& result, ConfigurationApplyResult phase);

// Add NotAttempted receipts for planned operations that have no receipt.
// Repeating completion at a repository or orchestration boundary is idempotent.
void completeConfigurationResult(ConfigurationApplyResult& result, const ConfigurationPlan& plan);

}  // namespace uwf::app
