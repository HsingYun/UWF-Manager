// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QItemSelectionModel>
#include <QListView>
#include <QMenu>
#include <QPushButton>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <QTimer>
#include <atomic>
#include <memory>
#include <utility>

#include "application/configuration/exclusions/ExclusionEditor.h"
#include "application/files/contracts/FileBrowser.h"
#include "application/staging/editing/FileStagingConflictPolicy.h"
#include "presentation/appearance/I18n.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/models/PageProxyModel.h"
#include "presentation/viewmodels/configuration/ExclusionListViewModel.h"
#include "presentation/views/configuration/exclusions/ExclusionListWidget.h"
#include "support/app/WorkerGate.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"

namespace {
using namespace uwf;
using Editor = app::ExclusionEditor;

class MenuFileBrowser final : public app::FileBrowser {
 public:
  QThread* uiThread = QThread::currentThread();
  std::shared_ptr<test::WorkerGate> gate = std::make_shared<test::WorkerGate>();
  mutable std::atomic_int reads{0}, uiReads{0};
  app::FileBrowserEntry inspect(const QString& path) const override {
    const auto call = ++reads;
    if (QThread::currentThread() == uiThread) ++uiReads;
    if (call == 1) gate->wait({}, false);
    return {.path = path, .name = {}, .directory = false, .size = 0, .modified = {}, .exists = true};
  }
  QVector<app::FileBrowserEntry> list(const QString&, std::stop_token) const override { throw std::logic_error("Unexpected directory read"); }
  QVector<app::FileBrowserPlace> places() const override { return {}; }
  void createDirectory(const QString&) const override { throw std::logic_error("Unexpected write"); }
};

struct Fixture {
  bool stagingRequired;
  test::FileDialogStub dialogs;
  std::shared_ptr<Editor> editor;
  std::shared_ptr<presentation::ExclusionListViewModel> model;
  ui::ExclusionListWidget widget;
  QString path;
  QPushButton* remove = nullptr;
  QListView* list = nullptr;

  explicit Fixture(const Editor::Kind kind, const bool required = false, app::FileStagingConflictPolicy* conflicts = nullptr)
      : stagingRequired(required),
        editor(std::make_shared<Editor>(kind, conflicts)),
        model(std::make_shared<presentation::ExclusionListViewModel>(editor)),
        widget(model, dialogs) {
    if (kind == Editor::Kind::File) {
      model->setDriveLetter(QStringLiteral("Z:"));
      path = QStringLiteral("Z:\\Data\\settings.bin");
    } else {
      editor->setFileStagingDataProbe([this] { return stagingRequired; });
      path = required ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager") : QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\OrdinaryFixture");
    }
    model->setBaseline({path}, {path});
    widget.resize(640, 320);
    widget.show();
    QCoreApplication::processEvents();
    remove = widget.findChild<QPushButton*>(QStringLiteral("dangerBtn"));
    list = widget.findChild<QListView*>(QStringLiteral("exclusionList"));
  }

  bool selectEntry() {
    if (!list) return false;
    for (int row = 0; row < list->model()->rowCount(); ++row) {
      const auto index = list->model()->index(row, 0);
      if (index.data(presentation::PathRole).toString().compare(path, Qt::CaseInsensitive) == 0) {
        list->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        return true;
      }
    }
    return false;
  }
};

class ExclusionRemovalHintTests final : public QObject {
  Q_OBJECT

