// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QTranslator>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>

#include "application/operations/contracts/OperationError.h"
#include "presentation/viewmodels/overlay/OverlayFilesViewModel.h"
#include "support/app/WorkerGate.h"
#include "support/ui/OverlayFilesFixture.h"

namespace {
using namespace uwf;

class OverlayFailureTranslator final : public QTranslator {
 public:
  bool isEmpty() const override { return false; }
  QString translate(const char* context, const char* source, const char*, int) const override {
    return context && source && qstrcmp(context, "uwf") == 0 ? QStringLiteral("localized: ") + QString::fromUtf8(source) : QString{};
  }
};

class OverlayFilesViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void overlayFileModelNormalizesStreamsAndDetachesFromItsQuery();
  void overlayFilesCancellationRemainsSilent();
  void terminalSignalsPublishOneSnapshot_data();
  void terminalSignalsPublishOneSnapshot();
  void retainedFailuresFollowTheCurrentLanguage_data();
  void retainedFailuresFollowTheCurrentLanguage();
};

void OverlayFilesViewModelTests::overlayFileModelNormalizesStreamsAndDetachesFromItsQuery() {
  presentation::OverlayFilesViewModel model(
      QStringLiteral("C:"), uwf::test::overlayFilesVolumeGuid(QStringLiteral("C:")), uwf::test::overlayFilesQueryManager([](const QString&, std::stop_token) {
        return QVector<app::OverlayFileEntry>{
            {QStringLiteral("\\dir\\one.txt:stream"), {}, false, false, 8}, {QStringLiteral("\\dir\\ONE.txt"), {}, false, false, 16},
            {QStringLiteral("\\$Mft:stream"), {}, false, false, 32},        {QStringLiteral("\\$Extend:$INDEX_ALLOCATION"), {}, false, false, 64},
            {QStringLiteral("\\$Work\\file.txt"), {}, false, false, 4},     {QStringLiteral("\\Docs\\$Budget.txt"), {}, false, false, 4}};
      }));
  QSignalSpy loaded(&model, &presentation::OverlayFilesViewModel::entriesLoaded);
  model.startLoading();
  QTRY_COMPARE(loaded.count(), 1);
  QCOMPARE(model.entries().size(), 5);
  QVERIFY(model.entries().front().isSystemMetadata);
  const auto merged = std::ranges::find_if(model.entries(), [](const auto& entry) { return entry.absolutePath == QStringLiteral("C:\\dir\\one.txt"); });
  QVERIFY(merged != model.entries().end());
  QCOMPARE(merged->fileSize, qulonglong{24});
  model.setCommitAvailability(true);
  QSignalSpy requested(&model, &presentation::OverlayFilesViewModel::commitFileRequested);
  QVERIFY(!model.canCommit(QStringLiteral("C:\\$Mft")));
  QVERIFY(!model.canCommit(QStringLiteral("C:\\$Extend")));
  model.requestCommit(QStringLiteral("C:\\$Mft"));
  QCOMPARE(requested.size(), 0);
  for (const auto& path : {QStringLiteral("C:\\$Work\\file.txt"), QStringLiteral("C:\\Docs\\$Budget.txt")}) {
    QVERIFY(model.canCommit(path));
    model.requestCommit(path);
    QCOMPARE(requested.back().front().toString(), QStringLiteral("\\\\?\\Volume{11111111-2222-3333-4444-000000000043}") + path.mid(2));
  }
  QCOMPARE(requested.size(), 2);
  const auto exported = model.exportContent().split(u'\n');
  for (const auto& entry : model.entries()) {
    const auto line = std::ranges::find_if(exported, [&](const QString& row) { return row.endsWith(u'\t' + entry.absolutePath); });
    QVERIFY(line != exported.end());
    const auto expected = entry.isSystemMetadata ? QStringLiteral("metadata") : entry.isDirectory ? QStringLiteral("dir") : QStringLiteral("file");
    QCOMPARE(line->section(u'\t', 0, 0), expected);
  }
  auto gate = std::make_shared<test::WorkerGate>();
  const auto unblock = qScopeGuard([gate] { gate->release(); });
  const auto queries = test::overlayFilesQueryManager([gate](const QString&, const std::stop_token token) {
    gate->wait(token);
    return QVector<app::OverlayFileEntry>{};
  });
  {
    auto canceling = std::make_unique<presentation::OverlayFilesViewModel>(QStringLiteral("C:"), test::overlayFilesVolumeGuid(QStringLiteral("C:")), queries);
    canceling->startLoading();
    QTRY_COMPARE(gate->started.load(), 1);
  }
  QCOMPARE(gate->canceled.load(), 0);
  QCOMPARE(gate->finished.load(), 0);
  gate->release();
  QTRY_COMPARE(gate->finished.load(), 1);
  QCOMPARE(gate->canceled.load(), 0);
}

