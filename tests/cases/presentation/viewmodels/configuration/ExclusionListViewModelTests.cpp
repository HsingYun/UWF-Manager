// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QDir>
#include <QFile>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <QThreadPool>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include "application/configuration/exclusions/ExclusionEditor.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/configuration/planning/ConfigurationScript.h"
#include "application/staging/contracts/FileStagingStore.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "application/volumes/contracts/VolumeRepository.h"
#include "cases/presentation/viewmodels/common/ViewModelStateFixture.h"
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/viewmodels/configuration/ExclusionListViewModel.h"
#include "support/app/ReadOnlyFileBrowserFixture.h"
#include "support/app/WorkerGate.h"
#include "support/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

enum class FileTargetRegistration { Absent, CurrentOnly, Next };
enum class FileAdditionSource { Selected, Imported, RestoreCurrent };
enum class FileProtectionDraft { Unchanged, Protect, WithdrawProtect };
class ObservedFileBrowser final : public app::FileBrowser {
 public:
  explicit ObservedFileBrowser(QThread* receiverThread) : receiverThread(receiverThread) {}
  QThread* receiverThread;
  mutable std::atomic_int inspections{0};
  mutable std::atomic_int receiverInspections{0};
  std::atomic_bool directory{true};
  std::atomic_bool exists{true};
  std::atomic_bool fail{false};
  std::shared_ptr<test::WorkerGate> firstRead;
  app::FileBrowserEntry inspect(const QString& path) const override {
    const int call = ++inspections;
    if (QThread::currentThread() == receiverThread) ++receiverInspections;
    const bool observedDirectory = directory.load();
    const bool observedExists = exists.load();
    if (call == 1 && firstRead) firstRead->wait({}, false);
    if (fail.load()) throw std::runtime_error("Metadata read failed");
    return {.path = path, .name = {}, .directory = observedDirectory, .size = 0, .modified = {}, .exists = observedExists};
  }
  QVector<app::FileBrowserEntry> list(const QString&, std::stop_token) const override { throw std::logic_error("Unexpected enumeration"); }
  QVector<app::FileBrowserPlace> places() const override { throw std::logic_error("Unexpected places read"); }
  void createDirectory(const QString&) const override { throw std::logic_error("Unexpected write"); }
};

class ExclusionListViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void unconfirmedMetadataCannotEnableCommit_data();
  void unconfirmedMetadataCannotEnableCommit();
  void persistenceMenuTracksDraftAndReadOnlyState();
  void exclusionKindsResolveAgainstTheSelectedVolume();
  void fileMetadataReadsAreAsyncAndReusedAcrossDraftEdits();
  void fileMetadataFromAnOldBaselineCannotReplaceTheCurrentObservation();
  void configurationImportsPublishOneExclusionSnapshotPerTarget();
  void registryProtectionRefreshesWhenStagingChanges_data();
  void registryProtectionRefreshesWhenStagingChanges();
  void failedProbeProtectsTheWholeSelection();
  void exclusionCaseAndReversalSemanticsRemainStable();
  void fileAddressAliasesShareOneRowAndBothSessionStates();
  void mixedDraftPartitionsPreserveRepresentativeSpellingsAndSessionStates();
  void stagingRegistryTooltipExplainsTheDependencyWithoutChangingRemovalRules();
  void configuredExclusionSpellingsKeepSeparateSessionStates_data();
  void configuredExclusionSpellingsKeepSeparateSessionStates();
  void fileExclusionsRequireTheFinalNextRegistration_data();
  void fileExclusionsRequireTheFinalNextRegistration();
  void fileExclusionImportMaintainsCaseInsensitiveDeltas();
  void fileExclusionImportRejectsWrongVolumeAndUwfBlacklist();
  void registryExclusionImportsFollowTheEditingPolicy();
};

void ExclusionListViewModelTests::fileAddressAliasesShareOneRowAndBothSessionStates() {
  const std::string volume = "Volume{11111111-2222-3333-4444-555555555555}";
  const QString drive = QStringLiteral("Z:\\Data\\state.bin");
  const QString guid = QString::fromStdString(compat::volumePath(volume)) + QStringLiteral("Data\\state.bin");
  auto editor = std::make_shared<app::ExclusionEditor>(app::ExclusionEditor::Kind::File);
  editor->setDriveLetter(QStringLiteral("Z:"));
  editor->setFileVolumeIdentity(volume);
  presentation::ExclusionListViewModel model(editor);
  editor->setBaseline({guid}, {drive, guid});
  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(model.data(model.index(0), presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Normal));
  QCOMPARE(model.data(model.index(0), presentation::BadgeRole).toInt(), static_cast<int>(presentation::ItemBadge::None));
  QVERIFY(editor->removeSelection({{drive, std::nullopt}}));
  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(editor->pendingRemoved().size(), 2);
  QCOMPARE(model.data(model.index(0), presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Removed));
  QVERIFY(editor->restoreSelection({{guid, std::nullopt}}));
  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(model.data(model.index(0), presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Normal));
}

