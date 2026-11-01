// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QScopeGuard>
#include <QTest>

#include "application/staging/editing/FileStagingEditor.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/views/configuration/exclusions/ExclusionListWidget.h"
#include "presentation/views/staging/FileStagingWidget.h"
#include "support/ui/LocalFileDialogs.h"
#include "support/ui/StagingControls.h"

namespace {
class RecoveringStagingStore final : public uwf::app::FileStagingStore {
 public:
  bool failRead = true;
  QList<uwf::app::FileStagingEntry> load() const override {
    if (failRead) throw std::runtime_error("storage unavailable\nregistry access denied");
    return {{uwf::app::FileStagingKind::File, QStringLiteral("C:\\restored.bin")}};
  }
  void replace(const QList<uwf::app::FileStagingEntry>&) override {}
};
class FileStagingViewTests final : public QObject {
  Q_OBJECT

 private slots:
  void removedPathsUseTheSameRestorePresentation() {
    using namespace uwf;
    auto& language = ui::I18n::instance();
    const auto previousLanguage = language.lang();
    const auto restoreLanguage = qScopeGuard([&] { language.setLang(previousLanguage); });
    language.setLang(ui::I18n::Lang::En);
    auto store = std::make_shared<RecoveringStagingStore>();
    store->failRead = false;
    app::FileStagingDraft draft(store);
    auto stagingEditor = std::make_shared<app::FileStagingEditor>(QStringLiteral("C:"), draft, windows::systemCommitTraversalSource());
    auto stagingModel = std::make_shared<presentation::FileStagingViewModel>(stagingEditor);
    ui::FileStagingWidget staging(stagingModel, test::localFileDialogs());
    auto exclusionModel = std::make_shared<presentation::ExclusionListViewModel>(app::ExclusionEditor::Kind::File);
    exclusionModel->setDriveLetter(QStringLiteral("C:"));
    const QString path = QStringLiteral("C:\\restored.bin");
    exclusionModel->setBaseline({path}, {path});
    ui::ExclusionListWidget exclusions(exclusionModel, test::localFileDialogs());
    staging.resize(600, 350);
    exclusions.resize(600, 350);
    staging.show();
    exclusions.show();
    QVERIFY(QTest::qWaitForWindowExposed(&staging));
    QVERIFY(QTest::qWaitForWindowExposed(&exclusions));
    QTRY_VERIFY(!stagingModel->busy());
    auto* stagingList = test::stagingEntryList(staging);
    auto* exclusionList = exclusions.findChild<QListView*>(QStringLiteral("exclusionList"));
    auto* stagingRemove = staging.findChild<QPushButton*>(QStringLiteral("dangerBtn"));
    auto* exclusionRemove = exclusions.findChild<QPushButton*>(QStringLiteral("dangerBtn"));
    QVERIFY(stagingList && exclusionList && stagingRemove && exclusionRemove);
    const auto selectRow = [](QListView& list) {
      list.selectionModel()->select(list.model()->index(0, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    };
    auto& theme = ui::ThemeManager::instance();
    const auto previousTheme = theme.current();
    const auto restoreTheme = qScopeGuard([&] { theme.apply(previousTheme); });
    for (const auto mode : {app::Theme::Light, app::Theme::Dark}) {
      theme.apply(mode);
      selectRow(*stagingList);
      selectRow(*exclusionList);
      QVERIFY(stagingRemove->isEnabled() && exclusionRemove->isEnabled());
      QCOMPARE(stagingRemove->property("designRole").toString(), QStringLiteral("dangerQuiet"));
      QCOMPARE(exclusionRemove->property("designRole"), stagingRemove->property("designRole"));
      stagingRemove->click();
      exclusionRemove->click();
      QTRY_VERIFY(!stagingModel->busy());
      selectRow(*stagingList);
      selectRow(*exclusionList);
      QCOMPARE(stagingRemove->property("accessibleName").toString(), QStringLiteral("Undo removal"));
      QCOMPARE(exclusionRemove->property("accessibleName"), stagingRemove->property("accessibleName"));
      QCOMPARE(stagingRemove->property("designRole").toString(), QStringLiteral("quiet"));
      QCOMPARE(exclusionRemove->property("designRole"), stagingRemove->property("designRole"));
      staging.viewModel().setAvailability({.readOnly = true});
      exclusions.setReadOnly(true);
      QVERIFY(!stagingRemove->isEnabled() && !exclusionRemove->isEnabled());
      QCOMPARE(stagingRemove->property("designRole").toString(), QStringLiteral("quiet"));
      QCOMPARE(exclusionRemove->property("designRole"), stagingRemove->property("designRole"));
      staging.viewModel().setAvailability({});
      exclusions.setReadOnly(false);
      stagingRemove->click();
      exclusionRemove->click();
      QTRY_VERIFY(!stagingModel->busy());
      selectRow(*stagingList);
      selectRow(*exclusionList);
      QCOMPARE(stagingRemove->property("designRole").toString(), QStringLiteral("dangerQuiet"));
      QCOMPARE(exclusionRemove->property("designRole"), stagingRemove->property("designRole"));
    }
  }

  void storageRecoveryRestoresExistingControlsAndKeepsFailureTextOnThemeChanges() {
    auto storeOwner = std::make_shared<RecoveringStagingStore>();
    auto& store = *storeOwner;
    uwf::app::FileStagingDraft draft(storeOwner);
    auto editor = std::make_shared<uwf::app::FileStagingEditor>(QStringLiteral("C:"), draft, uwf::windows::systemCommitTraversalSource());
    auto model = std::make_shared<uwf::presentation::FileStagingViewModel>(editor);
    uwf::ui::FileStagingWidget view(model, uwf::test::localFileDialogs());
    view.resize(600, 350);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QTRY_VERIFY(!model->busy());
    auto* list = uwf::test::stagingEntryList(view);
    auto* filter = view.findChild<QLineEdit*>(QStringLiteral("filterInput"));
    auto* summary = view.findChild<QLabel*>(QStringLiteral("paginationSummary"));
    auto* add = view.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
    QVERIFY(list && filter && summary && add);
    QVERIFY(summary->isVisibleTo(&view));
    QVERIFY(!list->isEnabled() && !filter->isEnabled() && !add->isEnabled());
    const QString failure = summary->text();
    QVERIFY(!failure.isEmpty());
    QVERIFY(!failure.contains(u'\n'));
    QVERIFY(summary->toolTip().contains(model->storageError()));
    const auto size = view.size();
    auto& theme = uwf::ui::ThemeManager::instance();
    const auto previous = theme.current();
    const auto restoreTheme = qScopeGuard([&] { theme.apply(previous); });
    theme.toggle();
    QCOMPARE(summary->text(), failure);
    theme.apply(previous);
    store.failRead = false;
    model->load();
    QTRY_VERIFY(!model->busy());
    QVERIFY(list->isEnabled() && filter->isEnabled() && add->isEnabled());
    QCOMPARE(list->model()->rowCount(), 1);
    QVERIFY(model->storageError().isEmpty());
    QVERIFY(summary->text() != failure);
    QCOMPARE(view.size(), size);
  }
};

}  // namespace

QTEST_MAIN(FileStagingViewTests)
#include "FileStagingViewTests.moc"
