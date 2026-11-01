// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractTableModel>
#include <QList>
#include <QString>
#include <QStringList>
#include <optional>

#include "application/commit/contracts/CommitPlan.h"
#include "application/commit/contracts/CommitResult.h"
namespace uwf::presentation {
enum class CommitRowStatus { Succeeded, Failed };
struct CommitCounts {
  int succeeded = 0;
  int failed = 0;
};
struct CommitCompletion {
  enum class Cause { Finished, Canceled, Interrupted };
  Cause cause = Cause::Finished;
  int untouched = 0;
  qsizetype unreadBranches = 0;
};
struct CommitOutcome {
  enum class Tone { Success, Warning, Error, Neutral };
  QString title, summary, icon;
  Tone tone = Tone::Neutral;
};
struct CommitReportRow {
  CommitRowStatus status = CommitRowStatus::Failed;
  QString path;       // Full display path, including the value name for registry values.
  QString errorCode;  // Formatted diagnostic, or "-" on success.
  QString reason;     // User-facing explanation, or "-" on success.
  // Deletion observations only; ordinary commits leave existence unknown.
  std::optional<bool> existedBefore;
  std::optional<bool> existsAfter;
  QString targetName{};  // Built from the typed target; never split a rendered registry value.
};

[[nodiscard]] QString commitStatusText(CommitRowStatus status);
[[nodiscard]] CommitCounts countCommitResults(const QList<CommitReportRow>& rows);
[[nodiscard]] CommitOutcome describeCommitOutcome(CommitCounts counts, CommitCompletion completion = {});

[[nodiscard]] QString explainCommitFailure(app::OperationFailureKind kind, app::OperationDiagnostic diagnostic);
[[nodiscard]] QString formatErrorCode(app::OperationDiagnostic diagnostic);
[[nodiscard]] CommitReportRow makeCommitReportRow(const app::CommitTargetResult& result, const app::CommitNode& target);
class CommitReportViewModel final : public QAbstractTableModel {
  Q_OBJECT
 public:
  using Counts = CommitCounts;
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  CommitReportViewModel(const QList<CommitReportRow>& rows, app::CommitOperation operation, CommitCompletion completion = {});
  [[nodiscard]] CommitOutcome outcome() const { return describeCommitOutcome(m_counts, m_completion); }
  [[nodiscard]] const QList<CommitReportRow>& rows() const { return m_rows; }
  [[nodiscard]] bool showExistence() const { return m_showExistence; }
  [[nodiscard]] QString summary() const { return m_summary; }
  [[nodiscard]] QString description() const;
  [[nodiscard]] bool descriptionWarning() const;
  [[nodiscard]] Counts counts() const { return m_counts; }
  [[nodiscard]] QString details(int row) const;
  [[nodiscard]] const QStringList& headers() const { return m_headers; }
  [[nodiscard]] QString allRowsToText() const;
  [[nodiscard]] static QString existenceText(std::optional<bool> value);

 private:
  QList<CommitReportRow> m_rows;
  bool m_showExistence;
  QString m_summary;
  QStringList m_headers;
  Counts m_counts;
  CommitCompletion m_completion;
};
}  // namespace uwf::presentation
