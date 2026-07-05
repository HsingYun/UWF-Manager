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

#include <QWidget>
#include <optional>

#include "../core/UwfModel.h"

class QLabel;
class QComboBox;
class QSpinBox;
class QScrollArea;

namespace uwf::ui {

class SwitchButton;
class OverlayUsageBar;
class RoundedCornerOverlay;

class GlobalStatusPanel : public QWidget {
  Q_OBJECT
 public:
  explicit GlobalStatusPanel(QWidget* parent = nullptr);

  void setData(const core::SessionSnapshot& current, const core::SessionSnapshot& next, const core::OverlayRuntime& runtime);
  void setUnavailable(const QString& reason);
  void showVolumeInfoWarning(const QString& reason) const;

  // UWF 可读但进程未提权时、在 setData 之后调用：在状态横幅里写一条红色的
  // "需要管理员权限"提示。UWF 不可用时不要调用——那条不可用横幅优先级更高，
  // 由 setUnavailable 负责。
  void showElevationRequired() const;

  // 在 setData 之后调用，决定滚动区内的控件是否可交互。UWF 可读但进程未提权
  // 时用它把控件全部置灰——数据照常显示，但不能改。setUnavailable 已自行置灰，
  // 该路径下再调用为幂等。
  void setControlsEnabled(bool enabled) const;

  // 系统版本不在受支持清单内时调用：在标题下方常驻一条兼容模式警告。
  // 与 setData / setUnavailable 控制的状态横幅相互独立，刷新时不会被清掉。
  void setCompatibilityNotice(const QString& text);

  // 仅刷新覆盖层占用（占用条 + 已用标签）——供 5s 定时器周期更新，不动其他
  // 控件、不触发 pendingChanged。面板处于不可用状态时为空操作。
  void updateUsage(const core::OverlayRuntime& runtime);

  // 滚动内容（筛选器 + 覆盖层两张卡片）不滚动所需的完整高度。滚动区最小高恒为 0、
  // 不向布局上报内容高度，故 MainWindow 用「外壳最小高 + 本值」算窗口下限，使其恰好
  // 容得下整块内容、无横幅时不滚动；有横幅占高放不下时滚动区照常滚动（不被裁切）。
  [[nodiscard]] int preferredContentHeight() const;

  [[nodiscard]] std::optional<bool> pendingFilterEnabled() const;
  [[nodiscard]] core::OverlayConfigDelta pendingOverlay() const;

  // 给"导入 uwfmgr 命令"用：直接拨控件到目标值，触发同样的 pendingChanged 流程。
  // 返回 true = 控件值发生了变化；false = 已是目标值（caller 把这条标"重复"）。
  // 注意：当 UWF 当前会话已启用、type / max 控件在 setData 里被 setEnabled(false)
  // 锁住时，setValue / setCurrentIndex 仍可程序写入并触发 pendingChanged——
  // apply 阶段 WMI 那边再统一拒。这里不在导入侧抢着拒，让用户能看到完整的
  // pending 列表后自行决定是否撤销。
  bool importFilterEnabled(bool v);
  bool importOverlayType(core::OverlayType t);
  bool importOverlayMaxMb(uint32_t mb);
  bool importOverlayWarnMb(uint32_t mb);
  bool importOverlayCritMb(uint32_t mb);

  // 一批 import* 调用结束后调用。import* 用 setValue 写入，只触发 valueChanged、
  // 不触发 editingFinished，约束链不会自动收紧、range 也停在导入时放宽的状态。
  // 这里补跑一次 reconfigureRanges：收紧 range 并重建 warn ≤ crit ≤ max，让面板
  // 回到自洽状态，避免之后任意一次无关交互触发收紧时静默改写导入值。
  void finishImport() const;

 signals:
  void pendingChanged();

 protected:
  // 监听滚动区 viewport 的 Resize，让圆角遮罩层（m_cornerOverlay）跟着改大小。
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  void emitIfChanged();
  void updateDirtyStyle();

  // 按"约束向下传递"重算三个阈值 spinbox 的 range：max 的上限来自
  // RAM/磁盘语义，crit 上限 < max 当前值，warn 上限 < crit 当前值。
  // setRange 会把超出新区间的现值自动钳回边界，实现联动下压。
  void reconfigureRanges() const;
  // 根据当前类型更新 "最大大小" 标签后缀与进度条 scale。
  void refreshTypeDependentUi();
  // 主题切换时刷新 pixmap、RichText 等含主题色的元素。
  void applyTheme() const;

  QLabel* m_banner = nullptr;
  // 兼容模式警告横幅——独立于 m_banner，一经显示便常驻。
  QLabel* m_compatBanner = nullptr;
  // 滚动区本体 + 内容宿主。UWF 不可用时只禁用宿主（而非整个面板），让
  // QScrollArea 本身保持可用，内部控件变灰但仍能滚动查看。
  QScrollArea* m_scroll = nullptr;
  QWidget* m_scrollHost = nullptr;
  // 盖在 viewport 上、抗锯齿补圆角的遮罩层（卡片滚到边缘时仍保持圆角）。
  RoundedCornerOverlay* m_cornerOverlay = nullptr;

  QLabel* m_filterCur = nullptr;
  SwitchButton* m_filterNext = nullptr;

  QComboBox* m_overlayTypeNext = nullptr;
  QSpinBox* m_maxNext = nullptr;
  QSpinBox* m_warnNext = nullptr;
  QSpinBox* m_critNext = nullptr;
  QLabel* m_maxLabel = nullptr;
  QLabel* m_usedLabel = nullptr;
  // "?" 锁定提示：UWF 当前会话启用时显示，hover 提示要先禁用 UWF 才能改。
  QLabel* m_typeLockedHint = nullptr;
  QLabel* m_maxLockedHint = nullptr;
  // 主题切换时需要重设 RichText 的图例 QLabel。
  QLabel* m_overlayLegend = nullptr;
  OverlayUsageBar* m_usageBar = nullptr;

  core::FilterState m_baselineFilter;
  core::OverlayConfig m_baselineOverlay;
  uint32_t m_totalRamMb = 0;
  uint32_t m_currentConsumptionMb = 0;
  bool m_available = true;
};

}  // namespace uwf::ui
