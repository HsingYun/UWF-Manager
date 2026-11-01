// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractItemModelTester>
#include <QCoreApplication>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/staging/editing/FileStagingEditor.h"
#include "cases/presentation/viewmodels/staging/MemoryStagingStore.h"
#include "presentation/viewmodels/common/AutomaticCommitText.h"
#include "presentation/viewmodels/staging/FileStagingViewModel.h"
#include "support/app/FileTargetFixture.h"
#include "support/app/WorkerGate.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

app::AutomaticCommitSummary summaryFor(const std::initializer_list<app::AutomaticCommitDisposition> dispositions) {
  app::AutomaticCommitSummary summary;
  for (const auto disposition : dispositions) summary.include(disposition);
  return summary;
}

class ProjectedStagingEditing final : public app::FileStagingEditing {
 public:
  QList<app::FileStagingEntry> baseline;
  QList<app::FileStagingEntry> current;
  QString drive = QStringLiteral("C:");
  QString error;
  bool available = true;
  bool working = false;
  const QString& driveLetter() const override { return drive; }
  const QList<app::FileStagingEntry>& entries() const override { return current; }
  QList<app::FileStagingEntry> baselineEntries() const override { return baseline; }
  bool storageAvailable() const override { return available; }
  bool busy() const override { return working; }
  const QString& storageError() const override { return error; }
  void load() override { emit entriesChanged(); }
  void addEntries(const QList<app::FileStagingEntry>&) override { throw std::logic_error("Unexpected edit"); }
  void removePaths(const QStringList&) override { throw std::logic_error("Unexpected edit"); }
  void restorePaths(const QStringList&) override { throw std::logic_error("Unexpected edit"); }
};

class GatedStagingSource final : public app::CommitTraversalSource {
 public:
  const std::shared_ptr<test::WorkerGate> gate = std::make_shared<test::WorkerGate>();
  std::function<app::FileCommitTarget(const QString&)> targetResolver;
  std::function<std::string(const QString&)> driveResolver;
  void blockNextObservation() {
    m_observedOnOwner = false;
    m_watching = true;
    m_blockNext = true;
  }
  [[nodiscard]] bool observedOnOwner() const { return m_observedOnOwner; }
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override {
    observe();
    return app::bindFileCommitTarget(*this, target);
  }
  app::FileCommitTarget resolveFileTarget(const QString& path) const override {
    observe();
    return targetResolver ? targetResolver(path) : test::application::memoryFileTarget(path);
  }
  std::string driveLetterForPath(const QString& path) const override {
    observe();
    return driveResolver ? driveResolver(path) : path.left(2).toUpper().toStdString();
  }
  bool exists(const app::CommitNode&) const override { throw std::logic_error("Unexpected existence read"); }
  QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override {
    throw std::logic_error("Unexpected traversal");
  }

 private:
  void observe() const {
    if (!m_watching) return;
    if (std::this_thread::get_id() == m_owner) m_observedOnOwner = true;
    if (m_blockNext.exchange(false)) gate->wait({}, false);
  }
  const std::thread::id m_owner = std::this_thread::get_id();
  std::atomic_bool m_watching{false};
  mutable std::atomic_bool m_blockNext{false};
  mutable std::atomic_bool m_observedOnOwner{false};
};

class FileStagingViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void stagingEditsPreserveOtherVolumesAndReloadPartialWrites();
  void stagingRowsProjectTheCompleteDraftWithoutReadSideEffects();
  void stagingAvailabilityKeepsReadOnlyAndEnhancedIndependent();
  void stagingCleanupDoesNotAdmitAdditionsOrRestoration();
  void stagingCleanupKeepsExistingWriteBarriers_data();
  void stagingCleanupKeepsExistingWriteBarriers();
  void stagingStorageRecoveryClearsErrorsAndPublishesAvailability();
  void rowStatusUsesTheSameSelectionChangesAsTheDraft_data();
  void rowStatusUsesTheSameSelectionChangesAsTheDraft();
  void explicitAddressEditsRemainPendingThroughTheUiModel_data();
  void explicitAddressEditsRemainPendingThroughTheUiModel();
  void automaticCommitDispositionIsIndependentOfListWriteAdmission_data();
  void automaticCommitDispositionIsIndependentOfListWriteAdmission();
  void automaticCommitDispositionChangesRefreshRowsAndSummary();
  void automaticCommitSummariesPreserveEveryScopeAndRecover();
  void automaticCommitRowsUseTheirTargetVolumes();
  void automaticCommitVolumeChangesRefreshRowsWithTheSameAggregate();
  void automaticCommitSummaryFollowsCurrentEntries();
  void initialPreparationPublishesEveryWaitingVolume();
  void initialReadFailureAndRecoveryPublishToEveryWaitingVolume();
  void destroyingTheInitialLoaderAllowsAnotherVolumeToRetry();
  void editingPreparationKeepsTheEventLoopResponsive_data();
  void editingPreparationKeepsTheEventLoopResponsive();
  void completedPreparationCannotReplaceANewerDraft();
  void destroyingTheViewModelReleasesSharedBusyWithoutJoining();
};

void FileStagingViewModelTests::automaticCommitDispositionIsIndependentOfListWriteAdmission_data() {
  using Disposition = app::AutomaticCommitDisposition;
  using Restriction = app::RuntimeWriteRestriction;
  QTest::addColumn<int>("disposition");
  QTest::addColumn<int>("restriction");
  QTest::addColumn<bool>("readOnly");
  QTest::addColumn<QString>("status");
  QTest::addColumn<bool>("writable");
  const auto row = [](const char* name, const Disposition disposition, const Restriction restriction, const bool readOnly, const QString& status,
                      const bool writable) {
    QTest::newRow(name) << static_cast<int>(disposition) << static_cast<int>(restriction) << readOnly << status << writable;
  };
  row("normal", Disposition::CheckDurability, Restriction::None, false, QStringLiteral("Ready"), true);
  row("retained-overlay", Disposition::SkipRetainedOverlay, Restriction::None, false, QStringLiteral("Paused"), true);
  row("rom-protected-registry", Disposition::SkipReadOnlyMedia, Restriction::ReadOnlyMediaActive, false, QStringLiteral("Paused"), false);
  row("rom-independent-list-admission", Disposition::SkipReadOnlyMedia, Restriction::None, false, QStringLiteral("Paused"), true);
  row("unavailable", Disposition::Unavailable, Restriction::None, false, QStringLiteral("Unavailable"), true);
  row("already-persistent", Disposition::AlreadyPersistent, Restriction::None, false, QStringLiteral("No files to commit"), true);
  row("ordinary-list-write-blocked", Disposition::CheckDurability, Restriction::HormUnknown, false, QStringLiteral("Ready"), false);
  row("read-only-retained-overlay", Disposition::SkipRetainedOverlay, Restriction::None, true, QStringLiteral("Paused"), false);
}

