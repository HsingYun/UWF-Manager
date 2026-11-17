// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <dwmapi.h>
#include <qt_windows.h>

#include <QAbstractButton>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QFontMetrics>
#include <QFontMetricsF>
#include <QImage>
#include <QLabel>
#include <QListView>
#include <QMainWindow>
#include <QMargins>
#include <QMenu>
#include <QMouseEvent>
#include <QPoint>
#include <QPushButton>
#include <QRect>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QSize>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QTabBar>
#include <QTest>
#include <QTextLayout>
#include <QTextOption>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <array>
#include <utility>

#include "cases/presentation/views/widgets/WidgetUiFixture.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/appearance/WindowAppearance.h"
#include "presentation/controls/feedback/TransientLabel.h"
#include "presentation/controls/text/SummaryLabel.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/painting/TextPainting.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogLayout.h"
#include "presentation/dialogs/PanelDialog.h"
#include "presentation/views/shell/HoverHintController.h"
#include "presentation/views/shell/StatusSurface.h"
#include "presentation/views/shell/WindowChromeController.h"
#include "presentation/views/shell/WindowTitleBar.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/InkBounds.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

using namespace uwf::test::widgets;

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
  void statusSurfaceAlignsSelectionDetail_data();
  void statusSurfaceAlignsSelectionDetail();
  void titleBarAlignsApplicationNameWithIcon_data();
  void titleBarAlignsApplicationNameWithIcon();
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
  uwf::test::moveMouse(plain);
  uwf::test::moveMouse(hinted);
  QTest::qWait(100);
  QCOMPARE(hintLabel->text(), QStringLiteral("Baseline"));
  QTRY_COMPARE_WITH_TIMEOUT(hintLabel->text(), QStringLiteral("Action description"), 400);

  uwf::test::moveMouse(plain);
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
  const auto modes = QStringLiteral("Read-only ROM · Freespace Passthrough · Persistent Overlay · Compatibility mode");
  status->setRuntimeModes(modes);
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
    uwf::test::moveMouse(content);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QCOMPARE(status->geometry(), footer);
    QCOMPARE(content->geometry(), body);
    QTRY_VERIFY_WITH_TIMEOUT(!copy->isVisibleTo(status), 500);
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
  auto* modesLabel = status->findChild<TextLabel*>(QStringLiteral("runtimeModeBadge"));
  QVERIFY(modesLabel);
  QCOMPARE(host.width(), 600);
  QCOMPARE(modesLabel->toolTip(), modes);
  QCOMPARE(modesLabel->elideMode(), Qt::ElideRight);
  QVERIFY(modesLabel->width() < modesLabel->sizeHint().width());
  QVERIFY(status->rect().contains(QRect(modesLabel->mapTo(status, QPoint()), modesLabel->size())));
  status->setRuntimeModes({});
  QVERIFY(modesLabel->isHidden());
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
  auto* icon = status->findChild<QWidget*>(QStringLiteral("statusBarIcon"));
  auto* pendingIcon = status->findChild<QWidget*>(QStringLiteral("statusPendingIndicator"));
  auto* pendingLabel = status->findChild<uwf::ui::TextLabel*>(QStringLiteral("statusPendingLabel"));
  QVERIFY(label && icon && pendingIcon && pendingLabel);
  uwf::ui::TransientLabel transient(status, status);
  const auto font = label->font();
  const int origin = label->mapTo(status, QPoint()).y();
  const int height = label->height();
  QRect reference;
  // Observe the same info.svg in the composed footer, independently of the
  // production alignment reference. Text changes must not move its raster.
  QRectF infoReference;
  for (const auto& locale : uwf::ui::I18n::languages()) {
    language.setLang(locale.id);
    const QStringList messages{uwf::ui::I18n::tr("Ready"),
                               QStringLiteral("Windows 11 Enterprise · 96G · RTX 5090"),
                               QStringLiteral("C:\\用户目录\\配置.json"),
                               uwf::ui::I18n::tr("UWF is not available"),
                               uwf::ui::I18n::tr("%1 pending change(s)").arg(127).repeated(30),
                               QStringLiteral("First line\r\n第二行\nالسطر الثالث"),
                               QStringLiteral("WINDOWS 11"),
                               QStringLiteral("gypqj"),
                               QStringLiteral("ÁÉÎÖÜ"),
                               QStringLiteral("系统就绪，配置未应用"),
                               QStringLiteral("文件夹：C:\\Program Files (x86)\\Microsoft\\Edge 当前会话：排除 下次会话：排除"),
                               QStringLiteral("Windows 11 Enterprise · 96G · RTX 5090")};
    for (qsizetype index = 0; index < messages.size(); ++index) {
      status->setDetail(messages[index]);
      transient.setBaseline(messages[index]);
      status->setPending(index % 2 ? 12 : 0);
      status->setAccess(index % 2 == 0);
      status->setRuntimeModes(index % 2 != 0 ? uwf::ui::I18n::tr("Compatibility mode") : QString{});
      QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
      const qreal actualScale = status->devicePixelRatioF();
      const auto physicallyAligned = [&](const QRectF& first, const QRectF& second) {
        return !first.isEmpty() && !second.isEmpty() && std::abs(first.center().y() - second.center().y()) * actualScale <= 1.0;
      };
      const auto frame = status->grab().toImage();
      const auto& system = uwf::ui::DesignSystem::instance();
      const auto iconInkInFrame = [&](QWidget* graphic, const uwf::ui::design::Color role, const QImage& image) {
        return uwf::test::visibleInkBounds(image, system.color(role), system.color(uwf::ui::design::Color::Footer),
                                           QRectF(graphic->mapTo(status, QPoint()), graphic->size()));
      };
      const auto detailReference = label->alignmentBounds(true).translated(label->mapTo(status, QPoint()));
      const auto detailIconInk = iconInkInFrame(icon, uwf::ui::design::Color::Muted, frame);
      QVERIFY2(physicallyAligned(detailReference, detailIconInk), qUtf8Printable(QStringLiteral("%1 / %2: status font reference/info center %3 / %4")
                                                                                     .arg(QString(locale.code), messages[index])
                                                                                     .arg(detailReference.center().y())
                                                                                     .arg(detailIconInk.center().y())));
      if (messages[index] != uwf::ui::I18n::tr("Ready")) {
        if (infoReference.isEmpty()) infoReference = detailIconInk;
        QVERIFY2(physicallyAligned(infoReference, detailIconInk), qUtf8Printable(QStringLiteral("%1 / %2: info icon moved from %3 to %4")
                                                                                     .arg(QString(locale.code), messages[index])
                                                                                     .arg(infoReference.center().y())
                                                                                     .arg(detailIconInk.center().y())));
      }
      if (pendingIcon->isVisibleTo(status)) {
        const auto dotInk = iconInkInFrame(pendingIcon, uwf::ui::design::Color::Warning, frame);
        const auto pendingReference = pendingLabel->alignmentBounds(true).translated(pendingLabel->mapTo(status, QPoint()));
        QVERIFY2(physicallyAligned(dotInk, pendingReference), qUtf8Printable(QStringLiteral("%1: pending dot/font reference center %2 / %3")
                                                                                 .arg(QString(locale.code))
                                                                                 .arg(dotInk.center().y())
                                                                                 .arg(pendingReference.center().y())));
      }
      QCOMPARE(label->font(), font);
      QCOMPARE(label->mapTo(status, QPoint()).y(), origin);
      QCOMPARE(label->height(), height);
      QVERIFY(!label->text().contains(u'\n'));
      QVERIFY(uwf::test::centersAligned(label->firstLineBoxBounds().translated(label->mapTo(status, QPoint())), status->rect()));
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
      const auto markerBounds = uwf::test::logicalBounds(marker, scale);
      const auto referenceBounds = uwf::test::logicalBounds(reference, scale);
      QVERIFY(physicallyAligned(markerBounds, referenceBounds));
      QVERIFY(std::abs(markerBounds.height() - referenceBounds.height()) <= referenceBounds.height() * uwf::test::kVisualAlignmentRatio);
      const auto markerFrame = status->grab().toImage();
      const auto iconInk = iconInkInFrame(icon, uwf::ui::design::Color::Muted, markerFrame);
      QVERIFY(physicallyAligned(iconInk, label->alignmentBounds(true).translated(label->mapTo(status, QPoint()))));
      if (infoReference.isEmpty()) infoReference = iconInk;
      QVERIFY2(physicallyAligned(infoReference, iconInk), qUtf8Printable(QStringLiteral("%1 / H %2: info icon moved from %3 to %4")
                                                                             .arg(QString(locale.code), messages[index])
                                                                             .arg(infoReference.center().y())
                                                                             .arg(iconInk.center().y())));
      transient.restoreAfter(0);
      QCOMPARE(status->detail(), messages[index]);
    }
  }
}

