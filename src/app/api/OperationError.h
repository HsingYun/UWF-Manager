// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <system_error>
#include <utility>

namespace uwf::app {
enum class WriteEffect { None, Confirmed, Uncertain };

// Failure and side effects are independent facts. Composite operations must
// report effects from earlier steps even when the failing step was a read.
class OperationError : public std::system_error {
 public:
  OperationError(std::error_code code, std::string detail, WriteEffect effect = WriteEffect::None)
      : std::system_error(code, std::move(detail)), m_effect(effect) {}
  [[nodiscard]] WriteEffect writeEffect() const noexcept { return m_effect; }

 private:
  WriteEffect m_effect;
};
}  // namespace uwf::app
