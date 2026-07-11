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
#include "OverlayTaskbarWidget.h"

#include <dwmapi.h>
#include <windows.h>

#include <QContextMenuEvent>
#include <QFontMetrics>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <numbers>

#include "I18n.h"
#include "OverlayHudPalette.h"
#include "OverlayHudRenderer.h"

namespace uwf::ui {

namespace {

constexpr int kMinimumLogicalWidth = 96;
constexpr int kLogicalHeight = 32;
constexpr int kHorizontalPadding = 8;
constexpr int kAnimationIntervalMs = 100;
constexpr int kTaskbarInset = 3;
constexpr int kRadius = 6;
constexpr qreal kWavePhaseStep = 0.12;

int scaled(const int logicalPixels, const UINT dpi) { return MulDiv(logicalPixels, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI); }

}  // namespace

OverlayTaskbarWidget::OverlayTaskbarWidget(QWidget* parent)
    : OverlayHubView(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus), m_animationTimer(new QTimer(this)) {
  setObjectName("overlayTaskbarWidget");
  setAttribute(Qt::WA_TranslucentBackground, true);
  setAttribute(Qt::WA_ShowWithoutActivating, true);
  setAttribute(Qt::WA_QuitOnClose, false);
  resize(kMinimumLogicalWidth, kLogicalHeight);
  setCursor(Qt::ArrowCursor);

  m_animationTimer->setInterval(kAnimationIntervalMs);
  m_animationTimer->setTimerType(Qt::CoarseTimer);
  connect(m_animationTimer, &QTimer::timeout, this, [this]() {
    m_wavePhase = std::fmod(m_wavePhase + kWavePhaseStep, 2.0 * std::numbers::pi_v<qreal>);
    update();
  });
  connect(&ThemeManager::instance(), &ThemeManager::systemThemeChanged, this, [this](Theme) { update(); });
}

void OverlayTaskbarWidget::updateUsage(const core::OverlayRuntime& runtime) {
  m_runtime = runtime;
  m_hasRuntime = true;
  update();
  updateAnimationTimer();
}

void OverlayTaskbarWidget::setUsageUnavailable() {
  m_hasRuntime = false;
  update();
  updateAnimationTimer();
}

void OverlayTaskbarWidget::setFilterEnabled(const bool enabled) {
  m_filterEnabled = enabled;
  update();
  updateAnimationTimer();
}

void OverlayTaskbarWidget::contextMenuEvent(QContextMenuEvent* ev) {
  QMenu menu(this);
  QAction* showMainAct = menu.addAction(I18n::tr("Show main window"));
  QAction* hideTaskbarAct = menu.addAction(I18n::tr("Hide taskbar view"));
  menu.addSeparator();
  QAction* exitAct = menu.addAction(I18n::tr("Exit application"));
  QAction* picked = menu.exec(ev->globalPos());
  if (picked == showMainAct) {
    emit showMainWindowRequested();
  } else if (picked == hideTaskbarAct) {
    emit hideViewRequested();
  } else if (picked == exitAct) {
    emit exitApplicationRequested();
  }
}

void OverlayTaskbarWidget::mouseReleaseEvent(QMouseEvent* ev) {
  if (ev->button() == Qt::LeftButton) {
    emit showMainWindowRequested();
    ev->accept();
    return;
  }
  QWidget::mouseReleaseEvent(ev);
}

void OverlayTaskbarWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  const auto colors = overlayHudPalette(ThemeManager::instance().systemTheme());

  const QRectF surface = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
  const bool showUsage = m_filterEnabled && m_hasRuntime;
  paintOverlayHud(painter, surface, kRadius, colors.taskbarSurface, colors, m_runtime, showUsage, m_wavePhase);
  painter.setPen(colors.text);
  QFont textFont = font();
  textFont.setWeight(QFont::DemiBold);
  painter.setFont(textFont);
  painter.drawText(rect(), Qt::AlignCenter, showUsage ? overlayUsageText(m_runtime) : QStringLiteral("—"));
  if (!m_hasPainted) {
    m_hasPainted = true;
    QTimer::singleShot(0, this, &OverlayTaskbarWidget::notifyPresentationChanged);
  }
}

