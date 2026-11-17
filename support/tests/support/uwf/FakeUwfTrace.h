// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "simulation/uwf/FakeUwfBackend.h"

namespace uwf::test {
inline std::vector<compat::Request> writeRequests(const uwf::simulation::FakeUwfBackend& backend) {
  std::vector<compat::Request> result;
  for (const auto& entry : backend.trace())
    if (compat::isWrite(entry.request)) result.push_back(entry.request);
  return result;
}
}  // namespace uwf::test
