// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>

#include "app/staging/DurableFileStagingStore.h"
#include "support/app/RecordingWmiOperations.h"

using namespace uwf;
namespace {
class Storage final : public app::FileStagingStorage {
 public:
  QList<app::FileStagingEntry> entries;
  int writes = 0;
  QList<app::FileStagingEntry> load() const override { return entries; }
  void replace(const QList<app::FileStagingEntry>& value) override {
    ++writes;
    entries = value;
  }
  bool exists() const override { return !entries.isEmpty(); }
  std::vector<app::RegistryCommitTarget> deletionPlan() const override {
    return {{std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue)},
            {std::string(app::kFileStagingRegistryKey), {}},
            {std::string(app::kFileStagingRegistryRoot), {}}};
  }
};
}  // namespace
class FileStagingDurabilityTests final : public QObject {
  Q_OBJECT
 private slots:
  void writesOnlyTheCurrentSession_data() {
    QTest::addColumn<bool>("filterEnabled");
    QTest::addColumn<bool>("covered");
    QTest::addColumn<bool>("clear");
    for (const bool enabled : {false, true})
      for (const bool covered : {false, true})
        for (const bool clear : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("filter-%1-covered-%2-clear-%3").arg(enabled).arg(covered).arg(clear))) << enabled << covered << clear;
  }
  void writesOnlyTheCurrentSession() {
    QFETCH(bool, filterEnabled);
    QFETCH(bool, covered);
    QFETCH(bool, clear);
    auto storage = std::make_shared<Storage>();
    storage->entries = {{app::FileStagingKind::File, QStringLiteral("C:\\data.txt")}};
    test::application::RecordingWmiOperations wmi;
    using app::wmi::WmiValue;
    wmi.queryResults = {{{{"__PATH", WmiValue::fromString("filter")},
                          {"CurrentEnabled", WmiValue::fromBool(filterEnabled)},
                          {"NextEnabled", WmiValue::fromBool(filterEnabled)}}},
                        {{{"__PATH", WmiValue::fromString("registry-current")},
                          {"CurrentSession", WmiValue::fromBool(true)},
                          {"PersistDomainSecretKey", WmiValue::fromBool(false)},
                          {"PersistTSCAL", WmiValue::fromBool(false)}}}};
    if (filterEnabled) {
      app::wmi::WmiMethodOutput exclusions;
      exclusions.arrays["ExcludedKeys"] =
          covered ? std::vector<app::wmi::WmiRow>{{{"RegistryKey", WmiValue::fromString("HKEY_LOCAL_MACHINE\\SOFTWARE")}}} : std::vector<app::wmi::WmiRow>{};
      wmi.readResults.push_back(exclusions);
    }
    wmi.invocationObserver = [storage, &wmi, clear] {
      if (clear && wmi.invocations.size() == 2) storage->entries.clear();
    };
    app::DurableFileStagingStore store(storage, wmi, app::UwfCapability::Available);
    const auto target = clear ? QList<app::FileStagingEntry>{} : storage->entries;
    store.replace(target);
    QCOMPARE(store.load(), target);
    const bool needsCommit = filterEnabled && !covered;
    QCOMPARE(wmi.invocations.size(), needsCommit ? (clear ? std::size_t{3} : std::size_t{1}) : std::size_t{0});
    for (const auto& method : wmi.invocations) QCOMPARE(method, clear ? QStringLiteral("CommitRegistryDeletion") : QStringLiteral("CommitRegistry"));
    for (const auto& path : wmi.invocationPaths) QCOMPARE(path, QStringLiteral("registry-current"));
    QVERIFY(wmi.queryResults.empty());
    QVERIFY(wmi.readResults.empty());
  }
};
QTEST_APPLESS_MAIN(FileStagingDurabilityTests)
#include "FileStagingDurabilityTests.moc"
