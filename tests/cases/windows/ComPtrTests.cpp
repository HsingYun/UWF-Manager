// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <utility>

#include "windows/resources/ComPtr.h"

namespace {

struct RefCountedProbe {
  unsigned long refs = 1;
  int releases = 0;
  unsigned long AddRef() { return ++refs; }
  unsigned long Release() {
    ++releases;
    return --refs;
  }
};

class ComPtrTests final : public QObject {
  Q_OBJECT

 private slots:
  void comPtrModelsAdoptRetainMovePutAndRelease();
};

void ComPtrTests::comPtrModelsAdoptRetainMovePutAndRelease() {
  RefCountedProbe first;
  {
    auto owned = uwf::windows::ComPtr<RefCountedProbe>::adopt(&first);
    QVERIFY(owned);
    QCOMPARE(owned->refs, 1UL);
    auto moved = std::move(owned);
    QCOMPARE(moved.get(), &first);
    QCOMPARE(moved.release(), &first);
    QVERIFY(!moved);
  }
  // release 后作用域退出仍没有 Release，证明移动源不再持有该对象。
  QCOMPARE(first.releases, 0);

  RefCountedProbe retained;
  RefCountedProbe replacement;
  RefCountedProbe erasedReplacement;
  {
    auto pointer = uwf::windows::ComPtr<RefCountedProbe>::retain(&retained);
    QCOMPARE(retained.refs, 2UL);
    *pointer.put() = &replacement;
    QCOMPARE(retained.releases, 1);
    QCOMPARE(replacement.releases, 0);
    *pointer.putVoid() = &erasedReplacement;
    QCOMPARE(replacement.releases, 1);
    pointer.reset();
    QCOMPARE(erasedReplacement.releases, 1);
  }
  QCOMPARE(retained.refs, 1UL);
  QCOMPARE(replacement.releases, 1);
}

}  // namespace

QTEST_APPLESS_MAIN(ComPtrTests)

#include "ComPtrTests.moc"
