// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "domain/uwf/contracts/Backend.h"

namespace uwf::app {
enum class UwfStateUpdate { Complete, RefreshRequired };
// Publish one successful raw entry-point receipt. Failed observations and writes
// retain the last published values; composed steps are recorded individually.
// The projection itself reports effects that its receipt and prior facts cannot
// determine, so the caller can refresh once after the complete operation.
UwfStateUpdate recordUwfState(compat::UwfStateFacts& state, const compat::Request& request, const compat::Response& response);

// Explicit configuration/runtime refresh through the UWF interface, excluding
// independent usage samples and private native state. Each failed read preserves
// existing state and contributes its original failure to the returned receipt.
[[nodiscard]] compat::Response refreshUwfState(const compat::Uwf& system, const compat::Context& context = {});
}  // namespace uwf::app