void FileStagingViewModelTests::automaticCommitDispositionIsIndependentOfListWriteAdmission() {
  QFETCH(int, disposition);
  QFETCH(int, restriction);
  QFETCH(bool, readOnly);
  QFETCH(QString, status);
  QFETCH(bool, writable);
  auto editing = std::make_shared<ProjectedStagingEditing>();
  editing->baseline = editing->current = {{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
  presentation::FileStagingViewModel model(editing);
  const auto automatic = static_cast<app::AutomaticCommitDisposition>(disposition);
  model.setAvailability(
      {.readOnly = readOnly, .editingRestriction = static_cast<app::RuntimeWriteRestriction>(restriction), .automaticCommit = summaryFor({automatic})});
  QCOMPARE(model.writable(), writable);
  QCOMPARE(model.index(0).data(presentation::StatusTextRole).toString(), status);
  const auto tooltip = model.index(0).data(Qt::ToolTipRole).toString();
  QVERIFY(tooltip.endsWith(model.automaticCommitPresentation().description));
  QVERIFY(!tooltip.contains(QStringLiteral("Committed automatically before safe shutdown or restart.")));
  QVERIFY(model.summaryText().endsWith(status));
  QCOMPARE(model.entries(), editing->baseline);
}

void FileStagingViewModelTests::automaticCommitDispositionChangesRefreshRowsAndSummary() {
  auto editing = std::make_shared<ProjectedStagingEditing>();
  editing->baseline = editing->current = {{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
  presentation::FileStagingViewModel model(editing);
  QSignalSpy rows(&model, &QAbstractItemModel::dataChanged);
  QSignalSpy summary(&model, &presentation::FileStagingViewModel::automaticCommitStateChanged);
  QSignalSpy edits(&model, &presentation::FileStagingViewModel::entriesChanged);
  QSignalSpy writes(&model, &presentation::FileStagingViewModel::modificationAvailabilityChanged);
  model.setAvailability({.automaticCommit = summaryFor({app::AutomaticCommitDisposition::SkipRetainedOverlay})});
  QCOMPARE(rows.count(), 1);
  QCOMPARE(summary.count(), 1);
  QCOMPARE(writes.count(), 0);
  QVERIFY(model.writable());
  QVERIFY(model.summaryText().endsWith(QStringLiteral("Paused")));
  model.setAvailability({.automaticCommit = summaryFor({app::AutomaticCommitDisposition::SkipReadOnlyMedia})});
  QCOMPARE(rows.count(), 2);
  QCOMPARE(summary.count(), 2);
  QCOMPARE(writes.count(), 0);
  QVERIFY(model.index(0).data(Qt::ToolTipRole).toString().contains(QStringLiteral("Read-only media")));
  model.setAvailability({.automaticCommit = summaryFor({app::AutomaticCommitDisposition::SkipReadOnlyMedia})});
  QCOMPARE(rows.count(), 2);
  QCOMPARE(summary.count(), 2);
  QCOMPARE(edits.count(), 0);
}

void FileStagingViewModelTests::automaticCommitSummariesPreserveEveryScopeAndRecover() {
  using D = app::AutomaticCommitDisposition;
  auto editing = std::make_shared<ProjectedStagingEditing>();
  editing->baseline = editing->current = {{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
  presentation::FileStagingViewModel model(editing);
  QCOMPARE(model.index(0).data(presentation::StatusTextRole).toString(), QStringLiteral("Unavailable"));
  QVERIFY(model.writable());
  auto summary = summaryFor({D::SkipHormActive, D::SkipHormTransition, D::SkipFullCommit, D::SkipReadOnlyMedia, D::SkipRetainedOverlay, D::CheckDurability,
                             D::AlreadyPersistent, D::Unavailable});
  QSignalSpy rows(&model, &QAbstractItemModel::dataChanged);
  QSignalSpy automatic(&model, &presentation::FileStagingViewModel::automaticCommitStateChanged);
  QSignalSpy writes(&model, &presentation::FileStagingViewModel::modificationAvailabilityChanged);
  model.setAvailability({.enhancedModeAvailable = true, .automaticCommit = summary});
  QCOMPARE(rows.count(), 1);
  QCOMPARE(automatic.count(), 1);
  QCOMPARE(writes.count(), 0);
  const auto unknown = model.automaticCommitPresentation();
  QCOMPARE(unknown.status, QStringLiteral("Unavailable"));
  for (const auto reason : {D::SkipHormActive, D::SkipHormTransition, D::SkipFullCommit, D::SkipReadOnlyMedia, D::SkipRetainedOverlay, D::Unavailable})
    QVERIFY(unknown.description.contains(presentation::automaticCommitDescription(reason)));
  QVERIFY(model.index(0).data(Qt::ToolTipRole).toString().endsWith(unknown.description));
  summary.unavailable = false;
  model.setAvailability({.enhancedModeAvailable = true, .automaticCommit = summary});
  QCOMPARE(model.index(0).data(presentation::StatusTextRole).toString(), QStringLiteral("Partially paused"));
  const auto beforeEditingLock = model.automaticCommitPresentation().description;
  model.setAvailability({.enhancedModeAvailable = true, .editingRestriction = app::RuntimeWriteRestriction::HormUnknown, .automaticCommit = summary});
  QVERIFY(!model.writable());
  QCOMPARE(model.automaticCommitPresentation().description, beforeEditingLock);
  QCOMPARE(automatic.count(), 2);
  QCOMPARE(rows.count(), 2);
  QCOMPARE(writes.count(), 1);
  model.setAvailability({.automaticCommit = summaryFor({D::AlreadyPersistent})});
  QVERIFY(model.writable());
  QCOMPARE(model.index(0).data(presentation::StatusTextRole).toString(), QStringLiteral("No files to commit"));
  QVERIFY(!model.index(0).data(Qt::ToolTipRole).toString().contains(QStringLiteral("Only eligible staged files will be committed.")));
  model.setAvailability({.automaticCommit = {}});
  QCOMPARE(model.index(0).data(presentation::StatusTextRole).toString(), QStringLiteral("Ready"));
  QVERIFY(!model.summaryText().contains(QStringLiteral("paused"), Qt::CaseInsensitive));
  QVERIFY(!model.index(0).data(Qt::ToolTipRole).toString().contains(presentation::automaticCommitDescription(D::SkipReadOnlyMedia)));
}

void FileStagingViewModelTests::automaticCommitRowsUseTheirTargetVolumes() {
  using D = app::AutomaticCommitDisposition;
  auto editing = std::make_shared<ProjectedStagingEditing>();
  editing->baseline =
      editing->current = {{app::FileStagingKind::File, QStringLiteral("C:\\ready.bin"), test::application::memoryFileTarget(QStringLiteral("C:\\ready.bin"))},
                          {app::FileStagingKind::File, QStringLiteral("D:\\paused.bin"),
                           app::FileCommitTarget{R"(\\?\Volume{66666666-7777-8888-9999-AAAAAAAAAAAA}\)", QStringLiteral("\\paused.bin")}},
                          {app::FileStagingKind::File, QStringLiteral("E:\\persistent.bin"),
                           test::application::memoryFileTarget(QStringLiteral("E:\\persistent.bin"), "bbbbbbbb-cccc-dddd-eeee-ffffffffffff")},
                          {app::FileStagingKind::File, QStringLiteral("F:\\offline.bin")}};
  presentation::FileStagingViewModel model(editing);
  model.setAvailability(
      {.automaticCommit = summaryFor({D::CheckDurability}),
       .automaticCommitVolumes = presentation::FileStagingViewModel::AutomaticCommitVolumes{
           .dispositions = {{test::application::fixtureVolumeId, D::CheckDurability}, {test::application::otherFixtureVolumeId, D::SkipRetainedOverlay}},
           .unregistered = D::AlreadyPersistent}});
  QCOMPARE(model.index(0).data(presentation::StatusTextRole).toString(), QStringLiteral("Ready"));
  QCOMPARE(model.index(1).data(presentation::StatusTextRole).toString(), QStringLiteral("Paused"));
  QCOMPARE(model.index(2).data(presentation::StatusTextRole).toString(), QStringLiteral("No files to commit"));
  QCOMPARE(model.index(3).data(presentation::StatusTextRole).toString(), QStringLiteral("Unavailable"));
  const auto pause = presentation::automaticCommitDescription(D::SkipRetainedOverlay);
  QVERIFY(!model.index(0).data(Qt::ToolTipRole).toString().contains(pause));
  QVERIFY(model.index(1).data(Qt::ToolTipRole).toString().contains(pause));
  QVERIFY(model.index(2).data(Qt::ToolTipRole).toString().contains(QStringLiteral("already on unprotected storage")));
  QVERIFY(model.index(3).data(Qt::ToolTipRole).toString().contains(presentation::automaticCommitDescription(D::Unavailable)));
  QCOMPARE(model.automaticCommitPresentation().status, QStringLiteral("Unavailable"));
  QVERIFY(model.automaticCommitPresentation().description.contains(pause));
  QVERIFY(model.writable());
}

void FileStagingViewModelTests::automaticCommitVolumeChangesRefreshRowsWithTheSameAggregate() {
  using D = app::AutomaticCommitDisposition;
  auto editing = std::make_shared<ProjectedStagingEditing>();
  editing->baseline =
      editing->current = {{app::FileStagingKind::File, QStringLiteral("C:\\one.bin"), test::application::memoryFileTarget(QStringLiteral("C:\\one.bin"))},
                          {app::FileStagingKind::File, QStringLiteral("D:\\two.bin"),
                           test::application::memoryFileTarget(QStringLiteral("D:\\two.bin"), test::application::otherFixtureVolumeId)}};
  presentation::FileStagingViewModel model(editing);
  presentation::FileStagingViewModel::Availability availability{
      .automaticCommit = summaryFor({D::CheckDurability}),
      .automaticCommitVolumes = presentation::FileStagingViewModel::AutomaticCommitVolumes{
          .dispositions = {{test::application::fixtureVolumeId, D::CheckDurability}, {test::application::otherFixtureVolumeId, D::SkipRetainedOverlay}}}};
  model.setAvailability(availability);
  const auto summary = model.summaryText();
  QVERIFY(summary.endsWith(QStringLiteral("Partially paused")));
  QSignalSpy rows(&model, &QAbstractItemModel::dataChanged);
  QSignalSpy automatic(&model, &presentation::FileStagingViewModel::automaticCommitStateChanged);
  QSignalSpy writes(&model, &presentation::FileStagingViewModel::modificationAvailabilityChanged);
  availability.automaticCommitVolumes->dispositions[test::application::fixtureVolumeId] = D::SkipRetainedOverlay;
  availability.automaticCommitVolumes->dispositions[test::application::otherFixtureVolumeId] = D::CheckDurability;
  model.setAvailability(availability);
  QCOMPARE(model.index(0).data(presentation::StatusTextRole).toString(), QStringLiteral("Paused"));
  QCOMPARE(model.index(1).data(presentation::StatusTextRole).toString(), QStringLiteral("Ready"));
  QCOMPARE(model.summaryText(), summary);
  QCOMPARE(rows.count(), 1);
  QCOMPARE(automatic.count(), 1);
  QCOMPARE(writes.count(), 0);
  model.setAvailability(availability);
  QCOMPARE(rows.count(), 1);
  QCOMPARE(automatic.count(), 1);
}

void FileStagingViewModelTests::automaticCommitSummaryFollowsCurrentEntries() {
  using D = app::AutomaticCommitDisposition;
  auto editing = std::make_shared<ProjectedStagingEditing>();
  const app::FileStagingEntry ready{app::FileStagingKind::File, QStringLiteral("C:\\one.bin"),
                                    test::application::memoryFileTarget(QStringLiteral("C:\\one.bin"))};
  const app::FileStagingEntry paused{app::FileStagingKind::File, QStringLiteral("D:\\two.bin"),
                                     test::application::memoryFileTarget(QStringLiteral("D:\\two.bin"), test::application::otherFixtureVolumeId)};
  editing->baseline = editing->current = {ready, paused};
  presentation::FileStagingViewModel model(editing);
  model.setAvailability(
      {.automaticCommit = summaryFor({D::CheckDurability}),
       .automaticCommitVolumes = presentation::FileStagingViewModel::AutomaticCommitVolumes{
           .dispositions = {{test::application::fixtureVolumeId, D::CheckDurability}, {test::application::otherFixtureVolumeId, D::SkipRetainedOverlay}}}});
  QSignalSpy automatic(&model, &presentation::FileStagingViewModel::automaticCommitStateChanged);
  editing->current = {paused};
  editing->load();
  QCOMPARE(automatic.count(), 1);
  QVERIFY(model.summaryText().endsWith(QStringLiteral("Paused")));
  QCOMPARE(model.index(0).data(presentation::StatusTextRole).toString(), QStringLiteral("Pending removal"));
  QVERIFY(!model.index(0).data(Qt::ToolTipRole).toString().contains(presentation::automaticCommitDescription(D::SkipRetainedOverlay)));
  editing->current.clear();
  editing->load();
  QCOMPARE(automatic.count(), 2);
  QVERIFY(model.summaryText().endsWith(QStringLiteral("Ready")));
  QCOMPARE(model.rowCount(), 2);
  QVERIFY(model.index(1).data(Qt::ToolTipRole).toString().contains(presentation::automaticCommitDescription(D::SkipRetainedOverlay)));
  editing->current = {ready};
  editing->load();
  QCOMPARE(automatic.count(), 2);
  editing->current = {ready, paused};
  editing->load();
  QCOMPARE(automatic.count(), 3);
  QVERIFY(model.summaryText().endsWith(QStringLiteral("Partially paused")));
}

void FileStagingViewModelTests::rowStatusUsesTheSameSelectionChangesAsTheDraft_data() {
  QTest::addColumn<int>("scenario");
  QTest::newRow("known-object-address-alias") << 0;
  QTest::newRow("binding-observed") << 1;
  QTest::newRow("binding-unavailable") << 2;
  QTest::newRow("known-different-volume-at-same-display") << 3;
  QTest::newRow("unbound-addresses-are-not-guessed") << 4;
}

void FileStagingViewModelTests::rowStatusUsesTheSameSelectionChangesAsTheDraft() {
  QFETCH(int, scenario);
  auto editing = std::make_shared<ProjectedStagingEditing>();
  app::FileStagingEntry before{app::FileStagingKind::File, QStringLiteral("C:\\Data\\state.bin")};
  auto after = before;
  const app::FileCommitTarget target{test::application::fixtureVolumeId, QStringLiteral("\\Data\\state.bin")};
  if (scenario != 1 && scenario != 4) before.fileTarget = target;
  if (scenario != 2) after.fileTarget = target;
  if (scenario == 0 || scenario == 4) after.path = QStringLiteral(R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\Data\state.bin)");
  if (scenario == 3) after.fileTarget->volumeId = test::application::otherFixtureVolumeId;
  editing->baseline = {before};
  editing->current = {after};
  presentation::FileStagingViewModel model(editing);
  if (scenario == 1 || scenario == 2) {
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Normal));
  } else {
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.index(0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Removed));
    QCOMPARE(model.index(1).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Added));
  }
}

void FileStagingViewModelTests::explicitAddressEditsRemainPendingThroughTheUiModel_data() {
  QTest::addColumn<QString>("originalDrive");
  QTest::newRow("saved-address-sorts-first") << QStringLiteral("C:");
  QTest::newRow("replacement-address-sorts-first") << QStringLiteral("E:");
}

void FileStagingViewModelTests::explicitAddressEditsRemainPendingThroughTheUiModel() {
  QFETCH(QString, originalDrive);
  using namespace test::application;
  const auto mountRoot = originalDrive + QStringLiteral("\\MountedD");
  const app::FileStagingEntry alias{app::FileStagingKind::File, mountRoot + QStringLiteral("\\state.bin")};
  const app::FileStagingEntry direct{app::FileStagingKind::File, QStringLiteral("D:\\state.bin")};
  bool mounted = false;
  bool retired = false;
  auto source = std::make_shared<GatedStagingSource>();
  source->targetResolver = [&](const QString& path) {
    if (mounted && path.startsWith(mountRoot + u'\\')) {
      if (retired) throw std::runtime_error("Old mount address is unavailable");
      return app::FileCommitTarget{otherFixtureVolumeId, path.mid(mountRoot.size())};
    }
    return memoryFileTarget(path, path.startsWith(u"D:") ? otherFixtureVolumeId : fixtureVolumeId);
  };
  source->driveResolver = [&](const QString& path) {
    return mounted && path.startsWith(mountRoot + u'\\') ? std::string("D:") : path.left(2).toStdString();
  };
  auto store = std::make_shared<MemoryStagingStore>();
  app::FileStagingDraft draft(store, source);
  presentation::FileStagingViewModel initialModel(std::make_shared<app::FileStagingEditor>(originalDrive, draft, source));
  initialModel.load();
  QTRY_VERIFY(!initialModel.busy());
  initialModel.addEntries({alias});
  QTRY_VERIFY(!initialModel.busy());
  QCOMPARE(initialModel.entries().constFirst().fileTarget->volumeId, std::string(fixtureVolumeId));
  draft.apply(*draft.changes());
  QCOMPARE(store->writes, 1);
  mounted = true;
  draft.reload();
  QVERIFY(initialModel.entries().isEmpty());
  QVERIFY(!draft.changes());
  presentation::FileStagingViewModel dModel(std::make_shared<app::FileStagingEditor>(QStringLiteral("D:"), draft, source));
  dModel.load();
  QTRY_VERIFY(!dModel.busy());
  const auto original = dModel.entries().constFirst().fileTarget;
  QCOMPARE(original->volumeId, std::string(otherFixtureVolumeId));
  dModel.removePaths({alias.path});
  QTRY_VERIFY(!dModel.busy());
  dModel.addEntries({direct});
  QTRY_VERIFY(!dModel.busy());
  QCOMPARE(dModel.entries(), QList<app::FileStagingEntry>{direct});
  QCOMPARE(dModel.entries().constFirst().fileTarget, original);
  QCOMPARE(dModel.rowCount(), 2);
  QCOMPARE(dModel.index(0).data(presentation::PathRole).toString(), alias.path);
  QCOMPARE(dModel.index(0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Removed));
  QCOMPARE(dModel.index(1).data(presentation::PathRole).toString(), direct.path);
  QCOMPARE(dModel.index(1).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Added));
  dModel.restorePaths({dModel.index(0).data(presentation::PathRole).toString()});
  QTRY_VERIFY(!dModel.busy());
  QCOMPARE(dModel.entries(), QList<app::FileStagingEntry>{alias});
  QCOMPARE(dModel.entries().constFirst().fileTarget, original);
  QCOMPARE(dModel.rowCount(), 1);
  QCOMPARE(dModel.index(0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Normal));
  QVERIFY(!draft.changes());
  QCOMPARE(store->entries, QList<app::FileStagingEntry>{alias});
  QCOMPARE(store->writes, 1);
  dModel.removePaths({alias.path});
  QTRY_VERIFY(!dModel.busy());
  dModel.addEntries({direct});
  QTRY_VERIFY(!dModel.busy());
  QCOMPARE(draft.changes()->count(), std::size_t{2});
  core::UwfSnapshot snapshot;
  snapshot.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
  const auto plan = app::planConfiguration({}, snapshot, draft.editSnapshot());
  QCOMPARE(plan.count(), std::size_t{2});
  QVERIFY(plan.hasStagingChanges());
  QCOMPARE(plan.operations().size(), std::size_t{1});
  QCOMPARE(store->entries, QList<app::FileStagingEntry>{alias});
  draft.apply(*plan.staging);
  QCOMPARE(store->writes, 2);
  QCOMPARE(store->entries, QList<app::FileStagingEntry>{direct});
  QCOMPARE(dModel.rowCount(), 1);
  QCOMPARE(dModel.index(0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Normal));
  retired = true;
  draft.reload();
  QCOMPARE(dModel.entries(), QList<app::FileStagingEntry>{direct});
  QVERIFY(!draft.changes());
}

void FileStagingViewModelTests::stagingEditsPreserveOtherVolumesAndReloadPartialWrites() {
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\one.txt")}, {app::FileStagingKind::File, QStringLiteral("D:\\two.txt")}};
  app::FileStagingDraft draft(storeOwner);
  auto editing = std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, std::make_shared<test::application::StoredPathSource>());
  presentation::FileStagingViewModel model(editing);
  model.load();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(model.entries().size(), 1);
  QSignalSpy warnings(&model, &presentation::FileStagingViewModel::warningRaised);
  store.failAfterReplace = true;
  model.removePaths({QStringLiteral("c:\\ONE.txt")});
  QTRY_VERIFY(!model.busy());
  QCOMPARE(store.writes, 0);
  QCOMPARE(store.entries.size(), 2);
  QCOMPARE(draft.load().size(), 1);
  QCOMPARE(draft.load().constFirst().path, QStringLiteral("D:\\two.txt"));
  QVERIFY(model.entries().isEmpty());
  QVERIFY(model.storageAvailable());
  QCOMPARE(warnings.count(), 0);
  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(model.index(0, 0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Removed));
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, draft.apply(*draft.changes()));
  QVERIFY(draft.changes());
  store.failAfterReplace = false;
  draft.apply(*draft.changes());
  QVERIFY(!draft.changes());
  QCoreApplication::processEvents();
  QCOMPARE(model.rowCount(), 0);
  store.failRead = true;
  QVERIFY_THROWS_EXCEPTION(std::runtime_error, draft.reload());
  QVERIFY(!model.storageAvailable());
  QVERIFY(!model.writable());
}

