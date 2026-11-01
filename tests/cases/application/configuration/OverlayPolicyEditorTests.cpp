// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>

#include "application/configuration/overlay/OverlayPolicyEditor.h"
#include "support/app/OverlayPolicyFixture.h"

namespace {
using namespace uwf;
using Field = app::OverlayPolicyField;
using Issue = app::OverlayPolicyIssue;
class OverlayPolicyEditorTests final : public QObject {
  Q_OBJECT
 private:
  core::UwfSnapshot snapshot() const {
    core::UwfSnapshot s;
    s.uwfAvailable = s.elevated = true;
    s.current.overlay.type = s.next.overlay.type = core::OverlayType::Disk;
    return s;
  }
  app::OverlayPolicySnapshot editablePolicy() const {
    auto policy = test::application::diskPersistentPolicy();
    policy.current.filterEnabled.value = false;
    return policy;
  }
 private slots:
  void importAndModalShareAdmissionWithoutValidatingIntermediatePolicy_data() {
    QTest::addColumn<int>("restriction");
    QTest::newRow("editable") << -1;
    QTest::newRow("unavailable") << 0;
    QTest::newRow("not-elevated") << 1;
    QTest::newRow("horm-active") << 2;
    QTest::newRow("controls-disabled") << 3;
  }
  void importAndModalShareAdmissionWithoutValidatingIntermediatePolicy() {
    QFETCH(int, restriction);
    app::OverlayPolicyEditor editor;
    auto state = snapshot();
    state.uwfAvailable = restriction != 0;
    state.elevated = restriction != 1;
    editor.reset(editablePolicy(), state, restriction == 2 ? std::optional{true} : std::nullopt);
    if (restriction == 3) editor.setControlsEnabled(false);
    const bool allowed = restriction < 0;
    QCOMPARE(editor.editingAllowed(), allowed);
    QCOMPARE(editor.canCommit(editor.beginEdit()), allowed);
    QSignalSpy changed(&editor, &app::OverlayPolicyEditor::pendingChanged);
    // Reset + disabled Persistent is deliberately inconsistent mid-import.
    // Admission must block read-only edits without validating a partial batch.
    QCOMPARE(editor.importChanges({.persistent = false, .reset = app::PersistentReset::Clear}), allowed);
    QCOMPARE(editor.pendingChanges().empty(), !allowed);
    QCOMPARE(changed.size(), allowed ? 1 : 0);
    if (allowed) {
      QVERIFY(!editor.conflicts(editor.beginEdit()).empty());
      QVERIFY(!editor.canCommit(editor.beginEdit()));
    }
  }
  void flagWritesRequireCurrentFilterOff_data() {
    QTest::addColumn<int>("field");
    QTest::addColumn<int>("currentStatus");
    QTest::addColumn<bool>("hasCurrentValue");
    QTest::addColumn<bool>("currentFilter");
    QTest::addColumn<bool>("nextFilter");
    QTest::addColumn<bool>("ram");
    QTest::addColumn<bool>("initial");
    QTest::addColumn<int>("issue");
    for (const bool current : {false, true})
      for (const bool next : {false, true})
        for (const bool ram : {false, true})
          for (const auto field : {Field::Passthrough, Field::Persistent})
            for (const bool initial : {false, true}) {
              if ((current && ram) || (ram && field == Field::Persistent && initial)) continue;
              const auto issue = current ? Issue::CurrentFilterMustBeOff : ram && field == Field::Persistent ? Issue::PersistentNeedsDisk : Issue::None;
              QTest::newRow(qUtf8Printable(QStringLiteral("current-%1-next-%2-%3-field-%4-from-%5")
                                               .arg(current)
                                               .arg(next)
                                               .arg(ram ? "ram" : "disk")
                                               .arg(static_cast<int>(field))
                                               .arg(initial)))
                  << static_cast<int>(field) << static_cast<int>(app::PolicyObservationStatus::Available) << true << current << next << ram << initial
                  << static_cast<int>(issue);
            }
    for (const auto status : {app::PolicyObservationStatus::Unobserved, app::PolicyObservationStatus::Unsupported, app::PolicyObservationStatus::Failed,
                              app::PolicyObservationStatus::Available})
      for (const auto field : {Field::Passthrough, Field::Persistent})
        QTest::newRow(qUtf8Printable(QStringLiteral("unknown-current-%1-field-%2").arg(static_cast<int>(status)).arg(static_cast<int>(field))))
            << static_cast<int>(field) << static_cast<int>(status) << false << false << true << false << true << static_cast<int>(Issue::FlagsUnavailable);
  }
  void flagWritesRequireCurrentFilterOff() {
    QFETCH(int, field);
    QFETCH(int, currentStatus);
    QFETCH(bool, hasCurrentValue);
    QFETCH(bool, currentFilter);
    QFETCH(bool, nextFilter);
    QFETCH(bool, ram);
    QFETCH(bool, initial);
    QFETCH(int, issue);
    const auto selected = static_cast<Field>(field);
    const auto expected = static_cast<Issue>(issue);
    auto policy = test::application::diskPersistentPolicy();
    policy.current.filterEnabled = {static_cast<app::PolicyObservationStatus>(currentStatus), hasCurrentValue ? std::optional{currentFilter} : std::nullopt};
    policy.next.filterEnabled.value = nextFilter;
    policy.next.overlayType.value = ram ? 0 : 1;
    const auto bit = selected == Field::Passthrough ? app::OverlayFlags::Passthrough : app::OverlayFlags::Persistent;
    policy.next.flags.value = app::OverlayFlags{initial ? bit : 0};
    auto state = snapshot();
    state.current.filter.enabled = currentFilter;
    state.next.filter.enabled = nextFilter;
    state.next.overlay.type = ram ? core::OverlayType::RAM : core::OverlayType::Disk;
    app::OverlayPolicyEditor editor;
    editor.reset(policy, state);
    auto edit = editor.beginEdit();
    QCOMPARE(editor.disabledReason(edit, selected), expected);
    QVERIFY(editor.canCommit(edit));  // Retaining observed flags does not request a write.
    app::OverlayPolicyEditor::setValue(edit, selected, !initial);
    const bool allowed = expected == Issue::None;
    QCOMPARE(editor.conflicts(edit), allowed ? std::vector<Issue>{} : std::vector{expected});
    QCOMPARE(editor.canCommit(edit), allowed);
    QCOMPARE(editor.commit(edit), allowed);
    if (allowed) {
      const auto pending = editor.pendingChanges();
      QCOMPARE(app::OverlayPolicyEditor::value(pending, selected), std::optional{!initial});
    } else {
      QVERIFY(editor.pendingChanges().empty());
    }
  }
  void resetChangesRequireAnActivePersistentDiskInBothDirections_data() {
    QTest::addColumn<int>("current");
    QTest::addColumn<bool>("initial");
    QTest::addColumn<int>("type");
    QTest::addColumn<bool>("persistent");
    QTest::addColumn<int>("issue");
    for (const bool initial : {false, true}) {
      for (const int current : {-1, 0, 1})
        QTest::newRow(qUtf8Printable(QStringLiteral("current-%1-reset-%2").arg(current).arg(initial)))
            << current << initial << 1 << true
            << static_cast<int>(current < 0    ? Issue::FlagsUnavailable
                                : current == 0 ? Issue::CurrentPersistentRequired
                                               : Issue::None);
      QTest::newRow(initial ? "cancel-on-ram" : "clear-on-ram") << 1 << initial << 0 << true << static_cast<int>(Issue::CurrentPersistentRequired);
      QTest::newRow(initial ? "cancel-without-persistent" : "clear-without-persistent")
          << 1 << initial << 1 << false << static_cast<int>(Issue::CurrentPersistentRequired);
    }
  }
  void resetChangesRequireAnActivePersistentDiskInBothDirections() {
    QFETCH(int, current);
    QFETCH(bool, initial);
    QFETCH(int, type);
    QFETCH(bool, persistent);
    QFETCH(int, issue);
    auto policy = test::application::diskPersistentPolicy();
    policy.current.filterEnabled = current < 0 ? app::PolicyObservation<bool>{app::PolicyObservationStatus::Failed}
                                               : app::PolicyObservation<bool>{app::PolicyObservationStatus::Available, current != 0};
    policy.current.overlayType.value = type;
    policy.current.flags.value = app::OverlayFlags{persistent ? app::OverlayFlags::Persistent : 0};
    policy.resetPersistentState.value = initial ? 1 : 0;
    app::OverlayPolicyEditor editor;
    editor.reset(policy, snapshot());
    auto edit = editor.beginEdit();
    QCOMPARE(editor.disabledReason(edit, Field::Reset), static_cast<Issue>(issue));
    app::OverlayPolicyEditor::setValue(edit, Field::Reset, !initial);
    const bool allowed = static_cast<Issue>(issue) == Issue::None;
    QCOMPARE(editor.canCommit(edit), allowed);
    QCOMPARE(editor.commit(edit), allowed);
    if (allowed)
      QCOMPARE(editor.pendingChanges().reset, std::optional{initial ? app::PersistentReset::Preserve : app::PersistentReset::Clear});
    else
      QVERIFY(editor.pendingChanges().empty());
  }
  void snapshotEligibilityIsIndependentOfTheCallerEditingGate_data() {
    QTest::addColumn<bool>("uwfAvailable");
    QTest::addColumn<bool>("elevated");
    QTest::newRow("available-administrator") << true << true;
    QTest::newRow("unavailable-administrator") << false << true;
    QTest::newRow("available-readonly") << true << false;
    QTest::newRow("unavailable-readonly") << false << false;
  }
  void snapshotEligibilityIsIndependentOfTheCallerEditingGate() {
    QFETCH(bool, uwfAvailable);
    QFETCH(bool, elevated);
    app::OverlayPolicyEditor editor;
    const auto policy = editablePolicy();
    auto state = snapshot();
    state.uwfAvailable = uwfAvailable;
    state.elevated = elevated;
    editor.reset(policy, state);
    auto edit = editor.beginEdit();
    app::OverlayPolicyEditor::setValue(edit, Field::Passthrough, false);
    QCOMPARE(editor.canCommit(edit), uwfAvailable && elevated);
    QCOMPARE(editor.commit(edit), uwfAvailable && elevated);
    if (!uwfAvailable || !elevated) QVERIFY(editor.pendingChanges().empty());

    editor.reset(policy, snapshot());
    QVERIFY(editor.canCommit(editor.beginEdit()));
    editor.setControlsEnabled(false);
    editor.reset(policy, snapshot());
    QCOMPARE(editor.disabledReason(editor.beginEdit(), Field::Passthrough), Issue::WritesBlocked);
    QVERIFY(!editor.canCommit(editor.beginEdit()));
    editor.setControlsEnabled(true);
    QVERIFY(editor.canCommit(editor.beginEdit()));
  }
  void modalTransactionHasNoEffectUntilCommitted() {
    app::OverlayPolicyEditor editor;
    editor.reset(editablePolicy(), snapshot());
    auto edit = editor.beginEdit();
    app::OverlayPolicyEditor::setValue(edit, Field::Passthrough, false);
    QVERIFY(editor.pendingChanges().empty());
    QCOMPARE(editor.current(Field::Passthrough), std::optional{true});
    QSignalSpy pending(&editor, &app::OverlayPolicyEditor::pendingChanged);
    QVERIFY(editor.commit(edit));
    QCOMPARE(pending.count(), 1);
    QCOMPARE(editor.pendingChanges().passthrough, std::optional{false});
    QCOMPARE(editor.current(Field::Passthrough), std::optional{true});
    QVERIFY(editor.commit(editor.beginEdit(true)));
    QVERIFY(editor.pendingChanges().empty());
  }
  void cancellingClearDoesNotPermitAnIncompatiblePersistenceWrite() {
    app::OverlayPolicyEditor editor;
    auto policy = test::application::diskPersistentPolicy();
    policy.resetPersistentState.value = 1;
    editor.reset(policy, snapshot());
    auto edit = editor.beginEdit();
    QCOMPARE(editor.disabledReason(edit, Field::Reset), Issue::None);
    app::OverlayPolicyEditor::setValue(edit, Field::Reset, false);
    QVERIFY(editor.commit(edit));
    QCOMPARE(editor.pendingChanges().reset, std::optional{app::PersistentReset::Preserve});
    QCOMPARE(editor.disabledReason(edit, Field::Persistent), Issue::CurrentFilterMustBeOff);
    app::OverlayPolicyEditor::setValue(edit, Field::Persistent, false);
    QVERIFY(!editor.commit(edit));
    QVERIFY(!editor.pendingChanges().persistent);
  }
  void typeOnlyRamChangeDoesNotCreatePersistentIntent() {
    app::OverlayPolicyEditor editor;
    editor.reset(editablePolicy(), snapshot());
    editor.setOverlayChanges({.type = core::OverlayType::RAM});
    auto edit = editor.beginEdit();
    QVERIFY(editor.conflicts(edit).empty());
    QVERIFY(editor.commit(edit));
    QVERIFY(editor.pendingChanges().empty());
    QCOMPARE(editor.disabledReason(edit, Field::Persistent), Issue::PersistentNeedsDisk);
    app::OverlayPolicyEditor::setValue(edit, Field::Persistent, false);
    QVERIFY(editor.conflicts(edit).empty());
    QVERIFY(editor.commit(edit));
    QCOMPARE(editor.disabledReason(edit, Field::Persistent), Issue::PersistentNeedsDisk);
    QCOMPARE(editor.disabledReason(edit, Field::Passthrough), Issue::None);
  }
  void missingReadsDoNotBecomeOffAndOtherFieldsRemainEditable() {
    app::OverlayPolicyEditor editor;
    auto policy = editablePolicy();
    policy.resetPersistentState = {app::PolicyObservationStatus::Failed};
    editor.reset(policy, snapshot());
    auto edit = editor.beginEdit();
    QVERIFY(!app::OverlayPolicyEditor::value(edit.values, Field::Reset));
    QCOMPARE(editor.disabledReason(edit, Field::Reset), Issue::ResetUnknown);
    app::OverlayPolicyEditor::setValue(edit, Field::Passthrough, false);
    QVERIFY(editor.commit(edit));
    QVERIFY(!editor.pendingChanges().reset);
    policy.next.flags = {app::PolicyObservationStatus::Failed};
    editor.reset(policy, snapshot());
    edit = editor.beginEdit();
    QVERIFY(!app::OverlayPolicyEditor::value(edit.values, Field::Passthrough));
    app::OverlayPolicyEditor::setValue(edit, Field::Passthrough, true);
    QVERIFY(!editor.commit(edit));
  }
  void staleReadonlyAndBusyTransactionsCannotCommit() {
    app::OverlayPolicyEditor editor;
    const auto policy = test::application::diskPersistentPolicy();
    editor.reset(policy, snapshot());
    const auto edit = editor.beginEdit();
    editor.reset(policy, snapshot());
    QCOMPARE(editor.conflicts(edit), std::vector{Issue::StaleEdit});
    QVERIFY(!editor.commit(edit));
    editor.setControlsEnabled(false);
    QVERIFY(!editor.commit(editor.beginEdit()));
    auto s = snapshot();
    s.elevated = false;
    editor.reset(policy, s);
    QVERIFY(editor.readOnly());
    QVERIFY(!editor.commit(editor.beginEdit()));
  }
  void savedSelectorRemainsUneditable() {
    app::OverlayPolicyEditor editor;
    auto policy = test::application::diskPersistentPolicy();
    policy.resetPersistentState.value = 255;
    editor.reset(policy, snapshot());
    auto edit = editor.beginEdit();
    QCOMPARE(editor.disabledReason(edit, Field::Reset), Issue::ResetUnknown);
    app::OverlayPolicyEditor::setValue(edit, Field::Reset, true);
    QVERIFY(!editor.commit(edit));
  }
  void mediaModeChangesInEitherDirectionRequireCurrentFilterOff() {
    app::OverlayPolicyEditor editor;
    auto policy = test::application::diskPersistentPolicy();
    auto s = snapshot();
    s.next.overlay.type = core::OverlayType::RAM;
    policy.next.overlayType.value = 0;
    policy.next.flags.value = app::OverlayFlags{1};
    editor.reset(policy, s);
    auto edit = editor.beginEdit();
    QCOMPARE(editor.disabledReason(edit, Field::ReadOnlyMedia), Issue::CurrentFilterMustBeOff);
    policy.current.filterEnabled.value = false;
    editor.reset(policy, s);
    edit = editor.beginEdit();
    QCOMPARE(editor.disabledReason(edit, Field::ReadOnlyMedia), Issue::None);
    app::OverlayPolicyEditor::setValue(edit, Field::ReadOnlyMedia, true);
    QVERIFY(editor.commit(edit));
    policy.next.flags.value = app::OverlayFlags{5};
    policy.current.filterEnabled.value = true;
    editor.reset(policy, s);
    edit = editor.beginEdit();
    QCOMPARE(editor.disabledReason(edit, Field::ReadOnlyMedia), Issue::CurrentFilterMustBeOff);
    QVERIFY(editor.canCommit(edit));
    app::OverlayPolicyEditor::setValue(edit, Field::ReadOnlyMedia, false);
    QCOMPARE(editor.conflicts(edit), std::vector{Issue::CurrentFilterMustBeOff});
    QVERIFY(!editor.commit(edit));
    QVERIFY(editor.commit(editor.beginEdit(true)));
    policy.current.filterEnabled.value = false;
    editor.reset(policy, s);
    edit = editor.beginEdit();
    app::OverlayPolicyEditor::setValue(edit, Field::ReadOnlyMedia, false);
    QVERIFY(editor.commit(edit));
  }
};
}  // namespace
QTEST_APPLESS_MAIN(OverlayPolicyEditorTests)
#include "OverlayPolicyEditorTests.moc"
