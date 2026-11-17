// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QJsonObject>
#include <QWidget>
#include <memory>

#include "application/overlay/contracts/OverlayUsage.h"
#include "composition/desktop/DesktopPlatformPorts.h"

namespace uwf::preview::host {
bool defaultInteractive();
composition::DesktopPlatformPorts desktopPorts();
void activateForCapture(QWidget* root, QWidget* background, bool topmost);
void setCaptionStyle(QWidget* root, const QString& style);
bool isForeground(QWidget* root);
bool isMaximizeButton(QWidget* button);
QJsonObject materialProperties(QWidget* root);
void flushCompositor();
std::unique_ptr<QWidget> overlayComponent(const QString& component, const app::OverlayUsageEnabled& usage);
bool taskbarPresented();
}  // namespace uwf::preview::host
