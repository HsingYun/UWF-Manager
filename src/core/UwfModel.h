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
#pragma once

// UWF 核心数据结构定义
//
// 本文件只包含 UWF（Unified Write Filter）的"纯数据"类型，
// 不依赖任何 Qt 类。所有 WMI 查询结果最终都应映射到这里定义的类型；
// UI 层只负责把这些结构渲染为字符串。
//
// 命名规范：
//   - Current* / Next* 对应 UWF 的当前会话与下一次启动后生效的会话；
//   - *Mb 结尾的数值字段单位均为 MB，与 WMI Schema 保持一致；
//   - bindByDriveLetter == true 表示按盘符绑定，false 表示按卷 ID 绑定。

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

// 覆盖层的"配置项"，对应 UWF_OverlayConfig 四个可写属性与 UWF_Overlay 四个
// Current* 属性。 读（Reader）与写（Writer）共用同一种结构：
//   - 读取 UWF_Overlay 时得到当前运行值；
//   - 读取 UWF_OverlayConfig 时得到下一次启动生效的值；
//   - 写入时必须拆成 SetType / SetMaximumSize / SetWarningLevel /
//   SetCriticalLevel 四次调用。
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

// UWF_OverlayFile：当前被缓存在 overlay 里的一个文件条目。
// 由 UWF_Overlay::GetOverlayFiles() 读出。UI 层可以选择性地展示。
struct OverlayFileInfo {
  std::string fileName;
  uint64_t fileSizeBytes = 0;
};

// 单个卷的保护状态。对应 UWF_Volume 的一行。
// isProtected 对应 CurrentSession / NextSession（布尔值，true
// 表示该卷处于保护状态）。
struct VolumeRecord {
  std::string volumeName;   // 设备路径，例如 "\\?\Volume{...}"
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
  std::map<std::string, std::vector<std::string>> fileExclusions;  // key: volumeName（如 \\?\Volume{GUID}\）
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
  std::vector<OverlayFileInfo> overlayFiles;  // UWF_Overlay::GetOverlayFiles() 的结果
  bool uwfAvailable = false;                  // UWF 命名空间是否可用
  bool elevated = false;                      // 当前进程是否以管理员身份运行
  std::string rawError;                       // 读取过程中的错误描述（UTF-8）
};

// 一个磁盘卷是否"能被 UWF 保护"的判定结果。
// UI 根据这个枚举显示对应文案，而不是把中文理由从读取层里带出来。
enum class DiskSupport : int {
  Supported,          // NTFS / FAT(32)：UWF 完全支持（含文件排除 / commit）
  NotFixedLocalDisk,  // 不是固定本地磁盘（CD、软盘、网络盘等）
  ExceedsMaxSize,     // 容量超过 UWF 单卷上限（16 TiB，见 config::kMaxProtectedVolumeBytes）
  FileSystemLimited,  // exFAT / ReFS 等：可保护卷，但不能加文件排除 / 提交文件
  QueryFailed,        // 调用 Win32 API 失败
};

// 本机上枚举出的一个磁盘卷的只读信息。
struct DiskInfo {
  std::string driveLetter;  // 归一化盘符
  std::string volumeName;
  std::string fileSystem;
  std::string label;
  uint64_t totalBytes = 0;
  uint64_t freeBytes = 0;
  DiskSupport support = DiskSupport::Supported;
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
  // 待变更条目数：filter 1 + overlay 每个被设字段各 1 + 每个卷的 protect/bind
  // 各 1 + 每条文件/注册表增删各 1 + 每个持久化开关 1。状态栏摘要与预览标题
  // 的数量都用它，保证两处计数口径一致。
  [[nodiscard]] std::size_t count() const;
  void clear();
};

}  // namespace uwf::core
