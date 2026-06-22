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

// 跨控件复用的小 UI helper——原本在多个文件里各写一份（逐字重复），收口到此。
// 都是薄封装：包一层 QString 边界 / Win32 Shell / 表单控件样式，无业务逻辑。

#include <QString>

class QComboBox;
class QVariant;
class QWidget;

namespace uwf::ui {

// 在资源管理器中定位并高亮 path：存在则打开父目录并选中目标，否则沿父目录回溯
// 到第一个真实存在的文件夹直接打开。内部统一转成本地分隔符，调用方不必预处理。
// ExclusionListWidget / OverlayFilesDialog 共用。
void revealInExplorer(const QString& path);

// QString 边界适配：从完整路径取归一化盘符（"C:"）；取不到（如卷 GUID 路径解析
// 失败）返回空串并把原因写日志。CommitDispatcher / ImportApplier 共用（路由到
// 对应 DiskTab）。盘符逻辑本身全在 uwf::drive。
[[nodiscard]] QString extractDriveLetter(const QString& path);

// 把 combo 的当前项切到 itemData == value 的那项；找不到则不动。两个状态面板共用。
void setComboValue(QComboBox* combo, const QVariant& value);

// 切换控件的 "dirty" 动态属性并重跑 style polish，让 QSS 据此变色。两个状态面板共用。
void markDirty(QWidget* w, bool dirty);

// 当前会话"启用 / 停用"状态文字的富文本：启用 → 绿（Sem::AddOk），停用 →
// 红（Sem::Danger）。塞进 QLabel 即可（设 Qt::RichText）。全局筛选器与单卷
// 保护的当前状态标签共用，配色随主题（rebuildUi 会重新 setData）。
[[nodiscard]] QString enabledStateLabel(bool enabled);

// 把"会话小标题（'本次会话' / '下次会话'）+ 值控件"打包成一张内嵌 mini 卡片
// （statusChip），用卡片边界把本次 / 下次两个状态在视觉上明确分隔。captionTooltip
// 挂在小标题（及卡片留白区）上解释"本次 / 下次会话"含义；值控件自己的 tooltip 仍
// 各管各的。value 会被 reparent 到卡片里。全局筛选器与单卷保护两处共用。
[[nodiscard]] QWidget* makeSessionChip(const QString& caption, const QString& captionTooltip, QWidget* value);

}  // namespace uwf::ui
