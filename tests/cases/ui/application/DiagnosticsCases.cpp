// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QDir>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTabWidget>
#include <QTableView>
#include <QTest>
#include <QTextEdit>
#include <QTimer>
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
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ApplicationUiFixture.h"
#include "ApplicationUiTest.h"
#include "app/api/UwfCapability.h"
#include "app/commit/CommitBatch.h"
#include "app/configuration/UwfmgrCommands.h"
#include "app/diagnostics/DiagnosticReportService.h"
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
#include "support/ui/DialogControls.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/WidgetCapture.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/commit/CommitReportDialog.h"
#include "ui/diagnostics/AboutDialog.h"
#include "ui/diagnostics/LogViewerDialog.h"
#include "ui/diagnostics/SystemInformationDialog.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/overlay/OverlayHudPalette.h"
#include "ui/shell/MainWindow.h"
#include "ui/shell/StatusSurface.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/SearchField.h"
#include "ui/widgets/SelectBox.h"
#include "ui/widgets/SummaryLabel.h"
#include "windows/system/SystemHardwareInfo.h"
#include "windows/system/WindowsShell.h"

class ApplicationDiagnosticsTests final : public ApplicationUiTest {
  Q_OBJECT
 private slots:
  void reportDetailsPreserveCompleteText_data();
  void reportDetailsPreserveCompleteText();
  void aboutDialogExposesVersionLicenseAndSafeClose();
  void diagnosticReportDistinguishesMissingStateAndBoundsLogs();
  void logBufferAndViewerPreserveMalformedAndStructuredLines();
  void logViewerFilteringSelectionAndClearConfirmation();
  void mainWindowMoreMenuOpensSystemInformation();
  void systemInformationFieldsPreserveLiteralSelectableText_data();
  void systemInformationFieldsPreserveLiteralSelectableText();
  void systemSummaryPrioritizesFieldsAndPinsCompleteStatus_data();
  void systemSummaryPrioritizesFieldsAndPinsCompleteStatus();
};

using namespace uwf;
using namespace uwf::test::application;

void ApplicationDiagnosticsTests::systemInformationFieldsPreserveLiteralSelectableText_data() {
  QTest::addColumn<bool>("dark");
  QTest::newRow("light") << false;
  QTest::newRow("dark") << true;
}

void ApplicationDiagnosticsTests::systemInformationFieldsPreserveLiteralSelectableText() {
  QFETCH(bool, dark);
  auto& theme = ui::ThemeManager::instance();
  const auto originalTheme = theme.current();
  const auto originalClipboard = QApplication::clipboard()->text();
  const auto restore = qScopeGuard([&] {
    theme.apply(originalTheme);
    QApplication::clipboard()->setText(originalClipboard);
  });
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  app::DiagnosticReportData report;
  report.system.windows = app::WindowsVersionInfo{};
  report.system.windows->productName = "Windows <b>Enterprise & managed deployment</b> with a long literal operating system name";
  const presentation::DiagnosticReportViewModel model(report, 8192);
  ui::SystemInformationDialog dialog(model);
  dialog.resize(720, 600);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  const auto values = dialog.findChildren<ui::ContentHintBox*>();
  for (const auto& section : model.sections()) {
    for (const auto& field : section.fields) {
      const auto found = std::ranges::find_if(values, [&](const auto* value) { return value->toPlainText() == field.value; });
      QVERIFY2(found != values.end(), qUtf8Printable(field.label + QStringLiteral(": ") + field.value));
      auto* value = *found;
      QCOMPARE(value->focusPolicy(), Qt::ClickFocus);
      QTRY_VERIFY(value->height() >= value->heightForWidth(value->width()));
      value->selectAll();
      QCOMPARE(value->textCursor().selectedText(), field.value);
      value->copy();
      QCOMPARE(QApplication::clipboard()->text(), field.value);
    }
  }
}

