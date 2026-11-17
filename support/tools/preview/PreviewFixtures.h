// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QThread>
#include <algorithm>
#include <stdexcept>

#include "application/commit/CommitPlanner.h"
#include "application/configuration/contracts/ConfigurationActions.h"
#include "application/diagnostics/contracts/LogSource.h"
#include "application/diagnostics/contracts/SystemInformationSource.h"
#include "application/enhanced/contracts/EnhancedModeActions.h"
#include "application/files/contracts/FileBrowser.h"
#include "application/paths/PathPolicy.h"
#include "application/registry/contracts/RegistryBrowser.h"
#include "application/staging/policy/AutomaticCommitPolicy.h"
#include "application/state/contracts/ApplicationStateSource.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "application/volumes/contracts/VolumeRepository.h"
#include "foundation/paths/DriveLetter.h"
#include "simulation/app/ConfigurationState.h"
#include "simulation/app/EnhancedMode.h"
#include "simulation/app/OverlayPolicy.h"
#include "simulation/uwf/FakeUwfBackend.h"

namespace uwf::preview {
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
inline uwf::simulation::FakeUwfBackend::State previewUwfState(const app::ApplicationState& source, const std::string& systemDriveLetter) {
  const auto& snapshot = source.snapshot;
  const auto& policy = source.overlayPolicy;
  const auto systemDrive = drive::normalize(systemDriveLetter);
  const auto systemVolume = std::ranges::find_if(
      source.volumes, [&](const core::VolumeInfo& volume) { return !systemDrive.empty() && drive::normalize(volume.driveLetter) == systemDrive; });
  if (systemVolume == source.volumes.end() || systemVolume->volumeName.empty())
    throw std::invalid_argument("The preview system volume is missing from the physical inventory");
  uwf::simulation::FakeUwfBackend::State state;
  const auto session = [](const core::SessionSnapshot& observed, const app::OverlayPolicySession& options) {
    uwf::simulation::FakeUwfBackend::Configuration target;
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
      uwf::simulation::FakeUwfBackend::VolumeConfiguration configured{
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
      result.append({app::paths::joinFilePath(path, name), name, directory, directory ? 0 : 2048, modified});
    }
    return result;
  }
  app::FileBrowserEntry inspect(const QString& path) const override { return {path, app::paths::fileName(path), !path.contains(u'.'), 0, {}, true}; }
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
  QMap<QString, QList<app::CommitNode>> children;
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override {
    auto result = target;
    if (target.kind == app::CommitTargetKind::RegistryKey || target.kind == app::CommitTargetKind::RegistryValue) return result;
    if (!result.fileTarget) result.fileTarget = resolveFileTarget(result.path);
    return result;
  }
  bool exists(const app::CommitNode&) const override { return true; }
  std::string driveLetterForPath(const QString& path) const override { return drive::normalize(path.left(2).toStdString()); }
  app::FileCommitTarget resolveFileTarget(const QString& path) const override {
    const auto root = driveLetterForPath(path);
    const auto volume = root == "C:"   ? uwf::simulation::application::fixtureVolumeCName
                        : root == "D:" ? uwf::simulation::application::fixtureVolumeDName
                                       : std::string{};
    if (volume.empty()) throw std::runtime_error("The preview path has no observed volume binding");
    auto relative = path.mid(2);
    relative.replace(u'/', u'\\');
    return {volume, relative};
  }
  app::CommitEnumerationError enumerateChildren(const app::CommitNode& target, std::stop_token stop, const app::CommitNodeVisitor& discover) const override {
    for (const auto& child : children.value(target.path)) {
      if (stop.stop_requested()) break;
      discover(child);
    }
    return {};
  }
};
class PreviewCommitBackend final : public app::CommitBackend {
 public:
  PreviewCommitBackend() {
    const app::CommitNode branch{app::CommitTargetKind::Folder, m_root.path + QStringLiteral("\\Branch"), {}, {}};
    const app::CommitNode leaf{app::CommitTargetKind::File, m_root.path + QStringLiteral("\\one.txt"), {}, {}};
    const app::CommitNode nested{app::CommitTargetKind::File, branch.path + QStringLiteral("\\two.txt"), {}, {}};
    m_source.children.insert(m_root.path, {branch, leaf});
    m_source.children.insert(branch.path, {nested});
  }
  app::CommitPlan prepare(const app::CommitRequest& request, app::CommitPlan previous, std::stop_token stop,
                          const app::CommitPreparationProgress& progress) override {
    return app::prepareCommit(request, m_source, std::move(previous), stop, progress);
  }
  app::CommitTargetResult execute(const app::CommitNode&, app::CommitOperation) override {
    app::CommitTargetResult result;
    result.succeeded = true;
    return result;
  }
  app::CommitRequest request(app::CommitOperation operation = app::CommitOperation::Persist) const { return {operation, m_root}; }

 private:
  app::CommitNode m_root{app::CommitTargetKind::Folder, QStringLiteral("C:\\Test"), {}, {}};
  PreviewCommitTraversalSource m_source;
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
  app::EnhancedModeStatus current = uwf::simulation::application::completeEnhancedModeStatus();
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

inline app::AutomaticCommitSummary previewAutomaticCommitSummary(const QString& scenario) {
  app::AutomaticCommitContext context{uwf::simulation::application::diskPersistentPolicy(), false, 0, false};
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
}  // namespace uwf::preview
