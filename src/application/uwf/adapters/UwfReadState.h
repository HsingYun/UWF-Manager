// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/uwf/contracts/UwfCapability.h"
#include "domain/uwf/contracts/Backend.h"
#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {
[[nodiscard]] UwfCapability probeUwfCapability(const compat::Uwf& system);
[[nodiscard]] core::OverlayType overlayTypeFromUwf(std::uint32_t type);
[[nodiscard]] core::VolumeRecord toVolumeRecord(const compat::Volume& volume);
[[nodiscard]] std::string toUwfFilePath(const std::string& path, const std::string& driveLetter);
// Project published configuration and an explicitly sampled runtime usage without I/O.
// File exclusions retain their original provider patterns under the owning GUID;
// display addresses are a separate consumer projection.
[[nodiscard]] core::UwfSnapshot publishedUwfSnapshot(const compat::UwfStateFacts& state, UwfCapability capability, bool elevated, compat::OverlayUsage usage);
void publishUwfPhysicalState(compat::UwfStateFacts& state, const std::vector<core::VolumeInfo>& volumes);
// Refresh configuration and then resolve each session's configured letter using
// this already collected complete inventory. No second physical read is made.
[[nodiscard]] compat::Response refreshUwfStateWithPhysicalInventory(const compat::Uwf& system, const std::vector<core::VolumeInfo>& volumes,
                                                                    const compat::Context& context = {});
}  // namespace uwf::app
