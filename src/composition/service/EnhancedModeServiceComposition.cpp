// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/service/EnhancedModeServiceComposition.h"

#include <QScopeGuard>
#include <chrono>
#include <exception>
#include <thread>

#include "application/staging/execution/FileStagingTask.h"
#include "composition/uwf/MaintenanceComposition.h"
#include "composition/uwf/SystemCapability.h"
#include "foundation/logging/Log.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "infrastructure/windows/management/wmi/WmiSession.h"
#include "infrastructure/windows/service/EnhancedModeServiceRuntime.h"
#include "infrastructure/windows/staging/RegistryFileStagingStore.h"

namespace uwf::composition {
namespace {
constexpr auto kCommitPreparationPollInterval = std::chrono::milliseconds{16};

app::FileStagingCommitResult commitStagedFiles() {
  bool taskFailure = false;
  try {
    // Release any lazily initialized WMI context when the task ends.
    const auto shutdownWmi = qScopeGuard([] { windows::shutdownWmiRuntime(); });
    const auto capability = composition::probeUwfCapability();
    windows::RegistryFileStagingStore store;
    app::FileStagingTask task(composition::systemMaintenanceRepository(), store.load(), capability, uwf::windows::systemCommitTraversalSource());
    try {
      while (!task.pollPreparation()) std::this_thread::sleep_for(kCommitPreparationPollInterval);
      while (!task.finished()) {
        static_cast<void>(task.advance());
        task.waitForWorker();
      }
      return task.result();
    } catch (...) {
      taskFailure = task.failed();
      throw;
    }
  } catch (const std::exception& error) {
    if (!taskFailure) UWF_LOG_E("service") << "enhanced mode staged commit startup failed: error=" << error.what();
    throw;
  } catch (...) {
    if (!taskFailure) UWF_LOG_E("service") << "enhanced mode staged commit startup failed: error=non-standard-exception";
    throw;
  }
}

}  // namespace

int runEnhancedModeService() { return windows::runEnhancedModeService(commitStagedFiles); }

}  // namespace uwf::composition
