// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractItemModelTester>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QPointer>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <QThreadPool>
#include <atomic>
#include <functional>
#include <stdexcept>

#include "presentation/common/ItemRoles.h"
#include "presentation/diagnostics/LogViewModel.h"
#include "presentation/overlay/OverlayFilesViewModel.h"
#include "support/app/WorkerGate.h"

namespace {
using namespace uwf;

class ControlledLogSource final : public app::LogSource {
 public:
  std::function<std::vector<std::string>()> read;
  int clears = 0;
  std::function<void()> clearOperation;
  std::vector<std::string> recentLines() const override { return read(); }
  void clear() override {
    ++clears;
    if (clearOperation) clearOperation();
  }
};

class AsyncModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void overlayReloadDiscardsSupersededResults_data();
  void overlayReloadDiscardsSupersededResults();
  void logReloadDiscardsSupersededResults_data();
  void logReloadDiscardsSupersededResults();
  void reloadAndDestructionNeverJoinUncooperativeWorkers_data();
  void reloadAndDestructionNeverJoinUncooperativeWorkers();
  void logFailureRetainsTheLastCompleteSnapshot();
  void logLoadingNotificationMayRequestANewerSnapshot();
  void logDestructionDiscardsPendingDelivery();
  void backgroundResultsFollowModelThreadAffinity_data();
  void backgroundResultsFollowModelThreadAffinity();
  void cancellationDiscardsBufferedResults_data();
  void cancellationDiscardsBufferedResults();
  void completionMayStartANewerRequest_data();
  void completionMayStartANewerRequest();
  void logFilteringRetainsRawRecordsAndCompleteDetails();
  void logClearFailureRetainsTheLastCompleteSnapshot();
};

