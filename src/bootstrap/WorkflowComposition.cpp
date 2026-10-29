// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WorkflowComposition.h"

#include <stdexcept>
#include <utility>

#include "FileDialogComposition.h"
#include "app/enhanced/EnhancedModeManager.h"
#include "presentation/staging/FileStagingProgressViewModel.h"
#include "ui/commit/CommitInteractionAdapter.h"
#include "ui/power/PowerInteractionAdapter.h"
#include "ui/staging/FileStagingProgressDialog.h"
#include "windows/commit/WindowsCommitTraversalSource.h"
#include "windows/filesystem/AtomicTextFileWriter.h"
namespace uwf::bootstrap {
namespace {
class ApplicationWindowWorkflowFactory final : public ui::WindowWorkflowFactory {
 public:
  ApplicationWindowWorkflowFactory(app::wmi::WmiOperations& session, app::FileStagingStore& store,
                                   std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode, app::EnhancedModeAgentConnection* agent,
                                   EnhancedModeAgentFactory agentFactory)
      : m_session(session), m_store(store), m_enhancedMode(enhancedMode), m_agent(agent), m_agentFactory(std::move(agentFactory)) {}
  std::unique_ptr<app::EnhancedModeSession> enhancedMode(app::UwfCapability capability) override {
    if (!m_enhancedMode) return {};
    auto actions = std::make_unique<app::EnhancedModeManager>(m_enhancedMode, m_session, capability);
    if (m_agent) return std::make_unique<app::EnhancedModeSession>(std::move(actions), *m_agent);
    if (!m_agentFactory) throw std::invalid_argument("an enhanced mode agent or factory is required");
    return std::make_unique<app::EnhancedModeSession>(std::move(actions), m_agentFactory());
  }
  std::unique_ptr<app::FileStagingCoordinator> fileStaging(app::UwfCapability capability, QWidget* parent) override {
    return createFileStagingCoordinator(m_session, m_store, capability, parent);
  }
  std::unique_ptr<app::PowerCoordinator> power(app::UwfCapability capability, app::FileStagingCoordinator& staging, app::PreshutdownCommitControl preshutdown,
                                               QWidget* parent) override {
    return createPowerCoordinator({m_session, m_store, capability, staging, uwf::windows::systemCommitTraversalSource(), std::move(preshutdown)}, parent);
  }
  std::unique_ptr<ui::CommitDispatcher> commit(const core::UwfSnapshot& snapshot, QTimer* usageTimer, QWidget* parent) override {
    return createCommitDispatcher(m_session, snapshot, usageTimer, parent);
  }

 private:
  app::wmi::WmiOperations& m_session;
  app::FileStagingStore& m_store;
  std::shared_ptr<app::EnhancedModeServiceControl> m_enhancedMode;
  app::EnhancedModeAgentConnection* m_agent;
  EnhancedModeAgentFactory m_agentFactory;
};
}  // namespace
std::shared_ptr<ui::WindowWorkflowFactory> createWindowWorkflowFactory(app::wmi::WmiOperations& session, app::FileStagingStore& store,
                                                                       std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode,
                                                                       app::EnhancedModeAgentConnection* agent, EnhancedModeAgentFactory agentFactory) {
  return std::make_shared<ApplicationWindowWorkflowFactory>(session, store, enhancedMode, agent, std::move(agentFactory));
}
std::unique_ptr<app::FileStagingCoordinator> createFileStagingCoordinator(app::wmi::WmiOperations& session, app::FileStagingStore& store,
                                                                          app::UwfCapability capability, QWidget* parentWindow, QObject* parent) {
  return std::make_unique<app::FileStagingCoordinator>(
      [&session, &store, capability, parentWindow](app::FileStagingCoordinator::Completion completion, app::FileStagingCoordinator::Progress progress) {
        auto model = std::make_unique<presentation::FileStagingProgressViewModel>(
            std::make_unique<app::FileStagingTask>(session, store, capability, uwf::windows::systemCommitTraversalSource()), std::move(completion),
            std::move(progress));
        ui::attachFileStagingProgressDialog(*model, parentWindow);
        return model;
      },
      parent);
}
std::unique_ptr<app::PowerCoordinator> createPowerCoordinator(app::PowerServices services, QWidget* parentWindow, QObject* parent) {
  return std::make_unique<app::PowerCoordinator>(std::move(services), ui::powerInteraction(parentWindow), parent);
}
std::unique_ptr<ui::CommitDispatcher> createCommitDispatcher(app::wmi::WmiOperations& session, const core::UwfSnapshot& snapshot, QTimer* usageTimer,
                                                             QWidget* parent) {
  auto interaction = ui::commitInteraction(parent, windows::writeUtf8TextFile, systemFileDialogs());
  return std::make_unique<ui::CommitDispatcher>(
      std::make_unique<app::CommitWorkflow>(session, snapshot, std::move(interaction), windows::systemCommitTraversalSource()), usageTimer);
}
}  // namespace uwf::bootstrap
