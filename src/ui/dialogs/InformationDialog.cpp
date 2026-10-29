// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "InformationDialog.h"

namespace uwf::ui {
InformationDialog::InformationDialog(QWidget* parent) : DialogWindow(Kind::Information, parent) {}
}  // namespace uwf::ui
#include "moc_InformationDialog.cpp"