void ExclusionListViewModelTests::mixedDraftPartitionsPreserveRepresentativeSpellingsAndSessionStates() {
  const std::string volume = "Volume{11111111-2222-3333-4444-555555555555}";
  const QString root = QString::fromStdString(compat::volumePath(volume)) + QStringLiteral("Fixture\\");
  auto state = std::make_shared<app::ExclusionDraft>();
  auto editor = std::make_shared<app::ExclusionEditor>(app::ExclusionEditor::Kind::File, state);
  editor->setDriveLetter(QStringLiteral("Z:"));
  editor->setBaseline({root + QStringLiteral("Shared"), QStringLiteral("Z:\\Fixture\\CurrentOnly"), QStringLiteral("Z:\\Fixture\\Added")},
                      {QStringLiteral("z:\\fixture\\SHARED"), QStringLiteral("Z:\\Fixture\\NextOnly"), QStringLiteral("Z:\\Fixture\\Removed"),
                       root + QStringLiteral("REMOVED")});
  // Before volume binding these addresses are independent. Binding merges
  // their row identity without rewriting either captured source spelling.
  QCOMPARE(editor->importRemove(root + QStringLiteral("REMOVED")), app::ExclusionImportOutcome::Applied);
  editor->setFileVolumeIdentity(volume);
  QCOMPARE(editor->importAdd(root + QStringLiteral("ADDED")), app::ExclusionImportOutcome::Applied);
  QCOMPARE(editor->importAdd(QStringLiteral("Z:\\Fixture\\New")), app::ExclusionImportOutcome::Applied);
  presentation::ExclusionListViewModel model(editor);
  struct ExpectedRow {
    QString path;
    bool current;
    bool next;
    bool added;
    bool removed;
  };
  const std::array expected{
      ExpectedRow{root + QStringLiteral("Shared"), true, true, false, false},
      ExpectedRow{QStringLiteral("Z:\\Fixture\\CurrentOnly"), true, false, false, false},
      ExpectedRow{root + QStringLiteral("ADDED"), true, false, true, false},
      ExpectedRow{QStringLiteral("Z:\\Fixture\\NextOnly"), false, true, false, false},
      ExpectedRow{root + QStringLiteral("REMOVED"), false, true, false, true},
      ExpectedRow{QStringLiteral("Z:\\Fixture\\New"), false, false, true, false},
  };
  QCOMPARE(model.rowCount(), static_cast<int>(expected.size()));
  int previousRank = -1;
  for (int row = 0; row < model.rowCount(); ++row) {
    const auto index = model.index(row);
    const auto path = index.data(presentation::PathRole).toString();
    const auto found = std::ranges::find(expected, path, &ExpectedRow::path);
    QVERIFY(found != expected.end());
    const bool removed = found->removed || (!found->added && found->current && !found->next);
    const bool added = found->added || (!found->removed && !found->current && found->next);
    const auto status = removed ? presentation::ItemStatus::Removed : added ? presentation::ItemStatus::Added : presentation::ItemStatus::Normal;
    QCOMPARE(index.data(presentation::StatusRole).toInt(), static_cast<int>(status));
    const int rank = removed ? 0 : added ? 1 : 2;
    QVERIFY(rank >= previousRank);
    previousRank = rank;
    const auto tooltip = index.data(Qt::ToolTipRole).toString();
    QVERIFY(tooltip.contains(QStringLiteral("Current session: %1\nNext session: %2")
                                 .arg(found->current ? QStringLiteral("Excluded") : QStringLiteral("Not excluded"),
                                      found->next ? QStringLiteral("Excluded") : QStringLiteral("Not excluded"))));
    QCOMPARE(tooltip.contains(QStringLiteral("Pending change:")), found->added || found->removed);
  }
  editor->resetPending();
  const QSet<QString> remaining{root + QStringLiteral("Shared"), QStringLiteral("Z:\\Fixture\\CurrentOnly"), QStringLiteral("Z:\\Fixture\\Added"),
                                QStringLiteral("Z:\\Fixture\\NextOnly"), root + QStringLiteral("REMOVED")};
  QCOMPARE(model.rowCount(), static_cast<int>(remaining.size()));
  for (int row = 0; row < model.rowCount(); ++row) {
    const auto index = model.index(row);
    QVERIFY(remaining.contains(index.data(presentation::PathRole).toString()));
    QVERIFY(!index.data(Qt::ToolTipRole).toString().contains(QStringLiteral("Pending change:")));
  }
}

