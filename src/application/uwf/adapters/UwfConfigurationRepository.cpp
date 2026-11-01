// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/adapters/UwfConfigurationRepository.h"

#include <algorithm>
#include <exception>
#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/configuration/execution/ConfigurationResult.h"
#include "application/localization/ApplicationText.h"
#include "application/localization/OverlayTypeText.h"
#include "application/uwf/adapters/UwfCall.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "domain/uwf/contracts/UwfStateFacts.h"
#include "domain/uwf/policies/OverlayWritePlan.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"
#include "foundation/logging/Log.h"
#include "foundation/paths/DriveLetter.h"

namespace uwf::app {

namespace {

using VolumeIdentities = std::map<std::string, std::string>;
VolumeIdentities prepareVolumeIdentities(const ConfigurationPlan& plan, const compat::Uwf& system) {
  VolumeIdentities identities;
  const auto state = system.state();
  const auto& changes = plan.changes;
  // Retain the reviewed configuration GUID. Letter targets use the published
  // physical mount, matching native management lookup independently of Binding.
  const auto include = [&](const auto& edits) {
    for (const auto& [target, value] : edits) {
      (void)value;
      if (const auto id = compat::parseVolumeId(target)) {
        identities.try_emplace(target, *id);
      } else if (const auto* configured = core::findVolumeConfiguration(plan.snapshot.next, target)) {
        const auto configuredId = compat::parseVolumeId(configured->volumeName);
        if (!configuredId) throw std::runtime_error("The reviewed volume configuration has no valid identity");
        identities.try_emplace(target, *configuredId);
      }
    }
  };
  include(changes.volumeProtect);
  include(changes.volumeBindings);
  include(changes.addFileExclusions);
  include(changes.removeFileExclusions);
  // Only published physical mounts establish a drive-to-volume mapping.
  for (const auto* session : {&state.current, &state.next})
    for (const auto& [id, volume] : session->volumes) {
      identities.try_emplace(id, id);
      if (volume.physicalDriveLetter.value && !volume.physicalDriveLetter.value->empty()) identities.try_emplace(*volume.physicalDriveLetter.value, id);
    }
  return identities;
}

std::uint32_t coreTypeToUwf(const core::OverlayType type) { return static_cast<std::uint32_t>(type); }
struct ApplyMessages {
  QString success;
  QString failure;
};

class ApplyJournal final {
 public:
  enum class StepResult { Confirmed, Failed };
  void begin(ConfigurationOperationKind kind, std::string target = {}) {
    m_operation = ConfigurationOperation{kind, std::move(target)};
    m_stepEffect = WriteEffect::None;
    m_stepRequiresReconciliation = false;
  }
  void endOperation() {
    m_operation.reset();
    m_stepEffect = WriteEffect::None;
    m_stepRequiresReconciliation = false;
  }

  template <typename Action>
  StepResult execute(const QString& failureContext, Action&& action) {
    try {
      m_stepEffect = WriteEffect::None;
      m_stepEffect = action();
      // A completed ordinary configuration command still requires a fresh
      // snapshot, including a provider-confirmed unchanged result.
      m_stepRequiresReconciliation = true;
      return StepResult::Confirmed;
    } catch (const std::exception& error) {
      recordError(failureContext, error);
    } catch (...) {
      const QString message = failureContext + ApplicationText::tr(": unknown error");
      UWF_LOG_E("apply") << "operation failed: error=" << message.toStdString();
      recordFailure(message);
    }
    return StepResult::Failed;
  }

  template <typename Action>
  StepResult apply(const ApplyMessages& messages, Action&& action, const bool prerequisiteConfirmed = true) {
    if (!prerequisiteConfirmed) {
      UWF_LOG_W("apply") << "dependent write skipped: context=" << messages.failure.toStdString();
      recordFailure(messages.failure + ": " + ApplicationText::tr("Skipped because a prerequisite operation failed or could not be confirmed."),
                    ConfigurationStepOutcome::DependencySkipped);
      return StepResult::Failed;
    }
    const auto result = execute(messages.failure, std::forward<Action>(action));
    if (result == StepResult::Confirmed) recordSuccess(messages.success);
    return result;
  }

