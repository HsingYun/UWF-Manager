// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <dwmapi.h>
#include <qt_windows.h>

#include <QAbstractItemModelTester>
#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFontMetrics>
#include <QHeaderView>
#include <QInputMethodEvent>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QLocale>
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QStyle>
#include <QTableView>
#include <QTableWidget>
#include <QTemporaryFile>
#include <QTest>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextFragment>
#include <QTextLayout>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWindow>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "WidgetUiFixture.h"
#include "core/UwfModel.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PageProxyModel.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/LocalFileDialogs.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/ViewMetrics.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/InkBounds.h"
#include "support/visual/WidgetCapture.h"
#include "ui/appearance/AppearanceTransition.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/appearance/WindowAppearance.h"
#include "ui/configuration/ApplyPlanDialog.h"
#include "ui/configuration/ExclusionListWidget.h"
#include "ui/configuration/GlobalStatusPanel.h"
#include "ui/configuration/ImportDialog.h"
#include "ui/configuration/VolumeStatusPanel.h"
#include "ui/design/DesignSystem.h"
#include "ui/design/TextPainting.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/dialogs/PanelDialog.h"
#include "ui/power/PowerConfirmDialog.h"
#include "ui/shell/HoverHintController.h"
#include "ui/shell/StatusSurface.h"
#include "ui/shell/WindowChromeController.h"
#include "ui/shell/WindowTitleBar.h"
#include "ui/widgets/CodeEditor.h"
#include "ui/widgets/CommandBar.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/EntryListView.h"
#include "ui/widgets/FlexibleColumnLayout.h"
#include "ui/widgets/FlowLayout.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/NumberBox.h"
#include "ui/widgets/PagedTableView.h"
#include "ui/widgets/PagedViewController.h"
#include "ui/widgets/PagedWindowLayout.h"
#include "ui/widgets/RecyclingListView.h"
#include "ui/widgets/SummaryLabel.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/SwitchButton.h"
#include "ui/widgets/TabView.h"
#include "ui/widgets/TableText.h"
#include "ui/widgets/TextLabel.h"
#include "ui/widgets/TransientLabel.h"
#include "ui/widgets/VerticalScrollArea.h"
#include "windows/filesystem/BoundedTextFileReader.h"
#include "windows/system/SystemHardwareInfo.h"

using namespace uwf::test::widgets;
using uwf::test::FileDialogStub;
namespace {
class WidgetControlsTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() { initializeWidgets(); }
  void switchButtonRespondsToMouseKeyboardAndDisabledState();
  void arabicNumberFieldsKeepUnitsOutsideTheEditor();
  void collapsingDisclosureRestoresDialogHeight();
  void standardContextMenusPreserveCopyAndLocale();
  void longCommandMenusKeepFullScrollbarsAndKeyboardActions();
  void scrollableMenuCommandsRunAfterInputAndBeforeExecReturns_data();
  void scrollableMenuCommandsRunAfterInputAndBeforeExecReturns();
  void scrollableMenusCancelStaleSelections_data();
  void scrollableMenusCancelStaleSelections();
  void codeEditorLinePitchSurvivesEmptyAndTypedParagraphs();
  void codeEditorGutterStaysOnTheTextBaselineAfterChineseInputAndScrolling();
  void switchDragPreviewsAndCommitsOnlyOnRelease();
  void groupedLayoutReflowsWithoutClippingOrRecreatingControls();
  void tableCopyReflectsTheActualSelectionAndMissingCells();
};

void WidgetControlsTests::switchButtonRespondsToMouseKeyboardAndDisabledState() {
  QWidget window;
  QVBoxLayout layout(&window);
  uwf::ui::SwitchButton button;
  layout.addWidget(&button);
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));

  QSignalSpy toggled(&button, &QAbstractButton::toggled);
  QVERIFY(!button.isChecked());
  QTest::mouseClick(&button, Qt::LeftButton);
  QVERIFY(button.isChecked());
  QCOMPARE(toggled.count(), 1);

  QTest::mouseClick(&button, Qt::RightButton);
  QVERIFY(button.isChecked());
  QCOMPARE(toggled.count(), 1);

  button.setFocus();
  QTest::keyClick(&button, Qt::Key_Space);
  QVERIFY(!button.isChecked());
  QCOMPARE(toggled.count(), 2);

  button.setEnabled(false);
  QTest::mouseClick(&button, Qt::LeftButton);
  QTest::keyClick(&button, Qt::Key_Space);
  QVERIFY(!button.isChecked());
  QCOMPARE(toggled.count(), 2);
}

