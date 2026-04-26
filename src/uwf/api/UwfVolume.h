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
#include "Types.h"

namespace uwf {

// commitFile 的结果分三档：
//   Ok      — UWF 已把 overlay 里该文件的修改写回磁盘
//   Skipped — 调用被底层拒绝但属于"可忽略"类：
//             * WBEM_E_FAILED    0x80041001 → 通常是文件正被其他进程占用；
//             * WBEM_E_NOT_FOUND 0x80041002 → overlay 里没这条（已与磁盘一致）。
//   Failed  — 其它错误（INVALID_PARAMETER / UWF rv != 0 等）。
enum class CommitOutcome { Ok, Skipped, Failed };

struct CommitFileResult {
  CommitOutcome outcome = CommitOutcome::Ok;
  int32_t hresult = 0;       // ExecMethod 的 HRESULT；invoked=true 时为 0
  uint32_t returnValue = 0;  // UWF 方法的 UInt32 返回值；0 = 成功
  std::string detail;        // 原始技术细节，仅用于日志；不要直接丢给用户看
};

class UwfVolume {
 public:
  explicit UwfVolume(WmiSession& session) : m_session(session) {}

  std::vector<api::VolumeRow> readAll(std::string* error = nullptr) const;

  bool protectVolume(const api::VolumeRow& row, std::string* error = nullptr) const;
  bool unprotect(const api::VolumeRow& row, std::string* error = nullptr) const;

  CommitFileResult commitFile(const api::VolumeRow& row, const std::string& fileFullPath) const;

  // 语义：把 overlay 里"删除"这个动作提交到物理盘。典型流程：
  //   1. 用户在受保护卷上删了某文件 → overlay 记下删除标记；
  //   2. 从 OS 视角看文件已不存在；物理盘上还在；
  //   3. CommitFileDeletion 同步物理盘，让两边一致。
  // 因此调用方应该先校验"该路径在当前 OS 视角下确实不存在"（否则说明
  // overlay 里没这个删除标记，调用多半会失败）。
  CommitFileResult commitFileDeletion(const api::VolumeRow& row, const std::string& fileName) const;

  // bBindByVolumeName=true 表示按卷名绑定（紧密绑定），false 按盘符绑定。
  bool setBindByDriveLetter(const api::VolumeRow& row, bool bBindByVolumeName, std::string* error = nullptr) const;

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

  // 删除指定的 UWF_Volume 实例（DeleteInstance）。一般在 unprotect 成功且
  // 该卷没有任何文件排除时使用，把"已注册但什么都不做"的空 next session
  // 实例从 UWF 数据库里清掉，避免 UWF_Volume 表里堆积无意义记录。
  bool deleteRow(const api::VolumeRow& row, std::string* error = nullptr) const;

 private:
  WmiSession& m_session;
};

}  // namespace uwf
