// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/staging/persistence/FileStagingRegistry.h"

#include "application/identity/contracts/ObjectIdentity.h"
#include "application/staging/contracts/FileStagingStore.h"
#include "foundation/paths/RegistryPath.h"

namespace uwf::app {

bool isFileStagingRegistryRoot(const QString& key) {
  const auto root = QString::fromLatin1(kFileStagingRegistryRoot.data(), static_cast<qsizetype>(kFileStagingRegistryRoot.size()));
  return equalObjectNames(QString::fromStdString(regkey::expandHive(key.toStdString())), root);
}

}  // namespace uwf::app
