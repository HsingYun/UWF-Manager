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

#include <windows.h>

#include <QContextMenuEvent>
#include <QEnterEvent>
#include <QEvent>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QToolTip>
#include <algorithm>
#include <cmath>
#include <numbers>

#include "I18n.h"
#include "OverlayHudPalette.h"
#include "OverlayHudRenderer.h"
#include "TaskbarLayoutCoordinator.h"

namespace uwf::ui {

namespace {

constexpr int kMinimumLogicalWidth = 96;
constexpr int kLogicalHeight = 32;
constexpr int kHorizontalPadding = 8;
constexpr int kAnimationIntervalMs = 100;
constexpr int kToolTipDelayMs = 800;
constexpr int kRadius = 6;
constexpr qreal kWavePhaseStep = 0.12;

enum class TaskbarEdge { Unknown, Left, Top, Right, Bottom };

class ToolTipLabel final : public QLabel {
 public:
  explicit ToolTipLabel(const Qt::WindowFlags flags) : QLabel(nullptr, flags) { setContentsMargins(8, 5, 8, 5); }

 protected:
  void paintEvent(QPaintEvent*) override {
    const QColor foreground = palette().color(QPalette::ToolTipText);
    QColor border = foreground;
    border.setAlpha(80);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(border, 1.0));
    painter.setBrush(palette().color(QPalette::ToolTipBase));
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 5.0, 5.0);
    painter.setPen(foreground);
    painter.setFont(font());
    painter.drawText(contentsRect(), static_cast<int>(alignment()), text());
  }
};

}  // namespace

OverlayTaskbarWidget::OverlayTaskbarWidget(QWidget* parent)
    : OverlayHubView(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus),
      m_animationTimer(new QTimer(this)),
      m_toolTipTimer(new QTimer(this)),
      m_layoutCoordinator(createDefaultTaskbarLayoutCoordinator([this]() { requestPresentationRefresh(); })) {
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

  m_toolTipTimer->setSingleShot(true);
  m_toolTipTimer->setInterval(kToolTipDelayMs);
  connect(m_toolTipTimer, &QTimer::timeout, this, [this]() {
    if (m_pointerInside && presentationRequested() && isVisible()) showToolTip();
  });
  connect(&ThemeManager::instance(), &ThemeManager::systemThemeChanged, this, [this](Theme) { update(); });
}

OverlayTaskbarWidget::~OverlayTaskbarWidget() = default;

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
  m_toolTipTimer->stop();
  hideToolTip();
  QMenu menu(this);
  addApplicationTitleToMenu(menu);
  QAction* showMainAct = menu.addAction(I18n::tr("Show main window"));
  QAction* hideHubAct = menu.addAction(I18n::tr("Hide overlay hub"));
  menu.addSeparator();
  QAction* exitAct = menu.addAction(I18n::tr("Exit application"));
  QAction* picked = menu.exec(ev->globalPos());
  if (picked == showMainAct) {
    emit showMainWindowRequested();
  } else if (picked == hideHubAct) {
    emit hideHubRequested();
  } else if (picked == exitAct) {
    emit exitApplicationRequested();
  }
}

void OverlayTaskbarWidget::enterEvent(QEnterEvent* ev) {
  m_pointerInside = true;
  m_toolTipTimer->start();
  OverlayHubView::enterEvent(ev);
}

void OverlayTaskbarWidget::leaveEvent(QEvent* ev) {
  m_pointerInside = false;
  m_toolTipTimer->stop();
  hideToolTip();
  OverlayHubView::leaveEvent(ev);
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
  // Win11 策略通过 SetWindowPos 设置原生 HWND 的物理像素尺寸，但 QWidget
  // 在第一次 show() 时还会应用自身保存的逻辑几何。先完成 polish 并同步最终
  // 逻辑尺寸，避免 show() 把策略刚设置的宽度覆盖回构造期的 96px，随后又被
  // 健康检查放大，形成启动时先窄后宽的闪动。
  ensurePolished();
  const QSize desiredSize(desiredLogicalWidth(), kLogicalHeight);
  if (size() != desiredSize) resize(desiredSize);

  releaseInvalidNativeWindow();
  if (!ensureNativeWindow()) return false;
  if (!m_layoutCoordinator->attach(internalWinId(), desiredSize)) return false;
  if (!isVisible()) show();
  updateAnimationTimer();
  return true;
}

