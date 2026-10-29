// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QList>

#include "ui/widgets/Surface.h"
class QAction;
class QHBoxLayout;
namespace uwf::ui {
class WindowTitleBar final : public Surface {
 public:
  explicit WindowTitleBar(QWidget* parent);
  void addUtility(QAction* action);

 private:
  QHBoxLayout* m_utilities;
  QList<QWidget*> m_focusOrder;
  QList<QWidget*> m_windowCommands;
};
}  // namespace uwf::ui
