// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QAbstractItemModelTester>
#include <QApplication>
#include <QCoreApplication>
#include <QDialog>
#include <QEvent>
#include <QHBoxLayout>
#include <QImage>
#include <QItemSelectionModel>
#include <QLabel>
#include <QListView>
#include <QPersistentModelIndex>
#include <QPoint>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSize>
#include <QStandardItemModel>
#include <QString>
#include <QTableView>
#include <QTest>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

#include "cases/presentation/views/widgets/WidgetUiFixture.h"
#include "domain/uwf/model/UwfModel.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/inputs/CodeEditor.h"
#include "presentation/controls/layout/FlexibleColumnLayout.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/layout/VerticalScrollArea.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/navigation/TabView.h"
#include "presentation/controls/pagination/PagedWindowLayout.h"
#include "presentation/controls/tables/PagedTableView.h"
#include "presentation/controls/tables/PathEntryControls.h"
#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/controls/tables/RecyclingListView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogLayout.h"
#include "presentation/dialogs/PanelDialog.h"
#include "presentation/models/PageProxyModel.h"
#include "presentation/views/configuration/review/ApplyPlanDialog.h"
#include "simulation/ui/FileDialogStub.h"

using namespace uwf::test::widgets;
using uwf::simulation::FileDialogStub;

namespace {
QString bottomPaintError(const QImage& frame, const QWidget& window, const uwf::ui::PathEntryListView& list) {
  if (frame.isNull()) return QStringLiteral("The displayed window could not be captured.");
  const QPoint origin = list.mapTo(&window, QPoint());
  const qreal scale = frame.devicePixelRatio();
  const int bottom = qCeil((origin.y() + list.height()) * scale) - 1;
  const int left = qFloor(origin.x() * scale);
  const int right = qCeil((origin.x() + list.width()) * scale) - 1;
  const int middle = qFloor((origin.x() + list.width() / 2.0) * scale);
  if (!frame.rect().contains(QPoint(left, bottom)) || !frame.rect().contains(QPoint(right, bottom)))
    return QStringLiteral("The list bottom edge lies outside the displayed window.");
  const auto closeColor = [](const QColor& actual, const QColor& expected, const int tolerance) {
    return std::abs(actual.red() - expected.red()) <= tolerance && std::abs(actual.green() - expected.green()) <= tolerance &&
           std::abs(actual.blue() - expected.blue()) <= tolerance;
  };
  const auto& design = uwf::ui::DesignSystem::instance();
  const QColor expectedEdge = design.color(uwf::ui::design::Color::Line);
  // Fractional scaling can put the final raster row outside the list's clip.
  // Sample its last logical pixel so antialiasing does not hide the border.
  QColor edge;
  bool borderPainted = false;
  for (int row = qFloor((origin.y() + list.height() - 1) * scale); row <= bottom; ++row) {
    edge = frame.pixelColor(middle, row);
    if (closeColor(edge, expectedEdge, 12)) {
      borderPainted = true;
      break;
    }
  }
  if (!borderPainted)
    return QStringLiteral("Missing painted bottom border: actual=%1 expected=%2 listHeight=%3 viewportHeight=%4")
        .arg(edge.name(), expectedEdge.name())
        .arg(list.height())
        .arg(list.viewport()->height());
  const QColor exterior = design.color(uwf::ui::design::Color::Window);
  if (!closeColor(frame.pixelColor(left, bottom), exterior, 3) || !closeColor(frame.pixelColor(right, bottom), exterior, 3))
    return QStringLiteral("The painted bottom corners do not reveal the exterior background.");
  return {};
}

class PaintCounter final : public QObject {
 public:
  int paints = 0;

 protected:
  bool eventFilter(QObject*, QEvent* event) override {
    if (event->type() == QEvent::Paint) ++paints;
    return false;
  }
};

class WidgetPaginationTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() { initializeWidgets(); }
  void largeApplyPlansRecycleRowsAndKeepCommandsAccessible();
  void recycledRowsFollowModelUpdatesAndReset();
  void recycledRowsReflowWhenScrollbarsChangeTheViewportWidth_data();
  void recycledRowsReflowWhenScrollbarsChangeTheViewportWidth();
  void entryListsScrollContinuouslyAndKeepPagination();
  void entryListBottomBorderTracksInteriorReflowWithoutViewportResize_data();
  void entryListBottomBorderTracksInteriorReflowWithoutViewportResize();
  void entryListBottomCornersPaintWhenHiddenRegistryFirstAppears();
  void pagedTablesScrollContinuouslyWithoutAllocatingRowWidgets();
  void pagedViewportsKeepWholeRowsAfterInteriorReflow_data();
  void pagedViewportsKeepWholeRowsAfterInteriorReflow();
  void pagedTablesKeepEnterActivationInsideTheView();
  void hiddenTabPagesDoNotPushReportsOutsideTheViewport();
};

