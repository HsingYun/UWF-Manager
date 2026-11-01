// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "application/configuration/editing/GlobalConfigurationEditor.h"
#include "application/configuration/editing/OverlayMaintenanceEditor.h"
#include "application/configuration/exclusions/ExclusionEditor.h"
#include "application/configuration/overlay/OverlayPolicyEditor.h"

namespace {
using namespace uwf;
enum class EditorKind { Global, Policy, Exclusion, Maintenance };

template <class Test>
void withEditor(const EditorKind kind, Test test) {
  switch (kind) {
    case EditorKind::Global:
      test(std::make_unique<app::GlobalConfigurationEditor>());
      break;
    case EditorKind::Policy:
      test(std::make_unique<app::OverlayPolicyEditor>());
      break;
    case EditorKind::Exclusion:
      test(std::make_unique<app::ExclusionEditor>(app::ExclusionEditor::Kind::File));
      break;
    case EditorKind::Maintenance:
      test(std::make_unique<app::OverlayMaintenanceEditor>());
      break;
  }
}
template <class Editor>
void mutate(Editor& editor) {
  if constexpr (std::is_same_v<Editor, app::GlobalConfigurationEditor>) {
    editor.setAvailable(true);
  } else if constexpr (std::is_same_v<Editor, app::OverlayPolicyEditor>) {
    editor.reset({}, {});
  } else if constexpr (std::is_same_v<Editor, app::ExclusionEditor>) {
    editor.setBaseline({}, {QStringLiteral("C:\\Data")});
    editor.setFileAddTargetAvailable(false);
  } else {
    editor.replaceDraft({.allVolumesProtected = true});
  }
}
template <class Editor, class Receiver>
void observeSnapshot(Editor& editor, QObject* context, Receiver receive) {
  if constexpr (std::is_same_v<Editor, app::ExclusionEditor>) QObject::connect(&editor, &Editor::baselineChanged, context, [receive] { receive("baseline"); });
  if constexpr (std::is_same_v<Editor, app::OverlayMaintenanceEditor>) {
    QObject::connect(&editor, &Editor::pendingChanged, context, [receive] { receive("pending"); });
    QObject::connect(&editor, &Editor::stateChanged, context, [receive] { receive("state"); });
  } else {
    QObject::connect(&editor, &Editor::stateChanged, context, [receive] { receive("state"); });
    if constexpr (std::is_same_v<Editor, app::GlobalConfigurationEditor>)
      QObject::connect(&editor, &Editor::availabilityChanged, context, [receive] { receive("availability"); });
    else
      QObject::connect(&editor, &Editor::pendingChanged, context, [receive] { receive("pending"); });
  }
  if constexpr (std::is_same_v<Editor, app::ExclusionEditor>)
    QObject::connect(&editor, &Editor::editingAvailabilityChanged, context, [receive] { receive("availability"); });
}
void editorRows() {
  QTest::addColumn<EditorKind>("kind");
  QTest::newRow("global") << EditorKind::Global;
  QTest::newRow("policy") << EditorKind::Policy;
  QTest::newRow("exclusion") << EditorKind::Exclusion;
  QTest::newRow("maintenance") << EditorKind::Maintenance;
}
class EditorPublicationTests final : public QObject {
  Q_OBJECT
 private slots:
  void updatesRestoreBlockingWithoutPublishing_data() { editorRows(); }
  void updatesRestoreBlockingWithoutPublishing() {
    QFETCH(EditorKind, kind);
    withEditor(kind, [&](auto editor) {
      using Editor = std::remove_reference_t<decltype(*editor)>;
      QStringList notifications;
      observeSnapshot(*editor, this, [&](const char* name) { notifications.append(QString::fromLatin1(name)); });
      std::optional<typename Editor::Update> moved;
      {
        auto update = editor->beginUpdate();
        mutate(*editor);
        moved.emplace(std::move(update));
      }
      QVERIFY(editor->signalsBlocked());
      QVERIFY(notifications.isEmpty());
      moved.reset();
      QVERIFY(!editor->signalsBlocked());
      QVERIFY(notifications.isEmpty());
      editor->blockSignals(true);
      {
        auto update = editor->beginUpdate();
      }
      QVERIFY(editor->signalsBlocked());
      editor->blockSignals(false);
    });
  }
  void snapshotsPublishOnlyAfterExplicitResume_data() { editorRows(); }
  void snapshotsPublishOnlyAfterExplicitResume() {
    QFETCH(EditorKind, kind);
    withEditor(kind, [&](auto editor) {
      QStringList notifications;
      observeSnapshot(*editor, this, [&](const char* name) {
        QVERIFY(!editor->signalsBlocked());
        notifications.append(QString::fromLatin1(name));
      });
      auto update = editor->beginUpdate();
      mutate(*editor);
      QVERIFY(notifications.isEmpty());
      update.resume();
      QVERIFY(notifications.isEmpty());
      QVERIFY(update.publishSnapshot());
      const QStringList expected = kind == EditorKind::Global      ? QStringList{"state", "availability"}
                                   : kind == EditorKind::Policy    ? QStringList{"state", "pending"}
                                   : kind == EditorKind::Exclusion ? QStringList{"baseline", "state", "pending", "availability"}
                                                                   : QStringList{"pending", "state"};
      QCOMPARE(notifications, expected);
    });
  }
  void invalidatedPublicationStopsBeforeEveryRemainingSignal_data() {
    QTest::addColumn<EditorKind>("kind");
    QTest::addColumn<int>("stopAfter");
    for (const auto kind : {EditorKind::Global, EditorKind::Policy, EditorKind::Exclusion, EditorKind::Maintenance})
      for (int stopAfter = 0; stopAfter <= (kind == EditorKind::Exclusion ? 4 : 2); ++stopAfter)
        QTest::newRow(qUtf8Printable(QStringLiteral("editor-%1-after-%2").arg(static_cast<int>(kind)).arg(stopAfter))) << kind << stopAfter;
  }
  void invalidatedPublicationStopsBeforeEveryRemainingSignal() {
    QFETCH(EditorKind, kind);
    QFETCH(int, stopAfter);
    withEditor(kind, [&](auto editor) {
      int notifications = 0;
      observeSnapshot(*editor, this, [&](const char*) { ++notifications; });
      auto update = editor->beginUpdate();
      mutate(*editor);
      update.resume();
      QVERIFY(!update.publishSnapshot([&] { return notifications < stopAfter; }));
      QCOMPARE(notifications, stopAfter);
    });
  }
  void updatesSurviveEditorDestruction_data() {
    QTest::addColumn<EditorKind>("kind");
    QTest::addColumn<int>("destruction");
    for (const auto kind : {EditorKind::Global, EditorKind::Policy, EditorKind::Exclusion, EditorKind::Maintenance})
      for (int destruction = 0; destruction < 3; ++destruction)
        QTest::newRow(qUtf8Printable(QStringLiteral("editor-%1-during-%2").arg(static_cast<int>(kind)).arg(destruction))) << kind << destruction;
  }
  void updatesSurviveEditorDestruction() {
    QFETCH(EditorKind, kind);
    QFETCH(int, destruction);
    withEditor(kind, [&](auto editor) {
      int notifications = 0;
      observeSnapshot(*editor, this, [&](const char*) {
        ++notifications;
        editor.reset();
      });
      auto update = editor->beginUpdate();
      mutate(*editor);
      if (destruction == 0) {
        editor.reset();
        update.resume();
        QVERIFY(!update.publishSnapshot());
      } else {
        update.resume();
        QVERIFY(!update.publishSnapshot([&] {
          if (destruction == 1) editor.reset();
          return true;
        }));
      }
      QVERIFY(!editor);
      QCOMPARE(notifications, destruction == 2 ? 1 : 0);
    });
  }
  void availabilityReadsTheCurrentAdmissionAtPublication() {
    app::GlobalConfigurationEditor global;
    global.setAvailable(true);
    app::OverlayPolicyEditor policy;
    app::OverlayPolicySnapshot baseline;
    baseline.transportAvailable = true;
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    policy.reset(baseline, snapshot, false);
    app::ExclusionEditor files(app::ExclusionEditor::Kind::File);
    QSignalSpy globalChanges(&global, &app::GlobalConfigurationEditor::availabilityChanged);
    QSignalSpy policyChanges(&policy, &app::OverlayPolicyEditor::stateChanged);
    QSignalSpy fileChanges(&files, &app::ExclusionEditor::editingAvailabilityChanged);
    auto globalUpdate = global.beginUpdate();
    auto policyUpdate = policy.beginUpdate();
    auto filesUpdate = files.beginUpdate();
    global.setControlsEnabled(false);
    policy.setControlsEnabled(false);
    files.setFileAddTargetAvailable(false);
    globalUpdate.resume();
    policyUpdate.resume();
    filesUpdate.resume();
    // An earlier participant's callback can restore a later participant's admission.
    connect(&global, &app::GlobalConfigurationEditor::availabilityChanged, this, [&] {
      policy.setControlsEnabled(true);
      files.setFileAddTargetAvailable(true);
    });
    QVERIFY(globalUpdate.publishAvailability());
    QCOMPARE(globalChanges.size(), 1);
    QCOMPARE(policyChanges.size(), 1);
    QCOMPARE(fileChanges.size(), 1);
    QVERIFY(policyUpdate.publishAvailability());
    QVERIFY(filesUpdate.publishAvailability());
    QCOMPARE(policyChanges.size(), 1);
    QCOMPARE(fileChanges.size(), 1);
  }
};
}  // namespace
QTEST_APPLESS_MAIN(EditorPublicationTests)
#include "EditorPublicationTests.moc"