void WidgetShellTests::statusSurfaceAlignsSelectionDetail_data() {
  QTest::addColumn<bool>("dark");
  QTest::addColumn<uwf::ui::I18n::Lang>("language");
  for (const bool dark : {false, true})
    for (const auto& language : uwf::ui::I18n::languages())
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(static_cast<int>(language.id)).arg(dark ? "dark" : "light"))) << dark << language.id;
}

void WidgetShellTests::statusSurfaceAlignsSelectionDetail() {
  QFETCH(bool, dark);
  QFETCH(uwf::ui::I18n::Lang, language);
  auto& theme = uwf::ui::ThemeManager::instance();
  auto& i18n = uwf::ui::I18n::instance();
  const auto originalLanguage = i18n.lang();
  const auto originalTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    i18n.setLang(originalLanguage);
    theme.apply(originalTheme);
  });
  i18n.setLang(language);
  theme.apply(dark ? uwf::app::Theme::Dark : uwf::app::Theme::Light);
  uwf::ui::StatusSurface status;
  status.resize(1100, status.height());
  status.setDetail(uwf::ui::I18n::tr("Folder: %1").arg(QStringLiteral("C:\\Program Files (x86)\\Microsoft\\Edge")) + u' ' +
                   uwf::ui::I18n::tr("Current session: %1\nNext session: %2").arg(uwf::ui::I18n::tr("Excluded"), uwf::ui::I18n::tr("Excluded")));
  status.show();
  QVERIFY(QTest::qWaitForWindowExposed(&status));
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  auto* label = status.findChild<uwf::ui::TextLabel*>(QStringLiteral("statusBarLabel"));
  auto* icon = status.findChild<QWidget*>(QStringLiteral("statusBarIcon"));
  auto* copy = status.findChild<QPushButton*>(QStringLiteral("statusCopyButton"));
  QVERIFY(label && icon && copy);
  const auto scale = status.devicePixelRatioF();
  const auto frame = status.grab().toImage();
  const auto& system = uwf::ui::DesignSystem::instance();
  const auto background = system.color(uwf::ui::design::Color::Footer);
  const auto ink = [&](QWidget* widget, const uwf::ui::design::Color role) {
    return uwf::test::visibleInkBounds(frame, system.color(role), background, QRectF(QRect(widget->mapTo(&status, QPoint()), widget->size())));
  };
  const auto textReference = label->alignmentBounds(true).translated(label->mapTo(&status, QPoint()));
  const auto iconInk = ink(icon, uwf::ui::design::Color::Muted);
  const auto copyRegion = QRect(copy->mapTo(&status, QPoint()), copy->size());
  const auto copyRuns = uwf::test::horizontalInkRuns(
      frame, system.color(uwf::ui::design::Color::Accent), background,
      QRect(qFloor(copyRegion.x() * scale), qFloor(copyRegion.y() * scale), qCeil(copyRegion.width() * scale), qCeil(copyRegion.height() * scale)),
      qCeil(4 * scale));
  QVERIFY(copyRuns.size() >= 2);
  QRect words;
  const int iconIndex = status.layoutDirection() == Qt::RightToLeft ? static_cast<int>(copyRuns.size()) - 1 : 0;
  const auto copyIcon = uwf::test::logicalBounds(copyRuns[iconIndex], scale);
  for (int index = 0; index < copyRuns.size(); ++index)
    if (index != iconIndex) words = words.united(copyRuns[index]);
  const auto copyText = uwf::test::logicalBounds(words, scale);
  // The copy button shares the fixed font baseline; its icon must not follow
  // the localized word's ascenders or descenders. Derive this reference from
  // the actual control position and primary font, independently of TextLabel.
  const QFontMetricsF metrics(copy->font());
  const qreal baseline = qRound((copyRegion.y() + copy->height() / 2.0 + (metrics.ascent() - metrics.descent()) / 2.0) * scale) / scale;
  const QRectF copyReference(copyRegion.x(), baseline - metrics.capHeight(), copyRegion.width(), metrics.capHeight());
  QVERIFY(uwf::test::saveScreenshot(status, QStringLiteral("selection-detail")));
  const auto aligned = [&](const QRectF& first, const QRectF& second) {
    return !first.isEmpty() && !second.isEmpty() && std::abs(first.center().y() - second.center().y()) * scale <= 1.0;
  };
  QVERIFY2(aligned(textReference, iconInk),
           qUtf8Printable(QStringLiteral("body font reference/info: %1 / %2").arg(textReference.center().y()).arg(iconInk.center().y())));
  QVERIFY(!copyText.isEmpty());
  QVERIFY2(aligned(copyReference, copyIcon),
           qUtf8Printable(QStringLiteral("copy font reference/icon: %1 / %2").arg(copyReference.center().y()).arg(copyIcon.center().y())));
  QVERIFY(status.rect().contains(copyRegion));
  QVERIFY(copy->parentWidget()->rect().contains(copy->geometry()));
  QVERIFY(icon->parentWidget()->rect().contains(icon->geometry()));

  // Different words have different descenders. Probe the shared baseline with
  // the same actual glyph after checking the complete, localized selection.
  label->setText(QStringLiteral("H"));
  copy->setText(QStringLiteral("H"));
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  const auto baselineFrame = status.grab().toImage();
  const auto bodyMarker = uwf::test::visibleInkBounds(baselineFrame, system.color(uwf::ui::design::Color::Muted), background,
                                                      label->firstLineBoxBounds().translated(label->mapTo(&status, QPoint())));
  const auto markerRegion = QRect(copy->mapTo(&status, QPoint()), copy->size());
  const auto markerRuns = uwf::test::horizontalInkRuns(
      baselineFrame, system.color(uwf::ui::design::Color::Accent), background,
      QRect(qFloor(markerRegion.x() * scale), qFloor(markerRegion.y() * scale), qCeil(markerRegion.width() * scale), qCeil(markerRegion.height() * scale)),
      qCeil(4 * scale));
  QCOMPARE(markerRuns.size(), 2);
  const auto copyMarker = uwf::test::logicalBounds(markerRuns[status.layoutDirection() == Qt::RightToLeft ? 0 : 1], scale);
  QVERIFY2(aligned(bodyMarker, copyMarker),
           qUtf8Printable(QStringLiteral("body/copy baseline: %1 / %2").arg(bodyMarker.center().y()).arg(copyMarker.center().y())));
}

