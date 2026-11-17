// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QTranslator>
#include <array>
#include <optional>

#include "cases/presentation/viewmodels/common/ViewModelStateFixture.h"
#include "cases/presentation/viewmodels/enhanced/FakeEnhancedModeActions.h"
#include "presentation/viewmodels/common/AutomaticCommitText.h"
#include "presentation/viewmodels/common/RuntimeWriteText.h"
#include "presentation/viewmodels/configuration/VolumeConfigurationViewModel.h"
#include "presentation/viewmodels/enhanced/EnhancedModeViewModel.h"
#include "presentation/viewmodels/staging/FileStagingViewModel.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

class StagingEditingProbe final : public app::FileStagingEditing {
 public:
  QString drive = QStringLiteral("C:"), error;
  QList<app::FileStagingEntry> saved{{app::FileStagingKind::File, QStringLiteral("C:\\work.txt")}};
  int mutations = 0;
  const QString& driveLetter() const override { return drive; }
  const QList<app::FileStagingEntry>& entries() const override { return saved; }
  QList<app::FileStagingEntry> baselineEntries() const override { return saved; }
  bool storageAvailable() const override { return true; }
  const QString& storageError() const override { return error; }
  void load() override {}
  app::FileStagingPreparation prepareAddEntries(const QList<app::FileStagingEntry>&) override {
    return {{}, [this](std::exception_ptr) { ++mutations; }, {}};
  }
  app::FileStagingPreparation prepareRemovePaths(const QStringList&) override {
    return {{}, [this](std::exception_ptr) { ++mutations; }, {}};
  }
  app::FileStagingPreparation prepareRestorePaths(const QStringList&) override {
    return {{}, [this](std::exception_ptr) { ++mutations; }, {}};
  }
};

class EmptyTranslator final : public QTranslator {
 public:
  bool isEmpty() const override { return false; }
  QString translate(const char*, const char*, const char*, int) const override { return QLatin1String(""); }
};

