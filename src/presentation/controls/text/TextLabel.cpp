// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/text/TextLabel.h"

#include <QEvent>
#include <QFontMetricsF>
#include <QGraphicsEffect>
#include <QPainter>
#include <QStyle>
#include <QTextLayout>
#include <algorithm>
#include <cmath>

#include "presentation/design/painting/TextPainting.h"
#include "presentation/design/theme/DesignSystem.h"
namespace uwf::ui {
namespace {
QPoint rasterOrigin(const QWidget& widget) {
  // Effects paint descendants into a local source pixmap. Its origin determines
  // the fractional raster phase used by the corresponding paint event.
  for (const QWidget* ancestor = &widget; ancestor; ancestor = ancestor->parentWidget()) {
    if (const auto* effect = ancestor->graphicsEffect(); effect && effect->isEnabled()) return widget.mapTo(ancestor, QPoint());
    if (ancestor->isWindow()) break;
  }
  return widget.mapTo(widget.window(), QPoint());
}
QString layoutValue(QString value) {
  return value.replace(QStringLiteral("\r\n"), QStringLiteral("\n")).replace(u'\r', u'\n').replace(u'\n', QChar(QChar::LineSeparator));
}
QSizeF layoutText(QTextLayout& layout, int width, qreal step) {
  layout.beginLayout();
  qreal y = 0, maxWidth = 0;
  while (true) {
    auto line = layout.createLine();
    if (!line.isValid()) break;
    line.setLineWidth(width);
    line.setPosition(QPointF(0, y + (step - line.height()) / 2));
    y += step;
    maxWidth = std::max(maxWidth, line.naturalTextWidth());
  }
  layout.endLayout();
  return {maxWidth, y};
}
qreal layoutTop(const QTextLayout& layout, const QSizeF& size, const QRect& bounds, const Qt::Alignment alignment, const TextVerticalReference reference) {
  if (alignment.testFlag(Qt::AlignVCenter)) {
    if (reference == TextVerticalReference::GlyphBounds) {
      const auto ink = textInkBounds(layout);
      if (!ink.isEmpty()) return bounds.top() + bounds.height() / 2.0 - ink.center().y();
    } else if (layout.lineCount() == 1) {
      const QFontMetricsF metrics(layout.font());
      const auto line = layout.lineAt(0);
      return bounds.top() + bounds.height() / 2.0 + (metrics.ascent() - metrics.descent()) / 2.0 - line.ascent() - line.y();
    }
    return bounds.top() + (bounds.height() - size.height()) / 2.0;
  }
  if (alignment.testFlag(Qt::AlignBottom)) {
    const auto ink = reference == TextVerticalReference::GlyphBounds ? textInkBounds(layout) : QRectF{};
    return bounds.top() + bounds.height() - (ink.isEmpty() ? size.height() : ink.bottom());
  }
  return bounds.top();
}
qreal alignedLayoutTop(const QTextLayout& layout, const QSizeF& size, const QRect& bounds, const Qt::Alignment alignment, const TextVerticalReference reference,
                       const qreal offset, const QTransform& device) {
  qreal top = layoutTop(layout, size, bounds, alignment, reference) + offset;
  if (layout.lineCount() == 0 || !alignment.testFlag(Qt::AlignVCenter)) return top;
  const auto line = layout.lineAt(0);
  const auto snap = [&](const qreal value) {
    if (layout.lineCount() != 1) return value;
    return snapTextBaseline(value + line.y() + line.ascent(), device) - line.y() - line.ascent();
  };
  top = snap(top);
  if (reference == TextVerticalReference::GlyphBounds) {
    const auto ink = textRasterBounds(layout, QPointF(bounds.left(), top), bounds, device);
    if (!ink.isEmpty()) top = snap(top + QRectF(bounds).center().y() + offset - ink.center().y());
  }
  return top;
}
}  // namespace
TextLabel::TextLabel(const QString& value, const TextStyle style, QWidget* parent) : QLabel(value, parent) {
  connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
  setTextFormat(Qt::PlainText);
  setTextInteractionFlags(Qt::NoTextInteraction);
  setTextStyle(style);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}
void TextLabel::setTextStyle(const TextStyle style) {
  QFont type = font();
  int pixels = design::fontSize;
  m_lineHeight = 1.6;
  type.setWeight(QFont::Normal);
  type.setLetterSpacing(QFont::AbsoluteSpacing, 0);
  switch (style) {
    case TextStyle::Body:
      break;
    case TextStyle::Label:
      pixels = 13;
      break;
    case TextStyle::Caption:
      pixels = 12;
      break;
    case TextStyle::Eyebrow:
      pixels = 11;
      break;
    case TextStyle::Subtitle:
      pixels = 13;
      m_lineHeight = 1.7;
      break;
    case TextStyle::Description:
      pixels = 13;
      m_lineHeight = 1.9;
      break;
    case TextStyle::Title:
      pixels = 25;
      type.setWeight(QFont::DemiBold);
      type.setLetterSpacing(QFont::AbsoluteSpacing, -.55);
      m_lineHeight = 1.4;
      break;
    case TextStyle::Section:
      pixels = 13;
      type.setWeight(QFont::DemiBold);
      break;
    case TextStyle::Metric:
      pixels = 25;
      type.setWeight(QFont::DemiBold);
      m_lineHeight = 1.3;
      break;
    case TextStyle::MessageTitle:
      pixels = design::messageTitleFontSize;
      type.setWeight(QFont::DemiBold);
      m_lineHeight = 1.5;
      break;
  }
  type.setPixelSize(pixels);
  setFont(type);
  updateGeometry();
  update();
}
void TextLabel::setColorRole(const design::Color role) {
  m_colorRole = role;
  update();
}
void TextLabel::setLineHeight(const qreal height) {
  if (height <= 0 || qFuzzyCompare(m_lineHeight, height)) return;
  m_lineHeight = height;
  updateGeometry();
  update();
}
void TextLabel::setVerticalReference(const TextVerticalReference reference) {
  m_verticalReference = reference;
  update();
}
void TextLabel::setVerticalAlignmentOffset(const qreal offset) {
  if (qFuzzyCompare(m_verticalOffset, offset)) return;
  m_verticalOffset = offset;
  update();
}
QRectF TextLabel::firstLineBoxBounds() const {
  const auto bounds = contentsRect();
  const auto height = measure(bounds.width()).height();
  const qreal offset = alignment().testFlag(Qt::AlignVCenter)  ? (bounds.height() - height) / 2.0
                       : alignment().testFlag(Qt::AlignBottom) ? bounds.height() - height
                                                               : 0;
  return QRectF(bounds.left(), bounds.top() + offset, bounds.width(), font().pixelSize() * m_lineHeight);
}
QRectF TextLabel::firstLineInkBounds() const { return textBounds(true, TextVerticalReference::GlyphBounds); }
QRectF TextLabel::inkBounds() const { return textBounds(false, TextVerticalReference::GlyphBounds); }
QRectF TextLabel::alignmentBounds(const bool firstLineOnly) const { return textBounds(firstLineOnly, m_verticalReference); }
QRectF TextLabel::textBounds(const bool firstLineOnly, const TextVerticalReference reference) const {
  const auto bounds = contentsRect();
  const QString value = m_elide == Qt::ElideNone ? text() : fontMetrics().elidedText(text(), m_elide, bounds.width());
  QTextLayout layout(layoutValue(value), font());
  QTextOption option;
  option.setAlignment(QStyle::visualAlignment(layoutDirection(), alignment()) & Qt::AlignHorizontal_Mask);
  option.setTextDirection(layoutDirection());
  option.setWrapMode(wordWrap() ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
  layout.setTextOption(option);
  const auto size = layoutText(layout, bounds.width(), font().pixelSize() * m_lineHeight);
  if (layout.lineCount() == 0) return {};
  const auto line = layout.lineAt(0);
  const auto origin = rasterOrigin(*this);
  const qreal scale = devicePixelRatioF();
  QTransform device;
  device.scale(scale, scale);
  device.translate(origin.x(), origin.y());
  const qreal top = alignedLayoutTop(layout, size, bounds, alignment(), m_verticalReference, m_verticalOffset, device);
  if (reference == TextVerticalReference::FontMetrics) return textCapBounds(layout, firstLineOnly).translated(bounds.left(), top).intersected(bounds);
  // A single line can have visible glyph edges beyond its typographic strut.
  const auto clip =
      firstLineOnly && layout.lineCount() > 1 ? QRectF(bounds.left(), top + line.y(), bounds.width(), line.height()).intersected(bounds) : QRectF(bounds);
  return textRasterBounds(layout, QPointF(bounds.left(), top), clip, device);
}
void TextLabel::setElideMode(const Qt::TextElideMode mode) {
  m_elide = mode;
  updateGeometry();
  update();
}
void TextLabel::setText(const QString& value) {
  if (text() == value) return;
  QLabel::setText(value);
  updateGeometry();
  update();
}
bool TextLabel::hasHeightForWidth() const { return wordWrap(); }
QSize TextLabel::measure(const int width) const {
  const auto margins = contentsMargins();
  QTextLayout layout(layoutValue(text()), font());
  QTextOption option;
  option.setWrapMode(wordWrap() ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
  layout.setTextOption(option);
  const int availableWidth = std::max(1, std::min(width, maximumWidth()) - margins.left() - margins.right());
  const QSizeF size = layoutText(layout, availableWidth, font().pixelSize() * m_lineHeight);
  return QSize(qCeil(size.width()) + margins.left() + margins.right(), qCeil(size.height()) + margins.top() + margins.bottom());
}
QSize TextLabel::sizeHint() const { return measure(100000); }
QSize TextLabel::minimumSizeHint() const {
  const auto full = measure(100000);
  return {wordWrap() || m_elide != Qt::ElideNone ? 0 : full.width(),
          wordWrap() ? static_cast<int>(std::ceil(font().pixelSize() * m_lineHeight)) : full.height()};
}
int TextLabel::heightForWidth(const int width) const { return measure(width).height(); }
void TextLabel::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setLayoutDirection(layoutDirection());
  const auto role = m_colorRole ? m_colorRole : m_toneRole;
  const QColor ink = role ? DesignSystem::instance().color(*role) : palette().color(isEnabled() ? QPalette::Active : QPalette::Disabled, QPalette::WindowText);
  painter.setPen(ink);
  painter.setFont(font());
  const QRect bounds = contentsRect();
  const QString value = m_elide == Qt::ElideNone ? text() : fontMetrics().elidedText(text(), m_elide, bounds.width());
  QTextLayout layout(layoutValue(value), font());
  QTextOption option;
  option.setAlignment(QStyle::visualAlignment(layoutDirection(), alignment()) & Qt::AlignHorizontal_Mask);
  option.setTextDirection(layoutDirection());
  option.setWrapMode(wordWrap() ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
  layout.setTextOption(option);
  const QSizeF size = layoutText(layout, bounds.width(), font().pixelSize() * m_lineHeight);
  const qreal y = alignedLayoutTop(layout, size, bounds, alignment(), m_verticalReference, m_verticalOffset, painter.deviceTransform());
  painter.setClipRect(bounds);
  layout.draw(&painter, QPointF(bounds.left(), y));
}
bool TextLabel::event(QEvent* event) {
  if (event->type() == QEvent::DynamicPropertyChange && static_cast<QDynamicPropertyChangeEvent*>(event)->propertyName() == "designTone") {
    const auto tone = property("designTone").toString();
    using design::Color;
    m_toneRole.reset();
    if (tone == QLatin1String("muted"))
      m_toneRole = Color::Muted;
    else if (tone == QLatin1String("faint"))
      m_toneRole = Color::Faint;
    else if (tone == QLatin1String("accent"))
      m_toneRole = Color::Accent;
    else if (tone == QLatin1String("success"))
      m_toneRole = Color::Success;
    else if (tone == QLatin1String("warning"))
      m_toneRole = Color::Warning;
    else if (tone == QLatin1String("danger"))
      m_toneRole = Color::Danger;
    update();
  }
  return QLabel::event(event);
}
}  // namespace uwf::ui
#include "moc_TextLabel.cpp"
