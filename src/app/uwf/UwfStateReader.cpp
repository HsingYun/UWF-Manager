// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UwfStateReader.h"

#include <algorithm>
#include <cstdint>
#include <format>
#include <map>
#include <set>
#include <utility>

#include "app/ports/VolumePaths.h"
#include "app/uwf/UwfFilter.h"
#include "app/uwf/UwfOverlay.h"
#include "app/uwf/UwfOverlayConfig.h"
#include "app/uwf/UwfRegistryFilter.h"
#include "app/uwf/UwfVolume.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "app/wmi/WmiRowUtil.h"
#include "core/UwfConstraints.h"
#include "shared/StringUtil.h"

namespace uwf::app {

using core::VolumeSupport;

namespace {

core::OverlayConfig toOverlayConfig(const app::provider::OverlayConfigRow& cfg, const uint32_t warningMb, const uint32_t criticalMb) {
  core::OverlayConfig o;
  o.type = cfg.type == app::provider::OverlayType::Disk ? core::OverlayType::Disk : core::OverlayType::RAM;
  o.maximumSizeMb = cfg.maximumSize;
  o.warningThresholdMb = warningMb;
  o.criticalThresholdMb = criticalMb;
  return o;
}

core::VolumeRecord toVolumeRecord(const app::provider::VolumeRow& v) {
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

std::string readNullableString(const wmi::WmiRow& row, const char* field) {
  const auto it = row.find(field);
  if (it == row.end()) throw wmi::WmiDecodeError("decode disk inventory", std::format("field '{}' is missing", field));
  return it->second.isValid() ? wmi::row::requireString(row, field) : std::string{};
}

uint64_t readNullableUInt64(const wmi::WmiRow& row, const char* field) {
  const auto it = row.find(field);
  if (it == row.end()) throw wmi::WmiDecodeError("decode disk inventory", std::format("field '{}' is missing", field));
  return it->second.isValid() ? wmi::row::requireUInt64(row, field) : 0;
}

}  // namespace

UwfCapability probeUwfCapability(wmi::WmiOperations& session) {
  try {
    session.ensureConnected();
    return session.classStatus("UWF_Filter") == wmi::WmiClassStatus::Present ? UwfCapability::Available : UwfCapability::Unavailable;
  } catch (const wmi::WmiInfrastructureError& error) {
    // 未安装任何 Embedded 功能时 namespace 本身也可能不存在；这是与类缺失
    // 等价的、已被 provider 明确确认的能力缺失，不是可重试的连接故障。
    if (error.code().category() == wmi::wmiErrorCategory() &&
        wmi::WmiError(static_cast<std::int32_t>(error.code().value())).code() == wmi::WmiErrorCode::InvalidNamespace) {
      return UwfCapability::Unavailable;
    }
    throw;
  }
}

core::UwfSnapshot readSnapshot(wmi::WmiOperations& s, const UwfCapability capability, const bool elevated, drive::MountLookup mounts) {
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

  const auto filter = app::provider::UwfFilter{s}.read();
  snap.current.filter.enabled = filter.currentEnabled;
  snap.next.filter.enabled = filter.nextEnabled;

  // 覆盖层文件枚举开销较大，由调用方按需加载，不进入常规快照。
  uint32_t warningMb = 0;
  uint32_t criticalMb = 0;
  const auto overlay = app::provider::UwfOverlay{s}.read();
  snap.runtime.currentConsumptionMb = overlay.overlayConsumption;
  snap.runtime.availableSpaceMb = overlay.availableSpace;
  warningMb = overlay.warningOverlayThreshold;
  criticalMb = overlay.criticalOverlayThreshold;

  const auto overlayConfigs = app::provider::UwfOverlayConfig{s}.readAll();
  bool hasCurrentOverlayConfig = false;
  bool hasNextOverlayConfig = false;
  for (const auto& c : overlayConfigs) {
    (c.currentSession ? hasCurrentOverlayConfig : hasNextOverlayConfig) = true;
    (c.currentSession ? snap.current.overlay : snap.next.overlay) = toOverlayConfig(c, warningMb, criticalMb);
  }
  if (!hasCurrentOverlayConfig || !hasNextOverlayConfig) throw wmi::WmiProtocolError("read UWF_OverlayConfig", "required current/next rows are missing");

  // 排除项按 volumeName 索引。当前未保护卷可跳过读取；下一会话即使未保护，也可能已配置排除项。
  const app::provider::UwfVolume volumes{s, std::move(mounts)};
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

  const app::provider::UwfRegistryFilter rf{s};
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
  if (!hasCurrentRegistryFilter || !hasNextRegistryFilter) throw wmi::WmiProtocolError("read UWF_RegistryFilter", "required current/next rows are missing");

  snap.uwfAvailable = true;
  core::sortSnapshot(snap);
  return snap;
}

std::vector<core::VolumeInfo> enumerateVolumes(wmi::WmiOperations& cim) {
  std::vector<core::VolumeInfo> out;
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
      throw wmi::WmiDecodeError("decode Win32_Volume", "field 'DriveLetter' is missing");
    }
    if (!driveIt->second.isValid()) continue;  // 无盘符卷是合法对象，但不参与逻辑盘映射
    const auto driveLetter = wmi::row::requireString(v, "DriveLetter", wmi::row::EmptyString::Reject);
    const auto deviceId = wmi::row::requireString(v, "DeviceID", wmi::row::EmptyString::Reject);
    const auto normalizedDrive = drive::normalize(driveLetter);
    if (normalizedDrive.empty()) {
      throw wmi::WmiDecodeError("decode Win32_Volume", std::format("invalid DriveLetter: {}", driveLetter));
    }
    if (!driveToGuid.emplace(normalizedDrive, deviceId).second) {
      throw wmi::WmiProtocolError("decode Win32_Volume", std::format("duplicate volume mapping for {}", normalizedDrive));
    }
  }

