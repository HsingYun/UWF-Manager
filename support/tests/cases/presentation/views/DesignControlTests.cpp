// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
// clang-format off
#include <Windows.h>
// clang-format on
#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QCursor>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QEnterEvent>
#include <QFocusFrame>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QHeaderView>
#include <QImage>
#include <QInputMethodEvent>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QProgressBar>
#include <QProxyStyle>
#include <QPushButton>
#include <QRadioButton>
#include <QSaveFile>
#include <QScopeGuard>
#include <QScreen>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QStyleOptionButton>
#include <QStyleOptionSlider>
#include <QStyleOptionTab>
#include <QTabBar>
#include <QTableView>
#include <QTest>
#include <QTextBlock>
#include <QTextLayout>
#include <QTimer>
#include <QToolButton>
#include <QTranslator>
#include <QTreeView>
#include <QUuid>
#include <QVBoxLayout>
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include "foundation/logging/Log.h"
#include "presentation/controls/clipboard/ClipboardText.h"
#include "presentation/controls/feedback/ContentState.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/feedback/StatusBadge.h"
#include "presentation/controls/inputs/CodeEditor.h"
#include "presentation/controls/inputs/NumberBox.h"
#include "presentation/controls/inputs/SearchField.h"
#include "presentation/controls/inputs/SelectBox.h"
#include "presentation/controls/inputs/SwitchButton.h"
#include "presentation/controls/layout/FlowLayout.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/navigation/CommandBar.h"
#include "presentation/controls/navigation/CommandMenu.h"
#include "presentation/controls/navigation/ContextMenus.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/navigation/TabView.h"
#include "presentation/controls/pagination/PaginationBar.h"
#include "presentation/controls/tables/ItemDelegate.h"
#include "presentation/controls/tables/PathEntryControls.h"
#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/controls/tables/RecordInspector.h"
#include "presentation/controls/tables/ResponsiveTableView.h"
#include "presentation/controls/tables/SelectionListView.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/painting/IconPainting.h"
#include "presentation/design/painting/ItemHighlight.h"
#include "presentation/design/painting/RasterInkBounds.h"
#include "presentation/design/painting/TextPainting.h"
#include "presentation/design/surfaces/PopupPlacement.h"
#include "presentation/design/theme/DesignStyle.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/models/PageProxyModel.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/AnimationClock.h"
#include "support/visual/InkBounds.h"
#include "support/visual/VisualGeometry.h"

namespace {
class ContextMenuTranslator final : public QTranslator {
 public:
  bool isEmpty() const override { return false; }
  QString translate(const char* context, const char* source, const char*, int) const override {
    if (!context || !source || qstrcmp(context, "uwf") != 0) return {};
    return QStringLiteral("Localized %1").arg(QString::fromUtf8(source));
  }
};

class ResizeDuringPointerDelivery final : public QObject {
 public:
  ResizeDuringPointerDelivery(QWidget& owner, QWidget& target, const QPoint point) : m_owner(owner), m_target(target), m_point(point) {
    qApp->installEventFilter(this);
  }
  ~ResizeDuringPointerDelivery() override { qApp->removeEventFilter(this); }
  bool armed = false;
  int geometryChanges = 0;
  int nativeQtMoves = 0;
  int enters = 0;

 protected:
  bool eventFilter(QObject* receiver, QEvent* event) override {
    if (receiver != &m_target) return false;
    const bool entering = event->type() == QEvent::Enter;
    const bool moving = event->type() == QEvent::MouseMove && event->spontaneous();
    if (!entering && !moving) return false;
    const auto global = entering ? static_cast<QEnterEvent*>(event)->globalPosition().toPoint() : static_cast<QMouseEvent*>(event)->globalPosition().toPoint();
    const auto expected = m_target.mapToGlobal(m_point);
    if (qAbs(global.x() - expected.x()) > 1 || qAbs(global.y() - expected.y()) > 1) return false;
    if (entering)
      ++enters;
    else
      ++nativeQtMoves;
    if (armed) {
      armed = false;  // resize can synchronously deliver more window events.
      ++geometryChanges;
      m_owner.resize(m_owner.width() + 40, m_owner.height());
    }
    return false;
  }

