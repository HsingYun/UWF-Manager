// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "ui/dialogs/Dialogs.h"
#include "windows/filesystem/LocalFileBrowser.h"
namespace uwf::test {
inline ui::dialogs::FileDialogProvider& localFileDialogs() {
  static const auto dialogs = ui::dialogs::createFileDialogs(windows::systemFileBrowser());
  return *dialogs;
}
}  // namespace uwf::test