void ApplicationDiagnosticsTests::systemSummaryPrioritizesFieldsAndPinsCompleteStatus_data() {
  QTest::addColumn<QString>("product");
  QTest::addColumn<int>("windowWidth");
  QTest::addColumn<int>("visibleParts");
  QTest::addColumn<bool>("dark");
  for (const bool dark : {false, true}) {
    const auto suffix = dark ? "dark" : "light";
    QTest::newRow(qUtf8Printable(QStringLiteral("full-%1").arg(suffix))) << QStringLiteral("Windows 11 Pro") << 1280 << 3 << dark;
    QTest::newRow(qUtf8Printable(QStringLiteral("system-memory-%1").arg(suffix))) << QStringLiteral("Windows 11 Enterprise multi-session") << 1280 << 2 << dark;
    QTest::newRow(qUtf8Printable(QStringLiteral("system-only-%1").arg(suffix)))
        << QStringLiteral("Windows 11 Enterprise for Virtual Desktops") << 1120 << 1 << dark;
  }
}

void ApplicationDiagnosticsTests::systemSummaryPrioritizesFieldsAndPinsCompleteStatus() {
  QFETCH(QString, product);
  QFETCH(int, windowWidth);
  QFETCH(int, visibleParts);
  QFETCH(bool, dark);
  auto& language = ui::I18n::instance();
  auto& theme = ui::ThemeManager::instance();
  const auto originalLanguage = language.lang();
  const auto originalTheme = theme.current();
  const auto originalClipboard = QApplication::clipboard()->text();
  const auto restore = qScopeGuard([&] {
    language.setLang(originalLanguage);
    theme.apply(originalTheme);
    QApplication::clipboard()->setText(originalClipboard);
  });
  language.setLang(ui::I18n::Lang::Zh_CN);
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  class SystemInformation final : public app::SystemInformationSource {
   public:
    app::WindowsVersionInfo version;
    app::SystemHardwareInfo hardwareInfo;
    uint32_t totalRamMb() const override { return 32768; }
    QString systemDriveLetter() const override { return QStringLiteral("C:"); }
    const app::WindowsVersionInfo& windowsVersion() const override { return version; }
    const app::SystemHardwareInfo& hardware() const override { return hardwareInfo; }
    app::SystemDiagnosticFacts diagnostics() const override { return {}; }
  } information;
  information.version.productName = product.toStdString();
  information.hardwareInfo.totalMemoryBytes = 32ULL << 30;
  information.hardwareInfo.graphicsAdapter.name = "NVIDIA GeForce RTX 4060";
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.current.filter.enabled = false;
  source.snapshot.next.filter.enabled = false;
  source.snapshot.next.overlay = source.snapshot.current.overlay;
  source.volumes = {{"C:", R"(\\?\Volume{c}\)", "NTFS", "Windows", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported}};
  auto services = bootstrap::mainWindowServices(wmi, source, stagingOwner);
  services.viewModelFactory = bootstrap::createViewModelFactory(wmi, windows::systemRegistryBrowser(), windows::loadOverlayFilesFromSystem, information);
  ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available, .compatibilityMode = false, .hubVisible = false});
  window.show();
  QVERIFY(QTest::qWaitForWindowExposed(&window));
  window.resize(windowWidth, 900);
  QTRY_COMPARE(window.windowOpacity(), 1.0);
  auto* summary = window.findChild<ui::SummaryLabel*>(QStringLiteral("systemBrief"));
  auto* status = window.findChild<ui::StatusSurface*>();
  auto* label = window.findChild<QLabel*>(QStringLiteral("statusBarLabel"));
  auto* copy = window.findChild<QPushButton*>(QStringLiteral("statusCopyButton"));
  QVERIFY(summary && status && label && copy);
  auto* settings = window.findChild<QScrollArea*>(QStringLiteral("globalSettingsScroll"));
  QVERIFY(settings);
  QTRY_VERIFY2(([&] {
                 settings->ensureWidgetVisible(summary);
                 return settings->viewport()->rect().contains(QRect(summary->mapTo(settings->viewport(), QPoint()), summary->size()));
               })(),
               qUtf8Printable(QStringLiteral("summary %1,%2 %3x%4, viewport %5x%6, scroll %7/%8")
                                  .arg(summary->mapTo(settings->viewport(), QPoint()).x())
                                  .arg(summary->mapTo(settings->viewport(), QPoint()).y())
                                  .arg(summary->width())
                                  .arg(summary->height())
                                  .arg(settings->viewport()->width())
                                  .arg(settings->viewport()->height())
                                  .arg(settings->verticalScrollBar()->value())
                                  .arg(settings->verticalScrollBar()->maximum())));
  const QStringList parts{product, QStringLiteral("32G"), QStringLiteral("RTX 4060")};
  const auto fullText = parts.join(QStringLiteral(" · "));
  const auto expected = parts.first(visibleParts).join(QStringLiteral(" · "));
  QTRY_COMPARE(summary->text(), expected);
  QVERIFY(summary->fontMetrics().horizontalAdvance(expected) <= summary->contentsRect().width());
  QCOMPARE(summary->fullText(), fullText);
  QTest::mouseClick(summary, Qt::LeftButton);
  QTRY_COMPARE(label->text(), fullText);
  QCOMPARE(status->detail(), fullText);
  // Move to an unhinted surface. A fixed central-widget coordinate can hit
  // a file row and correctly replace the pinned baseline with its hover hint.
  uwf::test::moveMouse(label);
  QEvent leave(QEvent::Leave);
  QApplication::sendEvent(summary, &leave);
  QTest::qWait(180);
  QCOMPARE(label->text(), fullText);
  QVERIFY(copy->isVisible());
  QTest::mouseClick(copy, Qt::LeftButton);
  QCOMPARE(QApplication::clipboard()->text(), fullText);
  QCOMPARE(label->text(), ui::I18n::tr("Copied to clipboard"));
  QVERIFY(copy->isHidden());
  QVERIFY(test::saveScreenshot(window, QStringLiteral("system-summary")));
  const auto screenshots = qEnvironmentVariable("UWF_TEST_SCREENSHOT_DIR");
  if (!screenshots.isEmpty()) {
    test::prepareScreenshotWindow(window);
    QTest::qWait(80);
    QVERIFY(test::windowScreenshot(window).save(
        QDir(screenshots).filePath(QStringLiteral("native-system-summary-") + QString::fromLatin1(QTest::currentDataTag()) + QStringLiteral(".png"))));
  }
  window.resize(windowWidth == 1280 ? 1120 : 1280, 900);
  QTest::qWait(100);
  QCOMPARE(status->detail(), fullText);
  QCOMPARE(label->text(), ui::I18n::tr("Copied to clipboard"));
  QTRY_COMPARE_WITH_TIMEOUT(label->text(), fullText, 4000);
  window.resize(windowWidth, 900);
  QTRY_COMPARE(summary->text(), expected);
  QCOMPARE(wmi.invocations.size(), std::size_t{0});
  QCOMPARE(staging.writes, 0);
}

