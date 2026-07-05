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

#include <QColor>
#include <QWidget>
#include <functional>

namespace uwf::ui {

// 盖在某个滚动 viewport 之上、抗锯齿补出圆角的兄弟遮罩层。
//
// 背景：Qt QSS 的 border-radius 不会裁剪滚动内容，内容滚到 viewport 边缘时圆角会被
// 切成直角；而 QWidget::setMask 用的 QRegion 是 1bit、圆角发毛、无法抗锯齿。本类改用
// 一个「不清自身底色」的兄弟层盖在 viewport 上，只在四角用背景色把「圆角矩形之外」那
// 一小块补平——直边由 viewport 的矩形裁剪天然得到（直线无需抗锯齿），四角则由 QPainter
// 开抗锯齿绘制，于是滚动中始终是平滑圆角。
//
// 用法：以要补圆角的宿主（viewport 或带圆角边框的容器本身）为 parent 构造；宿主尺寸
// 变化时调 syncToParent()；内容滚动时调 update()（把滚动条的 valueChanged 接到 update
// 即可）。点击穿透、不挡下层交互。
//
// 两种典型用法：
//  - 内容浮在纯色背景上、viewport 无边框（如卡片）：parent = viewport，colorFn 给背景色，
//    无需描边。
//  - 宿主自身是带 1px 圆角边框的容器（如列表）：parent = 容器本身，colorFn 给容器四周的
//    底色（盖住溢出到圆角外的内容），再给 borderFn + borderWidth 把那条圆角边框抗锯齿
//    地重描一遍——盖住溢进边框环里的内容，圆角边框因此始终平滑完整。
class RoundedCornerOverlay : public QWidget {
 public:
  // inset：圆角矩形相对宿主左右各内缩的像素——内容四周留边距时传该边距（如卡片的 6），
  //   铺满宿主时传 0（如列表）。radius：圆角半径。colorFn：每次重绘时取色，返回圆角矩形
  //   之外要补的底色（须与该处身后应有的颜色一致；用回调以支持随主题变化）。borderFn /
  //   borderWidth：可选，非空且宽度>0 时沿圆角描该色边框。edgesFn：可选，每次重绘时返回
  //   要描的边（含相邻圆角）——用于只补上/下边（滚动时被裁掉的那条）、且仅在该边确有内容
  //   被裁时才画；为空（默认）时描完整一圈（带边框容器用）。
  RoundedCornerOverlay(QWidget* host, qreal inset, qreal radius, std::function<QColor()> colorFn, std::function<QColor()> borderFn = {}, qreal borderWidth = 0,
                       std::function<Qt::Edges()> edgesFn = {});

  // 铺满 parent 宿主并置顶。宿主 Resize 后调用。
  void syncToParent();

 protected:
  void paintEvent(QPaintEvent* ev) override;

 private:
  qreal m_inset;
  qreal m_radius;
  std::function<QColor()> m_colorFn;
  std::function<QColor()> m_borderFn;
  qreal m_borderWidth;
  std::function<Qt::Edges()> m_edgesFn;
};

}  // namespace uwf::ui
