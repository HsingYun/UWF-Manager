// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ImportViewModel.h"

#include <QFileInfo>
#include <QStringList>
#include <algorithm>
#include <utility>

#include "presentation/common/ItemRoles.h"
#include "presentation/common/PresentationText.h"

namespace uwf::presentation {
namespace {
QString quotedFileCommand(const QString& path) { return QStringLiteral("uwfmgr file add-exclusion \"%1\"").arg(path); }

QString registryCommand(const QString& key) { return QStringLiteral("uwfmgr registry add-exclusion \"%1\"").arg(key); }

void appendCommandWithComment(QString& block, const QString& comment, const QString& command) {
  block += QStringLiteral(":: ");
  block += comment;
  block += QChar('\n');
  block += command;
  block += QChar('\n');
}

void appendSectionHeader(QString& block, const QString& title) {
  block += QStringLiteral(":: -- ");
  block += title;
  block += QStringLiteral(" --\n");
}

}  // namespace

void ImportViewModel::loadFile(const QString& path) {
  constexpr qint64 kMaxBytes = qint64{5} * 1024 * 1024;
  const auto result = m_textFileReader ? m_textFileReader(path, kMaxBytes) : app::TextFileReadResult{app::TextFileReadResult::Failure::Open, {}, {}};
  using Failure = app::TextFileReadResult::Failure;
  if (result.failure == Failure::TooLarge) {
    emit warningRaised(
        PresentationText::tr("File too large"),
        PresentationText::tr("File %1 is larger than 5 MB and was not parsed. Please filter it manually first.").arg(QFileInfo(path).fileName()));
    return;
  }
  if (result.failure != Failure::None) {
    emit warningRaised(PresentationText::tr("Cannot read file"),
                       PresentationText::tr("Could not open file %1: %2").arg(QFileInfo(path).fileName(), result.error));
    return;
  }
  const auto& bytes = result.bytes;

  QStringList matched;
  const QStringList lines = QString::fromUtf8(bytes).split('\n');
  const auto marker = QString::fromUtf8(app::kUwfmgrScriptMarker.data(), static_cast<qsizetype>(app::kUwfmgrScriptMarker.size()));
  const bool generatedScript = std::ranges::any_of(lines, [&](const auto& line) { return line.trimmed() == marker; });
  for (const QString& line : lines) {
    // 脚本封装是百分号字面量与编码合同的一部分，不能只保留其中的 uwfmgr 行。
    if (generatedScript || line.contains(QStringLiteral("uwfmgr"), Qt::CaseInsensitive)) matched << line;
  }

  QString block = QStringLiteral("# %1\n").arg(PresentationText::tr("File: %1").arg(QFileInfo(path).fileName()));
  if (matched.isEmpty()) {
    // 没有命令时追加说明，以区分空结果与加载失败。
    block += QStringLiteral("# %1\n").arg(PresentationText::tr("No uwfmgr commands found in the input."));
  } else {
    for (const auto& l : std::as_const(matched)) {
      block += l;
      block += QChar('\n');
    }
  }

  emit textBlockReady(block);
}

QString ImportViewModel::defaultRulesText(const int sections) const {
  QString sys = m_systemDrive;
  if (sys.isEmpty()) sys = QStringLiteral("C:");

  auto win = [&](const QString& path) { return sys + QStringLiteral("\\Windows") + path; };
  auto programData = [&](const QString& path) { return sys + QStringLiteral("\\ProgramData") + path; };
  auto programFiles = [&](const QString& path) { return sys + QStringLiteral("\\Program Files") + path; };

  QString block;
  block += QStringLiteral(":: ==== ");
  block += PresentationText::tr("Microsoft recommended UWF exclusions");
  block += QStringLiteral(" ====\n");
  block += QStringLiteral(":: ");
  block += PresentationText::tr("Review these recommendations before importing; folders should exist before UWF accepts file exclusions.");
  block += QStringLiteral("\n");
  block += QStringLiteral(":: ");
  block += PresentationText::tr("Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.");
  block += QStringLiteral("\n\n");

  auto addFile = [&](const QString& comment, const QString& path) { appendCommandWithComment(block, comment, quotedFileCommand(path)); };
  auto addReg = [&](const QString& comment, const QString& key) { appendCommandWithComment(block, comment, registryCommand(key)); };

  if (sections & Ceip) {
    appendSectionHeader(block, PresentationText::tr("Customer Experience Improvement Program (CEIP)"));
    addReg(PresentationText::tr("CEIP: persist policy opt-in state"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Policies\\Microsoft\\SQMClient\\Windows\\CEIPEnable"));
    addReg(PresentationText::tr("CEIP: persist local opt-in state"), QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\SQMClient\\Windows\\CEIPEnable"));
    addReg(PresentationText::tr("CEIP: persist upload-disable flag"), QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\SQMClient\\UploadDisableFlag"));
    block += QChar('\n');
  }

  if (sections & Bits) {
    appendSectionHeader(block, PresentationText::tr("Background Intelligent Transfer Service (BITS)"));
    addFile(PresentationText::tr("BITS: persist downloader queue files"), programData(QStringLiteral("\\Microsoft\\Network\\Downloader")));
    addReg(PresentationText::tr("BITS: persist transfer state index"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\BITS\\StateIndex"));
    block += QChar('\n');
  }

  if (sections & Network) {
    appendSectionHeader(block, PresentationText::tr("Network profiles and policies"));
    addReg(PresentationText::tr("Wireless network GPO policy"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\Wireless\\GPTWirelessPolicy"));
    addReg(PresentationText::tr("Wired network GPO policy"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\WiredL2\\GP_Policy"));
    addFile(PresentationText::tr("Wireless network GPO policy files"), win(QStringLiteral("\\wlansvc\\Policies")));
    addFile(PresentationText::tr("Wired network GPO policy files"), win(QStringLiteral("\\dot2svc\\Policies")));
    addReg(PresentationText::tr("Wireless network interface profiles"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\wlansvc"));
    addReg(PresentationText::tr("Wired network interface profiles"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\dot3svc"));
    addReg(PresentationText::tr("Wireless service configuration"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\Wlansvc"));
    addReg(PresentationText::tr("Mobile broadband service configuration"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\WwanSvc"));
    addReg(PresentationText::tr("Wired AutoConfig service configuration"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\dot3svc"));
    block += QStringLiteral(":: ");
    block += PresentationText::tr("Network profile XML files are per-device; add the concrete Interfaces\\{GUID}\\{GUID}.xml paths manually if needed.");
    block += QStringLiteral("\n\n");
  }

  if (sections & Dst) {
    appendSectionHeader(block, PresentationText::tr("Daylight saving time (DST)"));
    addReg(PresentationText::tr("DST: persist time zone definitions"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"));
    addReg(PresentationText::tr("DST: persist selected time zone information"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation"));
    block += QChar('\n');
  }

  if (sections & Defender) {
    appendSectionHeader(block, PresentationText::tr("Microsoft Defender"));
    addFile(PresentationText::tr("Defender: persist product files and updates"), programFiles(QStringLiteral("\\Windows Defender")));
    addFile(PresentationText::tr("Defender: persist ProgramData signatures and state"), programData(QStringLiteral("\\Microsoft\\Windows Defender")));
    addFile(PresentationText::tr("Defender: persist Windows Update log"), win(QStringLiteral("\\WindowsUpdate.log")));
    addFile(PresentationText::tr("Defender: persist MpCmdRun log"), win(QStringLiteral("\\Temp\\MpCmdRun.log")));
    addReg(PresentationText::tr("Defender: persist product registry state"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Defender"));
    addReg(PresentationText::tr("Defender: persist WdBoot service state"), QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdBoot"));
    addReg(PresentationText::tr("Defender: persist WdFilter service state"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdFilter"));
    addReg(PresentationText::tr("Defender: persist WdNisSvc service state"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdNisSvc"));
    addReg(PresentationText::tr("Defender: persist WdNisDrv service state"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdNisDrv"));
    addReg(PresentationText::tr("Defender: persist WinDefend service state"),
           QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WinDefend"));
    block += QChar('\n');
  }

  if (sections & Scep) {
    appendSectionHeader(block, PresentationText::tr("System Center Endpoint Protection"));
    addFile(PresentationText::tr("SCEP: persist client program files"), programFiles(QStringLiteral("\\Microsoft Security Client")));
    addFile(PresentationText::tr("SCEP: persist Windows Update log"), win(QStringLiteral("\\Windowsupdate.log")));
    addFile(PresentationText::tr("SCEP: persist MpCmdRun log"), win(QStringLiteral("\\Temp\\Mpcmdrun.log")));
    addFile(PresentationText::tr("SCEP: persist antimalware signatures and state"), programData(QStringLiteral("\\Microsoft\\Microsoft Antimalware")));
    addReg(PresentationText::tr("SCEP: persist antimalware registry state"), QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft Antimalware"));
  }

  return block;
}

void ImportViewModel::importText() {
  if (!m_applier) {
    emit warningRaised(PresentationText::tr("Import failed"), PresentationText::tr("Internal error: no applier registered."));
    return;
  }
  const QString text = m_text;

  const auto parsed = app::parseUwfmgrText(text.toStdString());
  QList<app::UwfmgrCommand> realCmds;
  realCmds.reserve(static_cast<int>(parsed.size()));
  for (const auto& c : parsed) {
    if (c.parseError == app::ParseError::Comment) continue;
    realCmds.append(c);
  }
  if (realCmds.isEmpty()) {
    emit warningRaised(PresentationText::tr("Nothing to import"), PresentationText::tr("No uwfmgr commands found in the input."));
    return;
  }

  const auto rows = m_applier(realCmds);
  appendReport(rows);
}

int ImportViewModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(m_rows.size()); }
int ImportViewModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : 4; }
Qt::ItemFlags ImportViewModel::flags(const QModelIndex& index) const {
  if (!index.isValid() || index.model() != this || index.row() < 0 || index.row() >= rowCount()) return Qt::NoItemFlags;
  return m_rows[index.row()].entry < 0 ? Qt::ItemFlags(Qt::ItemIsEnabled) : Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}
QVariant ImportViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole || section < 0 || section >= 4) return {};
  const QStringList headers{PresentationText::tr("Line"), PresentationText::tr("Status"), PresentationText::tr("Original command"),
                            PresentationText::tr("Processing result")};
  return headers[section];
}
QVariant ImportViewModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.model() != this || index.row() < 0 || index.row() >= rowCount() || index.column() < 0 || index.column() >= 4) return {};
  const auto location = m_rows[index.row()];
  if (location.entry < 0) {
    if (role == SeparatorRole) return true;
    if (role == Qt::TextAlignmentRole) return static_cast<int>(Qt::AlignCenter);
    if (role == StatusRole) return static_cast<int>(ItemStatus::Muted);
    if (role == Qt::DisplayRole && index.column() == 0) return PresentationText::tr("── Batch %1 ──").arg(location.batch + 1);
    return {};
  }
  const auto& row = m_batches[location.batch][location.entry];
  if (role == Qt::TextAlignmentRole) return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  if (role == StatusBadgeRole) return index.column() == 1;
  if (role == MonospaceRole) return index.column() == 2;
  if (role == FullTextRole) return index.column() == 2 ? row.lineText : index.column() == 3 ? QVariant(row.detail) : QVariant{};
  if (role == StatusRole && (index.column() == 0 || index.column() == 3)) return static_cast<int>(ItemStatus::Muted);
  if (role == StatusRole && index.column() == 1) {
    switch (row.status) {
      case app::ImportResult::Status::Success:
        return static_cast<int>(ItemStatus::Success);
      case app::ImportResult::Status::Duplicate:
        return static_cast<int>(ItemStatus::Muted);
      case app::ImportResult::Status::Failed:
        return static_cast<int>(ItemStatus::Failed);
      case app::ImportResult::Status::Unsupported:
        return static_cast<int>(ItemStatus::Unsupported);
    }
  }
  if (role == Qt::ToolTipRole && index.column() >= 2) return index.column() == 2 ? row.lineText : row.detail;
  if (role != Qt::DisplayRole) return {};
  if (index.column() == 0) return QString::number(row.lineNo);
  if (index.column() == 2) return row.lineText;
  if (index.column() == 3) return row.detail;
  switch (row.status) {
    case app::ImportResult::Status::Success:
      return PresentationText::tr("Imported");
    case app::ImportResult::Status::Duplicate:
      return PresentationText::tr("Duplicate");
    case app::ImportResult::Status::Failed:
      return PresentationText::tr("Failed");
    case app::ImportResult::Status::Unsupported:
      return PresentationText::tr("Unsupported");
  }
  return {};
}
void ImportViewModel::appendReport(const QList<app::ImportResult>& rows) {
  ++m_totals.batches;
  for (const auto& r : rows) {
    switch (r.status) {
      case app::ImportResult::Status::Success:
        ++m_totals.success;
        break;
      case app::ImportResult::Status::Duplicate:
        ++m_totals.duplicate;
        break;
      case app::ImportResult::Status::Failed:
        ++m_totals.failed;
        break;
      case app::ImportResult::Status::Unsupported:
        ++m_totals.unsupported;
        break;
    }
  }

  const int batch = static_cast<int>(m_batches.size());
  const int extra = static_cast<int>(rows.size()) + (batch > 0 ? 1 : 0);
  if (extra > 0) beginInsertRows({}, rowCount(), rowCount() + extra - 1);
  m_batches.append(rows);
  if (batch > 0) m_rows.append({batch, -1});
  for (int row = 0; row < rows.size(); ++row) m_rows.append({batch, row});
  if (extra > 0) endInsertRows();
  emit reportAppended(rows);
}

}  // namespace uwf::presentation

#include "moc_ImportViewModel.cpp"
