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

#include <QProxyStyle>

namespace uwf::ui {

// Qt 默认 QStyle 把 TextBesideIcon 的 QToolButton 和 QTabBar::tab
// 文字按"基线"对齐，视觉上文字会明显低于图标几何中心。
// 这里在样式层面把按钮类文字向上平移 1px，让 icon / 文字在肉眼看起来居中。
// 其他场景（QLabel 自绘、QLineEdit 等）并不经 drawItemText，所以不受影响。
//
// TAB 标签的情况稍特殊：Windows 样式插件绘制 CE_TabBarTabLabel 时，基线
// 偏移比普通按钮大一些，仅靠 drawItemText 的 -1 抬升不够。这里额外 override
// drawControl，把 CE_TabBarTabLabel 的 opt.rect 先整体向上挪 1px，再叠加
// drawItemText 自己的 -1，让 TAB 的文字看起来真正居中。
class CenteredTextStyle : public QProxyStyle {
 public:
  using QProxyStyle::QProxyStyle;

  void drawItemText(QPainter* painter, const QRect& rect, int flags, const QPalette& pal, bool enabled, const QString& text,
                    QPalette::ColorRole textRole) const override;

  void drawControl(ControlElement ce, const QStyleOption* opt, QPainter* p, const QWidget* w) const override;

  void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const override;
};

}  // namespace uwf::ui
