// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QMenu>
#include <QPointer>
namespace uwf::ui {
class CommandMenu : public QMenu {
  Q_OBJECT
 public:
  explicit CommandMenu(QWidget* parent = nullptr);
  CommandMenu(const QString& title, QWidget* parent = nullptr);
  void setAnchor(QWidget* anchor);
  void setMaximumVisibleHeight(int height);

 protected:
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;

 private:
  QPointer<QWidget> m_anchor;
  int m_visibleHeight = 0;
  QList<QPointer<QAction>> m_scrollActions;
  QPointer<QAction> m_scrollAction;
};
}  // namespace uwf::ui
