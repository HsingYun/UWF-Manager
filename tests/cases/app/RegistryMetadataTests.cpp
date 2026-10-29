// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <windows.h>

#include <QTest>
#include <algorithm>
#include <limits>

#include "app/ports/RegistryKey.h"
#include "app/registry/RegistryValueFormat.h"

namespace {
class RegistryMetadataTests final : public QObject {
  Q_OBJECT
 private slots:
  void registryRootsIncludeTheMachineHive() {
    const auto roots = uwf::regkey::rootHiveLongNames();
    QVERIFY(std::ranges::find(roots, std::string("HKEY_LOCAL_MACHINE")) != roots.end());
  }
  void valueTypeLabels_data() {
    QTest::addColumn<quint32>("type");
    QTest::addColumn<QString>("label");
    QTest::newRow("none") << quint32{REG_NONE} << QStringLiteral("REG_NONE");
    QTest::newRow("string") << quint32{REG_SZ} << QStringLiteral("REG_SZ");
    QTest::newRow("expandable-string") << quint32{REG_EXPAND_SZ} << QStringLiteral("REG_EXPAND_SZ");
    QTest::newRow("binary") << quint32{REG_BINARY} << QStringLiteral("REG_BINARY");
    QTest::newRow("dword") << quint32{REG_DWORD} << QStringLiteral("REG_DWORD");
    QTest::newRow("big-endian-dword") << quint32{REG_DWORD_BIG_ENDIAN} << QStringLiteral("REG_DWORD_BIG_ENDIAN");
    QTest::newRow("link") << quint32{REG_LINK} << QStringLiteral("REG_LINK");
    QTest::newRow("multi-string") << quint32{REG_MULTI_SZ} << QStringLiteral("REG_MULTI_SZ");
    QTest::newRow("resource-list") << quint32{REG_RESOURCE_LIST} << QStringLiteral("REG_RESOURCE_LIST");
    QTest::newRow("resource-descriptor") << quint32{REG_FULL_RESOURCE_DESCRIPTOR} << QStringLiteral("REG_FULL_RESOURCE_DESCRIPTOR");
    QTest::newRow("resource-requirements") << quint32{REG_RESOURCE_REQUIREMENTS_LIST} << QStringLiteral("REG_RESOURCE_REQUIREMENTS_LIST");
    QTest::newRow("qword") << quint32{REG_QWORD} << QStringLiteral("REG_QWORD");
    QTest::newRow("unknown") << std::numeric_limits<quint32>::max() << QStringLiteral("UNKNOWN(4294967295)");
  }
  void valueTypeLabels() {
    QFETCH(quint32, type);
    QFETCH(QString, label);
    QCOMPARE(QString::fromStdString(uwf::app::valueTypeName(type)), label);
  }
};
}  // namespace

QTEST_APPLESS_MAIN(RegistryMetadataTests)
#include "RegistryMetadataTests.moc"