void ExclusionListViewModelTests::unconfirmedMetadataCannotEnableCommit_data() {
  QTest::addColumn<bool>("fail");
  QTest::newRow("missing") << false;
  QTest::newRow("failed") << true;
}
void ExclusionListViewModelTests::unconfirmedMetadataCannotEnableCommit() {
  QFETCH(bool, fail);
  auto browser = std::make_shared<ObservedFileBrowser>(QThread::currentThread());
  browser->exists = false;
  browser->fail = fail;
  auto editor = std::make_shared<app::ExclusionEditor>(app::ExclusionEditor::Kind::File, nullptr, browser);
  presentation::ExclusionListViewModel model(editor);
  const QString path = QStringLiteral("Z:\\Data\\extensionless");
  model.setBaseline({}, {path});
  model.setCommitAvailability(true);
  QSignalSpy availability(&model, &presentation::ExclusionListViewModel::commitAvailabilityChanged);
  QVERIFY(model.fileCommitState(path).visible);
  QVERIFY(!model.fileCommitState(path).enabled);
  QTRY_VERIFY(!availability.isEmpty());
  QVERIFY(!model.fileCommitState(path).visible);
  QVERIFY(!model.fileCommitState(path).enabled);
  QCOMPARE(model.index(0).data(presentation::IconResourceRole).toString(), QStringLiteral(":/icons/folder.svg"));
  QCOMPARE(browser->inspections.load(), 1);
  QCOMPARE(browser->receiverInspections.load(), 0);
}
void ExclusionListViewModelTests::persistenceMenuTracksDraftAndReadOnlyState() {
  using Item = app::ExclusionEditor::PersistenceItem;
  presentation::ExclusionListViewModel model(app::ExclusionEditor::Kind::Registry);
  model.setPersistBaseline(false, true, false, false);
  QVERIFY(!model.canEnablePersistence(Item::DomainSecretKey));
  QVERIFY(model.canEnablePersistence(Item::Tscal));
  model.enablePersistence(Item::Tscal);
  QVERIFY(!model.canEnablePersistence(Item::Tscal));
  QVERIFY(model.removeSelection({{QString{}, Item::Tscal}}));
  QVERIFY(model.canEnablePersistence(Item::Tscal));
  model.setReadOnly(true);
  QVERIFY(!model.canEnablePersistence(Item::DomainSecretKey));
  QVERIFY(!model.canEnablePersistence(Item::Tscal));
  presentation::ExclusionListViewModel files(app::ExclusionEditor::Kind::File);
  QVERIFY(!files.canEnablePersistence(Item::Tscal));
}

void ExclusionListViewModelTests::exclusionKindsResolveAgainstTheSelectedVolume() {
  const QString folder = QStringLiteral("Z:\\Data\\folder.with.dot");
  const QString file = QStringLiteral("Z:\\Data\\extensionless");
  auto browser = std::make_shared<test::application::ReadOnlyFileBrowser>();
  browser->entries.emplace(
      folder, app::FileBrowserEntry{.path = folder, .name = QStringLiteral("folder.with.dot"), .directory = true, .size = 0, .modified = {}, .exists = true});
  browser->entries.emplace(
      file, app::FileBrowserEntry{.path = file, .name = QStringLiteral("extensionless"), .directory = false, .size = 0, .modified = {}, .exists = true});
  auto editor = std::make_shared<app::ExclusionEditor>(app::ExclusionEditor::Kind::File, nullptr, browser);
  presentation::ExclusionListViewModel model(std::move(editor));
  model.setDriveLetter(QStringLiteral("Z:"));
  model.setBaseline({folder.mid(2), file.mid(2)}, {folder.mid(2), file.mid(2)});
  QCOMPARE(model.rowCount(), 2);
  QStringList observedPaths;
  for (int row = 0; row < model.rowCount(); ++row) {
    const auto index = model.index(row);
    const auto absolutePath = model.absoluteFilePath(index.data(presentation::PathRole).toString());
    QVERIFY(absolutePath == folder || absolutePath == file);
    observedPaths.append(absolutePath);
    QTRY_COMPARE(index.data(presentation::IconResourceRole).toString(),
                 absolutePath == folder ? QStringLiteral(":/icons/folder.svg") : QStringLiteral(":/icons/file.svg"));
  }
  QVERIFY(observedPaths.contains(folder));
  QVERIFY(observedPaths.contains(file));
  QVERIFY(browser->inspections.contains(folder));
  QVERIFY(browser->inspections.contains(file));
  QVERIFY(std::ranges::all_of(browser->inspections, [&](const auto& path) { return path == folder || path == file; }));
}

void ExclusionListViewModelTests::fileMetadataReadsAreAsyncAndReusedAcrossDraftEdits() {
  auto browser = std::make_shared<ObservedFileBrowser>(QThread::currentThread());
  auto editor = std::make_shared<app::ExclusionEditor>(app::ExclusionEditor::Kind::File, nullptr, browser);
  editor->setDriveLetter(QStringLiteral("Z:"));
  presentation::ExclusionListViewModel model(editor);
  QSignalSpy resets(&model, &QAbstractItemModel::modelReset);
  QSignalSpy pending(editor.get(), &app::ExclusionEditor::pendingChanged);
  constexpr int count = 100;
  {
    const auto batch = editor->beginBatch();
    for (int i = 0; i < count; ++i) QCOMPARE(editor->importAdd(QStringLiteral("Z:\\Data\\folder%1.ext").arg(i)), app::ExclusionImportOutcome::Applied);
    QCOMPARE(resets.size(), 0);
  }
  QCOMPARE(resets.size(), 1);
  QCOMPARE(pending.size(), 1);
  QCOMPARE(model.rowCount(), count);
  QCOMPARE(browser->inspections.load(), 0);
  QTRY_COMPARE(model.index(0).data(presentation::IconResourceRole).toString(), QStringLiteral(":/icons/folder.svg"));
  QCOMPARE(browser->inspections.load(), count);
  QCOMPARE(browser->receiverInspections.load(), 0);
  const auto path = model.index(0).data(presentation::PathRole).toString();
  QCOMPARE(model.importRemove(path), app::ExclusionImportOutcome::Applied);
  QCOMPARE(model.importAdd(path), app::ExclusionImportOutcome::Applied);
  QCoreApplication::processEvents();
  QCOMPARE(browser->inspections.load(), count);

  // Menu admission reuses the confirmed observation. Execution owns revalidation.
  editor->setCommitEnabled(true);
  browser->exists = false;
  QCOMPARE(model.fileCommitState(path).kind, app::ExclusionEditor::FileEntryKind::Directory);
  QVERIFY(model.fileCommitState(path).enabled);
  QCOMPARE(browser->inspections.load(), count);
  QCOMPARE(browser->receiverInspections.load(), 0);
}

