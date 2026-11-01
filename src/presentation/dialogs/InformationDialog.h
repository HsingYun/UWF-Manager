// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "presentation/dialogs/DialogWindow.h"

namespace uwf::ui {
class InformationDialog : public DialogWindow {
  Q_OBJECT
 public:
  explicit InformationDialog(QWidget* parent = nullptr);
};
}  // namespace uwf::ui