void ApplicationDiagnosticsTests::mainWindowMoreMenuOpensSystemInformation() {
  RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  MutableApplicationStateSource source;
  source.snapshot = editableSnapshot();
  source.snapshot.current.filter.enabled = false;
  source.snapshot.next.filter.enabled = false;
  ui::MainWindow window(bootstrap::mainWindowServices(wmi, source, stagingOwner),
                        {.uwfCapability = app::UwfCapability::Available, .compatibilityMode = false, .hubVisible = false});
  window.show();
  QTRY_COMPARE_WITH_TIMEOUT(window.windowOpacity(), 1.0, 2000);
  QSignalSpy parentVisibility(window.windowHandle(), &QWindow::visibleChanged);
  auto* more = actionWithText(&window, ui::I18n::tr("More"));
  QVERIFY(more && more->menu());
  auto* action = more->menu()->findChild<QAction*>(QStringLiteral("systemInformationAction"));
  QVERIFY(action && action->isEnabled());
  bool inspected = false;
  const QString previousClipboard = QApplication::clipboard()->text();
  const auto restoreClipboard = qScopeGuard([&] { QApplication::clipboard()->setText(previousClipboard); });
  QTimer::singleShot(0, &window, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QVERIFY(dialog && dialog->objectName() == QStringLiteral("systemInformationDialog"));
    QVERIFY(window.isVisible());
    const auto close = qScopeGuard([&] { dialog->reject(); });
    auto* tabs = dialog->findChild<QTabWidget*>(QStringLiteral("systemInformationTabs"));
    auto* report = dialog->findChild<QTextEdit*>(QStringLiteral("systemInformationReport"));
    auto* copy = dialog->findChild<QPushButton*>(QStringLiteral("systemInformationCopyButton"));
    QVERIFY(tabs && tabs->count() == 2 && report && report->isReadOnly() && copy);
    QVERIFY(!report->toPlainText().isEmpty());
    QTest::mouseClick(copy, Qt::LeftButton);
    QCOMPARE(QApplication::clipboard()->text(), report->toPlainText());
    inspected = true;
  });
  action->trigger();
  QVERIFY(inspected);
  QTimer::singleShot(0, &window, [&] {
    auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QVERIFY(dialog && window.isVisible());
    dialog->reject();
  });
  auto* about = actionWithText(&window, ui::I18n::tr("About"));
  QVERIFY(about);
  about->trigger();
  for (const auto& change : std::as_const(parentVisibility)) QVERIFY(change.front().toBool());
  QVERIFY(wmi.invocations.empty());
  QCOMPARE(staging.writes, 0);
}

