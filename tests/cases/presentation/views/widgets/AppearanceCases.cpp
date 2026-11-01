// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <dwmapi.h>
#include <qt_windows.h>

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPoint>
#include <QPointer>
#include <QPushButton>
#include <QRegion>
#include <QScopeGuard>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSize>
#include <QSpinBox>
#include <QString>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QTest>
#include <QTextBlock>
#include <QTextFragment>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>

#include "cases/presentation/views/widgets/WidgetUiFixture.h"
#include "presentation/appearance/AppearanceTransition.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/appearance/WindowAppearance.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/inputs/CodeEditor.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/layout/VerticalScrollArea.h"
#include "presentation/controls/navigation/CommandBar.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogLayout.h"
#include "presentation/dialogs/PanelDialog.h"
#include "presentation/views/shell/WindowChromeController.h"
#include "presentation/views/shell/WindowTitleBar.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

using namespace uwf::test::widgets;
namespace {
class WidgetAppearanceTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() { initializeWidgets(); }
  void systemBackdropStartsSolidUntilConfirmed_data();
  void systemBackdropStartsSolidUntilConfirmed();
  void systemBackdropKeepsNativeFrameAndFallsBackToSolid();
  void systemBackdropRejectsMismatchedReadback_data();
  void systemBackdropRejectsMismatchedReadback();
  void systemBackdropFollowsRepeatedThemeChanges();
  void systemBackdropRefreshDoesNotOutliveWindow();
  void lightUtilityHoverMatchesTheH5ColorAfterThemeChanges();
  void appearanceRequestsCoalesceAndPreserveEditing();
  void appearanceChangesRespectOwnerLifetimes();
  void themeIconsReuseParsedAssets();
  void themeRoundTripRefreshesLiveSurfacesEditorsAndLinks();
  void productionStylesKeepFullDialogScrollbarsAndTheirCorners();
  void dialogScrollbarTracksPreserveBackdropAlpha();
};

void WidgetAppearanceTests::systemBackdropStartsSolidUntilConfirmed_data() {
  QTest::addColumn<bool>("light");
  QTest::newRow("light") << true;
  QTest::newRow("dark") << false;
}

void WidgetAppearanceTests::systemBackdropStartsSolidUntilConfirmed() {
  using namespace uwf::ui;
  QFETCH(bool, light);
  auto& theme = ThemeManager::instance();
  const auto previous = theme.current();
  const auto restore = qScopeGuard([&] { theme.apply(previous); });
  theme.apply(light ? uwf::app::Theme::Light : uwf::app::Theme::Dark);
  QWidget host;
  prepareWindowBackdrop(&host);
  QVERIFY(!host.testAttribute(Qt::WA_WState_Created));
  QVERIFY(!host.property("systemBackdropActive").toBool());
  Surface surface(design::Color::Window, &host);
  surface.setBackdropOpacity(0);
  surface.resize(160, 100);
  QImage layer(surface.size(), QImage::Format_ARGB32_Premultiplied);
  layer.fill(Qt::transparent);
  surface.render(&layer);
  QVERIFY(uwf::test::colorCoverage(layer, DesignSystem::instance().color(design::Color::Window), uwf::test::interiorRegion(surface.rect())) >=
          uwf::test::kVisualMinimumCoverage);
}

