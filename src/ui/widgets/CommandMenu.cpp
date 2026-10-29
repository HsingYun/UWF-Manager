// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommandMenu.h"

#include <QAbstractButton>
#include <QAbstractListModel>
#include <QAction>
#include <QHideEvent>
#include <QKeyEvent>
#include <QListView>
#include <QPainter>
#include <QScreen>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionMenuItem>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <algorithm>
#include <utility>

#include "ui/design/DesignSystem.h"
#include "ui/design/FocusIndicator.h"
#include "ui/design/PopupAppearance.h"
#include "ui/design/PopupPlacement.h"
#include "ui/design/TextPainting.h"
namespace uwf::ui {
namespace {
class ActionListModel final : public QAbstractListModel {
 public:
  ActionListModel(const QList<QAction*>& actions, QObject* parent) : QAbstractListModel(parent) {
    for (auto* action : actions) {
      if (!action->isVisible()) continue;
      const int row = static_cast<int>(m_actions.size());
      m_actions.append(action);
      connect(action, &QAction::changed, this, [this, row] { emit dataChanged(index(row), index(row)); });
    }
  }
  int rowCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : static_cast<int>(m_actions.size()); }
  QAction* action(const QModelIndex& index) const { return index.isValid() && index.row() < m_actions.size() ? m_actions[index.row()].data() : nullptr; }
  QVariant data(const QModelIndex& index, int role) const override {
    const auto* item = action(index);
    if (!item) return {};
    if (role == Qt::DisplayRole) return item->text();
    if (role == Qt::DecorationRole) return item->icon();
    if (role == Qt::CheckStateRole && item->isCheckable()) return item->isChecked() ? Qt::Checked : Qt::Unchecked;
    return {};
  }
  Qt::ItemFlags flags(const QModelIndex& index) const override {
    const auto* item = action(index);
    return item && item->isEnabled() && !item->isSeparator() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
  }

 private:
  QList<QPointer<QAction>> m_actions;
};
class ActionDelegate final : public QStyledItemDelegate {
 public:
  explicit ActionDelegate(CommandMenu* menu, QObject* parent) : QStyledItemDelegate(parent), m_menu(menu) {}
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    const auto* actions = static_cast<const ActionListModel*>(index.model());
    const auto* action = actions->action(index);
    if (action && action->isSeparator()) return {0, 9};
    return {QFontMetrics(option.font).horizontalAdvance(index.data().toString().remove(u'&')) + 62, design::menuRowHeight};
  }
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    const auto* action = static_cast<const ActionListModel*>(index.model())->action(index);
    if (!action) return;
    auto font = option.font;
    font.setPixelSize(action->property("designMenuHeading").toBool() ? 11 : 12);
    if (action->property("designMenuHeading").toBool()) {
      font.setWeight(QFont::DemiBold);
      painter->save();
      painter->setPen(DesignSystem::instance().color(design::Color::Muted));
      paintTextLine(*painter, option.rect.adjusted(10, 0, -10, 0), action->text(), font);
      painter->restore();
      return;
    }
    QStyleOptionMenuItem item;
    item.initFrom(option.widget);
    item.rect = option.rect;
    item.direction = option.direction;
    item.font = font;
    item.fontMetrics = QFontMetrics(font);
    item.text = action->text();
    item.icon = action->icon();
    item.menuItemType = action->isSeparator() ? QStyleOptionMenuItem::Separator : QStyleOptionMenuItem::Normal;
    item.checkType = action->isCheckable() ? QStyleOptionMenuItem::NonExclusive : QStyleOptionMenuItem::NotCheckable;
    item.checked = action->isChecked();
    item.state.setFlag(QStyle::State_Enabled, action->isEnabled());
    item.state.setFlag(QStyle::State_Selected, option.state.testFlag(QStyle::State_MouseOver) ||
                                                   (FocusIndicator::keyboardNavigation() && option.state.testFlag(QStyle::State_Selected)));
    m_menu->style()->drawControl(QStyle::CE_MenuItem, &item, painter, m_menu);
  }

 private:
  CommandMenu* m_menu;
};
class ActionListView final : public QListView {
 public:
  explicit ActionListView(CommandMenu* menu) : QListView(menu), m_menu(menu) {
    setLayoutMode(QListView::Batched);
    setBatchSize(64);
  }

