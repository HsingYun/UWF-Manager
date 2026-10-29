// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTest>
#include <QThreadPool>
#include <algorithm>
#include <stdexcept>
#include <thread>

#include "app/staging/FileStagingTask.h"
#include "windows/commit/WindowsCommitTraversalSource.h"

namespace {
using namespace uwf;
using namespace uwf::app;

class StagingWmi final : public uwf::app::wmi::WmiOperations {
 public:
  QString drive;
  QStringList exclusions;
  mutable int filterReads = 0;
  mutable int volumeReads = 0;
  mutable int exclusionReads = 0;
  mutable int commits = 0;
  std::function<void()> beforeFilter;
  std::function<void()> beforeCommit;

  void ensureConnected() const override {}
  std::vector<uwf::app::wmi::WmiRow> query(const std::string&) const override { throw std::logic_error("unexpected projection query"); }
  std::vector<uwf::app::wmi::WmiRow> queryInstances(const std::string& wql) const override {
    if (wql == "SELECT * FROM UWF_Filter") {
      if (beforeFilter) beforeFilter();
      ++filterReads;
      return {{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter")},
               {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
               {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}};
    }
    if (wql != "SELECT * FROM UWF_Volume") throw std::logic_error("unexpected instance query");
    ++volumeReads;
    return {{{"__PATH", uwf::app::wmi::WmiValue::fromString("volume")},
             {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(true)},
             {"DriveLetter", uwf::app::wmi::WmiValue::fromString(drive.toStdString())},
             {"VolumeName", uwf::app::wmi::WmiValue::fromString("Volume{staging-fixture}")},
             {"BindByDriveLetter", uwf::app::wmi::WmiValue::fromBool(true)},
             {"CommitPending", uwf::app::wmi::WmiValue::fromBool(false)},
             {"Protected", uwf::app::wmi::WmiValue::fromBool(true)}}};
  }
  uwf::app::wmi::WmiMethodOutput callMethodRead(const std::string& path, const std::string& method, const uwf::app::wmi::WmiRow&) const override {
    if (path != "volume" || method != "GetExclusions") throw std::logic_error("unexpected read method");
    ++exclusionReads;
    uwf::app::wmi::WmiMethodOutput result;
    auto& rows = result.arrays["ExcludedFiles"];
    for (const auto& exclusion : exclusions) rows.push_back({{"FileName", uwf::app::wmi::WmiValue::fromString(exclusion.toStdString())}});
    return result;
  }
  void invokeMethod(const std::string& path, const std::string& method, const uwf::app::wmi::WmiRow&) const override {
    if (path != "volume" || method != "CommitFile") throw std::logic_error("unexpected write method");
    if (beforeCommit) beforeCommit();
    ++commits;
  }
  uwf::app::wmi::WmiClassStatus classStatus(const std::string&) const override { throw std::logic_error("unexpected class lookup"); }
  uwf::app::wmi::WmiRow getObject(const std::string&) const override { throw std::logic_error("unexpected object read"); }
  uwf::app::wmi::WmiMethodOutput callMethodReadCancelable(const std::string&, const std::string&, const uwf::app::wmi::WmiRow&,
                                                          std::stop_token) const override {
    throw std::logic_error("unexpected cancellable read");
  }
  void putInstance(const std::string&, const uwf::app::wmi::WmiRow&, uwf::app::wmi::WmiPutMode) const override {
    throw std::logic_error("unexpected instance write");
  }
};

class StagingStore final : public FileStagingStore {
 public:
  QList<FileStagingEntry> entries;
  mutable int loads = 0;
  QList<FileStagingEntry> load() const override {
    ++loads;
    return entries;
  }
  void replace(const QList<FileStagingEntry>&) override { throw std::logic_error("preparation must not write the staging list"); }
};

QString nativePath(const QString& path) { return QDir::toNativeSeparators(path); }

bool createFile(const QString& path) {
  const qsizetype separator = path.lastIndexOf(u'/');
  if (separator >= 0 && !QDir().mkpath(path.first(separator))) return false;
  QFile file(path);
  return file.open(QIODevice::WriteOnly);
}

QStringList fixtureFiles() {
  return {QStringLiteral("cache/inside.bin"),
          QStringLiteral("cache.child/sibling.bin"),
          QStringLiteral("saved.bin"),
          QStringLiteral("Ärger/Öl.bin"),
          QStringLiteral("Ärger.child/sibling.bin"),
          QStringLiteral("中文文件/inside.bin"),
          QStringLiteral("中文文件.child/sibling.bin")};
}

class FileStagingPreparationTests final : public QObject {
  Q_OBJECT
 private slots:
  void taskNeverBlocksItsOwner_data() {
    QTest::addColumn<bool>("duringCommit");
    QTest::newRow("preparation") << false;
    QTest::newRow("commit") << true;
  }
  void taskNeverBlocksItsOwner() {
    QFETCH(bool, duringCommit);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = nativePath(directory.filePath(QStringLiteral("state.bin")));
    QVERIFY(createFile(path));
    const auto later = nativePath(directory.filePath(QStringLiteral("later.bin")));
    QVERIFY(createFile(later));
    StagingWmi wmi;
    wmi.drive = directory.path().first(2);
    std::atomic_bool entered{false};
    std::atomic_bool release{false};
    std::atomic_bool ranOnOwner{false};
    const auto ownerThread = std::this_thread::get_id();
    const auto unblock = qScopeGuard([&] {
      release = true;
      QThreadPool::globalInstance()->waitForDone();
    });
    const auto block = [&] {
      ranOnOwner = std::this_thread::get_id() == ownerThread;
      entered = true;
      QElapsedTimer emergency;
      emergency.start();
      while (!release && emergency.elapsed() < 2000) std::this_thread::yield();
    };
    if (duringCommit)
      wmi.beforeCommit = block;
    else
      wmi.beforeFilter = block;
    auto task = std::make_unique<FileStagingTask>(wmi, QList<FileStagingEntry>{{FileStagingKind::File, path}, {FileStagingKind::File, later}},
                                                  UwfCapability::Available, uwf::windows::systemCommitTraversalSource());
    QElapsedTimer elapsed;
    elapsed.start();
    QVERIFY(!task->pollPreparation());
    QVERIFY(elapsed.elapsed() < 100);
    if (duringCommit) {
      QTRY_VERIFY(task->pollPreparation());
      QCOMPARE(wmi.commits, 0);  // Preparation must never authorize a write.
      elapsed.restart();
      static_cast<void>(task->advance());
      QVERIFY(elapsed.elapsed() < 100);
    }
    QTRY_VERIFY(entered.load());
    QVERIFY(!ranOnOwner.load());
    elapsed.restart();
    task.reset();
    QVERIFY(elapsed.elapsed() < 100);
    QVERIFY(!release.load());
    release = true;
    QThreadPool::globalInstance()->waitForDone();
    QCOMPARE(wmi.commits, duringCommit ? 1 : 0);
  }
  void exclusionsPreserveFileSelection_data() {
    QTest::addColumn<QStringList>("excluded");
    QTest::addColumn<QStringList>("remaining");
    const auto files = fixtureFiles();
    const auto without = [&](const QString& file) {
      auto result = files;
      result.removeAll(file);
      return result;
    };
    QTest::newRow("no-exclusions") << QStringList{} << files;
    QTest::newRow("exact-file") << QStringList{QStringLiteral("saved.bin")} << without(QStringLiteral("saved.bin"));
    QTest::newRow("directory-boundary") << QStringList{QStringLiteral("cache")} << without(QStringLiteral("cache/inside.bin"));
    QTest::newRow("case-insensitive") << QStringList{QStringLiteral("CACHE")} << without(QStringLiteral("cache/inside.bin"));
    QTest::newRow("mixed-separators") << QStringList{QStringLiteral("cache\\")} << without(QStringLiteral("cache/inside.bin"));
    QTest::newRow("unicode-case-insensitive") << QStringList{QStringLiteral("ÄRGER")} << without(QStringLiteral("Ärger/Öl.bin"));
    QTest::newRow("unicode-directory") << QStringList{QStringLiteral("中文文件")} << without(QStringLiteral("中文文件/inside.bin"));
    QTest::newRow("normalized-exclusion") << QStringList{QStringLiteral("cache/../cache")} << without(QStringLiteral("cache/inside.bin"));
    QTest::newRow("duplicate-exclusions") << QStringList{QStringLiteral("cache"), QStringLiteral("CACHE\\"), QStringLiteral("cache/inside.bin")}
                                          << without(QStringLiteral("cache/inside.bin"));
    QTest::newRow("unrelated-prefix") << QStringList{QStringLiteral("cach"), QStringLiteral("saved.bin.extra")} << files;
    QTest::newRow("staged-root") << QStringList{QStringLiteral(".")} << QStringList{};
    QTest::newRow("volume-root") << QStringList{QStringLiteral("@drive")} << QStringList{};
  }

  void exclusionsPreserveFileSelection() {
    QFETCH(QStringList, excluded);
    QFETCH(QStringList, remaining);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto files = fixtureFiles();
    for (const auto& file : files) QVERIFY(createFile(directory.filePath(file)));
    StagingWmi wmi;
    wmi.drive = directory.path().first(2);
    for (const auto& exclusion : excluded) {
      wmi.exclusions.append(exclusion == QStringLiteral("@drive") ? QStringLiteral("\\") : directory.filePath(exclusion));
    }
    const QList<FileStagingEntry> entries{{FileStagingKind::Directory, nativePath(directory.path())},
                                          {FileStagingKind::File, nativePath(directory.filePath(QStringLiteral("saved.bin")))}};
    FileStagingCommitter committer(wmi, uwf::windows::systemCommitTraversalSource());
    auto operation = committer.beginCommit(committer.prepare(FileStagingCommitter::scan(committer.prepareScan(entries))));
    QStringList actual;
    while (!operation.finished()) actual.append(QDir(directory.path()).relativeFilePath(operation.advance()));
    std::ranges::sort(actual);
    std::ranges::sort(remaining);
    QCOMPARE(actual, remaining);
    QVERIFY(operation.result().succeeded());
    QCOMPARE(operation.result().committedFiles, static_cast<std::size_t>(remaining.size()));
    QCOMPARE(wmi.commits, remaining.size());
    QCOMPARE(wmi.volumeReads, 1);
    QCOMPARE(wmi.exclusionReads, 1);
  }

  void largePreparationUsesOneSnapshotAndCompletesEveryTarget() {
    constexpr int fileCount = 4096;
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    for (int index = 0; index < fileCount; ++index) QVERIFY(createFile(directory.filePath(QString::number(index) + QStringLiteral(".bin"))));
    StagingWmi wmi;
    wmi.drive = directory.path().first(2);
    for (int index = 0; index < 512; ++index) wmi.exclusions.append(directory.filePath(QStringLiteral("unrelated/%1").arg(index)));
    StagingStore store;
    store.entries = {{FileStagingKind::Directory, nativePath(directory.path())}};
    const auto snapshot = store.entries;
    FileStagingTask task(wmi, store, uwf::app::UwfCapability::Available, uwf::windows::systemCommitTraversalSource());
    QVERIFY(!task.pollPreparation());
    store.entries.clear();
    QTRY_VERIFY_WITH_TIMEOUT(task.pollPreparation(), 20000);
    QCOMPARE(task.sourceEntries(), std::optional{snapshot});
    QCOMPARE(task.discoveredDuringScan(), static_cast<std::size_t>(fileCount));
    QCOMPARE(task.totalFiles(), static_cast<std::size_t>(fileCount));
    QVERIFY(task.pollPreparation());
    QCOMPARE(store.loads, 1);
    QCOMPARE(wmi.filterReads, 1);
    QCOMPARE(wmi.volumeReads, 1);
    QCOMPARE(wmi.exclusionReads, 1);
    while (!task.finished()) {
      static_cast<void>(task.advance());
      task.waitForWorker();
    }
    QVERIFY(task.result().succeeded());
    QCOMPARE(task.result().committedFiles, static_cast<std::size_t>(fileCount));
    QCOMPARE(wmi.commits, fileCount);
  }
};
}  // namespace

QTEST_GUILESS_MAIN(FileStagingPreparationTests)
#include "FileStagingPreparationTests.moc"
