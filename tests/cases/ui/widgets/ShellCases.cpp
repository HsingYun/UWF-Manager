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
class WidgetShellTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() { initializeWidgets(); }
  void transientLabelPreservesNewBaselineUntilRestore();
  void hoverHintUsesRealWidgetEventsAndRestoresBaseline();
  void hoverHintDefersEverySourceAndCancelsStaleCandidates_data();
  void hoverHintDefersEverySourceAndCancelsStaleCandidates();
  void statusSurfaceKeepsGeometryAcrossHoverAndSelection();
  void summariesKeepCompletePartsAtMeasuredWidthBoundaries_data();
  void summariesKeepCompletePartsAtMeasuredWidthBoundaries();
  void statusSurfaceKeepsOneDynamicBaselineAcrossLanguages_data();
  void statusSurfaceKeepsOneDynamicBaselineAcrossLanguages();
  void framelessWindowsExposeAllResizeEdgesAndKeepUserSize();
  void captionButtonsSupportNativeCommandsAndCancelOutside();
  void startupGeometryFitsTheWorkArea_data();
  void startupGeometryFitsTheWorkArea();
};

void WidgetShellTests::transientLabelPreservesNewBaselineUntilRestore() {
  QLabel label;
  uwf::ui::TransientLabel transient(&label, &label);

  transient.setBaseline(QStringLiteral("Ready"));
  QCOMPARE(label.text(), QStringLiteral("Ready"));
  transient.show(QStringLiteral("Working"));
  QVERIFY(transient.isShowing());
  QCOMPARE(label.text(), QStringLiteral("Working"));

  transient.setBaseline(QStringLiteral("2 pending changes"));
  QCOMPARE(label.text(), QStringLiteral("Working"));
  transient.restoreAfter(1);
  QTRY_COMPARE_WITH_TIMEOUT(label.text(), QStringLiteral("2 pending changes"), 100);
  QVERIFY(!transient.isShowing());

  transient.flash(QStringLiteral("Copied"), 1);
  QCOMPARE(label.text(), QStringLiteral("Copied"));
  QTRY_COMPARE_WITH_TIMEOUT(label.text(), QStringLiteral("2 pending changes"), 100);
  transient.show(QStringLiteral("Held"));
  transient.restoreAfter(0);
  QCOMPARE(label.text(), QStringLiteral("2 pending changes"));
  transient.showHover(QStringLiteral("Hover"));
  transient.flash(QStringLiteral("Copied"), 300);
  transient.showHover(QStringLiteral("Another hover"));
  transient.setBaseline(QStringLiteral("Selected detail"));
  QTest::qWait(150);
  QCOMPARE(label.text(), QStringLiteral("Copied"));
  QTRY_COMPARE_WITH_TIMEOUT(label.text(), QStringLiteral("Another hover"), 500);
  transient.showHover({});
  QCOMPARE(label.text(), QStringLiteral("Selected detail"));
}

void WidgetShellTests::hoverHintUsesRealWidgetEventsAndRestoresBaseline() {
  QWidget window;
  window.resize(320, 120);
  auto* layout = new QVBoxLayout(&window);
  auto* hinted = new QPushButton(QStringLiteral("Target"), &window);
  hinted->setToolTip(QStringLiteral("Action description"));
  auto* plain = new QPushButton(QStringLiteral("Plain"), &window);
  auto* hintLabel = new QLabel(&window);
  layout->addWidget(hinted);
  layout->addWidget(plain);
  layout->addWidget(hintLabel);

  uwf::ui::TransientLabel transient(hintLabel, &window);
  transient.setBaseline(QStringLiteral("Baseline"));
  uwf::ui::HoverHintController controller(&window, &window);
  controller.setTarget(&transient);

  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  window.raise();
  window.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&window));
  uwf::test::moveMouse(window.windowHandle(), plain->mapTo(&window, plain->rect().center()));
  uwf::test::moveMouse(window.windowHandle(), hinted->mapTo(&window, hinted->rect().center()));
  QTest::qWait(100);
  QCOMPARE(hintLabel->text(), QStringLiteral("Baseline"));
  QTRY_COMPARE_WITH_TIMEOUT(hintLabel->text(), QStringLiteral("Action description"), 400);

  uwf::test::moveMouse(window.windowHandle(), plain->mapTo(&window, plain->rect().center()));
  QTRY_COMPARE_WITH_TIMEOUT(hintLabel->text(), QStringLiteral("Baseline"), 500);
}

