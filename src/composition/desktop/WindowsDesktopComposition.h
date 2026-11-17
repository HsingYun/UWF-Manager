// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "application/staging/persistence/DurableFileStagingStore.h"
#include "composition/desktop/DesktopPlatformPorts.h"
#include "presentation/views/shell/MainWindow.h"

namespace uwf::composition {
// Production Windows service resolution stays outside the injectable factories.
[[nodiscard]] DesktopPlatformPorts systemDesktopPlatformPorts();
std::shared_ptr<app::DurableFileStagingStore> systemFileStagingStore(app::UwfCapability capability);
ui::MainWindowServices systemMainWindowServices(app::UwfCapability capability);
}  // namespace uwf::composition
