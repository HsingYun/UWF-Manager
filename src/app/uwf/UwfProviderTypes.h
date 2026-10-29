// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// UWF WMI 类的值类型映射；每行保留 __PATH 以精确定位实例。
// 不依赖 Qt，聚合后的会话视图由 core::UwfSnapshot 表达。

#include <cstdint>
#include <string>
#include <vector>

namespace uwf::app::provider {

enum class Session { Current, Next };

enum class VolumeBinding { DriveLetter, VolumeName };

struct RegistryPersistence {
  bool domainSecretKey = false;
  bool terminalServicesClientAccessLicense = false;
};

// UWF_OverlayConfig.Type / UWF_Overlay.SetType 的枚举。
enum class OverlayType : uint32_t {
  RAM = 0,   // 基于 RAM 的覆盖层
  Disk = 1,  // 基于磁盘的覆盖层
};

// UWF_ExcludedFile — UWF_Volume.GetExclusions 返回的 EmbeddedInstance。
struct ExcludedFile {
  std::string fileName;  // 相对于卷的完整路径
};

// UWF_ExcludedRegistryKey — UWF_RegistryFilter.GetExclusions 返回的
// EmbeddedInstance。
struct ExcludedRegistryKey {
  std::string registryKey;  // 注册表项的完整路径
};

// UWF_OverlayFile — UWF_Overlay.GetOverlayFiles 返回的 EmbeddedInstance。
struct OverlayFileRow {
  std::string fileName;  // 文件名（相对卷的路径）
  uint64_t fileSize = 0;
};

// UWF_Filter —— 全局筛选器单例。
struct FilterRow {
  std::string path;  // __PATH；写方法定位此实例用
  bool currentEnabled = false;
  bool nextEnabled = false;
};

// UWF_Overlay —— 覆盖层运行时状态单例。
struct OverlayRow {
  std::string path;
  uint32_t overlayConsumption = 0;
  uint32_t availableSpace = 0;
  uint32_t criticalOverlayThreshold = 0;
  uint32_t warningOverlayThreshold = 0;
};

// UWF_OverlayConfig —— 按 CurrentSession 存在 2 个实例。
struct OverlayConfigRow {
  std::string path;
  bool currentSession = false;
  OverlayType type = OverlayType::RAM;
  uint32_t maximumSize = 0;
};

// UWF_RegistryFilter —— 按 CurrentSession 存在 2 个实例。
struct RegistryFilterRow {
  std::string path;
  bool currentSession = false;
  bool persistDomainSecretKey = false;
  bool persistTSCAL = false;
};

// UWF_Volume —— 每个受保护卷在每个 Session 下各有一行（key 三元组：
// CurrentSession + DriveLetter + VolumeName）。
struct VolumeRow {
  std::string path;
  bool currentSession = false;
  std::string driveLetter;  // 规范化的 "C:"；无盘符 provider 行不会进入应用模型
  std::string volumeName;   // UWF schema 的裸 "Volume{GUID}" 形式
  bool bindByDriveLetter = true;
  bool commitPending = false;  // 保留供 Microsoft 使用
  bool isProtected = false;    // WMI 上字段名为 "Protected"
};

// 按会话和附加谓词查找实例；找不到返回 nullptr。卷实例须提供盘符等附加条件。
template <typename Row, typename Pred>
[[nodiscard]] const Row* findBySession(const std::vector<Row>& rows, const Session session, Pred extra) {
  const bool wantCurrent = session == Session::Current;
  for (const auto& r : rows) {
    if (r.currentSession == wantCurrent && extra(r)) return &r;
  }
  return nullptr;
}

template <typename Row>
[[nodiscard]] const Row* findBySession(const std::vector<Row>& rows, const Session session) {
  return findBySession(rows, session, [](const Row&) { return true; });
}

}  // namespace uwf::app::provider
