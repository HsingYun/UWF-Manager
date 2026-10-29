// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PanelDialog.h"

namespace uwf::ui {
PanelDialog::PanelDialog(QWidget* parent) : DialogWindow(Kind::Panel, parent) {}
}  // namespace uwf::ui
#include "moc_PanelDialog.cpp"
