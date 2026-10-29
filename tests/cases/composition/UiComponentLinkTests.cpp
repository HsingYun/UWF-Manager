// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QApplication>
#include <QListView>
#include <QTableView>
#include <QTest>

#include "app/api/LogSource.h"
#include "presentation/configuration/ExclusionListViewModel.h"
#include "support/ui/FileDialogStub.h"
#include "ui/configuration/ExclusionListWidget.h"
#include "ui/diagnostics/LogViewerDialog.h"
#include "ui/dialogs/Dialogs.h"

namespace {
class MemoryLogSource final : public uwf::app::LogSource {
 public:
  std::vector<std::string> recentLines() const override { return {"[12:34:56.789 I test] injected log source"}; }
  void clear() override {}
};

class UiComponentLinkTests final : public QObject {
  Q_OBJECT
 private slots:
  void injectedViewsLinkAndRunWithoutBootstrap() {
    auto exclusions = std::make_shared<uwf::presentation::ExclusionListViewModel>(uwf::presentation::ExclusionListViewModel::Kind::File);
    uwf::test::FileDialogStub files;
    uwf::ui::ExclusionListWidget view(exclusions, files);
    exclusions->setBaseline({QStringLiteral("C:\\Data")}, {QStringLiteral("C:\\Data")});
    const auto* list = view.findChild<QListView*>();
    QVERIFY(list);
    QCOMPARE(list->model()->rowCount(), 1);
    auto log = std::make_shared<uwf::presentation::LogViewModel>(std::make_shared<MemoryLogSource>());
    uwf::ui::LogViewerDialog dialog(log);
    const auto* table = dialog.findChild<QTableView*>();
    QVERIFY(table);
    QTRY_COMPARE(table->model()->rowCount(), 1);
    QCOMPARE(table->model()->index(0, 3).data().toString(), QStringLiteral("injected log source"));
  }
};
}  // namespace
QTEST_MAIN(UiComponentLinkTests)
#include "UiComponentLinkTests.moc"
