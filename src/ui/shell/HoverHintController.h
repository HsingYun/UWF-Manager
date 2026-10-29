// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QPointer>
#include <QTimer>

#include "ui/widgets/TransientLabel.h"

class QWidget;

namespace uwf::ui {

// 把主窗口所有子控件的 tooltip / item tooltip 延迟转发到状态栏。
// 对话框仍使用原生 tooltip；菜单、TabBar 和 Item View 做各自的命中解析。
class HoverHintController : public QObject {
 public:
  explicit HoverHintController(QWidget* rootWindow, QObject* parent = nullptr);

  // rebuildUi 会销毁并重建提示控件；每轮构建后重新绑定目标即可。
  void setTarget(TransientLabel* target);

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;

 private:
  void queueHint(QWidget* source, const QString& text);
  void clearHint();
  QPointer<QWidget> m_rootWindow;
  QPointer<TransientLabel> m_target;
  QPointer<QWidget> m_source;
  QString m_hint;
  QTimer m_delay{this};
};

}  // namespace uwf::ui
