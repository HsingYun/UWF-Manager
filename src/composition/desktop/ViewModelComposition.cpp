// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/desktop/ViewModelComposition.h"

#include <QPointer>
#include <stdexcept>

#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/overlay/contracts/OverlayFileLoader.h"
#include "application/staging/editing/FileStagingEditor.h"
#include "application/state/contracts/ApplicationStateSource.h"
#include "application/uwf/CompositeAllVolumeProtection.h"
#include "application/uwf/adapters/UwfOverlayMaintenance.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfOverlayReaders.h"
#include "composition/uwf/ConfigurationComposition.h"
#include "presentation/appearance/I18n.h"
#include "presentation/viewmodels/common/PresentationText.h"
namespace uwf::composition {
namespace {
class ApplicationViewModelFactory final : public presentation::ViewModelFactory {
 public:
  ApplicationViewModelFactory(std::shared_ptr<compat::Uwf> system, DesktopPlatformPorts ports, app::OverlayFileLoader loader,
                              std::shared_ptr<app::OverlayFilesQueryManager> overlayFileQueries)
      : m_system(std::move(system)),
        m_ports(std::move(ports)),
        m_policy(app::uwfOverlayPolicy(m_system)),
        m_allVolumeProtection(app::allVolumeProtectionActions(app::composeAllVolumeProtection(m_system, {}))),
        m_overlayFiles(std::move(overlayFileQueries)) {
    validateDesktopPlatformPorts(m_ports);
    m_maintenance = app::uwfOverlayMaintenance(m_system, m_ports.volumes, m_ports.maintenanceObservations, m_allVolumeProtection);
    if (!m_overlayFiles) {
      if (!loader)
        loader = [system = m_system](const QString& volumeGuid, std::stop_token stop) { return app::loadUwfOverlayFiles(*system, volumeGuid, stop); };
      m_overlayFiles = std::make_shared<app::OverlayFilesQueryManager>(std::move(loader));
    }
  }
  std::unique_ptr<presentation::MainWindowViewModel> mainWindow(app::ApplicationStateSource& source, app::UwfCapability capability,
                                                                std::shared_ptr<app::FileStagingDraft> staging) override {
    auto model = std::make_unique<presentation::MainWindowViewModel>(
        source, capability, std::move(staging), [volumes = m_ports.volumes](const std::string& path) { return volumes->driveForPath(path); }, m_ports.files);
    QObject::connect(&ui::I18n::events(), &ui::LanguageEvents::changed, model.get(), &presentation::MainWindowViewModel::retranslate);
    return model;
  }
  std::shared_ptr<presentation::SystemInformationViewModel> systemInformation() override {
    return std::make_shared<presentation::SystemInformationViewModel>(m_ports.systemInformation);
  }
  std::shared_ptr<presentation::ImportViewModel> importCommands() override {
    return std::make_shared<presentation::ImportViewModel>(m_ports.readText, QString::fromStdString(m_ports.volumes->systemDrive()));
  }
  std::shared_ptr<presentation::LogViewModel> log() override { return std::make_shared<presentation::LogViewModel>(m_ports.log); }
  std::shared_ptr<presentation::UserConfigExportViewModel> userConfigExport() override {
    return std::make_shared<presentation::UserConfigExportViewModel>(m_ports.applicationDirectory, m_ports.createText);
  }
  std::shared_ptr<presentation::FileStagingViewModel> fileStaging(QString driveLetter, app::FileStagingDraft& draft, app::FileStagingConflictPolicy* conflicts,
                                                                  const app::FileStagingScope& scope) override {
    return std::make_shared<presentation::FileStagingViewModel>(
        std::make_shared<app::FileStagingEditor>(std::move(driveLetter), draft, m_ports.traversal, conflicts, scope));
  }
  std::shared_ptr<presentation::ApplyPlanViewModel> applyPlan(app::ConfigurationChanges changes, const core::UwfSnapshot& snapshot,
                                                              const app::OverlayPolicySnapshot& policy, std::shared_ptr<app::FileStagingDraft> staging = {},
                                                              std::optional<app::OverlayMaintenanceSnapshot> maintenance = {}) override {
    if (staging && !staging->cachedSnapshot())
      throw std::runtime_error(presentation::PresentationText::tr("The file staging list could not be read:\n%1")
                                   .arg(presentation::PresentationText::tr("Unavailable"))
                                   .toStdString());
    const auto stagingChanges = staging ? staging->editSnapshot() : std::nullopt;
    return std::make_shared<presentation::ApplyPlanViewModel>(
        app::reviewApplicationConfiguration(std::move(changes), snapshot, policy, stagingChanges, std::move(maintenance)),
        std::make_shared<app::ConfigurationExecutor>(composition::uwfConfigurationRepository(m_system, std::move(staging), m_policy), stagingChanges,
                                                     m_maintenance),
        m_ports.writeText);
  }
  std::shared_ptr<presentation::EnhancedModeViewModel> enhancedMode(app::EnhancedModeActions& actions) override {
    auto model = std::make_shared<presentation::EnhancedModeViewModel>(actions);
    QObject::connect(&ui::I18n::events(), &ui::LanguageEvents::changed, model.get(), &presentation::EnhancedModeViewModel::retranslate);
    return model;
  }
  std::shared_ptr<presentation::OverlayViewModel> overlay() override {
    return std::make_shared<presentation::OverlayViewModel>(app::uwfOverlayUsageReader(m_system));
  }
  std::shared_ptr<presentation::OverlayFilesViewModel> overlayFiles(std::shared_ptr<presentation::VolumeConfigurationViewModel> volume,
                                                                    presentation::MainWindowViewModel* windowModel) override {
    auto model = std::make_shared<presentation::OverlayFilesViewModel>(volume->driveLetter(), QString::fromStdString(volume->volume().volumeName),
                                                                       m_overlayFiles, m_ports.writeText);
    const QPointer window(windowModel);
    const auto updateAvailability = [target = model.get(), volume = QPointer(volume.get()), window] {
      if (!volume || !window) {
        target->setCommitAvailability(false);
        return;
      }
      const auto availability = volume->commitAvailability();
      const bool globallyAllowed = window->maintenanceActionsAllowed();
      target->setCommitAvailability(globallyAllowed && availability.files, globallyAllowed ? availability.commitHint : window->maintenanceActionRestriction());
    };
    QObject::connect(volume.get(), &presentation::VolumeConfigurationViewModel::stateChanged, model.get(), updateAvailability);
    QObject::connect(volume.get(), &QObject::destroyed, model.get(), [target = model.get()] { target->setCommitAvailability(false); });
    if (window) {
      QObject::connect(window, &presentation::MainWindowViewModel::interactionAvailabilityChanged, model.get(), updateAvailability);
      QObject::connect(window, &QObject::destroyed, model.get(), [target = model.get()] { target->setCommitAvailability(false); });
    }
    QObject::connect(&ui::I18n::events(), &ui::LanguageEvents::changed, model.get(), updateAvailability);
    updateAvailability();
    return model;
  }
  std::shared_ptr<presentation::RegistryPickerViewModel> registryPicker(presentation::RegistryPickerViewModel::Mode mode) override {
    return std::make_shared<presentation::RegistryPickerViewModel>(mode, m_ports.registry);
  }
  std::shared_ptr<presentation::OverlayMaintenanceViewModel> overlayMaintenance(std::shared_ptr<app::OverlayMaintenanceEditor> editor = {}) override {
    return std::make_shared<presentation::OverlayMaintenanceViewModel>(m_maintenance,
                                                                       editor ? std::move(editor) : std::make_shared<app::OverlayMaintenanceEditor>());
  }

 private:
  std::shared_ptr<compat::Uwf> m_system;
  DesktopPlatformPorts m_ports;
  std::shared_ptr<app::OverlayPolicyActions> m_policy;
  std::shared_ptr<app::AllVolumeProtectionActions> m_allVolumeProtection;
  std::shared_ptr<app::OverlayMaintenanceActions> m_maintenance;
  std::shared_ptr<app::OverlayFilesQueryManager> m_overlayFiles;
};
}  // namespace
std::shared_ptr<presentation::ViewModelFactory> createViewModelFactory(std::shared_ptr<compat::Uwf> system, DesktopPlatformPorts ports,
                                                                       app::OverlayFileLoader loader,
                                                                       std::shared_ptr<app::OverlayFilesQueryManager> overlayFileQueries) {
  return std::make_shared<ApplicationViewModelFactory>(std::move(system), std::move(ports), std::move(loader), std::move(overlayFileQueries));
}
}  // namespace uwf::composition
