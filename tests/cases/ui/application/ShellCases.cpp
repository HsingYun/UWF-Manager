// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <qt_windows.h>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QLocale>
#include <QMainWindow>
#include <QMenu>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSpinBox>
#include <QStyleHints>
#include <QSystemTrayIcon>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QWidgetAction>
#include <QWindow>
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
#include "app/ports/FileStagingStore.h"
#include "app/ports/VolumePaths.h"
#include "app/registry/RegistryTreeCommitter.h"
#include "app/settings/UserConfig.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "bootstrap/MainWindowComposition.h"
#include "bootstrap/ViewModelComposition.h"
#include "bootstrap/WorkflowComposition.h"
#include "core/UwfModel.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PageProxyModel.h"
#include "presentation/configuration/ImportViewModel.h"
#include "presentation/staging/FileStagingResultText.h"
#include "shared/Log.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/ViewMetrics.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/WidgetCapture.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/appearance/WindowAppearance.h"
#include "ui/configuration/ExclusionListWidget.h"
#include "ui/configuration/GlobalStatusPanel.h"
#include "ui/configuration/ImportDialog.h"
#include "ui/configuration/VolumeStatusPanel.h"
#include "ui/configuration/VolumeTab.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/overlay/OverlayContextMenu.h"
#include "ui/overlay/OverlayFloatingWidget.h"
#include "ui/overlay/OverlayHubView.h"
#include "ui/overlay/OverlayHudPalette.h"
#include "ui/overlay/OverlayPresentationController.h"
#include "ui/overlay/OverlayTaskbarWidget.h"
#include "ui/shell/MainWindow.h"
#include "ui/shell/StatusSurface.h"
#include "ui/shell/TrayController.h"
#include "ui/shell/WindowChromeController.h"
#include "ui/staging/FileStagingWidget.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/EntryListView.h"
#include "ui/widgets/PagedViewController.h"
#include "ui/widgets/PagedWindowLayout.h"
#include "ui/widgets/SwitchButton.h"
#include "windows/ipc/EnhancedModeAgent.h"
#include "windows/system/SystemHardwareInfo.h"
#include "windows/system/WindowsShell.h"

class ApplicationShellTests final : public ApplicationUiTest {
  Q_OBJECT
 private slots:
  void languageSwitchPreservesRecommendedDraftAndViewState_data();
  void languageSwitchPreservesRecommendedDraftAndViewState();
  void startupPreferencesReachTheRealUiWithoutPersistingToolbarChanges_data();
  void startupPreferencesReachTheRealUiWithoutPersistingToolbarChanges();
  void startupHiddenHubRemainsHiddenAcrossRefreshAndUiRebuild_data();
  void startupHiddenHubRemainsHiddenAcrossRefreshAndUiRebuild();
  void mainWindowQuietStartupInitializesWhileRemainingHidden();
  void mainWindowStartupGeometryIsStableFromTheFirstNativeShow_data();
  void mainWindowStartupGeometryIsStableFromTheFirstNativeShow();
  void mainWindowDistinguishesInitialFailureFromCommittedUnavailableState();
  void mainWindowCompactLayoutKeepsControlsReachable_data();
  void mainWindowCompactLayoutKeepsControlsReachable();
  void mainWindowCompactToolbarKeepsLanguageMenuUsable();
  void contextMenusKeepTextRenderingConsistent_data();
  void contextMenusKeepTextRenderingConsistent();
  void overlayMenusRouteActionsAndDeferBlockingDispatch();
  void trayUsageCardKeepsValuesAndTooltipsConsistent_data();
  void trayUsageCardKeepsValuesAndTooltipsConsistent();
};

using namespace uwf;
using namespace uwf::test::application;

namespace {
class StartupGeometryProbe final : public ui::MainWindow {
 public:
  using MainWindow::MainWindow;
  QList<QRect> visibleFrames;

 protected:
  bool nativeEvent(const QByteArray& type, void* message, qintptr* result) override {
    const auto* native = static_cast<const MSG*>(message);
    if ((native->message == WM_SHOWWINDOW && native->wParam) || (native->message == WM_WINDOWPOSCHANGED && IsWindowVisible(native->hwnd))) {
      RECT frame{};
      if (GetWindowRect(native->hwnd, &frame)) {
        const QRect bounds(frame.left, frame.top, frame.right - frame.left, frame.bottom - frame.top);
        if (visibleFrames.isEmpty() || visibleFrames.last() != bounds) visibleFrames.append(bounds);
      }
    }
    return MainWindow::nativeEvent(type, message, result);
  }
};
}  // namespace

void ApplicationShellTests::startupPreferencesReachTheRealUiWithoutPersistingToolbarChanges_data() {
  QTest::addColumn<QByteArray>("contents");
  QTest::addColumn<std::optional<app::Theme>>("expectedTheme");
  QTest::addColumn<std::optional<ui::I18n::Lang>>("expectedLanguage");
  struct Case {
    const char* name;
    QByteArray contents;
    std::optional<app::Theme> theme;
    std::optional<ui::I18n::Lang> language;
  };
  const std::array cases{
      Case{"dark-english", "[UI]\nTheme=dark\nLanguage=en\n", app::Theme::Dark, ui::I18n::Lang::En},
      Case{"light-chinese", "[UI]\nTheme=light\nLanguage=zh_CN\n", app::Theme::Light, ui::I18n::Lang::Zh_CN},
      Case{"dark-chinese", "[UI]\nTheme=dark\nLanguage=zh_CN\n", app::Theme::Dark, ui::I18n::Lang::Zh_CN},
      Case{"dark-french", "[UI]\nTheme=dark\nLanguage=fr\n", app::Theme::Dark, ui::I18n::Lang::Fr},
      Case{"light-arabic", "[UI]\nTheme=light\nLanguage=ar\n", app::Theme::Light, ui::I18n::Lang::Ar},
      Case{"light-english", "[UI]\nTheme=light\nLanguage=en\n", app::Theme::Light, ui::I18n::Lang::En},
      Case{"system", "[UI]\nTheme=system\nLanguage=system\n", {}, {}},
      Case{"empty", {}, {}, {}},
      Case{"invalid", "[UI]\nTheme=unknown\nLanguage=unknown\n", {}, {}},
      Case{"malformed", "[UI]\nTheme=dark\nLanguage=zh_CN\n[unterminated\n", {}, {}},
      Case{"partly-valid", "[UI]\nTheme=bad-value\nLanguage=zh_CN\n", {}, ui::I18n::Lang::Zh_CN},
  };
  for (const auto& entry : cases) QTest::newRow(entry.name) << entry.contents << entry.theme << entry.language;
}

