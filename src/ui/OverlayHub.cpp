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
#include <algorithm>
#include <utility>

#include "OverlayHubView.h"

namespace uwf::ui {

OverlayHub::OverlayHub(QObject* parent) : QObject(parent) {}

OverlayHub::~OverlayHub() {
  for (const auto& entry : m_views) entry.view->setPresentationRequested(false);
}

void OverlayHub::registerView(std::unique_ptr<OverlayHubView> view) {
  if (!view) return;

  OverlayHubView* const rawView = view.get();
  m_views.push_back({std::move(view), m_newViewsEnabled});
  std::stable_sort(m_views.begin(), m_views.end(), [](const ViewEntry& lhs, const ViewEntry& rhs) { return lhs.view->priority() > rhs.view->priority(); });

  rawView->setAttribute(Qt::WA_QuitOnClose, false);
  connect(rawView, &OverlayHubView::showMainWindowRequested, this, &OverlayHub::showMainWindowRequested);
  connect(rawView, &OverlayHubView::hideViewRequested, this, [this, rawView]() { hideView(rawView); });
  connect(rawView, &OverlayHubView::exitApplicationRequested, this, &OverlayHub::exitApplicationRequested);
  connect(rawView, &OverlayHubView::displayStateChanged, this, &OverlayHub::reconcilePresentation);

  rawView->setFilterEnabled(m_filterEnabled);
  if (m_runtime)
    rawView->updateUsage(*m_runtime);
  else
    rawView->setUsageUnavailable();
  rawView->setPresentationRequested(false);
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::updateUsage(const core::OverlayRuntime& runtime) {
  const bool availabilityChanged = !m_runtime.has_value();
  m_runtime = runtime;
  for (const auto& entry : m_views) entry.view->updateUsage(runtime);
  reconcilePresentation();
  if (availabilityChanged) emit stateChanged();
}

void OverlayHub::setUsageUnavailable() {
  if (!m_runtime) return;
  m_runtime.reset();
  for (const auto& entry : m_views) entry.view->setUsageUnavailable();
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::setFilterEnabled(const bool enabled) {
  if (m_filterEnabled == enabled) return;
  m_filterEnabled = enabled;
  for (const auto& entry : m_views) entry.view->setFilterEnabled(enabled);
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::setRequestedVisible(const bool visible) {
  m_newViewsEnabled = visible;
  for (auto& entry : m_views) entry.enabled = visible;
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

bool OverlayHub::enabled() const {
  return available() && std::ranges::any_of(m_views, [](const ViewEntry& entry) { return entry.enabled; });
}

bool OverlayHub::presented() const {
  return available() && !m_temporarilyHidden && m_presentedView && m_presentedView->displayState() == OverlayHubView::DisplayState::Confirmed;
}

void OverlayHub::reconcilePresentation() {
  if (m_reconciling) return;
  const QScopedValueRollback guard(m_reconciling, true);

  if (!enabled() || m_temporarilyHidden) {
    for (const auto& entry : m_views) entry.view->setPresentationRequested(false);
    m_presentedView = nullptr;
    return;
  }

  OverlayHubView* stableView = m_presentedView;
  const auto stableEntry = std::ranges::find_if(m_views, [stableView](const ViewEntry& entry) { return entry.view.get() == stableView; });
  if (stableEntry == m_views.end() || !stableEntry->enabled || stableView->displayState() != OverlayHubView::DisplayState::Confirmed) {
    stableView = nullptr;
  }

  std::size_t frontier = m_views.size();
  OverlayHubView* confirmedView = nullptr;
  for (std::size_t i = 0; i < m_views.size(); ++i) {
    auto& entry = m_views[i];
    if (!entry.enabled) {
      entry.view->setPresentationRequested(false);
      continue;
    }

    entry.view->setPresentationRequested(true);
    if (entry.view->displayState() == OverlayHubView::DisplayState::Confirmed) {
      frontier = i;
      confirmedView = entry.view.get();
      break;
    }
    if (entry.view->displayState() == OverlayHubView::DisplayState::Attaching) {
      frontier = i;
      break;
    }
  }

  m_presentedView = confirmedView ? confirmedView : stableView;

  // 所有高于 frontier 的不可用端点继续保持请求，以便自行恢复；第一个正在
  // Attaching/Confirmed 的端点是当前决策边界。更低优先级端点只保留已经确认的
  // stable fallback，避免冷启动时逐个闪现，也避免高优先级恢复时产生空白。
  for (std::size_t i = 0; i < m_views.size(); ++i) {
    auto& entry = m_views[i];
    const bool withinFrontier = frontier == m_views.size() || i <= frontier;
    const bool keepStable = entry.view.get() == m_presentedView;
    entry.view->setPresentationRequested(entry.enabled && (withinFrontier || keepStable));
  }
}

void OverlayHub::hideView(OverlayHubView* view) {
  const auto entry = std::ranges::find_if(m_views, [view](const ViewEntry& candidate) { return candidate.view.get() == view; });
  if (entry == m_views.end() || !entry->enabled) return;
  entry->enabled = false;
  if (std::ranges::none_of(m_views, [](const ViewEntry& candidate) { return candidate.enabled; })) m_newViewsEnabled = false;
  reconcilePresentation();
  emit stateChanged();
}

}  // namespace uwf::ui
