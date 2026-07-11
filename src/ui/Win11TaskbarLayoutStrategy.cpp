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
#include "Win11TaskbarLayoutStrategy.h"

#include <dwmapi.h>
#include <windows.h>

#include <algorithm>
#include <optional>

namespace uwf::ui {

namespace {

constexpr wchar_t kTaskbarClass[] = L"Shell_TrayWnd";
constexpr wchar_t kNotifyClass[] = L"TrayNotifyWnd";
constexpr wchar_t kCompositionBridgeClass[] = L"Windows.UI.Composition.DesktopWindowContentBridge";
constexpr int kTaskbarInset = 3;

struct Environment {
  HWND taskbar = nullptr;
  HWND notify = nullptr;
  HWND compositionBridge = nullptr;
  DWORD taskbarProcessId = 0;
  RECT taskbarRect{};
  RECT taskbarClientRect{};
  RECT notifyRect{};
  UINT dpi = USER_DEFAULT_SCREEN_DPI;
};

struct DescendantSearch {
  const wchar_t* className = nullptr;
  HWND result = nullptr;
};

struct Placement {
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

BOOL CALLBACK findDescendantCallback(const HWND window, const LPARAM parameter) {
  auto* const search = reinterpret_cast<DescendantSearch*>(parameter);
  wchar_t className[128]{};
  if (GetClassNameW(window, className, static_cast<int>(std::size(className))) > 0 && wcscmp(className, search->className) == 0) {
    search->result = window;
    return FALSE;
  }
  return TRUE;
}

HWND findDescendantByClass(const HWND parent, const wchar_t* className) {
  DescendantSearch search{className, nullptr};
  EnumChildWindows(parent, findDescendantCallback, reinterpret_cast<LPARAM>(&search));
  return search.result;
}

bool hasArea(const RECT& rect) { return rect.right > rect.left && rect.bottom > rect.top; }

int scaled(const int logicalPixels, const UINT dpi) { return MulDiv(logicalPixels, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI); }

bool setWindowLongChecked(const HWND window, const int index, const LONG_PTR value) {
  SetLastError(ERROR_SUCCESS);
  const LONG_PTR previous = SetWindowLongPtrW(window, index, value);
  return previous != 0 || GetLastError() == ERROR_SUCCESS;
}

std::optional<Environment> probeEnvironment() {
  Environment environment;
  environment.taskbar = FindWindowW(kTaskbarClass, nullptr);
  if (!environment.taskbar || !IsWindow(environment.taskbar) || !IsWindowVisible(environment.taskbar)) return std::nullopt;

  environment.compositionBridge = findDescendantByClass(environment.taskbar, kCompositionBridgeClass);
  environment.notify = FindWindowExW(environment.taskbar, nullptr, kNotifyClass, nullptr);
  if (!environment.compositionBridge || !environment.notify || !IsWindow(environment.compositionBridge) || !IsWindow(environment.notify) ||
      !IsWindowVisible(environment.notify) || !IsChild(environment.taskbar, environment.notify)) {
    return std::nullopt;
  }

  GetWindowThreadProcessId(environment.taskbar, &environment.taskbarProcessId);
  DWORD notifyProcessId = 0;
  DWORD bridgeProcessId = 0;
  GetWindowThreadProcessId(environment.notify, &notifyProcessId);
  GetWindowThreadProcessId(environment.compositionBridge, &bridgeProcessId);
  if (environment.taskbarProcessId == 0 || !GetWindowRect(environment.taskbar, &environment.taskbarRect) ||
      !GetClientRect(environment.taskbar, &environment.taskbarClientRect) || !GetWindowRect(environment.notify, &environment.notifyRect) ||
      !hasArea(environment.taskbarRect) || !hasArea(environment.taskbarClientRect) || !hasArea(environment.notifyRect) ||
      notifyProcessId != environment.taskbarProcessId || bridgeProcessId != environment.taskbarProcessId) {
    return std::nullopt;
  }

  // Win11 原生任务栏只接受横向布局。遇到第三方垂直任务栏时不套用未知
  // 几何规则，直接让 Hub 选择后备 View。
  if ((environment.taskbarRect.right - environment.taskbarRect.left) < (environment.taskbarRect.bottom - environment.taskbarRect.top)) return std::nullopt;

  const UINT dpi = GetDpiForWindow(environment.taskbar);
  environment.dpi = dpi == 0 ? USER_DEFAULT_SCREEN_DPI : dpi;
  return environment;
}

bool sameEnvironment(const Environment& lhs, const Environment& rhs) {
  return lhs.taskbar == rhs.taskbar && lhs.notify == rhs.notify && lhs.compositionBridge == rhs.compositionBridge &&
         lhs.taskbarProcessId == rhs.taskbarProcessId;
}

std::optional<Placement> calculatePlacement(const Environment& environment, const QSize& logicalSize) {
  if (logicalSize.width() <= 0 || logicalSize.height() <= 0) return std::nullopt;

  POINT notifyTopLeft{environment.notifyRect.left, environment.notifyRect.top};
  if (!ScreenToClient(environment.taskbar, &notifyTopLeft)) return std::nullopt;

  const int clientWidth = static_cast<int>(environment.taskbarClientRect.right - environment.taskbarClientRect.left);
  const int clientHeight = static_cast<int>(environment.taskbarClientRect.bottom - environment.taskbarClientRect.top);
  const int inset = scaled(kTaskbarInset, environment.dpi);
  const int width = scaled(logicalSize.width(), environment.dpi);
  const int height = std::min(scaled(logicalSize.height(), environment.dpi), clientHeight - 2 * inset);
  const int x = static_cast<int>(notifyTopLeft.x) - width - inset;
  const int y = (clientHeight - height) / 2;

  // 不截断、不覆盖通知区，也不靠魔数猜测位置；空间不足即明确失败。
  if (width <= 0 || height <= 0 || x < inset || y < 0 || x + width > static_cast<int>(notifyTopLeft.x) || x + width > clientWidth) {
    return std::nullopt;
  }
  return Placement{x, y, width, height};
}

bool matchesPlacement(const HWND window, const Environment& environment, const Placement& placement) {
  RECT windowRect{};
  POINT taskbarOrigin{};
  if (!GetWindowRect(window, &windowRect) || !ClientToScreen(environment.taskbar, &taskbarOrigin)) return false;
  return windowRect.left == taskbarOrigin.x + placement.x && windowRect.top == taskbarOrigin.y + placement.y &&
         windowRect.right - windowRect.left == placement.width && windowRect.bottom - windowRect.top == placement.height;
}

}  // namespace

struct Win11TaskbarLayoutStrategy::Impl {
  std::optional<Environment> attachment;
  QSize logicalSize;
};

Win11TaskbarLayoutStrategy::Win11TaskbarLayoutStrategy() : m_impl(std::make_unique<Impl>()) {}

Win11TaskbarLayoutStrategy::~Win11TaskbarLayoutStrategy() = default;

bool Win11TaskbarLayoutStrategy::available() const { return probeEnvironment().has_value(); }

bool Win11TaskbarLayoutStrategy::attach(const WId windowId, const QSize& logicalSize) {
  const auto environment = probeEnvironment();
  const HWND window = reinterpret_cast<HWND>(windowId);
  if (!environment || !window || !IsWindow(window)) return false;
  const auto placement = calculatePlacement(*environment, logicalSize);
  if (!placement) return false;

  if (m_impl->attachment && !sameEnvironment(*m_impl->attachment, *environment)) detach(windowId);

  DWORD windowProcessId = 0;
  GetWindowThreadProcessId(window, &windowProcessId);
  if (windowProcessId != GetCurrentProcessId()) return false;

  const LONG_PTR currentStyle = GetWindowLongPtrW(window, GWL_STYLE);
  LONG_PTR desiredStyle = currentStyle;
  desiredStyle &= ~static_cast<LONG_PTR>(WS_POPUP);
  desiredStyle |= static_cast<LONG_PTR>(WS_CHILD);
  const bool styleChanged = desiredStyle != currentStyle;
  if (styleChanged && !setWindowLongChecked(window, GWL_STYLE, desiredStyle)) return false;

  const LONG_PTR currentExStyle = GetWindowLongPtrW(window, GWL_EXSTYLE);
  LONG_PTR desiredExStyle = currentExStyle;
  desiredExStyle &= ~static_cast<LONG_PTR>(WS_EX_APPWINDOW);
  desiredExStyle |= static_cast<LONG_PTR>(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
  const bool exStyleChanged = desiredExStyle != currentExStyle;
  if (exStyleChanged && !setWindowLongChecked(window, GWL_EXSTYLE, desiredExStyle)) return false;

  const bool parentChanged = GetParent(window) != environment->taskbar;
  if (parentChanged) {
    SetLastError(ERROR_SUCCESS);
    const HWND previousParent = SetParent(window, environment->taskbar);
    if (!previousParent && GetLastError() != ERROR_SUCCESS) return false;
  }

  const bool frameChanged = styleChanged || exStyleChanged;
  const bool geometryChanged = parentChanged || !matchesPlacement(window, *environment, *placement);
  const bool visibilityChanged = !IsWindowVisible(window);
  if (frameChanged || geometryChanged || visibilityChanged) {
    UINT flags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW;
    if (frameChanged) flags |= SWP_FRAMECHANGED;
    if (!geometryChanged) flags |= SWP_NOMOVE | SWP_NOSIZE;
    if (!SetWindowPos(window, HWND_TOP, placement->x, placement->y, placement->width, placement->height, flags)) return false;
  }

  m_impl->attachment = environment;
  m_impl->logicalSize = logicalSize;
  return true;
}

bool Win11TaskbarLayoutStrategy::verify(const WId windowId) const {
  if (!m_impl->attachment) return false;
  const auto environment = probeEnvironment();
  const HWND window = reinterpret_cast<HWND>(windowId);
  if (!environment || !sameEnvironment(*m_impl->attachment, *environment) || !window || !IsWindow(window) || !IsWindowVisible(window) ||
      GetParent(window) != environment->taskbar) {
    return false;
  }

  DWORD windowProcessId = 0;
  GetWindowThreadProcessId(window, &windowProcessId);
  if (windowProcessId != GetCurrentProcessId()) return false;

  const LONG_PTR style = GetWindowLongPtrW(window, GWL_STYLE);
  if ((style & static_cast<LONG_PTR>(WS_CHILD)) == 0 || (style & static_cast<LONG_PTR>(WS_POPUP)) != 0) return false;
  const LONG_PTR exStyle = GetWindowLongPtrW(window, GWL_EXSTYLE);
  if ((exStyle & static_cast<LONG_PTR>(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE)) != static_cast<LONG_PTR>(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE) ||
      (exStyle & static_cast<LONG_PTR>(WS_EX_APPWINDOW)) != 0) {
    return false;
  }

  const auto placement = calculatePlacement(*environment, m_impl->logicalSize);
  if (!placement || !matchesPlacement(window, *environment, *placement)) return false;

  DWORD cloaked = 0;
  if (SUCCEEDED(DwmGetWindowAttribute(window, DWMWA_CLOAKED, &cloaked, static_cast<DWORD>(sizeof(cloaked)))) && cloaked != 0) return false;
  return true;
}

void Win11TaskbarLayoutStrategy::detach(const WId windowId) {
  const HWND window = reinterpret_cast<HWND>(windowId);
  if (window && IsWindow(window)) {
    ShowWindow(window, SW_HIDE);
    if (m_impl->attachment && GetParent(window) == m_impl->attachment->taskbar) SetParent(window, nullptr);

    LONG_PTR style = GetWindowLongPtrW(window, GWL_STYLE);
    style &= ~static_cast<LONG_PTR>(WS_CHILD);
    style |= static_cast<LONG_PTR>(WS_POPUP);
    setWindowLongChecked(window, GWL_STYLE, style);
  }
  m_impl->attachment.reset();
  m_impl->logicalSize = {};
}

void Win11TaskbarLayoutStrategy::invalidate() {
  m_impl->attachment.reset();
  m_impl->logicalSize = {};
}

}  // namespace uwf::ui
