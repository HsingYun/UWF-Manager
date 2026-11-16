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
#include <QEnterEvent>
#include <QFile>
#include <QFileInfo>
#include <QHoverEvent>
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
#include <QScreen>
#include <QScrollBar>
#include <QTabBar>
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
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#include "application/commit/CommitWorkflow.h"
#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/operations/contracts/OperationError.h"
#include "application/overlay/files/OverlayFilesQueryManager.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/staging/policy/AutomaticCommitPolicy.h"
#include "application/state/contracts/ApplicationState.h"
#include "application/uwf/adapters/UwfCall.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "application/uwf/adapters/UwfOverlayMaintenance.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "composition/desktop/MainWindowComposition.h"
#include "composition/desktop/ViewModelComposition.h"
#include "foundation/paths/DriveLetter.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/appearance/WindowAppearance.h"
#include "presentation/controls/navigation/Disclosure.h"
#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/models/PageProxyModel.h"
#include "presentation/viewmodels/enhanced/EnhancedModeViewModel.h"
#include "presentation/viewmodels/files/FilePickerViewModel.h"
#include "presentation/viewmodels/staging/FileStagingResultText.h"
#include "presentation/views/commit/CommitInteractionAdapter.h"
#include "presentation/views/commit/CommitOperationDialog.h"
#include "presentation/views/commit/CommitReportDialog.h"
#include "presentation/views/configuration/importing/ImportDialog.h"
#include "presentation/views/configuration/overlay/HormUsageDialog.h"
#include "presentation/views/configuration/overlay/OverlayAdvancedDetailsDialog.h"
#include "presentation/views/configuration/overlay/OverlayAdvancedDialog.h"
#include "presentation/views/configuration/overlay/OverlayMaintenanceDialogs.h"
#include "presentation/views/configuration/review/ApplyPlanDialog.h"
#include "presentation/views/diagnostics/AboutDialog.h"
#include "presentation/views/diagnostics/LogViewerDialog.h"
#include "presentation/views/diagnostics/SystemInformationDialog.h"
#include "presentation/views/enhanced/EnhancedModeDialog.h"
#include "presentation/views/files/FilePickerDialog.h"
#include "presentation/views/overlay/coordination/OverlayContextMenu.h"
#include "presentation/views/overlay/files/OverlayFilesDialog.h"
#include "presentation/views/overlay/hud/OverlayFloatingWidget.h"
#include "presentation/views/overlay/hud/OverlayTaskbarWidget.h"
#include "presentation/views/power/PowerActionDialog.h"
#include "presentation/views/registry/RegistryPickerDialog.h"
#include "presentation/views/shell/MainWindow.h"
#include "presentation/views/shell/TrayController.h"
#include "support/app/CommitScenario.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/EnhancedModeFixture.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/OverlayPolicyFixture.h"
#include "support/ui/FileDialogStub.h"
#include "support/ui/LocalFileDialogs.h"
#include "support/uwf/FakeUwfBackend.h"
using namespace uwf;
namespace {
class PreviewVolumeRepository final : public app::VolumeRepository {
 public:
  explicit PreviewVolumeRepository(std::vector<core::VolumeInfo> values) : m_volumes(std::move(values)) {}
  std::vector<core::VolumeInfo> read() const override { return m_volumes; }
  std::string driveForPath(const std::string& path) const override {
    for (const auto& volume : m_volumes)
      if (path.starts_with(volume.volumeName) || path.starts_with(volume.driveLetter)) return volume.driveLetter;
    return {};
  }
  std::vector<std::string> mountedLetters(const std::string& id) const override {
    for (const auto& volume : m_volumes)
      if (compat::normalizeVolumeId(volume.volumeName) == compat::normalizeVolumeId(id) && !volume.driveLetter.empty()) return {volume.driveLetter};
    return {};
  }
  std::vector<std::string> mountedPaths(const std::string& id) const override {
    std::vector<std::string> paths;
    for (const auto& letter : mountedLetters(id)) paths.push_back(letter + '\\');
    return paths;
  }
  std::string systemDrive() const override { return "C:"; }