void ApplicationShellTests::startupPreferencesReachTheRealUiWithoutPersistingToolbarChanges() {
  QFETCH(QByteArray, contents);
  QFETCH(std::optional<app::Theme>, expectedTheme);
  QFETCH(std::optional<ui::I18n::Lang>, expectedLanguage);
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFile file(directory.filePath(QStringLiteral("UWF.ini")));
  QVERIFY(file.open(QIODevice::WriteOnly));
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  const auto modifiedBefore = QFileInfo(file).lastModified();

  auto& theme = ui::ThemeManager::instance();
  auto& language = ui::I18n::instance();
  const app::Theme originalTheme = theme.current();
  const ui::I18n::Lang originalLanguage = language.lang();
  const auto restorePreferences = qScopeGuard([&] {
    theme.apply(originalTheme);
    language.setLang(originalLanguage);
  });
  const auto config = app::readUserConfig(directory.path());
  theme.applyStartupTheme(config.theme);
  language.applyStartupLanguage(config.language);
  const app::Theme startupTheme = expectedTheme ? *expectedTheme : ui::ThemeManager::detectSystemTheme();
  const ui::I18n::Lang startupLanguage = expectedLanguage ? *expectedLanguage : ui::I18n::detectSystemLang();
  QCOMPARE(theme.current(), startupTheme);
  QCOMPARE(language.lang(), startupLanguage);
  QCOMPARE(QGuiApplication::styleHints()->colorScheme(), startupTheme == app::Theme::Light ? Qt::ColorScheme::Light : Qt::ColorScheme::Dark);

  {
    RecordingWmiOperations wmi;
    auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
    auto& staging = *stagingOwner;
    MutableApplicationStateSource source;
    auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
    MemoryEnhancedModeAgent agent;
    source.snapshot.uwfAvailable = false;
    source.snapshot.unavailableReason = "provider unavailable in fixture";
    // 所有系统能力均停留在既有内存边界，真实窗口不会触及 UWF、SCM 或代理管道。
    ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner, serviceControlOwner, &agent),
                          {.uwfCapability = uwf::app::UwfCapability::Unavailable, .compatibilityMode = false, .hubVisible = config.hubVisible});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 2000);

    QTRY_VERIFY(refreshCompleted(window));
    QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 2000);
    const QString expectedTitle = startupLanguage == ui::I18n::Lang::En      ? QStringLiteral("Unified Write Filter (UWF) Manager")
                                  : startupLanguage == ui::I18n::Lang::Zh_CN ? QStringLiteral("统一写入筛选器 (UWF) 管理器")
                                                                             : ui::I18n::applicationTitle();
    const QString expectedRefresh = startupLanguage == ui::I18n::Lang::En      ? QStringLiteral("Refresh")
                                    : startupLanguage == ui::I18n::Lang::Zh_CN ? QStringLiteral("刷新")
                                                                               : ui::I18n::tr("Refresh");
    QCOMPARE(window.windowTitle(), expectedTitle);
    QVERIFY(actionWithText(&window, expectedRefresh));
    QCOMPARE(window.palette().color(QPalette::Window), theme.color(ui::Sem::Bg));
    QVERIFY(!QApplication::activeModalWidget());

    QToolButton* themeButton = nullptr;
    for (const auto buttons = window.findChildren<QToolButton*>(); auto* button : buttons) {
      if (button->defaultAction() && button->defaultAction()->toolTip() == ui::I18n::tr("Toggle light / dark theme")) {
        themeButton = button;
      }
    }
    QVERIFY(themeButton && themeButton->isVisible() && themeButton->isEnabled());
    QVERIFY(!themeButton->icon().isNull());
    QSignalSpy themeChanges(&theme, &ui::ThemeManager::themeChanged);
    QTest::mouseClick(themeButton, Qt::LeftButton);
    const app::Theme toggledTheme = startupTheme == app::Theme::Dark ? app::Theme::Light : app::Theme::Dark;
    QTRY_COMPARE_WITH_TIMEOUT(theme.current(), toggledTheme, 2000);
    QCOMPARE(source.reads, 1);
    QCOMPARE(language.lang(), startupLanguage);
    QVERIFY(window.updatesEnabled());
    QCOMPARE(window.palette().color(QPalette::Window), theme.color(ui::Sem::Bg));

    const ui::I18n::Lang toggledLanguage = startupLanguage == ui::I18n::Lang::En ? ui::I18n::Lang::Zh_CN : ui::I18n::Lang::En;
    QToolButton* languageButton = nullptr;
    for (const auto buttons = window.findChildren<QToolButton*>(); auto* button : buttons) {
      if (button->defaultAction() && button->defaultAction()->toolTip() == ui::I18n::tr("Switch display language")) languageButton = button;
    }
    QVERIFY(languageButton && languageButton->isVisible() && languageButton->isEnabled() && languageButton->defaultAction()->menu());
    const QPointer<QMenu> menu = languageButton->defaultAction()->menu();
    QPointer<QAction> selectedLanguage;
    for (const auto actions = menu->actions(); auto* action : actions) {
      if (action->text() == (toggledLanguage == ui::I18n::Lang::En ? QStringLiteral("English") : QStringLiteral("简体中文"))) selectedLanguage = action;
    }
    QVERIFY(selectedLanguage && selectedLanguage->isEnabled() && !selectedLanguage->isChecked());
    bool menuOpened = false;
    QTimer menuDriver;
    menuDriver.setInterval(1);
    connect(&menuDriver, &QTimer::timeout, this, [&] {
      if (!menu || !selectedLanguage || !menu->isVisible()) return;
      menuDriver.stop();
      menuOpened = true;
      auto* list = menu->findChild<QListView*>(QStringLiteral("scrollableMenuItems"));
      QVERIFY(list);
      for (int row = 0; row < list->model()->rowCount(); ++row) {
        const auto index = list->model()->index(row, 0);
        if (index.data().toString() != selectedLanguage->text()) continue;
        list->setCurrentIndex(index);
        list->scrollTo(index);
        const auto position = list->visualRect(index).center();
        uwf::test::moveMouse(*list->viewport(), position);
        QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, position);
        return;
      }
      QFAIL("The language action is missing from the scrollable menu");
    });
    // InstantPopup 会进入嵌套事件循环。超时只关闭本进程的弹出菜单，让按钮
    // 接线或选择行为错误表现为断言失败，而不是让整套测试永久等待。
    QTimer menuTimeout;
    menuTimeout.setSingleShot(true);
    connect(&menuTimeout, &QTimer::timeout, this, [] {
      if (auto* popup = QApplication::activePopupWidget()) popup->close();
    });
    menuDriver.start();
    menuTimeout.start(2000);
    // 重建会销毁工具栏按钮，窗口本身保持不变；窗口级事件入口可安全发送整次
    // 点击的按下/释放事件，不会在弹出菜单返回后再访问已销毁的旧按钮。
    const QPoint languageButtonPosition = window.mapFromGlobal(languageButton->mapToGlobal(languageButton->rect().center()));
    QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, languageButtonPosition);
    menuDriver.stop();
    menuTimeout.stop();
    QVERIFY(menuOpened);
    QTRY_COMPARE_WITH_TIMEOUT(language.lang(), toggledLanguage, 2000);
    QCOMPARE(source.reads, 1);
    QCOMPARE(window.windowTitle(),
             toggledLanguage == ui::I18n::Lang::En ? QStringLiteral("Unified Write Filter (UWF) Manager") : QStringLiteral("统一写入筛选器 (UWF) 管理器"));
    QVERIFY(actionWithText(&window, toggledLanguage == ui::I18n::Lang::En ? QStringLiteral("Refresh") : QStringLiteral("刷新")));
    QCOMPARE(theme.current(), toggledTheme);
    QCOMPARE(themeChanges.count(), 1);
    QCOMPARE(window.palette().color(QPalette::Window), theme.color(ui::Sem::Bg));
    QVERIFY(window.updatesEnabled());
    QVERIFY(wmi.invocations.empty());
    QCOMPARE(staging.writes, 0);
    QCOMPARE(agent.starts, 0);
  }

  QVERIFY(file.open(QIODevice::ReadOnly));
  QCOMPARE(file.readAll(), contents);
  file.close();
  QCOMPARE(QFileInfo(file).lastModified(), modifiedBefore);
  QCOMPARE(QDir(directory.path()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System), QStringList{QStringLiteral("UWF.ini")});
}

void ApplicationShellTests::languageSwitchPreservesRecommendedDraftAndViewState_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("filterEnabled");
  for (const auto& language : ui::I18n::languages()) {
    for (const bool dark : {false, true}) {
      for (const bool filterEnabled : {false, true}) {
        if (filterEnabled && language.id != ui::I18n::Lang::En && language.id != ui::I18n::Lang::Zh_CN && language.id != ui::I18n::Lang::Ar) continue;
        const auto name = QString(language.code).toLatin1() + (dark ? "-dark" : "-light") + (filterEnabled ? "-filter-on" : "-filter-off");
        QTest::newRow(name.constData()) << language.id << dark << filterEnabled;
      }
    }
  }
}