class RuntimeModePresentationTests final : public QObject {
  Q_OBJECT
 private slots:
  void automaticCommitFormatterPreservesMixedAndUnknownReasons_data() {
    QTest::addColumn<unsigned>("mask");
    QTest::addColumn<bool>("unknown");
    QTest::addColumn<bool>("requiresCommit");
    QTest::addColumn<int>("pending");
    for (unsigned mask = 1; mask != 32; ++mask)
      for (const bool unknown : {false, true})
        for (const bool required : {false, true})
          for (const int pending : {-1, 0, 4})
            QTest::newRow(qUtf8Printable(QStringLiteral("reasons-%1/unknown-%2/required-%3/pending-%4").arg(mask).arg(unknown).arg(required).arg(pending)))
                << mask << unknown << required << pending;
  }
  void automaticCommitFormatterPreservesMixedAndUnknownReasons() {
    QFETCH(unsigned, mask);
    QFETCH(bool, unknown);
    QFETCH(bool, requiresCommit);
    QFETCH(int, pending);
    using D = app::AutomaticCommitDisposition;
    const std::array reasons{D::SkipHormActive, D::SkipHormTransition, D::SkipFullCommit, D::SkipReadOnlyMedia, D::SkipRetainedOverlay};
    app::AutomaticCommitSummary summary;
    for (std::size_t index = 0; index < reasons.size(); ++index)
      if ((mask & (1u << index)) != 0) summary.include(reasons[index]);
    if (unknown) summary.include(D::Unavailable);
    if (requiresCommit) summary.include(D::CheckDurability);
    const auto view =
        presentation::automaticCommitPresentation(summary, pending < 0 ? std::nullopt : std::optional<std::size_t>{static_cast<std::size_t>(pending)});
    QCOMPARE(view.status, unknown ? QStringLiteral("Unavailable") : requiresCommit ? QStringLiteral("Partially paused") : QStringLiteral("Paused"));
    QCOMPARE(view.unavailable, unknown);
    QVERIFY(view.paused);
    QCOMPARE(view.partiallyPaused, requiresCommit);
    for (std::size_t index = 0; index < reasons.size(); ++index) {
      const auto description = presentation::automaticCommitDescription(reasons[index]);
      QCOMPARE(view.description.contains(description), (mask & (1u << index)) != 0);
      if ((mask & (1u << index)) != 0) QCOMPARE(view.description.count(description), 1);
    }
    QCOMPARE(view.description.contains(presentation::automaticCommitDescription(D::Unavailable)), unknown);
    if (pending == 0) {
      QVERIFY(!view.description.contains(QStringLiteral("Only eligible staged files will be committed.")));
      QVERIFY(!view.description.contains(QStringLiteral("Other staged files will still be committed.")));
    }
  }
  void automaticCommitFormatterSeparatesReadyFromAlreadyPersistent() {
    app::AutomaticCommitSummary summary;
    const auto ready = presentation::automaticCommitPresentation(summary);
    QCOMPARE(ready.status, QStringLiteral("Ready"));
    QCOMPARE(ready.description, QStringLiteral("Commits staged files before normal shutdown or restart."));
    QVERIFY(!ready.unavailable);
    QVERIFY(!ready.paused);
    QVERIFY(!ready.partiallyPaused);
    QVERIFY(!ready.alreadyPersistent);
    summary.include(app::AutomaticCommitDisposition::AlreadyPersistent);
    const auto persistent = presentation::automaticCommitPresentation(summary);
    QCOMPARE(persistent.status, QStringLiteral("No files to commit"));
    QCOMPARE(persistent.description, QStringLiteral("Staged files are already on unprotected storage. No automatic commit is needed."));
    QVERIFY(persistent.alreadyPersistent);
    QVERIFY(!persistent.paused);
    summary.include(app::AutomaticCommitDisposition::CheckDurability);
    const auto mixed = presentation::automaticCommitPresentation(summary);
    QCOMPARE(mixed.status, QStringLiteral("Ready"));
    QVERIFY(!mixed.description.contains(QStringLiteral("No automatic commit is needed.")));
  }
  void dispositionControlsRuntimeCommitAndExclusionAvailability() {
    auto state = snapshot();
    state.runtime.romDisposition = 3;
    presentation::VolumeConfigurationViewModel model(disk(), true);
    model.applySnapshot(state, false, std::uint32_t{4});
    QVERIFY(!model.currentReadOnlyMediaActive());
    QVERIFY(model.commitAvailability().files);
    QVERIFY(model.commitAvailability().registry);
    QVERIFY(!model.files()->readOnly());
    QVERIFY(!model.registry()->readOnly());
    QVERIFY(model.files()->currentSessionExclusionsEffective());
    state.runtime.romDisposition = 4;
    state.runtime.fullCommit = true;
    model.applySnapshot(state, false, std::uint32_t{4});
    QVERIFY(model.currentReadOnlyMediaActive());
    QVERIFY(!model.commitAvailability().files);
    QVERIFY(!model.commitAvailability().registry);
    QVERIFY(!model.files()->currentSessionExclusionsEffective());
  }
  void currentRomPreservesDiagnosticsAndBlocksVolatileExclusionDrafts() {
    auto state = snapshot();
    const auto volumeName = state.current.volumes.front().volumeName;
    state.current.fileExclusions[volumeName] = {"\\Data"};
    state.next.fileExclusions[volumeName] = {"\\Data"};
    state.current.registryExclusions = {"HKLM\\SOFTWARE\\Example"};
    state.next.registryExclusions = state.current.registryExclusions;
    presentation::VolumeConfigurationViewModel model(disk(), true);
    model.applySnapshot(state, false, std::uint32_t{4});

    QVERIFY(model.currentReadOnlyMediaActive());
    const auto rom = model.commitAvailability();
    QVERIFY(!rom.files && !rom.registry);
    QVERIFY(rom.overlay);
    QCOMPARE(
        model.runtimeNotice(),
        QStringLiteral("Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them."));
    QVERIFY(model.files()->readOnly());
    QVERIFY(model.registry()->readOnly());
    QVERIFY(!model.files()->currentSessionExclusionsEffective());
    QVERIFY(!model.registry()->currentSessionExclusionsEffective());
    QCOMPARE(model.files()->index(0).data(presentation::StatusTextRole).toString(), QStringLiteral("Inactive this session"));
    QVERIFY(model.files()->index(0).data(Qt::ToolTipRole).toString().contains(QStringLiteral("does not bypass protection")));
    QVERIFY(!model.files()->commitEnabled());

    // A Next draft would still be an ordinary SYSTEM-hive write in this
    // protected ROM startup, so reject it rather than promising persistence.
    model.files()->importAdd(QStringLiteral("C:\\Future"));
    QVERIFY(model.files()->pendingAdded().isEmpty());
    QVERIFY(!model.commitAvailability().files);
    QVERIFY(model.currentReadOnlyMediaActive());

    // A refreshed normal Current session restores capabilities. A ROM option
    // intended for a later boot must not be used in place of this Current fact.
    model.applySnapshot(state, false, std::uint32_t{0});
    QVERIFY(!model.currentReadOnlyMediaActive());
    QVERIFY(model.files()->currentSessionExclusionsEffective());
    QVERIFY(model.commitAvailability().files);
    QVERIFY(model.commitAvailability().registry);
    QVERIFY(!model.files()->readOnly());
    QVERIFY(!model.registry()->readOnly());
    QVERIFY(model.runtimeNotice().isEmpty());
  }

