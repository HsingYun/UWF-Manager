// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SurfaceShadow.h"

#include <QEvent>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QTimer>
#include <QWidget>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

#include "DesignSystem.h"

namespace uwf::ui {
namespace {
struct Layer {
  int y;
  int blur;
  int spread;
  QColor color;
};
std::size_t pixelOffset(const int width, const int row, const int column) {
  return static_cast<std::size_t>(row) * static_cast<std::size_t>(width) + static_cast<std::size_t>(column);
}
std::array<Layer, 2> layers(const ShadowKind kind, const bool dark) {
  if (kind == ShadowKind::Hud)
    return {{{design::hudShadowY, design::hudShadowBlur, 0, QColor(0, 0, 0, design::hudShadowAlpha)}, {0, 0, 0, QColor(0, 0, 0, 0)}}};
  if (kind == ShadowKind::Popup)
    return {{{design::popupShadowY, design::popupShadowBlur, 0, QColor(0, 0, 0, design::popupShadowAlpha)},
             {design::popupSecondaryShadowY, design::popupSecondaryShadowBlur, 0, QColor(0, 0, 0, design::popupSecondaryShadowAlpha)}}};
  if (dark)
    return {{{design::darkWindowShadowY, design::darkWindowShadowBlur, design::darkWindowShadowSpread, QColor(0, 0, 0, design::darkWindowShadowAlpha)},
             {design::darkWindowSecondaryShadowY, design::darkWindowSecondaryShadowBlur, 0, QColor(0, 0, 0, design::darkWindowSecondaryShadowAlpha)}}};
  return {{{design::windowShadowY, design::windowShadowBlur, design::windowShadowSpread, QColor(28, 39, 56, design::windowShadowAlpha)},
           {design::windowSecondaryShadowY, design::windowSecondaryShadowBlur, 0, QColor(28, 39, 56, design::windowSecondaryShadowAlpha)}}};
}
void boxBlur(std::vector<quint8>& pixels, std::vector<quint8>& buffer, const int width, const int height, const int radius, const bool horizontal) {
  if (radius <= 0) return;
  const int length = horizontal ? width : height;
  const int lines = horizontal ? height : width;
  const int kernel = radius * 2 + 1;
  const auto index = [width, horizontal](const int line, const int position) {
    return pixelOffset(width, horizontal ? line : position, horizontal ? position : line);
  };
  for (int line = 0; line < lines; ++line) {
    int sum = 0;
    for (int position = -radius; position <= radius; ++position)
      if (position >= 0 && position < length) sum += pixels[index(line, position)];
    for (int position = 0; position < length; ++position) {
      buffer[index(line, position)] = static_cast<quint8>((sum + kernel / 2) / kernel);
      if (position - radius >= 0) sum -= pixels[index(line, position - radius)];
      if (position + radius + 1 < length) sum += pixels[index(line, position + radius + 1)];
    }
  }
  pixels.swap(buffer);
}
void gaussianBlur(std::vector<quint8>& pixels, const int width, const int height, const qreal sigma) {
  const qreal ideal = std::sqrt(4 * sigma * sigma + 1);
  int lower = static_cast<int>(std::floor(ideal));
  if (lower % 2 == 0) --lower;
  lower = std::max(1, lower);
  const int count = std::clamp(qRound((12 * sigma * sigma - 3 * lower * lower - 12 * lower - 9) / (-4.0 * lower - 4)), 0, 3);
  std::vector<quint8> buffer(pixels.size());
  for (int pass = 0; pass < 3; ++pass) {
    const int radius = ((pass < count ? lower : lower + 2) - 1) / 2;
    boxBlur(pixels, buffer, width, height, radius, true);
    boxBlur(pixels, buffer, width, height, radius, false);
  }
}
QPixmap shadowTile(const QSize& content, const qreal dpr, const ShadowKind kind) {
  const bool dark = DesignSystem::instance().color(design::Color::Window).lightness() < 128;
  const QString key = QStringLiteral("uwf-shadow-%1-%2-%3-%4-%5").arg(static_cast<int>(kind)).arg(dark).arg(content.width()).arg(content.height()).arg(dpr);
  QPixmap cached;
  if (QPixmapCache::find(key, &cached)) return cached;
  const auto margins = surfaceShadowMargins(kind);
  const QSize logical = content + QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
  const QSize physical(qCeil(logical.width() * dpr), qCeil(logical.height() * dpr));
  const QRectF surface(margins.left(), margins.top(), content.width(), content.height());
  const qreal radius = kind == ShadowKind::Window ? design::windowRadius : design::menuRadius;
  QImage result(physical, QImage::Format_ARGB32_Premultiplied);
  result.setDevicePixelRatio(dpr);
  result.fill(Qt::transparent);
  for (const auto& layer : layers(kind, dark)) {
    if (layer.color.alpha() == 0) continue;
    QImage mask(physical, QImage::Format_ARGB32_Premultiplied);
    mask.setDevicePixelRatio(dpr);
    mask.fill(Qt::transparent);
    {
      QPainter shape(&mask);
      shape.setRenderHint(QPainter::Antialiasing);
      shape.setPen(Qt::NoPen);
      shape.setBrush(Qt::white);
      const QRectF expanded = surface.adjusted(-layer.spread, -layer.spread, layer.spread, layer.spread).translated(0, layer.y);
      shape.drawRoundedRect(expanded, std::max(qreal{0}, radius + layer.spread), std::max(qreal{0}, radius + layer.spread));
    }
    std::vector<quint8> alpha(static_cast<size_t>(physical.width()) * static_cast<size_t>(physical.height()));
    for (int y = 0; y < physical.height(); ++y) {
      const auto* row = reinterpret_cast<const QRgb*>(mask.constScanLine(y));
      for (int x = 0; x < physical.width(); ++x) alpha[pixelOffset(physical.width(), y, x)] = static_cast<quint8>(qAlpha(row[x]));
    }
    gaussianBlur(alpha, physical.width(), physical.height(), layer.blur * dpr / 2.0);
    for (int y = 0; y < physical.height(); ++y) {
      auto* row = reinterpret_cast<QRgb*>(mask.scanLine(y));
      for (int x = 0; x < physical.width(); ++x) {
        const int opacity = alpha[pixelOffset(physical.width(), y, x)] * layer.color.alpha() / 255;
        row[x] = qRgba(layer.color.red() * opacity / 255, layer.color.green() * opacity / 255, layer.color.blue() * opacity / 255, opacity);
      }
    }
    QPainter composite(&result);
    composite.drawImage(QPoint(), mask);
  }
  {
    QPainter punch(&result);
    punch.setRenderHint(QPainter::Antialiasing);
    punch.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    punch.setPen(Qt::NoPen);
    punch.setBrush(Qt::white);
    punch.drawRoundedRect(surface, radius, radius);
  }
  cached = QPixmap::fromImage(result);
  QPixmapCache::insert(key, cached);
  return cached;
}
class ShadowWindow final : public QWidget {
 public:
  ShadowWindow(QWidget* owner, const ShadowKind kind)
      : QWidget(owner, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus),
        m_owner(owner),
        m_kind(kind) {
    setObjectName(QStringLiteral("designSurfaceShadow"));
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::NoFocus);
    owner->installEventFilter(this);
    connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
  }