void ExclusionListViewModelTests::fileMetadataFromAnOldBaselineCannotReplaceTheCurrentObservation() {
  auto browser = std::make_shared<ObservedFileBrowser>(QThread::currentThread());
  auto gate = browser->firstRead = std::make_shared<test::WorkerGate>();
  const auto unblock = qScopeGuard([gate] { gate->release(); });
  auto editor = std::make_shared<app::ExclusionEditor>(app::ExclusionEditor::Kind::File, nullptr, browser);
  editor->setDriveLetter(QStringLiteral("Z:"));
  presentation::ExclusionListViewModel model(editor);
  const QString path = QStringLiteral("Z:\\Data\\extensionless");
  model.setBaseline({path}, {path});
  QTRY_COMPARE(gate->started.load(), 1);
  browser->directory = false;
  model.setBaseline({path}, {path});
  QTRY_COMPARE(model.index(0).data(presentation::IconResourceRole).toString(), QStringLiteral(":/icons/file.svg"));
  gate->release();
  QVERIFY(QThreadPool::globalInstance()->waitForDone(2000));
  QCoreApplication::processEvents();
  QCOMPARE(model.index(0).data(presentation::IconResourceRole).toString(), QStringLiteral(":/icons/file.svg"));
  QCOMPARE(browser->inspections.load(), 2);
  QCOMPARE(browser->receiverInspections.load(), 0);
}

void ExclusionListViewModelTests::configurationImportsPublishOneExclusionSnapshotPerTarget() {
  presentation::ConfigurationViewModel configuration(0);
  auto volume = configuration.addVolume(disk(), true);
  configuration.resetSnapshot(snapshot());
  QSignalSpy fileResets(volume->files().get(), &QAbstractItemModel::modelReset);
  QSignalSpy registryResets(volume->registry().get(), &QAbstractItemModel::modelReset);
  QString text;
  constexpr int count = 100;
  for (int i = 0; i < count; ++i)
    text += QStringLiteral("uwfmgr file add-exclusion C:\\Data\\file%1.txt\nuwfmgr registry add-exclusion HKLM\\SOFTWARE\\Batch\\Key%1\n").arg(i);
  QList<app::UwfmgrCommand> commands;
  for (const auto& command : app::parseUwfmgrText(text.toStdString())) commands.append(command);
  const auto report = configuration.importCommands(commands);
  QCOMPARE(report.size(), count * 2);
  QVERIFY(std::ranges::all_of(report, [](const auto& row) { return row.status == app::ImportResult::Status::Success; }));
  QCOMPARE(fileResets.size(), 1);
  QCOMPARE(registryResets.size(), 1);
  QCOMPARE(volume->files()->rowCount(), count);
  QCOMPARE(volume->registry()->rowCount(), count);
}

void ExclusionListViewModelTests::registryProtectionRefreshesWhenStagingChanges_data() {
  QTest::addColumn<QStringList>("configured");
  const auto full = QString::fromStdString(std::string(app::kFileStagingRegistryRoot));
  const auto shortKey = QStringLiteral("HKLM") + full.mid(QStringLiteral("HKEY_LOCAL_MACHINE").size());
  QTest::newRow("long-hive") << QStringList{full};
  QTest::newRow("short-hive") << QStringList{shortKey};
  QTest::newRow("both-hives") << QStringList{full, shortKey};
}

void ExclusionListViewModelTests::registryProtectionRefreshesWhenStagingChanges() {
  QFETCH(QStringList, configured);
  presentation::ConfigurationViewModel model(16384);
  auto state = snapshot();
  state.current.registryExclusions.clear();
  for (const auto& key : configured) state.current.registryExclusions.push_back(key.toStdString());
  state.next.registryExclusions = state.current.registryExclusions;
  auto volume = model.addVolume(disk(), true);
  model.resetSnapshot(state);
  bool savedEntries = false;
  volume->registry()->setFileStagingDataProbe([&] { return savedEntries; });
  for (const auto& key : configured) QCOMPARE(volume->registry()->importRemove(key), app::ExclusionImportOutcome::Applied);
  QVERIFY(!model.pendingChanges().removeRegistryExclusions.empty());
  savedEntries = true;
  model.refreshStagingConstraints();
  QVERIFY(model.pendingChanges().removeRegistryExclusions.empty());
  QVERIFY(volume->registry()->draft().removed().isEmpty());
}

