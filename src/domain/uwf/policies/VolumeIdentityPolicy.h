// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <string_view>

#include "domain/uwf/model/UwfModel.h"

namespace uwf::core {
// Canonical spelling for a published volume identity; does not resolve mounts.
[[nodiscard]] std::string volumeIdentity(std::string_view volume);
// Management configuration lookup uses the published volume GUID for both
// sessions. BindByDriveLetter is provider metadata, not a saved-letter lookup rule.
// A saved DriveLetter is not evidence of a current mount.
[[nodiscard]] std::string volumeConfigurationAddress(const VolumeRecord& volume);
[[nodiscard]] const VolumeRecord* findVolumeConfiguration(const SessionSnapshot& session, const VolumeInfo& physical);
// A GUID names a stored configuration. A letter uses its published physical
// address, matching the native CLI's mount-to-GUID resolution without querying.
[[nodiscard]] const VolumeRecord* findVolumeConfiguration(const SessionSnapshot& session, std::string_view target);
[[nodiscard]] std::string volumeConfigurationTarget(const VolumeRecord* next, const VolumeInfo& physical);
}  // namespace uwf::core
