// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ComboBoxSelection.h"

#include <QComboBox>
#include <QVariant>

namespace uwf::ui {

void setComboValue(QComboBox* combo, const QVariant& value) {
  for (int i = 0; i < combo->count(); ++i) {
    if (combo->itemData(i) == value) {
      combo->setCurrentIndex(i);
      return;
    }
  }
}

}  // namespace uwf::ui
