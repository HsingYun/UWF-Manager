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

#include <exception>
#include <optional>
#include <string>
#include <string_view>

#include "../wmi/WmiException.h"

namespace uwf::api {

// 写方法绝不自动重放。调用结果明确成功，或连接在调用发出后断开时，都通过
// 一次权威重读决定最终状态；其余失败保持原异常语义直接上抛。
template <typename Invoke, typename Observe>
void invokeAndConfirm(const std::string_view operation, Invoke&& invoke, Observe&& observe) {
  std::optional<std::string> uncertainInvocation;
  try {
    invoke();
  } catch (const WmiInvocationUncertain& error) {
    uncertainInvocation = error.what();
  }

  try {
    if (observe()) return;
  } catch (const std::exception& observationError) {
    const std::string detail =
        uncertainInvocation ? "write outcome was uncertain and the authoritative state could not be reread; original failure: " + *uncertainInvocation +
                                  "; reread failure: " + observationError.what()
                            : "write completed, but the authoritative state could not be reread; reread failure: " + std::string(observationError.what());
    std::throw_with_nested(WmiStateVerificationError(std::string(operation), detail));
  } catch (...) {
    const std::string detail =
        uncertainInvocation
            ? "write outcome was uncertain and the authoritative state reread failed with a non-standard exception; original failure: " + *uncertainInvocation
            : "write completed, but the authoritative state reread failed with a non-standard exception";
    std::throw_with_nested(WmiStateVerificationError(std::string(operation), detail));
  }

  const std::string detail = uncertainInvocation
                                 ? "write outcome was uncertain and the authoritative state does not match; original failure: " + *uncertainInvocation
                                 : "provider accepted the write, but the authoritative state does not match";
  throw WmiStateVerificationError(std::string(operation), detail);
}

}  // namespace uwf::api
