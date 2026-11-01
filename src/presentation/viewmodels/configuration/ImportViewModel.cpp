// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/configuration/ImportViewModel.h"

#include <QFileInfo>
#include <QPointer>
#include <QStringList>
#include <algorithm>
#include <utility>

#include "foundation/text/Utf8.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/common/PresentationText.h"

namespace uwf::presentation {
namespace {
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

void ImportViewModel::setPreparing(const bool preparing) {
  if (m_preparing == preparing) return;
  m_preparing = preparing;
  emit availabilityChanged();
}

void ImportViewModel::rejectInvalidInput() {
  emit warningRaised(PresentationText::tr("Import failed"), app::parseErrorMessage(app::ParseError::Unsupported, {}));
}

void ImportViewModel::loadFile(const QString& path) {
  constexpr qint64 kMaxBytes = qint64{5} * 1024 * 1024;
  const auto reader = m_textFileReader;
  const QPointer self(this);
  const auto result = reader ? reader(path, kMaxBytes) : app::TextFileReadResult{app::TextFileReadResult::Failure::Open, {}, {}};
  if (!self) return;
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
  const auto decoded = utf8ToUtf16(std::string_view(bytes.constData(), static_cast<std::size_t>(bytes.size())));
  if (!decoded || decoded->find(u'\0') != std::u16string::npos) {
    rejectInvalidInput();
    return;
  }

  QStringList matched;
  const QStringList lines = QString::fromStdU16String(*decoded).split('\n');
  const bool generatedScript = std::ranges::any_of(lines, [](const auto& line) { return app::isUwfmgrScriptMarker(line.toUtf8().toStdString()); });
  for (const QString& line : lines) {
    // The script wrapper carries percent-escaping and encoding semantics.
    if (generatedScript || line.contains(QStringLiteral("uwfmgr"), Qt::CaseInsensitive)) matched << line;
  }

  QString block = QStringLiteral("# %1\n").arg(PresentationText::tr("File: %1").arg(QFileInfo(path).fileName()));
  if (matched.isEmpty()) {
    // Distinguishes an empty command set from a load failure.
    block += QStringLiteral("# %1\n").arg(PresentationText::tr("No uwfmgr commands found in the input."));
  } else {
    for (const auto& l : std::as_const(matched)) {
      block += l;
      block += QChar('\n');
    }
  }

  emit textBlockReady(block);
}

QList<ImportViewModel::RecommendedGroup> ImportViewModel::recommendedGroups() const {
  QList<RecommendedGroup> groups;
  groups.reserve(static_cast<qsizetype>(m_recommendations.size()));
  for (const auto& group : m_recommendations) groups.append({group.section, PresentationText::tr(group.title)});
  return groups;
}

QString ImportViewModel::defaultRulesText(const int sections) const {
  QString block;
  block += QStringLiteral(":: ==== ");
  block += PresentationText::tr("Microsoft recommended UWF exclusions");
  block += QStringLiteral(" ====\n");
  block += QStringLiteral(":: ");
  block += PresentationText::tr("Review these recommendations before importing.");
  block += QStringLiteral("\n");
  block += QStringLiteral(":: ");
  block += PresentationText::tr("Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.");
  block += QStringLiteral("\n\n");
  for (const auto& group : m_recommendations) {
    if ((sections & group.section) == 0) continue;
    appendSectionHeader(block, PresentationText::tr(group.title));
    for (const auto& entry : group.entries)
      appendCommandWithComment(block, PresentationText::tr(entry.description), QString::fromStdString(app::renderUwfmgrCommand(entry.command)));
    if (group.note) block += QStringLiteral(":: ") + PresentationText::tr(group.note) + u'\n';
    block += u'\n';
  }
  return block;
}

void ImportViewModel::importText() {
  if (m_preparing) return;
  if (!m_applier) {
    emit warningRaised(PresentationText::tr("Import failed"), PresentationText::tr("Command import is not ready."));
    return;
  }
  const QString text = m_text;
  if (text.contains(QChar::Null) || !isValidUtf16(QStringView(text))) {
    rejectInvalidInput();
    return;
  }

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

  const auto apply = m_applier;
  const QPointer self(this);
  const auto rows = apply(realCmds);
  if (self) appendReport(rows);
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
  m_pendingReports.append(rows);
  if (m_publishingReport) return;
  m_publishingReport = true;
  const QPointer self(this);
  while (!m_pendingReports.isEmpty()) {
    const auto report = m_pendingReports.takeFirst();
    const int batch = static_cast<int>(m_batches.size());
    const int extra = static_cast<int>(report.size()) + (batch > 0 ? 1 : 0);
    if (extra > 0) beginInsertRows({}, rowCount(), rowCount() + extra - 1);
    if (!self) return;
    ++m_totals.batches;
    for (const auto& r : report) {
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

    m_batches.append(report);
    if (batch > 0) m_rows.append({batch, -1});
    for (int row = 0; row < report.size(); ++row) m_rows.append({batch, row});
    if (extra > 0) endInsertRows();
    if (!self) return;
    emit reportAppended(report);
    if (!self) return;
  }
  m_publishingReport = false;
}

}  // namespace uwf::presentation

#include "moc_ImportViewModel.cpp"
