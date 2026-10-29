// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QSpinBox>
namespace uwf::ui {
class TextLabel;
class NumberBox : public QSpinBox {
  Q_OBJECT
 public:
  explicit NumberBox(QWidget* parent = nullptr);
  void setUnit(const QString& text);
  [[nodiscard]] QSize sizeHint() const override;
  [[nodiscard]] QSize minimumSizeHint() const override;
 protected:
  void resizeEvent(QResizeEvent* event) override;
  void changeEvent(QEvent* event) override;
 private:
  void placeFields();
  TextLabel* m_unit = nullptr;
};
}  // namespace uwf::ui