void OverlayFilesViewModelTests::terminalSignalsPublishOneSnapshot_data() {
  QTest::addColumn<bool>("emptyReload");
  QTest::newRow("replace-with-current-files") << false;
  QTest::newRow("replace-with-empty-catalog") << true;
}

void OverlayFilesViewModelTests::terminalSignalsPublishOneSnapshot() {
  QFETCH(bool, emptyReload);
  std::atomic<int> reads{0};
  presentation::OverlayFilesViewModel model(
      QStringLiteral("C:"), uwf::test::overlayFilesVolumeGuid(QStringLiteral("C:")), uwf::test::overlayFilesQueryManager([&](const QString&, std::stop_token) {
        if (++reads == 1) return QVector<app::OverlayFileEntry>{{QStringLiteral("\\old.txt"), {}, false, false, 1}};
        return emptyReload ? QVector<app::OverlayFileEntry>{} : QVector<app::OverlayFileEntry>{{QStringLiteral("\\current.txt"), {}, false, false, 2}};
      }));
  model.setCommitAvailability(true);
  int generation = 0;
  int observations = 0;
  const auto inspectSnapshot = [&] {
    ++observations;
    const bool empty = generation == 1 && emptyReload;
    QCOMPARE(model.loading(), false);
    QVERIFY(model.loaded());
    QCOMPARE(model.loadState(), empty ? presentation::ContentLoadState::Empty : presentation::ContentLoadState::Ready);
    QCOMPARE(model.rowCount(), empty ? 0 : 1);
    QCOMPARE(model.entries().size(), empty ? 0 : 1);
    if (!empty) {
      const auto expected = generation == 0 ? QStringLiteral("C:\\old.txt") : QStringLiteral("C:\\current.txt");
      QCOMPARE(model.entries().front().absolutePath, expected);
      QVERIFY(model.canCommit(expected));
    }
  };
  connect(&model, &QAbstractItemModel::modelReset, &model, inspectSnapshot);
  connect(&model, &presentation::OverlayFilesViewModel::loadingStopped, &model, inspectSnapshot);
  connect(&model, &presentation::OverlayFilesViewModel::entriesLoaded, &model, inspectSnapshot);
  QSignalSpy loaded(&model, &presentation::OverlayFilesViewModel::entriesLoaded);
  for (generation = 0; generation != 2; ++generation) {
    model.startLoading();
    QTRY_COMPARE(loaded.size(), generation + 1);
    QCOMPARE(observations, (generation + 1) * 3);
  }
}

void OverlayFilesViewModelTests::overlayFilesCancellationRemainsSilent() {
  auto gate = std::make_shared<test::WorkerGate>();
  const auto unblock = qScopeGuard([gate] { gate->release(); });
  presentation::OverlayFilesViewModel canceled(QStringLiteral("C:"), uwf::test::overlayFilesVolumeGuid(QStringLiteral("C:")),
                                               uwf::test::overlayFilesQueryManager([gate](const QString&, std::stop_token) -> QVector<app::OverlayFileEntry> {
                                                 gate->wait({}, false);
                                                 throw app::OperationError(std::make_error_code(std::errc::operation_canceled), "enumerate overlay files");
                                               }));
  QSignalSpy stopped(&canceled, &presentation::OverlayFilesViewModel::loadingStopped);
  QSignalSpy failed(&canceled, &presentation::OverlayFilesViewModel::loadFailed);
  QSignalSpy loaded(&canceled, &presentation::OverlayFilesViewModel::entriesLoaded);
  canceled.startLoading();
  QTRY_COMPARE(gate->started.load(), 1);
  QVERIFY(stopped.isEmpty());
  QVERIFY(failed.isEmpty());
  QVERIFY(loaded.isEmpty());
  QVERIFY(!canceled.loaded());
  gate->release();
  QTRY_COMPARE_WITH_TIMEOUT(stopped.count(), 1, 2000);
  QCOMPARE(canceled.loadState(), presentation::ContentLoadState::Canceled);
  QVERIFY(failed.isEmpty());
  QVERIFY(loaded.isEmpty());

  presentation::OverlayFilesViewModel rejected(QStringLiteral("C:"), uwf::test::overlayFilesVolumeGuid(QStringLiteral("C:")),
                                               uwf::test::overlayFilesQueryManager([](const QString&, std::stop_token) -> QVector<app::OverlayFileEntry> {
                                                 throw std::runtime_error("enumeration rejected");
                                               }));
  QSignalSpy rejection(&rejected, &presentation::OverlayFilesViewModel::loadFailed);
  rejected.startLoading();
  QTRY_COMPARE(rejection.count(), 1);
  QVERIFY(rejection.front().front().toString().contains(QStringLiteral("enumeration rejected")));
}

