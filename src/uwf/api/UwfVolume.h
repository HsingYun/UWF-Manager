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

  // 语义：删除一个**当前仍存在**的受保护文件——CommitFileDeletion 由方法自身把
  // 该文件从覆盖层与物理卷一并删除，并非"提交一个已发生的删除"。因此调用方应先
  // 校验"该路径当前确实存在"（文件不存在时方法回 WBEM_E_NOT_FOUND）。
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
