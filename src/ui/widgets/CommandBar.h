// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "Surface.h"
class QAction;
class QHBoxLayout;
class QToolButton;
namespace uwf::ui {
class CommandBar final : public Surface {
  Q_OBJECT
 public:
  explicit CommandBar(QWidget* parent = nullptr);
  QAction* addCommand(const QString& text, bool optional = false);
  void addSeparator();
  void addStretch();
  [[nodiscard]] QToolButton* buttonForAction(QAction* action) const;

 protected:
  void resizeEvent(QResizeEvent*) override;

 private:
  void arrange();
  QHBoxLayout* m_layout;
  QList<QToolButton*> m_buttons;
};
}  // namespace uwf::ui
