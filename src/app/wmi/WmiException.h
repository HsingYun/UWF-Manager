// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <string>
#include <system_error>

#include "app/api/OperationError.h"

namespace uwf::app::wmi {

enum class WmiErrorCode : int32_t;

const std::error_category& wmiErrorCategory() noexcept;
std::error_code makeWmiErrorCode(int32_t value) noexcept;

// 异常对象只携带事实，不记录日志。日志与用户提示由决定恢复策略的 catch
// 边界统一完成，避免同一次失败在 WMI、领域和 UI 三层重复输出。
class WmiException : public OperationError {
 public:
  [[nodiscard]] const std::string& operation() const noexcept { return m_operation; }
  [[nodiscard]] bool hasCode(WmiErrorCode status) const noexcept;

 protected:
  WmiException(std::error_code code, std::string operation, std::string detail = {}, WriteEffect effect = WriteEffect::None);

 private:
  std::string m_operation;
};

class WmiTransportError final : public WmiException {
 public:
  WmiTransportError(int32_t hresult, std::string operation, std::string detail = {});
};

// COM/WMI 基础设施在请求层面返回的 HRESULT；它既不是 provider 方法
// ReturnValue，也不是可安全断线重试的传输错误。
class WmiInfrastructureError final : public WmiException {
 public:
  WmiInfrastructureError(int32_t hresult, std::string operation, std::string detail = {});
};

class WmiProviderError final : public WmiException {
 public:
  WmiProviderError(uint32_t returnValue, std::string operation, std::string detail = {});
  [[nodiscard]] uint32_t returnValue() const noexcept { return m_returnValue; }

 private:
  uint32_t m_returnValue;
};

class WmiProtocolError final : public WmiException {
 public:
  WmiProtocolError(std::string operation, std::string detail);
};

// 非 final，供输出解码边界用 std::throw_with_nested 保留原始本地异常。
class WmiDecodeError : public WmiException {
 public:
  WmiDecodeError(std::string operation, std::string detail);
};

// 这类失败发生后，写操作可能已经改变系统。捕获此类型意味着调用方必须通过
// 权威重读对账；普通 WmiException 不携带这种恢复策略。
class WmiWriteOutcomeError : public WmiException {
 protected:
  WmiWriteOutcomeError(std::error_code code, std::string operation, std::string detail)
      : WmiException(code, std::move(operation), std::move(detail), WriteEffect::Uncertain) {}
};

// 调用已经越过可能产生副作用的边界，但没有拿到可信的 provider 结果。这里
// 有意用独立类型表达，而不是把标志贯穿到每一种解码异常。保持非 final，使
// std::throw_with_nested 可以保留最初的技术异常，不必在本类中再存一份异常。
class WmiInvocationUncertain : public WmiWriteOutcomeError {
 public:
  WmiInvocationUncertain(int32_t hresult, std::string operation, std::string detail = {});
  WmiInvocationUncertain(std::string operation, std::string detail);
};

// 写操作已被接受或可能已经执行，但无法确认其权威可观测状态。保持非 final
// 的原因与 WmiInvocationUncertain 相同，用于保留嵌套异常。
class WmiStateVerificationError : public WmiWriteOutcomeError {
 public:
  WmiStateVerificationError(std::string operation, std::string detail);
};

class WmiCancelled final : public WmiException {
 public:
  explicit WmiCancelled(std::string operation);
};

}  // namespace uwf::app::wmi
