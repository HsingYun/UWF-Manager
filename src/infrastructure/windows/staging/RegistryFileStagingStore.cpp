// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/staging/RegistryFileStagingStore.h"

#include "application/staging/persistence/FileStagingRegistry.h"
#include "infrastructure/windows/registry/RegistryCommitPlan.h"
#include "infrastructure/windows/registry/RegistryKey.h"
#include "infrastructure/windows/staging/RegistryFileStagingCodec.h"

namespace uwf::windows {
QList<app::FileStagingEntry> RegistryFileStagingStore::load() const {
  const auto records = regkey::readMultiString(app::kFileStagingRegistryKey, app::kFileStagingEntriesValue);
  return records ? decodeFileStagingEntries(*records) : QList<app::FileStagingEntry>{};
}
void RegistryFileStagingStore::write(const QList<app::FileStagingEntry>& entries) {
  regkey::writeMultiString(app::kFileStagingRegistryKey, app::kFileStagingEntriesValue, encodeFileStagingEntries(entries));
}
void RegistryFileStagingStore::erase() { regkey::deleteTree(app::kFileStagingRegistryRoot); }
std::vector<app::RegistryCommitTarget> RegistryFileStagingStore::deletionPlan() const {
  return windows::planRegistryTreeDeletion(std::string(app::kFileStagingRegistryRoot));
}
}  // namespace uwf::windows
