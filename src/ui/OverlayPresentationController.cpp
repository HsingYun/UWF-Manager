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
#include "OverlayPresentationController.h"

#include <QAction>
#include <QMainWindow>
#include <QSignalBlocker>
#include <QTimer>

#include "GlobalStatusPanel.h"
#include "OverlayFloatingWidget.h"
#include "ThemeManager.h"
#include "TrayController.h"

namespace uwf::ui {

OverlayPresentationController::OverlayPresentationController(WmiSession& session, QMainWindow* ownerWindow, TrayController* tray, QObject* parent)
    : QObject(parent),
      m_ownerWindow(ownerWindow),
      m_tray(tray),
      m_floating(new OverlayFloatingWidget()),
      m_usageTimer(new QTimer(this)),
      m_filter(session),
      m_overlay(session) {
  m_floating->setAttribute(Qt::WA_QuitOnClose, false);
  connect(m_floating, &OverlayFloatingWidget::showMainWindowRequested, this, &OverlayPresentationController::activateMainWindowRequested);
  connect(m_floating, &OverlayFloatingWidget::closeFloatingWindowRequested, this, [this]() { setFloatingVisible(false); });
  connect(m_floating, &OverlayFloatingWidget::exitApplicationRequested, this, &OverlayPresentationController::exitApplicationRequested);

  m_usageTimer->setInterval(5000);
  connect(m_usageTimer, &QTimer::timeout, this, &OverlayPresentationController::refreshUsage);
  m_usageTimer->start();
  syncAvailability();
}

OverlayPresentationController::~OverlayPresentationController() { delete m_floating; }

void OverlayPresentationController::bindUi(GlobalStatusPanel* global, QAction* floatingAction) {
  m_global = global;
  m_action = floatingAction;
  syncAvailability();
}

void OverlayPresentationController::unbindUi() {
  m_global = nullptr;
  m_action = nullptr;
}

void OverlayPresentationController::applySnapshot(const core::UwfSnapshot& snapshot) {
  m_floatingAllowed = snapshot.uwfAvailable && snapshot.current.filter.enabled;
  if (snapshot.uwfAvailable) {
    m_floating->setFilterEnabled(snapshot.current.filter.enabled);
    m_floating->updateUsage(snapshot.runtime);
    m_usageTimer->start();
  } else {
    m_floating->setUnavailable();
    m_usageTimer->stop();
  }
  syncAvailability();
}

bool OverlayPresentationController::floatingVisible() const { return m_floating->isVisible(); }

void OverlayPresentationController::setFloatingVisible(const bool visible) {
  m_floatingRequested = visible;
  const bool nextVisible = m_floatingRequested && m_floatingAllowed;
  m_floating->setVisible(nextVisible);
  if (m_action && m_action->isChecked() != nextVisible) {
    const QSignalBlocker blocker(m_action);
    m_action->setChecked(nextVisible);
  }
  refreshActionIcon();
}

void OverlayPresentationController::hideFloatingTemporarily() { m_floating->hide(); }

void OverlayPresentationController::refreshActionIcon() {
  if (!m_action) return;
  constexpr Qt::Alignment kCenter = Qt::AlignHCenter | Qt::AlignVCenter;
  m_action->setIcon(ThemeManager::instance().icon(m_floating->isVisible() ? ":/icons/overlay_float_on.svg" : ":/icons/overlay_float_off.svg", kCenter));
}

void OverlayPresentationController::refreshUsage() {
  const auto filter = m_filter.read();
  m_floatingAllowed = filter && filter->currentEnabled;
  if (!filter)
    m_floating->setUnavailable();
  else
    m_floating->setFilterEnabled(filter->currentEnabled);

  if (filter && filter->currentEnabled) {
    const auto overlay = m_overlay.read();
    if (overlay) {
      core::OverlayRuntime runtime;
      runtime.currentConsumptionMb = overlay->overlayConsumption;
      runtime.availableSpaceMb = overlay->availableSpace;
      if (m_ownerWindow->isVisible() && m_global) m_global->updateUsage(runtime);
      m_floating->updateUsage(runtime);
    } else {
      m_floating->setUnavailable();
    }
  }
  syncAvailability();
  if (m_tray) m_tray->refreshUsage();
}

void OverlayPresentationController::syncAvailability() {
  const bool visible = m_floatingAllowed && m_floatingRequested;
  m_floating->setVisible(visible);
  if (m_action) {
    m_action->setVisible(m_floatingAllowed);
    if (m_action->isChecked() != visible) {
      const QSignalBlocker blocker(m_action);
      m_action->setChecked(visible);
    }
  }
  refreshActionIcon();
}

}  // namespace uwf::ui
