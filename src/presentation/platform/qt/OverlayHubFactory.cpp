// SPDX-License-Identifier: GPL-3.0-or-later
#include "presentation/views/overlay/coordination/OverlayHubFactory.h"

#include "presentation/views/overlay/coordination/OverlayHub.h"

namespace uwf::ui {
std::unique_ptr<OverlayHub> createDefaultOverlayHub() { return std::make_unique<OverlayHub>(); }

}  // namespace uwf::ui
