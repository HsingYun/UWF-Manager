// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <exception>
#include <functional>

#include "app/api/CommitPreparation.h"
#include "app/api/PowerInteraction.h"
#include "app/api/PreshutdownCommitControl.h"
#include "app/api/UwfCapability.h"
#include "app/ports/FileStagingStore.h"
#include "app/ports/WmiOperations.h"
#include "app/staging/FileStagingCoordinator.h"

namespace uwf::app {

struct PowerServices {
  wmi::WmiOperations& session;
  app::FileStagingStore& fileStaging;
  UwfCapability uwfCapability;
  FileStagingCoordinator& stagingCoordinator;
  std::shared_ptr<const CommitTraversalSource> traversal;
  PreshutdownCommitControl preshutdown{};
};

class PowerCoordinator : public QObject {
  Q_OBJECT
 public:
  PowerCoordinator(PowerServices services, PowerInteraction interaction, QObject* parent = nullptr);

 public slots:
  void safeShutdown();
  void safeRestart();
  void directRestart();

 private:
  void execute(PowerAction action);
  void executeDirectRestart();
  void executeReserved(PowerAction action, FileStagingCoordinator::ExternalBatch batch);
  void executeWithCompletedStaging(PowerAction action, FileStagingBatchResult stagingResult, FileStagingCoordinator::ExternalBatch batch);
  void invokePowerAction(PowerAction action);
  void reportPowerFailure(PowerAction action, const std::exception& error, PowerActionDialogMode mode);
  void reportUnknownPowerFailure(PowerAction action, PowerActionDialogMode mode);

  PowerInteraction m_interaction;
  wmi::WmiOperations& m_session;
  app::FileStagingStore& m_fileStaging;
  UwfCapability m_uwfCapability;
  FileStagingCoordinator& m_stagingCoordinator;
  std::shared_ptr<const CommitTraversalSource> m_traversal;
  PreshutdownCommitControl m_preshutdown;
  bool m_actionActive = false;
};

}  // namespace uwf::app
