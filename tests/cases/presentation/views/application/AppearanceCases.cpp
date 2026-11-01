// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <qt_windows.h>

#include <QAbstractScrollArea>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFontInfo>
#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QScopeGuard>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSpinBox>
#include <QStyle>
#include <QStyleOptionButton>
#include <QTableView>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "application/commit/contracts/CommitResult.h"
#include "application/enhanced/EnhancedModeManager.h"
#include "application/registry/contracts/RegistryBrowser.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "application/uwf/contracts/UwfCapability.h"
#include "cases/presentation/views/application/ApplicationUiFixture.h"
#include "cases/presentation/views/application/ApplicationUiTest.h"
#include "composition/desktop/MainWindowComposition.h"
#include "composition/desktop/ViewModelComposition.h"
#include "composition/desktop/WorkflowComposition.h"
#include "domain/uwf/model/UwfModel.h"
#include "foundation/logging/Log.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "infrastructure/windows/diagnostics/SystemHardwareInfo.h"
#include "infrastructure/windows/filesystem/text/BoundedTextFileReader.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/inputs/ComboBoxSelection.h"
#include "presentation/controls/inputs/SwitchButton.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogWindow.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/dialogs/PanelDialog.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/viewmodels/files/FilePickerViewModel.h"
#include "presentation/views/commit/CommitReportDialog.h"
#include "presentation/views/configuration/global/GlobalStatusPanel.h"
#include "presentation/views/configuration/importing/ImportDialog.h"
#include "presentation/views/configuration/review/ApplyPlanDialog.h"
#include "presentation/views/diagnostics/AboutDialog.h"
#include "presentation/views/diagnostics/LogViewerDialog.h"
#include "presentation/views/diagnostics/SystemInformationDialog.h"
#include "presentation/views/enhanced/EnhancedModeDialog.h"
#include "presentation/views/files/FilePickerDialog.h"
#include "presentation/views/overlay/coordination/OverlayContextMenu.h"
#include "presentation/views/overlay/files/OverlayFilesDialog.h"
#include "presentation/views/overlay/hud/OverlayHudPalette.h"
#include "presentation/views/overlay/usage/OverlayUsageBar.h"
#include "presentation/views/power/PowerActionDialog.h"
#include "presentation/views/registry/RegistryPickerDialog.h"
#include "presentation/views/shell/MainWindow.h"
#include "support/app/ConfigurationFixture.h"
#include "support/ui/DesktopPlatformFixture.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/OverlayFilesFixture.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/ViewMetrics.h"
#include "support/ui/WidgetInput.h"
#include "support/uwf/FakeUwfTrace.h"
#include "support/visual/InlineIconTextProbe.h"
#include "support/visual/VisualGeometry.h"
#include "support/visual/WidgetCapture.h"

class ApplicationAppearanceTests final : public ApplicationUiTest {
  Q_OBJECT
 private slots:
  void languageAndThemeChangesReachProductionWidgets();
  void themeRoundTripPreservesMainWindowDraftAndWidgets();
  void qtStandardWidgetsUseSupplementalTranslations_data();
  void qtStandardWidgetsUseSupplementalTranslations();
  void buttonContrastAndSwitchFocusStayVisible_data();
  void buttonContrastAndSwitchFocusStayVisible();
  void commonDialogsPreserveSafeDefaults();
  void hintAndUsageWidgetsHandleEmptyOverflowAndThresholdEdges();
  void comboSelectionUsesStableItemData();
  void dialogLayoutsKeepActionsVisible_data();
  void dialogLayoutsKeepActionsVisible();
  void longMessageDialogsKeepCompleteTextAndReachableActions_data();
  void longMessageDialogsKeepCompleteTextAndReachableActions();
  void textRenderingProducesValidGlyphsAndSmoothEdges();
};

using namespace uwf;
using namespace uwf::test::application;

void ApplicationAppearanceTests::themeRoundTripPreservesMainWindowDraftAndWidgets() {
  ui::ThemeManager::instance().apply(ui::ThemeManager::instance().current());
  const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  ui::MainWindow window(uwf::composition::mainWindowServices(system, source, stagingOwner, uwf::test::localDesktopPorts()),
                        {.uwfCapability = app::UwfCapability::Available, .compatibilityMode = false, .hubVisible = false});
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  window.raise();
  window.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&window));
  QTRY_COMPARE(source.reads, 1);

  QTRY_VERIFY(refreshCompleted(window));
  const QPointer<ui::GlobalStatusPanel> global = window.findChild<ui::GlobalStatusPanel*>();
  QVERIFY(global);
  QCOMPARE((global->viewModel().importFilterEnabled(false)).outcome, uwf::app::GlobalEditOutcome::Applied);
  auto* theme = actionWithText(&window, ui::I18n::tr("Theme"));
  QVERIFY(theme);
  const auto originalLanguage = ui::I18n::instance().lang();
  const auto restore = qScopeGuard([previous = ui::ThemeManager::instance().current(), originalLanguage] {
    ui::ThemeManager::instance().apply(previous);
    ui::I18n::instance().setLang(originalLanguage);
  });
  QVERIFY(test::saveScreenshot(window, QStringLiteral("theme-before")));
  for (int round = 0; round < 6; ++round) {
    const auto expectedTheme = ui::ThemeManager::instance().isLight() ? app::Theme::Dark : app::Theme::Light;
    QElapsedTimer elapsed;
    elapsed.start();
    theme->trigger();
    QApplication::processEvents();
    QCOMPARE(ui::ThemeManager::instance().current(), expectedTheme);
    qInfo() << "Main window theme applied:" << elapsed.elapsed() << "ms";
    if (qEnvironmentVariableIsSet("UWF_TEST_PROFILE_APPEARANCE")) {
      for (auto lines = recentLogLines(12, 2048); const auto& line : std::as_const(lines))
        if (line.find("appearance") != std::string::npos) qInfo().noquote() << QString::fromStdString(line);
    }
    const QString captures = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
    if (!captures.isEmpty() && round < 2) {
      QTest::qWait(35);
      QVERIFY(test::screenshot(window).save(QDir(captures).filePath(QStringLiteral("theme-%1-transition.png").arg(round))));
      if (QGuiApplication::platformName() == QLatin1String("windows"))
        QVERIFY(test::windowScreenshot(window).save(QDir(captures).filePath(QStringLiteral("native-theme-%1-transition.png").arg(round))));
      QTRY_VERIFY(window.findChildren<QWidget*>(QStringLiteral("appearanceTransition")).isEmpty());
      QTest::qWait(35);
      QVERIFY(test::screenshot(window).save(QDir(captures).filePath(QStringLiteral("theme-%1-complete.png").arg(round))));
      if (QGuiApplication::platformName() == QLatin1String("windows"))
        QVERIFY(test::windowScreenshot(window).save(QDir(captures).filePath(QStringLiteral("native-theme-%1-complete.png").arg(round))));
    }
    QVERIFY(!QApplication::activeModalWidget());
    QVERIFY(global);
    QCOMPARE(window.findChild<ui::GlobalStatusPanel*>(), global.data());
    QCOMPARE(global->viewModel().pendingFilterEnabled(), std::optional<bool>(false));
    QCOMPARE(source.reads, 1);
  }
  auto* language = actionWithText(&window, ui::I18n::tr("Language"));
  QVERIFY(language && language->menu());
  QAction* current = nullptr;
  QAction* other = nullptr;
  for (auto actions = language->menu()->actions(); auto* action : std::as_const(actions)) {
    if (!action->isCheckable()) continue;
    if (action->data().toInt() == static_cast<int>(originalLanguage))
      current = action;
    else
      other = action;
  }
  QVERIFY(current && other);
  QSignalSpy languages(&ui::I18n::events(), &ui::LanguageEvents::changed);
  other->trigger();
  current->trigger();
  QApplication::processEvents();
  QCOMPARE(ui::I18n::instance().lang(), originalLanguage);
  QCOMPARE(languages.count(), 0);
  QCOMPARE(global->viewModel().pendingFilterEnabled(), std::optional<bool>(false));
  QCOMPARE(source.reads, 1);
  QVERIFY(uwf::test::writeRequests(*system).empty());
  QCOMPARE(staging.writes, 0);
}

