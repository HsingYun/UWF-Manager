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
#include <cstdint>
#include <format>
#include <map>
#include <set>
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
#include "wmi/WmiError.h"
#include "wmi/WmiException.h"
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

std::string readNullableString(const WmiRow& row, const char* field) {
  const auto it = row.find(field);
  if (it == row.end()) throw WmiDecodeError("decode disk inventory", std::format("field '{}' is missing", field));
  return it->second.isValid() ? rowutil::requireString(row, field) : std::string{};
}

uint64_t readNullableUInt64(const WmiRow& row, const char* field) {
  const auto it = row.find(field);
  if (it == row.end()) throw WmiDecodeError("decode disk inventory", std::format("field '{}' is missing", field));
  return it->second.isValid() ? rowutil::requireUInt64(row, field) : 0;
}

}  // namespace

UwfCapability probeUwfCapability(WmiOperations& session) {
  try {
    session.ensureConnected();
    return session.classStatus("UWF_Filter") == WmiClassStatus::Present ? UwfCapability::Available : UwfCapability::Unavailable;
  } catch (const WmiInfrastructureError& error) {
    // 未安装任何 Embedded 功能时 namespace 本身也可能不存在；这是与类缺失
    // 等价的、已被 provider 明确确认的能力缺失，不是可重试的连接故障。
    if (error.code().category() == wmiErrorCategory() && WmiError(static_cast<std::int32_t>(error.code().value())).code() == WmiErrorCode::InvalidNamespace) {
      return UwfCapability::Unavailable;
    }
    throw;
  }
}

UwfCapability probeUwfCapability() { return probeUwfCapability(embeddedWmiSession()); }

core::UwfSnapshot readSnapshot(WmiOperations& s, const UwfCapability capability, const bool elevated) {
  core::UwfSnapshot snap;
  // 提权状态随快照交给所有 UI 消费者；UWF 能力则由启动期固定后注入，刷新
  // 只读取动态状态，不能因一次 WMI 故障重新解释运行环境。
  snap.elevated = elevated;
  if (capability == UwfCapability::Unavailable) {
    snap.uwfAvailable = false;
    snap.unavailableReason = "UWF is not registered";
    return snap;
  }

  s.ensureConnected();

  // ── UWF_Filter ───────────────────────────────────────────────
  const auto filter = api::UwfFilter{s}.read();
  snap.current.filter.enabled = filter.currentEnabled;
  snap.next.filter.enabled = filter.nextEnabled;

  // ── UWF_Overlay：runtime + 共享的阈值 ────────────────────────
  // 注意：GetOverlayFiles 很慢（会扫整卷），这里故意不调用；
  // snap.overlayFiles 保持为空，由调用方按需触发 UwfOverlay::getOverlayFiles()。
  uint32_t warningMb = 0;
  uint32_t criticalMb = 0;
  const auto overlay = api::UwfOverlay{s}.read();
  snap.runtime.currentConsumptionMb = overlay.overlayConsumption;
  snap.runtime.availableSpaceMb = overlay.availableSpace;
  warningMb = overlay.warningOverlayThreshold;
  criticalMb = overlay.criticalOverlayThreshold;

  // ── UWF_OverlayConfig：current/next 拆开 ─────────────────────
  const auto overlayConfigs = api::UwfOverlayConfig{s}.readAll();
  bool hasCurrentOverlayConfig = false;
  bool hasNextOverlayConfig = false;
  for (const auto& c : overlayConfigs) {
    (c.currentSession ? hasCurrentOverlayConfig : hasNextOverlayConfig) = true;
    (c.currentSession ? snap.current.overlay : snap.next.overlay) = toOverlayConfig(c, warningMb, criticalMb);
  }
  if (!hasCurrentOverlayConfig || !hasNextOverlayConfig) throw WmiProtocolError("read UWF_OverlayConfig", "required current/next rows are missing");

  // ── UWF_Volume：按 CurrentSession 归入 current/next ──────────
  // 注意：fileExclusions 的 key 必须用 volumeName（与 DiskTab::applySnapshot
  // 保持一致），否则 UI 按 volumeName 查询会拿到空列表。
  //
  // 性能：对 *当前会话* 中 isProtected=false 的卷跳过 GetExclusions——UWF
  // 对未保护卷在当前会话不暂存任何写入，运行时排除列表必然为空。但 *下次
  // 会话* 行总是要读：用户完全可以在卷尚未受保护时就为下次会话预先添加
  // 排除项，这时 isProtected=false 但 GetExclusions 会有内容。
  const api::UwfVolume volumes{s};
  const auto volumeRows = volumes.readAll();
  for (const auto& v : volumeRows) {
    auto& session = v.currentSession ? snap.current : snap.next;
    session.volumes.push_back(toVolumeRecord(v));

    if (v.volumeName.empty()) continue;
    if (v.currentSession && !v.isProtected) continue;

    auto& bucket = session.fileExclusions[v.volumeName];
    const auto exclusions = volumes.getExclusions(v);
    for (const auto& e : exclusions) {
      if (e.fileName.empty()) continue;
      bucket.push_back(prefixDriveLetter(e.fileName, v.driveLetter));
    }
  }

  // ── UWF_RegistryFilter：current/next 各自的排除列表 + 两个持久化开关 ──
  const api::UwfRegistryFilter rf{s};
  const auto registryRows = rf.readAll();
  bool hasCurrentRegistryFilter = false;
  bool hasNextRegistryFilter = false;
  for (const auto& r : registryRows) {
    (r.currentSession ? hasCurrentRegistryFilter : hasNextRegistryFilter) = true;
    auto& session = r.currentSession ? snap.current : snap.next;
    session.persistDomainSecretKey = r.persistDomainSecretKey;
    session.persistTSCAL = r.persistTSCAL;
    const auto exclusions = rf.getExclusions(r);
    for (const auto& [registryKey] : exclusions) {
      if (!registryKey.empty()) session.registryExclusions.push_back(registryKey);
    }
  }
  if (!hasCurrentRegistryFilter || !hasNextRegistryFilter) throw WmiProtocolError("read UWF_RegistryFilter", "required current/next rows are missing");

  snap.uwfAvailable = true;
  core::sortSnapshot(snap);
  return snap;
}

