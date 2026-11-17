// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <memory>

#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/enhanced/contracts/PreshutdownCommitControl.h"
#include "application/power/contracts/PowerInteraction.h"
#include "application/power/contracts/PowerTypes.h"
#include "application/staging/contracts/FileStagingStore.h"
#include "application/uwf/contracts/UwfCapability.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"

namespace uwf::app {

struct PowerServices {
  std::shared_ptr<UwfMaintenanceRepository> repository;
  app::FileStagingStore& fileStaging;
  UwfCapability uwfCapability;
  std::shared_ptr<const CommitTraversalSource> traversal;
  PreshutdownCommitControl preshutdown{};
};

class PowerCoordinator : public QObject {
  Q_OBJECT
 public:
  PowerCoordinator(PowerServices services, PowerInteraction interaction, QObject* parent = nullptr);

  [[nodiscard]] bool active() const { return m_actionActive; }
  void setSuspended(bool suspended) { m_suspended = suspended; }

 public slots:
  void safeShutdown();
  void safeRestart();
  void directRestart();

 signals:
  void activeChanged(bool active);

 private:
  class Operation;
  void execute(PowerAction action, PowerActionDialogMode mode);

  PowerServices m_services;
  PowerInteraction m_interaction;
  bool m_actionActive = false;
  bool m_suspended = false;
};

}  // namespace uwf::app