void WidgetControlsTests::arabicNumberFieldsKeepUnitsOutsideTheEditor() {
  QWidget host;
  host.setLayoutDirection(Qt::RightToLeft);
  uwf::ui::NumberBox number(&host);
  number.setLocale(QLocale(QLocale::Arabic, QLocale::SaudiArabia));
  auto font = number.font();
  font.setPixelSize(12);
  number.setFont(font);
  number.setRange(0, 65536);
  number.setUnit(QStringLiteral("MB"));
  number.setValue(4096);
  number.setFixedSize(111, 31);
  host.resize(140, 55);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  auto* edit = number.findChild<QLineEdit*>();
  QVERIFY(edit);
  QCOMPARE(number.layoutDirection(), Qt::LeftToRight);
  auto unitFont = number.font();
  unitFont.setPixelSize(11);
  const int unitLeft = number.width() - 8 - QFontMetrics(unitFont).horizontalAdvance(QStringLiteral("MB"));
  QVERIFY(edit->geometry().right() < unitLeft);
  QVERIFY(QFontMetrics(edit->font()).horizontalAdvance(edit->text()) <= edit->contentsRect().width());
  number.selectAll();
  QTest::keyClicks(&number, "3072");
  QTest::keyClick(&number, Qt::Key_Return);
  QCOMPARE(number.value(), 3072);
  uwf::ui::GlobalStatusPanel usage(std::make_shared<uwf::presentation::GlobalConfigurationViewModel>(8192), &host);
  uwf::core::SessionSnapshot current;
  current.filter.enabled = true;
  current.overlay.maximumSizeMb = 4096;
  usage.viewModel().setData(current, current, {3072, 1024});
  auto* value = usage.findChild<QLabel*>(QStringLiteral("statusCurrent"));
  QVERIFY(value);
  QCOMPARE(value->layoutDirection(), Qt::LeftToRight);
  QCOMPARE(value->text(), QStringLiteral("1,024 / 4,096 MB"));
}

void WidgetControlsTests::collapsingDisclosureRestoresDialogHeight() {
  using namespace uwf::ui;
  PanelDialog dialog;
  auto* layout = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(520, 0));
  auto* disclosure = new Disclosure(QStringLiteral("Details"), &dialog);
  auto* details = new QLabel(QStringLiteral("Expanded details"), &dialog);
  details->setFixedHeight(340);
  disclosure->setContent(details);
  layout->body()->addWidget(disclosure);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QTest::qWait(30);
  const int compact = dialog.height();
  disclosure->setOpen(true);
  QTRY_VERIFY(layout->scrollArea()->verticalScrollBar()->maximum() > 0);
  QCOMPARE(dialog.height(), compact);
  disclosure->setOpen(false);
  QTRY_COMPARE(dialog.height(), compact);
  QTRY_COMPARE(layout->scrollArea()->verticalScrollBar()->maximum(), 0);
}

void WidgetControlsTests::standardContextMenusPreserveCopyAndLocale() {
  using namespace uwf::ui;
  const auto language = I18n::instance().lang();
  const auto clipboard = QApplication::clipboard()->text();
  const auto restore = qScopeGuard([&] {
    I18n::instance().setLang(language);
    QApplication::clipboard()->setText(clipboard);
  });
  for (const auto locale : {I18n::Lang::Zh_CN, I18n::Lang::Ar}) {
    I18n::instance().setLang(locale);
    QWidget host;
    auto* layout = new QVBoxLayout(&host);
    auto* line = new QLineEdit(QStringLiteral("Selected 中文 text"), &host);
    auto* rich = new QTextEdit(QStringLiteral("Selected 中文 text"), &host);
    auto* plain = new QPlainTextEdit(QStringLiteral("Selected 中文 text"), &host);
    auto* literal = new QLabel(QStringLiteral("<b>Selected 中文 text</b>"), &host);
    literal->setTextFormat(Qt::RichText);
    rich->setReadOnly(true);
    for (QWidget* widget : std::array<QWidget*, 4>{line, rich, plain, literal}) layout->addWidget(widget);
    host.resize(600, 450);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    line->selectAll();
    rich->selectAll();
    plain->selectAll();
    int menuIndex = 0;
    for (QWidget* target : std::array<QWidget*, 4>{line, rich->viewport(), plain->viewport(), literal}) {
      const QString captureName = QStringLiteral("context-menu-%1-%2").arg(static_cast<int>(locale)).arg(menuIndex++);
      bool copied = false;
      QTimer::singleShot(30, &host, [&] {
        auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
        QVERIFY(menu);
        const auto close = qScopeGuard([menu] { menu->close(); });
        QVERIFY(menu->property("designPopupSurface").toBool());
        QCOMPARE(menu->layoutDirection(), QApplication::layoutDirection());
        for (const auto actions = menu->actions(); auto* action : actions) QCOMPARE(action->icon().isNull(), target != literal);
        QVERIFY(uwf::test::saveScreenshot(*menu, captureName));
        for (const auto actions = menu->actions(); auto* action : actions) {
          static const QRegularExpression suffix(QStringLiteral(R"(\s*\(&[A-Za-z0-9]\))"));
          const auto title = action->text().section(u'\t', 0, 0).remove(suffix).remove(u'&');
          if (title != I18n::tr("Copy")) continue;
          QVERIFY(action->isEnabled());
          action->trigger();
          copied = true;
          break;
        }
      });
      QContextMenuEvent context(QContextMenuEvent::Mouse, target->rect().center(), target->mapToGlobal(target->rect().center()));
      QApplication::sendEvent(target, &context);
      QVERIFY(copied);
      QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("Selected 中文 text"));
    }
  }
}