void ApplicationShellTests::languageSwitchPreservesRecommendedDraftAndViewState() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  QFETCH(bool, filterEnabled);
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto previousTheme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(previousLanguage);
    ui::ThemeManager::instance().apply(previousTheme);
  });
  ui::I18n::instance().setLang(locale);
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  const auto target = locale == ui::I18n::Lang::En ? ui::I18n::Lang::Zh_CN : ui::I18n::Lang::En;
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\Pending\\keep.txt")},
                     {app::FileStagingKind::File, QStringLiteral("C:\\Pending\\remove.txt")}};
  const auto savedStaging = staging.entries;
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.current.filter.enabled = filterEnabled;
  source.snapshot.next.filter.enabled = false;
  source.snapshot.current.persistDomainSecretKey = !filterEnabled;
  source.snapshot.next.persistDomainSecretKey = false;
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner),
                        {.uwfCapability = app::UwfCapability::Available, .compatibilityMode = false, .hubVisible = false});
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  bool timedOut = false;
  QTimer timeout;
  timeout.setSingleShot(true);
  connect(&timeout, &QTimer::timeout, this, [&] {
    timedOut = true;
    if (auto* popup = QApplication::activePopupWidget()) popup->close();
    if (auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget())) dialog->reject();
  });
  bool imported = false;
  QTimer::singleShot(0, &window, [&] {
    auto* dialog = qobject_cast<ui::ImportDialog*>(QApplication::activeModalWidget());
    QVERIFY(dialog);
    auto* editor = dialog->findChild<QTextEdit*>(QStringLiteral("importTextEdit"));
    auto* import = buttonWithText(dialog, ui::I18n::tr("Import configuration"));
    QVERIFY(editor && import);
    editor->setPlainText(presentation::ImportViewModel{}.defaultRulesText(presentation::ImportViewModel::AllRecommendedSections));
    import->click();
    imported = true;
    dialog->reject();
  });
  timeout.start(5000);
  window.showImport();
  timeout.stop();
  QVERIFY(!timedOut && imported);
  window.raise();
  window.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&window));
  const QPointer<ui::GlobalStatusPanel> global = window.findChild<ui::GlobalStatusPanel*>();
  const QPointer<ui::VolumeTab> volume = window.findChild<ui::VolumeTab*>();
  QVERIFY(global && volume);
  const auto addedFiles = volume->viewModel().files()->pendingAdded();
  const auto addedRegistry = volume->viewModel().registry()->pendingAdded();
  auto* registry = volume->viewModel().registry().get();
  QVERIFY(registry);
  if (filterEnabled) registry->enablePersistence(app::ExclusionEditor::PersistenceItem::DomainSecretKey);
  const auto pendingDomainSecret = volume->viewModel().registry()->pendingPersistDomainSecretKey();
  if (filterEnabled) QCOMPARE(pendingDomainSecret, std::optional<bool>(true));
  const auto filterPending = global->viewModel().pendingFilterEnabled();
  const auto overlayPending = global->viewModel().pendingOverlay();
  auto* staged = volume->findChild<ui::FileStagingWidget*>();
  QVERIFY(staged);
  staged->viewModel().removePaths({QStringLiteral("C:\\Pending\\remove.txt")});
  QCOMPARE(staged->viewModel().entries().size(), 1);
  const auto pendingStaging = staged->viewModel().entries();
  ui::ExclusionListWidget* files = nullptr;
  for (const auto widgets = volume->findChildren<ui::ExclusionListWidget*>(); auto* widget : widgets)
    if (widget->viewModel().kind() == ui::ExclusionListWidget::Kind::File) files = widget;
  QVERIFY(files);
  auto* list = static_cast<ui::EntryListView*>(files->findChild<QListView*>(QStringLiteral("exclusionList")));
  auto* search = files->commandWidget()->findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(list && search);
  search->setText(QStringLiteral("Windows"));
  list->pages().goLast();
  QVERIFY(list->model()->rowCount() > 0);
  const auto selected = list->model()->index(list->model()->rowCount() - 1, 0);
  list->selectionModel()->setCurrentIndex(selected, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  list->scrollTo(selected);
  search->setCursorPosition(3);
  const QPersistentModelIndex retainedIndex(selected);
  const int page = list->pages().pager().currentPage;
  const auto innerPage = volume->activeInfoPage();
  const int storageReads = staging.loads;
  QSignalSpy reset(&files->viewModel(), &QAbstractItemModel::modelReset);
  const QPointer<QPushButton> discard = buttonWithText(&window, ui::I18n::tr("Discard changes"));
  QVERIFY(discard && discard->isVisible());
  QToolButton* languageButton = nullptr;
  for (const auto buttons = window.findChildren<QToolButton*>(); auto* button : buttons)
    if (button->defaultAction() && button->defaultAction()->text() == ui::I18n::tr("Language")) languageButton = button;
  QVERIFY(languageButton && languageButton->defaultAction()->menu());
  const QPointer<QMenu> menu = languageButton->defaultAction()->menu();
  QPointer<QAction> targetAction;
  for (const auto actions = menu->actions(); auto* action : actions)
    if (action->isCheckable() && action->data().toInt() == static_cast<int>(target)) targetAction = action;
  QVERIFY(targetAction);
  bool menuSelected = false;
  QTimer menuDriver;
  menuDriver.setInterval(10);
  connect(&menuDriver, &QTimer::timeout, this, [&] {
    if (!menu || !menu->isVisible() || !targetAction) return;
    auto* options = menu->findChild<QListView*>(QStringLiteral("scrollableMenuItems"));
    if (!options) return;
    for (int row = 0; row < options->model()->rowCount(); ++row) {
      const auto index = options->model()->index(row, 0);
      if (index.data().toString() != targetAction->text()) continue;
      menuDriver.stop();
      menuSelected = true;
      options->setCurrentIndex(index);
      options->scrollTo(index);
      const auto position = options->visualRect(index).center();
      uwf::test::moveMouse(*options->viewport(), position);
      QTest::mouseClick(options->viewport(), Qt::LeftButton, Qt::NoModifier, position);
      return;
    }
  });
  timeout.start(5000);
  menuDriver.start();
  uwf::test::moveMouse(*languageButton);
  QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, languageButton->mapTo(&window, languageButton->rect().center()));
  QTRY_VERIFY_WITH_TIMEOUT(menuSelected, 3000);
  QTRY_COMPARE_WITH_TIMEOUT(ui::I18n::instance().lang(), target, 3000);
  timeout.stop();
  QVERIFY(!timedOut && !QApplication::activeModalWidget());
  QCOMPARE(window.windowTitle(), ui::I18n::applicationTitle());
  QCOMPARE(global->findChild<QLabel*>(QStringLiteral("panelTitle"))->text(), ui::I18n::tr("Global settings"));
  QCOMPARE(languageButton->defaultAction()->text(), ui::I18n::tr("Language"));
  QCOMPARE(list->findChild<QLabel*>(QStringLiteral("entryHeading"))->text(), ui::I18n::tr("File or folder"));
  QCOMPARE(buttonWithText(&window, ui::I18n::tr("Discard changes")), discard.data());
  bool persistenceTranslated = false;
  for (int row = 0; row < registry->rowCount(); ++row) {
    const auto index = registry->index(row);
    const auto flag = index.data(presentation::PersistenceRole);
    if (flag.isValid() && flag.toInt() == static_cast<int>(app::ExclusionEditor::PersistenceItem::DomainSecretKey))
      persistenceTranslated = index.data().toString() == ui::I18n::tr("Domain Secret Key persistence");
  }
  QVERIFY(persistenceTranslated);
  ui::I18n::instance().requestLang(locale);
  QTRY_COMPARE(ui::I18n::instance().lang(), locale);
  QTRY_VERIFY(window.findChildren<QWidget*>(QStringLiteral("appearanceTransition")).isEmpty());
  QCOMPARE(window.windowTitle(), ui::I18n::applicationTitle());
  QCOMPARE(global->findChild<QLabel*>(QStringLiteral("panelTitle"))->text(), ui::I18n::tr("Global settings"));
  QCOMPARE(list->findChild<QLabel*>(QStringLiteral("entryHeading"))->text(), ui::I18n::tr("File or folder"));
  QVERIFY(window.isVisible() && window.updatesEnabled());
  QCOMPARE(window.findChild<ui::GlobalStatusPanel*>(), global.data());
  QCOMPARE(window.findChild<ui::VolumeTab*>(), volume.data());
  QCOMPARE(global->viewModel().pendingFilterEnabled(), filterPending);
  QCOMPARE(global->viewModel().pendingOverlay().type, overlayPending.type);
  QCOMPARE(global->viewModel().pendingOverlay().maximumSizeMb, overlayPending.maximumSizeMb);
  QCOMPARE(global->viewModel().pendingOverlay().warningThresholdMb, overlayPending.warningThresholdMb);
  QCOMPARE(global->viewModel().pendingOverlay().criticalThresholdMb, overlayPending.criticalThresholdMb);
  QCOMPARE(volume->viewModel().files()->pendingAdded(), addedFiles);
  QCOMPARE(volume->viewModel().registry()->pendingAdded(), addedRegistry);
  QCOMPARE(volume->viewModel().registry()->pendingPersistDomainSecretKey(), pendingDomainSecret);
  QCOMPARE(staged->viewModel().entries(), pendingStaging);
  QCOMPARE(staging.entries, savedStaging);
  QCOMPARE(search->text(), QStringLiteral("Windows"));
  QCOMPARE(search->cursorPosition(), 3);
  QCOMPARE(volume->activeInfoPage(), innerPage);
  QCOMPARE(list->pages().pager().currentPage, page);
  QVERIFY(retainedIndex.isValid() && list->selectionModel()->isSelected(retainedIndex));
  QCOMPARE(reset.count(), 0);
  QCOMPARE(buttonWithText(&window, ui::I18n::tr("Discard changes")), discard.data());
  QVERIFY(discard->isVisible());
  QCOMPARE(source.reads, 1);
  QCOMPARE(staging.loads, storageReads);
  QVERIFY(wmi.invocations.empty());
  QCOMPARE(staging.writes, 0);
}

void ApplicationShellTests::startupHiddenHubRemainsHiddenAcrossRefreshAndUiRebuild_data() {
  QTest::addColumn<bool>("quiet");
  QTest::addColumn<bool>("filterEnabled");
  QTest::newRow("interactive-filter-enabled") << false << true;
  QTest::newRow("interactive-filter-disabled") << false << false;
  QTest::newRow("quiet-filter-enabled") << true << true;
  QTest::newRow("quiet-filter-disabled") << true << false;
}

void ApplicationShellTests::startupHiddenHubRemainsHiddenAcrossRefreshAndUiRebuild() {
  QFETCH(bool, quiet);
  QFETCH(bool, filterEnabled);
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QFile file(directory.filePath(QStringLiteral("UWF.ini")));
  const QByteArray contents("[UI]\nHubVisible=false\n");
  QVERIFY(file.open(QIODevice::WriteOnly));
  QCOMPARE(file.write(contents), contents.size());
  file.close();
  const auto modifiedBefore = QFileInfo(file).lastModified();
  const auto config = app::readUserConfig(directory.path());

  auto& theme = ui::ThemeManager::instance();
  auto& language = ui::I18n::instance();
  const auto originalTheme = theme.current();
  const auto originalLanguage = language.lang();
  const auto restorePreferences = qScopeGuard([&] {
    theme.apply(originalTheme);
    language.setLang(originalLanguage);
  });
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  MutableApplicationStateSource source;
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  MemoryEnhancedModeAgent agent;
  source.snapshot = editableSnapshot();
  source.snapshot.current.filter.enabled = filterEnabled;
  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner, serviceControlOwner, &agent),
                        {.uwfCapability = uwf::app::UwfCapability::Available, .compatibilityMode = false, .hubVisible = config.hubVisible});
  QList<QPointer<ui::OverlayHubView>> views;
  for (const auto widgets = QApplication::allWidgets(); auto* widget : widgets)
    if (auto* view = qobject_cast<ui::OverlayHubView*>(widget)) views.append(view);
  // 部分系统会静态过滤不兼容的任务栏端点；实际注册的端点均须保持未展示。
  QVERIFY(!views.isEmpty());
  const auto hubIsHidden = [&] {
    auto* action = actionWithText(&window, QStringLiteral("HUB"));
    return action && action->isVisible() == source.snapshot.current.filter.enabled && !action->isChecked() && std::ranges::all_of(views, [](const auto& view) {
             return view && !view->isVisible() && view->displayState() == ui::OverlayHubView::DisplayState::Disabled;
           });
  };
  if (quiet)
    window.startInTray();
  else
    window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 2000);

  QTRY_VERIFY(refreshCompleted(window));
  QCOMPARE(window.isVisible(), !quiet);
  QVERIFY(hubIsHidden());
  if (quiet) {
    window.raiseToFront();
    QTRY_COMPARE_WITH_TIMEOUT(source.reads, 2, 2000);

    QTRY_VERIFY(refreshCompleted(window));
  }
  QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 2000);

  // 请求隐藏保存在展示模型中；UWF 保护状态变化和工具栏重建均不得将其覆盖。
  for (const bool enabled : {false, true}) {
    source.snapshot.current.filter.enabled = enabled;
    const int before = source.reads;
    window.refresh();
    QTRY_VERIFY(refreshCompleted(window));
    QCOMPARE(source.reads, before + 1);
    QVERIFY(hubIsHidden());
  }
  int before = source.reads;
  theme.toggle();
  QCOMPARE(source.reads, before);
  QVERIFY(hubIsHidden());
  auto* languageAction = actionWithText(&window, ui::I18n::tr("Language"));
  QVERIFY(languageAction && languageAction->menu());
  auto* otherLanguage = actionWithText(languageAction->menu(), language.lang() == ui::I18n::Lang::En ? QStringLiteral("简体中文") : QStringLiteral("English"));
  QVERIFY(otherLanguage);
  before = source.reads;
  const auto targetLanguage = static_cast<ui::I18n::Lang>(otherLanguage->data().toInt());
  otherLanguage->trigger();
  QTRY_COMPARE(language.lang(), targetLanguage);
  QCOMPARE(source.reads, before);
  QVERIFY(hubIsHidden());
  QVERIFY(wmi.invocations.empty());
  QCOMPARE(staging.writes, 0);
  QCOMPARE(agent.starts, 0);
  QVERIFY(file.open(QIODevice::ReadOnly));
  QCOMPARE(file.readAll(), contents);
  QCOMPARE(QFileInfo(file).lastModified(), modifiedBefore);
  QCOMPARE(QDir(directory.path()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System), QStringList{QStringLiteral("UWF.ini")});
}

