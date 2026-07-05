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

// 纯分页算术——无 Qt、无渲染。维护 pageSize / currentPage，按总条数算页数、夹取
// 当前页、给出当前页的 [start, end) 区间与导航。三个分页对话框
// （CommitReportDialog / LogViewerDialog / OverlayFilesDialog）共用这套算术；
// 各自的渲染（表 / 列表）、控件接线、worker 线程仍由各对话框自己负责。
//
// 约定：pageCount(total)==0 表示"无内容"——是否显示成 0 页或 1 页由调用方决定。

#include <algorithm>

namespace uwf::ui {

struct Pager {
  int pageSize = 1;     // 每页行数，>=1
  int currentPage = 0;  // 0-based

  // 总页数；total<=0 → 0。
  [[nodiscard]] int pageCount(int total) const {
    if (total <= 0 || pageSize <= 0) return 0;
    return (total + pageSize - 1) / pageSize;
  }

  // 把 currentPage 夹回 [0, pageCount-1]（无页时 0）。渲染前调一次，吸收导航
  // 越界 / total 变化。
  void clamp(int total) {
    const int pages = pageCount(total);
    currentPage = std::clamp(currentPage, 0, pages > 0 ? pages - 1 : 0);
  }

  // 当前页在全量序列里的 [start, end)。调用方应先 clamp()。
  [[nodiscard]] int pageStart() const { return currentPage * pageSize; }
  [[nodiscard]] int pageEnd(int total) const { return std::min(total, pageStart() + std::max(1, pageSize)); }

  // 改 pageSize 并尽量保住"当前页第一条"指向的那条 entry（重新定位到它所在的
  // 新页）。返回 pageSize 是否真的变了。viewport resize 时用。
  bool setPageSize(int newSize) {
    newSize = std::max(1, newSize);
    if (newSize == pageSize) return false;
    const int firstIdx = currentPage * pageSize;
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
    if (currentPage + 1 < pageCount(total)) ++currentPage;
  }
  void goLast(int total) {
    const int pages = pageCount(total);
    currentPage = pages > 0 ? pages - 1 : 0;
  }

  // 导航按钮可用性（基于已 clamp 的 currentPage）。
  [[nodiscard]] bool hasPrev() const { return currentPage > 0; }
  [[nodiscard]] bool hasNext(int total) const { return currentPage + 1 < pageCount(total); }
};

}  // namespace uwf::ui
