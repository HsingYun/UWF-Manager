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

class QWidget;

namespace uwf::ui {

enum class PowerAction { Shutdown, Restart };

// UWF 安全电源操作专用确认框。关机 / 重启共享一致的风险信息与默认取消行为，
// 但使用各自的图标、标题和动作按钮。
bool confirmPowerAction(QWidget* parent, PowerAction action);

}  // namespace uwf::ui
