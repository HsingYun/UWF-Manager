// SPDX-License-Identifier: GPL-3.0-or-later
#include "presentation/platform/DesktopPlatform.h"

#include <QGuiApplication>
#include <QStyleHints>
#include <QWidget>

namespace uwf::ui::platform {
void configureCaptionButton(QWidget*, CaptionCommand) {}
void invokeCaptionCommand(QWidget* window, const CaptionCommand command) {
  switch (command) {
    case CaptionCommand::Minimize:
      window->showMinimized();
      break;
    case CaptionCommand::Maximize:
      window->isMaximized() ? window->showNormal() : window->showMaximized();
      break;
    case CaptionCommand::Close:
      window->close();
      break;
  }
}
app::Theme systemTheme(bool) noexcept { return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light ? app::Theme::Light : app::Theme::Dark; }
bool isThemeChangeEvent(const QByteArray&, void*) { return false; }
bool animationsEnabled() { return false; }
void revealFile(const QString&) {
  // Preview addresses refer to simulated volumes and have no host file to reveal.
}
}  // namespace uwf::ui::platform
