// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPushButton>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTextEdit>
#include <QThreadPool>
#include <QTimer>
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
#include "support/app/WorkerGate.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/LocalFileDialogs.h"
#include "support/ui/TextLayoutChecks.h"
#include "support/ui/WidgetInput.h"
#include "support/visual/WidgetCapture.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/configuration/ApplyPlanDialog.h"
#include "ui/configuration/ExclusionListWidget.h"
#include "ui/configuration/GlobalStatusPanel.h"
#include "ui/configuration/VolumeTab.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/overlay/OverlayHudPalette.h"
#include "ui/staging/FileStagingWidget.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/SwitchButton.h"
#include "windows/system/SystemHardwareInfo.h"
#include "windows/system/WindowsShell.h"

class ApplicationConfigurationTests final : public ApplicationUiTest {
  Q_OBJECT
 private slots:
  void configurationVisualStates_data();
  void configurationVisualStates();
  void diskTabsApplyCapabilityBoundariesAndPreserveInnerSelection();
  void diskHeadingKeepsVolumeMetadataAsLiteralText_data();
  void diskHeadingKeepsVolumeMetadataAsLiteralText();
  void diskTabKeepsExclusionsBoundToEachSessionVolume();
  void importRoutingAndPendingCollectionCoverInvalidDuplicateAndMissingTargets();
  void importCommandsPreserveOrderAcrossRepeatedTargets();
  void importGuardedCommandsPreserveDependencies();
  void applyPlanPreviewAndCopyUseTheSameProductionCommandMapping();
  void applyPlanConfirmedWritePublishesReconciliationAndPreventsReplay();
  void applyPlanConnectionFailureRemainsRetryableAndDoesNotRequestReconciliation();
  void applyPlanCloseRequestsRespectTheExecutionPhase_data();
  void applyPlanCloseRequestsRespectTheExecutionPhase();
  void applyPlanOverlayWritesRespectProviderConstraints_data();
  void applyPlanOverlayWritesRespectProviderConstraints();
  void applyPlanDependencyFailureIsLocalized_data();
  void applyPlanDependencyFailureIsLocalized();
};

using namespace uwf;
using namespace uwf::test::application;

void ApplicationConfigurationTests::diskTabsApplyCapabilityBoundariesAndPreserveInnerSelection() {
  const core::VolumeInfo supported{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  const core::VolumeInfo limited{"D:", "Volume{d}", "exFAT", "Data", 1000, 500, core::VolumeSupport::FileSystemLimited};
  const core::VolumeInfo unsupported{"E:", "Volume{e}", "NTFS", "USB", 1000, 500, core::VolumeSupport::NotFixedLocalDisk};
  ui::VolumeTab c(std::make_shared<presentation::VolumeConfigurationViewModel>(supported, true), uwf::test::localFileDialogs(), nullptr,
                  uwf::bootstrap::createViewModelFactory());
  ui::VolumeTab d(std::make_shared<presentation::VolumeConfigurationViewModel>(limited, false), uwf::test::localFileDialogs(), nullptr,
                  uwf::bootstrap::createViewModelFactory());
  ui::VolumeTab e(std::make_shared<presentation::VolumeConfigurationViewModel>(unsupported, false), uwf::test::localFileDialogs(), nullptr,
                  uwf::bootstrap::createViewModelFactory());
  const auto snapshot = editableSnapshot();
  c.viewModel().applySnapshot(snapshot);
  d.viewModel().applySnapshot(snapshot);
  e.viewModel().applySnapshot(snapshot);

  QVERIFY(c.supported());
  QVERIFY(c.canManageExclusions());
  QVERIFY(d.supported());
  QVERIFY(!d.canManageExclusions());
  QVERIFY(!e.supported());
  for (const QString& text : {QStringLiteral("Commit file changes…"), QStringLiteral("Commit folder changes…"), QStringLiteral("Delete and commit file…"),
                              QStringLiteral("Delete and commit folder…")}) {
    auto* limitedAction = actionWithText(&d, text);
    auto* unsupportedAction = actionWithText(&e, text);
    QVERIFY(limitedAction && !limitedAction->isEnabled());
    QVERIFY(unsupportedAction && !unsupportedAction->isEnabled());
  }
  auto* innerTabs = c.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(innerTabs);
  QCOMPARE(innerTabs->count(), 2);
  c.setActiveInfoPage(ui::VolumeTab::InfoPage::RegistryExclusions);
  QCOMPARE(c.activeInfoPage(), ui::VolumeTab::InfoPage::RegistryExclusions);
  c.setActiveInfoPage(ui::VolumeTab::InfoPage::FileStaging);
  QCOMPARE(c.activeInfoPage(), ui::VolumeTab::InfoPage::RegistryExclusions);

  FileDialogStub dialogs;
  auto stagingOwner = std::make_shared<MemoryFileStagingStore>();
  app::FileStagingDraft stagingDraft(stagingOwner);
  ui::VolumeTab stagedSupported(std::make_shared<presentation::VolumeConfigurationViewModel>(supported, false), dialogs, &stagingDraft,
                                uwf::bootstrap::createViewModelFactory());
  ui::VolumeTab stagedLimited(std::make_shared<presentation::VolumeConfigurationViewModel>(limited, false), dialogs, &stagingDraft,
                              uwf::bootstrap::createViewModelFactory());
  ui::VolumeTab stagedUnsupported(std::make_shared<presentation::VolumeConfigurationViewModel>(unsupported, false), dialogs, &stagingDraft,
                                  uwf::bootstrap::createViewModelFactory());
  QCOMPARE(stagedSupported.findChildren<ui::FileStagingWidget*>().size(), 1);
  QCOMPARE(stagedLimited.findChildren<ui::FileStagingWidget*>().size(), 0);
  QCOMPARE(stagedUnsupported.findChildren<ui::FileStagingWidget*>().size(), 0);
  QVERIFY(!presentation::volumeSupportText(core::VolumeSupport::FileSystemLimited, "exFAT").isEmpty());
  QVERIFY(!presentation::volumeSupportText(core::VolumeSupport::ExceedsMaxSize, "NTFS").isEmpty());
}

void ApplicationConfigurationTests::diskHeadingKeepsVolumeMetadataAsLiteralText_data() {
  QTest::addColumn<QString>("label");
  QTest::newRow("empty") << QString{};
  QTest::newRow("entity") << QStringLiteral("Work &copy; Archive");
  QTest::newRow("markup") << QStringLiteral("<b>Data & logs</b>");
  QTest::newRow("unicode") << QStringLiteral("资料 &amp; Данные");
}

void ApplicationConfigurationTests::diskHeadingKeepsVolumeMetadataAsLiteralText() {
  QFETCH(QString, label);
  const core::VolumeInfo volume{"C:", "Volume{c&copy;}", "NTFS &amp; metadata", label.toStdString(), 1000, 500, core::VolumeSupport::Supported};
  ui::VolumeTab tab(std::make_shared<presentation::VolumeConfigurationViewModel>(volume, false), uwf::test::localFileDialogs(), nullptr,
                    uwf::bootstrap::createViewModelFactory());
  auto* heading = tab.findChild<QLabel*>(QStringLiteral("diskHeading"));
  QVERIFY(heading);
  QCOMPARE(heading->textFormat(), Qt::PlainText);
  QCOMPARE(heading->text(), QStringLiteral("C:"));
  const auto* volumeLabel = tab.findChild<QLabel*>(QStringLiteral("diskVolumeLabel"));
  QVERIFY(volumeLabel);
  QCOMPARE(volumeLabel->textFormat(), Qt::PlainText);
  QCOMPARE(volumeLabel->text(), label);
  const auto labels = tab.findChildren<QLabel*>();
  QVERIFY(std::ranges::any_of(
      labels, [](const QLabel* text) { return text->textFormat() == Qt::PlainText && text->text().contains(QStringLiteral("NTFS &amp; metadata")); }));
  QCOMPARE(heading->toolTip(), QStringLiteral("Volume{c&copy;}"));
  auto* overlay = buttonWithText(&tab, ui::I18n::tr("View overlay"));
  auto* commit = tab.findChild<QPushButton*>(QStringLiteral("volumeCommitButton"));
  QVERIFY(overlay && commit);
  for (const int width : {600, 900}) {
    tab.resize(width, 420);
    tab.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tab));
    QVERIFY(test::saveScreenshot(tab, QStringLiteral("volume-heading-%1").arg(width)));
    // Metadata owns the available width. Its content must never center the
    // drive or stretch the action buttons when the optional label disappears.
    QTRY_COMPARE(heading->mapTo(&tab, QPoint()).x(), 0);
    QTRY_COMPARE(overlay->width(), overlay->sizeHint().width());
    QTRY_COMPARE(commit->width(), commit->sizeHint().width());
    QTRY_COMPARE(commit->mapTo(&tab, QPoint(commit->width(), 0)).x(), tab.width());
  }
}

