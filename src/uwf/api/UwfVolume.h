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

// UWF_Volume —— 每个卷在每个 Session 下各一行（key 三元组：
// CurrentSession + DriveLetter + VolumeName）。
//
// 字段：BindByDriveLetter / CommitPending / Protected
// 方法：Protect / Unprotect / CommitFile / CommitFileDeletion /
//       SetBindByDriveLetter / AddExclusion / RemoveExclusion /
//       RemoveAllExclusions / FindExclusion / GetExclusions

#include <optional>
#include <string>
#include <vector>

#include "../wmi/WmiClient.h"
#include "../wmi/WmiResult.h"
#include "Types.h"

namespace uwf::api {

// ensureNextSessionEntry 同时可能执行 PutInstance。把条目与写入进度作为一个
// 领域结果返回，使批量应用流程能区分纯读取失败与“写请求已发出但确认失败”。
struct EnsureVolumeResult {
  std::optional<api::VolumeRow> entry;
  bool writeAttempted = false;
  bool writeConfirmed = false;
  std::string error;
};

class UwfVolume {
 public:
  explicit UwfVolume(WmiSession& session) : m_session(session) {}

  std::vector<api::VolumeRow> readAll(std::string* error = nullptr) const;

  [[nodiscard]] WmiResult protectVolume(const api::VolumeRow& row) const;
  [[nodiscard]] WmiResult unprotect(const api::VolumeRow& row) const;

  [[nodiscard]] WmiResult commitFile(const api::VolumeRow& row, const std::string& fileFullPath) const;

  // 语义：删除一个**当前仍存在**的受保护文件——CommitFileDeletion 由方法自身把
  // 该文件从覆盖层与物理卷一并删除，并非"提交一个已发生的删除"。因此调用方应先
  // 校验"该路径当前确实存在"（文件不存在时方法回 WBEM_E_NOT_FOUND，归 Skipped）。
  [[nodiscard]] WmiResult commitFileDeletion(const api::VolumeRow& row, const std::string& fileName) const;

  // 对应 UWF_Volume.SetBindByDriveLetter(boolean bBindByDriveLetter) 官方签名：
  // bBindByDriveLetter=true 表示按盘符绑定（松绑定），false 表示按卷名绑定（紧绑定）。
  [[nodiscard]] WmiResult setBindByDriveLetter(const api::VolumeRow& row, bool bBindByDriveLetter) const;

  [[nodiscard]] WmiResult addExclusion(const api::VolumeRow& row, const std::string& fileName) const;
  [[nodiscard]] WmiResult removeExclusion(const api::VolumeRow& row, const std::string& fileName) const;
  [[nodiscard]] WmiResult removeAllExclusions(const api::VolumeRow& row) const;

  std::optional<bool> findExclusion(const api::VolumeRow& row, const std::string& fileName, std::string* error = nullptr) const;

  std::optional<std::vector<api::ExcludedFile>> getExclusions(const api::VolumeRow& row, std::string* error = nullptr) const;

  // 拿到指定卷的 next session 实例。如果 UWF_Volume 里没有 next session
  // 行（卷从未被 protect 过、没加过 exclusion），就从该卷的 current session
  // 行复制 VolumeName，用 PutInstance 创建一份 next session 实例后返回。
  // 之所以从 current 复制：UWF 自己列出来的 current 实例 VolumeName 格式
  // 必定规范（"Volume{GUID}"，无 \\?\ 前缀、无尾斜杠），caller 不必再做
  // 跨命名空间查 Win32_Volume 然后归一化的脏活。
  [[nodiscard]] EnsureVolumeResult ensureNextSessionEntry(const std::string& driveLetter) const;

 private:
  // CommitFile / CommitFileDeletion 共享的实现——除方法名外两者完全一致。
  WmiResult invokeFileCommit(const api::VolumeRow& row, const std::string& fileFullPath, const char* method) const;

  WmiSession& m_session;
};

}  // namespace uwf::api
