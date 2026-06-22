#include "RoundedCornerOverlay.h"

#include <QBrush>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

#include <utility>

namespace uwf::ui {

namespace {

// 只描指定边（上 / 下，含相邻圆角）的开口路径——用于"仅补上下边框"。左右竖直段
// 留空（由内容自身的边框补全）。r 为描边中线矩形，rad 为对应半径。
QPainterPath bracketPath(const QRectF& r, const qreal rad, const Qt::Edges edges) {
  QPainterPath path;
  const qreal d = 2 * rad;
  if (edges & Qt::TopEdge) {
    path.moveTo(r.left(), r.top() + rad);
    path.arcTo(r.left(), r.top(), d, d, 180, -90);  // 左上角
    path.lineTo(r.right() - rad, r.top());
    path.arcTo(r.right() - d, r.top(), d, d, 90, -90);  // 右上角
  }
  if (edges & Qt::BottomEdge) {
    path.moveTo(r.right(), r.bottom() - rad);
    path.arcTo(r.right() - d, r.bottom() - d, d, d, 0, -90);  // 右下角
    path.lineTo(r.left() + rad, r.bottom());
    path.arcTo(r.left(), r.bottom() - d, d, d, 270, -90);  // 左下角
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
  // 点击穿透到下面的内容控件。
  setAttribute(Qt::WA_TransparentForMouseEvents);
  // 不清自身底色——下层内容像素留在共享 backing store 里，fillPath 的 AA 边缘据此与
  // 内容平滑混合（QRegion mask 做不到这一点）。
  setAttribute(Qt::WA_NoSystemBackground);
}

void RoundedCornerOverlay::syncToParent() {
  auto* vp = parentWidget();
  if (!vp) return;
  setGeometry(vp->rect());
  raise();
}

void RoundedCornerOverlay::paintEvent(QPaintEvent*) {
  const QRectF r = rect();
  if (r.isEmpty() || !m_colorFn) return;
  const QRectF inner = r.adjusted(m_inset, 0, -m_inset, 0);
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  const bool hasBorder = m_borderFn && m_borderWidth > 0;
  // 整圈边框（无 edgesFn，如列表容器）：把内容圆角矩形「同心」内缩 (边框宽 + 1px)，
  // 即内容圆角与边框同心、且比边框内沿再退 1px，留一道底色缝。这样边框 AA 的内沿压在
  // 底色缝上、而非压在选中蓝上，角部就不会透出蓝色杂边。必须同心（半径与内缩量配套
  // 减）才不会让内容从边框环里钻出来。代价：选中/分隔线在四周内缩约 1px。
  const bool ringBorder = hasBorder && !m_edgesFn;
  const qreal g = ringBorder ? m_borderWidth + 1 : 0;
  const QRectF contentRect = inner.adjusted(g, g, -g, -g);
  const qreal contentRad = qMax(qreal(0), m_radius - g);

  // 把"内容圆角矩形之外"用底色填掉（边距 + 四角缺口 + 边框外缘），四角内容据此抗锯齿圆角。
  QPainterPath cover;
  cover.addRect(r);
  QPainterPath content;
  content.addRoundedRect(contentRect, contentRad, contentRad);
  p.fillPath(cover.subtracted(content), m_colorFn());

  if (!hasBorder) return;

  if (m_edgesFn) {
    // 只描指定边（上/下，含相邻圆角）的描线：以中线为准内缩半线宽，半径相应减半线宽。
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
    // 整圈 AA 边框环（外侧 borderWidth px）：两条同心圆角矩形相减填充，比描线更实，
    // 且能盖住下层 QSS 那条不抗锯齿的边框。环内沿压在上面留出的底色缝上，不碰选中蓝。
    QPainterPath outerRR;
    outerRR.addRoundedRect(inner, m_radius, m_radius);
    QPainterPath ringInner;
    ringInner.addRoundedRect(inner.adjusted(m_borderWidth, m_borderWidth, -m_borderWidth, -m_borderWidth), m_radius - m_borderWidth, m_radius - m_borderWidth);
    p.fillPath(outerRR.subtracted(ringInner), m_borderFn());
  }
}

}  // namespace uwf::ui