void WidgetAppearanceTests::systemBackdropKeepsNativeFrameAndFallsBackToSolid() {
  using namespace uwf::ui;
  QMainWindow host;
  host.setWindowFlags(host.windowFlags() | Qt::FramelessWindowHint);
  WindowChromeController chrome(&host);
  prepareWindowBackdrop(&host);
  auto* surface = new Surface(design::Color::Window, &host);
  surface->setBackdropOpacity(0);
  host.setCentralWidget(surface);
  host.resize(500, 280);
  host.show();
  host.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&host));
  applyWindowAppearance(&host);
  const auto handle = reinterpret_cast<HWND>(host.winId());
  QVERIFY(!(GetWindowLongPtrW(handle, GWL_EXSTYLE) & WS_EX_LAYERED));
  const auto style = GetWindowLongPtrW(handle, GWL_STYLE);
  QVERIFY(style & WS_THICKFRAME);
  QVERIFY(style & WS_MAXIMIZEBOX);
  int material = 0;
  const bool supported = SUCCEEDED(DwmGetWindowAttribute(handle, 38, &material, sizeof(material)));
  if (host.property("systemBackdropActive").toBool()) {
    QVERIFY(supported);
    QCOMPARE(material, 2);
    QImage layer(surface->size(), QImage::Format_ARGB32_Premultiplied);
    layer.fill(Qt::transparent);
    surface->render(&layer, QPoint(), QRegion(), QWidget::DrawChildren);
    QVERIFY(uwf::test::colorCoverage(layer, Qt::transparent, uwf::test::interiorRegion(surface->rect())) >= uwf::test::kVisualMinimumCoverage);
  }
  const auto geometry = host.geometry();
  host.setProperty("systemBackdropForceSolid", true);
  refreshWindowBackdrop(&host);
  QVERIFY(!host.property("systemBackdropActive").toBool());
  QCOMPARE(host.geometry(), geometry);
  QVERIFY(uwf::test::colorCoverage(surface->grab().toImage(), DesignSystem::instance().color(design::Color::Window),
                                   uwf::test::interiorRegion(surface->rect())) >= uwf::test::kVisualMinimumCoverage);
  if (supported) {
    QVERIFY(SUCCEEDED(DwmGetWindowAttribute(handle, 38, &material, sizeof(material))));
    QCOMPARE(material, 1);
  }
  PanelDialog dialog(&host);
  auto* dialogLayout = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(420, 240));
  dialogLayout->body()->addWidget(new QLabel(QStringLiteral("Material dialog"), &dialog));
  dialog.show();
  dialog.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&dialog));
  refreshWindowBackdrop(&dialog);
  QVERIFY(dialog.property("systemBackdropRequested").toBool());
  QVERIFY(!(GetWindowLongPtrW(reinterpret_cast<HWND>(dialog.winId()), GWL_EXSTYLE) & WS_EX_LAYERED));
  dialog.setProperty("systemBackdropForceSolid", true);
  refreshWindowBackdrop(&dialog);
  QVERIFY(!dialog.property("systemBackdropActive").toBool());
  // The opt-in must never change HUB or other ordinary widget surfaces.
  QWidget plain;
  (void)plain.winId();
  applyWindowAppearance(&plain);
  QVERIFY(!plain.property("systemBackdropRequested").toBool());
}

void WidgetAppearanceTests::systemBackdropRejectsMismatchedReadback_data() {
  QTest::addColumn<bool>("light");
  QTest::newRow("light") << true;
  QTest::newRow("dark") << false;
}

void WidgetAppearanceTests::systemBackdropRejectsMismatchedReadback() {
  using namespace uwf::ui;
  QFETCH(bool, light);
  auto& theme = ThemeManager::instance();
  const auto previous = theme.current();
  const auto restore = qScopeGuard([&] { theme.apply(previous); });
  theme.apply(light ? uwf::app::Theme::Light : uwf::app::Theme::Dark);
  QMainWindow host;
  host.setWindowFlags(host.windowFlags() | Qt::FramelessWindowHint);
  prepareWindowBackdrop(&host);
  auto* surface = new Surface(design::Color::Window, &host);
  surface->setBackdropOpacity(0);
  host.setCentralWidget(surface);
  host.resize(500, 280);
  host.show();
  refreshWindowBackdrop(&host);
  if (!host.property("systemBackdropActive").toBool()) QSKIP("Native system backdrop is unavailable");

  // Change only the native material, leaving the application's cached request
  // intact. A refresh must verify the material before retaining transparency.
  const auto handle = reinterpret_cast<HWND>(host.winId());
  const int noBackdrop = 1;
  QVERIFY(SUCCEEDED(DwmSetWindowAttribute(handle, 38, &noBackdrop, sizeof(noBackdrop))));
  QCOMPARE(host.property("systemBackdropConfiguredType").toInt(), 2);
  refreshWindowBackdrop(&host);
  QVERIFY(!host.property("systemBackdropActive").toBool());
  QVERIFY(uwf::test::colorCoverage(surface->grab().toImage(), DesignSystem::instance().color(design::Color::Window),
                                   uwf::test::interiorRegion(surface->rect())) >= uwf::test::kVisualMinimumCoverage);
}

