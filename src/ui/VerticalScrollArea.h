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

#include <QScrollArea>

namespace uwf::ui {

// 纵向滚动容器保留内容的横向最小尺寸，不把完整内容高度传播为窗口下限。
// 字体、译文或横幅改变后重新报告尺寸，不缓存首次 polish 前的内容宽度。
class VerticalScrollArea final : public QScrollArea {
 public:
  explicit VerticalScrollArea(QWidget* parent = nullptr);
  void setWidget(QWidget* content);
  [[nodiscard]] QSize minimumSizeHint() const override;
  [[nodiscard]] QSize sizeHint() const override;

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;
};

}  // namespace uwf::ui
