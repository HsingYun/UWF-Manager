// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "TextLayoutChecks.h"

#include <QAbstractButton>
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QDeadlineTimer>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QTableView>
#include <QTest>
#include <QTextDocument>
#include <QToolButton>
#include <QWidget>
#include <atomic>
#include <utility>

#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/text/TextLabel.h"

namespace uwf::test {

namespace {
QString waitForUnclippedText(const QWidget& root, QDeadlineTimer deadline) {
  QString clipping;
  (void)QTest::qWaitFor(
      [&] {
        clipping = clippedText(root);
        return clipping.isEmpty();
      },
      deadline);
  return clipping;
}
}  // namespace

QString waitForUnclippedText(const QWidget& root, const int timeoutMs) { return waitForUnclippedText(root, QDeadlineTimer(timeoutMs, Qt::PreciseTimer)); }

QString waitForUnclippedText(const QWidget& root) {
  return waitForUnclippedText(root, QDeadlineTimer(QTest::defaultTryTimeout.load(std::memory_order_relaxed)));
}

// Scrolling may clip a widget, but the widget must allocate space for its full text.
// Fixed-width labels and explicitly elided text use their own width contracts.
QString clippedText(const QWidget& root) {
  if (!root.isVisible()) return QStringLiteral("the inspected window is not visible");
  for (auto labels = root.findChildren<QLabel*>(); const auto* label : std::as_const(labels)) {
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
    } else if (const auto* designText = qobject_cast<const ui::TextLabel*>(label); (!designText || designText->elideMode() == Qt::ElideNone) &&
                                                                                   label->minimumWidth() != label->maximumWidth() &&
                                                                                   label->sizeHint().width() > label->width()) {
      return QStringLiteral("%1: text needs width %2, allocated %3; text=%4")
          .arg(label->objectName())
          .arg(label->sizeHint().width())
          .arg(label->width())
          .arg(label->text().left(120));
    }
    if (label->objectName() == QLatin1String("statusKey")) {
      const QRect labelBounds(label->mapTo(&root, QPoint()), label->size());
      for (auto controls = root.findChildren<QWidget*>(); const auto* control : std::as_const(controls)) {
        if (!control->isVisible() || (!qobject_cast<const QAbstractSpinBox*>(control) && !qobject_cast<const QComboBox*>(control))) continue;
        if (labelBounds.intersects(QRect(control->mapTo(&root, QPoint()), control->size())))
          return QStringLiteral("setting label overlaps its input: %1").arg(label->text());
      }
    }
  }
  for (auto buttons = root.findChildren<QAbstractButton*>(); const auto* button : std::as_const(buttons)) {
    if (!button->isVisible() || button->text().isEmpty() || button->property("pagerNavigation").toBool()) continue;
    if (const auto* tool = qobject_cast<const QToolButton*>(button); tool && tool->toolButtonStyle() == Qt::ToolButtonIconOnly) continue;
    if (button->width() < button->sizeHint().width())
      return QStringLiteral("button needs width %1, allocated %2; text=%3").arg(button->sizeHint().width()).arg(button->width()).arg(button->text());
  }
  for (auto tables = root.findChildren<QTableView*>(); const auto* table : std::as_const(tables)) {
    const auto* header = table->horizontalHeader();
    if (!table->isVisible() || !header->isVisible()) continue;
    for (int column = 0; table->model() && column < table->model()->columnCount(table->rootIndex()); ++column) {
      if (table->isColumnHidden(column)) continue;
      const QString text = table->model()->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
      if (text.isEmpty()) continue;
      const int textWidth = header->fontMetrics().horizontalAdvance(text);
      if (header->sectionSize(column) < textWidth)
        return QStringLiteral("table header needs width %1, allocated %2; text=%3").arg(textWidth).arg(header->sectionSize(column)).arg(text);
    }
  }
  for (auto boxes = root.findChildren<ui::ContentHintBox*>(); auto* box : std::as_const(boxes)) {
    if (!box->isVisible() || box->toPlainText().isEmpty()) continue;
    // Render before measuring: QTextDocument may initially report only its first line.
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