void WidgetControlsTests::longCommandMenusKeepFullScrollbarsAndKeyboardActions() {
  uwf::ui::CommandMenu menu;
  menu.setMaximumVisibleHeight(520);
  auto* group = new QActionGroup(&menu);
  group->setExclusive(true);
  QAction* last = nullptr;
  for (int row = 0; row < 20; ++row) {
    auto* action = menu.addAction(QStringLiteral("Option %1").arg(row + 1));
    action->setCheckable(true);
    action->setChecked(row == 0);
    group->addAction(action);
    last = action;
  }
  QSignalSpy triggered(last, &QAction::triggered);
  menu.popup(QPoint(100, 100));
  QVERIFY(QTest::qWaitForWindowExposed(&menu));
  auto* view = menu.findChild<QListView*>(QStringLiteral("scrollableMenuItems"));
  QVERIFY(view && view->isVisible());
  QCOMPARE(menu.height(), 520);
  QCOMPARE(view->verticalScrollBar()->width(), 15);
  QVERIFY(view->verticalScrollBar()->maximum() > 0);
  view->setFocus();
  QTest::keyClick(view, Qt::Key_End);
  QCOMPARE(view->currentIndex().row(), 19);
  QTest::keyClick(view, Qt::Key_Return);
  QTRY_VERIFY(!menu.isVisible());
  QTRY_COMPARE(triggered.count(), 1);
  QVERIFY(last->isChecked());
  QCOMPARE(menu.actions().size(), 20);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  menu.popup(QPoint(100, 100));
  QVERIFY(QTest::qWaitForWindowExposed(&menu));
  view = menu.findChild<QListView*>(QStringLiteral("scrollableMenuItems"));
  QVERIFY(view);
  QTRY_VERIFY(view->verticalScrollBar()->value() > 0);
  QTest::keyClick(view, Qt::Key_Escape);
  QCOMPARE(triggered.count(), 1);
  QCOMPARE(menu.actions().size(), 20);
}

void WidgetControlsTests::scrollableMenuCommandsRunAfterInputAndBeforeExecReturns_data() {
  QTest::addColumn<bool>("keyboard");
  QTest::addColumn<bool>("synchronous");
  QTest::newRow("mouse-popup") << false << false;
  QTest::newRow("keyboard-popup") << true << false;
  QTest::newRow("mouse-exec") << false << true;
  QTest::newRow("keyboard-exec") << true << true;
}

