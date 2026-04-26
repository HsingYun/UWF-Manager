#pragma once

#include <QWidget>
#include <cstdint>

namespace uwf::ui {

// 横向柱状图，直观展示 overlay 占用 vs 警告/严重阈值 vs 总大小。
// 单位统一用 MB，由调用方换算好。
// 颜色约定：
//   - 深灰底色：总大小（最大容量）
//   - 蓝色填充：当前已占用
//   - 橙色竖线：警告阈值
//   - 红色竖线：严重阈值
class OverlayUsageBar : public QWidget {
  Q_OBJECT
 public:
  explicit OverlayUsageBar(QWidget* parent = nullptr);

  // scaleMb 为 100% 对应的数值；传 0 表示使用 maximumMb。
  // 这样 RAM 类型可以把 100% 设为总内存，而 Disk 类型则使用 overlay max。
  void setData(uint32_t currentMb, uint32_t warningMb, uint32_t criticalMb, uint32_t maximumMb, uint32_t scaleMb = 0);

  [[nodiscard]] QSize sizeHint() const override;
  [[nodiscard]] QSize minimumSizeHint() const override;

 protected:
  void paintEvent(QPaintEvent*) override;

 private:
  uint32_t m_current = 0;
  uint32_t m_warning = 0;
  uint32_t m_critical = 0;
  uint32_t m_max = 0;
  uint32_t m_scale = 0;
};

}  // namespace uwf::ui
