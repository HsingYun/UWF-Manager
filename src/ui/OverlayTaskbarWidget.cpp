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
#include <QWindow>
#include <algorithm>
#include <cmath>
#include <numbers>

#include "I18n.h"
#include "OverlayHudPalette.h"
#include "OverlayHudRenderer.h"
#include "TaskbarLayoutCoordinator.h"
#include "Win11TaskbarEnvironment.h"

namespace uwf::ui {

namespace {

constexpr int kMinimumLogicalWidth = 96;
constexpr int kLogicalHeight = 32;
constexpr int kHorizontalPadding = 8;
constexpr int kAnimationIntervalMs = 100;
constexpr int kToolTipDelayMs = 800;
constexpr int kRadius = 6;
constexpr qreal kWavePhaseStep = 0.12;

enum class TaskbarEdge { Left, Top, Right, Bottom };

TaskbarEdge taskbarEdge(const QRect& hubRect, const QRect& screenRect) {
  const int leftDistance = std::abs(hubRect.left() - screenRect.left());
  const int topDistance = std::abs(hubRect.top() - screenRect.top());
  const int rightDistance = std::abs(screenRect.right() - hubRect.right());
  const int bottomDistance = std::abs(screenRect.bottom() - hubRect.bottom());
  const int minimumDistance = std::min({leftDistance, topDistance, rightDistance, bottomDistance});
  if (leftDistance == minimumDistance) return TaskbarEdge::Left;
  if (topDistance == minimumDistance) return TaskbarEdge::Top;
  if (rightDistance == minimumDistance) return TaskbarEdge::Right;
  return TaskbarEdge::Bottom;
}

QPoint taskbarPopupPosition(const QPoint& anchor, const QSize& popupSize, const QRect& hubRect, const QScreen& screen, const int gap) {
  const QRect screenRect = screen.geometry();
  const QRect availableRect = screen.availableGeometry();
  const TaskbarEdge edge = taskbarEdge(hubRect, screenRect);
  const bool reservesLeft = availableRect.left() > screenRect.left();
  const bool reservesTop = availableRect.top() > screenRect.top();
  const bool reservesRight = availableRect.right() < screenRect.right();
  const bool reservesBottom = availableRect.bottom() < screenRect.bottom();
  int x = anchor.x();
  int y = anchor.y();
  if (edge == TaskbarEdge::Left) {
    x = reservesLeft ? availableRect.left() + gap : hubRect.right() + gap;
  } else if (edge == TaskbarEdge::Top) {
    y = reservesTop ? availableRect.top() + gap : hubRect.bottom() + gap;
  } else if (edge == TaskbarEdge::Right) {
    x = reservesRight ? availableRect.right() - popupSize.width() - gap + 1 : hubRect.left() - popupSize.width() - gap;
  } else {
    y = reservesBottom ? availableRect.bottom() - popupSize.height() - gap + 1 : hubRect.top() - popupSize.height() - gap;
  }

  x = std::clamp(x, screenRect.left(), std::max(screenRect.left(), screenRect.right() - popupSize.width() + 1));
  y = std::clamp(y, screenRect.top(), std::max(screenRect.top(), screenRect.bottom() - popupSize.height() + 1));
  return {x, y};
}

QPalette systemToolTipPalette(const Theme theme) {
  QPalette palette;
  if (theme == Theme::Light) {
    palette.setColor(QPalette::ToolTipBase, QColor(0xF7, 0xF7, 0xF7, 242));
    palette.setColor(QPalette::ToolTipText, QColor(0x1A, 0x1A, 0x1A));
    palette.setColor(QPalette::Mid, QColor(0xCF, 0xCF, 0xCF, 230));
  } else {
    palette.setColor(QPalette::ToolTipBase, QColor(0x2B, 0x2B, 0x2B, 238));
    palette.setColor(QPalette::ToolTipText, QColor(0xF5, 0xF5, 0xF5));
    palette.setColor(QPalette::Mid, QColor(0x50, 0x50, 0x50, 224));
  }
  return palette;
}

class ToolTipLabel final : public QLabel {
 public:
  explicit ToolTipLabel(const Qt::WindowFlags flags) : QLabel(nullptr, flags) { setContentsMargins(8, 5, 8, 5); }