void WidgetAppearanceTests::systemBackdropFollowsRepeatedThemeChanges() {
  using namespace uwf::ui;
  auto& theme = ThemeManager::instance();
  const auto previous = theme.current();
  const auto restore = qScopeGuard([&] { theme.apply(previous); });
  QMainWindow host;
  host.setWindowFlags(host.windowFlags() | Qt::FramelessWindowHint);
  prepareWindowBackdrop(&host);
  WindowChromeController chrome(&host);
  connect(&theme, &ThemeManager::themeChanged, &host, [&] { chrome.applyTitleBarTheme(); });
  auto* surface = new Surface(design::Color::Window, &host);
  surface->setBackdropOpacity(0);
  host.setCentralWidget(surface);
  host.resize(500, 280);
  host.show();
  host.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&host));
  PanelDialog dialog(&host);
  auto* layout = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(420, 240));
  layout->body()->addWidget(new QLabel(QStringLiteral("Theme round trip"), &dialog));
  dialog.show();
  dialog.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&dialog));
  for (int cycle = 0; cycle < 3; ++cycle) {
    for (const auto target : {uwf::app::Theme::Dark, uwf::app::Theme::Light, uwf::app::Theme::Dark}) {
      theme.apply(target);
      QCoreApplication::processEvents();
      for (auto* window : {static_cast<QWidget*>(&host), static_cast<QWidget*>(&dialog)}) {
        const auto handle = reinterpret_cast<HWND>(window->winId());
        BOOL nativeDark = FALSE;
        if (FAILED(DwmGetWindowAttribute(handle, 20, &nativeDark, sizeof(nativeDark)))) QSKIP("Native dark-mode attributes are unavailable");
        QCOMPARE(nativeDark != FALSE, target == uwf::app::Theme::Dark);
        QVERIFY(!(GetWindowLongPtrW(handle, GWL_EXSTYLE) & WS_EX_LAYERED));
      }
      if (host.property("systemBackdropActive").toBool()) {
        QImage layer(surface->size(), QImage::Format_ARGB32_Premultiplied);
        layer.fill(Qt::transparent);
        surface->render(&layer, QPoint(), QRegion(), QWidget::DrawChildren);
        QVERIFY(uwf::test::colorCoverage(layer, Qt::transparent, uwf::test::interiorRegion(surface->rect())) >= uwf::test::kVisualMinimumCoverage);
      }
    }
  }
}

void WidgetAppearanceTests::systemBackdropRefreshDoesNotOutliveWindow() {
  using namespace uwf::ui;
  auto& theme = ThemeManager::instance();
  const auto previous = theme.current();
  const auto restore = qScopeGuard([&] { theme.apply(previous); });
  for (int cycle = 0; cycle < 8; ++cycle) {
    auto dialog = std::make_unique<PanelDialog>();
    auto* layout = new DialogLayout(dialog.get(), DialogContent::Scrollable, QSize(420, 240));
    layout->body()->addWidget(new QLabel(QStringLiteral("Closing during a theme change"), dialog.get()));
    dialog->show();
    QVERIFY(QTest::qWaitForWindowExposed(dialog.get()));
    theme.toggle();
    QEvent paletteChange(QEvent::ApplicationPaletteChange);
    QCoreApplication::sendEvent(dialog->windowHandle(), &paletteChange);
    const QPointer<QWidget> closed(dialog.get());
    dialog.reset();
    QCoreApplication::processEvents();
    QVERIFY(closed.isNull());
  }
}

