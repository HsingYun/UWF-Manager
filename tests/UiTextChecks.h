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
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QTableWidget>
#include <QTextDocument>
#include <QToolButton>
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
    if (label->objectName() == QLatin1String("statusKey")) {
      const QRect labelBounds(label->mapTo(&root, QPoint()), label->size());
      for (const auto controls = root.findChildren<QWidget*>(); const auto* control : controls) {
        if (!control->isVisible() || (!qobject_cast<const QAbstractSpinBox*>(control) && !qobject_cast<const QComboBox*>(control))) continue;
        if (labelBounds.intersects(QRect(control->mapTo(&root, QPoint()), control->size())))
          return QStringLiteral("setting label overlaps its input: %1").arg(label->text());
      }
    }
  }
  for (const auto buttons = root.findChildren<QAbstractButton*>(); const auto* button : buttons) {
    if (!button->isVisible() || button->text().isEmpty() || button->property("pagerNavigation").toBool()) continue;
    if (const auto* tool = qobject_cast<const QToolButton*>(button); tool && tool->toolButtonStyle() == Qt::ToolButtonIconOnly) continue;
    if (button->width() < button->sizeHint().width())
      return QStringLiteral("button needs width %1, allocated %2; text=%3").arg(button->sizeHint().width()).arg(button->width()).arg(button->text());
  }
  for (const auto tables = root.findChildren<QTableWidget*>(); const auto* table : tables) {
    const auto* header = table->horizontalHeader();
    if (!table->isVisible() || !header->isVisible()) continue;
    for (int column = 0; column < table->columnCount(); ++column) {
      const auto* item = table->horizontalHeaderItem(column);
      if (!item || table->isColumnHidden(column)) continue;
      const int textWidth = header->fontMetrics().horizontalAdvance(item->text());
      if (header->sectionSize(column) < textWidth)
        return QStringLiteral("table header needs width %1, allocated %2; text=%3").arg(textWidth).arg(header->sectionSize(column)).arg(item->text());
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
