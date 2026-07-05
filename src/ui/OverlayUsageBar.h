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

  // RAM / Disk 两种覆盖层类型的统一入口：按 ramMode 自动选定 100% 刻度——
  // RAM 模式以系统物理内存总量为刻度（overlay 上限之外的区域显示为网格），
  // Disk 模式以 overlay 上限为刻度。RAM / Disk 的取值差异只此一处，调用方
  // 不必各自重复判断。等价于 setData(..., ramMode ? systemTotalRamMb() : 0)。
  void setOverlayData(uint32_t currentMb, uint32_t warningMb, uint32_t criticalMb, uint32_t maximumMb, bool ramMode);

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

// 系统物理内存总量（MB）——RAM 覆盖层占用条的 100% 刻度。查询失败返回 0。
[[nodiscard]] uint32_t systemTotalRamMb();

}  // namespace uwf::ui
