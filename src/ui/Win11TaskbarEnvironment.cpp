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
#include "Win11TaskbarEnvironment.h"

#include <algorithm>
#include <iterator>

#include "../util/Log.h"

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
    UWF_LOG_D("taskbar") << "environment unavailable: taskbar=" << observation.taskbarAvailable << " notify=" << observation.notifyAvailable
                         << " hierarchy=" << observation.hierarchyValid << " process=" << observation.processIdentityValid
                         << " geometry=" << observation.geometryAvailable;
    return {};
  }
  if (observation.verticalLayout) {
    UWF_LOG_D("taskbar") << "environment incompatible: vertical taskbar";
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
  m_hostContext = GetWindowDpiAwarenessContext(hostWindow);
  if (!m_hostContext) return;
  m_previousContext = SetThreadDpiAwarenessContext(m_hostContext);
  if (!m_previousContext) m_hostContext = nullptr;
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
  if (logicalSize.width() <= 0 || logicalSize.height() <= 0) {
    UWF_LOG_D("taskbar") << "placement unavailable: invalid logical size " << logicalSize.width() << 'x' << logicalSize.height();
    return std::nullopt;
  }

  POINT notifyTopLeft{environment.notifyRect.left, environment.notifyRect.top};
  if (!ScreenToClient(environment.taskbar, &notifyTopLeft)) {
    UWF_LOG_D("taskbar") << "placement unavailable: ScreenToClient failed error=" << GetLastError();
    return std::nullopt;
  }

  const int clientWidth = static_cast<int>(environment.taskbarClientRect.right - environment.taskbarClientRect.left);
  const int clientHeight = static_cast<int>(environment.taskbarClientRect.bottom - environment.taskbarClientRect.top);
  const int inset = scaled(kTaskbarInset, environment.dpi);
  const int width = scaled(logicalSize.width(), environment.dpi);
  const int height = std::min(scaled(logicalSize.height(), environment.dpi), clientHeight - 2 * inset);
  const int x = static_cast<int>(notifyTopLeft.x) - width - inset;
  const int y = (clientHeight - height) / 2;
  if (width <= 0 || height <= 0 || x < inset || y < 0 || x + width > static_cast<int>(notifyTopLeft.x) || x + width > clientWidth) {
    UWF_LOG_D("taskbar") << "placement unavailable: client=" << clientWidth << 'x' << clientHeight << " notifyX=" << notifyTopLeft.x << " desired=" << width
                         << 'x' << height << " position=" << x << ',' << y << " inset=" << inset;
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

}  // namespace uwf::ui::win11_taskbar