 protected:
  void paintEvent(QPaintEvent*) override {
    const QColor foreground = palette().color(QPalette::ToolTipText);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(palette().color(QPalette::Mid), 1.0));
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
      m_layoutCoordinator(createDefaultTaskbarLayoutCoordinator([this](const TaskbarLayoutCoordinator::DetachEvent event) {
        if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Started) {
          notifyHostPresentationReleaseStarted();
          return;
        }
        if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Blocked) {
          notifyPresentationReleaseBlocked();
          return;
        }
        if (event.nativeWindowResetRequired) resetNativeWindow();
        // Explorer 重建直接触发重新发现；普通释放则把完成事件交回 HubView，
        // 由 Recovering 状态决定兑现显式重开，或进入稳定 fallback/低频探测。
        if (event.reason == TaskbarLayoutCoordinator::DetachReason::HostInvalidated) {
          notifyHostPresentationReleaseCompleted();
        } else {
          notifyPresentationReleaseCompleted();
        }
      })) {
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
  connect(&ThemeManager::instance(), &ThemeManager::systemThemeChanged, this, [this](const Theme theme) {
    update();
    if (!m_toolTipLabel) return;
    m_toolTipLabel->setPalette(systemToolTipPalette(theme));
    m_toolTipLabel->update();
  });
}

OverlayTaskbarWidget::~OverlayTaskbarWidget() {
  closeContextMenu();
  m_contextMenu.clear();
}

bool OverlayTaskbarWidget::isCompatible() const { return m_layoutCoordinator->isCompatible(); }

void OverlayTaskbarWidget::updateUsage(const core::OverlayRuntime& runtime) {
  m_runtime = runtime;
  m_hasRuntime = true;
  synchronizeContentGeometry();
  update();
  updateAnimationTimer();
}

void OverlayTaskbarWidget::setUsageUnavailable() {
  m_hasRuntime = false;
  synchronizeContentGeometry();
  update();
  updateAnimationTimer();
}

void OverlayTaskbarWidget::setFilterEnabled(const bool enabled) {
  m_filterEnabled = enabled;
  synchronizeContentGeometry();
  update();
  updateAnimationTimer();
}

void OverlayTaskbarWidget::contextMenuEvent(QContextMenuEvent* ev) {
  m_toolTipTimer->stop();
  hideToolTip();
  if (m_contextMenu) {
    closeContextMenu();
    return;
  }

  // Native attachment makes this QWidget an Explorer child even though Qt
  // still models it as a window. A popup parented to it gets an invalid
  // transient parent and can be visible without receiving QAction input.
  auto* const menu = new QMenu();
  m_contextMenu = menu;
  addApplicationTitleToMenu(*menu);
  QAction* const showMainAct = menu->addAction(I18n::tr("Show main window"));
  QAction* const hideHubAct = menu->addAction(I18n::tr("Hide overlay hub"));
  menu->addSeparator();
  QAction* const exitAct = menu->addAction(I18n::tr("Exit application"));
  connect(showMainAct, &QAction::triggered, this, &OverlayTaskbarWidget::showMainWindowRequested);
  connect(hideHubAct, &QAction::triggered, this, &OverlayTaskbarWidget::hideHubRequested);
  connect(exitAct, &QAction::triggered, this, &OverlayTaskbarWidget::exitApplicationRequested);
  connect(menu, &QMenu::aboutToHide, this, [this, menu]() {
    if (m_contextMenu == menu) m_contextMenu.clear();
    menu->deleteLater();
  });
  menu->ensurePolished();
  const QRect hubRect(mapToGlobal(QPoint(0, 0)), size());
  QScreen* screen = QGuiApplication::screenAt(hubRect.center());
  if (!screen) screen = QGuiApplication::primaryScreen();
  if (!screen) {
    menu->deleteLater();
    m_contextMenu.clear();
    return;
  }
  const QPoint position = taskbarPopupPosition(ev->globalPos(), menu->sizeHint(), hubRect, *screen, 6);
  menu->popup(position);
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

OverlayHubView::AttachResult OverlayTaskbarWidget::acquirePresentation() {
  // Win11 策略通过 SetWindowPos 设置原生 HWND 的物理像素尺寸，但 QWidget
  // 在第一次 show() 时还会应用自身保存的逻辑几何。先完成 polish 并同步最终
  // 逻辑尺寸，避免 show() 把策略刚设置的宽度覆盖回构造期的 96px，随后又被
  // 健康检查放大，形成启动时先窄后宽的闪动。
  ensurePolished();
  const QSize desiredSize(desiredLogicalWidth(), kLogicalHeight);
  if (size() != desiredSize) resize(desiredSize);

  const auto environmentProbe = win11_taskbar::probeEnvironment();
  const HWND taskbar = environmentProbe.environment ? environmentProbe.environment->taskbar : nullptr;
  const win11_taskbar::ScopedThreadDpiAwareness dpiScope(taskbar);
  (void)winId();
  QWindow* const window = windowHandle();
  if (!window) return AttachResult::Failed;
  const auto result = m_layoutCoordinator->prepareAttach(
      window, desiredSize,
      [this]() {
        if (!isVisible()) show();
        const HWND nativeWindow = reinterpret_cast<HWND>(internalWinId());
        if (nativeWindow && !IsWindowVisible(nativeWindow)) (void)ShowWindow(nativeWindow, SW_SHOWNOACTIVATE);
      },
      [this]() {
        if (isVisible()) hide();
      });
  if (result == TaskbarLayoutCoordinator::AttachResult::TemporarilyUnavailable) return AttachResult::TemporarilyUnavailable;
  if (result == TaskbarLayoutCoordinator::AttachResult::ReleasePending) return AttachResult::ReleasePending;
  if (result == TaskbarLayoutCoordinator::AttachResult::ReleaseBlocked) return AttachResult::ReleaseBlocked;
  if (result == TaskbarLayoutCoordinator::AttachResult::NativeWindowDestroyed) {
    resetNativeWindow();
    return AttachResult::Failed;
  }
  if (result != TaskbarLayoutCoordinator::AttachResult::Prepared) return AttachResult::Failed;
  return AttachResult::Prepared;
}

OverlayHubView::AttachResult OverlayTaskbarWidget::activatePresentation() {
  const auto result = m_layoutCoordinator->activatePrepared();
  if (result == TaskbarLayoutCoordinator::AttachResult::TemporarilyUnavailable) return AttachResult::TemporarilyUnavailable;
  if (result == TaskbarLayoutCoordinator::AttachResult::ReleasePending) return AttachResult::ReleasePending;
  if (result == TaskbarLayoutCoordinator::AttachResult::ReleaseBlocked) return AttachResult::ReleaseBlocked;
  if (result == TaskbarLayoutCoordinator::AttachResult::NativeWindowDestroyed) {
    resetNativeWindow();
    return AttachResult::Failed;
  }
  if (result != TaskbarLayoutCoordinator::AttachResult::Attached) return AttachResult::Failed;
  updateAnimationTimer();
  return AttachResult::Attached;
}

void OverlayTaskbarWidget::suspendPresentation() {
  m_animationTimer->stop();
  m_toolTipTimer->stop();
  m_pointerInside = false;
  m_hasPainted = false;
  hideToolTip();
  closeContextMenu();
  // suspend 只改变 QWidget 可见性并保留任务栏 attachment，供瞬时条件恢复后
  // 原地确认；完整 detach 才恢复父关系和原生样式。
  hide();
}

OverlayHubView::ReleaseResult OverlayTaskbarWidget::detachPresentation(const ReleaseReason) {
  suspendPresentation();
  return m_layoutCoordinator->detach() == TaskbarLayoutCoordinator::DetachStatus::Pending ? ReleaseResult::Pending : ReleaseResult::Complete;
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
  label.setPalette(systemToolTipPalette(ThemeManager::instance().systemTheme()));
  label.setText(I18n::applicationTitle());
  label.adjustSize();

  const QRect hubRect(mapToGlobal(QPoint(0, 0)), size());
  QScreen* screen = QGuiApplication::screenAt(hubRect.center());
  if (!screen) screen = QGuiApplication::primaryScreen();
  if (!screen) return;

  const QPoint anchor(hubRect.center().x() - label.width() / 2, hubRect.center().y() - label.height() / 2);
  label.move(taskbarPopupPosition(anchor, label.size(), hubRect, *screen, 4));
  label.show();
}

void OverlayTaskbarWidget::hideToolTip() {
  if (m_toolTipLabel) m_toolTipLabel->hide();
}

void OverlayTaskbarWidget::closeContextMenu() {
  if (m_contextMenu) m_contextMenu->close();
  m_contextMenu.clear();
}

void OverlayTaskbarWidget::resetNativeWindow() {
  m_animationTimer->stop();
  m_toolTipTimer->stop();
  m_pointerInside = false;
  hideToolTip();
  m_toolTipLabel.reset();
  closeContextMenu();
  m_hasPainted = false;
  releaseMouse();
  if (internalWinId()) QWidget::destroy(true, false);
}

void OverlayTaskbarWidget::synchronizeContentGeometry() {
  const QSize desiredSize(desiredLogicalWidth(), kLogicalHeight);
  if (size() == desiredSize) return;
  resize(desiredSize);
  if (!presentationConfirmed()) return;
  // Hub may be in the middle of propagating one runtime snapshot to every
  // view. Defer the layout transaction until that propagation is complete.
  QTimer::singleShot(0, this, [this]() {
    if (presentationConfirmed()) requestPresentationRefresh();
  });
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

int OverlayTaskbarWidget::retryIntervalMs(const int consecutiveFailures) const {
  if (consecutiveFailures <= 1) return 1000;
  if (consecutiveFailures == 2) return 2000;
  if (consecutiveFailures == 3) return 5000;
  return 10000;
}

OverlayHubView::VerificationResult OverlayTaskbarWidget::verifyPresentation() const {
  if (!presentationRequested() || !isVisible()) return VerificationResult::Invalid;
  const WId currentWindowId = internalWinId();
  if (!currentWindowId || !windowHandle()) return VerificationResult::Invalid;
  const auto layoutResult = m_layoutCoordinator->verify(windowHandle(), currentWindowId);
  switch (layoutResult) {
    case TaskbarLayoutStrategy::VerificationResult::Confirmed:
      return m_hasPainted ? VerificationResult::Confirmed : VerificationResult::Pending;
    case TaskbarLayoutStrategy::VerificationResult::Retained:
      return m_hasPainted ? VerificationResult::Retained : VerificationResult::Pending;
    case TaskbarLayoutStrategy::VerificationResult::RefreshRequired:
      return m_hasPainted ? VerificationResult::RefreshRequired : VerificationResult::Pending;
    case TaskbarLayoutStrategy::VerificationResult::Invalid:
      return VerificationResult::Invalid;
  }
  Q_UNREACHABLE_RETURN(VerificationResult::Invalid);
}

}  // namespace uwf::ui
