// SPDX-License-Identifier: GPL-3.0-or-later
#include "presentation/platform/DesktopPlatform.h"

#include <windows.h>

#include <QVariant>
#include <QWidget>
#include <exception>

#include "foundation/logging/Log.h"
#include "infrastructure/windows/registry/RegistryKey.h"
#include "infrastructure/windows/shell/WindowsShell.h"

namespace uwf::ui::platform {
void configureCaptionButton(QWidget* button, const CaptionCommand command) {
  button->setProperty("nativeCaptionHit", command == CaptionCommand::Minimize ? HTMINBUTTON : command == CaptionCommand::Maximize ? HTMAXBUTTON : HTCLOSE);
}
void invokeCaptionCommand(QWidget* window, const CaptionCommand command) {
  switch (command) {
    case CaptionCommand::Minimize:
      SendMessageW(reinterpret_cast<HWND>(window->winId()), WM_SYSCOMMAND, SC_MINIMIZE, 0);
      break;
    case CaptionCommand::Maximize:
      SendMessageW(reinterpret_cast<HWND>(window->winId()), WM_SYSCOMMAND, window->isMaximized() ? SC_RESTORE : SC_MAXIMIZE, 0);
      break;
    case CaptionCommand::Close:
      SendMessageW(reinterpret_cast<HWND>(window->winId()), WM_SYSCOMMAND, SC_CLOSE, 0);
      break;
  }
}
app::Theme systemTheme(const bool shell) noexcept {
  try {
    return regkey::readDword(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)",
                             shell ? "SystemUsesLightTheme" : "AppsUseLightTheme")
                       .value_or(0) == 0
               ? app::Theme::Dark
               : app::Theme::Light;
  } catch (const std::exception& error) {
    UWF_LOG_W("theme") << "system theme unavailable: fallback=dark error=" << error.what();
  } catch (...) {
    UWF_LOG_W("theme") << "system theme unavailable: fallback=dark error=non-standard-exception";
  }
  return app::Theme::Dark;
}
bool isThemeChangeEvent(const QByteArray& eventType, void* message) {
  if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG") return false;
  const auto* msg = static_cast<const MSG*>(message);
  return msg && (msg->message == WM_SETTINGCHANGE || msg->message == WM_THEMECHANGED);
}
bool animationsEnabled() {
  BOOL enabled = TRUE;
  return !SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &enabled, 0) || enabled;
}
void revealFile(const QString& path) { windows::shell::revealInExplorer(path); }
}  // namespace uwf::ui::platform
