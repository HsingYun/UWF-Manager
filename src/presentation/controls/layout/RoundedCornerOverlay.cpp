// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/layout/RoundedCornerOverlay.h"

#include <QBrush>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <utility>

namespace uwf::ui {

namespace {

// Selected edges include adjacent corners; r follows the stroke centerline.
QPainterPath bracketPath(const QRectF& r, const qreal rad, const Qt::Edges edges) {
  QPainterPath path;
  const qreal d = 2 * rad;
  if (edges & Qt::TopEdge) {
    path.moveTo(r.left(), r.top() + rad);
    path.arcTo(r.left(), r.top(), d, d, 180, -90);
    path.lineTo(r.right() - rad, r.top());
    path.arcTo(r.right() - d, r.top(), d, d, 90, -90);
  }
  if (edges & Qt::BottomEdge) {
    path.moveTo(r.right(), r.bottom() - rad);
    path.arcTo(r.right() - d, r.bottom() - d, d, d, 0, -90);
    path.lineTo(r.left() + rad, r.bottom());
    path.arcTo(r.left(), r.bottom() - d, d, d, 270, -90);
  }
  return path;
}

}  // namespace

RoundedCornerOverlay::RoundedCornerOverlay(QWidget* host, const qreal inset, const qreal radius, std::function<QColor()> colorFn,
                                           std::function<QColor()> borderFn, const qreal borderWidth, std::function<Qt::Edges()> edgesFn)
    : QWidget(host),
      m_inset(inset),
      m_radius(radius),
      m_colorFn(std::move(colorFn)),
      m_borderFn(std::move(borderFn)),
      m_borderWidth(borderWidth),
      m_edgesFn(std::move(edgesFn)) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  // Preserves the backing store for antialiased blending with underlying content.
  setAttribute(Qt::WA_NoSystemBackground);
  if (host) {
    // QAbstractScrollArea's resize hook follows its viewport, which can retain
    // its whole-row height while the surrounding list frame changes size.
    host->installEventFilter(this);
    syncToParent();
  }
}

bool RoundedCornerOverlay::eventFilter(QObject* object, QEvent* event) {
  if (object == parentWidget() && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) syncToParent();
  return QWidget::eventFilter(object, event);
}

void RoundedCornerOverlay::syncToParent() {
  auto* host = parentWidget();
  if (!host) return;
  setGeometry(host->rect());
  raise();
}

void RoundedCornerOverlay::paintEvent(QPaintEvent*) {
  const QRectF r = rect();
  if (r.isEmpty() || !m_colorFn) return;
  const QRectF inner = r.adjusted(m_inset, 0, -m_inset, 0);
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  const bool hasBorder = m_borderFn && m_borderWidth > 0;
  // A one-pixel gap separates selection fills from the border antialiasing.
  const bool ringBorder = hasBorder && !m_edgesFn;
  const qreal g = ringBorder ? m_borderWidth + 1 : 0;
  const QRectF contentRect = inner.adjusted(g, g, -g, -g);
  const qreal contentRad = qMax(qreal(0), m_radius - g);

  QPainterPath cover;
  cover.addRect(r);
  QPainterPath content;
  content.addRoundedRect(contentRect, contentRad, contentRad);
  p.fillPath(cover.subtracted(content), m_colorFn());

  if (!hasBorder) return;

  if (m_edgesFn) {
    // The half-stroke inset keeps the border inside the widget.
    const Qt::Edges edges = m_edgesFn();
    if (!edges) return;
    const qreal half = m_borderWidth / 2.0;
    const QRectF br = inner.adjusted(half, half, -half, -half);
    const qreal rad = m_radius - half;
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(m_borderFn(), m_borderWidth));
    if (edges == (Qt::TopEdge | Qt::BottomEdge | Qt::LeftEdge | Qt::RightEdge))
      p.drawRoundedRect(br, rad, rad);
    else
      p.drawPath(bracketPath(br, rad, edges));
  } else {
    // The ring covers the underlying non-antialiased stylesheet border.
    QPainterPath outerRR;
    outerRR.addRoundedRect(inner, m_radius, m_radius);
    QPainterPath ringInner;
    ringInner.addRoundedRect(inner.adjusted(m_borderWidth, m_borderWidth, -m_borderWidth, -m_borderWidth), m_radius - m_borderWidth, m_radius - m_borderWidth);
    p.fillPath(outerRR.subtracted(ringInner), m_borderFn());
  }
}

}  // namespace uwf::ui
