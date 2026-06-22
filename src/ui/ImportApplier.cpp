/*
* Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ImportApplier.h"

#include <QChar>
#include <QDir>
#include <QSet>
#include <QString>

#include "../core/UwfModel.h"
#include "../util/DriveLetter.h"
#include "../util/Log.h"
#include "DiskTab.h"
#include "ExclusionListWidget.h"
#include "GlobalStatusPanel.h"
#include "I18n.h"
#include "UiUtil.h"

namespace uwf::ui {

namespace {

DiskTab* findTab(const QVector<QPointer<DiskTab>>& tabs, const QString& driveLetter) {
  for (const auto& t : tabs) {
    if (t && t->driveLetter().compare(driveLetter, Qt::CaseInsensitive) == 0) return t.data();
  }
  return nullptr;
}

ImportReportRow outcomeToRow(const api::UwfmgrCommand& c, ExclusionListWidget::ImportOutcome o, const QString& kindLabel) {
  ImportReportRow r;
  r.lineNo = c.sourceLineNo;
  r.lineText = QString::fromStdString(c.rawLine).trimmed();
  switch (o) {
    case ExclusionListWidget::ImportOutcome::Applied:
      r.status = ImportReportRow::Status::Success;
      r.detail = I18n::tr("Queued as a pending %1 change").arg(kindLabel);
      break;
    case ExclusionListWidget::ImportOutcome::NoOp:
      r.status = ImportReportRow::Status::Duplicate;
      r.detail = I18n::tr("Already in the target state — no-op");
      break;
    case ExclusionListWidget::ImportOutcome::RejectedNotOnVolume:
      r.status = ImportReportRow::Status::Failed;
      r.detail = I18n::tr("Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)");
      break;
    case ExclusionListWidget::ImportOutcome::RejectedForbidden:
      r.status = ImportReportRow::Status::Failed;
      r.detail = I18n::tr("Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)");
      break;
  }
  return r;
}

}  // namespace

QList<ImportReportRow> applyImportCommands(const QList<api::UwfmgrCommand>& cmds, GlobalStatusPanel* global, const QVector<QPointer<DiskTab>>& diskTabs) {
  QList<ImportReportRow> out;
  out.reserve(cmds.size());

  // within-batch 去重的 canonical key：kind + 大小写无关化的 arg0。
  QSet<QString> seen;
  auto canon = [](const api::UwfmgrCommand& c) {
    const QString a0 = c.args.empty() ? QString{} : QString::fromStdString(c.args[0]).toLower();
    return QString::number(static_cast<int>(c.kind)) + QChar('|') + a0;
  };

  for (const auto& c : cmds) {
    ImportReportRow r;
    r.lineNo = c.sourceLineNo;
    r.lineText = QString::fromStdString(c.rawLine).trimmed();

    // 解析阶段失败的命令直接打包：
    // - Unsupported = 整段没识别 → Status::Unsupported；
    // - 其它非 None / Comment = cat/verb 已认出但参数非法 → Status::Failed。
    // parseErrorMessage 把 enum 翻译成中文（来自 ImportDialog.cpp 的 helper）。
    if (c.parseError != api::ParseError::None && c.parseError != api::ParseError::Comment) {
      r.status = c.parseError == api::ParseError::Unsupported ? ImportReportRow::Status::Unsupported : ImportReportRow::Status::Failed;
      r.detail = parseErrorMessage(c.parseError, QString::fromStdString(c.parseErrorContext));
      out.append(r);
      continue;
    }

    // within-batch dedup：第二条等价命令标 Duplicate，跳过 apply。
    const QString key = canon(c);
    if (seen.contains(key)) {
      r.status = ImportReportRow::Status::Duplicate;
      r.detail = I18n::tr("Same command was already issued earlier in this batch");
      out.append(r);
      continue;
    }
    seen.insert(key);

    // 把 args[0] 提到 QString 一次，下面分支统一用。args[0] 永远存在，因为
    // parser 只有在 args 完整时才把 parseError 设回 None；上面的 parseError 检查
    // 已经把缺参数的全部过滤掉了。
    const QString a0 = c.args.empty() ? QString{} : QString::fromStdString(c.args[0]);

    switch (c.kind) {
      case api::UwfmgrKind::FilterEnable:
      case api::UwfmgrKind::FilterDisable: {
        const bool want = c.kind == api::UwfmgrKind::FilterEnable;
        const bool changed = global ? global->importFilterEnabled(want) : false;
        r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
        r.detail =
            changed ? I18n::tr("Pending filter %1").arg(want ? I18n::tr("enable") : I18n::tr("disable")) : I18n::tr("Filter is already in the target state");
        break;
      }
      case api::UwfmgrKind::OverlaySetType: {
        const auto t = a0 == QStringLiteral("Disk") ? core::OverlayType::Disk : core::OverlayType::RAM;
        const bool changed = global ? global->importOverlayType(t) : false;
        r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
        r.detail = changed ? I18n::tr("Pending overlay type → %1").arg(a0) : I18n::tr("Overlay type already %1").arg(a0);
        break;
      }
      case api::UwfmgrKind::OverlaySetSize:
      case api::UwfmgrKind::OverlaySetWarningThreshold:
      case api::UwfmgrKind::OverlaySetCriticalThreshold: {
        bool ok = false;
        const auto mb = a0.toUInt(&ok);
        if (!ok) {
          r.status = ImportReportRow::Status::Failed;
          r.detail = I18n::tr("Invalid size value: %1").arg(a0);
          break;
        }
        bool changed = false;
        QString label;
        if (c.kind == api::UwfmgrKind::OverlaySetSize) {
          label = I18n::tr("maximum size");
          changed = global ? global->importOverlayMaxMb(mb) : false;
        } else if (c.kind == api::UwfmgrKind::OverlaySetWarningThreshold) {
          label = I18n::tr("warning threshold");
          changed = global ? global->importOverlayWarnMb(mb) : false;
        } else {
          label = I18n::tr("critical threshold");
          changed = global ? global->importOverlayCritMb(mb) : false;
        }
        r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
        r.detail = changed ? I18n::tr("Pending overlay %1 → %2 MB").arg(label).arg(mb) : I18n::tr("Overlay %1 already %2 MB").arg(label).arg(mb);
        break;
      }
      case api::UwfmgrKind::VolumeProtect:
      case api::UwfmgrKind::VolumeUnprotect: {
        auto* tab = findTab(diskTabs, a0);
        if (!tab) {
          r.status = ImportReportRow::Status::Failed;
          r.detail = I18n::tr("Unknown volume %1 (no UWF-eligible disk with that drive letter)").arg(a0);
          break;
        }
        const bool want = c.kind == api::UwfmgrKind::VolumeProtect;
        const bool changed = tab->importProtect(want);
        r.status = changed ? ImportReportRow::Status::Success : ImportReportRow::Status::Duplicate;
        r.detail = changed ? I18n::tr("Pending volume %1 protection %2").arg(a0, want ? I18n::tr("enable") : I18n::tr("disable"))
                           : I18n::tr("Volume %1 is already in the target protection state").arg(a0);
        break;
      }
      case api::UwfmgrKind::FileAddExclusion:
      case api::UwfmgrKind::FileRemoveExclusion: {
        const QString native = QDir::toNativeSeparators(a0);
        // 路径需要 "<盘符>:" 前缀来路由到对应 DiskTab；缺前缀 → 没办法定位。
        const QString dl = extractDriveLetter(native);
        if (dl.isEmpty()) {
          r.status = ImportReportRow::Status::Failed;
          r.detail = I18n::tr("Path %1 has no drive letter; cannot route to a volume tab").arg(native);
          break;
        }
        auto* tab = findTab(diskTabs, dl);
        if (!tab) {
          r.status = ImportReportRow::Status::Failed;
          r.detail = I18n::tr("No UWF-eligible disk for drive letter %1").arg(dl);
          break;
        }
        const auto outcome = c.kind == api::UwfmgrKind::FileAddExclusion ? tab->importAddFileExclusion(native) : tab->importRemoveFileExclusion(native);
        r = outcomeToRow(c, outcome, I18n::tr("file exclusion"));
        break;
      }
      case api::UwfmgrKind::RegistryAddExclusion:
      case api::UwfmgrKind::RegistryRemoveExclusion: {
        // 注册表排除是全局的，只挂在系统盘 TAB 上。其它 TAB 的 import* 在
        // m_regs == null 时直接返回 RejectedNotOnVolume，所以这里依次尝试
        // 每个 TAB——第一个非 RejectedNotOnVolume 的结果即视作系统盘 TAB
        // 的处理结果。所有 TAB 都拒说明压根没有系统盘 TAB。
        bool dispatched = false;
        for (const auto& t : diskTabs) {
          if (!t) continue;
          const auto outcome = c.kind == api::UwfmgrKind::RegistryAddExclusion ? t->importAddRegistryExclusion(a0) : t->importRemoveRegistryExclusion(a0);
          if (outcome != ExclusionListWidget::ImportOutcome::RejectedNotOnVolume) {
            r = outcomeToRow(c, outcome, I18n::tr("registry exclusion"));
            dispatched = true;
            break;
          }
        }
        if (!dispatched) {
          r.status = ImportReportRow::Status::Failed;
          r.detail = I18n::tr("Registry exclusions are only available on the system drive tab, which is not present");
        }
        break;
      }
      case api::UwfmgrKind::Unknown:
        // 解析阶段 Unknown 已在前面分支处理了；落到这里说明 parseError
        // 是 None 而 kind 又是 Unknown，理论上不会发生，安全兜底。
        r.status = ImportReportRow::Status::Unsupported;
        r.detail = I18n::tr("Unsupported command");
        break;
    }
    out.append(r);
  }

  // 导入命令逐条 setValue 写入，不触发 spinbox 的 editingFinished，约束链
  // （warn ≤ crit ≤ max）不会自动收紧、range 也停在导入时放宽的状态。批量
  // 导入结束补一次收紧，让面板回到自洽——否则之后任意一次无关交互触发
  // reconfigureRanges 时会静默改写导入值。
  if (global) global->finishImport();
  return out;
}

}  // namespace uwf::ui
