// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <utility>

#include "application/identity/contracts/ObjectIdentity.h"
#include "application/staging/contracts/FileStagingTypes.h"

namespace uwf::app {

// Staging paths must preserve their exact Unicode identity through UTF-8
// persistence and management commands; malformed UTF-16 is never repaired.
// This is syntax admission, independent of mounts, existence and binding.
[[nodiscard]] bool isAbsoluteLocalFileStagingPath(const QString& path);
using FileStagingEntryKey = std::pair<FileStagingKind, ObjectIdentity>;
[[nodiscard]] FileStagingEntryKey fileStagingEntryKey(const FileStagingEntry& entry);
// The same physical object can have multiple mounted display paths. A missing
// observation retains its display identity solely for recovery/removal.
[[nodiscard]] QString fileStagingObjectPath(const FileStagingEntry& entry);

}  // namespace uwf::app