core::UwfSnapshot readSnapshot(const UwfCapability capability) { return readSnapshot(embeddedWmiSession(), capability, isElevated()); }

std::vector<core::DiskInfo> enumerateDisks(WmiOperations& cim) {
  std::vector<core::DiskInfo> out;
  cim.ensureConnected();

  const auto rows =
      cim.query("SELECT DeviceID, FileSystem, VolumeName, Size, FreeSpace, DriveType FROM Win32_LogicalDisk WHERE DriveType = 2 OR DriveType = 3");

  // Win32_Volume.DeviceID 形如 "\\?\Volume{GUID}\"，与 UWF_Volume 使用的
  // 裸 "Volume{GUID}" 格式不同。这里仅按 DriveLetter 给磁盘信息补充可显示的
  // GUID；UWF 的实例键和排除列表仍始终使用 provider 返回的裸 VolumeName。
  const auto volRows = cim.query("SELECT DeviceID, DriveLetter FROM Win32_Volume");
  std::map<std::string, std::string> driveToGuid;
  for (const auto& v : volRows) {
    const auto driveIt = v.find("DriveLetter");
    if (driveIt == v.end()) {
      throw WmiDecodeError("decode Win32_Volume", "field 'DriveLetter' is missing");
    }
    if (!driveIt->second.isValid()) continue;  // 无盘符卷是合法对象，但不参与逻辑盘映射
    const auto driveLetter = rowutil::requireString(v, "DriveLetter", rowutil::EmptyString::Reject);
    const auto deviceId = rowutil::requireString(v, "DeviceID", rowutil::EmptyString::Reject);
    const auto normalizedDrive = drive::normalize(driveLetter);
    if (normalizedDrive.empty()) {
      throw WmiDecodeError("decode Win32_Volume", std::format("invalid DriveLetter: {}", driveLetter));
    }
    if (!driveToGuid.emplace(normalizedDrive, deviceId).second) {
      throw WmiProtocolError("decode Win32_Volume", std::format("duplicate volume mapping for {}", normalizedDrive));
    }
  }

  std::set<std::string> logicalDrives;
  for (const auto& r : rows) {
    const auto deviceId = rowutil::requireString(r, "DeviceID", rowutil::EmptyString::Reject);
    const auto driveType = rowutil::requireInt(r, "DriveType");

    core::DiskInfo d;
    d.driveLetter = drive::normalize(deviceId);
    if (d.driveLetter.empty()) {
      throw WmiDecodeError("decode Win32_LogicalDisk", std::format("invalid DeviceID: {}", deviceId));
    }
    if (!logicalDrives.emplace(d.driveLetter).second) {
      throw WmiProtocolError("decode Win32_LogicalDisk", std::format("duplicate logical disk row for {}", d.driveLetter));
    }
    d.fileSystem = readNullableString(r, "FileSystem");
    d.label = readNullableString(r, "VolumeName");
    d.totalBytes = readNullableUInt64(r, "Size");
    d.freeBytes = readNullableUInt64(r, "FreeSpace");
    if (auto it = driveToGuid.find(d.driveLetter); it != driveToGuid.end()) {
      d.volumeName = it->second;
    }

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

std::vector<core::DiskInfo> enumerateDisks() { return enumerateDisks(cimv2WmiSession()); }

}  // namespace uwf
