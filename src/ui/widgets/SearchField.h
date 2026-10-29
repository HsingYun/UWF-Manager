// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QLineEdit>
namespace uwf::ui {
class SearchField final : public QLineEdit {
  Q_OBJECT
 public:
  explicit SearchField(QWidget* parent = nullptr);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void changeEvent(QEvent* event) override;

 private:
  void updateMargins();
};
}  // namespace uwf::ui
