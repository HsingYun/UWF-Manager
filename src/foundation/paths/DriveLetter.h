// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>

namespace uwf::drive {

struct PathSplit {
  std::string letter;  // Uppercase letter plus colon, or empty when no literal drive prefix exists.
  std::string rest;    // Remaining path after stripping any device prefix and drive letter.
};

// Trim ASCII whitespace and extract an uppercase drive letter from a bare letter
// or drive-prefixed path, optionally prefixed by \\?\ or \\.\. Other inputs return empty.
// This is lexical parsing, not volume resolution.
std::string normalize(const std::string& raw);

// Split a literal drive prefix without trimming path characters or resolving
// volume GUIDs. Device prefixes are stripped even when no drive letter follows.
PathSplit split(const std::string& path);

}  // namespace uwf::drive
