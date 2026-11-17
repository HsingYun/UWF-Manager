// SPDX-License-Identifier: GPL-3.0-or-later
#include "presentation/appearance/WindowAppearance.h"

#include <QMenu>
#include <QWidget>
#include <QWindow>

#include "presentation/appearance/I18n.h"

namespace uwf::ui {
void applyWindowAppearance(QWidget*, Sem) {}
void applyWindowAppearance(QWidget*) {}
void prepareWindowBackdrop(QWidget*) {}
void refreshWindowBackdrop(QWidget*) {}
void startWindowDrag(QWidget* window, const QPoint&) {
  if (window && window->windowHandle()) window->windowHandle()->startSystemMove();
}
bool handleWindowResize(QWidget*, void*, qintptr*) { return false; }
bool handleWindowMenu(QWidget*, void*, qintptr*) { return false; }
void showWindowMenu(QWidget* window, const QPoint& position) {
  QMenu menu(window);
  menu.addAction(I18n::tr("Minimize"), window, &QWidget::showMinimized);
  menu.addAction(I18n::tr("Close"), window, &QWidget::close);
  menu.exec(position);
}
}  // namespace uwf::ui
