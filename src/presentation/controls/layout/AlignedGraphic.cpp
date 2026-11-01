// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/layout/AlignedGraphic.h"

#include <QAbstractButton>
#include <QEvent>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "presentation/controls/feedback/BusyIndicator.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"

namespace uwf::ui {
void setGraphicVerticalAlignmentOffset(QWidget& graphic, const qreal offset) {
  if (auto* icon = dynamic_cast<IconView*>(&graphic)) {
    icon->setVerticalAlignmentOffset(offset);
  } else if (auto* spinner = qobject_cast<BusyIndicator*>(&graphic)) {
    spinner->setVerticalAlignmentOffset(offset);
  } else if (auto* text = qobject_cast<TextLabel*>(&graphic)) {
    text->setVerticalAlignmentOffset(offset);
  } else if (auto* button = qobject_cast<QAbstractButton*>(&graphic)) {
    const auto previous = button->property("designVerticalAlignmentOffset");
    if (!previous.isValid() || !qFuzzyCompare(previous.toReal(), offset)) {
      button->setProperty("designVerticalAlignmentOffset", offset);
      button->update();
    }
  }
}
AlignedGraphic::AlignedGraphic(QWidget* graphic, TextLabel* reference, QWidget* parent) : AlignedGraphic(graphic, reference, TextAnchor::FirstLine, parent) {}
AlignedGraphic::AlignedGraphic(QWidget* graphic, QWidget* reference, const TextAnchor anchor, QWidget* parent) : QWidget(parent), m_graphic(graphic) {
  if (!m_graphic) throw std::invalid_argument("an aligned graphic requires a widget");
  const bool hidden = m_graphic->isHidden() && m_graphic->testAttribute(Qt::WA_WState_ExplicitShowHide);
  m_graphic->setParent(this);
  syncGraphicSize();
  if (!hidden) m_graphic->show();
  setVisible(!hidden);
  m_graphic->installEventFilter(this);
  setReferenceWidget(reference, anchor);
}
void AlignedGraphic::syncGraphicSize() {
  const auto size = m_graphic->sizeHint().expandedTo(m_graphic->minimumSize()).boundedTo(m_graphic->maximumSize());
  if (m_graphic->size() != size) m_graphic->resize(size);
  if (minimumWidth() != size.width() || maximumWidth() != size.width()) setFixedWidth(size.width());
  if (minimumHeight() != size.height()) setMinimumHeight(size.height());
}
void AlignedGraphic::setReference(TextLabel* reference) { setReferenceWidget(reference, TextAnchor::FirstLine); }
void AlignedGraphic::setReferenceWidget(QWidget* reference, const TextAnchor anchor) {
  if (m_reference) m_reference->removeEventFilter(this);
  m_reference = reference;
  m_anchor = anchor;
  if (m_reference) m_reference->installEventFilter(this);
  alignGraphic();
}
QRectF AlignedGraphic::referenceBounds() const {
  if (!m_reference) return {};
  if (m_anchor == TextAnchor::FirstLine) {
    const auto* text = qobject_cast<TextLabel*>(m_reference);
    if (!text) return {};
    const auto bounds = text->alignmentBounds(true);
    return bounds.isEmpty() ? text->firstLineBoxBounds() : bounds;
  }
  QRectF block;
  const auto include = [&](TextLabel* text) {
    if (!text->isVisibleTo(m_reference)) return;
    block = block.united(text->alignmentBounds().translated(text->mapTo(m_reference, QPoint())));
  };
  if (auto* text = qobject_cast<TextLabel*>(m_reference)) include(text);
  for (auto texts = m_reference->findChildren<TextLabel*>(); auto* text : std::as_const(texts)) include(text);
  return block.isEmpty() ? QRectF(m_reference->rect()) : block;
}
void AlignedGraphic::alignGraphic() {
  if (!m_reference) return;
  const auto line = referenceBounds();
  const auto origin = mapFromGlobal(m_reference->mapToGlobal(QPoint()));
  const qreal top = origin.y() + line.center().y() - m_graphic->height() / 2.0;
  const int position = std::clamp(qRound(top), 0, std::max(0, height() - m_graphic->height()));
  m_graphic->move(0, position);
  const qreal center = origin.y() + line.center().y() - position;
  setGraphicVerticalAlignmentOffset(*m_graphic, center - m_graphic->height() / 2.0);
  for (auto widgets = m_graphic->findChildren<QWidget*>(); auto* widget : std::as_const(widgets)) {
    if (auto* icon = dynamic_cast<IconView*>(widget)) {
      const auto local = icon->mapTo(m_graphic, QPoint());
      setGraphicVerticalAlignmentOffset(*icon, center - local.y() - icon->height() / 2.0);
    }
  }
}
bool AlignedGraphic::event(QEvent* event) {
  const bool handled = QWidget::event(event);
  if (event->type() == QEvent::Show || event->type() == QEvent::LayoutRequest || event->type() == QEvent::Paint) syncGraphicSize();
  if (event->type() == QEvent::Resize || event->type() == QEvent::Move || event->type() == QEvent::Show || event->type() == QEvent::LayoutRequest ||
      event->type() == QEvent::Paint)
    alignGraphic();
  return handled;
}
bool AlignedGraphic::eventFilter(QObject* object, QEvent* event) {
  if (object == m_graphic && event->type() == QEvent::ShowToParent) show();
  if (object == m_graphic && event->type() == QEvent::HideToParent) hide();
  if (object == m_graphic &&
      (event->type() == QEvent::FontChange || event->type() == QEvent::LayoutRequest || event->type() == QEvent::Show || event->type() == QEvent::Paint))
    syncGraphicSize();
  if (event->type() == QEvent::Resize || event->type() == QEvent::Move || event->type() == QEvent::Show || event->type() == QEvent::FontChange ||
      event->type() == QEvent::Paint || event->type() == QEvent::LayoutRequest)
    alignGraphic();
  return QWidget::eventFilter(object, event);
}
}  // namespace uwf::ui
