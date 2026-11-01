// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "presentation/dialogs/PanelDialog.h"

namespace uwf::ui {

// A passive text view: command execution and result interpretation stay with
// the caller. Help, successful output and diagnostics share the same surface.
class CommandOutputDialog final : public PanelDialog {
  Q_OBJECT
 public:
  explicit CommandOutputDialog(const QString& title, const QString& text, QWidget* parent = nullptr);
};

}  // namespace uwf::ui
