// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <qt_windows.h>

#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFontInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSpinBox>
#include <QStyle>
#include <QStyleOptionButton>
#include <QTableView>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ApplicationUiFixture.h"
#include "ApplicationUiTest.h"
#include "app/api/UwfCapability.h"
#include "app/commit/CommitBatch.h"
#include "app/configuration/UwfmgrCommands.h"
#include "app/enhanced/EnhancedModeManager.h"
#include "app/ports/FileStagingStore.h"
#include "app/ports/VolumePaths.h"
#include "app/registry/RegistryTreeCommitter.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "bootstrap/MainWindowComposition.h"
#include "bootstrap/ViewModelComposition.h"
#include "bootstrap/WorkflowComposition.h"
#include "core/UwfModel.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/configuration/ConfigurationViewModel.h"
#include "presentation/staging/FileStagingResultText.h"
#include "shared/Log.h"
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/WidgetCapture.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/commit/CommitReportDialog.h"
#include "ui/configuration/ApplyPlanDialog.h"
#include "ui/configuration/GlobalStatusPanel.h"
#include "ui/configuration/ImportDialog.h"
#include "ui/diagnostics/AboutDialog.h"
#include "ui/diagnostics/LogViewerDialog.h"
#include "ui/diagnostics/SystemInformationDialog.h"
#include "ui/dialogs/DialogAppearance.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/dialogs/InformationDialog.h"
#include "ui/dialogs/PanelDialog.h"
#include "ui/enhanced/EnhancedModeDialog.h"
#include "ui/overlay/OverlayFilesDialog.h"
#include "ui/overlay/OverlayHudPalette.h"
#include "ui/overlay/OverlayUsageBar.h"
#include "ui/power/PowerConfirmDialog.h"
#include "ui/registry/RegistryPickerDialog.h"
#include "ui/shell/MainWindow.h"
#include "ui/widgets/ComboBoxSelection.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/Notice.h"
#include "ui/widgets/SwitchButton.h"
#include "windows/filesystem/BoundedTextFileReader.h"
#include "windows/system/SystemHardwareInfo.h"
#include "windows/system/WindowsShell.h"

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
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner),
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
  QVERIFY(global->viewModel().importFilterEnabled(false));
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
      for (const auto lines = recentLogLines(12, 2048); const auto& line : lines)
        if (line.find("appearance") != std::string::npos) qInfo().noquote() << QString::fromStdString(line);
    }
    const QString captures = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
    if (!captures.isEmpty() && round < 2) {
      QTest::qWait(35);
      QVERIFY(test::windowScreenshot(window).save(QDir(captures).filePath(QStringLiteral("theme-%1-transition.png").arg(round))));
      QTRY_VERIFY(window.findChildren<QWidget*>(QStringLiteral("appearanceTransition")).isEmpty());
      QTest::qWait(35);
      QVERIFY(test::windowScreenshot(window).save(QDir(captures).filePath(QStringLiteral("theme-%1-complete.png").arg(round))));
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
  for (const auto actions = language->menu()->actions(); auto* action : actions) {
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
  QVERIFY(wmi.invocations.empty());
  QCOMPARE(staging.writes, 0);
}

void ApplicationAppearanceTests::languageAndThemeChangesReachProductionWidgets() {
  QCOMPARE(ui::I18n::applicationTitle(), QStringLiteral("Unified Write Filter (UWF) Manager"));
  ui::I18n::instance().setLang(ui::I18n::Lang::Zh_CN);
  QCOMPARE(ui::I18n::instance().lang(), ui::I18n::Lang::Zh_CN);
  QVERIFY(!ui::I18n::applicationTitle().isEmpty());
  QVERIFY(ui::I18n::applicationTitle() != QStringLiteral("Unified Write Filter (UWF) Manager"));
  QCOMPARE(ui::I18n::enhancedModeServiceDescription(), QStringLiteral("UWF Manager 增强模式辅助服务"));
  ui::AboutDialog localizedDialog([] { return uwf::bootstrap::createViewModelFactory()->systemInformation()->diagnosticDocument({}); });
  QCOMPARE(localizedDialog.closeButton()->accessibleName(), QStringLiteral("关闭"));
  QVERIFY(!localizedDialog.closeButton()->icon().isNull());
  ui::I18n::instance().setLang(ui::I18n::Lang::En);
  QCOMPARE(localizedDialog.closeButton()->accessibleName(), ui::I18n::tr("Close"));
  QCOMPARE(ui::I18n::enhancedModeServiceDescription(), QStringLiteral("UWF Manager enhanced mode helper service"));

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
    QTRY_VERIFY2(test::clippedText(widget).isEmpty(), qUtf8Printable(test::clippedText(widget)));
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
  dialog.resize(520, 280);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  capture(dialog, QStringLiteral("qt-editors"));
  const auto inspectMenu = [&](std::unique_ptr<QMenu> menu, const QString& name, const QString& copy, const bool selected) {
    QAction* copyAction = nullptr;
    for (const auto actions = menu->actions(); auto* action : actions)
      if (action->text().section('\t', 0, 0).remove('&') == QString(copy).remove('&')) copyAction = action;
    QVERIFY(copyAction);
    QCOMPARE(copyAction->isEnabled(), selected);
    menu->popup(dialog.mapToGlobal(QPoint(20, 30)));
    QTRY_VERIFY(menu->isVisible());
    for (const auto actions = menu->actions(); auto* action : actions) {
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
    for (const auto actions = menu->actions(); auto* action : actions) labels.append(action->text().section('\t', 0, 0));
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
  const auto surface = [](QPushButton* button) {
    const auto image = button->grab().toImage();
    return image.pixelColor(image.width() / 2, qRound(5 * image.devicePixelRatio()));
  };
  const auto contrast = [&](QPushButton* button, const QColor& background) {
    const auto image = button->grab().toImage();
    const double backdrop = luminance(background);
    double result = 1;
    const int inset = qRound(7 * image.devicePixelRatio());
    for (int y = inset; y < image.height() - inset; ++y)
      for (int x = inset; x < image.width() - inset; ++x) {
        const auto foreground = luminance(image.pixelColor(x, y));
        result = std::max(result, (std::max(foreground, backdrop) + .05) / (std::min(foreground, backdrop) + .05));
      }
    return result;
  };
  for (auto* button : {primary, danger, enable, repair}) {
    uwf::test::moveMouse(host.windowHandle(), other->mapTo(&host, other->rect().center()));
    QTRY_VERIFY(!button->underMouse());
    const QColor normal = surface(button);
    QVERIFY(contrast(button, normal) >= 4.5);
    uwf::test::moveMouse(host.windowHandle(), button->mapTo(&host, button->rect().center()));
    QTRY_VERIFY(button->underMouse());
    QTRY_VERIFY(surface(button) != normal);
    const QColor hovered = surface(button);
    QVERIFY(hovered != normal);
    QVERIFY(contrast(button, hovered) >= 4.5);
    QTest::mousePress(button, Qt::LeftButton);
    if (button == primary || button == danger) QTRY_VERIFY(surface(button) != hovered);
    const QColor pressed = surface(button);
    QVERIFY(contrast(button, pressed) >= 4.5);
    QTest::mouseRelease(button, Qt::LeftButton);
  }
  other->setFocus();
  QTRY_VERIFY(other->hasFocus());
  const auto focusImage = [&] { return host.grab(QRect(toggle->mapTo(&host, QPoint()), toggle->size()).adjusted(-6, -6, 6, 6)).toImage(); };
  const auto before = focusImage();
  const QSize originalSize = toggle->size();
  QTest::keyClick(other, Qt::Key_Tab);
  QTRY_VERIFY(toggle->hasFocus());
  QVERIFY(focusImage() != before);
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
    QVERIFY(focusImage() != focused);
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
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    for (const auto labels = dialog->findChildren<ui::ContentHintBox*>(); auto* label : labels) {
      warningSelectable = warningSelectable || label->textInteractionFlags().testFlag(Qt::TextSelectableByMouse);
    }
    dialog->accept();
  });
  ui::dialogs::warning(nullptr, QStringLiteral("Diagnostic"), QStringLiteral("A selectable provider error"));
  QVERIFY(warningSelectable);

  bool cancelWasDefault = false;
  QTimer::singleShot(0, this, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog) return;
    if (auto* box = dialog->findChild<QDialogButtonBox*>()) {
      for (const auto buttons = box->buttons(); auto* button : buttons) {
        if (box->buttonRole(button) == QDialogButtonBox::RejectRole) {
          cancelWasDefault = qobject_cast<QPushButton*>(button)->isDefault();
          QTest::mouseClick(button, Qt::LeftButton);
          return;
        }
      }
    }
    dialog->reject();
  });
  QVERIFY(!ui::dialogs::confirm(nullptr, QStringLiteral("Confirm"), QStringLiteral("Dangerous operation")));
  QVERIFY(cancelWasDefault);
}

