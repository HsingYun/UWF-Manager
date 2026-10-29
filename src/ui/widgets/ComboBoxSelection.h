// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

class QComboBox;
class QVariant;

namespace uwf::ui {

// 把 combo 的当前项切到 itemData == value 的那项；找不到则不动。两个状态面板共用。
void setComboValue(QComboBox* combo, const QVariant& value);

}  // namespace uwf::ui
