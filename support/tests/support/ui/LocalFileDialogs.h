// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "infrastructure/windows/filesystem/browsing/LocalFileBrowser.h"
#include "presentation/views/files/FilePickerDialogFactory.h"

namespace uwf::test {
inline ui::dialogs::FileDialogProvider& localFileDialogs() {
  static const auto dialogs = ui::createFilePickerDialogs(windows::systemFileBrowser());
  return *dialogs;
}
}  // namespace uwf::test