void ApplicationAppearanceTests::languageAndThemeChangesReachProductionWidgets() {
  QCOMPARE(ui::I18n::applicationTitle(), QStringLiteral("Unified Write Filter (UWF) Manager"));
  ui::I18n::instance().setLang(ui::I18n::Lang::Zh_CN);
  QCOMPARE(ui::I18n::instance().lang(), ui::I18n::Lang::Zh_CN);
  QVERIFY(!ui::I18n::applicationTitle().isEmpty());
  QVERIFY(ui::I18n::applicationTitle() != QStringLiteral("Unified Write Filter (UWF) Manager"));
  ui::AboutDialog localizedDialog([] {
    return uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())
        ->systemInformation()
        ->diagnosticDocument({});
  });
  QCOMPARE(localizedDialog.closeButton()->accessibleName(), QStringLiteral("关闭"));
  QVERIFY(!localizedDialog.closeButton()->icon().isNull());
  ui::I18n::instance().setLang(ui::I18n::Lang::En);
  QCOMPARE(localizedDialog.closeButton()->accessibleName(), ui::I18n::tr("Close"));

  auto& theme = ui::ThemeManager::instance();
  QSignalSpy changed(&theme, &ui::ThemeManager::themeChanged);
  theme.apply(app::Theme::Light);
  QCOMPARE(theme.current(), app::Theme::Light);
  QCOMPARE(qApp->palette().color(QPalette::Window), theme.color(ui::Sem::Bg));
  QCOMPARE(changed.count(), 1);
  const auto lightHud = ui::overlayHudPalette(app::Theme::Light);
  const auto darkHud = ui::overlayHudPalette(app::Theme::Dark);
  QVERIFY(lightHud.text != darkHud.text);
  QVERIFY(lightHud.floatingSurface.alpha() < lightHud.taskbarSurface.alpha());
  theme.toggle();
  QCOMPARE(theme.current(), app::Theme::Dark);
}

void ApplicationAppearanceTests::qtStandardWidgetsUseSupplementalTranslations_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  for (const auto& language : ui::I18n::languages()) {
    if (language.id == ui::I18n::Lang::En) continue;
    for (const bool dark : {false, true}) {
      const QByteArray name = QString(language.code).toLatin1() + (dark ? "-dark" : "-light");
      QTest::newRow(name.constData()) << language.id << dark;
    }
  }
}

