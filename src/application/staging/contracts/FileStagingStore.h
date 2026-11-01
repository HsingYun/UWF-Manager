// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string_view>

#include "application/staging/contracts/FileStagingTypes.h"

namespace uwf::app {

inline constexpr std::string_view kFileStagingRegistryRoot = R"(HKEY_LOCAL_MACHINE\SOFTWARE\HsingYun\UWF Manager)";
inline constexpr std::string_view kFileStagingRegistryKey = R"(HKEY_LOCAL_MACHINE\SOFTWARE\HsingYun\UWF Manager\FileStaging)";
inline constexpr std::string_view kFileStagingEntriesValue = "Entries";

// Failed replacement may leave partial effects; re-observe before accepting edits.
// OperationError describes the whole replacement: None proves no mutation was
// submitted; attempted storage/persistence writes yield Uncertain on failure.
// Preserve its diagnostic. Unclassified exceptions provide no effect guarantee.
class FileStagingStore {
 public:
  virtual ~FileStagingStore() = default;
  [[nodiscard]] virtual QList<FileStagingEntry> load() const = 0;
  virtual void replace(const QList<FileStagingEntry>& entries) = 0;
};

}  // namespace uwf::app
