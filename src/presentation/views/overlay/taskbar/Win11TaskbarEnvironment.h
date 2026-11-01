// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <windows.h>

#include <QSize>
#include <optional>

namespace uwf::ui::win11_taskbar {

struct Environment {
  HWND taskbar = nullptr;
  HWND notify = nullptr;
  DWORD taskbarProcessId = 0;
  RECT taskbarRect{};
  RECT taskbarClientRect{};
  RECT notifyRect{};
  UINT dpi = USER_DEFAULT_SCREEN_DPI;
};

struct Placement {
  // Physical pixels in the host client area, quantized through integer logical
  // geometry so both Qt and Win32 preserve the same placement after callbacks.
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

// Runtime layout status is independent of static process compatibility.
// Missing or rebuilding Explorer windows are temporary conditions.
enum class RuntimeAvailability { Available, TemporarilyUnavailable, IncompatibleLayout };

struct EnvironmentProbe {
  RuntimeAvailability availability = RuntimeAvailability::TemporarilyUnavailable;
  std::optional<Environment> environment;
};

class ScopedThreadDpiAwareness final {
 public:
  explicit ScopedThreadDpiAwareness(HWND hostWindow);
  ~ScopedThreadDpiAwareness();
  ScopedThreadDpiAwareness(const ScopedThreadDpiAwareness&) = delete;
  ScopedThreadDpiAwareness& operator=(const ScopedThreadDpiAwareness&) = delete;
  [[nodiscard]] bool active() const { return m_previousContext != nullptr; }

 private:
  DPI_AWARENESS_CONTEXT m_previousContext = nullptr;
};

namespace detail {

// Pure classification separates observed Win32 facts from runtime availability.
struct EnvironmentObservation {
  Environment environment;
  bool taskbarAvailable = false;
  bool notifyAvailable = false;
  bool hierarchyValid = false;
  bool processIdentityValid = false;
  bool geometryAvailable = false;
  bool verticalLayout = false;
};

[[nodiscard]] EnvironmentProbe classifyEnvironmentObservation(const EnvironmentObservation& observation);

// A transient cold probe preserves the retained result rather than declaring identity loss.
[[nodiscard]] EnvironmentProbe resolveRetainedProbe(const EnvironmentProbe& retainedProbe, const EnvironmentProbe& currentProbe);

enum class PlacementObservation { Confirmed, Retained, RefreshRequired };
[[nodiscard]] PlacementObservation classifyPlacementObservation(bool calculable, bool matches);
}  // namespace detail

[[nodiscard]] EnvironmentProbe probeEnvironment();
[[nodiscard]] EnvironmentProbe refreshEnvironment(const Environment& retained);
[[nodiscard]] bool sameEnvironment(const Environment& lhs, const Environment& rhs);
[[nodiscard]] std::optional<Placement> calculatePlacement(const Environment& environment, const QSize& logicalSize);
[[nodiscard]] bool matchesPlacement(HWND window, const Environment& environment, const Placement& placement);

}  // namespace uwf::ui::win11_taskbar
