// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QSet>
#include <QString>
#include <QStringList>
#include <optional>

#include "application/paths/PathPolicy.h"

namespace uwf::app {

struct PersistenceFlagDraft {
  bool baseCurrent = false;
  bool baseNext = false;
  std::optional<bool> pendingNext;
  [[nodiscard]] bool visible() const { return baseCurrent || baseNext || pendingNext == true; }
  [[nodiscard]] bool effectiveNext() const { return pendingNext.value_or(baseNext); }
};

class ExclusionDraft final {
 public:
  [[nodiscard]] const QStringList& current() const { return m_current; }
  [[nodiscard]] const QStringList& next() const { return m_next; }
  [[nodiscard]] const QSet<QString>& added() const { return m_added; }
  [[nodiscard]] const QSet<QString>& removed() const { return m_removed; }
  void setBaseline(QStringList current, QStringList next);
  PersistenceFlagDraft domainSecretKey;
  PersistenceFlagDraft tscal;

  // File addresses share a volume identity while their literal pattern suffix
  // retains management-list comparison semantics. Source records stay in the
  // lists above so removing one logical row can remove every actual spelling.
  void setFileIdentityContext(paths::VolumeIdentities volumes, std::string volumeId, QString selectedDrive = {});
  [[nodiscard]] const paths::VolumeIdentities& fileVolumes() const { return m_volumes; }
  [[nodiscard]] QString identityKey(const QString& path) const;
  // Resolve only an address owned by this editor. A legacy context without a
  // GUID can establish its selected drive spelling, never another volume alias.
  // Before binding, a single-root relative pattern has an empty volume; it may
  // be edited provisionally but cannot be extracted for provider dispatch.
  [[nodiscard]] std::optional<paths::VolumeFilePath> fileAddress(const QString& path) const;
  [[nodiscard]] QString filePattern(const QString& path) const;
  [[nodiscard]] bool equal(const QString& first, const QString& second) const;
  [[nodiscard]] QStringList effectiveFileExclusions() const;
  [[nodiscard]] bool isRemoved(const QString& path) const { return m_removedByIdentity.contains(identityKey(path)); }
  [[nodiscard]] bool add(const QString& path);
  [[nodiscard]] bool remove(const QString& path);
  void reset();
  void togglePersistence(PersistenceFlagDraft& flag);
  void enablePersistence(PersistenceFlagDraft& flag);

 private:
  void rebuildIdentityIndex();
  void qualifyFilePaths();
  QStringList m_current, m_next;
  QSet<QString> m_added, m_removed;
  // Retain every stored source spelling for dispatch, but resolve each identity
  // once when its baseline/context changes instead of for every imported item.
  QHash<QString, QStringList> m_nextByIdentity, m_addedByIdentity, m_removedByIdentity;
  paths::VolumeIdentities m_volumes;
  std::string m_volumeId;
  QString m_selectedDrive;
  bool m_fileIdentity = false;
};

}  // namespace uwf::app
