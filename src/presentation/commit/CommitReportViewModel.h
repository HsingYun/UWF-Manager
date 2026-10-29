// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractTableModel>
#include <QList>
#include <QString>
#include <QStringList>
#include <optional>

#include "app/api/CommitResult.h"
namespace uwf::presentation {
enum class CommitRowStatus { Succeeded, Skipped, Failed };
struct CommitCounts {
  int succeeded = 0;
  int skipped = 0;
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
  QString path;       // 完整路径 / 注册表键
  QString errorCode;  // "0x80041001" 之类，成功为 "-"
  QString reason;     // 面向普通用户的解释，成功为 "-"
  // 仅删除操作填充：执行前 / 后目标是否存在。提交操作留 nullopt，结果表不显示这两列。
  std::optional<bool> existedBefore;
  std::optional<bool> existsAfter;
};

[[nodiscard]] QString commitStatusText(CommitRowStatus status);
[[nodiscard]] CommitCounts countCommitResults(const QList<CommitReportRow>& rows);
[[nodiscard]] CommitOutcome describeCommitOutcome(CommitCounts counts, CommitCompletion completion = {});

[[nodiscard]] QString explainCommitFailure(int32_t hresult, uint32_t returnValue, app::CommitOperation operation);
[[nodiscard]] QString formatErrorCode(int32_t hresult, uint32_t returnValue);
[[nodiscard]] CommitReportRow makeCommitReportRow(const app::CommitTargetResult& result, QString display, app::CommitOperation operation);
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
