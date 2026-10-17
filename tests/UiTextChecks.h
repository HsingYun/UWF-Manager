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
#include <QScrollArea>
#include <QScrollBar>
#include <QTextDocument>
#include <QWidget>

#include "ui/ContentHintBox.h"

namespace uwf::test {

// 外部滚动容器可以裁切整个控件，但文本控件自身必须给完整文本分配几何。
// 固定大小的图标/徽标不按 QLabel 的首选宽度判定，避免把原生按钮基线误当文本需求。
inline QString clippedLabel(const QWidget& root) {
  if (!root.isVisible()) return QStringLiteral("the inspected window is not visible");
  for (const auto labels = root.findChildren<QLabel*>(); const auto* label : labels) {
    if (!label->isVisible() || label->text().isEmpty()) continue;
    if (label->wordWrap()) {
      const int requiredHeight = label->heightForWidth(label->width());
      if (requiredHeight > label->height()) {
        return QStringLiteral("%1: text needs height %2 at width %3, allocated %4; text=%5")
            .arg(label->objectName())
            .arg(requiredHeight)
            .arg(label->width())
            .arg(label->height())
            .arg(label->text().left(120));
      }
    } else if (label->minimumWidth() != label->maximumWidth() && label->sizeHint().width() > label->width()) {
      return QStringLiteral("%1: text needs width %2, allocated %3; text=%4")
          .arg(label->objectName())
          .arg(label->sizeHint().width())
          .arg(label->width())
          .arg(label->text().left(120));
    }
  }
  for (const auto boxes = root.findChildren<ui::ContentHintBox*>(); auto* box : boxes) {
    if (!box->isVisible() || box->toPlainText().isEmpty()) continue;
    // QTextDocument 的惰性排版可能先上报首行高度；先走真实绘制再读取几何，
    // 避免“字节完整、测量也通过，屏幕却只显示前缀”的假阳性。
    if (box->grab().isNull()) return QStringLiteral("text widget could not be rendered");
    const auto* parent = box->parentWidget();
    const auto* scroll = parent ? qobject_cast<const QScrollArea*>(parent->parentWidget()) : nullptr;
    if (parent && !(scroll && scroll->widget() == box) && !parent->rect().contains(box->geometry()))
      return QStringLiteral("text extends outside its layout host: %1").arg(box->objectName());
    const qreal documentHeight = box->document()->size().height();
    if (documentHeight <= 0 || documentHeight > box->viewport()->height() || box->verticalScrollBar()->maximum() != 0 ||
        box->horizontalScrollBar()->maximum() != 0) {
      return QStringLiteral("%1: document height %2, viewport %3x%4; text=%5")
          .arg(box->objectName())
          .arg(documentHeight)
          .arg(box->viewport()->width())
          .arg(box->viewport()->height())
          .arg(box->toPlainText().left(120));
    }
  }
  return {};
}

}  // namespace uwf::test