void ApplicationDiagnosticsTests::reportDetailsPreserveCompleteText_data() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  for (const auto& language : ui::I18n::languages()) {
    if (language.id != ui::I18n::Lang::En && language.id != ui::I18n::Lang::Zh_CN && language.id != ui::I18n::Lang::Ar) continue;
    for (const bool dark : {false, true}) QTest::newRow((QString(language.code) + (dark ? "-dark" : "-light")).toLatin1().constData()) << language.id << dark;
  }
}

void ApplicationDiagnosticsTests::reportDetailsPreserveCompleteText() {
  QFETCH(ui::I18n::Lang, locale);
  QFETCH(bool, dark);
  auto& language = ui::I18n::instance();
  auto& theme = ui::ThemeManager::instance();
  const auto oldLanguage = language.lang();
  const auto oldTheme = theme.current();
  const auto restore = qScopeGuard([&] {
    language.setLang(oldLanguage);
    theme.apply(oldTheme);
    clearLogLines();
  });
  language.setLang(locale);
  theme.apply(dark ? app::Theme::Dark : app::Theme::Light);
  const QString fullText = QStringLiteral("<literal>& data 中文 %1\n") + QStringLiteral("long-details-").repeated(1000);
  const auto inspectInline = [&](QDialog& owner, QTableView* table, ui::ContentHintBox* details) {
    const auto geometry = owner.geometry();
    const QPointer<QWidget> previousModal = QApplication::activeModalWidget();
    bool unexpectedDialog = false;
    QTimer guard;
    guard.setInterval(5);
    connect(&guard, &QTimer::timeout, this, [&] {
      if (auto* modal = qobject_cast<QDialog*>(QApplication::activeModalWidget()); modal && modal != previousModal) {
        unexpectedDialog = true;
        modal->reject();
      }
    });
    guard.start();
    const auto point = table->visualRect(table->currentIndex()).center();
    QTest::mouseDClick(table->viewport(), Qt::LeftButton, Qt::NoModifier, point);
    QTest::mouseRelease(table->viewport(), Qt::LeftButton, Qt::NoModifier, point);
    table->setFocus();
    QTest::keyClick(table, Qt::Key_Return);
    QCoreApplication::processEvents();
    guard.stop();
    QVERIFY(!unexpectedDialog);
    QCOMPARE(QApplication::activeModalWidget(), previousModal.data());
    QVERIFY(owner.isVisible());
    QCOMPARE(owner.geometry(), geometry);
    QVERIFY(details->toPlainText().contains(fullText));
  };
  clearLogLines();
  logLine('E', "details-test", fullText.toStdString());
  {
    ui::LogViewerDialog dialog(uwf::test::screenshotLogModel());
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* table = dialog.findChild<QTableView*>();
    QVERIFY(table && !table->showGrid());
    QTRY_VERIFY(table->model()->rowCount() > 0);
    QTest::qWait(80);
    QVERIFY(!table->model()->index(0, 1).data(uwf::presentation::StatusIconRole).isNull());
    QCOMPARE(table->model()->index(0, 1).data(presentation::FullTextRole).toString(), QStringLiteral("E"));
    table->clearSelection();
    QVERIFY(table->selectionModel()->selectedIndexes().isEmpty());
    QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("interaction-log-unselected")));
    const auto position = table->viewport()->mapTo(&dialog, table->visualRect(table->model()->index(0, 3)).center());
    QTest::mouseClick(dialog.windowHandle(), Qt::LeftButton, Qt::NoModifier, position);
    QTRY_VERIFY(!table->selectionModel()->selectedIndexes().isEmpty());
    QVERIFY(uwf::test::saveScreenshot(dialog, QStringLiteral("interaction-log-selected")));
    auto* inlineDetails = dialog.findChild<ui::ContentHintBox*>(QStringLiteral("logMessageDetails"));
    auto* copyEntry = dialog.findChild<QPushButton*>(QStringLiteral("copyLogEntry"));
    QVERIFY(inlineDetails && inlineDetails->isVisible() && copyEntry && copyEntry->isEnabled());
    QVERIFY(inlineDetails->toPlainText().contains(fullText));
    copyEntry->click();
    QVERIFY(QApplication::clipboard()->text().contains(fullText));
    inspectInline(dialog, table, inlineDetails);
  }
  bool reportInspected = false;
  QTimer::singleShot(0, this, [&] {
    auto* report = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QVERIFY(report);
    const auto close = qScopeGuard([&] { report->reject(); });
    auto* table = report->findChild<QTableView*>();
    QVERIFY(table && !table->showGrid());
    QCOMPARE(table->model()->rowCount(), 1);
    QVERIFY(!table->model()->index(0, 0).data(uwf::presentation::StatusIconRole).isNull());
    table->clearSelection();
    QVERIFY(table->selectionModel()->selectedIndexes().isEmpty());
    QVERIFY(uwf::test::saveScreenshot(*report, QStringLiteral("interaction-report-unselected")));
    const auto position = table->viewport()->mapTo(report, table->visualRect(table->model()->index(0, 0)).center());
    QTest::mouseClick(report->windowHandle(), Qt::LeftButton, Qt::NoModifier, position);
    QTRY_VERIFY(!table->selectionModel()->selectedIndexes().isEmpty());
    QVERIFY(uwf::test::saveScreenshot(*report, QStringLiteral("interaction-report-selected")));
    auto* details = report->findChild<ui::ContentHintBox*>(QStringLiteral("commitReportDetails"));
    QVERIFY(details && details->toPlainText().contains(fullText));
    auto* copyRow = report->findChild<QPushButton*>(QStringLiteral("commitReportCopyRowButton"));
    QVERIFY(copyRow && copyRow->isEnabled());
    QTest::mouseClick(copyRow, Qt::LeftButton);
    QVERIFY(QApplication::clipboard()->text().contains(fullText));
    table->setFocus();
    inspectInline(*report, table, details);
    reportInspected = true;
  });
  ui::showCommitReport(nullptr,
                       {{uwf::presentation::CommitRowStatus::Failed, QStringLiteral("C:\\Example.txt"), QStringLiteral("0x80041001"), fullText, true, true}},
                       app::CommitOperation::Persist);
  QVERIFY(reportInspected);
}

