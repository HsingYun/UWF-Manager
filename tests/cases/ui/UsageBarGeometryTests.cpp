// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <array>
#include <cmath>

#include "ui/overlay/UsageBarGeometry.h"

namespace {

class UsageBarGeometryTests final : public QObject {
  Q_OBJECT

 private slots:
  void usageBarGeometryClampsVisualHints();
  void usageBarGeometryPropertiesRemainFiniteAndBounded();
};

void UsageBarGeometryTests::usageBarGeometryClampsVisualHints() {
  QCOMPARE(uwf::ui::visibleUsedWidth(0, 100), qreal{0});
  const qreal visible = uwf::ui::visibleUsedWidth(0.1, 100);
  QVERIFY(visible >= 0.1 && visible <= 100);
  const qreal limited = uwf::ui::visibleUsedWidth(0.1, 100, 2);
  QVERIFY(limited >= 0.1 && limited <= 1);
  QCOMPARE(uwf::ui::visibleUsedWidth(120, 100), qreal{100});

  const QRectF bounds(10, 20, 100, 40);
  constexpr qreal excursion = 3;
  const QPainterPath path = uwf::ui::waveProgressPath(bounds, 25, 0, excursion);
  QVERIFY(path.contains(QPointF(10.5, 30)));
  QVERIFY(!path.contains(QPointF(50, 30)));
  QVERIFY(path.boundingRect().left() >= bounds.left());
  QVERIFY(path.boundingRect().right() <= bounds.left() + 25 + excursion);
}

void UsageBarGeometryTests::usageBarGeometryPropertiesRemainFiniteAndBounded() {
  constexpr std::array widths = {-10.0, 0.0, 0.01, 1.0, 50.0, 100.0, 1000.0};
  for (const qreal full : widths) {
    for (const qreal natural : widths) {
      const qreal visible = uwf::ui::visibleUsedWidth(natural, full, 20, 40, 80);
      QVERIFY(std::isfinite(visible));
      QVERIFY(visible >= 0);
      if (full > 0) QVERIFY(visible <= full);
      if (natural > 0 && full > 0) QVERIFY(visible >= std::min(natural, full));
    }
  }

  const QRectF bounds(3, 5, 127, 41);
  for (const qreal used : widths) {
    for (const qreal phase : {-100.0, 0.0, 1.0, 100.0}) {
      for (const qreal excursion : {0.0, 1.5, 3.0}) {
        const QRectF pathBounds = uwf::ui::waveProgressPath(bounds, used, phase, excursion).boundingRect();
        QVERIFY(pathBounds.left() >= bounds.left());
        QVERIFY(pathBounds.top() >= bounds.top());
        QVERIFY(pathBounds.bottom() <= bounds.bottom());
        QVERIFY(pathBounds.right() <= bounds.right() + excursion);
      }
    }
  }
}

}  // namespace

QTEST_APPLESS_MAIN(UsageBarGeometryTests)

#include "UsageBarGeometryTests.moc"
