// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "presentation/dialogs/Dialogs.h"

namespace uwf::ui {
// Concrete application file-picker views over the host-supplied browser.
[[nodiscard]] std::unique_ptr<dialogs::FileDialogProvider> createFilePickerDialogs(std::shared_ptr<const app::FileBrowser> browser);
}  // namespace uwf::ui
