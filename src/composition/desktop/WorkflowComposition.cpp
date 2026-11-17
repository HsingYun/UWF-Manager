// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/desktop/WorkflowComposition.h"

#include <stdexcept>
#include <utility>

#include "application/enhanced/EnhancedModeManager.h"
#include "presentation/viewmodels/common/ViewModelFactory.h"
#include "presentation/views/commit/CommitInteractionAdapter.h"
#include "presentation/views/power/PowerInteractionAdapter.h"

namespace uwf::composition {
namespace {
class ApplicationWindowWorkflowFactory final : public ui::WindowWorkflowFactory {
 public:
  ApplicationWindowWorkflowFactory(std::shared_ptr<app::UwfMaintenanceRepository> repository, app::ApplicationStateSource& state,
                                   std::shared_ptr<app::FileStagingStore> store, std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode,
                                   app::EnhancedModeAgentConnection* agent, EnhancedModeAgentFactory agentFactory, DesktopPlatformPorts ports)
      : m_repository(std::move(repository)),
        m_state(state),
        m_store(std::move(store)),
        m_enhancedMode(std::move(enhancedMode)),
        m_agent(agent),
        m_agentFactory(std::move(agentFactory)),
        m_ports(std::move(ports)) {
    if (!m_store) throw std::invalid_argument("a window requires a file staging store");
    validateDesktopPlatformPorts(m_ports);
  }
  std::unique_ptr<presentation::MainWindowViewModel> mainWindow(std::shared_ptr<presentation::ViewModelFactory> models, app::UwfCapability capability,
                                                                QWidget* parent) override {
    auto model = models->mainWindow(m_state, capability, std::make_shared<app::FileStagingDraft>(m_store, m_ports.traversal));
    std::unique_ptr<app::EnhancedModeSession> enhanced;
    if (m_enhancedMode) {
      auto actions = std::make_unique<app::EnhancedModeManager>(m_enhancedMode, m_repository, capability);
      if (m_agent)
        enhanced = std::make_unique<app::EnhancedModeSession>(std::move(actions), *m_agent);
      else {
        if (!m_agentFactory) throw std::invalid_argument("an enhanced mode agent or factory is required");
        enhanced = std::make_unique<app::EnhancedModeSession>(std::move(actions), m_agentFactory());
      }
    }
    auto power = createPowerCoordinator(
        {m_repository, *m_store, capability, m_ports.traversal, enhanced ? enhanced->preshutdownControl() : app::PreshutdownCommitControl{}}, parent);
    model->initializeWorkflows(std::move(models), std::move(enhanced), std::move(power));
    return model;
  }
  std::unique_ptr<ui::CommitDispatcher> commit(ui::CommitDispatcher::PauseObservation pauseObservation, ui::CommitDispatcher::ActivityChanged activityChanged,
                                                QWidget* parent) override {
    return createCommitDispatcher(m_repository, std::move(pauseObservation), m_ports.traversal, m_ports.writeText, m_ports.fileDialogs, parent,
                                  std::move(activityChanged));
  }

 private:
  std::shared_ptr<app::UwfMaintenanceRepository> m_repository;
  app::ApplicationStateSource& m_state;
  std::shared_ptr<app::FileStagingStore> m_store;
  std::shared_ptr<app::EnhancedModeServiceControl> m_enhancedMode;
  app::EnhancedModeAgentConnection* m_agent;
  EnhancedModeAgentFactory m_agentFactory;
  DesktopPlatformPorts m_ports;
};
}  // namespace
std::shared_ptr<ui::WindowWorkflowFactory> createWindowWorkflowFactory(std::shared_ptr<app::UwfMaintenanceRepository> repository,
                                                                       app::ApplicationStateSource& state, std::shared_ptr<app::FileStagingStore> store,
                                                                       std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode,
                                                                       app::EnhancedModeAgentConnection* agent, EnhancedModeAgentFactory agentFactory,
                                                                       DesktopPlatformPorts ports) {
  return std::make_shared<ApplicationWindowWorkflowFactory>(std::move(repository), state, std::move(store), std::move(enhancedMode), agent,
                                                            std::move(agentFactory), std::move(ports));
}
std::unique_ptr<app::PowerCoordinator> createPowerCoordinator(app::PowerServices services, QWidget* parentWindow, QObject* parent) {
  return std::make_unique<app::PowerCoordinator>(std::move(services), ui::powerInteraction(parentWindow), parent);
}
std::unique_ptr<ui::CommitDispatcher> createCommitDispatcher(std::shared_ptr<app::UwfMaintenanceRepository> repository,
                                                             ui::CommitDispatcher::PauseObservation pauseObservation,
                                                             std::shared_ptr<const app::CommitTraversalSource> traversal, app::TextFileWriter writeText,
                                                             std::shared_ptr<ui::dialogs::FileDialogProvider> fileDialogs, QWidget* parent,
                                                             ui::CommitDispatcher::ActivityChanged activityChanged) {
  if (!traversal || !writeText || !fileDialogs) throw std::invalid_argument("commit dispatch requires traversal, text writing, and file dialogs");
  auto interaction = ui::commitInteraction(parent, std::move(writeText), *fileDialogs);
  interaction.run = [run = std::move(interaction.run), files = std::move(fileDialogs)](const app::CommitRequest& request,
                                                                                       std::shared_ptr<app::CommitBackend> backend) {
    static_cast<void>(files);  // The UI adapter borrows this provider during run.
    run(request, std::move(backend));
  };
  return std::make_unique<ui::CommitDispatcher>(std::make_unique<app::CommitWorkflow>(repository, std::move(interaction), std::move(traversal)),
                                                std::move(pauseObservation), std::move(activityChanged));
}
}  // namespace uwf::composition
