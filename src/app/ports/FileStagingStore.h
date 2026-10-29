// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string_view>

#include "app/api/FileStagingTypes.h"

namespace uwf::app {

inline constexpr std::string_view kFileStagingRegistryRoot = R"(HKEY_LOCAL_MACHINE\SOFTWARE\HsingYun\UWF Manager)";
inline constexpr std::string_view kFileStagingRegistryKey = R"(HKEY_LOCAL_MACHINE\SOFTWARE\HsingYun\UWF Manager\FileStaging)";
inline constexpr std::string_view kFileStagingEntriesValue = "Entries";

// Persistent snapshots only. A failed replacement may have written part of the
// operation; callers must re-observe storage before accepting further edits.
class FileStagingStore {
 public:
  virtual ~FileStagingStore() = default;
  [[nodiscard]] virtual QList<FileStagingEntry> load() const = 0;
  virtual void replace(const QList<FileStagingEntry>& entries) = 0;
};

}  // namespace uwf::app