void ApplicationShellTests::mainWindowQuietStartupInitializesWhileRemainingHidden() {
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  MutableApplicationStateSource source;
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;
  source.snapshot.unavailableReason = "embedded provider absent";

  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner),
                        {.uwfCapability = uwf::app::UwfCapability::Unavailable, .compatibilityMode = false});
  window.startInTray();

  QCOMPARE(source.reads, 1);
  QVERIFY(!window.isVisible());
  QVERIFY(!window.isMinimized());
  QVERIFY(window.findChild<QTabWidget*>(QStringLiteral("mainTabs")));
  QCOMPARE(window.windowOpacity(), 1.0);

  // 激活时在原生窗口映射前重新布局，保留正常启动和静默启动相同的首帧准备过程。
  window.raiseToFront();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 2, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  QTRY_VERIFY_WITH_TIMEOUT(window.isVisible(), 1000);
  QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 1000);
}

void ApplicationShellTests::mainWindowStartupGeometryIsStableFromTheFirstNativeShow_data() {
  QTest::addColumn<int>("screenIndex");
  QTest::addColumn<ui::I18n::Lang>("language");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("quiet");
  for (int screen = 0; screen < QApplication::screens().size(); ++screen) {
    for (const auto& locale : ui::I18n::languages()) {
      if (locale.id != ui::I18n::Lang::En && locale.id != ui::I18n::Lang::Zh_CN && locale.id != ui::I18n::Lang::Ar) continue;
      for (const bool dark : {false, true}) {
        for (const bool quiet : {false, true}) {
          const QByteArray name =
              QStringLiteral("screen%1-%2-%3-%4").arg(screen).arg(QString(locale.code)).arg(dark ? "dark" : "light", quiet ? "quiet" : "normal").toLatin1();
          QTest::newRow(name.constData()) << screen << locale.id << dark << quiet;
        }
      }
    }
  }
}

void ApplicationShellTests::mainWindowStartupGeometryIsStableFromTheFirstNativeShow() {
  QFETCH(int, screenIndex);
  QFETCH(ui::I18n::Lang, language);
  QFETCH(bool, dark);
  QFETCH(bool, quiet);
  const auto previousLanguage = ui::I18n::instance().lang();
  const auto previousTheme = ui::ThemeManager::instance().current();
  const auto restore = qScopeGuard([&] {
    ui::I18n::instance().setLang(previousLanguage);
    ui::ThemeManager::instance().apply(previousTheme);
  });
  ui::I18n::instance().setLang(language);
  ui::ThemeManager::instance().apply(dark ? app::Theme::Dark : app::Theme::Light);
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  MutableApplicationStateSource source;
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  MemoryEnhancedModeAgent agent;
  source.snapshot = editableSnapshot();
  source.snapshot.current.filter.enabled = false;
  source.snapshot.next.filter.enabled = false;
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  StartupGeometryProbe window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner, serviceControlOwner, &agent),
                              {.uwfCapability = app::UwfCapability::Available, .compatibilityMode = false, .hubVisible = false});
  if (quiet) {
    window.startInTray();
    QCOMPARE(source.reads, 1);
    QVERIFY(!window.isVisible());
    QVERIFY(window.visibleFrames.isEmpty());
  }
  auto* target = QApplication::screens().at(screenIndex);
  window.setScreen(target);
  window.move(target->availableGeometry().topLeft());
  window.show();
  QCOMPARE(source.reads, quiet ? 2 : 1);
  QVERIFY(window.findChild<QTabWidget*>(QStringLiteral("mainTabs")));
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  QCOMPARE(window.screen(), target);
  QCOMPARE(window.windowOpacity(), 1.0);
  const QRect initial = window.geometry();
  const auto frames = window.visibleFrames;
  qInfo() << "startup" << target->name() << "available" << target->availableGeometry() << "dpr" << window.devicePixelRatioF() << "client" << initial << "frame"
          << window.frameGeometry() << "margins" << window.windowHandle()->frameMargins() << "visible frames" << frames;
  QVERIFY2(!frames.isEmpty(), "No native window show was observed");
  QCOMPARE(frames.size(), 1);
  QVERIFY(target->availableGeometry().contains(window.frameGeometry()));
  QVERIFY(std::abs(window.frameGeometry().center().x() - target->availableGeometry().center().x()) <= 1);
  QVERIFY(std::abs(window.frameGeometry().center().y() - target->availableGeometry().center().y()) <= 1);
  QTest::qWait(150);
  QCOMPARE(window.geometry(), initial);
  QCOMPARE(window.visibleFrames, frames);
  for (const auto views = window.findChildren<QAbstractItemView*>(); const auto* view : views) {
    const int rowHeight = uwf::test::displayedRowHeight(view);
    if (rowHeight > 0 && view->isVisibleTo(&window)) QCOMPARE(view->viewport()->height() % rowHeight, 0);
  }
  if (!quiet && !dark && language == ui::I18n::Lang::En) {
    QAbstractItemView* list = nullptr;
    for (const auto views = window.findChildren<QAbstractItemView*>(); auto* view : views)
      if (view->isVisibleTo(&window) && uwf::test::displayedRowHeight(view) > 0) {
        list = view;
        break;
      }
    QVERIFY(list);
    auto* pagination = list->parentWidget()->findChild<QWidget*>(QStringLiteral("entryPagination"));
    auto* footer = window.findChild<QWidget*>(QStringLiteral("mainStatusSurface"));
    QVERIFY(list && pagination && footer && list->isVisibleTo(&window));
    const auto gapAfterList = [&] { return pagination->mapTo(&window, QPoint()).y() - list->mapTo(&window, QPoint(0, list->height())).y(); };
    const auto gapBeforeStatus = [&] { return footer->mapTo(&window, QPoint()).y() - pagination->mapTo(&window, QPoint(0, pagination->height())).y(); };
    const int listGap = gapAfterList();
    const int statusGap = gapBeforeStatus();
    const int rowHeight = uwf::test::displayedRowHeight(list);
    const int chrome = window.height() - list->viewport()->height();
    for (int requestedHeight = 760; requestedHeight < 840; requestedHeight += 7) {
      window.resize(window.width(), requestedHeight);
      QTRY_COMPARE(window.height(), chrome + (requestedHeight - chrome) / rowHeight * rowHeight);
      QCOMPARE(list->viewport()->height() % rowHeight, 0);
      QCOMPARE(gapAfterList(), listGap);
      QCOMPARE(gapBeforeStatus(), statusGap);
      ui::PagedWindowLayout::forWindow(&window).fit();
      QCOMPARE(gapBeforeStatus(), statusGap);
    }
    RECT nativeBounds{}, clientBounds{};
    const auto nativeHandle = reinterpret_cast<HWND>(window.winId());
    QVERIFY(GetWindowRect(nativeHandle, &nativeBounds));
    QVERIFY(GetClientRect(nativeHandle, &clientBounds));
    const auto scale = window.devicePixelRatioF();
    const int frameHeight = nativeBounds.bottom - nativeBounds.top - (clientBounds.bottom - clientBounds.top);
    for (const auto edge : {WMSZ_TOP, WMSZ_BOTTOM}) {
      RECT requested = nativeBounds;
      requested.bottom = requested.top + qRound(702 * scale) + frameHeight;
      const auto before = requested;
      MSG sizing{nativeHandle, WM_SIZING, static_cast<WPARAM>(edge), reinterpret_cast<LPARAM>(&requested), 0, {}};
      qintptr result = 0;
      QVERIFY(ui::handleWindowResize(&window, &sizing, &result));
      QCOMPARE(result, TRUE);
      if (edge == WMSZ_TOP)
        QCOMPARE(requested.bottom, before.bottom);
      else
        QCOMPARE(requested.top, before.top);
      const int fittedHeight = qRound((requested.bottom - requested.top - frameHeight) / scale);
      // Exercise a resize delivered after the native sizing loop has exited.
      MSG finished{nativeHandle, WM_EXITSIZEMOVE, 0, 0, 0, {}};
      ui::handleWindowResize(&window, &finished, &result);
      window.resize(window.width(), fittedHeight);
      QTRY_COMPARE(list->viewport()->height() % rowHeight, 0);
      QCOMPARE(ui::PagedWindowLayout::forWindow(&window).requestedSize().height(), 702);
      QCOMPARE(window.findChild<QTabWidget*>(QStringLiteral("mainTabs"))->tabBar()->height(), 47);
      QCOMPARE(gapAfterList(), listGap);
      QCOMPARE(gapBeforeStatus(), statusGap);
    }
    auto* commands = window.findChild<QWidget*>(QStringLiteral("mainCommands"));
    auto* driveHeading = window.findChild<QLabel*>(QStringLiteral("diskHeading"));
    auto* global = window.findChild<ui::GlobalStatusPanel*>();
    auto* settingsHeading = global ? global->findChild<QLabel*>(QStringLiteral("panelTitle")) : nullptr;
    QVERIFY(commands && driveHeading && settingsHeading);
    std::array<std::optional<std::array<int, 4>>, 3> modeGaps;
    for (const int requestedHeight : {610, 618, 620, 625, 650, 680, 695, 700, 701, 702, 710, 740, 780}) {
      window.resize(window.width(), requestedHeight);
      ui::PagedWindowLayout::forWindow(&window).fit();
      QTRY_COMPARE(list->viewport()->height() % rowHeight, 0);
      const int rawHeight = ui::PagedWindowLayout::forWindow(&window).requestedSize().height();
      const std::size_t mode = rawHeight <= 620 ? 0u : rawHeight <= 700 ? 1u : 2u;
      const int toolbarBottom = commands->mapTo(&window, QPoint(0, commands->height())).y();
      const std::array gaps{gapAfterList(), gapBeforeStatus(), driveHeading->mapTo(&window, QPoint()).y() - toolbarBottom,
                            settingsHeading->mapTo(&window, QPoint()).y() - toolbarBottom};
      if (modeGaps[mode])
        QCOMPARE(gaps, *modeGaps[mode]);
      else
        modeGaps[mode] = gaps;
      QCOMPARE(window.findChild<QTabWidget*>(QStringLiteral("mainTabs"))->tabBar()->height(), mode < 2 ? 38 : 47);
    }
    window.resize(initial.size());
    QTRY_COMPARE(window.size(), initial.size());
  }
  QTRY_VERIFY2_WITH_TIMEOUT(test::clippedText(window).isEmpty(), qUtf8Printable(test::clippedText(window)), 2000);
  QVERIFY(test::saveScreenshot(window, QStringLiteral("main-window-startup")));
  const auto screenshotDirectory = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!quiet && !screenshotDirectory.isEmpty())
    QVERIFY(
        test::nativeScreenshot(window).save(QDir(screenshotDirectory).filePath(QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral("-native.png"))));

  window.resize(window.width() - 20, window.height() - 20);
  const QRect adjusted = window.geometry();
  const int reads = source.reads;
  window.hide();
  window.show();
  QCOMPARE(window.geometry(), adjusted);
  QCOMPARE(source.reads, reads);
  ui::ThemeManager::instance().toggle();
  QCOMPARE(source.reads, reads);
  QCOMPARE(window.geometry(), adjusted);
  QVERIFY(wmi.invocations.empty());
}

