// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "presentation/staging/FileStagingProgressViewModel.h"
class QWidget;
namespace uwf::ui {
void attachFileStagingProgressDialog(presentation::FileStagingProgressViewModel& model, QWidget* parent);
}  // namespace uwf::ui
