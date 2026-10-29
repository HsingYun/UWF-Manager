// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 不依赖 Qt 的 UWF 领域模型。Current/Next 区分当前与重启后会话；*Mb 字段单位与 WMI 一致。

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace uwf::core {

// 覆盖层（overlay）的存放介质类型。对应 UWF_Overlay.Type /
// UWF_OverlayConfig.Type。
enum class OverlayType : int {
  Unknown = -1,  // 读取失败或未初始化
  RAM = 0,       // 覆盖层放在内存中
  Disk = 1,      // 覆盖层放在磁盘上
};

// 筛选器总开关的状态，对应 UWF_Filter.CurrentEnabled / NextEnabled。
struct FilterState {
  bool enabled = false;
};

// 覆盖层配置汇总：当前值由 UWF_Overlay 及 current OverlayConfig 读取，下一会话值由 next OverlayConfig 读取。
// 阈值为 0 时只关闭对应事件；两个阈值均非零时，警告值必须小于严重值。
struct OverlayConfig {
  OverlayType type = OverlayType::Unknown;
  uint32_t maximumSizeMb = 0;        // 覆盖层最大容量
  uint32_t warningThresholdMb = 0;   // 警告阈值
  uint32_t criticalThresholdMb = 0;  // 严重阈值
};

// 覆盖层配置的"字段级 delta"：每个字段单独 optional，nullopt 表示"不动"。
// 用在 PendingChanges 中，避免一次改动连带把另外 3 个字段一起重写：
//   - type / maximumSizeMb：UWF_OverlayConfig::SetType / SetMaximumSize；
//     这两项还要求 UWF_Filter.CurrentEnabled==false，否则 WMI 会报
//     WBEM_E_INVALID_PARAMETER（0x80041008）。
//   - warningThresholdMb / criticalThresholdMb：UWF_Overlay::SetWarningThreshold
//     / SetCriticalThreshold，无 session 区分，也无需先禁用筛选器。
struct OverlayConfigDelta {
  std::optional<OverlayType> type;
  std::optional<uint32_t> maximumSizeMb;
  std::optional<uint32_t> warningThresholdMb;
  std::optional<uint32_t> criticalThresholdMb;

  [[nodiscard]] bool empty() const { return !type && !maximumSizeMb && !warningThresholdMb && !criticalThresholdMb; }
  // 是否触及需要"先禁用 UWF"才能调的两个字段。
  [[nodiscard]] bool touchesOverlayConfig() const { return type.has_value() || maximumSizeMb.has_value(); }
};

// 覆盖层的"运行时状态"，对应 UWF_Overlay.AvailableSpace 等非配置属性。
// 这类字段只读，不属于 OverlayConfig。
struct OverlayRuntime {
  uint32_t availableSpaceMb = 0;      // 剩余可用
  uint32_t currentConsumptionMb = 0;  // 已占用
};

enum class OverlayUsageSeverity { Normal, Warning, Critical };

[[nodiscard]] constexpr uint64_t overlayTotalMb(const OverlayRuntime& runtime) {
  return static_cast<uint64_t>(runtime.currentConsumptionMb) + runtime.availableSpaceMb;
}
[[nodiscard]] constexpr OverlayUsageSeverity overlayUsageSeverity(uint32_t usedMb, uint32_t warningMb, uint32_t criticalMb) {
  return criticalMb && usedMb >= criticalMb ? OverlayUsageSeverity::Critical
         : warningMb && usedMb >= warningMb ? OverlayUsageSeverity::Warning
                                            : OverlayUsageSeverity::Normal;
}

// 单卷状态；isProtected 对应 UWF_Volume.Protected。
struct VolumeRecord {
  std::string volumeName;   // UWF_Volume 的裸 "Volume{GUID}" 形式
  std::string driveLetter;  // 归一化后的盘符，如 "C:"
  bool isProtected = false;
  bool bindByDriveLetter = true;
};

// 一次"会话"的完整快照：filter + overlay + 各卷 + 各项排除。
// current 对应当前运行中的会话，next 对应保存在
// registry、下一次重启后生效的会话。
struct SessionSnapshot {
  FilterState filter;
  OverlayConfig overlay;
  std::vector<VolumeRecord> volumes;
  std::map<std::string, std::vector<std::string>> fileExclusions;  // key: UWF volumeName（如 Volume{GUID}）
  std::vector<std::string> registryExclusions;                     // 注册表排除是全局的，不按卷分
  // UWF_RegistryFilter 的两个全局开关：是否在覆盖层中持久化域机密密钥
  // 与终端服务客户端访问许可证（TSCAL）。
  bool persistDomainSecretKey = false;
  bool persistTSCAL = false;
};

// 一次"读取 UWF 状态"的总结果。
struct UwfSnapshot {
  SessionSnapshot current;
  SessionSnapshot next;
  OverlayRuntime runtime;
  bool uwfAvailable = false;      // UWF 能力已注册，且本快照完整可用
  bool elevated = false;          // 当前进程是否以管理员身份运行
  std::string unavailableReason;  // UWF 能力不可用的业务原因（UTF-8）
};

// 磁盘支持等级；界面层负责本地化说明。
enum class VolumeSupport : int {
  Supported,          // NTFS / FAT(32)：UWF 完全支持（含文件排除 / commit）
  NotFixedLocalDisk,  // 不是固定本地磁盘（CD、软盘、网络盘等）
  ExceedsMaxSize,     // 容量超过 UWF 单卷上限（16 TiB，见 core::kMaxProtectedVolumeBytes）
  FileSystemLimited,  // exFAT / ReFS 等：可保护卷，但不能加文件排除 / 提交文件
  QueryFailed,        // 调用 Win32 API 失败
};

[[nodiscard]] constexpr bool supportsVolumeProtection(const VolumeSupport support) noexcept {
  return support == VolumeSupport::Supported || support == VolumeSupport::FileSystemLimited;
}

[[nodiscard]] constexpr bool supportsFileOverlayOperations(const VolumeSupport support) noexcept { return support == VolumeSupport::Supported; }

// 本机上枚举出的一个磁盘卷的只读信息。
struct VolumeInfo {
  std::string driveLetter;  // 归一化盘符
  std::string volumeName;
  std::string fileSystem;
  std::string label;
  uint64_t totalBytes = 0;
  uint64_t freeBytes = 0;
  VolumeSupport support = VolumeSupport::Supported;
};

void sortExclusions(std::vector<std::string>& items);
void sortSnapshot(UwfSnapshot& snapshot);

// 用户在 UI 上累积、但还没真正提交到 WMI 的变更集合。
// 只包含"和基线不同"的部分；optional 为 std::nullopt 表示"不动"。
struct PendingChanges {
  std::optional<bool> setFilterEnabled;
  OverlayConfigDelta setOverlay;
  std::map<std::string, bool> volumeProtect;  // key: 盘符
  // true=按卷 ID 绑定（bBindByVolumeName=true / bindByDriveLetter=false）；
  // false=按盘符绑定。
  std::map<std::string, bool> volumeBindByVolumeName;                    // key: 盘符
  std::map<std::string, std::vector<std::string>> addFileExclusions;     // key: 盘符
  std::map<std::string, std::vector<std::string>> removeFileExclusions;  // key: 盘符
  std::vector<std::string> addRegistryExclusions;
  std::vector<std::string> removeRegistryExclusions;
  // UWF_RegistryFilter 的两个全局持久化开关；nullopt 表示不动。
  std::optional<bool> setPersistDomainSecretKey;
  std::optional<bool> setPersistTSCAL;

  [[nodiscard]] bool empty() const;
  // 配置变更按字段或单条增删计数，供状态栏和预览共用。
  [[nodiscard]] std::size_t count() const;
  void clear();
};

}  // namespace uwf::core