void ApplicationShellTests::mainWindowDistinguishesInitialFailureFromCommittedUnavailableState() {
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();

  MutableApplicationStateSource failingSource;
  failingSource.failure = QStringLiteral("initial provider failure");
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  {
    MemoryEnhancedModeAgent agent;
    ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, failingSource, stagingOwner, serviceControlOwner, &agent),
                          {.uwfCapability = uwf::app::UwfCapability::Available, .compatibilityMode = false});
    window.show();
    QTRY_COMPARE_WITH_TIMEOUT(failingSource.reads, 1, 1000);

    QTRY_VERIFY(refreshCompleted(window));
    auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"));
    QVERIFY(tabs);
    QCOMPARE(tabs->count(), 0);
    auto* refresh = actionWithText(&window, QStringLiteral("Refresh"));
    auto* import = actionWithText(&window, QStringLiteral("Import"));
    auto* enhanced = actionWithText(&window, QStringLiteral("Enhanced mode"));
    QVERIFY(refresh && refresh->isEnabled());
    QVERIFY(import && !import->isEnabled());
    QVERIFY(enhanced && enhanced->isEnabled());
    bool reasonVisible = false;
    for (const auto labels = window.findChildren<QLabel*>(); auto* label : labels)
      reasonVisible = reasonVisible || label->text().contains(failingSource.failure);
    QVERIFY(reasonVisible);
    window.close();
  }

  MutableApplicationStateSource source;
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;
  source.snapshot.unavailableReason = "embedded provider absent";
  {
    ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner),
                          {.uwfCapability = uwf::app::UwfCapability::Unavailable, .compatibilityMode = true});
    window.show();
    QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

    QTRY_VERIFY(refreshCompleted(window));
    auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"));
    QVERIFY(tabs);
    QCOMPARE(tabs->count(), 1);
    QCOMPARE(tabs->tabText(0), QStringLiteral("System (C:)"));
    auto* refresh = actionWithText(&window, QStringLiteral("Refresh"));
    auto* import = actionWithText(&window, QStringLiteral("Import"));
    auto* shutdown = actionWithText(&window, QStringLiteral("Safe shutdown"));
    QVERIFY(refresh && refresh->isEnabled());
    QVERIFY(import && !import->isEnabled());
    QVERIFY(shutdown && !shutdown->isEnabled());

    bool unavailableVisible = false;
    for (const auto labels = window.findChildren<QLabel*>(); auto* label : labels) {
      unavailableVisible = unavailableVisible || label->text().contains(QStringLiteral("embedded provider absent"));
    }
    auto* unavailableBadge = window.findChild<QLabel*>(QStringLiteral("compatibilityModeBadge"));
    QVERIFY(unavailableBadge);
    QVERIFY(!unavailableBadge->isVisibleTo(&window));
    QVERIFY(unavailableVisible);

    source.failure = QStringLiteral("later provider failure");
    window.refresh();
    QTRY_VERIFY(refreshCompleted(window));
    QCOMPARE(source.reads, 2);
    QCOMPARE(tabs->count(), 1);
    QCOMPARE(tabs->tabText(0), QStringLiteral("System (C:)"));
    bool laterFailureLeakedIntoUi = false;
    for (const auto labels = window.findChildren<QLabel*>(); auto* label : labels)
      laterFailureLeakedIntoUi = laterFailureLeakedIntoUi || label->text().contains(source.failure);
    QVERIFY(laterFailureLeakedIntoUi);
    window.close();
  }
  source.failure.clear();
  source.snapshot = editableSnapshot();
  {
    ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner),
                          {.uwfCapability = app::UwfCapability::Available, .compatibilityMode = true, .hubVisible = false});
    window.show();
    auto* badge = window.findChild<QLabel*>(QStringLiteral("compatibilityModeBadge"));
    QVERIFY(badge);
    QTRY_VERIFY(badge->isVisibleTo(&window));
    QCOMPARE(badge->text(), ui::I18n::tr("Compatibility mode"));
    const auto currentProtection = [&] {
      auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"));
      auto* current = tabs ? tabs->currentWidget() : nullptr;
      auto* protection = current ? current->findChild<QLabel*>(QStringLiteral("statusCurLabel")) : nullptr;
      return protection ? protection->text() : QString{};
    };
    const auto knownProtection = currentProtection();
    QVERIFY(!knownProtection.isEmpty());
    source.snapshot.uwfAvailable = false;
    window.refresh();
    QTRY_VERIFY(refreshCompleted(window));
    QTRY_VERIFY(!badge->isVisibleTo(&window));
    QTRY_COMPARE(currentProtection(), QStringLiteral("—"));
    source.snapshot.uwfAvailable = true;
    window.refresh();
    QTRY_VERIFY(refreshCompleted(window));
    QTRY_VERIFY(badge->isVisibleTo(&window));
    QTRY_COMPARE(currentProtection(), knownProtection);
  }
}

void ApplicationShellTests::mainWindowCompactLayoutKeepsControlsReachable_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<QSize>("windowSize");
  QTest::addColumn<bool>("warnings");
  for (const auto& locale : ui::I18n::languages()) {
    for (const bool dark : {false, true}) {
      for (const auto size : {QSize(1280, 720), QSize(1120, 680), QSize(1440, 900)}) {
        for (const bool warnings : {false, true}) {
          const QByteArray name = QStringLiteral("%1-%2-%3x%4-%5")
                                      .arg(QString(locale.code), dark ? "dark" : "light")
                                      .arg(size.width())
                                      .arg(size.height())
                                      .arg(warnings ? "warnings" : "normal")
                                      .toLatin1();
          QTest::newRow(name.constData()) << locale.id << dark << size << warnings;
        }
      }
    }
  }
}

