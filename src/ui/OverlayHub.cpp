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

#include "OverlayFloatingWidget.h"
#include "OverlayTaskbarWidget.h"

namespace uwf::ui {

namespace {

// 调试切换点：
//   true  = 优先任务栏视图，浮窗作为注入失败时的备选（产品默认）；
//   false = 优先浮窗，任务栏视图作为浮窗展示失败时的备选。
constexpr bool kUseFloatingAsFallback = true;

}  // namespace

OverlayHub::OverlayHub(QObject* parent) : QObject(parent), m_floating(new OverlayFloatingWidget()), m_taskbar(new OverlayTaskbarWidget()) {
  m_floating->setAttribute(Qt::WA_QuitOnClose, false);
  connect(m_floating, &OverlayFloatingWidget::showMainWindowRequested, this, &OverlayHub::showMainWindowRequested);
  connect(m_floating, &OverlayFloatingWidget::hideFloatingWindowRequested, this, &OverlayHub::hideFloatingView);
  connect(m_floating, &OverlayFloatingWidget::exitApplicationRequested, this, &OverlayHub::exitApplicationRequested);
  connect(m_floating, &OverlayFloatingWidget::displayConfirmationChanged, this, [this](bool) { reconcilePresentation(); });

  connect(m_taskbar, &OverlayTaskbarWidget::showMainWindowRequested, this, &OverlayHub::showMainWindowRequested);
  connect(m_taskbar, &OverlayTaskbarWidget::hideTaskbarViewRequested, this, &OverlayHub::hideTaskbarView);
  connect(m_taskbar, &OverlayTaskbarWidget::exitApplicationRequested, this, &OverlayHub::exitApplicationRequested);
  connect(m_taskbar, &OverlayTaskbarWidget::displayStateChanged, this, &OverlayHub::reconcilePresentation);

  reconcilePresentation();
}

OverlayHub::~OverlayHub() {
  delete m_taskbar;
  delete m_floating;
}

void OverlayHub::updateUsage(const core::OverlayRuntime& runtime) {
  const bool availabilityChanged = m_unavailable;
  m_unavailable = false;
  m_floating->updateUsage(runtime);
  m_taskbar->updateUsage(runtime);
  reconcilePresentation();
  if (availabilityChanged) emit stateChanged();
}

void OverlayHub::setUnavailable() {
  if (m_unavailable) return;
  m_unavailable = true;
  m_floating->setUnavailable();
  m_taskbar->setUnavailable();
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::setFilterEnabled(const bool enabled) {
  if (m_filterEnabled == enabled) return;
  m_filterEnabled = enabled;
  m_floating->setFilterEnabled(enabled);
  m_taskbar->setFilterEnabled(enabled);
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::setRequestedVisible(const bool visible) {
  m_floatingEnabled = visible;
  m_taskbarEnabled = visible;
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

bool OverlayHub::available() const { return !m_unavailable && m_filterEnabled; }

bool OverlayHub::present() const { return available() && (m_floatingEnabled || m_taskbarEnabled); }

void OverlayHub::reconcilePresentation() {
  if (m_reconciling) return;
  const QScopedValueRollback guard(m_reconciling, true);

  if (!available() || m_temporarilyHidden || (!m_floatingEnabled && !m_taskbarEnabled)) {
    m_floating->hide();
    m_taskbar->setTaskbarVisible(false);
    return;
  }

  if constexpr (kUseFloatingAsFallback) {
    if (m_taskbarEnabled) {
      m_taskbar->setTaskbarVisible(true);
      switch (m_taskbar->displayState()) {
        case OverlayTaskbarWidget::DisplayState::Confirmed:
          m_floating->hide();
          break;
        case OverlayTaskbarWidget::DisplayState::Unavailable:
          m_floating->setVisible(m_floatingEnabled);
          break;
        case OverlayTaskbarWidget::DisplayState::Attaching:
          // Keep the current stable presentation while the new taskbar HWND
          // waits for its first confirmed paint.  This avoids flashing the
          // fallback on a normal toolbar enable and avoids a blank gap while
          // recovering from an Explorer restart.
          if (!m_floatingEnabled) m_floating->hide();
          break;
      }
    } else {
      m_taskbar->setTaskbarVisible(false);
      m_floating->setVisible(m_floatingEnabled);
    }
  } else {
    if (m_floatingEnabled) {
      m_floating->show();
      m_taskbar->setTaskbarVisible(m_taskbarEnabled && !m_floating->displayConfirmed());
    } else {
      m_floating->hide();
      m_taskbar->setTaskbarVisible(m_taskbarEnabled);
    }
  }
}

void OverlayHub::hideFloatingView() {
  if (!m_floatingEnabled) return;
  m_floatingEnabled = false;
  reconcilePresentation();
  emit stateChanged();
}

void OverlayHub::hideTaskbarView() {
  if (!m_taskbarEnabled) return;
  m_taskbarEnabled = false;
  reconcilePresentation();
  emit stateChanged();
}

}  // namespace uwf::ui
