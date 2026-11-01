// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <functional>
#include <map>
#include <optional>

#include "application/identity/contracts/ObjectIdentity.h"
#include "application/staging/contracts/FileStagingTypes.h"

namespace uwf::app {

struct FileStagingExclusionConflict {
  FileStagingEntry entry;
  QString exclusion;
};
// Shared final-state validation for editors and application plans. Only staging
// object paths are normalized; exclusion patterns remain exact provider data.
[[nodiscard]] std::optional<FileStagingExclusionConflict> firstFileStagingExclusionConflict(const QList<FileStagingEntry>& entries,
                                                                                            const QStringList& exclusions,
                                                                                            const FileStagingVolumeIdentities& volumes = {});

// Reject staging/exclusion overlap using both management commit-skip and driver
// write-bypass rules. Stored patterns remain unnormalized.
class FileStagingConflictPolicy final : public QObject {
  Q_OBJECT
 public:
  explicit FileStagingConflictPolicy(QObject* parent = nullptr) : QObject(parent) {}
  void setVolumeIdentity(const QString& driveLetter, const std::string& volumeId);
  void setFileExclusions(QStringList exclusions);
  // The editor owns the live draft. Queries observe it on the owner thread;
  // asynchronous operations capture one immutable projection below.
  void setFileExclusionSource(std::function<QStringList()> source);
  void setStagedEntries(const QList<FileStagingEntry>& entries);

  [[nodiscard]] std::optional<QString> conflictingExclusion(const QString& stagedPath, FileStagingKind kind) const;
  [[nodiscard]] std::optional<QString> conflictingExclusion(const FileStagingEntry& entry) const;
  [[nodiscard]] std::optional<QString> conflictingStagedPath(const QString& exclusionPath) const;
  [[nodiscard]] std::function<std::optional<QString>(const FileStagingEntry&)> exclusionChecker() const {
    return [exclusions = fileExclusions(), volumes = m_volumes](const FileStagingEntry& entry) -> std::optional<QString> {
      const auto conflict = firstFileStagingExclusionConflict({entry}, exclusions, volumes);
      return conflict ? std::optional{conflict->exclusion} : std::nullopt;
    };
  }

 signals:
  void stagedEntriesChanged();

 private:
  [[nodiscard]] QStringList fileExclusions() const { return m_fileExclusionSource ? m_fileExclusionSource() : QStringList{}; }
  std::function<QStringList()> m_fileExclusionSource;
  QList<FileStagingEntry> m_stagedEntries;
  FileStagingVolumeIdentities m_volumes;
};

}  // namespace uwf::app
