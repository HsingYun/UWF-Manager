#include "UwfSnapshot.h"

#include <algorithm>
#include <map>

#include "../core/Config.h"
#include "../util/DriveLetter.h"
#include "../util/StringUtil.h"
#include "SystemCheck.h"
#include "api/UwfFilter.h"
#include "api/UwfOverlay.h"
#include "api/UwfOverlayConfig.h"
#include "api/UwfRegistryFilter.h"
#include "api/UwfVolume.h"
#include "wmi/WmiClient.h"
#include "wmi/WmiRowUtil.h"

namespace uwf {

using core::DiskSupport;

namespace {

core::OverlayConfig toOverlayConfig(const api::OverlayConfigRow& cfg, const uint32_t warningMb, const uint32_t criticalMb) {
  core::OverlayConfig o;
  o.type = cfg.type == api::OverlayType::Disk ? core::OverlayType::Disk : core::OverlayType::RAM;
  o.maximumSizeMb = static_cast<uint32_t>(std::max(0, cfg.maximumSize));
  o.warningThresholdMb = warningMb;
  o.criticalThresholdMb = criticalMb;
  return o;
}

core::VolumeRecord toVolumeRecord(const api::VolumeRow& v) {
  core::VolumeRecord r;
  r.volumeName = v.volumeName;
  r.driveLetter = v.driveLetter;
  r.isProtected = v.isProtected;
  r.bindByDriveLetter = v.bindByDriveLetter;
  return r;
}

// 如果卷内相对路径以单反斜杠开头，补上盘符前缀，方便 UI 展示。
std::string prefixDriveLetter(std::string path, const std::string& drive) {
  if (drive.empty() || path.empty()) return path;
  const bool unc = path.size() >= 2 && path[0] == '\\' && path[1] == '\\';
  if (path.front() == '\\' && !unc) return drive + path;
  return path;
}

}  // namespace

core::UwfSnapshot readSnapshot(std::string* error) {
  core::UwfSnapshot snap;
  // 提权状态和 uwfAvailable 一样是"本次会话固定"的事实，一并装进快照，
  // 让拿到快照的 UI 各自按需判断（二者用途不同，不合并成单一标志）。
  snap.elevated = isElevated();
  WmiSession s;
  std::string err;
  if (!s.connect(config::kWmiNamespaceEmbedded, &err)) {
    snap.uwfAvailable = false;
    snap.rawError = err;
    if (error) *error = err;
    return snap;
  }

  // root\standardcimv2\embedded 命名空间由多个 Windows 锁定功能共用（键盘
  // 筛选器 WEKF_* / Shell Launcher 等），连得上不代表 UWF 已安装。探测
  // UWF_Filter 类是否注册——UWF 的全部类由同一份 MOF 一次性注册，查一个
  // 有代表性的类即可判定整体可用性。classExists 只在确认类缺失时返回 false，
  // 未提权（实例 access-denied 但类存在）不会被误判。
  if (!s.classExists("UWF_Filter")) {
    snap.uwfAvailable = false;
    snap.rawError = "UWF is not registered";
    if (error) *error = snap.rawError;
    return snap;
  }

  // ── UWF_Filter ───────────────────────────────────────────────
  if (auto f = api::UwfFilter{s}.read()) {
    snap.current.filter.enabled = f->currentEnabled;
    snap.next.filter.enabled = f->nextEnabled;
  }

  // ── UWF_Overlay：runtime + 共享的阈值 ────────────────────────
  // 注意：GetOverlayFiles 很慢（会扫整卷），这里故意不调用；
  // snap.overlayFiles 保持为空，由调用方按需触发 UwfOverlay::getOverlayFiles()。
  uint32_t warningMb = 0;
  uint32_t criticalMb = 0;
  if (auto o = api::UwfOverlay{s}.read()) {
    snap.runtime.currentConsumptionMb = o->overlayConsumption;
    snap.runtime.availableSpaceMb = o->availableSpace;
    warningMb = o->warningOverlayThreshold;
    criticalMb = o->criticalOverlayThreshold;
  }

  // ── UWF_OverlayConfig：current/next 拆开 ─────────────────────
  for (const auto& c : api::UwfOverlayConfig{s}.readAll()) {
    (c.currentSession ? snap.current.overlay : snap.next.overlay) = toOverlayConfig(c, warningMb, criticalMb);
  }

  // ── UWF_Volume：按 CurrentSession 归入 current/next ──────────
  // 注意：fileExclusions 的 key 必须用 volumeName（与 DiskTab::applySnapshot
  // 保持一致），否则 UI 按 volumeName 查询会拿到空列表。
  //
  // 性能：对 *当前会话* 中 isProtected=false 的卷跳过 GetExclusions——UWF
  // 对未保护卷在当前会话不暂存任何写入，运行时排除列表必然为空。但 *下次
  // 会话* 行总是要读：用户完全可以在卷尚未受保护时就为下次会话预先添加
  // 排除项，这时 isProtected=false 但 GetExclusions 会有内容。
  const api::UwfVolume volumes{s};
  for (const auto& v : volumes.readAll()) {
    auto& session = v.currentSession ? snap.current : snap.next;
    session.volumes.push_back(toVolumeRecord(v));

    if (v.volumeName.empty()) continue;
    if (v.currentSession && !v.isProtected) continue;

    auto& bucket = session.fileExclusions[v.volumeName];
    for (const auto& e : volumes.getExclusions(v)) {
      if (e.fileName.empty()) continue;
      bucket.push_back(prefixDriveLetter(e.fileName, v.driveLetter));
    }
  }

  // ── UWF_RegistryFilter：current/next 各自的排除列表 + 两个持久化开关 ──
  const api::UwfRegistryFilter rf{s};
  for (const auto& r : rf.readAll()) {
    auto& session = r.currentSession ? snap.current : snap.next;
    session.persistDomainSecretKey = r.persistDomainSecretKey;
    session.persistTSCAL = r.persistTSCAL;
    for (const auto& [registryKey] : rf.getExclusions(r)) {
      if (!registryKey.empty()) session.registryExclusions.push_back(registryKey);
    }
  }

  snap.uwfAvailable = true;
  core::sortSnapshot(snap);
  return snap;
}

std::vector<core::DiskInfo> enumerateDisks(std::string* error) {
  std::vector<core::DiskInfo> out;
  WmiSession cim;
  if (!cim.connect(config::kWmiNamespaceCimv2, error)) return out;

  std::string err;
  const auto rows =
      cim.query("SELECT DeviceID, FileSystem, VolumeName, Size, FreeSpace, DriveType FROM Win32_LogicalDisk WHERE DriveType = 2 OR DriveType = 3", &err);
  if (!err.empty() && error) *error = err;

  // Win32_Volume.DeviceID 形如 "\\?\Volume{GUID}\"，是 UWF_Volume
  // 里的 VolumeName；用 DriveLetter 关联到 Win32_LogicalDisk 的每一行。
  // 未注册到 UWF 的盘在 UWF_Volume 里查不到，只能从这里取 GUID。
  std::string volErr;
  const auto volRows = cim.query("SELECT DeviceID, DriveLetter FROM Win32_Volume", &volErr);
  std::map<std::string, std::string> driveToGuid;
  for (const auto& v : volRows) {
    const std::string dl = drive::normalize(rowutil::getString(v, "DriveLetter"));
    const std::string id = rowutil::getString(v, "DeviceID");
    if (!dl.empty() && !id.empty()) driveToGuid[dl] = id;
  }

  for (const auto& r : rows) {
    core::DiskInfo d;
    d.driveLetter = drive::normalize(rowutil::getString(r, "DeviceID"));
    d.fileSystem = rowutil::getString(r, "FileSystem");
    d.label = rowutil::getString(r, "VolumeName");
    d.totalBytes = r.value("Size").toULongLong();
    d.freeBytes = r.value("FreeSpace").toULongLong();
    if (auto it = driveToGuid.find(d.driveLetter); it != driveToGuid.end()) {
      d.volumeName = it->second;
    }

    const int driveType = rowutil::getInt(r, "DriveType");
    const std::string fs = toUpperAscii(d.fileSystem);
    const bool fullySupportedFs = std::ranges::find(config::kFullySupportedFileSystems, fs) != config::kFullySupportedFileSystems.end();
    if (driveType != config::kDriveTypeFixedLocalDisk) {
      d.support = DiskSupport::NotFixedLocalDisk;
    } else if (d.totalBytes > config::kMaxProtectedVolumeBytes) {
      // 超过 16 TiB——UWF 直接拒整个卷，比 FS 限制更彻底；FS 改成 NTFS 也救不回
      // 来，所以排在 FileSystemLimited 之前判。
      d.support = DiskSupport::ExceedsMaxSize;
    } else if (!fullySupportedFs) {
      // exFAT / ReFS 等：UWF 文档明确说"可保护卷，但不能加文件排除 /
      // 提交文件操作"——所以不是完全 unsupported，标 FileSystemLimited
      // 让 UI 仅禁用文件排除 + commit 部分，protect 开关保持可用。
      d.support = DiskSupport::FileSystemLimited;
    } else {
      d.support = DiskSupport::Supported;
    }
    out.push_back(std::move(d));
  }
  return out;
}

}  // namespace uwf