 private slots:
  void contextMenuWaitsForMetadataWithoutReadingOnTheUiThread() {
    auto browser = std::make_shared<MenuFileBrowser>();
    const auto release = qScopeGuard([browser] { browser->gate->release(); });
    auto editor = std::make_shared<Editor>(Editor::Kind::File, nullptr, browser);
    auto model = std::make_shared<presentation::ExclusionListViewModel>(editor);
    const QString path = QStringLiteral("Z:\\Data\\ready.bin");
    model->setDriveLetter(QStringLiteral("Z:"));
    model->setBaseline({}, {path});
    model->setCommitAvailability(true);
    test::FileDialogStub dialogs;
    ui::ExclusionListWidget widget(model, dialogs);
    widget.resize(640, 320);
    widget.show();
    auto* list = widget.findChild<QListView*>(QStringLiteral("exclusionList"));
    QVERIFY(list);
    QTRY_COMPARE(browser->gate->started.load(), 1);
    const auto index = list->model()->index(0, 0);
    list->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    QSignalSpy requested(&widget, &ui::ExclusionListWidget::commitFileRequested);
    bool initiallyDisabled = false;
    bool becameEnabled = false;
    QTimer::singleShot(0, &widget, [&] {
      auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
      if (!menu) {
        browser->gate->release();
        return;
      }
      const auto close = qScopeGuard([menu] { menu->close(); });
      QAction* commit = nullptr;
      for (auto actions = menu->actions(); auto* action : std::as_const(actions))
        if (action->text() == ui::I18n::tr("Commit file changes to disk…")) commit = action;
      initiallyDisabled = commit && !commit->isEnabled();
      browser->gate->release();
      QVERIFY(commit);
      QTRY_VERIFY(commit->isEnabled());
      becameEnabled = true;
      commit->trigger();
    });
    const auto point = list->visualRect(index).center();
    QVERIFY(QMetaObject::invokeMethod(list, "customContextMenuRequested", Qt::DirectConnection, Q_ARG(QPoint, point)));
    QVERIFY(initiallyDisabled);
    QVERIFY(becameEnabled);
    QCOMPARE(requested.size(), 1);
    QCOMPARE(requested.front().front().toString(), path);
    QCOMPARE(browser->reads.load(), 1);
    QCOMPARE(browser->uiReads.load(), 0);
  }

  void initTestCase() { ui::I18n::instance().setLang(ui::I18n::Lang::En); }
  void cleanup() { ui::I18n::instance().setLang(ui::I18n::Lang::En); }

  void containingFolderRequiresAnAddressedVolume_data() {
    QTest::addColumn<QString>("drive");
    QTest::newRow("addressed-read-only") << QStringLiteral("Z:");
    QTest::newRow("unlettered-read-only") << QString{};
  }
  void containingFolderRequiresAnAddressedVolume() {
    QFETCH(QString, drive);
    Fixture fixture(Editor::Kind::File);
    fixture.model->setDriveLetter(drive);
    fixture.widget.setReadOnly(true);
    QVERIFY(fixture.selectEntry());
    QCOMPARE(fixture.model->canBrowseFiles(), !drive.isEmpty());
    bool observed = false;
    bool enabled = false;
    QTimer::singleShot(0, &fixture.widget, [&] {
      auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
      if (!menu) return;
      for (auto actions = menu->actions(); auto* action : std::as_const(actions))
        if (action->text() == ui::I18n::tr("Open containing folder")) {
          observed = true;
          enabled = action->isEnabled();
        }
      menu->close();
    });
    const auto index = fixture.list->selectionModel()->selectedRows().front();
    const auto point = fixture.list->visualRect(index).center();
    QVERIFY(QMetaObject::invokeMethod(fixture.list, "customContextMenuRequested", Qt::DirectConnection, Q_ARG(QPoint, point)));
    QVERIFY(observed);
    QCOMPARE(enabled, !drive.isEmpty());
    QVERIFY(fixture.dialogs.requests.isEmpty());
    QVERIFY(fixture.model->pendingAdded().isEmpty());
    QVERIFY(fixture.model->pendingRemoved().isEmpty());
  }

