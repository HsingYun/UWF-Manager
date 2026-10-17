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

// 按完整文档高度展示只读提示；与设置列共用外部滚动条，不在内部裁切或滚动。

#include <QTextBrowser>

class QResizeEvent;

namespace uwf::ui {

class ContentHintBox final : public QTextBrowser {
  Q_OBJECT
 public:
  explicit ContentHintBox(QWidget* parent = nullptr);
  [[nodiscard]] QSize sizeHint() const override;

 protected:
  void resizeEvent(QResizeEvent* e) override;

 private:
  // 等待 Qt 按最终视口宽度排版后测量；合并同轮文本、字体及尺寸变更。
  void scheduleFit();
  void fitDocument();
  [[nodiscard]] int documentHeight(int width) const;
  bool m_fitQueued = false;
};

}  // namespace uwf::ui