void ExclusionListViewModelTests::failedProbeProtectsTheWholeSelection() {
  presentation::ExclusionListViewModel model(presentation::ExclusionListViewModel::Kind::Registry);
  const QString root = QString::fromStdString(std::string(app::kFileStagingRegistryRoot));
  const QString other = QStringLiteral("HKLM\\SOFTWARE\\Vendor");
  model.setBaseline({root, other}, {root, other});
  model.setFileStagingDataProbe([]() -> bool { throw std::runtime_error("mock storage failure"); });
  const QVector<presentation::ExclusionListViewModel::Selection> selection{{root, std::nullopt}, {other, std::nullopt}};
  QVERIFY(!model.removeSelection(selection));
  QVERIFY(model.pendingRemoved().isEmpty());
}

void ExclusionListViewModelTests::exclusionCaseAndReversalSemanticsRemainStable() {
  presentation::ExclusionListViewModel model(presentation::ExclusionListViewModel::Kind::Registry);
  model.setBaseline({}, {QStringLiteral("HKLM\\SOFTWARE\\Vendor")});
  QCOMPARE(model.importRemove(QStringLiteral("hklm\\software\\vendor")), app::ExclusionImportOutcome::Applied);
  QCOMPARE(model.pendingRemoved().size(), 1);
  QCOMPARE(model.importAdd(QStringLiteral("HKLM\\SOFTWARE\\VENDOR")), app::ExclusionImportOutcome::Applied);
  QVERIFY(model.pendingRemoved().isEmpty());
  QVERIFY(model.pendingAdded().isEmpty());
  const QString path = model.draft().next().constFirst();
  model.setPersistBaseline(true, true, true, true);
  const QVector<presentation::ExclusionListViewModel::Selection> selected{{path, std::nullopt},
                                                                          {QString(), presentation::ExclusionListViewModel::PersistenceItem::DomainSecretKey},
                                                                          {QString(), presentation::ExclusionListViewModel::PersistenceItem::Tscal}};
  QVERIFY(model.removeSelection(selected));
  QCOMPARE(model.pendingRemoved().size(), 1);
  QCOMPARE(model.pendingPersistDomainSecretKey(), std::optional<bool>(false));
  QCOMPARE(model.pendingPersistTSCAL(), std::optional<bool>(false));
  model.setReadOnly(true);
  QVERIFY(!model.restoreSelection(selected));
  QCOMPARE(model.pendingRemoved().size(), 1);
  model.setReadOnly(false);
  QVERIFY(model.restoreSelection(selected));
  QVERIFY(model.pendingRemoved().isEmpty());
  QVERIFY(!model.pendingPersistDomainSecretKey().has_value());
  QVERIFY(!model.pendingPersistTSCAL().has_value());
}

void ExclusionListViewModelTests::stagingRegistryTooltipExplainsTheDependencyWithoutChangingRemovalRules() {
  presentation::ExclusionListViewModel model(app::ExclusionEditor::Kind::Registry);
  const QString root = QStringLiteral("hklm\\software\\hsingyun\\uwf manager");
  const QString sibling = root + QStringLiteral(" Backup");
  model.setBaseline({}, {root, sibling});
  const QString dependency = QStringLiteral("File staging relies on this registry exclusion to preserve its list across restarts.");
  bool required = true;
  model.setFileStagingDataProbe([&] { return required; });
  for (int row = 0; row < model.rowCount(); ++row) {
    const auto index = model.index(row, 0);
    const bool stagingRoot = index.data(presentation::PathRole).toString().endsWith(QStringLiteral("uwf manager"));
    QCOMPARE(index.data(presentation::RequiredRole).toBool(), stagingRoot);
    QCOMPARE(index.data(Qt::ToolTipRole).toString().contains(dependency), stagingRoot);
    QCOMPARE(index.data(presentation::BadgeRole).toInt(), static_cast<int>(presentation::ItemBadge::Add));
  }
  QVERIFY(!model.canRemoveSelection({root}));
  QVERIFY(model.canRemoveSelection({sibling}));
  required = false;
  model.editor()->refreshFileStagingConstraints();
  QVERIFY(model.canRemoveSelection({root}));
  QCOMPARE(model.importRemove(root), app::ExclusionImportOutcome::Applied);
}

void ExclusionListViewModelTests::configuredExclusionSpellingsKeepSeparateSessionStates_data() {
  QTest::addColumn<bool>("registry");
  QTest::addColumn<QString>("current");
  QTest::addColumn<QString>("next");
  for (const bool registry : {false, true}) {
    const auto root = registry ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Identity\\") : QStringLiteral("Z:\\Data\\");
    QTest::newRow(registry ? "registry-non-ascii" : "file-non-ascii") << registry << root + QChar(0x00c4) << root + QChar(0x00e4);
  }
  QTest::newRow("registry-hive-aliases") << true << QStringLiteral("HKLM\\SOFTWARE\\Identity") << QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Identity");
  QTest::newRow("file-trailing-separator") << false << QStringLiteral("Z:\\Data\\Identity") << QStringLiteral("Z:\\Data\\Identity\\");
}