  void volumeRegistrationControlsAddAndCurrentOnlyRestore_data() {
    QTest::addColumn<bool>("nextRegistered");
    QTest::newRow("current-only") << false;
    QTest::newRow("registered-next") << true;
  }
  void volumeRegistrationControlsAddAndCurrentOnlyRestore() {
    QFETCH(bool, nextRegistered);
    Fixture fixture(Editor::Kind::File);
    fixture.model->setBaseline({fixture.path}, nextRegistered ? QStringList{fixture.path} : QStringList{});
    fixture.editor->setFileAddTargetAvailable(nextRegistered);
    auto* add = fixture.widget.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
    QVERIFY(add);
    QCOMPARE(add->isEnabled(), nextRegistered);
    QVERIFY(fixture.list->isEnabled());
    QVERIFY(fixture.selectEntry());
    if (nextRegistered) {
      QVERIFY(fixture.remove->isEnabled());
      fixture.remove->click();
      QCOMPARE(fixture.model->pendingRemoved(), QStringList{fixture.path});
      QVERIFY(fixture.selectEntry());
      QVERIFY(fixture.remove->isEnabled());
    } else {
      QVERIFY(!fixture.remove->isEnabled());
      const auto reason = QStringLiteral("Enable volume protection in this draft before adding file exclusions.");
      QCOMPARE(add->toolTip(), reason);
      QCOMPARE(fixture.remove->toolTip(), reason);
      // The owner publishes eligibility when Protect enters the same draft.
      fixture.editor->setFileAddTargetAvailable(true);
      QVERIFY(add->isEnabled());
      QVERIFY(fixture.remove->isEnabled());
    }
    fixture.remove->click();
    QVERIFY(fixture.model->pendingRemoved().isEmpty());
    QCOMPARE(fixture.model->pendingAdded(), nextRegistered ? QStringList{} : QStringList{fixture.path});
    if (!nextRegistered) {
      fixture.editor->setFileAddTargetAvailable(false);
      QVERIFY(!add->isEnabled());
      QCOMPARE(fixture.model->pendingAdded(), QStringList{fixture.path});
    }
  }