void ApplicationDiagnosticsTests::aboutDialogExposesVersionLicenseAndSafeClose() {
  ui::AboutDialog dialog([] { return uwf::bootstrap::createViewModelFactory()->systemInformation()->diagnosticDocument({}); });
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  bool hasVersion = false;
  bool hasLicense = false;
  bool hasSourceLink = false;
  for (const auto labels = dialog.findChildren<QLabel*>(); auto* label : labels) {
    hasVersion = hasVersion || label->text().contains(QStringLiteral("Version "));
  }
  for (const auto texts = dialog.findChildren<ui::ContentHintBox*>(); auto* text : texts)
    hasLicense = hasLicense || text->toPlainText().contains(QStringLiteral("GNU General Public License"));
  for (const auto buttons = dialog.findChildren<QPushButton*>(); auto* button : buttons)
    hasSourceLink = hasSourceLink || (button->isVisible() && button->toolTip() == QStringLiteral("https://github.com/HsingYun/UWF-Manager"));
  QVERIFY(hasVersion);
  QVERIFY(hasLicense);
  QVERIFY(hasSourceLink);
  auto* license = dialog.findChild<ui::Disclosure*>(QStringLiteral("aboutLicense"));
  QVERIFY(license);
  QTest::mouseClick(license->toggleButton(), Qt::LeftButton);
  QVERIFY(license->isOpen());
  auto* licenseText = license->findChild<ui::ContentHintBox*>();
  QVERIFY(licenseText && licenseText->isVisible());
  QVERIFY(licenseText->toHtml().contains(QStringLiteral("https://www.gnu.org/licenses/gpl-3.0.html")));
  auto* copy = buttonWithText(&dialog, QStringLiteral("Copy version"));
  QVERIFY(copy);
  QTest::mouseClick(copy, Qt::LeftButton);
  QVERIFY(QApplication::clipboard()->text().startsWith(QStringLiteral("UWF Manager ")));
  QVERIFY(QApplication::clipboard()->text().contains(QStringLiteral("Qt %1").arg(QString::fromLatin1(qVersion()))));
  QVERIFY(buttonWithText(&dialog, QStringLiteral("System information")));
  auto* save = dialog.findChild<QPushButton*>(QStringLiteral("aboutSaveConfigButton"));
  QVERIFY(save && save->isVisible() && !save->isEnabled());
  QVERIFY(save->mapTo(&dialog, QPoint()).x() > copy->mapTo(&dialog, QPoint()).x());
  using Outcome = app::TextFileCreationResult::Outcome;
  presentation::UserConfigExportViewModel::Result exportResult{Outcome::Created, QStringLiteral("C:/Preview/UWF.ini"), {}};
  int exports = 0;
  int throwFailure = 0;
  dialog.setConfigurationExporter([&]() -> presentation::UserConfigExportViewModel::Result {
    ++exports;
    if (throwFailure == 1) throw std::runtime_error("Injected export failure");
    if (throwFailure == 2) throw 42;
    return exportResult;
  });
  QVERIFY(save->isEnabled());
  const auto inspectFeedback = [&](const QString& title, const QString& expectedText) {
    bool observed = false;
    QTimer::singleShot(0, &dialog, [&] {
      auto* feedback = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      QVERIFY(feedback && feedback != &dialog);
      const auto dismiss = qScopeGuard([feedback] { feedback->reject(); });
      QCOMPARE(feedback->windowTitle(), title);
      QString text;
      for (const auto labels = feedback->findChildren<QLabel*>(); auto* label : labels) text += label->text() + u'\n';
      for (const auto details = feedback->findChildren<ui::ContentHintBox*>(); auto* detail : details) text += detail->toPlainText() + u'\n';
      QVERIFY2(text.contains(expectedText), qUtf8Printable(text));
      observed = true;
    });
    save->click();
    QVERIFY(observed);
    QVERIFY(dialog.isVisible());
  };
  inspectFeedback(QStringLiteral("Configuration saved"), QStringLiteral("Saved the current interface settings to:\nC:\\Preview\\UWF.ini"));
  exportResult.outcome = Outcome::AlreadyExists;
  inspectFeedback(QStringLiteral("Export configuration"), QStringLiteral("UWF.ini already exists. The existing file was not changed:\nC:\\Preview\\UWF.ini"));
  exportResult.outcome = Outcome::Failed;
  exportResult.error = QStringLiteral("Access denied (0x80070005)");
  inspectFeedback(QStringLiteral("Export failed"), QStringLiteral("Could not save UWF.ini:\nC:\\Preview\\UWF.ini\nAccess denied (0x80070005)"));
  throwFailure = 1;
  inspectFeedback(QStringLiteral("Export failed"), QStringLiteral("Injected export failure"));
  throwFailure = 2;
  inspectFeedback(QStringLiteral("Export failed"), QStringLiteral("The operation failed with an unknown error."));
  QCOMPARE(exports, 5);
  dialog.setConfigurationExporter({});
  QVERIFY(!save->isEnabled());
  auto* close = dialog.closeButton();
  QVERIFY(close);
  QSignalSpy finished(&dialog, &QDialog::finished);
  QTest::mouseClick(close, Qt::LeftButton);
  QCOMPARE(finished.count(), 1);
  QCOMPARE(finished.at(0).at(0).toInt(), static_cast<int>(QDialog::Rejected));
}

