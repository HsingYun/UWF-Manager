// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ItemHighlight.h"

#include <QEvent>
#include <QVariantAnimation>
#include <QWidget>
#include <utility>

#include "DesignSystem.h"

namespace uwf::ui {
ItemHighlight::ItemHighlight(QWidget* viewport) : QObject(viewport) {
  setObjectName(QStringLiteral("designItemHighlight"));
  viewport->installEventFilter(this);
}
qreal ItemHighlight::opacity(const QWidget* viewport, const QPoint& itemPosition, const bool active) {
  if (!viewport) return active ? 1.0 : 0.0;
  auto* controller = viewport->findChild<ItemHighlight*>(QStringLiteral("designItemHighlight"), Qt::FindDirectChildrenOnly);
  if (!controller) controller = new ItemHighlight(const_cast<QWidget*>(viewport));
  auto found = controller->m_items.find(itemPosition);
  if (found == controller->m_items.end()) {
    if (!active) return 0;
    auto* animation = new QVariantAnimation(controller);
    animation->setDuration(design::hoverDurationMs);
    QEasingCurve easing(QEasingCurve::BezierSpline);
    easing.addCubicBezierSegment(QPointF(0, 0), QPointF(.58, 1), QPointF(1, 1));
    animation->setEasingCurve(easing);
    connect(animation, &QVariantAnimation::valueChanged, viewport, [viewport] { const_cast<QWidget*>(viewport)->update(); });
    connect(animation, &QVariantAnimation::finished, controller, [controller, itemPosition, animation] {
      const auto item = controller->m_items.find(itemPosition);
      if (item != controller->m_items.end() && item->animation == animation && !item->active) {
        controller->m_items.erase(item);
        animation->deleteLater();
      }
    });
    found = controller->m_items.insert(itemPosition, {animation, false});
  }
  auto& transition = found.value();
  if (transition.active != active) {
    const qreal current = transition.animation->currentValue().toReal();
    transition.animation->stop();
    transition.animation->setStartValue(current);
    transition.animation->setEndValue(active ? 1.0 : 0.0);
    transition.active = active;
    transition.animation->start();
  }
  return transition.animation->currentValue().toReal();
}
bool ItemHighlight::eventFilter(QObject*, QEvent* event) {
  if (event->type() == QEvent::Hide) {
    for (const auto& item : std::as_const(m_items)) {
      item.animation->stop();
      item.animation->deleteLater();
    }
    m_items.clear();
  }
  return false;
}
}  // namespace uwf::ui
#include "moc_ItemHighlight.cpp"
