// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>

#include "domain/uwf/solvers/AllVolumeProtectionSolver.h"

namespace {
using namespace uwf::compat;
constexpr char a[] = "11111111-1111-1111-1111-111111111111";
constexpr char b[] = "22222222-2222-2222-2222-222222222222";
class AllVolumeProtectionSolverTests final : public QObject {
  Q_OBJECT
 private slots:
  void aggregatePredicatesAreIndependentAndPreserveUnknown() {
    AllVolumeProtectionObservation facts{true, {{a, true, {}}, {b, true, {}}}, {}, {}};
    QCOMPARE(solveGetAllProtected({}, facts), std::optional<bool>{true});
    QCOMPARE(solveGetAllUnprotected({}, facts), std::optional<bool>{false});
    facts.volumes[0].nextProtected = false;
    QCOMPARE(solveGetAllProtected({}, facts), std::optional<bool>{false});
    QCOMPARE(solveGetAllUnprotected({}, facts), std::optional<bool>{false});
    facts.volumes[1].nextProtected = false;
    QCOMPARE(solveGetAllProtected({}, facts), std::optional<bool>{false});
    QCOMPARE(solveGetAllUnprotected({}, facts), std::optional<bool>{true});
    facts.volumes[0].nextProtected.reset();
    QVERIFY(!solveGetAllProtected({}, facts));
    QVERIFY(!solveGetAllUnprotected({}, facts));
    facts.volumes[0].nextProtected = false;
    facts.failure = Failure{ErrorKind::Unavailable, Effect::None, "Scope read failed", {}};
    QVERIFY(!solveGetAllProtected({}, facts));
    QVERIFY(!solveGetAllUnprotected({}, facts));
    facts.failure.reset();
    facts.volumes.clear();
    QVERIFY(!solveGetAllProtected({}, facts));
    QCOMPARE(solveGetAllUnprotected({}, facts), std::optional<bool>{true});
    facts.scopeConfirmed = false;
    QVERIFY(!solveGetAllUnprotected({}, facts));
  }
  void writeSolversValidateSessionAdmissionAndDirectionSpecificScope() {
    AllVolumeProtectionFacts facts{true, false, {true, {{a, true, {}}, {b, false, {}}}, {}, {}}};
    const auto protect = solveProtectAll({}, facts), unprotect = solveUnprotectAll({}, facts);
    QCOMPARE(protect.resolution, AllVolumeProtectionResolution::Ready);
    QCOMPARE(unprotect.resolution, AllVolumeProtectionResolution::Ready);
    facts.hormEnabled = true;
    QCOMPARE(solveProtectAll({}, facts).resolution, AllVolumeProtectionResolution::Rejected);
    QCOMPARE(solveUnprotectAll({}, facts).resolution, AllVolumeProtectionResolution::Rejected);
    facts.hormEnabled.reset();
    QCOMPARE(solveProtectAll({}, facts).resolution, AllVolumeProtectionResolution::ReadRequired);
    facts.administrator = false;
    QCOMPARE(solveUnprotectAll({}, facts).failure->kind, ErrorKind::AccessDenied);
    facts.administrator = true;
    facts.hormEnabled = false;
    facts.observation.volumes.clear();
    QCOMPARE(solveProtectAll({}, facts).resolution, AllVolumeProtectionResolution::Rejected);
    QCOMPARE(solveUnprotectAll({}, facts).resolution, AllVolumeProtectionResolution::Ready);
  }
  void volumeProtectionCannotAcknowledgeVolatileRomConfiguration() {
    using R = AllVolumeProtectionResolution;
    AllVolumeProtectionFacts facts{true, false, {true, {{a, true, {}}, {b, false, {}}}, {}, {}}};
    facts.mode = {true, true, {false, 4, 0, false}};
    const auto assertBoth = [&](const R expected) {
      return solveProtectAll({}, facts).resolution == expected && solveUnprotectAll({}, facts).resolution == expected;
    };
    QVERIFY(assertBoth(R::Rejected));
    QCOMPARE(solveProtectAll({}, facts).failure->kind, ErrorKind::AccessDenied);
    facts.mode.systemVolumeProtected = false;
    QVERIFY(assertBoth(R::Ready));
    facts.mode.systemVolumeProtected = true;
    facts.mode.filterEnabled = false;
    QVERIFY(assertBoth(R::Ready));
    facts.mode.filterEnabled = true;
    facts.mode.runtime.romDisposition = 3;
    QVERIFY(assertBoth(R::Ready));
    facts.mode.runtime.romDisposition = 4;
    facts.mode.runtime.fullCommit = true;
    QVERIFY(assertBoth(R::Ready));
    facts.mode.runtime.fullCommit = false;
    facts.mode.runtime.romDisposition = 0;
    facts.mode.systemVolumeProtected.reset();
    QVERIFY(solveProtectAll({}, facts).resolution != R::Rejected);
    QVERIFY(solveUnprotectAll({}, facts).resolution != R::Rejected);
    facts.mode = {};
    QVERIFY(solveProtectAll({}, facts).resolution != R::Rejected);
    QVERIFY(solveUnprotectAll({}, facts).resolution != R::Rejected);
  }
};
}  // namespace
QTEST_APPLESS_MAIN(AllVolumeProtectionSolverTests)
#include "AllVolumeProtectionSolverTests.moc"
