// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/layout/VerticalScrollArea.h"

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
  // minimumSizeHint reports width requirements without transient horizontal scrollbars.
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  setStyleSheet("QScrollArea { background: transparent; border: none; }");
  viewport()->setAutoFillBackground(false);
}

void VerticalScrollArea::setWidget(QWidget* content) {
  // Layout constraints propagate expanded content height into the scroll range.
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
  // Wrapped text contributes height without expanding the column to its preferred width.
  return QSize(minimumSizeHint().width(), widget() ? widget()->sizeHint().height() + 2 * frameWidth() : QScrollArea::sizeHint().height());
}

bool VerticalScrollArea::event(QEvent* event) {
  // QScrollArea updates ranges synchronously on resize. Choose the policy for
  // the new viewport first so a stale range cannot show an unnecessary bar.
  if (event->type() == QEvent::LayoutRequest || event->type() == QEvent::Resize) updateScrollPolicy();
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