void ApplicationShellTests::mainWindowCompactLayoutKeepsControlsReachable() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  QFETCH(QSize, windowSize);
  QFETCH(bool, warnings);
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
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  MemoryEnhancedModeAgent agent;
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  // 模拟关闭的筛选器，避免测试把真实窗口嵌入 Explorer 任务栏。
  // snapshot.elevated 只控制 mock 页面的可编辑性，不改变进程令牌。
  source.snapshot.current.filter.enabled = false;
  source.snapshot.next.filter.enabled = false;
  source.volumes = {{"C:", R"(\\?\Volume{12345678-1234-1234-1234-123456789abc}\)", "NTFS", "Workstation System Volume", 512ULL << 30, 256ULL << 30,
                     core::VolumeSupport::Supported}};
  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner, serviceControlOwner, &agent),
                        {.uwfCapability = uwf::app::UwfCapability::Available, .compatibilityMode = warnings});
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 2000);
  QVERIFY(window.screen()->availableGeometry().contains(window.frameGeometry()));
  const auto verifyRequestedSize = [&] {
    QTRY_COMPARE(window.width(), windowSize.width());
    int rowHeight = 0;
    for (const auto views = window.findChildren<QAbstractItemView*>(); const auto* view : views) {
      const int row = uwf::test::displayedRowHeight(view);
      if (row <= 0 || !view->isVisibleTo(&window)) continue;
      QTRY_COMPARE(view->viewport()->height() % row, 0);
      rowHeight = row;
    }
    QVERIFY(rowHeight > 0);
    QVERIFY(window.height() <= windowSize.height());
    QVERIFY(windowSize.height() - window.height() < rowHeight);
  };
  window.resize(windowSize);
  verifyRequestedSize();

  auto* global = window.findChild<ui::GlobalStatusPanel*>();
  auto* scroll = window.findChild<QScrollArea*>(QStringLiteral("globalSettingsScroll"));
  auto* disk = window.findChild<ui::VolumeTab*>();
  auto* status = disk ? disk->findChild<ui::VolumeStatusPanel*>() : nullptr;
  auto* inner = disk ? disk->findChild<QTabWidget*>(QStringLiteral("innerTabs")) : nullptr;
  QVERIFY(global && scroll && disk && status && inner);
  if (warnings) {
    auto* footer = window.findChild<ui::StatusSurface*>(QStringLiteral("mainStatusSurface"));
    QVERIFY(footer);
    footer->setText(QStringLiteral("Mock inventory warning: provider read failed.\nWQL: SELECT * FROM UWF_Volume\n") +
                    QStringLiteral("\\\\COMPUTER\\root\\standardcimv2\\embedded:") + QString(160, u'x') + QStringLiteral("\n旧数据保持不变"));
  }
  QTest::qWait(80);
  QTRY_VERIFY(scroll->viewport()->width() > 0 && scroll->viewport()->height() > 0);
  QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  QTRY_VERIFY(!scroll->horizontalScrollBar()->isVisible());
  auto* initialHint = window.findChild<QLabel*>(QStringLiteral("statusBarLabel"));
  QVERIFY(initialHint && !initialHint->text().isEmpty());
  QTRY_VERIFY2_WITH_TIMEOUT(test::clippedText(window).isEmpty(), qUtf8Printable(test::clippedText(window)), 2000);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!screenshots.isEmpty()) {
    QVERIFY(QDir().mkpath(screenshots));
    QVERIFY(uwf::test::screenshot(window).save(QDir(screenshots).filePath(QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral(".png"))));
  }
  auto* typeCombo = global->findChild<QComboBox*>();
  QVERIFY(typeCombo);
  const int originalType = typeCombo->currentIndex();
  typeCombo->setCurrentIndex(typeCombo->findData(static_cast<int>(core::OverlayType::RAM)));
  // 切换后先等待布局请求及原生滚动条更新；旧的几何可能暂时仍满足断言。
  QTest::qWait(80);
  QTRY_VERIFY2_WITH_TIMEOUT(test::clippedText(window).isEmpty(), qUtf8Printable(test::clippedText(window)), 2000);
  QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  QTRY_VERIFY(scroll->viewport()->width() >= scroll->widget()->minimumSizeHint().width());
  QCOMPARE(typeCombo->currentText(), QStringLiteral("RAM"));
  QVERIFY(typeCombo->isVisible());
  QVERIFY(typeCombo->width() >= typeCombo->minimumSizeHint().width());
  const auto ramScreenshot = uwf::test::screenshot(window);
  QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  QVERIFY(!scroll->horizontalScrollBar()->isVisible());
  for (const auto controls = global->findChildren<QWidget*>(); const auto* control : controls) {
    if (!control->isVisible() || (!qobject_cast<QLabel const*>(control) && !qobject_cast<QAbstractSpinBox const*>(control) &&
                                  !qobject_cast<QComboBox const*>(control) && !qobject_cast<QAbstractButton const*>(control)))
      continue;
    const QRect bounds(control->mapTo(scroll->viewport(), QPoint()), control->size());
    QVERIFY2(bounds.left() >= 0 && bounds.right() < scroll->viewport()->width(), control->metaObject()->className());
  }
  if (!screenshots.isEmpty())
    QVERIFY(ramScreenshot.save(QDir(screenshots).filePath(QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral("-ram.png"))));
  typeCombo->setCurrentIndex(originalType);
  QTest::qWait(80);
  QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
  verifyRequestedSize();
  qInfo() << "layout" << QTest::currentDataTag() << "dpr" << window.devicePixelRatioF() << "window" << window.size() << "minimum" << window.minimumSizeHint()
          << "status" << status->size() << "right" << scroll->size() << "content" << scroll->widget()->minimumSizeHint() << "horizontal overflow"
          << scroll->horizontalScrollBar()->maximum();
  QCOMPARE(window.findChildren<QScrollArea*>(QStringLiteral("globalSettingsScroll")).size(), 1);

  // 所有状态操作必须完整落在卡片内，不能靠压缩父窗口的 min-size 裁掉按钮。
  for (const auto widgets = status->findChildren<QWidget*>(); auto* widget : widgets) {
    if (!widget->isVisible() || (!qobject_cast<QAbstractButton*>(widget) && !qobject_cast<QComboBox*>(widget))) continue;
    const QRect bounds(widget->mapTo(status, QPoint()), widget->size());
    QVERIFY2(status->rect().contains(bounds), qUtf8Printable(QStringLiteral("clipped status control: %1 %2,%3 %4x%5 in %6x%7")
                                                                 .arg(widget->metaObject()->className())
                                                                 .arg(bounds.x())
                                                                 .arg(bounds.y())
                                                                 .arg(bounds.width())
                                                                 .arg(bounds.height())
                                                                 .arg(status->width())
                                                                 .arg(status->height())));
    QVERIFY(!widget->visibleRegion().isEmpty());
    QVERIFY(widget->width() >= widget->minimumSizeHint().width());
  }
  const auto protectionButtons = status->findChildren<QAbstractButton*>();
  auto* protection = status->findChild<ui::SwitchButton*>();
  QVERIFY(protection && protection->isEnabled());
  QTest::mouseClick(protection, Qt::LeftButton);
  QVERIFY(!protection->isChecked());
  QCOMPARE(disk->viewModel().status()->pendingProtection(), std::optional<bool>(false));
  const int heightAtTarget = status->height();
  window.resize(1800, windowSize.height());
  QTRY_VERIFY(status->height() <= heightAtTarget);
  window.resize(windowSize);
  verifyRequestedSize();
  QVERIFY(!protection->isChecked());
  QCOMPARE(disk->viewModel().status()->pendingProtection(), std::optional<bool>(false));
  QCOMPARE(status->findChildren<QAbstractButton*>(), protectionButtons);

  // 真正切换每个业务页，校验搜索、列表及操作区均在窗口内，重排不丢选择。
  for (int index = 0; index < inner->count(); ++index) {
    QTest::mouseClick(inner->tabBar(), Qt::LeftButton, Qt::NoModifier, inner->tabBar()->tabRect(index).center());
    QCOMPARE(inner->currentIndex(), index);
    auto* pagination = inner->widget(index)->findChild<QWidget*>(QStringLiteral("entryPagination"));
    auto* footer = window.findChild<QWidget*>(QStringLiteral("mainStatusSurface"));
    QVERIFY(pagination && footer);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    if (pagination->isVisible()) {
      const int gap = footer->mapTo(&window, QPoint()).y() - pagination->mapTo(&window, QPoint(0, pagination->height())).y();
      QVERIFY2(gap >= 0 && gap <= 12, qUtf8Printable(QStringLiteral("pagination/footer gap: %1 px").arg(gap)));
    } else {
      QVERIFY(warnings);
      auto* unavailable = inner->widget(index)->findChild<QWidget*>(QStringLiteral("volumeUnavailableState"));
      QVERIFY(unavailable && unavailable->isVisible());
    }
    if (!screenshots.isEmpty())
      QVERIFY(uwf::test::screenshot(window).save(
          QDir(screenshots).filePath(QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral("-tab%1.png").arg(index))));
    auto* page = inner->currentWidget();
    for (const auto widgets = page->findChildren<QWidget*>(); auto* widget : widgets) {
      if (!widget->isVisible() || (!qobject_cast<QLineEdit*>(widget) && !qobject_cast<QPushButton*>(widget) && !qobject_cast<QListView*>(widget))) continue;
      for (auto* ancestor = widget->parentWidget(); ancestor && ancestor != page; ancestor = ancestor->parentWidget())
        if (auto* contentScroll = qobject_cast<QScrollArea*>(ancestor)) contentScroll->ensureWidgetVisible(widget);
      QVERIFY2(window.rect().contains(QRect(widget->mapTo(&window, QPoint()), widget->size())), widget->metaObject()->className());
      QVERIFY(!widget->visibleRegion().isEmpty());
    }
  }

  // 逐项滚动到真实设置控件，并用鼠标改变开关，确认并非“滚动条存在”假通过。
  auto* filter = global->findChild<ui::SwitchButton*>();
  QVERIFY(filter && filter->isEnabled());
  scroll->ensureWidgetVisible(filter);
  QTRY_VERIFY(scroll->viewport()->rect().contains(QRect(filter->mapTo(scroll->viewport(), QPoint()), filter->size())));
  QTest::mouseClick(filter, Qt::LeftButton);
  QCOMPARE(global->viewModel().pendingFilterEnabled(), std::optional<bool>(true));
  for (const auto spins = global->findChildren<QSpinBox*>(); auto* spin : spins) {
    // ensureWidgetVisible 对输入控件只保证输入光标可见，而不是整个边框。
    // 这里验证整项设置可触达，必须按控件的完整矩形滚动。
    const QPoint center = spin->mapTo(scroll->widget(), spin->rect().center());
    scroll->ensureVisible(center.x(), center.y(), spin->width() / 2 + 1, spin->height() / 2 + 1);
    QTRY_VERIFY2(scroll->viewport()->rect().contains(QRect(spin->mapTo(scroll->viewport(), QPoint()), spin->size())),
                 qUtf8Printable(QStringLiteral("spin %1 at %2,%3 size %4x%5; viewport %6x%7 scroll %8/%9")
                                    .arg(spin->value())
                                    .arg(spin->mapTo(scroll->viewport(), QPoint()).x())
                                    .arg(spin->mapTo(scroll->viewport(), QPoint()).y())
                                    .arg(spin->width())
                                    .arg(spin->height())
                                    .arg(scroll->viewport()->width())
                                    .arg(scroll->viewport()->height())
                                    .arg(scroll->verticalScrollBar()->value())
                                    .arg(scroll->verticalScrollBar()->maximum())));
    QVERIFY(spin->width() >= spin->minimumSizeHint().width());
  }
  auto* hint = window.findChild<QLabel*>(QStringLiteral("statusBarLabel"));
  QVERIFY(hint);
  QTest::keyClick(scroll->verticalScrollBar(), Qt::Key_End);
  QTRY_VERIFY(window.rect().contains(QRect(hint->mapTo(&window, QPoint()), hint->size())));
  QVERIFY(hint->isVisible());
  scroll->verticalScrollBar()->setValue(0);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QVERIFY(wmi.invocations.empty());
  QCOMPARE(staging.writes, 0);
  QCOMPARE(serviceControl.installs + serviceControl.starts + serviceControl.removals, 0);
  QCOMPARE(agent.starts, 0);
}

