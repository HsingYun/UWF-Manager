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
#include "MarqueeHintBox.h"

#include <QScrollBar>
#include <QTimer>

namespace uwf::ui {

namespace {
// ~33fps 推进，够顺滑又不费 CPU；每拍滚 1px ≈ 33px/s，慢到能逐行读清。
// 顶 / 底各停约 1.4s，给眼睛留出读首尾行的时间。
constexpr int kTickMs = 30;
constexpr int kStepPx = 1;
constexpr int kHoldTicks = 1400 / kTickMs;
}  // namespace

MarqueeHintBox::MarqueeHintBox(QWidget* parent) : QTextBrowser(parent), m_timer(new QTimer(this)) {
  m_timer->setInterval(kTickMs);
  connect(m_timer, &QTimer::timeout, this, &MarqueeHintBox::tick);
  // 文本任何变化都重判溢出：基线与悬停提示互相覆盖（setText）都会发 textChanged。
  // TransientLabel 已对"文本未变"去重，这里收到的都是真正的内容切换，可放心复位。
  connect(this, &QTextEdit::textChanged, this, &MarqueeHintBox::scheduleReevaluate);
}

void MarqueeHintBox::resizeEvent(QResizeEvent* e) {
  QTextBrowser::resizeEvent(e);
  // 宽度变了换行结果随之变、溢出量也变——重判一次。（本框宽高其实都固定，这里
  // 只为稳妥兜住首次布局等边角情况。）
  scheduleReevaluate();
}

void MarqueeHintBox::scheduleReevaluate() {
  if (m_reevalQueued) return;
  m_reevalQueued = true;
  QTimer::singleShot(0, this, &MarqueeHintBox::reevaluate);
}

void MarqueeHintBox::reevaluate() {
  m_reevalQueued = false;
  auto* bar = verticalScrollBar();
  // 关掉滚动条（ScrollBarAlwaysOff）不影响 range / setValue：scrollbar 对象仍按
  // 文档高度维护范围，maximum() > 0 即表示有内容被裁在可视区外。
  bar->setValue(0);  // 新内容一律从顶部开始读
  if (bar->maximum() <= 0) {
    m_timer->stop();  // 放得下：静止
    return;
  }
  m_phase = Phase::HoldTop;
  m_holdTicks = kHoldTicks;
  if (!m_timer->isActive()) m_timer->start();
}

void MarqueeHintBox::tick() {
  auto* bar = verticalScrollBar();
  const int max = bar->maximum();
  if (max <= 0) {  // 内容已缩短到放得下
    m_timer->stop();
    return;
  }
  switch (m_phase) {
    case Phase::HoldTop:
      if (--m_holdTicks <= 0) m_phase = Phase::ScrollDown;
      break;
    case Phase::ScrollDown: {
      const int v = bar->value() + kStepPx;
      if (v >= max) {
        bar->setValue(max);
        m_phase = Phase::HoldBottom;
        m_holdTicks = kHoldTicks;
      } else {
        bar->setValue(v);
      }
      break;
    }
    case Phase::HoldBottom:
      if (--m_holdTicks <= 0) m_phase = Phase::ScrollUp;
      break;
    case Phase::ScrollUp: {
      const int v = bar->value() - kStepPx;
      if (v <= 0) {
        bar->setValue(0);
        m_phase = Phase::HoldTop;
        m_holdTicks = kHoldTicks;
      } else {
        bar->setValue(v);
      }
      break;
    }
  }
}

}  // namespace uwf::ui