void WidgetShellTests::hoverHintDefersEverySourceAndCancelsStaleCandidates_data() {
  QTest::addColumn<int>("kind");
  QTest::newRow("widget") << 0;
  QTest::newRow("tabs") << 1;
  QTest::newRow("items") << 2;
  QTest::newRow("menu") << 3;
}

void WidgetShellTests::hoverHintDefersEverySourceAndCancelsStaleCandidates() {
  QFETCH(int, kind);
  QWidget host;
  host.resize(500, 300);
  auto* layout = new QVBoxLayout(&host);
  auto* label = new QLabel(&host);
  layout->addWidget(label);
  uwf::ui::TransientLabel transient(label, &host);
  transient.setBaseline(QStringLiteral("Baseline"));
  uwf::ui::HoverHintController controller(&host, &host);
  QWidget* source = nullptr;
  std::array<QPoint, 2> positions;
  QStandardItemModel model(2, 1);
  if (kind == 0) {
    source = new QPushButton(QStringLiteral("Hover target"), &host);
    source->setToolTip(QStringLiteral("First"));
    layout->addWidget(source);
  } else if (kind == 1) {
    auto* tabs = new QTabBar(&host);
    for (const auto& text : {QStringLiteral("First"), QStringLiteral("Second")}) tabs->setTabToolTip(tabs->addTab(text), text);
    layout->addWidget(tabs);
    source = tabs;
  } else if (kind == 2) {
    auto* view = new QListView(&host);
    model.setData(model.index(0, 0), QStringLiteral("First"), Qt::ToolTipRole);
    model.setData(model.index(1, 0), QStringLiteral("Second"), Qt::ToolTipRole);
    view->setModel(&model);
    layout->addWidget(view);
    source = view->viewport();
  } else {
    auto* menu = new QMenu(&host);
    menu->addAction(QStringLiteral("One"))->setToolTip(QStringLiteral("First"));
    menu->addAction(QStringLiteral("Two"))->setToolTip(QStringLiteral("Second"));
    source = menu;
  }
  // Exercise candidate/debounce policy with a controlled widget event stream.
  // Native exposure/paint can block beyond the debounce interval; real window
  // input is covered separately by hoverHintUsesRealWidgetEventsAndRestoresBaseline.
  host.ensurePolished();
  layout->activate();
  if (auto* menu = qobject_cast<QMenu*>(source)) {
    menu->ensurePolished();
    menu->adjustSize();
  }
  if (auto* tabs = qobject_cast<QTabBar*>(source))
    positions = {tabs->tabRect(0).center(), tabs->tabRect(1).center()};
  else if (kind == 2) {
    auto* view = qobject_cast<QListView*>(source->parentWidget());
    positions = {view->visualRect(model.index(0, 0)).center(), view->visualRect(model.index(1, 0)).center()};
  } else if (auto* menu = qobject_cast<QMenu*>(source)) {
    const auto actions = menu->actions();
    positions = {menu->actionGeometry(actions.at(0)).center(), menu->actionGeometry(actions.at(1)).center()};
  } else
    positions = {source->rect().center(), source->rect().center() + QPoint(1, 0)};
  const auto move = [&](QPoint point) {
    const auto global = source->mapToGlobal(point);
    QMouseEvent event(QEvent::MouseMove, point, global, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(source, &event);
  };
  controller.setTarget(&transient);
  move(positions[0]);
  QCOMPARE(label->text(), QStringLiteral("Baseline"));
  QTest::qWait(80);
  if (kind == 0) source->setToolTip(QStringLiteral("Second"));
  QElapsedTimer elapsed;
  elapsed.start();
  move(positions[1]);
  QTest::qWait(130);
  QCOMPARE(label->text(), QStringLiteral("Baseline"));
  // Continued movement in the same item must not postpone a stable hint.
  move(positions[1]);
  QTRY_COMPARE_WITH_TIMEOUT(label->text(), QStringLiteral("Second"), 400);
  QVERIFY(elapsed.elapsed() >= 200);
  QEvent leave(QEvent::Leave);
  QApplication::sendEvent(source, &leave);
  QTRY_COMPARE_WITH_TIMEOUT(label->text(), QStringLiteral("Baseline"), 500);
  move(positions[0]);
  controller.setTarget(nullptr);
  QTest::qWait(250);
  QCOMPARE(label->text(), QStringLiteral("Baseline"));
}

void WidgetShellTests::statusSurfaceKeepsGeometryAcrossHoverAndSelection() {
  using namespace uwf::ui;
  QWidget host;
  auto* column = new QVBoxLayout(&host);
  auto* content = new QPushButton(QStringLiteral("Hover target"), &host);
  column->addWidget(content, 1);
  auto* status = new StatusSurface(&host);
  column->addWidget(status);
  TransientLabel transient(status, status);
  HoverHintController hover(&host);
  hover.setTarget(&transient);
  const QString detail = QStringLiteral("C:\\Long folder\\file.txt\r\nComplete detail\nThird line");
  status->setDetail(detail);
  transient.setBaseline(detail);
  host.resize(920, 240);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  auto* copy = status->findChild<QPushButton*>(QStringLiteral("statusCopyButton"));
  auto* label = status->findChild<QLabel*>(QStringLiteral("statusBarLabel"));
  QVERIFY(copy && label);
  const QRect footer = status->geometry(), body = content->geometry();
  const auto previousClipboard = QApplication::clipboard()->text();
  const auto restore = qScopeGuard([&] { QApplication::clipboard()->setText(previousClipboard); });
  for (const auto& hint : {QStringLiteral("Short hint"), QStringLiteral("Long hover text ").repeated(80), QStringLiteral("First\nSecond\r\nThird")}) {
    content->setToolTip(hint);
    uwf::test::moveMouse(host.windowHandle(), QPoint(1, 1));
    uwf::test::moveMouse(host.windowHandle(), content->mapTo(&host, content->rect().center()));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QCOMPARE(status->geometry(), footer);
    QCOMPARE(content->geometry(), body);
    QTRY_VERIFY_WITH_TIMEOUT(copy->isHidden(), 500);
    QVERIFY(!label->text().contains(u'\n'));
    transient.restoreAfter(0);
    QTRY_VERIFY(copy->isVisible());
    QTest::mouseClick(copy, Qt::LeftButton);
    QCOMPARE(QApplication::clipboard()->text(), detail);
    QCOMPARE(status->geometry(), footer);
    status->setPending(12);
    status->setAccess(false);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QCOMPARE(status->geometry(), footer);
    QCOMPARE(content->geometry(), body);
  }
  host.resize(600, 240);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QCOMPARE(status->height(), footer.height());
  QVERIFY(status->rect().contains(QRect(copy->mapTo(status, QPoint()), copy->size())));
}

void WidgetShellTests::summariesKeepCompletePartsAtMeasuredWidthBoundaries_data() {
  QTest::addColumn<QStringList>("parts");
  QTest::newRow("latin") << QStringList{QStringLiteral("Windows 11 Pro"), QStringLiteral("32G"), QStringLiteral("RTX 4060")};
  QTest::newRow("cjk") << QStringList{QStringLiteral("系统测试"), QStringLiteral("32G 内存"), QStringLiteral("显卡信息")};
  QTest::newRow("rtl") << QStringList{QStringLiteral("نظام التشغيل"), QStringLiteral("32G"), QStringLiteral("بطاقة الرسومات")};
  QTest::newRow("missing-hardware") << QStringList{QStringLiteral("Windows"), QString(), QString()};
}

void WidgetShellTests::summariesKeepCompletePartsAtMeasuredWidthBoundaries() {
  QFETCH(QStringList, parts);
  QWidget host;
  uwf::ui::SummaryLabel summary(parts, &host);
  summary.setContentsMargins(3, 0, 5, 0);
  host.resize(700, 100);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  parts.removeAll(QString());
  const auto fullText = parts.join(QStringLiteral(" · "));
  const auto naturalSize = summary.sizeHint();
  for (int count = 1; count <= parts.size(); ++count) {
    const auto prefix = parts.first(count).join(QStringLiteral(" · "));
    const int boundary = summary.fontMetrics().horizontalAdvance(prefix) + 8;
    for (const int adjustment : {0, -1, 1}) {
      summary.resize(boundary + adjustment, 30);
      const auto expected = count > 1 && adjustment < 0 ? parts.first(count - 1).join(QStringLiteral(" · ")) : prefix;
      QTRY_COMPARE(summary.text(), expected);
      QCOMPARE(summary.fullText(), fullText);
      QCOMPARE(summary.toolTip(), fullText);
#if QT_CONFIG(accessibility)
      QCOMPARE(summary.accessibleName(), fullText);
#endif
      QCOMPARE(summary.sizeHint(), naturalSize);
    }
  }
  if (parts.size() > 1) {
    summary.resize(summary.fontMetrics().horizontalAdvance(parts.first(2).join(QStringLiteral(" · "))) + 8, 50);
    auto largeFont = summary.font();
    largeFont.setPixelSize(largeFont.pixelSize() * 2);
    summary.setFont(largeFont);
    QTRY_COMPARE(summary.text(), parts.front());
  }
  summary.resize(10, 50);
  QTRY_COMPARE(summary.text(), parts.front());
  QSignalSpy activated(&summary, &uwf::ui::SummaryLabel::activated);
  QTest::mouseClick(&summary, Qt::RightButton);
  QCOMPARE(activated.count(), 0);
  QTest::mousePress(&summary, Qt::LeftButton);
  QTest::mouseRelease(&summary, Qt::LeftButton, Qt::NoModifier, QPoint(-1, -1));
  QCOMPARE(activated.count(), 0);
  QTest::mouseClick(&summary, Qt::LeftButton);
  QTest::keyClick(&summary, Qt::Key_Return);
  QTest::keyClick(&summary, Qt::Key_Space);
  QCOMPARE(activated.count(), 3);
  for (const auto& activation : activated) QCOMPARE(activation.front().toString(), fullText);
  summary.setParts({});
  QCOMPARE(summary.text(), QString());
  QCOMPARE(summary.fullText(), QString());
  QCOMPARE(summary.toolTip(), QString());
}

void WidgetShellTests::statusSurfaceKeepsOneDynamicBaselineAcrossLanguages_data() {
  QTest::addColumn<bool>("dark");
  QTest::newRow("light") << false;
  QTest::newRow("dark") << true;
}

void WidgetShellTests::statusSurfaceKeepsOneDynamicBaselineAcrossLanguages() {
  QFETCH(bool, dark);
  auto& theme = uwf::ui::ThemeManager::instance();
  auto& language = uwf::ui::I18n::instance();
  const auto originalTheme = theme.current();
  const auto originalLanguage = language.lang();
  const auto restore = qScopeGuard([&] {
    theme.apply(originalTheme);
    language.setLang(originalLanguage);
  });
  theme.apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  language.setLang(uwf::ui::I18n::Lang::En);
  QWidget host;
  auto* column = new QVBoxLayout(&host);
  column->addStretch();
  auto* status = new uwf::ui::StatusSurface(&host);
  column->addWidget(status);
  host.resize(1100, 180);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  auto* label = status->findChild<uwf::ui::TextLabel*>(QStringLiteral("statusBarLabel"));
  QVERIFY(label);
  uwf::ui::TransientLabel transient(status, status);
  const auto font = label->font();
  const int origin = label->mapTo(status, QPoint()).y();
  const int height = label->height();
  QRect reference;
  for (const auto& locale : uwf::ui::I18n::languages()) {
    language.setLang(locale.id);
    const QStringList messages{uwf::ui::I18n::tr("Ready"),
                               QStringLiteral("Windows 11 Enterprise · 96G · RTX 5090"),
                               QStringLiteral("C:\\用户目录\\配置.json"),
                               uwf::ui::I18n::tr("UWF is not available"),
                               uwf::ui::I18n::tr("%1 pending change(s)").arg(127).repeated(30),
                               QStringLiteral("First line\r\n第二行\nالسطر الثالث")};
    for (qsizetype index = 0; index < messages.size(); ++index) {
      status->setDetail(messages[index]);
      transient.setBaseline(messages[index]);
      status->setPending(index % 2 ? 12 : 0);
      status->setAccess(index % 2 == 0);
      status->setCompatibility(index % 2 != 0);
      QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
      QCOMPARE(label->font(), font);
      QCOMPARE(label->mapTo(status, QPoint()).y(), origin);
      QCOMPARE(label->height(), height);
      QVERIFY(!label->text().contains(u'\n'));
      const qreal lineCenter = origin + label->firstLineBoxBounds().center().y();
      QVERIFY(qAbs(lineCenter - status->height() / 2.0) < .5);
      if ((locale.id == uwf::ui::I18n::Lang::En || locale.id == uwf::ui::I18n::Lang::Zh_CN || locale.id == uwf::ui::I18n::Lang::Ar) && index < 3)
        QVERIFY(uwf::test::saveScreenshot(*status, QStringLiteral("dynamic-status-%1-%2").arg(QString(locale.code)).arg(index)));

      // Follow one real Latin glyph through text, fallback-font and RTL changes.
      // Its raster position catches baseline motion that whole-word centering hides.
      transient.show(QStringLiteral("H ") + messages[index]);
      QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
      const auto value = label->fontMetrics().elidedText(label->text(), Qt::ElideRight, label->contentsRect().width());
      QTextLayout layout(value, label->font());
      QTextOption option;
      option.setWrapMode(QTextOption::NoWrap);
      option.setTextDirection(label->layoutDirection());
      option.setAlignment(QStyle::visualAlignment(label->layoutDirection(), label->alignment()) & Qt::AlignHorizontal_Mask);
      layout.setTextOption(option);
      layout.beginLayout();
      auto line = layout.createLine();
      QVERIFY(line.isValid());
      line.setLineWidth(label->contentsRect().width());
      layout.endLayout();
      const auto pixels = label->grab().toImage();
      const qreal scale = pixels.devicePixelRatio();
      const auto glyph = uwf::ui::textInkBounds(layout, 0, 1);
      const int left = qFloor(glyph.left() * scale), right = qCeil(glyph.right() * scale);
      const auto runs =
          uwf::test::horizontalInkRuns(pixels, dark ? Qt::white : Qt::black, pixels.pixelColor(0, 0), QRect(left, 0, right - left, pixels.height()), 1);
      QRect marker;
      for (const auto& run : runs) marker = marker.united(run);
      QVERIFY2(!marker.isEmpty(), qUtf8Printable(QStringLiteral("%1 / %2: missing marker in x=%3..%4, label width=%5")
                                                     .arg(QString(locale.code), messages[index])
                                                     .arg(left)
                                                     .arg(right)
                                                     .arg(pixels.width())));
      if (reference.isEmpty()) reference = marker;
      QVERIFY2(marker.top() == reference.top() && marker.bottom() == reference.bottom(),
               qUtf8Printable(QStringLiteral("%1 / %2: marker y=%3..%4, reference=%5..%6")
                                  .arg(QString(locale.code), messages[index])
                                  .arg(marker.top())
                                  .arg(marker.bottom())
                                  .arg(reference.top())
                                  .arg(reference.bottom())));
      transient.restoreAfter(0);
      QCOMPARE(status->detail(), messages[index]);
    }
  }
}

void WidgetShellTests::framelessWindowsExposeAllResizeEdgesAndKeepUserSize() {
  using namespace uwf::ui;
  class MainWindowProbe final : public QMainWindow {
   public:
    MainWindowProbe() : chrome(this) { setWindowFlag(Qt::FramelessWindowHint); }
    WindowChromeController chrome;

   protected:
    bool nativeEvent(const QByteArray& type, void* message, qintptr* result) override {
      return chrome.nativeEvent(message, result) || QMainWindow::nativeEvent(type, message, result);
    }
  } main;
  PanelDialog dialog;
  auto* layout = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(520, 320), QSize(300, 200));
  auto* message = new QLabel(QStringLiteral("Resizable content"), &dialog);
  layout->body()->addWidget(message);
  for (QWidget* window : {static_cast<QWidget*>(&main), static_cast<QWidget*>(&dialog)}) {
    window->resize(520, 320);
    window->showNormal();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    applyWindowAppearance(window);
    const auto handle = reinterpret_cast<HWND>(window->winId());
    SendMessageW(handle, WM_SYSCOMMAND, SC_RESTORE, 0);
    QTRY_VERIFY(!IsZoomed(handle));
    QVERIFY(GetWindowLongPtrW(handle, GWL_STYLE) & WS_THICKFRAME);
    RECT client{};
    QVERIFY(GetClientRect(handle, &client));
    RECT bounds{};
    QVERIFY(GetWindowRect(handle, &bounds));
    QCOMPARE(client.right - client.left, bounds.right - bounds.left);
    QCOMPARE(client.bottom - client.top, bounds.bottom - bounds.top);
    // A top/left resize changes the origin as well as the size. Returning zero
    // would let Windows move an old frame before Qt paints the new layout.
    NCCALCSIZE_PARAMS calculation{};
    calculation.rgrc[0] = bounds;
    calculation.rgrc[0].top += 20;
    calculation.rgrc[1] = calculation.rgrc[2] = bounds;
    WINDOWPOS proposedPosition{handle, nullptr, bounds.left, bounds.top + 20, bounds.right - bounds.left, bounds.bottom - bounds.top - 20, SWP_NOZORDER};
    calculation.lppos = &proposedPosition;
    const auto proposed = calculation.rgrc[0];
    QCOMPARE(SendMessageW(handle, WM_NCCALCSIZE, TRUE, reinterpret_cast<LPARAM>(&calculation)), static_cast<LRESULT>(WVR_REDRAW));
    QCOMPARE(calculation.rgrc[0].top, proposed.top);
    QCOMPARE(calculation.rgrc[0].bottom, proposed.bottom);
    const int left = bounds.left + 1, right = bounds.right - 2, top = bounds.top + 1, bottom = bounds.bottom - 2;
    const int x = (left + right) / 2, y = (top + bottom) / 2;
    for (const auto& [point, expected] : std::array<std::pair<QPoint, int>, 8>{{{{left, top}, HTTOPLEFT},
                                                                                {{x, top}, HTTOP},
                                                                                {{right, top}, HTTOPRIGHT},
                                                                                {{right, y}, HTRIGHT},
                                                                                {{right, bottom}, HTBOTTOMRIGHT},
                                                                                {{x, bottom}, HTBOTTOM},
                                                                                {{left, bottom}, HTBOTTOMLEFT},
                                                                                {{left, y}, HTLEFT}}})
      QCOMPARE(SendMessageW(handle, WM_NCHITTEST, 0, MAKELPARAM(point.x(), point.y())), static_cast<LRESULT>(expected));
    RECT visible{};
    if (SUCCEEDED(DwmGetWindowAttribute(handle, DWMWA_EXTENDED_FRAME_BOUNDS, &visible, sizeof(visible)))) {
      const int visibleX = (visible.left + visible.right) / 2;
      QCOMPARE(SendMessageW(handle, WM_NCHITTEST, 0, MAKELPARAM(visibleX, visible.top + 1)), static_cast<LRESULT>(HTTOP));
      QCOMPARE(SendMessageW(handle, WM_NCHITTEST, 0, MAKELPARAM(visible.left + 1, visible.top + 1)), static_cast<LRESULT>(HTTOPLEFT));
    }
    const QSize enlarged = window->size() + QSize(80, 60);
    SendMessageW(handle, WM_ENTERSIZEMOVE, 0, 0);
    window->resize(enlarged);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QTest::qWait(30);
    QCOMPARE(window->size(), enlarged);
    SendMessageW(handle, WM_EXITSIZEMOVE, 0, 0);
    message->setText(QStringLiteral("Updated content after a user resize"));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QTest::qWait(30);
    QCOMPARE(window->size(), enlarged);
    window->setFixedWidth(window->width());
    QVERIFY(SendMessageW(handle, WM_NCHITTEST, 0, MAKELPARAM(left, y)) != HTLEFT);
    window->hide();
  }
}