  void hintExplainsTheActualAvailability_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("required");
    QTest::addColumn<bool>("selected");
    QTest::addColumn<bool>("readOnly");
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<QString>("tooltip");
    const QString noSelection = QStringLiteral("Select entries to remove.");
    const QString writable = QStringLiteral("Remove the selected entries from the exclusion list. Takes effect after Apply.");
    const QString readonly = QStringLiteral("Read-only");
    const QString dependency =
        QStringLiteral("This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.");
    QTest::newRow("file-none") << false << false << false << false << false << noSelection;
    QTest::newRow("file-selected") << false << false << true << false << true << writable;
    QTest::newRow("file-readonly-none") << false << false << false << true << false << readonly;
    QTest::newRow("file-readonly-selected") << false << false << true << true << false << readonly;
    QTest::newRow("registry-none") << true << false << false << false << false << noSelection;
    QTest::newRow("registry-selected") << true << false << true << false << true << writable;
    QTest::newRow("required-registry-none") << true << true << false << false << false << noSelection;
    QTest::newRow("required-registry-selected") << true << true << true << false << false << dependency;
    QTest::newRow("required-registry-readonly") << true << true << true << true << false << readonly;
  }

  void hintExplainsTheActualAvailability() {
    QFETCH(bool, registry);
    QFETCH(bool, required);
    QFETCH(bool, selected);
    QFETCH(bool, readOnly);
    QFETCH(bool, enabled);
    QFETCH(QString, tooltip);
    Fixture fixture(registry ? Editor::Kind::Registry : Editor::Kind::File, required);
    QVERIFY(fixture.remove && fixture.list);
    if (selected) QVERIFY(fixture.selectEntry());
    fixture.widget.setReadOnly(readOnly);
    QCOMPARE(fixture.remove->isEnabled(), enabled);
    QCOMPARE(fixture.remove->toolTip(), tooltip);
  }

  void changingSelectionAndDependencyRefreshesTheSameHint() {
    Fixture fixture(Editor::Kind::Registry, true);
    QVERIFY(fixture.remove && fixture.list);
    QVERIFY(fixture.selectEntry());
    QVERIFY(!fixture.remove->isEnabled());
    QVERIFY(fixture.remove->toolTip().contains(QStringLiteral("File staging contains saved entries")));
    fixture.stagingRequired = false;
    fixture.editor->refreshFileStagingConstraints();
    QVERIFY(fixture.remove->isEnabled());
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("Remove the selected entries from the exclusion list. Takes effect after Apply."));
    fixture.stagingRequired = true;
    fixture.editor->refreshFileStagingConstraints();
    QVERIFY(!fixture.remove->isEnabled());
    fixture.list->clearSelection();
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("Select entries to remove."));
  }

  void restorationConflictsRemainExplainedUntilStagingChanges_data() {
    QTest::addColumn<bool>("undo");
    QTest::addColumn<bool>("chinese");
    for (const bool undo : {false, true})
      for (const bool chinese : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("undo-%1-chinese-%2").arg(undo).arg(chinese))) << undo << chinese;
  }
  void restorationConflictsRemainExplainedUntilStagingChanges() {
    QFETCH(bool, undo);
    QFETCH(bool, chinese);
    app::FileStagingConflictPolicy conflicts;
    Fixture fixture(Editor::Kind::File, false, &conflicts);
    QVERIFY(fixture.remove && fixture.list);
    fixture.model->setBaseline(undo ? QStringList{} : QStringList{fixture.path}, undo ? QStringList{fixture.path} : QStringList{});
    QVERIFY(fixture.selectEntry());
    if (undo) {
      fixture.remove->click();
      QCOMPARE(fixture.editor->pendingRemoved(), QStringList{fixture.path});
      QVERIFY(fixture.selectEntry());
    }
    const auto selected = fixture.list->selectionModel()->selectedRows();
    QVERIFY(fixture.remove->isEnabled());
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("Undo removal"));
    // A changed staged set updates eligibility without replacing the selected
    // rows or relying on a view to push editing state back into the model.
    conflicts.setStagedEntries({{app::FileStagingKind::File, fixture.path}});
    QVERIFY(!fixture.remove->isEnabled());
    const auto previousRemoved = fixture.editor->pendingRemoved();
    ui::I18n::instance().setLang(chinese ? ui::I18n::Lang::Zh_CN : ui::I18n::Lang::En);
    const auto reason = ui::I18n::tr("This path overlaps a file staging entry. Remove the staged path before adding the exclusion:\n%1").arg(fixture.path);
    QCOMPARE(fixture.remove->toolTip(), reason);
    QCOMPARE(fixture.list->selectionModel()->selectedRows(), selected);
    QVERIFY(!fixture.remove->isEnabled());
    fixture.remove->click();
    QCOMPARE(fixture.editor->pendingRemoved(), previousRemoved);
    QVERIFY(fixture.editor->pendingAdded().isEmpty());

    // A delayed action revalidates the same state and presents the same reason.
    bool failureShown = false;
    QVERIFY(test::driveModalDialog(
        fixture.widget, [&] { QVERIFY(QMetaObject::invokeMethod(&fixture.widget, "onRemove", Qt::DirectConnection)); },
        [&](QDialog& dialog) {
          const auto* body = dialog.findChild<ui::ContentHintBox*>();
          failureShown = dialog.windowTitle() == ui::I18n::tr("Undo removal") && body && body->toPlainText() == reason;
          dialog.reject();
        }));
    QVERIFY(failureShown);
    QCOMPARE(fixture.editor->pendingRemoved(), previousRemoved);
    QVERIFY(fixture.editor->pendingAdded().isEmpty());
    QCOMPARE(fixture.list->selectionModel()->selectedRows(), selected);
    conflicts.setStagedEntries({});
    QVERIFY(fixture.remove->isEnabled());
    QCOMPARE(fixture.remove->toolTip(), ui::I18n::tr("Undo removal"));
    fixture.remove->click();
    QVERIFY(fixture.editor->pendingRemoved().isEmpty());
    QCOMPARE(fixture.editor->pendingAdded(), undo ? QStringList{} : QStringList{fixture.path});
  }

  void readOnlyRestoreRemainsDisabledAcrossLanguageChanges() {
    Fixture fixture(Editor::Kind::File);
    QVERIFY(fixture.remove && fixture.list);
    QVERIFY(fixture.selectEntry());
    fixture.remove->click();
    QVERIFY(fixture.selectEntry());
    const auto selected = fixture.list->selectionModel()->selectedRows();
    fixture.widget.setReadOnly(true);
    QVERIFY(!fixture.remove->isEnabled());
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("Read-only"));
    auto& language = ui::I18n::instance();
    language.setLang(ui::I18n::Lang::Zh_CN);
    QCOMPARE(language.lang(), ui::I18n::Lang::Zh_CN);
    QVERIFY(!fixture.remove->isEnabled());
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("只读"));
    QCOMPARE(fixture.list->selectionModel()->selectedRows(), selected);
    QCOMPARE(fixture.editor->pendingRemoved(), QStringList{fixture.path});
    fixture.widget.setReadOnly(false);
    QVERIFY(fixture.remove->isEnabled());
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("撤销移除"));
    fixture.remove->click();
    QVERIFY(fixture.editor->pendingRemoved().isEmpty());
  }

  void readOnlyChangesPreservePaginationAvailability_data() {
    QTest::addColumn<bool>("registry");
    QTest::newRow("files") << false;
    QTest::newRow("registry") << true;
  }
  void readOnlyChangesPreservePaginationAvailability() {
    QFETCH(bool, registry);
    Fixture fixture(registry ? Editor::Kind::Registry : Editor::Kind::File);
    QStringList entries;
    for (int index = 0; index < 64; ++index)
      entries.append(registry ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Fixture%1").arg(index) : QStringLiteral("Z:\\Data\\entry%1.txt").arg(index));
    fixture.model->setBaseline(entries, entries);
    QCoreApplication::processEvents();
    auto& pages = static_cast<ui::PathEntryListView*>(fixture.list)->pages();
    pages.setPageSize(5);
    const auto pageButton = [&](const QString& name) -> QPushButton* {
      for (auto buttons = fixture.widget.findChildren<QPushButton*>(); auto* button : std::as_const(buttons))
        if (button->property("accessibleName").toString() == name) return button;
      return nullptr;
    };
    auto* first = pageButton(QStringLiteral("First page"));
    auto* previous = pageButton(QStringLiteral("Previous page"));
    auto* next = pageButton(QStringLiteral("Next page"));
    auto* last = pageButton(QStringLiteral("Last page"));
    auto* add = fixture.widget.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
    QVERIFY(first && previous && next && last && add);
    QVERIFY(!first->isEnabled());
    QVERIFY(!previous->isEnabled());
    QVERIFY(next->isEnabled());
    QVERIFY(last->isEnabled());
    fixture.widget.setReadOnly(true);
    QVERIFY(!add->isEnabled());
    QVERIFY(!fixture.remove->isEnabled());
    QVERIFY(next->isEnabled());
    QVERIFY(last->isEnabled());
    next->click();
    QTRY_VERIFY(pages.pager().hasPrev());
    QVERIFY(first->isEnabled());
    QVERIFY(previous->isEnabled());
    last->click();
    QTRY_VERIFY(!pages.pager().hasNext(pages.totalRows()));
    const auto finalPage = pages.pager().currentPage;
    QVERIFY(!next->isEnabled());
    QVERIFY(!last->isEnabled());
    fixture.widget.setReadOnly(false);
    QVERIFY(add->isEnabled());
    QCOMPARE(pages.pager().currentPage, finalPage);
    QVERIFY(!next->isEnabled());
    QVERIFY(!last->isEnabled());
    first->click();
    QTRY_VERIFY(!pages.pager().hasPrev());
    for (const bool readOnly : {true, false}) {
      fixture.widget.setReadOnly(readOnly);
      QVERIFY(!first->isEnabled());
      QVERIFY(!previous->isEnabled());
      QVERIFY(next->isEnabled());
      QVERIFY(last->isEnabled());
    }
    QVERIFY(fixture.model->pendingAdded().isEmpty());
    QVERIFY(fixture.model->pendingRemoved().isEmpty());
  }

  void aNewDependencyStillBlocksRemovalAtExecution() {
    Fixture fixture(Editor::Kind::Registry, true);
    QVERIFY(fixture.remove && fixture.list);
    fixture.stagingRequired = false;
    QVERIFY(fixture.selectEntry());
    QVERIFY(fixture.remove->isEnabled());
    fixture.stagingRequired = true;
    fixture.remove->click();
    QVERIFY(fixture.editor->pendingRemoved().isEmpty());
    QVERIFY(!fixture.remove->isEnabled());
    QVERIFY(fixture.remove->toolTip().contains(QStringLiteral("File staging contains saved entries")));
  }
};
}  // namespace

QTEST_MAIN(ExclusionRemovalHintTests)
#include "ExclusionRemovalHintTests.moc"
