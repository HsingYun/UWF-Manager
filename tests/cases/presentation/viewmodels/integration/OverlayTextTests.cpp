// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <QTranslator>
#include <algorithm>
#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "application/configuration/editing/GlobalConfigurationEditor.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/configuration/importing/ConfigurationImporter.h"
#include "application/configuration/overlay/OverlayPolicyEditor.h"
#include "application/localization/OverlayTypeText.h"
#include "application/settings/UiPreferences.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"
#include "presentation/viewmodels/configuration/ImportViewModel.h"
#include "presentation/viewmodels/configuration/MaintenanceValidationText.h"
#include "presentation/viewmodels/configuration/OverlayAdvancedViewModel.h"
#include "presentation/viewmodels/diagnostics/DiagnosticReportViewModel.h"
#include "support/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;

QList<app::UwfmgrCommand> commands(const std::string& text) {
  const auto parsed = app::parseUwfmgrText(text);
  return {parsed.begin(), parsed.end()};
}

class OverlayTextTests final : public QObject {
  Q_OBJECT
 private slots:
  void cleanup() { QCoreApplication::removeTranslator(&m_translator); }

  void maintenanceValidationUsesTheSelectedLanguage_data() {
    QTest::addColumn<QString>("locale");
    for (const auto& language : app::languages()) QTest::newRow(qUtf8Printable(QString(language.code))) << QString(language.code);
  }
  void maintenanceValidationUsesTheSelectedLanguage() {
    QFETCH(QString, locale);
    std::vector<app::MaintenanceValidationIssue> issues;
    for (int value = 0; value < static_cast<int>(app::MaintenancePlanIssue::Count); ++value) issues.emplace_back(static_cast<app::MaintenancePlanIssue>(value));
    for (int value = 1; value <= static_cast<int>(app::HormIssue::VolumesUnprotected); ++value) issues.emplace_back(static_cast<app::HormIssue>(value));
    QStringList english;
    for (const auto& issue : issues) english.append(presentation::maintenanceValidationText(issue));
    QVERIFY(useLocale(locale));
    for (std::size_t i = 0; i < issues.size(); ++i) {
      const auto text = presentation::maintenanceValidationText(issues[i]);
      const auto context = QStringLiteral("%1/issue-%2").arg(locale).arg(i);
      QVERIFY2(!text.isEmpty(), qUtf8Printable(context));
      QCOMPARE(text == english[static_cast<qsizetype>(i)], locale == QLatin1String("en"));
    }

    const QString volume = QStringLiteral("storage-volume");
    const std::array conflicts{std::pair{app::OverlayMaintenanceChanges{{}, true, true}, app::MaintenancePlanIssue::FullCommitAndHormConflict},
                               std::pair{app::OverlayMaintenanceChanges{volume, true, false}, app::MaintenancePlanIssue::StorageAndHormConflict},
                               std::pair{app::OverlayMaintenanceChanges{volume, {}, true}, app::MaintenancePlanIssue::StorageAndFullCommitConflict},
                               std::pair{app::OverlayMaintenanceChanges{volume, {}, false}, app::MaintenancePlanIssue::StorageAndRamConflict}};
    constexpr auto fullCommitRiskSource =
        "Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.";
    const auto fullCommitRisk = QCoreApplication::translate("uwf", fullCommitRiskSource);
    QCOMPARE(fullCommitRisk == QString::fromUtf8(fullCommitRiskSource), locale == QLatin1String("en"));
    for (const auto& [changes, issue] : conflicts) {
      app::ConfigurationPlan plan;
      plan.snapshot.next.overlay = {core::OverlayType::Disk, 4096, 512, 1024};
      plan.maintenance = changes;
      if (issue == app::MaintenancePlanIssue::StorageAndRamConflict) plan.changes.setOverlay.type = core::OverlayType::RAM;
      presentation::ApplyPlanViewModel review(plan, {});
      QStringList expectedWarnings{presentation::maintenanceValidationText(issue)};
      if (changes.fullCommit) expectedWarnings.append(fullCommitRisk);
      QCOMPARE(review.warning(), expectedWarnings.join(u'\n'));
      QVERIFY(!review.canApply());
      QVERIFY(!review.canExport());
    }
  }