  void runtimeRestrictionsUseTheProtectedTarget() {
    auto state = snapshot();
    presentation::VolumeConfigurationViewModel model(disk(), true);
    state.current.volumes.front().isProtected = false;
    state.runtime.systemVolumeProtected = false;
    model.applySnapshot(state, false, std::uint32_t{4});
    QVERIFY(!model.currentReadOnlyMediaActive());
    QVERIFY(model.files()->currentSessionExclusionsEffective());

    state.current.volumes.front().isProtected = true;
    state.runtime.systemVolumeProtected = true;
    state.current.filter.enabled = false;
    model.applySnapshot(state, false, std::uint32_t{4});
    QVERIFY(!model.currentReadOnlyMediaActive());
    QVERIFY(model.files()->currentSessionExclusionsEffective());

    state.current.filter.enabled = true;
    model.applySnapshot(state, true, std::uint32_t{4});
    QVERIFY(model.currentHormActive());
    QVERIFY(!model.commitAvailability().files);
    QVERIFY(!model.commitAvailability().registry);
    QVERIFY(model.files()->readOnly());
    QVERIFY(model.registry()->readOnly());
    QVERIFY(model.runtimeNotice().contains(QStringLiteral("HORM")));
  }

  void stagingRestrictionKeepsEntriesInspectableAndBlocksEveryMutation() {
    auto editing = std::make_shared<StagingEditingProbe>();
    presentation::FileStagingViewModel model(editing);
    const auto baseline = model.entries();
    model.setAvailability({.editingRestriction = app::RuntimeWriteRestriction::ReadOnlyMediaActive});
    QVERIFY(model.storageAvailable());
    QVERIFY(!model.readOnly());
    QVERIFY(!model.writable());
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.entries(), baseline);
    QCOMPARE(model.index(0).data(presentation::PathRole).toString(), QStringLiteral("C:\\work.txt"));
    QCOMPARE(model.index(0).data(presentation::StatusTextRole).toString(), QStringLiteral("Unavailable"));
    QVERIFY(model.index(0).data(Qt::ToolTipRole).toString().contains(presentation::automaticCommitDescription(app::AutomaticCommitDisposition::Unavailable)));
    QVERIFY(!model.index(0).data(Qt::ToolTipRole).toString().contains(QStringLiteral("does not support individual commits")));
    model.addEntries({{app::FileStagingKind::File, QStringLiteral("C:\\new.txt")}});
    model.removePaths({QStringLiteral("C:\\work.txt")});
    model.restorePaths({QStringLiteral("C:\\work.txt")});
    QCOMPARE(editing->mutations, 0);

