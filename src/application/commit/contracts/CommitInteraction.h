// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <memory>

namespace uwf::app {
struct CommitRequest;
class CommitBackend;
// Requests and backends are owned values across asynchronous task boundaries.
struct CommitInteraction {
  std::function<void(const CommitRequest&, std::shared_ptr<CommitBackend>)> run;
};
}  // namespace uwf::app