void WidgetAppearanceTests::lightUtilityHoverMatchesTheH5ColorAfterThemeChanges() {
  const auto previous = uwf::ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] { uwf::ui::ThemeManager::instance().apply(previous); });
  QWidget host;
  host.setAutoFillBackground(true);
  auto* layout = new QVBoxLayout(&host);
  uwf::ui::WindowTitleBar title(&host);
  layout->addWidget(&title);
  uwf::ui::CommandBar commands(&host);
  auto* refresh = commands.buttonForAction(commands.addCommand(QStringLiteral("Refresh")));
  layout->addWidget(&commands);
  layout->addStretch();
  QAction action(QStringLiteral("Language"), &title);
  title.addUtility(&action);
  auto* button = title.findChild<QToolButton*>();
  QVERIFY(button && button->autoRaise());
  host.resize(700, 240);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  host.raise();
  host.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&host));
  for (const auto theme : {uwf::app::Theme::Light, uwf::app::Theme::Dark, uwf::app::Theme::Light}) {
    uwf::ui::ThemeManager::instance().apply(theme);
    for (auto* control : {button, refresh}) {
      uwf::test::moveMouse(host, QPoint(680, 220));
      QTRY_COMPARE(control->property("designHover").toReal(), 0.0);
      uwf::test::moveMouse(*control);
      QTRY_COMPARE(control->property("designHover").toReal(), 1.0);
      const QRectF fillRegion(control->width() * .1, control->height() * .25, control->width() * .15, control->height() * .5);
      QVERIFY(uwf::test::colorCoverage(control->grab().toImage(),
                                       QColor(theme == uwf::app::Theme::Light ? QStringLiteral("#dce3ec") : QStringLiteral("#333841")),
                                       fillRegion) >= uwf::test::kVisualMinimumCoverage);
      uwf::test::pressMouse(control, Qt::LeftButton);
      QTRY_COMPARE(control->property("designPress").toReal(), 1.0);
      QVERIFY(uwf::test::colorCoverage(control->grab().toImage(),
                                       QColor(theme == uwf::app::Theme::Light ? QStringLiteral("#cbd5e2") : QStringLiteral("#3c424c")),
                                       fillRegion) >= uwf::test::kVisualMinimumCoverage);
      uwf::test::releaseMouse(control, Qt::LeftButton);
      QTRY_COMPARE(control->property("designPress").toReal(), 0.0);
      uwf::test::moveMouse(host, QPoint(680, 220));
      QTRY_COMPARE(control->property("designHover").toReal(), 0.0);
      control->setEnabled(false);
      const auto disabled = control->grab().toImage();
      uwf::test::moveMouse(*control);
      QTest::qWait(160);
      QVERIFY(uwf::test::changedCoverage(disabled, control->grab().toImage(), fillRegion) <= uwf::test::kVisualAlignmentRatio);
      control->setEnabled(true);
    }
  }
}

