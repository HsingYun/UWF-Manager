// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// UWF_Volume —— 每个卷在每个 Session 下各一行（key 三元组：
// CurrentSession + DriveLetter + VolumeName）。
//
// 字段：BindByDriveLetter / CommitPending / Protected
// 方法：Protect / Unprotect / CommitFile / CommitFileDeletion /
//       SetBindByDriveLetter / AddExclusion / RemoveExclusion /
//       RemoveAllExclusions / FindExclusion / GetExclusions

#include <functional>
#include <string>
#include <vector>

#include "UwfProviderTypes.h"
#include "app/ports/VolumePaths.h"
#include "app/ports/WmiOperations.h"

namespace uwf::app::provider {

enum class VolumeRegistrationDisposition { AlreadyPresent, Created, ConcurrentlyCreated, ConfirmedAfterUncertainWrite };

struct VolumeRegistration {
  provider::VolumeRow row;
  VolumeRegistrationDisposition disposition;
};

class UwfVolume {
 public:
  // 挂载查询接收完整卷 GUID 路径，返回该卷当前挂载的所有规范盘符。
  // 这是文件系统边界；默认实现直接查询 Windows 卷管理器。
  using MountLookup = drive::MountLookup;

  explicit UwfVolume(wmi::WmiOperations& session, MountLookup mounts);

  // WMI 身份是会话、盘符、卷名三元组。两个会话独立保留配置，各自出现盘符
  // 冲突时按真实挂载关系和绑定语义消歧，向消费者提供每会话、每盘符至多一条
  // 可操作记录；真正重复或仍有歧义的实例报告异常，不能随枚举顺序任取一条。
  std::vector<provider::VolumeRow> readAll() const;

  void protectVolume(const provider::VolumeRow& row) const;
  void unprotect(const provider::VolumeRow& row) const;

  void commitFile(const provider::VolumeRow& row, const std::string& fileFullPath) const;

  // CommitFileDeletion 直接删除仍存在的目标，并持久化删除结果；不存在时 provider 返回 NotFound。
  void commitFileDeletion(const provider::VolumeRow& row, const std::string& fileName) const;

  // 对应 UWF_Volume.SetBindByDriveLetter(boolean bBindByDriveLetter) 官方签名：
  // bBindByDriveLetter=true 表示按盘符绑定（松绑定），false 表示按卷名绑定（紧绑定）。
  void setBinding(const provider::VolumeRow& row, provider::VolumeBinding binding) const;

  void addExclusion(const provider::VolumeRow& row, const std::string& fileName) const;
  void removeExclusion(const provider::VolumeRow& row, const std::string& fileName) const;
  void removeAllExclusions(const provider::VolumeRow& row) const;

  bool findExclusion(const provider::VolumeRow& row, const std::string& fileName) const;

  std::vector<provider::ExcludedFile> getExclusions(const provider::VolumeRow& row) const;

  // 缺少 next-session 行时，复制 current 行的规范卷标识创建下一会话实例。
  // 返回值区分本次写入与并发收敛，失败通过异常报告。
  [[nodiscard]] VolumeRegistration ensureNextSessionEntry(const std::string& driveLetter) const;

 private:
  wmi::WmiOperations& m_session;
  MountLookup m_mounts;
};

}  // namespace uwf::app::provider
