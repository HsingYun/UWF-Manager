// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPoint>
class QWidget;
class QWindow;

namespace uwf::test {

void sendWheel(QWidget& target, QPoint angleDelta);
void moveMouse(QWindow* window, QPoint position = {}, int delay = -1);
void moveMouse(QWidget& target, QPoint position = {}, int delay = -1);
inline void moveMouse(QWidget* target, const QPoint position = {}, const int delay = -1) { moveMouse(*target, position, delay); }

}  // namespace uwf::test
