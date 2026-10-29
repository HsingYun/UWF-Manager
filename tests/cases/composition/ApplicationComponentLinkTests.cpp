// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <stdexcept>

#include "app/commit/CommitWorkflow.h"
#include "app/overlay/OverlayFilesFailure.h"
#include "app/staging/FileStagingCommitter.h"
#include "app/uwf/UwfFilter.h"
#include "app/uwf/UwfStateReader.h"
#include "app/uwf/UwfVolume.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "support/app/RecordingWmiOperations.h"

namespace {
using namespace uwf;
class MemoryCommitSource final : public app::CommitTraversalSource {
 public:
  app::CommitTargetKind kind = app::CommitTargetKind::File;
  bool present = true;
  bool unexpectedChild = false;
  mutable int inspections = 0;
  mutable int verifications = 0;
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override {
    ++inspections;
    if (!present) return std::nullopt;
    auto node = target;
    node.kind = kind;
    return node;
  }
  bool exists(const app::CommitNode&) const override {
    ++verifications;
    return present;
  }
  std::string volumeForPath(const QString&) const override { return "C:"; }
  std::vector<std::string> mountedLetters(const std::string&) const override { return {"C:"}; }
  QString read(const app::CommitNode& target, std::stop_token, const app::CommitNodeVisitor& visitor) const override {
    if (unexpectedChild) visitor({kind, target.path + QStringLiteral("\\new"), {}, {}});
    return {};
  }
};
app::wmi::WmiRow commitProviderRow(bool registry) {
  using Value = app::wmi::WmiValue;
  if (registry)
    return {{"__PATH", Value::fromString("registry")},
            {"CurrentSession", Value::fromBool(true)},
            {"PersistDomainSecretKey", Value::fromBool(false)},
            {"PersistTSCAL", Value::fromBool(false)}};
  return {{"__PATH", Value::fromString("volume")},      {"CurrentSession", Value::fromBool(true)},
          {"DriveLetter", Value::fromString("C:")},     {"VolumeName", Value::fromString("Volume{memory}")},
          {"BindByDriveLetter", Value::fromBool(true)}, {"CommitPending", Value::fromBool(false)},
          {"Protected", Value::fromBool(true)}};
}

class MemoryWmiOperations final : public uwf::app::wmi::WmiOperations {
 public:
  mutable bool nextEnabled = false;
  mutable int writes = 0;
  mutable int rereads = 0;
  void ensureConnected() const override {}
  std::vector<uwf::app::wmi::WmiRow> query(const std::string&) const override { return {}; }
  std::vector<uwf::app::wmi::WmiRow> queryInstances(const std::string& query) const override {
    if (query == "SELECT * FROM UWF_Filter") return {filterRow()};
    if (query == "SELECT * FROM UWF_Volume") return {};
    throw std::logic_error("unexpected instance query");
  }
  uwf::app::wmi::WmiClassStatus classStatus(const std::string&) const override { return uwf::app::wmi::WmiClassStatus::Present; }
  uwf::app::wmi::WmiRow getObject(const std::string& path) const override {
    if (path != "filter") throw std::logic_error("unexpected object read");
    ++rereads;
    return filterRow();
  }
  void invokeMethod(const std::string& path, const std::string& method, const uwf::app::wmi::WmiRow&) const override {
    if (path != "filter" || method != "Enable") throw std::logic_error("unexpected write");
    ++writes;
    nextEnabled = true;
  }
  uwf::app::wmi::WmiMethodOutput callMethodRead(const std::string&, const std::string&, const uwf::app::wmi::WmiRow&) const override {
    throw std::logic_error("unexpected method read");
  }
  uwf::app::wmi::WmiMethodOutput callMethodReadCancelable(const std::string&, const std::string&, const uwf::app::wmi::WmiRow&,
                                                          std::stop_token) const override {
    throw std::logic_error("unexpected cancelable method read");
  }
  void putInstance(const std::string&, const uwf::app::wmi::WmiRow&, uwf::app::wmi::WmiPutMode) const override {
    throw std::logic_error("unexpected instance write");
  }

