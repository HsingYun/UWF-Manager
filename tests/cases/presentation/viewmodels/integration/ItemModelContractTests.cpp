// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractItemModelTester>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "application/diagnostics/ProcessLogSource.h"
#include "application/staging/editing/FileStagingEditor.h"
#include "cases/presentation/viewmodels/registry/ControlledRegistryBrowser.h"
#include "cases/presentation/viewmodels/registry/RegistryPickerTestActions.h"
#include "cases/presentation/viewmodels/staging/MemoryStagingStore.h"
#include "presentation/viewmodels/commit/CommitReportViewModel.h"
#include "presentation/viewmodels/configuration/ExclusionListViewModel.h"
#include "presentation/viewmodels/configuration/ImportViewModel.h"
#include "presentation/viewmodels/diagnostics/LogViewModel.h"
#include "presentation/viewmodels/overlay/OverlayFilesViewModel.h"
#include "presentation/viewmodels/registry/RegistryPickerViewModel.h"
#include "presentation/viewmodels/staging/FileStagingViewModel.h"
#include "support/app/FileTargetFixture.h"
#include "support/ui/OverlayFilesFixture.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

enum class PublicationModel { Exclusions, Staging, RegistryValues, Imports, Logs, OverlayFiles };

class MemoryEditing final : public app::FileStagingEditing {
 public:
  const QString& driveLetter() const override { return m_drive; }
  const QList<app::FileStagingEntry>& entries() const override { return m_entries; }
  QList<app::FileStagingEntry> baselineEntries() const override { return {}; }
  bool storageAvailable() const override { return true; }
  const QString& storageError() const override { return m_error; }
  void load() override { emit entriesChanged(); }
  app::FileStagingPreparation prepareAddEntries(const QList<app::FileStagingEntry>& entries) override {
    return {{},
            [this, entries](std::exception_ptr) {
              m_entries.append(entries);
              emit entriesChanged();
            },
            {}};
  }
  app::FileStagingPreparation prepareRemovePaths(const QStringList&) override { return {{}, [](std::exception_ptr) {}, {}}; }
  app::FileStagingPreparation prepareRestorePaths(const QStringList&) override { return {{}, [](std::exception_ptr) {}, {}}; }

 private:
  QString m_drive = QStringLiteral("C:"), m_error;
  QList<app::FileStagingEntry> m_entries;
};

class MemoryLogs final : public app::LogSource {
 public:
  std::atomic_int value{0};
  std::vector<std::string> recentLines() const override { return {std::to_string(value.load())}; }
  void clear() override {}
};

struct PublicationHarness {
  std::unique_ptr<QAbstractItemModel> model;
  std::function<void(int)> publish;
  std::function<bool()> busy = [] { return false; };
  std::function<bool(const QModelIndex&)> representsInput = [](const QModelIndex&) { return true; };
  bool appends = false;
  bool asynchronous = false;
  bool inserts = false;

  QStringList entries() const {
    QStringList result;
    for (int row = 0; row < model->rowCount(); ++row) {
      const auto first = model->index(row, 0);
      if (!representsInput(first)) continue;
      if (inserts && first.data(presentation::SeparatorRole).toBool()) continue;
      const auto path = first.data(presentation::PathRole);
      // Single-digit inputs identify records independently of presentation order.
      const auto text = inserts ? model->index(row, 2).data().toString() : path.isValid() ? path.toString() : model->index(row, 3).data().toString();
      result.append(text.right(1));
    }
    result.sort();
    return result;
  }

  void observe(const bool after, QObject* context, std::function<void()> action) const {
    if (inserts) {
      if (after)
        QObject::connect(model.get(), &QAbstractItemModel::rowsInserted, context, [action](const QModelIndex&, int, int) { action(); });
      else
        QObject::connect(model.get(), &QAbstractItemModel::rowsAboutToBeInserted, context, [action](const QModelIndex&, int, int) { action(); });
    } else if (after)
      QObject::connect(model.get(), &QAbstractItemModel::modelReset, context, action);
    else
      QObject::connect(model.get(), &QAbstractItemModel::modelAboutToBeReset, context, action);
  }
};