void AsyncModelTests::overlayReloadDiscardsSupersededResults_data() {
  QTest::addColumn<bool>("oldFailure");
  QTest::newRow("old-success") << false;
  QTest::newRow("old-failure") << true;
}
void AsyncModelTests::overlayReloadDiscardsSupersededResults() {
  QFETCH(bool, oldFailure);
  std::atomic<int> reads{0};
  presentation::OverlayFilesViewModel model(QStringLiteral("C:"), [&](const QString&, std::stop_token) {
    const int request = ++reads;
    if (request == 1 && oldFailure) throw std::runtime_error("superseded failure");
    return QVector<app::OverlayFileEntry>{{request == 1 ? QStringLiteral("\\old.txt") : QStringLiteral("\\current.txt"), {}, false, false, 8}};
  });
  QAbstractItemModelTester contract(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
  QSignalSpy loaded(&model, &presentation::OverlayFilesViewModel::entriesLoaded);
  QSignalSpy failed(&model, &presentation::OverlayFilesViewModel::loadFailed);
  QSignalSpy stopped(&model, &presentation::OverlayFilesViewModel::loadingStopped);
  QElapsedTimer elapsed;
  elapsed.start();
  model.startLoading();
  QTRY_COMPARE(reads.load(), 1);
  model.startLoading();
  QTRY_COMPARE(reads.load(), 2);
  QTRY_VERIFY(!loaded.isEmpty());
  QCOMPARE(loaded.count(), 1);
  QCOMPARE(stopped.count(), 1);
  QVERIFY(failed.isEmpty());
  QCOMPARE(model.entries().front().absolutePath, QStringLiteral("C:\\current.txt"));
  QVERIFY(elapsed.elapsed() >= 1000);
}

void AsyncModelTests::logReloadDiscardsSupersededResults_data() { overlayReloadDiscardsSupersededResults_data(); }
void AsyncModelTests::logReloadDiscardsSupersededResults() {
  QFETCH(bool, oldFailure);
  std::atomic<int> reads{0};
  auto source = std::make_shared<ControlledLogSource>();
  source->read = [&] {
    const int request = ++reads;
    if (request == 1 && oldFailure) throw std::runtime_error("superseded failure");
    return std::vector<std::string>{request == 1 ? "old" : "current"};
  };
  presentation::LogViewModel model(source);
  QAbstractItemModelTester contract(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
  QSignalSpy loaded(&model, &presentation::LogViewModel::loadingFinished);
  QSignalSpy failed(&model, &presentation::LogViewModel::loadFailed);
  QElapsedTimer elapsed;
  elapsed.start();
  model.reload();
  QTRY_COMPARE(reads.load(), 1);
  model.reload();
  QTRY_COMPARE(reads.load(), 2);
  QTRY_COMPARE(loaded.count(), 1);
  QVERIFY(failed.isEmpty());
  QCOMPARE(model.entries().front()[3], QStringLiteral("current"));
  QVERIFY(elapsed.elapsed() >= 1000);
}

void AsyncModelTests::reloadAndDestructionNeverJoinUncooperativeWorkers_data() { backgroundResultsFollowModelThreadAffinity_data(); }
void AsyncModelTests::reloadAndDestructionNeverJoinUncooperativeWorkers() {
  QFETCH(bool, overlay);
  auto gate = std::make_shared<test::WorkerGate>();
  const auto unblock = qScopeGuard([&] {
    gate->release();
    QThreadPool::globalInstance()->waitForDone(2000);
  });
  const auto block = [gate] { gate->wait({}, false); };
  std::unique_ptr<QAbstractItemModel> model;
  std::function<void()> load;
  if (overlay) {
    auto files = std::make_unique<presentation::OverlayFilesViewModel>(QStringLiteral("C:"), [block](const QString&, std::stop_token) {
      block();
      return QVector<app::OverlayFileEntry>{};
    });
    load = [target = files.get()] { target->startLoading(); };
    model = std::move(files);
  } else {
    auto source = std::make_shared<ControlledLogSource>();
    source->read = [block] {
      block();
      return std::vector<std::string>{};
    };
    auto logs = std::make_unique<presentation::LogViewModel>(source);
    load = [target = logs.get()] { target->reload(); };
    model = std::move(logs);
  }
  load();
  QTRY_COMPARE(gate->started.load(), 1);
  QElapsedTimer elapsed;
  elapsed.start();
  load();
  QVERIFY(elapsed.elapsed() < 100);
  QTRY_COMPARE(gate->started.load(), 2);
  elapsed.restart();
  model.reset();
  QVERIFY(elapsed.elapsed() < 100);
}

void AsyncModelTests::logFailureRetainsTheLastCompleteSnapshot() {
  auto source = std::make_shared<ControlledLogSource>();
  source->read = [] { return std::vector<std::string>{"[12:00 I tag] message"}; };
  presentation::LogViewModel model(source);
  QSignalSpy loaded(&model, &presentation::LogViewModel::loadingFinished);
  QSignalSpy failed(&model, &presentation::LogViewModel::loadFailed);
  bool wrongThread = false;
  connect(&model, &QAbstractItemModel::modelReset, &model, [&] { wrongThread |= QThread::currentThread() != model.thread(); }, Qt::DirectConnection);
  model.reload();
  QTRY_COMPARE(loaded.count(), 1);
  QCOMPARE(model.rowCount(), 1);
  const auto complete = model.allRowsToText();
  source->read = []() -> std::vector<std::string> { throw std::runtime_error("read rejected"); };
  model.reload();
  QTRY_COMPARE(failed.count(), 1);
  QCOMPARE(loaded.count(), 1);
  QCOMPARE(model.allRowsToText(), complete);
  source->read = [] { return std::vector<std::string>{}; };
  model.clear();
  QTRY_COMPARE(loaded.count(), 2);
  QCOMPARE(source->clears, 1);
  QCOMPARE(model.rowCount(), 0);
  QVERIFY(!wrongThread);
}

void AsyncModelTests::logDestructionDiscardsPendingDelivery() {
  auto source = std::make_shared<ControlledLogSource>();
  source->read = [] { return std::vector<std::string>{"pending"}; };
  int delivered = 0;
  auto model = std::make_unique<presentation::LogViewModel>(source);
  connect(model.get(), &presentation::LogViewModel::loadingFinished, this, [&] { ++delivered; });
  model->reload();
  QVERIFY(QThreadPool::globalInstance()->waitForDone(2000));
  model.reset();
  QCoreApplication::sendPostedEvents();
  QCOMPARE(delivered, 0);
  QVERIFY(!model);
}

void AsyncModelTests::logLoadingNotificationMayRequestANewerSnapshot() {
  auto source = std::make_shared<ControlledLogSource>();
  source->read = [] { return std::vector<std::string>{"current"}; };
  presentation::LogViewModel model(source);
  bool restarted = false;
  connect(&model, &presentation::LogViewModel::loadingStarted, &model, [&] {
    if (restarted) return;
    restarted = true;
    model.reload();
  });
  QSignalSpy loaded(&model, &presentation::LogViewModel::loadingFinished);
  model.reload();
  QTRY_COMPARE(loaded.count(), 1);
  QCOMPARE(model.entries().front()[3], QStringLiteral("current"));
}

void AsyncModelTests::backgroundResultsFollowModelThreadAffinity_data() {
  QTest::addColumn<bool>("overlay");
  QTest::newRow("overlay-files") << true;
  QTest::newRow("log") << false;
}

void AsyncModelTests::backgroundResultsFollowModelThreadAffinity() {
  QFETCH(bool, overlay);
  QThread owner;
  owner.start();
  std::unique_ptr<QAbstractItemModel> model;
  std::function<void()> load;
  std::atomic<int> delivered{0};
  std::atomic<bool> wrongThread{false};
  if (overlay) {
    auto files = std::make_unique<presentation::OverlayFilesViewModel>(QStringLiteral("C:"), [](const QString&, std::stop_token) {
      return QVector<app::OverlayFileEntry>{{QStringLiteral("\\current.txt"), {}, false, false, 8}};
    });
    load = [target = files.get()] { target->startLoading(); };
    connect(
        files.get(), &presentation::OverlayFilesViewModel::entriesLoaded, this,
        [&] {
          wrongThread = QThread::currentThread() != &owner;
          ++delivered;
        },
        Qt::DirectConnection);
    model = std::move(files);
  } else {
    auto source = std::make_shared<ControlledLogSource>();
    source->read = [] { return std::vector<std::string>{"current"}; };
    auto logs = std::make_unique<presentation::LogViewModel>(source);
    load = [target = logs.get()] { target->reload(); };
    connect(
        logs.get(), &presentation::LogViewModel::loadingFinished, this,
        [&] {
          wrongThread = QThread::currentThread() != &owner;
          ++delivered;
        },
        Qt::DirectConnection);
    model = std::move(logs);
  }
  model->moveToThread(&owner);
  const auto cleanup = qScopeGuard([&] {
    QMetaObject::invokeMethod(model.get(), [&] { model.reset(); }, Qt::BlockingQueuedConnection);
    owner.quit();
    owner.wait();
  });
  QVERIFY(QMetaObject::invokeMethod(model.get(), load, Qt::QueuedConnection));
  QTRY_COMPARE(delivered.load(), 1);
  QVERIFY(!wrongThread.load());
}

void AsyncModelTests::cancellationDiscardsBufferedResults_data() { backgroundResultsFollowModelThreadAffinity_data(); }
void AsyncModelTests::cancellationDiscardsBufferedResults() {
  QFETCH(bool, overlay);
  std::atomic<int> reads{0};
  int delivered = 0;
  std::unique_ptr<QAbstractItemModel> model;
  std::function<void()> load;
  std::function<void()> cancel;
  std::function<bool()> loading;
  if (overlay) {
    auto files = std::make_unique<presentation::OverlayFilesViewModel>(QStringLiteral("C:"), [&](const QString&, std::stop_token) {
      ++reads;
      return QVector<app::OverlayFileEntry>{{QStringLiteral("\\current.txt"), {}, false, false, 8}};
    });
    load = [target = files.get()] { target->startLoading(); };
    cancel = [target = files.get()] { target->cancelLoading(); };
    loading = [target = files.get()] { return target->loading(); };
    connect(files.get(), &presentation::OverlayFilesViewModel::entriesLoaded, this, [&] { ++delivered; });
    model = std::move(files);
  } else {
    auto source = std::make_shared<ControlledLogSource>();
    source->read = [&] {
      ++reads;
      return std::vector<std::string>{"current"};
    };
    auto logs = std::make_unique<presentation::LogViewModel>(source);
    load = [target = logs.get()] { target->reload(); };
    cancel = [target = logs.get()] { target->cancelLoading(); };
    loading = [target = logs.get()] { return target->loading(); };
    connect(logs.get(), &presentation::LogViewModel::loadingFinished, this, [&] { ++delivered; });
    model = std::move(logs);
  }
  load();
  QVERIFY(loading());
  QTRY_COMPARE(reads.load(), 1);
  cancel();
  QVERIFY(!loading());
  QTest::qWait(1100);
  QCOMPARE(delivered, 0);
  QCOMPARE(model->rowCount(), 0);
  load();
  QTRY_COMPARE(delivered, 1);
  QVERIFY(!loading());
  QCOMPARE(model->rowCount(), 1);
}

void AsyncModelTests::completionMayStartANewerRequest_data() { backgroundResultsFollowModelThreadAffinity_data(); }
void AsyncModelTests::completionMayStartANewerRequest() {
  QFETCH(bool, overlay);
  std::atomic<int> reads{0};
  int delivered = 0;
  bool restarted = false;
  bool completedWhileLoading = false;
  std::unique_ptr<QAbstractItemModel> model;
  std::function<void()> load;
  if (overlay) {
    auto files = std::make_unique<presentation::OverlayFilesViewModel>(QStringLiteral("C:"), [&](const QString&, std::stop_token) {
      const int request = ++reads;
      return QVector<app::OverlayFileEntry>{{request == 1 ? QStringLiteral("\\old.txt") : QStringLiteral("\\current.txt"), {}, false, false, 8}};
    });
    load = [target = files.get()] { target->startLoading(); };
    connect(files.get(), &presentation::OverlayFilesViewModel::loadingStopped, this, [&] {
      if (restarted) return;
      restarted = true;
      load();
    });
    connect(files.get(), &presentation::OverlayFilesViewModel::entriesLoaded, this, [&, target = files.get()] {
      completedWhileLoading |= target->loading();
      ++delivered;
    });
    model = std::move(files);
  } else {
    auto source = std::make_shared<ControlledLogSource>();
    source->read = [&] { return std::vector<std::string>{++reads == 1 ? "old" : "current"}; };
    auto logs = std::make_unique<presentation::LogViewModel>(source);
    load = [target = logs.get()] { target->reload(); };
    connect(logs.get(), &presentation::LogViewModel::entriesChanged, this, [&] {
      if (restarted) return;
      restarted = true;
      load();
    });
    connect(logs.get(), &presentation::LogViewModel::loadingFinished, this, [&, target = logs.get()] {
      completedWhileLoading |= target->loading();
      ++delivered;
    });
    model = std::move(logs);
  }
  load();
  QTRY_VERIFY(restarted);
  QTRY_COMPARE(delivered, 1);
  QCOMPARE(reads.load(), 2);
  QVERIFY(!completedWhileLoading);
  const QString current = model->index(0, overlay ? 0 : 3).data().toString();
  QVERIFY2(current.contains(QStringLiteral("current")), qUtf8Printable(current));
}
void AsyncModelTests::logFilteringRetainsRawRecordsAndCompleteDetails() {
  auto source = std::make_shared<ControlledLogSource>();
  source->read = [] {
    return std::vector<std::string>{"[12:00 I main] Ready", "[12:01 E registry] denied\nHKEY_LOCAL_MACHINE\\SECURITY", "[12:02 F system] fatal",
                                    "unstructured"};
  };
  presentation::LogViewModel model(source);
  auto* filtered = model.filteredModel();
  QAbstractItemModelTester contract(filtered, QAbstractItemModelTester::FailureReportingMode::QtTest);
  QSignalSpy loaded(&model, &presentation::LogViewModel::loadingFinished);
  model.reload();
  QTRY_COMPARE(loaded.count(), 1);
  QCOMPARE(model.loadState(), presentation::ContentLoadState::Ready);
  QCOMPARE(filtered->rowCount(), 4);
  model.setLevelFilter(presentation::LogLevelFilter::Error);
  QCOMPARE(filtered->rowCount(), 2);
  model.setQuery(QStringLiteral("security"));
  QCOMPARE(filtered->rowCount(), 1);
  QCOMPARE(filtered->index(0, 1).data(presentation::FullTextRole).toString(), QStringLiteral("E"));
  QCOMPARE(filtered->index(0, 3).data().toString(), QStringLiteral("denied"));
  QVERIFY(filtered->index(0, 3).data(presentation::FullTextRole).toString().contains(QStringLiteral("HKEY_LOCAL_MACHINE\\SECURITY")));
  model.setQuery(QStringLiteral("ready"));
  QCOMPARE(filtered->rowCount(), 0);
  model.setLevelFilter(presentation::LogLevelFilter::All);
  QCOMPARE(filtered->rowCount(), 1);
  model.setQuery(QStringLiteral("MAIN"));
  QCOMPARE(filtered->rowCount(), 1);
  QVERIFY(model.allRowsToText().contains(QStringLiteral("\tF\t")));
  QVERIFY(model.allRowsToText().contains(QStringLiteral("denied\nHKEY_LOCAL_MACHINE\\SECURITY")));
  model.setQuery(QStringLiteral("fatal"));
  source->read = [] { return std::vector<std::string>{"[12:03 W replacement] fatal replacement"}; };
  model.reload();
  QTRY_COMPARE(loaded.count(), 2);
  QCOMPARE(model.query(), QStringLiteral("fatal"));
  QCOMPARE(filtered->rowCount(), 1);
  QCOMPARE(filtered->index(0, 3).data().toString(), QStringLiteral("fatal replacement"));
}
void AsyncModelTests::logClearFailureRetainsTheLastCompleteSnapshot() {
  auto source = std::make_shared<ControlledLogSource>();
  source->read = [] { return std::vector<std::string>{"[12:00 I main] complete"}; };
  presentation::LogViewModel model(source);
  QSignalSpy loaded(&model, &presentation::LogViewModel::loadingFinished);
  QSignalSpy failed(&model, &presentation::LogViewModel::loadFailed);
  model.reload();
  QTRY_COMPARE(loaded.count(), 1);
  const auto complete = model.allRowsToText();
  source->clearOperation = [] { throw std::runtime_error("clear rejected"); };
  model.clear();
  QCOMPARE(failed.count(), 1);
  QCOMPARE(model.loadState(), presentation::ContentLoadState::Failed);
  QVERIFY(model.errorText().contains(QStringLiteral("clear rejected")));
  QCOMPARE(model.allRowsToText(), complete);
  QVERIFY(!model.loading());
}
}  // namespace

QTEST_GUILESS_MAIN(AsyncModelTests)
#include "AsyncModelTests.moc"
