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

// 把 GlobalStatusPanel 与各 DiskTab 上累积的待应用变更收集成
// core::PendingChanges。只采集"和基线不同"的字段。ApplyPlanDialog（写入 +
// 预览）与 MainWindow（状态栏计数）共用同一次遍历，避免对同一组 pendingX()
// getter 多处各写一遍、容易漏字段或口径不一致。

#include <QPointer>
#include <QVector>

#include "../core/UwfModel.h"

namespace uwf::ui {

class GlobalStatusPanel;
class DiskTab;

[[nodiscard]] core::PendingChanges collectPending(const GlobalStatusPanel* global, const QVector<QPointer<DiskTab>>& diskTabs);

}  // namespace uwf::ui
