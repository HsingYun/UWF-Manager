// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>

#include "infrastructure/windows/power/WindowsHibernationCapabilities.h"

namespace {
class HibernationCapabilitiesTests final : public QObject {
  Q_OBJECT
 private slots:
  void sdkFileTypesRequireFullFileAndSystemS4() {
    using namespace uwf;
    QCOMPARE(windows::decodeWindowsHibernationCapabilities(true, false, 0).fileType, app::HibernationFileType::None);
    QCOMPARE(windows::decodeWindowsHibernationCapabilities(true, true, 1).fileType, app::HibernationFileType::Reduced);
    const auto full = windows::decodeWindowsHibernationCapabilities(true, true, 2);
    QCOMPARE(full.fileType, app::HibernationFileType::Full);
    QVERIFY(full.available());
    QVERIFY(!windows::decodeWindowsHibernationCapabilities(false, true, 2).available());
    QVERIFY(!windows::decodeWindowsHibernationCapabilities(true, false, 2).available());
    QVERIFY(!windows::decodeWindowsHibernationCapabilities(true, true, 1).available());
    const auto unknown = windows::decodeWindowsHibernationCapabilities(true, true, 255);
    QCOMPARE(unknown.fileType, app::HibernationFileType::Unknown);
    QVERIFY(!unknown.available());
  }
};
}  // namespace
QTEST_APPLESS_MAIN(HibernationCapabilitiesTests)
#include "HibernationCapabilitiesTests.moc"
