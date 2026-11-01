// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QPointer>
#include <memory>

#include "application/overlay/contracts/OverlayUsage.h"
#include "presentation/viewmodels/overlay/OverlayViewModel.h"

class QAction;

namespace uwf::ui {

class OverlayHub;
class TrayController;

// Binds the usage model to HUD selection, tray presentation and toolbar visibility.
class OverlayPresentationController : public QObject {
  Q_OBJECT
 public:
  OverlayPresentationController(std::shared_ptr<presentation::OverlayViewModel> viewModel, TrayController& tray, QObject* parent = nullptr);
  ~OverlayPresentationController() override;

  void bindAction(QAction* displaysAction);

  [[nodiscard]] presentation::ObservationPause pauseObservation() { return m_viewModel->pauseObservation(); }
  [[nodiscard]] bool hubEnabled() const;
  [[nodiscard]] bool hubRequestedVisible() const;
  [[nodiscard]] bool hubPresented() const;

  void setHubEnabled(bool enabled);
  void setOperationAvailability(bool powerAllowed, bool exitAllowed);
  void hideHubTemporarily();
  void restoreHub();
  void refreshActionIcon();

 signals:
  void activateMainWindowRequested();
  void safeShutdownRequested();
  void safeRestartRequested();
  void exitApplicationRequested();

 private:
  void refreshUsage();
  void publishUsageState(const app::OverlayUsageState& state);
  void syncAvailability();

  TrayController& m_tray;
  std::unique_ptr<OverlayHub> m_hub;
  std::shared_ptr<presentation::OverlayViewModel> m_viewModel;
  QPointer<QAction> m_action;
};

}  // namespace uwf::ui
