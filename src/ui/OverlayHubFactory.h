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

class QObject;

namespace uwf::ui {

class OverlayHub;

// 应用当前使用的 Hub 组合根。新增展示端点只需实现 OverlayHubView 并在这里
// 注册；控制器和 Hub 的 fallback 算法都不需要认识具体实现类。
[[nodiscard]] OverlayHub* createDefaultOverlayHub(QObject* parent = nullptr);

}  // namespace uwf::ui
