// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QStringListModel>
#include <QTest>
#include <array>
#include <cstddef>
#include <memory>
#include <utility>

#include "presentation/models/PageProxyModel.h"

namespace {
using namespace uwf;

class PageProxyModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void pageProxyPreservesSourceMappingAndIndependentViewState();
  void pageProxyTracksSourceChangesAndDetachment();
  void pageProxyTracksReorderingAndSourceDestruction();
  void continuousNavigationKeepsTheLatestObserverDecision_data();
  void continuousNavigationKeepsTheLatestObserverDecision();
};

void PageProxyModelTests::pageProxyPreservesSourceMappingAndIndependentViewState() {
  QStringList rows;
  for (int row = 0; row < 205; ++row) rows.append(QString::number(row));
  QStringListModel source(rows);
  presentation::PageProxyModel first;
  presentation::PageProxyModel second;
  first.setPageSize(200);
  second.setPageSize(10);
  first.setSourceModel(&source);
  second.setSourceModel(&source);
  QAbstractItemModelTester contract(&first, QAbstractItemModelTester::FailureReportingMode::QtTest);
  QCOMPARE(first.rowCount(), 200);
  first.goLast();
  QCOMPARE(first.rowCount(), 5);
  QCOMPARE(first.index(0, 0).data().toString(), QStringLiteral("200"));
  QCOMPARE(first.mapToSource(first.index(4, 0)), source.index(204, 0));
  QVERIFY(!first.mapFromSource(source.index(0, 0)).isValid());
  QCOMPARE(second.index(0, 0).data().toString(), QStringLiteral("0"));
  first.setPageSize(10);
  QCOMPARE(first.pager().pageStart(), 200);
  source.setStringList({QStringLiteral("retained")});
  QCOMPARE(first.pager().currentPage, 0);
  QCOMPARE(first.rowCount(), 1);
  first.goNext();
  QCOMPARE(first.pager().currentPage, 0);
  source.setStringList({});
  first.goLast();
  QCOMPARE(first.rowCount(), 0);
  QCOMPARE(first.pager().currentPage, 0);
}

void PageProxyModelTests::pageProxyTracksSourceChangesAndDetachment() {
  QStringListModel source({QStringLiteral("first"), QStringLiteral("second"), QStringLiteral("third")});
  presentation::PageProxyModel pages;
  pages.setPageSize(2);
  pages.setSourceModel(&source);
  QAbstractItemModelTester contract(&pages, QAbstractItemModelTester::FailureReportingMode::QtTest);
  pages.goLast();
  QCOMPARE(pages.rowCount(), 1);
  QVERIFY(source.insertRow(3));
  QVERIFY(source.setData(source.index(3, 0), QStringLiteral("fourth")));
  QCOMPARE(pages.rowCount(), 2);
  QCOMPARE(pages.index(1, 0).data().toString(), QStringLiteral("fourth"));
  QVERIFY(source.removeRows(1, 3));
  QCOMPARE(pages.rowCount(), 1);
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("first"));
  QVERIFY(source.setData(source.index(0, 0), QStringLiteral("updated")));
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("updated"));

  QStringListModel replacement({QStringLiteral("replacement")});
  pages.setSourceModel(&replacement);
  source.setStringList({QStringLiteral("stale")});
  QCOMPARE(pages.rowCount(), 1);
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("replacement"));
  pages.setSourceModel(nullptr);
  QCOMPARE(pages.rowCount(), 0);
  QCOMPARE(pages.totalRows(), 0);
  pages.goLast();
  pages.goNext();
  QVERIFY(!pages.index(0, 0).isValid());
  pages.setSourceModel(&source);
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("stale"));
}

void PageProxyModelTests::pageProxyTracksReorderingAndSourceDestruction() {
  auto source = std::make_unique<QStringListModel>(QStringList{QStringLiteral("0"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")});
  presentation::PageProxyModel pages;
  pages.setPageSize(2);
  pages.setSourceModel(source.get());
  QAbstractItemModelTester contract(&pages, QAbstractItemModelTester::FailureReportingMode::QtTest);
  pages.goLast();
  QVERIFY(source->moveRows({}, 0, 1, {}, 4));
  QCOMPARE(pages.rowCount(), 2);
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("3"));
  QCOMPARE(pages.index(1, 0).data().toString(), QStringLiteral("0"));
  source->sort(0, Qt::DescendingOrder);
  QCOMPARE(pages.index(0, 0).data().toString(), QStringLiteral("1"));
  QCOMPARE(pages.index(1, 0).data().toString(), QStringLiteral("0"));
  QSignalSpy changed(&pages, &presentation::PageProxyModel::pageChanged);
  source.reset();
  QCOMPARE(pages.totalRows(), 0);
  QCOMPARE(pages.rowCount(), 0);
  QCOMPARE(pages.pager().currentPage, 0);
  QTRY_VERIFY(!changed.isEmpty());
}

void PageProxyModelTests::continuousNavigationKeepsTheLatestObserverDecision_data() {
  QTest::addColumn<int>("reaction");
  const std::array reactions{"release", "navigate", "navigate-away-and-back", "change-mode", "detach-source", "scroll", "resize-capacity"};
  for (std::size_t reaction = 0; reaction < reactions.size(); ++reaction) QTest::newRow(reactions[reaction]) << static_cast<int>(reaction);
}

void PageProxyModelTests::continuousNavigationKeepsTheLatestObserverDecision() {
  QFETCH(int, reaction);
  QStringList rows;
  for (int row = 0; row < 20; ++row) rows.append(QString::number(row));
  QStringListModel source(rows);
  auto pages = std::make_unique<presentation::PageProxyModel>();
  pages->setSourceModel(&source);
  pages->setPageSize(4);
  pages->setContinuous(true);
  QList<int> requests;
  connect(pages.get(), &presentation::PageProxyModel::pageNavigationRequested, this, [&](const int row) { requests.append(row); });
  bool reacted = false;
  connect(pages.get(), &presentation::PageProxyModel::pageChanged, this, [&] {
    if (std::exchange(reacted, true)) return;
    switch (reaction) {
      case 0:
        pages.reset();
        break;
      case 1:
        pages->goLast();
        break;
      case 2:
        pages->goFirst();
        pages->goNext();
        break;
      case 3:
        pages->setContinuous(false);
        break;
      case 4:
        pages->setSourceModel(nullptr);
        break;
      case 5:
        pages->syncScrollPosition(12, false);
        break;
      case 6:
        pages->setPageSize(3);
        break;
    }
  });
  pages->goNext();
  if (reaction == 1)
    QCOMPARE(requests, QList<int>{16});
  else if (reaction == 2)
    QCOMPARE(requests, QList<int>({0, 4}));
  else if (reaction == 6)
    QCOMPARE(requests, QList<int>{3});
  else if (reaction == 4)
    QCOMPARE(requests, QList<int>{0});
  else
    QVERIFY(requests.isEmpty());
}

}  // namespace

QTEST_GUILESS_MAIN(PageProxyModelTests)
#include "PageProxyModelTests.moc"
