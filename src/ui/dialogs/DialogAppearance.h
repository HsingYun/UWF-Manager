// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

class QDialog;
class QPushButton;

namespace uwf::ui {

enum class Sem;
enum class DialogAction { Secondary, Primary, Destructive, DestructiveSecondary, Close };

void configureDialogWindow(QDialog* dialog);
void setDialogAction(QPushButton* button, DialogAction action);

}  // namespace uwf::ui