 private:
  uwf::app::wmi::WmiRow filterRow() const {
    return {{"__PATH", uwf::app::wmi::WmiValue::fromString("filter")},
            {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(false)},
            {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(nextEnabled)}};
  }
};

class ApplicationComponentLinkTests final : public QObject {
  Q_OBJECT
 private slots:
  void deletionUsesInjectedStateBeforeAndAfterWrites_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("newContent");
    for (const bool registry : {false, true})
      for (const bool changed : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("registry-%1-new-content-%2").arg(registry).arg(changed))) << registry << changed;
  }
  void deletionUsesInjectedStateBeforeAndAfterWrites() {
    QFETCH(bool, registry);
    QFETCH(bool, newContent);
    test::application::RecordingWmiOperations wmi;
    wmi.queryResults.push_back({commitProviderRow(registry)});
    auto source = std::make_shared<MemoryCommitSource>();
    source->kind = registry ? app::CommitTargetKind::RegistryKey : app::CommitTargetKind::Folder;
    wmi.invocationObserver = [source] { source->present = false; };
    bool called = false;
    app::CommitInteraction interaction;
    interaction.run = [&](const app::CommitRequest& request, const std::shared_ptr<app::CommitBackend> backend) {
      called = true;
      const auto plan = backend->prepare(request, {}, {}, {});
      QCOMPARE(plan.progress().failed, 0);
      const auto targets = plan.targets();
      QCOMPARE(targets.size(), 1);
      source->unexpectedChild = newContent;
      const auto result = backend->execute(targets.front(), request.operation);
      QCOMPARE(result.succeeded, !newContent);
      QCOMPARE(wmi.invocations.size(), newContent ? std::size_t{0} : std::size_t{1});
      QCOMPARE(source->verifications, 2);
      if (newContent) QVERIFY(!result.failureDetail.isEmpty());
    };
    const core::UwfSnapshot snapshot;
    app::CommitWorkflow workflow(wmi, snapshot, std::move(interaction), source);
    if (registry)
      workflow.commitRegistryDeletionKey(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\VirtualCommitFixture"), {});
    else
      workflow.commitFileDeletionPath(QStringLiteral("C:\\VirtualCommitFixture"));
    QVERIFY(called);
  }
  void stagingScanAndCommitUseTheSameInjectedSource() {
    test::application::RecordingWmiOperations wmi;
    wmi.queryResults.push_back({commitProviderRow(false)});
    app::wmi::WmiMethodOutput exclusions;
    exclusions.arrays["ExcludedFiles"] = {};
    wmi.readResults.push_back(exclusions);
    auto source = std::make_shared<MemoryCommitSource>();
    app::FileStagingCommitter committer(wmi, source);
    const QString path = QStringLiteral("C:\\VirtualCommitFixture\\file.bin");
    auto operation = committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan({{app::FileStagingKind::File, path}}))));
    QCOMPARE(operation.totalFiles(), std::size_t{1});
    QCOMPARE(operation.advance(), path);
    QCOMPARE(source->inspections, 2);
    QCOMPARE(operation.result().committedFiles, std::size_t{1});
  }
  void injectedProviderClientsLinkAndRunWithoutWindowsAdapters() {
    MemoryWmiOperations transport;
    const uwf::app::provider::UwfFilter filter(transport);
    const auto state = filter.read();
    QVERIFY(!state.nextEnabled);
    filter.enable(state);
    QCOMPARE(transport.writes, 1);
    QCOMPARE(transport.rereads, 1);
    QVERIFY(filter.read().nextEnabled);
    const uwf::drive::MountLookup mounts = [](const std::string&) { return std::vector<std::string>{}; };
    QVERIFY(uwf::app::provider::UwfVolume(transport, mounts).readAll().empty());
    QCOMPARE(uwf::app::probeUwfCapability(transport), uwf::app::UwfCapability::Available);
    QVERIFY(uwf::app::enumerateVolumes(transport).empty());
    const auto unavailable = uwf::app::readSnapshot(transport, uwf::app::UwfCapability::Unavailable, true, mounts);
    QVERIFY(!unavailable.uwfAvailable);
    QVERIFY(unavailable.elevated);
    const auto failure =
        std::make_exception_ptr(uwf::app::wmi::WmiInfrastructureError(static_cast<int32_t>(uwf::app::wmi::WmiErrorCode::RpcServerFault), "read overlay files"));
    QCOMPARE(uwf::app::classifyOverlayFilesFailure(failure).kind, uwf::app::OverlayFilesFailure::Kind::ProviderCrashed);
  }
};

}  // namespace

QTEST_APPLESS_MAIN(ApplicationComponentLinkTests)
#include "ApplicationComponentLinkTests.moc"
