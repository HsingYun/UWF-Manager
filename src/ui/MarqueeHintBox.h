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

// QTextBrowser 子类：内容超出固定可视高度时，自动在垂直方向做"循环来回滚动"
// （停在顶部 → 缓慢滚到底部 → 停在底部 → 缓慢滚回顶部 → 周而复始），让放不下的
// 提示文字也能被完整看到；内容能放下时保持静止。
//
// 缘由：右下角悬停提示框（MainWindow::m_hoverHint）固定高 110px、关掉了滚动条，
// 英文语言下个别条目（域机密密钥 / TSCAL 的默认排除说明）文字偏长，超出部分被裁
// 掉看不全。本类只管"溢出就自动滚动"这一件事——文本仍由外部经 TransientLabel
// 通过 setText 驱动，对调用方透明：把创建处的 QTextBrowser 换成本类即可，其余
// 配置（objectName / 换行 / 关滚动条 / 只读 / QSS 外观）都不变。

#include <QTextBrowser>

class QResizeEvent;
class QTimer;

namespace uwf::ui {

class MarqueeHintBox : public QTextBrowser {
  Q_OBJECT
 public:
  explicit MarqueeHintBox(QWidget* parent = nullptr);

 protected:
  void resizeEvent(QResizeEvent* e) override;

 private:
  // 文本或尺寸变化后重新判定溢出：复位到顶部，溢出则启动滚动、放得下则停表静止。
  // 延后到下一轮事件循环执行——setText 触发的 textChanged 发出时文档尚未按视口
  // 宽度排版完成，此刻读 scrollbar 的 maximum() 还不准。
  void scheduleReevaluate();
  void reevaluate();
  // 定时器一拍：按当前阶段推进滚动条 / 递减停顿计数。
  void tick();

  // 顶部停顿 → 向下滚 → 底部停顿 → 向上滚，四相循环。
  enum class Phase { HoldTop, ScrollDown, HoldBottom, ScrollUp };

  QTimer* m_timer;
  Phase m_phase = Phase::HoldTop;
  int m_holdTicks = 0;          // 停顿阶段剩余的 tick 数
  bool m_reevalQueued = false;  // 去重：一轮事件循环内只排一次 reevaluate
};

}  // namespace uwf::ui
