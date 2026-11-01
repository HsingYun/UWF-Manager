// SPDX-License-Identifier: GPL-3.0-or-later
#include "presentation/controls/navigation/ContextMenus.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextEdit>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>

#include "presentation/controls/navigation/CommandMenu.h"

namespace uwf::ui {
namespace {
class ContextMenus final : public QObject {
 public:
  using QObject::QObject;

 protected:
  bool eventFilter(QObject* object, QEvent* event) override {
    if (event->type() != QEvent::ContextMenu) return false;
    auto* target = qobject_cast<QWidget*>(object);
    if (!target) return false;
    const QPointer lifetime(target);
    auto* context = static_cast<QContextMenuEvent*>(event);
    auto menu = createContextMenu(*target, *context, writeClipboardText);
    if (!lifetime) {
      context->accept();
      return true;
    }
    if (!menu) return false;
    QWidget* owner = target;
    if (auto* richEditor = qobject_cast<QTextEdit*>(target->parentWidget()); richEditor && richEditor->viewport() == target)
      owner = richEditor;
    else if (auto* plainEditor = qobject_cast<QPlainTextEdit*>(target->parentWidget()); plainEditor && plainEditor->viewport() == target)
      owner = plainEditor;
    const QPoint position = context->reason() == QContextMenuEvent::Keyboard ? owner->mapToGlobal(owner->rect().center()) : context->globalPos();
    menu->exec(position);
    context->accept();
    return true;
  }
};
}  // namespace
std::unique_ptr<CommandMenu> createContextMenu(QWidget& target, const QContextMenuEvent& event, ClipboardTextWriter writer) {
  if (!writer) throw std::invalid_argument("Context menus require a clipboard text writer");
  for (auto* parent = &target; parent; parent = parent->parentWidget())
    if (parent->property("windowDragSurface").toBool()) return {};
  QWidget* owner = &target;
  std::unique_ptr<QMenu> standard;
  if (auto* lineEditor = qobject_cast<QLineEdit*>(&target))
    standard.reset(lineEditor->createStandardContextMenu());
  else if (auto* richEditor = qobject_cast<QTextEdit*>(target.parentWidget()); richEditor && richEditor->viewport() == &target) {
    owner = richEditor;
    standard.reset(richEditor->createStandardContextMenu(event.pos()));
  } else if (auto* plainEditor = qobject_cast<QPlainTextEdit*>(target.parentWidget()); plainEditor && plainEditor->viewport() == &target) {
    owner = plainEditor;
    standard.reset(plainEditor->createStandardContextMenu(event.pos()));
  }
  if (owner->contextMenuPolicy() != Qt::DefaultContextMenu) return {};
  // C++ is the sole menu owner. A context target may retire during exec's
  // nested event loop; that closes the popup without deleting this handle.
  auto menu = std::make_unique<CommandMenu>();
  QObject::connect(owner, &QObject::destroyed, menu.get(), &QWidget::close);
  if (standard) {
    for (auto actions = standard->actions(); auto* action : std::as_const(actions)) {
      standard->removeAction(action);
      action->setParent(menu.get());
      action->setIcon(QIcon());
      menu->addAction(action);
    }
  } else if (auto* label = qobject_cast<QLabel*>(&target); label && !label->text().isEmpty()) {
    QString text = label->hasSelectedText() ? label->selectedText() : label->text();
    if (!label->hasSelectedText() && (label->textFormat() == Qt::RichText || (label->textFormat() == Qt::AutoText && Qt::mightBeRichText(text))))
      text = QTextDocumentFragment::fromHtml(text).toPlainText();
    menu->addAction(QIcon(QStringLiteral(":/icons/copy.svg")), QCoreApplication::translate("uwf", "Copy"), menu.get(),
                    [label = QPointer(label), text, writer = std::move(writer)] {
                      if (label) static_cast<void>(writer(text));
                    });
  } else if (auto* header = qobject_cast<QHeaderView*>(target.parentWidget())) {
    menu->addAction(QIcon(QStringLiteral(":/icons/columns.svg")), QCoreApplication::translate("uwf", "Fit columns to contents"), menu.get(),
                    [header = QPointer(header)] {
                      if (header) header->resizeSections(QHeaderView::ResizeToContents);
                    });
  } else if (auto* view = qobject_cast<QAbstractItemView*>(target.parentWidget());
             view && view->viewport() == &target && view->contextMenuPolicy() == Qt::DefaultContextMenu) {
    const QPointer guardedTarget(&target);
    const auto index = view->indexAt(event.pos());
    const QPointer guardedView(view);
    if (index.isValid() && !view->selectionModel()->isSelected(index)) view->setCurrentIndex(index);
    if (!guardedView || !guardedTarget) return {};
    auto* copy = menu->addAction(QIcon(QStringLiteral(":/icons/copy.svg")), QCoreApplication::translate("uwf", "Copy"), menu.get(),
                                 [view = guardedView, writer = std::move(writer)] {
                                   if (!view || !view->selectionModel()) return;
                                   QStringList records;
                                   auto indexes = view->selectionModel()->selectedIndexes();
                                   std::sort(indexes.begin(), indexes.end(), [](const QModelIndex& left, const QModelIndex& right) {
                                     return left.row() != right.row() ? left.row() < right.row() : left.column() < right.column();
                                   });
                                   QStringList cells;
                                   int row = -1;
                                   for (const auto& item : std::as_const(indexes)) {
                                     if (row != item.row() && !cells.isEmpty()) {
                                       records.append(cells.join(u'\t'));
                                       cells.clear();
                                     }
                                     row = item.row();
                                     cells.append(item.data().toString());
                                   }
                                   if (!cells.isEmpty()) records.append(cells.join(u'\t'));
                                   static_cast<void>(writer(records.join(u'\n')));
                                 });
    copy->setEnabled(view->selectionModel()->hasSelection());
    menu->addAction(QIcon(QStringLiteral(":/icons/check.svg")), QCoreApplication::translate("uwf", "Select All"), view, &QAbstractItemView::selectAll);
  } else
    return {};
  return menu;
}
void installContextMenuPresentation() {
  static QPointer<ContextMenus> controller;
  if (!controller) {
    controller = new ContextMenus(qApp);
    qApp->installEventFilter(controller);
  }
}
}  // namespace uwf::ui