PublicationHarness publicationHarness(const PublicationModel kind) {
  PublicationHarness result;
  switch (kind) {
    case PublicationModel::Exclusions: {
      auto model = std::make_unique<presentation::ExclusionListViewModel>(app::ExclusionEditor::Kind::File);
      model->setDriveLetter(QStringLiteral("C:"));
      result.publish = [target = model.get()](const int value) { target->importAdd(QStringLiteral("C:\\entry%1").arg(value)); };
      result.appends = true;
      result.model = std::move(model);
      break;
    }
    case PublicationModel::Staging: {
      auto model = std::make_unique<presentation::FileStagingViewModel>(std::make_shared<MemoryEditing>());
      result.publish = [target = model.get()](const int value) {
        target->addEntries({{app::FileStagingKind::File, QStringLiteral("C:\\entry%1").arg(value)}});
      };
      result.appends = true;
      result.model = std::move(model);
      break;
    }
    case PublicationModel::RegistryValues: {
      auto model = std::make_unique<presentation::RegistryValueModel>(presentation::RegistryValueModel::Selection::AllValues);
      result.publish = [target = model.get()](const int value) { target->setValues({{std::to_string(value), 1u, std::vector<std::uint8_t>{}}}); };
      result.representsInput = [](const QModelIndex& index) { return index.flags().testFlag(Qt::ItemIsSelectable); };
      result.model = std::move(model);
      break;
    }
    case PublicationModel::Imports: {
      auto model = std::make_unique<presentation::ImportViewModel>();
      result.publish = [target = model.get()](const int value) {
        target->appendReport({{app::ImportResult::Status::Success, value, QString::number(value), {}}});
      };
      result.appends = result.inserts = true;
      result.model = std::move(model);
      break;
    }
    case PublicationModel::Logs: {
      const auto source = std::make_shared<MemoryLogs>();
      auto model = std::make_unique<presentation::LogViewModel>(source);
      result.publish = [target = model.get(), source](const int value) {
        source->value = value;
        target->reload();
      };
      result.busy = [target = model.get()] { return target->loading(); };
      result.asynchronous = true;
      result.model = std::move(model);
      break;
    }
    case PublicationModel::OverlayFiles: {
      const auto value = std::make_shared<std::atomic_int>(0);
      auto model = std::make_unique<presentation::OverlayFilesViewModel>(
          QStringLiteral("C:"), uwf::test::overlayFilesVolumeGuid(QStringLiteral("C:")),
          uwf::test::overlayFilesQueryManager([value](const QString&, std::stop_token) {
            return QVector<app::OverlayFileEntry>{{QStringLiteral("\\entry%1").arg(value->load()), {}, false, false, 1}};
          }));
      result.publish = [target = model.get(), value](const int next) {
        *value = next;
        target->startLoading();
      };
      result.busy = [target = model.get()] { return target->loading(); };
      result.asynchronous = true;
      result.model = std::move(model);
      break;
    }
  }
  return result;
}

void publicationBoundaries() {
  QTest::addColumn<int>("kind");
  QTest::addColumn<bool>("after");
  const std::pair<const char*, PublicationModel> kinds[]{
      {"exclusions", PublicationModel::Exclusions}, {"staging", PublicationModel::Staging}, {"registry-values", PublicationModel::RegistryValues},
      {"imports", PublicationModel::Imports},       {"logs", PublicationModel::Logs},       {"overlay-files", PublicationModel::OverlayFiles}};
  for (const auto& [name, kind] : kinds)
    for (const bool after : {false, true})
      QTest::newRow(qPrintable(QString::fromLatin1(name) + (after ? QStringLiteral("-published") : QStringLiteral("-about-to-publish"))))
          << static_cast<int>(kind) << after;
}

class ItemModelContractTests final : public QObject {
  Q_OBJECT
 private slots:
  void itemModelsHonorTheQtContractAcrossUpdates();
  void nestedPublicationsRemainSerial_data() { publicationBoundaries(); }
  void nestedPublicationsRemainSerial();
  void destructionAtPublicationBoundariesStopsFurtherAccess_data() { publicationBoundaries(); }
  void destructionAtPublicationBoundariesStopsFurtherAccess();
};

void ItemModelContractTests::nestedPublicationsRemainSerial() {
  QFETCH(int, kind);
  QFETCH(bool, after);
  auto harness = publicationHarness(static_cast<PublicationModel>(kind));
  QAbstractItemModelTester contract(harness.model.get(), QAbstractItemModelTester::FailureReportingMode::QtTest);
  int depth = 0, maximumDepth = 0;
  harness.observe(false, this, [&] { maximumDepth = std::max(maximumDepth, ++depth); });
  harness.observe(true, this, [&] { --depth; });
  bool nested = false;
  harness.observe(after, this, [&] {
    if (std::exchange(nested, true)) return;
    harness.publish(2);
    // Simulate a modal observer: a completed worker must defer publication until the active reset closes.
    if (harness.asynchronous) QVERIFY(QTest::qWaitFor([&] { return !harness.busy(); }));
  });
  harness.publish(1);
  QTRY_VERIFY(!harness.busy());
  const QStringList expected = harness.appends ? QStringList{QStringLiteral("1"), QStringLiteral("2")} : QStringList{QStringLiteral("2")};
  QTRY_COMPARE(harness.entries(), expected);
  QCOMPARE(maximumDepth, 1);
  QCOMPARE(depth, 0);
}

