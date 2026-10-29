// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <cstdint>
#include <limits>
#include <string>

#include "app/overlay/OverlayFileCatalog.h"

namespace {

class OverlayCatalogTests final : public QObject {
  Q_OBJECT

 private slots:
  void overlayCatalogPreservesStreamsAndSaturatesSizes();
};

void OverlayCatalogTests::overlayCatalogPreservesStreamsAndSaturatesSizes() {
  const auto maximum = std::numeric_limits<qulonglong>::max();
  const auto entries = uwf::app::buildOverlayFileCatalog(QStringLiteral("C:"), {{QStringLiteral("\\Folder\\Item:stream"), {}, false, false, maximum - 4},
                                                                                {QStringLiteral("\\folder\\ITEM"), {}, false, false, 10},
                                                                                {QStringLiteral("\\$Mft:stream"), {}, false, false, 3},
                                                                                {QStringLiteral("\\DIR:$INDEX_ALLOCATION"), {}, false, false, 0}});
  QCOMPARE(entries.size(), 3);
  QVERIFY(entries[0].isSystemMetadata);
  QCOMPARE(entries[0].absolutePath, QStringLiteral("C:\\$Mft"));
  QVERIFY(entries[1].isDirectory);
  QCOMPARE(entries[1].absolutePath, QStringLiteral("C:\\DIR"));
  QCOMPARE(entries[2].rawName, QStringLiteral("\\Folder\\Item:stream"));
  QCOMPARE(entries[2].absolutePath, QStringLiteral("C:\\Folder\\Item"));
  QCOMPARE(entries[2].fileSize, maximum);
  QCOMPARE(uwf::app::overlayFileCatalogSize(entries), maximum);
  QCOMPARE(uwf::app::overlayFileCatalogSize({}), qulonglong{0});
}

}  // namespace

QTEST_APPLESS_MAIN(OverlayCatalogTests)

#include "OverlayCatalogTests.moc"
