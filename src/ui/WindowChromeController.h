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

#include <QObject>

class QMainWindow;
class QToolBar;

namespace uwf::ui {

// Windows 平台窗口外观与交互：DWM 标题栏、窗口前置、工具栏空白区域拖动，
// 以及工具栏溢出按钮的矢量雪佛龙绘制。
class WindowChromeController : public QObject {
 public:
  explicit WindowChromeController(QMainWindow* window, QObject* parent = nullptr);

  void applyTitleBarTheme() const;
  void raiseToFront(bool contentInitialized) const;
  void decorateToolbar(QToolBar* toolbar) const;

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  QMainWindow* m_window;
};

}  // namespace uwf::ui