 private:
  std::vector<core::VolumeInfo> m_volumes;
};
class PreviewApplicationStateSource final : public app::ApplicationStateSource {
 public:
  std::vector<core::VolumeInfo> volumes;
  core::UwfSnapshot snapshot;
  app::OverlayPolicySnapshot policy;
  QString failure;
  bool hormObservationUnavailable = false;
  std::shared_ptr<compat::Uwf> backend;
  ReadOperation readOperation(app::UwfCapability capability) override {
    return [candidate = app::ApplicationState{volumes, snapshot, policy}, error = failure, system = backend, capability,
            hormUnavailable = hormObservationUnavailable](std::stop_token) mutable {
      if (!error.isEmpty()) throw std::runtime_error(error.toStdString());
      if (system && candidate.snapshot.uwfAvailable) {
        const auto usage = compat::call(*system, compat::GetOverlayUsage{});
        auto facts = system->state();
        if (hormUnavailable) facts.runtime.horm = compat::Fact<bool>::unknown();
        candidate.snapshot = app::publishedUwfSnapshot(facts, capability, candidate.snapshot.elevated, usage);
        candidate.overlayPolicy = app::UwfOverlayOperations::snapshot(facts);
        candidate.hormEnabled = facts.runtime.horm.value;
      }
      return candidate;
    };
  }
};
// Derive runtime fixture facts from physical inventory, independently of UWF registration.
test::FakeUwfBackend::State previewUwfState(const app::ApplicationState& source, const std::string& systemDriveLetter) {
  const auto& snapshot = source.snapshot;
  const auto& policy = source.overlayPolicy;
  const auto systemDrive = drive::normalize(systemDriveLetter);
  const auto systemVolume = std::ranges::find_if(
      source.volumes, [&](const core::VolumeInfo& volume) { return !systemDrive.empty() && drive::normalize(volume.driveLetter) == systemDrive; });
  if (systemVolume == source.volumes.end() || systemVolume->volumeName.empty())
    throw std::invalid_argument("The preview system volume is missing from the physical inventory");
  test::FakeUwfBackend::State state;
  const auto session = [](const core::SessionSnapshot& observed, const app::OverlayPolicySession& options) {
    test::FakeUwfBackend::Configuration target;
    target.filterEnabled = observed.filter.enabled;
    target.overlayType = static_cast<std::uint32_t>(observed.overlay.type);
    target.maximumOverlaySize = observed.overlay.maximumSizeMb;
    target.warningThreshold = observed.overlay.warningThresholdMb;
    target.criticalThreshold = observed.overlay.criticalThresholdMb;
    target.overlayFlags = options.flags.value ? options.flags.value->raw : 0;
    target.registryExclusions = observed.registryExclusions;
    target.persistDomainSecretKey = observed.persistDomainSecretKey;
    target.persistTscal = observed.persistTSCAL;
    for (const auto& volume : observed.volumes) {
      const auto id = compat::normalizeVolumeId(volume.volumeName);
      test::FakeUwfBackend::VolumeConfiguration configured{
          .volume = {id, volume.driveLetter, volume.isProtected, volume.binding == core::VolumeBinding::DriveLetter, {}},
          .fileExclusions = {},
          .swapfileSize = 0};
      if (const auto found = observed.fileExclusions.find(volume.volumeName); found != observed.fileExclusions.end()) configured.fileExclusions = found->second;
      target.volumes.emplace(id, std::move(configured));
    }
    return target;
  };
  state.current = session(snapshot.current, policy.current);
  state.next = session(snapshot.next, policy.next);
  state.runtime.administrator = snapshot.elevated;
  state.runtime.systemVolumeId = compat::normalizeVolumeId(systemVolume->volumeName);
  state.runtime.resetPersistentState = static_cast<std::uint32_t>(policy.resetPersistentState.value.value_or(0));
  state.runtime.usageOverride = compat::OverlayUsage{snapshot.runtime.currentConsumptionMb, snapshot.runtime.availableSpaceMb};
  state.runtime.hormEnabled = snapshot.runtime.hormEnabled.value_or(false);
  state.runtime.romDisposition = snapshot.runtime.romDisposition.value_or(0);
  return state;
}

class PreviewFileBrowser final : public app::FileBrowser {
 public:
  QString scenario;
  QVector<app::FileBrowserEntry> list(const QString& path, const std::stop_token stop) const override {
    if (scenario == QLatin1String("loading"))
      while (!stop.stop_requested()) QThread::msleep(10);
    if (scenario == QLatin1String("error")) throw std::runtime_error("拒绝访问 · 0x80070005");
    if (scenario == QLatin1String("error-long"))
      throw std::runtime_error(
          "Access denied: C:/Enterprise deployment/Managed application data/<literal & complete>/Protected directory\n"
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
class PreviewCommitTraversalSource final : public app::CommitTraversalSource {
 public:
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override {
    auto result = target;
    if (!result.fileTarget) result.fileTarget = resolveFileTarget(result.path);
    return result;
  }
  bool exists(const app::CommitNode&) const override { return true; }
  std::string driveLetterForPath(const QString& path) const override { return drive::normalize(path.left(2).toStdString()); }
  app::FileCommitTarget resolveFileTarget(const QString& path) const override {
    const auto root = driveLetterForPath(path);
    const auto volume = root == "C:" ? test::application::fixtureVolumeCName : root == "D:" ? test::application::fixtureVolumeDName : std::string{};
    if (volume.empty()) throw std::runtime_error("The preview path has no observed volume binding");
    auto relative = path.mid(2);
    relative.replace(u'/', u'\\');
    return {volume, relative};
  }
  app::CommitEnumerationError enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override { return {}; }
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
  ChangeWork prepareChange(app::EnhancedModeAction, const QString&) override {
    return [result = current] { return app::EnhancedModeChangeResult{result, {}}; };
  }
};
// Exercise the production confirmation/progress view without touching a live
// provider. Delay belongs to the worker, so screenshots also expose GUI stalls.
class PreviewApplyActions final : public app::ConfigurationActions {
 public:
  int workerDelayMs = 0;
  bool fail = false;
  ApplyOperation prepareApply(const core::PendingChanges& changes, const core::UwfSnapshot& snapshot) override {
    return prepareApply(app::planConfiguration(changes, snapshot));
  }
  ApplyOperation prepareApply(const app::ConfigurationPlan& plan) override {
    return [delay = workerDelayMs, failure = fail, operations = plan.operations()] {
      if (delay > 0) QThread::msleep(static_cast<unsigned long>(delay));
      if (failure) throw std::runtime_error("Preview injected configuration failure");
      app::ConfigurationApplyResult result;
      for (const auto& operation : operations)
        result.steps.push_back({"Preview configuration operation", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed, operation});
      return result;
    };
  }
};

app::AutomaticCommitSummary previewAutomaticCommitSummary(const QString& scenario) {
  app::AutomaticCommitContext context{test::application::diskPersistentPolicy(), false, 0, false};
  const bool retained = scenario == QLatin1String("skip-retained") || scenario == QLatin1String("mixed-retained") ||
                        scenario == QLatin1String("mixed-reasons") || scenario == QLatin1String("skip-unknown-retained");
  if (!retained) {
    context.overlay.current.overlayType.value = context.overlay.next.overlayType.value = 0;
    context.overlay.current.flags.value = context.overlay.next.flags.value = app::OverlayFlags{0};
  }
  if (scenario == QLatin1String("skip-horm")) {
    context.hormEnabled = true;
    context.romDisposition = 2;
    context.overlay.current.flags.value = context.overlay.next.flags.value = app::OverlayFlags{4};
  } else if (scenario == QLatin1String("skip-rom")) {
    context.overlay.current.flags.value = context.overlay.next.flags.value = app::OverlayFlags{4};
  } else if (scenario == QLatin1String("skip-transition")) {
    context.romDisposition = 3;
    context.overlay.current.flags.value = context.overlay.next.flags.value = app::OverlayFlags{4};
  } else if (scenario == QLatin1String("skip-fullcommit")) {
    context.fullCommit = true;
    context.romDisposition = 4;
    context.overlay.current.flags.value = context.overlay.next.flags.value = app::OverlayFlags{4};
  }
  app::AutomaticCommitSummary summary;
  summary.include(app::automaticFileCommitDisposition(context, true, true));
  if (scenario == QLatin1String("mixed-retained") || scenario == QLatin1String("mixed-reasons"))
    summary.include(app::automaticFileCommitDisposition(context, true, false));
  if (scenario == QLatin1String("mixed-reasons")) summary.include(app::automaticFileCommitDisposition(context, false, false));
  if (scenario == QLatin1String("mixed-reasons") || scenario == QLatin1String("skip-unknown-retained"))
    summary.include(app::automaticFileCommitDisposition(context, std::nullopt, std::nullopt));
  if (scenario == QLatin1String("skip-already-persistent")) {
    summary = {};
    summary.include(app::automaticFileCommitDisposition(context, false, false));
  }
  return summary;
}
class PreviewRegistryBrowser final : public app::RegistryBrowser {
 public:
  bool failure = false;
  std::vector<std::string> rootHiveLongNames(std::stop_token = {}) const override {
    return {"HKEY_LOCAL_MACHINE", "HKEY_CURRENT_USER", "HKEY_CLASSES_ROOT", "HKEY_USERS", "HKEY_CURRENT_CONFIG"};
  }
  std::vector<std::string> subkeyNames(std::string_view key, std::stop_token = {}) const override {
    if (key == "HKEY_LOCAL_MACHINE") return {"SOFTWARE", "SYSTEM", "SECURITY"};
    if (key == "HKEY_LOCAL_MACHINE\\SOFTWARE") return {"MyApp", "Microsoft", "Policies"};
    if (key == "HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp") return {"Settings", "Data"};
    if (failure && key.find("MyApp") != std::string_view::npos) throw std::runtime_error("拒绝访问 · 0x80070005");
    return {};
  }
  bool hasSubkeys(std::string_view key, std::stop_token = {}) const override {
    return key == "HKEY_LOCAL_MACHINE" || key == "HKEY_LOCAL_MACHINE\\SOFTWARE" || key == "HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp";
  }
  std::vector<app::RegistryValueInfo> values(std::string_view key, std::stop_token = {}) const override {
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
  QDir().mkpath(QFileInfo(output).absolutePath());
  const auto page = value(QStringLiteral("--page"), QStringLiteral("main"));
  const auto scenario = value(QStringLiteral("--scenario"), QStringLiteral("normal"));
  const bool applyProgress = page == QLatin1String("review") && scenario.startsWith(QLatin1String("apply-"));
  const bool docs = arguments.contains(QStringLiteral("--docs"));
  const bool visibleCapture = arguments.contains(QStringLiteral("--visible-capture"));
  if (docs && QGuiApplication::platformName() != QLatin1String("windows")) return 6;
  const bool mainPage = page == QLatin1String("main") || page == QLatin1String("overview");
  const bool stagingScenario = mainPage && scenario.startsWith(QLatin1String("staging-"));
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
  const auto uwfSystem = std::make_shared<test::FakeUwfBackend>();
  auto stagingOwner = std::make_shared<test::application::MemoryFileStagingStore>();
  auto& staging = *stagingOwner;
  PreviewApplicationStateSource source;
  auto serviceOwner = std::make_shared<test::application::MemoryEnhancedModeServiceControl>();
  test::application::MemoryEnhancedModeAgent agent;
  source.snapshot = test::application::editableSnapshot();
  source.snapshot.next = source.snapshot.current;
  source.snapshot.current.overlay = {core::OverlayType::RAM, 4096, 3072, 3800};
  source.snapshot.next.overlay = source.snapshot.current.overlay;
  source.snapshot.runtime = {3072, 1024};
  source.snapshot.current.fileExclusions[uwf::test::application::fixtureVolumeCName] = {"\\ProgramData\\MyApp\\Logs", "\\Users\\Public\\Documents",
                                                                                        "\\Windows\\Temp", "\\ProgramData\\MyApp\\settings.json"};
  source.snapshot.next.fileExclusions = source.snapshot.current.fileExclusions;
  source.snapshot.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp", "HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun\\UWF Manager"};
  source.snapshot.next.registryExclusions = source.snapshot.current.registryExclusions;
  source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "系统", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported},
                    {"D:", uwf::test::application::fixtureVolumeDName, "NTFS", "数据", 1ULL << 40, 608ULL << 30, core::VolumeSupport::Supported}};
  if (scenario == QLatin1String("unsupported-exfat"))
    source.volumes.push_back({"E:",
                              "Volume{bd2f3e85-b66d-40ed-9c1a-191b4e83f34c}",
                              "exFAT",
                              "Ventoy",
                              476ULL << 30,
                              472ULL << 30,
                              core::VolumeSupport::FileSystemLimited,
                              {true, false}});
  if (docs) {
    source.snapshot.current.fileExclusions[uwf::test::application::fixtureVolumeCName] = {"\\ProgramData\\MyApp\\Logs",
                                                                                          "\\ProgramData\\MyApp\\settings.json",
                                                                                          "\\ProgramData\\DeviceAgent\\Logs",
                                                                                          "\\ProgramData\\DeviceAgent\\state.json",
                                                                                          "\\ProgramData\\Kiosk\\Cache",
                                                                                          "\\ProgramData\\Kiosk\\Uploads",
                                                                                          "\\ProgramData\\Microsoft\\Windows\\WER",
                                                                                          "\\ProgramData\\Terminal\\Receipts",
                                                                                          "\\ProgramData\\Terminal\\Reports",
                                                                                          "\\Users\\Public\\Documents",
                                                                                          "\\Users\\Public\\Pictures",
                                                                                          "\\Windows\\Temp"};
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
  if (stagingScenario) {
    staging.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\ProgramData\\DeviceAgent\\state.json")},
                       {app::FileStagingKind::Directory, QStringLiteral("C:\\ProgramData\\DeviceAgent\\Logs")}};
    serviceOwner->current = test::application::completeEnhancedModeStatus();
    agent.connectOnStart = true;
  }
  const auto state = value(QStringLiteral("--state"));
  if (scenario == QLatin1String("hidden-protected")) {
    const auto current = std::string("Volume{aaaaaaaa-1111-2222-3333-444444444444}");
    const auto next = std::string("Volume{bbbbbbbb-1111-2222-3333-444444444444}");
    source.snapshot.current.volumes.push_back({current, {}, true, core::VolumeBinding::VolumeId});
    source.snapshot.next.volumes.push_back({current, {}, false, core::VolumeBinding::VolumeId});
    source.snapshot.next.volumes.push_back({next, {}, true, core::VolumeBinding::VolumeId});
  }
  if (state == QLatin1String("warning")) source.snapshot.runtime = {902, 3194};
  if (state == QStringLiteral("readonly")) source.snapshot.elevated = false;
  if (state == QStringLiteral("unavailable")) {
    source.snapshot.uwfAvailable = false;
    source.snapshot.unavailableReason = "UWF_Overlay.GetOverlayConfig\n0x80041004 · WBEM_E_PROVIDER_FAILURE";
  }
  if (state == QStringLiteral("filter-off")) source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = false;
  if (page == QLatin1String("overlay-advanced") || scenario.startsWith(QLatin1String("overlay-"))) {
    source.policy = test::application::diskPersistentPolicy();
    source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::Disk;
    if (scenario == QLatin1String("readonly")) source.snapshot.elevated = false;
    if (scenario == QLatin1String("dll-failure")) source.policy.transportAvailable = false;
    if (scenario == QLatin1String("read-failure")) {
      source.policy.current.flags = source.policy.next.flags = {app::PolicyObservationStatus::Failed};
      source.policy.resetPersistentState = {app::PolicyObservationStatus::Failed};
    }
    if (scenario == QLatin1String("unsupported")) source.policy.resetPersistentState = {app::PolicyObservationStatus::Unsupported};
    if (scenario == QLatin1String("saved-selector")) source.policy.resetPersistentState.value = 255;
    if (scenario == QLatin1String("reset-pending")) source.policy.resetPersistentState.value = 1;
    if (scenario == QLatin1String("disk-off") || scenario == QLatin1String("ram")) {
      source.snapshot.current.filter.enabled = false;
      source.policy.current.filterEnabled.value = false;
    }
    if (scenario == QLatin1String("ram") || scenario == QLatin1String("active") || scenario == QLatin1String("rom-active")) {
      source.policy.current.overlayType.value = source.policy.next.overlayType.value = 0;
      source.policy.current.flags.value = source.policy.next.flags.value = app::OverlayFlags{scenario == QLatin1String("rom-active") ? 5U : 1U};
      source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::RAM;
    }
  }
  const auto runtimeMode = state.isEmpty() ? scenario : state;
  const bool maintenancePage =
      page == QLatin1String("maintenance") || page == QLatin1String("full-commit") || page == QLatin1String("swap") || page == QLatin1String("horm");
  const bool romMode = runtimeMode.startsWith(QLatin1String("rom-")) || runtimeMode.startsWith(QLatin1String("horm-")) ||
                       page == QLatin1String("full-commit") || page == QLatin1String("horm");
  const bool swapMode = runtimeMode.startsWith(QLatin1String("swap-")) || page == QLatin1String("swap");
  if (romMode || swapMode || maintenancePage) {
    source.policy = test::application::diskPersistentPolicy();
    source.snapshot.current.filter.enabled = !swapMode;
    source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::RAM;
    source.policy.current.filterEnabled.value = !swapMode;
    source.policy.current.overlayType.value = source.policy.next.overlayType.value = 0;
    source.policy.current.flags.value = app::OverlayFlags{romMode && runtimeMode != QLatin1String("rom-next") ? 4U : 0U};
    source.policy.next.flags.value = app::OverlayFlags{romMode ? 4U : 0U};
    if (runtimeMode.startsWith(QLatin1String("horm-")) || page == QLatin1String("horm")) {
      for (auto& volume : source.snapshot.current.volumes) volume.isProtected = true;
      for (auto& volume : source.snapshot.next.volumes) volume.isProtected = true;
    }
  }
  if (runtimeMode == QLatin1String("horm-filter-off")) {
    source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = false;
    source.policy.current.filterEnabled.value = source.policy.next.filterEnabled.value = false;
  }
  if (stagingScenario) {
    const bool alreadyPersistent = scenario == QLatin1String("staging-already-persistent");
    source.policy = test::application::diskPersistentPolicy();
    source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::Disk;
    source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = !alreadyPersistent;
    source.policy.current.filterEnabled.value = source.policy.next.filterEnabled.value = !alreadyPersistent;
  }
  const auto system = std::make_shared<PreviewSystemInformation>();
  auto ports = composition::systemDesktopPlatformPorts();
  ports.systemInformation = system;
  ports.volumes = std::make_shared<PreviewVolumeRepository>(source.volumes);
  if (stagingScenario) {
    ports.traversal = std::make_shared<PreviewCommitTraversalSource>();
    ports.files = std::make_shared<PreviewFileBrowser>();
  }
  auto runtimeState = previewUwfState({source.volumes, source.snapshot, source.policy}, system->systemDriveLetter().toStdString());
  runtimeState.runtime.hormEnabled = runtimeMode == QLatin1String("horm-active") || runtimeMode == QLatin1String("horm-disable");
  if (runtimeState.runtime.hormEnabled) runtimeState.runtime.romDisposition = 2;
  if (runtimeMode == QLatin1String("rom-commit-pending")) runtimeState.runtime.romDisposition = 4;
  if (runtimeMode == QLatin1String("horm-no-hiberfile")) runtimeState.runtime.hiberfileExists = false;
  if (runtimeMode == QLatin1String("horm-unprotected")) runtimeState.runtime.hormFixedVolumeSafety = test::FakeUwfBackend::SafetyObservation::Rejected;
  if (runtimeMode == QLatin1String("horm-state-changed")) runtimeState.runtime.romDisposition = 3;
  if (runtimeMode == QLatin1String("swap-selected") || runtimeMode == QLatin1String("swap-pending")) {
    runtimeState.next.overlayType = 1;
    runtimeState.next.volumes.at(compat::normalizeVolumeId(test::application::fixtureVolumeDName)).swapfileSize = 1;
  }
  const auto seedPreviewSystem = [uwfSystem, runtimeMode, &source](test::FakeUwfBackend::State state) {
    uwfSystem->seed(std::move(state));
    auto facts = uwfSystem->state();
    std::vector<std::string> targets;
    for (const auto& [id, configured] : facts.next.volumes)
      if (configured.present.value == true) targets.push_back(id);
    // Include an additional partition identity outside the picker inventory.
    targets.push_back("dddddddd-eeee-ffff-aaaa-444444444444");
    if (runtimeMode == QLatin1String("horm-unprotected")) {
      targets.insert(targets.end(), {"66666666-7777-8888-9999-aaaaaaaaaaaa", "22222222-3333-4444-5555-666666666666", "aaaaaaaa-bbbb-cccc-dddd-111111111111",
                                     "bbbbbbbb-cccc-dddd-eeee-222222222222", "cccccccc-dddd-eeee-ffff-333333333333"});
    }
    std::ranges::sort(targets);
    targets.erase(std::unique(targets.begin(), targets.end()), targets.end());
    facts.runtime.protectScope = compat::Fact<std::vector<std::string>>::known(std::move(targets));
    uwfSystem->updateState(std::move(facts));
    // Keep the core UI snapshot and provider facts in agreement through the
    // production projection, including the system-volume GUID/protection.
    source.snapshot.runtime = app::publishedUwfSnapshot(uwfSystem->state(), app::UwfCapability::Available, source.snapshot.elevated,
                                                        {source.snapshot.runtime.currentConsumptionMb, source.snapshot.runtime.availableSpaceMb})
                                  .runtime;
  };
  seedPreviewSystem(std::move(runtimeState));
  // Replace all production maintenance observations with preview fixtures.
  ports.maintenanceObservations = {};

  ports.maintenanceObservations.hibernationCapabilities = [uwfSystem, runtimeMode](const std::stop_token stop) {
    if (stop.stop_requested()) throw std::runtime_error("Preview power-capability observation canceled");
    return app::OverlayHibernationCapabilities{
        runtimeMode != QLatin1String("horm-no-s4"), uwfSystem->snapshot().runtime.hiberfileExists,
        runtimeMode == QLatin1String("horm-reduced-hiberfile") ? app::HibernationFileType::Reduced : app::HibernationFileType::Full};
  };

  ports.maintenanceObservations.hormVolumeProtection = [uwfSystem](const std::stop_token stop) {
    if (stop.stop_requested()) throw std::runtime_error("Preview observation canceled");
    const auto safety = uwfSystem->snapshot().runtime.hormFixedVolumeSafety;
    if (safety == test::FakeUwfBackend::SafetyObservation::Unknown) throw std::runtime_error("The preview runtime volume observation is unavailable");
    const bool protectedVolumes = safety == test::FakeUwfBackend::SafetyObservation::Satisfied;
    app::OverlayHormVolumeProtection result{protectedVolumes, {}, {}};
    if (!protectedVolumes) {
      // Runtime blockers are layout fixtures independent of the storage-picker inventory.
      result.blockedVolumes = {{QStringLiteral("66666666-7777-8888-9999-aaaaaaaaaaaa"), QStringLiteral("D:")},
                               {QStringLiteral("22222222-3333-4444-5555-666666666666"), QStringLiteral("F:")},
                               {QStringLiteral("aaaaaaaa-bbbb-cccc-dddd-111111111111"), {}},
                               {QStringLiteral("bbbbbbbb-cccc-dddd-eeee-222222222222"), {}},
                               {QStringLiteral("cccccccc-dddd-eeee-ffff-333333333333"), {}}};
    }
    return result;
  };
  if (runtimeMode == QLatin1String("rom-state-unknown"))
    uwfSystem->setCapability(compat::GetBoolean{compat::BooleanProperty::OverlayCommit}, {compat::Support::Unavailable, "Fixture read failure"});
  if (runtimeMode == QLatin1String("rom-commit-unavailable"))
    uwfSystem->setCapability(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true}, {compat::Support::Unsupported, "Fixture unsupported"});
  if (runtimeMode == QLatin1String("horm-unknown") || runtimeMode == QLatin1String("mode-unknown"))
    uwfSystem->setCapability(compat::GetBoolean{compat::BooleanProperty::HormEnabled}, {compat::Support::Unavailable, "Fixture read failure"});
  if (runtimeMode == QLatin1String("horm-unavailable"))
    uwfSystem->setCapability(compat::SetBoolean{compat::BooleanProperty::HormEnabled, true}, {compat::Support::Unsupported, "Fixture unsupported"});
  source.backend = uwfSystem;
  source.hormObservationUnavailable = runtimeMode == QLatin1String("horm-unknown") || runtimeMode == QLatin1String("mode-unknown");
  auto services = composition::mainWindowServices(uwfSystem, source, stagingOwner, ports, serviceOwner, &agent);
  services.viewModelFactory = composition::createViewModelFactory(uwfSystem, ports);
  const auto factory = services.viewModelFactory;
  ui::MainWindow window(std::move(services), {.uwfCapability = app::UwfCapability::Available,
                                              .compatibilityMode = state == QLatin1String("compatibility"),
                                              .hubVisible = docs && page == QLatin1String("overview")});
  test::FileDialogStub fileDialogs;
  PreviewEnhancedActions enhanced;
  enhanced.current.agentState = app::EnhancedModeAgentState::Connected;
  const auto registry = std::make_shared<PreviewRegistryBrowser>();
  std::unique_ptr<QMenu> menu;
  std::unique_ptr<QDialog> panel;
  std::shared_ptr<presentation::ApplyPlanViewModel> reviewModel;
  std::unique_ptr<QWidget> component;
  if (maintenancePage) {
    const auto maintenance = factory->overlayMaintenance();
    maintenance->setInteractionContext(source.snapshot.elevated, false);
    maintenance->refresh();
    if (page == QLatin1String("full-commit"))
      panel = std::make_unique<ui::FullCommitDialog>(maintenance);
    else if (page == QLatin1String("swap"))
      panel = std::make_unique<ui::SwapDialog>(maintenance);
    else if (page == QLatin1String("horm"))
      panel = std::make_unique<ui::HormDialog>(maintenance);
    else {
      auto editor = std::make_shared<app::OverlayPolicyEditor>();
      editor->reset(source.policy, source.snapshot);
      auto advanced = std::make_shared<presentation::OverlayAdvancedViewModel>(std::move(editor));
      auto dialog = std::make_unique<ui::OverlayAdvancedDialog>(std::move(advanced), maintenance);
      dialog->findChild<QTabWidget*>(QStringLiteral("overlayAdvancedTabs"))->setCurrentIndex(1);
      panel = std::move(dialog);
    }
  } else if (page == QLatin1String("horm-usage")) {
    panel = std::make_unique<ui::HormUsageDialog>();
  } else if (page == QLatin1String("overlay-advanced")) {
    auto editor = std::make_shared<app::OverlayPolicyEditor>();
    editor->reset(source.policy, source.snapshot);
    if (scenario == QLatin1String("conflict-type")) editor->setOverlayChanges({.type = core::OverlayType::RAM});
    if (scenario == QLatin1String("conflict-reset")) {
      editor->importChanges({.persistent = false, .reset = app::PersistentReset::Clear});
    }
    if (scenario == QLatin1String("draft")) {
      editor->importChanges({.passthrough = false});
    }
    auto model = std::make_shared<presentation::OverlayAdvancedViewModel>(std::move(editor));
    const auto detail = value(QStringLiteral("--detail"));
    if (detail.isEmpty()) {
      auto maintenance = factory->overlayMaintenance();
      maintenance->setInteractionContext(source.snapshot.elevated, false);
      maintenance->refresh();
      panel = std::make_unique<ui::OverlayAdvancedDialog>(std::move(model), std::move(maintenance));
    } else {
      using Details = ui::OverlayAdvancedDetailsDialog;
      const auto field = value(QStringLiteral("--field"));
      const auto selected = field == QLatin1String("persistent") ? Details::Field::Persistent
                            : field == QLatin1String("reset")    ? Details::Field::Reset
                            : field == QLatin1String("rom")      ? Details::Field::ReadOnlyMedia
                                                                 : Details::Field::Passthrough;
      const auto purpose = detail == QLatin1String("reason")      ? Details::Purpose::Restriction
                           : detail == QLatin1String("conflicts") ? Details::Purpose::Conflicts
                                                                  : Details::Purpose::Feature;
      panel = std::make_unique<Details>(model, model->beginEdit(), purpose, selected);
    }
  }
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
    auto model = factory->enhancedMode(enhanced);
    if (scenario.startsWith(QLatin1String("automatic-"))) {
      const auto kind = scenario.mid(10);
      const auto summary = previewAutomaticCommitSummary(kind == QLatin1String("mixed")           ? QStringLiteral("mixed-retained")
                                                         : kind == QLatin1String("mixed-reasons") ? kind
                                                                                                  : QStringLiteral("skip-") + kind);
      const auto restriction = kind == QLatin1String("horm")                                         ? app::RuntimeWriteRestriction::HormActive
                               : kind == QLatin1String("fullcommit") || kind == QLatin1String("rom") ? app::RuntimeWriteRestriction::ReadOnlyMediaActive
                                                                                                     : app::RuntimeWriteRestriction::None;
      model->setWriteRestriction(restriction);
      model->setAutomaticCommitSummary(summary);
    }
    panel = std::make_unique<ui::EnhancedModeDialog>(std::move(model));
  }
  if (page == QStringLiteral("registry")) {
    registry->failure = scenario == QLatin1String("error");
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
    const bool hormDisable = scenario == QLatin1String("horm-disable");
    const bool hormEnable = scenario == QLatin1String("horm-enable");
    const bool fullCommit = scenario == QLatin1String("full-commit");
    const bool storageChange = scenario == QLatin1String("storage-change");
    const bool resetClear = scenario == QLatin1String("reset-clear");
    const bool resetPreserve = scenario == QLatin1String("reset-preserve");
    const bool resetReview = resetClear || resetPreserve;
    const bool nativeMaintenance = hormDisable || hormEnable || fullCommit || storageChange;
    if (nativeMaintenance) {
      source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = !storageChange;
      source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::RAM;
      source.policy = test::application::diskPersistentPolicy();
      source.policy.current.filterEnabled.value = source.policy.next.filterEnabled.value = !storageChange;
      source.policy.current.overlayType.value = source.policy.next.overlayType.value = 0;
      source.policy.current.flags.value = source.policy.next.flags.value = app::OverlayFlags{storageChange ? 0U : 4U};
      source.snapshot.runtime.hormEnabled = hormDisable;
      source.snapshot.runtime.fullCommit = false;
      source.snapshot.runtime.romDisposition = hormDisable ? 2U : 0U;
      source.snapshot.runtime.currentOverlayFlags = storageChange ? 0U : 4U;
      source.snapshot.runtime.systemVolumeProtected = true;
    } else {
      source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = false;
    }
    if (resetReview) {
      source.snapshot.current.filter.enabled = source.snapshot.next.filter.enabled = true;
      source.snapshot.current.overlay.type = source.snapshot.next.overlay.type = core::OverlayType::Disk;
      source.policy = test::application::diskPersistentPolicy();
      source.policy.resetPersistentState.value = resetClear ? 0 : 1;
      source.snapshot.runtime.hormEnabled = source.snapshot.runtime.fullCommit = false;
      source.snapshot.runtime.romDisposition = 0;
      source.snapshot.runtime.currentOverlayFlags = source.policy.current.flags.value->raw;
      source.snapshot.runtime.systemVolumeProtected = true;
    }
    source.snapshot.current.overlay.maximumSizeMb = source.snapshot.next.overlay.maximumSizeMb = 2048;
    source.snapshot.current.overlay.warningThresholdMb = source.snapshot.next.overlay.warningThresholdMb = 1536;
    source.snapshot.current.overlay.criticalThresholdMb = source.snapshot.next.overlay.criticalThresholdMb = 1900;
    core::PendingChanges changes;
    if (!nativeMaintenance && !resetReview && scenario != QLatin1String("empty") && !scenario.startsWith(QLatin1String("staging-")) &&
        !scenario.startsWith(QLatin1String("bulk-"))) {
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
    app::ConfigurationChanges all{changes, {}};
    if (resetReview) all.overlayPolicy.reset = resetClear ? app::PersistentReset::Clear : app::PersistentReset::Preserve;
    std::optional<app::OverlayMaintenanceSnapshot> maintenanceFacts;
    if (nativeMaintenance) {
      if (hormDisable || hormEnable) all.maintenance.hormEnabled = hormEnable;
      if (fullCommit) all.maintenance.fullCommit = true;
      if (storageChange) all.maintenance.storageVolume = QString::fromLatin1(test::application::fixtureVolumeDName);
      maintenanceFacts.emplace();
      maintenanceFacts->filterEnabled = source.snapshot.current.filter.enabled;
      maintenanceFacts->hormEnabled = hormDisable;
      maintenanceFacts->overlayCommit = false;
      maintenanceFacts->systemVolumeProtected = source.snapshot.runtime.systemVolumeProtected;
      maintenanceFacts->currentFlags = maintenanceFacts->nextFlags = app::OverlayFlags{storageChange ? 0U : 4U};
      maintenanceFacts->currentOverlayType = maintenanceFacts->nextOverlayType = 0;
      maintenanceFacts->maximumOverlaySize = 2048;
      maintenanceFacts->romDisposition = hormDisable ? 2U : 0U;
      maintenanceFacts->hibernationAvailable = true;
      maintenanceFacts->allVolumesProtected = std::ranges::all_of(source.snapshot.current.volumes, [](const auto& volume) { return volume.isProtected; });
      maintenanceFacts->volumesReadable = true;
      maintenanceFacts->hormCapability.support = maintenanceFacts->fullCommitCapability.support = maintenanceFacts->swapCapability.support =
          app::OverlayMaintenanceSupport::Supported;
      maintenanceFacts->volumes.push_back({QString::fromLatin1(test::application::fixtureVolumeDName), QStringLiteral("D:"), {}, 608ULL << 30, true, {}, {}});
    }
    if (scenario.startsWith(QLatin1String("bulk-"))) {
      all.maintenance.allVolumesProtected = scenario == QLatin1String("bulk-protect");
      source.snapshot.next.volumes.push_back({"Volume{aaaaaaaa-bbbb-cccc-dddd-111111111111}", {}, true, core::VolumeBinding::VolumeId});
      maintenanceFacts.emplace();
      maintenanceFacts->systemVolumeProtected = source.snapshot.runtime.systemVolumeProtected;
      maintenanceFacts->protectAllCapability.support = maintenanceFacts->unprotectAllCapability.support = app::OverlayMaintenanceSupport::Supported;
      maintenanceFacts->unprotectScopeConfirmed = true;
      for (const auto& volume : source.snapshot.next.volumes) maintenanceFacts->unprotectTargetVolumes.push_back(QString::fromStdString(volume.volumeName));
    }
    if (scenario.startsWith(QLatin1String("overlay-"))) {
      all.overlayPolicy.passthrough = false;
      if (scenario == QLatin1String("overlay-clear")) all.overlayPolicy.reset = app::PersistentReset::Clear;
      if (scenario == QLatin1String("overlay-cancel")) {
        source.policy.resetPersistentState.value = 1;
        all.overlayPolicy.reset = app::PersistentReset::Preserve;
      }
    }
    seedPreviewSystem(previewUwfState({source.volumes, source.snapshot, source.policy}, system->systemDriveLetter().toStdString()));
    auto model = factory->applyPlan(all, source.snapshot, source.policy, std::move(draft), std::move(maintenanceFacts));
    if (applyProgress) {
      auto actions = std::make_shared<PreviewApplyActions>();
      actions->workerDelayMs = scenario == QLatin1String("apply-long") ? 5000 : 0;
      actions->fail = scenario == QLatin1String("apply-failure");
      model = std::make_shared<presentation::ApplyPlanViewModel>(changes, source.snapshot, std::move(actions));
    }
    reviewTimings.insert(QStringLiteral("modelMs"), static_cast<double>(reviewTimer.nsecsElapsed()) / 1000000.0);
    reviewTimer.restart();
    reviewModel = model;
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
    report.system.windows = system->windowsVersion();
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
    auto model = std::make_shared<presentation::OverlayFilesViewModel>(
        QStringLiteral("C:"), QStringLiteral("11111111-2222-3333-4444-555555555555"),
        std::make_shared<app::OverlayFilesQueryManager>([scenario, docs](const QString&, std::stop_token) -> QVector<app::OverlayFileEntry> {
          if (scenario == QLatin1String("failure"))
            throw app::OperationError(std::make_error_code(std::errc::no_such_device), "read overlay files\n0x80010105\npreview provider failure");
          if (scenario == QLatin1String("empty")) return {};
          return {{QStringLiteral("\\ProgramData\\MyApp\\settings.json"), {}, false, false, 2048},
                  {QStringLiteral("\\Users\\User\\AppData\\Roaming\\Code\\User\\settings.json"), {}, false, false, 4096},
                  {docs ? QStringLiteral("\\Users\\User\\Documents\\Reports\\monthly-report.docx") : QStringLiteral("\\Users\\User\\Documents\\工作记录.docx"),
                   {},
                   false,
                   false,
                   285184},
                  {QStringLiteral("\\ProgramData\\MyApp\\Data"), {}, true, false, 0},
                  {QStringLiteral("\\$Mft"), {}, false, true, 25165824},
                  {QStringLiteral("\\Windows\\Temp\\session.log"), {}, false, false, 16384}};
        }),
        [](const QString&, const QString&) { return app::TextFileWriteResult{}; });
    const bool writable = source.snapshot.uwfAvailable && source.snapshot.elevated && source.snapshot.current.filter.enabled;
    model->setCommitAvailability(
        writable, !source.snapshot.uwfAvailable ? ui::I18n::tr("UWF is currently unavailable")
                  : !source.snapshot.elevated   ? ui::I18n::tr("Read-only")
                  : !source.snapshot.current.filter.enabled
                      ? ui::I18n::tr("The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.")
                      : QString{});
    panel = std::make_unique<ui::OverlayFilesDialog>(std::move(model), fileDialogs);
    auto* overlay = static_cast<ui::OverlayFilesDialog*>(panel.get());
    QObject::connect(overlay, &ui::OverlayFilesDialog::commitFileRequested, overlay, [uwfSystem, overlay](const QString& path) {
      app::CommitWorkflow workflow(app::uwfMaintenanceRepository(uwfSystem), ui::commitInteraction(overlay, {}, uwf::test::localFileDialogs()),
                                   windows::systemCommitTraversalSource());
      workflow.commitFilePath(path);
    });
  }
  QWidget* root = component            ? component.get()
                  : productionTrayMenu ? productionTrayMenu
                  : menu               ? static_cast<QWidget*>(menu.get())
                  : panel              ? static_cast<QWidget*>(panel.get())
                                       : &window;
  if (arguments.contains(QStringLiteral("--solid"))) root->setProperty("systemBackdropForceSolid", true);
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
      QFile timings(output + QStringLiteral(".timings.json"));
      if (timings.open(QIODevice::WriteOnly)) timings.write(QJsonDocument(reviewTimings).toJson());
    });
  }
  const auto size = value(QStringLiteral("--size"), QStringLiteral("1200x798")).split(u'x');
  if (mainPage && size.size() == 2) window.resize(size[0].toInt(), size[1].toInt());
  if (panel && arguments.contains(QStringLiteral("--size")) && size.size() == 2) root->resize(size[0].toInt(), size[1].toInt());
  const auto tab = value(QStringLiteral("--tab"));
  const auto selectBusinessTab = [&] {
    if (tab.isEmpty()) return;
    for (auto groups = window.findChildren<QTabWidget*>(QStringLiteral("innerTabs")); auto* tabs : std::as_const(groups))
      if (tabs->isVisibleTo(&window)) tabs->setCurrentIndex(tab == QStringLiteral("registry") ? 1 : tab == QStringLiteral("staging") ? 2 : 0);
  };
  selectBusinessTab();
  const auto stagingTabEvidence = [&] {
    QJsonObject evidence;
    for (auto groups = window.findChildren<QTabWidget*>(QStringLiteral("innerTabs")); auto* tabs : std::as_const(groups)) {
      if (!tabs->isVisibleTo(&window) || tabs->count() < 3) continue;
      evidence.insert(QStringLiteral("tooltip"), tabs->tabToolTip(2));
      evidence.insert(QStringLiteral("indicator"), tabs->tabBar()->property("designTabIndicators").toMap().value(QStringLiteral("2")).toString());
      evidence.insert(QStringLiteral("currentIndex"), tabs->currentIndex());
      break;
    }
    return evidence;
  };
  QJsonObject stagingRecoveryEvidence;
  if (stagingScenario && scenario == QLatin1String("staging-restored"))
    QTimer::singleShot(250, &application, [&] {
      selectBusinessTab();
      stagingRecoveryEvidence.insert(QStringLiteral("before"), stagingTabEvidence());
      auto restored = uwfSystem->snapshot();
      restored.current.overlayFlags = restored.next.overlayFlags = 0;
      restored.current.overlayType = restored.next.overlayType = 0;
      seedPreviewSystem(std::move(restored));
      window.refresh();
      stagingRecoveryEvidence.insert(QStringLiteral("refreshRequested"), true);
    });
  QCursor::setPos(application.primaryScreen()->availableGeometry().topLeft());
  QWidget* target = nullptr;
  const auto control = value(QStringLiteral("--control"));
  const auto controlState = value(QStringLiteral("--control-state"), QStringLiteral("rest"));
  bool captureComplete = false;
  int captureResult = 1;
  if (applyProgress)
    QTimer::singleShot(100, &application, [&] {
      auto* apply = panel->findChild<QPushButton*>(QStringLiteral("primaryBtn"));
      if (!apply || !apply->isEnabled()) {
        application.exit(9);
        return;
      }
      apply->click();
      auto* confirmation = panel->findChild<QDialog*>(QStringLiteral("applyExecutionConfirmation"));
      if (!confirmation) {
        application.exit(9);
        return;
      }
      confirmation->accept();
    });
  const bool waitForLoad =
      power ? scenario != QStringLiteral("preparing")
            : (page == QStringLiteral("logs") || page == QStringLiteral("overlay") || page == QLatin1String("picker")) && scenario != QStringLiteral("loading");
  const int captureDelay = value(QStringLiteral("--capture-delay"), scenario == QLatin1String("staging-restored") ? QStringLiteral("1100")
                                                                    : waitForLoad                                 ? QStringLiteral("1300")
                                                                                                                  : QStringLiteral("200"))
                               .toInt();
  QTimer::singleShot(captureDelay, &application, [&] {
    if (blocking || applyProgress) root = QApplication::activeModalWidget();
    if (!root) return;
    if (arguments.contains(QStringLiteral("--protect-all-draft"))) {
      if (auto* protect = root->findChild<QPushButton*>(QStringLiteral("hormProtectAll")); protect && protect->isEnabled()) protect->click();
    }
    if (mainPage && arguments.contains(QStringLiteral("--volume-index")))
      if (auto* volumes = window.findChild<QTabWidget*>(QStringLiteral("mainTabs"))) volumes->setCurrentIndex(value(QStringLiteral("--volume-index")).toInt());
    if (mainPage && scenario == QLatin1String("tab-hover-leave")) {
      if (auto* volumes = window.findChild<QTabWidget*>(QStringLiteral("mainTabs")); volumes && volumes->count() > 1) {
        auto* bar = volumes->tabBar();
        const auto position = bar->tabRect(1).center();
        volumes->setCurrentIndex(1);
        const auto global = bar->mapToGlobal(position);
        QCursor::setPos(global);
        QEnterEvent enter(position, bar->mapTo(bar->window(), position), global);
        QApplication::sendEvent(bar, &enter);
        QHoverEvent hovered(QEvent::HoverMove, position, global, {}, Qt::NoModifier);
        QApplication::sendEvent(bar, &hovered);
        QTimer::singleShot(60, bar, [bar, volumes] {
          volumes->setCurrentIndex(0);
          QCursor::setPos(bar->mapToGlobal(QPoint(-20, -20)));
          QEvent leave(QEvent::Leave);
          QApplication::sendEvent(bar, &leave);
          QHoverEvent left(QEvent::HoverLeave, QPointF(-1, -1), QCursor::pos(), {}, Qt::NoModifier);
          QApplication::sendEvent(bar, &left);
        });
      }
    }
    if (page == QLatin1String("review") && arguments.contains(QStringLiteral("--command-preview")))
      if (auto* tabs = root->findChild<QTabWidget*>(QStringLiteral("reviewTabs"))) tabs->setCurrentIndex(1);
    if ((!mainPage || docs) && arguments.contains(QStringLiteral("--size")) && size.size() == 2) root->resize(size[0].toInt(), size[1].toInt());
    if (docs) {
      const auto area = root->screen()->availableGeometry();
      root->move(page == QLatin1String("overview") ? area.bottomRight() - QPoint(root->width() - 1, root->height() - 1)
                                                   : area.center() - QPoint(root->width() / 2, root->height() / 2));
      root->raise();
      root->activateWindow();
      SetForegroundWindow(reinterpret_cast<HWND>(root->winId()));
      if (visibleCapture) {
        if (documentBackground)
          SetWindowPos(reinterpret_cast<HWND>(documentBackground->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        SetWindowPos(reinterpret_cast<HWND>(root->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      }
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
    for (auto disclosures = root->findChildren<ui::Disclosure*>(); auto* disclosure : std::as_const(disclosures))
      if (expand == QLatin1String("all") || (!expand.isEmpty() && disclosure->objectName().contains(expand, Qt::CaseInsensitive)) ||
          (page == QLatin1String("review") &&
           (scenario == QLatin1String("horm-disable") || scenario == QLatin1String("horm-enable") || scenario == QLatin1String("full-commit") ||
            scenario == QLatin1String("storage-change") || scenario == QLatin1String("reset-clear") || scenario == QLatin1String("reset-preserve")) &&
           disclosure->objectName() == QLatin1String("changeCommandsDisclosure")))
        disclosure->setOpen(true);
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
        for (auto bars = root->findChildren<QScrollBar*>(); auto* bar : std::as_const(bars))
          if (bar->isVisibleTo(root)) bar->setValue(bar->maximum());
      });
    }
    if (page == QStringLiteral("logs")) {
      if (auto* pages = root->findChild<presentation::PageProxyModel*>()) pages->goFirst();
      if (auto* table = root->findChild<QTableView*>(); table && table->model()->rowCount() > 5) table->selectRow(5);
    }
    if (!value(QStringLiteral("--select-row")).isEmpty()) {
      for (auto lists = window.findChildren<QListView*>(); auto* list : std::as_const(lists))
        if (dynamic_cast<ui::PathEntryListView*>(list) && list->isVisibleTo(&window)) {
          list->setCurrentIndex(list->model()->index(value(QStringLiteral("--select-row")).toInt(), 0));
          break;
        }
    }
    if (control.isEmpty()) return;
    for (auto candidates = root->findChildren<QWidget*>(control); auto* candidate : std::as_const(candidates))
      if (candidate->isVisibleTo(root)) {
        target = candidate;
        break;
      }
    if (!target && control == QLatin1String("captionMaximize"))
      for (auto buttons = root->findChildren<QAbstractButton*>(); auto* button : std::as_const(buttons))
        if (button->property("nativeCaptionHit").toInt() == HTMAXBUTTON) {
          target = button;
          break;
        }
    if (!target && control == QLatin1String("languageMenu"))
      for (auto buttons = root->findChildren<QToolButton*>(); auto* button : std::as_const(buttons))
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
        captureResult = popup->grab().save(output) ? 0 : 1;
        captureComplete = true;
        QJsonArray commands;
        for (auto actions = popup->actions(); const auto* action : std::as_const(actions))
          if (!action->isSeparator())
            commands.append(QJsonObject{{"text", action->text()}, {"icon", !action->icon().isNull()}, {"enabled", action->isEnabled()}});
        QFile commandFile(output + QStringLiteral(".json"));
        if (commandFile.open(QIODevice::WriteOnly)) commandFile.write(QJsonDocument(commands).toJson());
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
      else if (auto* plainText = qobject_cast<QPlainTextEdit*>(target))
        plainText->selectAll();
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
    if (blocking || aboutFeedback || applyProgress) root = QApplication::activeModalWidget();
    if (!root) {
      application.exit(4);
      return;
    }
    if (!visibleCapture && (docs || arguments.contains(QStringLiteral("--require-foreground"))) &&
        GetForegroundWindow() != reinterpret_cast<HWND>(root->winId())) {
      if (docs && reviewTimer.elapsed() > 10000) {
        application.exit(7);
        return;
      }
      captureTimer.start(250);
      return;
    }
    if (!visibleCapture && (docs || arguments.contains(QStringLiteral("--require-foreground"))) && !foregroundSettled) {
      foregroundSettled = true;
      QCursor::setPos(target && controlState == QLatin1String("hover") ? target->mapToGlobal(target->rect().center())
                                                                       : root->screen()->availableGeometry().topLeft());
      captureTimer.start(value(QStringLiteral("--settle-delay"), QStringLiteral("250")).toInt());
      return;
    }
    const auto metrics = value(QStringLiteral("--metrics"));
    if (!metrics.isEmpty()) {
      QJsonArray widgets;
      for (auto children = root->findChildren<QWidget*>(); const auto* widget : std::as_const(children)) {
        if (!widget->isVisibleTo(root)) continue;
        const auto position = widget->mapTo(root, QPoint());
        QJsonObject record{{"class", QString::fromLatin1(widget->metaObject()->className())},
                           {"name", widget->objectName()},
                           {"enabled", widget->isEnabled()},
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
        if (const auto* editor = qobject_cast<const QTextEdit*>(widget)) record["text"] = editor->toPlainText();
        if (const auto* button = qobject_cast<const QAbstractButton*>(widget)) record["checked"] = button->isChecked();
        if (const auto* tabs = qobject_cast<const QTabWidget*>(widget)) {
          record["currentIndex"] = tabs->currentIndex();
          QJsonArray tabFacts;
          for (int index = 0; index < tabs->count(); ++index)
            tabFacts.append(QJsonObject{{"text", tabs->tabText(index)},
                                        {"tooltip", tabs->tabToolTip(index)},
                                        {"indicator", tabs->tabBar()->property("designTabIndicators").toMap().value(QString::number(index)).toString()}});
          record["tabs"] = tabFacts;
        }
        if (const auto* view = qobject_cast<const QAbstractItemView*>(widget); view && view->model()) {
          QJsonArray rows;
          for (int row = 0; row < std::min(50, view->model()->rowCount()); ++row) {
            const auto index = view->model()->index(row, 0);
            rows.append(QJsonObject{{"text", index.data(Qt::DisplayRole).toString()},
                                    {"status", index.data(presentation::StatusTextRole).toString()},
                                    {"tooltip", index.data(Qt::ToolTipRole).toString()}});
          }
          record["rows"] = rows;
        }
        if (const auto* scroll = qobject_cast<const QScrollBar*>(widget)) record["range"] = scroll->maximum() - scroll->minimum();
        widgets.append(record);
      }
      if (!stagingRecoveryEvidence.isEmpty()) {
        stagingRecoveryEvidence.insert(QStringLiteral("after"), stagingTabEvidence());
        stagingRecoveryEvidence.insert(QStringLiteral("class"), QStringLiteral("PreviewEvidence"));
        stagingRecoveryEvidence.insert(QStringLiteral("name"), QStringLiteral("stagingRecoveryEvidence"));
        widgets.append(stagingRecoveryEvidence);
      }
      if (reviewModel) {
        using Scope = app::ConfigurationScriptScope;
        const auto* tabs = panel->findChild<QTabWidget*>(QStringLiteral("reviewTabs"));
        const bool next = tabs && tabs->currentIndex() == 1;
        const auto scope = next ? Scope::NextConfiguration : Scope::PendingChanges;
        QJsonArray commands;
        for (const auto& command : reviewModel->exportCommands(scope)) commands.append(QString::fromStdString(app::renderUwfmgrCommand(command)));
        auto* editor = panel->findChild<QTextEdit*>(next ? QStringLiteral("configurationCommandPreview") : QStringLiteral("changeCommandPreview"));
        bool exportEnabled = false;
        const auto exportButtons = panel->findChildren<QPushButton*>();
        for (const auto* button : std::as_const(exportButtons))
          if (button->text() == ui::I18n::tr("Export commands")) exportEnabled = button->isEnabled();
        widgets.append(QJsonObject{{"class", "PreviewEvidence"},
                                   {"name", "reviewExportEvidence"},
                                   {"scope", next ? "NextConfiguration" : "PendingChanges"},
                                   {"commands", commands},
                                   {"commandCount", static_cast<int>(commands.size())},
                                   {"pendingCommandCount", static_cast<int>(reviewModel->changeCommandCount())},
                                   {"exportEnabled", exportEnabled},
                                   {"modelExportEnabled", reviewModel->canExport(scope)},
                                   {"restriction", reviewModel->exportRestrictionText(scope)},
                                   {"previewText", editor ? editor->toPlainText() : QString{}}});
      }
      QDir().mkpath(QFileInfo(metrics).absolutePath());
      QFile metricsFile(metrics);
      if (metricsFile.open(QIODevice::WriteOnly)) metricsFile.write(QJsonDocument(widgets).toJson());
    }
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
      QFile materialFile(output + QStringLiteral(".material.json"));
      if (materialFile.open(QIODevice::WriteOnly)) materialFile.write(QJsonDocument(material).toJson());
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
        for (auto widgets = QApplication::allWidgets(); const auto* widget : std::as_const(widgets)) {
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
      QFile screenFile(output + QStringLiteral(".screen.json"));
      if (screenFile.open(QIODevice::WriteOnly)) screenFile.write(QJsonDocument(environment).toJson());
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
  const int progressAge =
      applyProgress ? value(QStringLiteral("--progress-age"), scenario == QLatin1String("apply-long") ? QStringLiteral("1800") : QStringLiteral("400")).toInt()
                    : 0;
  captureTimer.start(std::max({waitForLoad ? 1500 : 500, captureDelay + 250, progressAge + 100}));
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
                                                   QStringLiteral("—"), ui::I18n::tr("Committed to disk"), true, false, QStringLiteral("settings.json")},
                                                  {presentation::CommitRowStatus::Failed, QStringLiteral("C:\\ProgramData\\MyApp\\Data\\cache.db"),
                                                   QStringLiteral("0x80070005"), ui::I18n::tr("Access denied"), true, true, QStringLiteral("cache.db")}};
        if (scenario == QLatin1String("details")) {
          rows = {{presentation::CommitRowStatus::Failed,
                   QStringLiteral("C:\\Users\\User\\Documents\\项目资料\\年度归档\\设备配置与运行记录\\"
                                  "需要在重启前保存的现场诊断数据与说明\\设备配置备份与现场工作记录.json"),
                   QStringLiteral("0x80070020"),
                   QStringLiteral("文件被其他进程占用，无法提交到磁盘。\n请关闭正在编辑此文件的程序后重试。\n"
                                  "失败详情保留完整路径与错误原因，可选择文本或复制当前记录。"),
                   true, true, QStringLiteral("设备配置备份与现场工作记录.json")}};
          for (int index = 1; index < 12; ++index)
            rows.append({presentation::CommitRowStatus::Succeeded,
                         QStringLiteral("C:\\ProgramData\\MyApp\\Data\\配置记录-%1.json").arg(index, 3, 10, QChar('0')), QStringLiteral("—"),
                         ui::I18n::tr("Committed to disk"), true, false, QStringLiteral("配置记录-%1.json").arg(index, 3, 10, QChar('0'))});
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
      if (scenario.startsWith(QLatin1String("skip-")) || scenario == QLatin1String("mixed-retained")) {
        const auto summary = previewAutomaticCommitSummary(scenario);
        request.pollStagingPreparation = [summary]() -> std::optional<app::PowerStagingPreparation> {
          if (!summary.commitRequired) return app::PowerStagingNotRequired{summary};
          return app::PowerStagingCommit{24,
                                         [summary] {
                                           app::PowerStagingProgress progress;
                                           progress.processedFiles = progress.totalFiles = 24;
                                           progress.automaticCommit = summary;
                                           return progress;
                                         },
                                         summary};
        };
      } else if (scenario.startsWith(QLatin1String("failure"))) {
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
        request.pollStagingPreparation = [result = std::move(result)]() -> std::optional<app::PowerStagingPreparation> {
          return app::PowerStagingFailure{presentation::renderFileStagingResult(result), result};
        };
      } else if (scenario == QLatin1String("infrastructure-failure")) {
        request.pollStagingPreparation = []() -> std::optional<app::PowerStagingPreparation> {
          return app::PowerStagingFailure{
              QStringLiteral("无法读取文件暂存列表。\n原始错误：拒绝访问 · 0x80070005\nRegistry staging store could not be opened.")};
        };
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
