// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PopupPlacement.h"

#include <QApplication>
#include <QEvent>
#include <QPointer>
#include <QScopedValueRollback>
#include <QScreen>
#include <QWidget>
#include <algorithm>

#include "design/DesignTokens.h"

namespace uwf::ui {
namespace {
class PopupAnchor final : public QObject {
 public:
  explicit PopupAnchor(QWidget* popup) : QObject(popup), m_popup(popup) {
    setObjectName(QStringLiteral("designPopupAnchor"));
    qApp->installEventFilter(this);
  }
  void setAnchor(QWidget* anchor) {
    m_anchor = anchor;
    reposition();
  }

 protected:
  bool eventFilter(QObject* object, QEvent* event) override {
    if (m_positioning || !m_anchor || !m_popup) return false;
    const auto type = event->type();
    if (type != QEvent::Move && type != QEvent::Resize && type != QEvent::Show && type != QEvent::Hide && type != QEvent::LayoutDirectionChange &&
        type != QEvent::ScreenChangeInternal && type != QEvent::EnabledChange)
      return false;
    auto* widget = qobject_cast<QWidget*>(object);
    if (!widget || (widget != m_popup && widget != m_anchor && !widget->isAncestorOf(m_anchor))) return false;
    if (widget != m_popup && (type == QEvent::Hide || (type == QEvent::EnabledChange && !m_anchor->isEnabled())))
      m_popup->hide();
    else
      reposition();
    return false;
  }

 private:
  void reposition() {
    if (m_positioning || !m_popup || !m_anchor || !m_popup->isVisible()) return;
    const QScopedValueRollback<bool> guard(m_positioning, true);
    const QRect geometry = anchoredPopupGeometry(QRect(m_anchor->mapToGlobal(QPoint()), m_anchor->size()), m_popup->size(),
                                                 m_anchor->screen()->availableGeometry(), m_anchor->layoutDirection());
    if (m_popup->geometry() != geometry) m_popup->setGeometry(geometry);
  }
  QPointer<QWidget> m_popup;
  QPointer<QWidget> m_anchor;
  bool m_positioning = false;
};
}  // namespace
void bindPopupToAnchor(QWidget* popup, QWidget* anchor) {
  auto* controller = popup->findChild<QObject*>(QStringLiteral("designPopupAnchor"), Qt::FindDirectChildrenOnly);
  if (!controller && !anchor) return;
  if (!controller) controller = new PopupAnchor(popup);
  static_cast<PopupAnchor*>(controller)->setAnchor(anchor);
}
QRect anchoredPopupGeometry(const QRect& anchor, QSize preferredSize, const QRect& workArea, const Qt::LayoutDirection direction) {
  const int inset = std::min({design::popupScreenInset, std::max(0, (workArea.width() - 1) / 2), std::max(0, (workArea.height() - 1) / 2)});
  const QRect available = workArea.adjusted(inset, inset, -inset, -inset);
  preferredSize = preferredSize.expandedTo(QSize(1, 1)).boundedTo(available.size());
  int left = direction == Qt::RightToLeft ? anchor.right() + 1 - preferredSize.width() : anchor.left();
  int top = anchor.bottom() + 1 + design::popupGap;
  if (top + preferredSize.height() > available.bottom() + 1) top = anchor.top() - preferredSize.height() - design::popupGap;
  left = std::clamp(left, available.left(), available.right() + 1 - preferredSize.width());
  top = std::clamp(top, available.top(), available.bottom() + 1 - preferredSize.height());
  return {QPoint(left, top), preferredSize};
}
}  // namespace uwf::ui
