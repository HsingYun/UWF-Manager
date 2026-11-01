// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "infrastructure/windows/management/wmi/WmiOperations.h"

namespace uwf::windows {

// Each thread owns two fixed-namespace sessions and a COM apartment lease.
// First access initializes them; TLS destruction releases them. Explicit
// shutdown permanently retires the thread's context, so later access throws.
windows::wmi::WmiOperations& embeddedWmiSession();
windows::wmi::WmiOperations& cimv2WmiSession();

// Dedicated threads may explicitly release the context after their clients
// are destroyed; pooled threads normally retain it until thread exit.
void initializeWmiRuntime();
void shutdownWmiRuntime() noexcept;

}  // namespace uwf::windows
