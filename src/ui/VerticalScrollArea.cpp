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
#include "VerticalScrollArea.h"

#include <QEvent>
#include <QLayout>
#include <QScrollBar>

namespace uwf::ui {

VerticalScrollArea::VerticalScrollArea(QWidget* parent) : QScrollArea(parent) {
  setFrameShape(QFrame::NoFrame);
  setWidgetResizable(true);
  // 横向空间由 minimumSizeHint 传给父布局；类型切换的中间帧也不应
  // 临时显示横向滚动条，进一步挤占纵向空间并触发重复重排。
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  setStyleSheet("QScrollArea { background: transparent; border: none; }");
  viewport()->setAutoFillBackground(false);
}

void VerticalScrollArea::setWidget(QWidget* content) {
  // 动态正文的尺寸变化必须传到滚动宿主；否则子控件虽已展开，宿主仍可能
  // 停在旧高度，滚动范围也不会覆盖全文。使用 Qt 的布局约束驱动同步更新。
  if (content && content->layout()) content->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
  QScrollArea::setWidget(content);
}

QSize VerticalScrollArea::minimumSizeHint() const {
  QSize size = QScrollArea::minimumSizeHint();
  if (widget()) size.setWidth(widget()->minimumSizeHint().width() + verticalScrollBar()->sizeHint().width() + 2 * frameWidth());
  return size;
}

QSize VerticalScrollArea::sizeHint() const {
  // 换行警告的首选宽度受文字长度影响；不能用它改变整列宽度。
  return QSize(minimumSizeHint().width(), widget() ? widget()->sizeHint().height() + 2 * frameWidth() : QScrollArea::sizeHint().height());
}

bool VerticalScrollArea::eventFilter(QObject* watched, QEvent* event) {
  const bool handled = QScrollArea::eventFilter(watched, event);
  if (watched == widget() && event->type() == QEvent::LayoutRequest) updateGeometry();
  return handled;
}

}  // namespace uwf::ui
