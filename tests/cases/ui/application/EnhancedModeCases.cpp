// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QDir>
#include <QImage>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollArea>
#include <QTest>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QTimer>
#include <QToolButton>
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
#include "presentation/staging/FileStagingResultText.h"
#include "shared/Log.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/WidgetCapture.h"
#include "ui/design/DesignSystem.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/enhanced/EnhancedModeDialog.h"
#include "ui/overlay/OverlayHudPalette.h"
#include "ui/shell/MainWindow.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/Notice.h"
#include "windows/ipc/EnhancedModeAgent.h"
#include "windows/system/SystemHardwareInfo.h"
#include "windows/system/WindowsShell.h"

class ApplicationEnhancedModeTests final : public ApplicationUiTest {
  Q_OBJECT
 private slots:
  void enhancedModeDialogReflectsDisabledEnabledAndRepairStates();
  void enhancedModePathsShowSelectionAndCopyCompleteText_data();
  void enhancedModePathsShowSelectionAndCopyCompleteText();
  void mainWindowEnhancedModeActionTracksTheServiceLifecycle();
  void mainWindowEnhancedModeIconUsesStatusColors_data();
  void mainWindowEnhancedModeIconUsesStatusColors();
  void mainWindowCreatesAndOwnsTheInjectedAgent();
  void mainWindowRetriesTransientEnhancedModeStatusFailures();
  void mainWindowTreatsAuthenticatedServiceConnectionAsStatusOnly();
};

using namespace uwf;
using namespace uwf::test::application;

namespace {
QToolButton* commandButtonForAction(QWidget& window, QAction* action) {
  for (const auto buttons = window.findChildren<QToolButton*>(); auto* button : buttons)
    if (button->defaultAction() == action) return button;
  return nullptr;
}

bool renderedIconHasColor(QToolButton& button, const QColor& expected) {
  const auto size = button.iconSize();
  const QRect bounds(QPoint((button.width() - size.width()) / 2, (button.height() - size.height()) / 2), size);
  const auto pixels = button.grab(bounds).toImage();
  int matching = 0;
  for (int y = 0; y < pixels.height(); ++y)
    for (int x = 0; x < pixels.width(); ++x)
      if (pixels.pixelColor(x, y) == expected) ++matching;
  return matching > 2;
}
}  // namespace

void ApplicationEnhancedModeTests::enhancedModePathsShowSelectionAndCopyCompleteText_data() {
  QTest::addColumn<bool>("dark");
  QTest::addColumn<bool>("wrapped");
  for (const bool dark : {false, true})
    for (const bool wrapped : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(dark ? "dark" : "light", wrapped ? "wrapped" : "single-line"))) << dark << wrapped;
}