void WidgetControlsTests::scrollableMenuCommandsRunAfterInputAndBeforeExecReturns() {
  QFETCH(bool, keyboard);
  QFETCH(bool, synchronous);
  uwf::ui::CommandMenu menu;
  menu.setMaximumVisibleHeight(120);
  for (int row = 0; row < 20; ++row) menu.addAction(QStringLiteral("Option %1").arg(row));
  QPointer<QListView> view;
  bool inputReturned = true;
  int invoked = 0;
  bool modalAccepted = false;
  connect(menu.actions().constFirst(), &QAction::triggered, this, [&] {
    ++invoked;
    QVERIFY(inputReturned);
    QVERIFY(!menu.isVisible());
    QDialog dialog;
    QTimer::singleShot(0, &dialog, [&] {
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
      const bool viewWasDestroyed = view.isNull();
      dialog.accept();
      QVERIFY(viewWasDestroyed);
    });
    modalAccepted = dialog.exec() == QDialog::Accepted;
  });
  const auto select = [&] {
    view = menu.findChild<QListView*>(QStringLiteral("scrollableMenuItems"));
    QVERIFY(view);
    const auto index = view->model()->index(0, 0);
    view->setCurrentIndex(index);
    inputReturned = false;
    if (keyboard)
      QTest::keyClick(view, Qt::Key_Return);
    else
      QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, view->visualRect(index).center());
    inputReturned = true;
  };
  QTimer timeout;
  timeout.setSingleShot(true);
  connect(&timeout, &QTimer::timeout, &menu, &QMenu::close);
  timeout.start(3000);
  if (synchronous) {
    QTimer::singleShot(0, &menu, select);
    menu.exec(QPoint(100, 100));
    QCOMPARE(invoked, 1);
    QVERIFY(modalAccepted);
  } else {
    menu.popup(QPoint(100, 100));
    QVERIFY(QTest::qWaitForWindowExposed(&menu));
    select();
    QTRY_COMPARE(invoked, 1);
    QVERIFY(modalAccepted);
  }
  QCOMPARE(menu.actions().size(), 20);
  QVERIFY(view.isNull());
}

void WidgetControlsTests::scrollableMenusCancelStaleSelections_data() {
  QTest::addColumn<QString>("change");
  for (const auto* change : {"hide", "reopen", "delete-menu", "delete-action", "disable-action", "duplicate"})
    QTest::newRow(change) << QString::fromLatin1(change);
}

void WidgetControlsTests::scrollableMenusCancelStaleSelections() {
  QFETCH(QString, change);
  QWidget owner;
  auto menu = std::make_unique<uwf::ui::CommandMenu>();
  menu->setMaximumVisibleHeight(120);
  // The command can outlive a transient popup; it must still be cancelled.
  std::vector<std::unique_ptr<QAction>> actions;
  for (int row = 0; row < 20; ++row) {
    actions.push_back(std::make_unique<QAction>(QStringLiteral("Option %1").arg(row)));
    menu->addAction(actions.back().get());
  }
  const QPointer<QAction> action = menu->actions().constFirst();
  int invoked = 0;
  connect(action, &QAction::triggered, this, [&] { ++invoked; });
  menu->popup(QPoint(100, 100));
  QVERIFY(QTest::qWaitForWindowExposed(menu.get()));
  auto* view = menu->findChild<QListView*>(QStringLiteral("scrollableMenuItems"));
  QVERIFY(view);
  view->setCurrentIndex(view->model()->index(0, 0));
  QTest::keyClick(view, Qt::Key_Return);
  if (change == QStringLiteral("hide") || change == QStringLiteral("reopen")) {
    menu->close();
    if (change == QStringLiteral("reopen")) menu->popup(QPoint(100, 100));
  } else if (change == QStringLiteral("delete-menu")) {
    menu.reset();
    QVERIFY(action);
  } else if (change == QStringLiteral("delete-action")) {
    actions.front().reset();
  } else if (change == QStringLiteral("disable-action")) {
    action->setEnabled(false);
  } else {
    emit view->activated(view->currentIndex());
  }
  bool dispatched = false;
  QTimer::singleShot(0, &owner, [&] { dispatched = true; });
  QTRY_VERIFY(dispatched);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  QCOMPARE(invoked, change == QStringLiteral("duplicate") ? 1 : 0);
}

void WidgetControlsTests::codeEditorLinePitchSurvivesEmptyAndTypedParagraphs() {
  uwf::ui::CodeEditor editor;
  editor.resize(500, 260);
  editor.show();
  QVERIFY(QTest::qWaitForWindowExposed(&editor));
  const auto check = [&] {
    int previous = -10000;
    for (auto block = editor.document()->firstBlock(); block.isValid(); block = block.next()) {
      const int y = editor.cursorRect(QTextCursor(block)).top() + editor.verticalScrollBar()->value();
      if (previous != -10000) QCOMPARE(y - previous, 28);
      previous = y;
    }
  };
  QTest::keyClick(&editor, Qt::Key_Return);
  QCOMPARE(editor.blockCount(), 2);
  check();
  QTest::keyClicks(&editor, "abc");
  QTest::keyClick(&editor, Qt::Key_Return);
  QCOMPARE(editor.blockCount(), 3);
  check();
  QTest::keyClick(&editor, Qt::Key_Return);
  QCOMPARE(editor.blockCount(), 4);
  check();
  QTest::keyClicks(&editor, "text");
  QTest::keyClick(&editor, Qt::Key_Return);
  QCOMPARE(editor.blockCount(), 5);
  check();
  editor.insertPlainText(QStringLiteral("\n1\n\n2\n3\n\n4\n5\n6\n7\n8\n9\n10"));
  check();
}

