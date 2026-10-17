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

#include <QtTest/qtestspontaneevent.h>

#include <QApplication>
#include <QTest>
#include <QWheelEvent>
#include <QWidget>
#include <QWindow>

namespace uwf::test {

// Qt 6.11 的 QTest::wheelEvent 把逻辑坐标直接交给接收原生像素的接口，
// 高 DPI 下会投递到错误位置。沿用 QtTest 的 spontaneous 事件机制，
// 从真实窗口分发逻辑坐标，保留命中测试、控件处理及父链传播，不直接操作滚动条。
inline void sendWheel(QWidget& target, const QPoint angleDelta) {
  auto* window = target.window();
  QVERIFY(window->isVisible() && window->windowHandle());
  const QPoint position = target.mapTo(window, target.rect().center());
  QWheelEvent event(position, window->mapToGlobal(position), QPoint(), angleDelta, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
  QSpontaneKeyEvent::setSpontaneous(&event);
  qApp->notify(window->windowHandle(), &event);
  QCoreApplication::processEvents();
}

}  // namespace uwf::test
