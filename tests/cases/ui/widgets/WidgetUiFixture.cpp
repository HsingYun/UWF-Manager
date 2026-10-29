// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include "WidgetUiFixture.h"

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
namespace uwf::test::widgets {
using uwf::test::FileDialogStub;

QAction* actionWithText(QWidget* root, const QString& text) {
  for (const auto actions = root->findChildren<QAction*>(); auto* action : actions) {
    if (action->text() == text) return action;
  }
  return nullptr;
}

QPushButton* buttonWithText(QWidget* root, const QString& text) {
  for (const auto buttons = root->findChildren<QPushButton*>(); auto* button : buttons) {
    if (button->text() == text || button->accessibleName() == text) return button;
  }
  return nullptr;
}

void checkWholeRowScrolling(QAbstractItemView* view) {
  const int rowHeight = uwf::test::displayedRowHeight(view);
  QVERIFY(rowHeight > 0);
  QCOMPARE(view->verticalScrollMode(), QAbstractItemView::ScrollPerItem);
  QCOMPARE(view->verticalScrollBar()->singleStep(), 1);
  const QPoint local(40, 30);
  for (const int delta : {-15, -30, -60, -120, -240, 15, 30, 60, 120, 240}) {
    QWheelEvent wheel(local, view->viewport()->mapToGlobal(local), QPoint(), QPoint(0, delta), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(view->viewport(), &wheel);
    const auto first = view->indexAt(QPoint(10, 0));
    QVERIFY(first.isValid());
    QCOMPARE(view->visualRect(first).top(), 0);
    QCOMPARE(view->viewport()->height() % rowHeight, 0);
    const int lastRow = first.row() + view->viewport()->height() / rowHeight - 1;
    if (lastRow < view->model()->rowCount()) QCOMPARE(view->visualRect(view->model()->index(lastRow, first.column())).bottom() + 1, view->viewport()->height());
  }
  view->verticalScrollBar()->setValue(7);
  const auto first = view->indexAt(QPoint(10, 0));
  QCOMPARE(first.row(), 7);
  QCOMPARE(view->visualRect(first).top(), 0);
}

void checkHoverAndSmoothDragging(QAbstractItemView* view, uwf::presentation::PageProxyModel& pages) {
  auto* bar = view->findChild<QScrollBar*>(QStringLiteral("entryScrollOverlay"));
  QVERIFY(bar);
  const int rowHeight = uwf::test::displayedRowHeight(view);
  const QPoint edge(view->viewport()->width() - 2, 20);
  QMouseEvent hover(QEvent::MouseMove, edge, view->viewport()->mapToGlobal(edge), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  QCoreApplication::sendEvent(view->viewport(), &hover);
  QVERIFY(bar->isVisible());
  bar->setSliderDown(true);
  QCOMPARE(view->verticalScrollMode(), QAbstractItemView::ScrollPerPixel);
  const int pixels = rowHeight * 2 + rowHeight / 2;
  bar->setValue(pixels);
  QCOMPARE(view->verticalScrollBar()->value(), pixels);
  QVERIFY(view->visualRect(view->indexAt(QPoint(10, 0))).top() < 0);
  bar->setSliderDown(false);
  QWheelEvent wheel(edge, view->viewport()->mapToGlobal(edge), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
  QCoreApplication::sendEvent(view->viewport(), &wheel);
  QCOMPARE(view->verticalScrollMode(), QAbstractItemView::ScrollPerItem);
  QCOMPARE(view->visualRect(view->indexAt(QPoint(10, 0))).top(), 0);
  bar->setSliderDown(true);
  bar->setValue(pixels);
  bar->setSliderDown(false);
  pages.goNext();
  QCOMPARE(view->verticalScrollMode(), QAbstractItemView::ScrollPerItem);
  QCOMPARE(view->visualRect(view->indexAt(QPoint(10, 0))).top(), 0);
  QEvent leave(QEvent::Leave);
  QCoreApplication::sendEvent(bar, &leave);
  QCoreApplication::sendEvent(view->viewport(), &leave);
}

void initializeWidgets() {
  uwf::test::initializeVisualComparison();
  uwf::ui::I18n::instance().setLang(uwf::ui::I18n::Lang::En);
  uwf::ui::ThemeManager::instance().apply(uwf::app::Theme::Dark);
}
}  // namespace uwf::test::widgets