void ApplicationEnhancedModeTests::enhancedModePathsShowSelectionAndCopyCompleteText() {
  QFETCH(bool, dark);
  QFETCH(bool, wrapped);
  auto& theme = ui::ThemeManager::instance();
  const auto originalTheme = theme.current();
  const auto originalClipboard = QApplication::clipboard()->text();
  const auto originalProgramFiles = qgetenv("ProgramFiles");
  const bool hadProgramFiles = qEnvironmentVariableIsSet("ProgramFiles");
  const auto restore = qScopeGuard([&] {
    theme.apply(originalTheme);
    QApplication::clipboard()->setText(originalClipboard);
    if (hadProgramFiles) qputenv("ProgramFiles", originalProgramFiles);
    else qunsetenv("ProgramFiles");
  });
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  const QString programFiles = wrapped ? QStringLiteral("C:/Program Files/Enterprise application deployment/Managed security tools/Long installation directory")
                                       : QStringLiteral("C:/Apps");
  qputenv("ProgramFiles", programFiles.toUtf8());
  const auto expected = QDir::toNativeSeparators(QDir(programFiles).filePath(QStringLiteral("UWF Manager/Service/UWF.exe")));
  RecordingWmiOperations wmi;
  auto serviceControl = std::make_shared<MemoryEnhancedModeServiceControl>();
  app::EnhancedModeManager manager(serviceControl, wmi, app::UwfCapability::Unavailable);
  ui::EnhancedModeDialog dialog(bootstrap::createViewModelFactory()->enhancedMode(manager));
  dialog.resize(wrapped ? 540 : 740, 690);
  auto* safety = dialog.findChild<ui::Disclosure*>(QStringLiteral("enhancedModeSafety"));
  auto* details = dialog.findChild<ui::Disclosure*>(QStringLiteral("enhancedServiceDetails"));
  auto* scroll = dialog.findChild<QScrollArea*>(QStringLiteral("enhancedModeScroll"));
  auto* location = dialog.findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeSuggestedLocation"));
  QVERIFY(safety && details && scroll && location);
  safety->setOpen(true);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QTRY_VERIFY(location->height() >= location->heightForWidth(location->width()));
  scroll->ensureWidgetVisible(location);
  QCoreApplication::processEvents();
  QCOMPARE(location->toPlainText(), expected);
  QCOMPARE(location->font().pixelSize(), 12);
  QCOMPARE(location->font().families(), ui::DesignSystem::monospaceFamilies());
  QCOMPARE(location->focusPolicy(), Qt::ClickFocus);
  QVERIFY(!location->textInteractionFlags().testFlag(Qt::TextEditable));
  auto* layout = location->document()->firstBlock().layout();
  QVERIFY(layout);
  QVERIFY(wrapped ? layout->lineCount() > 1 : layout->lineCount() == 1);
  location->setFocus();
  const auto before = location->viewport()->grab().toImage();
  QTextCursor first(location->document());
  QTextCursor last(location->document());
  last.movePosition(QTextCursor::End);
  const auto start = location->cursorRect(first).center();
  const auto end = location->cursorRect(last).center() + QPoint(1, 0);
  QTest::mousePress(location->viewport(), Qt::LeftButton, Qt::NoModifier, start);
  QTest::mouseMove(location->viewport(), end, 20);
  QTest::mouseRelease(location->viewport(), Qt::LeftButton, Qt::NoModifier, end);
  QTRY_COMPARE(location->textCursor().selectedText(), expected);
  const auto selected = location->viewport()->grab().toImage();
  QVERIFY(selected != before);
  const auto highlight = location->palette().color(QPalette::Highlight);
  const auto ratio = selected.devicePixelRatio();
  for (int line = 0; line < layout->lineCount(); ++line) {
    QTextCursor lineStart(location->document());
    lineStart.setPosition(layout->lineAt(line).textStart());
    const auto bounds = location->cursorRect(lineStart);
    int highlighted = 0;
    for (int y = std::max(0, qFloor(bounds.top() * ratio)); y < std::min(selected.height(), qCeil((bounds.bottom() + 1) * ratio)); ++y)
      for (int x = 0; x < selected.width(); ++x)
        if (selected.pixelColor(x, y) == highlight && before.pixelColor(x, y) != highlight) ++highlighted;
    QVERIFY2(highlighted > 10, qUtf8Printable(QStringLiteral("No painted selection on wrapped line %1").arg(line)));
  }
  QTest::keyClick(location, Qt::Key_C, Qt::ControlModifier);
  QCOMPARE(QApplication::clipboard()->text(), expected);
  QVERIFY(test::saveScreenshot(dialog, QStringLiteral("enhanced-mode-selected-path")));
  safety->setOpen(false);
  details->setOpen(true);
  for (int index = 0; index < 5; ++index) {
    auto* value = dialog.findChild<ui::ContentHintBox*>(QStringLiteral("enhancedModeServiceValue%1").arg(index));
    QVERIFY(value && !value->toPlainText().isEmpty());
    QCOMPARE(value->focusPolicy(), Qt::ClickFocus);
    value->selectAll();
    const auto text = value->toPlainText();
    QCOMPARE(value->textCursor().selectedText(), text);
    value->copy();
    QCOMPARE(QApplication::clipboard()->text(), text);
  }
}

