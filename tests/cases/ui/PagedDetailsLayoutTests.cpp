// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QHeaderView>
#include <QScrollBar>
#include <QScopeGuard>
#include <QStandardItemModel>
#include <QTableView>
#include <QTest>
#include <QVBoxLayout>

#include "presentation/common/PageProxyModel.h"
#include "ui/design/ItemViewGeometry.h"
#include "ui/widgets/FlexibleColumnLayout.h"
#include "ui/widgets/PagedTableView.h"
#include "ui/widgets/VerticalScrollArea.h"

namespace {
using namespace uwf;
constexpr int kRowHeight = 42;
constexpr int kGap = 10;

struct Records {
  QStandardItemModel source;
  presentation::PageProxyModel pages;
  QWidget content;
  ui::FlexibleColumnLayout* layout = new ui::FlexibleColumnLayout(&content);
  QWidget* host = new QWidget(&content);
  ui::PagedTableView* view = nullptr;
  QWidget* details = new QWidget(&content);

  Records() {
    source.setColumnCount(2);
    source.setRowCount(100);
    for (int row = 0; row < source.rowCount(); ++row) source.setData(source.index(row, 0), QStringLiteral("Record %1").arg(row));
    pages.setSourceModel(&source);
    view = new ui::PagedTableView(pages, kRowHeight, host);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    view->setColumnWidth(0, 100);
    view->setColumnWidth(1, 100);
    auto* inner = new QVBoxLayout(host);
    inner->setContentsMargins(1, 1, 1, 1);
    inner->addWidget(view);
    details->setMinimumHeight(40);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kGap);
    layout->addWidget(host, 1);
    layout->addWidget(details);
    layout->setPagedDetails(host, view, details, kRowHeight);
  }

  bool legal() const {
    return host->rect().contains(view->geometry()) && host->height() >= host->minimumSizeHint().height() &&
           view->viewport()->height() >= kRowHeight && view->viewport()->height() % kRowHeight == 0 &&
           ui::availableItemViewportHeight(*view) == view->viewport()->height() &&
           details->y() - (host->y() + host->height()) == kGap;
  }
};

class PagedDetailsLayoutTests final : public QObject {
  Q_OBJECT

 private slots:
  void minimumSurfaceCannotShrinkBelowItsTable() {
    QWidget content;
    auto* layout = new ui::FlexibleColumnLayout(&content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kGap);
    auto* host = new QWidget(&content);
    auto* inner = new QVBoxLayout(host);
    inner->setContentsMargins(1, 1, 1, 1);
    auto* view = new QTableView(host);
    view->setMinimumHeight(80);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    inner->addWidget(view);
    auto* details = new QWidget(&content);
    details->setMinimumHeight(40);
    layout->addWidget(host, 1);
    layout->addWidget(details);
    layout->setPagedDetails(host, view, details, kRowHeight);
    content.resize(400, 132);
    content.show();
    QVERIFY(QTest::qWaitForWindowExposed(&content));
    QTRY_VERIFY(host->rect().contains(view->geometry()));
    QTRY_VERIFY(host->height() >= host->minimumSizeHint().height());
    QTRY_VERIFY(ui::availableItemViewportHeight(*view) >= kRowHeight);
    QTRY_COMPARE(ui::availableItemViewportHeight(*view) % kRowHeight, 0);
    QTRY_COMPARE(details->y() - (host->y() + host->height()), kGap);
  }

  void scrollbarsAndExplicitMinimumsKeepWholeRows_data() {
    QTest::addColumn<bool>("alwaysOn");
    QTest::addColumn<int>("minimum");
    for (const bool always : {false, true})
      for (const int minimum : {0, 150, 200})
        QTest::newRow(qUtf8Printable(QStringLiteral("always-%1-minimum-%2").arg(always).arg(minimum))) << always << minimum;
  }

  void scrollbarsAndExplicitMinimumsKeepWholeRows() {
    QFETCH(bool, alwaysOn);
    QFETCH(int, minimum);
    Records records;
    records.host->setMinimumHeight(minimum);
    records.view->setHorizontalScrollBarPolicy(alwaysOn ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAsNeeded);
    records.content.resize(400, 132);
    records.content.show();
    QVERIFY(QTest::qWaitForWindowExposed(&records.content));
    QTRY_VERIFY(records.legal());
    const int originalMinimum = records.layout->minimumSize().height();
    records.view->setColumnWidth(0, 1200);
    QTRY_VERIFY(records.view->horizontalScrollBar()->isVisible());
    QTRY_VERIFY(records.legal());
    const int withScrollbar = records.layout->minimumSize().height();
    QVERIFY(withScrollbar >= originalMinimum);
    for (int cycle = 0; cycle < 3; ++cycle) {
      for (const QSize size : {QSize(400, 132), QSize(500, 260), QSize(400, 360)}) {
        records.content.resize(size);
        records.details->setMinimumHeight(120);
        QTRY_VERIFY(records.legal());
        records.details->setMinimumHeight(40);
        QTRY_VERIFY(records.legal());
      }
    }
    records.view->setColumnWidth(0, 100);
    if (!alwaysOn) QTRY_VERIFY(!records.view->horizontalScrollBar()->isVisible());
    records.content.resize(400, 132);
    QTRY_VERIFY(records.legal());
    QTRY_COMPARE(records.layout->minimumSize().height(), originalMinimum);
  }

  void insufficientSpaceUsesTheExistingOuterScrollArea() {
    Records records;
    records.view->setColumnWidth(0, 1200);
    QWidget window;
    auto* outer = new QVBoxLayout(&window);
    auto* scroll = new ui::VerticalScrollArea(&window);
    scroll->setWidget(&records.content);
    const auto detach = qScopeGuard([&] { static_cast<void>(scroll->takeWidget()); });
    outer->addWidget(scroll);
    window.resize(400, 130);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const auto windowGeometry = window.geometry();
    QTRY_VERIFY(records.view->horizontalScrollBar()->isVisible());
    QTRY_VERIFY(records.legal());
    QTRY_VERIFY(scroll->verticalScrollBar()->maximum() > 0);
    QTRY_VERIFY(records.content.height() >= records.layout->minimumSize().height());
    records.details->setMinimumHeight(240);
    QTRY_VERIFY(records.legal());
    QTRY_VERIFY(scroll->verticalScrollBar()->maximum() >= 200);
    records.details->setMinimumHeight(40);
    QTRY_VERIFY(records.legal());
    QCOMPARE(window.geometry(), windowGeometry);
  }
};
}  // namespace

QTEST_MAIN(PagedDetailsLayoutTests)
#include "PagedDetailsLayoutTests.moc"
