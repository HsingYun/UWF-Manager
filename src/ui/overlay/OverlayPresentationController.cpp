// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "OverlayPresentationController.h"

#include <QAction>
#include <QSignalBlocker>
#include <QTimer>

#include "OverlayHub.h"
#include "OverlayHubFactory.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/configuration/GlobalStatusPanel.h"
#include "ui/shell/TrayController.h"

namespace uwf::ui {

OverlayPresentationController::OverlayPresentationController(std::shared_ptr<presentation::OverlayViewModel> viewModel, TrayController& tray, QObject* parent)
    : QObject(parent), m_tray(tray), m_hub(createDefaultOverlayHub()), m_viewModel(std::move(viewModel)) {
  connect(m_hub.get(), &OverlayHub::showMainWindowRequested, this, &OverlayPresentationController::activateMainWindowRequested);
  connect(m_hub.get(), &OverlayHub::safeShutdownRequested, this, &OverlayPresentationController::safeShutdownRequested);
  connect(m_hub.get(), &OverlayHub::safeRestartRequested, this, &OverlayPresentationController::safeRestartRequested);
  connect(m_hub.get(), &OverlayHub::exitApplicationRequested, this, &OverlayPresentationController::exitApplicationRequested);
  connect(m_hub.get(), &OverlayHub::stateChanged, this, &OverlayPresentationController::syncAvailability);
  connect(&m_tray, &TrayController::refreshRequested, this, &OverlayPresentationController::refreshUsage);
  connect(&m_tray, &TrayController::toggleHubRequested, this, [this] { setHubEnabled(!hubEnabled()); });
  connect(&m_tray, &TrayController::safeShutdownRequested, this, &OverlayPresentationController::safeShutdownRequested);
  connect(&m_tray, &TrayController::safeRestartRequested, this, &OverlayPresentationController::safeRestartRequested);

  connect(m_viewModel.get(), &presentation::OverlayViewModel::usageChanged, this, &OverlayPresentationController::publishUsageState);
  connect(m_viewModel.get(), &presentation::OverlayViewModel::availabilityChanged, this, &OverlayPresentationController::syncAvailability);
  connect(&I18n::events(), &LanguageEvents::changed, this, [this] { m_hub->retranslate(); });
  syncAvailability();
}

OverlayPresentationController::~OverlayPresentationController() = default;

void OverlayPresentationController::bindUi(GlobalStatusPanel* global, QAction* displaysAction) {
  m_global = global;
  m_action = displaysAction;
  if (m_global) m_global->viewModel().applyUsageState(m_viewModel->usage());
  syncAvailability();
}

void OverlayPresentationController::unbindUi() {
  m_global = nullptr;
  m_action = nullptr;
}

void OverlayPresentationController::applySnapshot(const core::UwfSnapshot& snapshot) { m_viewModel->applySnapshot(snapshot); }

bool OverlayPresentationController::hubEnabled() const { return m_hub->enabled(); }

bool OverlayPresentationController::hubRequestedVisible() const { return m_hub->requestedVisible(); }

bool OverlayPresentationController::hubPresented() const { return m_hub->presented(); }

void OverlayPresentationController::setHubEnabled(const bool enabled) { m_hub->setRequestedVisible(enabled); }
void OverlayPresentationController::setOperationAvailability(const bool powerAllowed, const bool exitAllowed) {
  m_hub->setOperationAvailability(powerAllowed, exitAllowed);
  m_tray.setOperationAvailability(powerAllowed, exitAllowed);
}

void OverlayPresentationController::hideHubTemporarily() { m_hub->hideTemporarily(); }

void OverlayPresentationController::restoreHub() { m_hub->restoreAfterTemporaryHide(); }

void OverlayPresentationController::refreshActionIcon() {
  if (!m_action) return;
  constexpr Qt::Alignment kCenter = Qt::AlignHCenter | Qt::AlignVCenter;
  m_action->setIcon(ThemeManager::instance().icon(m_hub->enabled() ? ":/icons/overlay_float_on.svg" : ":/icons/overlay_float_off.svg", kCenter));
}

void OverlayPresentationController::refreshUsage() { m_viewModel->refreshUsage(); }

void OverlayPresentationController::publishUsageState(const app::OverlayUsageState& state) {
  if (m_global) m_global->viewModel().applyUsageState(state);
  m_hub->applyUsageState(state);
  m_tray.applyUsageState(state);
}

void OverlayPresentationController::syncAvailability() {
  m_tray.setHubState(m_hub->available(), m_hub->enabled());
  if (m_action) {
    m_action->setVisible(m_hub->available());
    if (m_action->isChecked() != m_hub->enabled()) {
      const QSignalBlocker blocker(m_action);
      m_action->setChecked(m_hub->enabled());
    }
  }
  refreshActionIcon();
}

}  // namespace uwf::ui

#include "moc_OverlayPresentationController.cpp"