void ApplicationEnhancedModeTests::enhancedModeDialogReflectsDisabledEnabledAndRepairStates() {
  RecordingWmiOperations wmi;
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  app::EnhancedModeManager manager(serviceControlOwner, wmi, uwf::app::UwfCapability::Unavailable);
  ui::EnhancedModeDialog dialog(uwf::bootstrap::createViewModelFactory()->enhancedMode(manager));
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));

  auto* status = dialog.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* change = dialog.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* remove = dialog.findChild<QPushButton*>(QStringLiteral("enhancedModeRemoveButton"));
  QVERIFY(status && change && remove);
  QCOMPARE(status->text(), QStringLiteral("Disabled"));
  QCOMPARE(change->text(), QStringLiteral("Enable enhanced mode"));
  QVERIFY(!remove->isEnabled());
  QTest::mouseClick(change, Qt::LeftButton);
  QTRY_VERIFY(!manager.operationInProgress());
  QCOMPARE(serviceControl.installs, 1);
  QCOMPARE(serviceControl.installedDescription, QStringLiteral("UWF Manager enhanced mode helper service"));
  QCOMPARE(status->text(), QStringLiteral("Enabled"));
  QCOMPARE(change->text(), QStringLiteral("Disable enhanced mode"));
  QVERIFY(remove->isEnabled());

  const int queriesBeforeAgentStateChanges = serviceControl.queries;
  manager.setAgentState(app::EnhancedModeAgentState::Connecting);
  auto* issue = dialog.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(issue);
  QCOMPARE(issue->text(), QStringLiteral("Waiting for the UI agent to complete authentication."));

  manager.setAgentState(app::EnhancedModeAgentState::Disconnected);
  QCOMPARE(status->text(), QStringLiteral("Enabled"));
  QCOMPARE(change->text(), QStringLiteral("Disable enhanced mode"));
  QCOMPARE(issue->text(), QStringLiteral("The service is running, but no authenticated UI agent is connected."));

  manager.setAgentState(app::EnhancedModeAgentState::Connected);
  QVERIFY(!issue->isVisible());
  QCOMPARE(serviceControl.queries, queriesBeforeAgentStateChanges);

  QTest::mouseClick(change, Qt::LeftButton);
  QTRY_VERIFY(!manager.operationInProgress());
  QCOMPARE(serviceControl.removals, 1);
  QCOMPARE(status->text(), QStringLiteral("Disabled"));
  QCOMPARE(serviceControl.activeDeletionBarriers, 0);
  QVERIFY(!serviceControl.queriedDuringDeletion);
  QVERIFY(!remove->isEnabled());

  serviceControl.current = {};
  serviceControl.current.state = app::EnhancedModeState::Stopped;
  serviceControl.current.serviceExists = true;
  serviceControl.current.ownProcess = true;
  serviceControl.current.automaticStart = true;
  serviceControl.current.localSystemAccount = true;
  serviceControl.current.executableMatches = true;
  serviceControl.current.preshutdownTimeoutConfigured = true;
  serviceControl.current.requiredPrivilegesConfigured = true;
  serviceControl.current.serviceRegistryPresent = true;
  ui::EnhancedModeDialog stopped(uwf::bootstrap::createViewModelFactory()->enhancedMode(manager));
  auto* start = stopped.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* stoppedStatus = stopped.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* stoppedIssue = stopped.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(start && stoppedStatus && stoppedIssue);
  QCOMPARE(stoppedStatus->text(), QStringLiteral("Service stopped"));
  QCOMPARE(start->text(), QStringLiteral("Start service"));
  QVERIFY(!stoppedIssue->isHidden());
  QVERIFY(stoppedIssue->text().contains(QStringLiteral("installed but stopped")));
  QTest::mouseClick(start, Qt::LeftButton);
  QTRY_VERIFY(!manager.operationInProgress());
  QCOMPARE(serviceControl.starts, 1);
  QCOMPARE(serviceControl.installs, 1);
  QCOMPARE(stoppedStatus->text(), QStringLiteral("Enabled"));

  serviceControl.current.running = true;
  serviceControl.current.state = app::EnhancedModeState::RepairRequired;
  serviceControl.current.executableMatches = false;
  ui::EnhancedModeDialog repairRequired(uwf::bootstrap::createViewModelFactory()->enhancedMode(manager));
  auto* repair = repairRequired.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* repairStatus = repairRequired.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* repairIssue = repairRequired.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(repair && repairStatus && repairIssue);
  QCOMPARE(repairStatus->text(), QStringLiteral("Repair required"));
  QCOMPARE(repair->text(), QStringLiteral("Repair enhanced mode"));
  QVERIFY(repairIssue->text().contains(QStringLiteral("executable path or startup arguments")));

  serviceControl.current = {};
  serviceControl.current.state = app::EnhancedModeState::RepairRequired;
  serviceControl.current.serviceRegistryPresent = true;
  ui::EnhancedModeDialog residual(uwf::bootstrap::createViewModelFactory()->enhancedMode(manager));
  auto* residualRemove = residual.findChild<QPushButton*>(QStringLiteral("enhancedModeRemoveButton"));
  auto* residualIssue = residual.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(residualRemove && residualIssue);
  QVERIFY(residualRemove->isEnabled());
  QCOMPARE(residualIssue->text(), QStringLiteral("The service is absent, but its service registry data remains."));

  serviceControl.queryFailure = QStringLiteral("SCM unavailable");
  ui::EnhancedModeDialog unavailable(uwf::bootstrap::createViewModelFactory()->enhancedMode(manager));
  auto* unavailableStatus = unavailable.findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
  auto* unavailableChange = unavailable.findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
  auto* unavailableRemove = unavailable.findChild<QPushButton*>(QStringLiteral("enhancedModeRemoveButton"));
  auto* unavailableIssue = unavailable.findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
  QVERIFY(unavailableStatus && unavailableChange && unavailableRemove && unavailableIssue);
  QCOMPARE(unavailableStatus->text(), QStringLiteral("Status unavailable"));
  QVERIFY(unavailableIssue->text().contains(QStringLiteral("service status could not be read")));
  bool hasDiagnostic = false;
  for (const auto texts = unavailable.findChildren<ui::ContentHintBox*>(); const auto* text : texts)
    hasDiagnostic = hasDiagnostic || text->toPlainText().contains(serviceControl.queryFailure);
  QVERIFY(hasDiagnostic);
  QVERIFY(unavailableChange->isEnabled());
  QCOMPARE(unavailableChange->text(), QStringLiteral("Retry"));
  QVERIFY(!unavailableRemove->isEnabled());
  serviceControl.queryFailure.clear();
  serviceControl.current = completeEnhancedModeStatus();
  const int queriesBeforeRetry = serviceControl.queries;
  QTest::mouseClick(unavailableChange, Qt::LeftButton);
  QCOMPARE(serviceControl.queries, queriesBeforeRetry + 1);
  QCOMPARE(unavailableStatus->text(), QStringLiteral("Enabled"));
  QCOMPARE(unavailableChange->text(), QStringLiteral("Disable enhanced mode"));
  QVERIFY(unavailableRemove->isEnabled());
}

