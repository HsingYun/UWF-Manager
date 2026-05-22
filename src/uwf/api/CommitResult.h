#pragma once

// 文件 / 注册表「提交（commit）」操作的统一结果类型与失败归类。
//
// UWF_Volume 的 CommitFile / CommitFileDeletion 与 UWF_RegistryFilter 的
// CommitRegistry / CommitRegistryDeletion 共用同一套结果语义，故抽到此处
// 单一定义，避免 file / registry 两侧各写一份分类逻辑、改一处漏一处。

#include <cstdint>
#include <string>

#include "../wmi/WmiClient.h"
#include "../wmi/WmiError.h"

namespace uwf {

// commit 类操作的结果分三档：
//   Ok      — UWF 已把 overlay 里的改动写回磁盘 / 注册表。
//   Skipped — 调用被底层拒绝但属于「可忽略」类：
//             * WBEM_E_FAILED    0x80041001 → 目标被其他进程占用；
//             * WBEM_E_NOT_FOUND 0x80041002 → overlay 里没有相关待提交条目
//               （目标未被改动过，或根本不存在——HRESULT 不区分这两种）。
//             UI 只平静提示、不当作错误。
//   Failed  — 其它错误（INVALID_PARAMETER / UWF rv != 0 等）。
enum class CommitOutcome { Ok, Skipped, Failed };

struct CommitResult {
  CommitOutcome outcome = CommitOutcome::Ok;
  int32_t hresult = 0;       // ExecMethod 的 HRESULT；invoked=true 时为 0
  uint32_t returnValue = 0;  // UWF 方法的 UInt32 返回值；0 = 成功
  std::string detail;        // 原始技术细节，仅用于日志；不要直接丢给用户看
};

// commit 类方法失败时的归类：WBEM_E_FAILED / WBEM_E_NOT_FOUND 归 Skipped，
// 其它归 Failed。ExecMethod 本身成功（invoked）但 UWF 返回非 0 一律 Failed。
inline CommitOutcome classifyCommitFailure(const WmiMethodResult& r) {
  if (r.invoked) return CommitOutcome::Failed;
  const WmiError err(r.hresult);
  return (err == WmiErrorCode::Failed || err == WmiErrorCode::NotFound) ? CommitOutcome::Skipped : CommitOutcome::Failed;
}

}  // namespace uwf