void ExclusionListViewModelTests::configuredExclusionSpellingsKeepSeparateSessionStates() {
  QFETCH(bool, registry);
  QFETCH(QString, current);
  QFETCH(QString, next);
  using Model = presentation::ExclusionListViewModel;
  Model model(registry ? Model::Kind::Registry : Model::Kind::File);
  model.setDriveLetter(QStringLiteral("Z:"));
  model.setBaseline({current}, {next});
  QCOMPARE(model.rowCount(), 2);
  for (int row = 0; row < model.rowCount(); ++row) {
    const auto item = model.index(row, 0);
    const auto path = item.data(presentation::PathRole).toString();
    QVERIFY(path == current || path == next);
    QCOMPARE(item.data(presentation::BadgeRole).toInt(), static_cast<int>(path == current ? presentation::ItemBadge::Remove : presentation::ItemBadge::Add));
  }
  QCOMPARE(model.importAdd(current), Model::ImportOutcome::Applied);
  QCOMPARE(model.importRemove(next), Model::ImportOutcome::Applied);
  QCOMPARE(model.pendingAdded(), QStringList{current});
  QCOMPARE(model.pendingRemoved(), QStringList{next});
  QCOMPARE(model.rowCount(), 2);
}

void ExclusionListViewModelTests::fileExclusionsRequireTheFinalNextRegistration_data() {
  QTest::addColumn<FileTargetRegistration>("registration");
  QTest::addColumn<FileAdditionSource>("source");
  QTest::addColumn<FileProtectionDraft>("protectionDraft");
  for (const auto [registration, name] : {std::pair{FileTargetRegistration::Absent, "absent"}, std::pair{FileTargetRegistration::CurrentOnly, "current-only"},
                                          std::pair{FileTargetRegistration::Next, "next"}})
    for (const auto [source, sourceName] : {std::pair{FileAdditionSource::Selected, "selected"}, std::pair{FileAdditionSource::Imported, "imported"},
                                            std::pair{FileAdditionSource::RestoreCurrent, "restore-current"}}) {
      if (source == FileAdditionSource::RestoreCurrent && registration != FileTargetRegistration::CurrentOnly) continue;
      for (const auto [draft, draftName] : {std::pair{FileProtectionDraft::Unchanged, "unchanged"}, std::pair{FileProtectionDraft::Protect, "protect"},
                                            std::pair{FileProtectionDraft::WithdrawProtect, "withdraw-protect"}})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2/%3").arg(name, sourceName, draftName))) << registration << source << draft;
    }
}

