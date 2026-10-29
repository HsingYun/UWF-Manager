// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QColor>
#include <QWidget>
#include <functional>

namespace uwf::ui {

// 通过透明绘制层补全滚动容器圆角；QSS 不裁剪内容，QRegion 掩码不支持抗锯齿。
// 宿主为 viewport 或容器；尺寸变化后同步几何，滚动时重绘，鼠标事件穿透。
class RoundedCornerOverlay : public QWidget {
 public:
  // inset 和 radius 使用逻辑像素。colorFn 提供圆角外部背景色；borderFn 与 borderWidth 可选。
  // edgesFn 指定需补绘的边及相邻圆角；为空时绘制完整边框。
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