void WidgetShellTests::captionButtonsSupportNativeCommandsAndCancelOutside() {
  class CaptionWindow final : public QMainWindow {
   public:
    CaptionWindow() : chrome(this) { setWindowFlag(Qt::FramelessWindowHint); }
    uwf::ui::WindowChromeController chrome;
    bool closeAttempted = false;

   protected:
    bool nativeEvent(const QByteArray& type, void* message, qintptr* result) override {
      return chrome.nativeEvent(message, result) || QMainWindow::nativeEvent(type, message, result);
    }
    void closeEvent(QCloseEvent* event) override {
      closeAttempted = true;
      event->ignore();
    }
  } window;
  auto* content = new QWidget(&window);
  auto* column = new QVBoxLayout(content);
  column->setContentsMargins(0, 0, 0, 0);
  column->addWidget(new uwf::ui::WindowTitleBar(content));
  column->addStretch();
  window.setCentralWidget(content);
  window.resize(640, 400);
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  uwf::ui::applyWindowAppearance(&window);
  const auto handle = reinterpret_cast<HWND>(window.winId());
  const auto styles = GetWindowLongPtrW(handle, GWL_STYLE);
  QVERIFY(styles & WS_SYSMENU);
  QVERIFY(styles & WS_THICKFRAME);
  QVERIFY(styles & WS_MINIMIZEBOX);
  QVERIFY(styles & WS_MAXIMIZEBOX);
  QVERIFY(!(styles & WS_CAPTION));
  QAbstractButton* maximize = nullptr;
  for (const auto buttons = window.findChildren<QAbstractButton*>(); auto* button : buttons)
    if (button->property("nativeCaptionHit").toInt() == HTMAXBUTTON) maximize = button;
  QVERIFY(maximize);
  QCOMPARE(maximize->toolTip(), uwf::ui::I18n::tr("Maximize"));
  const auto clickCaption = [&](const int hit, const bool cancel = false) {
    QAbstractButton* button = nullptr;
    for (const auto buttons = window.findChildren<QAbstractButton*>(); auto* candidate : buttons)
      if (candidate->property("nativeCaptionHit").toInt() == hit) button = candidate;
    QVERIFY(button);
    const auto local = button->mapTo(&window, button->rect().center());
    const auto scale = window.devicePixelRatioF();
    POINT screen{qRound(local.x() * scale), qRound(local.y() * scale)};
    QVERIFY(ClientToScreen(handle, &screen));
    QCOMPARE(SendMessageW(handle, WM_NCHITTEST, 0, MAKELPARAM(screen.x, screen.y)), static_cast<LRESULT>(hit));
    SendMessageW(handle, WM_NCLBUTTONDOWN, static_cast<WPARAM>(hit), MAKELPARAM(screen.x, screen.y));
    QVERIFY(button->isDown());
    const QPoint released = cancel ? QPoint(30, 100) : local;
    SendMessageW(handle, WM_LBUTTONUP, 0, MAKELPARAM(qRound(released.x() * scale), qRound(released.y() * scale)));
    QVERIFY(!button->isDown());
  };
  clickCaption(HTMAXBUTTON, true);
  QVERIFY(!window.isMaximized());
  clickCaption(HTMAXBUTTON);
  QTRY_VERIFY(window.isMaximized());
  QTRY_COMPARE(maximize->toolTip(), uwf::ui::I18n::tr("Restore"));
  MONITORINFO monitor{sizeof(MONITORINFO), {}, {}, 0};
  QVERIFY(GetMonitorInfoW(MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST), &monitor));
  RECT client{};
  QVERIFY(GetClientRect(handle, &client));
  POINT origin{};
  QVERIFY(ClientToScreen(handle, &origin));
  QCOMPARE(origin.x, monitor.rcWork.left);
  QCOMPARE(origin.y, monitor.rcWork.top);
  QCOMPARE(client.right, monitor.rcWork.right - monitor.rcWork.left);
  QCOMPARE(client.bottom, monitor.rcWork.bottom - monitor.rcWork.top);
  clickCaption(HTMAXBUTTON);
  QTRY_VERIFY(!window.isMaximized());
  QTRY_COMPARE(maximize->toolTip(), uwf::ui::I18n::tr("Maximize"));
  auto* title = window.findChild<QLabel*>(QStringLiteral("mainApplicationName"));
  QVERIFY(title);
  const auto local = title->mapTo(&window, title->rect().center());
  POINT titlePosition{qRound(local.x() * window.devicePixelRatioF()), qRound(local.y() * window.devicePixelRatioF())};
  QVERIFY(ClientToScreen(handle, &titlePosition));
  QCOMPARE(SendMessageW(handle, WM_NCHITTEST, 0, MAKELPARAM(titlePosition.x, titlePosition.y)), static_cast<LRESULT>(HTCAPTION));
  SendMessageW(handle, WM_NCLBUTTONDBLCLK, HTCAPTION, MAKELPARAM(titlePosition.x, titlePosition.y));
  QTRY_VERIFY(window.isMaximized());
  SendMessageW(handle, WM_NCLBUTTONDBLCLK, HTCAPTION, MAKELPARAM(titlePosition.x, titlePosition.y));
  QTRY_VERIFY(!window.isMaximized());
  clickCaption(HTMINBUTTON);
  QTRY_VERIFY(window.isMinimized());
  SendMessageW(handle, WM_SYSCOMMAND, SC_RESTORE, 0);
  QTRY_VERIFY(!window.isMinimized());
  clickCaption(HTCLOSE);
  QTRY_VERIFY(window.closeAttempted);
  QVERIFY(window.isVisible());
}