 protected:
  bool eventFilter(QObject* object, QEvent* event) override {
    if (object != m_owner) return QWidget::eventFilter(object, event);
    switch (event->type()) {
      case QEvent::Show:
      case QEvent::Move:
      case QEvent::Resize:
      case QEvent::WindowStateChange:
      case QEvent::ScreenChangeInternal:
        if (!m_pending) {
          m_pending = true;
          QTimer::singleShot(0, this, [this] {
            m_pending = false;
            synchronize();
          });
        }
        break;
      case QEvent::Hide:
        hide();
        break;
      case QEvent::Close:
        hide();
        QTimer::singleShot(0, this, [this] { synchronize(); });
        break;
      default:
        break;
    }
    return QWidget::eventFilter(object, event);
  }
  void paintEvent(QPaintEvent*) override {
    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    paintSurfaceShadow(painter, QRectF(rect().marginsRemoved(surfaceShadowMargins(m_kind))), m_kind);
  }

 private:
  void synchronize() {
    // A redirected Mica HWND already has a compositor-owned shadow. A second
    // top-level shadow cannot move atomically with it and leaves drag trails.
    if (m_owner->property("systemBackdropActive").toBool()) {
      hide();
      return;
    }
    if (!m_owner->isVisible() || m_owner->isMinimized() || m_owner->isMaximized() || m_owner->isFullScreen()) {
      hide();
      return;
    }
    const auto margins = surfaceShadowMargins(m_kind);
    setGeometry(QRect(m_owner->mapToGlobal(QPoint()), m_owner->size()).marginsAdded(margins));
    if (!isVisible()) show();
    update();
  }
  QWidget* m_owner;
  ShadowKind m_kind;
  bool m_pending = false;
};
}  // namespace
QMargins surfaceShadowMargins(const ShadowKind kind) {
  const int pad = kind == ShadowKind::Window ? design::windowShadowPadding : kind == ShadowKind::Popup ? design::popupShadowPadding : design::hudShadowPadding;
  return {pad, pad, pad, pad};
}
void paintSurfaceShadow(QPainter& painter, const QRectF& surface, const ShadowKind kind) {
  if (surface.isEmpty()) return;
  const int reach = kind == ShadowKind::Window ? design::windowShadowReach : kind == ShadowKind::Popup ? design::popupShadowReach : design::hudShadowReach;
  const QSize core(std::min(qCeil(surface.width()), reach * 2 + 1), std::min(qCeil(surface.height()), reach * 2 + 1));
  const auto tile = shadowTile(core, painter.device()->devicePixelRatioF(), kind);
  const auto margins = surfaceShadowMargins(kind);
  const QRectF target = surface.marginsAdded(QMarginsF(margins));
  const qreal dpr = tile.devicePixelRatioF();
  const QSizeF sourceSize(tile.width() / dpr, tile.height() / dpr);
  const qreal sideX = margins.left() + (core.width() - 1) / 2.0;
  const qreal sideY = margins.top() + (core.height() - 1) / 2.0;
  const std::array<qreal, 4> sx{0, sideX, sourceSize.width() - sideX, sourceSize.width()};
  const std::array<qreal, 4> sy{0, sideY, sourceSize.height() - sideY, sourceSize.height()};
  const std::array<qreal, 4> dx{target.left(), target.left() + sideX, target.right() - sideX, target.right()};
  const std::array<qreal, 4> dy{target.top(), target.top() + sideY, target.bottom() - sideY, target.bottom()};
  painter.save();
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  for (size_t y = 0; y < 3; ++y)
    for (size_t x = 0; x < 3; ++x) {
      if (x == 1 && y == 1) continue;
      painter.drawPixmap(QRectF(dx[x], dy[y], dx[x + 1] - dx[x], dy[y + 1] - dy[y]), tile,
                         QRectF(sx[x] * dpr, sy[y] * dpr, (sx[x + 1] - sx[x]) * dpr, (sy[y + 1] - sy[y]) * dpr));
    }
  painter.restore();
}
void installSurfaceShadow(QWidget* owner, const ShadowKind kind) {
  if (owner->findChild<QWidget*>(QStringLiteral("designSurfaceShadow"), Qt::FindDirectChildrenOnly)) return;
  new ShadowWindow(owner, kind);
}
}  // namespace uwf::ui
