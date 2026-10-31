// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QApplication>
#include <QDialog>
#include <QItemSelectionModel>
#include <QListView>
#include <QPushButton>
#include <QTest>
#include <QTimer>
#include <memory>

#include "app/configuration/ExclusionEditor.h"
#include "app/staging/FileStagingConflictPolicy.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/configuration/ExclusionListViewModel.h"
#include "support/ui/FileDialogStub.h"
#include "ui/appearance/I18n.h"
#include "ui/configuration/ExclusionListWidget.h"
#include "ui/widgets/ContentHintBox.h"

namespace {
using namespace uwf;
using Editor = app::ExclusionEditor;

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
      : stagingRequired(required), editor(std::make_shared<Editor>(kind, conflicts)),
        model(std::make_shared<presentation::ExclusionListViewModel>(editor)), widget(model, dialogs) {
    if (kind == Editor::Kind::File) {
      widget.setDriveLetter(QStringLiteral("Z:"));
      path = QStringLiteral("Z:\\Data\\settings.bin");
    } else {
      editor->setFileStagingDataProbe([this] { return stagingRequired; });
      path = required ? QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager")
                      : QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\OrdinaryFixture");
    }
    widget.setBaseline({path}, {path});
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
  void initTestCase() { ui::I18n::instance().setLang(ui::I18n::Lang::En); }
  void cleanup() { ui::I18n::instance().setLang(ui::I18n::Lang::En); }

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
    const QString dependency = QStringLiteral(
        "This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.");
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

  void removalAndUndoKeepTheirExistingActions() {
    Fixture fixture(Editor::Kind::File);
    QVERIFY(fixture.remove && fixture.list);
    QVERIFY(fixture.selectEntry());
    QVERIFY(fixture.remove->isEnabled());
    fixture.remove->click();
    QCOMPARE(fixture.editor->pendingRemoved(), QStringList{fixture.path});
    QVERIFY(fixture.selectEntry());
    QVERIFY(fixture.remove->isEnabled());
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("Undo removal"));
    fixture.remove->click();
    QVERIFY(fixture.editor->pendingRemoved().isEmpty());
    QVERIFY(fixture.editor->pendingAdded().isEmpty());
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("Select entries to remove."));
  }

  void restoreFailuresRemainRetryableAcrossLanguageChanges() {
    app::FileStagingConflictPolicy conflicts;
    Fixture fixture(Editor::Kind::File, false, &conflicts);
    QVERIFY(fixture.remove && fixture.list);
    QVERIFY(fixture.selectEntry());
    fixture.remove->click();
    QCOMPARE(fixture.editor->pendingRemoved(), QStringList{fixture.path});
    conflicts.setStagedEntries({{app::FileStagingKind::File, fixture.path}});
    QVERIFY(fixture.selectEntry());
    const auto selected = fixture.list->selectionModel()->selectedRows();
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("Undo removal"));

    const auto tryRestore = [&](const QString& title, const QString& message) {
      bool failureShown = false;
      QTimer::singleShot(0, &fixture.widget, [&failureShown, title, message] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (!dialog) return;
        const auto* body = dialog->findChild<ui::ContentHintBox*>();
        failureShown = dialog->windowTitle() == title && body && body->toPlainText() == message;
        dialog->reject();
      });
      fixture.remove->click();
      QCoreApplication::processEvents();
      return failureShown;
    };
    QVERIFY(tryRestore(QStringLiteral("Undo removal"), QStringLiteral("Selected entries cannot currently be restored.")));
    QCOMPARE(fixture.editor->pendingRemoved(), QStringList{fixture.path});
    QCOMPARE(fixture.list->selectionModel()->selectedRows(), selected);
    QVERIFY(fixture.remove->isEnabled());
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("Undo removal"));

    auto& language = ui::I18n::instance();
    language.setLang(ui::I18n::Lang::Zh_CN);
    QCOMPARE(language.lang(), ui::I18n::Lang::Zh_CN);
    QCOMPARE(fixture.list->selectionModel()->selectedRows(), selected);
    QCOMPARE(fixture.editor->pendingRemoved(), QStringList{fixture.path});
    QVERIFY(fixture.remove->isEnabled());
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("撤销移除"));
    QVERIFY(tryRestore(QStringLiteral("撤销移除"), QStringLiteral("当前无法恢复所选条目。")));
    QCOMPARE(fixture.editor->pendingRemoved(), QStringList{fixture.path});
    QCOMPARE(fixture.list->selectionModel()->selectedRows(), selected);
    QVERIFY(fixture.remove->isEnabled());
    QCOMPARE(fixture.remove->toolTip(), QStringLiteral("撤销移除"));

    conflicts.setStagedEntries({});
    QVERIFY(!tryRestore(QStringLiteral("撤销移除"), QStringLiteral("当前无法恢复所选条目。")));
    QVERIFY(fixture.editor->pendingRemoved().isEmpty());
    QVERIFY(fixture.editor->pendingAdded().isEmpty());
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