void WidgetAppearanceTests::appearanceRequestsCoalesceAndPreserveEditing() {
  auto& theme = uwf::ui::ThemeManager::instance();
  auto& language = uwf::ui::I18n::instance();
  const auto oldTheme = theme.current();
  const auto oldLanguage = language.lang();
  const auto restore = qScopeGuard([&] {
    theme.apply(oldTheme);
    language.setLang(oldLanguage);
  });
  QWidget host;
  auto* layout = new QVBoxLayout(&host);
  auto* spin = new QSpinBox(&host);
  spin->setRange(0, 9999);
  spin->setKeyboardTracking(false);
  spin->setValue(50);
  auto* button = new QPushButton(QStringLiteral("Keep input responsive"), &host);
  auto* document = new QPlainTextEdit(QStringLiteral("Keep native editor colors in sync"), &host);
  layout->addWidget(spin);
  layout->addWidget(button);
  layout->addWidget(document);
  host.show();
  host.raise();
  host.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&host));
  auto* editor = spin->findChild<QLineEdit*>();
  QVERIFY(editor);
  editor->setFocus();
  editor->setText(QStringLiteral("1234"));
  editor->setSelection(1, 2);
  const int cursor = editor->cursorPosition();
  QSignalSpy themes(&theme, &uwf::ui::ThemeManager::themeChanged);
  const auto structure = qApp->styleSheet();
  QSignalSpy languages(&uwf::ui::I18n::events(), &uwf::ui::LanguageEvents::changed);
  const auto targetLanguage = oldLanguage == uwf::ui::I18n::Lang::En ? uwf::ui::I18n::Lang::Zh_CN : uwf::ui::I18n::Lang::En;
  QElapsedTimer elapsed;
  elapsed.start();
  theme.requestToggle();
  language.requestLang(uwf::ui::I18n::Lang::Ar);
  theme.requestToggle();
  language.requestLang(oldLanguage);
  theme.requestToggle();
  language.requestLang(targetLanguage);
  QTRY_COMPARE(languages.count(), 1);
  QTRY_COMPARE(themes.count(), 1);
  qInfo() << "Coalesced theme and language update:" << elapsed.elapsed() << "ms";
  QCOMPARE(language.lang(), targetLanguage);
  QCOMPARE(theme.current(), oldTheme == uwf::app::Theme::Dark ? uwf::app::Theme::Light : uwf::app::Theme::Dark);
  QCOMPARE(spin->value(), 50);
  QCOMPARE(editor->text(), QStringLiteral("1234"));
  QCOMPARE(editor->selectedText(), QStringLiteral("23"));
  QCOMPARE(editor->cursorPosition(), cursor);
  QCOMPARE(host.windowOpacity(), 1.0);
  QCOMPARE(qApp->styleSheet(), structure);
  QCOMPARE(document->palette().color(QPalette::Base), uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Surface));
  const auto documentImage = document->viewport()->grab().toImage();
  const QRectF documentBottom(0, document->viewport()->height() * .75, document->viewport()->width(), document->viewport()->height() * .2);
  QVERIFY(uwf::test::colorCoverage(documentImage, uwf::ui::DesignSystem::instance().color(uwf::ui::design::Color::Surface), documentBottom) >=
          uwf::test::kVisualMinimumCoverage);
  QVERIFY(host.updatesEnabled());
  const auto layers = host.findChildren<QWidget*>(QStringLiteral("appearanceTransition"));
  QVERIFY(layers.size() <= 1);
  for (auto* layer : layers) QVERIFY(layer->testAttribute(Qt::WA_TransparentForMouseEvents));
  QSignalSpy clicked(button, &QPushButton::clicked);
  QTest::mouseClick(host.windowHandle(), Qt::LeftButton, Qt::NoModifier, button->mapTo(&host, button->rect().center()));
  QCOMPARE(clicked.count(), 1);
  QTRY_VERIFY(host.findChildren<QWidget*>(QStringLiteral("appearanceTransition")).isEmpty());
  themes.clear();
  languages.clear();
  theme.requestToggle();
  theme.requestToggle();
  language.requestLang(language.lang());
  bool drained = false;
  QTimer::singleShot(0, &host, [&] { drained = true; });
  QTRY_VERIFY(drained);
  QCOMPARE(themes.count(), 0);
  QCOMPARE(languages.count(), 0);
  QVERIFY(host.findChildren<QWidget*>(QStringLiteral("appearanceTransition")).isEmpty());
}

void WidgetAppearanceTests::appearanceChangesRespectOwnerLifetimes() {
  bool invoked = false;
  auto owner = std::make_unique<QObject>();
  uwf::ui::AppearanceTransition::request(owner.get(), [&] { invoked = true; });
  owner.reset();
  auto window = std::make_unique<QWidget>();
  const QPointer<QWidget> observed = window.get();
  window->show();
  QVERIFY(QTest::qWaitForWindowExposed(window.get()));
  uwf::ui::AppearanceTransition::run([&] { window.reset(); });
  QVERIFY(observed.isNull());
  QApplication::processEvents();
  QVERIFY(!invoked);
}

void WidgetAppearanceTests::themeIconsReuseParsedAssets() {
  const auto light = uwf::ui::ThemeManager::iconWithColor(QStringLiteral(":/icons/language.svg"), QColor(Qt::black));
  const auto dark = uwf::ui::ThemeManager::iconWithColor(QStringLiteral(":/icons/language.svg"), QColor(Qt::white));
  QVERIFY(!light.isNull() && !dark.isNull());
  QVERIFY(light.cacheKey() != dark.cacheKey());
  QCOMPARE(uwf::ui::ThemeManager::iconWithColor(QStringLiteral(":/icons/language.svg"), QColor(Qt::black)).cacheKey(), light.cacheKey());
  QCOMPARE(uwf::ui::ThemeManager::iconWithColor(QStringLiteral(":/icons/language.svg"), QColor(Qt::white)).cacheKey(), dark.cacheKey());
  const QColor requested(QStringLiteral("#e93457"));
  for (const auto* resource : {":/icons/arrow.svg", ":/icons/arrow_up.svg", ":/icons/disk_off.svg", ":/icons/export.svg"}) {
    const auto pixels = uwf::ui::ThemeManager::iconWithColor(QString::fromLatin1(resource), requested).pixmap(QSize(32, 32)).toImage();
    QVERIFY2(uwf::test::colorCoverage(pixels, requested, QRectF(QPointF(), pixels.deviceIndependentSize())) > 0, resource);
  }
}

