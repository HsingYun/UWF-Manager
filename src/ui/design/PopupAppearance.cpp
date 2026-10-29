// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PopupAppearance.h"

#include <QApplication>
#include <QEvent>
#include <QMenu>
#include <QPainter>
#include <QWidget>

#include "ControlShape.h"
#include "DesignSystem.h"
#include "SurfaceShadow.h"

namespace uwf::ui {
namespace {
class PopupSurface final : public QObject {
 public:
  explicit PopupSurface(QWidget* widget) : QObject(widget), m_popup(widget) { widget->installEventFilter(this); }

 protected:
  bool eventFilter(QObject* object, QEvent* event) override {
    if (event->type() == QEvent::Paint && qobject_cast<QMenu*>(m_popup)) {
      QPainter painter(m_popup);
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.fillRect(m_popup->rect(), Qt::transparent);
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
      paintPopupSurface(painter, m_popup->rect());
    }
    return QObject::eventFilter(object, event);
  }

 private:
  QWidget* m_popup;
};
}  // namespace
void configurePopupAppearance(QWidget* popup) {
  if (popup->property("designPopupSurface").toBool()) return;
  popup->setProperty("designPopupSurface", true);
  if (qobject_cast<QMenu*>(popup)) popup->setMinimumWidth(216);
  popup->setLayoutDirection(QApplication::layoutDirection());
  popup->setWindowFlags(popup->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
  popup->setAttribute(Qt::WA_TranslucentBackground);
  popup->setAutoFillBackground(false);
  new PopupSurface(popup);
  installSurfaceShadow(popup, ShadowKind::Popup);
}
void paintPopupSurface(QPainter& painter, const QRectF& bounds) {
  auto& system = DesignSystem::instance();
  paintControlShape(painter, bounds, design::menuRadius, system.color(design::Color::Surface), system.color(design::Color::Line));
}
}  // namespace uwf::ui