void ApplicationEnhancedModeTests::mainWindowEnhancedModeActionTracksTheServiceLifecycle() {
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  MemoryEnhancedModeAgent agent;
  MutableApplicationStateSource source;
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;

  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner, serviceControlOwner, &agent),
                        {.uwfCapability = uwf::app::UwfCapability::Unavailable, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  window.resize(ui::design::smallMainWidth, ui::design::smallMainHeight);
  auto* enhancedAction = actionWithText(&window, QStringLiteral("Enhanced mode"));
  QVERIFY(enhancedAction);
  QVERIFY(!enhancedAction->icon().isNull());
  QCOMPARE(enhancedAction->toolTip(), QStringLiteral("Coordinate automatic file staging with Windows shutdown and restart."));
  auto* enhancedButton = commandButtonForAction(window, enhancedAction);
  QVERIFY(enhancedButton);
  QTRY_COMPARE(enhancedButton->toolButtonStyle(), Qt::ToolButtonIconOnly);
  auto& theme = ui::ThemeManager::instance();
  QVERIFY(renderedIconHasColor(*enhancedButton, theme.color(ui::Sem::Fg)));

  bool enableDialogObserved = false;
  QTimer::singleShot(0, &window, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog || dialog->objectName() != QStringLiteral("enhancedModeDialog")) return;
    auto* status = dialog->findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
    auto* issue = dialog->findChild<ui::Notice*>(QStringLiteral("enhancedModeIssue"));
    auto* change = dialog->findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
    if (!status || !issue || !change || status->text() != QStringLiteral("Disabled")) return;
    const auto close = qScopeGuard([dialog] { dialog->reject(); });
    enableDialogObserved = true;
    change->click();
    QTRY_COMPARE(issue->text(), QStringLiteral("Waiting for the UI agent to complete authentication."));
  });
  enhancedAction->trigger();
  QVERIFY(enableDialogObserved);
  QCOMPARE(serviceControl.installs, 1);
  QCOMPARE(serviceControl.current.state, app::EnhancedModeState::Enabled);
  QCOMPARE(agent.starts, 1);
  QVERIFY(renderedIconHasColor(*enhancedButton, theme.color(ui::Sem::Accent)));

  bool disableDialogObserved = false;
  QTimer::singleShot(0, &window, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (!dialog || dialog->objectName() != QStringLiteral("enhancedModeDialog")) return;
    auto* status = dialog->findChild<QLabel*>(QStringLiteral("enhancedModeStatus"));
    auto* change = dialog->findChild<QPushButton*>(QStringLiteral("enhancedModeChangeButton"));
    if (!status || !change || status->text() != QStringLiteral("Enabled")) return;
    const auto close = qScopeGuard([dialog] { dialog->reject(); });
    disableDialogObserved = true;
    change->click();
    QTRY_COMPARE(status->text(), QStringLiteral("Disabled"));
  });
  enhancedAction->trigger();
  QVERIFY(disableDialogObserved);
  QCOMPARE(serviceControl.removals, 1);
  QCOMPARE(serviceControl.current.state, app::EnhancedModeState::Disabled);
  QCOMPARE(agent.stops, 1);
  QVERIFY(renderedIconHasColor(*enhancedButton, theme.color(ui::Sem::Fg)));
}