void ApplicationDiagnosticsTests::diagnosticReportDistinguishesMissingStateAndBoundsLogs() {
  clearLogLines();
  const QString missingReport = uwf::bootstrap::createViewModelFactory()->systemInformation()->diagnosticReport({});
  QVERIFY(missingReport.contains(QStringLiteral("snapshot.available = no")));
  QVERIFY(!missingReport.contains(QStringLiteral("user.account = <empty>")));
  QVERIFY(!missingReport.contains(QStringLiteral("user.sid = <empty>")));
  QVERIFY(missingReport.contains(QStringLiteral("user.account = ")) || missingReport.contains(QStringLiteral("user.account.error = ")) ||
          missingReport.contains(QStringLiteral("user.error = ")));
  QVERIFY(missingReport.contains(QStringLiteral("user.sid = ")) || missingReport.contains(QStringLiteral("user.sid.error = ")) ||
          missingReport.contains(QStringLiteral("user.error = ")));
  QVERIFY(missingReport.contains(QStringLiteral("integrity.level = ")) || missingReport.contains(QStringLiteral("integrity.error = ")));

  const core::UwfSnapshot snapshot = editableSnapshot();
  const app::EnhancedModeStatus enhancedMode = completeEnhancedModeStatus();
  const auto context = app::DiagnosticReportService::capture(uwf::app::UwfCapability::Available, &snapshot, &enhancedMode, true);
  QString report = uwf::bootstrap::createViewModelFactory()->systemInformation()->diagnosticReport(context);
  QVERIFY(report.contains(QStringLiteral("snapshot.available = yes")));
  QVERIFY(report.contains(QStringLiteral("current.filter.enabled = yes")));
  QVERIFY(report.contains(QStringLiteral("next.overlay.type = disk")));
  QVERIFY(report.contains(QStringLiteral("service.contract_satisfied = yes")));

  clearLogLines();
  logLine('I', "diagnostic-test", "diagnostic-oldest-marker");
  for (int index = 0; index < 68; ++index) logLine('I', "diagnostic-test", "diagnostic-middle-" + std::to_string(index));
  logLine('W', "diagnostic-test", "diagnostic-newest-marker-" + std::string(2048, 'x'));
  report = uwf::bootstrap::createViewModelFactory()->systemInformation()->diagnosticReport(context);
  QVERIFY(report.contains(QStringLiteral("lines.count = 64")));
  QVERIFY(!report.contains(QStringLiteral("diagnostic-oldest-marker")));
  QVERIFY(report.contains(QStringLiteral("diagnostic-newest-marker")));
  QVERIFY(report.contains(QStringLiteral("... <truncated>")));
  clearLogLines();
}

