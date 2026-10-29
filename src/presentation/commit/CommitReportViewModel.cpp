// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitReportViewModel.h"

#include "app/commit/CommitFailure.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PresentationText.h"
#include "shared/Log.h"
namespace uwf::presentation {
QString explainCommitFailure(int32_t hresult, uint32_t returnValue, const app::CommitOperation operation) {
  const int32_t errorCode = hresult != 0 ? hresult : static_cast<int32_t>(returnValue);
  if (errorCode != 0) {
    switch (app::classifyCommitFailure(hresult, returnValue)) {
      // WBEM_E_FAILED 无法确定根因；提示只列出可能的占用情况，详细信息保留在日志中。
      case app::CommitFailureKind::GeneralFailure:
        return PresentationText::tr(
            "The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any "
            "program holding it and try again.");
      case app::CommitFailureKind::NotFound:
        if (operation == app::CommitOperation::DeleteAndPersist) {
          return PresentationText::tr(
              "Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It "
              "will disappear on reboot; commit-delete is neither needed nor possible.");
        }
        return PresentationText::tr("The target was not found; there is nothing in the overlay to commit.");
      case app::CommitFailureKind::InvalidParameter:
        return PresentationText::tr("A parameter was rejected by the system (invalid path or argument).");
      default:
        return hresult != 0 ? PresentationText::tr("The operation failed (see log for details).")
                            : PresentationText::tr("Operation rejected (code %1).").arg(returnValue);
    }
  }
  return PresentationText::tr("Unknown cause.");
}
QString formatErrorCode(int32_t hresult, uint32_t returnValue) {
  if (hresult != 0) return QString::asprintf("0x%08X", static_cast<uint32_t>(hresult));
  if (returnValue != 0) return QString("rv=%1").arg(returnValue);
  return "-";
}
CommitReportRow makeCommitReportRow(const app::CommitTargetResult& result, QString display, const app::CommitOperation operation) {
  const auto& existedBefore = result.existedBefore;
  const auto& existsAfter = result.existsAfter;
  const auto hresult = result.hresult;
  const auto returnValue = result.returnValue;
  const auto& failureDetail = result.failureDetail;
  const auto& authoritativeFailure = result.authoritativeFailure;
  const bool succeeded = result.succeeded;
  if (result.targetAlreadyAbsent) {
    CommitReportRow row;
    row.path = display;
    row.existedBefore = false;
    row.existsAfter = false;
    row.status = CommitRowStatus::Skipped;
    row.errorCode = QStringLiteral("-");
    row.reason = PresentationText::tr("The target no longer exists, so there is nothing to delete.");
    return row;
  }
  CommitReportRow row;
  row.path = display;
  row.existedBefore = existedBefore;
  row.existsAfter = existsAfter;
  if (succeeded) {
    row.status = CommitRowStatus::Succeeded;
    row.errorCode = QStringLiteral("-");
    row.reason = QStringLiteral("-");
  } else {
    const int32_t code = hresult != 0 ? hresult : static_cast<int32_t>(returnValue);
    const bool skipped = app::classifyCommitFailure(hresult, returnValue) == app::CommitFailureKind::NotFound;
    row.status = skipped ? CommitRowStatus::Skipped : CommitRowStatus::Failed;
    row.errorCode = code == 0 ? QStringLiteral("-") : formatErrorCode(hresult, returnValue);
    row.reason = !authoritativeFailure.isEmpty() ? authoritativeFailure : (code == 0 ? failureDetail : explainCommitFailure(hresult, returnValue, operation));
    const QString diagnostic = !authoritativeFailure.isEmpty() ? authoritativeFailure : failureDetail;
    UWF_LOG_W("commit") << "operation failed: target=" << row.path.toStdString() << " error=" << diagnostic.toStdString();
  }
  return row;
}
QString commitStatusText(const CommitRowStatus status) {
  switch (status) {
    case CommitRowStatus::Succeeded:
      return PresentationText::tr("Succeeded");
    case CommitRowStatus::Skipped:
      return PresentationText::tr("Skipped");
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
      case CommitRowStatus::Skipped:
        ++counts.skipped;
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
    result.title = counts.succeeded > 0 ? PresentationText::tr("Some changes committed") : PresentationText::tr("No changes could be committed");
    result.tone = counts.succeeded > 0 ? Tone::Warning : Tone::Error;
  } else if (completion.unreadBranches > 0 || completion.untouched > 0) {
    result.title = counts.succeeded > 0 ? PresentationText::tr("Some changes committed") : PresentationText::tr("Traversal completed with unread content");
    result.tone = Tone::Warning;
  } else if (counts.succeeded > 0) {
    result.title = counts.skipped > 0 ? PresentationText::tr("Commit complete") : PresentationText::tr("All changes committed");
    result.tone = Tone::Success;
  } else {
    result.title = PresentationText::tr("No changes to commit");
  }
  result.summary = PresentationText::tr("%1 succeeded; %2 skipped; %3 failed.").arg(counts.succeeded).arg(counts.skipped).arg(counts.failed);
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
    if (role == StatusRole)
      return static_cast<int>(row.status == CommitRowStatus::Succeeded ? ItemStatus::Success
                              : row.status == CommitRowStatus::Skipped ? ItemStatus::Muted
                                                                       : ItemStatus::Error);
  }
  if (role != Qt::DisplayRole && role != FullTextRole && role != Qt::ToolTipRole) return {};
  if (index.column() == 0) return commitStatusText(row.status);
  if (index.column() == 1) {
    if (role != Qt::DisplayRole) return row.path;
    const auto path = QString(row.path).replace(u'/', u'\\');
    const auto name = path.section(u'\\', -1);
    return name.isEmpty() ? row.path : name;
  }
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
