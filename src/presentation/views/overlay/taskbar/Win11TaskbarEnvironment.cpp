// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/overlay/taskbar/Win11TaskbarEnvironment.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <limits>

#include "foundation/logging/Log.h"

namespace uwf::ui::win11_taskbar {

namespace {

constexpr wchar_t kTaskbarClass[] = L"Shell_TrayWnd";
constexpr wchar_t kNotifyClass[] = L"TrayNotifyWnd";
constexpr int kTaskbarInset = 3;

bool hasArea(const RECT& rect) { return rect.right > rect.left && rect.bottom > rect.top; }

bool isVerticalTaskbar(const RECT& rect) { return rect.right - rect.left < rect.bottom - rect.top; }

bool hasWindowClass(const HWND window, const wchar_t* const expectedClass) {
  wchar_t actualClass[128]{};
  return GetClassNameW(window, actualClass, static_cast<int>(std::size(actualClass))) > 0 && wcscmp(actualClass, expectedClass) == 0;
}

int scaled(const int logicalPixels, const UINT dpi) { return MulDiv(logicalPixels, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI); }

}  // namespace

EnvironmentProbe detail::classifyEnvironmentObservation(const EnvironmentObservation& observation) {
  if (!observation.taskbarAvailable || !observation.notifyAvailable || !observation.hierarchyValid || !observation.processIdentityValid ||
      !observation.geometryAvailable) {
    UWF_LOG_D("taskbar") << "environment unavailable: taskbar=" << observation.taskbarAvailable << " notificationArea=" << observation.notifyAvailable
                         << " hierarchy=" << observation.hierarchyValid << " process=" << observation.processIdentityValid
                         << " geometry=" << observation.geometryAvailable;
    return {};
  }
  if (observation.verticalLayout) {
    UWF_LOG_D("taskbar") << "environment incompatible: reason=vertical-taskbar";
    return {RuntimeAvailability::IncompatibleLayout, std::nullopt};
  }
  return {RuntimeAvailability::Available, observation.environment};
}

EnvironmentProbe detail::resolveRetainedProbe(const EnvironmentProbe& retainedProbe, const EnvironmentProbe& currentProbe) {
  if (retainedProbe.availability != RuntimeAvailability::TemporarilyUnavailable) return retainedProbe;
  return currentProbe.availability == RuntimeAvailability::TemporarilyUnavailable ? retainedProbe : currentProbe;
}

detail::PlacementObservation detail::classifyPlacementObservation(const bool calculable, const bool matches) {
  if (!calculable) return PlacementObservation::Retained;
  return matches ? PlacementObservation::Confirmed : PlacementObservation::RefreshRequired;
}

ScopedThreadDpiAwareness::ScopedThreadDpiAwareness(const HWND hostWindow) {
  if (!hostWindow || !IsWindow(hostWindow)) return;
  const auto hostContext = GetWindowDpiAwarenessContext(hostWindow);
  if (hostContext) m_previousContext = SetThreadDpiAwarenessContext(hostContext);
}

ScopedThreadDpiAwareness::~ScopedThreadDpiAwareness() {
  if (m_previousContext) (void)SetThreadDpiAwarenessContext(m_previousContext);
}

EnvironmentProbe probeEnvironment() {
  detail::EnvironmentObservation observation;
  Environment& environment = observation.environment;
  environment.taskbar = FindWindowW(kTaskbarClass, nullptr);
  observation.taskbarAvailable = environment.taskbar && IsWindow(environment.taskbar);
  environment.notify = observation.taskbarAvailable ? FindWindowExW(environment.taskbar, nullptr, kNotifyClass, nullptr) : nullptr;
  observation.notifyAvailable = environment.notify && IsWindow(environment.notify);
  observation.hierarchyValid = observation.notifyAvailable && IsChild(environment.taskbar, environment.notify);

  GetWindowThreadProcessId(environment.taskbar, &environment.taskbarProcessId);
  DWORD notifyProcessId = 0;
  GetWindowThreadProcessId(environment.notify, &notifyProcessId);
  observation.processIdentityValid = environment.taskbarProcessId != 0 && notifyProcessId == environment.taskbarProcessId;
  observation.geometryAvailable = GetWindowRect(environment.taskbar, &environment.taskbarRect) &&
                                  GetClientRect(environment.taskbar, &environment.taskbarClientRect) &&
                                  GetWindowRect(environment.notify, &environment.notifyRect) && hasArea(environment.taskbarRect) &&
                                  hasArea(environment.taskbarClientRect) && hasArea(environment.notifyRect);
  observation.verticalLayout = observation.geometryAvailable && isVerticalTaskbar(environment.taskbarRect);
  const UINT dpi = GetDpiForWindow(environment.taskbar);
  environment.dpi = dpi == 0 ? USER_DEFAULT_SCREEN_DPI : dpi;
  return detail::classifyEnvironmentObservation(observation);
}

EnvironmentProbe refreshEnvironment(const Environment& retained) {
  detail::EnvironmentObservation observation;
  Environment& environment = observation.environment;
  environment = retained;
  observation.taskbarAvailable =
      IsWindow(environment.taskbar) && FindWindowW(kTaskbarClass, nullptr) == environment.taskbar && hasWindowClass(environment.taskbar, kTaskbarClass);
  observation.notifyAvailable = IsWindow(environment.notify) && hasWindowClass(environment.notify, kNotifyClass);
  observation.hierarchyValid = observation.taskbarAvailable && observation.notifyAvailable && IsChild(environment.taskbar, environment.notify);
  DWORD taskbarProcessId = 0;
  DWORD notifyProcessId = 0;
  GetWindowThreadProcessId(environment.taskbar, &taskbarProcessId);
  GetWindowThreadProcessId(environment.notify, &notifyProcessId);
  observation.processIdentityValid = taskbarProcessId == environment.taskbarProcessId && notifyProcessId == taskbarProcessId;
  observation.geometryAvailable = GetWindowRect(environment.taskbar, &environment.taskbarRect) &&
                                  GetClientRect(environment.taskbar, &environment.taskbarClientRect) &&
                                  GetWindowRect(environment.notify, &environment.notifyRect) && hasArea(environment.taskbarRect) &&
                                  hasArea(environment.taskbarClientRect) && hasArea(environment.notifyRect);
  observation.verticalLayout = observation.geometryAvailable && isVerticalTaskbar(environment.taskbarRect);
  const UINT dpi = GetDpiForWindow(environment.taskbar);
  environment.dpi = dpi == 0 ? USER_DEFAULT_SCREEN_DPI : dpi;
  return detail::classifyEnvironmentObservation(observation);
}

bool sameEnvironment(const Environment& lhs, const Environment& rhs) {
  return lhs.taskbar == rhs.taskbar && lhs.notify == rhs.notify && lhs.taskbarProcessId == rhs.taskbarProcessId;
}

std::optional<Placement> calculatePlacement(const Environment& environment, const QSize& logicalSize) {
  if (logicalSize.width() <= 0 || logicalSize.height() <= 0 || environment.dpi == 0 || environment.dpi > static_cast<UINT>(std::numeric_limits<int>::max())) {
    UWF_LOG_D("taskbar") << "placement unavailable: reason=invalid-logical-size-or-scale width=" << logicalSize.width() << " height=" << logicalSize.height()
                         << " dpi=" << environment.dpi;
    return std::nullopt;
  }

  POINT notifyTopLeft{environment.notifyRect.left, environment.notifyRect.top};
  if (!ScreenToClient(environment.taskbar, &notifyTopLeft)) {
    UWF_LOG_D("taskbar") << "placement unavailable: reason=screen-to-client-failed win32Error=" << GetLastError();
    return std::nullopt;
  }

  const auto clientWidth = static_cast<std::int64_t>(environment.taskbarClientRect.right) - environment.taskbarClientRect.left;
  const auto clientHeight = static_cast<std::int64_t>(environment.taskbarClientRect.bottom) - environment.taskbarClientRect.top;
  const int inset = scaled(kTaskbarInset, environment.dpi);
  const int width = scaled(logicalSize.width(), environment.dpi);
  const auto unavailable = [&]() -> std::optional<Placement> {
    UWF_LOG_D("taskbar") << "placement unavailable: reason=insufficient-space clientWidth=" << clientWidth << " clientHeight=" << clientHeight
                         << " notificationX=" << notifyTopLeft.x << " desiredWidth=" << width << " desiredHeight=" << logicalSize.height()
                         << " dpi=" << environment.dpi << " inset=" << inset;
    return std::nullopt;
  };
  if (clientWidth <= 0 || clientHeight <= 0 || clientWidth > std::numeric_limits<int>::max() || clientHeight > std::numeric_limits<int>::max() || width <= 0 ||
      inset < 0) {
    return unavailable();
  }
  // Qt stores child-window geometry in integer logical pixels. A native-only
  // target between those pixels is rewritten by Qt on a later event (for
  // example 2107 -> 2108 at 125%), causing every health check to repair it
  // again. Plan and verify one target on Qt's representable coordinate grid.
  // The height bound also accounts for MulDiv's rounding: choose the largest
  // logical height whose native extent still fits, rather than clipping to an
  // extent Qt cannot preserve.
  const auto availableHeight = clientHeight - 2 * static_cast<std::int64_t>(inset);
  if (availableHeight <= 0) return unavailable();
  const auto logicalHeightLimit = (availableHeight * USER_DEFAULT_SCREEN_DPI + (USER_DEFAULT_SCREEN_DPI - 1) / 2) / environment.dpi;
  const int height = scaled(static_cast<int>(std::min<std::int64_t>(logicalSize.height(), logicalHeightLimit)), environment.dpi);
  const auto nativeX = static_cast<std::int64_t>(notifyTopLeft.x) - width - inset;
  if (height <= 0 || nativeX < 0 || nativeX > std::numeric_limits<int>::max()) return unavailable();
  const int logicalX = MulDiv(static_cast<int>(nativeX), USER_DEFAULT_SCREEN_DPI, static_cast<int>(environment.dpi));
  const int logicalY = MulDiv(static_cast<int>(clientHeight - height), USER_DEFAULT_SCREEN_DPI / 2, static_cast<int>(environment.dpi));
  if (logicalX < 0 || logicalY < 0) return unavailable();
  const int x = scaled(logicalX, environment.dpi);
  const int y = scaled(logicalY, environment.dpi);
  if (x < inset || y < 0 || static_cast<std::int64_t>(x) + width > notifyTopLeft.x || static_cast<std::int64_t>(x) + width > clientWidth ||
      static_cast<std::int64_t>(y) + height > clientHeight) {
    return unavailable();
  }
  return Placement{x, y, width, height};
}

bool matchesPlacement(const HWND window, const Environment& environment, const Placement& placement) {
  RECT windowRect{};
  POINT taskbarOrigin{};
  if (!GetWindowRect(window, &windowRect) || !ClientToScreen(environment.taskbar, &taskbarOrigin)) return false;
  return windowRect.left == static_cast<std::int64_t>(taskbarOrigin.x) + placement.x &&
         windowRect.top == static_cast<std::int64_t>(taskbarOrigin.y) + placement.y &&
         static_cast<std::int64_t>(windowRect.right) - windowRect.left == placement.width &&
         static_cast<std::int64_t>(windowRect.bottom) - windowRect.top == placement.height;
}

}  // namespace uwf::ui::win11_taskbar
