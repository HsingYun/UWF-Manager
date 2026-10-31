// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ConfigurationImporter.h"

#include <QDir>
#include <array>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>

#include "ExclusionEditor.h"
#include "app/common/ApplicationText.h"

namespace uwf::app {

namespace {

enum class GlobalField : std::size_t { Filter, OverlayType, MaximumSize, WarningThreshold, CriticalThreshold, Count };
using GlobalValue = std::variant<bool, core::OverlayType, uint32_t>;
using GlobalValues = std::array<GlobalValue, static_cast<std::size_t>(GlobalField::Count)>;

struct GlobalCommandEffect {
  qsizetype reportIndex;
  GlobalField field;
  GlobalValue requested;
  GlobalValue before;
  bool changed;
};

std::optional<GlobalField> globalField(const UwfmgrKind kind) {
  switch (kind) {
    case UwfmgrKind::FilterEnable:
    case UwfmgrKind::FilterDisable:
      return GlobalField::Filter;
    case UwfmgrKind::OverlaySetType:
      return GlobalField::OverlayType;
    case UwfmgrKind::OverlaySetSize:
      return GlobalField::MaximumSize;
    case UwfmgrKind::OverlaySetWarningThreshold:
      return GlobalField::WarningThreshold;
    case UwfmgrKind::OverlaySetCriticalThreshold:
      return GlobalField::CriticalThreshold;
    default:
      return std::nullopt;
  }
}

GlobalValues globalValues(const GlobalConfigurationEditing& editor) {
  const auto overlay = editor.overlayConfiguration();
  return {editor.filterEnabled(), overlay.type, overlay.maximumSizeMb, overlay.warningThresholdMb, overlay.criticalThresholdMb};
}

std::optional<GlobalValue> requestedValue(const GlobalField field, const UwfmgrKind kind, const QString& argument) {
  if (field == GlobalField::Filter) return kind == UwfmgrKind::FilterEnable;
  if (field == GlobalField::OverlayType) return argument == QStringLiteral("Disk") ? core::OverlayType::Disk : core::OverlayType::RAM;
  bool valid = false;
  const auto number = argument.toUInt(&valid);
  return valid ? std::optional<GlobalValue>{number} : std::nullopt;
}

bool applyGlobalValue(GlobalConfigurationEditing& editor, const GlobalField field, const GlobalValue& value) {
  switch (field) {
    case GlobalField::Filter:
      return editor.importFilterEnabled(std::get<bool>(value));
    case GlobalField::OverlayType:
      return editor.importOverlayType(std::get<core::OverlayType>(value));
    case GlobalField::MaximumSize:
      return editor.importOverlayMaxMb(std::get<uint32_t>(value));
    case GlobalField::WarningThreshold:
      return editor.importOverlayWarnMb(std::get<uint32_t>(value));
    case GlobalField::CriticalThreshold:
      return editor.importOverlayCritMb(std::get<uint32_t>(value));
    case GlobalField::Count:
      break;
  }
  throw std::logic_error("invalid global import field");
}

QString fieldName(const GlobalField field) {
  switch (field) {
    case GlobalField::Filter:
      return ApplicationText::tr("Write protection");
    case GlobalField::OverlayType:
      return ApplicationText::tr("Overlay type");
    case GlobalField::MaximumSize:
      return ApplicationText::tr("maximum size");
    case GlobalField::WarningThreshold:
      return ApplicationText::tr("warning threshold");
    case GlobalField::CriticalThreshold:
      return ApplicationText::tr("critical threshold");
    case GlobalField::Count:
      break;
  }
  throw std::logic_error("invalid global import field");
}

QString valueText(const GlobalValue& value) {
  if (const auto* enabled = std::get_if<bool>(&value)) return *enabled ? ApplicationText::tr("Enabled") : ApplicationText::tr("Disabled");
  if (const auto* type = std::get_if<core::OverlayType>(&value)) return *type == core::OverlayType::Disk ? QStringLiteral("Disk") : QStringLiteral("RAM");
  return QString::number(std::get<uint32_t>(value)) + QStringLiteral(" MB");
}

void describeGlobalEffect(ImportResult& report, const GlobalCommandEffect& effect, const GlobalValue& initial, const GlobalValue& actual,
                          const bool latest) {
  const auto name = fieldName(effect.field);
  const auto requested = valueText(effect.requested);
  const auto final = valueText(actual);
  if (!latest) {
    // Keep the execution result, but never describe an overwritten intent as
    // the field's pending value. This rule is shared by every global scalar.
    report.status = effect.changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
    report.detail = ApplicationText::tr("Requested %1 → %2; replaced by a later command. Final draft value → %3").arg(name, requested, final);
    return;
  }

  const bool changed = effect.changed && actual != effect.before && actual != initial;
  report.status = changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
  if (actual != effect.requested) {
    report.detail = ApplicationText::tr("Final draft %1 → %2 (requested %3)").arg(name, final, requested);
  } else {
    report.detail = ApplicationText::tr("Final draft %1 → %2").arg(name, final);
  }
  if (actual == initial) report.detail += QStringLiteral(" — ") + ApplicationText::tr("No net change for this setting");
}

std::shared_ptr<VolumeConfigurationEditing> findVolumeEditor(const VolumeConfigurationEditors& editors, const QString& driveLetter) {
  for (const auto& editor : editors) {
    auto candidate = editor.lock();
    if (candidate && candidate->driveLetter().compare(driveLetter, Qt::CaseInsensitive) == 0) return candidate;
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
      r.detail = ApplicationText::tr("Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)");
      break;
    case ExclusionImportOutcome::RejectedConflict:
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("The path overlaps an existing file staging entry");
      break;
    case ExclusionImportOutcome::RejectedRequired:
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("The registry exclusion is required while File staging contains saved entries");
      break;
  }
  return r;
}

}  // namespace

QList<ImportResult> applyImportCommands(const QList<app::UwfmgrCommand>& cmds, GlobalConfigurationEditing* global,
                                        const VolumeConfigurationEditors& volumeEditors, ExclusionEditor* registry,
                                        const DriveResolver& resolveDrive) {
  QList<ImportResult> out;
  out.reserve(cmds.size());
  QList<GlobalCommandEffect> effects;
  std::optional<GlobalValues> initial;
  bool previousAccepted = false;
  const auto appendResult = [&](ImportResult result) {
    previousAccepted = result.status == ImportResult::Status::Success || result.status == ImportResult::Status::Duplicate;
    out.append(std::move(result));
  };

  // 按输入顺序更新目标；幂等判断由各入口完成，不跨反向操作去重。
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

    // 保留解析错误分类，并生成本地化报告。
    if (c.parseError != app::ParseError::None && c.parseError != app::ParseError::Comment) {
      r.status = c.parseError == app::ParseError::Unsupported ? ImportResult::Status::Unsupported : ImportResult::Status::Failed;
      r.detail = parseErrorMessage(c.parseError, QString::fromStdString(c.parseErrorContext));
      appendResult(std::move(r));
      continue;
    }

    const auto field = globalField(c.kind);
    if (field && (!global || !global->available())) {
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("Global configuration is unavailable; the command was not queued.");
      appendResult(std::move(r));
      continue;
    }

    const QString a0 = c.args.empty() ? QString{} : QString::fromStdString(c.args[0]);

    if (field) {
      const auto requested = requestedValue(*field, c.kind, a0);
      if (!requested) {
        r.status = ImportResult::Status::Failed;
        r.detail = ApplicationText::tr("Invalid size value: %1").arg(a0);
        appendResult(std::move(r));
        continue;
      }
      const auto before = globalValues(*global);
      if (!initial) initial = before;
      const bool changed = applyGlobalValue(*global, *field, *requested);
      effects.append({out.size(), *field, *requested, before[static_cast<std::size_t>(*field)], changed});
      r.status = changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
      appendResult(std::move(r));
      continue;
    }

    switch (c.kind) {
      case app::UwfmgrKind::VolumeProtect:
      case app::UwfmgrKind::VolumeUnprotect: {
        auto editor = findVolumeEditor(volumeEditors, a0);
        if (!editor) {
          r.status = ImportResult::Status::Failed;
          r.detail = ApplicationText::tr("Unknown volume %1 (no UWF-eligible disk with that drive letter)").arg(a0);
          break;
        }
        const bool want = c.kind == app::UwfmgrKind::VolumeProtect;
        const bool changed = editor->importProtect(want);
        r.status = changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
        r.detail = changed
                       ? ApplicationText::tr("Updated volume %1 protection draft → %2").arg(a0, valueText(GlobalValue{want}))
                       : ApplicationText::tr("Volume %1 is already in the target protection state").arg(a0);
        break;
      }
      case app::UwfmgrKind::FileAddExclusion:
      case app::UwfmgrKind::FileRemoveExclusion: {
        const QString native = QDir::toNativeSeparators(a0);
        // 文件命令按盘符路由至对应磁盘页。
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
          r.detail = ApplicationText::tr("Path %1 has no drive letter; cannot route to a volume tab").arg(native);
          break;
        }
        auto editor = findVolumeEditor(volumeEditors, dl);
        if (!editor) {
          r.status = ImportResult::Status::Failed;
          r.detail = ApplicationText::tr("No UWF-eligible disk for drive letter %1").arg(dl);
          break;
        }
        const auto outcome = c.kind == app::UwfmgrKind::FileAddExclusion ? editor->importAddFileExclusion(native) : editor->importRemoveFileExclusion(native);
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

  // 整批结束后统一恢复阈值约束，避免中间状态截断后续目标值。
  if (global) {
    global->finishImport();
    const auto final = globalValues(*global);
    std::array<bool, static_cast<std::size_t>(GlobalField::Count)> described{};
    for (auto effect = effects.crbegin(); effect != effects.crend(); ++effect) {
      auto& report = out[effect->reportIndex];
      if (!global->available()) {
        report.status = ImportResult::Status::Failed;
        report.detail = ApplicationText::tr("Global configuration is unavailable; the command was not queued.");
        continue;
      }
      const auto index = static_cast<std::size_t>(effect->field);
      const bool latest = !std::exchange(described[index], true);
      describeGlobalEffect(report, *effect, (*initial)[index], final[index], latest);
    }
  }
  return out;
}

}  // namespace uwf::app
