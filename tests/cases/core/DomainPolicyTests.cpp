// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <vector>

#include "core/OverlayWritePlan.h"
#include "core/RegistryExclusionPolicy.h"
#include "core/UwfModel.h"
#include "shared/StringUtil.h"

namespace {

class DomainPolicyTests final : public QObject {
  Q_OBJECT

 private slots:
  void registryExclusionPolicyClassifiesDocumentedBoundaries();
  void pendingChangesCountsEveryBusinessField();
  void exclusionSortingIsCaseInsensitiveAndStable();
  void overlayUsageBoundaries();
};

void DomainPolicyTests::overlayUsageBoundaries() {
  using namespace uwf::core;
  using enum OverlayUsageSeverity;
  struct Boundary { uint32_t used, warning, critical; OverlayUsageSeverity expected; };
  const std::array boundaries{
      Boundary{0, 4, 8, Normal}, Boundary{3, 4, 8, Normal}, Boundary{4, 4, 8, Warning}, Boundary{7, 4, 8, Warning},
      Boundary{8, 4, 8, Critical}, Boundary{9, 4, 8, Critical}, Boundary{8, 0, 8, Critical}, Boundary{4, 4, 0, Warning},
      Boundary{4, 8, 4, Critical}, Boundary{std::numeric_limits<uint32_t>::max(), 0, 0, Normal}};
  for (const auto& boundary : boundaries)
    QCOMPARE(overlayUsageSeverity(boundary.used, boundary.warning, boundary.critical), boundary.expected);
  const auto maximum = std::numeric_limits<uint32_t>::max();
  for (const uint32_t available : {0u, 1u, maximum})
    for (const uint32_t used : {0u, 1u, maximum}) {
      const auto total = overlayTotalMb({available, used});
      QCOMPARE(total - used, static_cast<uint64_t>(available));
      QVERIFY(total >= used && total >= available);
    }
}

void DomainPolicyTests::registryExclusionPolicyClassifiesDocumentedBoundaries() {
  using enum uwf::core::RegExclusionClass;
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(HKLM\Software\Vendor\Product)"), Allowed);
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(hkey_local_machine\system\CurrentControlSet\Services\Demo)"), Allowed);
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(HKLM\SOFTWARE)"), Container);
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(HKEY_LOCAL_MACHINE)"), Container);
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC)"), MachineAccount);
  QCOMPARE(uwf::core::classifyRegistryExclusion(R"(HKCU\Software\Vendor)"), OutOfScope);
  QCOMPARE(uwf::core::classifyRegistryExclusion(""), OutOfScope);
}

void DomainPolicyTests::pendingChangesCountsEveryBusinessField() {
  uwf::core::PendingChanges changes;
  QVERIFY(changes.empty());
  QCOMPARE(changes.count(), std::size_t{0});

  changes.setFilterEnabled = false;
  changes.setOverlay.type = uwf::core::OverlayType::Disk;
  changes.setOverlay.maximumSizeMb = 4096;
  changes.setOverlay.warningThresholdMb = 2048;
  changes.setOverlay.criticalThresholdMb = 3072;
  changes.volumeProtect["C:"] = true;
  changes.volumeBindByVolumeName["D:"] = false;
  changes.addFileExclusions["C:"] = {R"(C:\A)", R"(C:\B)"};
  changes.removeFileExclusions["D:"] = {R"(D:\Old)"};
  changes.addRegistryExclusions = {R"(HKLM\Software\A)"};
  changes.removeRegistryExclusions = {R"(HKLM\System\B)"};
  changes.setPersistDomainSecretKey = true;
  changes.setPersistTSCAL = false;

  QVERIFY(!changes.empty());
  QCOMPARE(changes.count(), std::size_t{14});
  QVERIFY(changes.setOverlay.touchesOverlayConfig());

  changes.clear();
  QVERIFY(changes.empty());
  QCOMPARE(changes.count(), std::size_t{0});

  changes.addFileExclusions["C:"] = {};
  changes.removeFileExclusions["D:"] = {};
  QVERIFY(changes.empty());
  QCOMPARE(changes.count(), std::size_t{0});
}

void DomainPolicyTests::exclusionSortingIsCaseInsensitiveAndStable() {
  std::vector<std::string> items = {R"(C:\zeta)", R"(c:\Alpha)", R"(C:\ALPHA)", R"(C:\beta)"};
  uwf::core::sortExclusions(items);
  QCOMPARE(items.size(), std::size_t{3});
  QCOMPARE(QString::fromUtf8(items[0]), QStringLiteral("c:\\Alpha"));
  QCOMPARE(QString::fromUtf8(items[1]), QStringLiteral("C:\\beta"));
  QCOMPARE(QString::fromUtf8(items[2]), QStringLiteral("C:\\zeta"));

  uwf::core::UwfSnapshot snapshot;
  snapshot.current.fileExclusions["Volume{A}"] = {"B", "a", "A"};
  snapshot.next.registryExclusions = {"z", "Y", "y"};
  uwf::core::sortSnapshot(snapshot);
  QCOMPARE(snapshot.current.fileExclusions.at("Volume{A}").size(), std::size_t{2});
  QCOMPARE(snapshot.next.registryExclusions.size(), std::size_t{2});

  std::vector<std::string> permutation = {"Zulu", "alpha", "ALPHA", "beta", "BETA"};
  std::ranges::sort(permutation);
  do {
    auto sorted = permutation;
    uwf::core::sortExclusions(sorted);
    QCOMPARE(sorted.size(), std::size_t{3});
    QCOMPARE(uwf::toLowerAscii(sorted[0]), std::string("alpha"));
    QCOMPARE(uwf::toLowerAscii(sorted[1]), std::string("beta"));
    QCOMPARE(uwf::toLowerAscii(sorted[2]), std::string("zulu"));
    const auto once = sorted;
    uwf::core::sortExclusions(sorted);
    QCOMPARE(sorted, once);
  } while (std::ranges::next_permutation(permutation).found);
}

}  // namespace

QTEST_APPLESS_MAIN(DomainPolicyTests)

#include "DomainPolicyTests.moc"
