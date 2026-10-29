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
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

// 仅描述运行时任务栏布局。系统家族等静态兼容性由 Strategy::isCompatible()
// 判断；Explorer 窗口缺失、重建或暂时不可枚举永远不是“不支持”。
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
  [[nodiscard]] DPI_AWARENESS_CONTEXT hostContext() const { return m_hostContext; }

 private:
  DPI_AWARENESS_CONTEXT m_hostContext = nullptr;
  DPI_AWARENESS_CONTEXT m_previousContext = nullptr;
};

namespace detail {

// Win32 调用只负责收集事实，以下纯分类函数统一决定运行时语义。
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

// retained 快照不可观测时才采用冷探测结果；冷探测同样处于过渡时保留原
// 结果，避免一次 Shell 瞬态被升级为身份失效。
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
