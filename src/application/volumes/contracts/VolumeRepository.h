// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <string>
#include <vector>

#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {
// Live volume information and topology. Implementations must be usable from
// worker threads; failure never becomes an empty successful inventory.
class VolumeRepository {
 public:
  virtual ~VolumeRepository() = default;
  [[nodiscard]] virtual std::vector<core::VolumeInfo> read() const = 0;
  [[nodiscard]] virtual std::string driveForPath(const std::string& path) const = 0;
  // Complete mount paths, including directory mounts. A mount's host drive is
  // not a drive-letter alias for the mounted volume.
  [[nodiscard]] virtual std::vector<std::string> mountedPaths(const std::string& volume) const = 0;
  [[nodiscard]] virtual std::vector<std::string> mountedLetters(const std::string& volume) const = 0;
  [[nodiscard]] virtual std::string systemDrive() const = 0;
};
}  // namespace uwf::app