void WidgetShellTests::titleBarAlignsApplicationNameWithIcon_data() {
  QTest::addColumn<uwf::ui::I18n::Lang>("language");
  for (const auto language : {uwf::ui::I18n::Lang::En, uwf::ui::I18n::Lang::Zh_CN, uwf::ui::I18n::Lang::Ar})
    QTest::newRow(qUtf8Printable(QString::number(static_cast<int>(language)))) << language;
}

void WidgetShellTests::titleBarAlignsApplicationNameWithIcon() {
  QFETCH(uwf::ui::I18n::Lang, language);
  const auto previous = uwf::ui::I18n::instance().lang();
  const auto restore = qScopeGuard([&] { uwf::ui::I18n::instance().setLang(previous); });
  uwf::ui::I18n::instance().setLang(language);
  QWidget host;
  uwf::ui::WindowTitleBar title(&host);
  host.resize(1100, title.height());
  title.resize(1100, title.height());
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  title.ensurePolished();
  title.layout()->activate();
  auto* icon = title.findChild<QWidget*>(QStringLiteral("mainApplicationIcon"));
  auto* name = title.findChild<uwf::ui::TextLabel*>(QStringLiteral("mainApplicationName"));
  QVERIFY(icon && name);
  const auto scale = title.devicePixelRatioF();
  const auto iconInk = uwf::test::renderedInkBounds(*icon, scale).translated(icon->mapTo(&title, QPoint()));
  const auto nameInk = uwf::test::renderedInkBounds(*name, scale).translated(name->mapTo(&title, QPoint()));
  QVERIFY2(!nameInk.isEmpty() && !iconInk.isEmpty() && qRound(std::abs(nameInk.center().y() - iconInk.center().y()) * scale * 2) <= 2,
           qUtf8Printable(QStringLiteral("%1: centers %2 / %3").arg(name->text()).arg(nameInk.center().y()).arg(iconInk.center().y())));
  QVERIFY(uwf::test::saveScreenshot(title, QStringLiteral("title-bar-inline")));
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
  for (auto buttons = window.findChildren<QAbstractButton*>(); auto* button : std::as_const(buttons))
    if (button->property("nativeCaptionHit").toInt() == HTMAXBUTTON) maximize = button;
  QVERIFY(maximize);
  QCOMPARE(maximize->toolTip(), uwf::ui::I18n::tr("Maximize"));
  const auto clickCaption = [&](const int hit, const bool cancel = false) {
    QAbstractButton* button = nullptr;
    for (auto buttons = window.findChildren<QAbstractButton*>(); auto* candidate : std::as_const(buttons))
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
  // More workspace must not shrink the default window; translating the desktop must preserve its relative geometry.
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