void OverlayFilesViewModelTests::retainedFailuresFollowTheCurrentLanguage_data() {
  QTest::addColumn<int>("errorCode");
  QTest::addColumn<bool>("unexpected");
  QTest::addColumn<QString>("hint");
  QTest::newRow("provider-detail") << 0 << false << QString{};
  QTest::newRow("unavailable") << int(std::errc::no_such_device) << false << QStringLiteral("The UWF service is unavailable while enumerating overlay files.");
  const auto resources = QStringLiteral(
      "Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.");
  QTest::newRow("unsupported") << int(std::errc::operation_not_supported) << false << resources;
  QTest::newRow("resource-exhausted") << int(std::errc::not_enough_memory) << false << resources;
  QTest::newRow("unexpected") << 0 << true << QString{};
}

void OverlayFilesViewModelTests::retainedFailuresFollowTheCurrentLanguage() {
  QFETCH(int, errorCode);
  QFETCH(bool, unexpected);
  QFETCH(QString, hint);
  const auto providerDetail = QStringLiteral("Provider diagnostic\n0x80010105\nPreserve this raw detail.");
  const auto failure = unexpected ? std::make_exception_ptr(17)
                       : errorCode
                           ? std::make_exception_ptr(app::OperationError(std::make_error_code(static_cast<std::errc>(errorCode)), providerDetail.toStdString()))
                           : std::make_exception_ptr(std::runtime_error(providerDetail.toStdString()));
  QString raw;
  if (!unexpected) {
    try {
      std::rethrow_exception(failure);
    } catch (const std::exception& error) {
      raw = QString::fromUtf8(error.what());
    }
  }
  std::atomic_int reads{0};
  presentation::OverlayFilesViewModel model(QStringLiteral("C:"), uwf::test::overlayFilesVolumeGuid(QStringLiteral("C:")),
                                            uwf::test::overlayFilesQueryManager([&](const QString&, std::stop_token) {
                                              if (++reads == 1) std::rethrow_exception(failure);
                                              return QVector<app::OverlayFileEntry>{};
                                            }));
  QSignalSpy failed(&model, &presentation::OverlayFilesViewModel::loadFailed);
  model.startLoading();
  QTRY_COMPARE(failed.size(), 1);
  const auto sourceError = unexpected ? QStringLiteral("Unexpected error while enumerating overlay files.") : raw;
  const auto sourceFormat = hint.isEmpty() ? QStringLiteral("Failed: %1") : QStringLiteral("Raw error: %1");
  const auto sourceDetail = sourceFormat.arg(sourceError);
  const auto sourceSummary = hint.isEmpty() ? sourceDetail : hint + QStringLiteral("\n\n") + sourceDetail;
  QCOMPARE(model.errorHint(), hint);
  QCOMPARE(model.errorDetail(), sourceDetail);
  QCOMPARE(model.errorText(), sourceSummary);
  QCOMPARE(failed.front().front().toString(), sourceSummary);

  OverlayFailureTranslator translator;
  QVERIFY(QCoreApplication::installTranslator(&translator));
  const auto restore = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });
  model.retranslate();
  const auto localizedHint = hint.isEmpty() ? QString{} : QStringLiteral("localized: ") + hint;
  const auto localizedError = unexpected ? QStringLiteral("localized: ") + sourceError : raw;
  const auto localizedDetail = (QStringLiteral("localized: ") + sourceFormat).arg(localizedError);
  QCOMPARE(model.errorHint(), localizedHint);
  QCOMPARE(model.errorDetail(), localizedDetail);
  QCOMPARE(model.errorText(), localizedHint.isEmpty() ? localizedDetail : localizedHint + QStringLiteral("\n\n") + localizedDetail);
  QCOMPARE(model.loadState(), presentation::ContentLoadState::Failed);
  QVERIFY(!model.loading());
  QCOMPARE(reads.load(), 1);
  QCOMPARE(failed.size(), 1);

  QCoreApplication::removeTranslator(&translator);
  model.retranslate();
  QCOMPARE(model.errorText(), sourceSummary);
  model.startLoading();
  QVERIFY(model.errorText().isEmpty());
  QVERIFY(model.errorHint().isEmpty());
  QVERIFY(model.errorDetail().isEmpty());
  QTRY_COMPARE(model.loadState(), presentation::ContentLoadState::Empty);
  QCOMPARE(reads.load(), 2);
  QCOMPARE(failed.size(), 1);
}

}  // namespace

QTEST_GUILESS_MAIN(OverlayFilesViewModelTests)
#include "OverlayFilesViewModelTests.moc"