void ApplicationShellTests::mainWindowCompactToolbarKeepsLanguageMenuUsable() {
  auto& language = ui::I18n::instance();
  const auto oldLanguage = language.lang();
  const auto restore = qScopeGuard([&] { language.setLang(oldLanguage); });
  language.setLang(ui::I18n::Lang::En);
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  MutableApplicationStateSource source;
  source.snapshot.uwfAvailable = false;
  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner),
                        {.uwfCapability = uwf::app::UwfCapability::Unavailable, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 2000);
  window.resize(900, 680);
  auto* action = actionWithText(&window, QStringLiteral("Language"));
  QVERIFY(action && action->menu());
  QPointer<QToolButton> languageButton;
  for (const auto buttons = window.findChildren<QToolButton*>(); auto* button : buttons)
    if (button->defaultAction() == action) languageButton = button;
  QVERIFY(languageButton);
  QTRY_VERIFY(languageButton->isVisible());
  QVERIFY(window.rect().contains(QRect(languageButton->mapTo(&window, QPoint()), languageButton->size())));
  bool selected = false;
  QTimer driver;
  driver.setInterval(10);
  connect(&driver, &QTimer::timeout, this, [&] {
    auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    if (!menu) {
      return;
    }
    if (auto* options = menu->findChild<QListView*>(QStringLiteral("scrollableMenuItems"))) {
      QVERIFY(menu->height() <= 520);
      QVERIFY(options->verticalScrollBar()->maximum() > 0);
      for (int row = 0; row < options->model()->rowCount(); ++row) {
        const auto index = options->model()->index(row, 0);
        if (index.data().toString() != QStringLiteral("简体中文")) continue;
        driver.stop();
        selected = true;
        options->scrollTo(index);
        QTest::mouseClick(options->viewport(), Qt::LeftButton, Qt::NoModifier, options->visualRect(index).center());
        return;
      }
    }
    for (const auto actions = menu->actions(); auto* candidate : actions) {
      if (candidate->text() == QStringLiteral("Language") && candidate->menu()) {
        QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(candidate).center());
        return;
      }
      if (candidate->text() == QStringLiteral("简体中文")) {
        driver.stop();
        selected = true;
        QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(candidate).center());
        return;
      }
    }
  });
  QTimer timeout;
  timeout.setSingleShot(true);
  connect(&timeout, &QTimer::timeout, this, [&] {
    driver.stop();
    for (const auto widgets = QApplication::topLevelWidgets(); auto* widget : widgets)
      if (auto* menu = qobject_cast<QMenu*>(widget)) menu->close();
  });
  driver.start();
  timeout.start(3000);
  QTest::mouseClick(languageButton, Qt::LeftButton);
  QTRY_VERIFY_WITH_TIMEOUT(selected, 3500);
  timeout.stop();
  QTRY_COMPARE(language.lang(), ui::I18n::Lang::Zh_CN);
  QTRY_COMPARE(window.windowTitle(), QStringLiteral("统一写入筛选器 (UWF) 管理器"));
  QVERIFY(wmi.invocations.empty());
  QCOMPARE(staging.writes, 0);
}

void ApplicationShellTests::overlayMenusRouteActionsAndDeferBlockingDispatch() {
  ui::OverlayContextMenu menu;
  auto* restore = menu.findChild<QAction*>(QStringLiteral("overlayMenuRestorePosition"));
  QVERIFY(restore && !restore->isVisible());
  menu.setRestorePositionAvailable(true);
  QSignalSpy restored(&menu, &ui::OverlayContextMenu::restorePositionRequested);
  QTimer::singleShot(0, &menu, [&] {
    menu.setActiveAction(restore);
    QTest::keyClick(&menu, Qt::Key_Return);
    QCOMPARE(restored.count(), 0);
  });
  menu.execAt(QPoint(100, 100));
  QCOMPARE(restored.count(), 1);

  if (!QSystemTrayIcon::isSystemTrayAvailable()) QSKIP("The remaining checks require a native system tray.");
  RecordingWmiOperations wmi;
  QMainWindow owner;
  ui::TrayController tray(&owner);
  ui::OverlayPresentationController presentation(uwf::bootstrap::createViewModelFactory(wmi)->overlay(), tray);
  auto* trayMenu = owner.findChild<ui::OverlayContextMenu*>();
  QVERIFY(trayMenu);
  QSignalSpy shown(&tray, &ui::TrayController::activateWindowRequested);
  QSignalSpy toggled(&tray, &ui::TrayController::toggleHubRequested);
  QSignalSpy shutdown(&presentation, &ui::OverlayPresentationController::safeShutdownRequested);
  QSignalSpy restart(&presentation, &ui::OverlayPresentationController::safeRestartRequested);
  QSignalSpy exited(&tray, &ui::TrayController::exitApplicationRequested);
  const auto trigger = [&](const char* name, QSignalSpy& spy) {
    auto* action = trayMenu->findChild<QAction*>(QString::fromLatin1(name));
    QVERIFY(action && action->isEnabled());
    action->trigger();
    QCOMPARE(spy.count(), 1);
  };
  trigger("overlayMenuShowMain", shown);
  auto* usageAction = trayMenu->findChild<QAction*>(QStringLiteral("overlayMenuUsage"));
  QVERIFY(usageAction);
  usageAction->trigger();
  QCOMPARE(shown.count(), 2);
  tray.setHubState(true, true);
  trigger("overlayMenuToggleHub", toggled);
  tray.setOperationAvailability(true, true);
  trigger("overlayMenuSafeShutdown", shutdown);
  trigger("overlayMenuSafeRestart", restart);
  trigger("overlayMenuExit", exited);
  tray.setOperationAvailability(false, true);
  auto* shutdownAction = trayMenu->findChild<QAction*>(QStringLiteral("overlayMenuSafeShutdown"));
  auto* restartAction = trayMenu->findChild<QAction*>(QStringLiteral("overlayMenuSafeRestart"));
  QVERIFY(shutdownAction && restartAction);
  QVERIFY(!shutdownAction->isEnabled() && !restartAction->isEnabled());
  shutdownAction->trigger();
  restartAction->trigger();
  QCOMPARE(shutdown.count(), 1);
  QCOMPARE(restart.count(), 1);
  QVERIFY(wmi.invocations.empty());
}

void ApplicationShellTests::trayUsageCardKeepsValuesAndTooltipsConsistent_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  for (const auto& language : ui::I18n::languages()) {
    for (const bool dark : {false, true}) {
      const QByteArray name = QStringLiteral("%1-%2").arg(QString(language.code), dark ? "dark" : "light").toLatin1();
      QTest::newRow(name.constData()) << language.id << dark;
    }
  }
}

