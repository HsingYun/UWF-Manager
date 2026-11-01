// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/paths/ExclusionCoverage.h"

#include <QDir>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "application/identity/contracts/ObjectIdentity.h"
#include "application/localization/ApplicationText.h"
#include "application/paths/PathPolicy.h"
#include "domain/uwf/policies/ExclusionCoverage.h"

namespace uwf::app {
namespace {
std::u16string_view view(const QString& path) { return {QStringView(path).utf16(), static_cast<std::size_t>(path.size())}; }
template <class Entry>
QString entryText(const Entry& entry) {
  if constexpr (std::is_same_v<Entry, QString>)
    return entry;
  else
    return QString::fromStdString(entry);
}
template <class Entries, class Evaluate>
ExclusionCoverage coverage(const Entries& entries, const QString& target, Evaluate evaluate) {
  ExclusionCoverage result;
  for (const auto& entry : entries) {
    const QString pattern = entryText(entry);
    const auto effects = evaluate(pattern, target);
    if (effects.driverBypassesWrites && result.directWriteExclusion.isEmpty()) result.directWriteExclusion = pattern;
    if (effects.managementSkipsCommit && result.commitSkipExclusion.isEmpty()) result.commitSkipExclusion = pattern;
    if (result.writesBypass() && result.commitSkipped()) break;
  }
  return result;
}
}  // namespace
QString qualifyFileExclusionPath(const QString& path, const QString& driveLetter) {
  const bool rooted = (path.startsWith(u'\\') && !path.startsWith(QLatin1String("\\\\"))) || (path.startsWith(u'/') && !path.startsWith(QLatin1String("//")));
  return !driveLetter.isEmpty() && rooted ? driveLetter + path : path;
}
QString ExclusionCoverage::unreliableCommitReason() const {
  return commitSkipped() && !writesBypass()
             ? ApplicationText::tr(
                   "UWF exclusion settings prevent a reliable commit of this target. Remove the conflicting exclusion and restart before "
                   "committing it.")
             : QString{};
}
namespace {
core::ExclusionEffects fileEffects(const QString& pattern, const QString& target, const paths::VolumeIdentities& volumes) {
  const auto storedAddress = paths::resolveVolumeFilePath(pattern, volumes);
  const auto actualAddress = paths::resolveVolumeFilePath(target, volumes);
  if (storedAddress && actualAddress) {
    if (storedAddress->volume != actualAddress->volume) return {};
    return core::fileExclusionEffects(view(storedAddress->relativePath), view(actualAddress->relativePath));
  }
  auto stored = pattern;
  auto actual = QDir::toNativeSeparators(target);
  if (stored.size() >= 2 && actual.size() >= 2 && stored[1] == u':' && actual[1] == u':') {
    if (!equalObjectNames(QStringView(stored).first(2), QStringView(actual).first(2))) return {};
    stored.remove(0, 2);
    actual.remove(0, 2);
  }
  return core::fileExclusionEffects(view(stored), view(actual));
}
template <class Entries, class SkipsCommit>
QString commitSkipExclusion(const Entries& entries, const QString& target, SkipsCommit skipsCommit) {
  for (const auto& entry : entries) {
    const auto pattern = entryText(entry);
    if (skipsCommit(pattern, target)) return pattern;
  }
  return {};
}
bool registrySkipsCommit(const QString& pattern, const QString& target) { return core::registryManagementSkipsCommit(view(pattern), view(target)); }
template <class Entries>
ExclusionCoverage registryCoverage(const Entries& entries, const QString& target, const std::function<std::uint32_t()>& currentControlSet) {
  ExclusionCoverage result;
  QStringList pending;
  for (const auto& entry : entries) {
    const QString pattern = entryText(entry);
    if (!result.commitSkipped() && core::registryManagementSkipsCommit(view(pattern), view(target))) result.commitSkipExclusion = pattern;
    if (!result.writesBypass()) {
      if (core::registryExclusionNeedsCurrentControlSet(view(pattern), view(target)))
        pending.append(pattern);
      else if (core::registryExclusionEffects(view(pattern), view(target)).driverBypassesWrites)
        result.directWriteExclusion = pattern;
    }
    if (result.writesBypass() && result.commitSkipped()) break;
  }
  if (result.writesBypass() || pending.isEmpty()) return result;
  if (!currentControlSet) throw std::runtime_error("The current registry control set is unavailable");
  const auto selected = currentControlSet();
  for (const auto& pattern : std::as_const(pending)) {
    if (core::registryExclusionEffects(view(pattern), view(target), selected).driverBypassesWrites) {
      result.directWriteExclusion = pattern;
      break;
    }
  }
  return result;
}
}  // namespace
ExclusionCoverage fileExclusionCoverage(const QStringList& exclusions, const QString& target, const paths::VolumeIdentities& volumes) {
  return coverage(exclusions, target, [&](const auto& pattern, const auto& actual) { return fileEffects(pattern, actual, volumes); });
}
ExclusionCoverage fileExclusionCoverage(const std::vector<std::string>& exclusions, const QString& target, const paths::VolumeIdentities& volumes) {
  return coverage(exclusions, target, [&](const auto& pattern, const auto& actual) { return fileEffects(pattern, actual, volumes); });
}
ExclusionCoverage registryExclusionCoverage(const QStringList& exclusions, const QString& target, const std::function<std::uint32_t()>& currentControlSet) {
  return registryCoverage(exclusions, target, currentControlSet);
}
ExclusionCoverage registryExclusionCoverage(const std::vector<std::string>& exclusions, const QString& target,
                                            const std::function<std::uint32_t()>& currentControlSet) {
  return registryCoverage(exclusions, target, currentControlSet);
}
QString fileCommitSkipExclusion(const QStringList& exclusions, const QString& target, const paths::VolumeIdentities& volumes) {
  return commitSkipExclusion(exclusions, target,
                             [&](const auto& pattern, const auto& actual) { return fileEffects(pattern, actual, volumes).managementSkipsCommit; });
}
QString fileCommitSkipExclusion(const std::vector<std::string>& exclusions, const QString& target, const paths::VolumeIdentities& volumes) {
  return commitSkipExclusion(exclusions, target,
                             [&](const auto& pattern, const auto& actual) { return fileEffects(pattern, actual, volumes).managementSkipsCommit; });
}
QString registryCommitSkipExclusion(const QStringList& exclusions, const QString& target) {
  return commitSkipExclusion(exclusions, target, registrySkipsCommit);
}
QString registryCommitSkipExclusion(const std::vector<std::string>& exclusions, const QString& target) {
  return commitSkipExclusion(exclusions, target, registrySkipsCommit);
}
bool fileExclusionIntersectsDirectory(const QString& pattern, const QString& directory, const paths::VolumeIdentities& volumes) {
  const auto stored = paths::resolveVolumeFilePath(pattern, volumes);
  const auto target = paths::resolveVolumeFilePath(directory, volumes);
  if (stored && target) {
    if (stored->volume != target->volume) return false;
    return fileExclusionIntersectsDirectory(stored->relativePath, target->relativePath);
  }
  if (fileExclusionCoverage(QStringList{pattern}, directory, volumes).preventsCommit()) return true;
  // An actual descendant has a normal object name. A management precheck may
  // remove one final pattern separator; it must not clean repeated separators
  // or dot segments into a different exclusion.
  auto root = pattern;
  if (root.endsWith(u'\\')) root.chop(1);
  if (root != paths::normalize(root) || !paths::contains(directory, root)) return false;
  return true;
}
}  // namespace uwf::app
