// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/execution/ConfigurationResult.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace uwf::app {

void recordConfigurationStep(ConfigurationApplyResult& result, ConfigurationStepResult step) { result.steps.push_back(std::move(step)); }

void mergeConfigurationResult(ConfigurationApplyResult& result, ConfigurationApplyResult phase) {
  const auto append = [](auto& target, auto& source) {
    target.insert(target.end(), std::make_move_iterator(source.begin()), std::make_move_iterator(source.end()));
  };
  append(result.steps, phase.steps);
  if (phase.observedOverlayPolicy) result.observedOverlayPolicy = std::move(phase.observedOverlayPolicy);
  if (phase.observedOverlayMaintenance) result.observedOverlayMaintenance = std::move(phase.observedOverlayMaintenance);
}

void completeConfigurationResult(ConfigurationApplyResult& result, const ConfigurationPlan& plan) {
  for (const auto& operation : plan.operations())
    if (std::ranges::none_of(result.steps, [&](const auto& step) { return step.operation == operation; }))
      result.steps.push_back({{}, ConfigurationStepOutcome::NotAttempted, WriteEffect::None, operation});
}

}  // namespace uwf::app