void ApplicationAppearanceTests::qtStandardWidgetsUseSupplementalTranslations() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  auto& language = ui::I18n::instance();
  auto& theme = ui::ThemeManager::instance();
  const auto oldLanguage = language.lang();
  const auto oldTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    language.setLang(oldLanguage);
    theme.apply(oldTheme);
  });
  language.setLang(locale);
  QCOMPARE(language.lang(), locale);
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  const QString lineCopy = QCoreApplication::translate("QLineEdit", "&Copy");
  const QString textCopy = QCoreApplication::translate("QWidgetTextControl", "&Copy");
  const QString stepUp = QCoreApplication::translate("QAbstractSpinBox", "&Step up");
  const QString stepDown = QCoreApplication::translate("QAbstractSpinBox", "Step &down");
  const QString selectAll = QCoreApplication::translate("QAbstractSpinBox", "&Select All");
  QVERIFY(lineCopy != QStringLiteral("&Copy"));
  QVERIFY(textCopy != QStringLiteral("&Copy"));
  QVERIFY(stepUp != QStringLiteral("&Step up"));
  QVERIFY(stepDown != QStringLiteral("Step &down"));
  QVERIFY(selectAll != QStringLiteral("&Select All"));
  QVERIFY(QCoreApplication::translate("QUnicodeControlCharacterMenu", "Insert Unicode control character") !=
          QStringLiteral("Insert Unicode control character"));
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  const auto capture = [&](QWidget& widget, const QString& name) {
    const auto clipping = test::waitForUnclippedText(widget);
    QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
    if (!screenshots.isEmpty()) {
      QVERIFY(QDir().mkpath(screenshots));
      QVERIFY(uwf::test::screenshot(widget).save(QDir(screenshots).filePath(name + "-" + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
    }
  };
  QDialog dialog;
  auto* layout = new QVBoxLayout(&dialog);
  auto* line = new QLineEdit(QStringLiteral("C:\\Example.txt"), &dialog);
  auto* text = new QTextEdit(QStringLiteral("Example"), &dialog);
  auto* spin = new QSpinBox(&dialog);
  spin->setRange(0, 10);
  spin->setValue(5);
  layout->addWidget(line);
  layout->addWidget(text);
  layout->addWidget(spin);
  dialog.resize(test::boundedWindowSize(dialog, QSize(520, 280)));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  capture(dialog, QStringLiteral("qt-editors"));
  const auto inspectMenu = [&](std::unique_ptr<QMenu> menu, const QString& name, const QString& copy, const bool selected) {
    QAction* copyAction = nullptr;
    for (auto actions = menu->actions(); auto* action : std::as_const(actions))
      if (action->text().section('\t', 0, 0).remove('&') == QString(copy).remove('&')) copyAction = action;
    QVERIFY(copyAction);
    QCOMPARE(copyAction->isEnabled(), selected);
    menu->popup(dialog.mapToGlobal(QPoint(20, 30)));
    QTRY_VERIFY(menu->isVisible());
    for (auto actions = menu->actions(); auto* action : std::as_const(actions)) {
      if (action->isSeparator()) continue;
      QVERIFY(menu->rect().contains(menu->actionGeometry(action)));
    }
    capture(*menu, name);
    menu->hide();
  };
  line->deselect();
  text->moveCursor(QTextCursor::Start);
  inspectMenu(std::unique_ptr<QMenu>(line->createStandardContextMenu()), QStringLiteral("qt-line-menu"), lineCopy, false);
  inspectMenu(std::unique_ptr<QMenu>(text->createStandardContextMenu()), QStringLiteral("qt-text-menu"), textCopy, false);
  line->selectAll();
  text->selectAll();
  inspectMenu(std::unique_ptr<QMenu>(line->createStandardContextMenu()), QStringLiteral("qt-line-selected-menu"), lineCopy, true);
  inspectMenu(std::unique_ptr<QMenu>(text->createStandardContextMenu()), QStringLiteral("qt-text-selected-menu"), textCopy, true);
  line->setReadOnly(true);
  text->setReadOnly(true);
  inspectMenu(std::unique_ptr<QMenu>(line->createStandardContextMenu()), QStringLiteral("qt-line-readonly-menu"), lineCopy, true);
  inspectMenu(std::unique_ptr<QMenu>(text->createStandardContextMenu()), QStringLiteral("qt-text-readonly-menu"), textCopy, true);

  bool spinMenuTranslated = false;
  QTimer::singleShot(0, &dialog, [&] {
    auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    if (!menu) return;
    const auto close = qScopeGuard([&] { menu->close(); });
    QStringList labels;
    for (auto actions = menu->actions(); auto* action : std::as_const(actions)) labels.append(action->text().section('\t', 0, 0));
    spinMenuTranslated = labels.contains(stepUp) && labels.contains(stepDown) && labels.contains(selectAll);
    capture(*menu, QStringLiteral("qt-spin-menu"));
  });
  const QPoint position = spin->rect().center();
  QContextMenuEvent event(QContextMenuEvent::Mouse, position, spin->mapToGlobal(position));
  QApplication::sendEvent(spin, &event);
  QVERIFY(spinMenuTranslated);
}

void ApplicationAppearanceTests::buttonContrastAndSwitchFocusStayVisible_data() {
  QTest::addColumn<bool>("dark");
  QTest::newRow("light") << false;
  QTest::newRow("dark") << true;
}

void ApplicationAppearanceTests::buttonContrastAndSwitchFocusStayVisible() {
  QFETCH(bool, dark);
  auto& theme = ui::ThemeManager::instance();
  const auto original = theme.current();
  const auto restore = qScopeGuard([&] { theme.apply(original); });
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  ui::PanelDialog host;
  auto* layout = new QVBoxLayout(&host);
  auto* primary = new QPushButton(QStringLiteral("Apply"), &host);
  primary->setObjectName(QStringLiteral("primaryBtn"));
  ui::setDialogAction(primary, ui::DialogAction::Primary);
  auto* danger = new QPushButton(QStringLiteral("Remove"), &host);
  danger->setObjectName(QStringLiteral("dangerBtn"));
  ui::setDialogAction(danger, ui::DialogAction::Destructive);
  auto* enable = new QPushButton(QStringLiteral("Enable enhanced mode"), &host);
  enable->setObjectName(QStringLiteral("enhancedModeChangeButton"));
  ui::setDialogAction(enable, ui::DialogAction::Primary);
  auto* repair = new QPushButton(QStringLiteral("Repair enhanced mode"), &host);
  repair->setObjectName(QStringLiteral("enhancedModeChangeButton"));
  ui::setDialogAction(repair, ui::DialogAction::Primary);
  auto* toggle = new ui::SwitchButton(&host);
  auto* other = new QPushButton(QStringLiteral("Other"), &host);
  layout->addWidget(primary);
  layout->addWidget(danger);
  layout->addWidget(enable);
  layout->addWidget(repair);
  layout->addWidget(toggle);
  layout->addWidget(other);
  QWidget::setTabOrder(other, toggle);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  host.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&host));
  const auto luminance = [](const QColor& color) {
    const auto linear = [](const double value) { return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4); };
    return 0.2126 * linear(color.redF()) + 0.7152 * linear(color.greenF()) + 0.0722 * linear(color.blueF());
  };
  const auto contrast = [&](const QColor& foreground, const QColor& background) {
    const double backdrop = luminance(background);
    const double ink = luminance(foreground);
    return (std::max(ink, backdrop) + .05) / (std::min(ink, backdrop) + .05);
  };
  const auto coverage = [](QPushButton* button, const QColor& background) {
    return test::colorCoverage(button->grab().toImage(), background, test::interiorRegion(button->rect()));
  };
  const auto verifyContrast = [&](QPushButton* button, const QColor& background) {
    const auto foreground = button == danger ? QColor(Qt::white) : ui::DesignSystem::instance().color(ui::design::Color::AccentText);
    QVERIFY(contrast(foreground, background) >= 4.5);
    QTRY_VERIFY(coverage(button, background) >= test::kVisualMinimumCoverage);
    const auto image = button->grab().toImage();
    QVERIFY(!image.isNull());
    const auto region = test::interiorRegion(button->rect());
    QVERIFY(!region.isEmpty());
    const auto ink = test::visibleInkBounds(image, foreground, background, region);
    QVERIFY(!ink.isEmpty());
    QVERIFY(ink.width() >= button->fontMetrics().horizontalAdvance(button->text()) * test::kVisualMinimumCoverage);
    QVERIFY(ink.height() >= button->fontMetrics().xHeight() * test::kVisualMinimumCoverage);
    // Verify glyph foreground color independently of the button background.
    constexpr qreal minimumForegroundCoverage = .05;
    QVERIFY(test::colorCoverage(image, foreground, ink) >= minimumForegroundCoverage);
  };
  for (auto* button : {primary, danger, enable, repair}) {
    uwf::test::moveMouse(other);
    QTRY_VERIFY(!button->underMouse());
    const auto& design = ui::DesignSystem::instance();
    const QColor normal = design.color(button == danger ? ui::design::Color::DangerFill : ui::design::Color::Accent);
    verifyContrast(button, normal);
    uwf::test::moveMouse(button);
    QTRY_VERIFY(button->underMouse());
    const QColor hovered = design.color(button == danger ? ui::design::Color::DangerFillHover : ui::design::Color::AccentHover);
    QVERIFY(hovered != normal);
    verifyContrast(button, hovered);
    uwf::test::pressMouse(button, Qt::LeftButton);
    const QColor pressed = design.color(button == danger ? ui::design::Color::DangerFillPressed : ui::design::Color::AccentPressed);
    QVERIFY(pressed != hovered);
    verifyContrast(button, pressed);
    uwf::test::releaseMouse(button, Qt::LeftButton);
  }
  other->setFocus();
  QTRY_VERIFY(other->hasFocus());
  const int focusMargin = toggle->style()->pixelMetric(QStyle::PM_FocusFrameVMargin, nullptr, toggle);
  const auto focusImage = [&] {
    return host.grab(QRect(toggle->mapTo(&host, QPoint()), toggle->size()).adjusted(-focusMargin, -focusMargin, focusMargin, focusMargin)).toImage();
  };
  const auto focusCoverage = [&](const QImage& image) {
    const auto bounds = test::logicalBounds(image.rect(), image.devicePixelRatio());
    return test::colorCoverage(image, ui::DesignSystem::instance().color(ui::design::Color::Accent), QRectF(0, 0, bounds.width(), focusMargin));
  };
  const auto before = focusImage();
  const QSize originalSize = toggle->size();
  QTest::keyClick(other, Qt::Key_Tab);
  QTRY_VERIFY(toggle->hasFocus());
  QTRY_VERIFY(focusCoverage(focusImage()) > focusCoverage(before));
  QCOMPARE(toggle->size(), originalSize);
  QTest::keyClick(toggle, Qt::Key_Space);
  QVERIFY(toggle->isChecked());
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!screenshots.isEmpty()) {
    QVERIFY(QDir().mkpath(screenshots));
    QVERIFY(uwf::test::screenshot(host).save(QDir(screenshots).filePath(QStringLiteral("focus-") + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
  }
  QTest::keyClick(toggle, Qt::Key_Tab, Qt::ShiftModifier);
  QTRY_VERIFY(!toggle->hasFocus());

  for (const bool checked : {false, true}) {
    toggle->setChecked(checked);
    toggle->setFocus(Qt::TabFocusReason);
    QTRY_VERIFY(toggle->hasFocus());
    const QImage focused = focusImage();
    const QString state = checked ? QStringLiteral("on") : QStringLiteral("off");
    if (!screenshots.isEmpty()) {
      QVERIFY(uwf::test::screenshot(host).save(
          QDir(screenshots).filePath(QStringLiteral("switch-%1-focused-%2.png").arg(state, QString::fromLatin1(QTest::currentDataTag())))));
    }
    QTest::keyClick(toggle, Qt::Key_Tab, Qt::ShiftModifier);
    QTRY_VERIFY(!toggle->hasFocus());
    QTRY_VERIFY(focusCoverage(focusImage()) < focusCoverage(focused));
    QCOMPARE(toggle->isChecked(), checked);
    QCOMPARE(toggle->size(), originalSize);
    if (!screenshots.isEmpty()) {
      QVERIFY(uwf::test::screenshot(host).save(
          QDir(screenshots).filePath(QStringLiteral("switch-%1-unfocused-%2.png").arg(state, QString::fromLatin1(QTest::currentDataTag())))));
    }
  }
}

void ApplicationAppearanceTests::commonDialogsPreserveSafeDefaults() {
  bool warningSelectable = false;
  QVERIFY(test::driveModalDialog(
      *this, [] { ui::dialogs::warning(nullptr, QStringLiteral("Diagnostic"), QStringLiteral("A selectable provider error")); },
      [&](QDialog& dialog) {
        for (auto labels = dialog.findChildren<ui::ContentHintBox*>(); auto* label : std::as_const(labels)) {
          warningSelectable = warningSelectable || label->textInteractionFlags().testFlag(Qt::TextSelectableByMouse);
        }
        dialog.accept();
      }));
  QVERIFY(warningSelectable);

  bool cancelWasDefault = false;
  QVERIFY(test::driveModalDialog(
      *this, [] { QVERIFY(!ui::dialogs::confirm(nullptr, QStringLiteral("Confirm"), QStringLiteral("Dangerous operation"))); },
      [&](QDialog& dialog) {
        if (auto* box = dialog.findChild<QDialogButtonBox*>()) {
          for (auto buttons = box->buttons(); auto* button : std::as_const(buttons)) {
            if (box->buttonRole(button) == QDialogButtonBox::RejectRole) {
              cancelWasDefault = qobject_cast<QPushButton*>(button)->isDefault();
              QTest::mouseClick(button, Qt::LeftButton);
              return;
            }
          }
        }
        dialog.reject();
      }));
  QVERIFY(cancelWasDefault);
}

void ApplicationAppearanceTests::hintAndUsageWidgetsHandleEmptyOverflowAndThresholdEdges() {
  QScrollArea host;
  host.setWidgetResizable(true);
  host.resize(test::boundedWindowSize(host, QSize(240, 160)));
  auto* hint = new ui::ContentHintBox(&host);
  host.setWidget(hint);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  hint->setText(QStringLiteral("short"));
  QTRY_COMPARE_WITH_TIMEOUT(hint->verticalScrollBar()->maximum(), 0, 500);
  const int shortHeight = hint->height();
  hint->setText(QStringLiteral("a long information line ").repeated(50));
  QTRY_VERIFY_WITH_TIMEOUT(hint->height() > shortHeight, 1000);
  QTRY_COMPARE_WITH_TIMEOUT(hint->verticalScrollBar()->maximum(), 0, 1000);
  QVERIFY(hint->document()->size().height() <= hint->viewport()->height());
  QVERIFY(host.verticalScrollBar()->maximum() > 0);
  host.verticalScrollBar()->setValue(0);
  test::sendWheel(*host.viewport(), QPoint(0, -120));
  QTRY_VERIFY(host.verticalScrollBar()->value() > 0);
  const int narrowHeight = hint->height();
  host.resize(test::boundedWindowSize(host, QSize(480, 160)));
  QTRY_VERIFY_WITH_TIMEOUT(hint->height() < narrowHeight, 1000);
  QTRY_COMPARE_WITH_TIMEOUT(hint->verticalScrollBar()->maximum(), 0, 1000);
  hint->setText(QStringLiteral("short again"));
  QTRY_COMPARE_WITH_TIMEOUT(host.verticalScrollBar()->maximum(), 0, 1000);
  hint->clear();
  QTRY_COMPARE_WITH_TIMEOUT(hint->verticalScrollBar()->maximum(), 0, 500);

  ui::OverlayUsageBar bar;
  QCOMPARE(bar.minimumSizeHint().height(), bar.sizeHint().height());
  bar.resize(420, bar.sizeHint().height());
  bar.setData(0, 0, 0, 0);
  QImage empty(bar.size(), QImage::Format_ARGB32_Premultiplied);
  empty.fill(Qt::transparent);
  bar.render(&empty);
  QVERIFY(!empty.isNull());
  bar.setData(150, 60, 80, 100);
  QImage thresholds(bar.size(), QImage::Format_ARGB32_Premultiplied);
  thresholds.fill(Qt::transparent);
  bar.render(&thresholds);
  const QRectF usageRegion(0, bar.height() * .25, bar.width(), bar.height() * .5);
  const auto critical = ui::ThemeManager::instance().color(ui::Sem::Danger);
  QVERIFY(test::colorCoverage(thresholds, critical, usageRegion) >= test::kVisualMinimumCoverage);
  QVERIFY(test::colorCoverage(empty, critical, usageRegion) < test::kVisualMinimumCoverage);

  ui::Notice banner;
  banner.setText(QStringLiteral("Critical warning"));
  banner.setTone(ui::Notice::Tone::Error);
  banner.resize(320, 50);
  QImage rendered(banner.size(), QImage::Format_ARGB32_Premultiplied);
  rendered.fill(Qt::transparent);
  banner.render(&rendered);
  QVERIFY(!rendered.isNull());

  banner.setObjectName(QStringLiteral("statusBanner"));
  const QString longError = QStringLiteral("WMI query failed: <raw error> & code\nC:\\") + QString(180, QLatin1Char('x')) + QStringLiteral("\\data\n");
  for (const int width : {180, 320, 480}) {
    banner.setText(longError + QStringLiteral("Ending AAA"));
    const int required = banner.heightForWidth(width);
    QVERIFY(required > 2 * banner.fontMetrics().height());
    QVERIFY(banner.minimumSizeHint().width() <= width);
    banner.resize(width, required);
    QImage first(banner.size(), QImage::Format_ARGB32_Premultiplied);
    first.fill(Qt::transparent);
    banner.render(&first);
    banner.setText(longError + QStringLiteral("Ending BBB"));
    const int secondRequired = banner.heightForWidth(width);
    banner.resize(width, secondRequired);
    QImage second(banner.size(), QImage::Format_ARGB32_Premultiplied);
    second.fill(Qt::transparent);
    banner.render(&second);
    // The changed final line must appear in the rendered pixels.
    const int lastLineHeight = 2 * banner.fontMetrics().height();
    const auto firstLastLine = first.copy(QRect(0, required - lastLineHeight, width, lastLineHeight));
    const auto secondLastLine = second.copy(QRect(0, secondRequired - lastLineHeight, width, lastLineHeight));
    QVERIFY(test::changedCoverage(firstLastLine, secondLastLine, QRectF(0, 0, width, lastLineHeight)) > 0);
  }
  const auto errorImage = banner.grab().toImage();
  const ui::IconView* toneIcon = nullptr;
  for (auto children = banner.findChildren<QWidget*>(); auto* child : std::as_const(children))
    if (const auto* candidate = dynamic_cast<ui::IconView*>(child)) {
      toneIcon = candidate;
      break;
    }
  QVERIFY(toneIcon);
  const QRectF toneRegion(toneIcon->mapTo(&banner, QPoint()), toneIcon->size());
  banner.setTone(ui::Notice::Tone::Warning);
  QCOMPARE(banner.tone(), ui::Notice::Tone::Warning);
  QVERIFY(test::changedCoverage(errorImage, banner.grab().toImage(), toneRegion) > 0);
}

void ApplicationAppearanceTests::comboSelectionUsesStableItemData() {
  QComboBox combo;
  combo.addItem(QStringLiteral("A"), 10);
  combo.addItem(QStringLiteral("B"), 20);
  ui::setComboValue(&combo, 20);
  QCOMPARE(combo.currentData().toInt(), 20);
  combo.setItemText(1, QStringLiteral("Translated caption"));
  QCOMPARE(combo.currentData().toInt(), 20);
  ui::setComboValue(&combo, 30);
  QCOMPARE(combo.currentData().toInt(), 20);
}

void ApplicationAppearanceTests::dialogLayoutsKeepActionsVisible_data() { addLocaleThemeRows(); }

void ApplicationAppearanceTests::dialogLayoutsKeepActionsVisible() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  auto& language = ui::I18n::instance();
  auto& theme = ui::ThemeManager::instance();
  const auto oldLanguage = language.lang();
  const auto oldTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    language.setLang(oldLanguage);
    theme.apply(oldTheme);
  });
  language.setLang(locale);
  QCOMPARE(language.lang(), locale);
  // Batch construction-time translations under one palette. Changing the
  // application theme for every dialog would repeatedly repolish all widgets.
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  {
    ui::OverlayContextMenu menu;
    menu.setRestorePositionAvailable(true);
    menu.setOperationAvailability(true, true);
    menu.setHubState(true, true);
    menu.popup(QPoint(100, 100));
    QVERIFY(QTest::qWaitForWindowExposed(&menu));
    const auto alignmentError = test::probeInlineIconText(menu, QStringLiteral("tray-hud-menu"), QString::fromLatin1(QTest::currentDataTag()));
    QVERIFY2(alignmentError.isEmpty(), qUtf8Printable(alignmentError));
    menu.hide();
  }
  const auto inspect = [&](QDialog& dialog, const QString& name, const bool boundedContent = false) {
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    const QString captureTag = QString::fromLatin1(QTest::currentDataTag());
    if (const auto* window = qobject_cast<ui::DialogWindow*>(&dialog)) {
      QVERIFY(dialog.windowFlags().testFlag(Qt::FramelessWindowHint));
      const auto* close = window->closeButton();
      const auto* title = window->findChild<QLabel*>(QStringLiteral("dialogTitle"));
      QVERIFY(close && title);
      const QRect titleInkArea(title->mapTo(&dialog, title->contentsRect().topLeft()), title->contentsRect().size());
      const QRect closeArea(close->mapTo(&dialog, QPoint()), close->size());
      QVERIFY(!closeArea.intersects(titleInkArea));
      QCOMPARE(title->layoutDirection(), dialog.layoutDirection());
    }
    const auto geometryDescription = [&] {
      QString description;
      QDebug debug(&description);
      debug << name << "window" << dialog.size() << "minimum" << dialog.minimumSize() << "layout minimum" << dialog.layout()->minimumSize();
      if (const auto* footer = dialog.findChild<QFrame*>(QStringLiteral("dialogFooter")); footer && footer->layout()) {
        debug << "footer" << footer->geometry() << "minimum" << footer->layout()->minimumSize();
        for (auto boxes = footer->findChildren<QDialogButtonBox*>(); const auto* box : std::as_const(boxes))
          debug << "buttons" << box->geometry() << "preferred" << box->sizeHint() << "minimum" << box->minimumSize();
        for (auto labels = footer->findChildren<QLabel*>(); const auto* label : std::as_const(labels))
          debug << "hint" << label->geometry() << "minimum" << label->minimumSize();
      }
      for (auto areas = dialog.findChildren<QAbstractScrollArea*>(); const auto* area : std::as_const(areas)) {
        if (!area->isVisibleTo(&dialog)) continue;
        debug << "scroll" << area->objectName() << area->viewport()->size() << "ranges" << area->horizontalScrollBar()->maximum()
              << area->verticalScrollBar()->maximum();
        if (const auto* scroll = qobject_cast<const QScrollArea*>(area); scroll && scroll->widget()) {
          debug << "body" << scroll->widget()->size() << "minimum" << scroll->widget()->minimumSizeHint() << "preferred" << scroll->widget()->sizeHint();
          debug << "heightForWidth" << scroll->widget()->layout()->totalHeightForWidth(scroll->widget()->width());
          debug << "layout geometry" << scroll->widget()->layout()->geometry();
        }
      }
      if (const auto* header = dialog.findChild<QWidget*>(QStringLiteral("dialogHeader")))
        debug << "header" << header->geometry() << "preferred" << header->sizeHint() << "heightForWidth" << header->heightForWidth(dialog.width());
      return description;
    };
    QVERIFY(dialog.minimumSize() != dialog.maximumSize());
    const QSize workArea = boundedContent ? test::availableClientSize(dialog) : test::boundedWindowSize(dialog, QSize(1200, 700));
    QVERIFY(!workArea.isEmpty());
    dialog.resize(dialog.size().boundedTo(workArea));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    const auto clipping = test::waitForUnclippedText(dialog, 2000);
    QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping.isEmpty() ? QString{} : clipping + geometryDescription()));
    if (!name.startsWith(QStringLiteral("enhanced-")) && name != QStringLiteral("apply-plan") && name != QStringLiteral("import")) {
      const auto alignmentError = test::probeInlineIconText(dialog, name, captureTag);
      QVERIFY2(alignmentError.isEmpty(), qUtf8Printable(alignmentError));
      for (auto areas = dialog.findChildren<QScrollArea*>(); auto* area : std::as_const(areas)) {
        if (!area->isVisibleTo(&dialog) || area->verticalScrollBar()->maximum() <= 0) continue;
        const int before = area->verticalScrollBar()->value();
        area->verticalScrollBar()->setValue(area->verticalScrollBar()->maximum());
        const auto scrolledError = test::probeInlineIconText(dialog, name, captureTag + QStringLiteral("-scrolled"));
        area->verticalScrollBar()->setValue(before);
        QVERIFY2(scrolledError.isEmpty(), qUtf8Printable(scrolledError));
      }
    }
    if (!screenshots.isEmpty()) {
      QVERIFY(QDir().mkpath(screenshots));
      QVERIFY(uwf::test::screenshot(dialog).save(QDir(screenshots).filePath(name + "-" + captureTag + ".png")));
      if (QGuiApplication::platformName() == QLatin1String("windows"))
        QVERIFY(uwf::test::windowScreenshot(dialog).save(QDir(screenshots).filePath("native-" + name + "-" + captureTag + ".png")));
    }
    QVERIFY2(dialog.width() <= workArea.width() && dialog.height() <= workArea.height(),
             qUtf8Printable(QStringLiteral("%1 exceeds compact work area: %2x%3").arg(name).arg(dialog.width()).arg(dialog.height())));
    if (const auto* footer = dialog.findChild<QFrame*>(QStringLiteral("dialogFooter")); footer && footer->isVisible()) {
      QVERIFY(footer->height() >= footer->minimumSizeHint().height());
      for (auto boxes = footer->findChildren<QDialogButtonBox*>(); const auto* box : std::as_const(boxes)) {
        if (!box->isVisibleTo(footer)) continue;
        QVERIFY2(footer->rect().contains(QRect(box->mapTo(footer, QPoint()), box->size())), qUtf8Printable(name));
        QVERIFY(test::centersAligned(QRectF(box->mapTo(footer, QPoint()), box->size()), footer->rect()));
      }
      for (auto labels = footer->findChildren<QLabel*>(); const auto* label : std::as_const(labels)) {
        if (!label->isVisibleTo(footer)) continue;
        QVERIFY(footer->rect().contains(QRect(label->mapTo(footer, QPoint()), label->size())));
        QVERIFY(test::centersAligned(QRectF(label->mapTo(footer, QPoint()), label->size()), footer->rect()));
        QVERIFY(label->alignment().testFlag(Qt::AlignVCenter));
      }
    }
    for (auto buttons = dialog.findChildren<QPushButton*>(); auto* button : std::as_const(buttons)) {
      if (!button->isVisible()) continue;
      for (auto* ancestor = button->parentWidget(); ancestor && ancestor != &dialog; ancestor = ancestor->parentWidget())
        if (auto* scroll = qobject_cast<QScrollArea*>(ancestor)) scroll->ensureWidgetVisible(button);
      QVERIFY2(dialog.rect().contains(QRect(button->mapTo(&dialog, QPoint()), button->size())), qUtf8Printable(name + ": " + button->text()));
      QVERIFY2(!button->visibleRegion().isEmpty(), qUtf8Printable(name + ": " + button->text()));
      QStyleOptionButton option;
      option.initFrom(button);
      const QRect contents = button->style()->subElementRect(QStyle::SE_PushButtonContents, &option, button);
      if (!button->text().isEmpty()) {
        const int textWidth = button->fontMetrics().horizontalAdvance(button->text().remove('&'));
        QVERIFY2(contents.width() >= textWidth, qUtf8Printable(name + ": " + button->text()));
      }
    }
    // Check keyboard traversal with LTR/RTL representatives; screenshot export retains every locale/theme focus state.
    if (name.startsWith(QStringLiteral("enhanced-")) && (!screenshots.isEmpty() || (!dark && (locale == ui::I18n::Lang::En || locale == ui::I18n::Lang::Ar)))) {
      auto* close = buttonWithText(&dialog, ui::I18n::tr("Close"));
      QVERIFY(close);
      dialog.activateWindow();
      QVERIFY(QTest::qWaitForWindowActive(&dialog));
      close->setFocus(Qt::TabFocusReason);
      QTest::keyClick(close, Qt::Key_Tab);
      QTRY_VERIFY(!close->hasFocus());
      // The shared focus indicator paints outside the control's own rectangle.
      const int focusMargin = close->style()->pixelMetric(QStyle::PM_FocusFrameVMargin, nullptr, close);
      const QRect focusBounds =
          QRect(close->mapTo(&dialog, QPoint()), close->size()).adjusted(-focusMargin, -focusMargin, focusMargin, focusMargin).intersected(dialog.rect());
      const auto unfocused = dialog.grab(focusBounds).toImage();
      for (int stop = 0; stop < 20 && !close->hasFocus(); ++stop) QTest::keyClick(QApplication::focusWidget(), Qt::Key_Tab);
      QVERIFY(close->hasFocus());
      QTRY_VERIFY(test::changedCoverage(unfocused, dialog.grab(focusBounds).toImage(), test::logicalBounds(unfocused.rect(), unfocused.devicePixelRatio())) >
                  0);
      if (!screenshots.isEmpty())
        QVERIFY(uwf::test::screenshot(dialog).save(QDir(screenshots).filePath(name + QStringLiteral("-close-focus-") + captureTag + ".png")));
    }
    if (!name.startsWith(QStringLiteral("enhanced-")) && name != QStringLiteral("apply-plan") && name != QStringLiteral("import")) {
      const auto menuError = test::probeInlineMenus(dialog, name, captureTag);
      QVERIFY2(menuError.isEmpty(), qUtf8Printable(menuError));
    }
    dialog.hide();
  };
  const auto inspectModal = [&](const QString& name, const std::function<void()>& open, const bool boundedContent = false,
                                const std::function<bool(QDialog&)>& ready = {}) {
    bool inspected = false;
    QVERIFY2(test::driveModalDialog(*this, open,
                                    [&](QDialog& dialog) {
                                      QTRY_VERIFY_WITH_TIMEOUT(!ready || ready(dialog), 5000);
                                      inspect(dialog, name, boundedContent);
                                      inspected = true;
                                      dialog.reject();
                                    }),
             qUtf8Printable(name));
    QVERIFY2(inspected, qUtf8Printable(name));
  };
  {
    ui::AboutDialog dialog([] { return presentation::DiagnosticReportViewModel(QStringLiteral("Mock report, no machine data")); }, nullptr);
    inspect(dialog, QStringLiteral("about"), true);
  }
  {
    FileDialogStub files;
    ui::ImportDialog dialog(std::make_shared<uwf::presentation::ImportViewModel>(uwf::windows::readBoundedTextFile), files);
    inspect(dialog, QStringLiteral("import"));
    auto* recommended = buttonWithText(&dialog, ui::I18n::tr("Load recommended configuration"));
    QVERIFY(recommended);
    inspectModal(QStringLiteral("recommended-configuration"), [&] { recommended->click(); }, true);
  }
  {
    app::DiagnosticReportData report;
    report.processArchitecture = QStringLiteral("x64");
    ui::SystemInformationDialog dialog(presentation::DiagnosticReportViewModel(report, 8192));
    inspect(dialog, QStringLiteral("system-information"));
  }
  {
    const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
    FileDialogStub files;
    auto snapshot = editableSnapshot();
    presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
    ui::GlobalStatusPanel global(configuration.global());
    configuration.resetSnapshot(snapshot);
    QCOMPARE((global.viewModel().importFilterEnabled(false)).outcome, uwf::app::GlobalEditOutcome::Applied);
    ui::ApplyPlanDialog dialog(
        uwf::composition::createViewModelFactory(system, uwf::test::localDesktopPorts())->applyPlan(configuration.pendingChanges(), snapshot), files);
    inspect(dialog, QStringLiteral("apply-plan"));
    QVERIFY(uwf::test::writeRequests(*system).empty());
  }
  {
    if (uwf::test::visualComparisonEnabled()) {
      clearLogLines();
      logLine('I', "visual-test", "The configuration was loaded.");
      logLine('W', "visual-test", "A setting needs attention.");
      logLine('E', "visual-test", "The operation could not be completed.");
    }
    ui::LogViewerDialog dialog(uwf::test::screenshotLogModel());
    if (uwf::test::visualComparisonEnabled()) {
      dialog.show();
      auto* table = dialog.findChild<QTableView*>();
      QVERIFY(table);
      QTRY_COMPARE(table->model()->rowCount(), 3);
    }
    inspect(dialog, QStringLiteral("log"));
  }
  for (const auto state :
       {app::EnhancedModeState::Disabled, app::EnhancedModeState::Enabled, app::EnhancedModeState::Stopped, app::EnhancedModeState::RepairRequired}) {
    const auto system = std::make_shared<uwf::test::FakeUwfBackend>();
    auto controlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
    auto& control = *controlOwner;
    if (state != app::EnhancedModeState::Disabled) control.current = completeEnhancedModeStatus();
    control.current.state = state;
    if (state == app::EnhancedModeState::Stopped) control.current.running = false;
    if (state == app::EnhancedModeState::RepairRequired) {
      control.current.executableMatches = false;
      control.current.detail =
          QStringLiteral("Mock service contract mismatch:\nC:\\") + QString(180, QLatin1Char('x')) + QStringLiteral("\\UWF.exe\n最后一行错误详情");
    }
    app::EnhancedModeManager manager(controlOwner, uwf::app::uwfMaintenanceRepository(system), uwf::app::UwfCapability::Unavailable);
    ui::EnhancedModeDialog dialog(
        uwf::composition::createViewModelFactory(std::make_shared<uwf::test::FakeUwfBackend>(), uwf::test::localDesktopPorts())->enhancedMode(manager));
    inspect(dialog, QStringLiteral("enhanced-%1").arg(static_cast<int>(state)), true);
    QVERIFY(uwf::test::writeRequests(*system).empty());
    QCOMPARE(control.installs + control.starts + control.removals, 0);
  }
  class OfflineRegistryBrowser final : public app::RegistryBrowser {
   public:
    std::vector<std::string> rootHiveLongNames(std::stop_token = {}) const override { return {"HKEY_LOCAL_MACHINE"}; }
    std::vector<std::string> subkeyNames(std::string_view key, std::stop_token = {}) const override {
      return key == "HKEY_LOCAL_MACHINE" ? std::vector<std::string>{"SOFTWARE"} : std::vector<std::string>{};
    }
    bool hasSubkeys(std::string_view key, std::stop_token = {}) const override { return key == "HKEY_LOCAL_MACHINE"; }
    std::vector<uwf::app::RegistryValueInfo> values(std::string_view, std::stop_token = {}) const override {
      return {{"MockValue", REG_DWORD, std::vector<uint8_t>{1, 0, 0, 0}}};
    }
  };
  class OfflineFileBrowser final : public app::FileBrowser {
   public:
    explicit OfflineFileBrowser(const int state) : m_state(state) {}
    QVector<app::FileBrowserEntry> list(const QString&, std::stop_token) const override {
      if (m_state == 2) throw app::FileBrowserError(app::FileBrowserErrorCode::AccessDenied);
      if (m_state == 1) return {};
      return {{QStringLiteral("C:\\Program Files (x86)\\Microsoft\\EdgeWebView"), QStringLiteral("Microsoft EdgeWebView"), true},
              {QStringLiteral("C:\\Program Files (x86)\\Ubisoft"), QStringLiteral("Ubisoft"), true},
              {QStringLiteral("C:\\Program Files\\Windows Defender"), QStringLiteral("Windows Defender"), true},
              {QStringLiteral("C:\\Program Files (x86)\\configuration.txt"), QStringLiteral("configuration.txt"), false, 1024}};
    }
    app::FileBrowserEntry inspect(const QString& path) const override { return {path, path, true}; }
    QVector<app::FileBrowserPlace> places() const override {
      return {{QStringLiteral("Program Files"), QStringLiteral("C:\\Program Files"), false}, {QStringLiteral("System (C:)"), QStringLiteral("C:\\"), true}};
    }
    void createDirectory(const QString&) const override {}

   private:
    int m_state;
  };
  for (int state = 0; state < 3; ++state) {
    auto model =
        std::make_shared<presentation::FilePickerViewModel>(presentation::FilePickerViewModel::Mode::Mixed, std::make_shared<OfflineFileBrowser>(state));
    ui::FilePickerDialog dialog(model, ui::I18n::tr("Select files or folders"));
    model->start(QStringLiteral("C:\\Program Files (x86)"));
    dialog.show();
    QTRY_VERIFY(!model->busy());
    inspect(dialog, QStringLiteral("file-picker-%1").arg(state));
  }
  const auto browser = std::make_shared<OfflineRegistryBrowser>();
  for (const auto mode :
       {ui::RegistryPickerDialog::Mode::CommitValue, ui::RegistryPickerDialog::Mode::DeleteValue, ui::RegistryPickerDialog::Mode::Exclusion}) {
    const auto model = std::make_shared<uwf::presentation::RegistryPickerViewModel>(mode, browser);
    ui::RegistryPickerDialog dialog(model, ui::I18n::tr("Commit registry changes"));
    QSignalSpy navigated(model.get(), &uwf::presentation::RegistryPickerViewModel::navigationFinished);
    dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE"));
    QTRY_COMPARE(navigated.count(), 1);
    QTRY_VERIFY(!model->valuesLoading());
    inspect(dialog, QStringLiteral("registry-%1").arg(static_cast<int>(mode)));
  }
  for (const bool failure : {false, true}) {
    FileDialogStub files;
    ui::OverlayFilesDialog dialog(std::make_shared<uwf::presentation::OverlayFilesViewModel>(
                                      QStringLiteral("C:"), uwf::test::overlayFilesVolumeGuid(QStringLiteral("C:")),
                                      uwf::test::overlayFilesQueryManager([failure](const QString&, std::stop_token) -> QVector<app::OverlayFileEntry> {
                                        if (failure) throw std::runtime_error("Mock provider error");
                                        return {{QStringLiteral("\\Mock.txt"), {}, false, false, 1024}, {QStringLiteral("\\Settings"), {}, true, false, 0}};
                                      })),
                                  files);
    dialog.show();
    QTRY_VERIFY(!dialog.findChildren<QTableView*>().isEmpty());
    inspect(dialog, failure ? QStringLiteral("overlay-error") : QStringLiteral("overlay-files"));
  }
  for (const auto operation : {app::CommitOperation::Persist, app::CommitOperation::DeleteAndPersist}) {
    inspectModal(QStringLiteral("commit-report-%1").arg(static_cast<int>(operation)), [&] {
      ui::showCommitReport(
          nullptr,
          {{uwf::presentation::CommitRowStatus::Succeeded, QStringLiteral("C:\\Mock.txt"), QStringLiteral("-"), QStringLiteral("-"), true, false},
           {uwf::presentation::CommitRowStatus::Failed, QStringLiteral("C:\\Blocked.txt"), QStringLiteral("0x80041001"),
            ui::I18n::tr("The operation failed (see log for details)."), true, true}},
          operation, {presentation::CommitCompletion::Cause::Canceled, 2});
    });
  }
  for (const auto action : {app::PowerAction::Shutdown, app::PowerAction::Restart}) {
    for (const auto mode : {app::PowerActionDialogMode::Safe, app::PowerActionDialogMode::Direct}) {
      inspectModal(
          QStringLiteral("power-%1-%2").arg(static_cast<int>(action)).arg(static_cast<int>(mode)),
          [&] { QCOMPARE(ui::runPowerActionDialog(nullptr, {action, {}, mode}), app::PowerActionDialogOutcome::Canceled); }, true);
    }
    inspectModal(
        QStringLiteral("power-failure-%1").arg(static_cast<int>(action)),
        [&] {
          const auto outcome = ui::runPowerActionDialog(
              nullptr,
              {action, []() -> std::optional<app::PowerStagingPreparation> { return app::PowerStagingFailure{QStringLiteral("Mock file staging failure")}; }},
              std::chrono::milliseconds::zero());
          QCOMPARE(outcome, app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
        },
        true,
        [](QDialog& dialog) {
          const auto* disclosure = dialog.findChild<ui::Disclosure*>(QStringLiteral("powerFailureDisclosure"));
          return disclosure && disclosure->isVisibleTo(&dialog);
        });
  }
}

