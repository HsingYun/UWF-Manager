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
//   Skipped — 合法的「无内容可提交」：
//             * WBEM_E_NOT_FOUND 0x80041002 → overlay 里没有相关待提交条目
//               （目标未被改动过，或根本不存在——HRESULT 不区分这两种）。
//             UI 只平静提示、不当作错误。
//   Failed  — 其它错误（WBEM_E_FAILED / INVALID_PARAMETER / UWF rv != 0 等）。
//
// 注意 WBEM_E_FAILED (0x80041001) 是 UWF provider 的「泛失败」码，没有确定原因。
// 历史版本曾据此猜「目标被其他进程占用」归到 Skipped——实测下来 procmon trace
// 显示这一猜测**确实是最常见的一种诱因**（资源管理器打开着目录、编辑器打开
// 着文件时 CommitFileDeletion 必报这个码），但其它原因（权限、UWF 内部状态等）
// 也会落到同一个 HRESULT，无法仅凭代码区分。归到 Failed（用户看见，不当作
// 静默 skip），让 explainCommitFailure 在 reason 文本里把"可能被占用"作为
// **可能性**提示给用户（不能断言）。
enum class CommitOutcome { Ok, Skipped, Failed };

struct CommitResult {
  CommitOutcome outcome = CommitOutcome::Ok;
  int32_t hresult = 0;       // ExecMethod 的 HRESULT；invoked=true 时为 0
  uint32_t returnValue = 0;  // UWF 方法的 UInt32 返回值；0 = 成功
  std::string detail;        // 原始技术细节，仅用于日志；不要直接丢给用户看
};

// commit 类方法失败时的归类：只有 WBEM_E_NOT_FOUND 归 Skipped（"没东西要提交"
// 是合法终态）；其它一律 Failed。ExecMethod 本身成功（invoked）但 UWF 返回非 0
// 一律 Failed。
inline CommitOutcome classifyCommitFailure(const WmiMethodResult& r) {
  if (r.invoked) return CommitOutcome::Failed;
  return WmiError(r.hresult).code() == WmiErrorCode::NotFound ? CommitOutcome::Skipped : CommitOutcome::Failed;
}

}  // namespace uwf
