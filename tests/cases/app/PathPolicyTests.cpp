// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "app/paths/PathPolicy.h"

namespace {

QString text(const std::string& value) { return QString::fromUtf8(value); }

class PathPolicyTests final : public QObject {
  Q_OBJECT

 private slots:
  void pathMatchingHonorsSegmentBoundaries();
  void pathMatchingPropertiesPreserveSegmentSemantics();
  void fileNormalizationResolvesEquivalentPaths_data();
  void fileNormalizationResolvesEquivalentPaths();
  void registryContainmentPreservesLiteralDotSegments();
};

void PathPolicyTests::pathMatchingHonorsSegmentBoundaries() {
  QCOMPARE(text(uwf::app::paths::stripTrailingSeparators(R"(C:\Data\\/)")), QStringLiteral("C:\\Data"));
  QVERIFY(uwf::app::paths::contains(std::string(R"(C:\DATA)"), std::string(R"(c:\data)")));
  QVERIFY(uwf::app::paths::contains(std::string(R"(c:/data)"), std::string(R"(C:/Data/Child/file.bin)")));
  QVERIFY(!uwf::app::paths::contains(std::string(R"(C:\Data)"), std::string(R"(C:\Database\file.bin)")));
  QVERIFY(!uwf::app::paths::contains(std::string(R"(C:\Data)"), std::string(R"(C:\Data2)")));
  QVERIFY(!uwf::app::paths::contains(std::string(""), std::string(R"(C:\Data)")));

  const std::vector<std::string> exclusions = {R"(C:\Program Data\)", R"(D:\Cache)"};
  QCOMPARE(text(uwf::app::paths::findCoveringExclusion(exclusions, R"(c:/program data/app/state.db)")), QStringLiteral("C:\\Program Data\\"));
  QVERIFY(uwf::app::paths::findCoveringExclusion(exclusions, R"(C:\Program)").empty());
}

void PathPolicyTests::pathMatchingPropertiesPreserveSegmentSemantics() {
  const std::array roots = {std::string(R"(C:\Root\Node)"), std::string(R"(c:/root/node)"), std::string(R"(C:/ROOT\NODE)")};
  for (const auto& root : roots) {
    const std::string canonical = uwf::app::paths::stripTrailingSeparators(root + R"(\//)");
    QVERIFY(uwf::app::paths::contains(std::string(uwf::app::paths::stripTrailingSeparators(root)), std::string(canonical)));
    QVERIFY(uwf::app::paths::contains(std::string(uwf::app::paths::stripTrailingSeparators(root)), std::string(canonical + R"(/child\leaf.bin)")));
    QVERIFY(!uwf::app::paths::contains(std::string(uwf::app::paths::stripTrailingSeparators(root)), std::string(canonical + "-sibling")));
    QVERIFY(!uwf::app::paths::contains(std::string(uwf::app::paths::stripTrailingSeparators(root)), std::string(canonical.substr(0, canonical.size() - 1))));
  }

  const std::vector<std::string> exclusions{R"(C:\Root\)", R"(C:\Root\Longer\)", R"(D:/Data/)"};
  QCOMPARE(uwf::app::paths::findCoveringExclusion(exclusions, R"(c:/root/longer/file)"), exclusions.front());
  QVERIFY(uwf::app::paths::findCoveringExclusion(exclusions, R"(C:\Rooted\file)").empty());
  QVERIFY(uwf::app::paths::stripTrailingSeparators("////").empty());
}

void PathPolicyTests::fileNormalizationResolvesEquivalentPaths_data() {
  QTest::addColumn<QString>("path");
  QTest::addColumn<QString>("expected");
  QTest::newRow("absolute") << QStringLiteral("Z:/Data/State.bin") << QStringLiteral("Z:\\Data\\State.bin");
  QTest::newRow("volume-rooted") << QStringLiteral("/Data/State.bin") << QStringLiteral("Z:\\Data\\State.bin");
  QTest::newRow("dot-segments") << QStringLiteral("Z:/Data/./Cache/../State.bin") << QStringLiteral("Z:\\Data\\State.bin");
  QTest::newRow("rooted-dot-segments") << QStringLiteral("/Scratch/../Data/State.bin/") << QStringLiteral("Z:\\Data\\State.bin");
  QTest::newRow("volume-root") << QStringLiteral("/Data/..") << QStringLiteral("Z:\\");
  QTest::newRow("other-volume") << QStringLiteral("Y:/Data/../State.bin") << QStringLiteral("Y:\\State.bin");
  QTest::newRow("unc") << QStringLiteral("//server/share/Data/../State.bin") << QStringLiteral("\\\\server\\share\\State.bin");
  QTest::newRow("empty") << QString{} << QString{};
}

void PathPolicyTests::fileNormalizationResolvesEquivalentPaths() {
  QFETCH(QString, path);
  QFETCH(QString, expected);
  const auto normalized = uwf::app::paths::normalize(path, QStringLiteral("Z:"));
  QCOMPARE(normalized, expected);
  QCOMPARE(uwf::app::paths::normalize(normalized, QStringLiteral("Z:")), normalized);
}

void PathPolicyTests::registryContainmentPreservesLiteralDotSegments() {
  using uwf::app::paths::contains;
  const QString root = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor");
  QVERIFY(contains(root + QStringLiteral("\\.."), root + QStringLiteral("\\..\\Child")));
  QVERIFY(!contains(root + QStringLiteral("\\Child"), root + QStringLiteral("\\.\\Child")));
  QVERIFY(!contains(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Child"), root + QStringLiteral("\\..\\Child")));
}

}  // namespace

QTEST_APPLESS_MAIN(PathPolicyTests)

#include "PathPolicyTests.moc"
