#pragma once

#include <QWidget>
#include <optional>

#include "../core/UwfConfig.h"

class QLabel;
class QComboBox;
class QSpinBox;

namespace uwf::ui {

class SwitchButton;
class OverlayUsageBar;

class GlobalStatusPanel : public QWidget {
  Q_OBJECT
 public:
  explicit GlobalStatusPanel(QWidget* parent = nullptr);

  void setData(const core::SessionSnapshot& current, const core::SessionSnapshot& next, const core::OverlayRuntime& runtime);
  void setUnavailable(const QString& reason);

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

 signals:
  void pendingChanged();

 private:
  void emitIfChanged();
  void updateDirtyStyle();

  // 按"约束向下传递"重算三个阈值 spinbox 的 range：max 的上限来自
  // RAM/磁盘语义，crit 上限 = max 当前值，warn 上限 = crit 当前值。
  // setRange 会把超出新区间的现值自动钳回边界，实现联动下压。
  void reconfigureRanges() const;
  // 根据当前类型更新 "最大大小" 标签后缀与进度条 scale。
  void refreshTypeDependentUi();
  // 主题切换时刷新 pixmap、RichText 等含主题色的元素。
  void applyTheme() const;

  QLabel* m_banner = nullptr;

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
  // 主题切换时需要重设 pixmap / RichText 的两个 QLabel。
  QLabel* m_filterArrow = nullptr;
  QLabel* m_overlayLegend = nullptr;
  OverlayUsageBar* m_usageBar = nullptr;

  core::FilterState m_baselineFilter;
  core::OverlayConfig m_baselineOverlay;
  uint32_t m_totalRamMb = 0;
  uint32_t m_currentConsumptionMb = 0;
  bool m_available = true;
};

}  // namespace uwf::ui
