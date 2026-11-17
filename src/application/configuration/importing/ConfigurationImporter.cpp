// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/importing/ConfigurationImporter.h"

#include <array>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/configuration/exclusions/ExclusionEditor.h"
#include "application/configuration/exclusions/ExclusionRules.h"
#include "application/configuration/overlay/OverlayPolicyEditor.h"
#include "application/localization/ApplicationText.h"
#include "application/localization/OverlayTypeText.h"
#include "application/paths/PathPolicy.h"
#include "domain/uwf/contracts/Types.h"

namespace uwf::app {

namespace {

enum class ImportField : std::size_t {
  Filter,
  OverlayType,
  MaximumSize,
  WarningThreshold,
  CriticalThreshold,
  Passthrough,
  Persistent,
  ReadOnlyMedia,
  Reset,
  Count
};
using ImportValue = std::variant<bool, core::OverlayType, uint32_t>;
struct ImportFieldState {
  std::optional<ImportValue> value;
  // An option may retain its value through a structural change only because
  // its explicit intent supplies a write after that change.
  bool operationPending = false;
};
using ImportValues = std::array<ImportFieldState, static_cast<std::size_t>(ImportField::Count)>;

struct ImportCommandEffect {
  qsizetype reportIndex;
  ImportField field;
  ImportValue requested;
  std::optional<ImportValue> before;
  bool changed;
};

std::optional<ImportField> commandField(const UwfmgrKind kind) {
  switch (kind) {
    case UwfmgrKind::FilterEnable:
    case UwfmgrKind::FilterDisable:
      return ImportField::Filter;
    case UwfmgrKind::OverlaySetType:
      return ImportField::OverlayType;
    case UwfmgrKind::OverlaySetSize:
      return ImportField::MaximumSize;
    case UwfmgrKind::OverlaySetWarningThreshold:
      return ImportField::WarningThreshold;
    case UwfmgrKind::OverlaySetCriticalThreshold:
      return ImportField::CriticalThreshold;
    case UwfmgrKind::OverlaySetPassthrough:
      return ImportField::Passthrough;
    case UwfmgrKind::OverlaySetPersistent:
      return ImportField::Persistent;
    case UwfmgrKind::OverlaySetReadOnlyMedia:
      return ImportField::ReadOnlyMedia;
    case UwfmgrKind::OverlayResetPersistentState:
      return ImportField::Reset;
    default:
      return std::nullopt;
  }
}

std::optional<OverlayPolicyField> policyField(const ImportField field) {
  switch (field) {
    case ImportField::Passthrough:
      return OverlayPolicyField::Passthrough;
    case ImportField::Persistent:
      return OverlayPolicyField::Persistent;
    case ImportField::ReadOnlyMedia:
      return OverlayPolicyField::ReadOnlyMedia;
    case ImportField::Reset:
      return OverlayPolicyField::Reset;
    default:
      return std::nullopt;
  }
}

ImportValues configurationValues(const GlobalConfigurationEditing* global, const OverlayPolicyEditor* policy) {
  ImportValues result;
  if (global) {
    const auto overlay = global->overlayConfiguration();
    result[static_cast<std::size_t>(ImportField::Filter)].value = global->filterEnabled();
    result[static_cast<std::size_t>(ImportField::OverlayType)].value = overlay.type;
    result[static_cast<std::size_t>(ImportField::MaximumSize)].value = overlay.maximumSizeMb;
    result[static_cast<std::size_t>(ImportField::WarningThreshold)].value = overlay.warningThresholdMb;
    result[static_cast<std::size_t>(ImportField::CriticalThreshold)].value = overlay.criticalThresholdMb;
  }
  if (policy) {
    const auto edit = policy->beginEdit();
    const auto& observedType = policy->baseline().next.overlayType;
    const bool typeObserved = observedType.status == PolicyObservationStatus::Available && observedType.value.has_value();
    const auto targetType = policy->targetType(edit);
    // An uninitialized policy editor must not replace a known global value.
    // A shared storage selection supplies its own Disk target without a getter.
    if ((typeObserved || edit.maintenance.storageVolume) && targetType != core::OverlayType::Unknown)
      result[static_cast<std::size_t>(ImportField::OverlayType)].value = targetType;
    const auto& values = edit.values;
    const auto changes = policy->pendingChanges();
    for (const auto field : {ImportField::Passthrough, ImportField::Persistent, ImportField::ReadOnlyMedia, ImportField::Reset}) {
      auto& state = result[static_cast<std::size_t>(field)];
      const auto option = *policyField(field);
      if (const auto value = OverlayPolicyEditor::value(values, option)) state.value = *value;
      state.operationPending = OverlayPolicyEditor::value(changes, option).has_value();
    }
  }
  return result;
}

std::optional<ImportValue> requestedValue(const ImportField field, const UwfmgrKind kind, const QString& argument) {
  if (field == ImportField::Filter) return kind == UwfmgrKind::FilterEnable;
  if (field == ImportField::OverlayType) return argument == QStringLiteral("Disk") ? core::OverlayType::Disk : core::OverlayType::RAM;
  bool valid = false;
  const auto number = argument.toUInt(&valid);
  return valid ? std::optional<ImportValue>{number} : std::nullopt;
}

GlobalEditResult applyGlobalValue(GlobalConfigurationEditing& editor, const ImportField field, const ImportValue& value) {
  switch (field) {
    case ImportField::Filter:
      return editor.importFilterEnabled(std::get<bool>(value));
    case ImportField::OverlayType:
      return editor.importOverlayType(std::get<core::OverlayType>(value));
    case ImportField::MaximumSize:
      return editor.importOverlayMaxMb(std::get<uint32_t>(value));
    case ImportField::WarningThreshold:
      return editor.importOverlayWarnMb(std::get<uint32_t>(value));
    case ImportField::CriticalThreshold:
      return editor.importOverlayCritMb(std::get<uint32_t>(value));
    default:
      break;
  }
  throw std::logic_error("invalid global import field");
}

QString fieldName(const ImportField field) {
  switch (field) {
    case ImportField::Filter:
      return ApplicationText::tr("Write protection");
    case ImportField::OverlayType:
      return ApplicationText::tr("Overlay type");
    case ImportField::MaximumSize:
      return ApplicationText::tr("maximum capacity");
    case ImportField::WarningThreshold:
      return ApplicationText::tr("warning threshold");
    case ImportField::CriticalThreshold:
      return ApplicationText::tr("critical threshold");
    case ImportField::Passthrough:
      return ApplicationText::tr("Freespace Passthrough");
    case ImportField::Persistent:
      return ApplicationText::tr("Persistent Overlay");
    case ImportField::ReadOnlyMedia:
      return ApplicationText::tr("Read-Only Media mode");
    case ImportField::Reset:
      return ApplicationText::tr("Clear persistent data on next startup");
    case ImportField::Count:
      break;
  }
  throw std::logic_error("invalid global import field");
}

QString valueText(const ImportValue& value) {
  if (const auto* enabled = std::get_if<bool>(&value)) return *enabled ? ApplicationText::tr("Enabled") : ApplicationText::tr("Disabled");
  if (const auto* type = std::get_if<core::OverlayType>(&value)) return overlayTypeText(*type);
  return QString::number(std::get<uint32_t>(value)) + QStringLiteral(" MB");
}

void describeEffect(ImportResult& report, const ImportCommandEffect& effect, const ImportFieldState& initial, const ImportFieldState& finalState,
                    const bool latest) {
  const auto& actual = *finalState.value;
  const auto name = fieldName(effect.field);
  const auto requested = valueText(effect.requested);
  const auto final = valueText(actual);
  if (!latest) {
    // Report overwritten commands without presenting their requested value as pending.
    report.status = effect.changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
    report.detail = ApplicationText::tr("Requested %1 → %2; replaced by a later command. Final draft value → %3").arg(name, requested, final);
    return;
  }

  const bool changed = effect.changed && actual != effect.before && (actual != initial.value || finalState.operationPending != initial.operationPending);
  report.status = changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
  if (actual != effect.requested) {
    report.detail = ApplicationText::tr("Final draft %1 → %2 (requested %3)").arg(name, final, requested);
  } else {
    report.detail = ApplicationText::tr("Final draft %1 → %2").arg(name, final);
  }
  if (actual == initial.value && finalState.operationPending == initial.operationPending)
    report.detail += QStringLiteral(" — ") + ApplicationText::tr("No net change for this setting");
}

std::shared_ptr<VolumeConfigurationEditing> findVolumeEditor(const VolumeConfigurationEditors& editors, const QString& target) {
  if (target.isEmpty()) return {};
  const auto identity = compat::parseVolumeId(target.toStdString());
  // A GUID selects the stored Next configuration directly. Physical lookup is
  // used only for a known volume that does not have a Next registration yet.
  if (identity) {
    for (const auto& editor : editors) {
      auto candidate = editor.lock();
      if (candidate && compat::parseVolumeId(candidate->configurationVolumeId().toStdString()) == identity) return candidate;
    }
    for (const auto& editor : editors) {
      auto candidate = editor.lock();
      if (candidate && candidate->configurationVolumeId().isEmpty() && compat::parseVolumeId(candidate->volumeIdentity().toStdString()) == identity)
        return candidate;
    }
    return {};
  }
  for (const auto& editor : editors) {
    auto candidate = editor.lock();
    if (candidate && candidate->driveLetter().compare(target, Qt::CaseInsensitive) == 0) return candidate;
  }
  return {};
}

ImportResult outcomeToRow(const app::UwfmgrCommand& c, ExclusionImportOutcome o, const QString& kindLabel) {
  ImportResult r;
  r.lineNo = c.sourceLineNo;
  r.lineText = QString::fromStdString(c.rawLine).trimmed();
  switch (o) {
    case ExclusionImportOutcome::Applied:
      r.status = ImportResult::Status::Success;
      r.detail = ApplicationText::tr("Updated the %1 draft").arg(kindLabel);
      break;
    case ExclusionImportOutcome::NoOp:
      r.status = ImportResult::Status::Duplicate;
      r.detail = ApplicationText::tr("Already in the target state — no-op");
      break;
    case ExclusionImportOutcome::RejectedNotOnVolume:
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)");
      break;
    case ExclusionImportOutcome::RejectedForbidden:
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("This path cannot be added as an exclusion.");
      break;
    case ExclusionImportOutcome::RejectedConflict:
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("The path overlaps an existing file staging entry");
      break;
    case ExclusionImportOutcome::RejectedMissingVolume:
      r.status = ImportResult::Status::Failed;
      r.detail = QString::fromStdString(fileExclusionTargetRequirement());
      break;
    case ExclusionImportOutcome::RejectedRequired:
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("The registry exclusion is required while File staging contains saved entries");
      break;
    case ExclusionImportOutcome::RejectedReadOnly:
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("Read-only");
      break;
  }
  return r;
}

std::optional<QString> maintenanceCommandLabel(const UwfmgrKind kind) {
  switch (kind) {
    case UwfmgrKind::FilterEnableHorm:
    case UwfmgrKind::FilterDisableHorm:
      return ApplicationText::tr("Hibernate once / resume many") + QStringLiteral(" (HORM)");
    case UwfmgrKind::OverlayCommit:
      return ApplicationText::tr("Full volume commit");
    case UwfmgrKind::VolumeCreateSwapfile:
      return ApplicationText::tr("Configure storage location");
    default:
      return std::nullopt;
  }
}

}  // namespace

