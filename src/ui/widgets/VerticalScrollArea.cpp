// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "VerticalScrollArea.h"

#include <QCoreApplication>
#include <QEvent>
#include <QLayout>
#include <QScopedValueRollback>
#include <QScrollBar>
#include <algorithm>

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
  scheduleScrollUpdate();
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

bool VerticalScrollArea::event(QEvent* event) {
  if (event->type() == QEvent::LayoutRequest) updateScrollPolicy();
  const bool handled = QScrollArea::event(event);
  if (event->type() == QEvent::Resize) scheduleScrollUpdate();
  return handled;
}

bool VerticalScrollArea::eventFilter(QObject* watched, QEvent* event) {
  if (watched == widget() && (event->type() == QEvent::Resize || event->type() == QEvent::LayoutRequest)) {
    // A synchronous scrollbar update can resize the content again before its
    // current layout pass ends, leaving child geometry at the previous size.
    // Let the content finish that pass before QScrollArea measures it again.
    scheduleScrollUpdate();
    if (event->type() == QEvent::LayoutRequest) updateGeometry();
    return false;
  }
  return QScrollArea::eventFilter(watched, event);
}

void VerticalScrollArea::scheduleScrollUpdate() { QCoreApplication::postEvent(this, new QEvent(QEvent::LayoutRequest)); }

void VerticalScrollArea::updateScrollPolicy() {
  if (m_updatingScrollPolicy || !widget() || !widgetResizable()) return;
  const QScopedValueRollback updating(m_updatingScrollPolicy, true);
  const auto available = maximumViewportSize();
  // Decide whether wrapping content fits before subtracting the scrollbar.
  // Measuring only the narrowed viewport can retain a scrollbar after the
  // content shrinks, even though the complete viewport would fit it.
  const auto* layout = widget()->layout();
  const bool wraps = layout ? layout->hasHeightForWidth() : widget()->sizePolicy().hasHeightForWidth();
  const bool fits = wraps && std::max(widget()->minimumHeight(), widget()->heightForWidth(available.width())) <= available.height();
  setVerticalScrollBarPolicy(fits ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
}

}  // namespace uwf::ui