void ApplicationEnhancedModeTests::mainWindowEnhancedModeIconUsesStatusColors_data() {
  QTest::addColumn<bool>("dark");
  QTest::addColumn<int>("serviceState");
  QTest::addColumn<int>("agentState");
  QTest::addColumn<int>("expectedColor");
  const auto row = [](const char* name, const app::EnhancedModeState service, const app::EnhancedModeAgentState agent, const ui::Sem color) {
    for (const bool dark : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString::fromLatin1(name), dark ? "dark" : "light")))
          << dark << static_cast<int>(service) << static_cast<int>(agent) << static_cast<int>(color);
  };
  row("disabled", app::EnhancedModeState::Disabled, app::EnhancedModeAgentState::Unobserved, ui::Sem::Fg);
  row("connecting", app::EnhancedModeState::Enabled, app::EnhancedModeAgentState::Connecting, ui::Sem::Accent);
  row("connected", app::EnhancedModeState::Enabled, app::EnhancedModeAgentState::Connected, ui::Sem::AddOk);
  row("disconnected", app::EnhancedModeState::Enabled, app::EnhancedModeAgentState::Disconnected, ui::Sem::Accent);
  row("stopped", app::EnhancedModeState::Stopped, app::EnhancedModeAgentState::Unobserved, ui::Sem::Warn);
  row("repair-required", app::EnhancedModeState::RepairRequired, app::EnhancedModeAgentState::Unobserved, ui::Sem::Warn);
}

void ApplicationEnhancedModeTests::mainWindowEnhancedModeIconUsesStatusColors() {
  QFETCH(bool, dark);
  QFETCH(int, serviceState);
  QFETCH(int, agentState);
  QFETCH(int, expectedColor);
  auto& theme = ui::ThemeManager::instance();
  const auto originalTheme = theme.current();
  const auto restore = qScopeGuard([&] { theme.apply(originalTheme); });
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);

  RecordingWmiOperations wmi;
  auto staging = std::make_shared<MemoryFileStagingStore>();
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  const auto state = static_cast<app::EnhancedModeState>(serviceState);
  if (state != app::EnhancedModeState::Disabled) service->current = completeEnhancedModeStatus();
  service->current.state = state;
  if (state == app::EnhancedModeState::Stopped) service->current.running = false;
  if (state == app::EnhancedModeState::RepairRequired) service->current.executableMatches = false;
  MemoryEnhancedModeAgent agent;
  agent.connectOnStart = agentState == static_cast<int>(app::EnhancedModeAgentState::Connected);
  agent.canStart = agentState != static_cast<int>(app::EnhancedModeAgentState::Disconnected);
  MutableApplicationStateSource source;
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;
  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, staging, service, &agent),
                        {.uwfCapability = app::UwfCapability::Unavailable, .compatibilityMode = false});
  window.show();
  QTRY_VERIFY(refreshCompleted(window));
  window.resize(ui::design::smallMainWidth, ui::design::smallMainHeight);
  auto* action = actionWithText(&window, QStringLiteral("Enhanced mode"));
  QVERIFY(action);
  auto* button = commandButtonForAction(window, action);
  QVERIFY(button);
  QTRY_COMPARE(button->toolButtonStyle(), Qt::ToolButtonIconOnly);
  QVERIFY(button->isEnabled());
  QVERIFY(renderedIconHasColor(*button, theme.color(static_cast<ui::Sem>(expectedColor))));

  if (state == app::EnhancedModeState::Enabled) {
    agent.publishConnectionState(true);
    QVERIFY(renderedIconHasColor(*button, theme.color(ui::Sem::AddOk)));
    agent.publishConnectionState(false);
    QVERIFY(renderedIconHasColor(*button, theme.color(ui::Sem::Accent)));
  }
  theme.toggle();
  const auto finalColor = state == app::EnhancedModeState::Enabled ? ui::Sem::Accent : static_cast<ui::Sem>(expectedColor);
  QVERIFY(renderedIconHasColor(*button, theme.color(finalColor)));
}

