#pragma once

// WMI 写操作（CommitFile / CommitRegistry / Protect / Set* / AddExclusion /
// PutInstance / DeleteInstance ...）的**统一**结果类型。覆盖整个项目所有写
// 操作的回路——不再分"bool + std::string*"与"CommitResult"两套，避免调用方
// 来回适配。调用方拿到 WmiResult 后看 .ok / .detail；commit / delete 这类有
// "无内容可提交"语义的调用方再用 commitOutcome() 把"非 OK"细分成 Skipped /
// Failed。

#include <cstdint>
#include <string>
#include <utility>

#include "WmiClient.h"
#include "WmiError.h"

namespace uwf {

struct WmiResult {
  bool ok = false;           // 业务上是否成功（hresult==0 && returnValue==0）
  int32_t hresult = 0;       // ExecMethod 的 HRESULT；ok=true 时为 0
  uint32_t returnValue = 0;  // 方法返回值（UInt32，UWF 约定 0=成功）；ok=true 时为 0
  std::string detail;        // 失败时的可读信息（来自 WmiMethodResult::error 或调用方自填）

  // 把 WmiMethodResult 折叠成 WmiResult 的常规收尾。所有 Uwf*::xxx 的 WMI
  // 调用都用这条路径，保证字段口径一致。
  [[nodiscard]] static WmiResult fromMethodResult(const WmiMethodResult& r) {
    WmiResult out;
    out.ok = r.ok();
    out.hresult = r.hresult;
    out.returnValue = r.returnValue;
    if (!out.ok) out.detail = r.error;
    return out;
  }

  // 纯本地失败（不经 WMI 就直接拒）的构造——参数校验失败、缺 __PATH 等。
  // hresult / returnValue 留 0，调用方靠 .ok + .detail 判定即可。
  [[nodiscard]] static WmiResult failed(std::string msg) {
    WmiResult out;
    out.detail = std::move(msg);
    return out;
  }
};

// commit 类操作（CommitFile / CommitRegistry / Commit{File,Registry}Deletion）
// 的失败细分。WBEM_E_NOT_FOUND（0x80041002）是合法的"无内容可提交"（overlay
// 没改动 / 物理盘没目标 / commit-delete 时目标不在持久化存储），UI 视为 Skipped；
// 其它失败一律 Failed。
//
// 普通写操作（Protect / Set* / AddExclusion 等）的失败**不要走这里**——它们
// 没有"Skipped"语义，直接看 .ok 即可。
//
// 注 WBEM_E_FAILED (0x80041001) 是 UWF provider 的「泛失败」码，实测最常见的
// 一种诱因是"目标被其他进程占着 handle"——但其它原因（权限、UWF 内部状态等）
// 也会落到同一个 HRESULT，无法仅凭码区分。归到 Failed（用户看见），由 UI
// 在 reason 文本里把"可能被占用"作为**可能性**提示（不能断言）。
enum class CommitOutcome { Ok, Skipped, Failed };

[[nodiscard]] inline CommitOutcome commitOutcome(const WmiResult& r) {
  if (r.ok) return CommitOutcome::Ok;
  return WmiError(r.hresult).code() == WmiErrorCode::NotFound ? CommitOutcome::Skipped : CommitOutcome::Failed;
}

}  // namespace uwf
