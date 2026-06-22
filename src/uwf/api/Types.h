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

// ┌──────────────────────────────────────────────────────────────────────────┐
// │ uwf::api —— root\standardcimv2\embedded 命名空间里 UWF_* WMI 类的        │
// │ 1:1 值类型映射。字段名、类型、含义与 Microsoft 官方文档保持一致；        │
// │ 每个 *Row 结构对应 WMI 查询返回的一行，并额外保留该行的 __PATH，         │
// │ 方便在写操作时精确定位到该实例。                                         │
// │                                                                          │
// │ 本文件 **不依赖 Qt**；UI 层会在 core::UwfSnapshot 里把这些原始行聚合     │
// │ 成 "current + next" 视图。                                               │
// └──────────────────────────────────────────────────────────────────────────┘

#include <cstdint>
#include <string>
#include <vector>

namespace uwf::api {

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
  int32_t maximumSize = 0;  // 实机 schema 为 UInt32；MB 级数值远小于 INT_MAX
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
  std::string driveLetter;  // 可能为空（没有盘符的卷）
  std::string volumeName;   // \\?\Volume{GUID}\ 形式
  bool bindByDriveLetter = true;
  bool commitPending = false;  // 保留供 Microsoft 使用
  bool isProtected = false;    // WMI 上字段名为 "Protected"
};

// 按 CurrentSession 在 rows 里挑一行：wantCurrent=true → current 实例，
// =false → next 实例。extra 是可选的额外条件（例如按 DriveLetter 进一步过滤）。
// 没传 extra（默认 lambda 永真）时，单实例类如 OverlayConfigRow / RegistryFilterRow
// 直接靠 wantCurrent 唯一定位；按盘符的 VolumeRow 用 extra 二次过滤。
// 找不到返回 nullptr。
template <typename Row, typename Pred>
[[nodiscard]] const Row* findBySession(const std::vector<Row>& rows, bool wantCurrent, Pred extra) {
  for (const auto& r : rows) {
    if (r.currentSession == wantCurrent && extra(r)) return &r;
  }
  return nullptr;
}

template <typename Row>
[[nodiscard]] const Row* findBySession(const std::vector<Row>& rows, bool wantCurrent) {
  return findBySession(rows, wantCurrent, [](const Row&) { return true; });
}

}  // namespace uwf::api