  std::set<std::string> logicalDrives;
  for (const auto& r : rows) {
    const auto deviceId = wmi::row::requireString(r, "DeviceID", wmi::row::EmptyString::Reject);
    const auto driveType = wmi::row::requireInt(r, "DriveType");

    core::VolumeInfo d;
    d.driveLetter = drive::normalize(deviceId);
    if (d.driveLetter.empty()) {
      throw wmi::WmiDecodeError("decode Win32_LogicalDisk", std::format("invalid DeviceID: {}", deviceId));
    }
    if (!logicalDrives.emplace(d.driveLetter).second) {
      throw wmi::WmiProtocolError("decode Win32_LogicalDisk", std::format("duplicate logical disk row for {}", d.driveLetter));
    }
    d.fileSystem = readNullableString(r, "FileSystem");
    d.label = readNullableString(r, "VolumeName");
    d.totalBytes = readNullableUInt64(r, "Size");
    d.freeBytes = readNullableUInt64(r, "FreeSpace");
    if (auto it = driveToGuid.find(d.driveLetter); it != driveToGuid.end()) {
      d.volumeName = it->second;
    }

    const std::string fs = toUpperAscii(d.fileSystem);
    const bool fullySupportedFs = std::ranges::find(core::kFullySupportedFileSystems, fs) != core::kFullySupportedFileSystems.end();
    if (driveType != core::kDriveTypeFixedLocalDisk) {
      d.support = VolumeSupport::NotFixedLocalDisk;
    } else if (d.totalBytes > core::kMaxProtectedVolumeBytes) {
      // 容量超限优先于文件系统限制判定，不能通过更换文件系统恢复支持。
      d.support = VolumeSupport::ExceedsMaxSize;
    } else if (!fullySupportedFs) {
      // 受限文件系统可保护卷，但不支持文件排除或文件提交。
      d.support = VolumeSupport::FileSystemLimited;
    } else {
      d.support = VolumeSupport::Supported;
    }
    out.push_back(std::move(d));
  }
  return out;
}

}  // namespace uwf::app