void ApplicationAppearanceTests::hintAndUsageWidgetsHandleEmptyOverflowAndThresholdEdges() {
  QScrollArea host;
  host.setWidgetResizable(true);
  host.resize(240, 160);
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
  host.resize(480, 160);
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
  QVERIFY(thresholds != empty);

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
    QCOMPARE(banner.heightForWidth(width), required);
    QImage second(banner.size(), QImage::Format_ARGB32_Premultiplied);
    second.fill(Qt::transparent);
    banner.render(&second);
    // 只改变末行，末行所在区域必须实际绘制变化，不能仅验证控件返回的高度。
    const int lastLineHeight = 2 * banner.fontMetrics().height();
    const QRect lastLine(0, required - lastLineHeight, width, lastLineHeight);
    QVERIFY(first.copy(lastLine) != second.copy(lastLine));
  }
  const auto errorImage = banner.grab().toImage();
  banner.setTone(ui::Notice::Tone::Warning);
  QVERIFY(banner.grab().toImage() != errorImage);
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
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  const auto inspect = [&](QDialog& dialog, const QString& name, const bool boundedContent = false) {
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    const auto* initialFooter = dialog.findChild<QFrame*>(QStringLiteral("dialogFooter"));
    const int footerHeight = initialFooter && initialFooter->isVisible() ? initialFooter->height() : 0;
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
        for (const auto boxes = footer->findChildren<QDialogButtonBox*>(); const auto* box : boxes)
          debug << "buttons" << box->geometry() << "preferred" << box->sizeHint() << "minimum" << box->minimumSize();
        for (const auto labels = footer->findChildren<QLabel*>(); const auto* label : labels)
          debug << "hint" << label->geometry() << "minimum" << label->minimumSize();
      }
      for (const auto areas = dialog.findChildren<QAbstractScrollArea*>(); const auto* area : areas) {
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
    const QSize workArea = boundedContent ? dialog.screen()->availableGeometry().size() : QSize(1200, 700);
    dialog.resize(dialog.size().boundedTo(workArea));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QTRY_VERIFY2_WITH_TIMEOUT(test::clippedText(dialog).isEmpty(), qUtf8Printable(test::clippedText(dialog) + geometryDescription()), 2000);
    if (!screenshots.isEmpty()) {
      QVERIFY(QDir().mkpath(screenshots));
      QVERIFY(uwf::test::screenshot(dialog).save(QDir(screenshots).filePath(name + "-" + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
      QVERIFY(
          uwf::test::windowScreenshot(dialog).save(QDir(screenshots).filePath("native-" + name + "-" + QString::fromLatin1(QTest::currentDataTag()) + ".png")));
    }
    QVERIFY2(dialog.width() <= workArea.width() && dialog.height() <= workArea.height(),
             qUtf8Printable(QStringLiteral("%1 exceeds compact work area: %2x%3").arg(name).arg(dialog.width()).arg(dialog.height())));
    if (const auto* footer = dialog.findChild<QFrame*>(QStringLiteral("dialogFooter")); footer && footer->isVisible()) {
      QCOMPARE(footer->height(), footerHeight);
      for (const auto boxes = footer->findChildren<QDialogButtonBox*>(); const auto* box : boxes) {
        if (!box->isVisibleTo(footer)) continue;
        QVERIFY2(footer->rect().contains(QRect(box->mapTo(footer, QPoint()), box->size())), qUtf8Printable(name));
        QVERIFY(std::abs(box->mapTo(footer, box->rect().center()).y() - footer->rect().center().y()) <= 1);
      }
      for (const auto labels = footer->findChildren<QLabel*>(); const auto* label : labels) {
        if (!label->isVisibleTo(footer)) continue;
        QVERIFY(footer->rect().contains(QRect(label->mapTo(footer, QPoint()), label->size())));
        QVERIFY(std::abs(label->mapTo(footer, label->rect().center()).y() - footer->rect().center().y()) <= 1);
        QVERIFY(label->alignment().testFlag(Qt::AlignVCenter));
      }
    }
    for (const auto buttons = dialog.findChildren<QPushButton*>(); auto* button : buttons) {
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
    if (name.startsWith(QStringLiteral("enhanced-"))) {
      auto* close = buttonWithText(&dialog, ui::I18n::tr("Close"));
      QVERIFY(close);
      dialog.activateWindow();
      QVERIFY(QTest::qWaitForWindowActive(&dialog));
      close->setFocus(Qt::TabFocusReason);
      QTest::keyClick(close, Qt::Key_Tab);
      QTRY_VERIFY(!close->hasFocus());
      // The shared focus indicator paints outside the control's own rectangle.
      const QRect focusBounds = QRect(close->mapTo(&dialog, QPoint()), close->size()).adjusted(-6, -6, 6, 6).intersected(dialog.rect());
      const auto unfocused = dialog.grab(focusBounds).toImage();
      for (int stop = 0; stop < 20 && !close->hasFocus(); ++stop) QTest::keyClick(QApplication::focusWidget(), Qt::Key_Tab);
      QVERIFY(close->hasFocus());
      QTRY_VERIFY(dialog.grab(focusBounds).toImage() != unfocused);
      QVERIFY(uwf::test::saveScreenshot(dialog, name + QStringLiteral("-close-focus")));
    }
    dialog.hide();
  };
  const auto inspectModal = [&](const QString& name, const std::function<void()>& open, const bool boundedContent = false) {
    bool inspected = false;
    QTimer::singleShot(0, this, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      QVERIFY(dialog);
      const auto close = qScopeGuard([dialog] { dialog->reject(); });
      inspect(*dialog, name, boundedContent);
      inspected = true;
    });
    open();
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
    RecordingWmiOperations wmi;
    MemoryFileStagingStore store;
    QWidget parent;
    auto coordinatorOwner = uwf::bootstrap::createFileStagingCoordinator(wmi, store, uwf::app::UwfCapability::Unavailable, &parent);
    auto& coordinator = *coordinatorOwner;
    coordinator.requestCommit([](const app::FileStagingBatchResult&) {});
    auto* progress = parent.findChild<QDialog*>(QStringLiteral("fileStagingProgressDialog"));
    QVERIFY(progress);
    inspect(*progress, QStringLiteral("file-staging-progress"));
    QVERIFY(wmi.invocations.empty());
    QCOMPARE(store.writes, 0);
  }
  {
    ui::SystemInformationDialog dialog(presentation::DiagnosticReportViewModel(QStringLiteral("[Mock system]\nos=Windows 11\n[Mock token]\nelevated=false\n")));
    inspect(dialog, QStringLiteral("system-information"));
  }
  {
    RecordingWmiOperations wmi;
    FileDialogStub files;
    auto snapshot = editableSnapshot();
    presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
    ui::GlobalStatusPanel global(configuration.global());
    global.viewModel().setData(snapshot.current, snapshot.next, snapshot.runtime);
    QVERIFY(global.viewModel().importFilterEnabled(false));
    ui::ApplyPlanDialog dialog(uwf::bootstrap::createViewModelFactory(wmi)->applyPlan(configuration.pendingChanges(), snapshot), files);
    inspect(dialog, QStringLiteral("apply-plan"));
    QVERIFY(wmi.invocations.empty());
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
    RecordingWmiOperations wmi;
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
    app::EnhancedModeManager manager(controlOwner, wmi, uwf::app::UwfCapability::Unavailable);
    ui::EnhancedModeDialog dialog(uwf::bootstrap::createViewModelFactory()->enhancedMode(manager));
    inspect(dialog, QStringLiteral("enhanced-%1").arg(static_cast<int>(state)), true);
    QVERIFY(wmi.invocations.empty());
    QCOMPARE(control.installs + control.starts + control.removals, 0);
  }
  class OfflineRegistryBrowser final : public app::RegistryBrowser {
   public:
    std::vector<std::string> rootHiveLongNames() const override { return {"HKEY_LOCAL_MACHINE"}; }
    std::vector<std::string> subkeyNames(std::string_view key) const override {
      return key == "HKEY_LOCAL_MACHINE" ? std::vector<std::string>{"SOFTWARE"} : std::vector<std::string>{};
    }
    bool hasSubkeys(std::string_view key) const override { return key == "HKEY_LOCAL_MACHINE"; }
    std::vector<uwf::app::RegistryValueInfo> values(std::string_view) const override { return {{"MockValue", REG_DWORD, std::vector<uint8_t>{1, 0, 0, 0}}}; }
  } browser;
  for (const auto mode :
       {ui::RegistryPickerDialog::Mode::CommitValue, ui::RegistryPickerDialog::Mode::DeleteValue, ui::RegistryPickerDialog::Mode::Exclusion}) {
    ui::RegistryPickerDialog dialog(std::make_shared<uwf::presentation::RegistryPickerViewModel>(mode, browser), ui::I18n::tr("Commit registry changes"));
    dialog.preselectKey(QStringLiteral("HKLM\\SOFTWARE"));
    inspect(dialog, QStringLiteral("registry-%1").arg(static_cast<int>(mode)));
  }
  for (const bool failure : {false, true}) {
    FileDialogStub files;
    ui::OverlayFilesDialog dialog(
        std::make_shared<uwf::presentation::OverlayFilesViewModel>(QStringLiteral("C:"),
                                                                   [failure](const QString&, std::stop_token) -> QVector<app::OverlayFileEntry> {
                                                                     if (failure) throw std::runtime_error("Mock provider error");
                                                                     return {{QStringLiteral("\\Mock.txt"), {}, false, false, 1024}};
                                                                   }),
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
          [&] { QCOMPARE(ui::runPowerActionDialog(nullptr, {action, {}, {}, mode}), app::PowerActionDialogOutcome::Canceled); }, true);
    }
    inspectModal(
        QStringLiteral("power-failure-%1").arg(static_cast<int>(action)),
        [&] {
          const auto outcome = ui::runPowerActionDialog(nullptr, {action, {}, app::PowerStagingFailure{QStringLiteral("Mock file staging failure")}});
          QVERIFY(outcome == app::PowerActionDialogOutcome::Canceled || outcome == app::PowerActionDialogOutcome::CanceledAfterStagingFailure);
        },
        true);
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
  const QString target = QStringLiteral("C:\\") + QString(180, u'x') + QStringLiteral("\\<literal>&data.txt");
  const QString details = (QStringLiteral("WMI: 0x80041003 — mock failure; no operation performed.\n") + target + "\n").repeated(25);
  for (int purpose = 0; purpose < 2; ++purpose) {
    bool inspected = false;
    QTimer::singleShot(0, this, [&] {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      if (!dialog) return;
      const auto close = qScopeGuard([dialog] { dialog->reject(); });
      dialog->resize(800, 500);
      QTRY_COMPARE(dialog->size(), QSize(800, 500));
      QTRY_VERIFY2_WITH_TIMEOUT(test::clippedText(*dialog).isEmpty(), qUtf8Printable(test::clippedText(*dialog)), 2000);
      auto* scroll = dialog->findChild<QScrollArea*>();
      QVERIFY(scroll);
      QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
      QTRY_VERIFY(scroll->verticalScrollBar()->maximum() > 0);
      const auto texts = dialog->findChildren<ui::ContentHintBox*>();
      QVERIFY(!texts.isEmpty());
      QVERIFY(std::ranges::any_of(texts, [&](const auto* text) { return text->toPlainText().contains(target); }));
      if (purpose >= 2) QVERIFY(std::ranges::any_of(texts, [&](const auto* text) { return text->toPlainText() == target; }));
      for (const auto boxes = dialog->findChildren<QDialogButtonBox*>(); auto* box : boxes) {
        QVERIFY(dialog->rect().contains(QRect(box->mapTo(dialog, QPoint()), box->size())));
        QVERIFY(!box->visibleRegion().isEmpty());
      }
      QVERIFY(uwf::test::saveScreenshot(*dialog, QStringLiteral("long-message-%1-top").arg(purpose)));
      auto* scrollbar = scroll->verticalScrollBar();
      QTest::keyClick(scrollbar, Qt::Key_End);
      QTRY_COMPARE(scrollbar->value(), scrollbar->maximum());
      QVERIFY(uwf::test::saveScreenshot(*dialog, QStringLiteral("long-message-%1-bottom").arg(purpose)));
      inspected = true;
    });
    switch (purpose) {
      case 0:
        ui::dialogs::warning(nullptr, ui::I18n::tr("Error"), details);
        break;
      case 1:
        QVERIFY(!ui::dialogs::confirm(nullptr, ui::I18n::tr("Confirm"), details));
        break;
    }
    QVERIFY(inspected);
  }
}

void ApplicationAppearanceTests::textRenderingProducesValidGlyphsAndSmoothEdges() {
  auto& theme = ui::ThemeManager::instance();
  theme.apply(theme.current());
  QLabel label(QStringLiteral("UWF Manager · 文件暂存 · 0123456789"));
  label.setStyleSheet("QLabel { background: white; color: black; padding: 0; border: 0; }");
  label.resize(600, 80);
  label.show();
  QVERIFY(QTest::qWaitForWindowExposed(&label));
  for (const auto weight : {QFont::Normal, QFont::Medium, QFont::DemiBold}) {
    for (const auto& text : {QStringLiteral("UWF Manager · 0123456789"), QStringLiteral("全局设置 · 筛选器 · 覆盖层")}) {
      label.setText(text);
      QFont font = label.font();
      font.setWeight(weight);
      label.setFont(font);
      const auto image = label.grab().toImage().convertToFormat(QImage::Format_RGB32);
      int intermediatePixels = 0;
      for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x) {
          const QRgb pixel = image.pixel(x, y);
          if (pixel != qRgb(255, 255, 255) && pixel != qRgb(0, 0, 0)) ++intermediatePixels;
        }
      QVERIFY2(intermediatePixels > 0, "real QLabel rendering must contain antialiased edges, including semibold text");
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
        qInfo() << "typography" << label.devicePixelRatioF() << QFontInfo(font).family() << weight << run.rawFont().familyName() << run.rawFont().styleName();
      }
      const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
      if (!screenshots.isEmpty()) {
        QVERIFY(QDir().mkpath(screenshots));
        const QString name = QStringLiteral("typography-%1-%2.png").arg(static_cast<int>(weight)).arg(text.startsWith("UWF") ? "en" : "zh");
        QVERIFY(uwf::test::screenshot(label).save(QDir(screenshots).filePath(name)));
        label.repaint();
        QTest::qWait(100);
        QVERIFY(uwf::test::nativeScreenshot(label).save(QDir(screenshots).filePath("native-" + name)));
      }
    }
  }
  ui::GlobalStatusPanel panel(std::make_shared<uwf::presentation::GlobalConfigurationViewModel>(uwf::windows::systemTotalRamMb()));
  panel.viewModel().setData(editableSnapshot().current, editableSnapshot().next, editableSnapshot().runtime);
  panel.show();
  QVERIFY(QTest::qWaitForWindowExposed(&panel));
  auto* title = panel.findChild<QLabel*>(QStringLiteral("panelTitle"));
  QVERIFY(title);
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!screenshots.isEmpty()) {
    QTest::qWait(100);
    QVERIFY(uwf::test::nativeScreenshot(panel).save(QDir(screenshots).filePath(QStringLiteral("native-settings.png"))));
  }
}

QTEST_MAIN(ApplicationAppearanceTests)
#include "AppearanceCases.moc"
