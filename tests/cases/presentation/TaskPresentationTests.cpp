// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <stdexcept>

#include "presentation/power/PowerDialogViewModel.h"
#include "presentation/staging/FileStagingProgressViewModel.h"
#include "windows/commit/WindowsCommitTraversalSource.h"
namespace {
namespace wmi = uwf::app::wmi;
class RejectingWmiOperations final : public wmi::WmiOperations {
 public:
  void ensureConnected() const override { unexpected(); }
  std::vector<wmi::WmiRow> query(const std::string&) const override { unexpected(); }
  std::vector<wmi::WmiRow> queryInstances(const std::string&) const override { unexpected(); }
  wmi::WmiClassStatus classStatus(const std::string&) const override { unexpected(); }
  wmi::WmiRow getObject(const std::string&) const override { unexpected(); }
  void invokeMethod(const std::string&, const std::string&, const wmi::WmiRow&) const override { unexpected(); }
  wmi::WmiMethodOutput callMethodRead(const std::string&, const std::string&, const wmi::WmiRow&) const override { unexpected(); }
  wmi::WmiMethodOutput callMethodReadCancelable(const std::string&, const std::string&, const wmi::WmiRow&, std::stop_token) const override { unexpected(); }
  void putInstance(const std::string&, const wmi::WmiRow&, wmi::WmiPutMode) const override { unexpected(); }

 private:
  [[noreturn]] static void unexpected() { throw std::logic_error("WMI must not be used for unavailable UWF capability"); }
};
class TaskPresentationTests final : public QObject {
  Q_OBJECT
 private slots:
  void directPowerConfirmationDefersPreparationUntilCanceled() {
    int reads = 0;
    uwf::presentation::PowerDialogViewModel model({uwf::app::PowerAction::Shutdown, [&]() -> std::optional<uwf::app::PowerStagingPreparation> {
                                                     ++reads;
                                                     return uwf::app::PowerStagingNotRequired{};
                                                   }});
    QSignalSpy ready(&model, &uwf::presentation::PowerDialogViewModel::preparationReady);
    model.start();
    model.beginDirectConfirmation();
    QTest::qWait(40);
    QCOMPARE(reads, 0);
    model.cancelDirectConfirmation();
    QTRY_COMPARE_WITH_TIMEOUT(ready.count(), 1, 2000);
    QCOMPARE(reads, 1);
    QCOMPARE(model.finish(false), uwf::app::PowerActionDialogOutcome::Canceled);
  }
  void stagingCompletesWithoutAProgressWindow_data() {
    QTest::addColumn<bool>("failRead");
    QTest::newRow("completed") << false;
    QTest::newRow("preparation-failed") << true;
  }
  void stagingCompletesWithoutAProgressWindow() {
    QFETCH(bool, failRead);
    class FailingStore final : public uwf::app::FileStagingStore {
     public:
      QList<uwf::app::FileStagingEntry> load() const override { throw std::runtime_error("read failed"); }
      void replace(const QList<uwf::app::FileStagingEntry>&) override { throw std::logic_error("unexpected write"); }
    } store;
    RejectingWmiOperations transport;
    QStringList notifications;
    std::optional<uwf::app::FileStagingBatchResult> completed;
    const QList<uwf::app::FileStagingEntry> entries{{uwf::app::FileStagingKind::Directory, QStringLiteral("C:\\Data")}};
    auto task = failRead ? std::make_unique<uwf::app::FileStagingTask>(transport, store, uwf::app::UwfCapability::Unavailable,
                                                                       uwf::windows::systemCommitTraversalSource())
                         : std::make_unique<uwf::app::FileStagingTask>(transport, entries, uwf::app::UwfCapability::Unavailable,
                                                                       uwf::windows::systemCommitTraversalSource());
    uwf::presentation::FileStagingProgressViewModel model(
        std::move(task),
        [&](const auto& result) {
          notifications.append(QStringLiteral("completion"));
          completed = result;
        },
        [](std::size_t, std::size_t) {});
    connect(&model, &uwf::presentation::FileStagingProgressViewModel::presentationRequested, this, [&] { notifications.append(QStringLiteral("requested")); });
    connect(&model, &uwf::presentation::FileStagingProgressViewModel::presentationFinished, this, [&] { notifications.append(QStringLiteral("finished")); });
    model.start();
    QTRY_VERIFY(completed.has_value());
    QCOMPARE(completed->command.completed(), !failRead);
    QCOMPARE(model.state().phase, uwf::presentation::FileStagingProgressViewModel::Phase::Finished);
    QCOMPARE(model.state().outcome, completed->command.outcome);
    QCOMPARE(completed->command.skippedEntries, failRead ? std::size_t{0} : std::size_t{1});
    QCOMPARE(completed->sourceEntries.has_value(), !failRead);
    if (!failRead) QCOMPARE(*completed->sourceEntries, entries);
    QCOMPARE(notifications, (QStringList{QStringLiteral("requested"), QStringLiteral("finished"), QStringLiteral("completion")}));
  }
};
}  // namespace
QTEST_GUILESS_MAIN(TaskPresentationTests)
#include "TaskPresentationTests.moc"
