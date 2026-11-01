// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <chrono>

#include "infrastructure/windows/service/PreshutdownCommitGate.h"

namespace {
using uwf::windows::PreshutdownCommitGate;
using Clock = PreshutdownCommitGate::Clock;

class PreshutdownCommitGateTests final : public QObject {
  Q_OBJECT
 private slots:
  void recentPrecommitSkipsExactlyOneServiceCommit() {
    PreshutdownCommitGate gate;
    const auto now = Clock::time_point{};
    QVERIFY(!gate.consumeHandled(now));
    gate.markHandled(now);
    QVERIFY(gate.consumeHandled(now));
    QVERIFY(!gate.consumeHandled(now));
  }

  void receiptExpiresAfterFiveMinutes_data() {
    QTest::addColumn<qint64>("elapsedMilliseconds");
    QTest::addColumn<bool>("canSkip");
    const qint64 lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::minutes{5}).count();
    QTest::newRow("before-deadline") << lifetime - 1 << true;
    QTest::newRow("at-deadline") << lifetime << true;
    QTest::newRow("after-deadline") << lifetime + 1 << false;
  }
  void receiptExpiresAfterFiveMinutes() {
    QFETCH(qint64, elapsedMilliseconds);
    QFETCH(bool, canSkip);
    PreshutdownCommitGate gate;
    const auto started = Clock::time_point{};
    gate.markHandled(started);
    const auto shutdown = started + std::chrono::milliseconds{elapsedMilliseconds};
    QCOMPARE(gate.consumeHandled(shutdown), canSkip);
    QVERIFY(!gate.consumeHandled(shutdown));
  }

  void aNewCompletedPrecommitRenewsTheWindow() {
    PreshutdownCommitGate gate;
    const auto started = Clock::time_point{};
    gate.markHandled(started);
    gate.markHandled(started + std::chrono::minutes{4});
    QVERIFY(gate.consumeHandled(started + std::chrono::minutes{8}));
    QVERIFY(!gate.consumeHandled(started + std::chrono::minutes{8}));
  }

  void requiringCommitRevokesTheRecentReceipt() {
    PreshutdownCommitGate gate;
    const auto started = Clock::time_point{};
    gate.markHandled(started);
    gate.requireCommit();
    QVERIFY(!gate.consumeHandled(started + std::chrono::minutes{1}));
    gate.markHandled(started + std::chrono::minutes{2});
    QVERIFY(gate.consumeHandled(started + std::chrono::minutes{3}));
  }
};
}  // namespace

QTEST_APPLESS_MAIN(PreshutdownCommitGateTests)
#include "PreshutdownCommitGateTests.moc"
