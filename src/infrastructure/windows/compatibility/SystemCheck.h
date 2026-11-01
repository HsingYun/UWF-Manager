// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Startup checks return structured facts; presentation owns localized messages.

#include <string>

namespace uwf::windows {

enum class CheckStatus {
  Ok,                 // Supported Windows 10/11 client edition.
  UnsupportedSystem,  // Unsupported family or edition.
};

struct SystemCheckResult {
  CheckStatus status = CheckStatus::Ok;
  std::string editionId;    // Registry EditionID.
  std::string productName;  // Display name corrected for the observed OS family.
};

SystemCheckResult runSystemChecks();
bool isElevated();
std::string uwfmgrPath();

}  // namespace uwf::windows
