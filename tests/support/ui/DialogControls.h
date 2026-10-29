// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVariant>
#include <QWidget>

namespace uwf::test {

[[nodiscard]] inline QPushButton* buttonByAccessibleName(QWidget* parent, const QString& name) {
  if (!parent) return nullptr;
  for (const auto buttons = parent->findChildren<QPushButton*>(); auto* button : buttons)
    if (button->property("accessibleName").toString() == name) return button;
  return nullptr;
}

[[nodiscard]] inline QPushButton* buttonByRole(QWidget* parent, const QDialogButtonBox::ButtonRole role) {
  if (!parent) return nullptr;
  for (const auto boxes = parent->findChildren<QDialogButtonBox*>(); auto* box : boxes)
    for (const auto buttons = box->buttons(); auto* button : buttons)
      if (box->buttonRole(button) == role) return qobject_cast<QPushButton*>(button);
  return nullptr;
}

}  // namespace uwf::test
