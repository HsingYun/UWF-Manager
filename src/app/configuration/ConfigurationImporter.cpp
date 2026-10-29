// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ConfigurationImporter.h"

#include <QDir>
#include <exception>

#include "ExclusionEditor.h"
#include "app/common/ApplicationText.h"
#include "app/ports/VolumePaths.h"

namespace uwf::app {

namespace {

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
      r.detail = ApplicationText::tr("Queued as a pending %1 change").arg(kindLabel);
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
                                        const VolumeConfigurationEditors& volumeEditors, ExclusionEditor* registry) {
  QList<ImportResult> out;
  out.reserve(cmds.size());

  // 按输入顺序更新目标；幂等判断由各入口完成，不跨反向操作去重。
  for (const auto& c : cmds) {
    ImportResult r;
    r.lineNo = c.sourceLineNo;
    r.lineText = QString::fromStdString(c.rawLine).trimmed();

    if (c.requiresPrevious && (out.isEmpty() || (out.back().status != ImportResult::Status::Success && out.back().status != ImportResult::Status::Duplicate))) {
      r.status = ImportResult::Status::Failed;
      r.detail = ApplicationText::tr("Skipped because a prerequisite operation failed or could not be confirmed.");
      out.append(r);
      continue;
    }

    // 保留解析错误分类，并生成本地化报告。
    if (c.parseError != app::ParseError::None && c.parseError != app::ParseError::Comment) {
      r.status = c.parseError == app::ParseError::Unsupported ? ImportResult::Status::Unsupported : ImportResult::Status::Failed;
      r.detail = parseErrorMessage(c.parseError, QString::fromStdString(c.parseErrorContext));
      out.append(r);
      continue;
    }

    const QString a0 = c.args.empty() ? QString{} : QString::fromStdString(c.args[0]);

    switch (c.kind) {
      case app::UwfmgrKind::FilterEnable:
      case app::UwfmgrKind::FilterDisable: {
        const bool want = c.kind == app::UwfmgrKind::FilterEnable;
        const bool changed = global ? global->importFilterEnabled(want) : false;
        r.status = changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
        r.detail = changed ? ApplicationText::tr("Pending filter %1").arg(want ? ApplicationText::tr("enable") : ApplicationText::tr("disable"))
                           : ApplicationText::tr("Filter is already in the target state");
        break;
      }
      case app::UwfmgrKind::OverlaySetType: {
        const auto t = a0 == QStringLiteral("Disk") ? core::OverlayType::Disk : core::OverlayType::RAM;
        const bool changed = global ? global->importOverlayType(t) : false;
        r.status = changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
        r.detail = changed ? ApplicationText::tr("Pending overlay type → %1").arg(a0) : ApplicationText::tr("Overlay type already %1").arg(a0);
        break;
      }
      case app::UwfmgrKind::OverlaySetSize:
      case app::UwfmgrKind::OverlaySetWarningThreshold:
      case app::UwfmgrKind::OverlaySetCriticalThreshold: {
        bool ok = false;
        const auto mb = a0.toUInt(&ok);
        if (!ok) {
          r.status = ImportResult::Status::Failed;
          r.detail = ApplicationText::tr("Invalid size value: %1").arg(a0);
          break;
        }
        bool changed = false;
        QString label;
        if (c.kind == app::UwfmgrKind::OverlaySetSize) {
          label = ApplicationText::tr("maximum size");
          changed = global ? global->importOverlayMaxMb(mb) : false;
        } else if (c.kind == app::UwfmgrKind::OverlaySetWarningThreshold) {
          label = ApplicationText::tr("warning threshold");
          changed = global ? global->importOverlayWarnMb(mb) : false;
        } else {
          label = ApplicationText::tr("critical threshold");
          changed = global ? global->importOverlayCritMb(mb) : false;
        }
        r.status = changed ? ImportResult::Status::Success : ImportResult::Status::Duplicate;
        r.detail =
            changed ? ApplicationText::tr("Pending overlay %1 → %2 MB").arg(label).arg(mb) : ApplicationText::tr("Overlay %1 already %2 MB").arg(label).arg(mb);
        break;
      }
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
                       ? ApplicationText::tr("Pending volume %1 protection %2").arg(a0, want ? ApplicationText::tr("enable") : ApplicationText::tr("disable"))
                       : ApplicationText::tr("Volume %1 is already in the target protection state").arg(a0);
        break;
      }
      case app::UwfmgrKind::FileAddExclusion:
      case app::UwfmgrKind::FileRemoveExclusion: {
        const QString native = QDir::toNativeSeparators(a0);
        // 文件命令按盘符路由至对应磁盘页。
        QString dl;
        try {
          dl = QString::fromStdString(drive::fromPath(native.toStdString()));
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
        r.status = ImportResult::Status::Unsupported;
        r.detail = ApplicationText::tr("Unsupported command");
        break;
    }
    out.append(r);
  }

  // 整批结束后统一恢复阈值约束，避免中间状态截断后续目标值。
  if (global) global->finishImport();
  return out;
}

}  // namespace uwf::app
