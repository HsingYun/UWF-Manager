// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"

#include <QLocale>
#include <QPointer>
#include <QStringList>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "application/configuration/execution/ConfigurationPlanExecution.h"
#include "application/configuration/execution/ConfigurationResult.h"
#include "application/configuration/volumes/VolumeConfigurationEditor.h"
#include "application/localization/OverlayTypeText.h"
#include "application/operations/contracts/OperationError.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"
#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/common/TextFileExport.h"
#include "presentation/viewmodels/configuration/MaintenanceValidationText.h"
#include "presentation/viewmodels/configuration/OverlayAdvancedViewModel.h"
#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"

namespace uwf::presentation {
namespace {
QString commandPreview(const app::ConfigurationScript& script) {
  QStringList lines;
  for (const auto& command : script.commands) lines.append(QString::fromStdString(app::renderUwfmgrCommand(command)));
  const auto scope = app::configurationScriptScope(script);
  if (!scope.empty()) {
    if (!lines.isEmpty()) lines.append(QString{});
    lines.append(QString::fromStdString(scope));
  }
  return lines.join(u'\n');
}
QString activationTimingText(const app::ConfigurationActivationTiming timing) {
  switch (timing) {
    case app::ConfigurationActivationTiming::OnApply:
      return PresentationText::tr("Takes effect when applied");
    case app::ConfigurationActivationTiming::WhenUwfStarts:
      return PresentationText::tr("Used when UWF starts");
    case app::ConfigurationActivationTiming::AfterRestart:
      return PresentationText::tr("Takes effect after restart");
    case app::ConfigurationActivationTiming::Mixed:
      return PresentationText::tr("Mixed activation times");
  }
  return {};
}
}  // namespace
ApplyPlanViewModel::ApplyPlanViewModel(core::PendingChanges changes, core::UwfSnapshot snapshot, std::shared_ptr<app::ConfigurationActions> actions,
                                       std::optional<app::FileStagingChanges> stagingChanges, app::TextFileWriter textFileWriter)
    : ApplyPlanViewModel(app::planConfiguration(std::move(changes), std::move(snapshot), std::move(stagingChanges)), std::move(actions),
                         std::move(textFileWriter)) {}

ApplyPlanViewModel::ApplyPlanViewModel(app::ConfigurationPlan plan, std::shared_ptr<app::ConfigurationActions> actions, app::TextFileWriter textFileWriter)
    : m_plan(std::move(plan)), m_actions(std::move(actions)), m_textFileWriter(std::move(textFileWriter)) {
  const auto validation = app::validateOverlayPolicyPlan(m_plan);
  m_validPlan = validation.valid();
  for (const auto& conflict : validation.conflicts) {
    if (!m_warning.isEmpty()) m_warning += u'\n';
    m_warning += OverlayAdvancedViewModel::conflictText(conflict.issue);
  }
  try {
    app::validateOverlayMaintenancePlan(m_plan);
    if (m_validPlan) {
      m_changeScript = app::configurationChangeScript(m_plan);
      m_targetScript = app::configurationTargetScript(m_plan);
    }
  } catch (const app::OverlayPolicyError&) {
    m_validPlan = false;
    m_warning =
        PresentationText::tr("The draft contains incompatible or unavailable overlay settings. Return to editing and correct the overlay configuration.");
  } catch (const app::MaintenanceValidationError& error) {
    m_validPlan = false;
    m_warning = maintenanceValidationText(error.issue());
  } catch (const app::OperationError& error) {
    m_validPlan = false;
    m_warning = QString::fromUtf8(error.what());
  } catch (const std::invalid_argument& error) {
    m_validPlan = false;
    m_warning = QString::fromUtf8(error.what());
  }
  m_changeCommandCount = m_changeScript.commands.size();
  if (m_plan.changes.setOverlay.touchesOverlayConfig() && m_plan.snapshot.current.filter.enabled)
    m_warning = PresentationText::tr("Disable UWF and restart before changing the overlay type or maximum capacity.");
  m_canApply = m_validPlan && changeCount() > 0;
  const auto enabledText = [](const bool value) { return value ? PresentationText::tr("On") : PresentationText::tr("Off"); };
  const auto sizeText = [](const uint32_t value) { return QLocale(QLocale::English, QLocale::UnitedStates).toString(value) + QStringLiteral(" MB"); };
  const auto overlayBaseline = app::configurationOverlayBaseline(m_plan);
  const auto overlayTarget = app::configurationOverlayTarget(m_plan);
  ChangeGroup protection{PresentationText::tr("Protection and overlay"), QStringLiteral(":/icons/enhanced.svg"), {}};
  const auto& baseline = m_plan.snapshot.next;
  const auto overlayTypeBefore = overlayBaseline.next.overlayType.status == app::PolicyObservationStatus::Available
                                     ? core::overlayTypeFromValue(overlayBaseline.next.overlayType.value)
                                     : baseline.overlay.type;
  if (m_plan.changes.setFilterEnabled)
    protection.items.append({PresentationText::tr("Write protection"), PresentationText::tr("UWF filter"), enabledText(baseline.filter.enabled),
                             enabledText(*m_plan.changes.setFilterEnabled)});
  const auto& overlay = m_plan.changes.setOverlay;
  if (overlay.type || (m_plan.maintenance.storageVolume && overlayTypeBefore != overlayTarget.overlay.type))
    protection.items.append({PresentationText::tr("Overlay type"), PresentationText::tr("Storage"), app::overlayTypeText(overlayTypeBefore),
                             app::overlayTypeText(overlayTarget.overlay.type)});
  if (overlay.maximumSizeMb)
    protection.items.append({PresentationText::tr("Maximum overlay capacity"), app::overlayTypeText(overlayTarget.overlay.type),
                             sizeText(baseline.overlay.maximumSizeMb), sizeText(*overlay.maximumSizeMb)});
  if (overlay.warningThresholdMb)
    protection.items.append({PresentationText::tr("Warning threshold"),
                             activationTimingText(m_plan.activationTiming(app::ConfigurationOperationKind::WarningThreshold)),
                             sizeText(baseline.overlay.warningThresholdMb), sizeText(*overlay.warningThresholdMb)});
  if (overlay.criticalThresholdMb)
    protection.items.append({PresentationText::tr("Critical threshold"),
                             activationTimingText(m_plan.activationTiming(app::ConfigurationOperationKind::CriticalThreshold)),
                             sizeText(baseline.overlay.criticalThresholdMb), sizeText(*overlay.criticalThresholdMb)});
  if (m_plan.maintenance.allVolumesProtected) {
    const bool enabled = *m_plan.maintenance.allVolumesProtected;
    const auto& observed = m_plan.maintenanceSnapshot;
    const auto confirmed = observed ? (enabled ? observed->allVolumesNextProtected : observed->allVolumesNextUnprotected) : std::nullopt;
    protection.items.append({enabled ? PresentationText::tr("Protect all") : PresentationText::tr("Unprotect all"),
                             PresentationText::tr("Will take effect at the next startup after applying."),
                             confirmed ? (*confirmed ? PresentationText::tr("Met") : PresentationText::tr("Not met")) : PresentationText::tr("Unknown"),
                             PresentationText::tr("Met")});
  }
  for (const auto& [drive, protect] : m_plan.changes.volumeProtect) {
    const auto identity = app::VolumeConfigurationEditor::volumeIdentity({.driveLetter = {}, .volumeName = drive, .fileSystem = {}, .label = {}});
    const auto* found = core::findVolumeConfiguration(baseline, drive);
    const auto target = QString::fromStdString(found ? core::volumeConfigurationAddress(*found) : drive);
    const bool letter = target.size() == 2 && target.back() == QLatin1Char(':');
    const auto bulk = m_plan.maintenance.allVolumesProtected;
    const auto before = bulk && protect != *bulk ? enabledText(*bulk) : !found ? PresentationText::tr("Unknown") : enabledText(found->isProtected);
    protection.items.append({letter ? PresentationText::tr("Volume %1 protection").arg(target) : PresentationText::tr("Volume protection"),
                             letter ? PresentationText::tr("Write protection") : identity, before, enabledText(protect)});
  }
  for (const auto& [drive, targetBinding] : m_plan.changes.volumeBindings) {
    const auto binding = [](const core::VolumeBinding value) {
      return value == core::VolumeBinding::VolumeId ? PresentationText::tr("By volume ID") : PresentationText::tr("By drive letter");
    };
    const auto* found = core::findVolumeConfiguration(baseline, drive);
    const auto before = found                                            ? binding(found->binding)
                        : m_plan.maintenance.allVolumesProtected == true ? binding(core::VolumeBinding::VolumeId)
                                                                         : PresentationText::tr("Unknown");
    const auto address = found ? core::volumeConfigurationAddress(*found) : std::string{};
    protection.items.append({PresentationText::tr("Volume %1 binding").arg(QString::fromStdString(address.empty() ? drive : address)),
                             PresentationText::tr("Volume identity"), before, binding(targetBinding)});
  }
  if (!protection.items.isEmpty()) m_groups.append(std::move(protection));
  ChangeGroup advanced{PresentationText::tr("Overlay advanced settings"), QStringLiteral(":/icons/settings.svg"), {}};
  const auto flagsBefore = overlayBaseline.next.flags.status == app::PolicyObservationStatus::Available ? overlayBaseline.next.flags.value : std::nullopt;
  const auto addFlag = [&](OverlayAdvancedViewModel::Field field, const std::optional<bool> requested, const auto readFlag) {
    const auto before = flagsBefore ? std::optional{readFlag(*flagsBefore)} : std::nullopt;
    const auto after = overlayTarget.flags ? std::optional{readFlag(*overlayTarget.flags)} : requested;
    if (!requested && (!before || !after || before == after)) return;
    advanced.items.append({OverlayAdvancedViewModel::fieldName(field), PresentationText::tr("Will take effect at the next startup after applying."),
                           before ? enabledText(*before) : PresentationText::tr("Unknown"), after ? enabledText(*after) : PresentationText::tr("Unknown")});
  };
  addFlag(OverlayAdvancedViewModel::Field::Passthrough, m_plan.overlayPolicy.passthrough, [](const app::OverlayFlags flags) { return flags.passthrough(); });
  addFlag(OverlayAdvancedViewModel::Field::Persistent, m_plan.overlayPolicy.persistent, [](const app::OverlayFlags flags) { return flags.persistent(); });
  addFlag(OverlayAdvancedViewModel::Field::ReadOnlyMedia, m_plan.overlayPolicy.readOnlyMedia,
          [](const app::OverlayFlags flags) { return flags.readOnlyMedia(); });
  if (!advanced.items.isEmpty()) m_groups.append(std::move(advanced));
  if (m_plan.overlayPolicy.reset) {
    const auto clearText = [](const bool clear) { return clear ? PresentationText::tr("Scheduled to clear") : PresentationText::tr("Not scheduled to clear"); };
    const bool clear = m_plan.overlayPolicy.reset == app::PersistentReset::Clear;
    const auto description = clear
                                 ? PresentationText::tr("Clears once at the next startup, then preserves data. The current session is not cleared immediately.")
                                 : PresentationText::tr("Cancels the scheduled clear at the next startup, preserving persistent overlay data.");
    m_groups.append(
        {PresentationText::tr("Reset persistent overlay · One-time request"),
         QStringLiteral(":/icons/restart.svg"),
         {{OverlayAdvancedViewModel::fieldName(OverlayAdvancedViewModel::Field::Reset), description,
           m_plan.policySnapshot.resetMode() ? clearText(m_plan.policySnapshot.resetMode() == app::PersistentReset::Clear) : PresentationText::tr("Unknown"),
           clearText(clear)}}});
    if (clear && m_validPlan) {
      if (!m_warning.isEmpty()) m_warning += u'\n';
      m_warning += description;
    }
  }
  ChangeGroup files{PresentationText::tr("File exclusions"), QStringLiteral(":/icons/folder.svg"), {}};
  const auto appendPaths = [](ChangeGroup& group, const auto& paths, const bool added, const QString& description) {
    for (const auto& path : paths)
      group.items.append({QString::fromStdString(path), description, {}, added ? PresentationText::tr("Added") : PresentationText::tr("Removed"), true});
  };
  for (const auto& [drive, paths] : m_plan.changes.addFileExclusions) {
    Q_UNUSED(drive);
    appendPaths(files, paths, true, PresentationText::tr("Add this path to the next-session exclusions."));
  }
  for (const auto& [drive, paths] : m_plan.changes.removeFileExclusions) {
    Q_UNUSED(drive);
    appendPaths(files, paths, false, PresentationText::tr("Remove this path from the next-session exclusions."));
  }
  if (!files.items.isEmpty()) m_groups.append(std::move(files));
  ChangeGroup registry{PresentationText::tr("Registry exclusions"), QStringLiteral(":/icons/registry.svg"), {}};
  for (const auto& path : m_plan.changes.addRegistryExclusions) {
    const auto key = QString::fromStdString(path);
    const bool stagingDependency = m_plan.staging && !m_plan.staging->after.isEmpty() && app::isFileStagingRegistryRoot(key);
    registry.items.append({key,
                           stagingDependency ? PresentationText::tr("File staging relies on this registry exclusion to preserve its list across restarts.")
                                             : PresentationText::tr("Add this key to the next-session exclusions."),
                           {},
                           stagingDependency ? PresentationText::tr("Built-in") : PresentationText::tr("Added"),
                           true});
  }
  appendPaths(registry, m_plan.changes.removeRegistryExclusions, false, PresentationText::tr("Remove this key from the next-session exclusions."));
  if (m_plan.changes.setPersistDomainSecretKey)
    registry.items.append({PresentationText::tr("Domain Secret Key persistence"),
                           {},
                           enabledText(baseline.persistDomainSecretKey),
                           enabledText(*m_plan.changes.setPersistDomainSecretKey)});
  if (m_plan.changes.setPersistTSCAL)
    registry.items.append(
        {PresentationText::tr("Terminal Services license persistence"), {}, enabledText(baseline.persistTSCAL), enabledText(*m_plan.changes.setPersistTSCAL)});
  if (!registry.items.isEmpty()) m_groups.append(std::move(registry));
  if (hasStagingChanges()) {
    ChangeGroup staging{PresentationText::tr("File staging"), QStringLiteral(":/icons/file_staging.svg"), {}};
    if (m_plan.staging->persistencePending && m_plan.staging->added().isEmpty() && m_plan.staging->removed().isEmpty()) {
      if (m_plan.staging->after.isEmpty())
        staging.items.append({PresentationText::tr("File staging"),
                              PresentationText::tr("Confirm that the saved staging list is empty before removing its registry exclusion."),
                              PresentationText::tr("Pending confirmation"), PresentationText::tr("Empty")});
      else
        staging.items.append({PresentationText::tr("File staging"), PresentationText::tr("File staging is saved after Apply"),
                              PresentationText::tr("Unavailable"), PresentationText::tr("Ready")});
    }
    for (auto entries = m_plan.staging->removed(); const auto& entry : std::as_const(entries))
      staging.items.append({entry.path, PresentationText::tr("Remove from automatic file staging after Apply."), {}, PresentationText::tr("Removed"), true});
    for (auto entries = m_plan.staging->added(); const auto& entry : std::as_const(entries))
      staging.items.append({entry.path, PresentationText::tr("Add to automatic file staging after Apply."), {}, PresentationText::tr("Added"), true});
    m_groups.append(std::move(staging));
    if (m_plan.staging->after.isEmpty()) {
      if (!m_warning.isEmpty()) m_warning += u'\n';
      m_warning += PresentationText::tr("Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.");
    }
  }
  if (!m_plan.maintenance.empty()) {
    ChangeGroup maintenance{PresentationText::tr("Maintenance and recovery"), QStringLiteral(":/icons/settings.svg"), {}};
    const auto& requested = m_plan.maintenance;
    const auto& observed = m_plan.maintenanceSnapshot;
    const auto volumeName = [&](const QString& id) {
      if (observed) {
        const auto found = std::ranges::find_if(observed->volumes, [&](const auto& volume) { return volume.id == id; });
        if (found != observed->volumes.end()) return found->driveLetter.isEmpty() ? found->id : found->driveLetter;
      }
      return id;
    };
    if (requested.storageVolume) {
      maintenance.items.append({PresentationText::tr("Configure storage location"),
                                PresentationText::tr("Will take effect at the next startup after applying."),
                                observed ? OverlayMaintenanceViewModel::storageLocationText(observed->nextStorage) : PresentationText::tr("Unknown"),
                                volumeName(*requested.storageVolume)});
    }
    if (requested.hormEnabled)
      maintenance.items.append({PresentationText::tr("Hibernate once / resume many"), QStringLiteral("HORM"),
                                observed && observed->hormEnabled ? enabledText(*observed->hormEnabled) : PresentationText::tr("Unknown"),
                                enabledText(*requested.hormEnabled)});
    if (requested.fullCommit) {
      maintenance.items.append({PresentationText::tr("Full volume commit"),
                                PresentationText::tr("Applies to every protected volume; starts only when this draft is applied."),
                                PresentationText::tr("Not started"), PresentationText::tr("Will start when applied")});
      if (!m_warning.isEmpty()) m_warning += u'\n';
      m_warning += PresentationText::tr(
          "Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.");
    }
    if (!maintenance.items.isEmpty()) m_groups.append(std::move(maintenance));
  }
  for (int group = 0; group < m_groups.size(); ++group) {
    m_rows.append({group, -1});
    for (int entry = 0; entry < m_groups[group].items.size(); ++entry) m_rows.append({group, entry});
  }
}

int ApplyPlanViewModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(m_rows.size()) + 1; }
QVariant ApplyPlanViewModel::data(const QModelIndex& index, const int role) const {
  if (!index.isValid() || index.column() != 0 || index.row() < 0 || index.row() >= rowCount()) return {};
  if (index.row() == m_rows.size()) return role == KindRole ? QVariant(Commands) : QVariant{};
  const auto& row = m_rows[index.row()];
  const auto& group = m_groups[row.group];
  if (role == KindRole) return row.entry < 0 ? Heading : Entry;
  if (row.entry < 0) {
    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) return group.title;
    if (role == Qt::DecorationRole) return group.icon;
    return {};
  }
  const auto& change = group.items[row.entry];
  switch (role) {
    case Qt::DisplayRole:
      return change.title;
    case Qt::AccessibleTextRole:
      return change.title + u'\n' + change.description + u'\n' + change.before + u'\n' + change.after;
    case DescriptionRole:
      return change.description;
    case BeforeRole:
      return change.before;
    case AfterRole:
      return change.after;
    case PathRole:
      return change.path;
    case LastRole:
      return row.entry + 1 == group.items.size();
    default:
      return {};
  }
}
Qt::ItemFlags ApplyPlanViewModel::flags(const QModelIndex& index) const { return index.isValid() ? Qt::ItemIsEnabled : Qt::NoItemFlags; }

