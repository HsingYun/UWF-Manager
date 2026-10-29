// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PagedWindowLayout.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QEvent>
#include <QLayout>
#include <QScopedValueRollback>
#include <QTimer>
#include <QWidget>
#include <algorithm>

#include "PagedViewController.h"

namespace uwf::ui {
namespace {
void activateLayouts(QWidget* widget) {
  if (widget->layout()) widget->layout()->activate();
  const auto children = widget->findChildren<QWidget*>(QString{}, Qt::FindDirectChildrenOnly);
  for (auto* child : children)
    if (!child->isWindow() && child->isVisibleTo(widget)) activateLayouts(child);
}
}  // namespace

PagedWindowLayout::PagedWindowLayout(QWidget* window) : QObject(window), m_window(window), m_requested(window->size()), m_fitTimer(this) {
  m_fitTimer.setSingleShot(true);
  connect(&m_fitTimer, &QTimer::timeout, this, [this] {
    if (m_window->isVisible()) fit();
  });
  qApp->installEventFilter(this);
  scheduleFit();
}
PagedWindowLayout& PagedWindowLayout::forWindow(QWidget* window) {
  if (auto* existing = window->findChild<PagedWindowLayout*>(QString{}, Qt::FindDirectChildrenOnly)) return *existing;
  return *new PagedWindowLayout(window);
}
QSize PagedWindowLayout::requestedSize() const { return m_requested.isValid() ? m_requested : m_window->size(); }
void PagedWindowLayout::observeResize(const QSize size) {
  if (!m_fitting && !m_nativeSizing && size != m_fitted) m_requested = size;
}
void PagedWindowLayout::setPageVisible(PagedViewController& page, const bool visible) {
  std::erase_if(m_visiblePages, [&](const auto& item) { return !item || item == &page; });
  if (visible) m_visiblePages.emplace_back(&page);
}
const PagedViewController* PagedWindowLayout::activePage() const {
  const PagedViewController* result = nullptr;
  for (const auto& page : m_visiblePages) {
    if (!page) continue;
    // Independent visible viewports cannot share an arbitrary first page's
    // height constraint. Each controller constrains its own viewport instead.
    if (result) return nullptr;
    result = page;
  }
  return result;
}
bool PagedWindowLayout::hasPageHeightConstraint() const { return activePage() != nullptr; }
int PagedWindowLayout::alignedHeight(const int requestedHeight) const {
  const auto* page = activePage();
  if (!page || page->view()->viewport()->height() <= 0) return requestedHeight;
  const int row = page->rowHeight();
  const int chrome = m_window->height() - page->availableViewportHeight();
  const int minimumRows = std::max(1, (m_window->minimumHeight() - chrome + row - 1) / row);
  return chrome + std::max(minimumRows, (requestedHeight - chrome) / row) * row;
}
QSize PagedWindowLayout::constrainNativeResize(const QSize requested, const bool anchorBottom) {
  m_requested = requested;
  m_nativeSizing = true;
  m_anchorBottom = anchorBottom;
  m_fitted = {requested.width(), alignedHeight(requested.height())};
  return m_fitted;
}
void PagedWindowLayout::endNativeResize() {
  m_nativeSizing = false;
  m_anchorBottom = false;
}
void PagedWindowLayout::scheduleFit() {
  if (m_fitTimer.isActive() || m_fitting) return;
  m_fitTimer.start(0);
}
void PagedWindowLayout::fit(const bool keepCenter) {
  // An explicit fit is a barrier: a resize request queued before it must not
  // run again later against different interior content.
  m_fitTimer.stop();
  if (m_fitting || m_window->isMaximized() || m_window->isFullScreen()) return;
  observeResize(m_window->size());
  const QScopedValueRollback<bool> fitting(m_fitting, true);
  const auto center = m_window->frameGeometry().center();
  activateLayouts(m_window);
  const int height = alignedHeight(requestedSize().height());
  if (height != m_window->height()) {
    const int bottom = m_window->geometry().bottom();
    m_window->resize(m_window->width(), height);
    if (!keepCenter && m_anchorBottom) m_window->move(m_window->x(), bottom - height + 1);
    activateLayouts(m_window);
  }
  m_fitted = m_window->size();
  if (keepCenter) m_window->move(center - QPoint(m_window->frameGeometry().width() / 2, m_window->frameGeometry().height() / 2));
}
bool PagedWindowLayout::eventFilter(QObject* object, QEvent* event) {
  if (m_fitting || object != m_window) return false;
  if (event->type() != QEvent::Resize && event->type() != QEvent::Show) return false;
  // Only an outer-window resize changes the user's requested size. Interior
  // content changes (such as a selected record's wrapped details) reallocate
  // the viewport; its controller already constrains it to complete rows.
  if (event->type() == QEvent::Resize) observeResize(m_window->size());
  scheduleFit();
  return false;
}
}  // namespace uwf::ui
#include "moc_PagedWindowLayout.cpp"