  void maintenanceValidationSurvivesApplyBoundaries_data() {
    QTest::addColumn<QString>("locale");
    QTest::addColumn<int>("boundary");
    for (const auto& locale : {QStringLiteral("en"), QStringLiteral("zh_CN")})
      for (const int boundary : {0, 1, 2, 3}) QTest::newRow(qUtf8Printable(QStringLiteral("%1/boundary-%2").arg(locale).arg(boundary))) << locale << boundary;
  }
  void maintenanceValidationSurvivesApplyBoundaries() {
    QFETCH(QString, locale);
    QFETCH(int, boundary);
    class ValidationActions final : public app::ConfigurationActions {
     public:
      int boundary = 0;
      ApplyOperation prepareApply(const app::ConfigurationPlan& plan) override { return prepareApply(plan.changes, plan.snapshot); }
      ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
        if (boundary == 0) throw app::MaintenanceValidationError(app::MaintenancePlanIssue::StorageVolumeUnavailable);
        return [stage = boundary] {
          if (stage == 1) throw app::MaintenanceValidationError(app::MaintenancePlanIssue::StorageVolumeUnavailable);
          app::ConfigurationApplyResult result;
          app::ConfigurationStepResult receipt{{}, stage == 2 ? app::ConfigurationStepOutcome::AdmissionFailed : app::ConfigurationStepOutcome::Failed};
          receipt.validationIssue = app::MaintenancePlanIssue::StorageVolumeUnavailable;
          if (stage == 3) {
            receipt.operation = app::ConfigurationOperation{app::ConfigurationOperationKind::OverlayStorage, "storage-volume"};
            receipt.failure = app::OperationFailure{app::OperationFailureKind::InvalidArgument, app::WriteEffect::None, {}};
            receipt.maintenance = app::ConfigurationMaintenanceDetail{};
          }
          result.steps.push_back(std::move(receipt));
          return result;
        };
      }
    };
    QVERIFY(useLocale(locale));
    auto actions = std::make_shared<ValidationActions>();
    actions->boundary = boundary;
    app::ConfigurationPlan plan;
    plan.snapshot.next.overlay = {core::OverlayType::RAM, 4096, 512, 1024};
    if (boundary == 3) {
      plan.maintenance.storageVolume = QStringLiteral("storage-volume");
      plan.hasPolicyContext = plan.policySnapshot.transportAvailable = true;
      plan.policySnapshot.current.filterEnabled = {app::PolicyObservationStatus::Available, false};
      plan.policySnapshot.current.overlayType = {app::PolicyObservationStatus::Available, 0};
      plan.policySnapshot.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{0}};
      plan.policySnapshot.next = plan.policySnapshot.current;
      plan.policySnapshot.resetPersistentState = {app::PolicyObservationStatus::Available, 0};
    } else {
      plan.changes.setFilterEnabled = true;
    }
    presentation::ApplyPlanViewModel review(plan, actions);
    QVERIFY(review.canApply());
    review.apply();
    QTRY_VERIFY(!review.applying());
    QVERIFY(review.resultFailed());
    QVERIFY(!review.result().reconciliationRequired());
    QCOMPARE(review.resultText(), presentation::maintenanceValidationText(app::MaintenancePlanIssue::StorageVolumeUnavailable));
    QVERIFY(review.result().steps.front().validationIssue.has_value());
    QCOMPARE(review.result().batchFailures(), boundary == 3 ? 0 : 1);
    QCOMPARE(review.result().notAttemptedOperations(), boundary == 3 ? 0 : 1);
    QCOMPARE(review.result().failedOperations(), boundary == 3 ? 1 : 0);
    QVERIFY(std::ranges::all_of(review.result().steps, [](const auto& step) { return step.description.empty(); }));
  }

  void typeDisplaysUseTheSelectedLanguage_data() {
    QTest::addColumn<QString>("locale");
    QTest::addColumn<int>("type");
    QTest::addColumn<QString>("expected");
    for (const auto& locale : {QStringLiteral("en"), QStringLiteral("zh_CN")}) {
      const bool chinese = locale == QLatin1String("zh_CN");
      QTest::newRow(qUtf8Printable(locale + QStringLiteral("-ram"))) << locale << 0 << QStringLiteral("RAM");
      QTest::newRow(qUtf8Printable(locale + QStringLiteral("-disk"))) << locale << 1 << (chinese ? QStringLiteral("磁盘") : QStringLiteral("Disk"));
      QTest::newRow(qUtf8Printable(locale + QStringLiteral("-unknown"))) << locale << -1 << (chinese ? QStringLiteral("未知") : QStringLiteral("Unknown"));
    }
  }
  void typeDisplaysUseTheSelectedLanguage() {
    QFETCH(QString, locale);
    QFETCH(int, type);
    QFETCH(QString, expected);
    QVERIFY(useLocale(locale));
    const auto value = static_cast<core::OverlayType>(type);
    QCOMPARE(app::overlayTypeText(value), expected);

    app::OverlayPolicySnapshot policy;
    policy.current.overlayType = {app::PolicyObservationStatus::Available, static_cast<std::int64_t>(type), {}};
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.next.overlay = {value, 4096, 1000, 3000};
    auto editor = std::make_shared<app::OverlayPolicyEditor>();
    editor->reset(policy, snapshot);
    presentation::OverlayAdvancedViewModel advanced(editor);
    QCOMPARE(advanced.currentTypeText(), expected);
    QCOMPARE(advanced.targetTypeText(), expected);
    // Official feature aliases stay English independently of type values.
    QCOMPARE(advanced.officialName(app::OverlayPolicyField::Persistent), QStringLiteral("Persistent Overlay"));

    app::DiagnosticReportData report;
    report.context.uwf.snapshotPresent = report.context.uwf.uwfAvailable = true;
    report.context.uwf.current.overlayType = type == 0 ? QStringLiteral("RAM") : type == 1 ? QStringLiteral("disk") : QStringLiteral("unknown");
    const presentation::DiagnosticReportViewModel diagnostics(report, 8192);
    QCOMPARE(diagnostics.sections().at(2).fields.at(0).value, expected);

    if (value == core::OverlayType::Unknown) return;
    const auto before = value == core::OverlayType::Disk ? core::OverlayType::RAM : core::OverlayType::Disk;
    snapshot.next.overlay.type = before;
    core::PendingChanges changes;
    changes.setOverlay.type = value;
    presentation::ApplyPlanViewModel review(changes, snapshot, {});
    QCOMPARE(review.groups().front().items.front().after, expected);

    app::GlobalConfigurationEditor global;
    global.reset(snapshot.next);
    const auto command = std::string("uwfmgr overlay set-type ") + (type == 0 ? "RAM" : "Disk");
    const auto imported = app::applyImportCommands(commands(command), &global, {}, nullptr, {});
    QCOMPARE(imported.size(), 1);
    QCOMPARE(imported.front().status, app::ImportResult::Status::Success);
    QVERIFY(imported.front().detail.endsWith(expected));

    test::FakeUwfBackend::State state;
    state.next.overlayType = static_cast<std::uint32_t>(before);
    state.next.maximumOverlaySize = 4096;
    auto system = std::make_shared<test::FakeUwfBackend>(std::move(state));
    app::ConfigurationExecutor executor(app::uwfConfigurationRepository(system));
    const auto applied = executor.apply(changes, snapshot);
    QCOMPARE(applied.failedOperations(), 0);
    QCOMPARE(applied.succeededOperations(), 1);
    QCOMPARE(system->snapshot().next.overlayType, static_cast<std::uint32_t>(value));
    const auto expectedLine =
        locale == QLatin1String("zh_CN") ? QStringLiteral("✓ 覆盖层 类型设为 %1").arg(expected) : QStringLiteral("✓ Overlay type set to %1").arg(expected);
    QVERIFY(std::ranges::any_of(applied.steps, [&](const auto& step) { return step.description == expectedLine.toStdString(); }));
  }

  void advancedValuesKeepCurrentStateSeparateFromNextSessionEdits_data() {
    QTest::addColumn<bool>("reset");
    QTest::newRow("persistent-edit-current-off") << false;
    QTest::newRow("reset-cancellation-current-on") << true;
  }
  void advancedValuesKeepCurrentStateSeparateFromNextSessionEdits() {
    QFETCH(bool, reset);
    using Model = presentation::OverlayAdvancedViewModel;
    using Field = Model::Field;
    using State = Model::ValueState;
    app::OverlayPolicySnapshot policy;
    policy.transportAvailable = true;
    policy.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{3}};
    policy.current.filterEnabled = {app::PolicyObservationStatus::Available, reset};
    policy.current.overlayType = {app::PolicyObservationStatus::Available, 1};
    policy.next = policy.current;
    policy.next.flags.value = app::OverlayFlags{2};
    policy.resetPersistentState = {app::PolicyObservationStatus::Available, reset ? 1 : 0};
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.next.overlay = {core::OverlayType::Disk, 4096, 1000, 3000};
    auto editor = std::make_shared<app::OverlayPolicyEditor>();
    editor->reset(policy, snapshot);
    Model model(editor);
    QCOMPARE(model.currentState(Field::Passthrough), State::On);
    QCOMPARE(model.currentState(Field::Persistent), State::On);
    QCOMPARE(model.currentState(Field::ReadOnlyMedia), State::Off);
    QCOMPARE(model.currentState(Field::Reset), State::Off);
    QCOMPARE(model.currentText(Field::Passthrough), QStringLiteral("On"));
    QCOMPARE(model.currentText(Field::ReadOnlyMedia), QStringLiteral("Off"));
    QCOMPARE(model.currentText(Field::Reset), QStringLiteral("Off"));
    QCOMPARE(model.currentFilterState(), reset ? State::On : State::Off);
    QCOMPARE(model.currentFilterText(), reset ? QStringLiteral("Enabled") : QStringLiteral("Disabled"));

    auto edit = model.beginEdit();
    QCOMPARE(Model::value(edit, Field::Passthrough), std::optional{false});
    QCOMPARE(Model::value(edit, Field::Reset), std::optional{reset});
    QVERIFY(!model.changed(edit, Field::Passthrough));
    QVERIFY(!model.hasPendingChanges());
    const auto field = reset ? Field::Reset : Field::Persistent;
    Model::setValue(edit, field, false);
    QVERIFY(model.changed(edit, field));
    QVERIFY(model.canCommit(edit));
    QCOMPARE(model.pendingCount(edit), 1);
    QVERIFY(!model.hasPendingChanges());  // The modal transaction has not been published.
    QVERIFY(model.commit(edit));
    QVERIFY(model.hasPendingChanges());
    QCOMPARE(Model::value(model.beginEdit(), field), std::optional{false});
    QCOMPARE(model.currentState(Field::Persistent), State::On);
    QCOMPARE(model.currentText(Field::Reset), QStringLiteral("Off"));
  }

  void advancedUnknownObservationsRemainUnknown() {
    using Model = presentation::OverlayAdvancedViewModel;
    app::OverlayPolicySnapshot policy;
    policy.current.flags = {app::PolicyObservationStatus::Failed, app::OverlayFlags{7}};
    policy.current.filterEnabled = {app::PolicyObservationStatus::Failed, true};
    policy.current.overlayType = {app::PolicyObservationStatus::Failed, 1};
    policy.resetPersistentState = {app::PolicyObservationStatus::Available, 255};
    auto editor = std::make_shared<app::OverlayPolicyEditor>();
    editor->reset(policy, {});
    const Model model(editor);
    for (const auto field : {Model::Field::Passthrough, Model::Field::Persistent, Model::Field::ReadOnlyMedia, Model::Field::Reset}) {
      QCOMPARE(model.currentState(field), Model::ValueState::Unknown);
      QCOMPARE(model.currentText(field), QStringLiteral("Unknown"));
    }
    QCOMPARE(model.currentFilterState(), Model::ValueState::Unknown);
    QCOMPARE(model.currentFilterText(), QStringLiteral("Unknown"));
    QCOMPARE(model.currentTypeText(), QStringLiteral("Unknown"));
    QCOMPARE(model.targetTypeText(), QStringLiteral("Unknown"));
  }

  void policyImportFeedbackUsesTheSelectedLanguage_data() {
    QTest::addColumn<QString>("locale");
    QTest::addColumn<QString>("replaced");
    QTest::addColumn<QString>("finalDetail");
    QTest::addColumn<QString>("unavailable");
    QTest::newRow("english") << QStringLiteral("en")
                             << QStringLiteral("Requested Freespace Passthrough → Enabled; replaced by a later command. Final draft value → Enabled")
                             << QStringLiteral("Final draft Freespace Passthrough → Enabled")
                             << QStringLiteral("The overlay configuration could not be read. Refresh before editing.");
    QTest::newRow("chinese") << QStringLiteral("zh_CN") << QStringLiteral("请求值：可用空间直通 → 已启用；已被后续命令替代。草稿最终值 → 已启用")
                             << QStringLiteral("草稿最终值：可用空间直通 → 已启用") << QStringLiteral("无法读取覆盖层配置；请刷新后再编辑。");
  }
  void policyImportFeedbackUsesTheSelectedLanguage() {
    QFETCH(QString, locale);
    QFETCH(QString, replaced);
    QFETCH(QString, finalDetail);
    QFETCH(QString, unavailable);
    QVERIFY(useLocale(locale));
    const auto input = commands("uwfmgr overlay set-passthrough on");
    const auto failed = app::applyImportCommands(input, nullptr, {}, nullptr, {});
    QCOMPARE(failed.front().status, app::ImportResult::Status::Failed);
    QCOMPARE(failed.front().detail, unavailable);

    app::OverlayPolicySnapshot baseline;
    baseline.transportAvailable = true;
    baseline.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{0}, {}};
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    app::OverlayPolicyEditor policy;
    policy.reset(baseline, snapshot);
    presentation::ImportViewModel model;
    model.setApplier([&](const auto& batch) { return app::applyImportCommands(batch, nullptr, {}, nullptr, {}, &policy); });
    model.setText(QStringLiteral("uwfmgr overlay set-passthrough on\nuwfmgr overlay set-passthrough on"));
    model.importText();
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.index(0, 3).data().toString(), replaced);
    QCOMPARE(model.index(1, 3).data().toString(), finalDetail);
    QCOMPARE(model.index(0, 3).data(Qt::ToolTipRole).toString(), replaced);
    QCOMPARE(model.index(1, 3).data(Qt::ToolTipRole).toString(), finalDetail);
  }

 private:
  bool useLocale(const QString& locale) {
    if (locale == QLatin1String("en")) return true;
    return m_translator.load(QStringLiteral(":/i18n/%1.qm").arg(locale)) && QCoreApplication::installTranslator(&m_translator);
  }
  QTranslator m_translator;
};
}  // namespace

QTEST_GUILESS_MAIN(OverlayTextTests)
#include "OverlayTextTests.moc"