  void recordSuccess(const QString& message) { recordSteps(message, ConfigurationStepOutcome::Succeeded); }

  void recordFailure(const QString& message, ConfigurationStepOutcome outcome = ConfigurationStepOutcome::Failed) { recordSteps(message, outcome); }

  void recordError(const QString& context, const std::exception& error) {
    const auto* operation = dynamic_cast<const OperationError*>(&error);
    if (operation) m_stepEffect = operation->writeEffect();
    const bool canceled = operation && operation->writeEffect() == WriteEffect::None && operation->failureKind() == OperationFailureKind::Canceled;
    recordFailure(context + ": " + QString::fromUtf8(error.what()));
    LogStream log(canceled ? 'I' : 'E', "apply");
    log << (canceled ? "operation canceled: context=" : "operation failed: context=") << context.toStdString()
        << " confirmedWrite=" << m_result.anyWriteConfirmed() << " reconciliationRequired=" << m_result.reconciliationRequired() << " error=" << error.what();
  }

  [[nodiscard]] ConfigurationApplyResult finish(const ConfigurationPlan& plan, std::optional<FileStagingWriteResult> staging) && {
    completeConfigurationResult(m_result, plan);
    if (staging) {
      const auto operation = ConfigurationOperation{ConfigurationOperationKind::Staging, {}};
      const auto step = std::ranges::find_if(m_result.steps, [&](const auto& value) { return value.operation == operation; });
      if (step == m_result.steps.end()) throw std::logic_error("A staging receipt requires its planned operation");
      step->staging = std::move(staging);
    }
    return std::move(m_result);
  }
  [[nodiscard]] bool satisfiesStagingCompletion(const ConfigurationPlan& plan) const {
    return plan.stagingCompletionAlternatives.empty() || std::ranges::any_of(plan.stagingCompletionAlternatives, [&](const auto& operation) {
             return std::ranges::any_of(m_result.steps,
                                        [&](const auto& step) { return step.operation == operation && step.outcome == ConfigurationStepOutcome::Succeeded; });
           });
  }

