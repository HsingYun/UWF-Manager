// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <array>
#include <limits>

#include "support/ui/PointerPressHistory.h"

Q_DECLARE_METATYPE(uwf::test::PointerWindowIdentity)
Q_DECLARE_METATYPE(uwf::test::PointerPoint)

namespace {
using namespace uwf::test;
constexpr PointerWindowIdentity firstWindow{11, 101};
constexpr PointerWindowIdentity secondWindow{12, 102};
constexpr std::uint32_t firstButton = 1;
constexpr PointerPoint firstPoint{100, 200};
constexpr SinglePressPolicy platformPolicy{500, 4, 4};

class PointerPressHistoryTests final : public QObject {
  Q_OBJECT
 private slots:
  void independentPressPartitions_data() {
    QTest::addColumn<PointerWindowIdentity>("window");
    QTest::addColumn<bool>("retired");
    QTest::addColumn<quint32>("button");
    QTest::addColumn<PointerPoint>("point");
    QTest::addColumn<qulonglong>("time");
    QTest::addColumn<qulonglong>("expected");
    struct Window {
      const char* name;
      PointerWindowIdentity identity;
      bool retire, same;
    };
    constexpr std::array windows{Window{"same-window", firstWindow, false, true}, Window{"other-window", secondWindow, false, false},
                                 Window{"native-recreated", {11, 102}, false, false}, Window{"native-id-reused", {12, 101}, false, false},
                                 Window{"destroyed", firstWindow, true, false}};
    struct Position {
      const char* name;
      PointerPoint point;
      bool nearby;
    };
    constexpr std::array positions{Position{"same-point", firstPoint, true}, Position{"distance-boundary", {104, 204}, true},
                                   Position{"outside-x", {105, 200}, false}, Position{"outside-y", {100, 205}, false},
                                   Position{"integer-extremes", {std::numeric_limits<int>::min(), std::numeric_limits<int>::max()}, false}};
    struct Time {
      const char* name;
      std::uint64_t now, remaining;
    };
    constexpr std::array times{Time{"immediate", 1000, 501}, Time{"before-boundary", 1499, 2}, Time{"at-boundary", 1500, 1}, Time{"after-boundary", 1501, 0},
                               Time{"clock-regressed", 999, 0}};
    for (const auto& window : windows)
      for (const auto& position : positions)
        for (const auto& time : times)
          for (const quint32 button : {1U, 2U}) {
            const auto name = QByteArray(window.name) + '/' + position.name + '/' + time.name + "/button-" + QByteArray::number(button);
            const auto expected = window.same && position.nearby && button == firstButton ? time.remaining : 0;
            QTest::newRow(name.constData()) << window.identity << window.retire << button << position.point << static_cast<qulonglong>(time.now)
                                            << static_cast<qulonglong>(expected);
          }
  }
  void independentPressPartitions() {
    QFETCH(PointerWindowIdentity, window);
    QFETCH(bool, retired);
    QFETCH(quint32, button);
    QFETCH(PointerPoint, point);
    QFETCH(qulonglong, time);
    QFETCH(qulonglong, expected);
    PointerPressHistory history;
    history.record(firstWindow, firstButton, firstPoint, 1000);
    if (retired) history.retire(firstWindow);
    QCOMPARE(history.remainingDelay(window, button, point, time, platformPolicy), expected);
  }
  void emptyAndRetiredSurfacesDoNotDelay() {
    PointerPressHistory history;
    QCOMPARE(history.remainingDelay(firstWindow, firstButton, firstPoint, 1000, platformPolicy), std::uint64_t{0});
    history.record(firstWindow, firstButton, firstPoint, 1000);
    history.retire(secondWindow);
    QCOMPARE(history.remainingDelay(firstWindow, firstButton, firstPoint, 1000, platformPolicy), std::uint64_t{501});
    history.retire(firstWindow);
    // A later window may reuse both opaque identities; retirement is required
    // to make that a fresh lifetime rather than the previous click sequence.
    QCOMPARE(history.remainingDelay(firstWindow, firstButton, firstPoint, 1000, platformPolicy), std::uint64_t{0});
    history.record(firstWindow, firstButton, firstPoint, 1100);
    QCOMPARE(history.remainingDelay(firstWindow, firstButton, firstPoint, 1100, platformPolicy), std::uint64_t{501});
  }
  void onlyTheLatestRealPressParticipates() {
    PointerPressHistory history;
    history.record(firstWindow, firstButton, firstPoint, 1000);
    history.record(secondWindow, firstButton, firstPoint, 1050);
    QCOMPARE(history.remainingDelay(firstWindow, firstButton, firstPoint, 1050, platformPolicy), std::uint64_t{0});
    QCOMPARE(history.remainingDelay(secondWindow, firstButton, firstPoint, 1050, platformPolicy), std::uint64_t{501});
    history.retire(firstWindow);
    QCOMPARE(history.remainingDelay(secondWindow, firstButton, firstPoint, 1050, platformPolicy), std::uint64_t{501});
    history.record(secondWindow, 2, {200, 300}, 1100);
    QCOMPARE(history.remainingDelay(secondWindow, firstButton, firstPoint, 1100, platformPolicy), std::uint64_t{0});
    QCOMPARE(history.remainingDelay(secondWindow, 2, {200, 300}, 1100, platformPolicy), std::uint64_t{501});
  }
  void extremePolicyCannotWrapIntoAnImmediatePress() {
    PointerPressHistory history;
    history.record(firstWindow, firstButton, firstPoint, 0);
    const SinglePressPolicy maximum{std::numeric_limits<std::uint64_t>::max(), 4, 4};
    QCOMPARE(history.remainingDelay(firstWindow, firstButton, firstPoint, 0, maximum), std::numeric_limits<std::uint64_t>::max());
  }
};
}  // namespace

QTEST_APPLESS_MAIN(PointerPressHistoryTests)
#include "PointerPressHistoryTests.moc"
