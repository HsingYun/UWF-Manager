// SPDX-License-Identifier: GPL-3.0-or-later
#include "../PreviewHost.h"

#include "composition/desktop/WindowsDesktopComposition.h"
// clang-format off
#include <qt_windows.h>
#include <dwmapi.h>
// clang-format on
#include <QApplication>
#include <QWindow>

#include "presentation/views/overlay/hud/OverlayFloatingWidget.h"
#include "presentation/views/overlay/hud/OverlayTaskbarWidget.h"

namespace uwf::preview::host {
bool defaultInteractive() { return false; }
composition::DesktopPlatformPorts desktopPorts() { return composition::systemDesktopPlatformPorts(); }
void activateForCapture(QWidget* root, QWidget* background, const bool topmost) {
  SetForegroundWindow(reinterpret_cast<HWND>(root->winId()));
  if (topmost) {
    if (background) SetWindowPos(reinterpret_cast<HWND>(background->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(reinterpret_cast<HWND>(root->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  }
}
void setCaptionStyle(QWidget* root, const QString& style) {
  const auto handle = reinterpret_cast<HWND>(root->winId());
  auto styles = GetWindowLongPtrW(handle, GWL_STYLE) | WS_SYSMENU;
  if (style == QLatin1String("frameless-menu")) styles &= ~WS_CAPTION;
  SetWindowLongPtrW(handle, GWL_STYLE, styles);
  SetWindowPos(handle, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}
bool isForeground(QWidget* root) { return GetForegroundWindow() == reinterpret_cast<HWND>(root->winId()); }
bool isMaximizeButton(QWidget* button) { return button->property("nativeCaptionHit").toInt() == HTMAXBUTTON; }
QJsonObject materialProperties(QWidget* root) {
  const auto native = reinterpret_cast<HWND>(root->winId());
  int backdrop = -1;
  const auto status = DwmGetWindowAttribute(native, 38, &backdrop, sizeof(backdrop));
  BOOL dark = FALSE;
  const auto darkStatus = DwmGetWindowAttribute(native, 20, &dark, sizeof(dark));
  return {{QStringLiteral("requested"), root->property("systemBackdropRequested").toBool()},
          {QStringLiteral("active"), root->property("systemBackdropActive").toBool()},
          {QStringLiteral("qtActive"), root->isActiveWindow()},
          {QStringLiteral("foreground"), GetForegroundWindow() == native},
          {QStringLiteral("dwmQuerySucceeded"), SUCCEEDED(status)},
          {QStringLiteral("dwmBackdropType"), backdrop},
          {QStringLiteral("dwmDarkQuerySucceeded"), SUCCEEDED(darkStatus)},
          {QStringLiteral("dwmDark"), dark != FALSE},
          {QStringLiteral("layered"), (GetWindowLongPtrW(native, GWL_EXSTYLE) & WS_EX_LAYERED) != 0},
          {QStringLiteral("nativeStyle"), QString::number(GetWindowLongPtrW(native, GWL_STYLE), 16)},
          {QStringLiteral("remoteSession"), GetSystemMetrics(SM_REMOTESESSION) != 0},
          {QStringLiteral("alphaBuffer"), root->windowHandle()->format().hasAlpha()}};
}
void flushCompositor() { DwmFlush(); }
std::unique_ptr<QWidget> overlayComponent(const QString& component, const app::OverlayUsageEnabled& usage) {
  if (component == QLatin1String("floating")) {
    auto widget = std::make_unique<ui::OverlayFloatingWidget>();
    widget->applyUsageState(usage);
    return widget;
  }
  if (component == QLatin1String("taskbar")) {
    auto widget = std::make_unique<ui::OverlayTaskbarWidget>();
    widget->applyUsageState(usage);
    return widget;
  }
  return {};
}
bool taskbarPresented() {
  for (auto widgets = QApplication::allWidgets(); const auto* widget : std::as_const(widgets)) {
    const auto* taskbar = qobject_cast<const ui::OverlayTaskbarWidget*>(widget);
    if (taskbar && taskbar->presentationConfirmed()) return true;
  }
  return false;
}
}  // namespace uwf::preview::host
