// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QString>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "application/commit/contracts/CommitPlan.h"
#include "application/operations/contracts/OperationError.h"
#include "application/staging/contracts/AutomaticCommitDisposition.h"

namespace uwf::app {

enum class FileStagingKind {
  File,
  Directory,
};

struct FileStagingEntry {
  FileStagingKind kind;
  QString path;
  // An observation for this editing/review session, never registry data. Keep
  // the user's path for display and re-resolve it before filesystem writes.
  std::optional<FileCommitTarget> fileTarget{};

  bool operator==(const FileStagingEntry& other) const { return kind == other.kind && path == other.path; }
};
using FileStagingVolumeIdentities = std::map<QString, std::string, ObjectNameLess>;

// Prepared host observations. Publishing a receipt must not resolve paths again.
struct FileStagingProjection {
  QList<FileStagingEntry> entries;
  FileStagingVolumeIdentities volumes;
  std::map<QString, QString, ObjectNameLess> drives;
};

struct FileStagingChanges {
  QList<FileStagingEntry> before;
  QList<FileStagingEntry> after;
  bool persistencePending = false;
  // A draft projects legacy storage into a logical set without scheduling a
  // repair write. Preserve its actual read alongside the reviewed logical
  // baseline so normalization cannot conceal a concurrent storage mutation.
  std::optional<QList<FileStagingEntry>> storedBefore{};
  // Mounted roots observed with the selected objects. Session registrations
  // take precedence; this also addresses a newly registered Next volume.
  FileStagingVolumeIdentities volumeIdentities{};
  [[nodiscard]] QList<FileStagingEntry> added() const;
  [[nodiscard]] QList<FileStagingEntry> removed() const;
  [[nodiscard]] std::size_t count() const;
};

struct FileStagingWriteResult {
  std::optional<QList<FileStagingEntry>> observed;
  std::exception_ptr failure;
  bool attempted = false;
  std::optional<FileStagingProjection> projection{};
  std::uint64_t revision = 0;
};
using FileStagingWriteOperation = std::function<FileStagingWriteResult()>;

enum class FileStagingCommitFailureKind {
  InvalidPath,
  ReparsePoint,
  PathInspectionFailed,
  FileTypeChanged,
  DirectoryTypeChanged,
  DirectoryEnumerationFailed,
  VolumeResolutionFailed,
  VolumeRoot,
  ProviderFailure,
  UnknownFailure,
};

struct FileStagingCommitFailure {
  QString path;
  FileStagingCommitFailureKind kind;
  QString detail;
  std::optional<OperationFailure> operation{};
};

// Equality describes the reviewed execution scope, not merely a file count.
// Identities retain volume binding and distinguish equally sized proposals.
struct AutomaticCommitProposal {
  std::vector<std::pair<CommitNodeIdentity, AutomaticCommitDisposition>> scopes{};
  std::vector<CommitNodeIdentity> files{};
  bool operator==(const AutomaticCommitProposal&) const = default;
};

struct FileStagingCommitResult {
  std::size_t discoveredFiles = 0;
  std::size_t committedFiles = 0;
  std::size_t skippedFiles = 0;
  std::size_t skippedEntries = 0;
  QList<FileStagingCommitFailure> failures;
  WriteEffect effect = WriteEffect::None;
  std::size_t pausedEntries = 0;
  std::size_t pausedFiles = 0;
  AutomaticCommitSummary automaticCommit{};

  [[nodiscard]] bool succeeded() const { return failures.isEmpty(); }
  [[nodiscard]] WriteEffect writeEffect() const {
    auto combined = effect;
    for (const auto& failure : failures)
      if (failure.operation) combined = combineWriteEffects(combined, failure.operation->effect);
    return combined;
  }
};

// A staging task can fail before producing a commit result (for example while
// reading the saved list). Power flows retain this summary across preparation
// and commit polling to distinguish target failures from infrastructure errors.
enum class FileStagingCommandOutcome { Succeeded, CompletedWithFailures, Failed };

struct FileStagingCommandResult {
  FileStagingCommandOutcome outcome = FileStagingCommandOutcome::Succeeded;
  std::size_t discoveredFiles = 0;
  std::size_t committedFiles = 0;
  std::size_t skippedFiles = 0;
  std::size_t skippedEntries = 0;
  std::size_t failedTargets = 0;
  QString detail;

  std::size_t pausedEntries = 0;
  std::size_t pausedFiles = 0;

  [[nodiscard]] static FileStagingCommandResult infrastructureFailure(QString detail) {
    FileStagingCommandResult result;
    result.outcome = FileStagingCommandOutcome::Failed;
    result.detail = std::move(detail);
    return result;
  }

  [[nodiscard]] bool completed() const {
    return outcome == FileStagingCommandOutcome::Succeeded || outcome == FileStagingCommandOutcome::CompletedWithFailures;
  }
};

}  // namespace uwf::app