QList<ImportResult> applyImportCommands(const QList<app::UwfmgrCommand>& cmds, GlobalConfigurationEditing* global,
                                        const VolumeConfigurationEditors& volumeEditors, ExclusionEditor* registry, const DriveResolver& resolveDrive,
                                        OverlayPolicyEditor* policy, const AllVolumeProtectionImporter& stageAll,
                                        const MaintenanceCommandImporter& stageMaintenance) {
  QList<ImportResult> out;
  out.reserve(cmds.size());
  QList<ImportCommandEffect> effects;
  const auto initial = configurationValues(global, policy);
  bool previousAccepted = false;
  const auto appendResult = [&](ImportResult result) {
    previousAccepted = result.status == ImportResult::Status::Success || result.status == ImportResult::Status::Duplicate;
    out.append(std::move(result));
  };

  // Apply commands in order; reverse operations must not be deduplicated across the batch.
  for (const auto& c : cmds) {
    ImportResult r;
    r.lineNo = c.sourceLineNo;
    r.lineText = QString::fromStdString(c.rawLine).trimmed();

    if (c.requiresPrevious && !previousAccepted) {
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("Skipped because a prerequisite operation failed or could not be confirmed.");
      appendResult(std::move(r));
      continue;
    }

    if (c.parseError != app::ParseError::None && c.parseError != app::ParseError::Comment) {
      r.status = c.parseError == app::ParseError::Unsupported ? ImportResult::Status::Unsupported : ImportResult::Status::Failed;
      r.detail = parseErrorMessage(c.parseError, QString::fromStdString(c.parseErrorContext));
      appendResult(std::move(r));
      continue;
    }

    // The ordinary configuration lock includes HORM itself. Maintenance uses
    // the session callback's admission and its existing capability-aware editor
    // so disabling HORM can be staged without unlocking other configuration.
    if (const auto label = maintenanceCommandLabel(c.kind)) {
      if (stageMaintenance) {
        try {
          const auto outcome = stageMaintenance(c);
          if (outcome == ExclusionImportOutcome::RejectedMissingVolume) {
            const auto volume = c.args.empty() ? QString{} : QString::fromStdString(c.args.front());
            r.status = ImportResult::Status::Failed;
            r.detail = ApplicationText::tr("Unknown volume %1 (no matching volume configuration)").arg(volume);
          } else {
            r = outcomeToRow(c, outcome, *label);
          }
        } catch (const std::exception& error) {
          r.status = ImportResult::Status::Failed;
          r.detail = QString::fromUtf8(error.what());
        }
      } else {
        r.status = ImportResult::Status::Unsupported;
        r.detail = parseErrorMessage(ParseError::Unsupported, {});
      }
      appendResult(std::move(r));
      continue;
    }

    const auto field = commandField(c.kind);
    const auto policyTarget = field ? policyField(*field) : std::nullopt;
    if (field && !policyTarget && (!global || !global->available())) {
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("Global configuration is unavailable; the command was not queued.");
      appendResult(std::move(r));
      continue;
    }

    // Check session admission even when the destination subeditor is independently writable.
    if (global && !global->editingAllowed()) {
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("Read-only");
      appendResult(std::move(r));
      continue;
    }

    const QString a0 = c.args.empty() ? QString{} : QString::fromStdString(c.args[0]);

    if (policyTarget) {
      const auto& baseline = policy ? policy->baseline() : OverlayPolicySnapshot{};
      const bool readable = *policyTarget == OverlayPolicyField::Reset ? baseline.resetPersistentState.status == PolicyObservationStatus::Available
                                                                       : baseline.next.flags.status == PolicyObservationStatus::Available;
      if (!policy || !readable || c.args.size() != 1 || (a0 != QStringLiteral("on") && a0 != QStringLiteral("off"))) {
        r.status = ImportResult::Status::Failed;
        r.detail = ApplicationText::tr("The overlay configuration could not be read. Refresh before editing.");
      } else if (!policy->editingAllowed()) {
        r.status = ImportResult::Status::Failed;
        r.detail = ApplicationText::tr("Read-only");
      } else {
        OverlayPolicyDelta delta;
        const bool value = a0 == QStringLiteral("on");
        OverlayPolicyEditor::setValue(delta, *policyTarget, value);
        const auto before = configurationValues(nullptr, policy);
        const auto pendingBefore = policy->pendingChanges();
        if (!policy->importChanges(delta)) {
          r.status = ImportResult::Status::Failed;
          r.detail = ApplicationText::tr("The overlay configuration could not be read. Refresh before editing.");
          appendResult(std::move(r));
          continue;
        }
        const bool changed = pendingBefore != policy->pendingChanges();
        effects.append({out.size(), *field, value, before[static_cast<std::size_t>(*field)].value, changed});
        r.status = changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
      }
      appendResult(std::move(r));
      continue;
    }

    if (field) {
      const auto requested = requestedValue(*field, c.kind, a0);
      if (!requested) {
        r.status = ImportResult::Status::Failed;
        r.detail = ApplicationText::tr("Invalid size value: %1").arg(a0);
        appendResult(std::move(r));
        continue;
      }
      const auto before = configurationValues(global, nullptr);
      const auto result = applyGlobalValue(*global, *field, *requested);
      if (result.outcome == GlobalEditOutcome::Rejected) {
        r.status = ImportResult::Status::Failed;
        r.detail = result.reason;
        appendResult(std::move(r));
        continue;
      }
      const bool changed = result.outcome == GlobalEditOutcome::Applied;
      effects.append({out.size(), *field, *requested, before[static_cast<std::size_t>(*field)].value, changed});
      r.status = changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
      appendResult(std::move(r));
      continue;
    }

    switch (c.kind) {
      case app::UwfmgrKind::VolumeProtectAll:
        if (stageAll) {
          try {
            r = outcomeToRow(c, stageAll(true), ApplicationText::tr("Volume protection"));
          } catch (const std::exception& error) {
            r.status = ImportResult::Status::Failed;
            r.detail = QString::fromUtf8(error.what());
          }
        } else {
          r.status = ImportResult::Status::Unsupported;
          r.detail = parseErrorMessage(ParseError::Unsupported, {});
        }
        break;
      case app::UwfmgrKind::VolumeProtect:
      case app::UwfmgrKind::VolumeUnprotect: {
        auto editor = findVolumeEditor(volumeEditors, a0);
        if (!editor) {
          r.status = ImportResult::Status::Failed;
          r.detail = ApplicationText::tr("Unknown volume %1 (no matching volume configuration)").arg(a0);
          break;
        }
        if (!editor->protectionEditingAllowed()) {
          r.status = ImportResult::Status::Failed;
          r.detail = ApplicationText::tr("Read-only");
          break;
        }
        const bool want = c.kind == app::UwfmgrKind::VolumeProtect;
        const bool changed = editor->importProtect(want, a0);
        r.status = changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
        r.detail = changed ? ApplicationText::tr("Updated volume %1 protection draft → %2").arg(a0, valueText(ImportValue{want}))
                           : ApplicationText::tr("Volume %1 is already in the target protection state").arg(a0);
        break;
      }
      case app::UwfmgrKind::FileAddExclusion:
      case app::UwfmgrKind::FileRemoveExclusion: {
        if (c.kind == app::UwfmgrKind::FileAddExclusion && a0.contains(u'/')) {
          r.status = ImportResult::Status::Failed;
          r.detail = FileExclusionRules::rejectionReason(a0, {}, {});
          break;
        }
        const QString native = paths::windowsSeparators(a0);
        const auto target = paths::splitVolumeFilePath(a0);
        std::shared_ptr<VolumeConfigurationEditing> editor;
        QString pattern = a0;
        if (target && compat::parseVolumeId(target->volume)) {
          // The explicit GUID selects the configuration in memory. Only its
          // address changes for the editor; the original pattern stays literal.
          const auto identity = QString::fromStdString(target->volume);
          editor = findVolumeEditor(volumeEditors, identity);
          if (!editor) {
            r.status = ImportResult::Status::Failed;
            r.detail = ApplicationText::tr("Unknown volume %1 (no matching volume configuration)").arg(identity);
            break;
          }
          pattern = editor->driveLetter() + target->relativePath;
        } else {
          QString dl;
          try {
            dl = QString::fromStdString(resolveDrive(native.toStdString()));
          } catch (const std::exception& error) {
            r.status = ImportResult::Status::Failed;
            r.detail = ApplicationText::tr("Failed to resolve the volume for path %1: %2").arg(native, QString::fromUtf8(error.what()));
            break;
          }
          if (dl.isEmpty()) {
            r.status = ImportResult::Status::Failed;
            r.detail = ApplicationText::tr("Path %1 has no drive letter; its volume tab cannot be identified.").arg(native);
            break;
          }
          editor = findVolumeEditor(volumeEditors, dl);
          if (!editor) {
            r.status = ImportResult::Status::Failed;
            r.detail = ApplicationText::tr("No UWF-eligible disk for drive letter %1").arg(dl);
            break;
          }
          if (target) pattern = QString::fromStdString(target->volume) + target->relativePath;
        }
        // Path resolution may revoke admission; recheck before editing the resolved target.
        if (global && !global->editingAllowed()) {
          r.status = ImportResult::Status::Failed;
          r.detail = ApplicationText::tr("Read-only");
          break;
        }
        const auto outcome = c.kind == app::UwfmgrKind::FileAddExclusion ? editor->importAddFileExclusion(pattern) : editor->importRemoveFileExclusion(pattern);
        r = outcomeToRow(c, outcome, ApplicationText::tr("file exclusion"));
        break;
      }
      case app::UwfmgrKind::RegistryAddExclusion:
      case app::UwfmgrKind::RegistryRemoveExclusion: {
        if (registry) {
          const auto outcome = c.kind == app::UwfmgrKind::RegistryAddExclusion ? registry->importAdd(a0) : registry->importRemove(a0);
          r = outcomeToRow(c, outcome, ApplicationText::tr("registry exclusion"));
        } else {
          r.status = ImportResult::Status::Failed;
          r.detail = ApplicationText::tr("Registry exclusions are only available on the registry tab, which is not present");
        }
        break;
      }
      case app::UwfmgrKind::Unknown:
      default:
        r.status = ImportResult::Status::Unsupported;
        r.detail = ApplicationText::tr("Unsupported command");
        break;
    }
    appendResult(std::move(r));
  }

  // Normalize once after the batch so intermediate thresholds do not clamp later targets.
  if (global) global->finishImport();
  // Type/storage side effects can change the meaning of an earlier option
  // command. Settle every field against the complete draft after normalization.
  const auto final = configurationValues(global, policy);
  std::array<bool, static_cast<std::size_t>(ImportField::Count)> described{};
  for (auto effect = effects.crbegin(); effect != effects.crend(); ++effect) {
    auto& report = out[effect->reportIndex];
    if (!policyField(effect->field) && global && !global->available()) {
      report.status = ImportResult::Status::Failed;
      report.detail = ApplicationText::tr("Global configuration is unavailable; the command was not queued.");
      continue;
    }
    const auto index = static_cast<std::size_t>(effect->field);
    if (!final[index].value) {
      report.status = ImportResult::Status::Failed;
      report.detail = ApplicationText::tr("The overlay configuration could not be read. Refresh before editing.");
      continue;
    }
    const bool latest = !std::exchange(described[index], true);
    describeEffect(report, *effect, initial[index], final[index], latest);
  }
  return out;
}

}  // namespace uwf::app
