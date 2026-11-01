// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/desktop/MainWindowComposition.h"

#include <QCoreApplication>
#include <utility>

#include "application/diagnostics/ProcessLogSource.h"
#include "application/state/contracts/ApplicationStateSource.h"
#include "application/uwf/adapters/UwfApplicationStateSource.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "application/uwf/adapters/UwfOverlayReaders.h"
#include "composition/desktop/ViewModelComposition.h"
#include "composition/desktop/WorkflowComposition.h"
#include "composition/uwf/MaintenanceComposition.h"
#include "composition/uwf/SystemUwf.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "infrastructure/windows/compatibility/SystemCheck.h"
#include "infrastructure/windows/diagnostics/WindowsSystemInformationSource.h"
#include "infrastructure/windows/filesystem/browsing/LocalFileBrowser.h"
#include "infrastructure/windows/filesystem/text/AtomicTextFileWriter.h"
#include "infrastructure/windows/filesystem/text/BoundedTextFileReader.h"
#include "infrastructure/windows/filesystem/text/CreateOnlyTextFileWriter.h"
#include "infrastructure/windows/ipc/EnhancedModeAgent.h"
#include "infrastructure/windows/overlay/WindowsOverlayMaintenanceObservations.h"
#include "infrastructure/windows/registry/WindowsRegistryBrowser.h"
#include "infrastructure/windows/service/WindowsEnhancedModeServiceControl.h"
#include "infrastructure/windows/staging/RegistryFileStagingStore.h"
#include "infrastructure/windows/storage/WindowsVolumeRepository.h"
#include "presentation/views/files/FilePickerDialogFactory.h"
#include "presentation/views/shell/MainWindow.h"

namespace uwf::composition {

namespace {
const DesktopPlatformPorts& resolvedSystemDesktopPlatformPorts() {
  static const auto ports = [] {
    const auto files = windows::systemFileBrowser();
    auto result = DesktopPlatformPorts{windows::systemVolumeRepository(),
                                       files,
                                       windows::systemCommitTraversalSource(),
                                       std::shared_ptr<const app::RegistryBrowser>(&windows::systemRegistryBrowser(), [](const auto*) {}),
                                       std::make_shared<windows::WindowsSystemInformationSource>(),
                                       std::make_shared<app::ProcessLogSource>(),
                                       ui::createFilePickerDialogs(files),
                                       windows::readBoundedTextFile,
                                       windows::writeUtf8TextFile,
                                       windows::createUtf8TextFile,
                                       QCoreApplication::applicationDirPath()};
    result.maintenanceObservations = windows::windowsOverlayMaintenanceObservations(result.volumes);
    return result;
  }();
  return ports;
}

std::shared_ptr<app::OverlayFilesQueryManager> systemOverlayFileQueries() {
  static const auto queries = [] {
    auto result = std::make_shared<app::OverlayFilesQueryManager>(
        [system = systemUwf()](const QString& volumeGuid, std::stop_token stop) { return app::loadUwfOverlayFiles(*system, volumeGuid, stop); });
    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, result.get(), [queries = std::weak_ptr(result)] {
      if (const auto manager = queries.lock()) manager->shutdown();
    });
    return result;
  }();
  return queries;
}

}  // namespace

DesktopPlatformPorts systemDesktopPlatformPorts() { return resolvedSystemDesktopPlatformPorts(); }

ui::MainWindowServices mainWindowServices(std::shared_ptr<compat::Uwf> system, app::ApplicationStateSource& state,
                                          std::shared_ptr<app::FileStagingStore> staging, DesktopPlatformPorts ports,
                                          std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode, app::EnhancedModeAgentConnection* agent,
                                          EnhancedModeAgentFactory agentFactory, std::shared_ptr<app::OverlayFilesQueryManager> overlayFileQueries) {
  validateDesktopPlatformPorts(ports);
  return {createViewModelFactory(system, ports, {}, std::move(overlayFileQueries)),
          createWindowWorkflowFactory(app::uwfMaintenanceRepository(system), state, staging, enhancedMode, agent, std::move(agentFactory), ports),
          *ports.fileDialogs};
}
std::shared_ptr<app::DurableFileStagingStore> systemFileStagingStore(const app::UwfCapability capability) {
  static const auto store = std::make_shared<app::DurableFileStagingStore>(std::make_shared<windows::RegistryFileStagingStore>(),
                                                                           composition::systemMaintenanceRepository(), capability);
  return store;
}
ui::MainWindowServices systemMainWindowServices(const app::UwfCapability capability) {
  const auto& ports = systemDesktopPlatformPorts();
  static const auto state = app::uwfApplicationStateSource(systemUwf(), ports.volumes, windows::isElevated());
  static const auto enhancedMode = std::make_shared<windows::WindowsEnhancedModeServiceControl>();
  return mainWindowServices(composition::systemUwf(), *state, systemFileStagingStore(capability), ports, enhancedMode, nullptr,
                            windows::createEnhancedModeAgent, systemOverlayFileQueries());
}
}  // namespace uwf::composition
