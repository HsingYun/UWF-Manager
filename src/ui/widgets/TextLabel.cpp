// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "TextLabel.h"

#include <QEvent>
#include <QPainter>
#include <QStyle>
#include <QTextLayout>
#include <algorithm>
#include <cmath>

#include "ui/design/DesignSystem.h"
#include "ui/design/TextPainting.h"
namespace uwf::ui {
namespace {
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
QRectF TextLabel::firstLineBoxBounds() const {
  const auto bounds = contentsRect();
  const auto height = measure(bounds.width()).height();
  const qreal offset = alignment().testFlag(Qt::AlignVCenter)  ? (bounds.height() - height) / 2.0
                       : alignment().testFlag(Qt::AlignBottom) ? bounds.height() - height
                                                               : 0;
  return QRectF(bounds.left(), bounds.top() + offset, bounds.width(), font().pixelSize() * m_lineHeight);
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
  if (layout.lineCount() == 1 && alignment().testFlag(Qt::AlignVCenter)) {
    paintTextLine(painter, bounds, value, font(), alignment(), false, m_verticalReference);
    return;
  }
  qreal y = bounds.top();
  if (alignment() & Qt::AlignVCenter) {
    const auto inkBounds = m_verticalReference == TextVerticalReference::GlyphBounds ? textInkBounds(layout) : QRectF{};
    y += inkBounds.isEmpty() ? (bounds.height() - size.height()) / 2.0 : bounds.height() / 2.0 - inkBounds.center().y();
  } else if (alignment() & Qt::AlignBottom)
    y += bounds.height() - size.height();
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