QString ApplyPlanViewModel::changeCommandsText() const { return m_validPlan ? commandPreview(m_changeScript) : m_warning; }
QString ApplyPlanViewModel::targetCommandsText() const { return m_validPlan ? commandPreview(m_targetScript) : m_warning; }
QString ApplyPlanViewModel::exportRestrictionText(const ScriptScope scope) const {
  if (!script(scope).exportSupported) return PresentationText::tr("Bulk unprotection requires a confirmed complete next-session volume scope.");
  if (hasStagingChanges()) return PresentationText::tr("File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.");
  if (!script(scope).commands.empty()) return {};
  return script(scope).omissions.empty() ? PresentationText::tr("No pending changes") : QString::fromStdString(app::configurationScriptScope(script(scope)));
}
QString ApplyPlanViewModel::resultText() const {
  QStringList lines;
  for (const auto& receipt : m_result.steps) {
    if (!receipt.description.empty()) lines.append(QString::fromStdString(receipt.description));
    if (receipt.validationIssue) lines.append(maintenanceValidationText(*receipt.validationIssue));
    if (!receipt.maintenance) continue;
    const auto& maintenance = *receipt.maintenance;
    if (maintenance.allVolumeItems.empty()) continue;
    lines.append(QString{});
    for (const auto& item : maintenance.allVolumeItems) {
      const auto identity =
          app::VolumeConfigurationEditor::volumeIdentity({.driveLetter = {}, .volumeName = item.volumeId.toStdString(), .fileSystem = {}, .label = {}});
      const auto volume = std::ranges::find_if(maintenance.snapshot.volumes, [&](const auto& entry) {
        return app::VolumeConfigurationEditor::volumeIdentity({.driveLetter = {}, .volumeName = entry.id.toStdString(), .fileSystem = {}, .label = {}}) ==
               identity;
      });
      QString name;
      if (volume != maintenance.snapshot.volumes.end()) {
        name = volume->label.trimmed();
        if (!volume->driveLetter.isEmpty()) name = name.isEmpty() ? volume->driveLetter : QStringLiteral("%1 (%2)").arg(name, volume->driveLetter);
      }
      if (name.isEmpty() || name.contains(identity, Qt::CaseInsensitive)) name = PresentationText::tr("Partition");
      const bool applied =
          item.error.isEmpty() && !item.failure && item.completionConfirmed && item.confirmedProtection == m_plan.maintenance.allVolumesProtected;
      const auto status = applied                                 ? PresentationText::tr("Applied")
                          : item.effect == app::WriteEffect::None ? PresentationText::tr("Not applied")
                                                                  : PresentationText::tr("Could not confirm");
      lines.append(QStringLiteral("%1 — %2").arg(name, status));
      if (!item.volumeId.isEmpty()) lines.append(QStringLiteral("  %1").arg(item.volumeId));
      if (!item.error.isEmpty())
        for (auto details = item.error.split(u'\n'); const auto& detail : std::as_const(details)) lines.append(QStringLiteral("  %1").arg(detail));
    }
  }
  return lines.join(u'\n');
}

