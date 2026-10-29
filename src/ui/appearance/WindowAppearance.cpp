// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowAppearance.h"

#include <dwmapi.h>
#include <windows.h>

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QLayout>
#include <QPointer>
#include <QScopeGuard>
#include <QSettings>
#include <QTimer>
#include <QWidget>
#include <QWindow>
#include <algorithm>

#include "I18n.h"
#include "ThemeManager.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/PagedWindowLayout.h"

namespace uwf::ui {
namespace {

constexpr DWORD kDarkModeAttribute = 20;
constexpr DWORD kCornerPreferenceAttribute = 33;
constexpr DWORD kBorderColorAttribute = 34;
constexpr DWORD kCaptionColorAttribute = 35;
constexpr DWORD kTextColorAttribute = 36;
constexpr int kRoundedCorners = 2;
constexpr DWORD kSystemBackdropAttribute = 38;
constexpr DWORD kRedirectionBitmapAlphaAttribute = 39;
constexpr int kNoBackdrop = 1;
constexpr int kMicaBackdrop = 2;

bool materialAllowed() {
  HIGHCONTRASTW contrast{sizeof(HIGHCONTRASTW), 0, nullptr};
  if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0) && (contrast.dwFlags & HCF_HIGHCONTRASTON)) return false;
  SYSTEM_POWER_STATUS power{};
  if (GetSystemPowerStatus(&power) && power.SystemStatusFlag == 1) return false;
  const QSettings personalization(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
                                  QSettings::NativeFormat);
  return personalization.value(QStringLiteral("EnableTransparency"), 1).toBool();
}

class BackdropEvents final : public QObject, public QAbstractNativeEventFilter {
 public:
  explicit BackdropEvents(QWidget* window) : QObject(window), m_window(window) {
    window->installEventFilter(this);
    qApp->installNativeEventFilter(this);
  }
  ~BackdropEvents() override {
    if (qApp) qApp->removeNativeEventFilter(this);
  }

 protected:
  bool eventFilter(QObject* object, QEvent* event) override {
    if (object == m_nativeWindow && event->type() == QEvent::ApplicationPaletteChange) {
      // Qt resets frameless HWNDs to a light frame while delivering this event.
      // Restore the material theme after the platform window has handled it.
      scheduleRefresh();
    }
    if (object != m_window) return QObject::eventFilter(object, event);
    switch (event->type()) {
      case QEvent::WinIdChange:
        m_window->setProperty("systemBackdropConfiguredType", 0);
        [[fallthrough]];
      case QEvent::Show:
        if (auto* nativeWindow = m_window->windowHandle(); nativeWindow != m_nativeWindow) {
          if (m_nativeWindow) m_nativeWindow->removeEventFilter(this);
          m_nativeWindow = nativeWindow;
          if (nativeWindow) nativeWindow->installEventFilter(this);
        }
        // Prepare the redirected surface before the first visible frame.
        // DWM handles active/inactive material transitions itself.
        refreshWindowBackdrop(m_window);
        break;
      default:
        break;
    }
    return QObject::eventFilter(object, event);
  }
  bool nativeEventFilter(const QByteArray&, void* message, qintptr* result) override {
    if (!m_window || !message) return false;
    const auto* native = static_cast<const MSG*>(message);
    if (native->message == WM_NCACTIVATE && native->hwnd == reinterpret_cast<HWND>(m_window->effectiveWinId())) {
      // Qt 6.12 consumes WM_NCACTIVATE for frameless windows without notifying
      // DefWindowProc. DWM then keeps Mica's inactive, solid fallback even when
      // the HWND has keyboard focus. Forward activation with -1 to suppress
      // native frame repaint while preserving the system material's state.
      *result = DefWindowProcW(native->hwnd, WM_NCACTIVATE, native->wParam, -1);
      return true;
    }
    if (native->message != WM_SETTINGCHANGE && native->message != WM_THEMECHANGED && native->message != WM_DWMCOMPOSITIONCHANGED &&
        native->message != WM_POWERBROADCAST)
      return false;
    // Native filters also run during CreateWindowEx, before Qt assigns its HWND.
    // effectiveWinId never creates a window or re-enters platform initialization.
    if (native->hwnd != reinterpret_cast<HWND>(m_window->effectiveWinId())) return false;
    if (native->message == WM_DWMCOMPOSITIONCHANGED) m_window->setProperty("systemBackdropConfiguredType", 0);
    scheduleRefresh();
    return false;
  }

 private:
  void scheduleRefresh() {
    if (m_refreshPending) return;
    m_refreshPending = true;
    QTimer::singleShot(0, this, [this] {
      m_refreshPending = false;
      if (m_window) refreshWindowBackdrop(m_window);
    });
  }

