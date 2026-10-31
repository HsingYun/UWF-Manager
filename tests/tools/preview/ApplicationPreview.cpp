// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <dwmapi.h>
#include <qt_windows.h>

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCursor>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSaveFile>
#include <QScreen>
#include <QScrollBar>
#include <QTabWidget>
#include <QTableView>
#include <QTextEdit>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QWindow>
#include <algorithm>
#include <stdexcept>

#include "app/commit/CommitWorkflow.h"
#include "app/staging/FileStagingDraft.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "bootstrap/MainWindowComposition.h"
#include "bootstrap/ViewModelComposition.h"
#include "presentation/common/PageProxyModel.h"
#include "presentation/files/FilePickerViewModel.h"
#include "presentation/staging/FileStagingResultText.h"
#include "support/app/CommitScenario.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/EnhancedModeFixture.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/RecordingWmiOperations.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/LocalFileDialogs.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/appearance/WindowAppearance.h"
#include "ui/commit/CommitInteractionAdapter.h"
#include "ui/commit/CommitOperationDialog.h"
#include "ui/commit/CommitReportDialog.h"
#include "ui/configuration/ApplyPlanDialog.h"
#include "ui/configuration/ImportDialog.h"
#include "ui/diagnostics/AboutDialog.h"
#include "ui/diagnostics/LogViewerDialog.h"
#include "ui/diagnostics/SystemInformationDialog.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/enhanced/EnhancedModeDialog.h"
#include "ui/files/FilePickerDialog.h"
#include "ui/overlay/OverlayContextMenu.h"
#include "ui/overlay/OverlayFilesDialog.h"
#include "ui/overlay/OverlayFloatingWidget.h"
#include "ui/overlay/OverlayTaskbarWidget.h"
#include "ui/power/PowerConfirmDialog.h"
#include "ui/registry/RegistryPickerDialog.h"
#include "ui/shell/MainWindow.h"
#include "ui/shell/TrayController.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/EntryListView.h"
#include "windows/commit/WindowsCommitTraversalSource.h"
using namespace uwf;
namespace {
class PreviewFileBrowser final : public app::FileBrowser {
 public:
  QString scenario;
  QVector<app::FileBrowserEntry> list(const QString& path, const std::stop_token stop) const override {
    if (scenario == QLatin1String("loading"))
      while (!stop.stop_requested()) QThread::msleep(10);
    if (scenario == QLatin1String("error")) throw std::runtime_error("拒绝访问 · 0x80070005");
    if (scenario == QLatin1String("error-long"))
      throw std::runtime_error("Access denied: C:/Enterprise deployment/Managed application data/<literal & complete>/Protected directory\n"
                               "Original provider details are retained completely.\nPlease check directory permissions and retry.");
    if (scenario == QLatin1String("empty")) return {};
    QVector<app::FileBrowserEntry> result;
    const QDateTime modified(QDate(2026, 10, 25), QTime(14, 20));
    for (const auto& name : {QStringLiteral("项目资料"), QStringLiteral("配置备份"), QStringLiteral("uwf-settings.txt"), QStringLiteral("uwf-startup.ps1"),
                             QStringLiteral("uwf-exclusions.cmd"), QStringLiteral("工作记录.txt")}) {
      const bool directory = !name.contains(u'.');
      result.append({QDir(path).filePath(name), name, directory, directory ? 0 : 2048, modified});
    }
    return result;
  }
  app::FileBrowserEntry inspect(const QString& path) const override { return {path, QFileInfo(path).fileName(), !path.contains(u'.'), 0, {}, true}; }
  QVector<app::FileBrowserPlace> places() const override {
    using Kind = app::FileBrowserPlaceKind;
    return {{QStringLiteral("Home"), QStringLiteral("C:/Users/User"), false, false, Kind::Home},
            {QStringLiteral("AppData"), QStringLiteral("C:/Users/User/AppData")},
            {QStringLiteral("Desktop"), QStringLiteral("C:/Users/User/Desktop"), false, false, Kind::Desktop},
            {QStringLiteral("Downloads"), QStringLiteral("C:/Users/User/Downloads"), false, false, Kind::Downloads},
            {QStringLiteral("Documents"), QStringLiteral("C:/Users/User/Documents"), false, false, Kind::Documents},
            {QStringLiteral("本地磁盘 (C:)"), QStringLiteral("C:/"), true, true},
            {QStringLiteral("数据 (D:)"), QStringLiteral("D:/"), true, false}};
  }
  void createDirectory(const QString&) const override {}
  void renameEntry(const QString&, const QString&) const override {}
};
class PreviewLogSource final : public app::LogSource {
 public:
  explicit PreviewLogSource(QString scenario) : m_scenario(std::move(scenario)) {}
  std::vector<std::string> recentLines() const override {
    if (m_scenario == QLatin1String("failure")) throw std::runtime_error("读取日志失败：拒绝访问。\n0x80070005 · ERROR_ACCESS_DENIED");
    if (m_scenario == QLatin1String("empty")) return {};
    return {"[14:32:08.214 I main] 应用已启动，系统检查完成。",
            "[14:32:08.236 I wmi] 已连接 UWF 管理命名空间。",
            "[14:32:08.451 I ui] 状态刷新完成：2 个磁盘，UWF 可用。耗时 186 ms。",
            "[14:32:11.293 W staging] 已跳过排除路径：C:\\Users\\User\\Documents\\Cache。该路径中的文件不会提交。",
            "[14:32:12.840 I staging] 扫描完成：58,543 个文件，5 个目录。耗时 1,902 ms。",
            "[14:32:13.017 E registry] 枚举注册表键失败：拒绝访问。\n路径：HKEY_LOCAL_MACHINE\\SECURITY\n错误代码：0x80070005（ERROR_ACCESS_DENIED）",
            "[14:32:13.105 I ui] 已在注册表列表中显示访问异常。",
            "[14:32:17.408 D overlay] 覆盖层状态采样完成：已用 1,024 MB，最大 4,096 MB。",
            "[14:33:01.028 I power] 已读取文件暂存列表：5 项。",
            "[14:33:02.935 I power] 安全重启准备完成：58,543 个待提交文件。",
            "[14:33:06.312 I power] 用户取消安全重启。未执行关机或重启。"};
  }
  void clear() override { m_scenario = QStringLiteral("empty"); }

