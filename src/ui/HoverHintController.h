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
#include <QPointer>

#include "TransientLabel.h"

class QWidget;

namespace uwf::ui {

// 把主窗口所有子控件的 tooltip / item tooltip 转发到右下角提示面板。
// 对话框仍使用原生 tooltip；菜单、TabBar 和 Item View 做各自的命中解析。
class HoverHintController : public QObject {
 public:
  explicit HoverHintController(QWidget* rootWindow, QObject* parent = nullptr);

  // rebuildUi 会销毁并重建提示控件；每轮构建后重新绑定目标即可。
  void setTarget(TransientLabel* target) { m_target = target; }

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  QWidget* m_rootWindow;
  QPointer<TransientLabel> m_target;
};

}  // namespace uwf::ui
