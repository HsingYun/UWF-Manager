// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/design/painting/IconPainting.h"

#include <QFile>
#include <QHash>
#include <QIconEngine>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QSvgRenderer>
#include <algorithm>
#include <utility>
#include "presentation/design/painting/RasterInkBounds.h"
namespace uwf::ui {
namespace {
qreal centerOffset(const QRectF& bounds, const QRectF& ink) { return bounds.center().y() - ink.center().y(); }
class ColoredSvgIconEngine final : public QIconEngine {
 public:
  ColoredSvgIconEngine(QByteArray bytes, Qt::Alignment align) : m_bytes(std::move(bytes)), m_renderer(m_bytes), m_align(align) {}
  void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State) override {
    painter->save();
    if (mode == QIcon::Disabled) painter->setOpacity(painter->opacity() * .38);
    const int side = std::min(rect.width(), rect.height());
    const int x = m_align.testFlag(Qt::AlignHCenter) ? rect.x() + (rect.width() - side) / 2
                  : m_align.testFlag(Qt::AlignRight) ? rect.right() - side + 1
                                                     : rect.x();
    m_renderer.render(painter, QRectF(x, rect.y() + (rect.height() - side) / 2, side, side));
    painter->restore();
  }
  QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override {
    QPixmap bitmap(size);
    bitmap.fill(Qt::transparent);
    QPainter painter(&bitmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    paint(&painter, QRect(QPoint{}, size), mode, state);
    return bitmap;
  }
  [[nodiscard]] QIconEngine* clone() const override { return new ColoredSvgIconEngine(m_bytes, m_align); }
  [[nodiscard]] QString key() const override { return QStringLiteral("colored-svg"); }

 private:
  QByteArray m_bytes;
  QSvgRenderer m_renderer;
  Qt::Alignment m_align;
};
}  // namespace
QIcon coloredSvgIcon(const QString& resourcePath, const QColor& ink, const Qt::Alignment align) {
  static QHash<QString, QByteArray> sources;
  static QHash<QString, QIcon> icons;
  const auto key = resourcePath + u'|' + ink.name(QColor::HexArgb) + u'|' + QString::number(align.toInt());
  if (const auto cached = icons.constFind(key); cached != icons.cend()) return *cached;
  if (!sources.contains(resourcePath)) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) return {};
    sources.insert(resourcePath, file.readAll());
  }
  auto source = QString::fromUtf8(sources.value(resourcePath));
  source.replace("#E8EAED", ink.name(QColor::HexRgb), Qt::CaseInsensitive);
  const QIcon icon(new ColoredSvgIconEngine(source.toUtf8(), align));
  if (icons.size() >= 512) icons.clear();
  icons.insert(key, icon);
  return icon;
}
void paintIcon(QPainter& painter, const QIcon& icon, const QRectF& bounds, const QColor& ink) {
  if (icon.isNull() || bounds.isEmpty()) return;
  auto bitmap = icon.pixmap(bounds.size().toSize(), painter.device()->devicePixelRatioF(), QIcon::Normal, QIcon::Off);
  if (ink.isValid()) {
    QPainter tint(&bitmap);
    tint.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tint.fillRect(bitmap.rect(), ink);
  }
  const auto pixels = bitmap.toImage();
  const auto visible = rasterInkBounds(pixels);
  QRectF target = bounds;
  if (!visible.isEmpty()) {
    const qreal scale = bounds.height() / pixels.height();
    target.translate(0, centerOffset(bounds, QRectF(bounds.left(), bounds.top() + visible.top() * scale, bounds.width(), visible.height() * scale)));
  }
  painter.save();
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  painter.drawPixmap(target, bitmap, QRectF(bitmap.rect()));
  painter.restore();
}
void paintIconPath(QPainter& painter, const QPainterPath& path, const QRectF& bounds, const QColor& ink, const qreal strokeWidth) {
  if (path.isEmpty() || bounds.isEmpty()) return;
  painter.save();
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(QPen(ink, strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter.setBrush(Qt::NoBrush);
  painter.translate(0, centerOffset(bounds, path.boundingRect()));
  painter.drawPath(path);
  painter.restore();
}
}  // namespace uwf::ui