void ApplicationConfigurationTests::diskTabKeepsExclusionsBoundToEachSessionVolume() {
  const core::VolumeInfo disk{"C:", "", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  ui::VolumeTab tab(std::make_shared<presentation::VolumeConfigurationViewModel>(disk, false), uwf::test::localFileDialogs(), nullptr,
                    uwf::bootstrap::createViewModelFactory());
  auto snapshot = editableSnapshot();
  snapshot.current.volumes.front().volumeName = "Volume{mounted-guid}";
  snapshot.next.volumes.front().volumeName = "Volume{old}";
  snapshot.next.volumes.front().bindByDriveLetter = true;
  snapshot.current.fileExclusions = {{"Volume{mounted-guid}", {R"(C:\CurrentOnly)"}}, {"Volume{old}", {R"(C:\WrongCurrent)"}}};
  snapshot.next.fileExclusions = {{"Volume{old}", {R"(C:\NextOnly)"}}, {"Volume{mounted-guid}", {R"(C:\WrongNext)"}}};
  tab.viewModel().applySnapshot(snapshot);
  tab.resize(700, 450);
  tab.show();
  QVERIFY(QTest::qWaitForWindowExposed(&tab));
  auto* tabs = tab.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(tabs);
  auto* files = tabs->widget(0)->findChild<ui::ExclusionListWidget*>();
  QVERIFY(files);
  auto* list = files->findChild<QListView*>(QStringLiteral("exclusionList"));
  QVERIFY(list);
  QCOMPARE(list->model()->rowCount(), 2);
  bool currentObserved = false;
  bool nextObserved = false;
  for (int index = 0; index < list->model()->rowCount(); ++index) {
    const auto item = list->model()->index(index, 0);
    const auto path = item.data(Qt::UserRole).toString();
    if (path == QStringLiteral(R"(C:\CurrentOnly)")) {
      currentObserved = true;
      QVERIFY(item.data(Qt::ToolTipRole).toString().contains(QStringLiteral("Current session: Excluded\nNext session: Not excluded")));
    } else if (path == QStringLiteral(R"(C:\NextOnly)")) {
      nextObserved = true;
      QVERIFY(item.data(Qt::ToolTipRole).toString().contains(QStringLiteral("Current session: Not excluded\nNext session: Excluded")));
    } else {
      QFAIL("the displayed exclusions came from the wrong session volume");
    }
  }
  QVERIFY(currentObserved);
  QVERIFY(nextObserved);
  QVERIFY(files->pendingAdded().isEmpty());
  QVERIFY(files->pendingRemoved().isEmpty());
  const auto labels = tab.findChildren<QLabel*>();
  QVERIFY(std::ranges::any_of(labels, [](const QLabel* label) {
    return label->text().contains(QStringLiteral("mounted-guid")) || label->toolTip().contains(QStringLiteral("mounted-guid"));
  }));

  const auto nextVolumes = snapshot.next.volumes;
  snapshot.next.volumes.clear();
  tab.viewModel().applySnapshot(snapshot);
  QCOMPARE(list->model()->rowCount(), 1);
  QCOMPARE(list->model()->index(0, 0).data(Qt::UserRole).toString(), QStringLiteral(R"(C:\CurrentOnly)"));

  snapshot.next.volumes = nextVolumes;
  snapshot.current.volumes.clear();
  tab.viewModel().applySnapshot(snapshot);
  QCOMPARE(list->model()->rowCount(), 1);
  QCOMPARE(list->model()->index(0, 0).data(Qt::UserRole).toString(), QStringLiteral(R"(C:\NextOnly)"));

  snapshot.next.volumes.clear();
  tab.viewModel().applySnapshot(snapshot);
  QCOMPARE(list->model()->rowCount(), 0);
  QVERIFY(files->pendingAdded().isEmpty());
  QVERIFY(files->pendingRemoved().isEmpty());
}

void ApplicationConfigurationTests::importRoutingAndPendingCollectionCoverInvalidDuplicateAndMissingTargets() {
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  const auto snapshot = editableSnapshot();
  global.viewModel().setData(snapshot.current, snapshot.next, snapshot.runtime);
  const core::VolumeInfo cInfo{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  const core::VolumeInfo dInfo{"D:", "Volume{d}", "exFAT", "Data", 1000, 500, core::VolumeSupport::FileSystemLimited};
  auto cOwner =
      std::make_unique<ui::VolumeTab>(configuration.addVolume(cInfo, true), uwf::test::localFileDialogs(), nullptr, uwf::bootstrap::createViewModelFactory());
  auto dOwner =
      std::make_unique<ui::VolumeTab>(configuration.addVolume(dInfo, false), uwf::test::localFileDialogs(), nullptr, uwf::bootstrap::createViewModelFactory());
  auto* c = cOwner.get();
  auto* d = dOwner.get();
  c->viewModel().applySnapshot(snapshot);
  d->viewModel().applySnapshot(snapshot);

  const auto parsed = app::parseUwfmgrText(
      "filter disable\n"
      "filter disable\n"
      "overlay set-warningthreshold 5000\n"
      "volume unprotect C:\n"
      "volume protect Z:\n"
      "file add-exclusion C:\\Cache\n"
      "file add-exclusion D:\\Cache\n"
      "file add-exclusion relative\\Cache\n"
      "registry add-exclusion HKLM\\Software\\Vendor\n"
      "file add-exclusion \"C:\\unterminated\n"
      "filter disable extra\n"
      "unknown command\n");
  QList<app::UwfmgrCommand> commands;
  for (const auto& command : parsed) commands.append(command);
  const auto report = configuration.importCommands(commands);
  QCOMPARE(report.size(), commands.size());
  QCOMPARE(report[0].status, app::ImportResult::Status::Success);
  QCOMPARE(report[1].status, app::ImportResult::Status::Duplicate);
  QCOMPARE(report[4].status, app::ImportResult::Status::Failed);
  QCOMPARE(report[6].status, app::ImportResult::Status::Failed);
  QCOMPARE(report[7].status, app::ImportResult::Status::Failed);
  QCOMPARE(report[9].status, app::ImportResult::Status::Failed);
  QCOMPARE(report[10].status, app::ImportResult::Status::Failed);
  QCOMPARE(report[11].status, app::ImportResult::Status::Unsupported);

  const auto pending = configuration.pendingChanges();
  QCOMPARE(pending.setFilterEnabled, std::optional<bool>(false));
  QCOMPARE(pending.volumeProtect.at("C:"), false);
  QCOMPARE(pending.addFileExclusions.at("C:").front(), std::string("C:\\Cache"));
  QCOMPARE(pending.addRegistryExclusions.front(), std::string("HKEY_LOCAL_MACHINE\\Software\\Vendor"));
  QVERIFY(!pending.addFileExclusions.contains("D:"));

  const auto withoutGlobal = app::applyImportCommands(commands.mid(0, 1), nullptr, {}, nullptr, drive::fromPath);
  QCOMPARE(withoutGlobal.front().status, app::ImportResult::Status::Failed);
}

void ApplicationConfigurationTests::importCommandsPreserveOrderAcrossRepeatedTargets() {
  auto snapshot = editableSnapshot();
  snapshot.current.filter.enabled = false;
  snapshot.next.overlay.type = core::OverlayType::Disk;
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  global.viewModel().setData(snapshot.current, snapshot.next, snapshot.runtime);
  const core::VolumeInfo disk{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
  ui::VolumeTab tab(configuration.addVolume(disk, true), uwf::test::localFileDialogs(), nullptr, uwf::bootstrap::createViewModelFactory());
  tab.viewModel().applySnapshot(snapshot);
  const auto parsed = app::parseUwfmgrText(
      "filter disable\nfilter enable\nfilter disable\n"
      "overlay set-size 10240\noverlay set-size 12288\noverlay set-size 10240\n"
      "volume unprotect C:\nvolume protect C:\nvolume unprotect C:\n"
      "file add-exclusion C:\\Cache\nfile remove-exclusion C:\\Cache\nfile add-exclusion C:\\Cache\n"
      "registry add-exclusion HKLM\\SOFTWARE\\Vendor\nregistry remove-exclusion HKLM\\SOFTWARE\\Vendor\n"
      "registry add-exclusion HKLM\\SOFTWARE\\Vendor\n");
  QList<app::UwfmgrCommand> commands;
  for (const auto& command : parsed) commands.append(command);
  const auto report = configuration.importCommands(commands);
  QCOMPARE(report.size(), qsizetype{15});
  for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
  const auto pending = configuration.pendingChanges();
  QCOMPARE(pending.setFilterEnabled, std::optional<bool>(false));
  QCOMPARE(pending.setOverlay.maximumSizeMb, std::optional<uint32_t>(10240));
  QCOMPARE(pending.volumeProtect.at("C:"), false);
  QCOMPARE(pending.addFileExclusions.at("C:"), std::vector<std::string>{R"(C:\Cache)"});
  QVERIFY(pending.removeFileExclusions.empty());
  QCOMPARE(pending.addRegistryExclusions, std::vector<std::string>{R"(HKEY_LOCAL_MACHINE\SOFTWARE\Vendor)"});
  QVERIFY(pending.removeRegistryExclusions.empty());
}

void ApplicationConfigurationTests::importGuardedCommandsPreserveDependencies() {
  const auto snapshot = editableSnapshot();
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  global.viewModel().setData(snapshot.current, snapshot.next, snapshot.runtime);
  const auto apply = [&](const std::string& text) {
    QList<app::UwfmgrCommand> commands;
    for (const auto& command : app::parseUwfmgrText(text)) commands.append(command);
    return configuration.importCommands(commands);
  };
  const auto rejected = apply("overlay set-type Flash && overlay set-size 2048 && overlay set-warningthreshold 512");
  QCOMPARE(rejected.size(), 3);
  for (const auto& row : rejected) QCOMPARE(row.status, app::ImportResult::Status::Failed);
  QVERIFY(rejected[1].detail.contains(QStringLiteral("Skipped because a prerequisite operation")));
  QVERIFY(global.viewModel().pendingOverlay().empty());
  const auto unsupported = apply("filter enable && # note && overlay set-size 2048");
  QCOMPARE(unsupported.size(), 3);
  QCOMPARE(unsupported[1].status, app::ImportResult::Status::Unsupported);
  QCOMPARE(unsupported[2].status, app::ImportResult::Status::Failed);
  QVERIFY(global.viewModel().pendingOverlay().empty());
  const auto queued = apply("overlay set-type Disk && overlay set-size 4096\noverlay set-warningthreshold 512 && overlay set-criticalthreshold 768");
  QCOMPARE(queued.size(), 4);
  QCOMPARE(queued[0].status, app::ImportResult::Status::Duplicate);
  for (qsizetype index = 1; index < queued.size(); ++index) QCOMPARE(queued[index].status, app::ImportResult::Status::Success);
  QCOMPARE(global.viewModel().pendingOverlay().maximumSizeMb, std::optional<uint32_t>(4096));
  QCOMPARE(global.viewModel().pendingOverlay().warningThresholdMb, std::optional<uint32_t>(512));
  QCOMPARE(global.viewModel().pendingOverlay().criticalThresholdMb, std::optional<uint32_t>(768));
}

void ApplicationConfigurationTests::applyPlanPreviewAndCopyUseTheSameProductionCommandMapping() {
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  auto snapshot = editableSnapshot();
  snapshot.next.overlay.type = core::OverlayType::Disk;
  snapshot.next.overlay.maximumSizeMb = 8192;
  global.viewModel().setData(snapshot.current, snapshot.next, snapshot.runtime);
  QVERIFY(global.viewModel().importFilterEnabled(false));
  RecordingWmiOperations wmi;
  QTemporaryDir exportDirectory;
  QVERIFY(exportDirectory.isValid());
  const QString exportPath = exportDirectory.filePath(QStringLiteral("commands.cmd"));
  {
    QFile previous(exportPath);
    QVERIFY(previous.open(QIODevice::WriteOnly));
    QCOMPARE(previous.write("stale"), qint64{5});
  }
  FileDialogStub files;
  files.savedFile = exportPath;
  ui::ApplyPlanDialog dialog(uwf::bootstrap::createViewModelFactory(wmi)->applyPlan(configuration.pendingChanges(), snapshot), files);
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  if (auto* disclosure = dialog.findChild<ui::Disclosure*>(QStringLiteral("changeCommandsDisclosure"))) disclosure->setOpen(true);
  QTextEdit* pending = nullptr;
  for (const auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : edits) {
    if (edit->toPlainText().contains(QStringLiteral("uwfmgr.exe filter disable"))) pending = edit;
  }
  QVERIFY(pending);
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(apply);
  QVERIFY(apply->isEnabled());
  if (auto* disclosure = dialog.findChild<ui::Disclosure*>(QStringLiteral("changeCommandsDisclosure"))) disclosure->setOpen(true);
  pending->selectAll();
  pending->setFocus();
  QTest::keyClick(pending, Qt::Key_C, Qt::ControlModifier);
  const auto copied = QApplication::clipboard()->text();
  QCOMPARE(copied.trimmed(), pending->toPlainText().trimmed());
  const auto copiedCommands = app::parseUwfmgrText(copied.toStdString());
  QCOMPARE(copiedCommands.size(), std::size_t{1});
  QCOMPARE(copiedCommands.front().kind, app::UwfmgrKind::FilterDisable);
  QCOMPARE(copiedCommands.front().parseError, app::ParseError::None);
  auto* exportButton = buttonWithText(&dialog, QStringLiteral("Export commands"));
  QVERIFY(exportButton);
  bool exportSucceeded = false;
  QTimer::singleShot(0, this, [&] {
    if (auto* information = qobject_cast<QDialog*>(QApplication::activeModalWidget())) {
      exportSucceeded = information->windowTitle() == QStringLiteral("Export finished");
      information->accept();
    }
  });
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QVERIFY(exportSucceeded);
  QFile output(exportPath);
  QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
  const QString exportedText = QString::fromUtf8(output.readAll());
  QVERIFY(exportedText.contains(QStringLiteral("uwfmgr.exe filter disable")));
  QVERIFY(!exportedText.contains(QStringLiteral("uwfmgr.exe filter enable")));
  QVERIFY(exportedText.contains(QStringLiteral("uwfmgr.exe overlay set-type Disk")));
  QVERIFY(exportedText.contains(QStringLiteral("uwfmgr.exe overlay set-size 8192")));
  QVERIFY(exportedText.contains(QStringLiteral("uwfmgr.exe overlay set-size 8192 && uwfmgr.exe overlay set-type Disk")));
  presentation::ConfigurationViewModel replayConfiguration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel replay(replayConfiguration.global());
  replay.viewModel().setData(snapshot.current, snapshot.next, snapshot.runtime);
  QList<app::UwfmgrCommand> exportedCommands;
  for (const auto& command : app::parseUwfmgrText(exportedText.toStdString())) exportedCommands.append(command);
  (void)replayConfiguration.importCommands(exportedCommands);
  QCOMPARE(replay.viewModel().pendingFilterEnabled(), std::optional<bool>(false));
  QVERIFY(replay.viewModel().pendingOverlay().empty());
  QVERIFY(exportedText.count(QChar('\n')) > 1);
  QVERIFY(exportedText.startsWith(QString::fromUtf8(app::kUwfmgrScriptMarker.data(), static_cast<qsizetype>(app::kUwfmgrScriptMarker.size()))));
  QCOMPARE(files.requests.size(), 1);
  QVERIFY(files.requests.front().initialPath.contains(QStringLiteral("uwfmgr-commands-")));
  QVERIFY(files.requests.front().initialPath.endsWith(QStringLiteral(".cmd")));

  files.savedFile.clear();
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QCOMPARE(files.requests.size(), 2);

  files.savedFile = QDir::tempPath();
  bool failureReported = false;
  QTimer::singleShot(0, this, [&] {
    auto* warning = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (warning) {
      for (const auto labels = warning->findChildren<ui::ContentHintBox*>(); auto* label : labels) {
        failureReported = failureReported || label->toPlainText().contains(QStringLiteral("Could not open file for writing"));
      }
    }
    if (warning) warning->accept();
  });
  QTest::mouseClick(exportButton, Qt::LeftButton);
  QVERIFY(failureReported);
  QCOMPARE(files.requests.size(), 3);
  QVERIFY(wmi.invocations.empty());
}

void ApplicationConfigurationTests::applyPlanConfirmedWritePublishesReconciliationAndPreventsReplay() {
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  const auto snapshot = editableSnapshot();
  global.viewModel().setData(snapshot.current, snapshot.next, snapshot.runtime);
  QVERIFY(global.viewModel().importFilterEnabled(false));

  RecordingWmiOperations wmi;
  wmi.queryResults.push_back({{{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                               {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(true)}}});
  wmi.objectResults.push_back({{"__PATH", uwf::app::wmi::WmiValue::fromString("filter-path")},
                               {"CurrentEnabled", uwf::app::wmi::WmiValue::fromBool(true)},
                               {"NextEnabled", uwf::app::wmi::WmiValue::fromBool(false)}});

  ui::ApplyPlanDialog dialog(uwf::bootstrap::createViewModelFactory(wmi)->applyPlan(configuration.pendingChanges(), snapshot), uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy reconciliation(&dialog, &ui::ApplyPlanDialog::reconciliationRequired);
  QSignalSpy safeRestartRequested(&dialog, &ui::ApplyPlanDialog::safeRestartRequested);
  QSignalSpy directRestartRequested(&dialog, &ui::ApplyPlanDialog::directRestartRequested);
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  auto* restart = dialog.findChild<QPushButton*>(QStringLiteral("restartBtn"));
  auto* directRestart = dialog.findChild<QPushButton*>(QStringLiteral("directRestartBtn"));
  QVERIFY(apply && apply->isEnabled());
  QVERIFY(restart && restart->isHidden());
  QVERIFY(directRestart && directRestart->isHidden());

  QTimer::singleShot(0, this, [] {
    if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->accept();
  });
  QTest::mouseClick(apply, Qt::LeftButton);
  QTRY_VERIFY(applyCompleted(dialog));

  QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("Disable")});
  QCOMPARE(reconciliation.count(), 1);
  QVERIFY(!apply->isEnabled());
  QVERIFY(!restart->isHidden());
  QVERIFY(!directRestart->isHidden());
  QVERIFY(directRestart->toolTip().contains(QStringLiteral("skips File staging commits")));
  dialog.resize(820, 560);
  QTRY_COMPARE(dialog.size(), QSize(820, 560));
  QTRY_VERIFY2_WITH_TIMEOUT(test::clippedText(dialog).isEmpty(), qUtf8Printable(test::clippedText(dialog)), 2000);
  for (auto* button : {apply, restart, directRestart}) QVERIFY(dialog.rect().contains(QRect(button->mapTo(&dialog, QPoint()), button->size())));
  QTest::mouseClick(restart, Qt::LeftButton);
  QTest::mouseClick(directRestart, Qt::LeftButton);
  QCOMPARE(safeRestartRequested.count(), 1);
  QCOMPARE(directRestartRequested.count(), 1);
  bool successReported = false;
  for (const auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : edits) {
    successReported = successReported || edit->toPlainText().contains(QStringLiteral("Filter: Disabled"));
  }
  QVERIFY(successReported);

  QTest::mouseClick(apply, Qt::LeftButton);
  QTRY_VERIFY(applyCompleted(dialog));
  QCOMPARE(wmi.invocations.size(), std::size_t{1});
}

void ApplicationConfigurationTests::applyPlanConnectionFailureRemainsRetryableAndDoesNotRequestReconciliation() {
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  const auto snapshot = editableSnapshot();
  global.viewModel().setData(snapshot.current, snapshot.next, snapshot.runtime);
  QVERIFY(global.viewModel().importFilterEnabled(false));
  RecordingWmiOperations wmi;
  wmi.connectionFailure = QStringLiteral("transport unavailable");
  ui::ApplyPlanDialog dialog(uwf::bootstrap::createViewModelFactory(wmi)->applyPlan(configuration.pendingChanges(), snapshot), uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy reconciliation(&dialog, &ui::ApplyPlanDialog::reconciliationRequired);
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(apply && apply->isEnabled());

  QTimer::singleShot(0, this, [] {
    if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->accept();
  });
  QTest::mouseClick(apply, Qt::LeftButton);
  QTRY_VERIFY(applyCompleted(dialog));
  QVERIFY(apply->isEnabled());
  QCOMPARE(reconciliation.count(), 0);
  bool failureReported = false;
  for (const auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : edits) {
    failureReported = failureReported || (edit->toPlainText().contains(QStringLiteral("transport unavailable")) &&
                                          edit->toPlainText().contains(QStringLiteral("Failed to connect")));
  }
  QVERIFY(failureReported);
  QVERIFY(wmi.invocations.empty());
}

void ApplicationConfigurationTests::applyPlanCloseRequestsRespectTheExecutionPhase_data() {
  QTest::addColumn<QString>("phase");
  QTest::addColumn<QString>("entry");
  for (const QString& phase : {QStringLiteral("ready"), QStringLiteral("applying"), QStringLiteral("complete"), QStringLiteral("failed")})
    for (const QString& entry : {QStringLiteral("title"), QStringLiteral("footer"), QStringLiteral("escape"), QStringLiteral("window")})
      QTest::newRow(qUtf8Printable(phase + '-' + entry)) << phase << entry;
}

void ApplicationConfigurationTests::applyPlanCloseRequestsRespectTheExecutionPhase() {
  QFETCH(QString, phase);
  QFETCH(QString, entry);
  class Actions final : public app::ConfigurationActions {
   public:
    std::shared_ptr<test::WorkerGate> gate;
    bool fail = false;
    int prepares = 0;
    int completions = 0;
    ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
      ++prepares;
      return [gate = gate, fail = fail] {
        gate->wait({}, false);
        app::ConfigurationApplyResult result;
        result.anyWriteConfirmed = result.reconciliationRequired = !fail;
        result.succeededOperations = fail ? 0 : 1;
        result.failedOperations = fail ? 1 : 0;
        return result;
      };
    }
    void completeApply(const app::ConfigurationApplyResult&) override { ++completions; }
  };
  auto gate = std::make_shared<test::WorkerGate>();
  const auto release = qScopeGuard([gate] {
    gate->release();
    QThreadPool::globalInstance()->waitForDone(2000);
  });
  auto actions = std::make_shared<Actions>();
  actions->gate = gate;
  actions->fail = phase == QStringLiteral("failed");
  core::PendingChanges changes;
  changes.setFilterEnabled = false;
  auto model = std::make_shared<presentation::ApplyPlanViewModel>(changes, editableSnapshot(), actions);
  ui::ApplyPlanDialog dialog(model, uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* footer = buttonWithText(&dialog, QStringLiteral("Cancel"));
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(footer && apply);
  QSignalSpy rejected(&dialog, &QDialog::rejected);
  QSignalSpy finished(&dialog, &QDialog::finished);
  QSignalSpy results(model.get(), &presentation::ApplyPlanViewModel::resultChanged);
  const auto requestClose = [&] {
    if (entry == QStringLiteral("title"))
      QTest::mouseClick(dialog.closeButton(), Qt::LeftButton);
    else if (entry == QStringLiteral("footer"))
      QTest::mouseClick(footer, Qt::LeftButton);
    else if (entry == QStringLiteral("escape"))
      QTest::keyClick(&dialog, Qt::Key_Escape);
    else
      dialog.close();
  };
  QVERIFY(dialog.closeButton()->isEnabled());
  QVERIFY(footer->isEnabled());
  if (phase != QStringLiteral("ready")) {
    QTimer::singleShot(0, &dialog, [] {
      if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->accept();
    });
    QTest::mouseClick(apply, Qt::LeftButton);
    QTRY_COMPARE(gate->started.load(), 1);
    QVERIFY(model->applying());
    QVERIFY(!dialog.closeButton()->isEnabled());
    QVERIFY(!footer->isEnabled());
    if (phase == QStringLiteral("applying")) {
      requestClose();
      QVERIFY(dialog.isVisible());
      QVERIFY(model->applying());
      QCOMPARE(rejected.count(), 0);
      QCOMPARE(finished.count(), 0);
      QCOMPARE(actions->completions, 0);
    }
    gate->release();
    QTRY_VERIFY(!model->applying());
    QCOMPARE(actions->prepares, 1);
    QCOMPARE(actions->completions, 1);
    QCOMPARE(results.count(), 1);
    QCOMPARE(model->resultFailed(), actions->fail);
    QCOMPARE(apply->isEnabled(), actions->fail);
    QVERIFY(dialog.closeButton()->isEnabled());
    QVERIFY(footer->isEnabled());
  }
  requestClose();
  QVERIFY(!dialog.isVisible());
  QCOMPARE(rejected.count(), 1);
  QCOMPARE(finished.count(), 1);
  QCOMPARE(finished.front().front().toInt(), static_cast<int>(QDialog::Rejected));
  QCOMPARE(actions->prepares, phase == QStringLiteral("ready") ? 0 : 1);
  QCOMPARE(actions->completions, actions->prepares);
}

void ApplicationConfigurationTests::applyPlanOverlayWritesRespectProviderConstraints_data() {
  QTest::addColumn<QString>("scenario");
  QTest::addColumn<int>("failureMode");
  for (const QString& scenario :
       {QStringLiteral("threshold-raise"), QStringLiteral("threshold-lower"), QStringLiteral("threshold-equality"), QStringLiteral("threshold-enable"),
        QStringLiteral("threshold-disable"), QStringLiteral("threshold-fresh-state"), QStringLiteral("threshold-enable-critical-with-warning"),
        QStringLiteral("threshold-disable-critical-only"), QStringLiteral("threshold-enable-critical-only"), QStringLiteral("ram-small-to-disk"),
        QStringLiteral("ram-large-to-disk"), QStringLiteral("disk-to-small-ram"), QStringLiteral("disk-to-large-ram")}) {
    for (int failureMode = 0; failureMode <= 4; ++failureMode) {
      const QByteArray name = scenario.toLatin1() + '-' + QByteArray::number(failureMode);
      QTest::newRow(name.constData()) << scenario << failureMode;
    }
  }
}

void ApplicationConfigurationTests::applyPlanOverlayWritesRespectProviderConstraints() {
  QFETCH(QString, scenario);
  QFETCH(int, failureMode);
  const bool thresholds = scenario.startsWith(QStringLiteral("threshold-"));
  core::OverlayConfig baseline{core::OverlayType::RAM, 4096, 512, 768};
  auto target = baseline;
  QStringList expected;
  if (thresholds) {
    expected = {QStringLiteral("SetCriticalThreshold"), QStringLiteral("SetWarningThreshold")};
    target.warningThresholdMb = 1024;
    target.criticalThresholdMb = 1536;
    if (scenario == QStringLiteral("threshold-lower")) {
      baseline.warningThresholdMb = 1024;
      baseline.criticalThresholdMb = 1536;
      target.warningThresholdMb = 256;
      target.criticalThresholdMb = 512;
      expected = {QStringLiteral("SetWarningThreshold"), QStringLiteral("SetCriticalThreshold")};
    } else if (scenario == QStringLiteral("threshold-equality")) {
      target.warningThresholdMb = 768;
      target.criticalThresholdMb = 1024;
    } else if (scenario == QStringLiteral("threshold-enable")) {
      baseline.warningThresholdMb = 0;
      baseline.criticalThresholdMb = 0;
    } else if (scenario == QStringLiteral("threshold-disable")) {
      target.warningThresholdMb = 0;
      target.criticalThresholdMb = 0;
      expected = {QStringLiteral("SetWarningThreshold"), QStringLiteral("SetCriticalThreshold")};
    } else if (scenario == QStringLiteral("threshold-fresh-state")) {
      target.warningThresholdMb = 600;
    } else if (scenario == QStringLiteral("threshold-enable-critical-with-warning")) {
      baseline.warningThresholdMb = 1024;
      baseline.criticalThresholdMb = 0;
      target.warningThresholdMb = 256;
      target.criticalThresholdMb = 512;
      expected = {QStringLiteral("SetWarningThreshold"), QStringLiteral("SetCriticalThreshold")};
    } else if (scenario == QStringLiteral("threshold-disable-critical-only")) {
      target.warningThresholdMb = baseline.warningThresholdMb;
      target.criticalThresholdMb = 0;
      expected = {QStringLiteral("SetCriticalThreshold")};
    } else if (scenario == QStringLiteral("threshold-enable-critical-only")) {
      baseline.criticalThresholdMb = 0;
      target.warningThresholdMb = baseline.warningThresholdMb;
      target.criticalThresholdMb = 768;
      expected = {QStringLiteral("SetCriticalThreshold")};
    }
  } else {
    baseline.warningThresholdMb = target.warningThresholdMb = 128;
    baseline.criticalThresholdMb = target.criticalThresholdMb = 256;
    if (scenario.startsWith(QStringLiteral("ram-"))) {
      baseline.maximumSizeMb = scenario == QStringLiteral("ram-small-to-disk") ? 512 : 4096;
      target.type = core::OverlayType::Disk;
      target.maximumSizeMb = 2048;
      expected = {QStringLiteral("SetMaximumSize"), QStringLiteral("SetType")};
    } else {
      baseline.type = core::OverlayType::Disk;
      baseline.maximumSizeMb = 2048;
      target.type = core::OverlayType::RAM;
      target.maximumSizeMb = scenario == QStringLiteral("disk-to-small-ram") ? 512 : 4096;
      expected = {QStringLiteral("SetType"), QStringLiteral("SetMaximumSize")};
    }
  }
  auto snapshot = editableSnapshot();
  snapshot.current.filter.enabled = snapshot.next.filter.enabled = false;
  snapshot.current.overlay = snapshot.next.overlay = baseline;
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  global.viewModel().setData(snapshot.current, snapshot.next, {});
  global.viewModel().importOverlayType(target.type);
  global.viewModel().importOverlayMaxMb(target.maximumSizeMb);
  global.viewModel().importOverlayWarnMb(target.warningThresholdMb);
  global.viewModel().importOverlayCritMb(target.criticalThresholdMb);
  global.viewModel().finishImport();

  OverlayContractWmiOperations wmi;
  wmi.state = baseline;
  if (scenario == QStringLiteral("threshold-fresh-state")) {
    wmi.state.warningThresholdMb = 400;
    wmi.state.criticalThresholdMb = 550;
  }
  const auto initial = wmi.state;
  wmi.failureMode = failureMode;
  ui::ApplyPlanDialog dialog(uwf::bootstrap::createViewModelFactory(wmi)->applyPlan(configuration.pendingChanges(), snapshot), uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  QSignalSpy reconciliation(&dialog, &ui::ApplyPlanDialog::reconciliationRequired);
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(apply && apply->isEnabled());
  QTimer::singleShot(0, this, [] {
    if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->accept();
  });
  QTest::mouseClick(apply, Qt::LeftButton);
  QTRY_VERIFY(applyCompleted(dialog));
  QVERIFY(!wmi.invalidIntermediate);
  const bool succeeded = failureMode == 0 || failureMode == 3;
  const bool hasDependent = expected.size() > 1;
  if (!succeeded) expected = expected.mid(0, 1);
  QCOMPARE(wmi.invocations, expected);
  const auto& final = succeeded ? target : initial;
  QCOMPARE(wmi.state.type, final.type);
  QCOMPARE(wmi.state.maximumSizeMb, final.maximumSizeMb);
  QCOMPARE(wmi.state.warningThresholdMb, final.warningThresholdMb);
  QCOMPARE(wmi.state.criticalThresholdMb, final.criticalThresholdMb);
  QCOMPARE(reconciliation.count(), failureMode == 1 ? 0 : 1);
  QCOMPARE(apply->isEnabled(), failureMode == 1);
  if (!succeeded && hasDependent) {
    bool skipReported = false;
    for (const auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : edits)
      skipReported = skipReported || edit->toPlainText().contains(ui::I18n::tr("Skipped because a prerequisite operation failed or could not be confirmed."));
    QVERIFY(skipReported);
  }
}

void ApplicationConfigurationTests::applyPlanDependencyFailureIsLocalized_data() {
  QTest::addColumn<ui::I18n::Lang>("language");
  for (const auto& language : ui::I18n::languages()) QTest::newRow(QString(language.code).toLatin1().constData()) << language.id;
}

void ApplicationConfigurationTests::applyPlanDependencyFailureIsLocalized() {
  QFETCH(ui::I18n::Lang, language);
  ui::I18n::instance().setLang(language);
  const auto restoreLanguage = qScopeGuard([] { ui::I18n::instance().setLang(ui::I18n::Lang::En); });
  auto snapshot = editableSnapshot();
  snapshot.current.filter.enabled = snapshot.next.filter.enabled = false;
  snapshot.current.overlay = snapshot.next.overlay = {core::OverlayType::RAM, 512, 128, 256};
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb());
  ui::GlobalStatusPanel global(configuration.global());
  global.viewModel().setData(snapshot.current, snapshot.next, {});
  global.viewModel().importOverlayType(core::OverlayType::Disk);
  global.viewModel().importOverlayMaxMb(2048);
  global.viewModel().finishImport();
  OverlayContractWmiOperations wmi;
  wmi.state = snapshot.next.overlay;
  wmi.failureMode = 1;
  ui::ApplyPlanDialog dialog(uwf::bootstrap::createViewModelFactory(wmi)->applyPlan(configuration.pendingChanges(), snapshot), uwf::test::localFileDialogs());
  dialog.show();
  QVERIFY(QTest::qWaitForWindowExposed(&dialog));
  auto* apply = dialog.findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  QVERIFY(apply && apply->isEnabled());
  const auto guarded = QStringLiteral("uwfmgr.exe overlay set-size 2048 && uwfmgr.exe overlay set-type Disk");
  if (auto* disclosure = dialog.findChild<ui::Disclosure*>(QStringLiteral("changeCommandsDisclosure"))) disclosure->setOpen(true);
  QTextEdit* pending = nullptr;
  for (const auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : edits)
    if (edit->toPlainText().contains(guarded)) pending = edit;
  QVERIFY(pending);
  if (language == ui::I18n::Lang::En) {
    if (auto* disclosure = dialog.findChild<ui::Disclosure*>(QStringLiteral("changeCommandsDisclosure"))) disclosure->setOpen(true);
    pending->selectAll();
    pending->setFocus();
    QTest::keyClick(pending, Qt::Key_C, Qt::ControlModifier);
    const auto copied = QApplication::clipboard()->text();
    QCOMPARE(copied.trimmed(), pending->toPlainText().trimmed());
    QVERIFY(copied.contains(guarded));
  }
  QTimer::singleShot(0, this, [] {
    if (auto* confirmation = qobject_cast<QDialog*>(QApplication::activeModalWidget())) confirmation->accept();
  });
  QTest::mouseClick(apply, Qt::LeftButton);
  QTRY_VERIFY(applyCompleted(dialog));
  QCOMPARE(wmi.invocations, QStringList{QStringLiteral("SetMaximumSize")});
  const auto message = ui::I18n::tr("Skipped because a prerequisite operation failed or could not be confirmed.");
  if (language != ui::I18n::Lang::En) QVERIFY(message != QStringLiteral("Skipped because a prerequisite operation failed or could not be confirmed."));
  bool translatedMessageObserved = false;
  for (const auto edits = dialog.findChildren<QTextEdit*>(); auto* edit : edits) translatedMessageObserved |= edit->toPlainText().contains(message);
  QVERIFY(translatedMessageObserved);
}

void ApplicationConfigurationTests::configurationVisualStates_data() { addLocaleThemeRows(); }

void ApplicationConfigurationTests::configurationVisualStates() {
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
  FileDialogStub files;
  auto storeOwner = std::make_shared<MemoryFileStagingStore>();
  auto& store = *storeOwner;
  store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\Data\\state.bin")},
                   {app::FileStagingKind::Directory, QStringLiteral("C:\\Data\\Settings")}};
  auto snapshot = editableSnapshot();
  snapshot.current.filter.enabled = false;
  QWidget host;
  auto* layout = new QHBoxLayout(&host);
  const core::VolumeInfo info{"C:", "Volume{c}", "NTFS", "System", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported};
  app::FileStagingDraft stagingDraft(storeOwner);
  presentation::ConfigurationViewModel configuration(uwf::windows::systemTotalRamMb(), [&stagingDraft] { return !stagingDraft.load().isEmpty(); });
  ui::VolumeTab disk(configuration.addVolume(info, true), files, &stagingDraft, uwf::bootstrap::createViewModelFactory());
  ui::GlobalStatusPanel global(configuration.global());
  layout->addWidget(&disk, 1);
  layout->addWidget(&global);
  disk.viewModel().applySnapshot(snapshot);
  global.viewModel().setData(snapshot.current, snapshot.next, snapshot.runtime);
  host.resize(1440, 900);
  host.show();
  QVERIFY(QTest::qWaitForWindowExposed(&host));
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-initial")));

  auto* filter = global.findChild<ui::SwitchButton*>();
  QVERIFY(filter && filter->isChecked());
  QTest::mouseClick(filter, Qt::LeftButton);
  QVERIFY(!filter->isChecked());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-switch-unchecked")));
  QTest::mouseClick(filter, Qt::LeftButton);
  QVERIFY(filter->isChecked());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-switch-checked")));
  filter->setFocus(Qt::TabFocusReason);
  QTRY_VERIFY(filter->hasFocus());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-switch-focus")));

  auto* type = global.findChild<QComboBox*>();
  QVERIFY(type && type->isEnabled());
  type->setFocus(Qt::TabFocusReason);
  QTest::keyClick(type, Qt::Key_Home);
  QCOMPARE(type->currentData().toInt(), static_cast<int>(core::OverlayType::RAM));
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-overlay-ram")));
  QTest::keyClick(type, Qt::Key_End);
  QCOMPARE(type->currentData().toInt(), static_cast<int>(core::OverlayType::Disk));
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-overlay-disk")));

  auto* tabs = disk.findChild<QTabWidget*>(QStringLiteral("innerTabs"));
  QVERIFY(tabs && tabs->count() == 3);
  auto* filePage = tabs->widget(0)->findChild<ui::ExclusionListWidget*>();
  QVERIFY(filePage);
  auto* list = filePage->findChild<QListView*>();
  QVERIFY(list && list->model()->rowCount() == 1);
  QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualRect(list->model()->index(0, 0)).center());
  QCOMPARE(list->selectionModel()->selectedIndexes().size(), 1);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-file-selected")));
  QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(list->viewport()->width() - 8, list->viewport()->height() - 8));
  QVERIFY(list->selectionModel()->selectedIndexes().isEmpty());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-file-unselected")));

  auto* add = filePage->commandWidget()->findChild<QPushButton*>(QStringLiteral("primaryBtn"));
  auto* remove = buttonWithText(filePage->commandWidget(), ui::I18n::tr("Remove selected"));
  QVERIFY(add && add->findChild<QMenu*>() && remove);
  files.openedFile = QStringLiteral("C:\\VisualNew.txt");
  files.openedFiles = {files.openedFile};
  bool addMenuInspected = false;
  QTimer::singleShot(20, &host, [&] {
    auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    const auto closeMenu = qScopeGuard([menu] {
      if (menu) menu->close();
    });
    QVERIFY(menu && !menu->actions().isEmpty());
    QVERIFY(uwf::test::saveScreenshot(*menu, QStringLiteral("interaction-file-add-menu")));
    auto* action = menu->actions().constFirst();
    QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(action).center());
    addMenuInspected = true;
  });
  QContextMenuEvent addContext(QContextMenuEvent::Mouse, add->rect().center(), add->mapToGlobal(add->rect().center()));
  QApplication::sendEvent(add, &addContext);
  QTRY_VERIFY(addMenuInspected);
  QCOMPARE(list->model()->rowCount(), 2);
  QVERIFY(disk.viewModel().files()->pendingAdded().contains(files.openedFile));
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-file-added")));
  QModelIndex added;
  for (int row = 0; row < list->model()->rowCount(); ++row) {
    if (list->model()->index(row, 0).data(Qt::UserRole).toString() == files.openedFile) added = list->model()->index(row, 0);
  }
  QVERIFY(added.isValid());
  QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualRect(added).center());
  QTest::mouseClick(remove, Qt::LeftButton);
  QVERIFY(disk.viewModel().files()->pendingAdded().isEmpty());
  QCOMPARE(list->model()->rowCount(), 1);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-file-removed")));

  QTest::mouseClick(tabs->tabBar(), Qt::LeftButton, Qt::NoModifier, tabs->tabBar()->tabRect(1).center());
  QCOMPARE(tabs->currentIndex(), 1);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-registry-tab")));
  QTest::mouseClick(tabs->tabBar(), Qt::LeftButton, Qt::NoModifier, tabs->tabBar()->tabRect(2).center());
  QCOMPARE(tabs->currentIndex(), 2);
  auto* staging = disk.findChild<ui::FileStagingWidget*>();
  auto* staged = staging ? staging->findChild<QListView*>() : nullptr;
  QVERIFY(staged && staged->model()->rowCount() == 2);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-staging-unselected")));
  QTest::mouseClick(staged->viewport(), Qt::LeftButton, Qt::NoModifier, staged->visualRect(staged->model()->index(0, 0)).center());
  QCOMPARE(staged->selectionModel()->selectedIndexes().size(), 1);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-staging-selected")));
  auto* removeStaged = buttonWithText(staging->commandWidget(), ui::I18n::tr("Remove selected"));
  QVERIFY(removeStaged);
  QTest::mouseClick(removeStaged, Qt::LeftButton);
  QCOMPARE(staged->model()->rowCount(), 2);
  QCOMPARE(staged->model()->index(0, 0).data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Removed));
  QCOMPARE(store.entries.size(), 2);
  QCOMPARE(store.writes, 0);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-staging-removed")));
  auto* search = staging->commandWidget()->findChild<QLineEdit*>(QStringLiteral("filterInput"));
  QVERIFY(search);
  search->setFocus();
  QTest::keyClicks(search, QStringLiteral("missing"));
  QCOMPARE(staged->model()->rowCount(), 0);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-search-empty")));
  QTest::keyClick(search, Qt::Key_A, Qt::ControlModifier);
  QTest::keyClick(search, Qt::Key_Backspace);
  QCOMPARE(staged->model()->rowCount(), 2);
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-search-cleared")));
  global.viewModel().setControlsEnabled(false);
  QVERIFY(!filter->isEnabled() && !type->isEnabled());
  QVERIFY(uwf::test::saveScreenshot(host, QStringLiteral("interaction-disabled")));
}

QTEST_MAIN(ApplicationConfigurationTests)
#include "ConfigurationCases.moc"