void WidgetPaginationTests::largeApplyPlansRecycleRowsAndKeepCommandsAccessible() {
  using namespace uwf;
  class Actions final : public app::ConfigurationActions {
   public:
    int writes = 0;
    ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
      ++writes;
      return [] { return app::ConfigurationApplyResult{}; };
    }
  };
  auto actions = std::make_shared<Actions>();
  core::UwfSnapshot snapshot;
  snapshot.next.volumes.push_back({"Volume{11111111-2222-3333-4444-555555555555}", "C:", false, core::VolumeBinding::DriveLetter, std::string("C:")});
  core::PendingChanges pending;
  for (int row = 0; row < 1000; ++row) pending.addFileExclusions["C:"].push_back("C:\\Data\\entry-" + std::to_string(row));
  auto model = std::make_shared<presentation::ApplyPlanViewModel>(pending, snapshot, actions);
  QAbstractItemModelTester contract(model.get(), QAbstractItemModelTester::FailureReportingMode::QtTest);
  uwf::simulation::FileDialogStub files;
  ui::ApplyPlanDialog dialog(model, files);
  dialog.resize(920, 600);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* view = dialog.findChild<QListView*>(QStringLiteral("applyChangesList"));
  QVERIFY(view);
  QCOMPARE(view->model()->rowCount(), 1002);
  const auto boundedControls = [&] { return view->findChildren<QWidget*>().size() < 250; };
  QTRY_VERIFY(boundedControls());
  for (const int row : {1, 501, 1000, 21}) {
    const auto index = model->index(row);
    view->scrollTo(index, QAbstractItemView::PositionAtTop);
    const auto visibleText = [&] {
      for (auto labels = view->findChildren<ui::TextLabel*>(); auto* label : std::as_const(labels))
        if (label->isVisibleTo(view) && label->text() == index.data().toString()) return true;
      for (auto texts = view->findChildren<ui::ContentHintBox*>(); auto* text : std::as_const(texts))
        if (text->isVisibleTo(view) && text->toPlainText() == index.data().toString()) return true;
      return false;
    };
    QTRY_VERIFY(visibleText());
    QVERIFY(boundedControls());
  }
  const int beforeResize = view->verticalScrollBar()->value();
  dialog.resize(dialog.width() - 80, dialog.height());
  QCoreApplication::processEvents();
  QCOMPARE(view->verticalScrollBar()->value(), beforeResize);
  view->scrollTo(model->index(model->rowCount() - 1), QAbstractItemView::PositionAtBottom);
  auto* commands = dialog.findChild<ui::Disclosure*>(QStringLiteral("changeCommandsDisclosure"));
  QVERIFY(commands);
  QTRY_VERIFY(commands->isVisibleTo(view));
  const int position = view->verticalScrollBar()->value();
  commands->setOpen(true);
  QCoreApplication::processEvents();
  QCOMPARE(view->verticalScrollBar()->value(), position);
  QTRY_VERIFY(commands->findChild<ui::CodeEditor*>()->toPlainText().contains(QStringLiteral("entry-999")));
  QVERIFY(boundedControls());
  QCOMPARE(actions->writes, 0);
}

void WidgetPaginationTests::recycledRowsFollowModelUpdatesAndReset() {
  QStandardItemModel source;
  for (int row = 0; row < 100; ++row) source.appendRow(new QStandardItem(QStringLiteral("Entry %1").arg(row)));
  uwf::ui::RecyclingListView view(
      Qt::UserRole, [](int, QWidget* parent) { return new QLabel(parent); },
      [](QWidget* widget, const QModelIndex& index) { static_cast<QLabel*>(widget)->setText(index.data().toString()); },
      [](const QModelIndex&, const int width) { return QSize(width, 28); });
  view.setModel(&source);
  view.resize(400, 200);
  view.show();
  QVERIFY(QTest::qWaitForWindowExposed(&view));
  const auto hasText = [&](const QString& text) {
    for (auto labels = view.findChildren<QLabel*>(); auto* label : std::as_const(labels))
      if (label->isVisibleTo(&view) && label->text() == text) return true;
    return false;
  };
  QTRY_VERIFY(hasText(QStringLiteral("Entry 0")));
  source.setData(source.index(0, 0), QStringLiteral("Updated"));
  QTRY_VERIFY(hasText(QStringLiteral("Updated")));
  source.clear();
  source.appendRow(new QStandardItem(QStringLiteral("Reset")));
  QTRY_VERIFY(hasText(QStringLiteral("Reset")));
  QVERIFY(view.findChildren<QLabel*>().size() < 20);
}

