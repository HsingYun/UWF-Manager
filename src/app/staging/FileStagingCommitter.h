// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <atomic>
#include <cstddef>
#include <functional>
#include <stop_token>
#include <vector>

#include "app/api/CommitPreparation.h"
#include "app/api/FileStagingTypes.h"
#include "app/uwf/UwfVolume.h"

namespace uwf::app {

class FileStagingCommitOperation;
class FileStagingCommitter;

// 准备阶段先冻结当前受保护卷和真正需要扫描的暂存条目。未受保护卷在这里
// 整项跳过，避免关机阶段先递归遍历一个最终不会调用 CommitFile 的大目录。
class FileStagingScanPlan {
 private:
  friend class FileStagingCommitter;

  std::shared_ptr<const CommitTraversalSource> m_source;

  QList<app::FileStagingEntry> m_entries;
  QStringList m_fileExclusions;
  std::vector<provider::VolumeRow> m_volumes;
  std::size_t m_skippedEntries = 0;
  QList<FileStagingCommitFailure> m_failures;
};

// 文件系统扫描结果已经携带准备阶段冻结的卷快照。scan() 只访问文件系统，
// 可安全地在工作线程执行；prepare() 只是把完成的扫描结果收束成提交计划。
class FileStagingScan {
 private:
  friend class FileStagingCommitter;

  std::shared_ptr<const CommitTraversalSource> m_source;

  QList<QString> m_files;
  QStringList m_fileExclusions;
  std::vector<provider::VolumeRow> m_volumes;
  std::size_t m_skippedFiles = 0;
  std::size_t m_skippedEntries = 0;
  QList<FileStagingCommitFailure> m_failures;
};

// prepare() 生成的提交快照。文件数只包含当前位于受保护卷、确实需要调用
// CommitFile 的普通文件；未受保护卷、缺失路径和扫描失败分别保留为跳过或失败
// 结果。计划一旦生成便不再吸收目录中后续新增的文件，确保确认框展示的数量与
// 本轮实际处理范围一致。
class FileStagingCommitPlan {
 public:
  [[nodiscard]] std::size_t fileCount() const { return static_cast<std::size_t>(m_files.size()); }

 private:
  friend class FileStagingCommitter;
  friend class FileStagingCommitOperation;

  std::shared_ptr<const CommitTraversalSource> m_source;

  QList<QString> m_files;
  std::vector<provider::VolumeRow> m_volumes;
  std::size_t m_discoveredFiles = 0;
  std::size_t m_skippedFiles = 0;
  std::size_t m_skippedEntries = 0;
  QList<FileStagingCommitFailure> m_failures;
};

// Synchronous execution primitive. FileStagingTask runs it on a worker and
// publishes progress; callers must not execute provider calls on the UI thread.
class FileStagingCommitOperation {
 public:
  [[nodiscard]] bool finished() const { return m_nextFile >= m_files.size(); }
  [[nodiscard]] std::size_t totalFiles() const { return static_cast<std::size_t>(m_files.size()); }
  [[nodiscard]] std::size_t processedFiles() const { return static_cast<std::size_t>(m_nextFile); }
  [[nodiscard]] const FileStagingCommitResult& result() const { return m_result; }

  // 返回本次处理的规范化路径；finished() 为 true 时调用属于编程错误。
  [[nodiscard]] QString advance();

 private:
  friend class FileStagingCommitter;

  FileStagingCommitOperation(const std::function<wmi::WmiOperations&()>& session, FileStagingCommitPlan plan);

  std::function<wmi::WmiOperations&()> m_session;
  std::shared_ptr<const CommitTraversalSource> m_source;
  QList<QString> m_files;
  std::vector<provider::VolumeRow> m_volumes;
  qsizetype m_nextFile = 0;
  FileStagingCommitResult m_result;
};

// 把持久化暂存条目展开成实际文件并提交到当前会话卷。目录递归时跳过所有
// reparse point；重叠目录和显式文件按大小写不敏感路径去重。失败逐项收集，
// 由电源用例决定是否继续关机/重启。
class FileStagingCommitter {
 public:
  FileStagingCommitter(wmi::WmiOperations& session, std::shared_ptr<const CommitTraversalSource> source);

  // Provider snapshot failures propagate to the batch owner, without target results.
  [[nodiscard]] FileStagingScanPlan prepareScan(const QList<app::FileStagingEntry>& entries) const;
  [[nodiscard]] static FileStagingScan scan(FileStagingScanPlan plan, std::stop_token stopToken = {}, std::atomic_size_t* discoveredProgress = nullptr);
  [[nodiscard]] FileStagingCommitPlan prepare(FileStagingScan scan) const;
  [[nodiscard]] FileStagingCommitOperation beginCommit(FileStagingCommitPlan plan) const;

 private:
  std::function<wmi::WmiOperations&()> m_session;
  std::shared_ptr<const CommitTraversalSource> m_source;
  provider::UwfVolume m_volume;
};

}  // namespace uwf::app