bool OverlayTaskbarWidget::attachPresentation() {
  releaseInvalidNativeWindow();
  if (!attachAndPosition()) return false;
  if (!isVisible()) show();
  updateAnimationTimer();
  return true;
}

void OverlayTaskbarWidget::detachPresentation() {
  m_animationTimer->stop();
  m_hasPainted = false;
  hide();
  releaseNativeWindow();
}

bool OverlayTaskbarWidget::attachAndPosition() {
  const HWND taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
  const HWND notify = taskbar ? FindWindowExW(taskbar, nullptr, L"TrayNotifyWnd", nullptr) : nullptr;
  if (!taskbar || !notify) return false;

  if (!ensureNativeWindow()) return false;
  const HWND widget = reinterpret_cast<HWND>(internalWinId());

  if (GetParent(widget) != taskbar) {
    LONG_PTR style = GetWindowLongPtrW(widget, GWL_STYLE);
    style &= ~static_cast<LONG_PTR>(WS_POPUP);
    style |= static_cast<LONG_PTR>(WS_CHILD);
    SetWindowLongPtrW(widget, GWL_STYLE, style);

    LONG_PTR exStyle = GetWindowLongPtrW(widget, GWL_EXSTYLE);
    exStyle &= ~static_cast<LONG_PTR>(WS_EX_APPWINDOW);
    exStyle |= static_cast<LONG_PTR>(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
    SetWindowLongPtrW(widget, GWL_EXSTYLE, exStyle);
    SetLastError(ERROR_SUCCESS);
    const HWND previousParent = SetParent(widget, taskbar);
    if (!previousParent && GetLastError() != ERROR_SUCCESS) return false;
  }

  RECT taskbarRect{};
  RECT notifyRect{};
  if (!GetWindowRect(taskbar, &taskbarRect) || !GetWindowRect(notify, &notifyRect)) return false;

  POINT notifyTopLeft{notifyRect.left, notifyRect.top};
  if (!ScreenToClient(taskbar, &notifyTopLeft)) return false;

  const UINT dpi = std::max(GetDpiForWindow(taskbar), static_cast<UINT>(USER_DEFAULT_SCREEN_DPI));
  const int taskbarWidth = static_cast<int>(taskbarRect.right - taskbarRect.left);
  const int taskbarHeight = static_cast<int>(taskbarRect.bottom - taskbarRect.top);
  const int notifyX = static_cast<int>(notifyTopLeft.x);
  const int notifyY = static_cast<int>(notifyTopLeft.y);
  const int nativeWidth = scaled(desiredLogicalWidth(), dpi);
  const int nativeHeight = std::min(scaled(kLogicalHeight, dpi), std::max(taskbarHeight - scaled(2 * kTaskbarInset, dpi), 1));
  const bool horizontal = taskbarWidth >= taskbarHeight;

  int x = 0;
  int y = 0;
  if (horizontal) {
    x = notifyX - nativeWidth - scaled(kTaskbarInset, dpi);
    y = (taskbarHeight - nativeHeight) / 2;
  } else {
    x = (taskbarWidth - nativeWidth) / 2;
    y = notifyY - nativeHeight - scaled(kTaskbarInset, dpi);
  }

  return SetWindowPos(widget, HWND_TOP, std::max(x, 0), std::max(y, 0), nativeWidth, nativeHeight, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW) !=
         FALSE;
}

bool OverlayTaskbarWidget::ensureNativeWindow() {
  releaseInvalidNativeWindow();

  HWND widget = reinterpret_cast<HWND>(internalWinId());
  DWORD processId = 0;
  if (widget && IsWindow(widget)) GetWindowThreadProcessId(widget, &processId);
  if (widget && IsWindow(widget) && processId == GetCurrentProcessId()) return true;

  // Explorer owns the taskbar parent and destroys all of its native child
  // windows when it restarts.  The QWidget and its QObject children remain
  // alive, so first release Qt's stale platform-window state, then create a
  // fresh HWND that can be embedded into the new taskbar.
  m_hasPainted = false;
  QWidget::create(0, true, false);

  widget = reinterpret_cast<HWND>(internalWinId());
  processId = 0;
  if (widget && IsWindow(widget)) GetWindowThreadProcessId(widget, &processId);
  return widget && IsWindow(widget) && processId == GetCurrentProcessId();
}

void OverlayTaskbarWidget::releaseInvalidNativeWindow() {
  const HWND widget = reinterpret_cast<HWND>(internalWinId());
  if (!widget) return;

  DWORD processId = 0;
  if (IsWindow(widget)) GetWindowThreadProcessId(widget, &processId);
  if (IsWindow(widget) && processId == GetCurrentProcessId()) return;
  releaseNativeWindow();
}

void OverlayTaskbarWidget::releaseNativeWindow() {
  if (!internalWinId()) return;
  m_hasPainted = false;
  QWidget::destroy(true, false);
}

void OverlayTaskbarWidget::updateAnimationTimer() {
  const uint64_t totalMb = overlayTotalMb(m_runtime);
  const bool animate = presentationRequested() && isVisible() && m_filterEnabled && m_hasRuntime && m_runtime.currentConsumptionMb > 0 &&
                       static_cast<uint64_t>(m_runtime.currentConsumptionMb) < totalMb;
  if (animate) {
    if (!m_animationTimer->isActive()) m_animationTimer->start();
  } else {
    m_animationTimer->stop();
  }
}

int OverlayTaskbarWidget::desiredLogicalWidth() const {
  const bool showUsage = m_filterEnabled && m_hasRuntime;
  const QString text = showUsage ? overlayUsageText(m_runtime) : QStringLiteral("—");
  QFont textFont = font();
  textFont.setWeight(QFont::DemiBold);
  return std::max(kMinimumLogicalWidth, QFontMetrics(textFont).horizontalAdvance(text) + 2 * kHorizontalPadding);
}

bool OverlayTaskbarWidget::verifyPresentation() const {
  if (!presentationRequested() || !m_hasPainted || !isVisible()) return false;

  const HWND taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
  const HWND notify = taskbar ? FindWindowExW(taskbar, nullptr, L"TrayNotifyWnd", nullptr) : nullptr;
  const HWND widget = reinterpret_cast<HWND>(internalWinId());
  if (!taskbar || !notify || !widget || !IsWindow(widget) || !IsWindowVisible(widget) || GetParent(widget) != taskbar) return false;

  const LONG_PTR style = GetWindowLongPtrW(widget, GWL_STYLE);
  if ((style & static_cast<LONG_PTR>(WS_CHILD)) == 0 || (style & static_cast<LONG_PTR>(WS_POPUP)) != 0) return false;

  RECT widgetRect{};
  RECT taskbarRect{};
  RECT notifyRect{};
  RECT intersection{};
  if (!GetWindowRect(widget, &widgetRect) || !GetWindowRect(taskbar, &taskbarRect) || !GetWindowRect(notify, &notifyRect) ||
      widgetRect.right <= widgetRect.left || widgetRect.bottom <= widgetRect.top || !IntersectRect(&intersection, &widgetRect, &taskbarRect)) {
    return false;
  }

  const bool horizontal = (taskbarRect.right - taskbarRect.left) >= (taskbarRect.bottom - taskbarRect.top);
  if ((horizontal && widgetRect.right > notifyRect.left) || (!horizontal && widgetRect.bottom > notifyRect.top)) return false;

  DWORD cloaked = 0;
  if (SUCCEEDED(DwmGetWindowAttribute(widget, DWMWA_CLOAKED, &cloaked, static_cast<DWORD>(sizeof(cloaked)))) && cloaked != 0) return false;
  return true;
}

}  // namespace uwf::ui
