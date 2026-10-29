// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QtTypes>

class QWidget;
class QPoint;

namespace uwf::ui {

enum class Sem;
void applyWindowAppearance(QWidget* window, Sem caption);
void applyWindowAppearance(QWidget* window);
// Prepare before HWND creation. Application windows/dialogs opt in;
// menus, tooltips and the HUB retain their own surfaces.
void prepareWindowBackdrop(QWidget* window);
void refreshWindowBackdrop(QWidget* window);
void startWindowDrag(QWidget* window, const QPoint& globalPosition);
bool handleWindowResize(QWidget* window, void* message, qintptr* result);
void showWindowMenu(QWidget* window, const QPoint& position);
bool handleWindowMenu(QWidget* window, void* message, qintptr* result);

}  // namespace uwf::ui
