// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPoint>
#include <QString>
class QWidget;
class QWindow;

namespace uwf::test {

bool prepareClipboardText(const QString& text);
// Verify a distinct initial receipt, then retry the real shortcut until Windows
// transfers the selection. Never seed the expected copied text.
bool copySelectionToClipboard(QWidget& target, const QString& expectedText);
void sendWheel(QWidget& target, QPoint angleDelta);
// Native pointer input requires Windows QPA and UWF_TEST_ALLOW_DESKTOP=1.
// Unsupported platforms and failed tests submit no further gestures.
void moveMouse(QWindow* window, QPoint position = {}, int delay = -1);
void moveMouse(QWidget& target, QPoint position = {}, int delay = -1);
void moveMouse(QWidget* target, QPoint position = {}, int delay = -1);
// Use these helpers for complete native gestures; QTest button injection does not establish native capture.
// Await native/Qt input receipts, then let callers assert the resulting action.
// A default point follows the center; explicit nonzero points retain local coordinates.
// Target retirement ends input and its independent-press timing.
// An idle move may leave/re-enter once; existing capture permits this only for the active target popup with no held buttons.
void clickMouse(QWindow* target, Qt::MouseButton button, Qt::KeyboardModifiers modifiers = Qt::NoModifier, QPoint position = {}, int delay = -1);
void clickMouse(QWidget* target, Qt::MouseButton button, Qt::KeyboardModifiers modifiers = Qt::NoModifier, QPoint position = {}, int delay = -1);
void pressMouse(QWindow* target, Qt::MouseButton button, Qt::KeyboardModifiers modifiers = Qt::NoModifier, QPoint position = {}, int delay = -1);
void pressMouse(QWidget* target, Qt::MouseButton button, Qt::KeyboardModifiers modifiers = Qt::NoModifier, QPoint position = {}, int delay = -1);
void releaseMouse(QWindow* target, Qt::MouseButton button, Qt::KeyboardModifiers modifiers = Qt::NoModifier, QPoint position = {}, int delay = -1);
void releaseMouse(QWidget* target, Qt::MouseButton button, Qt::KeyboardModifiers modifiers = Qt::NoModifier, QPoint position = {}, int delay = -1);

}  // namespace uwf::test
