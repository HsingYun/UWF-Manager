// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>

namespace uwf::app {

enum class PreshutdownControlResult {
  NotApplicable,
  Acknowledged,
  Unacknowledged,
};

// A lost acknowledgement can follow a successful write, so failed power actions
// must attempt to revoke the skip token regardless of the markHandled result.
struct PreshutdownCommitControl {
  std::function<PreshutdownControlResult()> markHandled;
  std::function<PreshutdownControlResult()> markRequired;
};

}  // namespace uwf::app
