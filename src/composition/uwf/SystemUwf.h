// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "domain/uwf/contracts/Backend.h"
#include "infrastructure/uwf/windows/environment/WindowsConfigurationEnvironment.h"

namespace uwf::windows::wmi {
class WmiOperations;
}
namespace uwf::app {
class VolumeRepository;
}
namespace uwf::composition {
// Production requests retain their synchronous value contract while backend
// construction, routing, execution and destruction run on a shared MTA pool.
[[nodiscard]] std::shared_ptr<compat::Uwf> systemUwf();
// Shared state publisher for production and explicitly injected transports.
// Supplying physical inventory initializes configuration and mounts together;
// otherwise construction retains its lightweight, explicitly refreshed contract.
[[nodiscard]] std::shared_ptr<compat::Uwf> uwfWithStateRefresh(std::shared_ptr<compat::Uwf> system, const windows::WindowsConfigurationEnvironment& environment,
                                                               bool hostInventory = false, std::shared_ptr<const app::VolumeRepository> volumes = {});
// Explicit transport injection is isolated: a fake WMI transport never falls
// through into the real machine's other backends. Its accessor resolves the
// session in the executing MTA; COM interface pointers are not copied from STA.
[[nodiscard]] std::shared_ptr<compat::Uwf> wmiUwf(windows::wmi::WmiOperations& session,
                                                  windows::WindowsConfigurationEnvironment environment = windows::windowsConfigurationEnvironment());
}  // namespace uwf::composition
