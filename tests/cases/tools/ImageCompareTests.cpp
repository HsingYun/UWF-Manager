// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QFile>
#include <QJsonArray>
#include <QTemporaryDir>
#include <QTest>
#include <stdexcept>

#include "tools/images/ImageCompare.h"

namespace {
using namespace uwf::test::tools;

class ImageCompareTests final : public QObject {
  Q_OBJECT

 private slots:
  void reportsPixelsAndBoundsIncludingAlpha() {
    QImage reference(7, 5, QImage::Format_RGBA8888);
    reference.fill(QColor(30, 60, 90, 255));
    QImage actual = reference;
    actual.setPixelColor(1, 2, QColor(30, 60, 90, 254));
    actual.setPixelColor(4, 3, QColor(31, 60, 90, 255));
    const auto difference = comparePixels(reference, actual);
    QCOMPARE(difference.changedPixels, qint64{2});
    QCOMPARE(difference.bounds, QRect(1, 2, 4, 2));
    QVERIFY(!difference.sizeMismatch);
  }
  void comparesDecodedPixelsAcrossFormatsAndStride() {
    QImage reference(3, 5, QImage::Format_RGB888);
    reference.fill(QColor(10, 20, 30));
    QCOMPARE(comparePixels(reference, reference.convertToFormat(QImage::Format_ARGB32)).changedPixels, qint64{0});
    QImage differentSize(5, 3, QImage::Format_RGB32);
    differentSize.fill(Qt::black);
    QVERIFY(comparePixels(reference, differentSize).sizeMismatch);
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, comparePixels(reference, QImage()));
  }
  void comparesCaptureSetsWithoutStoredImages() {
    QTemporaryDir reference;
    QTemporaryDir actual;
    QVERIFY(reference.isValid() && actual.isValid());
    QImage image(3, 4, QImage::Format_ARGB32);
    image.fill(QColor(20, 40, 60, 120));
    QVERIFY(image.save(reference.filePath(QStringLiteral("scene.png")), "PNG", 0));
    QImage differentEncoding = image;
    differentEncoding.setText(QStringLiteral("Capture"), QStringLiteral("Actual"));
    QVERIFY(differentEncoding.save(actual.filePath(QStringLiteral("scene.png")), "PNG", 100));
    QFile referenceFile(reference.filePath(QStringLiteral("scene.png")));
    QFile actualFile(actual.filePath(QStringLiteral("scene.png")));
    QVERIFY(referenceFile.open(QIODevice::ReadOnly) && actualFile.open(QIODevice::ReadOnly));
    QVERIFY(referenceFile.readAll() != actualFile.readAll());
    referenceFile.close();
    actualFile.close();
    QVERIFY(compareImageDirectories(reference.path(), actual.path(), 1).value(QStringLiteral("passed")).toBool());
    QVERIFY(!compareImageDirectories(reference.path(), actual.path(), 2).value(QStringLiteral("passed")).toBool());
    QVERIFY(image.save(actual.filePath(QStringLiteral("extra.png"))));
    auto report = compareImageDirectories(reference.path(), actual.path());
    QVERIFY(!report.value(QStringLiteral("passed")).toBool());
    QCOMPARE(report.value(QStringLiteral("unexpectedActual")).toArray().size(), qsizetype{1});
    QVERIFY(QFile::remove(actual.filePath(QStringLiteral("scene.png"))));
    report = compareImageDirectories(reference.path(), actual.path());
    QCOMPARE(report.value(QStringLiteral("missingActual")).toArray().size(), qsizetype{1});
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, compareImageDirectories(reference.path(), reference.path()));
  }
  void rejectsIdenticalCorruptFilesAndEmptyCaptures() {
    QTemporaryDir reference;
    QTemporaryDir actual;
    QVERIFY(reference.isValid() && actual.isValid());
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, compareImageDirectories(reference.path(), actual.path()));
    for (const auto& path : {reference.filePath(QStringLiteral("bad.png")), actual.filePath(QStringLiteral("bad.png"))}) {
      QFile file(path);
      QVERIFY(file.open(QIODevice::WriteOnly));
      QCOMPARE(file.write("invalid png"), qint64{11});
    }
    const auto report = compareImageDirectories(reference.path(), actual.path());
    QVERIFY(!report.value(QStringLiteral("passed")).toBool());
    QCOMPARE(report.value(QStringLiteral("differences")).toArray().size(), qsizetype{1});
  }
};
}  // namespace

QTEST_APPLESS_MAIN(ImageCompareTests)
#include "ImageCompareTests.moc"
