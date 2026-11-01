// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

class QComboBox;
class QVariant;

namespace uwf::ui {

// Leaves the selection unchanged when no item has the requested data.
void setComboValue(QComboBox* combo, const QVariant& value);

}  // namespace uwf::ui