bool ApplyPlanViewModel::resultFailed() const { return m_result.failed(); }

bool ApplyPlanViewModel::canRestart() const { return m_result.anyWriteConfirmed(); }
bool ApplyPlanViewModel::canSafeRestart() const { return canRestart() && !m_applying && m_interaction.safeRestart; }
bool ApplyPlanViewModel::canDirectRestart() const { return canRestart() && !m_applying && m_interaction.directRestart; }
QString ApplyPlanViewModel::safeRestartRestriction() const {
  if (canSafeRestart()) return {};
  if (!m_interaction.safeRestart) return m_interaction.safeRestartRestriction;
  if (m_applying) return PresentationText::tr("Please wait for the operation to finish.");
  return {};
}
QString ApplyPlanViewModel::directRestartRestriction() const {
  return canDirectRestart() ? PresentationText::tr("Direct restart skips staged file commits.") : m_interaction.directRestartRestriction;
}

QString ApplyPlanViewModel::resultHeading() const {
  return resultFailed() ? PresentationText::tr("Some changes could not be applied") : PresentationText::tr("Configuration applied");
}

QString ApplyPlanViewModel::activationText() const { return activationTimingText(m_plan.activationTiming()); }

QString ApplyPlanViewModel::resultDescription() const {
  if (resultFailed())
    return m_result.anyWriteConfirmed() || m_result.reconciliationRequired()
               ? PresentationText::tr("Some changes may already have been written. Refresh the system state before retrying.")
               : PresentationText::tr("No configuration changes were written. You can retry.");
  return m_result.anyWriteConfirmed() ? PresentationText::tr("The configuration has been written to the system.") + u'\n' + activationText()
                                      : PresentationText::tr("No configuration changes were needed.");
}

