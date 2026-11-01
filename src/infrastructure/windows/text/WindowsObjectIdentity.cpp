// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <Windows.h>

#include <limits>
#include <stdexcept>
#include <system_error>

#include "foundation/text/ObjectNameComparison.h"

namespace uwf {
int compareObjectNames(const std::u16string_view left, const std::u16string_view right) {
  if (left.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()) || right.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    throw std::length_error("The object name is too long for an ordinal comparison");
  const auto* leftText = left.empty() ? L"" : reinterpret_cast<const wchar_t*>(left.data());
  const auto* rightText = right.empty() ? L"" : reinterpret_cast<const wchar_t*>(right.data());
  const auto result = CompareStringOrdinal(leftText, static_cast<int>(left.size()), rightText, static_cast<int>(right.size()), TRUE);
  if (!result) throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "Object name comparison failed");
  return result - CSTR_EQUAL;
}
}  // namespace uwf
