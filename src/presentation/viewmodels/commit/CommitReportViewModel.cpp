// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/commit/CommitReportViewModel.h"

#include "application/commit/CommitFailure.h"
#include "foundation/logging/Log.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/common/PresentationText.h"
namespace uwf::presentation {
namespace {
QString commitTargetName(const app::CommitNode& target) {
  if (target.kind == app::CommitTargetKind::RegistryValue) return target.valueName.isEmpty() ? PresentationText::tr("(Default)") : target.valueName;
  // Only file-system paths accept '/' as a separator. A registry key component
  // may contain '/', and a registry value name may also contain '\\' or " : ".
  const QString path = target.kind == app::CommitTargetKind::RegistryKey ? target.path : QString(target.path).replace(u'/', u'\\');
  const auto leaf = path.section(u'\\', -1);
  return leaf.isEmpty() ? target.path : leaf;
}
}  // namespace

QString explainCommitFailure(app::OperationFailureKind kind, app::OperationDiagnostic diagnostic) {
  switch (app::classifyCommitFailure(kind)) {
    // General failures do not identify the cause; provider details remain in the log.
    case app::CommitFailureKind::GeneralFailure:
      return PresentationText::tr(
          "The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any "
          "program holding it and try again.");
    case app::CommitFailureKind::NotFound:
      return PresentationText::tr("The system did not find the target.");
    case app::CommitFailureKind::InvalidParameter:
      return PresentationText::tr("A parameter was rejected by the system (invalid path or argument).");
    default:
      if (diagnostic.code == 0 && diagnostic.result == 0) return PresentationText::tr("Unknown cause.");
      return diagnostic.code != 0 ? PresentationText::tr("The operation failed (see log for details).")
                                  : PresentationText::tr("Operation rejected (code %1).").arg(diagnostic.result);
  }
}
QString formatErrorCode(app::OperationDiagnostic diagnostic) {
  if (diagnostic.code != 0) return QString::asprintf("0x%08X", static_cast<uint32_t>(diagnostic.code));
  if (diagnostic.result != 0) return QString("rv=%1").arg(diagnostic.result);
  return "-";
}
CommitReportRow makeCommitReportRow(const app::CommitTargetResult& result, const app::CommitNode& target) {
  const auto& existedBefore = result.existedBefore;
  const auto& existsAfter = result.existsAfter;
  const auto diagnostic = result.diagnostic;
  const auto& failureDetail = result.failureDetail;
  const auto& authoritativeFailure = result.authoritativeFailure;
  const bool succeeded = result.succeeded;
  CommitReportRow row;
  row.path = target.display();
  row.targetName = commitTargetName(target);
  row.existedBefore = existedBefore;
  row.existsAfter = existsAfter;
  if (succeeded) {
    row.status = CommitRowStatus::Succeeded;
    row.errorCode = QStringLiteral("-");
    row.reason = QStringLiteral("-");
  } else {
    const int32_t code = diagnostic.code != 0 ? diagnostic.code : static_cast<int32_t>(diagnostic.result);
    row.status = CommitRowStatus::Failed;
    row.errorCode = code == 0 ? QStringLiteral("-") : formatErrorCode(diagnostic);
    row.reason =
        !authoritativeFailure.isEmpty()
            ? authoritativeFailure
            : (result.failureKind == app::OperationFailureKind::Other && code == 0 ? failureDetail : explainCommitFailure(result.failureKind, diagnostic));
    const QString detail = !authoritativeFailure.isEmpty() ? authoritativeFailure : failureDetail;
    UWF_LOG_E("commit") << "operation failed: target=" << row.path.toStdString() << " effect=" << static_cast<int>(result.failureEffect)
                        << " error=" << detail.toStdString();
  }
  return row;
}
QString commitStatusText(const CommitRowStatus status) {
  switch (status) {
    case CommitRowStatus::Succeeded:
      return PresentationText::tr("Succeeded");
    case CommitRowStatus::Failed:
      return PresentationText::tr("Failed");
  }
  return {};
}
CommitCounts countCommitResults(const QList<CommitReportRow>& rows) {
  CommitCounts counts;
  for (const auto& row : rows) {
    switch (row.status) {
      case CommitRowStatus::Succeeded:
        ++counts.succeeded;
        break;
      case CommitRowStatus::Failed:
        ++counts.failed;
        break;
    }
  }
  return counts;
}
CommitOutcome describeCommitOutcome(const CommitCounts counts, const CommitCompletion completion) {
  CommitOutcome result;
  using Tone = CommitOutcome::Tone;
  if (completion.cause == CommitCompletion::Cause::Canceled) {
    result.title = PresentationText::tr("Commit canceled");
    result.tone = Tone::Warning;
  } else if (completion.cause == CommitCompletion::Cause::Interrupted) {
    result.title = PresentationText::tr("Commit interrupted");
    result.tone = Tone::Warning;
  } else if (counts.failed > 0) {
    result.title = counts.succeeded > 0 ? PresentationText::tr("Some operations succeeded") : PresentationText::tr("No operations succeeded");
    result.tone = counts.succeeded > 0 ? Tone::Warning : Tone::Error;
  } else if (completion.unreadBranches > 0 || completion.untouched > 0) {
    result.title = counts.succeeded > 0 ? PresentationText::tr("Some operations succeeded") : PresentationText::tr("Traversal completed with unread content");
    result.tone = Tone::Warning;
  } else if (counts.succeeded > 0) {
    result.title = PresentationText::tr("All operations succeeded");
    result.tone = Tone::Success;
  } else {
    result.title = PresentationText::tr("No targets to process");
  }
  result.summary = PresentationText::tr("%1 succeeded; %2 failed.").arg(counts.succeeded).arg(counts.failed);
  if (completion.untouched > 0) result.summary += u'\n' + PresentationText::tr("%1 items were not processed.").arg(completion.untouched);
  if (completion.unreadBranches > 0) result.summary += PresentationText::tr(" · %1 branches could not be read").arg(completion.unreadBranches);
  result.icon = result.tone == Tone::Success   ? QStringLiteral(":/icons/status_success.svg")
                : result.tone == Tone::Error   ? QStringLiteral(":/icons/status_error.svg")
                : result.tone == Tone::Warning ? QStringLiteral(":/icons/status_warning.svg")
                                               : QStringLiteral(":/icons/info.svg");
  return result;
}
CommitReportViewModel::CommitReportViewModel(const QList<CommitReportRow>& rows, const app::CommitOperation operation, const CommitCompletion completion)
    : m_rows(rows), m_showExistence(operation == app::CommitOperation::DeleteAndPersist), m_counts(countCommitResults(rows)), m_completion(completion) {
  m_summary = outcome().summary;
  m_headers = {PresentationText::tr("Result"), PresentationText::tr("Target")};
  if (m_showExistence) m_headers << PresentationText::tr("Existed before") << PresentationText::tr("Exists after");
  m_headers << PresentationText::tr("Error code") << PresentationText::tr("Reason");
}
bool CommitReportViewModel::descriptionWarning() const { return m_completion.untouched > 0 || m_completion.unreadBranches > 0; }
QString CommitReportViewModel::description() const {
  if (descriptionWarning()) return m_summary;
  const auto result = outcome();
  if (result.tone == CommitOutcome::Tone::Success) return PresentationText::tr("The selected operations completed. UWF may skip excluded targets.");
  if (result.tone == CommitOutcome::Tone::Neutral) return result.title;
  return PresentationText::tr("Some items were not completed or could not be verified. Select an item to view its full path and details.");
}
int CommitReportViewModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(m_rows.size()); }
int CommitReportViewModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(m_headers.size()); }
QVariant CommitReportViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
  return orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < m_headers.size() ? QVariant(m_headers[section]) : QVariant{};
}
QVariant CommitReportViewModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.model() != this || index.row() < 0 || index.row() >= rowCount() || index.column() < 0 || index.column() >= columnCount())
    return {};
  const auto& row = m_rows[index.row()];
  if (role == MonospaceRole) return index.column() == columnCount() - 2;
  if (role == StatusRole && index.column() > 1) return static_cast<int>(ItemStatus::Muted);
  if (index.column() == 0) {
    if (role == StatusBadgeRole || role == StatusIconRole) return true;
    if (role == StatusRole) return static_cast<int>(row.status == CommitRowStatus::Succeeded ? ItemStatus::Success : ItemStatus::Error);
  }
  if (role != Qt::DisplayRole && role != FullTextRole && role != Qt::ToolTipRole) return {};
  if (index.column() == 0) return commitStatusText(row.status);
  if (index.column() == 1) return role == Qt::DisplayRole && !row.targetName.isEmpty() ? row.targetName : row.path;
  if (m_showExistence && index.column() == 2) return existenceText(row.existedBefore);
  if (m_showExistence && index.column() == 3) return existenceText(row.existsAfter);
  return index.column() == columnCount() - 2 ? row.errorCode : row.reason;
}
QString CommitReportViewModel::details(const int row) const {
  if (row < 0 || row >= rowCount()) return {};
  const auto& item = m_rows[row];
  QStringList lines{item.path};
  if (m_showExistence)
    lines.append(PresentationText::tr("Existed before") + QStringLiteral(": ") + existenceText(item.existedBefore) + QStringLiteral(" · ") +
                 PresentationText::tr("Exists after") + QStringLiteral(": ") + existenceText(item.existsAfter));
  if (!item.errorCode.isEmpty() && item.errorCode != QLatin1String("-") && item.errorCode != QStringLiteral("—")) lines.append(item.errorCode);
  if (!item.reason.isEmpty() && item.reason != QLatin1String("-")) lines.append(item.reason);
  return lines.join(u'\n');
}
QString CommitReportViewModel::existenceText(const std::optional<bool> value) {
  if (!value) return PresentationText::tr("Unknown");
  return *value ? PresentationText::tr("Yes") : PresentationText::tr("No");
}
QString CommitReportViewModel::allRowsToText() const {
  QString out = m_headers.join('\t');
  for (const auto& r : m_rows) {
    out += '\n' + commitStatusText(r.status) + '\t' + r.path;
    if (m_showExistence) {
      out += '\t';
      out += existenceText(r.existedBefore);
      out += '\t';
      out += existenceText(r.existsAfter);
    }
    out += '\t' + r.errorCode + '\t' + r.reason;
  }
  return out;
}
}  // namespace uwf::presentation

#include "moc_CommitReportViewModel.cpp"
