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

#include "../uwf/api/UwfFilter.h"

class QWidget;

namespace uwf::ui {

// UWF 安全关机 / 重启用例：统一二次确认、Filter 查询、WMI 调用和失败提示。
// 工具栏与应用完成对话框直接连接这里，避免维护多份入口逻辑。
class PowerController : public QObject {
  Q_OBJECT
 public:
  PowerController(WmiSession& session, QWidget* dialogParent, QObject* parent = nullptr);

 public slots:
  void safeShutdown();
  void safeRestart();

 private:
  QWidget* m_dialogParent;
  api::UwfFilter m_filter;
};

}  // namespace uwf::ui
