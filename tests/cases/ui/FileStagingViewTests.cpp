// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QScopeGuard>
#include <QTest>

#include "app/staging/FileStagingEditor.h"
#include "support/ui/LocalFileDialogs.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/staging/FileStagingWidget.h"
#include "windows/commit/WindowsCommitTraversalSource.h"

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
    auto* list = view.findChild<QListView*>();
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
