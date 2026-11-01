// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QImage>
#include <QTest>

#include "support/visual/VisualGeometry.h"

class VisualGeometryTests final : public QObject {
  Q_OBJECT
 private slots:
  void inkRunsPreserveClippingGapsAndAntialiasedEdges_data() {
    QTest::addColumn<int>("format");
    QTest::newRow("rgb32") << int(QImage::Format_RGB32);
    QTest::newRow("argb32") << int(QImage::Format_ARGB32);
    QTest::newRow("premultiplied") << int(QImage::Format_ARGB32_Premultiplied);
    QTest::newRow("rgb888") << int(QImage::Format_RGB888);
    QTest::newRow("rgba8888") << int(QImage::Format_RGBA8888);
  }
  void inkRunsPreserveClippingGapsAndAntialiasedEdges() {
    QFETCH(int, format);
    QImage source(14, 10, QImage::Format_ARGB32);
    source.fill(Qt::white);
    source.setPixelColor(0, 0, Qt::black);  // Outside the requested region.
    for (int y = 2; y <= 4; ++y) source.setPixelColor(2, y, Qt::black);
    source.setPixelColor(3, 3, QColor(224, 224, 224));  // Just over 12% ink.
    source.setPixelColor(4, 3, QColor(225, 225, 225));  // Just under 12% ink.
    source.setPixelColor(7, 6, QColor(0, 0, 0, 128));
    const auto image = source.convertToFormat(static_cast<QImage::Format>(format));
    const QVector<QRect> separate{QRect(2, 2, 2, 3), QRect(7, 6, 1, 1)};
    QCOMPARE(uwf::test::horizontalInkRuns(image, Qt::black, Qt::white, QRect(1, 1, 9, 8), 2), separate);
    QCOMPARE(uwf::test::horizontalInkRuns(image, Qt::black, Qt::white, QRect(1, 1, 9, 8), 4), QVector<QRect>{QRect(2, 2, 6, 5)});
    QVERIFY(uwf::test::horizontalInkRuns(image, Qt::white, Qt::white, image.rect(), 1).isEmpty());
    QVERIFY(uwf::test::horizontalInkRuns(image, Qt::black, Qt::white, QRect(20, 20, 3, 3), 1).isEmpty());
  }
  void inferredInkUsesTheDominantBackgroundAndKeepsLowContrastEmpty() {
    QImage image(12, 8, QImage::Format_RGB32);
    image.fill(Qt::white);
    for (int y = 2; y <= 4; ++y) image.setPixelColor(2, y, Qt::black);
    image.setPixelColor(3, 3, QColor(224, 224, 224));
    image.setPixelColor(4, 3, QColor(225, 225, 225));
    image.setPixelColor(7, 6, Qt::black);
    const QVector<QRect> expected{QRect(2, 2, 2, 3), QRect(7, 6, 1, 1)};
    QCOMPARE(uwf::test::contrastingInkRuns(image, image.rect(), 2), expected);
    image.fill(QColor(250, 250, 250));
    image.setPixelColor(2, 2, QColor(253, 253, 253));
    QVERIFY(uwf::test::contrastingInkRuns(image, image.rect(), 1).isEmpty());
    QVERIFY(uwf::test::contrastingInkRuns({}, {}, 1).isEmpty());
  }
  void premultipliedInkUsesItsUnpremultipliedColor() {
    QImage image(8, 8, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);
    // Stored RGB is 16; its actual color is 32, just above the 12% threshold.
    image.setPixelColor(4, 3, QColor(32, 32, 32, 128));
    QCOMPARE(uwf::test::horizontalInkRuns(image, Qt::white, Qt::black, image.rect(), 1), QVector<QRect>{QRect(4, 3, 1, 1)});
  }
  void absentEvidenceCannotConfirmEitherColorPresenceOrAbsence() {
    QImage image(20, 20, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    const auto reject = [](const QImage& source, const QRectF& region) {
      const auto coverage = uwf::test::colorCoverage(source, Qt::white, region);
      QVERIFY(!(coverage >= uwf::test::kVisualMinimumCoverage));
      QVERIFY(!(coverage < 1));
    };
    reject({}, QRectF(0, 0, 10, 10));
    reject(image, {});
    reject(image, QRectF(30, 30, 10, 10));
    reject(image, QRectF(0, 0, .1, .1));
    QVERIFY(uwf::test::colorCoverage(image, Qt::white, QRectF(0, 0, 10, 10)) >= uwf::test::kVisualMinimumCoverage);
    QVERIFY(uwf::test::colorCoverage(image, Qt::black, QRectF(0, 0, 10, 10)) < 1);
  }
  void alignmentAndCoverageUseTheSameLogicalContentAtDifferentScales() {
    for (const qreal scale : {1.0, 1.25, 2.0}) {
      QImage image(qRound(40 * scale), qRound(20 * scale), QImage::Format_ARGB32_Premultiplied);
      image.setDevicePixelRatio(scale);
      image.fill(Qt::white);
      const auto logical = uwf::test::logicalBounds(image.rect(), image.devicePixelRatio());
      QVERIFY(uwf::test::similarSize(logical, QRectF(0, 0, 40, 20)));
      QVERIFY(uwf::test::centersAligned(QRectF(10, 5, 20, 10), logical));
      QVERIFY(!uwf::test::centersAligned(QRectF(10, 25, 20, 10), logical));
      QVERIFY(uwf::test::colorCoverage(image, Qt::white, uwf::test::interiorRegion(logical)) >= uwf::test::kVisualMinimumCoverage);
    }
  }
};

QTEST_APPLESS_MAIN(VisualGeometryTests)
#include "VisualGeometryTests.moc"