 private:
  QWidget& m_owner;
  QWidget& m_target;
  QPoint m_point;
};

class TabHoverStyle final : public QProxyStyle {
 public:
  TabHoverStyle() : QProxyStyle(new uwf::ui::DesignStyle()) {}
  mutable std::map<int, bool> hovered;
  void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override {
    if (element == CE_TabBarTab)
      if (const auto* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) hovered[tab->tabIndex] = tab->state.testFlag(State_MouseOver);
    QProxyStyle::drawControl(element, option, painter, widget);
  }
};

class DesignControlTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() {
    QApplication::setStyle(new uwf::ui::DesignStyle());
    QApplication::setFont(uwf::ui::DesignSystem::defaultFont());
    uwf::ui::installContextMenuPresentation();
  }
  void rebuiltTabsReleaseHoverWhenThePointerMovesOrLeaves_data() {
    QTest::addColumn<bool>("light");
    QTest::addColumn<int>("kind");
    for (const bool light : {true, false})
      for (const int kind : {0, 1, 2}) QTest::newRow(qPrintable(QStringLiteral("%1-kind%2").arg(light ? "light" : "dark").arg(kind))) << light << kind;
  }
  void rebuiltTabsReleaseHoverWhenThePointerMovesOrLeaves() {
    QFETCH(bool, light);
    QFETCH(int, kind);
    if (QApplication::platformName() != QStringLiteral("offscreen"))
      QSKIP("This regression uses the offscreen virtual pointer; native pointer gestures have separate desktop coverage.");
    const auto originalCursor = QCursor::pos();
    const auto restoreCursor = qScopeGuard([&] { QCursor::setPos(originalCursor); });
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    TabHoverStyle observer;
    uwf::ui::TabView tabs;
    tabs.setPalette(design.palette());
    tabs.setKind(static_cast<uwf::ui::TabView::Kind>(kind));
    auto* bar = tabs.tabBar();
    bar->setStyle(&observer);
    QVERIFY(bar->testAttribute(Qt::WA_Hover));
    const auto rebuild = [&] {
      while (tabs.count()) {
        auto* page = tabs.widget(0);
        tabs.removeTab(0);
        delete page;
      }
      for (const auto* title : {"C:", "D:", "Partition"}) tabs.addTab(new QWidget, QString::fromLatin1(title));
      QApplication::processEvents();
    };
    const auto movePoint = [&](const QPoint point) {
      // Keep the offscreen virtual cursor consistent with QPA input. Qt also
      // reads QCursor when a page's visibility synthesizes enter/leave events.
      // The platform guard above prevents any real desktop cursor movement.
      QCursor::setPos(tabs.mapToGlobal(point));
      QTest::mouseMove(tabs.windowHandle(), point, 0);
      bar->repaint();
      QApplication::processEvents();
    };
    const auto move = [&](const int index) { movePoint(bar->mapTo(&tabs, bar->tabRect(index).center())); };
    rebuild();
    tabs.resize(600, 160);
    tabs.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tabs));
    move(1);
    QTest::mouseClick(tabs.windowHandle(), Qt::LeftButton, Qt::NoModifier, bar->mapTo(&tabs, bar->tabRect(1).center()), 0);
    rebuild();
    tabs.setCurrentIndex(2);
    move(0);
    QVERIFY(observer.hovered[0]);
    QVERIFY(!observer.hovered[1]);
    QCOMPARE(tabs.currentIndex(), 2);
    // Moving into the page makes Qt generate both Leave and HoverLeave.
    movePoint(QPoint(tabs.width() - 2, tabs.height() - 2));
    bar->repaint();
    QApplication::processEvents();
    for (int index = 0; index < tabs.count(); ++index) QVERIFY(!observer.hovered[index]);
    // A fresh paint and a new selection must not resurrect the old D: hover.
    tabs.setCurrentIndex(1);
    tabs.repaint();
    QApplication::processEvents();
    for (int index = 0; index < tabs.count(); ++index) QVERIFY(!observer.hovered[index]);
  }
  void disabledSwitchesPreserveKnownStateAndSeparateUnknown_data() {
    QTest::addColumn<bool>("light");
    QTest::newRow("light") << true;
    QTest::newRow("dark") << false;
  }
  void disabledSwitchesPreserveKnownStateAndSeparateUnknown() {
    QFETCH(bool, light);
    using uwf::ui::design::Color;
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    QWidget host;
    host.setPalette(design.palette());
    host.setAutoFillBackground(true);
    auto* layout = new QHBoxLayout(&host);
    std::array<uwf::ui::SwitchButton*, 3> switches{};
    for (auto*& control : switches) {
      control = new uwf::ui::SwitchButton(&host);
      layout->addWidget(control);
    }
    auto& off = *switches[0];
    auto& on = *switches[1];
    auto& unknown = *switches[2];
    on.setChecked(true);
    unknown.setUnknown(true);
    for (auto* control : switches) control->setEnabled(false);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    const auto disabledOff = off.grab().toImage();
    const auto disabledOn = on.grab().toImage();
    const auto disabledUnknown = unknown.grab().toImage();
    const QRectF track(off.width() / 2.0 - 1, off.height() / 2.0 - 1, 2, 2);
    const auto onTrack = uwf::test::regionColor(disabledOn, track);
    const auto offTrack = uwf::test::regionColor(disabledOff, track);
    QVERIFY(std::abs(onTrack.blueF() - onTrack.redF()) > .08);
    QVERIFY(std::abs(onTrack.blueF() - offTrack.blueF()) + std::abs(onTrack.redF() - offTrack.redF()) > .08);
    const qreal thumb = off.height() - 2.0 * uwf::ui::design::switchOffset;
    const QRectF left(uwf::ui::design::switchOffset + thumb / 2.0 - 1, off.height() / 2.0 - 1, 2, 2);
    const QRectF right(off.width() - uwf::ui::design::switchOffset - thumb / 2.0 - 1, off.height() / 2.0 - 1, 2, 2);
    QVERIFY(uwf::test::changedCoverage(disabledOff, disabledOn, left) > .8);
    QVERIFY(uwf::test::changedCoverage(disabledOff, disabledOn, right) > .8);
    QVERIFY(uwf::test::changedCoverage(disabledUnknown, disabledOn, right) > .8);
    QSignalSpy toggled(&on, &QAbstractButton::toggled);
    QTest::mouseClick(&on, Qt::LeftButton);
    QTest::keyClick(&on, Qt::Key_Space);
    QVERIFY(on.isChecked());
    QCOMPARE(toggled.size(), 0);
    QCOMPARE(on.cursor().shape(), Qt::ArrowCursor);
    // Disabling during the normal checked-state animation settles on the
    // semantic value immediately, rather than preserving a mid-track thumb.
    on.setEnabled(true);
    on.setChecked(false);
    on.setChecked(true);
    on.setEnabled(false);
    const auto settled = on.grab().toImage();
    QVERIFY(uwf::test::changedCoverage(disabledOn, settled, QRectF(on.rect())) <= uwf::test::kVisualColorRatio);
    if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/disabled-switches"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(host.grab().save(output.filePath(light ? QStringLiteral("light.png") : QStringLiteral("dark.png"))));
    }
  }
  void scrollBarRolesPreserveGeometryAndPaintingAcrossRenaming_data() {
    QTest::addColumn<bool>("thin");
    QTest::addColumn<int>("orientation");
    for (const bool thin : {false, true})
      for (const auto orientation : {Qt::Horizontal, Qt::Vertical})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(thin ? "thin" : "full", orientation == Qt::Vertical ? "vertical" : "horizontal")))
            << thin << static_cast<int>(orientation);
  }
  void scrollBarRolesPreserveGeometryAndPaintingAcrossRenaming() {
    QFETCH(bool, thin);
    QFETCH(int, orientation);
    QScrollBar bar(static_cast<Qt::Orientation>(orientation));
    if (thin) bar.setProperty("designThinScrollBar", true);
    bar.resize(orientation == Qt::Vertical ? QSize(16, 180) : QSize(180, 16));
    QStyleOptionSlider option;
    option.initFrom(&bar);
    option.rect = bar.rect();
    option.orientation = static_cast<Qt::Orientation>(orientation);
    option.minimum = 0;
    option.maximum = 100;
    option.pageStep = 20;
    option.sliderPosition = option.sliderValue = 35;
    const auto render = [&] {
      QImage image(bar.size(), QImage::Format_ARGB32_Premultiplied);
      image.fill(Qt::transparent);
      {
        QPainter painter(&image);
        bar.style()->drawComplexControl(QStyle::CC_ScrollBar, &option, &painter, &bar);
      }
      return image;
    };
    const int extent = bar.style()->pixelMetric(QStyle::PM_ScrollBarExtent, &option, &bar);
    const auto thumb = bar.style()->subControlRect(QStyle::CC_ScrollBar, &option, QStyle::SC_ScrollBarSlider, &bar);
    QVERIFY(!thumb.isEmpty());
    QCOMPARE(bar.style()->subControlRect(QStyle::CC_ScrollBar, &option, QStyle::SC_ScrollBarSubLine, &bar).isEmpty(), thin);
    const auto frame = render();
    const auto thumbColor = uwf::ui::DesignSystem::instance().color(thin ? uwf::ui::design::Color::Faint : uwf::ui::design::Color::ScrollbarThumb);
    const QRectF thumbRegion(thumb);
    const auto originalInk = uwf::test::visibleInkBounds(frame, thumbColor, Qt::transparent, thumbRegion);
    QVERIFY(!originalInk.isEmpty());
    QVERIFY(uwf::test::colorCoverage(frame, thumbColor, thumbRegion) > 0);
    for (const auto& name : {QStringLiteral("entryScrollOverlay"), QStringLiteral("renamedScrollBar")}) {
      bar.setObjectName(name);
      QCOMPARE(bar.style()->pixelMetric(QStyle::PM_ScrollBarExtent, &option, &bar), extent);
      QCOMPARE(bar.style()->subControlRect(QStyle::CC_ScrollBar, &option, QStyle::SC_ScrollBarSlider, &bar), thumb);
      const auto renamed = render();
      const auto renamedInk = uwf::test::visibleInkBounds(renamed, thumbColor, Qt::transparent, thumbRegion);
      QVERIFY(!renamedInk.isEmpty());
      QVERIFY(uwf::test::colorCoverage(renamed, thumbColor, thumbRegion) > 0);
      QVERIFY(uwf::test::similarSize(renamedInk, originalInk));
      QVERIFY(uwf::test::centersAligned(renamedInk, originalInk, Qt::Horizontal));
      QVERIFY(uwf::test::centersAligned(renamedInk, originalInk));
      QVERIFY(uwf::test::changedCoverage(frame, renamed, thumbRegion) <= uwf::test::kVisualAlignmentRatio);
    }
  }
  void contextMenusFollowWidgetCapabilitiesAndQtTranslations_data() {
    QTest::addColumn<int>("kind");
    QTest::addColumn<bool>("localized");
    QTest::addColumn<bool>("selected");
    QTest::addColumn<bool>("writeConfirmed");
    for (int kind = 0; kind < 3; ++kind)
      for (const bool localized : {false, true})
        for (const bool selected : {false, true}) {
          if (kind != 1 && !selected) continue;
          for (const bool confirmed : {false, true}) {
            if (kind == 2 && !confirmed) continue;
            QTest::newRow(
                qUtf8Printable(QStringLiteral("%1-%2-selected-%3-write-%4").arg(kind).arg(localized ? "localized" : "source").arg(selected).arg(confirmed)))
                << kind << localized << selected << confirmed;
          }
        }
  }
  void contextMenusFollowWidgetCapabilitiesAndQtTranslations() {
    QFETCH(int, kind);
    QFETCH(bool, localized);
    QFETCH(bool, selected);
    QFETCH(bool, writeConfirmed);
    ContextMenuTranslator translator;
    if (localized) QVERIFY(QCoreApplication::installTranslator(&translator));
    const auto restore = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });
    QWidget host;
    auto* layout = new QVBoxLayout(&host);
    QLabel label(QStringLiteral("A sufficiently long first cell"), &host);
    QTableView table(&host);
    QStandardItemModel model(2, 1);
    model.setData(model.index(0, 0), label.text());
    model.setData(model.index(1, 0), QStringLiteral("Second cell"));
    table.setModel(&model);
    table.setSelectionBehavior(QAbstractItemView::SelectRows);
    table.setSelectionMode(QAbstractItemView::ExtendedSelection);
    if (kind == 0) {
      table.hide();
      layout->addWidget(&label);
    } else {
      label.hide();
      layout->addWidget(&table);
    }
    host.resize(400, 240);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    const auto expected = [&](const QString& source) { return localized ? QStringLiteral("Localized ") + source : source; };
    for (const auto& name : {QStringLiteral("dialogHeader"), QStringLiteral("renamedContent")}) {
      host.setObjectName(name);
      table.clearSelection();
      if (selected) table.selectRow(0);
      table.horizontalHeader()->resizeSection(0, 35);
      QStringList captions;
      QStringList submitted;
      auto* target = kind == 0 ? static_cast<QWidget*>(&label) : kind == 1 ? table.viewport() : table.horizontalHeader()->viewport();
      const auto position = kind == 1 ? selected ? table.visualRect(model.index(0, 0)).center() : QPoint(-1, -1) : target->rect().center();
      QContextMenuEvent event(QContextMenuEvent::Mouse, position, target->mapToGlobal(position));
      const auto receipt = writeConfirmed ? uwf::ui::ClipboardWriteResult::Confirmed : uwf::ui::ClipboardWriteResult::Failed;
      auto menu = uwf::ui::createContextMenu(*target, event, [&](const QString& text) {
        submitted.append(text);
        return receipt;
      });
      QVERIFY(menu);
      QVERIFY(submitted.isEmpty());
      for (auto actions = menu->actions(); auto* action : std::as_const(actions)) captions.append(action->text());
      auto* first = menu->actions().constFirst();
      QSignalSpy triggered(first, &QAction::triggered);
      QCOMPARE(first->isEnabled(), kind != 1 || selected);
      first->trigger();
      QCOMPARE(triggered.size(), kind != 1 || selected ? 1 : 0);
      if (kind == 2) {
        QCOMPARE(captions, QStringList{expected(QStringLiteral("Fit columns to contents"))});
        QVERIFY(table.columnWidth(0) > 35);
        QVERIFY(submitted.isEmpty());
      } else {
        const auto expectedCaptions =
            kind == 0 ? QStringList{expected(QStringLiteral("Copy"))} : QStringList{expected(QStringLiteral("Copy")), expected(QStringLiteral("Select All"))};
        QCOMPARE(captions, expectedCaptions);
        const QStringList expectedSubmission = kind == 0 || selected ? QStringList{QStringLiteral("A sufficiently long first cell")} : QStringList{};
        // Copy failure must not replay the command; disabled commands must not invoke the writer.
        QCOMPARE(submitted, expectedSubmission);
        if (kind == 1) {
          menu->actions().constLast()->trigger();
          QCOMPARE(table.selectionModel()->selectedRows().size(), 2);
          QCOMPARE(submitted, expectedSubmission);
        }
      }
    }
  }
  void qtClipboardSubmissionReportsItsSingleReceipt() {
    auto* clipboard = QApplication::clipboard();
    QVERIFY(clipboard);
    const auto previous = clipboard->text();
    const auto submitted = QStringLiteral("UWF clipboard single submission ") + QUuid::createUuid().toString();
    const auto receipt = uwf::ui::writeClipboardText(submitted);
    const auto restore = qScopeGuard([&] {
      if (receipt == uwf::ui::ClipboardWriteResult::Confirmed) static_cast<void>(uwf::ui::writeClipboardText(previous));
    });
    const auto reports = uwf::recentLogLines(1, 1024);
    QCOMPARE(reports.size(), std::size_t{1});
    const auto& report = reports.front();
    QVERIFY(report.find(" clipboard] event=write result=") != std::string::npos);
    if (receipt == uwf::ui::ClipboardWriteResult::Confirmed) {
      QVERIFY(report.find("result=confirmed") != std::string::npos);
      QVERIFY(report.find("owned=true text_verified=true") != std::string::npos);
      qInfo("Qt clipboard single submission receipt=Confirmed (ownership/text confirmed at call time)");
    } else {
      QCOMPARE(receipt, uwf::ui::ClipboardWriteResult::Failed);
      QVERIFY(report.find("result=failed reason=") != std::string::npos);
      QVERIFY(report.find("text_verified=false") != std::string::npos);
      qInfo("Qt clipboard single submission receipt=Failed (platform write not confirmed; native Qt warnings remain in this result)");
    }
    // Later clipboard changes must not alter the completed write receipt.
    qInfo().noquote() << QString::fromStdString(report);
  }
  void clipboardRequestsOutsideTheGuiThreadAreRejected() {
    const auto submitted = QStringLiteral("No clipboard access from a worker");
    auto receipt = uwf::ui::ClipboardWriteResult::Confirmed;
    std::jthread worker([&] { receipt = uwf::ui::writeClipboardText(submitted); });
    worker.join();
    QCOMPARE(receipt, uwf::ui::ClipboardWriteResult::Failed);
    const auto reports = uwf::recentLogLines(1, 1024);
    QCOMPARE(reports.size(), std::size_t{1});
    QVERIFY(reports.front().find("clipboard] event=write result=failed reason=gui-thread-required") != std::string::npos);
  }
  void contextMenuRejectsAnEmptyClipboardWriter() {
    QLabel label(QStringLiteral("Copy target"));
    const QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint{}, QPoint{});
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(uwf::ui::createContextMenu(label, event, {})));
  }
  void contextMenusHaveOneOwnerWhenTheirTargetRetires_data() {
    QTest::addColumn<int>("kind");
    QTest::addColumn<bool>("duringExec");
    for (int kind = 0; kind < 3; ++kind)
      for (const bool duringExec : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("target-%1-exec-%2").arg(kind).arg(duringExec))) << kind << duringExec;
  }
  void contextMenusHaveOneOwnerWhenTheirTargetRetires() {
    QFETCH(int, kind);
    QFETCH(bool, duringExec);
    auto owner = std::make_unique<QWidget>();
    auto* layout = new QVBoxLayout(owner.get());
    QStandardItemModel model(1, 1);
    model.setData(model.index(0, 0), QStringLiteral("Retiring target"));
    auto* label = new QLabel(QStringLiteral("Retiring target"), owner.get());
    auto* table = new QTableView(owner.get());
    table->setModel(&model);
    table->selectRow(0);
    if (kind == 0) {
      table->hide();
      layout->addWidget(label);
    } else {
      label->hide();
      layout->addWidget(table);
    }
    owner->resize(400, 240);
    owner->show();
    QVERIFY(QTest::qWaitForWindowExposed(owner.get()));
    auto* target = kind == 0 ? static_cast<QWidget*>(label) : kind == 1 ? table->viewport() : table->horizontalHeader()->viewport();
    const auto position = kind == 1 ? table->visualRect(model.index(0, 0)).center() : target->rect().center();
    QContextMenuEvent event(QContextMenuEvent::Mouse, position, target->mapToGlobal(position));
    int writes = 0;
    auto menu = uwf::ui::createContextMenu(*target, event, [&](const QString&) {
      ++writes;
      return uwf::ui::ClipboardWriteResult::Confirmed;
    });
    QVERIFY(menu);
    QVERIFY(!menu->parent());
    const QPointer lifetime(menu.get());
    if (duringExec) {
      QTimer::singleShot(0, menu.get(), [&] { owner.reset(); });
      menu->exec(event.globalPos());
    } else
      owner.reset();
    QVERIFY(!owner);
    QVERIFY(lifetime);
    QVERIFY(!menu->isVisible());
    // Retained actions must remain inert after the menu closes.
    for (auto actions = menu->actions(); auto* action : std::as_const(actions)) action->trigger();
    QCOMPARE(writes, 0);
    menu.reset();
    QVERIFY(lifetime.isNull());
  }
  void pointerTransitionsAcrossRecreatedWindowsAtTheSamePosition() {
    for (int generation = 0; generation < 3; ++generation) {
      QDialog dialog;
      dialog.setGeometry(150, 150, 300, 180);
      QPushButton button(QStringLiteral("Target"), &dialog);
      button.setGeometry(80, 60, 120, 40);
      dialog.show();
      QVERIFY(QTest::qWaitForWindowExposed(&dialog));
      uwf::test::moveMouse(button);
      QTRY_VERIFY(button.underMouse());
      // Keep the last logical pointer position for the replacement window.
      dialog.close();
    }
  }
  void itemHighlightsReleaseFinishedTransitionsAndCanRestart() {
    using uwf::ui::ItemHighlight;
    QWidget viewport;
    uwf::test::AnimationClock clock;
    const auto initialChildren = viewport.findChildren<QVariantAnimation*>().size();
    for (int row = 0; row < 128; ++row) {
      const QPoint position(0, row);
      static_cast<void>(ItemHighlight::opacity(&viewport, position, true));
      clock.advanceBy(200);
      QCOMPARE(ItemHighlight::opacity(&viewport, position, true), 1.0);
      static_cast<void>(ItemHighlight::opacity(&viewport, position, false));
      clock.advanceBy(200);
      QCOMPARE(ItemHighlight::opacity(&viewport, position, false), 0.0);
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
    QCOMPARE(viewport.findChildren<QVariantAnimation*>().size(), initialChildren);
    static_cast<void>(ItemHighlight::opacity(&viewport, QPoint(0, 0), true));
    clock.advanceBy(200);
    QCOMPARE(ItemHighlight::opacity(&viewport, QPoint(0, 0), true), 1.0);
  }
  void semanticTextColorsFollowTheThemeWithoutStylesheetRepolishing() {
    auto& design = uwf::ui::DesignSystem::instance();
    const auto original = design.isLight();
    const auto restore = qScopeGuard([&] { design.setLight(original); });
    uwf::ui::TextLabel label(QStringLiteral("MMMM"));
    label.setProperty("designTone", "muted");
    auto font = label.font();
    font.setPixelSize(30);
    label.setFont(font);
    label.resize(180, 70);
    label.show();
    QVERIFY(QTest::qWaitForWindowExposed(&label));
    for (const bool light : {false, true}) {
      design.setLight(light);
      const auto pixels = label.grab().toImage();
      const auto expected = design.color(uwf::ui::design::Color::Muted);
      QVERIFY(uwf::test::colorCoverage(pixels, expected, label.rect()) > 0);
    }
  }
  void buttonIconColorsPreserveTextAndCheckedPresentation() {
    using uwf::ui::design::Color;
    auto& design = uwf::ui::DesignSystem::instance();
    const bool originalLight = design.isLight();
    const auto originalPalette = QApplication::palette();
    const auto restore = qScopeGuard([&] {
      design.setLight(originalLight);
      QApplication::setPalette(originalPalette);
    });
    uwf::test::AnimationClock clock;
    QWidget host;
    host.resize(340, 110);
    QPixmap glyph(16, 16);
    const QColor sourceColor(QStringLiteral("#ff00ff"));
    glyph.fill(sourceColor);
    QToolButton command(&host);
    command.setGeometry(20, 20, 230, 36);
    command.setProperty("designRole", "quiet");
    command.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    command.setText(QStringLiteral("MMMM"));
    command.setIcon(QIcon(glyph));
    command.setIconSize({16, 16});
    command.setFocusPolicy(Qt::NoFocus);
    QToolButton toggle(&host);
    toggle.setGeometry(270, 20, 32, 32);
    toggle.setProperty("designRole", "quiet");
    toggle.setProperty("designCheckedIconOnly", true);
    toggle.setToolButtonStyle(Qt::ToolButtonIconOnly);
    toggle.setCheckable(true);
    toggle.setIcon(QIcon(glyph));
    toggle.setIconSize({16, 16});
    toggle.setFocusPolicy(Qt::NoFocus);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    uwf::test::moveMouse(host.windowHandle(), QPoint(320, 90));
    QTRY_VERIFY(!command.underMouse() && !toggle.underMouse());
    clock.advanceBy(200);
    for (const bool light : {false, true}) {
      design.setLight(light);
      QApplication::setPalette(design.palette());
      QCoreApplication::processEvents();
      command.setText({});
      command.setIcon({});
      const auto blank = command.grab().toImage();
      command.setText(QStringLiteral("MMMM"));
      command.setIcon(QIcon(glyph));
      command.setProperty("designIconColor", QColor{});
      const auto ordinary = command.grab().toImage();
      // The right half contains text and excludes the leading square icon.
      // Keep the text visually stable when only the icon's semantic color changes.
      const QRect textArea(ordinary.width() / 2, 0, ordinary.width() - ordinary.width() / 2, ordinary.height());
      const auto textBounds = uwf::test::logicalBounds(textArea, ordinary.devicePixelRatio());
      const auto textBackground = uwf::test::regionColor(blank, textBounds);
      const auto ordinaryInk = uwf::test::visibleInkBounds(ordinary, design.color(Color::Text), textBackground, textBounds);
      QVERIFY(!ordinaryInk.isEmpty());
      QVERIFY(uwf::test::changedCoverage(blank, ordinary, textBounds) > 0);
      QVERIFY(uwf::test::colorCoverage(ordinary, design.color(Color::Text), command.rect()) > 0);
      QCOMPARE(uwf::test::colorCoverage(ordinary, sourceColor, command.rect()), 0.0);
      command.setProperty("designIconColor", design.color(Color::Success));
      const auto status = command.grab().toImage();
      QVERIFY(uwf::test::colorCoverage(status, design.color(Color::Success), command.rect()) > 0);
      const auto statusInk = uwf::test::visibleInkBounds(status, design.color(Color::Text), textBackground, textBounds);
      QVERIFY(!statusInk.isEmpty());
      QVERIFY(uwf::test::colorCoverage(status, design.color(Color::Text), statusInk) > 0);
      QVERIFY(uwf::test::similarSize(statusInk, ordinaryInk));
      QVERIFY(uwf::test::centersAligned(statusInk, ordinaryInk, Qt::Horizontal));
      QVERIFY(uwf::test::centersAligned(statusInk, ordinaryInk));
      QVERIFY(uwf::test::changedCoverage(ordinary, status, ordinaryInk.united(statusInk)) <= uwf::test::kVisualAlignmentRatio);
      QCOMPARE(uwf::test::colorCoverage(status, sourceColor, command.rect()), 0.0);
      for (const bool checked : {false, true, false}) {
        toggle.setChecked(checked);
        const auto frame = toggle.grab().toImage();
        const QRectF iconBounds((toggle.width() - toggle.iconSize().width()) / 2.0, (toggle.height() - toggle.iconSize().height()) / 2.0,
                                toggle.iconSize().width(), toggle.iconSize().height());
        QVERIFY(uwf::test::colorCoverage(frame, design.color(checked ? Color::Accent : Color::Text), uwf::test::interiorRegion(iconBounds)) >=
                uwf::test::kVisualMinimumCoverage);
      }
    }
  }
  void contentStatesWrapTheirCompleteTextAtTheAvailableWidth() {
    QWidget host;
    host.resize(350, 430);
    auto* layout = new QVBoxLayout(&host);
    auto* state = new uwf::ui::ContentState(&host);
    layout->addWidget(state);
    state->setText(QStringLiteral("Reading the complete diagnostic log"),
                   QStringLiteral("Records from the current application run will appear here when reading is complete. 诊断日志会完整显示。"));
    state->setLoading(true);
    state->setAction(QStringLiteral("Cancel reading"));
    const auto capture = [&](const QString& name) {
      if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
        QDir output(directory + QStringLiteral("/content-state"));
        QVERIFY(output.mkpath(QStringLiteral(".")));
        QVERIFY(host.grab().save(output.filePath(name + QStringLiteral(".png"))));
      }
    };
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    for (auto labels = state->findChildren<uwf::ui::TextLabel*>(); const auto* label : std::as_const(labels)) {
      QVERIFY(!label->text().isEmpty());
      QTRY_VERIFY2(
          label->height() >= label->heightForWidth(label->width()),
          qUtf8Printable(
              QStringLiteral("%1: %2x%3 needs %4").arg(label->text()).arg(label->width()).arg(label->height()).arg(label->heightForWidth(label->width()))));
      QVERIFY(label->width() >= state->width() / 2);
      QVERIFY(uwf::test::centersAligned(label->geometry(), state->rect(), Qt::Horizontal));
    }
    capture(QStringLiteral("narrow"));
    host.resize(750, 430);
    for (auto labels = state->findChildren<uwf::ui::TextLabel*>(); const auto* label : std::as_const(labels)) {
      QTRY_VERIFY(label->height() >= label->heightForWidth(label->width()));
      QVERIFY(label->width() <= state->width());
      QVERIFY(uwf::test::centersAligned(label->geometry(), state->rect(), Qt::Horizontal));
    }
    capture(QStringLiteral("wide"));
  }
  void styleSheetsPreserveDesignedHeadersAndProgressMotion() {
    using uwf::ui::design::Color;
    auto& design = uwf::ui::DesignSystem::instance();
    uwf::test::AnimationClock animation;
    QWidget host;
    host.resize(460, 240);
    host.setStyleSheet(QStringLiteral("QAbstractScrollArea { border: none; background: %1; }").arg(design.color(Color::Surface).name()));
    QTableView table(&host);
    table.setGeometry(20, 20, 420, 140);
    QStandardItemModel model(2, 2);
    model.setHorizontalHeaderLabels({QStringLiteral("Time"), QStringLiteral("Message")});
    table.setModel(&model);
    table.horizontalHeader()->setFixedHeight(38);
    table.horizontalHeader()->setDefaultSectionSize(160);
    QProgressBar progress(&host);
    progress.setGeometry(20, 190, 220, 4);
    progress.setTextVisible(false);
    progress.setRange(0, 100);
    progress.setValue(50);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    const auto header = table.horizontalHeader()->grab().toImage();
    const auto headerBounds = QRectF(table.horizontalHeader()->rect());
    const QRectF headerBand(headerBounds.width() * .1, headerBounds.height() * .1, headerBounds.width() * .8, headerBounds.height() * .1);
    QVERIFY(uwf::test::colorCoverage(header, design.color(Color::SurfaceSoft), headerBand) >= uwf::test::kVisualMinimumCoverage);
    const auto half = progress.grab().toImage();
    const QRectF filled(progress.width() * .1, progress.height() * .2, progress.width() * .3, progress.height() * .6);
    const QRectF unfilled(progress.width() * .6, progress.height() * .2, progress.width() * .3, progress.height() * .6);
    QVERIFY(uwf::test::colorCoverage(half, design.color(Color::Accent), filled) >= uwf::test::kVisualMinimumCoverage);
    QVERIFY(uwf::test::colorCoverage(half, design.color(Color::Line), unfilled) >= uwf::test::kVisualMinimumCoverage);
    progress.setRange(0, 0);
    static_cast<void>(progress.grab());
    animation.advanceBy(300);
    const auto early = progress.grab().toImage();
    animation.advanceBy(400);
    const auto later = progress.grab().toImage();
    QVERIFY(uwf::test::changedCoverage(early, later, progress.rect()) > 0);
    progress.hide();
    const auto phase = progress.property("designBusyPhase");
    animation.advanceBy(600);
    QCOMPARE(progress.property("designBusyPhase"), phase);
  }
  void determinateProgressInterpolatesWithoutDelayingTheReportedValue() {
    uwf::test::AnimationClock animation;
    QProgressBar progress;
    progress.setTextVisible(false);
    progress.setRange(0, 100);
    progress.setValue(0);
    progress.resize(200, 4);
    progress.show();
    QVERIFY(QTest::qWaitForWindowExposed(&progress));
    progress.setValue(80);
    QCOMPARE(progress.value(), 80);
    animation.advanceBy(125);
    const auto halfway = progress.grab().toImage();
    const QRectF firstPart(progress.width() * .1, progress.height() * .2, progress.width() * .1, progress.height() * .6);
    const QRectF laterPart(progress.width() * .6, progress.height() * .2, progress.width() * .1, progress.height() * .6);
    QVERIFY(uwf::test::colorCoverage(halfway, uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Accent), firstPart) >=
            uwf::test::kVisualMinimumCoverage);
    QVERIFY(uwf::test::colorCoverage(halfway, uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Line), laterPart) >=
            uwf::test::kVisualMinimumCoverage);
    animation.advanceBy(140);
    const auto complete = progress.grab().toImage();
    QVERIFY(uwf::test::colorCoverage(complete, uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Accent), laterPart) >=
            uwf::test::kVisualMinimumCoverage);
    QCOMPARE(progress.value(), 80);
  }
  void blockedModelResetUpdatesSwitchPresentation_data() {
    QTest::addColumn<bool>("light");
    QTest::newRow("light") << true;
    QTest::newRow("dark") << false;
  }
  void blockedModelResetUpdatesSwitchPresentation() {
    QFETCH(bool, light);
    auto& theme = uwf::ui::DesignSystem::instance();
    theme.setLight(light);
    QApplication::setPalette(theme.palette());
    QWidget window;
    window.resize(200, 100);
    uwf::ui::SwitchButton control(&window);
    control.move(20, 20);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    uwf::test::moveMouse(window.windowHandle(), QPoint(170, 80));
    QTRY_VERIFY(!control.underMouse());
    QSignalSpy changes(&control, &QAbstractButton::toggled);
    {
      const QSignalBlocker blocker(&control);
      control.setChecked(true);
    }
    QVERIFY(control.isChecked());
    QCOMPARE(changes.count(), 0);
    const auto frame = control.grab().toImage();
    const QRectF thumb(control.width() * .6, control.height() * .3, control.width() * .2, control.height() * .4);
    const QRectF track(control.width() * .15, control.height() * .3, control.width() * .2, control.height() * .4);
    QVERIFY(uwf::test::colorCoverage(frame, theme.color(uwf::ui::design::Color::AccentText), thumb) >= uwf::test::kVisualMinimumCoverage);
    QVERIFY(uwf::test::colorCoverage(frame, theme.color(uwf::ui::design::Color::Accent), track) >= uwf::test::kVisualMinimumCoverage);
    QTest::keyClick(&control, Qt::Key_Space);
    QVERIFY(!control.isChecked());
    QCOMPARE(changes.count(), 1);
    control.setEnabled(false);
    QTest::keyClick(&control, Qt::Key_Space);
    QVERIFY(!control.isChecked());
    QCOMPARE(changes.count(), 1);
  }
  void switchUserInputMovesCenteredThumb_data() {
    QTest::addColumn<bool>("light");
    QTest::newRow("dark") << false;
    QTest::newRow("light") << true;
  }
  void switchUserInputMovesCenteredThumb() {
    QFETCH(bool, light);
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    QApplication::setPalette(design.palette());
    uwf::test::AnimationClock clock;
    QWidget window;
    window.resize(220, 110);
    uwf::ui::SwitchButton control(&window);
    control.move(30, 30);
    connect(&control, &QAbstractButton::toggled, &control, [&](bool checked) {
      const QSignalBlocker blocker(&control);
      control.setChecked(checked);
    });
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    uwf::test::moveMouse(window.windowHandle(), QPoint(190, 90));
    QTRY_VERIFY(!control.underMouse());
    clock.advanceBy(180);
    const auto center = [&](const QString& frameName) {
      const auto frame = control.grab().toImage();
      const auto scale = frame.devicePixelRatio();
      const auto onInk = design.color(uwf::ui::design::Color::AccentText);
      const auto offInk = design.color(uwf::ui::design::Color::Muted);
      QPointF total;
      int count = 0;
      for (int y = 0; y < frame.height(); ++y)
        for (int x = 0; x < frame.width(); ++x)
          if (frame.pixelColor(x, y) == onInk || frame.pixelColor(x, y) == offInk) {
            total += QPointF((x + .5) / scale, (y + .5) / scale);
            ++count;
          }
      if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
        const auto path = directory + (light ? QStringLiteral("/light-switch/") : QStringLiteral("/dark-switch/"));
        QDir().mkpath(path);
        frame.save(path + frameName + QStringLiteral(".png"));
      }
      return count ? total / count : QPointF(-1, -1);
    };
    const qreal diameter = control.height();
    const qreal offPosition = diameter / 2;
    const qreal onPosition = control.width() - diameter / 2;
    const auto aligned = [&](const QPointF& point, const qreal position) {
      return point.x() >= 0 && qAbs(point.x() - position) / diameter <= uwf::test::kVisualAlignmentRatio &&
             qAbs(point.y() - diameter / 2) / diameter <= uwf::test::kVisualAlignmentRatio;
    };
    const auto off = center(QStringLiteral("00-off"));
    QVERIFY(aligned(off, offPosition));
    uwf::test::clickMouse(&control, Qt::LeftButton);
    QVERIFY(control.isChecked());
    uwf::test::moveMouse(window.windowHandle(), QPoint(190, 90));
    QTRY_VERIFY(!control.underMouse());
    clock.advanceBy(35);
    const auto movingOn = center(QStringLiteral("01-moving-on"));
    QVERIFY(movingOn.x() > offPosition && movingOn.x() < onPosition);
    QVERIFY(qAbs(movingOn.y() - diameter / 2) / diameter <= uwf::test::kVisualAlignmentRatio);
    clock.advanceBy(160);
    const auto on = center(QStringLiteral("02-on"));
    QVERIFY(aligned(on, onPosition));
    QTest::keyClick(&control, Qt::Key_Space);
    QVERIFY(!control.isChecked());
    clock.advanceBy(35);
    const auto movingOff = center(QStringLiteral("03-moving-off"));
    QVERIFY(movingOff.x() > offPosition && movingOff.x() < onPosition);
    QVERIFY(qAbs(movingOff.y() - diameter / 2) / diameter <= uwf::test::kVisualAlignmentRatio);
    clock.advanceBy(160);
    const auto offAgain = center(QStringLiteral("04-off-again"));
    QVERIFY(aligned(offAgain, offPosition));
    QTest::keyClick(&control, Qt::Key_Space);
    clock.advanceBy(35);
    const auto beforeReverse = center(QStringLiteral("05-reversing"));
    QTest::keyClick(&control, Qt::Key_Space);
    const auto reversed = center(QStringLiteral("06-reversed"));
    QVERIFY(qAbs(reversed.x() - beforeReverse.x()) / diameter <= uwf::test::kVisualAlignmentRatio);
    clock.advanceBy(180);
    QVERIFY(aligned(center(QStringLiteral("07-restored")), offPosition));
  }
  void repeatedClickDeselectsWithoutBreakingExtendedSelection() {
    QStandardItemModel model;
    for (int row = 0; row < 8; ++row) model.appendRow(new QStandardItem(QString::number(row)));
    uwf::ui::SelectionListView view;
    view.setModel(&model);
    view.resize(360, 300);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    const auto click = [&](int row, Qt::KeyboardModifiers modifiers = Qt::NoModifier) {
      uwf::test::clickMouse(view.viewport(), Qt::LeftButton, modifiers, view.visualRect(model.index(row, 0)).center());
    };
    click(1);
    QCOMPARE(view.selectionModel()->selectedRows().size(), 1);
    click(1);
    QVERIFY(view.selectionModel()->selectedRows().isEmpty());
    click(1);
    click(3, Qt::ControlModifier);
    QCOMPARE(view.selectionModel()->selectedRows().size(), 2);
    click(3, Qt::ControlModifier);
    QCOMPARE(view.selectionModel()->selectedRows().size(), 1);
    view.clearSelection();
    view.setCurrentIndex({});
    click(1);
    click(4, Qt::ShiftModifier);
    QCOMPARE(view.selectionModel()->selectedRows().size(), 4);
    const QPoint start = view.visualRect(model.index(1, 0)).center(), end = view.visualRect(model.index(5, 0)).center();
    view.clearSelection();
    uwf::test::pressMouse(view.viewport(), Qt::LeftButton, Qt::NoModifier, start);
    uwf::test::moveMouse(view.viewport(), end, 30);
    uwf::test::releaseMouse(view.viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QVERIFY(view.selectionModel()->selectedRows().size() >= 4);
  }
  void keyboardTraversalVisitsEachControlOnce() {
    QWidget window;
    auto* layout = new QVBoxLayout(&window);
    auto* first = new QPushButton(QStringLiteral("First"), &window);
    auto* toggle = new uwf::ui::SwitchButton(&window);
    auto* last = new QPushButton(QStringLiteral("Last"), &window);
    layout->addWidget(first);
    layout->addWidget(toggle);
    layout->addWidget(last);
    window.show();
    window.raise();
    window.activateWindow();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(QTest::qWaitForWindowActive(&window));
    first->setFocus(Qt::TabFocusReason);
    QTRY_VERIFY(first->hasFocus());
    QTest::keyClick(first, Qt::Key_Tab);
    QTRY_VERIFY(toggle->hasFocus());
    QTest::keyClick(toggle, Qt::Key_Tab);
    QTRY_VERIFY(last->hasFocus());
  }
  void dangerButtonFeedback_data() {
    QTest::addColumn<bool>("light");
    QTest::addColumn<QString>("role");
    for (const bool light : {false, true})
      for (const auto& role : {QStringLiteral("danger"), QStringLiteral("dangerQuiet"), QStringLiteral("destructiveSecondary")})
        QTest::newRow(qUtf8Printable((light ? QStringLiteral("light-") : QStringLiteral("dark-")) + role)) << light << role;
  }
  void dangerButtonFeedback() {
    QFETCH(bool, light);
    QFETCH(QString, role);
    using uwf::ui::design::Color;
    auto& design = uwf::ui::DesignSystem::instance();
    const auto previous = design.isLight();
    const auto restore = qScopeGuard([&] { design.setLight(previous); });
    design.setLight(light);
    QPushButton button;
    button.setProperty("designRole", role);
    button.resize(160, 36);
    for (const bool pressed : {false, true}) {
      QStyleOptionButton option;
      option.rect = button.rect();
      option.state = QStyle::State_Enabled | QStyle::State_MouseOver;
      if (pressed) option.state |= QStyle::State_Sunken;
      QImage image(button.size(), QImage::Format_ARGB32_Premultiplied);
      image.fill(design.color(Color::Footer));
      QPainter painter(&image);
      QApplication::style()->drawControl(QStyle::CE_PushButton, &option, &painter, &button);
      painter.end();
      QVERIFY(uwf::test::colorCoverage(image, design.color(pressed ? Color::DangerButtonPressed : Color::DangerButtonHover),
                                       uwf::test::interiorRegion(button.rect())) >= uwf::test::kVisualMinimumCoverage);
      const auto outline = uwf::ui::DesignSystem::mix(design.color(Color::Line), design.color(Color::Danger), .4);
      QVERIFY(uwf::test::colorCoverage(image, outline, button.rect()) > 0);
    }
  }
  void buttonMotionAndCanceledPress_data() {
    QTest::addColumn<bool>("light");
    QTest::addColumn<QString>("role");
    for (bool light : {false, true})
      for (const auto& role :
           {QStringLiteral("secondary"), QStringLiteral("primary"), QStringLiteral("destructive"), QStringLiteral("close"), QStringLiteral("quiet")})
        QTest::newRow(qUtf8Printable((light ? QStringLiteral("light-") : QStringLiteral("dark-")) + role)) << light << role;
  }
  void buttonMotionAndCanceledPress() {
    QFETCH(bool, light);
    QFETCH(QString, role);
    using uwf::ui::design::Color;
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    QApplication::setPalette(design.palette());
    uwf::test::AnimationClock clock;
    QWidget window;
    window.resize(260, 120);
    QPushButton button(QStringLiteral("Action"), &window);
    button.setProperty("designRole", role);
    button.setGeometry(20, 20, 160, 36);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.raise();
    window.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&window));
    QSignalSpy clicks(&button, &QAbstractButton::clicked);
    const QString directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT");
    QDir output(directory);
    if (!directory.isEmpty()) QVERIFY(output.mkpath(QString::fromLatin1(QTest::currentDataTag())));
    QJsonArray timeline;
    int frameNumber = 0;
    const auto frame = [&](const QString& step) {
      const auto image = button.grab().toImage();
      const QRectF fillRegion(button.width() * .1, button.height() * .25, button.width() * .15, button.height() * .5);
      const QColor ink = uwf::test::regionColor(image, fillRegion);
      timeline.append(QJsonObject{{"step", step}, {"elapsedMs", static_cast<double>(clock.totalTime())}, {"background", ink.name()}});
      if (!directory.isEmpty() && !image.save(output.filePath(QString::fromLatin1(QTest::currentDataTag()) +
                                                              QStringLiteral("/%1-%2.png").arg(frameNumber++, 3, 10, QLatin1Char('0')).arg(step))))
        QTest::qFail("Could not save the motion frame.", __FILE__, __LINE__);
      return image;
    };
    uwf::test::moveMouse(window.windowHandle(), QPoint(240, 90));
    QTRY_VERIFY(!button.underMouse());
    clock.advanceBy(160);
    const auto resting = frame(QStringLiteral("rest"));
    const Color hover = role == QStringLiteral("primary")       ? Color::AccentHover
                        : role == QStringLiteral("destructive") ? Color::DangerFillHover
                        : role == QStringLiteral("close")       ? Color::CloseHover
                                                                : Color::ButtonHover;
    uwf::test::moveMouse(button);
    QTRY_VERIFY(button.underMouse());
    clock.advanceBy(40);
    const auto entering = frame(QStringLiteral("hover-40ms"));
    clock.advanceBy(120);
    const auto hovered = frame(QStringLiteral("hover-settled"));
    const QRectF fillRegion(button.width() * .1, button.height() * .25, button.width() * .15, button.height() * .5);
    QVERIFY(uwf::test::changedCoverage(resting, entering, fillRegion) > 0);
    QVERIFY(uwf::test::changedCoverage(entering, hovered, fillRegion) > 0);
    QVERIFY(uwf::test::colorCoverage(hovered, design.color(hover), fillRegion) >= uwf::test::kVisualMinimumCoverage);
    const Color active = role == QStringLiteral("primary")       ? Color::AccentPressed
                         : role == QStringLiteral("destructive") ? Color::DangerFillPressed
                         : role == QStringLiteral("close")       ? Color::ClosePressed
                                                                 : Color::ButtonPressed;
    uwf::test::pressMouse(&button, Qt::LeftButton);
    QTRY_VERIFY(button.isDown());
    QCOMPARE(QApplication::mouseButtons(), Qt::MouseButtons{Qt::LeftButton});
    clock.advanceBy(40);
    const auto pressing = frame(QStringLiteral("press-40ms"));
    clock.advanceBy(120);
    const auto pressed = frame(QStringLiteral("press-settled"));
    QVERIFY(uwf::test::changedCoverage(hovered, pressing, fillRegion) > 0);
    QVERIFY(uwf::test::changedCoverage(pressing, pressed, fillRegion) > 0);
    QVERIFY(uwf::test::colorCoverage(pressed, design.color(active), fillRegion) >= uwf::test::kVisualMinimumCoverage);
    uwf::test::moveMouse(window.windowHandle(), QPoint(240, 90));
    uwf::test::releaseMouse(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, QPoint(240, 90));
    QCOMPARE(QApplication::mouseButtons(), Qt::MouseButtons{Qt::NoButton});
    QTRY_VERIFY(!button.underMouse());
    clock.advanceBy(160);
    QVERIFY(uwf::test::changedCoverage(resting, frame(QStringLiteral("canceled-outside")), fillRegion) <= uwf::test::kVisualAlignmentRatio);
    QCOMPARE(clicks.count(), 0);
    uwf::test::clickMouse(&button, Qt::LeftButton);
    QCOMPARE(clicks.count(), 1);
    clock.advanceBy(160);
    frame(QStringLiteral("clicked"));
    button.setEnabled(false);
    clock.advanceBy(160);
    const auto disabled = frame(QStringLiteral("disabled"));
    uwf::test::moveMouse(window.windowHandle(), QPoint(240, 90));
    uwf::test::moveMouse(button);
    uwf::test::clickMouse(&button, Qt::LeftButton);
    clock.advanceBy(160);
    QVERIFY(uwf::test::changedCoverage(disabled, frame(QStringLiteral("disabled-input")), fillRegion) <= uwf::test::kVisualAlignmentRatio);
    QCOMPARE(clicks.count(), 1);
    if (!directory.isEmpty()) {
      QSaveFile file(output.filePath(QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral("/timeline.json")));
      QVERIFY(file.open(QIODevice::WriteOnly));
      file.write(QJsonDocument(timeline).toJson());
      QVERIFY(file.commit());
    }
  }
  void repeatedPointerEventsDoNotRestartButtonMotion() {
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(true);
    QApplication::setPalette(design.palette());
    uwf::test::AnimationClock clock;
    QWidget window;
    window.resize(240, 100);
    QPushButton button(QStringLiteral("Action"), &window);
    button.setGeometry(20, 20, 140, 36);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.raise();
    window.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&window));
    uwf::test::moveMouse(window.windowHandle(), QPoint(220, 90));
    QTRY_VERIFY(!button.underMouse());
    clock.advanceBy(160);
    uwf::test::moveMouse(button);
    QTRY_VERIFY(button.underMouse());
    clock.advanceBy(80);
    const QPointF local(button.rect().center());
    QEnterEvent repeated(local, button.mapTo(&window, local.toPoint()), button.mapToGlobal(local.toPoint()));
    QApplication::sendEvent(&button, &repeated);
    clock.advanceBy(40);
    const auto frame = button.grab().toImage();
    const QRectF fillRegion(button.width() * .1, button.height() * .25, button.width() * .15, button.height() * .5);
    QVERIFY(uwf::test::colorCoverage(frame, design.color(uwf::ui::design::Color::ButtonHover), fillRegion) >= uwf::test::kVisualMinimumCoverage);
  }
  void animationClockPreservesElapsedTimeAndRestoresNormalTiming() {
    QVariantAnimation animation;
    animation.setDuration(120);
    animation.setStartValue(0.0);
    animation.setEndValue(1.0);
    {
      uwf::test::AnimationClock clock;
      clock.advanceBy(500);
      animation.start();
      clock.advanceBy(40);
      QCOMPARE(animation.currentTime(), 40);
      QVERIFY(animation.currentValue().toReal() > 0 && animation.currentValue().toReal() < 1);
      clock.advanceBy(80);
      QCOMPARE(animation.currentValue().toReal(), 1.0);
    }
    animation.start();
    QTRY_COMPARE(animation.currentValue().toReal(), 1.0);
  }
  void focusIsVisibleOnlyForKeyboardNavigation_data() {
    QTest::addColumn<bool>("light");
    QTest::addColumn<QString>("kind");
    for (bool light : {false, true})
      for (const auto& kind : {QStringLiteral("button"), QStringLiteral("switch"), QStringLiteral("select")})
        QTest::newRow(qUtf8Printable((light ? QStringLiteral("light-") : QStringLiteral("dark-")) + kind)) << light << kind;
  }
  void focusIsVisibleOnlyForKeyboardNavigation() {
    QFETCH(bool, light);
    QFETCH(QString, kind);
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    QApplication::setPalette(design.palette());
    QWidget window;
    window.resize(260, 130);
    QWidget* control = kind == QStringLiteral("switch")   ? static_cast<QWidget*>(new uwf::ui::SwitchButton(&window))
                       : kind == QStringLiteral("select") ? static_cast<QWidget*>(new uwf::ui::SelectBox(&window))
                                                          : static_cast<QWidget*>(new QPushButton(QStringLiteral("Action"), &window));
    control->move(40, 40);
    if (kind != QStringLiteral("switch")) control->resize(120, 30);
    window.show();
    window.activateWindow();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(QTest::qWaitForWindowActive(&window));
    uwf::test::clickMouse(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, QPoint(230, 100));
    const auto outlineCoverage = [&] {
      const auto image = window.grab().toImage();
      const auto accent = design.color(uwf::ui::design::Color::Accent);
      const qreal reach = control->height() * .2;
      const QRectF band(control->x(), control->y() - reach, control->width(), reach);
      return uwf::test::colorCoverage(image, accent, band);
    };
    control->setFocus(Qt::TabFocusReason);
    QTRY_VERIFY(control->hasFocus());
    QVERIFY(outlineCoverage() <= uwf::test::kVisualColorRatio);
    control->clearFocus();
    QTest::keyClick(&window, Qt::Key_Tab);
    control->setFocus(Qt::OtherFocusReason);
    QTRY_VERIFY(control->hasFocus());
    QTRY_VERIFY(outlineCoverage() > uwf::test::kVisualAlignmentRatio);
    uwf::test::clickMouse(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, QPoint(230, 100));
    QTRY_VERIFY(outlineCoverage() <= uwf::test::kVisualColorRatio);
    control->clearFocus();
    control->setFocus(Qt::OtherFocusReason);
    QVERIFY(outlineCoverage() <= uwf::test::kVisualColorRatio);
  }
  void editingTextDoesNotEnableTheNavigationFocusRing_data() {
    QTest::addColumn<bool>("light");
    QTest::addColumn<QString>("kind");
    for (const bool light : {false, true})
      for (const auto& kind : {QStringLiteral("line"), QStringLiteral("number"), QStringLiteral("editable-select")})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(light ? "light" : "dark", kind))) << light << kind;
  }
  void editingTextDoesNotEnableTheNavigationFocusRing() {
    QFETCH(bool, light);
    QFETCH(QString, kind);
    auto& design = uwf::ui::DesignSystem::instance();
    const auto originalLight = design.isLight();
    const auto originalPalette = QApplication::palette();
    const auto originalClipboard = QApplication::clipboard()->text();
    const auto restore = qScopeGuard([&] {
      design.setLight(originalLight);
      QApplication::setPalette(originalPalette);
      QApplication::clipboard()->setText(originalClipboard);
    });
    design.setLight(light);
    QApplication::setPalette(design.palette());
    QWidget host;
    host.resize(320, 160);
    QWidget* field = nullptr;
    QLineEdit* editor = nullptr;
    if (kind == QStringLiteral("number")) {
      auto* number = new uwf::ui::NumberBox(&host);
      number->setRange(0, 9999);
      editor = number->findChild<QLineEdit*>();
      field = number;
    } else if (kind == QStringLiteral("editable-select")) {
      auto* select = new uwf::ui::SelectBox(&host);
      select->setEditable(true);
      select->addItems({QStringLiteral("12"), QStringLiteral("13")});
      editor = select->lineEdit();
      field = select;
    } else {
      editor = new QLineEdit(&host);
      field = editor;
    }
    QVERIFY(editor);
    field->setGeometry(40, 40, 220, 36);
    host.show();
    host.activateWindow();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    QVERIFY(QTest::qWaitForWindowActive(&host));
    uwf::test::clickMouse(editor, Qt::LeftButton);
    QTRY_VERIFY(editor->hasFocus());
    const auto hasRing = [&] {
      for (auto frames = host.findChildren<QFocusFrame*>(); auto* frame : std::as_const(frames))
        if (frame->isVisible() && frame->widget() == field) return true;
      return false;
    };
    QVERIFY(!hasRing());
    const QString pasted = QStringLiteral("42");
    QVERIFY(uwf::test::prepareClipboardText(pasted));
    editor->selectAll();
    // Deliver through QWindow, as real key input is seen there before the
    // focused editor receives it. This also covers embedded line edits.
    QTest::keyClick(host.windowHandle(), Qt::Key_V, Qt::ControlModifier);
    QCOMPARE(editor->text(), pasted);
    QVERIFY(!hasRing());
    for (const auto& [key, modifiers] :
         {std::pair{Qt::Key_Left, Qt::NoModifier}, std::pair{Qt::Key_Right, Qt::NoModifier}, std::pair{Qt::Key_Home, Qt::ControlModifier},
          std::pair{Qt::Key_End, Qt::ControlModifier}, std::pair{Qt::Key_A, Qt::ControlModifier}, std::pair{Qt::Key_C, Qt::ControlModifier},
          std::pair{Qt::Key_X, Qt::ControlModifier}, std::pair{Qt::Key_Z, Qt::ControlModifier}, std::pair{Qt::Key_Y, Qt::ControlModifier},
          std::pair{Qt::Key_Backspace, Qt::NoModifier}, std::pair{Qt::Key_Delete, Qt::NoModifier}, std::pair{Qt::Key_7, Qt::NoModifier}}) {
      QTest::keyClick(host.windowHandle(), key, modifiers);
      QVERIFY(!hasRing());
    }
    QVERIFY(uwf::test::prepareClipboardText(pasted));
    editor->selectAll();
    QTest::keyClick(host.windowHandle(), Qt::Key_Insert, Qt::ShiftModifier);
    QCOMPARE(editor->text(), pasted);
    QVERIFY(!hasRing());
    QInputMethodEvent input;
    input.setCommitString(QStringLiteral("8"));
    QApplication::sendEvent(editor, &input);
    QVERIFY(!hasRing());
  }
  void tabNavigationKeepsItsFocusRingWhileEditing_data() {
    QTest::addColumn<bool>("light");
    QTest::addColumn<bool>("backward");
    for (const bool light : {false, true})
      for (const bool backward : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(light ? "light" : "dark", backward ? "backtab" : "tab"))) << light << backward;
  }
  void tabNavigationKeepsItsFocusRingWhileEditing() {
    QFETCH(bool, light);
    QFETCH(bool, backward);
    auto& design = uwf::ui::DesignSystem::instance();
    const auto originalLight = design.isLight();
    const auto originalPalette = QApplication::palette();
    const auto originalClipboard = QApplication::clipboard()->text();
    const auto restore = qScopeGuard([&] {
      design.setLight(originalLight);
      QApplication::setPalette(originalPalette);
      QApplication::clipboard()->setText(originalClipboard);
    });
    design.setLight(light);
    QApplication::setPalette(design.palette());
    QWidget host;
    host.resize(360, 180);
    QPushButton before(QStringLiteral("Before"), &host), after(QStringLiteral("After"), &host);
    QLineEdit editor(&host);
    before.setGeometry(30, 20, 100, 30);
    editor.setGeometry(30, 65, 260, 36);
    after.setGeometry(30, 120, 100, 30);
    QWidget::setTabOrder(&before, &editor);
    QWidget::setTabOrder(&editor, &after);
    host.show();
    host.activateWindow();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    QVERIFY(QTest::qWaitForWindowActive(&host));
    uwf::test::clickMouse(backward ? &after : &before, Qt::LeftButton);
    QTest::keyClick(host.windowHandle(), Qt::Key_Tab, backward ? Qt::ShiftModifier : Qt::NoModifier);
    QTRY_VERIFY(editor.hasFocus());
    const auto hasRing = [&] {
      for (auto frames = host.findChildren<QFocusFrame*>(); auto* frame : std::as_const(frames))
        if (frame->isVisible() && frame->widget() == &editor) return true;
      return false;
    };
    QTRY_VERIFY(hasRing());
    const QString pasted = QStringLiteral("Pasted text");
    QVERIFY(uwf::test::prepareClipboardText(pasted));
    QTest::keyClick(host.windowHandle(), Qt::Key_V, Qt::ControlModifier);
    QCOMPARE(editor.text(), pasted);
    QVERIFY(hasRing());
    QTest::keyClick(host.windowHandle(), Qt::Key_Left);
    QVERIFY(hasRing());
    uwf::test::clickMouse(&editor, Qt::LeftButton);
    QTRY_VERIFY(!hasRing());
  }
  void selectionNavigationStillEnablesTheFocusRing() {
    QWidget host;
    host.resize(340, 200);
    QListView list(&host);
    QStringListModel model({QStringLiteral("First"), QStringLiteral("Second")});
    list.setModel(&model);
    list.setGeometry(20, 20, 160, 140);
    QPushButton button(QStringLiteral("Action"), &host);
    button.setGeometry(200, 50, 100, 36);
    host.show();
    host.activateWindow();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    QVERIFY(QTest::qWaitForWindowActive(&host));
    uwf::test::clickMouse(list.viewport(), Qt::LeftButton, Qt::NoModifier, list.visualRect(model.index(0, 0)).center());
    QTest::keyClick(host.windowHandle(), Qt::Key_Down);
    QCOMPARE(list.currentIndex().row(), 1);
    button.setFocus(Qt::OtherFocusReason);
    QTRY_VERIFY(button.hasFocus());
    const auto hasRing = [&] {
      for (auto frames = host.findChildren<QFocusFrame*>(); auto* frame : std::as_const(frames))
        if (frame->isVisible() && frame->widget() == &button) return true;
      return false;
    };
    QTRY_VERIFY(hasRing());
  }
  void selectPreservesValueUntilAccepted_data() {
    QTest::addColumn<bool>("light");
    QTest::newRow("dark") << false;
    QTest::newRow("light") << true;
  }
  void nativePointerReceiptsSurvivePopupGeometryEpoch_data() {
    QTest::addColumn<bool>("popup");
    QTest::addColumn<bool>("rtl");
    for (const bool popup : {false, true})
      for (const bool rtl : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(popup ? "popup-capture" : "ordinary-window", rtl ? "rtl" : "ltr"))) << popup << rtl;
  }
  void nativePointerReceiptsSurvivePopupGeometryEpoch() {
    QFETCH(bool, popup);
    QFETCH(bool, rtl);
    QWidget host;
    host.resize(400, 300);
    host.move(QGuiApplication::primaryScreen()->availableGeometry().center() - QPoint(200, 150));
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    uwf::test::moveMouse(&host, QPoint(380, 280));
    QWidget owner(&host, (popup ? Qt::Popup : Qt::Window) | Qt::FramelessWindowHint);
    owner.setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
    owner.resize(280, 160);
    QPushButton action(QStringLiteral("Native action"), &owner);
    action.setGeometry(30, 30, 180, 40);
    action.setMouseTracking(true);
    owner.move(host.mapToGlobal(QPoint(30, 30)));
    owner.show();
    QVERIFY(QTest::qWaitForWindowExposed(&owner));
    const auto nativeOwner = reinterpret_cast<HWND>(owner.winId());
    if (popup) {
      QCOMPARE(QApplication::activePopupWidget(), &owner);
      QTRY_COMPARE(GetCapture(), nativeOwner);
    } else {
      owner.windowHandle()->requestActivate();
      QVERIFY(QTest::qWaitForWindowActive(&owner));
      QCOMPARE(GetCapture(), nullptr);
    }
    const QPoint point(60, 20);
    const auto originalPoint = action.mapToGlobal(point);
    ResizeDuringPointerDelivery delivery(owner, action, point);
    QSignalSpy clicks(&action, &QPushButton::clicked);
    delivery.armed = true;
    // Resize the owner without moving the target point to exercise Qt's cached pointer position.
    uwf::test::moveMouse(&action, point);
    QCOMPARE(delivery.geometryChanges, 1);
    QCOMPARE(owner.size(), QSize(320, 160));
    QCOMPARE(action.mapToGlobal(point), originalPoint);
    QVERIFY(action.underMouse());
    QVERIFY(delivery.enters + delivery.nativeQtMoves >= 2);
    QCOMPARE(QApplication::mouseButtons(), Qt::NoButton);
    QVERIFY((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0);
    if (popup) {
      QCOMPARE(QApplication::activePopupWidget(), &owner);
      QCOMPARE(GetCapture(), nativeOwner);
    }
    uwf::test::clickMouse(&action, Qt::LeftButton, Qt::NoModifier, point);
    QTRY_COMPARE(clicks.count(), 1);
    QCOMPARE(QApplication::mouseButtons(), Qt::NoButton);
    QVERIFY((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0);
  }
  void selectPopupsContainOnlyTheirVisibleRows_data() {
    QTest::addColumn<bool>("light");
    QTest::addColumn<bool>("rtl");
    QTest::addColumn<int>("count");
    for (const bool light : {false, true})
      for (const bool rtl : {false, true})
        for (const int count : {0, 1, 2, 20})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(light ? "light" : "dark", rtl ? "rtl" : "ltr").arg(count))) << light << rtl << count;
  }
  void selectPopupsContainOnlyTheirVisibleRows() {
    QFETCH(bool, light);
    QFETCH(bool, rtl);
    QFETCH(int, count);
    auto& design = uwf::ui::DesignSystem::instance();
    const auto originalLight = design.isLight();
    const auto originalPalette = QApplication::palette();
    const auto restore = qScopeGuard([&] {
      design.setLight(originalLight);
      QApplication::setPalette(originalPalette);
    });
    design.setLight(light);
    QApplication::setPalette(design.palette());
    QWidget host;
    host.setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
    host.resize(400, 300);
    uwf::ui::SelectBox control(&host);
    control.setGeometry(40, 40, 240, 36);
    control.setMaxVisibleItems(4);
    for (int row = 0; row < count; ++row) control.addItem(QStringLiteral("Supported files %1").arg(row));
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    control.showPopup();
    auto* popup = QApplication::activePopupWidget();
    if (count == 0) {
      QVERIFY(!popup);
      QVERIFY(!control.property("designExpanded").toBool());
      return;
    }
    QVERIFY(popup);
    auto* options = popup->findChild<QListView*>(QStringLiteral("selectOptions"));
    QVERIFY(options);
    QCOMPARE(options->model()->rowCount(), count);
    const int rows = std::min(count, control.maxVisibleItems());
    QTRY_COMPARE(options->viewport()->height(), rows * uwf::ui::design::menuRowHeight);
    QCOMPARE(options->verticalScrollBar()->maximum() > 0, count > rows);
    for (int row = 0; row < rows; ++row) {
      const auto index = options->model()->index(row, 0);
      const auto bounds = options->visualRect(index);
      QVERIFY(options->viewport()->rect().contains(bounds));
      QCOMPARE(options->indexAt(bounds.center()), index);
      QVERIFY(index.flags().testFlag(Qt::ItemIsEnabled));
      QVERIFY(index.flags().testFlag(Qt::ItemIsSelectable));
      QCOMPARE(index.data().toString(), QStringLiteral("Supported files %1").arg(row));
    }
    // The viewport bottom must hit the last row despite QListView's preferred minimum size.
    QCOMPARE(options->indexAt(QPoint(options->viewport()->width() / 2, options->viewport()->height() - 1)).row(), rows - 1);
    const auto first = options->model()->index(0, 0);
    const auto firstRow = options->visualRect(first);
    const auto rendered = options->viewport()->grab(firstRow).toImage();
    control.setItemText(0, {});
    const auto checkOnly = options->viewport()->grab(firstRow).toImage();
    QVERIFY(rendered != checkOnly);
    control.setCurrentIndex(-1);
    const auto empty = options->viewport()->grab(firstRow).toImage();
    QVERIFY(checkOnly != empty);
    control.setItemText(0, QStringLiteral("Supported files 0"));
    control.setCurrentIndex(0);
    QSignalSpy activations(&control, &QComboBox::activated);
    QSignalSpy changes(&control, &QComboBox::currentIndexChanged);
    uwf::test::clickMouse(options->viewport(), Qt::LeftButton, Qt::NoModifier, firstRow.center());
    QTRY_VERIFY(!popup->isVisible());
    QCOMPARE(control.currentIndex(), 0);
    QCOMPARE(activations.count(), 1);
    QCOMPARE(changes.count(), 0);
    control.showPopup();
    QTRY_VERIFY(popup->isVisible());
    QTRY_COMPARE(options->viewport()->height(), rows * uwf::ui::design::menuRowHeight);
    QTest::keyClick(options, Qt::Key_Return);
    QTRY_VERIFY(!popup->isVisible());
    QCOMPARE(control.currentIndex(), 0);
    QCOMPARE(activations.count(), 2);
    QCOMPARE(changes.count(), 0);
  }
  void selectPopupAdaptsWhenOptionsAreReplaced_data() {
    QTest::addColumn<bool>("hover");
    QTest::addColumn<bool>("rtl");
    QTest::addColumn<int>("width");
    for (const bool hover : {false, true})
      for (const bool rtl : {false, true})
        for (const int width : {120, 240})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(hover ? "hover-then-enter" : "keyboard", rtl ? "rtl" : "ltr").arg(width)))
              << hover << rtl << width;
  }
  void selectPopupAdaptsWhenOptionsAreReplaced() {
    QFETCH(bool, hover);
    QFETCH(bool, rtl);
    QFETCH(int, width);
    QWidget host;
    host.setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
    host.resize(400, 300);
    uwf::ui::SelectBox control(&host);
    control.move(40, 40);
    control.setFixedSize(width, 36);
    control.setMaxVisibleItems(4);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    QSignalSpy activations(&control, &QComboBox::activated);
    int accepted = 0;
    for (const int count : {0, 1, 5, 1, 0, 2, 1}) {
      control.clear();
      for (int row = 0; row < count; ++row) control.addItem(QStringLiteral("Filter %1").arg(row));
      if (count) control.setCurrentIndex(count - 1);
      // Reopening and resizing the popup can place a row under the native
      // pointer. Start outside it so hover cannot change keyboard selection.
      uwf::test::moveMouse(control);
      control.showPopup();
      auto* popup = QApplication::activePopupWidget();
      if (count == 0) {
        QVERIFY(!popup);
        continue;
      }
      QVERIFY(popup);
      auto* options = popup->findChild<QListView*>(QStringLiteral("selectOptions"));
      QVERIFY(options);
      QCOMPARE(options->model()->rowCount(), count);
      QTRY_COMPARE(options->viewport()->height(), std::min(count, control.maxVisibleItems()) * uwf::ui::design::menuRowHeight);
      const auto index = options->model()->index(count - 1, 0);
      QTRY_VERIFY(options->viewport()->rect().contains(options->visualRect(index)));
      QCOMPARE(options->indexAt(options->visualRect(index).center()), index);
      QCOMPARE(options->currentIndex(), index);
      int expectedRow = count - 1;
      if (hover) {
        expectedRow = count / 2;
        const auto hovered = options->model()->index(expectedRow, 0);
        uwf::test::moveMouse(options->viewport(), options->visualRect(hovered).center());
        QTRY_COMPARE(options->currentIndex(), hovered);
      }
      QCOMPARE(control.currentIndex(), count - 1);
      QCOMPARE(activations.count(), accepted);
      QTest::keyClick(options, Qt::Key_Return);
      QTRY_VERIFY(!popup->isVisible());
      QCOMPARE(control.currentIndex(), expectedRow);
      QCOMPARE(activations.count(), ++accepted);
    }
  }
  void selectPreservesValueUntilAccepted() {
    QFETCH(bool, light);
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    QApplication::setPalette(design.palette());
    QWidget window;
    window.resize(320, 240);
    uwf::ui::SelectBox control(&window);
    control.setGeometry(30, 30, 140, 31);
    control.addItem(QStringLiteral("按盘符"), false);
    control.addItem(QStringLiteral("按卷 ID"), true);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QSignalSpy changes(&control, &QComboBox::currentIndexChanged);
    QSignalSpy activations(&control, &QComboBox::activated);
    control.showPopup();
    auto* popup = QApplication::activePopupWidget();
    QVERIFY(popup);
    auto* options = popup->findChild<QListView*>(QStringLiteral("selectOptions"));
    QVERIFY(options);
    QTest::keyClick(options, Qt::Key_Down);
    QCOMPARE(control.currentIndex(), 0);
    QCOMPARE(options->currentIndex().row(), 1);
    QTest::keyClick(options, Qt::Key_Escape);
    QVERIFY(!popup->isVisible());
    QCOMPARE(control.currentIndex(), 0);
    QCOMPARE(changes.count(), 0);
    QCOMPARE(activations.count(), 0);
    control.showPopup();
    QTest::keyClick(options, Qt::Key_Down);
    QTest::keyClick(options, Qt::Key_Return);
    QVERIFY(!popup->isVisible());
    QCOMPARE(control.currentData().toBool(), true);
    QCOMPARE(changes.count(), 1);
    QCOMPARE(activations.count(), 1);
    control.showPopup();
    QTest::keyClick(options, Qt::Key_Return);
    QCOMPARE(changes.count(), 1);
    QCOMPARE(activations.count(), 2);
    control.setEnabled(false);
    control.showPopup();
    QVERIFY(!popup->isVisible());
  }
  void selectReturnsToRestAfterMouseSelection_data() {
    QTest::addColumn<bool>("light");
    QTest::newRow("dark") << false;
    QTest::newRow("light") << true;
  }
  void selectReturnsToRestAfterMouseSelection() {
    QFETCH(bool, light);
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    QApplication::setPalette(design.palette());
    QWidget window;
    window.resize(380, 260);
    uwf::ui::SelectBox control(&window);
    control.setGeometry(30, 30, 140, 31);
    control.addItem(QStringLiteral("盘符"), false);
    control.addItem(QStringLiteral("卷 ID"), true);
    window.show();
    window.activateWindow();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(QTest::qWaitForWindowActive(&window));
    const auto backgroundCoverage = [&] {
      const auto image = control.grab().toImage();
      const QRectF fillRegion(control.width() * .05, control.height() * .2, control.width() * .05, control.height() * .6);
      return uwf::test::colorCoverage(image, design.color(uwf::ui::design::Color::Surface), fillRegion);
    };
    for (int attempt = 0; attempt < 8; ++attempt) {
      uwf::test::clickMouse(&control, Qt::LeftButton);
      auto* popup = QApplication::activePopupWidget();
      QVERIFY(popup);
      auto* options = popup->findChild<QListView*>(QStringLiteral("selectOptions"));
      QVERIFY(options);
      const int next = 1 - control.currentIndex();
      const QPoint row = options->visualRect(options->model()->index(next, 0)).center();
      uwf::test::clickMouse(options->viewport(), Qt::LeftButton, Qt::NoModifier, row);
      QTRY_VERIFY(!popup->isVisible());
      QCOMPARE(control.currentIndex(), next);
      uwf::test::moveMouse(window.windowHandle(), QPoint(340, 220));
      QTRY_VERIFY(backgroundCoverage() >= uwf::test::kVisualMinimumCoverage);
    }
  }
  void commandAndSelectionPopupsShareTransparentCorners_data() {
    QTest::addColumn<bool>("light");
    QTest::newRow("dark") << false;
    QTest::newRow("light") << true;
  }
  void commandAndSelectionPopupsShareTransparentCorners() {
    QFETCH(bool, light);
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    QApplication::setPalette(design.palette());
    QWidget window;
    window.resize(320, 240);
    uwf::ui::SelectBox select(&window);
    select.setGeometry(20, 20, 140, 31);
    select.addItems({QStringLiteral("First"), QStringLiteral("Second")});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    select.showPopup();
    auto* popup = QApplication::activePopupWidget();
    QVERIFY(popup);
    QTRY_VERIFY(popup->isVisible());
    QVERIFY(popup->testAttribute(Qt::WA_TranslucentBackground));
    const auto selectImage = popup->grab().toImage();
    const QRectF corner(0, 0, uwf::ui::design::menuRadius, uwf::ui::design::menuRadius);
    const auto selectCorner = uwf::test::regionColor(selectImage, corner);
    const auto selectCenter = uwf::test::regionColor(selectImage, uwf::test::interiorRegion(popup->rect()));
    QVERIFY(selectCorner.isValid() && selectCenter.isValid());
    QVERIFY(selectCenter.alphaF() - selectCorner.alphaF() > uwf::test::kVisualColorRatio);
    select.hidePopup();
    uwf::ui::CommandMenu menu(&window);
    auto* first = menu.addAction(QStringLiteral("First"));
    menu.addAction(QStringLiteral("Unavailable"))->setEnabled(false);
    auto* second = menu.addAction(QStringLiteral("Second"));
    QSignalSpy firstCalls(first, &QAction::triggered), secondCalls(second, &QAction::triggered);
    menu.popup(window.mapToGlobal(QPoint(20, 60)));
    QTRY_COMPARE(QApplication::activePopupWidget(), &menu);
    QVERIFY(menu.testAttribute(Qt::WA_TranslucentBackground));
    const auto menuImage = menu.grab().toImage();
    const auto menuCorner = uwf::test::regionColor(menuImage, corner);
    const auto menuCenter = uwf::test::regionColor(menuImage, uwf::test::interiorRegion(menu.rect()));
    QVERIFY(menuCorner.isValid() && menuCenter.isValid());
    QVERIFY(menuCenter.alphaF() - menuCorner.alphaF() > uwf::test::kVisualColorRatio);
    QVERIFY(qAbs(menuCorner.alphaF() - selectCorner.alphaF()) <= uwf::test::kVisualColorRatio);
    QTest::keyClick(&menu, Qt::Key_Down);
    QCOMPARE(menu.activeAction(), first);
    QTest::keyClick(&menu, Qt::Key_Down);
    QCOMPARE(menu.activeAction(), second);
    QTest::keyClick(&menu, Qt::Key_Return);
    QTRY_VERIFY(!menu.isVisible());
    QCOMPARE(firstCalls.count(), 0);
    QCOMPARE(secondCalls.count(), 1);
    menu.popup(window.mapToGlobal(QPoint(20, 60)));
    QTest::keyClick(&menu, Qt::Key_Escape);
    QTRY_VERIFY(!menu.isVisible());
    QCOMPARE(secondCalls.count(), 1);
  }
  void buttonIconsUseGeometricCenter_data() {
    QTest::addColumn<bool>("tool");
    QTest::addColumn<QSize>("size");
    for (bool tool : {false, true})
      for (const QSize size : {QSize(32, 30), QSize(32, 31), QSize(32, 32), QSize(40, 36), QSize(28, 28)})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2x%3").arg(tool ? "tool" : "push").arg(size.width()).arg(size.height()))) << tool << size;
  }
  void buttonIconsUseGeometricCenter() {
    QFETCH(bool, tool);
    QFETCH(QSize, size);
    QWidget window;
    window.resize(240, 120);
    QAbstractButton* button = tool ? static_cast<QAbstractButton*>(new QToolButton(&window)) : static_cast<QAbstractButton*>(new QPushButton(&window));
    button->setProperty("designRole", "quiet");
    button->setGeometry(QRect(QPoint(30, 30), size));
    QPixmap glyph(16, 16);
    glyph.fill(Qt::white);
    button->setIcon(QIcon(glyph));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    uwf::test::moveMouse(window.windowHandle(), QPoint(210, 90));
    QTest::qWait(160);
    const auto image = button->grab().toImage();
    const auto ink = uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Text);
    const auto content =
        uwf::test::visibleInkBounds(image, ink, uwf::test::regionColor(image, QRectF(0, 0, button->width() * .1, button->height())), button->rect());
    QVERIFY(!content.isEmpty());
    QVERIFY(uwf::test::centersAligned(content, button->rect(), Qt::Horizontal));
    QVERIFY(uwf::test::centersAligned(content, button->rect()));
  }
  void numberUnitDoesNotChangeNumericEditing() {
    uwf::ui::NumberBox control;
    control.setRange(0, 8192);
    control.setUnit(QStringLiteral("MB"));
    control.setValue(4096);
    control.show();
    QVERIFY(QTest::qWaitForWindowExposed(&control));
    control.setFocus();
    control.selectAll();
    QTest::keyClicks(&control, "3072");
    QTest::keyClick(&control, Qt::Key_Return);
    QCOMPARE(control.value(), 3072);
    control.stepUp();
    QCOMPARE(control.value(), 3073);
  }
  void anchoredPopupsUseOnePlacementRule_data() {
    QTest::addColumn<QRect>("anchor");
    QTest::addColumn<QRect>("available");
    QTest::addColumn<Qt::LayoutDirection>("direction");
    QTest::addColumn<QRect>("expected");
    QTest::newRow("below") << QRect(200, 100, 90, 32) << QRect(0, 0, 1200, 800) << Qt::LeftToRight << QRect(200, 138, 216, 100);
    QTest::newRow("above") << QRect(200, 720, 90, 32) << QRect(0, 0, 1200, 800) << Qt::LeftToRight << QRect(200, 614, 216, 100);
    QTest::newRow("right-edge") << QRect(1100, 100, 90, 32) << QRect(0, 0, 1200, 800) << Qt::LeftToRight << QRect(968, 138, 216, 100);
    QTest::newRow("rtl") << QRect(200, 100, 90, 32) << QRect(0, 0, 1200, 800) << Qt::RightToLeft << QRect(74, 138, 216, 100);
    QTest::newRow("translated-monitor") << QRect(-1100, -600, 90, 32) << QRect(-1200, -800, 1200, 800) << Qt::LeftToRight << QRect(-1100, -562, 216, 100);
  }
  void anchoredPopupsUseOnePlacementRule() {
    QFETCH(QRect, anchor);
    QFETCH(QRect, available);
    QFETCH(Qt::LayoutDirection, direction);
    QFETCH(QRect, expected);
    QCOMPARE(uwf::ui::anchoredPopupGeometry(anchor, {216, 100}, available, direction), expected);
  }
  void menuAndSelectOpenAtTheSameAnchorDistance() {
    QWidget window;
    window.resize(480, 360);
    QPushButton command(QStringLiteral("Commands"), &window);
    command.setGeometry(40, 30, 120, 32);
    uwf::ui::CommandMenu menu(&command);
    menu.addAction(QStringLiteral("First"));
    menu.addAction(QStringLiteral("Second"));
    command.setMenu(&menu);
    uwf::ui::SelectBox select(&window);
    select.setGeometry(220, 30, 120, 32);
    select.addItems({QStringLiteral("First"), QStringLiteral("Second")});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    uwf::test::clickMouse(&command, Qt::LeftButton);
    QTRY_VERIFY(menu.isVisible());
    QCOMPARE(menu.pos().y() - command.mapToGlobal(QPoint(0, command.height())).y(), 6);
    QCOMPARE(menu.pos().x(), command.mapToGlobal(QPoint()).x());
    menu.hide();
    uwf::test::clickMouse(&select, Qt::LeftButton);
    auto* popup = QApplication::activePopupWidget();
    QVERIFY(popup);
    QCOMPARE(popup->pos().y() - select.mapToGlobal(QPoint(0, select.height())).y(), 6);
    QCOMPARE(popup->pos().x(), select.mapToGlobal(QPoint()).x());
    select.resize(150, 40);
    QTRY_COMPARE(popup->pos().y() - select.mapToGlobal(QPoint(0, select.height())).y(), 6);
    select.move(230, 50);
    QTRY_COMPARE(popup->pos().x(), select.mapToGlobal(QPoint()).x());
    QTRY_COMPARE(popup->pos().y() - select.mapToGlobal(QPoint(0, select.height())).y(), 6);
    select.setEnabled(false);
    QVERIFY(!popup->isVisible());
  }
  void codeEditingPreservesPlainTextLineSpacingAndUndo() {
    uwf::ui::CodeEditor editor;
    editor.resize(480, 300);
    editor.setPlainText(QStringLiteral("one\ntwo\nthree"));
    editor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&editor));
    const auto verifySpacing = [&] {
      auto* layout = editor.document()->documentLayout();
      for (auto block = editor.document()->firstBlock(); block.next().isValid(); block = block.next()) {
        const qreal step = layout->blockBoundingRect(block.next()).top() - layout->blockBoundingRect(block).top();
        if (qAbs(step - 28.0) > .1) return false;
      }
      return true;
    };
    QVERIFY(verifySpacing());
    QVERIFY(!editor.document()->isUndoAvailable());
    editor.moveCursor(QTextCursor::End);
    QTest::keyClick(&editor, Qt::Key_Return);
    QTest::keyClicks(&editor, "four");
    QCOMPARE(editor.blockCount(), 4);
    QVERIFY(verifySpacing());
    const auto before = editor.toPlainText();
    QInputMethodEvent ime;
    ime.setCommitString(QStringLiteral("中文"));
    QApplication::sendEvent(&editor, &ime);
    QCOMPARE(editor.toPlainText(), before + QStringLiteral("中文"));
    editor.undo();
    QCOMPARE(editor.toPlainText(), before);
    editor.redo();
    QCOMPARE(editor.toPlainText(), before + QStringLiteral("中文"));
    editor.selectAll();
    QVERIFY(uwf::test::prepareClipboardText(QStringLiteral("<b>plain</b>\nsecond")));
    editor.paste();
    QCOMPARE(editor.toPlainText(), QStringLiteral("<b>plain</b>\nsecond"));
    QVERIFY(verifySpacing());
    editor.undo();
    QCOMPARE(editor.toPlainText(), before + QStringLiteral("中文"));
    editor.clear();
    QCOMPARE(editor.blockCount(), 1);
    editor.insertPlainText(QStringLiteral("fresh\nlines"));
    QVERIFY(verifySpacing());
  }
  void dialogScrollbarsPreserveSurfaceAndCleanIntersections() {
    auto& design = uwf::ui::DesignSystem::instance();
    uwf::ui::Surface host(uwf::ui::design::Color::Surface);
    host.resize(420, 250);
    auto* layout = new QVBoxLayout(&host);
    layout->setContentsMargins(1, 1, 1, 1);
    auto* editor = new uwf::ui::CodeEditor(&host);
    editor->setLineNumbersVisible(false);
    editor->setReadOnly(true);
    editor->setPlainText(QStringLiteral("Raw report: ") + QString(180, u'X') + QStringLiteral("\nsecond line\n").repeated(40));
    layout->addWidget(editor);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    auto* vertical = editor->verticalScrollBar();
    auto* horizontal = editor->horizontalScrollBar();
    QVERIFY(vertical->maximum() > 0);
    QVERIFY(horizontal->maximum() > 0);
    QCOMPARE(vertical->width(), 15);
    QCOMPARE(horizontal->height(), 15);
    for (const bool light : {true, false, true, false}) {
      design.setLight(light);
      QApplication::setPalette(design.palette());
      const QColor track = design.color(uwf::ui::design::Color::Surface);
      for (const bool end : {false, true, false}) {
        vertical->setValue(end ? vertical->maximum() : 0);
        horizontal->setValue(end ? horizontal->maximum() : 0);
        QCoreApplication::processEvents();
        const auto image = host.grab().toImage();
        const auto v = vertical->mapTo(&host, QPoint());
        const auto h = horizontal->mapTo(&host, QPoint());
        const QRectF verticalTrack(v.x() + vertical->width() * .04, v.y() + vertical->height() * .15, vertical->width() * .1, vertical->height() * .7);
        const QRectF horizontalTrack(h.x() + horizontal->width() * .15, h.y() + horizontal->height() * .04, horizontal->width() * .7,
                                     horizontal->height() * .1);
        const auto corner = uwf::test::interiorRegion(QRectF(v.x(), h.y(), vertical->width(), horizontal->height()));
        QVERIFY(uwf::test::colorCoverage(image, track, verticalTrack) >= uwf::test::kVisualMinimumCoverage);
        QVERIFY(uwf::test::colorCoverage(image, track, horizontalTrack) >= uwf::test::kVisualMinimumCoverage);
        QVERIFY(uwf::test::colorCoverage(image, track, corner) >= uwf::test::kVisualMinimumCoverage);
      }
    }
  }
  void inlineIconsAndTextShareTheVisibleCenter_data() {
    QTest::addColumn<bool>("light");
    QTest::addColumn<QString>("role");
    QTest::addColumn<QString>("text");
    for (const bool light : {false, true})
      for (const auto& role : {QStringLiteral("primary"), QStringLiteral("quiet"), QStringLiteral("link")})
        for (const auto& text : {QStringLiteral("测试"), QStringLiteral("Apply")})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(light ? "light" : "dark", role, text))) << light << role << text;
  }
  void inlineIconsAndTextShareTheVisibleCenter() {
    QFETCH(bool, light);
    QFETCH(QString, role);
    QFETCH(QString, text);
    auto& system = uwf::ui::DesignSystem::instance();
    system.setLight(light);
    QApplication::setPalette(system.palette());
    QWidget window;
    window.resize(220, 100);
    QPushButton button(text, &window);
    button.setGeometry(20, 20, 180, role == QStringLiteral("link") ? 24 : 36);
    button.setProperty("designRole", role);
    auto font = button.font();
    font.setPixelSize(role == QStringLiteral("link") ? 11 : 14);
    font.setWeight(role == QStringLiteral("link") ? QFont::Normal : QFont::Medium);
    button.setFont(font);
    QPixmap glyph(16, 16);
    glyph.fill(Qt::transparent);
    {
      QPainter painter(&glyph);
      painter.fillRect(QRect(3, 1, 10, 9), Qt::white);
    }
    button.setIcon(QIcon(glyph));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    uwf::test::moveMouse(window.windowHandle(), QPoint(210, 90));
    QTest::qWait(160);
    const auto frame = button.grab().toImage();
    const qreal scale = frame.devicePixelRatio();
    const auto ink = system.color(role == QStringLiteral("primary") ? uwf::ui::design::Color::AccentText
                                  : role == QStringLiteral("link")  ? uwf::ui::design::Color::Accent
                                                                    : uwf::ui::design::Color::Text);
    const QColor background = frame.pixelColor(frame.width() / 2, qRound(2 * scale));
    const auto runs = uwf::test::horizontalInkRuns(
        frame, ink, background, frame.rect().adjusted(qRound(4 * scale), qRound(4 * scale), -qRound(4 * scale), -qRound(4 * scale)), qRound(4 * scale));
    QVERIFY(runs.size() >= 2);
    QRect textBounds;
    for (qsizetype i = 1; i < runs.size(); ++i) textBounds = textBounds.united(runs[i]);
    const auto iconBounds = uwf::test::logicalBounds(runs.front(), scale);
    const auto labelBounds = uwf::test::logicalBounds(textBounds, scale);
    const QString directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT");
    if (!directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/inline-center"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(frame.save(output.filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral(".png"))));
    }
    QVERIFY(uwf::test::centersAligned(iconBounds, button.rect()));
    QVERIFY(uwf::test::centersAligned(labelBounds, iconBounds));
  }
  void realCopyButtonGlyphsShareTheVisibleCenter_data() {
    QTest::addColumn<bool>("light");
    QTest::addColumn<QString>("role");
    QTest::addColumn<QString>("text");
    QTest::addColumn<qreal>("scale");
    QTest::addColumn<bool>("stable");
    for (const bool light : {false, true})
      for (const auto& role : {QStringLiteral("primary"), QStringLiteral("quiet"), QStringLiteral("link")})
        for (const bool chinese : {false, true})
          for (const qreal scale : {1.0, 1.25, 1.5, 2.0})
            for (const bool stable : {false, true})
              QTest::newRow(qUtf8Printable(
                  QStringLiteral("%1-%2-%3-%4x-%5").arg(light ? "light" : "dark", role, chinese ? "zh" : "en").arg(scale).arg(stable ? "metrics" : "glyph")))
                  << light << role << (chinese ? QStringLiteral("复制") : QStringLiteral("Copy")) << scale << stable;
  }
  void realCopyButtonGlyphsShareTheVisibleCenter() {
    QFETCH(bool, light);
    QFETCH(QString, role);
    QFETCH(QString, text);
    QFETCH(qreal, scale);
    QFETCH(bool, stable);
    using uwf::ui::design::Color;
    auto& design = uwf::ui::DesignSystem::instance();
    const bool previousLight = design.isLight();
    const auto previousPalette = QApplication::palette();
    const auto restore = qScopeGuard([&] {
      design.setLight(previousLight);
      QApplication::setPalette(previousPalette);
    });
    design.setLight(light);
    QApplication::setPalette(design.palette());
    const auto ink = design.color(role == QStringLiteral("primary") ? Color::AccentText : role == QStringLiteral("link") ? Color::Accent : Color::Text);
    QPushButton button(text);
    button.setProperty("designRole", role);
    if (stable) button.setProperty("designTextVerticalReference", QVariant::fromValue(uwf::ui::TextVerticalReference::FontMetrics));
    button.setIcon(uwf::ui::coloredSvgIcon(QStringLiteral(":/icons/copy.svg"), ink));
    button.setIconSize({14, 14});
    auto font = uwf::ui::DesignSystem::defaultFont();
    font.setPixelSize(11);
    button.setFont(font);
    button.resize(120, 24);
    button.ensurePolished();
    QImage frame(QSize(qCeil(button.width() * scale), qCeil(button.height() * scale)), QImage::Format_ARGB32_Premultiplied);
    frame.setDevicePixelRatio(scale);
    frame.fill(design.color(Color::Window));
    button.render(&frame);
    const auto background = frame.pixelColor(frame.width() / 2, qRound(2 * scale));
    const auto runs = uwf::test::horizontalInkRuns(
        frame, ink, background, frame.rect().adjusted(qRound(4 * scale), qRound(4 * scale), -qRound(4 * scale), -qRound(4 * scale)), qRound(3 * scale));
    QVERIFY(runs.size() >= 2);
    QRect textBounds;
    for (qsizetype index = 1; index < runs.size(); ++index) textBounds = textBounds.united(runs[index]);
    const auto iconBounds = uwf::test::logicalBounds(runs.front(), scale);
    const auto labelBounds = uwf::test::logicalBounds(textBounds, scale);
    if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/real-copy-buttons"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(frame.save(output.filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral(".png"))));
    }
    auto referenceFont = button.font();
    referenceFont.setWeight(role == QStringLiteral("link") ? QFont::Normal : QFont::Medium);
    const QFontMetricsF metrics(referenceFont);
    const qreal halfStrut = (metrics.ascent() - metrics.descent()) / 2.0;
    const qreal baseline = qRound((button.height() / 2.0 + halfStrut) * scale) / scale;
    const qreal referenceCenter = stable ? baseline - metrics.capHeight() / 2.0 : labelBounds.center().y();
    // GlyphBounds follows the rendered letters; FontMetrics keeps the primary
    // font's capital band on one snapped baseline across changing text.
    QVERIFY2(
        std::abs(iconBounds.center().y() - referenceCenter) * scale <= 1.0,
        qUtf8Printable(QStringLiteral("copy.svg center=%1, %2 reference=%3, scale=%4").arg(iconBounds.center().y()).arg(text).arg(referenceCenter).arg(scale)));
    if (!stable) return;

    button.resize(480, 24);
    const QStringList messages{
        QStringLiteral("H WINDOWS 11"),
        QStringLiteral("H Windows 11 Enterprise"),
        QStringLiteral("H gypqj"),
        QStringLiteral("H \u00c1\u00c9\u00ce\u00d6\u00dc"),
        QStringLiteral("H \u7cfb\u7edf\u5c31\u7eea\uff0c\u914d\u7f6e\u672a\u5e94\u7528"),
        QStringLiteral("H \u0627\u0644\u0646\u0638\u0627\u0645 \u062c\u0627\u0647\u0632"),
        QStringLiteral("H C:\\Users\\Example\\configuration.json"),
        metrics.elidedText(QStringLiteral("H C:\\Users\\Example\\Documents\\Configuration\\") + QStringLiteral("settings-").repeated(30), Qt::ElideRight, 300),
        QString{},
        QStringLiteral("H WINDOWS 11")};
    const qreal originalIconCenter = iconBounds.center().y();
    for (qsizetype index = 0; index < messages.size(); ++index) {
      button.setText(messages[index]);
      QImage changed(QSize(qCeil(button.width() * scale), qCeil(button.height() * scale)), QImage::Format_ARGB32_Premultiplied);
      changed.setDevicePixelRatio(scale);
      changed.fill(design.color(Color::Window));
      button.render(&changed);
      const auto changedBackground = changed.pixelColor(changed.width() / 2, qRound(2 * scale));
      const auto changedRuns = uwf::test::horizontalInkRuns(
          changed, ink, changedBackground, changed.rect().adjusted(qRound(4 * scale), qRound(4 * scale), -qRound(4 * scale), -qRound(4 * scale)),
          qRound(3 * scale));
      QVERIFY2(!changedRuns.isEmpty(), qUtf8Printable(messages[index]));
      if (!messages[index].isEmpty()) QVERIFY2(changedRuns.size() >= 2, qUtf8Printable(messages[index]));
      const auto changedIcon = uwf::test::logicalBounds(changedRuns.front(), scale);
      const qreal expected = messages[index].isEmpty() ? button.height() / 2.0 : baseline - metrics.capHeight() / 2.0;
      QVERIFY2(
          std::abs(changedIcon.center().y() - expected) * scale <= 1.0,
          qUtf8Printable(
              QStringLiteral("%1: icon center=%2, font reference=%3, scale=%4").arg(messages[index]).arg(changedIcon.center().y()).arg(expected).arg(scale)));
      if (!messages[index].isEmpty())
        QVERIFY2(std::abs(changedIcon.center().y() - originalIconCenter) * scale <= 1.0, qUtf8Printable(QStringLiteral("%1: icon moved from %2 to %3, scale=%4")
                                                                                                            .arg(messages[index])
                                                                                                            .arg(originalIconCenter)
                                                                                                            .arg(changedIcon.center().y())
                                                                                                            .arg(scale)));
      if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
        QDir output(directory + QStringLiteral("/real-copy-buttons"));
        QVERIFY(output.mkpath(QStringLiteral(".")));
        QVERIFY(changed.save(output.filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral("-message-%1.png").arg(index))));
      }
    }
  }
  void graphicalEntryPointsShareTheirVisibleCenter_data() {
    QTest::addColumn<QString>("entry");
    QTest::addColumn<QString>("text");
    QTest::addColumn<qreal>("scale");
    const QStringList entries{QStringLiteral("toolbar"),
                              QStringLiteral("native-menu"),
                              QStringLiteral("native-menu-checked"),
                              QStringLiteral("native-menu-submenu"),
                              QStringLiteral("command-menu"),
                              QStringLiteral("context-label-copy"),
                              QStringLiteral("context-table-copy"),
                              QStringLiteral("context-table-select"),
                              QStringLiteral("context-header-fit"),
                              QStringLiteral("checkbox"),
                              QStringLiteral("checkbox-icon"),
                              QStringLiteral("radio"),
                              QStringLiteral("radio-icon"),
                              QStringLiteral("select"),
                              QStringLiteral("select-popup"),
                              QStringLiteral("search"),
                              QStringLiteral("search-clear"),
                              QStringLiteral("disclosure"),
                              QStringLiteral("tab-pages"),
                              QStringLiteral("tab-volumes"),
                              QStringLiteral("tab-entries"),
                              QStringLiteral("status-badge"),
                              QStringLiteral("item-list"),
                              QStringLiteral("item-tree"),
                              QStringLiteral("item-table"),
                              QStringLiteral("item-secondary"),
                              QStringLiteral("item-status-badge"),
                              QStringLiteral("item-status-error"),
                              QStringLiteral("item-status-warning"),
                              QStringLiteral("item-status-success"),
                              QStringLiteral("item-status-information"),
                              QStringLiteral("item-add-badge"),
                              QStringLiteral("item-remove-badge"),
                              QStringLiteral("path-list"),
                              QStringLiteral("content-action"),
                              QStringLiteral("inspector-action")};
    for (const auto& entry : entries)
      for (const bool chinese : {false, true})
        for (const qreal scale : {1.0, 1.25, 1.5, 2.0})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3x").arg(entry, chinese ? "zh" : "en").arg(scale)))
              << entry << (chinese ? QStringLiteral("复制设置") : QStringLiteral("Copy settings")) << scale;
  }
  void graphicalEntryPointsShareTheirVisibleCenter() {
    QFETCH(QString, entry);
    QFETCH(QString, text);
    QFETCH(qreal, scale);
    using namespace uwf;
    using ui::design::Color;
    auto& design = ui::DesignSystem::instance();
    const bool previousLight = design.isLight();
    const auto previousPalette = QApplication::palette();
    const auto restore = qScopeGuard([&] {
      design.setLight(previousLight);
      QApplication::setPalette(previousPalette);
    });
    design.setLight(true);
    QApplication::setPalette(design.palette());
    QWidget host;
    host.resize(520, 240);
    QWidget* target = nullptr;
    QRect area;
    bool trailing = false;
    bool twoLeading = false;
    std::unique_ptr<ui::CommandMenu> contextMenu;
    const auto icon = ui::coloredSvgIcon(QStringLiteral(":/icons/copy.svg"), design.color(Color::Text));
    if (entry == QStringLiteral("toolbar")) {
      auto* bar = new ui::CommandBar(&host);
      auto* action = bar->addCommand(text);
      action->setIcon(icon);
      bar->addStretch();
      bar->resize(360, 58);
      target = bar->buttonForAction(action);
    } else if (entry.startsWith(QStringLiteral("native-menu")) || entry == QStringLiteral("command-menu")) {
      QMenu* menu = entry == QStringLiteral("command-menu") ? new ui::CommandMenu(&host) : new QMenu(&host);
      auto* action = menu->addAction(icon, text);
      if (entry == QStringLiteral("native-menu-checked")) {
        action->setCheckable(true);
        action->setChecked(true);
      }
      if (entry == QStringLiteral("native-menu-submenu")) {
        auto* submenu = new QMenu(menu);
        submenu->addAction(QStringLiteral("Child"));
        action->setMenu(submenu);
      }
      if (auto* commands = qobject_cast<ui::CommandMenu*>(menu)) {
        commands->setMaximumVisibleHeight(40);
        for (int index = 0; index < 4; ++index) commands->addAction(icon, text);
      }
      host.show();
      menu->popup(QPoint());
      QCoreApplication::processEvents();
      target = menu;
      area = menu->actionGeometry(action);
      if (auto* list = menu->findChild<QListView*>(); list && list->isVisible()) {
        target = list->viewport();
        area = list->visualRect(list->model()->index(0, 0));
      }
    } else if (entry.startsWith(QStringLiteral("context-"))) {
      QWidget* source = nullptr;
      QPoint position;
      if (entry == QStringLiteral("context-label-copy")) {
        source = new QLabel(text, &host);
        source->resize(280, 36);
        position = source->rect().center();
      } else {
        auto* table = new QTableView(&host);
        auto* model = new QStandardItemModel(1, 1, table);
        model->setData(model->index(0, 0), text);
        table->setModel(model);
        table->resize(360, 120);
        host.show();
        table->selectRow(0);
        source = entry == QStringLiteral("context-header-fit") ? table->horizontalHeader()->viewport() : table->viewport();
        position = entry == QStringLiteral("context-header-fit") ? source->rect().center() : table->visualRect(model->index(0, 0)).center();
      }
      const QContextMenuEvent event(QContextMenuEvent::Mouse, position, source->mapToGlobal(position));
      contextMenu = ui::createContextMenu(*source, event, [](const QString&) { return ui::ClipboardWriteResult::Confirmed; });
      QVERIFY(contextMenu);
      auto* action = contextMenu->actions().at(entry == QStringLiteral("context-table-select") ? 1 : 0);
      action->setText(text);
      host.show();
      contextMenu->popup(QPoint());
      QCoreApplication::processEvents();
      target = contextMenu.get();
      area = contextMenu->actionGeometry(action);
    } else if (entry.startsWith(QStringLiteral("checkbox")) || entry.startsWith(QStringLiteral("radio"))) {
      QAbstractButton* button = entry.startsWith(QStringLiteral("checkbox")) ? static_cast<QAbstractButton*>(new QCheckBox(text, &host))
                                                                             : static_cast<QAbstractButton*>(new QRadioButton(text, &host));
      if (entry.endsWith(QStringLiteral("-icon"))) {
        button->setIcon(icon);
        button->setIconSize({16, 16});
        twoLeading = true;
      }
      button->setChecked(true);
      button->resize(280, 36);
      target = button;
    } else if (entry == QStringLiteral("select") || entry == QStringLiteral("select-popup")) {
      auto* select = new ui::SelectBox(&host);
      select->addItem(text);
      select->resize(280, 34);
      target = select;
      trailing = true;
      if (entry == QStringLiteral("select-popup")) {
        host.show();
        select->showPopup();
        QCoreApplication::processEvents();
        auto* list = select->findChild<QListView*>(QStringLiteral("selectOptions"));
        QVERIFY(list);
        target = list->viewport();
        area = list->visualRect(list->model()->index(0, 0));
      }
    } else if (entry.startsWith(QStringLiteral("search"))) {
      auto* search = new ui::SearchField(&host);
      search->setFocusPolicy(Qt::NoFocus);
      if (entry == QStringLiteral("search-clear"))
        search->setText(text);
      else
        search->setPlaceholderText(text);
      search->resize(280, 34);
      target = search;
    } else if (entry == QStringLiteral("disclosure")) {
      auto* disclosure = new ui::Disclosure(text, &host);
      disclosure->setCount(12);
      disclosure->resize(340, 50);
      target = disclosure->findChild<QAbstractButton*>();
      QVERIFY(target);
    } else if (entry.startsWith(QStringLiteral("tab-"))) {
      auto* tabs = new ui::TabView(&host);
      tabs->setKind(entry == QStringLiteral("tab-pages")     ? ui::TabView::Kind::Pages
                    : entry == QStringLiteral("tab-volumes") ? ui::TabView::Kind::Volumes
                                                             : ui::TabView::Kind::Entries);
      tabs->addTab(new QWidget(tabs), icon, text);
      if (entry == QStringLiteral("tab-pages")) tabs->setTabCount(0, 12);
      if (entry == QStringLiteral("tab-volumes")) tabs->setTabIndicator(0, QStringLiteral(":/icons/status_warning.svg"), text);
      tabs->resize(380, 150);
      target = tabs->tabBar();
    } else if (entry == QStringLiteral("status-badge")) {
      auto* badge = new ui::StatusBadge(&host);
      badge->setText(text);
      badge->setIcon(icon);
      badge->resize(badge->sizeHint());
      target = badge;
    } else if (entry.startsWith(QStringLiteral("item-")) || entry == QStringLiteral("path-list")) {
      auto* model = new QStandardItemModel(&host);
      auto* item = new QStandardItem(text);
      item->setData(QStringLiteral(":/icons/folder.svg"), presentation::IconResourceRole);
      if (entry == QStringLiteral("item-secondary")) item->setData(QStringLiteral("C:\\Data\\settings.json"), presentation::SecondaryTextRole);
      if (entry == QStringLiteral("item-status-badge")) {
        item->setData(true, presentation::StatusBadgeRole);
        item->setData(static_cast<int>(presentation::ItemStatus::Success), presentation::StatusRole);
      }
      if (entry.startsWith(QStringLiteral("item-status-")) && entry != QStringLiteral("item-status-badge")) {
        item->setData({}, presentation::IconResourceRole);
        item->setData(true, presentation::StatusIconRole);
        const auto status = entry.endsWith(QStringLiteral("error"))     ? presentation::ItemStatus::Error
                            : entry.endsWith(QStringLiteral("warning")) ? presentation::ItemStatus::Warning
                            : entry.endsWith(QStringLiteral("success")) ? presentation::ItemStatus::Success
                                                                        : presentation::ItemStatus::Information;
        item->setData(static_cast<int>(status), presentation::StatusRole);
      }
      if (entry == QStringLiteral("item-add-badge") || entry == QStringLiteral("item-remove-badge"))
        item->setData(static_cast<int>(entry == QStringLiteral("item-add-badge") ? presentation::ItemBadge::Add : presentation::ItemBadge::Remove),
                      presentation::BadgeRole);
      model->appendRow(item);
      QAbstractItemView* view = nullptr;
      if (entry == QStringLiteral("item-tree")) {
        auto* widget = new QTreeView(&host);
        item->appendRow(new QStandardItem(QStringLiteral("child")));
        widget->setHeaderHidden(true);
        view = widget;
        twoLeading = true;
      } else if (entry == QStringLiteral("item-table") || entry == QStringLiteral("item-status-badge")) {
        auto* widget = new ui::ResponsiveTableView({{320, 1}}, &host);
        widget->horizontalHeader()->hide();
        widget->verticalHeader()->hide();
        widget->setColumnWidth(0, 320);
        widget->verticalHeader()->setDefaultSectionSize(40);
        widget->setShowGrid(false);
        view = widget;
      } else if (entry == QStringLiteral("path-list")) {
        auto* widget = new ui::PathEntryListView(QStringLiteral("Path"), QStringLiteral("Status"), &host);
        item->setData(QStringLiteral("C:\\Data\\") + text, Qt::DisplayRole);
        item->setData(static_cast<int>(presentation::ItemStatus::Normal), presentation::StatusRole);
        item->setData(QStringLiteral("Configured"), presentation::StatusTextRole);
        view = widget;
      } else {
        view = new QListView(&host);
      }
      if (entry != QStringLiteral("path-list")) {
        auto* delegate = new ui::ItemDelegate(view);
        delegate->setRowHeight(40);
        view->setItemDelegate(delegate);
      }
      view->setModel(model);
      view->setFrameShape(QFrame::NoFrame);
      view->setFocusPolicy(Qt::NoFocus);
      view->resize(400, 120);
      host.show();
      QCoreApplication::processEvents();
      target = view->viewport();
      area = view->visualRect(view->model()->index(0, 0));
      if (entry == QStringLiteral("path-list")) area.setWidth(area.width() - 132);
    } else if (entry == QStringLiteral("content-action")) {
      auto* state = new ui::ContentState(&host);
      state->setText(QStringLiteral("State"), QStringLiteral("Description"));
      state->setAction(text, icon);
      state->resize(380, 200);
      target = state->findChild<QPushButton*>();
    } else if (entry == QStringLiteral("inspector-action")) {
      auto* inspector = new ui::RecordInspector(QStringLiteral("Records"), 2, &host);
      target = inspector->addAction(text, icon);
      inspector->resize(480, 180);
    }
    QVERIFY(target);
    host.show();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QCoreApplication::processEvents();
    if (entry == QStringLiteral("search-clear")) QTest::qWait(180);
    if (area.isEmpty()) area = target->rect();
    if (entry == QStringLiteral("item-tree")) area.setLeft(0);
    if (entry.startsWith(QStringLiteral("tab-")))
      area.setHeight(area.height() - (entry == QStringLiteral("tab-pages") ? 13 : entry == QStringLiteral("tab-volumes") ? 3 : 6));
    const bool field = entry.startsWith(QStringLiteral("search")) || entry == QStringLiteral("select");
    const auto scan = field ? area.adjusted(6, 4, -6, -4) : area.adjusted(4, 2, -4, -2);
    QImage frame(QSize(qCeil(target->width() * scale), qCeil(target->height() * scale)), QImage::Format_ARGB32_Premultiplied);
    frame.setDevicePixelRatio(scale);
    frame.fill(design.color(Color::Window));
    target->render(&frame);
    const auto background = frame.pixelColor(qRound(scan.right() * scale), qRound(scan.top() * scale));
    const auto runs = test::horizontalInkRuns(
        frame, design.color(Color::Text), background,
        QRect(qRound(scan.x() * scale), qRound(scan.y() * scale), qRound(scan.width() * scale), qRound(scan.height() * scale)), qRound(3 * scale));
    if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/control-entry-points"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(frame.save(output.filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral(".png"))));
    }
    QVERIFY2(runs.size() >= (twoLeading ? 3 : 2), qUtf8Printable(QStringLiteral("%1: only %2 separate ink runs").arg(entry).arg(runs.size())));
    QRect graphic, copy, auxiliary;
    if (trailing) {
      graphic = runs.back();
      for (qsizetype index = 0; index + 1 < runs.size(); ++index) copy = copy.united(runs[index]);
    } else {
      graphic = runs.front();
      if (twoLeading) {
        auxiliary = graphic;
        graphic = runs[1];
      }
      const bool withAuxiliary = entry == QStringLiteral("tab-pages") || entry == QStringLiteral("tab-volumes") || entry == QStringLiteral("disclosure") ||
                                 entry == QStringLiteral("native-menu-checked") || entry == QStringLiteral("native-menu-submenu") ||
                                 entry == QStringLiteral("search-clear");
      if (withAuxiliary) auxiliary = runs.back();
      for (qsizetype index = twoLeading ? 2 : 1; index < runs.size() - (withAuxiliary ? 1 : 0); ++index) copy = copy.united(runs[index]);
    }
    const auto graphicBounds = test::logicalBounds(graphic, scale);
    const auto copyBounds = test::logicalBounds(copy, scale);
    QVERIFY2(std::abs(QRectF(graphic).center().y() - QRectF(copy).center().y()) <= 1.0,
             qUtf8Printable(QStringLiteral("%1: graphic center=%2, text center=%3, scale=%4")
                                .arg(entry)
                                .arg(graphicBounds.center().y())
                                .arg(copyBounds.center().y())
                                .arg(scale)));
    if (!auxiliary.isEmpty()) {
      const auto auxiliaryBounds = test::logicalBounds(auxiliary, scale);
      QVERIFY2(std::abs(QRectF(auxiliary).center().y() - QRectF(copy).center().y()) <= 1.0,
               qUtf8Printable(QStringLiteral("%1: auxiliary center=%2, text center=%3, scale=%4")
                                  .arg(entry)
                                  .arg(auxiliaryBounds.center().y())
                                  .arg(copyBounds.center().y())
                                  .arg(scale)));
    }
  }
  void iconOnlyEntryPointsKeepTheirVisibleCenter_data() {
    QTest::addColumn<QString>("entry");
    QTest::addColumn<qreal>("scale");
    const QStringList entries{
        QStringLiteral("pagination-first"), QStringLiteral("pagination-previous"), QStringLiteral("pagination-next"), QStringLiteral("pagination-last"),
        QStringLiteral("path-add"),         QStringLiteral("path-remove"),         QStringLiteral("path-restore"),    QStringLiteral("scroll-up"),
        QStringLiteral("scroll-down"),      QStringLiteral("scroll-left"),         QStringLiteral("scroll-right"),    QStringLiteral("switch-off"),
        QStringLiteral("switch-on"),        QStringLiteral("switch-unknown")};
    for (const auto& entry : entries)
      for (const qreal scale : {1.0, 1.25, 1.5, 2.0}) QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2x").arg(entry).arg(scale))) << entry << scale;
  }
  void iconOnlyEntryPointsKeepTheirVisibleCenter() {
    QFETCH(QString, entry);
    QFETCH(qreal, scale);
    using namespace uwf;
    using ui::design::Color;
    auto& design = ui::DesignSystem::instance();
    const bool previousLight = design.isLight();
    const auto previousPalette = QApplication::palette();
    const auto restore = qScopeGuard([&] {
      design.setLight(previousLight);
      QApplication::setPalette(previousPalette);
    });
    design.setLight(true);
    QApplication::setPalette(design.palette());
    QWidget host;
    host.resize(500, 240);
    QStandardItemModel model;
    for (int index = 0; index < 100; ++index) model.appendRow(new QStandardItem(QStringLiteral("row %1").arg(index)));
    presentation::PageProxyModel pages;
    pages.setPageSize(10);
    pages.setSourceModel(&model);
    QWidget* target = nullptr;
    QScrollBar* scroll = nullptr;
    QStyle::SubControl arrow = QStyle::SC_None;
    QRect area;
    if (entry.startsWith(QStringLiteral("pagination-"))) {
      auto* bar = new ui::PaginationBar(pages, &host);
      bar->resize(480, 30);
      pages.goNext();
      const auto buttons = bar->findChildren<QPushButton*>();
      QCOMPARE(buttons.size(), 4);
      target = buttons[entry == QStringLiteral("pagination-first")      ? 0
                       : entry == QStringLiteral("pagination-previous") ? 1
                       : entry == QStringLiteral("pagination-next")     ? 2
                                                                        : 3];
    } else if (entry.startsWith(QStringLiteral("path-"))) {
      const auto controls = ui::createPathEntryControls(host, model, QStringLiteral("Path"), QStringLiteral("Status"));
      controls.renderRemoval(entry == QStringLiteral("path-restore"), QStringLiteral("Remove"));
      target = entry == QStringLiteral("path-add") ? controls.add : controls.remove;
    } else if (entry.startsWith(QStringLiteral("switch-"))) {
      auto* control = new ui::SwitchButton(&host);
      control->setChecked(entry == QStringLiteral("switch-on"));
      control->setUnknown(entry == QStringLiteral("switch-unknown"));
      target = control;
    } else {
      const bool vertical = entry == QStringLiteral("scroll-up") || entry == QStringLiteral("scroll-down");
      scroll = new QScrollBar(vertical ? Qt::Vertical : Qt::Horizontal, &host);
      scroll->setRange(0, 100);
      scroll->setValue(50);
      scroll->resize(vertical ? QSize(16, 160) : QSize(160, 16));
      target = scroll;
      arrow = entry == QStringLiteral("scroll-up") || entry == QStringLiteral("scroll-left") ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
    }
    QVERIFY(target);
    host.show();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QCoreApplication::processEvents();
    area = target->rect();
    if (scroll) {
      QStyleOptionSlider option;
      option.initFrom(scroll);
      option.rect = scroll->rect();
      option.orientation = scroll->orientation();
      option.minimum = scroll->minimum();
      option.maximum = scroll->maximum();
      option.sliderPosition = option.sliderValue = scroll->value();
      area = scroll->style()->subControlRect(QStyle::CC_ScrollBar, &option, arrow, scroll);
    }
    QImage frame(QSize(qCeil(target->width() * scale), qCeil(target->height() * scale)), QImage::Format_ARGB32_Premultiplied);
    frame.setDevicePixelRatio(scale);
    frame.fill(ui::DesignSystem::instance().color(Color::Window));
    target->setAttribute(Qt::WA_UnderMouse, false);
    target->render(&frame);
    const auto scan = area.adjusted(4, 4, -4, -4);
    const auto background = frame.pixelColor(qRound(scan.left() * scale), qRound(scan.top() * scale));
    auto ink = test::visibleInkBounds(frame, ui::DesignSystem::instance().color(Color::Text), background, scan);
    if (entry.startsWith(QStringLiteral("switch-"))) {
      const auto foreground = design.color(entry == QStringLiteral("switch-on") ? Color::AccentText : Color::Muted);
      QRect pixels;
      for (int y = 0; y < frame.height(); ++y)
        for (int x = 0; x < frame.width(); ++x)
          if (frame.pixelColor(x, y) == foreground) pixels = pixels.united(QRect(x, y, 1, 1));
      ink = test::logicalBounds(pixels, scale);
      if (entry == QStringLiteral("switch-unknown"))
        ink = test::visibleInkBounds(frame, foreground, frame.pixelColor(frame.width() / 2, frame.height() / 4),
                                     QRectF(target->width() / 2.0 - 5, target->height() / 2.0 - 2, 10, 4));
    }
    if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/icon-only-entry-points"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(frame.save(output.filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral(".png"))));
    }
    QVERIFY2(!ink.isEmpty(), qUtf8Printable(entry));
    QVERIFY2(std::abs(ink.center().y() - QRectF(area).center().y()) * scale <= 1.0,
             qUtf8Printable(
                 QStringLiteral("%1: ink center=%2, control center=%3, scale=%4").arg(entry).arg(ink.center().y()).arg(QRectF(area).center().y()).arg(scale)));
  }
  void sourceSvgAssetsKeepTheirVisibleCenter_data() {
    QTest::addColumn<QString>("resource");
    QTest::addColumn<int>("size");
    QTest::addColumn<qreal>("scale");
    const auto names = QDir(QStringLiteral(":/icons")).entryList({QStringLiteral("*.svg")}, QDir::Files, QDir::Name);
    QVERIFY(!names.isEmpty());
    for (const auto& name : names) {
      const QList<int> sizes = name == QStringLiteral("dot.svg") ? QList<int>{5, 6, 14, 16, 23} : QList<int>{14, 16, 23};
      for (const int size : sizes)
        for (const qreal scale : {1.0, 1.25, 1.5, 2.0})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2px-%3x").arg(name).arg(size).arg(scale))) << QStringLiteral(":/icons/") + name << size << scale;
    }
  }
  void sourceSvgAssetsKeepTheirVisibleCenter() {
    QFETCH(QString, resource);
    QFETCH(int, size);
    QFETCH(qreal, scale);
    using namespace uwf;
    auto& design = ui::DesignSystem::instance();
    const bool previousLight = design.isLight();
    const auto previousPalette = QApplication::palette();
    const auto restore = qScopeGuard([&] {
      design.setLight(previousLight);
      QApplication::setPalette(previousPalette);
    });
    design.setLight(true);
    QApplication::setPalette(design.palette());
    const auto foreground = ui::DesignSystem::instance().color(ui::design::Color::Text);
    const auto background = ui::DesignSystem::instance().color(ui::design::Color::Window);
    ui::IconView icon(ui::coloredSvgIcon(resource, foreground), size, ui::design::Color::Text);
    if (resource == QStringLiteral(":/icons/dot.svg") && size <= 6) icon.setFixedSize(size + 2, size + 2);
    QImage frame(QSize(qCeil(icon.width() * scale), qCeil(icon.height() * scale)), QImage::Format_ARGB32_Premultiplied);
    frame.setDevicePixelRatio(scale);
    frame.fill(background);
    icon.render(&frame);
    const auto ink = test::visibleInkBounds(frame, foreground, background, icon.rect());
    if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/source-svg-assets"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(frame.save(output.filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral(".png"))));
    }
    QVERIFY2(!ink.isEmpty(), qUtf8Printable(resource));
    const qreal center = QRectF(icon.rect()).center().y();
    QVERIFY2(std::abs(ink.center().y() - center) * scale <= 1.0,
             qUtf8Printable(QStringLiteral("%1: ink center=%2, box center=%3, scale=%4").arg(resource).arg(ink.center().y()).arg(center).arg(scale)));
  }
  void flowCanAlignTechnicalNotesToTheBottomWithoutChangingOtherGroups() {
    QWidget host;
    auto* flow = new uwf::ui::FlowLayout(&host);
    auto* title = new QWidget(&host);
    auto* technical = new QWidget(&host);
    auto* centered = new QWidget(&host);
    title->setFixedSize(60, 30);
    technical->setFixedSize(30, 16);
    centered->setFixedSize(20, 16);
    for (auto* widget : {title, technical, centered}) flow->addWidget(widget);
    flow->setAlignment(technical, Qt::AlignBottom);
    host.resize(160, 60);
    flow->setGeometry(host.rect());
    QCOMPARE(technical->geometry().bottom(), title->geometry().bottom());
    QCOMPARE(centered->geometry().center().y(), title->geometry().center().y());
    host.resize(80, 120);
    flow->setGeometry(host.rect());
    QVERIFY(technical->y() > title->geometry().bottom());
    QCOMPARE(centered->y(), technical->y());
  }
  void differentSizedGlyphBoundsShareTheBottomEdge() {
    for (const qreal scale : {1.0, 1.25, 1.5, 2.0}) {
      QRectF reference;
      for (const auto& pair : {std::pair{QStringLiteral("D:"), 22}, std::pair{QStringLiteral("卷标测试"), 13}}) {
        uwf::ui::TextLabel label(pair.first, uwf::ui::TextStyle::Label);
        auto font = label.font();
        font.setPixelSize(pair.second);
        label.setFont(font);
        label.setAlignment(Qt::AlignLeading | Qt::AlignBottom);
        label.setVerticalReference(uwf::ui::TextVerticalReference::GlyphBounds);
        label.setColorRole(uwf::ui::design::Color::Text);
        label.resize(220, 44);
        QImage frame(QSize(qCeil(label.width() * scale), qCeil(label.height() * scale)), QImage::Format_ARGB32_Premultiplied);
        frame.setDevicePixelRatio(scale);
        frame.fill(Qt::white);
        label.render(&frame);
        const auto runs = uwf::test::horizontalInkRuns(frame, uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Text),
                                                       frame.pixelColor(frame.width() - 1, 0), frame.rect(), 1);
        QRect ink;
        for (const auto& run : runs) ink = ink.united(run);
        QVERIFY(!ink.isEmpty());
        const auto bounds = uwf::test::logicalBounds(ink, scale);
        if (reference.isEmpty()) reference = bounds;
        QVERIFY(uwf::test::bottomsAligned(bounds, reference));
      }
    }
  }
  void textRasterMeasurementPreservesShapedGlyphsWithoutDecorations_data() {
    QTest::addColumn<QString>("text");
    QTest::addColumn<qreal>("scale");
    const std::array texts{QStringLiteral("Copy gypqj"), QStringLiteral("\u590d\u5236\u8bbe\u7f6e"),
                           QStringLiteral("\u062c\u0627\u0631\u064d \u0642\u0631\u0627\u0621\u0629 \u0627\u0644\u062d\u0627\u0644\u0629\u2026")};
    for (size_t index = 0; index < texts.size(); ++index)
      for (const qreal scale : {1.0, 1.25, 1.5, 2.0}) QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2x").arg(index).arg(scale))) << texts[index] << scale;
  }
  void textRasterMeasurementPreservesShapedGlyphsWithoutDecorations() {
    QFETCH(QString, text);
    QFETCH(qreal, scale);
    const QRectF bounds(0, 0, 280, 40);
    const QPointF position(8, 6);
    auto baseFont = uwf::ui::DesignSystem::defaultFont();
    baseFont.setPixelSize(12);
    const auto configure = [&](QTextLayout& layout) {
      QTextOption option;
      option.setWrapMode(QTextOption::NoWrap);
      option.setTextDirection(text.isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight);
      layout.setTextOption(option);
      layout.beginLayout();
      auto line = layout.createLine();
      line.setLineWidth(bounds.width() - position.x());
      layout.endLayout();
    };
    QTextLayout native(text, baseFont);
    configure(native);
    QImage frame(QSize(qCeil(bounds.width() * scale), qCeil(bounds.height() * scale)), QImage::Format_ARGB32_Premultiplied);
    frame.setDevicePixelRatio(scale);
    frame.fill(Qt::transparent);
    QTransform device;
    {
      QPainter painter(&frame);
      painter.translate(.5, .25);
      device = painter.deviceTransform();
      native.draw(&painter, position);
    }
    const auto expected = uwf::ui::rasterInkBounds(frame);
    QVERIFY(!expected.isEmpty());
    for (int decorations = 0; decorations < 4; ++decorations) {
      auto font = baseFont;
      font.setUnderline((decorations & 1) != 0);
      font.setStrikeOut((decorations & 2) != 0);
      QTextLayout decorated(text, font);
      configure(decorated);
      const auto measured = uwf::ui::textRasterBounds(decorated, position, bounds, device);
      const auto pixels = device.mapRect(measured);
      QCOMPARE(QRect(qRound(pixels.x()), qRound(pixels.y()), qRound(pixels.width()), qRound(pixels.height())), expected);
    }
    if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/shaped-text-raster"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(frame.save(output.filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral(".png"))));
    }
  }
  void defaultLabelTextSharesItsVisibleCenterAcrossScripts_data() {
    QTest::addColumn<qreal>("scale");
    QTest::addColumn<int>("pixels");
    for (const qreal scale : {1.0, 1.25, 1.5, 2.0})
      for (const int pixels : {12, 14}) QTest::newRow(qUtf8Printable(QStringLiteral("%1x-%2px").arg(scale).arg(pixels))) << scale << pixels;
  }
  void defaultLabelTextSharesItsVisibleCenterAcrossScripts() {
    QFETCH(qreal, scale);
    QFETCH(int, pixels);
    auto& design = uwf::ui::DesignSystem::instance();
    const auto previous = design.isLight();
    const auto restore = qScopeGuard([&] { design.setLight(previous); });
    design.setLight(true);
    uwf::ui::TextLabel label({}, uwf::ui::TextStyle::Label);
    auto font = uwf::ui::DesignSystem::defaultFont();
    font.setPixelSize(pixels);
    label.setFont(font);
    label.setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
    label.setColorRole(uwf::ui::design::Color::Text);
    label.resize(260, 34);
    const auto ink = design.color(uwf::ui::design::Color::Text);
    for (const auto& text : {QStringLiteral("Apply"), QStringLiteral("gypqj"), QStringLiteral("测试标题"), QStringLiteral("اختبار")}) {
      label.setText(text);
      QImage frame(QSize(qCeil(label.width() * scale), qCeil(label.height() * scale)), QImage::Format_ARGB32_Premultiplied);
      frame.setDevicePixelRatio(scale);
      frame.fill(Qt::white);
      label.render(&frame);
      const auto bounds = uwf::test::visibleInkBounds(frame, ink, frame.pixelColor(frame.width() - 1, 0), label.contentsRect());
      QVERIFY2(!bounds.isEmpty(), qUtf8Printable(text));
      QVERIFY2(std::abs(QRectF(label.contentsRect()).center().y() * scale - bounds.center().y() * scale) <= 1.0,
               qUtf8Printable(
                   QStringLiteral("%1: text center=%2, box center=%3").arg(text).arg(bounds.center().y()).arg(QRectF(label.contentsRect()).center().y())));
    }
  }
  void noticeIconFollowsTheFirstVisibleTextLine_data() {
    QTest::addColumn<bool>("heading");
    QTest::addColumn<qreal>("scale");
    for (const bool heading : {false, true})
      for (const qreal scale : {1.0, 1.25, 1.5, 2.0})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2x").arg(heading ? "heading" : "body").arg(scale))) << heading << scale;
  }
  void noticeIconFollowsTheFirstVisibleTextLine() {
    QFETCH(bool, heading);
    QFETCH(qreal, scale);
    uwf::ui::Notice notice;
    notice.resize(300, 140);
    const uwf::ui::IconView* icon = nullptr;
    for (auto widgets = notice.findChildren<QWidget*>(); auto* candidate : std::as_const(widgets))
      if (const auto* graphic = dynamic_cast<uwf::ui::IconView*>(candidate)) icon = graphic;
    QVERIFY(icon);
    int scriptIndex = 0;
    for (const auto& first : {QStringLiteral("Apply changes"), QStringLiteral("测试标题"), QStringLiteral("اختبار")}) {
      const auto content = first + QStringLiteral("\nSecond line with details.");
      notice.setHeading(heading ? content : QString{});
      notice.setText(heading ? QStringLiteral("Additional description.") : content);
      notice.show();
      QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
      uwf::ui::TextLabel* text = nullptr;
      for (auto labels = notice.findChildren<uwf::ui::TextLabel*>(); auto* candidate : std::as_const(labels))
        if (candidate->text() == content) text = candidate;
      QVERIFY(text);
      QImage frame(QSize(qCeil(notice.width() * scale), qCeil(notice.height() * scale)), QImage::Format_ARGB32_Premultiplied);
      frame.setDevicePixelRatio(scale);
      frame.fill(Qt::transparent);
      notice.render(&frame);
      if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
        QDir output(directory + QStringLiteral("/notice-first-line"));
        QVERIFY(output.mkpath(QStringLiteral(".")));
        QVERIFY(frame.save(output.filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral("-%1.png").arg(scriptIndex))));
      }
      ++scriptIndex;
      const auto line = text->firstLineBoxBounds().translated(text->mapTo(&notice, QPoint()));
      const auto glyph = QRectF(icon->mapTo(&notice, QPoint()), icon->size());
      const auto foreground = uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Muted);
      const auto background = frame.pixelColor(qRound(line.right() * scale) - 1, qRound(line.top() * scale));
      const auto textInk = uwf::test::visibleInkBounds(frame, foreground, background, line);
      const auto iconInk = uwf::test::visibleInkBounds(frame, foreground, background, glyph);
      QVERIFY(!textInk.isEmpty() && !iconInk.isEmpty());
      QVERIFY2(std::abs(qRound(iconInk.center().y() * scale * 2) - qRound(textInk.center().y() * scale * 2)) <= 2,
               qUtf8Printable(QStringLiteral("%1: icon center=%2, text center=%3").arg(first).arg(iconInk.center().y()).arg(textInk.center().y())));
    }
  }
  void changingLabelTextKeepsTheSharedGlyphOnOneBaseline_data() {
    QTest::addColumn<qreal>("scale");
    QTest::addColumn<int>("pixels");
    for (const qreal scale : {1.0, 1.25, 1.5, 1.75, 2.0})
      for (const int pixels : {11, 13}) QTest::newRow(qUtf8Printable(QStringLiteral("%1x-%2px").arg(scale).arg(pixels))) << scale << pixels;
  }
  void changingLabelTextKeepsTheSharedGlyphOnOneBaseline() {
    QFETCH(qreal, scale);
    QFETCH(int, pixels);
    uwf::ui::TextLabel label({}, uwf::ui::TextStyle::Eyebrow);
    auto font = uwf::ui::DesignSystem::defaultFont();
    font.setPixelSize(pixels);
    label.setFont(font);
    label.setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
    label.setVerticalReference(uwf::ui::TextVerticalReference::FontMetrics);
    label.setElideMode(Qt::ElideRight);
    label.setColorRole(uwf::ui::design::Color::Text);
    label.resize(360, 24);
    const auto render = [&] {
      QImage frame(QSize(qCeil(label.width() * scale), qCeil(label.height() * scale)), QImage::Format_ARGB32_Premultiplied);
      frame.setDevicePixelRatio(scale);
      frame.fill(Qt::white);
      label.render(&frame);
      return frame;
    };
    const QStringList messages{QStringLiteral("H WINDOWS 11"),
                               QStringLiteral("H Windows 11 Enterprise · 96G · RTX 5090"),
                               QStringLiteral("H gypqj"),
                               QStringLiteral("H ÁÉÎÖÜ"),
                               QStringLiteral("H 系统就绪，配置未应用"),
                               QStringLiteral("H النظام جاهز"),
                               QStringLiteral("H C:\\用户目录\\配置.json"),
                               QStringLiteral("H ").repeated(200)};
    QRect reference;
    for (const auto& message : messages) {
      label.setText(message);
      const auto frame = render();
      const auto color = uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Text);
      const auto runs = uwf::test::horizontalInkRuns(frame, color, frame.pixelColor(frame.width() - 1, 0),
                                                     QRect(0, 0, qCeil(label.fontMetrics().horizontalAdvance(QStringLiteral("H")) * scale), frame.height()), 1);
      QRect marker;
      for (const auto& run : runs) marker = marker.united(run);
      QVERIFY(!marker.isEmpty());
      if (reference.isEmpty()) reference = marker;
      QVERIFY(uwf::test::similarSize(uwf::test::logicalBounds(marker, scale), uwf::test::logicalBounds(reference, scale)));
      QVERIFY(uwf::test::centersAligned(uwf::test::logicalBounds(marker, scale), uwf::test::logicalBounds(reference, scale)));
    }
  }
  void partiallySpecifiedListFontsUseTheApplicationTypography() {
    auto local = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    local.setResolveMask(0);
    local.setPixelSize(13);
    local.setWeight(QFont::Normal);
    auto expected = uwf::ui::DesignSystem::defaultFont();
    expected.setPixelSize(13);
    expected.setWeight(QFont::Normal);
    const QString text = QStringLiteral("已加入待应用变更 Abgp123");
    const auto render = [&](const QFont& font) {
      QImage frame(600, 100, QImage::Format_ARGB32_Premultiplied);
      frame.setDevicePixelRatio(2);
      frame.fill(Qt::white);
      QPainter painter(&frame);
      painter.setPen(Qt::black);
      uwf::ui::paintTextLine(painter, QRectF(10, 5, 280, 40), text, font);
      painter.end();
      return frame;
    };
    QCOMPARE(uwf::ui::textLineSize(text, local), uwf::ui::textLineSize(text, expected));
    const auto area = QRectF(0, 0, 300, 50);
    QVERIFY(uwf::test::changedCoverage(render(local), render(expected), area) <= uwf::test::kVisualAlignmentRatio);
    local.setFamilies({QStringLiteral("Consolas")});
    expected.setFamilies({QStringLiteral("Consolas")});
    QVERIFY(uwf::test::changedCoverage(render(local), render(expected), area) <= uwf::test::kVisualAlignmentRatio);
  }
  void smallListTextKeepsItsRasterAcrossRows_data() {
    QTest::addColumn<qreal>("scale");
    QTest::addColumn<int>("rowHeight");
    for (const qreal scale : {1.0, 1.25, 1.5, 1.75, 2.0})
      for (const int height : {38, 40, 42, 47}) QTest::newRow(qUtf8Printable(QStringLiteral("%1x-%2px").arg(scale).arg(height))) << scale << height;
  }
  void smallListTextKeepsItsRasterAcrossRows() {
    QFETCH(qreal, scale);
    QFETCH(int, rowHeight);
    auto font = uwf::ui::DesignSystem::defaultFont();
    font.setPixelSize(13);
    QImage frame(QSize(qCeil(240 * scale), qCeil(rowHeight * 6 * scale)), QImage::Format_ARGB32_Premultiplied);
    frame.setDevicePixelRatio(scale);
    frame.fill(Qt::white);
    {
      QPainter painter(&frame);
      painter.setPen(Qt::black);
      for (int row = 0; row < 6; ++row)
        uwf::ui::paintTextLine(painter, QRectF(14, row * rowHeight, 220, rowHeight), QStringLiteral("待应用变更 Abgp123"), font);
    }
    QRectF reference;
    for (int row = 0; row < 6; ++row) {
      QRect ink;
      for (int y = qCeil(row * rowHeight * scale); y < qFloor((row + 1) * rowHeight * scale); ++y)
        for (int x = 0; x < frame.width(); ++x)
          if (qGray(frame.pixel(x, y)) < 250) ink = ink.united(QRect(x, y, 1, 1));
      QVERIFY(!ink.isEmpty());
      const auto bounds = uwf::test::logicalBounds(ink, scale).translated(0, -row * rowHeight);
      if (reference.isEmpty()) reference = bounds;
      QVERIFY(uwf::test::similarSize(bounds, reference));
      QVERIFY(uwf::test::centersAligned(bounds, reference));
    }
  }
  void disclosuresPreserveKeyboardAndContentVisibility() {
    uwf::ui::Disclosure disclosure(QStringLiteral("Details"));
    auto* content = new QLineEdit(QStringLiteral("Complete details"));
    disclosure.setContent(content);
    disclosure.show();
    QVERIFY(QTest::qWaitForWindowExposed(&disclosure));
    QVERIFY(!content->isVisible());
    QSignalSpy toggled(&disclosure, &uwf::ui::Disclosure::openChanged);
    QTest::keyClick(disclosure.toggleButton(), Qt::Key_Space);
    QVERIFY(disclosure.isOpen());
    QVERIFY(content->isVisible());
    QTest::keyClick(disclosure.toggleButton(), Qt::Key_Space);
    QVERIFY(!disclosure.isOpen());
    QVERIFY(!content->isVisible());
    QCOMPARE(toggled.count(), 2);
  }
  void disclosureContentReplacementRetainsOneQtOwner_data() {
    QTest::addColumn<bool>("open");
    QTest::addColumn<bool>("nestedChild");
    for (const bool open : {false, true})
      for (const bool nestedChild : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2").arg(open ? "open" : "closed", nestedChild ? "nested-body" : "new-body"))) << open << nestedChild;
  }
  void disclosureContentReplacementRetainsOneQtOwner() {
    QFETCH(bool, open);
    QFETCH(bool, nestedChild);
    auto owner = std::make_unique<uwf::ui::Disclosure>(QStringLiteral("Details"));
    owner->setOpen(open);
    const QPointer retired(new QWidget);
    owner->setContent(retired);
    const QPointer replacement(new QWidget(nestedChild ? retired.data() : nullptr));
    owner->setContent(replacement);
    QCOMPARE(replacement->parentWidget(), owner.get());
    QCOMPARE(replacement->isHidden(), !open);
    // A repeated setter keeps the same body and does not schedule its removal.
    owner->setContent(replacement);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(!retired);
    QVERIFY(replacement);
    owner.reset();
    QVERIFY(!replacement);
  }
  void disclosureContentCanRetireBeforeTheContainer() {
    uwf::ui::Disclosure owner(QStringLiteral("Details"));
    const QPointer content(new QWidget);
    owner.setContent(content);
    content->deleteLater();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(!content);
    owner.setOpen(true);
    owner.setOpen(false);
    const QPointer replacement(new QWidget);
    owner.setContent(replacement);
    owner.setContent(nullptr);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(!replacement);
  }
  void disclosureContentAdoptionKeepsTheNewestReentrantBody() {
    class ObservedBody final : public QWidget {
     public:
      std::function<void()> adopted;

     protected:
      bool event(QEvent* event) override {
        const bool handled = QWidget::event(event);
        if (event->type() == QEvent::ParentChange && adopted) {
          const auto callback = std::exchange(adopted, {});
          callback();
        }
        return handled;
      }
    };
    uwf::ui::Disclosure owner(QStringLiteral("Details"));
    owner.setOpen(true);
    const QPointer original(new QWidget);
    owner.setContent(original);
    const QPointer intermediate(new ObservedBody);
    const QPointer newest(new QWidget);
    intermediate->adopted = [&] { owner.setContent(newest); };
    owner.setContent(intermediate);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(!original);
    QVERIFY(!intermediate);
    QVERIFY(newest);
    QCOMPARE(newest->parentWidget(), &owner);
    QVERIFY(!newest->isHidden());
  }
  void vectorIconsKeepVisibleCenterAtFractionalScale_data() {
    QTest::addColumn<qreal>("scale");
    QTest::addColumn<int>("height");
    for (const qreal scale : {1.0, 1.25, 1.5, 1.75, 2.0})
      for (const int height : {30, 31}) QTest::newRow(qUtf8Printable(QStringLiteral("%1x-%2px").arg(scale).arg(height))) << scale << height;
  }
  void vectorIconsKeepVisibleCenterAtFractionalScale() {
    QFETCH(qreal, scale);
    QFETCH(int, height);
    QImage frame(QSize(qCeil(32 * scale), qCeil(height * scale)), QImage::Format_ARGB32_Premultiplied);
    frame.setDevicePixelRatio(scale);
    frame.fill(Qt::black);
    QPainterPath path;
    path.moveTo(6, 2);
    path.lineTo(9, 5);
    path.lineTo(15, -1);
    {
      QPainter painter(&frame);
      uwf::ui::paintIconPath(painter, path, QRectF(0, 0, 32, height), Qt::white, 1.1);
    }
    const auto runs = uwf::test::horizontalInkRuns(frame, Qt::white, Qt::black, frame.rect(), frame.width());
    QCOMPARE(runs.size(), 1);
    QVERIFY(std::abs(QRectF(runs.front()).center().y() - height * scale / 2.0) <= 1.0);
    bool antialiased = false;
    for (int y = 0; y < frame.height(); ++y)
      for (int x = 0; x < frame.width(); ++x) {
        const int value = frame.pixelColor(x, y).red();
        antialiased = antialiased || (value > 0 && value < 255);
      }
    QVERIFY(antialiased);
    if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/vector-primitives"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(frame.save(output.filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral(".png"))));
    }
  }
  void roundedSurfacesClipChildPaintWithoutAFringe_data() {
    QTest::addColumn<bool>("light");
    QTest::newRow("light") << true;
    QTest::newRow("dark") << false;
  }
  void roundedSurfacesClipChildPaintWithoutAFringe() {
    QFETCH(bool, light);
    using uwf::ui::design::Color;
    auto& design = uwf::ui::DesignSystem::instance();
    design.setLight(light);
    uwf::ui::Surface surface;
    surface.setAttribute(Qt::WA_TranslucentBackground);
    surface.resize(96, 64);
    constexpr int cornerRadius = 6;
    surface.setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, cornerRadius);
    surface.setClipContent(true);
    auto* child = new uwf::ui::Surface(Color::Accent, &surface);
    child->setGeometry(surface.rect());
    surface.show();
    QVERIFY(QTest::qWaitForWindowExposed(&surface));
    const auto frame = surface.grab().toImage();
    QVERIFY(surface.testAttribute(Qt::WA_TranslucentBackground));
    const qreal corner = cornerRadius;
    const auto centerColor = uwf::test::regionColor(frame, uwf::test::interiorRegion(surface.rect()));
    QVERIFY(centerColor.isValid());
    for (const auto& region :
         {QRectF(0, 0, corner, corner), QRectF(surface.width() - corner, 0, corner, corner), QRectF(0, surface.height() - corner, corner, corner)})
      QVERIFY(centerColor.alphaF() - uwf::test::regionColor(frame, region).alphaF() > uwf::test::kVisualColorRatio);
    QVERIFY(uwf::test::colorCoverage(frame, design.color(Color::Accent), uwf::test::interiorRegion(surface.rect())) >= uwf::test::kVisualMinimumCoverage);
    QVERIFY(uwf::test::colorCoverage(frame, design.color(Color::Line), surface.rect()) > 0);
    qreal edgeSamples = 0, incorrectEdges = 0;
    for (int y = 0; y < frame.height(); ++y)
      for (int x = 0; x < frame.width(); ++x) {
        const auto pixel = frame.pixelColor(x, y);
        if (pixel.alpha() < 64 || pixel.alpha() > 192) continue;
        ++edgeSamples;
        const auto line = design.color(Color::Line);
        if (qAbs(pixel.redF() - line.redF()) > uwf::test::kVisualColorRatio || qAbs(pixel.greenF() - line.greenF()) > uwf::test::kVisualColorRatio ||
            qAbs(pixel.blueF() - line.blueF()) > uwf::test::kVisualColorRatio)
          ++incorrectEdges;
      }
    if (edgeSamples > 0) QVERIFY(incorrectEdges / edgeSamples <= uwf::test::kVisualColorRatio);
    if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/surface-clip"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(frame.save(output.filePath(light ? QStringLiteral("light.png") : QStringLiteral("dark.png"))));
    }
    child->setFill(Color::Success);
    const auto updated = surface.grab().toImage();
    QVERIFY(uwf::test::colorCoverage(updated, design.color(Color::Success), uwf::test::interiorRegion(surface.rect())) >= uwf::test::kVisualMinimumCoverage);
  }
};
}  // namespace
QTEST_MAIN(DesignControlTests)
#include "DesignControlTests.moc"
