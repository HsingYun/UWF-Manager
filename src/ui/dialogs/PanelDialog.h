// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "DialogWindow.h"

namespace uwf::ui {
class PanelDialog : public DialogWindow {
  Q_OBJECT
 public:
  explicit PanelDialog(QWidget* parent = nullptr);
};
}  // namespace uwf::ui
