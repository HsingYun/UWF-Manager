// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ViewModelComposition.h"

#include <QCoreApplication>

#include "app/configuration/ConfigurationExecutor.h"
#include "app/diagnostics/ProcessLogSource.h"
#include "app/overlay/UwfOverlayUsageReader.h"
#include "app/ports/VolumePaths.h"
#include "app/staging/FileStagingEditor.h"
#include "windows/commit/WindowsCommitTraversalSource.h"
#include "windows/filesystem/AtomicTextFileWriter.h"
#include "windows/filesystem/BoundedTextFileReader.h"
#include "windows/filesystem/CreateOnlyTextFileWriter.h"
#include "windows/wmi/WmiSession.h"
namespace uwf::bootstrap {
namespace {
class ApplicationViewModelFactory final : public presentation::ViewModelFactory {
 public:
  ApplicationViewModelFactory(app::wmi::WmiOperations& session, const app::RegistryBrowser& registry, app::OverlayFileLoader loader,
                              const app::SystemInformationSource& systemInformation)
      : m_session(session), m_registry(registry), m_overlayFiles(std::move(loader)), m_systemInformation(systemInformation) {}
  std::unique_ptr<presentation::MainWindowViewModel> mainWindow(app::ApplicationStateSource& source, app::UwfCapability capability,
                                                                std::shared_ptr<app::FileStagingDraft> staging) override {
    return std::make_unique<presentation::MainWindowViewModel>(source, capability, std::move(staging), drive::fromPath);
  }
  std::shared_ptr<presentation::SystemInformationViewModel> systemInformation() override {
    return std::make_shared<presentation::SystemInformationViewModel>(m_systemInformation);
  }
  std::shared_ptr<presentation::ImportViewModel> importCommands() override {
    return std::make_shared<presentation::ImportViewModel>(windows::readBoundedTextFile, QString::fromStdString(drive::systemLetter()));
  }
  std::shared_ptr<presentation::LogViewModel> log() override { return std::make_shared<presentation::LogViewModel>(std::make_shared<app::ProcessLogSource>()); }
  std::shared_ptr<presentation::UserConfigExportViewModel> userConfigExport() override {
    return std::make_shared<presentation::UserConfigExportViewModel>(QCoreApplication::applicationDirPath(), windows::createUtf8TextFile);
  }
  std::shared_ptr<presentation::FileStagingViewModel> fileStaging(QString drive, app::FileStagingDraft& store,
                                                                  app::FileStagingConflictPolicy* conflicts) override {
    return std::make_shared<presentation::FileStagingViewModel>(
        std::make_shared<app::FileStagingEditor>(std::move(drive), store, uwf::windows::systemCommitTraversalSource(), conflicts));
  }
  std::shared_ptr<presentation::ApplyPlanViewModel> applyPlan(core::PendingChanges changes, const core::UwfSnapshot& snapshot,
                                                              std::shared_ptr<app::FileStagingDraft> staging = {}) override {
    const auto stagingChanges = staging ? staging->changes() : std::nullopt;
    return std::make_shared<presentation::ApplyPlanViewModel>(
        std::move(changes), snapshot, std::make_shared<app::ConfigurationExecutor>(m_session, drive::mountedLetters, std::move(staging), stagingChanges),
        stagingChanges, windows::writeUtf8TextFile);
  }
  std::shared_ptr<presentation::EnhancedModeViewModel> enhancedMode(app::EnhancedModeActions& actions) override {
    return std::make_shared<presentation::EnhancedModeViewModel>(actions);
  }
  std::shared_ptr<presentation::OverlayViewModel> overlay() override {
    return std::make_shared<presentation::OverlayViewModel>(std::make_shared<app::UwfOverlayUsageReader>(m_session));
  }
  std::shared_ptr<presentation::OverlayFilesViewModel> overlayFiles(const QString& drive) override {
    return std::make_shared<presentation::OverlayFilesViewModel>(drive, m_overlayFiles, windows::writeUtf8TextFile);
  }
  std::shared_ptr<presentation::RegistryPickerViewModel> registryPicker(presentation::RegistryPickerViewModel::Mode mode) override {
    return std::make_shared<presentation::RegistryPickerViewModel>(mode, m_registry);
  }

 private:
  app::wmi::WmiOperations& m_session;
  const app::RegistryBrowser& m_registry;
  app::OverlayFileLoader m_overlayFiles;
  const app::SystemInformationSource& m_systemInformation;
};
}  // namespace
std::shared_ptr<presentation::ViewModelFactory> createViewModelFactory(app::wmi::WmiOperations& session, const app::RegistryBrowser& registry,
                                                                       app::OverlayFileLoader loader, const app::SystemInformationSource& systemInformation) {
  return std::make_shared<ApplicationViewModelFactory>(session, registry, std::move(loader), systemInformation);
}
std::shared_ptr<presentation::ViewModelFactory> createViewModelFactory() { return createViewModelFactory(windows::embeddedWmiSession()); }
}  // namespace uwf::bootstrap
