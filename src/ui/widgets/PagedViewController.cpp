// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PagedViewController.h"

#include <QAbstractItemView>
#include <QEnterEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScopedValueRollback>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QStyle>
#include <QStyleOptionSlider>
#include <algorithm>

#include "PagedWindowLayout.h"
#include "design/DesignTokens.h"
#include "presentation/common/PageProxyModel.h"
namespace uwf::ui {
namespace {
class OverlayScrollBar final : public QScrollBar {
 public:
  explicit OverlayScrollBar(QWidget* parent) : QScrollBar(Qt::Vertical, parent) {}
  bool overThumb(const QPoint& position) const {
    QStyleOptionSlider option;
    initStyleOption(&option);
    return style()->subControlRect(QStyle::CC_ScrollBar, &option, QStyle::SC_ScrollBarSlider, this).contains(position);
  }
};
}  // namespace
PagedViewController::PagedViewController(QAbstractItemView* view, presentation::PageProxyModel& pages, const int rowHeight)
    : QObject(view), m_view(view), m_pages(&pages), m_overlay(new OverlayScrollBar(view)), m_idle(this), m_rowHeight(rowHeight) {
  pages.setContinuous(true);
  view->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_overlay->setObjectName(QStringLiteral("entryScrollOverlay"));
  m_overlay->hide();
  m_idle.setSingleShot(true);
  m_idle.setInterval(design::scrollbarHideDelayMs);
  connect(&m_idle, &QTimer::timeout, this, [this] {
    if (!m_overlay->isSliderDown() && !m_edgeHovered && !m_overlayHovered) m_overlay->hide();
  });
  auto* bar = view->verticalScrollBar();
  connect(bar, &QScrollBar::rangeChanged, this, [this] {
    syncOverlay();
    syncPage();
  });
  connect(bar, &QScrollBar::valueChanged, this, [this] {
    syncOverlay();
    syncPage();
  });
  connect(m_overlay, &QScrollBar::valueChanged, bar, &QScrollBar::setValue);
  connect(m_overlay, &QScrollBar::sliderPressed, this, &PagedViewController::beginDrag);
  connect(m_overlay, &QScrollBar::sliderReleased, &m_idle, qOverload<>(&QTimer::start));
  connect(&pages, &presentation::PageProxyModel::pageNavigationRequested, this, [this](int firstRow) {
    if (!m_pages) return;
    useItemScrolling();
    m_view->scrollTo(m_pages->index(firstRow, 0), QAbstractItemView::PositionAtTop);
    syncPage();
  });
  view->installEventFilter(this);
  view->viewport()->installEventFilter(this);
  view->horizontalScrollBar()->installEventFilter(this);
  view->viewport()->setMouseTracking(true);
  m_overlay->setMouseTracking(true);
  m_overlay->installEventFilter(this);
}
PagedViewController::~PagedViewController() {
  if (m_windowLayout) m_windowLayout->setPageVisible(*this, false);
}
void PagedViewController::syncPage() {
  if (!m_pages || m_changingMode) return;
  const auto* bar = m_view->verticalScrollBar();
  const int firstRow = m_view->verticalScrollMode() == QAbstractItemView::ScrollPerPixel ? bar->value() / m_rowHeight : bar->value();
  m_pages->syncScrollPosition(firstRow, bar->maximum() > 0 && bar->value() == bar->maximum());
}
void PagedViewController::syncOverlay() {
  if (m_changingMode) return;
  const QSignalBlocker blocker(m_overlay);
  const auto* bar = m_view->verticalScrollBar();
  m_overlay->setRange(bar->minimum(), bar->maximum());
  m_overlay->setPageStep(bar->pageStep());
  m_overlay->setSingleStep(bar->singleStep());
  m_overlay->setValue(bar->value());
  if (bar->maximum() == bar->minimum()) m_overlay->hide();
}
void PagedViewController::showOverlay() {
  if (m_view->verticalScrollBar()->maximum() <= 0) return;
  m_overlay->show();
  m_overlay->raise();
  m_idle.stop();
}
void PagedViewController::useItemScrolling() {
  if (m_view->verticalScrollMode() == QAbstractItemView::ScrollPerItem) return;
  const int firstRow = m_view->verticalScrollBar()->value() / m_rowHeight;
  {
    const QScopedValueRollback<bool> changing(m_changingMode, true);
    m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    m_view->verticalScrollBar()->setValue(firstRow);
  }
  syncOverlay();
}
void PagedViewController::beginDrag() {
  m_idle.stop();
  if (m_view->verticalScrollMode() == QAbstractItemView::ScrollPerPixel) return;
  const int firstRow = m_view->verticalScrollBar()->value();
  {
    const QScopedValueRollback<bool> changing(m_changingMode, true);
    m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_view->verticalScrollBar()->setValue(firstRow * m_rowHeight);
  }
  syncOverlay();
}
int PagedViewController::availableViewportHeight() const {
  int available = m_view->maximumViewportSize().height();
  const auto* horizontal = m_view->horizontalScrollBar();
  // maximumViewportSize includes an AlwaysOn bar, but assumes an AsNeeded
  // bar has no range. Account for a visible bar unless the style overlays it.
  if (m_view->horizontalScrollBarPolicy() == Qt::ScrollBarAsNeeded && horizontal->isVisibleTo(m_view) &&
      horizontal->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarOverlap, nullptr, horizontal) == 0)
    available -= horizontal->height();
  return std::max(0, available);
}
void PagedViewController::updateGeometry() {
  if (!m_pages || m_changingMode || m_updatingGeometry) return;
  const QScopedValueRollback<bool> updating(m_updatingGeometry, true);
  auto* viewport = m_view->viewport();
  const int available = availableViewportHeight();
  const int aligned = available / m_rowHeight * m_rowHeight;
  // Constrain the real viewport, so Qt computes its final scroll offset from
  // whole rows too. A paint mask leaves the final row scrolled behind it.
  // Derive this from the view's capacity, not its previously constrained size,
  // so an interior reflow can grow the viewport again without resizing a window.
  viewport->setMaximumHeight(aligned);
  viewport->resize(viewport->width(), aligned);
  m_pages->setPageSize(std::max(1, aligned / m_rowHeight));
  syncOverlay();
  const auto area = m_view->viewport()->geometry();
  m_overlay->setGeometry(m_view->layoutDirection() == Qt::RightToLeft ? area.left() + 1 : area.right() - design::scrollBarWidth, area.top() + 2,
                         design::scrollBarWidth, std::max(0, area.height() - 4));
  m_overlay->raise();
  syncPage();
}
bool PagedViewController::eventFilter(QObject* object, QEvent* event) {
  if (event->type() == QEvent::Resize || event->type() == QEvent::Show || event->type() == QEvent::LayoutDirectionChange ||
      (object == m_view->horizontalScrollBar() && event->type() == QEvent::Hide)) {
    updateGeometry();
    if (object == m_view && event->type() == QEvent::Show) {
      // Publish the constraint to the owning window, including plain QWidget
      // hosts. Only the window coordinator decides whether to resize.
      auto* layout = &PagedWindowLayout::forWindow(m_view->window());
      if (m_windowLayout && m_windowLayout != layout) m_windowLayout->setPageVisible(*this, false);
      m_windowLayout = layout;
      layout->setPageVisible(*this, true);
      QTimer::singleShot(0, this, [this] { updateGeometry(); });
    }
  }
  if ((object == m_view->viewport() || object == m_overlay) && event->type() == QEvent::Wheel) {
    useItemScrolling();
    syncPage();
    showOverlay();
    m_idle.start();
  }
  if (object == m_view->viewport()) {
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::Enter) {
      const auto position = event->type() == QEvent::MouseMove ? static_cast<QMouseEvent*>(event)->position() : static_cast<QEnterEvent*>(event)->position();
      m_edgeHovered = m_view->layoutDirection() == Qt::RightToLeft ? position.x() < 2 * design::scrollBarWidth
                                                                   : position.x() >= m_view->viewport()->width() - 2 * design::scrollBarWidth;
      if (m_edgeHovered)
        showOverlay();
      else
        m_idle.start();
    } else if (event->type() == QEvent::Leave) {
      m_edgeHovered = false;
      m_idle.start();
    }
  }
  if (object == m_overlay) {
    if (event->type() == QEvent::Enter) {
      m_overlayHovered = true;
      showOverlay();
    }
    if (event->type() == QEvent::Leave) {
      m_overlayHovered = false;
      m_idle.start();
    }
    if (event->type() == QEvent::MouseMove && m_overlay->isSliderDown()) beginDrag();
    if (event->type() == QEvent::MouseButtonPress &&
        !static_cast<OverlayScrollBar*>(m_overlay)->overThumb(static_cast<QMouseEvent*>(event)->position().toPoint())) {
      useItemScrolling();
      syncPage();
    }
  }
  if ((object == m_view || object == m_overlay) && event->type() == QEvent::KeyPress) {
    const auto key = static_cast<QKeyEvent*>(event)->key();
    if (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_PageUp || key == Qt::Key_PageDown || key == Qt::Key_Home || key == Qt::Key_End) {
      useItemScrolling();
      syncPage();
    }
  }
  if (object == m_view && event->type() == QEvent::Hide) {
    if (m_windowLayout) m_windowLayout->setPageVisible(*this, false);
    m_edgeHovered = m_overlayHovered = false;
    m_idle.stop();
    m_overlay->hide();
  }
  return QObject::eventFilter(object, event);
}
}  // namespace uwf::ui
#include "moc_PagedViewController.cpp"
