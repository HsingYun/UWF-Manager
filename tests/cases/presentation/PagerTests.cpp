// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <array>
#include <limits>

#include "presentation/common/Pager.h"

namespace {

class PagerTests final : public QObject {
  Q_OBJECT

 private slots:
  void pagerPreservesTheVisibleAnchor();
  void pagerPropertiesRemainBoundedAtIntegerEdges();
};

void PagerTests::pagerPreservesTheVisibleAnchor() {
  uwf::presentation::Pager pager{10, 3};
  QCOMPARE(pager.pageCount(31), 4);
  QCOMPARE(pager.pageStart(), 30);
  QCOMPARE(pager.pageEnd(31), 31);
  QVERIFY(pager.hasPrev());
  QVERIFY(!pager.hasNext(31));

  QVERIFY(pager.setPageSize(7));
  QCOMPARE(pager.currentPage, 4);
  QCOMPARE(pager.pageStart(), 28);
  pager.clamp(20);
  QCOMPARE(pager.currentPage, 2);
  pager.goFirst();
  QVERIFY(!pager.hasPrev());
  pager.goLast(20);
  QCOMPARE(pager.currentPage, 2);
  pager.goNext(20);
  QCOMPARE(pager.currentPage, 2);
}

void PagerTests::pagerPropertiesRemainBoundedAtIntegerEdges() {
  constexpr std::array totals = {-1, 0, 1, 2, 7, 31, 1024, std::numeric_limits<int>::max()};
  constexpr std::array sizes = {-7, 0, 1, 2, 7, 1024, std::numeric_limits<int>::max()};
  constexpr std::array pages = {-9, 0, 1, 17, std::numeric_limits<int>::max()};

  for (const int total : totals) {
    for (const int size : sizes) {
      for (const int page : pages) {
        uwf::presentation::Pager pager{size, page};
        const int count = pager.pageCount(total);
        QVERIFY(count >= 0);
        pager.clamp(total);
        QVERIFY(pager.currentPage >= 0);
        QVERIFY(count == 0 ? pager.currentPage == 0 : pager.currentPage < count);
        const int start = pager.pageStart();
        const int end = pager.pageEnd(total);
        QVERIFY(start >= 0);
        QVERIFY(end >= 0);
        if (total > 0) {
          QVERIFY(start <= total);
          QVERIFY(end >= start);
          QVERIFY(end <= total);
        } else {
          QCOMPARE(end, 0);
        }
        QVERIFY(!pager.hasPrev() || pager.currentPage > 0);
        QVERIFY(!pager.hasNext(total) || pager.currentPage + 1 < count);
      }
    }
  }
}

}  // namespace

QTEST_APPLESS_MAIN(PagerTests)

#include "PagerTests.moc"
