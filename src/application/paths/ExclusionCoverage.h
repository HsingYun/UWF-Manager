// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <QStringList>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "application/paths/PathPolicy.h"

namespace uwf::app {
// Add a drive only to volume-rooted patterns. Preserve the provider's spelling,
// including separators, dot components and trailing characters.
[[nodiscard]] QString qualifyFileExclusionPath(const QString& path, const QString& driveLetter);
// Each nonempty field retains a stored entry proving that independent effect.
// Driver write bypass applies to ordinary mode, not ROM; management commit
// skipping does not prove persistence. Preserve stored patterns exactly,
// including separators and dot components. Only target paths may be normalized.
struct ExclusionCoverage {
  QString directWriteExclusion;
  QString commitSkipExclusion;
  [[nodiscard]] bool writesBypass() const { return !directWriteExclusion.isEmpty(); }
  [[nodiscard]] bool commitSkipped() const { return !commitSkipExclusion.isEmpty(); }
  [[nodiscard]] bool preventsCommit() const { return writesBypass() || commitSkipped(); }
  [[nodiscard]] QString unreliableCommitReason() const;
};
[[nodiscard]] ExclusionCoverage fileExclusionCoverage(const QStringList& exclusions, const QString& target, const paths::VolumeIdentities& volumes = {});
[[nodiscard]] ExclusionCoverage fileExclusionCoverage(const std::vector<std::string>& exclusions, const QString& target,
                                                     const paths::VolumeIdentities& volumes = {});
// Resolve the current control set at most once, and only if driver coverage
// remains unresolved. Missing required observations throw.
[[nodiscard]] ExclusionCoverage registryExclusionCoverage(const QStringList& exclusions, const QString& target,
                                                          const std::function<std::uint32_t()>& currentControlSet = {});
[[nodiscard]] ExclusionCoverage registryExclusionCoverage(const std::vector<std::string>& exclusions, const QString& target,
                                                          const std::function<std::uint32_t()>& currentControlSet = {});
// Return the original management-skip pattern, independent of driver state
// and CurrentControlSet. A match does not prove the target is persistent.
[[nodiscard]] QString fileCommitSkipExclusion(const QStringList& exclusions, const QString& target, const paths::VolumeIdentities& volumes = {});
[[nodiscard]] QString fileCommitSkipExclusion(const std::vector<std::string>& exclusions, const QString& target,
                                             const paths::VolumeIdentities& volumes = {});
[[nodiscard]] QString registryCommitSkipExclusion(const QStringList& exclusions, const QString& target);
[[nodiscard]] QString registryCommitSkipExclusion(const std::vector<std::string>& exclusions, const QString& target);
// Whether a pattern can prevent committing the selected directory or one of its
// descendants. File targets use fileExclusionCoverage instead.
[[nodiscard]] bool fileExclusionIntersectsDirectory(const QString& pattern, const QString& directory, const paths::VolumeIdentities& volumes = {});
}  // namespace uwf::app
