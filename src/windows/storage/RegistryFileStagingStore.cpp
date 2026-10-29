// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "RegistryFileStagingStore.h"

#include "app/ports/RegistryCommitPlan.h"
#include "app/ports/RegistryKey.h"
#include "app/staging/FileStagingCodec.h"
namespace uwf::windows {
QList<app::FileStagingEntry> RegistryFileStagingStore::load() const {
  const auto records = regkey::readMultiString(app::kFileStagingRegistryKey, app::kFileStagingEntriesValue);
  return records ? app::decodeFileStagingEntries(*records) : QList<app::FileStagingEntry>{};
}
void RegistryFileStagingStore::replace(const QList<app::FileStagingEntry>& entries) {
  if (entries.isEmpty())
    regkey::deleteTree(app::kFileStagingRegistryRoot);
  else
    regkey::writeMultiString(app::kFileStagingRegistryKey, app::kFileStagingEntriesValue, app::encodeFileStagingEntries(entries));
}
bool RegistryFileStagingStore::exists() const { return regkey::keyExists(app::kFileStagingRegistryRoot); }
std::vector<app::RegistryCommitTarget> RegistryFileStagingStore::deletionPlan() const {
  return app::planRegistryTreeDeletion(std::string(app::kFileStagingRegistryRoot));
}
}  // namespace uwf::windows