void ApplicationShellTests::trayUsageCardKeepsValuesAndTooltipsConsistent() {
  if (!QSystemTrayIcon::isSystemTrayAvailable()) QSKIP("This test requires a native system tray.");
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
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  QWidget owner;
  ui::TrayController tray(&owner);
  tray.setHubState(true, true);
  tray.setOperationAvailability(true, true);
  auto* icon = tray.findChild<QSystemTrayIcon*>();
  auto* menu = owner.findChild<ui::OverlayContextMenu*>();
  auto* progress = owner.findChild<QProgressBar*>(QStringLiteral("trayUsageProgress"));
  auto* percent = owner.findChild<QLabel*>(QStringLiteral("trayUsagePercent"));
  auto* numeric = owner.findChild<QLabel*>(QStringLiteral("trayUsageNumeric"));
  auto* card = owner.findChild<QWidget*>(QStringLiteral("trayUsageCard"));
  QVERIFY(icon && menu && progress && percent && numeric && card);
  const auto close = qScopeGuard([&] { menu->close(); });
  menu->popup(QPoint(100, 100));
  QVERIFY(QTest::qWaitForWindowExposed(menu));
  const auto inspect = [&](const QString& tag) {
    QCoreApplication::processEvents();
    menu->repaint();
    const QString clipped = test::clippedText(*menu);
    QVERIFY2(clipped.isEmpty(), qUtf8Printable(clipped));
    QVERIFY(menu->rect().contains(QRect(card->mapTo(menu, QPoint()), card->size())));
    const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
    if (!screenshots.isEmpty()) {
      QVERIFY(QDir().mkpath(screenshots));
      const QString name = QStringLiteral("usage-") + QString::fromLatin1(QTest::currentDataTag()) + u'-' + tag + QStringLiteral(".png");
      QVERIFY(uwf::test::nativeScreenshot(*menu).save(QDir(screenshots).filePath(name)));
    }
  };
  core::OverlayConfig config;
  config.type = core::OverlayType::RAM;
  config.maximumSizeMb = 4000;
  config.warningThresholdMb = 2800;
  config.criticalThresholdMb = 3600;
  for (const auto percentage : {0U, 25U, 70U, 95U, 100U}) {
    const core::OverlayRuntime runtime{4000U - percentage * 40U, percentage * 40U};
    tray.applyUsageState(app::OverlayUsageEnabled(runtime, config));
    QVERIFY(progress->isVisible() && percent->isVisible() && numeric->isVisible());
    QCOMPARE(progress->value(), static_cast<int>(percentage * 100U));
    QCOMPARE(percent->text(), QStringLiteral("%1%").arg(percentage));
    const QLocale numbers(QLocale::English, QLocale::UnitedStates);
    QCOMPARE(icon->toolTip(), QStringLiteral("UWF Manager\n%1% (%2/4,000 MB)").arg(percentage).arg(numbers.toString(runtime.currentConsumptionMb)));
    inspect(QString::number(percentage));
  }
  const auto maximum = std::numeric_limits<std::uint32_t>::max();
  tray.applyUsageState(app::OverlayUsageEnabled({maximum, maximum}, config));
  QCOMPARE(progress->value(), 5000);
  QCOMPARE(icon->toolTip(), QStringLiteral("UWF Manager\n50% (4,294,967,295/8,589,934,590 MB)"));
  inspect(QStringLiteral("large"));
  tray.applyUsageState(app::OverlayUsageEnabled({0, 0}, config));
  QCOMPARE(progress->value(), 0);
  QCOMPARE(icon->toolTip(), QStringLiteral("UWF Manager\n0% (0/0 MB)"));
  for (const bool unavailable : {false, true}) {
    tray.applyUsageState(unavailable ? app::OverlayUsageState(app::OverlayUsageUnavailable{}) : app::OverlayUsageState(app::OverlayUsageDisabled{}));
    QVERIFY(!progress->isVisible() && !percent->isVisible() && !numeric->isVisible());
    const QString status = unavailable ? ui::I18n::tr("UWF status unavailable") : ui::I18n::tr("UWF: Disabled");
    QCOMPARE(icon->toolTip(), QStringLiteral("UWF Manager\n") + status);
    inspect(unavailable ? QStringLiteral("unavailable") : QStringLiteral("disabled"));
  }
  auto* toggle = menu->findChild<QAction*>(QStringLiteral("overlayMenuToggleHub"));
  QVERIFY(toggle);
  tray.setHubState(true, false);
  QCOMPARE(toggle->text(), ui::I18n::tr("Show overlay hub"));
  tray.setHubState(true, true);
  QCOMPARE(toggle->text(), ui::I18n::tr("Hide overlay hub"));
}

void ApplicationShellTests::contextMenusKeepTextRenderingConsistent_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  QTest::addColumn<QString>("entryPoint");
  for (const auto locale : {ui::I18n::Lang::Zh_CN, ui::I18n::Lang::Zh_TW, ui::I18n::Lang::En}) {
    for (const bool dark : {false, true}) {
      for (const auto* entry : {"tray", "taskbar", "floating"}) {
        const QByteArray name = QStringLiteral("%1-%2-%3").arg(entry).arg(static_cast<int>(locale)).arg(dark ? "dark" : "light").toLatin1();
        QTest::newRow(name.constData()) << locale << dark << QString::fromLatin1(entry);
      }
    }
  }
}

void ApplicationShellTests::contextMenusKeepTextRenderingConsistent() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  QFETCH(QString, entryPoint);
  if (entryPoint == QLatin1String("tray") && !QSystemTrayIcon::isSystemTrayAvailable()) QSKIP("This test requires a native system tray.");
  auto& language = ui::I18n::instance();
  auto& theme = ui::ThemeManager::instance();
  const auto oldLanguage = language.lang();
  const auto oldTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    language.setLang(oldLanguage);
    theme.apply(oldTheme);
  });
  language.setLang(locale);
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  QWidget owner;
  std::unique_ptr<ui::TrayController> tray;
  std::unique_ptr<ui::OverlayHubView> hub;
  QMenu* trayMenu = nullptr;
  QWidget* contextTarget = nullptr;
  if (entryPoint == QLatin1String("tray")) {
    tray = std::make_unique<ui::TrayController>(&owner);
    const auto snapshot = editableSnapshot();
    tray->applyUsageState(app::OverlayUsageEnabled(snapshot.runtime, snapshot.current.overlay));
    tray->setHubState(true, true);
    tray->setOperationAvailability(true, true);
    trayMenu = owner.findChild<QMenu*>();
    QVERIFY(trayMenu);
  } else if (entryPoint == QLatin1String("taskbar")) {
    // Exercise the real menu without attaching a test HUB to Explorer.
    hub = std::make_unique<ui::OverlayTaskbarWidget>();
    contextTarget = hub.get();
  } else {
    hub = std::make_unique<ui::OverlayFloatingWidget>();
    contextTarget = hub->findChild<QWidget*>(QStringLiteral("overlayFloatingHandle"));
    QVERIFY(contextTarget);
  }

  bool inspected = false;
  QTimer inspectTimer;
  inspectTimer.setSingleShot(true);
  inspectTimer.setInterval(150);
  connect(&inspectTimer, &QTimer::timeout, &owner, [&] {
    auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    const auto closeMenu = qScopeGuard([&] {
      if (menu) menu->close();
      inspected = true;
    });
    QVERIFY(menu && menu->isVisible());
    const auto actions = menu->actions();
    QVERIFY(!actions.isEmpty());
    QStringList commonActions;
    for (const auto* action : actions) {
      if (!action->isVisible() || action->isSeparator() || qobject_cast<const QWidgetAction*>(action)) continue;
      commonActions.append(action->text());
    }
    QStringList expected{QStringLiteral("UWF Manager"), ui::I18n::tr("Show main window"), ui::I18n::tr("Hide overlay hub"),
                         ui::I18n::tr("Safe shutdown"), ui::I18n::tr("Safe restart"),     ui::I18n::tr("Exit application")};
    if (entryPoint == QLatin1String("floating")) expected.append(ui::I18n::tr("Restore default position"));
    for (const auto& text : expected) QVERIFY2(commonActions.contains(text), qUtf8Printable(text));
    const QString screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
    const auto capture = [&](const QString& phase) {
      if (screenshots.isEmpty()) return;
      QVERIFY(QDir().mkpath(screenshots));
      const QString suffix = QString::fromLatin1(QTest::currentDataTag()) + phase + ".png";
      QVERIFY(uwf::test::screenshot(*menu).save(QDir(screenshots).filePath("menu-" + suffix)));
      QVERIFY(uwf::test::nativeScreenshot(*menu).save(QDir(screenshots).filePath("native-menu-" + suffix)));
    };
    capture(QString());
    qInfo() << "menu" << entryPoint << "DPR" << menu->devicePixelRatioF() << "hinting" << menu->font().hintingPreference() << "strategy"
            << menu->font().styleStrategy();
    const QString clipped = test::clippedText(*menu);
    QVERIFY2(clipped.isEmpty(), qUtf8Printable(clipped));
    menu->setActiveAction(actions.back());
    menu->repaint();
    capture(QStringLiteral("-selected"));
    theme.toggle();
    menu->ensurePolished();
    menu->repaint();
    capture(QStringLiteral("-theme-changed"));
  });
  inspectTimer.start();
  if (trayMenu) {
    trayMenu->popup(QPoint(100, 100));
  } else {
    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(5, 5), QPoint(100, 100));
    QApplication::sendEvent(contextTarget, &event);
  }
  QTRY_VERIFY_WITH_TIMEOUT(inspected, 2000);
}

QTEST_MAIN(ApplicationShellTests)
#include "ShellCases.moc"