void WidgetShellTests::startupGeometryFitsTheWorkArea_data() {
  QTest::addColumn<QRect>("available");
  QTest::addColumn<QMargins>("frame");
  QTest::addColumn<QSize>("minimum");
  QTest::newRow("desktop") << QRect(0, 0, 1920, 1032) << QMargins(8, 31, 8, 8) << QSize(924, 472);
  QTest::newRow("laptop") << QRect(0, 0, 1366, 720) << QMargins(8, 31, 8, 8) << QSize(924, 472);
  QTest::newRow("compact") << QRect(0, 0, 1024, 720) << QMargins(8, 31, 8, 8) << QSize(924, 472);
  QTest::newRow("smaller-than-content") << QRect(0, 0, 800, 560) << QMargins(8, 31, 8, 8) << QSize(924, 620);
  QTest::newRow("portrait") << QRect(-1080, 48, 1080, 1872) << QMargins(8, 31, 8, 8) << QSize(924, 472);
  QTest::newRow("secondary-above") << QRect(622, -1440, 2048, 1090) << QMargins(6, 29, 6, 6) << QSize(924, 472);
  QTest::newRow("taskbar-left") << QRect(-1552, 0, 1552, 900) << QMargins(8, 31, 8, 8) << QSize(1050, 500);
  QTest::newRow("ultrawide") << QRect(0, 0, 3440, 1392) << QMargins(8, 31, 8, 8) << QSize(924, 472);
  QTest::newRow("empty") << QRect() << QMargins() << QSize();
}