void ExclusionListViewModelTests::fileExclusionsRequireTheFinalNextRegistration() {
  QFETCH(FileTargetRegistration, registration);
  QFETCH(FileAdditionSource, source);
  QFETCH(FileProtectionDraft, protectionDraft);
  constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";
  const auto system = std::make_shared<test::FakeUwfBackend>();
  const auto path = source == FileAdditionSource::RestoreCurrent ? QStringLiteral("C:\\Legacy\\data.bin") : QStringLiteral("C:\\Data\\fixture.bin");
  if (registration != FileTargetRegistration::Absent) {
    (void)compat::call(*system, compat::AddVolume{volumeId, "C:"});
    (void)compat::call(*system, compat::SetVolume{volumeId, compat::VolumeProperty::Protected, true});
    (void)compat::call(*system, compat::EditFileExclusion{volumeId, "\\Legacy\\data.bin", compat::ExclusionAction::Add});
    (void)compat::call(*system, compat::SetBoolean{compat::BooleanProperty::FilterEnabled, true});
    system->simulateRestart();
    if (registration == FileTargetRegistration::CurrentOnly)
      (void)compat::call(*system, compat::RemoveVolume{volumeId});
    else
      (void)compat::call(*system, compat::SetVolume{volumeId, compat::VolumeProperty::Protected, false});
  }
  auto physical = disk();
  physical.volumeName = "Volume{" + std::string(volumeId) + '}';
  auto published = system->snapshot();
  published.runtime.physicalVolumes[volumeId] = {physical.physical.fixedMedia.value_or(false), core::canRegisterVolume(physical)};
  published.runtime.fileMaintenancePrefixes[volumeId] = physical.driveLetter;
  system->seed(std::move(published));
  const auto before = system->snapshot();
  const auto baseline = app::publishedUwfSnapshot(system->state(), app::UwfCapability::Available, true, {});

  presentation::ConfigurationViewModel configuration(0);
  const auto volume = configuration.addVolume(physical, false);
  configuration.resetSnapshot(baseline);
  const auto files = volume->files();
  const bool registered = registration == FileTargetRegistration::Next;
  QCOMPARE(files->canAdd(), registered);
  if (!registered) QCOMPARE(files->additionHint(), QStringLiteral("Enable volume protection in this draft before adding file exclusions."));
  if (source == FileAdditionSource::RestoreCurrent) {
    QCOMPARE(files->rowCount(), 1);
    QVERIFY(!files->canRestoreSelection({{path, std::nullopt}}));
  }
  if (protectionDraft != FileProtectionDraft::Unchanged) QVERIFY(volume->status()->editProtection(true));
  const bool addAllowed = registered || protectionDraft != FileProtectionDraft::Unchanged;
  QCOMPARE(files->canAdd(), addAllowed);
  if (source == FileAdditionSource::Selected) {
    const auto result = files->addSelectedPath(path);
    QCOMPARE(result.outcome, addAllowed ? app::ExclusionImportOutcome::Applied : app::ExclusionImportOutcome::RejectedMissingVolume);
    if (!addAllowed) QCOMPARE(result.reason, files->additionHint());
  } else if (source == FileAdditionSource::Imported) {
    const auto text = std::string("uwfmgr.exe file add-exclusion ") + path.toStdString();
    QList<app::UwfmgrCommand> commands;
    for (const auto& command : app::parseUwfmgrText(text)) commands.append(command);
    const auto report = configuration.importCommands(commands);
    QCOMPARE(report.size(), 1);
    QCOMPARE(report.front().status, addAllowed ? app::ImportResult::Status::Success : app::ImportResult::Status::Failed);
    if (!addAllowed) QCOMPARE(report.front().detail, files->additionHint());
  } else {
    QCOMPARE(files->canRestoreSelection({{path, std::nullopt}}), addAllowed);
    QCOMPARE(files->restoreSelection({{path, std::nullopt}}), addAllowed);
  }
  QCOMPARE(files->pendingAdded(), addAllowed ? QStringList{path} : QStringList{});
  if (protectionDraft == FileProtectionDraft::WithdrawProtect) QVERIFY(volume->status()->editProtection(false));
  const bool targetAvailable = registered || protectionDraft == FileProtectionDraft::Protect;
  QCOMPARE(files->canAdd(), targetAvailable);
  QCOMPARE(files->pendingAdded(), addAllowed ? QStringList{path} : QStringList{});  // Preserve invalid user intent for correction.
  const auto plan = app::planConfiguration(configuration.pendingChanges(), baseline);
  const auto repository = app::uwfConfigurationRepository(system);
  const auto actions = std::make_shared<app::ConfigurationExecutor>(repository);
  presentation::ApplyPlanViewModel review(plan, actions, [](const QString&, const QString&) { return app::TextFileWriteResult{}; });
  system->clearTrace();
  const bool conflict = addAllowed && !targetAvailable;
  QCOMPARE(review.canApply(), !conflict && plan.count() != 0);
  QCOMPARE(review.canExport(), !conflict && plan.count() != 0);
  QCOMPARE(review.canExport(presentation::ApplyPlanViewModel::ScriptScope::NextConfiguration), !conflict);
  if (conflict) {
    QCOMPARE(review.warning(), QStringLiteral("Enable volume protection in this draft before adding file exclusions."));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(actions->prepareApply(plan)));
    const auto direct = repository->prepareWrite(app::ConfigurationSettingsPhase{.settings = plan.changes}, plan);
    const auto skipped = direct({});
    QCOMPARE(skipped.dependencySkippedOperations(), 1);
    QCOMPARE(skipped.succeededOperations(), 0);
    QCOMPARE(skipped.failedOperations(), 0);
    QCOMPARE(skipped.writeEffect(), app::WriteEffect::None);
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(app::configurationChangeScript(plan)));
    QVERIFY(system->trace().empty());
    QVERIFY(system->snapshot() == before);

    return;
  }
  const auto result = actions->apply(plan);
  QCOMPARE(result.failedOperations(), 0);

  for (const auto& entry : system->trace()) QVERIFY(compat::isWrite(entry.request));
  auto expected = before;
  if (protectionDraft == FileProtectionDraft::Protect) {
    if (!registered)
      expected.next.volumes.emplace(
          volumeId, test::FakeUwfBackend::VolumeConfiguration{.volume = {volumeId, "C:", true, true, std::nullopt}, .fileExclusions = {}, .swapfileSize = 0});
    else
      expected.next.volumes.at(volumeId).volume.protectedVolume = true;
  }
  if (addAllowed) expected.next.volumes.at(volumeId).fileExclusions.push_back(app::toUwfFilePath(path.toStdString(), "C:"));
  QVERIFY(system->snapshot() == expected);
  QVERIFY(system->snapshot().current == before.current);
  if (addAllowed && !registered) {
    const auto script = app::configurationChangeScript(plan);
    auto commands = app::parseUwfmgrText(app::renderConfigurationScript(script));
    std::erase_if(commands, [](const auto& command) { return command.parseError == app::ParseError::Comment; });
    QCOMPARE(commands.size(), std::size_t{2});
    QCOMPARE(commands[0].parseError, app::ParseError::None);
    QCOMPARE(commands[0].kind, app::UwfmgrKind::VolumeProtect);
    QCOMPARE(commands[0].args, std::vector<std::string>{"C:"});
    QVERIFY(!commands[0].requiresPrevious);
    QCOMPARE(commands[1].parseError, app::ParseError::None);
    QCOMPARE(commands[1].kind, app::UwfmgrKind::FileAddExclusion);
    QCOMPARE(commands[1].args, std::vector<std::string>{path.toStdString()});
    QVERIFY(commands[1].requiresPrevious);
  }
}

