// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include "cases/presentation/views/widgets/WidgetUiFixture.h"

#include <QAction>
#include <QCoreApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPushButton>
#include <QScrollBar>
#include <QString>
#include <QTest>
#include <QWheelEvent>
#include <QWidget>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/models/PageProxyModel.h"
#include "support/ui/ViewMetrics.h"
#include "support/visual/WidgetCapture.h"

namespace uwf::test::widgets {

QAction* actionWithText(QWidget* root, const QString& text) {
  for (auto actions = root->findChildren<QAction*>(); auto* action : std::as_const(actions)) {
    if (action->text() == text) return action;
  }
  return nullptr;
}

QPushButton* buttonWithText(QWidget* root, const QString& text) {
  for (auto buttons = root->findChildren<QPushButton*>(); auto* button : std::as_const(buttons)) {
    if (button->text() == text || button->accessibleName() == text) return button;
  }
  return nullptr;
}

void checkWholeRowScrolling(QAbstractItemView* view) {
  const int rowHeight = uwf::test::displayedRowHeight(view);
  QVERIFY(rowHeight > 0);
  QCOMPARE(view->verticalScrollMode(), QAbstractItemView::ScrollPerItem);
  QCOMPARE(view->verticalScrollBar()->singleStep(), 1);
  const QPoint local(40, 30);
  for (const int delta : {-15, -30, -60, -120, -240, 15, 30, 60, 120, 240}) {
    QWheelEvent wheel(local, view->viewport()->mapToGlobal(local), QPoint(), QPoint(0, delta), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(view->viewport(), &wheel);
    const auto first = view->indexAt(QPoint(10, 0));
    QVERIFY(first.isValid());
    QCOMPARE(view->visualRect(first).top(), 0);
    QCOMPARE(view->viewport()->height() % rowHeight, 0);
    const int lastRow = first.row() + view->viewport()->height() / rowHeight - 1;
    if (lastRow < view->model()->rowCount()) QCOMPARE(view->visualRect(view->model()->index(lastRow, first.column())).bottom() + 1, view->viewport()->height());
  }
  view->verticalScrollBar()->setValue(7);
  const auto first = view->indexAt(QPoint(10, 0));
  QCOMPARE(first.row(), 7);
  QCOMPARE(view->visualRect(first).top(), 0);
}

void checkHoverAndSmoothDragging(QAbstractItemView* view, uwf::presentation::PageProxyModel& pages) {
  auto* bar = view->findChild<QScrollBar*>(QStringLiteral("entryScrollOverlay"));
  QVERIFY(bar);
  const int rowHeight = uwf::test::displayedRowHeight(view);
  const QPoint edge(view->viewport()->width() - 2, 20);
  QMouseEvent hover(QEvent::MouseMove, edge, view->viewport()->mapToGlobal(edge), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  QCoreApplication::sendEvent(view->viewport(), &hover);
  QVERIFY(bar->isVisible());
  bar->setSliderDown(true);
  QCOMPARE(view->verticalScrollMode(), QAbstractItemView::ScrollPerPixel);
  const int pixels = rowHeight * 2 + rowHeight / 2;
  bar->setValue(pixels);
  QCOMPARE(view->verticalScrollBar()->value(), pixels);
  QVERIFY(view->visualRect(view->indexAt(QPoint(10, 0))).top() < 0);
  bar->setSliderDown(false);
  QWheelEvent wheel(edge, view->viewport()->mapToGlobal(edge), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
  QCoreApplication::sendEvent(view->viewport(), &wheel);
  QCOMPARE(view->verticalScrollMode(), QAbstractItemView::ScrollPerItem);
  QCOMPARE(view->visualRect(view->indexAt(QPoint(10, 0))).top(), 0);
  bar->setSliderDown(true);
  bar->setValue(pixels);
  bar->setSliderDown(false);
  pages.goNext();
  QCOMPARE(view->verticalScrollMode(), QAbstractItemView::ScrollPerItem);
  QCOMPARE(view->visualRect(view->indexAt(QPoint(10, 0))).top(), 0);
  QEvent leave(QEvent::Leave);
  QCoreApplication::sendEvent(bar, &leave);
  QCoreApplication::sendEvent(view->viewport(), &leave);
}

void initializeWidgets() {
  uwf::test::initializeVisualComparison();
  uwf::ui::I18n::instance().setLang(uwf::ui::I18n::Lang::En);
  uwf::ui::ThemeManager::instance().apply(uwf::app::Theme::Dark);
}
}  // namespace uwf::test::widgets