void ApplicationAppearanceTests::longMessageDialogsKeepCompleteTextAndReachableActions_data() { addLocaleThemeRows(); }

void ApplicationAppearanceTests::longMessageDialogsKeepCompleteTextAndReachableActions() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  const auto language = ui::I18n::instance().lang();
  const auto theme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(language);
    ui::ThemeManager::instance().apply(theme);
  });
  ui::I18n::instance().setLang(locale);
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  const QString target = QStringLiteral("C:\\") + QString(180, u'x') + QStringLiteral("\\<literal>&data.txt");
  const QString details = (QStringLiteral("WMI: 0x80041003 — mock failure; no operation performed.\n") + target + "\n").repeated(25);
  for (int purpose = 0; purpose < 2; ++purpose) {
    bool inspected = false;
    QVERIFY(test::driveModalDialog(
        *this,
        [&] {
          switch (purpose) {
            case 0:
              ui::dialogs::warning(nullptr, ui::I18n::tr("Error"), details);
              break;
            case 1:
              QVERIFY(!ui::dialogs::confirm(nullptr, ui::I18n::tr("Confirm"), details));
              break;
          }
        },
        [&](QDialog& modal) {
          auto* dialog = &modal;
          const auto requested = test::boundedWindowSize(*dialog, QSize(800, 500));
          QVERIFY(!requested.isEmpty());
          dialog->resize(requested);
          QTRY_VERIFY(test::similarSize(QRectF(QPointF(), dialog->size()), QRectF(QPointF(), requested)));
          const auto clipping = test::waitForUnclippedText(*dialog, 2000);
          QVERIFY2(clipping.isEmpty(), qUtf8Printable(clipping));
          auto* scroll = dialog->findChild<QScrollArea*>();
          QVERIFY(scroll);
          QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
          QTRY_VERIFY(scroll->verticalScrollBar()->maximum() > 0);
          const auto texts = dialog->findChildren<ui::ContentHintBox*>();
          QVERIFY(!texts.isEmpty());
          QVERIFY(std::ranges::any_of(texts, [&](const auto* text) { return text->toPlainText().contains(target); }));
          for (auto boxes = dialog->findChildren<QDialogButtonBox*>(); auto* box : std::as_const(boxes)) {
            QVERIFY(dialog->rect().contains(QRect(box->mapTo(dialog, QPoint()), box->size())));
            QVERIFY(!box->visibleRegion().isEmpty());
          }
          const QString captureTag = QString::fromLatin1(QTest::currentDataTag());
          const auto capture = [&](const QString& position) {
            if (screenshots.isEmpty()) return true;
            return QDir().mkpath(screenshots) &&
                   uwf::test::screenshot(*dialog).save(
                       QDir(screenshots).filePath(QStringLiteral("long-message-%1-%2-%3.png").arg(purpose).arg(position, captureTag)));
          };
          auto* scrollbar = scroll->verticalScrollBar();
          scrollbar->setValue(0);
          QVERIFY(capture(QStringLiteral("top")));
          QTest::keyClick(scrollbar, Qt::Key_End);
          QTRY_COMPARE(scrollbar->value(), scrollbar->maximum());
          QVERIFY(capture(QStringLiteral("bottom")));
          inspected = true;
        }));
    QVERIFY(inspected);
  }
}

