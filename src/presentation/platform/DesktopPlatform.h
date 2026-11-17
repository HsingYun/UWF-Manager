// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QByteArray>
#include <QString>

#include "application/settings/UiPreferences.h"
class QWidget;

namespace uwf::ui::platform {
enum class CaptionCommand { Minimize, Maximize, Close };
void configureCaptionButton(QWidget* button, CaptionCommand command);
void invokeCaptionCommand(QWidget* window, CaptionCommand command);
app::Theme systemTheme(bool shell) noexcept;
bool isThemeChangeEvent(const QByteArray& eventType, void* message);
bool animationsEnabled();
void revealFile(const QString& path);
}  // namespace uwf::ui::platform
