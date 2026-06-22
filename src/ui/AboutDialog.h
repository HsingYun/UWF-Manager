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

// "About UWF Manager" 对话框：logo + 标题 + 版本号 + GPL 说明 + UWF 依赖说明。
// 从 MainWindow::showAbout 拆出来——版面 + HTML 字符串占 100 行，没有业务逻辑，
// 留在主窗口里只是噪音。MainWindow::showAbout 现在退化成两行 dlg.exec()。

#include <QDialog>

namespace uwf::ui {

class AboutDialog : public QDialog {
  Q_OBJECT
 public:
  explicit AboutDialog(QWidget* parent = nullptr);
};

}  // namespace uwf::ui
