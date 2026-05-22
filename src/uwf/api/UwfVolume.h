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
#include "CommitResult.h"
#include "Types.h"

namespace uwf {

class UwfVolume {
 public:
  explicit UwfVolume(WmiSession& session) : m_session(session) {}

  std::vector<api::VolumeRow> readAll(std::string* error = nullptr) const;

  bool protectVolume(const api::VolumeRow& row, std::string* error = nullptr) const;
  bool unprotect(const api::VolumeRow& row, std::string* error = nullptr) const;

  CommitResult commitFile(const api::VolumeRow& row, const std::string& fileFullPath) const;

  // 语义：把 overlay 里"删除"这个动作提交到物理盘。典型流程：
  //   1. 用户在受保护卷上删了某文件 → overlay 记下删除标记；
  //   2. 从 OS 视角看文件已不存在；物理盘上还在；
  //   3. CommitFileDeletion 同步物理盘，让两边一致。
  // 因此调用方应该先校验"该路径在当前 OS 视角下确实不存在"（否则说明
  // overlay 里没这个删除标记，调用多半会失败）。
  CommitResult commitFileDeletion(const api::VolumeRow& row, const std::string& fileName) const;

  // 对应 UWF_Volume.SetBindByDriveLetter(boolean bBindByDriveLetter) 官方签名：
  // bBindByDriveLetter=true 表示按盘符绑定（松绑定），false 表示按卷名绑定（紧绑定）。
  bool setBindByDriveLetter(const api::VolumeRow& row, bool bBindByDriveLetter, std::string* error = nullptr) const;

  bool addExclusion(const api::VolumeRow& row, const std::string& fileName, std::string* error = nullptr) const;
  bool removeExclusion(const api::VolumeRow& row, const std::string& fileName, std::string* error = nullptr) const;
  bool removeAllExclusions(const api::VolumeRow& row, std::string* error = nullptr) const;

  std::optional<bool> findExclusion(const api::VolumeRow& row, const std::string& fileName, std::string* error = nullptr) const;

  std::vector<api::ExcludedFile> getExclusions(const api::VolumeRow& row, std::string* error = nullptr) const;

  // 拿到指定卷的 next session 实例。如果 UWF_Volume 里没有 next session
  // 行（卷从未被 protect 过、没加过 exclusion），就从该卷的 current session
  // 行复制 VolumeName，用 PutInstance 创建一份 next session 实例后返回。
  // 之所以从 current 复制：UWF 自己列出来的 current 实例 VolumeName 格式
  // 必定规范（"Volume{GUID}"，无 \\?\ 前缀、无尾斜杠），caller 不必再做
  // 跨命名空间查 Win32_Volume 然后归一化的脏活。
  std::optional<api::VolumeRow> ensureNextSessionEntry(const std::string& driveLetter, std::string* error = nullptr) const;

 private:
  WmiSession& m_session;
};

}  // namespace uwf
