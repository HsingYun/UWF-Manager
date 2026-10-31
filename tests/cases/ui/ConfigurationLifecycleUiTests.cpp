// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QListView>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QThreadPool>
#include <QTimer>
#include <QWindow>

#include "app/configuration/ExclusionEditor.h"
#include "application/ApplicationUiFixture.h"
#include "bootstrap/MainWindowComposition.h"
#include "presentation/common/ItemRoles.h"
#include "support/app/WorkerGate.h"
#include "ui/appearance/I18n.h"
#include "ui/configuration/ApplyPlanDialog.h"
#include "ui/configuration/GlobalStatusPanel.h"
#include "ui/configuration/VolumeTab.h"
#include "ui/overlay/OverlayContextMenu.h"
#include "ui/overlay/OverlayHub.h"
#include "ui/overlay/OverlayHubView.h"
#include "ui/shell/MainWindow.h"
#include "ui/staging/FileStagingWidget.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

class MenuEndpoint final : public ui::OverlayHubView {
 public:
  MenuEndpoint() : menu(this) { connectContextMenu(menu); }
  int priority() const override { return 0; }
  VerificationResult verifyPresentation() const override { return VerificationResult::Invalid; }
  ui::OverlayContextMenu menu;
};

class ConfigurationLifecycleUiTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() { ui::I18n::instance().setLang(ui::I18n::Lang::En); }

  void operationAvailabilityReachesExistingAndNewHubMenus_data() {
    QTest::addColumn<bool>("power");
    QTest::addColumn<bool>("exit");
    for (const bool power : {false, true})
      for (const bool exit : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("power-%1-exit-%2").arg(power).arg(exit))) << power << exit;
  }
  void operationAvailabilityReachesExistingAndNewHubMenus() {
    QFETCH(bool, power);
    QFETCH(bool, exit);
    ui::OverlayHub hub;
    auto first = std::make_unique<MenuEndpoint>();
    auto* existing = first.get();
    hub.registerView(std::move(first));
    hub.setOperationAvailability(power, exit);
    auto second = std::make_unique<MenuEndpoint>();
    auto* added = second.get();
    hub.registerView(std::move(second));
    for (auto* endpoint : {existing, added}) {
      QCOMPARE(endpoint->menu.findChild<QAction*>(QStringLiteral("overlayMenuSafeRestart"))->isEnabled(), power);
      QCOMPARE(endpoint->menu.findChild<QAction*>(QStringLiteral("overlayMenuExit"))->isEnabled(), exit);
    }
    hub.setOperationAvailability(true, true);
    for (auto* endpoint : {existing, added}) {
      QVERIFY(endpoint->menu.findChild<QAction*>(QStringLiteral("overlayMenuSafeRestart"))->isEnabled());
      QVERIFY(endpoint->menu.findChild<QAction*>(QStringLiteral("overlayMenuExit"))->isEnabled());
    }
  }

  void applicationWriteBlocksEveryExitAndPowerMenu_data() {
    QTest::addColumn<bool>("hubVisible");
    QTest::addColumn<bool>("writeFails");
    for (const bool hub : {false, true})
      for (const bool fails : {false, true}) QTest::newRow(qPrintable(QStringLiteral("hub-%1-failure-%2").arg(hub).arg(fails))) << hub << fails;
  }
  void applicationWriteBlocksEveryExitAndPowerMenu() {
    QFETCH(bool, hubVisible);
    QFETCH(bool, writeFails);
    auto gate = std::make_shared<test::WorkerGate>();
    const auto unblock = qScopeGuard([&] {
      gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    RecordingWmiOperations wmi;
    using app::wmi::WmiValue;
    wmi.queryResults.push_back(
        {{{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(true)}}});
    wmi.objectResults.push_back(
        {{"__PATH", WmiValue::fromString("filter-path")}, {"CurrentEnabled", WmiValue::fromBool(true)}, {"NextEnabled", WmiValue::fromBool(false)}});
    wmi.invocationObserver = [gate] { gate->wait({}, false); };
    if (writeFails) wmi.invocationFailure = std::make_exception_ptr(app::wmi::WmiProviderError(5, "filter", "rejected"));
    MutableApplicationStateSource source;
    source.snapshot = editableSnapshot();
    auto store = std::make_shared<MemoryFileStagingStore>();
    ui::MainWindow window(bootstrap::mainWindowServices(wmi, source, store), {.uwfCapability = app::UwfCapability::Available, .hubVisible = hubVisible});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* global = window.findChild<ui::GlobalStatusPanel*>();
    QVERIFY(global);
    QVERIFY(global->viewModel().importFilterEnabled(false));
    bool inspected = false;
    bool finish = false;
    QTimer closer;
    connect(&closer, &QTimer::timeout, &window, [&] {
      if (finish)
        if (auto* dialog = qobject_cast<ui::ApplyPlanDialog*>(QApplication::activeModalWidget()); dialog && applyCompleted(*dialog)) dialog->reject();
    });
    closer.start(50);
    QTimer::singleShot(0, &window, [&] {
      const auto cleanup = qScopeGuard([&] {
        gate->release();
        finish = true;
      });
      auto* dialog = qobject_cast<ui::ApplyPlanDialog*>(QApplication::activeModalWidget());
      QVERIFY(dialog);
      auto* apply = buttonWithText(dialog, ui::I18n::tr("Apply changes"));
      QVERIFY(apply && apply->isEnabled());
      QTimer::singleShot(0, dialog, [] {
        if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->accept();
      });
      QTest::mouseClick(apply, Qt::LeftButton);
      QTRY_COMPARE(gate->started.load(), 1);
      QCOMPARE(gate->finished.load(), 0);
      QVERIFY(!window.close());
      dialog->reject();
      QVERIFY(dialog->isVisible());
      int menus = 0;
      const auto widgets = QApplication::allWidgets();
      for (QWidget* widget : widgets) {
        auto* menu = qobject_cast<ui::OverlayContextMenu*>(widget);
        if (!menu) continue;
        ++menus;
        QSignalSpy exits(menu, &ui::OverlayContextMenu::exitApplicationRequested);
        QSignalSpy restarts(menu, &ui::OverlayContextMenu::safeRestartRequested);
        for (const auto* name : {"overlayMenuExit", "overlayMenuSafeRestart", "overlayMenuSafeShutdown"}) {
          auto* action = menu->findChild<QAction*>(QString::fromLatin1(name));
          QVERIFY(action && !action->isEnabled());
          menu->popup(window.mapToGlobal(QPoint(30, 30)));
          const auto closeMenu = qScopeGuard([menu] { menu->close(); });
          QTest::qWait(30);
          QTest::mouseClick(menu->windowHandle(), Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(action).center());
        }
        QCOMPARE(exits.size(), 0);
        QCOMPARE(restarts.size(), 0);
      }
      QVERIFY(menus > 0);
      inspected = true;
    });
    window.showPlan();
    closer.stop();
    QVERIFY(inspected);
    QCOMPARE(wmi.invocations.size(), std::size_t{1});
    auto* menu = window.findChild<ui::OverlayContextMenu*>();
    QVERIFY(menu);
    QTRY_VERIFY(menu->findChild<QAction*>(QStringLiteral("overlayMenuExit"))->isEnabled());
    QTRY_VERIFY(menu->findChild<QAction*>(QStringLiteral("overlayMenuSafeRestart"))->isEnabled());
  }

  void startupKeepsSummaryAndPlanConsistent_data() {
    QTest::addColumn<bool>("quiet");
    QTest::addColumn<bool>("filter");
    for (const bool quiet : {false, true})
      for (const bool filter : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("quiet-%1-filter-%2").arg(quiet).arg(filter))) << quiet << filter;
  }
  void startupKeepsSummaryAndPlanConsistent() {
    QFETCH(bool, quiet);
    QFETCH(bool, filter);
    RecordingWmiOperations wmi;
    auto storeOwner = std::make_shared<MemoryFileStagingStore>();
    auto& store = *storeOwner;
    MutableApplicationStateSource source;
    source.snapshot = editableSnapshot();
    source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = filter;
    source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
    ui::MainWindow window(bootstrap::mainWindowServices(wmi, source, storeOwner), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    if (quiet) window.startInTray();
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* global = window.findChild<ui::GlobalStatusPanel*>();
    QVERIFY(global);
    QVERIFY(!global->viewModel().pendingFilterEnabled());
    QVERIFY(global->viewModel().pendingOverlay().empty());
    auto* discard = buttonWithText(&window, ui::I18n::tr("Discard changes"));
    QVERIFY(discard);
    QVERIFY2(!discard->isVisibleTo(&window), "Startup must not present a dirty summary for an empty draft");
    bool inspected = false;
    QTimer::singleShot(0, &window, [&] {
      auto* plan = qobject_cast<ui::ApplyPlanDialog*>(QApplication::activeModalWidget());
      QVERIFY(plan);
      const auto close = qScopeGuard([plan] { plan->reject(); });
      auto* changes = plan->findChild<QListView*>(QStringLiteral("applyChangesList"));
      auto* apply = buttonWithText(plan, ui::I18n::tr("Apply changes"));
      QVERIFY(changes && apply);
      QCOMPARE(changes->model()->rowCount(), 1);  // Only the command disclosure remains.
      QVERIFY(!apply->isEnabled());
      inspected = true;
    });
    window.showPlan();
    QVERIFY(inspected);
    QCOMPARE(store.writes, 0);
    QVERIFY(wmi.invocations.empty());
  }

  void confirmedDiscardRestoresAllEditors_data() {
    QTest::addColumn<bool>("confirm");
    QTest::addColumn<bool>("stagingReadFails");
    QTest::addColumn<bool>("allEditors");
    for (const bool all : {false, true}) {
      const auto scope = all ? QStringLiteral("all-editors") : QStringLiteral("filter-only");
      QTest::newRow(qUtf8Printable(scope + QStringLiteral("-cancel"))) << false << false << all;
      QTest::newRow(qUtf8Printable(scope + QStringLiteral("-confirm"))) << true << false << all;
      QTest::newRow(qUtf8Printable(scope + QStringLiteral("-read-fails"))) << true << true << all;
    }
  }
  void confirmedDiscardRestoresAllEditors() {
    QFETCH(bool, confirm);
    QFETCH(bool, stagingReadFails);
    QFETCH(bool, allEditors);
    RecordingWmiOperations wmi;
    auto storeOwner = std::make_shared<MemoryFileStagingStore>();
    auto& store = *storeOwner;
    store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\saved.bin")}};
    const auto saved = store.entries;
    MutableApplicationStateSource source;
    source.snapshot = editableSnapshot();
    source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
    ui::MainWindow window(bootstrap::mainWindowServices(wmi, source, storeOwner), {.uwfCapability = app::UwfCapability::Available, .hubVisible = false});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* global = window.findChild<ui::GlobalStatusPanel*>();
    auto* volume = window.findChild<ui::VolumeTab*>();
    auto* staged = window.findChild<ui::FileStagingWidget*>();
    QVERIFY(global && volume && staged);
    const auto model = volume->viewModel().editor();
    QVERIFY(global->viewModel().importFilterEnabled(!source.snapshot.next.filter.enabled));
    if (allEditors) {
      QVERIFY(global->viewModel().importOverlayWarnMb(4000));
      QVERIFY(volume->viewModel().importProtect(!source.snapshot.next.volumes.front().isProtected));
      QCOMPARE(model->files()->importAdd(QStringLiteral("C:\\Data")), app::ExclusionImportOutcome::Applied);
      QCOMPARE(model->registry()->importAdd(QStringLiteral("HKLM\\SOFTWARE\\Pending")), app::ExclusionImportOutcome::Applied);
      model->registry()->enablePersistence(app::ExclusionEditor::PersistenceItem::DomainSecretKey);
      staged->viewModel().removePaths({saved.front().path});
    }
    auto* discard = buttonWithText(&window, ui::I18n::tr("Discard changes"));
    QVERIFY(discard && discard->isVisibleTo(&window));
    if (stagingReadFails) store.readFailure = QStringLiteral("read denied");
    bool answered = false;
    QTimer answer;
    connect(&answer, &QTimer::timeout, &window, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      if (!dialog) return;
      if (dialog->windowTitle() == ui::I18n::tr("Discard pending changes?")) {
        answered = true;
        auto* choice = buttonWithText(dialog, confirm ? ui::I18n::tr("Continue") : ui::I18n::tr("Cancel"));
        QVERIFY(choice);
        QTest::mouseClick(choice, Qt::LeftButton);
      } else {
        dialog->reject();
      }
    });
    answer.start(0);
    QTest::mouseClick(discard, Qt::LeftButton);
    answer.stop();
    QVERIFY(answered);
    const bool discarded = confirm && !(allEditors && stagingReadFails);
    const bool otherEditsRemain = allEditors && !discarded;
    QCOMPARE(global->viewModel().pendingFilterEnabled().has_value(), !discarded);
    QCOMPARE(global->viewModel().pendingOverlay().empty(), !otherEditsRemain);
    QCOMPARE(model->status()->pendingProtection().has_value(), otherEditsRemain);
    QCOMPARE(model->files()->pendingAdded().isEmpty(), !otherEditsRemain);
    QCOMPARE(model->registry()->pendingAdded().isEmpty(), !otherEditsRemain);
    QCOMPARE(model->registry()->pendingPersistDomainSecretKey().has_value(), otherEditsRemain);
    QTRY_COMPARE(staged->viewModel().entries().isEmpty(), otherEditsRemain);
    QCOMPARE(discard->isVisibleTo(&window), !discarded);
    QCOMPARE(store.entries, saved);
    QCOMPARE(store.writes, 0);
    QCOMPARE(source.reads, 1);  // Discard restores the committed snapshot, without another provider read.
    QVERIFY(wmi.invocations.empty());
  }
};
}  // namespace
QTEST_MAIN(ConfigurationLifecycleUiTests)
#include "ConfigurationLifecycleUiTests.moc"