void FileStagingViewModelTests::stagingRowsProjectTheCompleteDraftWithoutReadSideEffects() {
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  for (int index = 0; index < 2000; ++index)
    store.entries.append({index % 2 ? app::FileStagingKind::Directory : app::FileStagingKind::File, QStringLiteral("C:/Original/item-%1").arg(index)});
  app::FileStagingDraft draft(storeOwner);
  presentation::FileStagingViewModel model(
      std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, std::make_shared<test::application::StoredPathSource>()));
  model.load();
  QTRY_VERIFY(!model.busy());
  QAbstractItemModelTester contract(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
  QSignalSpy resets(&model, &QAbstractItemModel::modelReset);
  auto next = store.entries.mid(700);
  for (auto& entry : next) entry.path = entry.path.toUpper().replace(u'/', u'\\');
  for (int index = 0; index < 500; ++index) next.append({app::FileStagingKind::File, QStringLiteral("C:/New/item-%1").arg(index)});
  draft.replace(next);
  QTRY_COMPARE(resets.count(), 1);
  QCOMPARE(model.rowCount(), 2500);
  QCOMPARE(model.summaryText(), QStringLiteral("1800 staged entries · 1150 files · 650 folders · Unavailable"));
  for (int pass = 0; pass < 3; ++pass) {
    int added = 0, removed = 0, retained = 0;
    for (int row = 0; row < model.rowCount(); ++row) {
      const auto index = model.index(row);
      const auto status = static_cast<presentation::ItemStatus>(index.data(presentation::StatusRole).toInt());
      if (status == presentation::ItemStatus::Added)
        ++added;
      else if (status == presentation::ItemStatus::Removed)
        ++removed;
      else
        ++retained;
      QVERIFY(index.data(presentation::PathRole).isValid());
      QVERIFY(index.data(presentation::BadgeRole).isValid());
    }
    QCOMPARE(added, 500);
    QCOMPARE(removed, 700);
    QCOMPARE(retained, 1300);
  }
  QCOMPARE(resets.count(), 1);
  QCOMPARE(store.writes, 0);
}