 private:
  QString m_scenario;
};
class PreviewEnhancedActions final : public app::EnhancedModeActions {
 public:
  app::EnhancedModeStatus current = test::application::completeEnhancedModeStatus();
  bool unavailable = false;
  app::EnhancedModeStatus status() const override {
    if (unavailable) throw std::runtime_error("Service status: access denied (0x80070005)");
    return current;
  }
  void setAgentState(app::EnhancedModeAgentState state) override { current.agentState = state; }
  app::EnhancedModeChangeResult enable(const QString&) override { return {current, {}}; }
  app::EnhancedModeChangeResult start() override { return {current, {}}; }
  app::EnhancedModeChangeResult disable() override { return {current, {}}; }
  ChangeWork prepareChange(app::EnhancedModeAction, const QString&) override {
    return [result = current] { return app::EnhancedModeChangeResult{result, {}}; };
  }
};
class PreviewRegistryBrowser final : public app::RegistryBrowser {
 public:
  bool failure = false;
  std::vector<std::string> rootHiveLongNames() const override {
    return {"HKEY_LOCAL_MACHINE", "HKEY_CURRENT_USER", "HKEY_CLASSES_ROOT", "HKEY_USERS", "HKEY_CURRENT_CONFIG"};
  }
  std::vector<std::string> subkeyNames(std::string_view key) const override {
    if (key == "HKEY_LOCAL_MACHINE") return {"SOFTWARE", "SYSTEM", "SECURITY"};
    if (key == "HKEY_LOCAL_MACHINE\\SOFTWARE") return {"MyApp", "Microsoft", "Policies"};
    if (key == "HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp") return {"Settings", "Data"};
    if (failure && key.find("MyApp") != std::string_view::npos) throw std::runtime_error("拒绝访问 · 0x80070005");
    return {};
  }
  bool hasSubkeys(std::string_view key) const override {
    return key == "HKEY_LOCAL_MACHINE" || key == "HKEY_LOCAL_MACHINE\\SOFTWARE" || key == "HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp";
  }
  std::vector<app::RegistryValueInfo> values(std::string_view key) const override {
    if (failure && key.find("MyApp") != std::string_view::npos) throw std::runtime_error("拒绝访问 · 0x80070005");
    std::vector<uint8_t> installPath;
    const QString path = QStringLiteral("C:\\Program Files\\MyApp");
    for (const auto character : path) {
      installPath.push_back(static_cast<uint8_t>(character.unicode() & 0xFF));
      installPath.push_back(static_cast<uint8_t>(character.unicode() >> 8));
    }
    installPath.insert(installPath.end(), {0, 0});
    return {{"", 1, std::vector<uint8_t>{}}, {"Enabled", 4, std::vector<uint8_t>{1, 0, 0, 0}}, {"InstallPath", 1, std::move(installPath)}};
  }
};
class PreviewSystemInformation final : public app::SystemInformationSource {
 public:
  PreviewSystemInformation() {
    m_version.productName = "Windows 11 Enterprise";
    m_hardware.totalMemoryBytes = 64ULL << 30;
    m_hardware.graphicsAdapter.name = "RTX 5090";
  }
  uint32_t totalRamMb() const override { return 65536; }
  QString systemDriveLetter() const override { return QStringLiteral("C:"); }
  const app::WindowsVersionInfo& windowsVersion() const override { return m_version; }
  const app::SystemHardwareInfo& hardware() const override { return m_hardware; }
  app::SystemDiagnosticFacts diagnostics() const override { return {}; }

 private:
  app::WindowsVersionInfo m_version;
  app::SystemHardwareInfo m_hardware;
};
}  // namespace

