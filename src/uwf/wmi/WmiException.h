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

#include <cstdint>
#include <string>
#include <system_error>

namespace uwf {

const std::error_category& wmiErrorCategory() noexcept;
std::error_code makeWmiErrorCode(int32_t value) noexcept;

// 异常对象只携带事实，不记录日志。日志与用户提示由决定恢复策略的 catch
// 边界统一完成，避免同一次失败在 WMI、领域和 UI 三层重复输出。
class WmiException : public std::system_error {
 public:
  [[nodiscard]] const std::string& operation() const noexcept { return m_operation; }

 protected:
  WmiException(std::error_code code, std::string operation, std::string detail = {});

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
  using WmiException::WmiException;
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

}  // namespace uwf
