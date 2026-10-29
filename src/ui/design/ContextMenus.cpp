// SPDX-License-Identifier: GPL-3.0-or-later
#include "ContextMenus.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
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
#include <utility>

#include "ui/appearance/I18n.h"
#include "ui/widgets/CommandMenu.h"

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
    for (auto* parent = target; parent; parent = parent->parentWidget())
      if (parent->property("windowDragSurface").toBool() || parent->objectName() == QLatin1String("dialogHeader")) return false;
    QWidget* owner = target;
    std::unique_ptr<QMenu> standard;
    if (auto* lineEditor = qobject_cast<QLineEdit*>(target))
      standard.reset(lineEditor->createStandardContextMenu());
    else if (auto* richEditor = qobject_cast<QTextEdit*>(target->parentWidget()); richEditor && richEditor->viewport() == target) {
      owner = richEditor;
      standard.reset(richEditor->createStandardContextMenu(static_cast<QContextMenuEvent*>(event)->pos()));
    } else if (auto* plainEditor = qobject_cast<QPlainTextEdit*>(target->parentWidget()); plainEditor && plainEditor->viewport() == target) {
      owner = plainEditor;
      standard.reset(plainEditor->createStandardContextMenu(static_cast<QContextMenuEvent*>(event)->pos()));
    }
    if (owner->contextMenuPolicy() != Qt::DefaultContextMenu) return false;
    CommandMenu menu(owner);
    if (standard) {
      for (const auto actions = standard->actions(); auto* action : actions) {
        standard->removeAction(action);
        action->setParent(&menu);
        action->setIcon(QIcon());
        menu.addAction(action);
      }
    } else if (auto* label = qobject_cast<QLabel*>(target); label && !label->text().isEmpty()) {
      QString text = label->hasSelectedText() ? label->selectedText() : label->text();
      if (!label->hasSelectedText() && (label->textFormat() == Qt::RichText || (label->textFormat() == Qt::AutoText && Qt::mightBeRichText(text))))
        text = QTextDocumentFragment::fromHtml(text).toPlainText();
      menu.addAction(QIcon(QStringLiteral(":/icons/copy.svg")), I18n::tr("Copy"), &menu, [text] { QApplication::clipboard()->setText(text); });
    } else if (auto* header = qobject_cast<QHeaderView*>(target->parentWidget())) {
      menu.addAction(QIcon(QStringLiteral(":/icons/columns.svg")), QCoreApplication::translate("uwf", "Fit columns to contents"), &menu,
                     [header] { header->resizeSections(QHeaderView::ResizeToContents); });
    } else if (auto* view = qobject_cast<QAbstractItemView*>(target->parentWidget());
               view && view->viewport() == target && view->contextMenuPolicy() == Qt::DefaultContextMenu) {
      const auto index = view->indexAt(static_cast<QContextMenuEvent*>(event)->pos());
      if (index.isValid() && !view->selectionModel()->isSelected(index)) view->setCurrentIndex(index);
      auto* copy = menu.addAction(QIcon(QStringLiteral(":/icons/copy.svg")), I18n::tr("Copy"), &menu, [view] {
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
        QApplication::clipboard()->setText(records.join(u'\n'));
      });
      copy->setEnabled(view->selectionModel()->hasSelection());
      menu.addAction(QIcon(QStringLiteral(":/icons/check.svg")), I18n::tr("Select All"), view, &QAbstractItemView::selectAll);
    } else
      return false;
    auto* context = static_cast<QContextMenuEvent*>(event);
    const QPoint position = context->reason() == QContextMenuEvent::Keyboard ? owner->mapToGlobal(owner->rect().center()) : context->globalPos();
    menu.exec(position);
    context->accept();
    return true;
  }
};
}  // namespace
void installContextMenuPresentation() {
  static QPointer<ContextMenus> controller;
  if (!controller) {
    controller = new ContextMenus(qApp);
    qApp->installEventFilter(controller);
  }
}
}  // namespace uwf::ui
