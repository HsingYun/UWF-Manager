// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "domain/uwf/contracts/Backend.h"

namespace uwf::app {
// Cross-capability behavior consumes the caller's shared observation only.
// Downstream operations are raw writes/reads; no prerequisite or readback getter
// is issued while carrying out another capability.
[[nodiscard]] compat::Capability probeUwfOperation(const compat::Uwf& downstream, const compat::Request& request, const compat::Context& context = {});
[[nodiscard]] compat::Response executeUwfOperation(const compat::Uwf& downstream, const compat::Request& request, const compat::Context& context = {});
}  // namespace uwf::app
