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

// WMI 写操作（CommitFile / CommitRegistry / Protect / Set* / AddExclusion /
// PutInstance / DeleteInstance ...）的**统一**结果类型。覆盖整个项目所有写
// 操作的回路——不再分"bool + std::string*"与"CommitResult"两套，避免调用方
// 来回适配。调用方拿到 WmiResult 后看 .ok / .detail；commit / delete 这类有
// "无内容可提交"语义的调用方再用 commitOutcome() 把"非 OK"细分成 Skipped /
// Failed。

#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "WmiClient.h"
#include "WmiError.h"

namespace uwf {

enum class VerificationStatus { Matches, Mismatch, Unavailable };

struct StateVerification {
  VerificationStatus status = VerificationStatus::Unavailable;
  std::string detail;

  [[nodiscard]] static StateVerification matches() { return {VerificationStatus::Matches, {}}; }
  [[nodiscard]] static StateVerification mismatch(std::string detail = {}) { return {VerificationStatus::Mismatch, std::move(detail)}; }
  [[nodiscard]] static StateVerification unavailable(std::string detail) { return {VerificationStatus::Unavailable, std::move(detail)}; }
};

template <typename T, typename Predicate>
[[nodiscard]] StateVerification verifyObservedState(const std::optional<T>& observed, Predicate matches, std::string readError,
                                                    std::string missingDetail = "target state was not returned") {
  if (!observed) return StateVerification::unavailable(readError.empty() ? std::move(missingDetail) : std::move(readError));
  return matches(*observed) ? StateVerification::matches() : StateVerification::mismatch();
}

struct WmiResult {
  bool attempted = false;    // 写请求已提交给 WMI；本地校验/准备失败时为 false
  bool ok = false;           // provider 接受且（需要时）写后状态已确认
  bool outcomeUncertain = false;  // 调用可能已生效，但传输/返回包无法给出确定结论
  int32_t hresult = 0;       // 原始调用 HRESULT；模糊传输失败经状态确认后仍保留
  uint32_t returnValue = 0;  // 方法返回值（UInt32，UWF 约定 0=成功）
  std::string detail;        // 失败时的可读信息（来自 WmiMethodResult::error 或调用方自填）

  // 把 WmiMethodResult 折叠成 WmiResult 的常规收尾。所有 Uwf*::xxx 的 WMI
  // 调用都用这条路径，保证字段口径一致。
  [[nodiscard]] static WmiResult fromMethodResult(const WmiMethodResult& r) {
    WmiResult out;
    out.attempted = r.attempted;
    out.ok = r.ok();
    out.outcomeUncertain = r.attempted && ((!r.invoked && isWmiConnectionFailure(r.hresult)) || (r.invoked && !r.returnValuePresent));
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

// 写方法绝不重放。调用完成后由领域 API 重新读取对应状态，并把“方法返回”与
// “最终状态”合并成一个结果：
// - 正常返回成功但无法确认/状态不符，不能报成功；
// - 连接故障或不完整返回包使调用结果不确定，若重读状态已经符合预期，
//   则可确认成功；
// - provider 明确拒绝的写，即使状态此前就等于期望值，也不能把拒绝改报成功。
[[nodiscard]] inline WmiResult confirmWriteState(WmiResult result, StateVerification verification, std::string_view operation) {
  // 只有已经提交给 provider、且结果模型明确标记为不确定的调用，才允许由
  // 写后状态确认裁决。连接、方法签名或本地参数准备阶段的失败没有发出写
  // 请求，即使目标原本就处于期望状态，也不能改报成本次写成功。
  const bool ambiguous = result.attempted && !result.ok && result.outcomeUncertain;
  if (verification.status == VerificationStatus::Matches) {
    if (result.ok || ambiguous) {
      result.ok = true;
      if (ambiguous) {
        const std::string original = result.detail;
        result.detail = std::format("{} state confirmed after an uncertain invocation result", operation);
        if (!original.empty()) result.detail += std::format(": {}", original);
      }
    }
    return result;
  }

  // provider 已明确拒绝、且不是可能已产生副作用的传输故障时，重新读取只用于
  // 遵守“写后确认”的一致流程，不能把明确错误改写成“操作完成但状态不符”。
  // 只有原调用成功或结果不确定时，验证失败才参与最终裁决与错误说明。
  if (!result.ok && !ambiguous) return result;

  const std::string verificationDetail =
      verification.status == VerificationStatus::Unavailable
          ? std::format("{} state verification unavailable{}{}", operation, verification.detail.empty() ? "" : ": ", verification.detail)
          : std::format("{} completed but the reread state does not match{}{}", operation, verification.detail.empty() ? "" : ": ", verification.detail);
  if (result.detail.empty())
    result.detail = verificationDetail;
  else
    result.detail += std::format("; {}", verificationDetail);
  result.ok = false;
  return result;
}

// 读类方法（GetExclusions / FindExclusion / GetOverlayFiles 等）!ok() 时的统一
// 错误文案：WMI 层失败（invoked=false）直接用 r.error；ExecMethod 成功但方法
// 返回非 0 时给 "Class::Method returned N"。qualifiedMethod 形如
// "UWF_Volume::GetExclusions"。各处失败分支共用，避免重复这段三元拼接。
[[nodiscard]] inline std::string methodErrorDetail(const WmiMethodResult& r, std::string_view qualifiedMethod) {
  return r.invoked && r.returnValuePresent ? std::format("{} returned {}", qualifiedMethod, r.returnValue) : r.error;
}

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
  // ExecMethod 本身失败时错误在 hresult；调用抵达 provider 后，UWF 方法的
  // 业务错误则通过 UInt32 ReturnValue 返回。两条通道互斥，分类时必须取实际
  // 失败通道，不能因为 ExecMethod 的 HRESULT 是 S_OK 就丢掉 ReturnValue。
  const int32_t errorCode = r.hresult != 0 ? r.hresult : static_cast<int32_t>(r.returnValue);
  return WmiError(errorCode).code() == WmiErrorCode::NotFound ? CommitOutcome::Skipped : CommitOutcome::Failed;
}

}  // namespace uwf
