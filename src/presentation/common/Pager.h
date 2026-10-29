// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Pagination preserves the first visible entry when the viewport capacity changes.

#include <algorithm>
#include <cstdint>
#include <limits>

namespace uwf::presentation {

struct Pager {
  int pageSize = 1;     // 每页行数，>=1
  int currentPage = 0;  // 0-based

  // 总页数；total<=0 → 0。
  [[nodiscard]] int pageCount(int total) const {
    if (total <= 0) return 0;
    const int size = std::max(1, pageSize);
    return 1 + (total - 1) / size;
  }

  // 把 currentPage 夹回 [0, pageCount-1]（无页时 0）。渲染前调一次，吸收导航
  // 越界 / total 变化。
  void clamp(int total) {
    const int pages = pageCount(total);
    currentPage = std::clamp(currentPage, 0, pages > 0 ? pages - 1 : 0);
  }

  // 当前页在全量序列里的 [start, end)。调用方应先 clamp()。
  [[nodiscard]] int pageStart() const {
    const auto start = static_cast<std::int64_t>(std::max(0, currentPage)) * std::max(1, pageSize);
    return static_cast<int>(std::min(start, static_cast<std::int64_t>(std::numeric_limits<int>::max())));
  }
  [[nodiscard]] int pageEnd(int total) const {
    if (total <= 0) return 0;
    const int start = std::min(pageStart(), total);
    return start + std::min(std::max(1, pageSize), total - start);
  }

  // 改 pageSize 并尽量保住"当前页第一条"指向的那条 entry（重新定位到它所在的
  // 新页）。返回 pageSize 是否真的变了。viewport resize 时用。
  bool setPageSize(int newSize) {
    newSize = std::max(1, newSize);
    if (newSize == pageSize) return false;
    const int firstIdx = pageStart();
    pageSize = newSize;
    currentPage = firstIdx / newSize;
    return true;
  }

  // 导航：改 currentPage 后调用方负责重渲染（渲染里 clamp 会再兜一次越界）。
  void goFirst() { currentPage = 0; }
  void goPrev() {
    if (currentPage > 0) --currentPage;
  }
  void goNext(int total) {
    const int pages = pageCount(total);
    if (currentPage >= 0 && currentPage < pages - 1) ++currentPage;
  }
  void goLast(int total) {
    const int pages = pageCount(total);
    currentPage = pages > 0 ? pages - 1 : 0;
  }

  // 导航按钮可用性（基于已 clamp 的 currentPage）。
  [[nodiscard]] bool hasPrev() const { return currentPage > 0; }
  [[nodiscard]] bool hasNext(int total) const {
    const int pages = pageCount(total);
    return currentPage >= 0 && currentPage < pages - 1;
  }
};

}  // namespace uwf::presentation
