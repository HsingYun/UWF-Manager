// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileDialogComposition.h"

#include "windows/filesystem/LocalFileBrowser.h"
namespace uwf::bootstrap {
ui::dialogs::FileDialogProvider& systemFileDialogs() {
  static const auto dialogs = ui::dialogs::createFileDialogs(windows::systemFileBrowser());
  return *dialogs;
}
}  // namespace uwf::bootstrap
