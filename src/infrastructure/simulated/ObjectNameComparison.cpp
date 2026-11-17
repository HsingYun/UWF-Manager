// SPDX-License-Identifier: GPL-3.0-or-later
#include "foundation/text/ObjectNameComparison.h"

#include <QChar>
#include <algorithm>

namespace uwf {
int compareObjectNames(const std::u16string_view left, const std::u16string_view right) {
  // Simple UTF-16 uppercase comparison, not Unicode case folding (which merges
  // distinct Windows objects such as sharp S). Legacy micro sign and long S remain literal.
  // This preview adapter does not replace Windows CompareStringOrdinal tests.
  const auto upper = [](char16_t unit) { return (unit == u'\u00b5' || unit == u'\u017f') ? unit : QChar(unit).toUpper().unicode(); };
  for (std::size_t index = 0; index < std::min(left.size(), right.size()); ++index) {
    const auto first = upper(left[index]);
    const auto second = upper(right[index]);
    if (first != second) return first < second ? -1 : 1;
  }
  return left.size() == right.size() ? 0 : left.size() < right.size() ? -1 : 1;
}
}  // namespace uwf
