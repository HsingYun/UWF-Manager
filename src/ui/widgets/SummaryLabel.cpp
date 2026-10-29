// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SummaryLabel.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>

namespace uwf::ui {
namespace {
constexpr auto separator = QStringView(u" · ");
}
SummaryLabel::SummaryLabel(const QStringList& parts, QWidget* parent) : TextLabel({}, TextStyle::Eyebrow, parent) {
  setElideMode(Qt::ElideRight);
  setCursor(Qt::PointingHandCursor);
  setFocusPolicy(Qt::StrongFocus);
  setParts(parts);
}
void SummaryLabel::setParts(const QStringList& parts) {
  m_parts.clear();
  for (const auto& part : parts)
    if (!part.isEmpty()) m_parts.append(part);
  m_fullText = m_parts.join(separator);
  setToolTip(m_fullText);
#if QT_CONFIG(accessibility)
  setAccessibleName(m_fullText);
#endif
  updateText();
  updateGeometry();
}
QSize SummaryLabel::sizeHint() const {
  auto size = TextLabel::sizeHint();
  const auto margins = contentsMargins();
  size.setWidth(fontMetrics().horizontalAdvance(m_fullText) + margins.left() + margins.right());
  return size;
}
void SummaryLabel::updateText() {
  auto parts = m_parts;
  while (parts.size() > 1 && fontMetrics().horizontalAdvance(parts.join(separator)) > contentsRect().width()) parts.removeLast();
  const auto display = parts.join(separator);
  if (text() != display) TextLabel::setText(display);
}
void SummaryLabel::resizeEvent(QResizeEvent* event) {
  TextLabel::resizeEvent(event);
  updateText();
}
void SummaryLabel::changeEvent(QEvent* event) {
  TextLabel::changeEvent(event);
  if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange || event->type() == QEvent::ContentsRectChange) updateText();
}
void SummaryLabel::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    m_pressed = true;
    event->accept();
  } else {
    TextLabel::mousePressEvent(event);
  }
}
void SummaryLabel::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    const bool activate = m_pressed && rect().contains(event->position().toPoint());
    m_pressed = false;
    event->accept();
    if (activate) emit activated(m_fullText);
  } else {
    TextLabel::mouseReleaseEvent(event);
  }
}
void SummaryLabel::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space) {
    event->accept();
    if (!event->isAutoRepeat()) emit activated(m_fullText);
  } else {
    TextLabel::keyPressEvent(event);
  }
}
}  // namespace uwf::ui

#include "moc_SummaryLabel.cpp"