void ApplicationAppearanceTests::textRenderingProducesValidGlyphsAndSmoothEdges() {
  auto& theme = ui::ThemeManager::instance();
  theme.apply(theme.current());
  QLabel label(QStringLiteral("UWF Manager · 文件暂存 · 0123456789"));
  label.setStyleSheet("QLabel { background: white; color: black; padding: 0; border: 0; }");
  label.resize(test::boundedWindowSize(label, QSize(600, 80)));
  label.show();
  QVERIFY(QTest::qWaitForWindowExposed(&label));
  for (const auto weight : {QFont::Normal, QFont::Medium, QFont::DemiBold}) {
    for (const auto& text : {QStringLiteral("UWF Manager · 0123456789"), QStringLiteral("全局设置 · 筛选器 · 覆盖层")}) {
      label.setText(text);
      QFont font = label.font();
      font.setWeight(weight);
      label.setFont(font);
      const auto image = label.grab().toImage().convertToFormat(QImage::Format_RGB32);
      QVERIFY(!image.isNull());
      const auto region = test::logicalBounds(image.rect(), image.devicePixelRatio());
      QVERIFY(!region.isEmpty());
      const auto ink = test::visibleInkBounds(image, Qt::black, Qt::white, region);
      QVERIFY2(!ink.isEmpty(), "The label must paint visible glyphs with the current font rasterizer.");
      QVERIFY(ink.width() >= label.fontMetrics().horizontalAdvance(label.text()) * test::kVisualMinimumCoverage);
      QVERIFY(ink.height() >= label.fontMetrics().xHeight() * test::kVisualMinimumCoverage);
      QTextLayout layout(label.text(), font);
      layout.beginLayout();
      auto line = layout.createLine();
      QVERIFY(line.isValid());
      line.setLineWidth(label.width());
      layout.endLayout();
      const auto runs = layout.glyphRuns();
      QVERIFY(!runs.isEmpty());
      for (const auto& run : runs) {
        QVERIFY(run.rawFont().isValid());
        QVERIFY(std::ranges::none_of(run.glyphIndexes(), [](quint32 glyph) { return glyph == 0; }));
        qInfo() << "typography" << label.devicePixelRatioF() << QFontInfo(font).family() << weight << run.rawFont().familyName() << run.rawFont().styleName();
      }
      const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
      if (!screenshots.isEmpty()) {
        QVERIFY(QDir().mkpath(screenshots));
        const QString name = QStringLiteral("typography-%1-%2.png").arg(static_cast<int>(weight)).arg(text.startsWith("UWF") ? "en" : "zh");
        QVERIFY(uwf::test::screenshot(label).save(QDir(screenshots).filePath(name)));
        if (QGuiApplication::platformName() == QLatin1String("windows")) {
          label.repaint();
          QTest::qWait(100);
          QVERIFY(uwf::test::nativeScreenshot(label).save(QDir(screenshots).filePath("native-" + name)));
        }
      }
    }
  }
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel panel(configuration.global());
  configuration.resetSnapshot(editableSnapshot());
  panel.show();
  QVERIFY(QTest::qWaitForWindowExposed(&panel));
  auto* title = panel.findChild<QLabel*>(QStringLiteral("panelTitle"));
  QVERIFY(title);
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!screenshots.isEmpty()) {
    QVERIFY(QDir().mkpath(screenshots));
    QVERIFY(uwf::test::screenshot(panel).save(QDir(screenshots).filePath(QStringLiteral("settings.png"))));
    if (QGuiApplication::platformName() == QLatin1String("windows")) {
      QTest::qWait(100);
      QVERIFY(uwf::test::nativeScreenshot(panel).save(QDir(screenshots).filePath(QStringLiteral("native-settings.png"))));
    }
  }
}

QTEST_MAIN(ApplicationAppearanceTests)
#include "AppearanceCases.moc"
