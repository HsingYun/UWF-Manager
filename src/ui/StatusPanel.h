#pragma once

#include <QWidget>
#include <optional>

#include "../core/UwfModel.h"

class QLabel;
class QComboBox;
class QHBoxLayout;

namespace uwf::ui {

class SwitchButton;

class StatusPanel : public QWidget {
  Q_OBJECT
 public:
  explicit StatusPanel(QWidget* parent = nullptr);

  void setData(const core::VolumeRecord* currentVol, const core::VolumeRecord* nextVol);

  // 在 setData 之后调用，决定保护开关 / 绑定方式是否可交互。enabled=false
  // （UWF 不可用或进程未提权）时二者照常显示当前值但置灰不可改。无卷数据时
  // 二者本就禁用，此调用不会把它们点亮。
  void setControlsEnabled(bool enabled) const;

  void setUnsupported(const QString& reason);
  // 比 setUnsupported 弱：只显示一条警告横幅，不灰化任何控件。
  // 用于"卷可保护但有局部限制"的情况（如 exFAT 不支持文件排除）。
  void setNotice(const QString& text);

  // 把额外的控件（例如"持久化文件/目录/注册表修改"按钮）挂到保护状态那一行
  // 的右侧。调用方必须自己管控按钮生命周期与 enable 状态。
  void addTrailingAction(QWidget* w) const;

  [[nodiscard]] std::optional<bool> pendingVolumeProtected() const;
  // 返回 bBindByVolumeName 值（true=按卷 ID，false=按盘符）。
  [[nodiscard]] std::optional<bool> pendingBindByVolumeName() const;

  // 给"导入 uwfmgr 命令"用：直接拨控件到目标值，触发同样的 pendingChanged 流程。
  // 返回 true = 控件值发生了变化；false = 已是目标值，未变更（caller 标"重复"）。
  bool importProtect(bool v);
  bool importBindByVolumeName(bool v);

 signals:
  void pendingChanged();

 private:
  void emitIfChanged();
  void updateDirtyStyle();

  QLabel* m_banner = nullptr;

  QHBoxLayout* m_row = nullptr;  // 保护状态/绑定方式那一行的布局
  QLabel* m_protectCur = nullptr;
  SwitchButton* m_protectNext = nullptr;
  QComboBox* m_bindNext = nullptr;

  bool m_baselineProtect = false;
  bool m_baselineBindByVolumeName = false;  // !bindByDriveLetter
  bool m_hasVolume = false;
  bool m_supported = true;
  // setNotice 设了一行常驻横幅（区别于 setUnsupported 的"卷错误"横幅）。
  // setData 默认会 hide banner 让正常状态干净，但 sticky notice 例外。
  bool m_hasStickyNotice = false;
};

}  // namespace uwf::ui