void WidgetAppearanceTests::themeRoundTripRefreshesLiveSurfacesEditorsAndLinks() {
  using namespace uwf::ui;
  QWidget host;
  auto* layout = new QVBoxLayout(&host);
  auto* surface = new Surface(design::Color::Surface, &host);
  surface->setClipContent(true);
  auto* contents = new QVBoxLayout(surface);
  auto* editor = new CodeEditor(surface);
  editor->setPlainText(QStringLiteral("Draft 中文\nsecond line"));
  auto* link = new ContentHintBox(surface);
  link->setHtml(QStringLiteral("<p>Released under <a href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU General Public License v3.0</a></p>"));
  contents->addWidget(editor);
  contents->addWidget(link);
  layout->addWidget(surface);
  host.resize(550, 350);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  for (const auto theme : {uwf::app::Theme::Dark, uwf::app::Theme::Light, uwf::app::Theme::Dark, uwf::app::Theme::Light, uwf::app::Theme::Dark}) {
    ThemeManager::instance().apply(theme);
    QCoreApplication::processEvents();
    auto& colors = DesignSystem::instance();
    QCOMPARE(editor->toPlainText(), QStringLiteral("Draft 中文\nsecond line"));
    QCOMPARE(editor->palette().color(QPalette::Text), colors.color(design::Color::Text));
    const auto frame = surface->grab().toImage();
    QVERIFY(uwf::test::colorCoverage(frame, colors.color(design::Color::Surface), uwf::test::interiorRegion(surface->rect())) >=
            uwf::test::kVisualMinimumCoverage);
    for (auto block = link->document()->firstBlock(); block.isValid(); block = block.next())
      for (auto it = block.begin(); !it.atEnd(); ++it)
        if (it.fragment().charFormat().isAnchor()) QCOMPARE(it.fragment().charFormat().foreground().color(), colors.color(design::Color::Accent));
  }
}

void WidgetAppearanceTests::productionStylesKeepFullDialogScrollbarsAndTheirCorners() {
  using namespace uwf::ui;
  Surface host(design::Color::Surface);
  auto* layout = new QVBoxLayout(&host);
  layout->setContentsMargins(1, 1, 1, 1);
  auto* editor = new CodeEditor(&host);
  editor->setReadOnly(true);
  editor->setLineNumbersVisible(false);
  editor->setPlainText(QStringLiteral("Raw report: ") + QString(180, u'X') + QStringLiteral("\nsecond line\n").repeated(40));
  layout->addWidget(editor);
  host.resize(420, 250);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  auto* vertical = editor->verticalScrollBar();
  auto* horizontal = editor->horizontalScrollBar();
  for (const auto theme : {uwf::app::Theme::Light, uwf::app::Theme::Dark, uwf::app::Theme::Light, uwf::app::Theme::Dark}) {
    ThemeManager::instance().apply(theme);
    QCoreApplication::processEvents();
    QCOMPARE(vertical->width(), 15);
    QCOMPARE(horizontal->height(), 15);
    const QColor track = DesignSystem::instance().color(design::Color::Surface);
    const QColor thumb = theme == uwf::app::Theme::Light ? QColor(139, 139, 139) : QColor(159, 159, 159);
    for (const bool end : {false, true, false}) {
      vertical->setValue(end ? vertical->maximum() : 0);
      horizontal->setValue(end ? horizontal->maximum() : 0);
      const auto image = host.grab().toImage();
      const auto v = vertical->mapTo(&host, QPoint());
      const auto h = horizontal->mapTo(&host, QPoint());
      const QRectF verticalTrack(v.x() + vertical->width() * .04, v.y() + vertical->height() * .15, vertical->width() * .1, vertical->height() * .7);
      const QRectF horizontalTrack(h.x() + horizontal->width() * .15, h.y() + horizontal->height() * .04, horizontal->width() * .7, horizontal->height() * .1);
      const auto corner = uwf::test::interiorRegion(QRectF(v.x(), h.y(), vertical->width(), horizontal->height()));
      QVERIFY(uwf::test::colorCoverage(image, track, verticalTrack) >= uwf::test::kVisualMinimumCoverage);
      QVERIFY(uwf::test::colorCoverage(image, track, horizontalTrack) >= uwf::test::kVisualMinimumCoverage);
      QVERIFY(uwf::test::colorCoverage(image, track, corner) >= uwf::test::kVisualMinimumCoverage);
      QStyleOptionSlider option;
      option.initFrom(vertical);
      option.rect = vertical->rect();
      option.orientation = vertical->orientation();
      option.minimum = vertical->minimum();
      option.maximum = vertical->maximum();
      option.pageStep = vertical->pageStep();
      option.sliderPosition = vertical->sliderPosition();
      option.sliderValue = vertical->value();
      option.upsideDown = vertical->invertedAppearance();
      const auto slider = vertical->style()->subControlRect(QStyle::CC_ScrollBar, &option, QStyle::SC_ScrollBarSlider, vertical);
      QVERIFY(!slider.isEmpty());
      const auto sliderRegion = uwf::test::interiorRegion(QRectF(slider.translated(v)));
      QVERIFY(uwf::test::colorCoverage(image, thumb, sliderRegion) > 0);
    }
  }
}

