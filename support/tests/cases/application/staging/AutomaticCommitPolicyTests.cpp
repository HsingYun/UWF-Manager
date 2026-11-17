// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <optional>

#include "application/staging/policy/AutomaticCommitPolicy.h"

Q_DECLARE_METATYPE(uwf::app::AutomaticCommitContext)

namespace {
using namespace uwf::app;
using Disposition = AutomaticCommitDisposition;

AutomaticCommitContext ordinaryContext() {
  AutomaticCommitContext context;
  context.overlay.transportAvailable = true;
  context.overlay.current.filterEnabled = context.overlay.next.filterEnabled = {PolicyObservationStatus::Available, true};
  context.overlay.current.overlayType = context.overlay.next.overlayType = {PolicyObservationStatus::Available, 0};
  context.overlay.current.flags = context.overlay.next.flags = {PolicyObservationStatus::Available, OverlayFlags{0}};
  context.overlay.resetPersistentState = {PolicyObservationStatus::Available, 0};
  context.hormEnabled = false;
  context.romDisposition = 0;
  context.fullCommit = false;
  return context;
}
AutomaticCommitContext romContext() {
  auto context = ordinaryContext();
  context.overlay.current.flags.value = OverlayFlags{OverlayFlags::ReadOnlyMedia};
  return context;
}
AutomaticCommitContext retainedContext() {
  auto context = ordinaryContext();
  context.overlay.current.overlayType = context.overlay.next.overlayType = {PolicyObservationStatus::Available, 1};
  context.overlay.current.flags = context.overlay.next.flags = {PolicyObservationStatus::Available, OverlayFlags{OverlayFlags::Persistent}};
  return context;
}

class AutomaticCommitPolicyTests final : public QObject {
  Q_OBJECT
 private slots:
  void perVolumeDisposition_data() {
    QTest::addColumn<AutomaticCommitContext>("context");
    QTest::addColumn<int>("currentProtected");
    QTest::addColumn<int>("nextProtected");
    QTest::addColumn<int>("expected");
    const auto add = [](const char* name, const AutomaticCommitContext& context, const Disposition expected, const int current = 1, const int next = 1) {
      QTest::newRow(name) << context << current << next << static_cast<int>(expected);
    };

    add("ordinary-protected", ordinaryContext(), Disposition::CheckDurability);
    auto context = ordinaryContext();
    context.overlay.next.flags.value = OverlayFlags{OverlayFlags::ReadOnlyMedia};
    add("next-rom-does-not-pause-current", context, Disposition::CheckDurability);
    add("current-rom", romContext(), Disposition::SkipReadOnlyMedia);
    add("current-rom-next-unprotected", romContext(), Disposition::SkipReadOnlyMedia, 1, 0);
    add("current-rom-next-protection-unknown", romContext(), Disposition::SkipReadOnlyMedia, 1, -1);
    context = romContext();
    context.overlay.next = {};
    add("current-rom-next-overlay-unknown", context, Disposition::SkipReadOnlyMedia);
    context = romContext();
    context.overlay.next.filterEnabled.value = false;
    add("current-rom-next-filter-off", context, Disposition::SkipReadOnlyMedia);
    context = romContext();
    context.romDisposition = uwf::compat::disposition::filterChange;
    add("rom-filter-change-disposition", context, Disposition::CheckDurability);
    context.romDisposition = uwf::compat::disposition::horm;
    add("rom-horm-disposition", context, Disposition::SkipHormTransition);
    context.romDisposition = uwf::compat::disposition::horm | uwf::compat::disposition::filterChange;
    add("disabled-horm-before-first-restart", context, Disposition::SkipHormTransition);
    context.romDisposition = uwf::compat::disposition::fullCommit;
    add("rom-full-commit-disposition", context, Disposition::SkipFullCommit);
    context.romDisposition = uwf::compat::disposition::filterChange | uwf::compat::disposition::fullCommit;
    add("rom-full-commit-overrides-filter-change", context, Disposition::SkipFullCommit);
    context = romContext();
    context.romDisposition.reset();
    add("rom-disposition-unknown", context, Disposition::Unavailable);
    context.fullCommit = true;
    add("rom-full-commit-known-without-disposition", context, Disposition::SkipFullCommit);
    context = romContext();
    context.hormEnabled = true;
    add("rom-horm-pauses-before-durability-admission", context, Disposition::SkipHormActive);
    context.hormEnabled.reset();
    add("rom-horm-unknown", context, Disposition::Unavailable);
    context = romContext();
    context.overlay.current.filterEnabled = {};
    add("rom-current-filter-unknown", context, Disposition::Unavailable);
    add("unknown-filter-cannot-certify-unprotected-writes", context, Disposition::Unavailable, 0);
    context = romContext();
    context.overlay.current.flags = {};
    add("rom-current-flags-unknown", context, Disposition::Unavailable);
    context.overlay.current.flags = {PolicyObservationStatus::Failed, OverlayFlags{OverlayFlags::ReadOnlyMedia}};
    add("rom-failed-flags-value-is-not-authoritative", context, Disposition::Unavailable);
    add("rom-current-protection-unknown", romContext(), Disposition::Unavailable, -1);
    context = romContext();
    context.overlay.transportAvailable = false;
    add("rom-transport-unavailable", context, Disposition::Unavailable);

    context = romContext();
    context.overlay.current.filterEnabled.value = false;
    add("rom-current-filter-off-already-persistent", context, Disposition::AlreadyPersistent);
    add("filter-off-needs-no-protection-observation", context, Disposition::AlreadyPersistent, -1);
    add("rom-unprotected-already-persistent", romContext(), Disposition::AlreadyPersistent, 0);
    context = ordinaryContext();
    context.overlay.current.flags = {};
    context.hormEnabled.reset();
    add("unprotected-needs-no-mode-observation", context, Disposition::AlreadyPersistent, 0);

    add("retained-disk", retainedContext(), Disposition::SkipRetainedOverlay);
    add("retained-disk-next-unprotected", retainedContext(), Disposition::CheckDurability, 1, 0);
    add("retained-disk-next-protection-unknown", retainedContext(), Disposition::CheckDurability, 1, -1);
    context = retainedContext();
    context.overlay.resetPersistentState.value = 1;
    add("scheduled-clear-resumes-commits", context, Disposition::CheckDurability);
    context.overlay.resetPersistentState = {};
    add("retention-reset-unknown", context, Disposition::CheckDurability);
    context.overlay.resetPersistentState = {PolicyObservationStatus::Failed, 0};
    add("retention-failed-reset-value-is-not-authoritative", context, Disposition::CheckDurability);
    context.overlay.resetPersistentState = {PolicyObservationStatus::Available, 255};
    add("retention-use-saved-mode-needs-confirmed-preservation", context, Disposition::CheckDurability);
    context = retainedContext();
    context.overlay.next.filterEnabled.value = false;
    add("next-filter-off-resumes-commits", context, Disposition::CheckDurability);
    context.overlay.next.filterEnabled = {};
    add("retention-next-filter-unknown", context, Disposition::CheckDurability);
    context.overlay.next.filterEnabled = {PolicyObservationStatus::Failed, true};
    add("retention-failed-next-filter-value-is-not-authoritative", context, Disposition::CheckDurability);
    context = retainedContext();
    context.overlay.current.overlayType.value = 0;
    add("ram-does-not-retain-overlay", context, Disposition::CheckDurability);
    context.overlay.current.overlayType = {PolicyObservationStatus::Failed, 1};
    add("retention-failed-current-type-value-is-not-authoritative", context, Disposition::CheckDurability);
    context = retainedContext();
    context.overlay.current.flags.value = OverlayFlags{0};
    add("nonpersistent-disk-does-not-pause", context, Disposition::CheckDurability);
    context = retainedContext();
    context.overlay.next.overlayType.value = 0;
    add("next-ram-cannot-retain-current-persistent-overlay", context, Disposition::CheckDurability);
    context.overlay.next.overlayType = {};
    add("next-type-unknown-cannot-certify-retention", context, Disposition::CheckDurability);
    context.overlay.next.overlayType = {PolicyObservationStatus::Failed, 1};
    add("failed-next-type-is-not-retention-proof", context, Disposition::CheckDurability);
    context = retainedContext();
    context.overlay.next.flags.value = OverlayFlags{0};
    add("next-persistence-off-resumes-commits", context, Disposition::CheckDurability);
    context.overlay.next.flags = {};
    add("next-flags-unknown-cannot-certify-retention", context, Disposition::CheckDurability);
    context.overlay.next.flags = {PolicyObservationStatus::Failed, OverlayFlags{OverlayFlags::Persistent}};
    add("failed-next-flags-is-not-retention-proof", context, Disposition::CheckDurability);
    context.overlay.next.flags = {PolicyObservationStatus::Available, OverlayFlags{OverlayFlags::Persistent | OverlayFlags::ReadOnlyMedia}};
    add("next-rom-does-not-certify-persistent-retention", context, Disposition::CheckDurability);
    context = retainedContext();
    context.hormEnabled = true;
    add("horm-pause-is-distinct-from-retention", context, Disposition::SkipHormActive);
    context.hormEnabled.reset();
    add("retained-overlay-horm-unknown", context, Disposition::Unavailable);
  }
  void perVolumeDisposition() {
    QFETCH(AutomaticCommitContext, context);
    QFETCH(int, currentProtected);
    QFETCH(int, nextProtected);
    QFETCH(int, expected);
    const auto protection = [](const int value) { return value < 0 ? std::optional<bool>{} : std::optional{value != 0}; };
    const auto actual = automaticFileCommitDisposition(context, protection(currentProtected), protection(nextProtected));
    QCOMPARE(actual, static_cast<Disposition>(expected));
    QCOMPARE(automaticFileCommitPaused(actual), actual == Disposition::SkipReadOnlyMedia || actual == Disposition::SkipRetainedOverlay ||
                                                    actual == Disposition::SkipHormActive || actual == Disposition::SkipHormTransition ||
                                                    actual == Disposition::SkipFullCommit);
    if (actual == Disposition::AlreadyPersistent) QVERIFY(!automaticFileCommitPaused(actual));
    if (actual == Disposition::Unavailable) QVERIFY(!automaticFileCommitPaused(actual));
  }
  void summaryDistinguishesDurableTargetsFromPausedTargets_data() {
    QTest::addColumn<QList<int>>("targets");
    QTest::addColumn<bool>("rom");
    QTest::addColumn<bool>("retained");
    QTest::addColumn<bool>("required");
    QTest::addColumn<bool>("unavailable");
    const int check = static_cast<int>(Disposition::CheckDurability);
    const int durable = static_cast<int>(Disposition::AlreadyPersistent);
    const int rom = static_cast<int>(Disposition::SkipReadOnlyMedia);
    const int retained = static_cast<int>(Disposition::SkipRetainedOverlay);
    const int unavailable = static_cast<int>(Disposition::Unavailable);
    QTest::newRow("empty") << QList<int>{} << false << false << false << false;
    QTest::newRow("already-persistent") << QList<int>{durable, durable} << false << false << false << false;
    QTest::newRow("commit-required") << QList<int>{check, durable} << false << false << true << false;
    QTest::newRow("rom-paused") << QList<int>{rom} << true << false << false << false;
    QTest::newRow("retained-paused") << QList<int>{retained} << false << true << false << false;
    QTest::newRow("rom-with-unprotected-target") << QList<int>{rom, durable} << true << false << false << false;
    QTest::newRow("retained-with-unprotected-target") << QList<int>{retained, durable} << false << true << false << false;
    QTest::newRow("rom-partial") << QList<int>{rom, check, durable} << true << false << true << false;
    QTest::newRow("retained-partial") << QList<int>{check, durable, retained} << false << true << true << false;
    QTest::newRow("multiple-paused-reasons") << QList<int>{rom, retained, durable} << true << true << false << false;
    QTest::newRow("multiple-paused-reasons-and-commit-target") << QList<int>{rom, retained, check} << true << true << true << false;
    QTest::newRow("unknown-target") << QList<int>{unavailable} << false << false << false << true;
    QTest::newRow("durable-and-unknown-targets") << QList<int>{durable, unavailable} << false << false << false << true;
    QTest::newRow("rom-and-unknown-targets") << QList<int>{rom, unavailable} << true << false << false << true;
    QTest::newRow("retained-and-unknown-targets") << QList<int>{retained, unavailable} << false << true << false << true;
    QTest::newRow("mixed-paused-commit-and-unknown-targets") << QList<int>{rom, retained, check, unavailable, durable} << true << true << true << true;
  }
  void forcedPausesRemainDistinctFromRetainedDataAndUnknownState() {
    AutomaticCommitSummary summary;
    summary.include(Disposition::SkipHormActive);
    summary.include(Disposition::SkipHormTransition);
    summary.include(Disposition::SkipFullCommit);
    QVERIFY(summary.hormPaused);
    QVERIFY(summary.hormTransitionPaused);
    QVERIFY(summary.fullCommitPaused);
    QVERIFY(summary.unsupported());
    QVERIFY(summary.paused());
    QVERIFY(!summary.alreadyPersistent);
    QVERIFY(!summary.retainedOverlayPaused);
    QVERIFY(!summary.partiallyPaused());
    summary.include(Disposition::CheckDurability);
    QVERIFY(summary.partiallyPaused());
    summary.include(Disposition::Unavailable);
    QVERIFY(summary.unavailable);
  }
  void summaryDistinguishesDurableTargetsFromPausedTargets() {
    QFETCH(QList<int>, targets);
    QFETCH(bool, rom);
    QFETCH(bool, retained);
    QFETCH(bool, required);
    QFETCH(bool, unavailable);
    AutomaticCommitSummary summary;
    for (const auto target : targets) summary.include(static_cast<Disposition>(target));
    QCOMPARE(summary.readOnlyMediaPaused, rom);
    QCOMPARE(summary.retainedOverlayPaused, retained);
    QCOMPARE(summary.commitRequired, required);
    QCOMPARE(summary.unavailable, unavailable);
    QCOMPARE(summary.paused(), rom || retained);
    QCOMPARE(summary.partiallyPaused(), (rom || retained) && required);
  }
};
}  // namespace

QTEST_APPLESS_MAIN(AutomaticCommitPolicyTests)
#include "AutomaticCommitPolicyTests.moc"
