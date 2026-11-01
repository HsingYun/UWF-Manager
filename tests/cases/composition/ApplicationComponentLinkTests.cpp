// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>

#include "application/settings/UserConfig.h"
#include "application/staging/editing/FileStagingPath.h"

namespace {
class ApplicationComponentLinkTests final : public QObject {
  Q_OBJECT
 private slots:
  void applicationPoliciesLinkWithoutNativeAdapters() {
    QVERIFY(uwf::app::isAbsoluteLocalFileStagingPath(QStringLiteral("C:\\data\\state.bin")));
    QVERIFY(!uwf::app::isAbsoluteLocalFileStagingPath(QStringLiteral("relative\\state.bin")));
    QCOMPARE(uwf::app::serializeUserConfig({}), QStringLiteral("[UI]\nTheme=system\nLanguage=system\nHubVisible=true\n"));
  }
};
}  // namespace
QTEST_APPLESS_MAIN(ApplicationComponentLinkTests)
#include "ApplicationComponentLinkTests.moc"