void ExclusionListViewModelTests::fileExclusionImportMaintainsCaseInsensitiveDeltas() {
  using Outcome = app::ExclusionImportOutcome;
  presentation::ExclusionListViewModel list(app::ExclusionEditor::Kind::File);
  list.setDriveLetter(QStringLiteral("Z:"));
  list.setBaseline({QStringLiteral("Z:\\Current")}, {QStringLiteral("Z:\\Keep"), QStringLiteral("Z:\\Remove")});

  QCOMPARE(list.importAdd(QStringLiteral("z:\\New Folder")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("z:\\New Folder")}));
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\NEW FOLDER")), Outcome::NoOp);
  QCOMPARE(list.importRemove(QStringLiteral("Z:\\new folder")), Outcome::Applied);
  QVERIFY(list.pendingAdded().isEmpty());

  QCOMPARE(list.importRemove(QStringLiteral("z:\\remove")), Outcome::Applied);
  // Lookup is case-insensitive; the operation retains the observed source
  // spelling so grouping an address alias never replaces the stored pattern.
  QCOMPARE(list.pendingRemoved(), QStringList({QStringLiteral("Z:\\Remove")}));
  QCOMPARE(list.importRemove(QStringLiteral("Z:\\REMOVE")), Outcome::NoOp);
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\Remove")), Outcome::Applied);
  QVERIFY(list.pendingRemoved().isEmpty());

  list.resetPending();
  QVERIFY(list.pendingAdded().isEmpty());
  QVERIFY(list.pendingRemoved().isEmpty());
  // A Current-only exclusion still needs an explicit Next addition.
  QCOMPARE(list.importAdd(QStringLiteral("z:\\current")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("z:\\current")}));
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\CURRENT")), Outcome::NoOp);
  QCOMPARE(list.importRemove(QStringLiteral("Z:\\Current")), Outcome::Applied);
  QVERIFY(list.pendingAdded().isEmpty());
}

void ExclusionListViewModelTests::fileExclusionImportRejectsWrongVolumeAndUwfBlacklist() {
  using Outcome = app::ExclusionImportOutcome;
  presentation::ExclusionListViewModel list(app::ExclusionEditor::Kind::File);
  list.setDriveLetter(QStringLiteral("Z:"));
  list.setBaseline({}, {});

  QCOMPARE(list.importAdd(QStringLiteral("Y:\\Data")), Outcome::RejectedNotOnVolume);
  QCOMPARE(list.importAdd(QStringLiteral("Z:")), Outcome::RejectedForbidden);
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\pagefile.sys")), Outcome::RejectedForbidden);
  QCOMPARE(list.importAdd(QStringLiteral("Z:\\Data\\Cache")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("Z:\\Data\\Cache")}));
}

void ExclusionListViewModelTests::registryExclusionImportsFollowTheEditingPolicy() {
  using Outcome = app::ExclusionImportOutcome;
  auto editor = std::make_shared<uwf::app::ExclusionEditor>(uwf::app::ExclusionEditor::Kind::Registry);
  presentation::ExclusionListViewModel list(editor);
  const QString stagingRoot = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager");
  list.setBaseline({}, {QStringLiteral("HKLM\\SYSTEM\\Existing"), stagingRoot});

  QCOMPARE(list.importAdd(QStringLiteral("hklm\\software\\Vendor\\Product")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("hklm\\software\\Vendor\\Product")}));
  QCOMPARE(list.importAdd(QStringLiteral("HKLM\\SOFTWARE\\VENDOR\\PRODUCT")), Outcome::NoOp);
  QCOMPARE(list.importAdd(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\VENDOR\\PRODUCT")), Outcome::Applied);
  QCOMPARE(list.pendingAdded().size(), 2);
  QCOMPARE(list.importAdd(QStringLiteral("HKCU\\Software\\Vendor")), Outcome::RejectedForbidden);
  QCOMPARE(list.importAdd(QStringLiteral("HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC")), Outcome::RejectedForbidden);

  QCOMPARE(list.importRemove(QStringLiteral("hklm\\system\\existing")), Outcome::Applied);
  QCOMPARE(list.pendingRemoved(), QStringList({QStringLiteral("hklm\\system\\existing")}));

  bool stagingHasData = true;
  list.setFileStagingDataProbe([&stagingHasData] { return stagingHasData; });
  QCOMPARE(list.importRemove(stagingRoot), Outcome::RejectedRequired);
  QCOMPARE(list.pendingRemoved(), QStringList({QStringLiteral("hklm\\system\\existing")}));

  // Staging data cancels the pending dependency removal; clearing staging does not restore that intent.
  stagingHasData = false;
  QCOMPARE(list.importRemove(stagingRoot), Outcome::Applied);
  QVERIFY(list.pendingRemoved().contains(stagingRoot, Qt::CaseInsensitive));
  stagingHasData = true;
  editor->refreshFileStagingConstraints();
  QVERIFY(!list.pendingRemoved().contains(stagingRoot, Qt::CaseInsensitive));
  stagingHasData = false;
  QVERIFY(!list.pendingRemoved().contains(stagingRoot, Qt::CaseInsensitive));
  list.setBaseline({QStringLiteral("HKLM\\SOFTWARE\\Current")}, {});
  QCOMPARE(list.importAdd(QStringLiteral("hklm\\software\\current")), Outcome::Applied);
  QCOMPARE(list.pendingAdded(), QStringList({QStringLiteral("hklm\\software\\current")}));
}

}  // namespace

QTEST_GUILESS_MAIN(ExclusionListViewModelTests)
#include "ExclusionListViewModelTests.moc"
