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

#include "ThemeManager.h"

namespace uwf::ui {

struct OverlayHudPalette {
  QColor floatingSurface;
  QColor taskbarSurface;
  QColor progressFill;
  QColor text;
  QColor border;
  QColor handleFill;
  QColor handleIcon;
};

// HUD 覆盖的是桌面和任务栏的任意内容，不能直接复用主窗口的纯色卡片配色。
// 两套颜色均保留透明度：Dark 用亮色轮廓维持暗背景层次；Light 使用偏冷的
// 浅色面板、深色文字和深色轮廓，避免在白色任务栏上消失。
[[nodiscard]] inline OverlayHudPalette overlayHudPalette(const Theme theme) {
  if (theme == Theme::Light) {
    return {
        QColor(0xF4, 0xF7, 0xFB, 180),  // floatingSurface: 71%
        QColor(0xF4, 0xF7, 0xFB, 218),  // taskbarSurface: 85%
        QColor(0x00, 0x67, 0xC0, 105),  // progressFill: 41%
        QColor(0x17, 0x20, 0x33, 242),  // text: 95%
        QColor(0x15, 0x25, 0x36, 55),   // border: 22%
        QColor(0x10, 0x20, 0x30, 22),   // handleFill: 9%
        QColor(0x1D, 0x33, 0x48, 180),  // handleIcon: 71%
    };
  }

  return {
      QColor(0x17, 0x1A, 0x20, 163),  // floatingSurface: 64%
      QColor(0x17, 0x1A, 0x20, 210),  // taskbarSurface: 82%
      QColor(0x00, 0x78, 0xD4, 120),  // progressFill: 47%
      QColor(0xF5, 0xF7, 0xFA, 240),  // text: 94%
      QColor(0xFF, 0xFF, 0xFF, 46),   // border: 18%
      QColor(0xFF, 0xFF, 0xFF, 26),   // handleFill: 10%
      QColor(0xFF, 0xFF, 0xFF, 173),  // handleIcon: 68%
  };
}

}  // namespace uwf::ui
