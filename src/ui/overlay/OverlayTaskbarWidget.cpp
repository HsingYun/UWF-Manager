// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
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
#include <array>
#include <cmath>
#include <numbers>

#include "OverlayContextMenu.h"
#include "OverlayHudPalette.h"
#include "OverlayHudRenderer.h"
#include "presentation/overlay/OverlayViewModel.h"
#include "ui/appearance/I18n.h"
#include "ui/overlay/taskbar/TaskbarLayoutCoordinatorFactory.h"
#include "ui/overlay/taskbar/Win11TaskbarEnvironment.h"
#include "ui/widgets/HoverToolTip.h"

namespace uwf::ui {

namespace {

constexpr int kMinimumLogicalWidth = 96;
constexpr int kLogicalHeight = 32;
constexpr int kHorizontalPadding = 8;
constexpr int kAnimationIntervalMs = 100;
constexpr int kToolTipDelayMs = HoverToolTip::delayMs;
constexpr int kRadius = 6;
constexpr qreal kWavePhaseStep = 0.12;
constexpr std::array kRecoveryRetryIntervalsMs{1000, 2000, 5000, 10000};

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

}  // namespace

OverlayTaskbarWidget::OverlayTaskbarWidget(QWidget* parent)
    : OverlayHubView(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus),
      m_animationTimer(new QTimer(this)),
      m_toolTipTimer(new QTimer(this)),
      m_layoutCoordinator(
          createDefaultTaskbarLayoutCoordinator([this](const TaskbarLayoutCoordinator::DetachEvent event) { handleLayoutDetachEvent(event); })) {
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
  connect(&ThemeManager::instance(), &ThemeManager::systemThemeChanged, this, [this](const app::Theme) {
    update();
    if (!m_toolTipLabel) return;
    m_toolTipLabel->update();
  });
}

OverlayTaskbarWidget::~OverlayTaskbarWidget() {
  closeContextMenu();
  m_contextMenu.clear();
}

bool OverlayTaskbarWidget::isCompatible() const { return m_layoutCoordinator->isCompatible(); }

void OverlayTaskbarWidget::applyUsageState(const app::OverlayUsageState& state) {
  m_hoverContent = presentation::overlayUsageToolTip(state);
  if (m_toolTipLabel) m_toolTipLabel->setText(m_hoverContent);
  const auto* const enabled = std::get_if<app::OverlayUsageEnabled>(&state);
  m_filterEnabled = enabled != nullptr;
  m_hasRuntime = enabled != nullptr;
  if (enabled) m_runtime = enabled->runtime;
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
  auto* const menu = new OverlayContextMenu();
  m_contextMenu = menu;
  connectContextMenu(*menu);
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
        const HWND attachedWindow = reinterpret_cast<HWND>(internalWinId());
        const win11_taskbar::ScopedThreadDpiAwareness dpiScope(attachedWindow ? GetParent(attachedWindow) : nullptr);
        if (!isVisible()) show();
        const HWND nativeWindow = reinterpret_cast<HWND>(internalWinId());
        if (nativeWindow && !IsWindowVisible(nativeWindow)) (void)ShowWindow(nativeWindow, SW_SHOWNOACTIVATE);
      },
      [this]() {
        if (isVisible()) hide();
      },
      [this]() { return recreateNativeWindow(); });
  switch (result) {
    case TaskbarLayoutCoordinator::AttachResult::Prepared:
      return AttachResult::Prepared;
    case TaskbarLayoutCoordinator::AttachResult::Retained:
      return AttachResult::Retained;
    case TaskbarLayoutCoordinator::AttachResult::TemporarilyUnavailable:
      return AttachResult::TemporarilyUnavailable;
    case TaskbarLayoutCoordinator::AttachResult::Incompatible:
      return AttachResult::Incompatible;
    case TaskbarLayoutCoordinator::AttachResult::ReleasePending:
      return AttachResult::ReleasePending;
    case TaskbarLayoutCoordinator::AttachResult::ReleaseBlocked:
      return AttachResult::ReleaseBlocked;
    case TaskbarLayoutCoordinator::AttachResult::Attached:
    case TaskbarLayoutCoordinator::AttachResult::Failed:
      return AttachResult::Failed;
  }
  Q_UNREACHABLE_RETURN(AttachResult::Failed);
}

OverlayHubView::AttachResult OverlayTaskbarWidget::activatePresentation() {
  switch (m_layoutCoordinator->activatePrepared()) {
    case TaskbarLayoutCoordinator::AttachResult::Attached:
      updateAnimationTimer();
      return AttachResult::Attached;
    case TaskbarLayoutCoordinator::AttachResult::Retained:
      return AttachResult::Retained;
    case TaskbarLayoutCoordinator::AttachResult::TemporarilyUnavailable:
      return AttachResult::TemporarilyUnavailable;
    case TaskbarLayoutCoordinator::AttachResult::Incompatible:
      return AttachResult::Incompatible;
    case TaskbarLayoutCoordinator::AttachResult::ReleasePending:
      return AttachResult::ReleasePending;
    case TaskbarLayoutCoordinator::AttachResult::ReleaseBlocked:
      return AttachResult::ReleaseBlocked;
    case TaskbarLayoutCoordinator::AttachResult::Prepared:
    case TaskbarLayoutCoordinator::AttachResult::Failed:
      return AttachResult::Failed;
  }
  Q_UNREACHABLE_RETURN(AttachResult::Failed);
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
  if (!m_toolTipLabel) m_toolTipLabel = std::make_unique<HoverToolTip>();
  auto& label = *m_toolTipLabel;
  label.setText(m_hoverContent.isEmpty() ? presentation::overlayUsageToolTip(app::OverlayUsageUnavailable{}) : m_hoverContent);
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

QWindow* OverlayTaskbarWidget::recreateNativeWindow() {
  const auto environmentProbe = win11_taskbar::probeEnvironment();
  const HWND taskbar = environmentProbe.environment ? environmentProbe.environment->taskbar : nullptr;
  const win11_taskbar::ScopedThreadDpiAwareness dpiScope(taskbar);
  m_animationTimer->stop();
  m_toolTipTimer->stop();
  m_pointerInside = false;
  hideToolTip();
  m_toolTipLabel.reset();
  closeContextMenu();
  m_hasPainted = false;
  releaseMouse();
  if (internalWinId()) QWidget::destroy(true, false);
  (void)winId();
  return windowHandle();
}

void OverlayTaskbarWidget::handleLayoutDetachEvent(const TaskbarLayoutCoordinator::DetachEvent& event) {
  if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Started) {
    notifyHostPresentationReleaseStarted();
    return;
  }
  if (event.phase == TaskbarLayoutCoordinator::DetachPhase::Blocked) {
    notifyPresentationReleaseBlocked();
    return;
  }
  // Explorer 重建直接触发重新发现；普通释放则把完成事件交回 HubView，
  // 由 Recovering 状态决定兑现显式重开，或进入稳定 fallback/低频探测。
  if (event.reason == TaskbarLayoutCoordinator::DetachReason::HostInvalidated)
    notifyHostPresentationReleaseCompleted();
  else
    notifyPresentationReleaseCompleted();
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
  const uint64_t totalMb = core::overlayTotalMb(m_runtime);
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
  const auto attempt = static_cast<std::size_t>(std::max(1, consecutiveFailures) - 1);
  return kRecoveryRetryIntervalsMs[std::min(attempt, kRecoveryRetryIntervalsMs.size() - 1)];
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

#include "moc_OverlayTaskbarWidget.cpp"
