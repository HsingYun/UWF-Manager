// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <string_view>

namespace uwf::app {

inline constexpr std::string_view kFileStagingRegistryRoot = R"(HKEY_LOCAL_MACHINE\SOFTWARE\HsingYun\UWF Manager)";
inline constexpr std::string_view kFileStagingRegistryKey = R"(HKEY_LOCAL_MACHINE\SOFTWARE\HsingYun\UWF Manager\FileStaging)";
inline constexpr std::string_view kFileStagingEntriesValue = "Entries";

[[nodiscard]] bool isFileStagingRegistryRoot(const QString& key);

}  // namespace uwf::app