void WidgetPaginationTests::recycledRowsReflowWhenScrollbarsChangeTheViewportWidth_data() {
  QTest::addColumn<bool>("rtl");
  QTest::newRow("ltr") << false;
  QTest::newRow("rtl") << true;
}

void WidgetPaginationTests::recycledRowsReflowWhenScrollbarsChangeTheViewportWidth() {
  QFETCH(bool, rtl);
  QStandardItemModel source;
  for (int row = 0; row < 6; ++row) source.appendRow(new QStandardItem(QStringLiteral("Entry %1").arg(row)));
  uwf::ui::RecyclingListView view(
      Qt::UserRole, [](int, QWidget* parent) { return new QLabel(parent); },
      [](QWidget* widget, const QModelIndex& index) { static_cast<QLabel*>(widget)->setText(index.data().toString()); },
      [](const QModelIndex& index, const int width) { return QSize(width, 24 + index.row() % 3 * 6 + (width < 390 ? 18 : 0)); });
  view.setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
  view.setModel(&source);
  view.resize(400, 220);
  view.show();
  QVERIFY(QTest::qWaitForWindowExposed(&view));
  const auto rowsAreContiguous = [&] {
    for (int row = 1; row < source.rowCount(); ++row) {
      const auto previous = view.visualRect(source.index(row - 1, 0));
      const auto current = view.visualRect(source.index(row, 0));
      if (!previous.isValid() || !current.isValid() || current.top() != previous.bottom() + 1) return false;
    }
    return true;
  };
  QTRY_VERIFY(!view.verticalScrollBar()->isVisible());
  QTRY_VERIFY_WITH_TIMEOUT(rowsAreContiguous(), 1500);
  for (const int height : {150, 400, 150, 400}) {
    view.resize(400, height);
    QTRY_COMPARE(view.verticalScrollBar()->isVisible(), height == 150);
    QTRY_VERIFY_WITH_TIMEOUT(rowsAreContiguous(), 1500);
  }

  for (int row = 6; row < 200; ++row) source.appendRow(new QStandardItem(QStringLiteral("Entry %1").arg(row)));
  view.resize(400, 220);
  QTRY_VERIFY(view.verticalScrollBar()->maximum() > 3000);
  QTRY_VERIFY_WITH_TIMEOUT(rowsAreContiguous(), 1500);
  const QPersistentModelIndex anchor(source.index(150, 0));
  view.scrollTo(anchor, QAbstractItemView::PositionAtTop);
  QTRY_COMPARE(view.visualRect(anchor).top(), 0);
  view.verticalScrollBar()->setValue(view.verticalScrollBar()->value() + 7);
  const int offset = view.visualRect(anchor).top();
  QCOMPARE(offset, -7);
  for (const int width : {480, 400, 480}) {
    view.resize(width, 220);
    QTRY_VERIFY_WITH_TIMEOUT(rowsAreContiguous(), 1500);
    QTRY_COMPARE(view.visualRect(anchor).top(), offset);
    QCOMPARE(view.indexAt(QPoint(view.viewport()->width() / 2, 0)), QModelIndex(anchor));
    QVERIFY(view.findChildren<QLabel*>().size() < 30);
  }
}

