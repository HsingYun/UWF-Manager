/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "UwfSnapshot.h"

#include <algorithm>
#include <format>
#include <map>
#include <utility>

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
  o.maximumSizeMb = cfg.maximumSize;
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

bool readNullableString(const WmiRow& row, const char* field, std::string& value, std::string* error) {
  const auto it = row.find(field);
  if (it == row.end()) {
    if (error) *error = std::format("WMI field '{}' is missing", field);
    return false;
  }
  value.clear();
  if (!it->second.isValid()) return true;
  const auto decoded = rowutil::requireString(row, field, error);
  if (!decoded) return false;
  value = *decoded;
  return true;
}

bool readNullableUInt64(const WmiRow& row, const char* field, uint64_t& value, std::string* error) {
  const auto it = row.find(field);
  if (it == row.end()) {
    if (error) *error = std::format("WMI field '{}' is missing", field);
    return false;
  }
  value = 0;
  if (!it->second.isValid()) return true;
  const auto decoded = rowutil::requireUInt64(row, field, error);
  if (!decoded) return false;
  value = *decoded;
  return true;
}

}  // namespace

core::UwfSnapshot readSnapshot(std::string* error) {
  core::UwfSnapshot snap;
  if (error) error->clear();
  const auto fail = [&](const char* component, std::string detail) {
    if (detail.empty()) detail = "no data returned";
    snap.uwfAvailable = false;
    snap.rawError = std::format("{}: {}", component, detail);
    if (error) *error = snap.rawError;
    return snap;
  };
  // 提权状态和 uwfAvailable 一样是"本次会话固定"的事实，一并装进快照，
  // 让拿到快照的 UI 各自按需判断（二者用途不同，不合并成单一标志）。
  snap.elevated = isElevated();
  auto& s = embeddedWmiSession();
  std::string err;
  if (!s.ensureConnected(&err)) {
    snap.uwfAvailable = false;
    snap.rawError = err;
    if (error) *error = err;
    return snap;
  }

  // root\standardcimv2\embedded 命名空间由多个 Windows 锁定功能共用（键盘
  // 筛选器 WEKF_* / Shell Launcher 等），连得上不代表 UWF 已安装。探测
  // UWF_Filter 类是否注册——UWF 的全部类由同一份 MOF 一次性注册，查一个
  // 有代表性的类即可判定整体可用性。不存在和探测失败严格区分，后者保留
  // 原始错误，不能用“假定存在”继续执行。
  err.clear();
  const auto filterClass = s.classStatus("UWF_Filter", &err);
  if (filterClass == WmiClassStatus::Unknown) return fail("UWF_Filter class probe", std::move(err));
  if (filterClass == WmiClassStatus::Missing) {
    snap.uwfAvailable = false;
    snap.rawError = "UWF is not registered";
    if (error) *error = snap.rawError;
    return snap;
  }

  // ── UWF_Filter ───────────────────────────────────────────────
  err.clear();
  const auto filter = api::UwfFilter{s}.read(&err);
  if (!filter) return fail("UWF_Filter", std::move(err));
  snap.current.filter.enabled = filter->currentEnabled;
  snap.next.filter.enabled = filter->nextEnabled;

  // ── UWF_Overlay：runtime + 共享的阈值 ────────────────────────
  // 注意：GetOverlayFiles 很慢（会扫整卷），这里故意不调用；
  // snap.overlayFiles 保持为空，由调用方按需触发 UwfOverlay::getOverlayFiles()。
  uint32_t warningMb = 0;
  uint32_t criticalMb = 0;
  err.clear();
  const auto overlay = api::UwfOverlay{s}.read(&err);
  if (!overlay) return fail("UWF_Overlay", std::move(err));
  snap.runtime.currentConsumptionMb = overlay->overlayConsumption;
  snap.runtime.availableSpaceMb = overlay->availableSpace;
  warningMb = overlay->warningOverlayThreshold;
  criticalMb = overlay->criticalOverlayThreshold;

  // ── UWF_OverlayConfig：current/next 拆开 ─────────────────────
  err.clear();
  const auto overlayConfigs = api::UwfOverlayConfig{s}.readAll(&err);
  if (!err.empty()) return fail("UWF_OverlayConfig", std::move(err));
  bool hasCurrentOverlayConfig = false;
  bool hasNextOverlayConfig = false;
  for (const auto& c : overlayConfigs) {
    (c.currentSession ? hasCurrentOverlayConfig : hasNextOverlayConfig) = true;
    (c.currentSession ? snap.current.overlay : snap.next.overlay) = toOverlayConfig(c, warningMb, criticalMb);
  }
  if (!hasCurrentOverlayConfig || !hasNextOverlayConfig) return fail("UWF_OverlayConfig", "required current/next rows are missing");

  // ── UWF_Volume：按 CurrentSession 归入 current/next ──────────
  // 注意：fileExclusions 的 key 必须用 volumeName（与 DiskTab::applySnapshot
  // 保持一致），否则 UI 按 volumeName 查询会拿到空列表。
  //
  // 性能：对 *当前会话* 中 isProtected=false 的卷跳过 GetExclusions——UWF
  // 对未保护卷在当前会话不暂存任何写入，运行时排除列表必然为空。但 *下次
  // 会话* 行总是要读：用户完全可以在卷尚未受保护时就为下次会话预先添加
  // 排除项，这时 isProtected=false 但 GetExclusions 会有内容。
  const api::UwfVolume volumes{s};
  err.clear();
  const auto volumeRows = volumes.readAll(&err);
  if (!err.empty()) return fail("UWF_Volume", std::move(err));
  for (const auto& v : volumeRows) {
    auto& session = v.currentSession ? snap.current : snap.next;
    session.volumes.push_back(toVolumeRecord(v));

    if (v.volumeName.empty()) continue;
    if (v.currentSession && !v.isProtected) continue;

    auto& bucket = session.fileExclusions[v.volumeName];
    err.clear();
    const auto exclusions = volumes.getExclusions(v, &err);
    if (!exclusions) return fail("UWF_Volume::GetExclusions", std::move(err));
    for (const auto& e : *exclusions) {
      if (e.fileName.empty()) continue;
      bucket.push_back(prefixDriveLetter(e.fileName, v.driveLetter));
    }
  }

  // ── UWF_RegistryFilter：current/next 各自的排除列表 + 两个持久化开关 ──
  const api::UwfRegistryFilter rf{s};
  err.clear();
  const auto registryRows = rf.readAll(&err);
  if (!err.empty()) return fail("UWF_RegistryFilter", std::move(err));
  bool hasCurrentRegistryFilter = false;
  bool hasNextRegistryFilter = false;
  for (const auto& r : registryRows) {
    (r.currentSession ? hasCurrentRegistryFilter : hasNextRegistryFilter) = true;
    auto& session = r.currentSession ? snap.current : snap.next;
    session.persistDomainSecretKey = r.persistDomainSecretKey;
    session.persistTSCAL = r.persistTSCAL;
    err.clear();
    const auto exclusions = rf.getExclusions(r, &err);
    if (!exclusions) return fail("UWF_RegistryFilter::GetExclusions", std::move(err));
    for (const auto& [registryKey] : *exclusions) {
      if (!registryKey.empty()) session.registryExclusions.push_back(registryKey);
    }
  }
  if (!hasCurrentRegistryFilter || !hasNextRegistryFilter) return fail("UWF_RegistryFilter", "required current/next rows are missing");

  snap.uwfAvailable = true;
  core::sortSnapshot(snap);
  return snap;
}

