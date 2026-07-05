/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <QAbstractButton>

namespace uwf::ui {

// 仿 Windows 11 风格的纯开关控件（只画 pill + 圆点，不带文字）。
// 通过 property("dirty") 可以让 dirty 状态高亮。
class SwitchButton : public QAbstractButton {
  Q_OBJECT
 public:
  explicit SwitchButton(QWidget* parent = nullptr);

  [[nodiscard]] QSize sizeHint() const override;

 protected:
  void paintEvent(QPaintEvent*) override;
};

}  // namespace uwf::ui
