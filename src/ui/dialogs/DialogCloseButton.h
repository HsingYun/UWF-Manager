// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPushButton>

namespace uwf::ui {
class DialogCloseButton final : public QPushButton {
 public:
  explicit DialogCloseButton(QWidget* parent);

 protected:
  bool event(QEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

 private:
  void updateIcon();
};
}  // namespace uwf::ui
