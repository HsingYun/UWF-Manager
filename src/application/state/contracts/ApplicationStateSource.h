// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <stop_token>

#include "application/state/contracts/ApplicationState.h"
#include "application/uwf/contracts/UwfCapability.h"

namespace uwf::app {
// Prepare owned worker inputs on the caller thread. A read returns a complete
// observation or throws; it must not retain the source or GUI objects by reference.
class ApplicationStateSource {
 public:
  using ReadOperation = std::function<ApplicationState(std::stop_token)>;
  virtual ~ApplicationStateSource() = default;
  [[nodiscard]] virtual ReadOperation readOperation(UwfCapability capability) = 0;
  // Present already published UWF configuration; injected sources may retain
  // their existing full-read behavior until they provide a distinct projection.
  [[nodiscard]] virtual ReadOperation publishedOperation(UwfCapability capability) { return readOperation(capability); }
};
}  // namespace uwf::app