    model.setAvailability({.readOnly = true});
    QVERIFY(!model.writable());
    model.removePaths({QStringLiteral("C:\\work.txt")});
    QCOMPARE(editing->mutations, 0);
    model.setAvailability({});
    QVERIFY(model.writable());
    model.removePaths({QStringLiteral("C:\\work.txt")});
    QCOMPARE(editing->mutations, 1);
  }
  void availabilityObserversSeeTheCompleteStagingState() {
    auto editing = std::make_shared<StagingEditingProbe>();
    presentation::FileStagingViewModel model(editing);
    const presentation::FileStagingViewModel::Availability restricted{
        .readOnly = true, .enhancedModeAvailable = true, .editingRestriction = app::RuntimeWriteRestriction::HormActive};
    auto expected = restricted;
    int observations = 0, rows = 0, modifications = 0, enhanced = 0, automatic = 0;
    const auto verify = [&] {
      ++observations;
      QCOMPARE(model.availability(), expected);
      QCOMPARE(model.writable(), expected.editingRestriction == app::RuntimeWriteRestriction::None && !expected.readOnly);
    };
    connect(&model, &QAbstractItemModel::dataChanged, &model, [&] {
      ++rows;
      verify();
    });
    connect(&model, &presentation::FileStagingViewModel::modificationAvailabilityChanged, &model, [&] {
      ++modifications;
      verify();
    });
    connect(&model, &presentation::FileStagingViewModel::enhancedModeAvailabilityChanged, &model, [&] {
      ++enhanced;
      verify();
    });
    connect(&model, &presentation::FileStagingViewModel::automaticCommitStateChanged, &model, [&] {
      ++automatic;
      verify();
    });
    model.setAvailability(restricted);
    QCOMPARE(observations, 2);
    QCOMPARE(rows, 0);
    QCOMPARE(automatic, 0);
    QCOMPARE(modifications, 1);
    QCOMPARE(enhanced, 1);
    expected = {};
    model.setAvailability(expected);
    QCOMPARE(observations, 4);
    QCOMPARE(rows, 0);
    QCOMPARE(automatic, 0);
    QCOMPARE(modifications, 2);
    QCOMPARE(enhanced, 2);
    expected.automaticCommit = {};
    model.setAvailability(expected);
    QCOMPARE(observations, 6);
    QCOMPARE(rows, 1);
    QCOMPARE(automatic, 1);
    QCOMPARE(modifications, 2);
    QCOMPARE(enhanced, 2);
    model.setAvailability(expected);
    QCOMPARE(observations, 6);
  }
  void translatedRestrictionTextCannotGrantWriteAvailability_data() {
    QTest::addColumn<int>("restriction");
    QTest::newRow("horm-restart") << static_cast<int>(app::RuntimeWriteRestriction::HormRestartRequired);
    QTest::newRow("horm-active") << static_cast<int>(app::RuntimeWriteRestriction::HormActive);
    QTest::newRow("horm-unknown") << static_cast<int>(app::RuntimeWriteRestriction::HormUnknown);
    QTest::newRow("read-only-media") << static_cast<int>(app::RuntimeWriteRestriction::ReadOnlyMediaActive);
  }
  void translatedRestrictionTextCannotGrantWriteAvailability() {
    QFETCH(int, restriction);
    auto editing = std::make_shared<StagingEditingProbe>();
    presentation::FileStagingViewModel staging(editing);
    FakeEnhancedModeActions actions;
    actions.current.serviceRegistryPresent = true;
    presentation::EnhancedModeViewModel enhanced(actions);
    enhanced.refreshStatus();
    QTRY_VERIFY(enhanced.status().has_value());
    QVERIFY(enhanced.statusText().canApply);
    QVERIFY(enhanced.statusText().canRemove);
    const auto mode = static_cast<app::RuntimeWriteRestriction>(restriction);
    staging.setAvailability({.editingRestriction = mode});
    enhanced.setWriteRestriction(mode);
    EmptyTranslator translator;
    QVERIFY(QCoreApplication::installTranslator(&translator));
    const auto removeTranslator = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });
    enhanced.retranslate();
    staging.retranslate();
    QVERIFY(presentation::runtimeWriteRestrictionText(mode).isEmpty());
    QVERIFY(!staging.writable());
    staging.addEntries({{app::FileStagingKind::File, QStringLiteral("C:\\new.txt")}});
    staging.removePaths({QStringLiteral("C:\\work.txt")});
    staging.restorePaths({QStringLiteral("C:\\work.txt")});
    QCOMPARE(editing->mutations, 0);
    QVERIFY(!enhanced.statusText().canApply);
    QVERIFY(!enhanced.statusText().canRemove);
    enhanced.applyRequestedState();
    enhanced.removeService();
    QCOMPARE(actions.enables + actions.starts + actions.disables, 0);
  }
};
}  // namespace

QTEST_GUILESS_MAIN(RuntimeModePresentationTests)
#include "RuntimeModePresentationTests.moc"
