// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace uwf::drive {

class DriveLetterResolutionError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

// Returns drive-letter mounts only, excluding directory mounts. Missing or
// letterless volumes return an empty list; query failures throw.
[[nodiscard]] std::vector<std::string> mountedLetters(const std::string& volumeGuidPath);

// Includes directory mounts. Missing or unmounted volumes return an empty list.
[[nodiscard]] std::vector<std::string> mountedPaths(const std::string& volumeGuidPath);

// Splits literal drive roots; GUID paths require a mount lookup. UNC and
// relative paths return an empty string. Malformed or letterless GUID paths
// throw DriveLetterResolutionError; mount-query failures propagate.
std::string fromPath(const std::string& path);

// Returns the Windows volume's drive letter, or an empty string if unavailable.
std::string systemLetter();

}  // namespace uwf::drive