void WidgetShellTests::startupGeometryFitsTheWorkArea() {
  QFETCH(QRect, available);
  QFETCH(QMargins, frame);
  QFETCH(QSize, minimum);
  const auto geometry = uwf::ui::WindowChromeController::startupGeometry(available, frame, minimum);
  if (available.isEmpty()) {
    QVERIFY(geometry.isEmpty());
    return;
  }
  const auto outer = geometry.marginsAdded(frame);
  QVERIFY(available.contains(outer));
  QVERIFY(std::abs(outer.center().x() - available.center().x()) <= 1);
  QVERIFY(std::abs(outer.center().y() - available.center().y()) <= 1);
  QVERIFY(geometry.width() > 0 && geometry.height() > 0);
  const auto fullClient = available.size() - QSize(frame.left() + frame.right(), frame.top() + frame.bottom());
  QVERIFY(geometry.width() >= std::min(minimum.width(), fullClient.width()));
  QVERIFY(geometry.height() >= std::min(minimum.height(), fullClient.height()));
  // 更多工作区不能使默认窗口变小，桌面坐标平移不能改变尺寸和相对位置。
  const QRect larger(available.topLeft(), available.size() + QSize(300, 200));
  const auto expanded = uwf::ui::WindowChromeController::startupGeometry(larger, frame, minimum);
  QVERIFY(expanded.width() >= geometry.width());
  QVERIFY(expanded.height() >= geometry.height());
  const QPoint offset(-2471, 613);
  QCOMPARE(uwf::ui::WindowChromeController::startupGeometry(available.translated(offset), frame, minimum), geometry.translated(offset));
}

}  // namespace
QTEST_MAIN(WidgetShellTests)
#include "ShellCases.moc"