void ApplicationEnhancedModeTests::mainWindowCreatesAndOwnsTheInjectedAgent() {
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  MutableApplicationStateSource source;
  int created = 0;
  QPointer<MemoryEnhancedModeAgent> agent;
  auto services = uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner, serviceControlOwner, nullptr, [&] {
    ++created;
    auto owned = std::make_unique<MemoryEnhancedModeAgent>();
    agent = owned.get();
    return owned;
  });
  {
    ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Unavailable});
    QCOMPARE(created, 1);
    QVERIFY(agent);
    QVERIFY(wmi.invocations.empty());
  }
  QVERIFY(agent.isNull());
}

void ApplicationEnhancedModeTests::mainWindowRetriesTransientEnhancedModeStatusFailures() {
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  MemoryEnhancedModeAgent agent;
  serviceControl.queryFailure = QStringLiteral("SCM temporarily unavailable");
  MutableApplicationStateSource source;
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;

  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner, serviceControlOwner, &agent),
                        {.uwfCapability = uwf::app::UwfCapability::Unavailable, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  window.resize(ui::design::smallMainWidth, ui::design::smallMainHeight);
  auto* enhancedAction = actionWithText(&window, QStringLiteral("Enhanced mode"));
  QVERIFY(enhancedAction);
  auto* enhancedButton = commandButtonForAction(window, enhancedAction);
  QVERIFY(enhancedButton);
  QTRY_COMPARE(enhancedButton->toolButtonStyle(), Qt::ToolButtonIconOnly);
  auto& theme = ui::ThemeManager::instance();
  QVERIFY(renderedIconHasColor(*enhancedButton, theme.color(ui::Sem::Warn)));
  QCOMPARE(serviceControl.queries, 1);

  serviceControl.queryFailure.clear();
  QTRY_VERIFY_WITH_TIMEOUT(serviceControl.queries >= 2, 2500);
  QVERIFY(renderedIconHasColor(*enhancedButton, theme.color(ui::Sem::Fg)));
}

void ApplicationEnhancedModeTests::mainWindowTreatsAuthenticatedServiceConnectionAsStatusOnly() {
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\pending.bin")}};
  auto serviceControlOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& serviceControl = *serviceControlOwner;
  serviceControl.current = completeEnhancedModeStatus();
  MemoryEnhancedModeAgent agent;
  MutableApplicationStateSource source;
  source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
  source.snapshot.uwfAvailable = false;
  source.snapshot.elevated = true;

  ui::MainWindow window(uwf::bootstrap::mainWindowServices(wmi, source, stagingOwner, serviceControlOwner, &agent),
                        {.uwfCapability = uwf::app::UwfCapability::Unavailable, .compatibilityMode = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(source.reads, 1, 1000);

  QTRY_VERIFY(refreshCompleted(window));
  QCOMPARE(agent.starts, 1);

  const int queriesBeforeAuthentication = serviceControl.queries;
  const int loadsBeforeAuthentication = staging.loads;
  agent.publishConnectionState(true);
  QCoreApplication::processEvents();
  QCOMPARE(serviceControl.queries, queriesBeforeAuthentication);
  QCOMPARE(staging.loads, loadsBeforeAuthentication);
  QVERIFY(wmi.queryResults.empty());
  QVERIFY(wmi.invocations.empty());
}

QTEST_MAIN(ApplicationEnhancedModeTests)
#include "EnhancedModeCases.moc"