 protected:
  void keyPressEvent(QKeyEvent* event) override {
    if (event->key() == Qt::Key_Escape) {
      m_menu->close();
      return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space) {
      if (currentIndex().flags().testFlag(Qt::ItemIsEnabled)) emit activated(currentIndex());
      return;
    }
    QListView::keyPressEvent(event);
  }

 private:
  CommandMenu* m_menu;
};
}  // namespace
CommandMenu::CommandMenu(QWidget* parent) : CommandMenu(QString{}, parent) {}
CommandMenu::CommandMenu(const QString& title, QWidget* parent) : QMenu(title, parent), m_anchor(qobject_cast<QAbstractButton*>(parent)) {
  configurePopupAppearance(this);
  bindPopupToAnchor(this, m_anchor);
}
void CommandMenu::setAnchor(QWidget* anchor) {
  m_anchor = anchor;
  bindPopupToAnchor(this, anchor);
}
void CommandMenu::setMaximumVisibleHeight(const int height) { m_visibleHeight = height; }
void CommandMenu::showEvent(QShowEvent* event) {
  const auto available = screen()->availableGeometry();
  const int maximum = std::min(m_visibleHeight, available.height() - 40);
  if (maximum > 12 && sizeHint().height() > maximum && m_scrollActions.isEmpty()) {
    const auto commands = actions();
    // QWidgetAction menus already manage their own body (for example tray usage).
    const bool flat = std::none_of(commands.begin(), commands.end(), [](QAction* action) { return action->menu() || qobject_cast<QWidgetAction*>(action); });
    if (flat) {
      const int width = sizeHint().width();
      auto* view = new ActionListView(this);
      view->setObjectName(QStringLiteral("scrollableMenuItems"));
      view->setFrameShape(QFrame::NoFrame);
      view->setEditTriggers(QAbstractItemView::NoEditTriggers);
      view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      view->setMouseTracking(true);
      auto* model = new ActionListModel(commands, view);
      view->setModel(model);
      view->setItemDelegate(new ActionDelegate(this, view));
      view->setFixedSize(width - 12, maximum - 12);
      QModelIndex checked;
      for (int row = 0; row < model->rowCount(); ++row)
        if (auto* action = model->action(model->index(row)); action && action->isChecked()) checked = model->index(row);
      for (auto* action : commands) {
        m_scrollActions.append(action);
        removeAction(action);
      }
      auto* body = new QWidgetAction(this);
      body->setDefaultWidget(view);
      m_scrollAction = body;
      addAction(body);
      const auto select = [this, model, body = QPointer<QAction>(body)](const QModelIndex& index) {
        const QPointer<QAction> action = model->action(index);
        if (!action || !action->isEnabled()) return;
        // Finish the view's input event before either teardown or application
        // code can enter a nested event loop and destroy that view. Keep close
        // and trigger together so exec() cannot return before the command runs.
        QTimer::singleShot(0, this, [this, body, action] {
          if (!body || m_scrollAction != body || !isVisible() || !action || !action->isEnabled()) return;
          const QPointer<CommandMenu> guard(this);
          close();
          if (guard && action && action->isEnabled()) action->trigger();
        });
      };
      connect(view, &QListView::clicked, this, select);
      connect(view, &QListView::activated, this, select);
      connect(view, &QListView::entered, view, &QListView::setCurrentIndex);
      view->setCurrentIndex(checked.isValid() ? checked : model->index(0));
      resize(width, maximum);
      QTimer::singleShot(0, view, [view] {
        if (!view->isVisible()) return;
        view->setFocus(Qt::PopupFocusReason);
        view->scrollTo(view->currentIndex());
      });
    }
  }
  if (m_anchor && m_anchor->isVisible())
    setGeometry(anchoredPopupGeometry(QRect(m_anchor->mapToGlobal(QPoint()), m_anchor->size()), size(), m_anchor->screen()->availableGeometry(),
                                      m_anchor->layoutDirection()));
  QMenu::showEvent(event);
}
void CommandMenu::hideEvent(QHideEvent* event) {
  QMenu::hideEvent(event);
  if (!m_scrollAction) return;
  removeAction(m_scrollAction);
  m_scrollAction->deleteLater();
  m_scrollAction.clear();
  for (const auto& action : std::as_const(m_scrollActions))
    if (action) addAction(action);
  m_scrollActions.clear();
}
}  // namespace uwf::ui
#include "moc_CommandMenu.cpp"
