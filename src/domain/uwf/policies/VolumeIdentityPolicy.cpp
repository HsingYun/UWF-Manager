// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/policies/VolumeIdentityPolicy.h"

#include <stdexcept>

#include "domain/uwf/contracts/Types.h"
#include "foundation/paths/DriveLetter.h"

namespace uwf::core {
std::string volumeIdentity(const std::string_view volume) { return compat::normalizeVolumeId(std::string(volume)); }
namespace {
bool sameIdentity(const std::string& first, const std::string& second) {
  return !first.empty() && !second.empty() && volumeIdentity(first) == volumeIdentity(second);
}
template <class Match>
const VolumeRecord* select(const SessionSnapshot& session, Match&& match) {
  const VolumeRecord* selected = nullptr;
  for (const auto& record : session.volumes) {
    if (!match(record)) continue;
    if (selected) throw std::runtime_error("Multiple UWF configurations apply to the selected volume");
    selected = &record;
  }
  return selected;
}
}  // namespace
std::string volumeConfigurationAddress(const VolumeRecord& volume) { return drive::normalize(volume.physicalDriveLetter.value_or(std::string{})); }
const VolumeRecord* findVolumeConfiguration(const SessionSnapshot& session, const VolumeInfo& physical) {
  return select(session, [&](const auto& record) {
    if (!record.volumeName.empty() && !physical.volumeName.empty()) return sameIdentity(record.volumeName, physical.volumeName);
    const auto address = volumeConfigurationAddress(record);
    return !address.empty() && address == drive::normalize(physical.driveLetter);
  });
}
const VolumeRecord* findVolumeConfiguration(const SessionSnapshot& session, const std::string_view target) {
  if (const auto id = compat::parseVolumeId(std::string(target)))
    return select(session, [&](const auto& record) { return compat::parseVolumeId(record.volumeName) == id; });
  const auto address = drive::normalize(std::string(target));
  if (address.empty()) return nullptr;
  return select(session, [&](const auto& record) { return volumeConfigurationAddress(record) == address; });
}
std::string volumeConfigurationTarget(const VolumeRecord* next, const VolumeInfo& physical) {
  if (next)
    if (const auto id = compat::parseVolumeId(next->volumeName)) return *id;
  if (!physical.driveLetter.empty()) return drive::normalize(physical.driveLetter);
  return compat::parseVolumeId(physical.volumeName).value_or(std::string{});
}
}  // namespace uwf::core
