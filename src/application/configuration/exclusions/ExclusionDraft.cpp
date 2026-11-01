// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/exclusions/ExclusionDraft.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "application/configuration/contracts/ConfiguredExclusionIdentity.h"
#include "application/paths/ExclusionCoverage.h"
#include "domain/uwf/contracts/Types.h"
#include "foundation/paths/DriveLetter.h"

namespace uwf::app {

namespace {
void sortList(QStringList& list) {
  QSet<QString> seen;
  list.removeIf([&](const QString& path) {
    const auto key = configuredExclusionKey(path);
    if (seen.contains(key)) return true;
    seen.insert(key);
    return false;
  });
  std::stable_sort(list.begin(), list.end(), [](const QString& a, const QString& b) { return a.compare(b, Qt::CaseInsensitive) < 0; });
}
}  // namespace

void ExclusionDraft::setBaseline(QStringList current, QStringList next) {
  m_current = std::move(current);
  m_next = std::move(next);
  m_added.clear();
  m_removed.clear();
  qualifyFilePaths();
  rebuildIdentityIndex();
}

void ExclusionDraft::qualifyFilePaths() {
  for (auto* list : {&m_current, &m_next}) {
    if (m_fileIdentity)
      for (auto& path : *list) path = qualifyFileExclusionPath(path, m_selectedDrive);
    sortList(*list);
  }
  if (!m_fileIdentity) return;
  for (auto* set : {&m_added, &m_removed}) {
    QSet<QString> qualified;
    for (const auto& path : std::as_const(*set)) qualified.insert(qualifyFileExclusionPath(path, m_selectedDrive));
    *set = std::move(qualified);
  }
}

void ExclusionDraft::rebuildIdentityIndex() {
  const auto index = [&](const auto& entries, auto& result) {
    result.clear();
    for (const auto& path : entries) result[identityKey(path)].append(path);
  };
  index(m_next, m_nextByIdentity);
  index(m_added, m_addedByIdentity);
  index(m_removed, m_removedByIdentity);
}

void ExclusionDraft::setFileIdentityContext(paths::VolumeIdentities volumes, std::string volumeId, QString selectedDrive) {
  m_volumes = std::move(volumes);
  m_volumeId = compat::parseVolumeId(volumeId).value_or(std::string{});
  m_selectedDrive = QString::fromStdString(drive::normalize(selectedDrive.toStdString()));
  m_fileIdentity = true;
  qualifyFilePaths();
  rebuildIdentityIndex();
}
QString ExclusionDraft::identityKey(const QString& path) const {
  if (m_fileIdentity)
    if (const auto resolved = paths::resolveVolumeFilePath(path, m_volumes, m_volumeId)) return configuredExclusionKey(paths::volumeFilePath(*resolved));
  return configuredExclusionKey(path);
}
bool ExclusionDraft::equal(const QString& first, const QString& second) const { return identityKey(first) == identityKey(second); }
QStringList ExclusionDraft::effectiveFileExclusions() const {
  if (!m_fileIdentity) return {};
  QStringList result;
  QSet<QString> seen;
  const auto appendUnique = [&](const QString& path) {
    if (const auto key = identityKey(path); !seen.contains(key)) {
      seen.insert(key);
      result.append(path);
    }
  };
  // Current exclusions still affect commits until restart; pending removals
  // affect Next only. The source records retain their provider spelling.
  for (const auto& path : m_current) appendUnique(path);
  for (const auto& path : m_next)
    if (!isRemoved(path)) appendUnique(path);
  for (const auto& path : m_added) appendUnique(path);
  return result;
}
std::optional<paths::VolumeFilePath> ExclusionDraft::fileAddress(const QString& path) const {
  if (!m_fileIdentity) return std::nullopt;
  if (!m_volumeId.empty()) {
    const auto address = paths::resolveVolumeFilePath(path, m_volumes, m_volumeId);
    return address && address->volume == m_volumeId ? address : std::nullopt;
  }
  if (m_selectedDrive.isEmpty() && path.startsWith(u'\\') && !path.startsWith(QStringLiteral("\\\\")))
    return paths::VolumeFilePath{{}, path};
  const auto address = paths::splitVolumeFilePath(path);
  return address && !m_selectedDrive.isEmpty() && QString::fromStdString(address->volume) == m_selectedDrive ? address : std::nullopt;
}
QString ExclusionDraft::filePattern(const QString& path) const {
  if (!m_fileIdentity) return path;
  if (const auto address = fileAddress(path); address && !address->volume.empty()) return address->relativePath;
  throw std::invalid_argument("The file exclusion address cannot be assigned to this volume");
}

bool ExclusionDraft::add(const QString& p) {
  const auto key = identityKey(p);
  const auto restored = m_removedByIdentity.take(key);
  for (const auto& path : restored) m_removed.remove(path);
  if (!restored.isEmpty()) return true;
  if (!m_nextByIdentity.contains(key) && !m_addedByIdentity.contains(key)) {
    m_added.insert(p);
    m_addedByIdentity.insert(key, {p});
    return true;
  }
  return false;
}

bool ExclusionDraft::remove(const QString& p) {
  const auto key = identityKey(p);
  const auto added = m_addedByIdentity.take(key);
  for (const auto& path : added) m_added.remove(path);
  bool changed = !added.isEmpty();
  if (!m_nextByIdentity.contains(key) || (!m_fileIdentity && m_removedByIdentity.contains(key))) return changed;
  // File removals dispatch every observed address alias. Registry removals keep
  // the requested spelling, as the management-list contract permits.
  const auto sources = m_fileIdentity ? m_nextByIdentity.value(key) : QStringList{p};
  for (const auto& path : sources) {
    if (m_removed.contains(path)) continue;
    m_removed.insert(path);
    m_removedByIdentity[key].append(path);
    changed = true;
  }
  return changed;
}

void ExclusionDraft::reset() {
  m_added.clear();
  m_removed.clear();
  m_addedByIdentity.clear();
  m_removedByIdentity.clear();
  domainSecretKey.pendingNext.reset();
  tscal.pendingNext.reset();
}

void ExclusionDraft::togglePersistence(PersistenceFlagDraft& flag) {
  if (flag.pendingNext == true)
    flag.pendingNext.reset();
  else if (flag.baseNext)
    flag.pendingNext = false;
}

void ExclusionDraft::enablePersistence(PersistenceFlagDraft& flag) {
  if (flag.pendingNext == false)
    flag.pendingNext.reset();
  else if (!flag.baseNext)
    flag.pendingNext = true;
}

}  // namespace uwf::app
