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
#include "OverlayHub.h"

#include <QScopedValueRollback>
#include <QTimer>
#include <algorithm>
#include <utility>

#include "OverlayHubView.h"

namespace uwf::ui {

OverlayHub::OverlayHub(QObject* parent) : QObject(parent) {}

OverlayHub::~OverlayHub() {
  m_shuttingDown = true;
  m_reconciling = true;
  m_pendingViews.clear();
  for (const auto& view : m_views) {
    QObject::disconnect(view.get(), nullptr, this, nullptr);
    view->setPresentationRequested(false);
  }
  m_presentedView = nullptr;
}

void OverlayHub::registerView(std::unique_ptr<OverlayHubView> view) {
  if (!view || m_shuttingDown) return;
  if (m_reconciling) {
    m_pendingViews.push_back(std::move(view));
    schedulePendingViewFlush();
    return;
  }

  installView(std::move(view));
  sortViewsByPriority();
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::installView(std::unique_ptr<OverlayHubView> view) {
  OverlayHubView* const rawView = view.get();
  m_views.push_back(std::move(view));

  rawView->setAttribute(Qt::WA_QuitOnClose, false);
  rawView->setFilterEnabled(m_filterEnabled);
  if (m_runtime)
    rawView->updateUsage(*m_runtime);
  else
    rawView->setUsageUnavailable();
  rawView->setPresentationRequested(false);

  connect(rawView, &OverlayHubView::showMainWindowRequested, this, &OverlayHub::showMainWindowRequested);
  connect(rawView, &OverlayHubView::hideHubRequested, this, [this]() { setRequestedVisible(false); });
  connect(rawView, &OverlayHubView::exitApplicationRequested, this, &OverlayHub::exitApplicationRequested);
  connect(rawView, &OverlayHubView::displayStateChanged, this, &OverlayHub::reconcilePresentation);
}

void OverlayHub::sortViewsByPriority() {
  std::stable_sort(m_views.begin(), m_views.end(),
                   [](const std::unique_ptr<OverlayHubView>& lhs, const std::unique_ptr<OverlayHubView>& rhs) { return lhs->priority() > rhs->priority(); });
}

void OverlayHub::schedulePendingViewFlush() {
  if (m_pendingViewFlushScheduled || m_shuttingDown) return;
  m_pendingViewFlushScheduled = true;
  QTimer::singleShot(0, this, &OverlayHub::flushPendingViews);
}

void OverlayHub::flushPendingViews() {
  m_pendingViewFlushScheduled = false;
  if (m_shuttingDown) {
    m_pendingViews.clear();
    return;
  }
  if (m_reconciling) {
    schedulePendingViewFlush();
    return;
  }
  if (m_pendingViews.empty()) return;

  auto pendingViews = std::move(m_pendingViews);
  m_pendingViews.clear();
  m_views.reserve(m_views.size() + pendingViews.size());
  for (auto& pendingView : pendingViews) installView(std::move(pendingView));
  sortViewsByPriority();
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::updateUsage(const core::OverlayRuntime& runtime) {
  const bool availabilityChanged = !m_runtime.has_value();
  m_runtime = runtime;
  for (const auto& view : m_views) view->updateUsage(runtime);
  reconcilePresentation();
  if (availabilityChanged) emit stateChanged();
}

void OverlayHub::setUsageUnavailable() {
  if (!m_runtime) return;
  m_runtime.reset();
  for (const auto& view : m_views) view->setUsageUnavailable();
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::setFilterEnabled(const bool enabled) {
  if (m_filterEnabled == enabled) return;
  m_filterEnabled = enabled;
  for (const auto& view : m_views) view->setFilterEnabled(enabled);
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::setRequestedVisible(const bool visible) {
  m_requestedVisible = visible;
  m_temporarilyHidden = false;
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::hideTemporarily() {
  m_temporarilyHidden = true;
  reconcilePresentation();
}

void OverlayHub::restoreAfterTemporaryHide() {
  m_temporarilyHidden = false;
  reconcilePresentation();
}

bool OverlayHub::available() const { return m_runtime.has_value() && m_filterEnabled; }

bool OverlayHub::enabled() const { return available() && m_requestedVisible && !m_views.empty(); }

bool OverlayHub::presented() const { return available() && !m_temporarilyHidden && m_presentedView && m_presentedView->presentationVerified(); }

void OverlayHub::reconcilePresentation() {
  if (m_reconciling) return;
  const QScopedValueRollback guard(m_reconciling, true);

  if (!enabled() || m_temporarilyHidden) {
    for (const auto& view : m_views) view->setPresentationRequested(false);
    m_presentedView = nullptr;
    return;
  }

  OverlayHubView* stableView = m_presentedView;
  const auto stableEntry = std::ranges::find_if(m_views, [stableView](const std::unique_ptr<OverlayHubView>& view) { return view.get() == stableView; });
  if (stableEntry == m_views.end() || !stableView->presentationVerified()) {
    stableView = nullptr;
  }

  std::size_t frontier = m_views.size();
  OverlayHubView* confirmedView = nullptr;
  for (std::size_t i = 0; i < m_views.size(); ++i) {
    auto& view = m_views[i];
    view->setPresentationRequested(true);
    if (view->presentationVerified()) {
      frontier = i;
      confirmedView = view.get();
      break;
    }
    if (view->displayState() == OverlayHubView::DisplayState::Attaching) {
      frontier = i;
      break;
    }
  }

  m_presentedView = confirmedView ? confirmedView : stableView;

  // 所有高于 frontier 的不可用端点继续保持请求，以便自行恢复；第一个正在
  // Attaching/Confirmed 的端点是当前决策边界。更低优先级端点只保留已经确认的
  // stable fallback，避免冷启动时逐个闪现，也避免高优先级恢复时产生空白。
  for (std::size_t i = 0; i < m_views.size(); ++i) {
    auto& view = m_views[i];
    const bool withinFrontier = frontier == m_views.size() || i <= frontier;
    const bool keepStable = view.get() == m_presentedView;
    view->setPresentationRequested(withinFrontier || keepStable);
  }
}

}  // namespace uwf::ui