void ApplicationDiagnosticsTests::logBufferAndViewerPreserveMalformedAndStructuredLines() {
  clearLogLines();
  logLine('I', "test", "structured");
  logLine('W', "test", "warning");
  const auto raw = recentLogLines();
  QCOMPARE(raw.size(), std::size_t{2});
  QVERIFY(raw.front().find(" I test]") != std::string::npos);

  ui::LogViewerDialog dialog(uwf::test::screenshotLogModel());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* table = dialog.findChild<QTableView*>();
  QVERIFY(table);
  QTRY_COMPARE_WITH_TIMEOUT(table->model()->rowCount(), 2, 2000);
  QCOMPARE(table->model()->index(0, 1).data(presentation::FullTextRole).toString(), QStringLiteral("I"));
  QCOMPARE(table->model()->index(1, 3).data().toString(), QStringLiteral("warning"));

  auto* copyAll = buttonWithText(&dialog, QStringLiteral("Copy all"));
  QVERIFY(copyAll);
  QTest::mouseClick(copyAll, Qt::LeftButton);
  QVERIFY(QApplication::clipboard()->text().contains(QStringLiteral("structured")));
  QVERIFY(QApplication::clipboard()->text().contains(QStringLiteral("warning")));

  auto* clear = buttonWithText(&dialog, QStringLiteral("Clear…"));
  QVERIFY(clear);
  QTimer::singleShot(0, this, [&] {
    auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QVERIFY(confirmation);
    auto* accept = test::buttonByRole(confirmation, QDialogButtonBox::AcceptRole);
    QVERIFY(accept);
    QTest::mouseClick(accept, Qt::LeftButton);
  });
  QTest::mouseClick(clear, Qt::LeftButton);
  QTRY_COMPARE_WITH_TIMEOUT(recentLogLines().size(), std::size_t{0}, 1000);
  dialog.close();
}