 private:
  void recordSteps(const QString& message, ConfigurationStepOutcome outcome) {
    if (!m_operation && outcome == ConfigurationStepOutcome::Failed) outcome = ConfigurationStepOutcome::ExecutionFailed;
    recordConfigurationStep(m_result, {message.toStdString(), outcome, m_stepEffect, m_operation, m_stepRequiresReconciliation});
    endOperation();
  }
  ConfigurationApplyResult m_result;
  WriteEffect m_stepEffect = WriteEffect::None;
  bool m_stepRequiresReconciliation = false;
  std::optional<ConfigurationOperation> m_operation;
};

}  // namespace

namespace {
ConfigurationApplyResult executePlan(const compat::Uwf& system, const ConfigurationPlan& plan, const FileStagingWriteOperation& writeStaging,
                                     const VolumeIdentities& identities) {
  const auto& pending = plan.changes;
  std::optional<FileStagingWriteResult> stagingResult;
  ApplyJournal journal;

  if (plan.hasStagingChanges()) {
    journal.begin(ConfigurationOperationKind::Staging);
    const auto result = journal.apply({.success = ApplicationText::tr("File staging"), .failure = ApplicationText::tr("File staging could not be saved")}, [&] {
      if (!writeStaging) throw std::logic_error("file staging apply requires a prepared write");
      stagingResult = writeStaging();
      if (stagingResult->failure) std::rethrow_exception(stagingResult->failure);
      return stagingResult->attempted ? WriteEffect::Confirmed : WriteEffect::None;
    });
    if (result != ApplyJournal::StepResult::Confirmed) return std::move(journal).finish(plan, std::move(stagingResult));
  }

  if (pending.setFilterEnabled) {
    try {
      const bool enable = *pending.setFilterEnabled;
      journal.begin(ConfigurationOperationKind::Filter);
      journal.apply({.success = ApplicationText::tr("✓ Filter: %1").arg(enable ? ApplicationText::tr("Enabled") : ApplicationText::tr("Disabled")),
                     .failure = ApplicationText::tr("✘ Failed to %1 filter").arg(enable ? ApplicationText::tr("enable") : ApplicationText::tr("disable"))},
                    [&] { return callUwfWrite(system, compat::SetBoolean{compat::BooleanProperty::FilterEnabled, enable}); });
    } catch (const std::exception& error) {
      journal.recordError(ApplicationText::tr("✘ Failed to read filter state"), error);
    }
  }

  // Thresholds are global and writable while Current UWF is enabled.
  if (pending.setOverlay.warningThresholdMb || pending.setOverlay.criticalThresholdMb) {
    try {
      const core::OverlayConfigDelta changes{.type = std::nullopt,
                                             .maximumSizeMb = std::nullopt,
                                             .warningThresholdMb = pending.setOverlay.warningThresholdMb,
                                             .criticalThresholdMb = pending.setOverlay.criticalThresholdMb};
      const auto writes = [&] {
        if (!changes.warningThresholdMb || !changes.criticalThresholdMb)
          return std::vector{core::OverlayWriteStep{changes.warningThresholdMb ? core::OverlayField::WarningThreshold : core::OverlayField::CriticalThreshold}};
        const auto published = system.state();
        if (!published.runtime.warningThreshold.value || !published.runtime.criticalThreshold.value)
          throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global overlay threshold state is unavailable");
        const core::OverlayConfig baseline{.warningThresholdMb = *published.runtime.warningThreshold.value,
                                           .criticalThresholdMb = *published.runtime.criticalThreshold.value};
        return core::planOverlayWrites(baseline, changes);
      }();
      auto previous = ApplyJournal::StepResult::Confirmed;
      for (const auto& step : writes) {
        const bool ready = !step.requiresPrevious || previous == ApplyJournal::StepResult::Confirmed;
        if (step.field == core::OverlayField::WarningThreshold) {
          const auto value = *changes.warningThresholdMb;
          journal.begin(ConfigurationOperationKind::WarningThreshold);
          previous = journal.apply(
              {.success = ApplicationText::tr("✓ Overlay warning threshold set to %1 MB").arg(value),
               .failure = ApplicationText::tr("✘ Failed to set warning threshold")},
              [&] { return callUwfWrite(system, compat::SetNumber{compat::NumberProperty::WarningThreshold, value}); }, ready);
        } else {
          const auto value = *changes.criticalThresholdMb;
          journal.begin(ConfigurationOperationKind::CriticalThreshold);
          previous = journal.apply(
              {.success = ApplicationText::tr("✓ Overlay critical threshold set to %1 MB").arg(value),
               .failure = ApplicationText::tr("✘ Failed to set critical threshold")},
              [&] { return callUwfWrite(system, compat::SetNumber{compat::NumberProperty::CriticalThreshold, value}); }, ready);
        }
      }
    } catch (const std::exception& error) {
      journal.recordError(ApplicationText::tr("✘ Failed to read overlay state"), error);
    }
  }

  // Type/capacity pairs need a current baseline only to select their order.
  // Single setters retain their own admission and capability boundaries.
  if (pending.setOverlay.touchesOverlayConfig()) {
    try {
      const auto published = system.state();
      if (published.current.filterEnabled.value == true)
        throw OperationError(std::make_error_code(std::errc::permission_denied),
                             ApplicationText::tr("✘ Type / maximum capacity not applied: UWF is enabled. Disable UWF and restart first.").toStdString());
      const core::OverlayConfigDelta changes{.type = pending.setOverlay.type,
                                             .maximumSizeMb = pending.setOverlay.maximumSizeMb,
                                             .warningThresholdMb = std::nullopt,
                                             .criticalThresholdMb = std::nullopt};
      const auto writes = [&] {
        if (!changes.type || !changes.maximumSizeMb)
          return std::vector{core::OverlayWriteStep{changes.type ? core::OverlayField::Type : core::OverlayField::MaximumSize}};
        if (!published.next.overlayType.value || !published.next.maximumOverlaySize.value)
          throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global Next overlay configuration is unavailable");
        const core::OverlayConfig baseline{.type = overlayTypeFromUwf(*published.next.overlayType.value),
                                           .maximumSizeMb = *published.next.maximumOverlaySize.value};
        return core::planOverlayWrites(baseline, changes);
      }();
      auto previous = ApplyJournal::StepResult::Confirmed;
      for (const auto& step : writes) {
        const bool ready = !step.requiresPrevious || previous == ApplyJournal::StepResult::Confirmed;
        if (step.field == core::OverlayField::Type) {
          const auto type = *changes.type;
          journal.begin(ConfigurationOperationKind::OverlayType);
          previous = journal.apply(
              {.success = ApplicationText::tr("✓ Overlay type set to %1").arg(app::overlayTypeText(type)),
               .failure = ApplicationText::tr("✘ Failed to set overlay type")},
              [&] { return callUwfWrite(system, compat::SetNumber{compat::NumberProperty::OverlayType, coreTypeToUwf(type)}); }, ready);
        } else {
          const auto value = *changes.maximumSizeMb;
          journal.begin(ConfigurationOperationKind::OverlaySize);
          previous = journal.apply(
              {.success = ApplicationText::tr("✓ Maximum overlay capacity set to %1 MB").arg(value),
               .failure = ApplicationText::tr("✘ Failed to set maximum capacity")},
              [&] { return callUwfWrite(system, compat::SetNumber{compat::NumberProperty::MaximumOverlaySize, value}); }, ready);
        }
      }
    } catch (const std::exception& error) {
      journal.recordError(ApplicationText::tr("✘ Failed to read overlay configuration"), error);
    }
  }

  if (!pending.volumeProtect.empty() || !pending.volumeBindings.empty() || !pending.addFileExclusions.empty() || !pending.removeFileExclusions.empty()) {
    try {
      // Existing registrations admit independent edits. New registrations must
      // finish their protection prerequisite; a partially completed Add is insufficient.
      const auto initial = system.state().next;
      std::set<std::string> confirmedProtection;
      const auto volumeId = [&](const std::string& drive) -> const std::string& {
        const auto selected = identities.find(drive);
        if (selected == identities.end()) throw std::runtime_error("The prepared configuration has no physical volume identity for the requested drive");
        return selected->second;
      };
      const auto fileAddress = [&](const std::string& target) {
        if (const auto* configured = core::findVolumeConfiguration(plan.snapshot.next, target)) return core::volumeConfigurationAddress(*configured);
        const auto letter = drive::normalize(target);
        if (!letter.empty()) return letter;
        const auto found = initial.volumes.find(volumeId(target));
        if (found == initial.volumes.end()) return std::string{};
        const auto& volume = found->second;
        const core::VolumeRecord configured{
            .volumeName = target,
            .driveLetter = volume.driveLetter.value.value_or(std::string{}),
            .binding = volume.bindByDriveLetter.value == true ? core::VolumeBinding::DriveLetter : core::VolumeBinding::VolumeId,
            .physicalDriveLetter = volume.physicalDriveLetter.value};
        return core::volumeConfigurationAddress(configured);
      };
      const auto registrationReady = [&](const std::string& drive) {
        const auto& id = volumeId(drive);
        if (confirmedProtection.contains(id)) return true;
        const auto found = initial.volumes.find(id);
        if (found != initial.volumes.end() && found->second.present.value) return *found->second.present.value;
        if (found == initial.volumes.end() && initial.volumesObserved) return false;
        throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global Next volume membership is unavailable");
      };
      for (const auto& [drive, protect] : pending.volumeProtect) {
        journal.begin(ConfigurationOperationKind::VolumeProtection, drive);
        const auto protectionResult = journal.apply(
            {.success = ApplicationText::tr("✓ Volume %1 protection: %2")
                            .arg(QString::fromStdString(drive), protect ? ApplicationText::tr("Enabled") : ApplicationText::tr("Disabled")),
             .failure = ApplicationText::tr("✘ Failed to %1 protection on volume %2")
                            .arg(protect ? ApplicationText::tr("enable") : ApplicationText::tr("disable"), QString::fromStdString(drive))},
            [&] {
              const bool byIdentity = compat::parseVolumeId(drive).has_value();
              return callUwfWrite(
                  system, compat::SetVolume{volumeId(drive), compat::VolumeProperty::Protected, protect, byIdentity ? std::string{} : drive, !byIdentity});
            });
        if (protect && protectionResult == ApplyJournal::StepResult::Confirmed) confirmedProtection.insert(volumeId(drive));
      }
      for (const auto& [drive, binding] : pending.volumeBindings) {
        journal.begin(ConfigurationOperationKind::VolumeBinding, drive);
        journal.apply(
            {.success = ApplicationText::tr("✓ Volume %1 bind by: %2")
                            .arg(QString::fromStdString(drive),
                                 binding == core::VolumeBinding::VolumeId ? ApplicationText::tr("volume ID") : ApplicationText::tr("drive letter")),
             .failure = ApplicationText::tr("✘ Failed to set binding for volume %1").arg(QString::fromStdString(drive))},
            [&] {
              return callUwfWrite(system,
                                  compat::SetVolume{volumeId(drive), compat::VolumeProperty::BindByDriveLetter, binding == core::VolumeBinding::DriveLetter});
            },
            registrationReady(drive));
      }
      for (const auto& [drive, paths] : pending.addFileExclusions) {
        for (const auto& path : paths) {
          journal.begin(ConfigurationOperationKind::AddFileExclusion, drive + path);
          journal.apply(
              {.success = ApplicationText::tr("✓ Volume %1 added file exclusion: %2").arg(QString::fromStdString(drive), QString::fromStdString(path)),
               .failure = ApplicationText::tr("✘ Volume %1 failed to add file exclusion %2").arg(QString::fromStdString(drive), QString::fromStdString(path))},
              [&] {
                return callUwfWrite(system, compat::EditFileExclusion{volumeId(drive), toUwfFilePath(path, fileAddress(drive)), compat::ExclusionAction::Add});
              },
              registrationReady(drive));
        }
      }
      for (const auto& [drive, paths] : pending.removeFileExclusions) {
        for (const auto& path : paths) {
          journal.begin(ConfigurationOperationKind::RemoveFileExclusion, drive + path);
          journal.apply(
              {.success = ApplicationText::tr("✓ Volume %1 removed file exclusion: %2").arg(QString::fromStdString(drive), QString::fromStdString(path)),
               .failure =
                   ApplicationText::tr("✘ Volume %1 failed to remove file exclusion %2").arg(QString::fromStdString(drive), QString::fromStdString(path))},
              [&] {
                return callUwfWrite(system,
                                    compat::EditFileExclusion{volumeId(drive), toUwfFilePath(path, fileAddress(drive)), compat::ExclusionAction::Remove});
              });
        }
      }
    } catch (const std::exception& error) {
      journal.recordError(ApplicationText::tr("✘ Failed to read volume configuration"), error);
    }
  }

  journal.endOperation();
  if (!pending.addRegistryExclusions.empty() || !pending.removeRegistryExclusions.empty() || pending.setPersistDomainSecretKey || pending.setPersistTSCAL) {
    try {
      for (const auto& k : pending.addRegistryExclusions) {
        journal.begin(ConfigurationOperationKind::AddRegistryExclusion, k);
        journal.apply({.success = ApplicationText::tr("✓ Added registry exclusion: %1").arg(QString::fromStdString(k)),
                       .failure = ApplicationText::tr("✘ Failed to add registry exclusion %1").arg(QString::fromStdString(k))},
                      [&] { return callUwfWrite(system, compat::EditRegistryExclusion{k, compat::ExclusionAction::Add}); });
      }
      for (const auto& k : pending.removeRegistryExclusions) {
        journal.begin(ConfigurationOperationKind::RemoveRegistryExclusion, k);
        journal.apply({.success = ApplicationText::tr("✓ Removed registry exclusion: %1").arg(QString::fromStdString(k)),
                       .failure = ApplicationText::tr("✘ Failed to remove registry exclusion %1").arg(QString::fromStdString(k))},
                      [&] { return callUwfWrite(system, compat::EditRegistryExclusion{k, compat::ExclusionAction::Remove}); });
      }
      if (pending.setPersistDomainSecretKey) {
        journal.begin(ConfigurationOperationKind::DomainPersistence);
        journal.apply(
            {.success = ApplicationText::tr("✓ %1 persistence: %2")
                            .arg(ApplicationText::tr("Domain Secret Key (DomainSecretKey)"),
                                 *pending.setPersistDomainSecretKey ? ApplicationText::tr("Enabled") : ApplicationText::tr("Disabled")),
             .failure = ApplicationText::tr("✘ Failed to update registry persistence switches")},
            [&] { return callUwfWrite(system, compat::SetBoolean{compat::BooleanProperty::PersistDomainSecretKey, *pending.setPersistDomainSecretKey}); });
      }
      if (pending.setPersistTSCAL) {
        journal.begin(ConfigurationOperationKind::LicensePersistence);
        journal.apply({.success = ApplicationText::tr("✓ %1 persistence: %2")
                                      .arg(ApplicationText::tr("Terminal Services Client Access License (TSCAL)"),
                                           *pending.setPersistTSCAL ? ApplicationText::tr("Enabled") : ApplicationText::tr("Disabled")),
                       .failure = ApplicationText::tr("✘ Failed to update registry persistence switches")},
                      [&] { return callUwfWrite(system, compat::SetBoolean{compat::BooleanProperty::PersistTscal, *pending.setPersistTSCAL}); });
      }
    } catch (const std::exception& error) {
      journal.recordError(ApplicationText::tr("✘ Failed to read registry filter"), error);
    }
  }

  // Storage completion and future-session coverage are separate facts. Only
  // the plan's actual durability dependencies can retain staging repair intent.
  if (stagingResult && !stagingResult->failure && !journal.satisfiesStagingCompletion(plan))
    stagingResult->failure = std::make_exception_ptr(OperationError(
        std::make_error_code(std::errc::state_not_recoverable), ApplicationText::tr("File staging could not be saved").toStdString(), WriteEffect::Uncertain));

  return std::move(journal).finish(plan, std::move(stagingResult));
}

}  // namespace
namespace {
class UwfConfigurationRepository final : public ConfigurationRepository {
 public:
  UwfConfigurationRepository(std::shared_ptr<compat::Uwf> system, std::shared_ptr<FileStagingDraft> staging)
      : m_system(std::move(system)), m_staging(std::move(staging)) {
    if (!m_system) throw std::invalid_argument("UWF compatibility service is required");
  }
  bool supports(ConfigurationArea area) const override { return area == ConfigurationArea::Settings; }
  OverlayPolicySnapshot snapshot() const override { return UwfOverlayOperations(m_system).snapshot(); }
  WriteOperation prepareWrite(const ConfigurationPhase& phase, const ConfigurationPlan& plan) override {
    const auto* payload = std::get_if<ConfigurationSettingsPhase>(&phase);
    if (!payload) throw std::invalid_argument("Standard settings repository requires a settings phase");
    // Execute the reviewed ownership/dependency decisions unchanged. Running
    // the planner again would reinterpret a synthesized empty confirmation as
    // a new user edit and could remove additional registry exclusions.
    auto settings = plan;
    settings.changes = payload->settings;
    settings.staging = payload->staging;
    settings.stagingCompletionAlternatives = payload->stagingCompletionAlternatives;
    settings.overlayPolicy = {};
    settings.maintenance = {};
    auto write = m_staging && settings.hasStagingChanges() ? m_staging->prepareWrite(*settings.staging) : FileStagingWriteOperation{};
    return [system = m_system, settings = std::move(settings), write = std::move(write)](const OverlayPolicySnapshot&) {
      const auto identities = prepareVolumeIdentities(settings, *system);
      auto result = executePlan(*system, settings, write, identities);
      result.observedOverlayPolicy = UwfOverlayOperations(system).snapshot();
      return result;
    };
  }
  void completeWrite(const ConfigurationApplyResult& result) override {
    if (m_staging && result.staging()) m_staging->acceptWrite(*result.staging());
  }

 private:
  std::shared_ptr<compat::Uwf> m_system;
  std::shared_ptr<FileStagingDraft> m_staging;
};
}  // namespace

std::shared_ptr<ConfigurationRepository> uwfConfigurationRepository(std::shared_ptr<compat::Uwf> system, std::shared_ptr<FileStagingDraft> staging) {
  return std::make_shared<UwfConfigurationRepository>(std::move(system), std::move(staging));
}
}  // namespace uwf::app