void ItemModelContractTests::destructionAtPublicationBoundariesStopsFurtherAccess() {
  QFETCH(int, kind);
  QFETCH(bool, after);
  auto harness = publicationHarness(static_cast<PublicationModel>(kind));
  const QPointer alive(harness.model.get());
  int insertionsFinished = 0;
  connect(harness.model.get(), &QAbstractItemModel::rowsInserted, this, [&] { ++insertionsFinished; });
  harness.observe(after, this, [&] {
    if (harness.inserts && !after) {
      // beginInsertRows still accesses the model after rowsAboutToBeInserted; defer deletion until insertion closes.
      harness.model.release()->deleteLater();
    } else {
      harness.model.reset();
    }
  });
  harness.publish(1);
  if (harness.inserts && !after) QCOMPARE(insertionsFinished, 1);
  QTRY_VERIFY(!alive);
}

void ItemModelContractTests::itemModelsHonorTheQtContractAcrossUpdates() {
  using Tester = QAbstractItemModelTester;
  using Mode = Tester::FailureReportingMode;
  presentation::ExclusionListViewModel exclusions(presentation::ExclusionListViewModel::Kind::File);
  Tester exclusionContract(&exclusions, Mode::QtTest);
  exclusions.setDriveLetter(QStringLiteral("C:"));
  exclusions.setBaseline({QStringLiteral("C:\\Alpha")}, {QStringLiteral("C:\\Alpha"), QStringLiteral("C:\\Beta")});
  QCOMPARE(exclusions.rowCount(), 2);
  QPersistentModelIndex old = exclusions.index(0, 0);
  QCOMPARE(exclusions.importAdd(QStringLiteral("C:\\Gamma")), app::ExclusionImportOutcome::Applied);
  QVERIFY(!old.isValid());
  exclusions.resetPending();
  auto storeOwner = std::make_shared<MemoryStagingStore>();
  auto& store = *storeOwner;
  app::FileStagingDraft draft(storeOwner);
  auto editor = std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, std::make_shared<test::application::StoredPathSource>());
  presentation::FileStagingViewModel staging(editor);
  Tester stagingContract(&staging, Mode::QtTest);
  store.entries = {{app::FileStagingKind::Directory, QStringLiteral("C:\\Cache")}};
  staging.load();
  QTRY_VERIFY(!staging.busy());
  QCOMPARE(staging.rowCount(), 1);
  presentation::ImportViewModel imports;
  Tester importContract(&imports, Mode::QtTest);
  imports.appendReport({{app::ImportResult::Status::Success, 1, QStringLiteral("filter enable"), QStringLiteral("Queued")}});
  imports.appendReport({{app::ImportResult::Status::Duplicate, 2, QStringLiteral("filter enable"), QStringLiteral("Unchanged")}});
  QCOMPARE(imports.rowCount(), 3);
  QVERIFY(imports.index(1, 0).data(presentation::SeparatorRole).toBool());
  QList<presentation::CommitReportRow> rows;
  rows.append({uwf::presentation::CommitRowStatus::Succeeded, QStringLiteral("C:\\file.txt"), QStringLiteral("-"), QStringLiteral("-"), std::nullopt,
               std::nullopt, QStringLiteral("file.txt")});
  presentation::CommitReportViewModel report(rows, app::CommitOperation::Persist);
  Tester reportContract(&report, Mode::QtTest);
  presentation::OverlayFilesViewModel overlay(QStringLiteral("C:"), uwf::test::overlayFilesVolumeGuid(QStringLiteral("C:")),
                                              uwf::test::overlayFilesQueryManager([](const QString&, std::stop_token) {
                                                return QVector<app::OverlayFileEntry>{{QStringLiteral("\\file.txt"), {}, false, false, 123}};
                                              }));
  Tester overlayContract(&overlay, Mode::QtTest);
  overlay.startLoading();
  QTRY_VERIFY(overlay.loaded());
  const auto browser = std::make_shared<ControlledRegistryBrowser>();
  presentation::RegistryPickerViewModel registry(presentation::RegistryPickerViewModel::Mode::CommitValue, browser);
  QVERIFY(test::awaitRegistryRoots(registry));
  const auto root = registry.index(0, 0);
  QVERIFY(registry.canFetchMore(root));
  Tester registryContract(&registry, Mode::QtTest);
  registryContract.setUseFetchMore(false);
  Tester valueContract(&registry.valueModel(), Mode::QtTest);
  QVERIFY(test::loadRegistryChildren(registry, root));
  QCOMPARE(registry.rowCount(root), 2);
  QCOMPARE(registry.parent(registry.index(0, 0, root)), root);
  QVERIFY(!registry.canFetchMore(root));
  test::loadRegistryValues(registry, QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE"));
  QCOMPARE(registry.valueModel().rowCount(), 2);
  registry.resetTree();
  presentation::LogViewModel log(std::make_shared<app::ProcessLogSource>());
  Tester logContract(&log, Mode::QtTest);
  QSignalSpy loaded(&log, &presentation::LogViewModel::loadingFinished);
  log.reload();
  QTRY_COMPARE(loaded.size(), 1);
}

}  // namespace

QTEST_GUILESS_MAIN(ItemModelContractTests)
#include "ItemModelContractTests.moc"