void WidgetPaginationTests::entryListsScrollContinuouslyAndKeepPagination() {
  QWidget host;
  auto* layout = new uwf::ui::FlexibleColumnLayout(&host);
  uwf::ui::PathEntryListView list(QStringLiteral("Items"), QStringLiteral("Status"), &host);
  layout->addWidget(&list, 1);
  QStandardItemModel model;
  for (int row = 0; row < 40; ++row) model.appendRow(new QStandardItem(QStringLiteral("item-%1").arg(row, 2, 10, u'0')));
  list.setModel(&model);
  host.resize(520, 252);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  auto* overlay = list.findChild<QScrollBar*>(QStringLiteral("entryScrollOverlay"));
  QVERIFY(overlay && !overlay->isVisible());
  QVERIFY(list.pages().pager().pageCount(40) > 1);
  list.selectionModel()->select(list.model()->index(1, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
  const QPoint local(100, 100);
  QWheelEvent wheel(local, list.viewport()->mapToGlobal(local), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
  QCoreApplication::sendEvent(list.viewport(), &wheel);
  QTRY_VERIFY(list.verticalScrollBar()->value() > 0);
  QVERIFY(overlay->isVisible());
  QCOMPARE(list.selectionModel()->selectedRows().size(), 1);
  checkWholeRowScrolling(&list);
  checkHoverAndSmoothDragging(&list, list.pages());
  list.pages().goLast();
  QTRY_COMPARE(list.verticalScrollBar()->value(), list.verticalScrollBar()->maximum());
  QCOMPARE(list.pages().pager().currentPage, list.pages().pager().pageCount(40) - 1);
  QTRY_VERIFY_WITH_TIMEOUT(!overlay->isVisible(), 1500);
  list.pages().goFirst();
  QTRY_COMPARE(list.verticalScrollBar()->value(), 0);
  list.setFilterText(QStringLiteral("item-39"));
  QTRY_COMPARE(list.model()->rowCount(), 1);
  QCOMPARE(list.pages().pager().pageCount(list.pages().totalRows()), 1);
  QVERIFY(!overlay->isVisible());
}

void WidgetPaginationTests::entryListBottomBorderTracksInteriorReflowWithoutViewportResize_data() {
  QTest::addColumn<bool>("dark");
  QTest::newRow("light") << false;
  QTest::newRow("dark") << true;
}

void WidgetPaginationTests::entryListBottomBorderTracksInteriorReflowWithoutViewportResize() {
  using namespace uwf;
  QFETCH(bool, dark);
  auto& theme = ui::ThemeManager::instance();
  const auto previousTheme = theme.current();
  const auto restoreTheme = qScopeGuard([&] { theme.apply(previousTheme); });
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  QStandardItemModel source(40, 1);
  QWidget window;
  auto* layout = new QVBoxLayout(&window);
  auto* columns = new QHBoxLayout;
  auto* files = new QWidget(&window);
  auto* registry = new QWidget(&window);
  const auto fileControls = ui::createPathEntryControls(*files, source, QStringLiteral("Files"), QStringLiteral("Status"));
  const auto registryControls = ui::createPathEntryControls(*registry, source, QStringLiteral("Registry"), QStringLiteral("Status"));
  columns->addWidget(files, 1);
  columns->addWidget(registry, 1);
  layout->addLayout(columns, 1);
  auto* details = new QLabel(QStringLiteral("Selected record details"), &window);
  constexpr int initialDetailsHeight = 17;
  details->setFixedHeight(initialDetailsHeight);
  layout->addWidget(details);
  window.resize(1040, 427);
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  QTRY_VERIFY(fileControls.list->viewport()->height() > 0);
  QTRY_VERIFY(registryControls.list->viewport()->height() > 0);
  const auto windowSize = window.size();
  const int originalHeight = fileControls.list->height();
  const int originalViewportHeight = fileControls.list->viewport()->height();
  const int originalRegistryViewportHeight = registryControls.list->viewport()->height();

  // Two visible paged lists keep the window fixed. A one-pixel interior reflow
  // changes the list frame while its whole-row viewport can retain its size.
  details->setFixedHeight(initialDetailsHeight + 1);
  QTRY_VERIFY(fileControls.list->height() != originalHeight);
  if (fileControls.list->viewport()->height() != originalViewportHeight) {
    details->setFixedHeight(initialDetailsHeight - 1);
    QTRY_VERIFY(fileControls.list->height() != originalHeight);
  }
  QTRY_COMPARE(fileControls.list->viewport()->height(), originalViewportHeight);
  QTRY_COMPARE(registryControls.list->viewport()->height(), originalRegistryViewportHeight);
  QCOMPARE(window.size(), windowSize);

  // Inspect the painted bottom edge rather than the overlay's own geometry.
  // No resize, tab toggle, syncToParent or explicit layout fit repairs it first.
  const QImage frame = window.grab().toImage();
  for (const auto* list : {fileControls.list, registryControls.list}) {
    const auto error = bottomPaintError(frame, window, *list);
    QVERIFY2(error.isEmpty(), qUtf8Printable(error));
  }
}

void WidgetPaginationTests::entryListBottomCornersPaintWhenHiddenRegistryFirstAppears() {
  using namespace uwf;
  QStandardItemModel source(40, 1);
  QWidget window;
  auto* layout = new QVBoxLayout(&window);
  auto* columns = new QHBoxLayout;
  auto* tabs = new ui::TabView(&window);
  auto* files = new QWidget(tabs);
  auto* registry = new QWidget(tabs);
  const auto fileControls = ui::createPathEntryControls(*files, source, QStringLiteral("Files"), QStringLiteral("Status"));
  const auto registryControls = ui::createPathEntryControls(*registry, source, QStringLiteral("Registry"), QStringLiteral("Status"));
  tabs->addTab(files, QStringLiteral("Files"));
  tabs->addTab(registry, QStringLiteral("Registry"));
  auto* companion = new QWidget(&window);
  const auto companionControls = ui::createPathEntryControls(*companion, source, QStringLiteral("Other records"), QStringLiteral("Status"));
  columns->addWidget(tabs, 1);
  columns->addWidget(companion, 1);
  layout->addLayout(columns, 1);
  auto* details = new QLabel(QStringLiteral("Selected record details"), &window);
  constexpr int initialDetailsHeight = 17;
  details->setFixedHeight(initialDetailsHeight);
  layout->addWidget(details);
  PaintCounter registryPaints;
  registryControls.list->viewport()->installEventFilter(&registryPaints);
  window.resize(1040, 427);
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  QTRY_VERIFY(companionControls.list->viewport()->height() > 0);
  QVERIFY(!registryControls.list->isVisible());
  QCOMPARE(registryPaints.paints, 0);
  const auto fileError = bottomPaintError(window.grab().toImage(), window, *fileControls.list);
  QVERIFY2(fileError.isEmpty(), qUtf8Printable(fileError));

  tabs->setCurrentWidget(registry);
  QTRY_VERIFY(registryControls.list->isVisible());
  // The hidden page must receive a normal first paint before capture. Neither
  // switching away and back nor resizing the window may repair this frame.
  QTRY_VERIFY(registryPaints.paints > 0);
  const auto firstError = bottomPaintError(window.grab().toImage(), window, *registryControls.list);
  QVERIFY2(firstError.isEmpty(), qUtf8Printable(firstError));
  const auto windowSize = window.size();
  const int originalHeight = registryControls.list->height();
  const int originalViewportHeight = registryControls.list->viewport()->height();
  details->setFixedHeight(initialDetailsHeight + 1);
  QTRY_VERIFY(registryControls.list->height() != originalHeight);
  if (registryControls.list->viewport()->height() != originalViewportHeight) {
    details->setFixedHeight(initialDetailsHeight - 1);
    QTRY_VERIFY(registryControls.list->height() != originalHeight);
  }
  QTRY_COMPARE(registryControls.list->viewport()->height(), originalViewportHeight);
  QCOMPARE(window.size(), windowSize);
  const auto reflowError = bottomPaintError(window.grab().toImage(), window, *registryControls.list);
  QVERIFY2(reflowError.isEmpty(), qUtf8Printable(reflowError));
}

void WidgetPaginationTests::pagedTablesScrollContinuouslyWithoutAllocatingRowWidgets() {
  QStandardItemModel source(10000, 4);
  uwf::presentation::PageProxyModel pages;
  pages.setSourceModel(&source);
  QWidget host;
  auto* layout = new uwf::ui::FlexibleColumnLayout(&host);
  layout->setSpacing(15);
  auto* surface = new uwf::ui::Surface(uwf::ui::design::Color::Surface, &host);
  auto* surfaceLayout = new QVBoxLayout(surface);
  surfaceLayout->setContentsMargins(1, 1, 1, 1);
  auto* table = new uwf::ui::PagedTableView(pages, 40, surface);
  surfaceLayout->addWidget(table);
  layout->addWidget(surface, 1);
  auto* following = new QWidget(&host);
  following->setFixedHeight(24);
  layout->addWidget(following);
  host.resize(700, 371);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  QTRY_COMPARE(table->viewport()->height() % 40, 0);
  QTRY_COMPARE(table->viewport()->height(), table->maximumViewportSize().height());
  const auto margins = layout->contentsMargins();
  const int chrome = surface->height() - table->viewport()->height();
  const int otherContent = margins.top() + margins.bottom() + chrome + following->height() + layout->spacing();
  QCOMPARE(table->viewport()->height(), (371 - otherContent) / 40 * 40);
  const QSize initialSize(700, otherContent + table->viewport()->height());
  QCOMPARE(host.size(), initialSize);
  QCOMPARE(following->y() - surface->geometry().bottom() - 1, 15);
  QCOMPARE(table->model()->rowCount(), 10000);
  const int roundedHeight = table->viewport()->height();
  source.setRowCount(2);
  QTRY_COMPARE(table->model()->rowCount(), 2);
  QCOMPARE(table->viewport()->height(), roundedHeight);
  QCOMPARE(host.size(), initialSize);
  source.setRowCount(10000);
  QTRY_COMPARE(table->model()->rowCount(), 10000);
  QCOMPARE(table->viewport()->height(), roundedHeight);
  QVERIFY(table->findChildren<QWidget*>().size() < 30);
  const QPoint local(100, 100);
  QWheelEvent wheel(local, table->viewport()->mapToGlobal(local), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
  QCoreApplication::sendEvent(table->viewport(), &wheel);
  QTRY_VERIFY(table->verticalScrollBar()->value() > 0);
  checkWholeRowScrolling(table);
  checkHoverAndSmoothDragging(table, pages);
  pages.goLast();
  QTRY_COMPARE(table->verticalScrollBar()->value(), table->verticalScrollBar()->maximum());
  QCOMPARE(pages.pager().currentPage, pages.pager().pageCount(10000) - 1);
  pages.goFirst();
  QTRY_COMPARE(table->verticalScrollBar()->value(), 0);
  table->scrollTo(pages.index(5000, 0), QAbstractItemView::PositionAtTop);
  QTRY_COMPARE(pages.pager().currentPage, 5000 / pages.pager().pageSize);
  host.resize(700, 499);
  QTRY_COMPARE(table->viewport()->height() % 40, 0);
  QTRY_COMPARE(host.size(), QSize(700, otherContent + (499 - otherContent) / 40 * 40));
  QCOMPARE(table->viewport()->height(), (499 - otherContent) / 40 * 40);
  QCOMPARE(following->y() - surface->geometry().bottom() - 1, 15);
  QTRY_COMPARE(pages.pager().pageSize, table->viewport()->height() / 40);
  QCOMPARE(table->model()->rowCount(), 10000);
  for (int height = 380; height < 500; height += 7) {
    host.resize(700, height);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QTRY_COMPARE(table->viewport()->height(), (height - otherContent) / 40 * 40);
    QTRY_COMPARE(host.height(), otherContent + (height - otherContent) / 40 * 40);
    QCOMPARE(following->y() - surface->geometry().bottom() - 1, 15);
    QCOMPARE(host.height() - following->geometry().bottom() - 1, margins.bottom());
    layout->setGeometry(layout->geometry());
    layout->setGeometry(layout->geometry());
    QCOMPARE(following->y() - surface->geometry().bottom() - 1, 15);
    QCOMPARE(host.height(), otherContent + (height - otherContent) / 40 * 40);
  }
}

void WidgetPaginationTests::pagedViewportsKeepWholeRowsAfterInteriorReflow_data() {
  QTest::addColumn<bool>("table");
  QTest::addColumn<bool>("rightToLeft");
  QTest::newRow("table-ltr") << true << false;
  QTest::newRow("table-rtl") << true << true;
  QTest::newRow("list-ltr") << false << false;
  QTest::newRow("list-rtl") << false << true;
}

void WidgetPaginationTests::pagedViewportsKeepWholeRowsAfterInteriorReflow() {
  QFETCH(bool, table);
  QFETCH(bool, rightToLeft);
  const int rowHeight = table ? 40 : uwf::ui::design::tableRowHeight;
  QStandardItemModel source(103, 4);
  uwf::presentation::PageProxyModel tablePages;
  tablePages.setSourceModel(&source);
  QWidget host;
  host.setLayoutDirection(rightToLeft ? Qt::RightToLeft : Qt::LeftToRight);
  auto* layout = new QVBoxLayout(&host);
  auto* details = new QLabel(QStringLiteral("Selected record details"), &host);
  details->setFixedHeight(20);
  layout->addWidget(details);
  QAbstractItemView* view = nullptr;
  auto* pages = &tablePages;
  if (table) {
    view = new uwf::ui::PagedTableView(*pages, rowHeight, &host);
  } else {
    auto* list = new uwf::ui::PathEntryListView(QStringLiteral("Items"), QStringLiteral("Status"), &host);
    list->setModel(&source);
    pages = &list->pages();
    view = list;
  }
  layout->addWidget(view, 1);
  host.resize(640, 533);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  uwf::ui::PagedWindowLayout::forWindow(&host).fit();
  QTRY_COMPARE(view->viewport()->height() % rowHeight, 0);
  const auto windowGeometry = host.geometry();
  const int initialHeight = view->viewport()->height();
  QVERIFY(initialHeight >= 3 * rowHeight);
  const auto wholeRows = [&] {
    const auto* viewport = view->viewport();
    if (viewport->height() % rowHeight != 0 || !viewport->mask().isEmpty()) return false;
    const int visibleRows = viewport->height() / rowHeight;
    if (visibleRows < 1 || pages->pager().pageSize != visibleRows) return false;
    const auto first = view->indexAt(QPoint(viewport->width() / 2, 0));
    if (!first.isValid() || view->visualRect(first).top() != 0) return false;
    const int lastRow = std::min(view->model()->rowCount() - 1, first.row() + visibleRows - 1);
    const auto last = view->visualRect(view->model()->index(lastRow, first.column()));
    return last.height() == rowHeight && last.bottom() + 1 == viewport->height() &&
           (view->verticalScrollBar()->value() != view->verticalScrollBar()->maximum() || lastRow == view->model()->rowCount() - 1);
  };
  auto* overlay = view->findChild<QScrollBar*>(QStringLiteral("entryScrollOverlay"));
  QVERIFY(overlay);
  // A selected record can change the interior height without changing the
  // outer window. Exercise both growth and shrinkage of the real viewport.
  for (const int detailsHeight : {41, 20, 65, 20, 101, 1, 20}) {
    details->setFixedHeight(detailsHeight);
    const int expectedHeight = (initialHeight + 20 - detailsHeight) / rowHeight * rowHeight;
    QTRY_COMPARE(view->viewport()->height(), expectedHeight);
    QCOMPARE(host.geometry(), windowGeometry);
    view->scrollToTop();
    QTRY_VERIFY(wholeRows());
    view->scrollToBottom();
    QTRY_VERIFY(wholeRows());
    const auto last = view->visualRect(pages->index(pages->rowCount() - 1, 0));
    QCOMPARE(last.bottom() + 1, view->viewport()->height());
    pages->goFirst();
    QTRY_COMPARE(view->verticalScrollBar()->value(), 0);
    view->setCurrentIndex(pages->index(0, 0));
    for (int press = 0; press < 3; ++press) {
      QTest::keyClick(view, Qt::Key_PageDown);
      QTRY_VERIFY(wholeRows());
    }
    QTest::keyClick(view, Qt::Key_End, table ? Qt::ControlModifier : Qt::NoModifier);
    QTRY_COMPARE(view->verticalScrollBar()->value(), view->verticalScrollBar()->maximum());
    QTRY_VERIFY(wholeRows());
    pages->goFirst();
    pages->goLast();
    QTRY_COMPARE(view->verticalScrollBar()->value(), view->verticalScrollBar()->maximum());
    QTRY_VERIFY(wholeRows());
    QCOMPARE(pages->pager().currentPage, pages->pager().pageCount(source.rowCount()) - 1);
    overlay->setSliderDown(true);
    QCOMPARE(view->verticalScrollMode(), QAbstractItemView::ScrollPerPixel);
    overlay->setValue(rowHeight * 2 + rowHeight / 2);
    const auto partial = view->indexAt(QPoint(view->viewport()->width() / 2, 0));
    QVERIFY(partial.isValid());
    QVERIFY(view->visualRect(partial).top() < 0);
    overlay->setValue(overlay->maximum());
    QTRY_VERIFY(wholeRows());
    overlay->setSliderDown(false);
    QTest::keyClick(view, Qt::Key_Home, table ? Qt::ControlModifier : Qt::NoModifier);
    QCOMPARE(view->verticalScrollMode(), QAbstractItemView::ScrollPerItem);
    QTRY_VERIFY(wholeRows());
    QCOMPARE(host.geometry(), windowGeometry);
    QCOMPARE(overlay->x(), rightToLeft ? view->viewport()->geometry().left() + 1 : view->viewport()->geometry().right() - uwf::ui::design::scrollBarWidth);
  }
  if (auto* tableView = qobject_cast<QTableView*>(view)) {
    tableView->setColumnWidth(0, 1500);
    QTRY_VERIFY(tableView->horizontalScrollBar()->isVisible());
    QTRY_COMPARE(view->viewport()->height(), (initialHeight - tableView->horizontalScrollBar()->height()) / rowHeight * rowHeight);
    view->scrollToBottom();
    QTRY_VERIFY(wholeRows());
    tableView->setColumnWidth(0, 100);
    QTRY_VERIFY(!tableView->horizontalScrollBar()->isVisible());
    QTRY_COMPARE(view->viewport()->height(), initialHeight);
    QTRY_VERIFY(wholeRows());
    QCOMPARE(host.geometry(), windowGeometry);
  }
}

void WidgetPaginationTests::pagedTablesKeepEnterActivationInsideTheView() {
  QDialog dialog;
  auto* layout = new QVBoxLayout(&dialog);
  QStandardItemModel source;
  source.appendRow(new QStandardItem(QStringLiteral("Selected record")));
  uwf::presentation::PageProxyModel pages;
  pages.setSourceModel(&source);
  auto* table = new uwf::ui::PagedTableView(pages, 40, &dialog);
  layout->addWidget(table);
  auto* action = new QPushButton(QStringLiteral("Default action"), &dialog);
  action->setDefault(true);
  layout->addWidget(action);
  dialog.resize(500, 300);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  table->setCurrentIndex(pages.index(0, 0));
  table->setFocus();
  QTRY_VERIFY(table->hasFocus());
  QSignalSpy activated(table, &QTableView::activated);
  QSignalSpy triggered(action, &QPushButton::clicked);
  for (const auto key : {Qt::Key_Return, Qt::Key_Enter}) {
    QTest::keyClick(table, key);
    QCOMPARE(triggered.count(), 0);
    QVERIFY(dialog.isVisible());
  }
  QCOMPARE(activated.count(), 2);
}

void WidgetPaginationTests::hiddenTabPagesDoNotPushReportsOutsideTheViewport() {
  using namespace uwf::ui;
  PanelDialog dialog;
  dialog.setWindowTitle(QStringLiteral("Diagnostic report"));
  auto* frame = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(560, 400), QSize(400, 240));
  auto* tabs = new TabView(&dialog);
  tabs->setMeasureCurrentPage(true);
  auto* overview = new QWidget(tabs);
  overview->setMinimumHeight(900);
  tabs->addTab(overview, QStringLiteral("Overview"));
  auto* report = new CodeEditor(tabs);
  report->setReadOnly(true);
  report->setLineNumbersVisible(false);
  const QString text = QString(180, u'X') + QStringLiteral("\nDiagnostic line\n").repeated(100);
  report->setPlainText(text);
  tabs->addTab(report, QStringLiteral("Raw report"));
  frame->body()->addWidget(tabs, 1);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QTRY_VERIFY(frame->scrollArea()->verticalScrollBar()->maximum() > 0);
  const QSize size = dialog.size();
  for (int round = 0; round < 3; ++round) {
    tabs->setCurrentWidget(report);
    QTRY_COMPARE(frame->scrollArea()->verticalScrollBar()->maximum(), 0);
    QTRY_VERIFY(report->verticalScrollBar()->maximum() > 0);
    QTRY_VERIFY(report->horizontalScrollBar()->maximum() > 0);
    QCOMPARE(dialog.size(), size);
    report->verticalScrollBar()->setValue(round == 1 ? report->verticalScrollBar()->maximum() / 2 : report->verticalScrollBar()->maximum());
    report->horizontalScrollBar()->setValue(report->horizontalScrollBar()->maximum());
    QCOMPARE(frame->scrollArea()->verticalScrollBar()->value(), 0);
    QCOMPARE(report->toPlainText(), text);
    const auto reportPosition = report->verticalScrollBar()->value();
    tabs->setCurrentWidget(overview);
    QTRY_VERIFY(frame->scrollArea()->verticalScrollBar()->maximum() > 0);
    QCOMPARE(dialog.size(), size);
    tabs->setCurrentWidget(report);
    QTRY_COMPARE(frame->scrollArea()->verticalScrollBar()->maximum(), 0);
    QTRY_COMPARE(report->verticalScrollBar()->value(), reportPosition);
  }
}

}  // namespace
QTEST_MAIN(WidgetPaginationTests)
#include "PaginationCases.moc"