void FileStagingViewModelTests::stagingAvailabilityKeepsReadOnlyAndEnhancedIndependent() {
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\one.txt")}};
  app::FileStagingDraft draft(storeOwner);
  presentation::FileStagingViewModel model(
      std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, std::make_shared<test::application::StoredPathSource>()));
  model.load();
  QTRY_VERIFY(!model.busy());
  QVERIFY(!model.enhancedModeAvailable());
  QVERIFY(model.writable());
  model.setAvailability({.readOnly = true});
  model.removePaths({QStringLiteral("C:\\one.txt")});
  QCOMPARE(store.writes, 0);
  model.setAvailability({.readOnly = true, .enhancedModeAvailable = true});
  QVERIFY(!model.writable());
  model.setAvailability({});
  model.removePaths({QStringLiteral("C:\\one.txt")});
  QTRY_VERIFY(!model.busy());
  QCOMPARE(store.writes, 0);
  QVERIFY(model.entries().isEmpty());
}

void FileStagingViewModelTests::stagingCleanupDoesNotAdmitAdditionsOrRestoration() {
  const app::FileStagingEntry saved{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")};
  const app::FileStagingEntry added{app::FileStagingKind::File, QStringLiteral("C:\\new.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {saved};
  auto source = std::make_shared<GatedStagingSource>();
  app::FileStagingDraft draft(store, source);
  presentation::FileStagingViewModel model(std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, source));
  model.load();
  QTRY_VERIFY(!model.busy());
  QVERIFY(model.canAddEntries());
  QSignalSpy availability(&model, &presentation::FileStagingViewModel::modificationAvailabilityChanged);
  model.setAvailability({.additionsAllowed = false});
  QCOMPARE(availability.count(), 1);
  QVERIFY(model.writable());
  QVERIFY(!model.canAddEntries());
  model.addEntries({added});
  QVERIFY(!model.busy());
  QCOMPARE(model.entries(), QList<app::FileStagingEntry>{saved});
  QCOMPARE(availability.count(), 1);
  model.removePaths({saved.path});
  QTRY_VERIFY(!model.busy());
  QVERIFY(model.entries().isEmpty());
  QCOMPARE(model.index(0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Removed));
  QVERIFY(draft.changes());
  QCOMPARE(draft.changes()->removed(), QList<app::FileStagingEntry>{saved});
  availability.clear();
  model.restorePaths({saved.path});
  QVERIFY(!model.busy());
  QVERIFY(model.entries().isEmpty());
  QCOMPARE(availability.count(), 0);
  QCOMPARE(store->entries, QList<app::FileStagingEntry>{saved});
  QCOMPARE(store->writes, 0);
  model.setAvailability({.additionsAllowed = true});
  QCOMPARE(availability.count(), 1);
  QVERIFY(model.canAddEntries());
  model.restorePaths({saved.path});
  QTRY_VERIFY(!model.busy());
  QCOMPARE(model.entries(), QList<app::FileStagingEntry>{saved});
  model.addEntries({added});
  QTRY_VERIFY(!model.busy());
  QCOMPARE(model.entries().size(), 2);
}

void FileStagingViewModelTests::stagingCleanupKeepsExistingWriteBarriers_data() {
  QTest::addColumn<int>("barrier");
  QTest::newRow("global-read-only") << 0;
  QTest::newRow("storage-unavailable") << 1;
  QTest::newRow("mode-restriction") << 2;
  QTest::newRow("busy") << 3;
}

void FileStagingViewModelTests::stagingCleanupKeepsExistingWriteBarriers() {
  QFETCH(int, barrier);
  auto editing = std::make_shared<ProjectedStagingEditing>();
  editing->baseline = editing->current = {{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
  editing->available = barrier != 1;
  editing->working = barrier == 3;
  presentation::FileStagingViewModel model(editing);
  model.setAvailability({.readOnly = barrier == 0,
                         .editingRestriction = barrier == 2 ? app::RuntimeWriteRestriction::HormUnknown : app::RuntimeWriteRestriction::None,
                         .additionsAllowed = false});
  QVERIFY(!model.writable());
  QVERIFY(!model.canAddEntries());
  // The fixture rejects every editing call, including preparations completed synchronously.
  model.addEntries({{app::FileStagingKind::File, QStringLiteral("C:\\new.bin")}});
  model.removePaths({QStringLiteral("C:\\saved.bin")});
  model.restorePaths({QStringLiteral("C:\\saved.bin")});
  QCOMPARE(model.entries(), editing->baseline);
}

void FileStagingViewModelTests::stagingStorageRecoveryClearsErrorsAndPublishesAvailability() {
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  app::FileStagingDraft draft(storeOwner);
  auto editor = std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, std::make_shared<test::application::StoredPathSource>());
  presentation::FileStagingViewModel model(editor);
  QSignalSpy availability(&model, &presentation::FileStagingViewModel::storageAvailabilityChanged);
  store.failRead = true;
  model.load();
  QTRY_VERIFY(!model.busy());
  QVERIFY(!model.storageAvailable());
  QVERIFY(!model.storageError().isEmpty());
  store.failRead = false;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\restored.bin")}};
  model.load();
  QTRY_VERIFY(!model.busy());
  QVERIFY(model.storageAvailable());
  QVERIFY(model.storageError().isEmpty());
  QVERIFY(model.writable());
  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(availability.size(), 2);
  QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("C:\\restored.bin"));
}

void FileStagingViewModelTests::initialPreparationPublishesEveryWaitingVolume() {
  const app::FileStagingEntry cEntry{app::FileStagingKind::File, QStringLiteral("C:\\c-existing.bin")};
  const app::FileStagingEntry dEntry{app::FileStagingKind::File, QStringLiteral("D:\\d-existing.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {cEntry, dEntry};
  auto source = std::make_shared<GatedStagingSource>();
  app::FileStagingDraft draft(store, source);
  presentation::FileStagingViewModel cModel(std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, source));
  presentation::FileStagingViewModel dModel(std::make_shared<app::FileStagingEditor>(QStringLiteral("D:"), draft, source));
  QSignalSpy cPublished(&cModel, &presentation::FileStagingViewModel::entriesChanged);
  QSignalSpy dPublished(&dModel, &presentation::FileStagingViewModel::entriesChanged);
  const auto release = qScopeGuard([gate = source->gate] { gate->release(); });
  source->blockNextObservation();
  cModel.load();
  QTRY_COMPARE(source->gate->started.load(), 1);
  QVERIFY(cModel.busy() && dModel.busy());
  dModel.load();
  QVERIFY(cModel.entries().isEmpty() && dModel.entries().isEmpty());
  QCOMPARE(cPublished.count(), 0);
  QCOMPARE(dPublished.count(), 0);
  source->gate->release();
  QTRY_VERIFY(!cModel.busy() && !dModel.busy());
  QCOMPARE(cModel.entries(), QList<app::FileStagingEntry>{cEntry});
  QCOMPARE(dModel.entries(), QList<app::FileStagingEntry>{dEntry});
  QCOMPARE(cPublished.count(), 1);
  QCOMPARE(dPublished.count(), 1);
  QVERIFY(!source->observedOnOwner());
  QCOMPARE(store->writes, 0);
}

void FileStagingViewModelTests::initialReadFailureAndRecoveryPublishToEveryWaitingVolume() {
  const app::FileStagingEntry cEntry{app::FileStagingKind::File, QStringLiteral("C:\\c-existing.bin")};
  const app::FileStagingEntry dEntry{app::FileStagingKind::File, QStringLiteral("D:\\d-existing.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->failRead = true;
  auto source = std::make_shared<test::application::StoredPathSource>();
  app::FileStagingDraft draft(store, source);
  presentation::FileStagingViewModel cModel(std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, source));
  presentation::FileStagingViewModel dModel(std::make_shared<app::FileStagingEditor>(QStringLiteral("D:"), draft, source));
  QSignalSpy cAvailability(&cModel, &presentation::FileStagingViewModel::storageAvailabilityChanged);
  QSignalSpy dAvailability(&dModel, &presentation::FileStagingViewModel::storageAvailabilityChanged);
  // Both loads start before the owner event loop can deliver the shared read's result.
  cModel.load();
  dModel.load();
  QVERIFY(cModel.busy() && dModel.busy());
  QTRY_VERIFY(!cModel.busy() && !dModel.busy());
  QVERIFY(!cModel.storageAvailable() && !dModel.storageAvailable());
  QVERIFY(!cModel.writable() && !dModel.writable());
  QVERIFY(cModel.storageError().contains(QStringLiteral("read failed")));
  QCOMPARE(dModel.storageError(), cModel.storageError());
  QCOMPARE(cAvailability.count(), 1);
  QCOMPARE(dAvailability.count(), 1);

  store->failRead = false;
  store->entries = {cEntry, dEntry};
  // Recovery may be initiated by the page that skipped the failed shared load.
  dModel.load();
  QTRY_VERIFY(!cModel.busy() && !dModel.busy());
  QVERIFY(cModel.storageAvailable() && dModel.storageAvailable());
  QVERIFY(cModel.writable() && dModel.writable());
  QVERIFY(cModel.storageError().isEmpty() && dModel.storageError().isEmpty());
  QCOMPARE(cModel.entries(), QList<app::FileStagingEntry>{cEntry});
  QCOMPARE(dModel.entries(), QList<app::FileStagingEntry>{dEntry});
  QCOMPARE(cAvailability.count(), 2);
  QCOMPARE(dAvailability.count(), 2);
  QCOMPARE(store->writes, 0);
}

void FileStagingViewModelTests::destroyingTheInitialLoaderAllowsAnotherVolumeToRetry() {
  const app::FileStagingEntry cEntry{app::FileStagingKind::File, QStringLiteral("C:\\c-existing.bin")};
  const app::FileStagingEntry dEntry{app::FileStagingKind::File, QStringLiteral("D:\\d-existing.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {cEntry, dEntry};
  auto source = std::make_shared<GatedStagingSource>();
  app::FileStagingDraft draft(store, source);
  auto initial = std::make_unique<presentation::FileStagingViewModel>(std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, source));
  presentation::FileStagingViewModel peer(std::make_shared<app::FileStagingEditor>(QStringLiteral("D:"), draft, source));
  QSignalSpy changes(&draft, &app::FileStagingDraft::changed);
  const auto release = qScopeGuard([gate = source->gate] { gate->release(); });
  source->blockNextObservation();
  initial->load();
  QTRY_COMPARE(source->gate->started.load(), 1);
  peer.load();
  QVERIFY(peer.busy());
  initial.reset();
  QCOMPARE(source->gate->finished.load(), 0);
  QVERIFY(!peer.busy());

  peer.load();
  QTRY_VERIFY(!peer.busy());
  QCOMPARE(peer.entries(), QList<app::FileStagingEntry>{dEntry});
  QVERIFY(peer.storageAvailable() && peer.writable());
  QCOMPARE(changes.count(), 1);
  source->gate->release();
  QTRY_COMPARE(source->gate->finished.load(), 1);
  QCoreApplication::processEvents();
  QCOMPARE(peer.entries(), QList<app::FileStagingEntry>{dEntry});
  QCOMPARE(changes.count(), 1);
  QVERIFY(!source->observedOnOwner());
  QCOMPARE(store->writes, 0);
}

void FileStagingViewModelTests::editingPreparationKeepsTheEventLoopResponsive_data() {
  QTest::addColumn<int>("operation");
  QTest::newRow("add") << 0;
  QTest::newRow("remove") << 1;
  QTest::newRow("restore") << 2;
}

void FileStagingViewModelTests::editingPreparationKeepsTheEventLoopResponsive() {
  QFETCH(int, operation);
  const app::FileStagingEntry original{app::FileStagingKind::File, QStringLiteral("C:\\original.bin")};
  const app::FileStagingEntry retained{app::FileStagingKind::File, QStringLiteral("C:\\retained.bin")};
  const app::FileStagingEntry added{app::FileStagingKind::File, QStringLiteral("C:\\added.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {original, retained};
  auto source = std::make_shared<GatedStagingSource>();
  app::FileStagingDraft draft(store, source);
  auto editing = std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, source);
  presentation::FileStagingViewModel model(editing);
  model.load();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(model.entries().size(), 2);
  if (operation == 2) {
    editing->removePaths({original.path});
    QCOMPARE(model.entries().size(), 1);
  }
  presentation::FileStagingViewModel peer(std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, source));
  peer.load();
  QTRY_VERIFY(!peer.busy());
  const auto before = model.entries();
  QSignalSpy published(&model, &presentation::FileStagingViewModel::entriesChanged);
  QSignalSpy peerPublished(&peer, &presentation::FileStagingViewModel::entriesChanged);
  QSignalSpy changes(&draft, &app::FileStagingDraft::changed);
  const auto release = qScopeGuard([gate = source->gate] { gate->release(); });
  source->blockNextObservation();
  if (operation == 0)
    model.addEntries({added});
  else if (operation == 1)
    model.removePaths({original.path});
  else
    model.restorePaths({original.path});
  QVERIFY(model.busy());
  QVERIFY(peer.busy());
  QVERIFY(!model.writable() && !peer.writable());
  bool heartbeat = false;
  QTimer::singleShot(0, &model, [&heartbeat] { heartbeat = true; });
  QTRY_COMPARE(source->gate->started.load(), 1);
  QTRY_VERIFY(heartbeat);
  QCOMPARE(source->gate->finished.load(), 0);
  QCOMPARE(model.entries(), before);
  QCOMPARE(published.count(), 0);
  source->gate->release();
  QTRY_VERIFY(!model.busy());
  QVERIFY(!peer.busy());
  QVERIFY(model.writable() && peer.writable());
  QVERIFY(!source->observedOnOwner());
  QCOMPARE(changes.count(), 1);
  QCOMPARE(published.count(), 1);
  QCOMPARE(peerPublished.count(), 1);
  QCOMPARE(model.entries(), peer.entries());
  QCOMPARE(model.entries().size(), operation == 0 ? 3 : operation == 1 ? 1 : 2);
  QVERIFY(model.entries().contains(retained));
  QCOMPARE(model.entries().contains(original), operation != 1);
  QCOMPARE(model.entries().contains(added), operation == 0);
  QCOMPARE(store->writes, 0);
}

void FileStagingViewModelTests::completedPreparationCannotReplaceANewerDraft() {
  const app::FileStagingEntry original{app::FileStagingKind::File, QStringLiteral("C:\\original.bin")};
  const app::FileStagingEntry latest{app::FileStagingKind::File, QStringLiteral("C:\\latest.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {original};
  auto source = std::make_shared<GatedStagingSource>();
  app::FileStagingDraft draft(store, source);
  presentation::FileStagingViewModel model(std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, source));
  model.load();
  QTRY_VERIFY(!model.busy());
  QSignalSpy published(&model, &presentation::FileStagingViewModel::entriesChanged);
  QSignalSpy warnings(&model, &presentation::FileStagingViewModel::warningRaised);
  const auto release = qScopeGuard([gate = source->gate] { gate->release(); });
  source->blockNextObservation();
  model.addEntries({{app::FileStagingKind::File, QStringLiteral("C:\\older.bin")}});
  QTRY_COMPARE(source->gate->started.load(), 1);
  draft.replace({latest});
  const auto revision = draft.revision();
  QCOMPARE(model.entries().size(), 1);
  QCOMPARE(model.entries().constFirst().path, latest.path);
  source->gate->release();
  QTRY_VERIFY(!model.busy());
  QCOMPARE(draft.revision(), revision);
  QCOMPARE(model.entries().size(), 1);
  QCOMPARE(model.entries().constFirst().path, latest.path);
  QCOMPARE(published.count(), 1);
  QCOMPARE(warnings.count(), 0);
  QCOMPARE(store->writes, 0);
}

void FileStagingViewModelTests::destroyingTheViewModelReleasesSharedBusyWithoutJoining() {
  const app::FileStagingEntry original{app::FileStagingKind::File, QStringLiteral("C:\\original.bin")};
  auto store = std::make_shared<MemoryStagingStore>();
  store->entries = {original};
  auto source = std::make_shared<GatedStagingSource>();
  app::FileStagingDraft draft(store, source);
  auto model = std::make_unique<presentation::FileStagingViewModel>(std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, source));
  model->load();
  QTRY_VERIFY(!model->busy());
  presentation::FileStagingViewModel peer(std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, source));
  peer.load();
  QTRY_VERIFY(!peer.busy());
  const auto revision = draft.revision();
  QSignalSpy published(&peer, &presentation::FileStagingViewModel::entriesChanged);
  const auto release = qScopeGuard([gate = source->gate] { gate->release(); });
  source->blockNextObservation();
  model->addEntries({{app::FileStagingKind::File, QStringLiteral("C:\\canceled.bin")}});
  QTRY_COMPARE(source->gate->started.load(), 1);
  QVERIFY(peer.busy());
  model.reset();
  QCOMPARE(source->gate->finished.load(), 0);
  QVERIFY(!draft.busy());
  QVERIFY(!peer.busy());
  QVERIFY(peer.writable());
  source->gate->release();
  QTRY_COMPARE(source->gate->finished.load(), 1);
  QCoreApplication::processEvents();
  QCOMPARE(draft.revision(), revision);
  QCOMPARE(peer.entries().size(), 1);
  QCOMPARE(peer.entries().constFirst().path, original.path);
  QCOMPARE(published.count(), 0);
  QCOMPARE(store->writes, 0);
}

}  // namespace

QTEST_GUILESS_MAIN(FileStagingViewModelTests)
#include "FileStagingViewModelTests.moc"