  QPointer<QWidget> m_window;
  QPointer<QWindow> m_nativeWindow;
  bool m_refreshPending = false;
};

template <typename T>
void setAttribute(HWND window, const DWORD attribute, const T& value) {
  (void)DwmSetWindowAttribute(window, attribute, &value, static_cast<DWORD>(sizeof(value)));
}

COLORREF nativeColor(const QColor& color) { return RGB(color.red(), color.green(), color.blue()); }

}  // namespace

void applyWindowAppearance(QWidget* window, const Sem caption) {
  if (!window->testAttribute(Qt::WA_WState_Created)) return;
  const auto handle = reinterpret_cast<HWND>(window->winId());
  if (window->windowFlags().testFlag(Qt::FramelessWindowHint)) {
    // Qt creates frameless HWNDs as WS_POPUP without WS_THICKFRAME. Windows
    // needs the sizing style as well as edge hit tests to enter its resize loop.
    // Qt's frameless WM_NCCALCSIZE handler keeps the entire frame client drawn.
    const auto style = GetWindowLongPtrW(handle, GWL_STYLE);
    const bool resizable = window->minimumWidth() < window->maximumWidth() || window->minimumHeight() < window->maximumHeight();
    auto updated = resizable ? style | WS_THICKFRAME : style & ~WS_THICKFRAME;
    if (window->property("nativeCaptionCommands").toBool()) {
      // Snap Layouts requires a system-menu-capable HWND. Keep its sizing and
      // minimize/maximize styles, but suppress the native caption so DWM does
      // not draw a second set of buttons through the custom title bar.
      updated = (updated | WS_SYSMENU) & ~WS_CAPTION;
      if (window->windowFlags().testFlag(Qt::WindowMinimizeButtonHint)) updated |= WS_MINIMIZEBOX;
      if (resizable && window->windowFlags().testFlag(Qt::WindowMaximizeButtonHint)) updated |= WS_MAXIMIZEBOX;
    }
    if (style != updated) {
      SetWindowLongPtrW(handle, GWL_STYLE, updated);
      SetWindowPos(handle, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
  }
  const auto& theme = ThemeManager::instance();
  const BOOL dark = !theme.isLight();
  // Attribute numbers also compile against SDKs predating Windows 11. Older
  // Windows versions ignore attributes they do not support.
  setAttribute(handle, kDarkModeAttribute, dark);
  setAttribute(handle, kCornerPreferenceAttribute, kRoundedCorners);
  setAttribute(handle, kBorderColorAttribute, nativeColor(theme.color(Sem::Border)));
  // A fixed caption color would mask Mica in a title-height strip even when
  // the client content is transparent. Let DWM use the material there too.
  const COLORREF captionColor = window->property("systemBackdropRequested").toBool() ? 0xFFFFFFFF : nativeColor(theme.color(caption));
  setAttribute(handle, kCaptionColorAttribute, captionColor);
  setAttribute(handle, kTextColorAttribute, nativeColor(theme.color(Sem::Fg)));
  refreshWindowBackdrop(window);
}

void applyWindowAppearance(QWidget* window) { applyWindowAppearance(window, Sem::Surface); }

void prepareWindowBackdrop(QWidget* window) {
  if (window->property("systemBackdropRequested").toBool()) return;
  window->setProperty("systemBackdropRequested", true);
  // Keep an alpha backing store, but present it through the ordinary DWM
  // redirection surface (see refreshWindowBackdrop), never a layered window.
  window->setAttribute(Qt::WA_TranslucentBackground);
  window->setAutoFillBackground(false);
  new BackdropEvents(window);
}

void refreshWindowBackdrop(QWidget* window) {
  if (!window->property("systemBackdropRequested").toBool() || !window->testAttribute(Qt::WA_WState_Created)) return;
  const auto handle = reinterpret_cast<HWND>(window->effectiveWinId());
  if (!handle) return;
  const BOOL dark = !ThemeManager::instance().isLight();
  setAttribute(handle, kDarkModeAttribute, dark);
  const bool permitted = materialAllowed() && !window->property("systemBackdropForceSolid").toBool();
  const int material = permitted ? kMicaBackdrop : kNoBackdrop;
  const bool materialChanged = window->property("systemBackdropConfiguredType").toInt() != material;
  bool active = window->property("systemBackdropActive").toBool();
  if (materialChanged) {
    const bool supported = SUCCEEDED(DwmSetWindowAttribute(handle, kSystemBackdropAttribute, &material, sizeof(material)));
    active = permitted && supported;
    window->setProperty("systemBackdropConfiguredType", material);
  }
  // Qt's translucent frameless raster path normally uses UpdateLayeredWindow.
  // Mica requires a regular redirected HWND instead. Qt 6.12 checks the actual
  // WS_EX_LAYERED bit on each flush and uses BitBlt when it is absent, retaining
  // the premultiplied alpha backing store over the extended DWM frame.
  if (window->windowOpacity() == 1.0) {
    const auto extended = GetWindowLongPtrW(handle, GWL_EXSTYLE);
    if (extended & WS_EX_LAYERED) SetWindowLongPtrW(handle, GWL_EXSTYLE, extended & ~WS_EX_LAYERED);
  }
  // Windows 11 24H2+ can explicitly consume the premultiplied raster alpha.
  // Earlier versions ignore this attribute and use the extended glass frame.
  const BOOL useAlpha = TRUE;
  setAttribute(handle, kRedirectionBitmapAlphaAttribute, useAlpha);
  bool composited = active;
  if (materialChanged) {
    const MARGINS margins = active ? MARGINS{-1, -1, -1, -1} : MARGINS{0, 0, 0, 0};
    const bool extended = SUCCEEDED(DwmExtendFrameIntoClientArea(handle, &margins));
    composited = active && extended;
  }
  if (window->property("systemBackdropActive").toBool() != composited) {
    window->setProperty("systemBackdropActive", composited);
    window->update();
  }
}

void startWindowDrag(QWidget* window, const QPoint& globalPosition) {
  ReleaseCapture();
  SendMessageW(reinterpret_cast<HWND>(window->winId()), WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(globalPosition.x(), globalPosition.y()));
}
void showWindowMenu(QWidget* window, const QPoint& position) {
  CommandMenu menu(window);
  const QPointer<QWidget> owner(window);
  const auto add = [&](const QString& text, const QString& icon, const UINT command, const bool enabled) {
    auto* action = menu.addAction(ThemeManager::instance().icon(icon), text, &menu, [owner, command] {
      if (owner)
        QTimer::singleShot(0, owner, [owner, command] {
          if (!owner) return;
          switch (command) {
            case SC_RESTORE:
              owner->showNormal();
              break;
            case SC_MINIMIZE:
              owner->showMinimized();
              break;
            case SC_MAXIMIZE:
              owner->showMaximized();
              break;
            case SC_CLOSE:
              owner->close();
              break;
            default:
              ReleaseCapture();
              // Enter Windows' keyboard move/resize loop after the custom menu releases its grab.
              DefWindowProcW(reinterpret_cast<HWND>(owner->winId()), WM_SYSCOMMAND, command, 0);
              break;
          }
        });
    });
    action->setEnabled(enabled);
  };
  const bool normal = !window->isMaximized() && !window->isMinimized() && !window->isFullScreen();
  add(I18n::tr("Restore"), QStringLiteral(":/icons/restore.svg"), SC_RESTORE, !normal);
  add(I18n::tr("Move"), QStringLiteral(":/icons/move.svg"), SC_MOVE, normal);
  add(I18n::tr("Size"), QStringLiteral(":/icons/resize.svg"), SC_SIZE,
      normal && (window->minimumWidth() < window->maximumWidth() || window->minimumHeight() < window->maximumHeight()));
  add(I18n::tr("Minimize"), QStringLiteral(":/icons/minimize.svg"), SC_MINIMIZE,
      window->windowFlags().testFlag(Qt::WindowMinimizeButtonHint) && !window->isMinimized());
  add(I18n::tr("Maximize"), QStringLiteral(":/icons/maximize.svg"), SC_MAXIMIZE,
      window->windowFlags().testFlag(Qt::WindowMaximizeButtonHint) && !window->isMaximized());
  menu.addSeparator();
  const auto* close = window->findChild<QAbstractButton*>(QStringLiteral("dialogCloseButton"));
  add(I18n::tr("Close"), QStringLiteral(":/icons/close.svg"), SC_CLOSE, window->isEnabled() && (!close || close->isEnabled()));
  menu.exec(position);
}
bool handleWindowMenu(QWidget* window, void* message, qintptr* result) {
  const auto* native = static_cast<const MSG*>(message);
  if ((native->message == WM_NCRBUTTONUP && (native->wParam == HTCAPTION || native->wParam == HTSYSMENU)) ||
      (native->message == WM_SYSCOMMAND && (native->wParam & 0xfff0) == SC_KEYMENU && native->lParam == VK_SPACE)) {
    const auto position = QCursor::pos();
    QTimer::singleShot(0, window, [window, position] { showWindowMenu(window, position); });
    *result = 0;
    return true;
  }
  return false;
}

bool handleWindowResize(QWidget* window, void* message, qintptr* result) {
  const auto* native = static_cast<const MSG*>(message);
  auto& layout = PagedWindowLayout::forWindow(window);
  if (native->message == WM_EXITSIZEMOVE) layout.endNativeResize();
  if (native->message == WM_SIZING && !window->isMaximized() && !window->isFullScreen()) {
    if (layout.hasPageHeightConstraint()) {
      auto* requested = reinterpret_cast<RECT*>(native->lParam);
      RECT outer{}, client{};
      if (GetWindowRect(native->hwnd, &outer) && GetClientRect(native->hwnd, &client)) {
        const auto scale = window->devicePixelRatioF();
        const int frame = (outer.bottom - outer.top) - (client.bottom - client.top);
        const int height = qRound((requested->bottom - requested->top - frame) / scale);
        const int frameWidth = (outer.right - outer.left) - (client.right - client.left);
        const int width = qRound((requested->right - requested->left - frameWidth) / scale);
        const int edge = static_cast<int>(native->wParam);
        const bool anchorBottom = edge == WMSZ_TOP || edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT;
        const auto fitted = layout.constrainNativeResize(QSize(width, height), anchorBottom);
        const int aligned = qRound(fitted.height() * scale) + frame;
        if (edge == WMSZ_TOP || edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
          requested->top = requested->bottom - aligned;
        else
          requested->bottom = requested->top + aligned;
        *result = TRUE;
        return true;
      }
    }
  }
  if (native->message == WM_NCCALCSIZE && native->wParam && IsZoomed(native->hwnd) && !window->isFullScreen()) {
    // WS_THICKFRAME maximization includes an invisible sizing border outside
    // the work area. Keep Qt's client content inside the monitor's work area
    // so the title bar/footer are not clipped by those native frame margins.
    MONITORINFO monitor{sizeof(MONITORINFO), {}, {}, 0};
    if (GetMonitorInfoW(MonitorFromWindow(native->hwnd, MONITOR_DEFAULTTONEAREST), &monitor)) {
      reinterpret_cast<NCCALCSIZE_PARAMS*>(native->lParam)->rgrc[0] = monitor.rcWork;
      *result = 0;
      return true;
    }
  }
  if (native->message == WM_NCCALCSIZE && native->wParam && window->windowFlags().testFlag(Qt::FramelessWindowHint)) {
    // A top/left resize must repaint the client surface instead of blitting
    // the old client pixels into its new origin.
    *result = WVR_REDRAW;
    return true;
  }
  if (native->message != WM_NCHITTEST || window->isMaximized() || window->isFullScreen()) return false;
  RECT bounds{};
  if (!GetWindowRect(native->hwnd, &bounds)) return false;
  const int x = static_cast<short>(LOWORD(native->lParam));
  const int y = static_cast<short>(HIWORD(native->lParam));
  if (x < bounds.left || x >= bounds.right || y < bounds.top || y >= bounds.bottom) return false;
  // DWM excludes the invisible sizing margins from the visible frame. Anchor
  // our hot zones there as well, so they remain reachable inside the painted
  // top/left edge on high-DPI displays. Keep the outer margins interactive.
  RECT visible{};
  if (SUCCEEDED(DwmGetWindowAttribute(native->hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &visible, sizeof(visible))) && visible.left < visible.right &&
      visible.top < visible.bottom) {
    bounds.left = std::max(bounds.left, visible.left);
    bounds.top = std::max(bounds.top, visible.top);
    bounds.right = std::min(bounds.right, visible.right);
    bounds.bottom = std::min(bounds.bottom, visible.bottom);
  }
  const int border = qRound(6 * window->devicePixelRatioF());
  const bool horizontal = window->minimumWidth() < window->maximumWidth();
  const bool vertical = window->minimumHeight() < window->maximumHeight();
  const bool left = horizontal && x < bounds.left + border, right = horizontal && x >= bounds.right - border;
  const bool top = vertical && y < bounds.top + border, bottom = vertical && y >= bounds.bottom - border;
  const int hit = top && left       ? HTTOPLEFT
                  : top && right    ? HTTOPRIGHT
                  : bottom && left  ? HTBOTTOMLEFT
                  : bottom && right ? HTBOTTOMRIGHT
                  : left            ? HTLEFT
                  : right           ? HTRIGHT
                  : top             ? HTTOP
                  : bottom          ? HTBOTTOM
                                    : HTCLIENT;
  if (hit == HTCLIENT) return false;
  *result = hit;
  return true;
}

}  // namespace uwf::ui