void WidgetAppearanceTests::dialogScrollbarTracksPreserveBackdropAlpha() {
  using namespace uwf::ui;
  Surface host(design::Color::Window);
  host.setAttribute(Qt::WA_TranslucentBackground);
  host.setProperty("systemBackdropActive", true);
  host.setBackdropOpacity(0);
  auto* layout = new QVBoxLayout(&host);
  layout->setContentsMargins(0, 0, 0, 0);
  auto* scroll = new VerticalScrollArea(&host);
  auto* content = new QWidget;
  content->setFixedHeight(1200);
  scroll->setWidget(content);
  content->setAutoFillBackground(false);
  layout->addWidget(scroll);
  host.resize(320, 240);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  auto* bar = scroll->verticalScrollBar();
  QVERIFY(bar->isVisible() && bar->maximum() > 0);
  QCOMPARE(bar->width(), 15);
  for (const auto theme : {uwf::app::Theme::Light, uwf::app::Theme::Dark, uwf::app::Theme::Light}) {
    ThemeManager::instance().apply(theme);
    for (const bool end : {false, true, false}) {
      for (int state = 0; state < 3; ++state) {
        bar->setValue(end ? bar->maximum() : 0);
        const QPoint button(bar->width() / 2, bar->height() - 9);
        if (state == 0)
          uwf::test::moveMouse(&host, QPoint(10, 10));
        else
          uwf::test::moveMouse(bar, button);
        if (state == 2) {
          uwf::test::pressMouse(bar, Qt::LeftButton, Qt::NoModifier, button);
          if (!end) QVERIFY(bar->value() > 0);
        }
        const auto release = qScopeGuard([&] {
          if (state == 2) uwf::test::releaseMouse(bar, Qt::LeftButton, Qt::NoModifier, button);
        });
        QCoreApplication::processEvents();
        const qreal scale = host.devicePixelRatioF();
        QImage image(host.size() * scale, QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(scale);
        image.fill(Qt::transparent);
        host.render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
        const auto origin = bar->mapTo(&host, QPoint());
        const QRectF track(origin.x() + bar->width() * .04, origin.y() + bar->height() * .15, bar->width() * .1, bar->height() * .7);
        const auto thumb = uwf::test::interiorRegion(QRectF(origin, bar->size()));
        QVERIFY(uwf::test::colorCoverage(image, Qt::transparent, track) >= uwf::test::kVisualMinimumCoverage);
        QVERIFY(uwf::test::colorCoverage(image, Qt::transparent, thumb) < 1.0);
        QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("transparent-scrollbar-%1-%2-%3").arg(static_cast<int>(theme)).arg(end).arg(state), state == 2));
      }
    }
  }
}

}  // namespace
QTEST_MAIN(WidgetAppearanceTests)
#include "AppearanceCases.moc"
