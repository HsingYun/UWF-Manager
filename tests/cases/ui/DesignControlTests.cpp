// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QDialog>
#include <QDir>
#include <QEnterEvent>
#include <QFontDatabase>
#include <QFocusFrame>
#include <QHeaderView>
#include <QImage>
#include <QInputMethodEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QListView>
#include <QPainter>
#include <QPainterPath>
#include <QProgressBar>
#include <QPushButton>
#include <QSaveFile>
#include <QScopeGuard>
#include <QScreen>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QStyleOptionSlider>
#include <QTableView>
#include <QTest>
#include <QTextBlock>
#include <QTextLayout>
#include <QToolButton>
#include <QVBoxLayout>

#include "support/ui/WidgetInput.h"
#include "support/visual/AnimationClock.h"
#include "support/visual/InkBounds.h"
#include "ui/design/DesignStyle.h"
#include "ui/design/DesignSystem.h"
#include "ui/design/IconPainting.h"
#include "ui/design/ItemHighlight.h"
#include "ui/design/PopupPlacement.h"
#include "ui/design/TextPainting.h"
#include "ui/widgets/CodeEditor.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/ContentState.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/NumberBox.h"
#include "ui/widgets/SelectBox.h"
#include "ui/widgets/SelectionListView.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/SwitchButton.h"
#include "ui/widgets/TextLabel.h"
namespace {
class DesignControlTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() {
    QApplication::setStyle(new uwf::ui::DesignStyle());
    QApplication::setFont(uwf::ui::DesignSystem::defaultFont());
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
      int ink = 0;
      for (int y = 0; y < pixels.height(); ++y)
        for (int x = 0; x < pixels.width(); ++x)
          if (pixels.pixelColor(x, y) == expected) ++ink;
      QVERIFY(ink > 5);
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
    const auto coloredPixels = [](const QImage& frame, const QColor& color) {
      int count = 0;
      for (int y = 0; y < frame.height(); ++y)
        for (int x = 0; x < frame.width(); ++x)
          if (frame.pixelColor(x, y) == color) ++count;
      return count;
    };
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
      // Compare rendered text directly, including its antialiased edge pixels.
      const QRect textArea(ordinary.width() / 2, 0, ordinary.width() - ordinary.width() / 2, ordinary.height());
      const auto ordinaryText = ordinary.copy(textArea);
      QVERIFY(ordinaryText != blank.copy(textArea));
      QVERIFY(coloredPixels(ordinary, design.color(Color::Text)) > 100);
      QCOMPARE(coloredPixels(ordinary, sourceColor), 0);
      command.setProperty("designIconColor", design.color(Color::Success));
      const auto status = command.grab().toImage();
      QVERIFY(coloredPixels(status, design.color(Color::Success)) > 100);
      QCOMPARE(status.copy(textArea), ordinaryText);
      QCOMPARE(coloredPixels(status, sourceColor), 0);
      for (const bool checked : {false, true, false}) {
        toggle.setChecked(checked);
        const auto frame = toggle.grab().toImage();
        QCOMPARE(frame.pixelColor(frame.width() / 2, frame.height() / 2), design.color(checked ? Color::Accent : Color::Text));
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
    for (const auto labels = state->findChildren<uwf::ui::TextLabel*>(); const auto* label : labels) {
      QVERIFY(!label->text().isEmpty());
      QTRY_VERIFY2(
          label->height() >= label->heightForWidth(label->width()),
          qUtf8Printable(
              QStringLiteral("%1: %2x%3 needs %4").arg(label->text()).arg(label->width()).arg(label->height()).arg(label->heightForWidth(label->width()))));
      QVERIFY(label->width() >= 250);
      QVERIFY(qAbs(QRectF(label->geometry()).center().x() - state->rect().width() / 2.0) <= 1);
    }
    capture(QStringLiteral("narrow"));
    host.resize(750, 430);
    for (const auto labels = state->findChildren<uwf::ui::TextLabel*>(); const auto* label : labels) {
      QTRY_VERIFY(label->height() >= label->heightForWidth(label->width()));
      QVERIFY(label->width() <= 420);
      QVERIFY(qAbs(QRectF(label->geometry()).center().x() - state->rect().width() / 2.0) <= 1);
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
    const auto scale = header.devicePixelRatio();
    for (const int y : {4, 33})
      for (const int x : {4, 159, 160, 164}) QCOMPARE(header.pixelColor(qRound(x * scale), qRound(y * scale)), design.color(Color::SurfaceSoft));
    const auto half = progress.grab().toImage();
    QCOMPARE(half.pixelColor(qRound(50 * scale), qRound(2 * scale)), design.color(Color::Accent));
    QCOMPARE(half.pixelColor(qRound(170 * scale), qRound(2 * scale)), design.color(Color::Line));
    progress.setRange(0, 0);
    static_cast<void>(progress.grab());
    animation.advanceBy(300);
    const auto early = progress.grab().toImage();
    animation.advanceBy(400);
    const auto later = progress.grab().toImage();
    QVERIFY(early != later);
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
    const auto dpr = halfway.devicePixelRatio();
    QCOMPARE(halfway.pixelColor(qRound(30 * dpr), qRound(2 * dpr)), uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Accent));
    QCOMPARE(halfway.pixelColor(qRound(130 * dpr), qRound(2 * dpr)), uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Line));
    animation.advanceBy(140);
    const auto complete = progress.grab().toImage();
    QCOMPARE(complete.pixelColor(qRound(130 * dpr), qRound(2 * dpr)), uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Accent));
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
    const qreal scale = frame.devicePixelRatio();
    QCOMPARE(frame.pixelColor(qRound(27 * scale), qRound(9 * scale)), theme.color(uwf::ui::design::Color::AccentText));
    QCOMPARE(frame.pixelColor(qRound(9 * scale), qRound(9 * scale)), theme.color(uwf::ui::design::Color::Accent));
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
    const auto off = center(QStringLiteral("00-off"));
    QVERIFY(qAbs(off.x() - 10) < .3);
    QVERIFY(qAbs(off.y() - 10) < .3);
    QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, control.mapTo(&window, control.rect().center()));
    QVERIFY(control.isChecked());
    uwf::test::moveMouse(window.windowHandle(), QPoint(190, 90));
    QTRY_VERIFY(!control.underMouse());
    clock.advanceBy(35);
    const auto movingOn = center(QStringLiteral("01-moving-on"));
    QVERIFY(movingOn.x() > 10.5 && movingOn.x() < 27.5);
    QVERIFY(qAbs(movingOn.y() - 10) < .3);
    clock.advanceBy(160);
    const auto on = center(QStringLiteral("02-on"));
    QVERIFY(qAbs(on.x() - 28) < .3);
    QVERIFY(qAbs(on.y() - 10) < .3);
    QTest::keyClick(&control, Qt::Key_Space);
    QVERIFY(!control.isChecked());
    clock.advanceBy(35);
    const auto movingOff = center(QStringLiteral("03-moving-off"));
    QVERIFY(movingOff.x() > 10.5 && movingOff.x() < 27.5);
    QVERIFY(qAbs(movingOff.y() - 10) < .3);
    clock.advanceBy(160);
    const auto offAgain = center(QStringLiteral("04-off-again"));
    QVERIFY(qAbs(offAgain.x() - 10) < .3);
    QTest::keyClick(&control, Qt::Key_Space);
    clock.advanceBy(35);
    const auto beforeReverse = center(QStringLiteral("05-reversing"));
    QTest::keyClick(&control, Qt::Key_Space);
    const auto reversed = center(QStringLiteral("06-reversed"));
    QVERIFY(qAbs(reversed.x() - beforeReverse.x()) < 1);
    clock.advanceBy(180);
    QVERIFY(qAbs(center(QStringLiteral("07-restored")).x() - 10) < .3);
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
      QTest::mouseClick(view.viewport(), Qt::LeftButton, modifiers, view.visualRect(model.index(row, 0)).center());
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
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, start);
    uwf::test::moveMouse(view.viewport(), end, 30);
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, end);
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
      const auto scale = image.devicePixelRatio();
      const QColor ink = image.pixelColor(qRound(12 * scale), qRound(12 * scale));
      timeline.append(QJsonObject{{"step", step}, {"elapsedMs", static_cast<double>(clock.totalTime())}, {"background", ink.name()}});
      if (!directory.isEmpty() && !image.save(output.filePath(QString::fromLatin1(QTest::currentDataTag()) +
                                                              QStringLiteral("/%1-%2.png").arg(frameNumber++, 3, 10, QLatin1Char('0')).arg(step))))
        QTest::qFail("Could not save the motion frame.", __FILE__, __LINE__);
      return ink;
    };
    uwf::test::moveMouse(window.windowHandle(), QPoint(240, 90));
    QTRY_VERIFY(!button.underMouse());
    clock.advanceBy(160);
    const auto resting = frame(QStringLiteral("rest"));
    const Color hover = role == QStringLiteral("primary")       ? Color::AccentHover
                        : role == QStringLiteral("destructive") ? Color::DangerFillHover
                        : role == QStringLiteral("close")       ? Color::CloseHover
                                                                : Color::ButtonHover;
    uwf::test::moveMouse(window.windowHandle(), button.mapTo(&window, button.rect().center()));
    QTRY_VERIFY(button.underMouse());
    clock.advanceBy(40);
    const auto entering = frame(QStringLiteral("hover-40ms"));
    clock.advanceBy(120);
    const auto hovered = frame(QStringLiteral("hover-settled"));
    QVERIFY(entering != resting);
    QVERIFY(entering != hovered);
    QCOMPARE(hovered, design.color(hover));
    const Color active = role == QStringLiteral("primary")       ? Color::AccentPressed
                         : role == QStringLiteral("destructive") ? Color::DangerFillPressed
                         : role == QStringLiteral("close")       ? Color::ClosePressed
                                                                 : Color::ButtonPressed;
    QTest::mousePress(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, button.mapTo(&window, button.rect().center()));
    QTRY_VERIFY(button.isDown());
    clock.advanceBy(40);
    const auto pressing = frame(QStringLiteral("press-40ms"));
    clock.advanceBy(120);
    const auto pressed = frame(QStringLiteral("press-settled"));
    QVERIFY(pressing != hovered);
    QVERIFY(pressing != pressed);
    QCOMPARE(pressed, design.color(active));
    uwf::test::moveMouse(window.windowHandle(), QPoint(240, 90));
    QTest::mouseRelease(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, QPoint(240, 90));
    QTRY_VERIFY(!button.underMouse());
    clock.advanceBy(160);
    QCOMPARE(frame(QStringLiteral("canceled-outside")), resting);
    QCOMPARE(clicks.count(), 0);
    QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, button.mapTo(&window, button.rect().center()));
    QCOMPARE(clicks.count(), 1);
    clock.advanceBy(160);
    frame(QStringLiteral("clicked"));
    button.setEnabled(false);
    clock.advanceBy(160);
    const auto disabled = frame(QStringLiteral("disabled"));
    uwf::test::moveMouse(window.windowHandle(), QPoint(240, 90));
    uwf::test::moveMouse(window.windowHandle(), button.mapTo(&window, button.rect().center()));
    QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, button.mapTo(&window, button.rect().center()));
    clock.advanceBy(160);
    QCOMPARE(frame(QStringLiteral("disabled-input")), disabled);
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
    uwf::test::moveMouse(window.windowHandle(), button.mapTo(&window, button.rect().center()));
    QTRY_VERIFY(button.underMouse());
    clock.advanceBy(80);
    const QPointF local(button.rect().center());
    QEnterEvent repeated(local, button.mapTo(&window, local.toPoint()), button.mapToGlobal(local.toPoint()));
    QApplication::sendEvent(&button, &repeated);
    clock.advanceBy(40);
    const auto frame = button.grab().toImage();
    QCOMPARE(frame.pixelColor(qRound(12 * frame.devicePixelRatio()), qRound(12 * frame.devicePixelRatio())), design.color(uwf::ui::design::Color::ButtonHover));
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
    QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, QPoint(230, 100));
    const auto outlinePixels = [&] {
      const auto image = window.grab().toImage().scaled(window.size());
      const auto accent = design.color(uwf::ui::design::Color::Accent);
      int pixels = 0;
      const auto bounds = control->geometry().adjusted(-6, -6, 6, 6);
      for (int y = bounds.top(); y <= bounds.bottom(); ++y)
        for (int x = bounds.left(); x <= bounds.right(); ++x)
          if (!control->geometry().contains(x, y) && image.pixelColor(x, y) == accent) ++pixels;
      return pixels;
    };
    control->setFocus(Qt::TabFocusReason);
    QTRY_VERIFY(control->hasFocus());
    QCOMPARE(outlinePixels(), 0);
    control->clearFocus();
    QTest::keyClick(&window, Qt::Key_Tab);
    control->setFocus(Qt::OtherFocusReason);
    QTRY_VERIFY(control->hasFocus());
    QTRY_VERIFY(outlinePixels() > 20);
    QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, QPoint(230, 100));
    QTRY_COMPARE(outlinePixels(), 0);
    control->clearFocus();
    control->setFocus(Qt::OtherFocusReason);
    QCOMPARE(outlinePixels(), 0);
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
    QTest::mouseClick(editor, Qt::LeftButton);
    QTRY_VERIFY(editor->hasFocus());
    const auto hasRing = [&] {
      for (const auto frames = host.findChildren<QFocusFrame*>(); auto* frame : frames)
        if (frame->isVisible() && frame->widget() == field) return true;
      return false;
    };
    QVERIFY(!hasRing());
    const QString pasted = QStringLiteral("42");
    QApplication::clipboard()->setText(pasted);
    editor->selectAll();
    // Deliver through QWindow, as real key input is seen there before the
    // focused editor receives it. This also covers embedded line edits.
    QTest::keyClick(host.windowHandle(), Qt::Key_V, Qt::ControlModifier);
    QCOMPARE(editor->text(), pasted);
    QVERIFY(!hasRing());
    for (const auto& [key, modifiers] : {
             std::pair{Qt::Key_Left, Qt::NoModifier}, std::pair{Qt::Key_Right, Qt::NoModifier},
             std::pair{Qt::Key_Home, Qt::ControlModifier}, std::pair{Qt::Key_End, Qt::ControlModifier},
             std::pair{Qt::Key_A, Qt::ControlModifier}, std::pair{Qt::Key_C, Qt::ControlModifier},
             std::pair{Qt::Key_X, Qt::ControlModifier}, std::pair{Qt::Key_Z, Qt::ControlModifier},
             std::pair{Qt::Key_Y, Qt::ControlModifier}, std::pair{Qt::Key_Backspace, Qt::NoModifier},
             std::pair{Qt::Key_Delete, Qt::NoModifier}, std::pair{Qt::Key_7, Qt::NoModifier}}) {
      QTest::keyClick(host.windowHandle(), key, modifiers);
      QVERIFY(!hasRing());
    }
    QApplication::clipboard()->setText(pasted);
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
    QTest::mouseClick(backward ? &after : &before, Qt::LeftButton);
    QTest::keyClick(host.windowHandle(), Qt::Key_Tab, backward ? Qt::ShiftModifier : Qt::NoModifier);
    QTRY_VERIFY(editor.hasFocus());
    const auto hasRing = [&] {
      for (const auto frames = host.findChildren<QFocusFrame*>(); auto* frame : frames)
        if (frame->isVisible() && frame->widget() == &editor) return true;
      return false;
    };
    QTRY_VERIFY(hasRing());
    const QString pasted = QStringLiteral("Pasted text");
    QApplication::clipboard()->setText(pasted);
    QTest::keyClick(host.windowHandle(), Qt::Key_V, Qt::ControlModifier);
    QCOMPARE(editor.text(), pasted);
    QVERIFY(hasRing());
    QTest::keyClick(host.windowHandle(), Qt::Key_Left);
    QVERIFY(hasRing());
    QTest::mouseClick(&editor, Qt::LeftButton);
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
    QTest::mouseClick(list.viewport(), Qt::LeftButton, Qt::NoModifier, list.visualRect(model.index(0, 0)).center());
    QTest::keyClick(host.windowHandle(), Qt::Key_Down);
    QCOMPARE(list.currentIndex().row(), 1);
    button.setFocus(Qt::OtherFocusReason);
    QTRY_VERIFY(button.hasFocus());
    const auto hasRing = [&] {
      for (const auto frames = host.findChildren<QFocusFrame*>(); auto* frame : frames)
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
    // The bottom of the option viewport must still hit a real row, not a blank
    // area introduced by QListView's preferred minimum size.
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
    QTest::mouseClick(options->viewport(), Qt::LeftButton, Qt::NoModifier, firstRow.center());
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
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-%3").arg(hover ? "hover-then-enter" : "keyboard", rtl ? "rtl" : "ltr").arg(width))) << hover << rtl << width;
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
    const auto pixel = [&] {
      const auto image = control.grab().toImage();
      return image.pixelColor(qRound(7 * image.devicePixelRatio()), qRound(10 * image.devicePixelRatio()));
    };
    for (int attempt = 0; attempt < 8; ++attempt) {
      const QPoint arrow = control.mapTo(&window, QPoint(control.width() - 12, control.height() / 2));
      QCursor::setPos(window.mapToGlobal(arrow));
      QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, arrow);
      auto* popup = QApplication::activePopupWidget();
      QVERIFY(popup);
      auto* options = popup->findChild<QListView*>(QStringLiteral("selectOptions"));
      QVERIFY(options);
      const int next = 1 - control.currentIndex();
      const QPoint row = options->visualRect(options->model()->index(next, 0)).center();
      QCursor::setPos(options->viewport()->mapToGlobal(row));
      const QPoint inPopup = popup->mapFromGlobal(options->viewport()->mapToGlobal(row));
      QTest::mouseClick(popup->windowHandle(), Qt::LeftButton, Qt::NoModifier, inPopup);
      QTRY_VERIFY(!popup->isVisible());
      QCOMPARE(control.currentIndex(), next);
      QCursor::setPos(window.mapToGlobal(QPoint(340, 220)));
      uwf::test::moveMouse(window.windowHandle(), QPoint(340, 220));
      QTRY_COMPARE(pixel(), design.color(uwf::ui::design::Color::Surface));
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
    const auto selectImage = popup->grab().toImage();
    QCOMPARE(selectImage.pixelColor(0, 0).alpha(), 0);
    select.hidePopup();
    uwf::ui::CommandMenu menu(&window);
    auto* first = menu.addAction(QStringLiteral("First"));
    menu.addAction(QStringLiteral("Unavailable"))->setEnabled(false);
    auto* second = menu.addAction(QStringLiteral("Second"));
    QSignalSpy firstCalls(first, &QAction::triggered), secondCalls(second, &QAction::triggered);
    menu.popup(window.mapToGlobal(QPoint(20, 60)));
    QTRY_COMPARE(QApplication::activePopupWidget(), &menu);
    const auto menuImage = menu.grab().toImage();
    QCOMPARE(menuImage.pixelColor(0, 0).alpha(), 0);
    for (int y = 0; y < 5; ++y)
      for (int x = 0; x < 5; ++x) QCOMPARE(menuImage.pixelColor(x, y), selectImage.pixelColor(x, y));
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
    const auto scale = image.devicePixelRatio();
    const auto ink = uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Text);
    QPointF total;
    int count = 0;
    for (int y = 0; y < image.height(); ++y)
      for (int x = 0; x < image.width(); ++x)
        if (image.pixelColor(x, y) == ink) {
          total += QPointF(x + .5, y + .5);
          ++count;
        }
    QVERIFY(count > 50);
    const auto center = total / count;
    QVERIFY(qAbs(center.x() - size.width() * scale / 2.0) <= .5);
    QVERIFY(qAbs(center.y() - size.height() * scale / 2.0) <= .5);
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
    QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, command.geometry().center());
    QTRY_VERIFY(menu.isVisible());
    QCOMPARE(menu.pos().y() - command.mapToGlobal(QPoint(0, command.height())).y(), 6);
    QCOMPARE(menu.pos().x(), command.mapToGlobal(QPoint()).x());
    menu.hide();
    QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, select.geometry().center());
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
    QApplication::clipboard()->setText(QStringLiteral("<b>plain</b>\nsecond"));
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
        const qreal scale = image.devicePixelRatio();
        const auto pixel = [&](QPoint point) { return image.pixelColor(qRound(point.x() * scale), qRound(point.y() * scale)); };
        const auto v = vertical->mapTo(&host, QPoint());
        const auto h = horizontal->mapTo(&host, QPoint());
        // The side gutters reveal the editor's surface after either thumb moves.
        for (int y = 2; y < vertical->height() - 2; ++y) QCOMPARE(pixel(v + QPoint(1, y)), track);
        for (int x = 2; x < horizontal->width() - 2; ++x) QCOMPARE(pixel(h + QPoint(x, 1)), track);
        // The intersection reveals the same surface, with no native grip or stale lines.
        for (int y = 1; y < 14; ++y)
          for (int x = 1; x < 14; ++x) QCOMPARE(pixel(QPoint(v.x() + x, h.y() + y)), track);
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
    const qreal iconCenter = (runs.front().top() + runs.front().bottom() + 1) / 2.0;
    const qreal textCenter = (textBounds.top() + textBounds.bottom() + 1) / 2.0;
    const QString directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT");
    if (!directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/inline-center"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(frame.save(output.filePath(QString::fromUtf8(QTest::currentDataTag()) + QStringLiteral(".png"))));
    }
    QVERIFY2(qAbs(iconCenter - button.height() * scale / 2.0) <= 1, qUtf8Printable(QString::number(iconCenter)));
    QVERIFY2(qAbs(textCenter - iconCenter) <= 1, qUtf8Printable(QStringLiteral("text=%1 icon=%2").arg(textCenter).arg(iconCenter)));
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
      QCOMPARE(marker, reference);
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
    QCOMPARE(render(local), render(expected));
    local.setFamilies({QStringLiteral("Consolas")});
    expected.setFamilies({QStringLiteral("Consolas")});
    QCOMPARE(render(local), render(expected));
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
    QImage reference;
    for (int row = 0; row < 6; ++row) {
      QRect ink;
      for (int y = qCeil(row * rowHeight * scale); y < qFloor((row + 1) * rowHeight * scale); ++y)
        for (int x = 0; x < frame.width(); ++x)
          if (qGray(frame.pixel(x, y)) < 250) ink = ink.united(QRect(x, y, 1, 1));
      QVERIFY(!ink.isEmpty());
      const auto glyphs = frame.copy(ink);
      if (row == 0)
        reference = glyphs;
      else
        QCOMPARE(glyphs, reference);
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
    const qreal center = (runs.front().top() + runs.front().bottom() + 1) / 2.0;
    QVERIFY(qAbs(center - height * scale / 2) <= 1);
    bool antialiased = false;
    for (int y = 0; y < frame.height(); ++y)
      for (int x = 0; x < frame.width(); ++x) {
        const int value = frame.pixelColor(x, y).red();
        antialiased = antialiased || (value > 0 && value < 255);
      }
    QVERIFY(antialiased);
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
    surface.setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
    surface.setClipContent(true);
    auto* child = new uwf::ui::Surface(Color::Accent, &surface);
    child->setGeometry(surface.rect());
    surface.show();
    QVERIFY(QTest::qWaitForWindowExposed(&surface));
    const auto frame = surface.grab().toImage();
    QCOMPARE(frame.pixelColor(0, 0).alpha(), 0);
    QCOMPARE(frame.pixelColor(frame.width() - 1, 0).alpha(), 0);
    QCOMPARE(frame.pixelColor(0, frame.height() - 1).alpha(), 0);
    QCOMPARE(frame.pixelColor(frame.width() / 2, frame.height() / 2), design.color(Color::Accent));
    QCOMPARE(frame.pixelColor(0, frame.height() / 2), design.color(Color::Line));
    bool antialiased = false;
    for (int y = 0; y < frame.height(); ++y)
      for (int x = 0; x < frame.width(); ++x) {
        const auto pixel = frame.pixelColor(x, y);
        if (pixel.alpha() < 64 || pixel.alpha() > 192) continue;
        antialiased = true;
        const auto line = design.color(Color::Line);
        QVERIFY(qAbs(pixel.red() - line.red()) < 5);
        QVERIFY(qAbs(pixel.green() - line.green()) < 5);
        QVERIFY(qAbs(pixel.blue() - line.blue()) < 5);
      }
    QVERIFY(antialiased);
    if (const auto directory = qEnvironmentVariable("UWF_UI_MOTION_ROOT"); !directory.isEmpty()) {
      QDir output(directory + QStringLiteral("/surface-clip"));
      QVERIFY(output.mkpath(QStringLiteral(".")));
      QVERIFY(frame.save(output.filePath(light ? QStringLiteral("light.png") : QStringLiteral("dark.png"))));
    }
    child->setFill(Color::Success);
    const auto updated = surface.grab().toImage();
    QCOMPARE(updated.pixelColor(updated.width() / 2, updated.height() / 2), design.color(Color::Success));
  }
};
}  // namespace
QTEST_MAIN(DesignControlTests)
#include "DesignControlTests.moc"
