// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Page-size changes retain the page containing the previous first entry.

#include <algorithm>
#include <cstdint>
#include <limits>

namespace uwf::presentation {

struct Pager {
  int pageSize = 1;     // At least one row per page.
  int currentPage = 0;  // 0-based

  [[nodiscard]] int pageCount(int total) const {
    if (total <= 0) return 0;
    const int size = std::max(1, pageSize);
    return 1 + (total - 1) / size;
  }

  // Bounds the page index; an empty collection uses page zero.
  void clamp(int total) {
    const int pages = pageCount(total);
    currentPage = std::clamp(currentPage, 0, pages > 0 ? pages - 1 : 0);
  }

  // The caller clamps the page index before using the half-open row range.
  [[nodiscard]] int pageStart() const {
    const auto start = static_cast<std::int64_t>(std::max(0, currentPage)) * std::max(1, pageSize);
    return static_cast<int>(std::min(start, static_cast<std::int64_t>(std::numeric_limits<int>::max())));
  }
  [[nodiscard]] int pageEnd(int total) const {
    if (total <= 0) return 0;
    const int start = std::min(pageStart(), total);
    return start + std::min(std::max(1, pageSize), total - start);
  }

  // Selects the new page containing the previous first entry; returns whether the size changed.
  bool setPageSize(int newSize) {
    newSize = std::max(1, newSize);
    if (newSize == pageSize) return false;
    const int firstIdx = pageStart();
    pageSize = newSize;
    currentPage = firstIdx / newSize;
    return true;
  }

  [[nodiscard]] bool hasPrev() const { return currentPage > 0; }
  [[nodiscard]] bool hasNext(int total) const {
    const int pages = pageCount(total);
    return currentPage >= 0 && currentPage < pages - 1;
  }
};

}  // namespace uwf::presentation
