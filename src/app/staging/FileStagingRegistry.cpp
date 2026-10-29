// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingRegistry.h"

#include "app/ports/FileStagingStore.h"
#include "shared/RegistryPath.h"

namespace uwf::app {

bool isFileStagingRegistryRoot(const QString& key) {
  return QString::compare(QString::fromStdString(regkey::normalize(key.toStdString())),
                          QString::fromStdString(regkey::normalize(std::string(kFileStagingRegistryRoot))), Qt::CaseInsensitive) == 0;
}

}  // namespace uwf::app