void OverlayTaskbarWidget::detachPresentation() {
  m_animationTimer->stop();
  m_toolTipTimer->stop();
  m_pointerInside = false;
  m_hasPainted = false;
  hideToolTip();
  m_layoutCoordinator->detach(internalWinId());
  hide();
  releaseNativeWindow();
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

void OverlayTaskbarWidget::showToolTip() {
  if (!m_toolTipLabel) {
    m_toolTipLabel = std::make_unique<ToolTipLabel>(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    m_toolTipLabel->setAttribute(Qt::WA_ShowWithoutActivating, true);
    m_toolTipLabel->setAttribute(Qt::WA_QuitOnClose, false);
    m_toolTipLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_toolTipLabel->setTextFormat(Qt::PlainText);
    m_toolTipLabel->setAlignment(Qt::AlignCenter);
    m_toolTipLabel->setAutoFillBackground(false);
  }

  QLabel& label = *m_toolTipLabel;
  label.setFont(QToolTip::font());
  label.setPalette(QToolTip::palette());
  label.setText(I18n::applicationTitle());
  label.adjustSize();

  const QRect hubRect(mapToGlobal(QPoint(0, 0)), size());
  QScreen* screen = QGuiApplication::screenAt(hubRect.center());
  if (!screen) screen = QGuiApplication::primaryScreen();
  if (!screen) return;

  const QRect screenRect = screen->geometry();
  const QRect availableRect = screen->availableGeometry();
  const int leftInset = availableRect.left() - screenRect.left();
  const int bottomInset = screenRect.bottom() - availableRect.bottom();
  const int topInset = availableRect.top() - screenRect.top();
  const int rightInset = screenRect.right() - availableRect.right();
  const int maximumInset = std::max({leftInset, topInset, rightInset, bottomInset});
  constexpr int kGap = 4;

  int x = hubRect.center().x() - label.width() / 2;
  int y = hubRect.center().y() - label.height() / 2;
  TaskbarEdge edge = TaskbarEdge::Unknown;
  if (maximumInset > 0) {
    if (leftInset == maximumInset)
      edge = TaskbarEdge::Left;
    else if (topInset == maximumInset)
      edge = TaskbarEdge::Top;
    else if (rightInset == maximumInset)
      edge = TaskbarEdge::Right;
    else
      edge = TaskbarEdge::Bottom;
  } else {
    const int leftDistance = std::abs(hubRect.center().x() - screenRect.left());
    const int topDistance = std::abs(hubRect.center().y() - screenRect.top());
    const int rightDistance = std::abs(screenRect.right() - hubRect.center().x());
    const int bottomDistance = std::abs(screenRect.bottom() - hubRect.center().y());
    const int minimumDistance = std::min({leftDistance, topDistance, rightDistance, bottomDistance});
    if (leftDistance == minimumDistance)
      edge = TaskbarEdge::Left;
    else if (topDistance == minimumDistance)
      edge = TaskbarEdge::Top;
    else if (rightDistance == minimumDistance)
      edge = TaskbarEdge::Right;
    else
      edge = TaskbarEdge::Bottom;
  }

  if (edge == TaskbarEdge::Left) {
    x = availableRect.left() + kGap;
  } else if (edge == TaskbarEdge::Top) {
    y = availableRect.top() + kGap;
  } else if (edge == TaskbarEdge::Right) {
    x = availableRect.right() - label.width() - kGap + 1;
  } else {
    y = availableRect.bottom() - label.height() - kGap + 1;
  }

  x = std::clamp(x, availableRect.left(), std::max(availableRect.left(), availableRect.right() - label.width() + 1));
  y = std::clamp(y, availableRect.top(), std::max(availableRect.top(), availableRect.bottom() - label.height() + 1));
  label.move(x, y);
  label.show();
  label.raise();
}

void OverlayTaskbarWidget::hideToolTip() {
  if (m_toolTipLabel) m_toolTipLabel->hide();
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
  hideToolTip();
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
  return presentationRequested() && m_hasPainted && isVisible() && m_layoutCoordinator->verify(internalWinId());
}

}  // namespace uwf::ui
