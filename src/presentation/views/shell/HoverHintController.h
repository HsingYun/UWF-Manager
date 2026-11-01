// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QPointer>
#include <QTimer>

#include "presentation/controls/feedback/TransientLabel.h"

class QWidget;

namespace uwf::ui {

// Forwards delayed tooltips from the main window to its status surface.
// Dialog tooltips remain local; item views, tabs and menus use hit testing.
class HoverHintController : public QObject {
 public:
  explicit HoverHintController(QWidget* rootWindow, QObject* parent = nullptr);

  // The target may be rebound after the window rebuilds its status surface.
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