int main(int argc, char** argv) {
  QApplication application(argc, argv);
  const auto arguments = application.arguments();
  const auto value = [&](const QString& name, const QString& fallback = QString{}) {
    const auto position = arguments.indexOf(name);
    return position >= 0 && position + 1 < arguments.size() ? arguments[position + 1] : fallback;
  };
  const auto output = value(QStringLiteral("--output"));
  if (output.isEmpty()) return 2;
  const auto page = value(QStringLiteral("--page"), QStringLiteral("main"));
  const auto scenario = value(QStringLiteral("--scenario"), QStringLiteral("normal"));
  const bool docs = arguments.contains(QStringLiteral("--docs"));
  if (docs && QGuiApplication::platformName() != QLatin1String("windows")) return 6;
  const bool mainPage = page == QLatin1String("main") || page == QLatin1String("overview");
  QElapsedTimer reviewTimer;
  QJsonObject reviewTimings;
  ui::I18n::instance().setLang(ui::I18n::parseLanguageCode(value(QStringLiteral("--language"), QStringLiteral("zh_CN"))).value_or(ui::I18n::Lang::En));
  ui::ThemeManager::instance().apply(arguments.contains(QStringLiteral("--dark")) ? app::Theme::Dark : app::Theme::Light);
  std::unique_ptr<QWidget> documentBackground;
  if (docs) {
    auto* screen = application.primaryScreen();
    const auto available = screen->availableGeometry();
    documentBackground = std::make_unique<QWidget>(nullptr, Qt::Tool | Qt::FramelessWindowHint);
    documentBackground->setStyleSheet(QStringLiteral("background: white;"));
    documentBackground->setGeometry(available);
    documentBackground->show();
  }
  test::application::RecordingWmiOperations wmi;
  auto stagingOwner = std::make_shared<test::application::MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  test::application::MutableApplicationStateSource source;
  auto serviceOwner = std::make_shared<test::application::MemoryEnhancedModeServiceControl>();
  test::application::MemoryEnhancedModeAgent agent;
  source.snapshot = test::application::editableSnapshot();
  source.snapshot.next = source.snapshot.current;
  source.snapshot.current.overlay = {core::OverlayType::RAM, 4096, 3072, 3800};
  source.snapshot.next.overlay = source.snapshot.current.overlay;
  source.snapshot.runtime = {3072, 1024};
  source.snapshot.current.fileExclusions["Volume{c}"] = {"C:\\ProgramData\\MyApp\\Logs", "C:\\Users\\Public\\Documents", "C:\\Windows\\Temp",
                                                         "C:\\ProgramData\\MyApp\\settings.json"};
  source.snapshot.next.fileExclusions = source.snapshot.current.fileExclusions;
  source.snapshot.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp", "HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager"};
  source.snapshot.next.registryExclusions = source.snapshot.current.registryExclusions;
  source.volumes = {{"C:", "Volume{c}", "NTFS", "系统", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported},
                    {"D:", "Volume{d}", "NTFS", "数据", 1ULL << 40, 608ULL << 30, core::VolumeSupport::Supported}};
  if (docs) {
    source.snapshot.current.fileExclusions["Volume{c}"] = {
        "C:\\ProgramData\\MyApp\\Logs", "C:\\ProgramData\\MyApp\\settings.json", "C:\\ProgramData\\DeviceAgent\\Logs",
        "C:\\ProgramData\\DeviceAgent\\state.json", "C:\\ProgramData\\Kiosk\\Cache", "C:\\ProgramData\\Kiosk\\Uploads",
        "C:\\ProgramData\\Microsoft\\Windows\\WER", "C:\\ProgramData\\Terminal\\Receipts", "C:\\ProgramData\\Terminal\\Reports",
        "C:\\Users\\Public\\Documents", "C:\\Users\\Public\\Pictures", "C:\\Windows\\Temp"};
    source.snapshot.next.fileExclusions = source.snapshot.current.fileExclusions;
    if (ui::I18n::instance().lang() == ui::I18n::Lang::En) {
      source.volumes[0].label = "System";
      source.volumes[1].label = "Data";
    }
    staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\Users\\User\\AppData\\Roaming\\MyApp\\preferences.json")},
                       {app::FileStagingKind::Directory, QStringLiteral("C:\\Projects\\Workspace")},
                       {app::FileStagingKind::Directory, QStringLiteral("C:\\Users\\User\\Documents\\Reports")}};
    serviceOwner->current = test::application::completeEnhancedModeStatus();
    agent.connectOnStart = true;
  }
  const auto state = value(QStringLiteral("--state"));
  if (state == QLatin1String("warning")) source.snapshot.runtime = {902, 3194};
  if (state == QStringLiteral("readonly")) source.snapshot.elevated = false;
  if (state == QStringLiteral("unavailable")) {
    source.snapshot.uwfAvailable = false;
    source.snapshot.unavailableReason = "UWF_Overlay.GetOverlayConfig\n0x80041004 · WBEM_E_PROVIDER_FAILURE";
  }
  if (state == QStringLiteral("filter-off")) source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = false;
  PreviewSystemInformation system;
  auto services = bootstrap::mainWindowServices(wmi, source, stagingOwner, serviceOwner, &agent);
  services.viewModelFactory = bootstrap::createViewModelFactory(wmi, windows::systemRegistryBrowser(), windows::loadOverlayFilesFromSystem, system);
  const auto factory = services.viewModelFactory;
  ui::MainWindow window(std::move(services),
                        {.uwfCapability = app::UwfCapability::Available,
                         .compatibilityMode = state == QLatin1String("compatibility"),
                         .hubVisible = docs && page == QLatin1String("overview")});
  test::FileDialogStub fileDialogs;
  PreviewEnhancedActions enhanced;
  enhanced.current.agentState = app::EnhancedModeAgentState::Connected;
  PreviewRegistryBrowser registry;
  std::unique_ptr<QMenu> menu;
  std::unique_ptr<QDialog> panel;
  std::unique_ptr<QWidget> component;
  QWidget trayOwner;
  std::unique_ptr<ui::TrayController> trayPreview;
  QMenu* productionTrayMenu = nullptr;
  if (page == QLatin1String("picker")) {
    auto browser = std::make_shared<PreviewFileBrowser>();
    browser->scenario = scenario;
    using Mode = presentation::FilePickerViewModel::Mode;
    const auto mode = scenario == QLatin1String("folder")     ? Mode::Directory
                      : scenario == QLatin1String("mixed")    ? Mode::Mixed
                      : scenario == QLatin1String("save")     ? Mode::SaveFile
                      : scenario == QLatin1String("multiple") ? Mode::OpenFiles
                                                              : Mode::OpenFile;
    auto model = std::make_shared<presentation::FilePickerViewModel>(mode, browser);
    const auto scope = value(QStringLiteral("--scope"));
    if (!scope.isEmpty()) model->setAllowedRoots({scope});
    model->setFilters(ui::I18n::tr("Text and scripts (*.txt *.bat *.ps1 *.cmd);;Text files (*.txt);;All files (*)"));
    panel = std::make_unique<ui::FilePickerDialog>(model, scenario == QLatin1String("folder")     ? ui::I18n::tr("Select folder")
                                                          : scenario == QLatin1String("mixed")    ? ui::I18n::tr("Add files or folders")
                                                          : scenario == QLatin1String("save")     ? ui::I18n::tr("Save file")
                                                          : scenario == QLatin1String("multiple") ? ui::I18n::tr("Select files")
                                                                                                  : ui::I18n::tr("Open configuration file"));
    model->start((scope.startsWith(QLatin1String("D:"), Qt::CaseInsensitive) ? QStringLiteral("D:/") : QStringLiteral("C:/Users/User/Documents")) +
                 (mode == Mode::SaveFile ? QStringLiteral("/uwfmgr-commands.txt") : QString{}));
  }
  if (page == QStringLiteral("enhanced")) {
    enhanced.current.agentState = app::EnhancedModeAgentState::Connected;
    if (scenario == QLatin1String("disabled")) enhanced.current = {};
    if (scenario == QLatin1String("stopped")) {
      enhanced.current.state = app::EnhancedModeState::Stopped;
      enhanced.current.running = false;
    }
    if (scenario == QLatin1String("repair")) {
      enhanced.current.state = app::EnhancedModeState::RepairRequired;
      enhanced.current.executableMatches = false;
    }
    enhanced.unavailable = scenario == QLatin1String("error");
    panel = std::make_unique<ui::EnhancedModeDialog>(factory->enhancedMode(enhanced));
  }
  if (page == QStringLiteral("registry")) {
    registry.failure = scenario == QLatin1String("error");
    const auto mode = scenario != QLatin1String("value") && scenario != QLatin1String("delete") ? ui::RegistryPickerDialog::Mode::Exclusion
                      : scenario == QLatin1String("delete")                                     ? ui::RegistryPickerDialog::Mode::DeleteValue
                                                                                                : ui::RegistryPickerDialog::Mode::CommitValue;
    auto dialog = std::make_unique<ui::RegistryPickerDialog>(std::make_shared<presentation::RegistryPickerViewModel>(mode, registry),
                                                             mode == ui::RegistryPickerDialog::Mode::Exclusion ? ui::I18n::tr("Add registry exclusion")
                                                             : scenario == QLatin1String("delete") ? ui::I18n::tr("Select registry key or value to delete")
                                                                                                   : ui::I18n::tr("Select registry key or value"));
    dialog->preselectKey(QStringLiteral("HKLM\\SOFTWARE\\MyApp"));
    panel = std::move(dialog);
  }
  if (page == QStringLiteral("review")) {
    source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = false;
    source.snapshot.current.overlay.maximumSizeMb = source.snapshot.next.overlay.maximumSizeMb = 2048;
    source.snapshot.current.overlay.criticalThresholdMb = source.snapshot.next.overlay.criticalThresholdMb = 1900;
    core::PendingChanges changes;
    if (scenario != QLatin1String("empty") && !scenario.startsWith(QLatin1String("staging-"))) {
      changes.setFilterEnabled = true;
      changes.setOverlay.maximumSizeMb = 4096;
      changes.setOverlay.criticalThresholdMb = 3800;
      changes.addFileExclusions["C:"] = {"C:\\ProgramData\\MyApp"};
    }
    std::shared_ptr<app::FileStagingDraft> draft;
    const int reviewEntries = value(QStringLiteral("--review-entries"), QStringLiteral("0")).toInt();
    for (int index = 0; index < reviewEntries; ++index) changes.addFileExclusions["C:"].push_back("C:\\ProgramData\\Preview\\entry-" + std::to_string(index));
    if (scenario.startsWith(QLatin1String("staging-"))) {
      const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\Users\\User\\Documents\\settings.json")},
                                                 {app::FileStagingKind::Directory, QStringLiteral("D:\\Data\\Project")}};
      source.snapshot.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp"};
      source.snapshot.next.registryExclusions = source.snapshot.current.registryExclusions;
      if (scenario != QLatin1String("staging-add")) {
        staging.entries = entries;
        source.snapshot.current.registryExclusions.push_back(std::string(app::kFileStagingRegistryRoot));
        source.snapshot.next.registryExclusions = source.snapshot.current.registryExclusions;
      }
      draft = std::make_shared<app::FileStagingDraft>(stagingOwner);
      static_cast<void>(draft->load());
      draft->replace(scenario == QLatin1String("staging-add")       ? entries
                     : scenario == QLatin1String("staging-partial") ? QList<app::FileStagingEntry>{entries.back()}
                                                                    : QList<app::FileStagingEntry>{});
    }
    reviewTimer.start();
    auto model = factory->applyPlan(changes, source.snapshot, std::move(draft));
    reviewTimings.insert(QStringLiteral("modelMs"), static_cast<double>(reviewTimer.nsecsElapsed()) / 1000000.0);
    reviewTimer.restart();
    panel = std::make_unique<ui::ApplyPlanDialog>(std::move(model), fileDialogs);
    reviewTimings.insert(QStringLiteral("constructMs"), static_cast<double>(reviewTimer.nsecsElapsed()) / 1000000.0);
  }
  if (page == QStringLiteral("hub")) {
    const int percent = scenario == QLatin1String("critical") ? 94 : scenario == QLatin1String("warning") ? 78 : 25;
    const auto used = static_cast<uint32_t>(4096 * percent / 100);
    const app::OverlayUsageEnabled usage({4096U - used, used}, {core::OverlayType::RAM, 4096, 3072, 3800});
    if (value(QStringLiteral("--component")) == QLatin1String("floating")) {
      auto widget = std::make_unique<ui::OverlayFloatingWidget>();
      widget->applyUsageState(usage);
      component = std::move(widget);
    } else if (value(QStringLiteral("--component")) == QLatin1String("taskbar")) {
      auto widget = std::make_unique<ui::OverlayTaskbarWidget>();
      widget->applyUsageState(usage);
      component = std::move(widget);
    }
    auto context = std::make_unique<ui::OverlayContextMenu>();
    context->setOperationAvailability(true, true);
    context->setRestorePositionAvailable(scenario == QLatin1String("floating"));
    context->setHubState(true, true);
    if (scenario != QLatin1String("floating") && scenario != QLatin1String("taskbar")) {
      trayPreview = std::make_unique<ui::TrayController>(&trayOwner);
      trayPreview->applyUsageState(usage);
      trayPreview->setHubState(true, true);
      trayPreview->setOperationAvailability(true, true);
      productionTrayMenu = trayOwner.findChild<ui::OverlayContextMenu*>();
    }
    menu = std::move(context);
  }
  if (page == QStringLiteral("import")) {
    auto model = factory->importCommands();
    if (scenario != QLatin1String("empty"))
      model->setText(
          QStringLiteral("uwfmgr filter enable\nuwfmgr overlay set-type RAM\nuwfmgr overlay set-size 4096\nuwfmgr volume protect C:\n"
                         "uwfmgr file add-exclusion \"C:\\ProgramData\\MyApp\"\nuwfmgr registry add-exclusion HKLM\\SOFTWARE\\MyApp"));
    if (scenario == QLatin1String("multiline")) {
      QStringList lines;
      for (int line = 1; line <= 24; ++line) lines.append(line % 4 == 0 ? QString{} : QStringLiteral("line %1  ·  中文输入与 English 123").arg(line));
      model->setText(lines.join(u'\n'));
    }
    panel = std::make_unique<ui::ImportDialog>(model, fileDialogs);
    if (scenario == QLatin1String("report")) {
      model->setApplier([](const QList<app::UwfmgrCommand>& commands) {
        QList<app::ImportResult> results;
        for (int index = 0; index < commands.size(); ++index) {
          const auto& command = commands[index];
          app::ImportResult result;
          result.lineNo = command.sourceLineNo;
          result.lineText = QString::fromStdString(command.rawLine);
          result.status = index == 2   ? app::ImportResult::Status::Duplicate
                          : index == 3 ? app::ImportResult::Status::Unsupported
                          : index == 4 ? app::ImportResult::Status::Failed
                                       : app::ImportResult::Status::Success;
          result.detail = index == 2   ? QStringLiteral("已在待应用变更中")
                          : index == 3 ? QStringLiteral("查询命令无需导入")
                          : index == 4 ? QStringLiteral("参数无效：请输入可用的覆盖层大小")
                                       : QStringLiteral("已加入待应用变更");
          results.append(result);
        }
        return results;
      });
      model->setText(
          QStringLiteral("uwfmgr filter enable\nuwfmgr volume protect C:\nuwfmgr filter enable\nuwfmgr overlay get-consumption\nuwfmgr overlay set-size bad"));
      model->importText();
    }
  }
  const auto diagnosticDocument = [&] {
    app::DiagnosticReportData report;
    report.context = app::DiagnosticReportService::capture(app::UwfCapability::Available, &source.snapshot, &enhanced.current, false);
    report.context.uwf.protectedVolumes = {QStringLiteral("C: · NTFS")};
    report.system.windows = system.windowsVersion();
    report.system.architecture = QStringLiteral("x64");
    report.processArchitecture = QStringLiteral("x64");
    report.version = QStringLiteral("1.0.19");
    report.qtRuntime = QStringLiteral("6.12.0");
    report.qtBuild = report.qtRuntime;
    if (scenario == QLatin1String("failure")) report.system.windowsError = QStringLiteral("Partial inventory failure");
    return presentation::DiagnosticReportViewModel(report, scenario == QLatin1String("failure") ? 0 : 65536);
  };
  if (page == QStringLiteral("about")) {
    auto about = std::make_unique<ui::AboutDialog>(diagnosticDocument);
    const auto exportPath = value(QStringLiteral("--export-path"), QStringLiteral("C:/Program Files/UWF Manager/Service/UWF.ini"));
    const auto exportError = value(QStringLiteral("--export-error"), QStringLiteral("Access denied (0x00000005)."));
    about->setConfigurationExporter([scenario, exportPath, exportError] {
      using Outcome = app::TextFileCreationResult::Outcome;
      const auto outcome = scenario == QLatin1String("about-exists")    ? Outcome::AlreadyExists
                           : scenario == QLatin1String("about-failure") ? Outcome::Failed
                                                                        : Outcome::Created;
      return presentation::UserConfigExportViewModel::Result{outcome, exportPath, outcome == Outcome::Failed ? exportError : QString{}};
    });
    panel = std::move(about);
  }
  if (page == QStringLiteral("system")) panel = std::make_unique<ui::SystemInformationDialog>(diagnosticDocument());
  if (page == QStringLiteral("logs"))
    panel = std::make_unique<ui::LogViewerDialog>(std::make_shared<presentation::LogViewModel>(std::make_shared<PreviewLogSource>(scenario)));
  if (page == QStringLiteral("overlay")) {
    panel = std::make_unique<ui::OverlayFilesDialog>(
        std::make_shared<presentation::OverlayFilesViewModel>(
            QStringLiteral("C:"),
            [scenario, docs](const QString&, std::stop_token) -> QVector<app::OverlayFileEntry> {
              if (scenario == QLatin1String("failure"))
                throw app::wmi::WmiInfrastructureError(static_cast<int32_t>(app::wmi::WmiErrorCode::RpcServerFault), "UWF_Overlay.GetOverlayFiles",
                                                       "preview provider failure");
              if (scenario == QLatin1String("empty")) return {};
              return {{QStringLiteral("\\ProgramData\\MyApp\\settings.json"), {}, false, false, 2048},
                      {QStringLiteral("\\Users\\User\\AppData\\Roaming\\Code\\User\\settings.json"), {}, false, false, 4096},
                      {docs ? QStringLiteral("\\Users\\User\\Documents\\Reports\\monthly-report.docx")
                            : QStringLiteral("\\Users\\User\\Documents\\工作记录.docx"),
                       {}, false, false, 285184},
                      {QStringLiteral("\\ProgramData\\MyApp\\Data"), {}, true, false, 0},
                      {QStringLiteral("\\$Mft"), {}, false, true, 25165824},
                      {QStringLiteral("\\Windows\\Temp\\session.log"), {}, false, false, 16384}};
            }, [](const QString&, const QString&) { return app::TextFileWriteResult{}; }),
        fileDialogs);
    auto* overlay = static_cast<ui::OverlayFilesDialog*>(panel.get());
    QObject::connect(overlay, &ui::OverlayFilesDialog::commitFileRequested, overlay,
                     [transport = &wmi, snapshot = &source.snapshot, overlay](const QString& path) {
                       using app::wmi::WmiValue;
                       transport->queryResults.push_back({{{"__PATH", WmiValue::fromString("preview-volume-current")},
                                                           {"CurrentSession", WmiValue::fromBool(true)},
                                                           {"VolumeName", WmiValue::fromString("Volume{c}")},
                                                           {"DriveLetter", WmiValue::fromString("C:")},
                                                           {"BindByDriveLetter", WmiValue::fromBool(true)},
                                                           {"CommitPending", WmiValue::fromBool(false)},
                                                           {"Protected", WmiValue::fromBool(true)}}});
                       app::CommitWorkflow workflow(*transport, *snapshot, ui::commitInteraction(overlay, {}, uwf::test::localFileDialogs()),
                                                    windows::systemCommitTraversalSource());
                       workflow.commitFilePath(path);
                     });
  }
  QWidget* root = component            ? component.get()
                  : productionTrayMenu ? productionTrayMenu
                  : menu               ? static_cast<QWidget*>(menu.get())
                  : panel              ? static_cast<QWidget*>(panel.get())
                                       : &window;
  const bool power = page == QStringLiteral("shutdown") || page == QStringLiteral("restart");
  const bool aboutFeedback = page == QLatin1String("about") && (scenario == QLatin1String("about-save") || scenario == QLatin1String("about-exists") ||
                                                                scenario == QLatin1String("about-failure"));
  const bool blocking = power || page == QStringLiteral("commit") || page == QStringLiteral("dialogs");
  reviewTimer.start();
  if (!blocking) root->show();
  if (!reviewTimings.isEmpty()) {
    reviewTimings.insert(QStringLiteral("showMs"), static_cast<double>(reviewTimer.nsecsElapsed()) / 1000000.0);
    QTimer::singleShot(0, &application, [&] {
      reviewTimings.insert(QStringLiteral("firstEventLoopMs"), static_cast<double>(reviewTimer.nsecsElapsed()) / 1000000.0);
      QSaveFile timingFile(output + QStringLiteral(".timings.json"));
      if (timingFile.open(QIODevice::WriteOnly)) {
        timingFile.write(QJsonDocument(reviewTimings).toJson());
        timingFile.commit();
      }
    });
  }
  const auto size = value(QStringLiteral("--size"), QStringLiteral("1200x798")).split(u'x');
  if (mainPage && size.size() == 2) window.resize(size[0].toInt(), size[1].toInt());
  if (panel && arguments.contains(QStringLiteral("--size")) && size.size() == 2) root->resize(size[0].toInt(), size[1].toInt());
  const auto tab = value(QStringLiteral("--tab"));
  const auto selectBusinessTab = [&] {
    if (tab.isEmpty()) return;
    for (const auto groups = window.findChildren<QTabWidget*>(QStringLiteral("innerTabs")); auto* tabs : groups)
      if (tabs->isVisibleTo(&window)) tabs->setCurrentIndex(tab == QStringLiteral("registry") ? 1 : tab == QStringLiteral("staging") ? 2 : 0);
  };
  selectBusinessTab();
  // The preview always exits after capture; it never invokes workflow commands.
  QCursor::setPos(application.primaryScreen()->availableGeometry().topLeft());
  QWidget* target = nullptr;
  const auto control = value(QStringLiteral("--control"));
  const auto controlState = value(QStringLiteral("--control-state"), QStringLiteral("rest"));
  bool captureComplete = false;
  int captureResult = 1;
  const bool waitForLoad =
      power ? scenario != QStringLiteral("preparing")
            : (page == QStringLiteral("logs") || page == QStringLiteral("overlay") || page == QLatin1String("picker")) && scenario != QStringLiteral("loading");
  const int captureDelay = value(QStringLiteral("--capture-delay"), waitForLoad ? QStringLiteral("1300") : QStringLiteral("200")).toInt();
  QTimer::singleShot(captureDelay, &application, [&] {
    if (blocking) root = QApplication::activeModalWidget();
    if (!root) return;
    if ((!mainPage || docs) && arguments.contains(QStringLiteral("--size")) && size.size() == 2) root->resize(size[0].toInt(), size[1].toInt());
    if (docs) {
      const auto area = root->screen()->availableGeometry();
      root->move(page == QLatin1String("overview") ? area.bottomRight() - QPoint(root->width() - 1, root->height() - 1)
                                                 : area.center() - QPoint(root->width() / 2, root->height() / 2));
      root->raise();
      root->activateWindow();
      SetForegroundWindow(reinterpret_cast<HWND>(root->winId()));
    }
    const auto captionStyle = value(QStringLiteral("--caption-style"));
    if (!captionStyle.isEmpty()) {
      const auto handle = reinterpret_cast<HWND>(root->winId());
      auto styles = GetWindowLongPtrW(handle, GWL_STYLE) | WS_SYSMENU;
      if (captionStyle == QLatin1String("frameless-menu")) styles &= ~WS_CAPTION;
      SetWindowLongPtrW(handle, GWL_STYLE, styles);
      SetWindowPos(handle, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    // Main-window volume tabs are populated by the asynchronous first refresh.
    if (mainPage) selectBusinessTab();
    if (docs && page == QLatin1String("overlay")) {
      if (auto* table = root->findChild<QTableView*>(QStringLiteral("overlayFilesTable"))) table->selectRow(2);
    }
    if (page == QStringLiteral("system") && scenario == QLatin1String("raw"))
      if (auto* tabs = root->findChild<QTabWidget*>(QStringLiteral("systemInformationTabs"))) tabs->setCurrentIndex(1);
    const auto expand = value(QStringLiteral("--expand"));
    for (const auto disclosures = root->findChildren<ui::Disclosure*>(); auto* disclosure : disclosures)
      if (expand == QLatin1String("all") || (!expand.isEmpty() && disclosure->objectName().contains(expand, Qt::CaseInsensitive))) disclosure->setOpen(true);
    if (aboutFeedback) {
      auto* save = root->findChild<QPushButton*>(QStringLiteral("aboutSaveConfigButton"));
      if (!save) {
        application.exit(3);
        return;
      }
      save->click();
      return;
    }
    const int toggles = value(QStringLiteral("--theme-toggle-count"), QStringLiteral("0")).toInt();
    for (int count = 0; count < toggles; ++count) ui::ThemeManager::instance().toggle();
    if (arguments.contains(QStringLiteral("--scroll-end"))) {
      QTimer::singleShot(100, root, [root] {
        for (const auto bars = root->findChildren<QScrollBar*>(); auto* bar : bars)
          if (bar->isVisibleTo(root)) bar->setValue(bar->maximum());
      });
    }
    if (page == QStringLiteral("logs")) {
      if (auto* pages = root->findChild<presentation::PageProxyModel*>()) pages->goFirst();
      if (auto* table = root->findChild<QTableView*>(); table && table->model()->rowCount() > 5) table->selectRow(5);
    }
    if (!value(QStringLiteral("--select-row")).isEmpty()) {
      for (const auto lists = window.findChildren<QListView*>(); auto* list : lists)
        if (dynamic_cast<ui::EntryListView*>(list) && list->isVisibleTo(&window)) {
          list->setCurrentIndex(list->model()->index(value(QStringLiteral("--select-row")).toInt(), 0));
          break;
        }
    }
    if (control.isEmpty()) return;
    for (const auto candidates = root->findChildren<QWidget*>(control); auto* candidate : candidates)
      if (candidate->isVisibleTo(root)) {
        target = candidate;
        break;
      }
    if (!target && control == QLatin1String("captionMaximize"))
      for (const auto buttons = root->findChildren<QAbstractButton*>(); auto* button : buttons)
        if (button->property("nativeCaptionHit").toInt() == HTMAXBUTTON) {
          target = button;
          break;
        }
    if (!target && control == QLatin1String("languageMenu"))
      for (const auto buttons = root->findChildren<QToolButton*>(); auto* button : buttons)
        if (button->defaultAction() && button->defaultAction()->text() == ui::I18n::tr("Language")) {
          target = button;
          break;
        }
    if (!target) {
      application.exit(3);
      return;
    }
    if (controlState == QLatin1String("context") || controlState == QLatin1String("window-menu")) {
      QWidget* receiver = target;
      QPoint local = target->rect().center();
      if (auto* view = qobject_cast<QAbstractItemView*>(target)) {
        receiver = view->viewport();
        const auto index = view->model()->index(value(QStringLiteral("--context-row"), QStringLiteral("0")).toInt(), 0);
        if (index.isValid()) {
          view->scrollTo(index);
          local = view->visualRect(index).center();
        }
      }
      QTimer::singleShot(100, &application, [&] {
        auto* popup = qobject_cast<QMenu*>(QApplication::activePopupWidget());
        if (!popup) {
          application.exit(5);
          return;
        }
        QDir().mkpath(QFileInfo(output).absolutePath());
        captureResult = popup->grab().save(output) ? 0 : 1;
        captureComplete = true;
        QJsonArray commands;
        for (const auto actions = popup->actions(); const auto* action : actions)
          if (!action->isSeparator())
            commands.append(QJsonObject{{"text", action->text()}, {"icon", !action->icon().isNull()}, {"enabled", action->isEnabled()}});
        QSaveFile manifest(output + QStringLiteral(".json"));
        if (manifest.open(QIODevice::WriteOnly)) {
          manifest.write(QJsonDocument(commands).toJson());
          manifest.commit();
        }
        popup->close();
        if (blocking) static_cast<QDialog*>(root)->reject();
        application.exit(captureResult);
      });
      if (controlState == QLatin1String("window-menu"))
        ui::showWindowMenu(root, target->mapToGlobal(local));
      else {
        QContextMenuEvent context(QContextMenuEvent::Mouse, local, receiver->mapToGlobal(local));
        QApplication::sendEvent(receiver, &context);
      }
      return;
    }
    if (controlState == QStringLiteral("keyboard")) {
      QKeyEvent key(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
      QApplication::sendEvent(root, &key);
      target->setFocus(Qt::OtherFocusReason);
    }
    if (controlState == QStringLiteral("disabled")) target->setEnabled(false);
    if (controlState == QStringLiteral("select-all")) {
      target->setFocus(Qt::OtherFocusReason);
      if (auto* text = qobject_cast<QTextEdit*>(target))
        text->selectAll();
      else if (auto* text = qobject_cast<QPlainTextEdit*>(target))
        text->selectAll();
      else if (auto* label = qobject_cast<QLabel*>(target))
        label->setSelection(0, static_cast<int>(label->text().size()));
    }
    if (controlState == QStringLiteral("hover") || controlState == QStringLiteral("pressed")) QCursor::setPos(target->mapToGlobal(target->rect().center()));
    if (controlState == QStringLiteral("pressed")) {
      if (auto* button = qobject_cast<QAbstractButton*>(target)) button->setDown(true);
    }
    if (controlState == QStringLiteral("popup")) {
      const QPointF local = target->rect().center();
      const QPointF global = target->mapToGlobal(local.toPoint());
      QCursor::setPos(global.toPoint());
      QMouseEvent press(QEvent::MouseButtonPress, local, global, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      QApplication::sendEvent(target, &press);
      if (auto* popup = QApplication::activePopupWidget()) {
        QMouseEvent release(QEvent::MouseButtonRelease, popup->mapFromGlobal(global.toPoint()), global, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(popup, &release);
      }
    }
  });
  QTimer captureTimer;
  bool foregroundSettled = false;
  captureTimer.setSingleShot(true);
  QObject::connect(&captureTimer, &QTimer::timeout, &application, [&] {
    if (blocking || aboutFeedback) root = QApplication::activeModalWidget();
    if (!root) {
      application.exit(4);
      return;
    }
    if ((docs || arguments.contains(QStringLiteral("--require-foreground"))) && GetForegroundWindow() != reinterpret_cast<HWND>(root->winId())) {
      if (docs && reviewTimer.elapsed() > 10000) {
        application.exit(7);
        return;
      }
      captureTimer.start(250);
      return;
    }
    if ((docs || arguments.contains(QStringLiteral("--require-foreground"))) && !foregroundSettled) {
      foregroundSettled = true;
      QCursor::setPos(target && controlState == QLatin1String("hover") ? target->mapToGlobal(target->rect().center())
                                                                       : root->screen()->availableGeometry().topLeft());
      captureTimer.start(value(QStringLiteral("--settle-delay"), QStringLiteral("250")).toInt());
      return;
    }
    const auto metrics = value(QStringLiteral("--metrics"));
    if (!metrics.isEmpty()) {
      QJsonArray widgets;
      for (const auto children = root->findChildren<QWidget*>(); const auto* widget : children) {
        if (!widget->isVisibleTo(root)) continue;
        const auto position = widget->mapTo(root, QPoint());
        QJsonObject record{{"class", QString::fromLatin1(widget->metaObject()->className())},
                           {"name", widget->objectName()},
                           {"x", position.x()},
                           {"y", position.y()},
                           {"w", widget->width()},
                           {"h", widget->height()},
                           {"minimumW", widget->minimumSizeHint().width()},
                           {"minimumH", widget->minimumSizeHint().height()},
                           {"hintH", widget->sizeHint().height()},
                           {"hasHfw", widget->hasHeightForWidth()},
                           {"hfw", widget->heightForWidth(widget->width())},
                           {"fontSize", widget->font().pixelSize()},
                           {"font", widget->font().family()}};
        if (const auto* layout = widget->layout()) {
          record["layoutHasHfw"] = layout->hasHeightForWidth();
          record["layoutHfw"] = layout->heightForWidth(widget->contentsRect().width());
          record["layoutMinimumHfw"] = layout->minimumHeightForWidth(widget->contentsRect().width());
          record["layoutTotalHfw"] = layout->totalHeightForWidth(widget->width());
        }
        if (const auto* label = qobject_cast<const QLabel*>(widget)) record["text"] = label->text();
        if (const auto* scroll = qobject_cast<const QScrollBar*>(widget)) record["range"] = scroll->maximum() - scroll->minimum();
        widgets.append(record);
      }
      QSaveFile file(metrics);
      if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(widgets).toJson());
        file.commit();
      }
    }
    QDir().mkpath(QFileInfo(output).absolutePath());
    if (arguments.contains(QStringLiteral("--dump-material"))) {
      const auto native = reinterpret_cast<HWND>(root->winId());
      int backdrop = -1;
      const auto status = DwmGetWindowAttribute(native, 38, &backdrop, sizeof(backdrop));
      BOOL dark = FALSE;
      const auto darkStatus = DwmGetWindowAttribute(native, 20, &dark, sizeof(dark));
      const QJsonObject material{{QStringLiteral("requested"), root->property("systemBackdropRequested").toBool()},
                                 {QStringLiteral("active"), root->property("systemBackdropActive").toBool()},
                                 {QStringLiteral("qtActive"), root->isActiveWindow()},
                                 {QStringLiteral("foreground"), GetForegroundWindow() == native},
                                 {QStringLiteral("dwmQuerySucceeded"), SUCCEEDED(status)},
                                 {QStringLiteral("dwmBackdropType"), backdrop},
                                 {QStringLiteral("dwmDarkQuerySucceeded"), SUCCEEDED(darkStatus)},
                                 {QStringLiteral("dwmDark"), dark != FALSE},
                                 {QStringLiteral("layered"), (GetWindowLongPtrW(native, GWL_EXSTYLE) & WS_EX_LAYERED) != 0},
                                 {QStringLiteral("nativeStyle"), QString::number(GetWindowLongPtrW(native, GWL_STYLE), 16)},
                                 {QStringLiteral("remoteSession"), GetSystemMetrics(SM_REMOTESESSION) != 0},
                                 {QStringLiteral("alphaBuffer"), root->windowHandle()->format().hasAlpha()}};
      QSaveFile info(output + QStringLiteral(".material.json"));
      if (info.open(QIODevice::WriteOnly)) {
        info.write(QJsonDocument(material).toJson());
        info.commit();
      }
      root->grab().save(output + QStringLiteral(".qt-layer.png"));
    }
    // QWidget::grab paints the widget tree alone and omits DWM materials.
    // Capture this window's composited pixels when reviewing native backdrops.
    QImage capture;
    if (docs) {
      auto* screen = root->screen();
      if (page == QLatin1String("overview")) {
        const auto area = screen->availableGeometry();
        root->move(area.bottomRight() - QPoint(root->width() - 1, root->height() - 1));
        bool taskbarConfirmed = false;
        const auto widgets = QApplication::allWidgets();
        for (const auto* widget : widgets) {
          const auto* taskbar = qobject_cast<const ui::OverlayTaskbarWidget*>(widget);
          if (taskbar && taskbar->presentationConfirmed()) taskbarConfirmed = true;
        }
        if (!taskbarConfirmed) {
          application.exit(8);
          return;
        }
      }
      DwmFlush();
      QRect documentBounds(root->mapToGlobal(QPoint()), root->size());
      if (page == QLatin1String("overview")) documentBounds.setBottomRight(screen->geometry().bottomRight());
      const auto origin = documentBounds.topLeft() - screen->geometry().topLeft();
      capture = screen->grabWindow(0, origin.x(), origin.y(), documentBounds.width(), documentBounds.height()).toImage();
    } else if (root->property("systemBackdropRequested").toBool() && GetForegroundWindow() == reinterpret_cast<HWND>(root->winId())) {
      auto* screen = root->screen();
      const QPoint origin = root->mapToGlobal(QPoint()) - screen->geometry().topLeft();
      capture = screen->grabWindow(0, origin.x(), origin.y(), root->width(), root->height()).toImage();
    } else {
      capture = root->grab().toImage();
    }
    const qreal scale = docs || arguments.contains(QStringLiteral("--native-scale")) ? capture.devicePixelRatio() : 1.0;
    if (!docs && scale == 1.0) {
      capture = capture.scaled(root->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      capture.setDevicePixelRatio(1);
    }
    QRect bounds = root->rect();
    if (target && !arguments.contains(QStringLiteral("--full-frame"))) {
      bounds = QRect(target->mapTo(root, QPoint()), target->size()).adjusted(-12, -12, 12, 12);
      if (auto* popup = QApplication::activePopupWidget(); popup && controlState == QStringLiteral("popup")) {
        const QRect popupBounds(root->mapFromGlobal(popup->mapToGlobal(QPoint())), popup->size());
        popup->grab().save(QFileInfo(output).absolutePath() + QStringLiteral("/popup-raw-qt.png"));
        QPainter composite(&capture);
        composite.drawImage(popupBounds, popup->grab().toImage());
        bounds = bounds.united(popupBounds.adjusted(-12, -12, 12, 12));
      }
    }
    const QRect crop(qRound(bounds.x() * scale), qRound(bounds.y() * scale), qRound(bounds.width() * scale), qRound(bounds.height() * scale));
    const bool saved = (docs ? capture : capture.copy(crop.intersected(capture.rect()))).save(output);
    if (docs && saved) {
      const auto* screen = root->screen();
      const QJsonObject environment{{"platform", QGuiApplication::platformName()},
                                    {"screen", screen->name()},
                                    {"screenWidth", screen->geometry().width()},
                                    {"screenHeight", screen->geometry().height()},
                                    {"dpr", capture.devicePixelRatio()},
                                    {"pixelWidth", capture.width()},
                                    {"pixelHeight", capture.height()},
                                    {"windowWidth", root->width()},
                                    {"windowHeight", root->height()}};
      QSaveFile manifest(output + QStringLiteral(".screen.json"));
      if (manifest.open(QIODevice::WriteOnly)) {
        manifest.write(QJsonDocument(environment).toJson());
        manifest.commit();
      }
    }
    captureResult = saved ? 0 : 1;
    captureComplete = true;
    if (arguments.contains(QStringLiteral("--interactive"))) {
      if (arguments.contains(QStringLiteral("--repeat-capture"))) captureTimer.start(1000);
      return;
    }
    if (!blocking)
      application.exit(captureResult);
    else if (!power || scenario != QStringLiteral("staging"))
      static_cast<QDialog*>(root)->reject();
  });
  captureTimer.start(std::max(waitForLoad ? 1500 : 500, captureDelay + 250));
  if (!power && blocking)
    QTimer::singleShot(0, &application, [&] {
      if (page == QLatin1String("dialogs")) {
        if (scenario == QLatin1String("confirm"))
          ui::dialogs::confirm(nullptr, ui::I18n::tr("Discard pending changes?"),
                               ui::I18n::tr("There are pending changes that have not been applied. Continue and discard them?"));
        else
          ui::dialogs::warning(nullptr, ui::I18n::tr("Unable to complete the operation"), ui::I18n::tr("Access denied. Check permissions and try again."));
      } else if (scenario == QLatin1String("confirm") || scenario == QLatin1String("delete")) {
        auto backend = std::make_shared<test::commit::Backend>(false);
        presentation::CommitOperationViewModel model(
            backend->request(scenario == QLatin1String("delete") ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist), backend);
        ui::CommitOperationDialog dialog(model, uwf::test::localFileDialogs(), {}, nullptr);
        QTimer::singleShot(0, &model, &presentation::CommitOperationViewModel::start);
        dialog.exec();
      } else {
        QList<presentation::CommitReportRow> rows{{presentation::CommitRowStatus::Succeeded, QStringLiteral("C:\\ProgramData\\MyApp\\settings.json"),
                                                   QStringLiteral("—"), ui::I18n::tr("Committed to disk"), true, false},
                                                  {presentation::CommitRowStatus::Failed, QStringLiteral("C:\\ProgramData\\MyApp\\Data\\cache.db"),
                                                   QStringLiteral("0x80070005"), ui::I18n::tr("Access denied"), true, true}};
        if (scenario == QLatin1String("details")) {
          rows = {{presentation::CommitRowStatus::Failed,
                   QStringLiteral("C:\\Users\\User\\Documents\\项目资料\\年度归档\\设备配置与运行记录\\"
                                  "需要在重启前保存的现场诊断数据与说明\\设备配置备份与现场工作记录.json"),
                   QStringLiteral("0x80070020"),
                   QStringLiteral("文件被其他进程占用，无法提交到磁盘。\n请关闭正在编辑此文件的程序后重试。\n"
                                  "失败详情保留完整路径与错误原因，可选择文本或复制当前记录。"),
                   true, true}};
          for (int index = 1; index < 12; ++index)
            rows.append({presentation::CommitRowStatus::Succeeded,
                         QStringLiteral("C:\\ProgramData\\MyApp\\Data\\配置记录-%1.json").arg(index, 3, 10, QChar('0')), QStringLiteral("—"),
                         ui::I18n::tr("Committed to disk"), true, false});
        }
        ui::showCommitReport(
            nullptr, rows, app::CommitOperation::Persist,
            {scenario == QLatin1String("canceled") ? presentation::CommitCompletion::Cause::Canceled : presentation::CommitCompletion::Cause::Finished,
             scenario == QLatin1String("canceled") ? 3 : 0});
      }
      application.exit(captureResult);
    });
  if (power)
    QTimer::singleShot(0, &application, [&] {
      const auto action = page == QStringLiteral("shutdown") ? app::PowerAction::Shutdown : app::PowerAction::Restart;
      app::PowerActionDialogRequest request(action);
      if (scenario.startsWith(QLatin1String("failure"))) {
        const int failureCount = scenario == QLatin1String("failure100") ? 100 : scenario == QLatin1String("failure8") ? 8 : 2;
        app::FileStagingCommitResult result;
        result.discoveredFiles = static_cast<std::size_t>(failureCount);
        result.failures = {
            {QStringLiteral("C:\\ProgramData\\MyApp\\cache.db"), app::FileStagingCommitFailureKind::ProviderFailure, QStringLiteral("拒绝访问 · 0x80070005")},
            {QStringLiteral("C:\\Users\\User\\Documents\\工作记录.docx"), app::FileStagingCommitFailureKind::ProviderFailure,
             QStringLiteral("文件被占用 · 0x80070020")}};
        for (int index = 2; index < failureCount; ++index) {
          const auto number = QStringLiteral("%1").arg(index + 1, 3, 10, QChar('0'));
          const auto path = index == 2 ? QStringLiteral(
                                             "C:\\Users\\User\\Documents\\项目资料\\年度归档\\设备配置与运行记录\\"
                                             "需要在重启前保存的现场诊断数据与说明\\配置备份-%1.json")
                                             .arg(number)
                                       : QStringLiteral("C:\\ProgramData\\MyApp\\Data\\配置记录-%1.json").arg(number);
          result.failures.append({path, app::FileStagingCommitFailureKind::ProviderFailure,
                                  index % 2 == 0 ? QStringLiteral("拒绝访问 · 0x80070005") : QStringLiteral("文件被占用 · 0x80070020")});
        }
        request.completedStagingFailure = app::PowerStagingFailure{presentation::renderFileStagingResult(result), result};
      } else if (scenario == QLatin1String("infrastructure-failure")) {
        request.completedStagingFailure = app::PowerStagingFailure{
            QStringLiteral("无法读取文件暂存列表。\n原始错误：拒绝访问 · 0x80070005\nRegistry staging store could not be opened.")};
      } else if (scenario != QStringLiteral("empty"))
        request.pollStagingPreparation = [scenario, &captureComplete]() -> std::optional<app::PowerStagingPreparation> {
          if (scenario == QStringLiteral("preparing")) return std::nullopt;
          return app::PowerStagingCommit{58543, [&captureComplete] {
                                           return app::PowerStagingProgress{
                                               captureComplete ? app::PowerStagingState::Completed : app::PowerStagingState::InProgress,
                                               17000,
                                               58543,
                                               QStringLiteral("C:\\Users\\User\\AppData\\Roaming\\Code\\User\\settings.json"),
                                               {}};
                                         }};
        };
      QTimer driver;
      driver.setInterval(20);
      if (scenario == QStringLiteral("staging")) {
        QObject::connect(&driver, &QTimer::timeout, &application, [&] {
          auto* modal = QApplication::activeModalWidget();
          auto* button = modal
                             ? modal->findChild<QPushButton*>(action == app::PowerAction::Shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn"))
                             : nullptr;
          if (button && button->isEnabled()) {
            driver.stop();
            button->click();
          }
        });
        driver.start();
      }
      ui::runPowerActionDialog(nullptr, std::move(request));
      application.exit(captureResult);
    });
  return application.exec();
}