void ApplyPlanViewModel::setInteractionAvailability(InteractionAvailability availability) {
  if (m_interaction == availability) return;
  m_interaction = std::move(availability);
  emit availabilityChanged();
}
void ApplyPlanViewModel::apply() {
  if (!canApply()) return;
  m_canApply = false;
  m_applying = true;
  const QPointer guard(this);
  emit applyingChanged();
  if (!guard) return;
  emit availabilityChanged();
  if (!guard) return;
  app::ConfigurationActions::ApplyOperation operation;
  try {
    operation = m_actions->prepareApply(m_plan);
  } catch (const app::MaintenanceValidationError& error) {
    app::ConfigurationApplyResult result;
    app::ConfigurationStepResult failure{{}, app::ConfigurationStepOutcome::PreparationFailed};
    failure.validationIssue = error.issue();
    result.steps.push_back(std::move(failure));
    app::logConfigurationException("prepare", std::current_exception(), result);
    publishResult(std::move(result));
    return;
  } catch (const std::exception& error) {
    app::ConfigurationApplyResult result;
    result.steps.push_back({error.what(), app::ConfigurationStepOutcome::PreparationFailed});
    app::logConfigurationException("prepare", std::current_exception(), result);
    publishResult(std::move(result));
    return;
  } catch (...) {
    app::ConfigurationApplyResult result;
    result.steps.push_back({PresentationText::tr(": unknown error").toStdString(), app::ConfigurationStepOutcome::PreparationFailed});
    app::logConfigurationException("prepare", std::current_exception(), result);
    publishResult(std::move(result));
    return;
  }
  m_apply.start([operation = std::move(operation)](std::stop_token) { return operation(); },
                [this](AsyncResult<app::ConfigurationApplyResult> result) {
                  if (auto* value = std::get_if<app::ConfigurationApplyResult>(&result)) {
                    publishResult(std::move(*value));
                  } else {
                    app::ConfigurationApplyResult failure;
                    app::ConfigurationStepResult receipt{{}, app::ConfigurationStepOutcome::ExecutionFailed, app::WriteEffect::Uncertain};
                    try {
                      std::rethrow_exception(std::get<std::exception_ptr>(result));
                    } catch (const app::MaintenanceValidationError& error) {
                      receipt.outcome = app::ConfigurationStepOutcome::AdmissionFailed;
                      receipt.writeEffect = app::WriteEffect::None;
                      receipt.validationIssue = error.issue();
                    } catch (const app::OperationError& error) {
                      receipt.writeEffect = error.writeEffect();
                      receipt.description = error.what();
                    } catch (const std::exception& error) {
                      receipt.description = error.what();
                    } catch (...) {
                      receipt.description = PresentationText::tr(": unknown error").toStdString();
                    }
                    failure.steps.push_back(std::move(receipt));
                    app::logConfigurationException("worker", std::get<std::exception_ptr>(result), failure);
                    publishResult(std::move(failure));
                  }
                });
}
void ApplyPlanViewModel::publishResult(app::ConfigurationApplyResult result) {
  const QPointer guard(this);
  app::completeConfigurationResult(result, m_plan);
  m_actions->completeApply(result);
  if (!guard) return;
  m_result = std::move(result);
  m_canApply = m_validPlan && !m_result.anyWriteConfirmed() && !m_result.reconciliationRequired();
  // Install the reconciliation gate before releasing the write gate.
  if (m_result.reconciliationRequired()) emit reconciliationRequired();
  if (!guard) return;
  m_applying = false;
  emit resultChanged();
  if (!guard) return;
  emit applyingChanged();
  if (!guard) return;
  emit availabilityChanged();
}

QString ApplyPlanViewModel::exportFile(const QString& path, const ScriptScope scope) const {
  if (!canExport(scope)) return PresentationText::tr("Export failed");
  return exportTextFile(path, QString::fromStdString(app::renderConfigurationScript(script(scope))), m_textFileWriter);
}
}  // namespace uwf::presentation

#include "moc_ApplyPlanViewModel.cpp"
