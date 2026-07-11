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

#include <QString>

namespace uwf::ui {

// 读取 Windows / CPU / RAM / GPU 摘要，并渲染成主窗口悬停提示框使用的 HTML。
// 平台查询和展示格式集中在这里，MainWindow 只消费最终文本。
class SystemInfoProvider {
 public:
  [[nodiscard]] static QString summaryHtml();
};

}  // namespace uwf::ui
