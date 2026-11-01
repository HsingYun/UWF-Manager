// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <variant>

#include "domain/uwf/contracts/Types.h"

namespace uwf::compat {
// Adapter observations used by a backend's own business steps. These values
// carry no solver decision, required-fact plan, or permission to skip execution.
struct VolumeAdmission {
  bool fixedMedia = false;
  bool protectionEligible = false;
  bool operator==(const VolumeAdmission&) const = default;
};
enum class VolumeAdmissionPurpose { Registration, Binding };
using VolumeAdmissionResult = std::variant<VolumeAdmission, Failure>;

enum class FileMaintenanceKind { Missing, File, EmptyDirectory, NonemptyDirectory };
struct FileMaintenanceFacts {
  std::size_t fullPathUtf16Units = 0;
  FileMaintenanceKind kind = FileMaintenanceKind::File;
  bool reserved = false;
};

namespace servicingStatus {
// UwfServicingSvc runtime Status values. Keep observations as raw DWORD values
// so an unrecognized provider status remains distinguishable from these states.
inline constexpr std::uint32_t notStarted = 0, started = 1, completed = 2, failed = 3;
}  // namespace servicingStatus

struct ServicingAdmissionFacts {
  std::uint32_t status = servicingStatus::notStarted;
  std::optional<bool> savedFilter;
  bool operator==(const ServicingAdmissionFacts&) const = default;
};
using ServicingAdmissionResult = std::variant<ServicingAdmissionFacts, Failure>;

}  // namespace uwf::compat