std::vector<core::DiskInfo> enumerateDisks(std::string* error) {
  std::vector<core::DiskInfo> out;
  if (error) error->clear();
  auto& cim = cimv2WmiSession();
  if (!cim.ensureConnected(error)) return out;

  std::string err;
  const auto rows =
      cim.query("SELECT DeviceID, FileSystem, VolumeName, Size, FreeSpace, DriveType FROM Win32_LogicalDisk WHERE DriveType = 2 OR DriveType = 3", &err);
  if (!err.empty()) {
    if (error) *error = err;
    return out;
  }

  // Win32_Volume.DeviceID 形如 "\\?\Volume{GUID}\"，与 UWF_Volume 使用的
  // 裸 "Volume{GUID}" 格式不同。这里仅按 DriveLetter 给磁盘信息补充可显示的
  // GUID；UWF 的实例键和排除列表仍始终使用 provider 返回的裸 VolumeName。
  std::string volErr;
  const auto volRows = cim.query("SELECT DeviceID, DriveLetter FROM Win32_Volume", &volErr);
  if (!volErr.empty()) {
    if (error) *error = volErr;
    return out;
  }
  std::map<std::string, std::string> driveToGuid;
  for (const auto& v : volRows) {
    const auto driveIt = v.find("DriveLetter");
    if (driveIt == v.end()) {
      if (error) *error = "Win32_Volume row is missing DriveLetter";
      return {};
    }
    if (!driveIt->second.isValid()) continue;  // 无盘符卷是合法对象，但不参与逻辑盘映射
    const auto driveLetter = rowutil::requireString(v, "DriveLetter", error, false);
    const auto deviceId = rowutil::requireString(v, "DeviceID", error, false);
    if (!driveLetter || !deviceId) return {};
    const auto normalizedDrive = drive::normalize(*driveLetter);
    if (normalizedDrive.empty()) {
      if (error) *error = std::format("Win32_Volume returned an invalid DriveLetter: {}", *driveLetter);
      return {};
    }
    driveToGuid[normalizedDrive] = *deviceId;
  }

  for (const auto& r : rows) {
    const auto deviceId = rowutil::requireString(r, "DeviceID", error, false);
    const auto driveType = rowutil::requireInt(r, "DriveType", error);
    if (!deviceId || !driveType) return {};

    std::string fileSystem;
    std::string label;
    uint64_t totalBytes = 0;
    uint64_t freeBytes = 0;
    if (!readNullableString(r, "FileSystem", fileSystem, error) || !readNullableString(r, "VolumeName", label, error) ||
        !readNullableUInt64(r, "Size", totalBytes, error) || !readNullableUInt64(r, "FreeSpace", freeBytes, error))
      return {};

    core::DiskInfo d;
    d.driveLetter = drive::normalize(*deviceId);
    if (d.driveLetter.empty()) {
      if (error) *error = std::format("Win32_LogicalDisk returned an invalid DeviceID: {}", *deviceId);
      return {};
    }
    d.fileSystem = std::move(fileSystem);
    d.label = std::move(label);
    d.totalBytes = totalBytes;
    d.freeBytes = freeBytes;
    if (auto it = driveToGuid.find(d.driveLetter); it != driveToGuid.end()) {
      d.volumeName = it->second;
    }

    const std::string fs = toUpperAscii(d.fileSystem);
    const bool fullySupportedFs = std::ranges::find(config::kFullySupportedFileSystems, fs) != config::kFullySupportedFileSystems.end();
    if (*driveType != config::kDriveTypeFixedLocalDisk) {
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
