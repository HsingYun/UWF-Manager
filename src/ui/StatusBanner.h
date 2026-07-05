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

#include <QLabel>

namespace uwf::ui {

// 状态横幅（错误 / 警告条）。替代原先靠 QSS border-radius 画圆角底+边框的 QLabel：
// Qt 的 QSS 圆角不抗锯齿，且半透明底叠半透明边在锯齿角上会叠出杂色。本类改用 QPainter
// 抗锯齿绘制圆角底+边框，文字仍由 QLabel 基类按 QSS 的 color / padding 绘制。
//
// 配色按动态属性 "level" 取："warn" → 琥珀色警告，其余 → 红色错误；并随当前主题切换。
// 用法和原来一致：setObjectName("statusBanner")（沿用 QSS 的文字色 / 内边距）、需要警告
// 色时 setProperty("level", "warn")。主题切换时自动重绘。
class StatusBanner : public QLabel {
 public:
  explicit StatusBanner(QWidget* parent = nullptr);

 protected:
  void paintEvent(QPaintEvent* ev) override;
};

}  // namespace uwf::ui