void WidgetControlsTests::codeEditorGutterStaysOnTheTextBaselineAfterChineseInputAndScrolling() {
  using namespace uwf::ui;
  Surface host(design::Color::Surface);
  CodeEditor editor(&host);
  host.resize(480, 270);
  editor.resize(480, 270);
  editor.setPlainText(QStringLiteral("1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n1"));
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  auto* gutter = editor.findChild<QWidget*>(QStringLiteral("codeLineNumbers"));
  QVERIFY(gutter);
  const auto before = gutter->grab().toImage();
  editor.moveCursor(QTextCursor::Start);
  editor.moveCursor(QTextCursor::EndOfBlock);
  QInputMethodEvent composition(QStringLiteral("中文"), {});
  QApplication::sendEvent(&editor, &composition);
  QCOMPARE(gutter->grab().toImage(), before);
  QInputMethodEvent commit;
  commit.setCommitString(QStringLiteral("中文"));
  QApplication::sendEvent(&editor, &commit);
  QCOMPARE(gutter->grab().toImage(), before);
  for (const auto runs = editor.document()->firstBlock().layout()->glyphRuns(); const auto& run : runs) {
    QVERIFY2(!run.rawFont().familyName().contains(QLatin1String("SimSun")), qUtf8Printable(run.rawFont().familyName()));
  }
  const auto checkRows = [&] {
    const auto image = host.grab().toImage();
    if (qEnvironmentVariableIsSet("UWF_TEST_SCREENSHOT_DIR"))
      image.save(qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR") + QStringLiteral("/editor-baselines.png"));
    const qreal scale = image.devicePixelRatio();
    const auto background = DesignSystem::instance().color(design::Color::Surface);
    const auto ink = [&](int left, int right, int top, int bottom) {
      QRect result;
      for (int y = top; y <= bottom; ++y)
        for (int x = left; x <= right; ++x) {
          const auto pixel = image.pixelColor(x, y);
          if (qAbs(pixel.red() - background.red()) + qAbs(pixel.green() - background.green()) + qAbs(pixel.blue() - background.blue()) > 70)
            result = result.united(QRect(x, y, 1, 1));
        }
      return result;
    };
    for (auto block = editor.document()->firstBlock(); block.isValid(); block = block.next()) {
      const int y = editor.cursorRect(QTextCursor(block)).top() + editor.viewport()->y();
      if (y < editor.viewport()->y() || y + 28 > editor.viewport()->geometry().bottom()) continue;
      const int top = qRound(y * scale), bottom = qRound((y + 27) * scale);
      const auto label = ink(qRound(18 * scale), qRound((gutter->width() - 10) * scale), top, bottom);
      const auto body = ink(qRound(editor.viewport()->x() * scale), qRound((editor.viewport()->x() + 8) * scale), top, bottom);
      QVERIFY(!label.isEmpty());
      QVERIFY(!body.isEmpty());
      QVERIFY2(qAbs(label.top() - body.top()) <= 1,
               qUtf8Printable(QStringLiteral("line %1: gutter %2 text %3").arg(block.blockNumber() + 1).arg(label.top()).arg(body.top())));
      QVERIFY(qAbs(label.bottom() - body.bottom()) <= 1);
    }
  };
  checkRows();
  editor.verticalScrollBar()->setValue(137);
  checkRows();
  editor.verticalScrollBar()->setValue(editor.verticalScrollBar()->maximum());
  checkRows();
  editor.undo();
  QCOMPARE(editor.toPlainText().section(u'\n', 0, 0), QStringLiteral("1"));
  editor.redo();
  QCOMPARE(editor.toPlainText().section(u'\n', 0, 0), QStringLiteral("1中文"));
}

void WidgetControlsTests::switchDragPreviewsAndCommitsOnlyOnRelease() {
  uwf::ui::SwitchButton toggle;
  toggle.show();
  QVERIFY(QTest::qWaitForWindowExposed(&toggle));
  QSignalSpy changed(&toggle, &QAbstractButton::toggled);
  QTest::mousePress(&toggle, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
  uwf::test::moveMouse(&toggle, QPoint(30, 10));
  QVERIFY(!toggle.isChecked());
  QCOMPARE(changed.count(), 0);
  QTest::mouseRelease(&toggle, Qt::LeftButton, Qt::NoModifier, QPoint(30, 10));
  QVERIFY(toggle.isChecked());
  QCOMPARE(changed.count(), 1);
  QTest::mousePress(&toggle, Qt::LeftButton, Qt::NoModifier, QPoint(30, 10));
  uwf::test::moveMouse(&toggle, QPoint(10, 10));
  QVERIFY(toggle.isChecked());
  QCOMPARE(changed.count(), 1);
  QTest::mouseRelease(&toggle, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
  QVERIFY(!toggle.isChecked());
  QCOMPARE(changed.count(), 2);
}

void WidgetControlsTests::groupedLayoutReflowsWithoutClippingOrRecreatingControls() {
  QWidget host;
  auto* layout = new uwf::ui::FlowLayout(&host);
  const std::array sizes{QSize(180, 40), QSize(120, 28), QSize(200, 32)};
  std::array<QWidget*, 3> groups{};
  for (size_t i = 0; i < groups.size(); ++i) {
    groups[i] = new QWidget(&host);
    groups[i]->setFixedSize(sizes[i]);
    layout->addWidget(groups[i]);
  }
  layout->setAlignment(groups.back(), Qt::AlignRight);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  for (const auto direction : {Qt::LeftToRight, Qt::RightToLeft}) {
    host.setLayoutDirection(direction);
    for (int width = 200; width <= 1000; width += 13) {
      host.resize(width, layout->heightForWidth(width));
      QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
      for (size_t i = 0; i < groups.size(); ++i) {
        QCOMPARE(groups[i]->size(), sizes[i]);
        QVERIFY(host.rect().contains(groups[i]->geometry()));
        for (size_t j = i + 1; j < groups.size(); ++j) QVERIFY(!groups[i]->geometry().intersects(groups[j]->geometry()));
      }
    }
  }
  const int allVisibleHeight = layout->heightForWidth(200);
  groups[1]->hide();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QVERIFY(layout->heightForWidth(200) < allVisibleHeight);
  groups[1]->show();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QCOMPARE(layout->heightForWidth(200), allVisibleHeight);
  QCOMPARE(layout->count(), 3);
  QVERIFY(!layout->itemAt(-1));
  QVERIFY(!layout->takeAt(3));
  const std::unique_ptr<QLayoutItem> removed(layout->takeAt(1));
  QVERIFY(removed);
  QCOMPARE(removed->widget(), groups[1]);
  QCOMPARE(layout->count(), 2);
}

void WidgetControlsTests::tableCopyReflectsTheActualSelectionAndMissingCells() {
  QTableWidget table(3, 3);
  table.setHorizontalHeaderLabels({QStringLiteral("State"), QStringLiteral("Target"), QStringLiteral("Detail")});
  table.setItem(0, 0, new QTableWidgetItem(QStringLiteral("OK")));
  table.setItem(0, 1, new QTableWidgetItem(QStringLiteral("C:")));
  table.setItem(0, 2, new QTableWidgetItem(QStringLiteral("Protected")));
  table.setItem(1, 0, new QTableWidgetItem(QStringLiteral("Failed")));
  table.setItem(1, 2, new QTableWidgetItem(QStringLiteral("Unavailable")));
  table.setItem(2, 1, new QTableWidgetItem(QStringLiteral("D:")));

  table.setRangeSelected(QTableWidgetSelectionRange(0, 1, 1, 2), true);
  QCOMPARE(uwf::ui::tableSelectionToText(&table), QStringLiteral("C:\tProtected\n\tUnavailable\n"));
  QCOMPARE(uwf::ui::tableRowsToText(&table, 0, table.rowCount()), QStringLiteral("State\tTarget\tDetail\nOK\tC:\tProtected\nFailed\t\tUnavailable\n\tD:\t\n"));

  table.clearSelection();
  QVERIFY(uwf::ui::tableSelectionToText(&table).isEmpty());
}

}  // namespace
QTEST_MAIN(WidgetControlsTests)
#include "ControlsCases.moc"
