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
#include "OverlayHub.h"
#include "ThemeManager.h"
#include "TrayController.h"

namespace uwf::ui {

OverlayPresentationController::OverlayPresentationController(WmiSession& session, QMainWindow* ownerWindow, TrayController* tray, QObject* parent)
    : QObject(parent),
      m_ownerWindow(ownerWindow),
      m_tray(tray),
      m_hub(new OverlayHub()),
      m_usageTimer(new QTimer(this)),
      m_filter(session),
      m_overlay(session) {
  connect(m_hub, &OverlayHub::showMainWindowRequested, this, &OverlayPresentationController::activateMainWindowRequested);
  connect(m_hub, &OverlayHub::exitApplicationRequested, this, &OverlayPresentationController::exitApplicationRequested);
  connect(m_hub, &OverlayHub::stateChanged, this, &OverlayPresentationController::syncAvailability);

  m_usageTimer->setInterval(5000);
  connect(m_usageTimer, &QTimer::timeout, this, &OverlayPresentationController::refreshUsage);
  m_usageTimer->start();
  syncAvailability();
}

OverlayPresentationController::~OverlayPresentationController() { delete m_hub; }

void OverlayPresentationController::bindUi(GlobalStatusPanel* global, QAction* displaysAction) {
  m_global = global;
  m_action = displaysAction;
  syncAvailability();
}

void OverlayPresentationController::unbindUi() {
  m_global = nullptr;
  m_action = nullptr;
}

void OverlayPresentationController::applySnapshot(const core::UwfSnapshot& snapshot) {
  if (snapshot.uwfAvailable) {
    m_hub->setFilterEnabled(snapshot.current.filter.enabled);
    m_hub->updateUsage(snapshot.runtime);
    m_usageTimer->start();
  } else {
    m_hub->setUnavailable();
    m_usageTimer->stop();
  }
  syncAvailability();
}

bool OverlayPresentationController::hubPresent() const { return m_hub->present(); }

void OverlayPresentationController::setHubVisible(const bool visible) { m_hub->setRequestedVisible(visible); }

void OverlayPresentationController::hideHubTemporarily() { m_hub->hideTemporarily(); }

void OverlayPresentationController::restoreHub() { m_hub->restoreAfterTemporaryHide(); }

void OverlayPresentationController::refreshActionIcon() {
  if (!m_action) return;
  constexpr Qt::Alignment kCenter = Qt::AlignHCenter | Qt::AlignVCenter;
  m_action->setIcon(ThemeManager::instance().icon(m_hub->present() ? ":/icons/overlay_float_on.svg" : ":/icons/overlay_float_off.svg", kCenter));
}

void OverlayPresentationController::refreshUsage() {
  const auto filter = m_filter.read();
  if (!filter)
    m_hub->setUnavailable();
  else
    m_hub->setFilterEnabled(filter->currentEnabled);

  if (filter && filter->currentEnabled) {
    const auto overlay = m_overlay.read();
    if (overlay) {
      core::OverlayRuntime runtime;
      runtime.currentConsumptionMb = overlay->overlayConsumption;
      runtime.availableSpaceMb = overlay->availableSpace;
      if (m_ownerWindow->isVisible() && m_global) m_global->updateUsage(runtime);
      m_hub->updateUsage(runtime);
    } else {
      m_hub->setUnavailable();
    }
  }
  syncAvailability();
  if (m_tray) m_tray->refreshUsage();
}

void OverlayPresentationController::syncAvailability() {
  if (m_action) {
    m_action->setVisible(m_hub->available());
    if (m_action->isChecked() != m_hub->present()) {
      const QSignalBlocker blocker(m_action);
      m_action->setChecked(m_hub->present());
    }
  }
  refreshActionIcon();
}

}  // namespace uwf::ui
