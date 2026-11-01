// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <memory>

#include "presentation/controls/clipboard/ClipboardText.h"

class QContextMenuEvent;
class QWidget;

namespace uwf::ui {
class CommandMenu;

// The event presentation and semantic tests consume this same menu builder.
// Each menu owns its explicitly supplied writer; no process-wide override.
[[nodiscard]] std::unique_ptr<CommandMenu> createContextMenu(QWidget& target, const QContextMenuEvent& event, ClipboardTextWriter writer);
void installContextMenuPresentation();
}  // namespace uwf::ui