void ApplicationDiagnosticsTests::logViewerFilteringSelectionAndClearConfirmation() {
  class Records final : public app::LogSource {
   public:
    std::vector<std::string> lines{"[01:02:03.456 I main] Application started",
                                   "[01:02:04.567 E registry] Access denied\nHKEY_LOCAL_MACHINE\\SECURITY\n0x80070005",
                                   "[01:02:05.678 W staging] Skipped excluded path"};
    int clearCalls = 0;
    std::vector<std::string> recentLines() const override { return lines; }
    void clear() override {
      ++clearCalls;
      lines.clear();
    }
  };
  auto records = std::make_shared<Records>();
  auto model = std::make_shared<presentation::LogViewModel>(records);
  ui::LogViewerDialog dialog(model);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QTRY_VERIFY_WITH_TIMEOUT(!model->loading(), 2500);
  auto* table = dialog.findChild<QTableView*>(QStringLiteral("logTable"));
  auto* search = dialog.findChild<ui::SearchField*>(QStringLiteral("logSearch"));
  auto* levels = dialog.findChild<ui::SelectBox*>(QStringLiteral("logLevelFilter"));
  auto* details = dialog.findChild<ui::ContentHintBox*>(QStringLiteral("logMessageDetails"));
  QVERIFY(table && search && levels && details);
  QTRY_COMPARE_WITH_TIMEOUT(table->model()->rowCount(), 3, 1000);
  QTRY_COMPARE(table->viewport()->height() % table->verticalHeader()->defaultSectionSize(), 0);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  const QRect originalDetails = details->geometry();
  const QRect originalTable = table->geometry();
  table->selectRow(1);
  const QString fullMessage = QStringLiteral("Access denied\nHKEY_LOCAL_MACHINE\\SECURITY\n0x80070005");
  QCOMPARE(details->toPlainText(), fullMessage);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  QCOMPARE(details->geometry(), originalDetails);
  QCOMPARE(table->geometry(), originalTable);
  QTest::mouseClick(table->viewport(), Qt::LeftButton, Qt::NoModifier, table->visualRect(table->model()->index(1, 3)).center());
  QTest::mouseDClick(table->viewport(), Qt::LeftButton, Qt::NoModifier, table->visualRect(table->model()->index(1, 3)).center());
  QTest::keyClick(table, Qt::Key_Return);
  QVERIFY(!QApplication::activeModalWidget());
  QCOMPARE(table->model()->index(1, 3).data().toString(), QStringLiteral("Access denied"));
  auto* copy = dialog.findChild<QPushButton*>(QStringLiteral("copyLogEntry"));
  QVERIFY(copy);
  QTest::mouseClick(copy, Qt::LeftButton);
  QVERIFY(QApplication::clipboard()->text().contains(fullMessage));
  search->setText(QStringLiteral("security"));
  QCOMPARE(table->model()->rowCount(), 1);
  QCOMPARE(details->toPlainText(), fullMessage);
  table->setFocus();
  QTest::keyClick(table, Qt::Key_C, Qt::ControlModifier);
  QVERIFY(QApplication::clipboard()->text().contains(fullMessage));
  levels->setCurrentIndex(levels->findData(static_cast<int>(presentation::LogLevelFilter::Warning)));
  QCOMPARE(table->model()->rowCount(), 0);
  auto* reset = buttonWithText(&dialog, QStringLiteral("Clear filters"));
  QVERIFY(reset && reset->isVisible());
  QTest::mouseClick(reset, Qt::LeftButton);
  QCOMPARE(table->model()->rowCount(), 3);
  QVERIFY(model->query().isEmpty());
  QCOMPARE(model->levelFilter(), presentation::LogLevelFilter::All);
  auto* clear = buttonWithText(&dialog, QStringLiteral("Clear…"));
  QVERIFY(clear);
  for (const bool accept : {false, true}) {
    QTimer::singleShot(0, this, [accept] {
      auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      QVERIFY(confirmation);
      auto* cancel = test::buttonByRole(confirmation, QDialogButtonBox::RejectRole);
      auto* proceed = test::buttonByRole(confirmation, QDialogButtonBox::AcceptRole);
      QVERIFY(cancel && proceed && cancel->isDefault());
      if (accept)
        QTest::mouseClick(proceed, Qt::LeftButton);
      else
        QTest::keyClick(confirmation, Qt::Key_Return);
    });
    QTest::mouseClick(clear, Qt::LeftButton);
    QCOMPARE(records->clearCalls, accept ? 1 : 0);
    if (!accept) QCOMPARE(table->model()->rowCount(), 3);
  }
  QTRY_VERIFY_WITH_TIMEOUT(!model->loading(), 2500);
  QCOMPARE(table->model()->rowCount(), 0);
  QVERIFY(!clear->isEnabled());
  dialog.close();
}

QTEST_MAIN(ApplicationDiagnosticsTests)
#include "DiagnosticsCases.moc"
