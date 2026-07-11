/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "HoverHintController.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QDialog>
#include <QHoverEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QTabBar>
#include <QWidget>

namespace uwf::ui {

HoverHintController::HoverHintController(QWidget* rootWindow, QObject* parent) : QObject(parent), m_rootWindow(rootWindow) { qApp->installEventFilter(this); }

bool HoverHintController::eventFilter(QObject* obj, QEvent* ev) {
  constexpr int kHoverRestoreMs = 120;
  if (!m_target) return QObject::eventFilter(obj, ev);

  // 模态对话框使用原生 tooltip；其 QObject 父链仍能追溯到所属 QDialog。
  for (QObject* p = obj; p; p = p->parent()) {
    if (qobject_cast<QDialog*>(p)) return QObject::eventFilter(obj, ev);
  }

  const auto type = ev->type();
  // 主窗口内禁止原生气泡，说明统一显示到右下角面板。
  if (type == QEvent::ToolTip) return true;

  if (auto* menu = qobject_cast<QMenu*>(obj)) {
    if (type == QEvent::MouseMove || type == QEvent::HoverMove || type == QEvent::Enter || type == QEvent::HoverEnter) {
      const QAction* act = menu->activeAction();
      const QString tip = act ? act->toolTip() : QString();
      if (act && !tip.isEmpty() && tip != act->text())
        m_target->show(tip);
      else
        m_target->restoreAfter(kHoverRestoreMs);
    } else if (type == QEvent::Leave || type == QEvent::HoverLeave || type == QEvent::Hide) {
      m_target->restoreAfter(kHoverRestoreMs);
    }
    return QObject::eventFilter(obj, ev);
  }

  if (type == QEvent::Enter || type == QEvent::HoverEnter || type == QEvent::MouseMove || type == QEvent::HoverMove) {
    auto* w = qobject_cast<QWidget*>(obj);
    if (!w) return QObject::eventFilter(obj, ev);

    if (auto* bar = qobject_cast<QTabBar*>(w)) {
      QPoint pos;
      if (auto* me = dynamic_cast<QMouseEvent*>(ev))
        pos = me->pos();
      else if (auto* he = dynamic_cast<QHoverEvent*>(ev))
        pos = he->position().toPoint();
      else
        pos = bar->mapFromGlobal(QCursor::pos());
      const int idx = bar->tabAt(pos);
      if (idx >= 0) {
        const QString tip = bar->tabToolTip(idx);
        if (!tip.isEmpty()) {
          m_target->show(tip);
          return QObject::eventFilter(obj, ev);
        }
      }
    }

    if (auto* view = qobject_cast<QAbstractItemView*>(w->parentWidget()); view && view->viewport() == w) {
      QPoint pos;
      if (auto* me = dynamic_cast<QMouseEvent*>(ev))
        pos = me->pos();
      else if (auto* he = dynamic_cast<QHoverEvent*>(ev))
        pos = he->position().toPoint();
      else
        pos = w->mapFromGlobal(QCursor::pos());
      const QModelIndex idx = view->indexAt(pos);
      if (idx.isValid()) {
        const QString tip = idx.data(Qt::ToolTipRole).toString();
        if (!tip.isEmpty()) {
          m_target->show(tip);
          return QObject::eventFilter(obj, ev);
        }
      }
    }

    QWidget* cur = w;
    while (cur && cur->toolTip().isEmpty() && cur != m_rootWindow) cur = cur->parentWidget();
    if (cur && !cur->toolTip().isEmpty()) m_target->show(cur->toolTip());
  } else if (type == QEvent::Leave || type == QEvent::HoverLeave) {
    m_target->restoreAfter(kHoverRestoreMs);
  }
  return QObject::eventFilter(obj, ev);
}

}  // namespace uwf::ui
