// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "MainWindowComposition.h"

#include <utility>

#include "FileDialogComposition.h"
#include "ViewModelComposition.h"
#include "WorkflowComposition.h"
#include "app/state/ApplicationState.h"
#include "ui/shell/MainWindow.h"
#include "windows/ipc/EnhancedModeAgent.h"
#include "windows/service/WindowsEnhancedModeServiceControl.h"
#include "windows/storage/RegistryFileStagingStore.h"
#include "windows/uwf/SystemUwfStateReader.h"
#include "windows/wmi/WmiSession.h"

namespace uwf::bootstrap {

using app::ApplicationState;
using app::ApplicationStateSource;

namespace {
class SystemApplicationStateSource final : public ApplicationStateSource {
 public:
  ReadOperation readOperation(const app::UwfCapability capability) override {
    return [capability](std::stop_token) { return ApplicationState{windows::enumerateVolumes(), windows::readSnapshot(capability)}; };
  }
};

ApplicationStateSource& systemApplicationStateSource() {
  static SystemApplicationStateSource source;
  return source;
}

std::shared_ptr<app::EnhancedModeServiceControl> systemEnhancedModeServiceControl() {
  static const auto control = std::make_shared<windows::WindowsEnhancedModeServiceControl>();
  return control;
}

}  // namespace

ui::MainWindowServices mainWindowServices(app::wmi::WmiOperations& session, app::ApplicationStateSource& state, std::shared_ptr<app::FileStagingStore> staging,
                                          std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode, app::EnhancedModeAgentConnection* agent,
                                          EnhancedModeAgentFactory agentFactory) {
  if (!agentFactory) agentFactory = windows::createEnhancedModeAgent;
  return {state, staging, createViewModelFactory(session), createWindowWorkflowFactory(session, *staging, enhancedMode, agent, std::move(agentFactory)),
          systemFileDialogs()};
}
std::shared_ptr<app::DurableFileStagingStore> systemFileStagingStore(const app::UwfCapability capability) {
  static const auto store =
      std::make_shared<app::DurableFileStagingStore>(std::make_shared<windows::RegistryFileStagingStore>(), windows::embeddedWmiSession(), capability);
  return store;
}
ui::MainWindowServices systemMainWindowServices(const app::UwfCapability capability) {
  return mainWindowServices(windows::embeddedWmiSession(), systemApplicationStateSource(), systemFileStagingStore(capability),
                            systemEnhancedModeServiceControl());
}
}  // namespace uwf::bootstrap
