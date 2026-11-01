// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <functional>
#include <memory>

#include "application/commit/contracts/CommitInteraction.h"
#include "application/files/contracts/TextFileWriter.h"
class QWidget;
namespace uwf::ui {
namespace dialogs {
class FileDialogProvider;
}
[[nodiscard]] app::CommitInteraction commitInteraction(QWidget* parent, app::TextFileWriter writer, dialogs::FileDialogProvider& files);
}  // namespace uwf::ui
