// SPDX-License-Identifier: GPL-3.0-or-later
#include "foundation/logging/LocalTime.h"

namespace uwf::detail {
std::tm localTime(const std::time_t time) noexcept {
  std::tm result{};
  localtime_s(&result, &time);
  return result;
}
}  // namespace uwf::detail
